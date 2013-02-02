#include "QsExecute.hpp"

#include <QCoreApplication>
#include <QtScript>
#include <QStringList>
#include <QScopedPointer>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QtDebug>

#include <iostream>

namespace QsExecute {

#ifdef Q_OS_WIN32
static const char *os_name = "windows";
#elif defined(Q_OS_LINUX)
static const char *os_name = "linux";
#elif defined(Q_OS_MAC)
static const char *os_name = "macos";
#elif defined(Q_OS_UNIX)
static const char *os_name = "unix";
#else
static const char *os_name = "unknown";
#endif

Error::Error(QString const &s)
    : std::runtime_error(s.toStdString()),
      msg(s)
{ }

JsError::JsError(QScriptEngine &engine, QString const &file)
    : Error(errorMessage(engine, file))
{}

QString JsError::errorMessage(QScriptEngine &engine, QString const &file)
{
    QString res;
    QTextStream stream(&res);
    stream << "Exception: "
           << engine.uncaughtException().toString() << "\n"
           << file << ":" << engine.uncaughtExceptionLineNumber()
           << engine.uncaughtExceptionBacktrace().join("\n")
           << "\n";
    return res;

}

struct ScriptValueAccessor
{
    ScriptValueAccessor(QString const &name, QScriptValue v)
        : name(name), value(v) {}

    QString name;
    QScriptValue value;
};

ScriptValueAccessor operator <<
(ScriptValueAccessor to, ScriptValueAccessor const& child)
{
    to.value.setProperty(child.name, child.value);
    return to;
}

QScriptValue& operator <<
(QScriptValue &to, ScriptValueAccessor const& child)
{
    to.setProperty(child.name, child.value);
    return to;
}

static ScriptValueAccessor nameValue(QString const &n, QScriptValue v)
{
    return ScriptValueAccessor(n, v);
}

static QMap<QString, QScriptValue> saveProps
(QScriptValue const &parent, QStringList const &names)
{
    QMap<QString, QScriptValue> res;
    for (auto &name : names)
        res[name] = parent.property(name);
    return res;
}

static void restoreProps
(QScriptValue &parent, QMap<QString, QScriptValue> const& props)
{
    for (auto kv = props.begin(); kv != props.end(); ++kv)
        parent.setProperty(kv.key(), kv.value());
}

QScriptValue findProperty(QScriptValue const& root, QStringList const &path)
{
    QScriptValue res(root);
    for (auto &name : path)
        res = res.property(name);

    return res;
}

static QScriptValue findProperty
(QScriptEngine const& engine, QStringList const &path)
{
    return findProperty(engine.globalObject(), path);
}

static QScriptValue load(QString file_name, QScriptEngine &engine)
{
    static QSet<QString> loaded_files;

    QFileInfo file_info(file_name);
    file_name = file_info.absoluteFilePath();

    QString canonical_name = file_info.canonicalFilePath();
    if (loaded_files.contains(canonical_name))
        return QScriptValue();

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly))
        throw Error(QString("Can't open %1").arg(file_name));

    QString contents;
    contents.reserve(file_info.size());

    int line_nr = 2;

    QTextStream dst(&contents);
    QTextStream input(&file);
    QString first = input.readLine();
    if (!first.startsWith("#!")) {
        dst << first << "\n";
        line_nr = 1;
    }

    while (!input.atEnd())
        dst << input.readLine() << "\n";

    auto script = findProperty(engine, {"qtscript", "script"});

    auto saved_props = std::move(saveProps(script, {"filename", "cwd"}));
    script << nameValue("filename", engine.toScriptValue(file_name))
           << nameValue("cwd", engine.toScriptValue(file_info.absolutePath()));

    auto res = engine.evaluate(contents, file_name, line_nr);
    restoreProps(script, saved_props);

    if (engine.hasUncaughtException())
        throw JsError(engine, file_name);

    loaded_files.insert(canonical_name);
    return res;
}

static QList<QDir> lib_dirs;

static QString findFile(QScriptEngine &engine, QString const &file_name)
{
    auto script = findProperty(engine, {"app", "script"});
    QDir cwd(script.property("cwd").toString());
    QString res;

    if (QFileInfo(file_name).isRelative())
        res = cwd.filePath(file_name);

    if (QFileInfo(res).exists())
        return res;

    for (auto &d : lib_dirs) {
        res = d.filePath(file_name);
        if (QFileInfo(res).exists())
            return res;
    }
    return QString();
}

static QScriptValue jsLoad(QScriptContext *context, QScriptEngine *engine)
{
    QString file_name = context->argument(0).toString();
    try {
        QString full_name = findFile(*engine, file_name);
        if (full_name.isEmpty())
            throw Error(QString("Can't find file %1").arg(file_name));
        return load(full_name, *engine);
    } catch (JsError const &e) {
        return context->throwError
            (QString("Exception loading file %1").arg(file_name) + e.msg);
    } catch (Error const &e) {
        return context->throwError
            (QString("Exception loading file %1:\n%2").arg(file_name, e.msg));
    } catch (...) {
        return context->throwError
            (QString("Unknown error loading file %1:\n").arg(file_name));
    }
}

static QScriptValue jsUse(QScriptContext *context, QScriptEngine *engine)
{
    return engine->importExtension(context->argument(0).toString());
}

static QMap<QString, QVariant> mkEnv()
{
    QMap<QString, QVariant> res;
    QStringList env = QProcess::systemEnvironment();
    for (auto &item : env) {
        QStringList kv = item.split('=');
        auto s = kv.size();
        if (s)
            res.insert(kv[0], (s == 1) ? QString() : kv[1]);
    }
    return res;
}

typedef QScriptValue (*qscript_file_loader_type)(QString, QScriptEngine &);

qscript_file_loader_type setupEngine
(QCoreApplication &app, QScriptEngine &engine, QScriptValue global)
{
    auto env = std::move(mkEnv());

    auto lib_paths = env["QTSCRIPT_LIBRARY_PATH"].toString().split(":");
    for (auto path : {"/usr/lib/qt4/plugins", "/usr/lib32/qt4/plugins",
                "/usr/lib64/qt4/plugins"}) {
        if (QDir(path).exists())
            lib_paths.push_back(path);
    }

    for (auto &path : lib_paths)
        lib_dirs.push_back(QDir(path));
    app.setLibraryPaths(lib_paths);

    auto obj = [&engine](QString const &name)
        { return nameValue(name, engine.newObject()); };

    auto fn = [&engine]
        (QString const &name, QScriptEngine::FunctionSignature sig)
        { return nameValue(name, engine.newFunction(sig)); };

    //auto global = engine.globalObject();
    auto script_args = app.arguments();
    script_args.pop_front(); // remove interpreter name
    global << (obj("qtscript")
               << (obj("system")
                   << nameValue("os", engine.toScriptValue(QString(os_name)))
                   << nameValue("env", engine.toScriptValue(env))
                   << nameValue("path", engine.toScriptValue(lib_paths)))
               << fn("load", jsLoad)
               << fn("use", jsUse)
               << (obj("script")
                   << nameValue("args", engine.toScriptValue(script_args))))
           << obj("lib");
    return &load;
}

}
