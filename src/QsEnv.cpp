#include "util.hpp"

#include "QsEnv.hpp"
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QFile>

#include <unistd.h>

Q_DECLARE_METATYPE(QsExecuteEnv*);
Q_DECLARE_METATYPE(QsExecuteModule*);

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

Agent::Agent(Env *env)
    : QScriptEngineAgent(&env->engine())
    , env_(env)
{}

void Agent::exceptionThrow
(qint64 scriptId, const QScriptValue &exception, bool hasHandler)
{
    auto eng = engine();
    if (eng) {
        auto ctx = engine()->currentContext();
        env_->saveBacktrace(ctx);
    }

    QScriptEngineAgent::exceptionThrow(scriptId, exception, hasHandler);
}

Error::Error(QString const &s)
    : std::runtime_error(s.toStdString()),
      msg(s)
{ }

JsError::JsError(Env *env, QString const &file)
    : Error(errorMessage(env, file))
{}

QString JsError::errorMessage(Env *env, QString const &file)
{
    if (!env)
        return "";

    auto &engine = env->engine();
    QString res;
    QTextStream stream(&res);
    stream << file << ":" << engine.uncaughtExceptionLineNumber()
           << ": exception" << engine.uncaughtException().toString()
           << "\nBacktrace:\n";
    if (env->getDebug())
        stream << env->getBacktrace().join("\n") << "\n";
    else
        stream << engine.uncaughtExceptionBacktrace().join("\n") << "\n";
    stream.flush();
    return res;
}

QScriptValue findProperty(QScriptValue const& root, QStringList const &path)
{
    QScriptValue res(root);
    for (auto &name : path)
        res = res.property(name);

    return res;
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

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine, QScriptValue old_global)
{
    auto global = new Global(app, engine, old_global);
    return global->env();
}

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine)
{
    return loadEnv(app, engine, engine.globalObject());
}

static QScriptValue jsPrintImpl
(QTextStream &out, int begin, QScriptContext *context, QScriptEngine *engine)
{
    auto len = context->argumentCount();
    if (len >= begin) {
        --len;
        for (int i = begin; i < len; ++i)
            out << context->argument(i).toString() << " ";

        out << context->argument(len).toString() << endl;
    }
    return engine->undefinedValue();
}

static QScriptValue jsPrintStdout(QScriptContext *context, QScriptEngine *engine)
{
    QTextStream out(stdout);
    return jsPrintImpl(out, 0, context, engine);
}

static QScriptValue jsFPrint(QScriptContext *context, QScriptEngine *engine)
{
    QTextStream out(stdout);
    auto res = engine->undefinedValue();
    auto len = context->argumentCount();
    if (!len)
        return res;
    auto dst = context->argument(0);
    if (dst.isNumber()) {
        auto i = dst.toInt32();
        FILE *f = (i == STDOUT_FILENO
                   ? stdout : (i == STDERR_FILENO
                               ? stderr : nullptr));
        if (!f)
            return res;

        QTextStream out(f);
        return jsPrintImpl(out, 1, context, engine);
    } else if (dst.isQObject()){
        QIODevice *dev = dynamic_cast<QIODevice*>(dst.toQObject());
        if (dev) {
            QTextStream out(dev);
            return jsPrintImpl(out, 1, context, engine);
        }

        qDebug() << dst.toString() << " is not QIODevice";
    } else {
        qDebug() << dst.toString() << " is not file nr or QIODevice";
    }
    return res;
}

static QScriptValue jsRequire(QScriptContext *context, QScriptEngine *engine)
{
    auto qtscript = findProperty(engine->globalObject(), {"qtscript"});
    auto include = findProperty(qtscript, {"include"});
    auto len = context->argumentCount();
    if (!len)
        return engine->undefinedValue();
    auto params = engine->newArray(1);
    auto name = context->argument(0).toString();
    params.setProperty(0, QScriptValue(name));
    return include.call(qtscript, params);
}

Global::Global(QCoreApplication &app, QScriptEngine &engine, QScriptValue & global)
    : QObject(&engine)
    , env_(new Env(this, app, engine))
{
    auto self = engine.newQObject(this);
    qScriptRegisterMetaType
        (&engine
         , anyToScriptValue<QsExecuteModule>
         , anyFromScriptValue<QsExecuteModule>);
    qScriptRegisterMetaType
        (&engine
         , anyToScriptValue<QsExecuteEnv>
         , anyFromScriptValue<QsExecuteEnv>);
    self.setPrototype(global);
    engine.setGlobalObject(self);
    self.setProperty("print", engine.newFunction(jsPrintStdout));
    self.setProperty("require", engine.newFunction(jsRequire));
    // process - used by node.js etc. modules
    self.setProperty("process", engine.newObject());
}

Env *Global::env() const
{
    return env_;
}

QScriptValue Global::exports() const
{
    auto module = static_cast<Module*>(env_->module());
    return module->exports();
}

void Global::setExports(QScriptValue v)
{
    auto module = static_cast<Module*>(env_->module());
    module->setExports(v);
}

QObject * Global::qtscript() const
{
    return env_;
}

Module * Global::module() const
{
    return env_->module();
}

Env::Env(Global *parent, QCoreApplication &app, QScriptEngine &engine)
    : QObject(parent)
    , engine_(engine)
    , agent_(nullptr)
    , actor_count_(0)
    , fprint_(engine.newFunction(jsFPrint))
    , is_waiting_exit_(false)
{
    setObjectName("qtscript");
    args_ = app.arguments();
    args_.pop_front(); // remove interpreter name

    scripts_.push(new Module(this, ""));
    auto env = std::move(mkEnv());

    auto paths = std::move(env["QTSCRIPT_LIBRARY_PATH"].toString().split(":"));
    // some hard-coded predefined paths, TODO avoid hard-coding :)
    for (auto path
             : { "/usr/share/cutes"
                 , "/usr/lib/qt4/plugins"
                 , "/usr/lib32/qt4/plugins"
                 , "/usr/lib64/qt4/plugins" })
        if (QDir(path).exists())
            paths.push_back(path);

    env_ = engine.toScriptValue(env);
    for (auto &path : paths) {
        lib_path_.push_back(QDir(path).canonicalPath());
    }
    app.setLibraryPaths(paths);
}

Module * Env::module() const
{
    return scripts_.top();
}

bool Env::getDebug() const
{
    return agent_ != nullptr;
}

void Env::setDebug(bool isEnabled)
{
    if (isEnabled) {
        if (!agent_) {
            agent_ = new Agent(this);
            engine_.setAgent(agent_);
        }
    } else if (agent_) {
        engine_.setAgent(nullptr);
    }
}

QScriptValue Env::getFPrint() const
{
    return fprint_;
}

QStringList const& Env::getBacktrace() const
{
    return backtrace_;
}


void Env::saveBacktrace(QScriptContext *ctx)
{
    backtrace_.clear();
    backtrace_ = ctx->backtrace();
}

QScriptValue Env::actor()
{
    auto actor = new QtScriptActor(&engine_);
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

QScriptValue Env::extend(QString const& extension)
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
    scripts_.push(new Module(this, file_name));
}

QString Env::findFile(QString const &file_name)
{
    if (QFileInfo(file_name).isAbsolute())
        return file_name;

    QString res;

    auto mkRelative = [&res, &file_name](QDir const& dir) {
        res = dir.filePath(file_name);
        if (!QFileInfo(res).exists())
            res = dir.filePath(file_name + ".js");
        return QFileInfo(res).exists();
    };

    auto script = scripts_.top();

    // first - relative to cwd
    if (mkRelative(QDir(script->cwd())))
        return res;

    // then - relative to file_name dir
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

    Module *script = new Module(this, file_name);

    file_name = script->fileName();
    auto p = modules_.find(file_name);
    if (p != modules_.end() && !is_reload) {
        script->deleteLater();
        return p.value()->exports();
    }

    auto scope = mk_scope
        ([this, script](){ scripts_.push(script); }
        , [this]() { scripts_.pop(); });
    auto res = script->load(engine_);
    if (p != modules_.end()) {
        p.value()->deleteLater();
    }
    modules_[file_name] = script;
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

QScriptEngine &Env::engine()
{
    return engine_;
}

Module::Module(Env *parent, QString const& fname)
    : QObject(parent)
    , info_(fname)
    , exports_(parent->engine().newObject())
    , is_loaded_(false)
{
    setObjectName("script");
}

QScriptValue Module::require(QString const& name, bool is_reload)
{
    return env()->include(name, is_reload);
}

bool Module::loaded() const
{
    return is_loaded_;
}

QScriptValue Module::args() const
{
    return env()->args();
}

QScriptValue Module::exports() const
{
    return exports_;
}

void Module::setExports(QScriptValue v)
{
    exports_ = v;
}


QString Module::cwd() const
{
    return info_.canonicalPath();
}

QString Module::fileName() const
{
    return info_.canonicalFilePath();
}

QScriptValue Module::load(QScriptEngine &engine)
{
    auto file_name = fileName();
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

    auto res = engine.evaluate(contents, file_name, line_nr);

    if (engine.hasUncaughtException())
        throw JsError(env(), file_name);
    is_loaded_ = true;
    return exports();
}

}
