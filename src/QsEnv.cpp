#include "QsEnv.hpp"

#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QFile>

typedef QsExecute::Env QsExecuteEnv;
Q_DECLARE_METATYPE(QsExecuteEnv*);

typedef QsExecute::Script QsExecuteScript;
Q_DECLARE_METATYPE(QsExecuteScript*);

Q_DECLARE_METATYPE(QDir);

namespace QsExecute {

#ifdef Q_OS_WIN32
const char *os_name = "windows";
#elif defined(Q_OS_LINUX)
const char *os_name = "linux";
#elif defined(Q_OS_MAC)
const char *os_name = "macos";
#elif defined(Q_OS_UNIX)
const char *os_name = "unix";
#else
const char *os_name = "unknown";
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

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine, QScriptValue global)
{
    return new Env(app, engine, global);
}

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine)
{
    return loadEnv(app, engine, engine.globalObject());
}


Env::Env(QCoreApplication &app, QScriptEngine &engine, QScriptValue & global)
    : QObject(&engine)
    , engine_(engine)
    , actor_count_(0)
    , is_waiting_exit_(false)
{
    setObjectName("qtscript");
    auto script_args = app.arguments();
    script_args.pop_front(); // remove interpreter name

    auto env = std::move(mkEnv());

    auto paths = std::move(env["QTSCRIPT_LIBRARY_PATH"].toString().split(":"));
    for (auto path 
             : { "/usr/lib/qt4/plugins",
                 "/usr/lib32/qt4/plugins",
                 "/usr/lib64/qt4/plugins"})
        if (QDir(path).exists())
            paths.push_back(path);

    env_ = engine.toScriptValue(env);
    for (auto &path : paths)
        lib_path_.push_back(QDir(path));
    app.setLibraryPaths(paths);
    
    auto self = engine.newQObject(this);
    self.setProperty("lib", engine.newObject());
    global.setProperty("qtscript", self);
}

QObject * Env::script() const
{
    return !scripts_.empty() ? scripts_.top() : new QObject();
}

QScriptValue Env::actor()
{
    auto actor = new Actor(&engine_);
    connect(actor, SIGNAL(acquired()),
            this, SLOT(actorAcquired()));
    connect(actor, SIGNAL(released()),
            this, SLOT(actorReleased()));
    return engine_.newQObject(actor, QScriptEngine::ScriptOwnership);
}

void Env::actorAcquired()
{
    ++actor_count_;
}

void Env::actorReleased()
{
    if (--actor_count_ == 0 && is_waiting_exit_)
        exit(0);
}

bool Env::shouldWait()
{
    if (actor_count_) {
        is_waiting_exit_ = true;
    }
    return is_waiting_exit_;
}

void Env::exit(int rc)
{
    QCoreApplication::instance()->exit(rc);
}

QScriptValue Env::extension(QString const& extension)
{
    return engine_.importExtension(extension);
}

void Env::addSearchPath(QString const &path, Position pos)
{
    if (pos == Front)
        lib_path_.push_front(path);
    else
        lib_path_.push_back(path);
}

void Env::pushParentScriptPath(QString const &file_name)
{
    scripts_.push(new Script(this, file_name));
}

QString Env::findFile(QString const &file_name)
{
    if (QFileInfo(file_name).isAbsolute())
        return file_name;

    QString res;

    auto mkRelative = [&res, &file_name](QDir const& dir) {
        res = dir.filePath(file_name);
        return (QFileInfo(res).exists());
    };

    if (!scripts_.empty()) {
        auto script = scripts_.top();

        // first - relative to cwd
        // then - relative to file_name dir
        if (mkRelative(QDir(script->cwd())))
            return res;
    }
    if (mkRelative(QDir(QFileInfo(file_name).path())))
        return res;

    // search in path
    for (auto &d : lib_path_)
        if (mkRelative(d))
            return res;

    return QString();
}

QScriptValue Env::include(QString const &file_name, bool is_reload)
{
    auto context = engine_.currentContext();
    try {
        return load(file_name, is_reload);
    } catch (JsError const &e) {
        return context->throwError
            (QString("Exception loading file %1:%2").arg(file_name, e.msg));
    } catch (Error const &e) {
        return context->throwError
            (QString("Exception loading file %1:\n%2").arg(file_name, e.msg));
    } catch (...) {
        return context->throwError
            (QString("Unknown error loading file %1:\n").arg(file_name));
    }
}

QScriptValue Env::load(QString const &script_name, bool is_reload)
{
    QString file_name = findFile(script_name);

    if (file_name.isEmpty())
        throw Error(QString("Can't find file %1").arg(script_name));

    Script *script = new Script(this, file_name);

    file_name = script->fileName();
    auto p = modules_.find(file_name);
    if (p != modules_.end() && !is_reload)
        return *p;

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly))
        throw Error(QString("Can't open %1").arg(file_name));

    QString contents;
    contents.reserve(file.size());

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

    scripts_.push(script);

    auto res = engine_.evaluate(contents, file_name, line_nr);
    scripts_.top()->deleteLater();
    scripts_.pop();

    if (engine_.hasUncaughtException())
        throw JsError(engine_, file_name);

    modules_[file_name] = res;
    return res;
}


QString Env::os() const
{
    return QString(os_name);
}

QScriptValue Env::env() const
{
    return env_;
}

QScriptValue Env::path() const
{
    return qScriptValueFromSequence(&engine_, lib_path_);
}

QScriptValue Env::args() const
{
    return qScriptValueFromSequence(&engine_, args_);
}


Script::Script(Env *parent, QString const& fname)
    : QObject(parent)
    , info_(fname)
{
    setObjectName("script");
}

QScriptValue Script::args() const
{
    return env()->args();
}

QString Script::cwd() const
{
    return info_.canonicalPath();
}

QString Script::fileName() const
{
    return info_.canonicalFilePath();
}


}
