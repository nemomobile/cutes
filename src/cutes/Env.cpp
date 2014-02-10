#include "util.hpp"
#include <cutes/util.hpp>

#include "Env.hpp"
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QLibrary>
#include <QJSValueIterator>
#include <QQmlContext>

#include <unistd.h>
#include <iostream>
// Q_DECLARE_METATYPE(QsExecuteEnv*);
// Q_DECLARE_METATYPE(QsExecuteModule*);
// Q_DECLARE_METATYPE(CutesModule*);

// Q_DECLARE_METATYPE(QDir);

Q_DECLARE_METATYPE(cutes::Env*);
//Q_DECLARE_METATYPE(cutes::StringMap);

namespace cutes {

static char const *error_converter_try = "%1 try {\n";

static char const *error_converter_catch =
    "} catch (e) { \n"
    "    if (e instanceof Error) throw e;\n"
    "    var res = new Error('Wrapped error: ' + e);\n"
    "    res.isWrapped = true;\n"
    "    res.originalError = e;\n"
    "    throw res;\n"
    "}; %1\n";

QString errorConverterTry(QString const &prolog)
{
    return QString(error_converter_try).arg(prolog);
}

QString errorConverterCatch(QString const &epilog)
{
    return QString(error_converter_catch).arg(epilog);
}

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

static bool is_trace = false;

bool isTrace()
{
    return is_trace;
}

Error::Error(QString const &s)
    : std::runtime_error(s.toStdString()),
      msg(s)
{ }

JsError::JsError(Env *env, QString const &file)
    : Error(errorMessage(env, file))
{}

QString JsError::errorMessage(Env *env, QString const &// file
                              )
{
    if (!env)
        return "";

    // auto &engine = env->engine();
    // // TODO
    QString res("");
    // QTextStream stream(&res);
    // stream << file << ":" << engine.uncaughtExceptionLineNumber()
    //        << ": exception" << engine.uncaughtException().toString()
    //        << "\nBacktrace:\n";
    // if (env->getDebug())
    //     stream << env->getBacktrace().join("\n") << "\n";
    // else
    //     stream << engine.uncaughtExceptionBacktrace().join("\n") << "\n";
    // stream.flush();
    return res;
}

/**
 * convert system environment from "name=value" to map
 *
 * @return system env map
 */
static QVariantMap mkEnv()
{
    QVariantMap res;
    QStringList env = QProcess::systemEnvironment();
    for (auto &item : env) {
        QStringList kv = item.split('=');
        auto s = kv.size();
        if (s)
            res.insert(kv[0], (s == 1) ? QString() : kv[1]);
    }
    return res;
}

Env *loadEnv(QCoreApplication &app, QJSEngine &engine, QJSValue /*global*/)
{
    return loadEnv(app, engine);
}

Env *loadEnv(QCoreApplication &app, QJSEngine &engine)
{
    auto res = new Env(&engine, app, engine);
    return res;
}

static js::VHandle fprintImpl(FILE *f, int first, v8::Arguments const &args)
{
    QTextStream out(f);
    auto engine = js::engine(args);
    auto len = args.Length();

    if (engine && len > first) {
        out << toQJSValue(*engine, args[first]).toString();
        for (int i = first + 1; i < len; ++i)
            out << " " << toQJSValue(*engine, args[i]).toString();
        out << '\n';
    }

    return js::VHandle();
}

static js::VHandle jsFPrint(v8::Arguments const &args)
{
    auto len = args.Length();
    if (!len)
        return js::VHandle();

    auto engine = js::engine(args);

    auto i = toQJSValue(*engine, args[0]).toVariant().toInt();
    FILE *f = (i == STDOUT_FILENO
               ? stdout : (i == STDERR_FILENO
                           ? stderr : nullptr));
    return fprintImpl(f, 1, args);
}

static js::VHandle jsPrint(v8::Arguments const &args)
{
    return fprintImpl(stdout, 0, args);
}

static js::VHandle jsReadline(v8::Arguments const &)
{
    QTextStream stream(stdin);
    return cutes::js::ValueToV8(stream.readLine());
}

static void setupEngine(QJSEngine &engine)
{
    // if it is created by some ecmascript engine there should be print()
    auto v8e = engine.handle();
    v8::Context::Scope cscope(v8e->context());

    engine.globalObject().setProperty("process", engine.newObject());
    if (engine.globalObject().property("print").isUndefined())
        js::Set(engine, engine.globalObject(), "print", jsPrint);

    js::Set(engine, engine.globalObject(), "fprint", jsFPrint);
    js::Set(engine, engine.globalObject(), "readline", jsReadline);
}

Env::Env(QObject *parent, QCoreApplication &app, QJSEngine &engine)
    : QObject(parent)
    , engine_(engine)
    , module_engine_(nullptr)
    , actor_count_(0)
    , is_waiting_exit_(false)
{
    //qRegisterMetaType<StringMap>("StringMap");
    setObjectName("cutes");

    args_ = app.arguments();
    args_.pop_front(); // remove interpreter name

    // to allow safe access to top w/o checking
    scripts_.push(new Module(this, "", QDir::currentPath()));

    auto env = std::move(mkEnv());

    auto env_paths = env["CUTES_LIBRARY_PATH"];
    is_trace = env["CUTES_TRACE"].toBool();
    // remove empty paths
    auto paths = std::move
        (filter
         (env_paths.toString().split(":"), [](QString const &v) {
             return v.size() > 0;
         }));

    auto plugin_path = QString(DQUOTESTR(CUTES_LIB_PATH));
    for (auto path : plugin_path.split(":", QString::SkipEmptyParts))
        if (QDir(path).exists())
            paths.push_back(path);

    env_ = std::move(env);
    for (auto &path : paths)
        path_.push_back(QDir(path).canonicalPath());

    if (isTrace()) trace() << "Path:" << path_;
    app.setLibraryPaths(path_);

    /// if qmlengine is used it is impossible to modify global object,
    /// so cutes is added to qml context
    auto qml_engine = dynamic_cast<QQmlEngine*>(&engine_);
    if (qml_engine) {
        qml_engine->rootContext()->setContextProperty("cutes", this);
    } else {
        setupEngine(engine);
        engine.globalObject().setProperty("cutes", engine.newQObject(this));
        // engine has non-frozen global object so using the same
        // engine to load modules
        module_engine_ = &engine;
    }
}

Env::~Env()
{
    cutes::js::isolateRelease();
}

Module *Env::current_module()
{
    return scripts_.top();
}

QJSValue Env::module()
{
    auto m = current_module();
    QQmlData *ddata = QQmlData::get(m, true);
    if (ddata) {
        ddata->indestructible = true;
        ddata->explicitIndestructibleSet = true;
    }

    return engine().newQObject(m);
}

QJSValue Env::actor()
{
    auto actor = new StdActor(&engine_);
    connect(actor, SIGNAL(acquired()),
            this, SLOT(actorAcquired()));
    connect(actor, SIGNAL(released()),
            this, SLOT(actorReleased()));
    return engine_.newQObject(actor);//, QJSEngine::ScriptOwnership);
}

void Env::actorAcquired()
{
    ++actor_count_;
}

void Env::actorReleased()
{
    if (--actor_count_ == 0 && is_waiting_exit_) {
        QCoreApplication::quit();
    }
}

bool Env::shouldWait()
{
    idle();
    if (actor_count_) {
        is_waiting_exit_ = true;
    }
    return is_waiting_exit_;
}

void Env::exit(int rc)
{
    QCoreApplication::instance()->exit(rc);
}

class EnvEvent : public QEvent
{
public:
    enum Type {
        Deferred = QEvent::User,
    };

    EnvEvent(QJSValue const& fn)
        : QEvent(static_cast<QEvent::Type>(Deferred))
        , fn_(fn)
    {}
    virtual ~EnvEvent() {}

    void call() {
        fn_.callWithInstance(fn_);
    }

private:
    QJSValue fn_;
};

// EventQueue::EventQueue(unsigned long max_len)
//     : serial_(0)
//     , max_len_(max_len)
//     , len_(0) // TODO c++11 should provide O(1) list::size(), check
// {
// }

// unsigned long EventQueue::enqueue(QJSValue const &fn)
// {
//     if (!fn.isCallable()) {
//         QString err("Can enqueue only function, got %1");
//         err.arg(fn.toString());
//         // TODO fn.engine()->currentContext()->throwError(err);
//         return 0;
//     }
//     if (len_ == max_len_) {
//         // QString err("Can enqueue, queue is full");
//         // TODO fn.engine()->currentContext()->throwError(err);
//         return 0;
//     }

//     events_.push_back(std::make_pair(++serial_, fn));
//     ++len_;
//     return serial_;
// }

// /// O(n), considered like rarely used
// bool EventQueue::remove(unsigned long id)
// {
//     auto p = std::find_if(events_.begin(), events_.end()
//                           , [&id](Deferred const &v) {
//                               return v.first == id;
//                           });
//     if (p == events_.end())
//         return false;

//     events_.erase(p);
//     --len_;
//     return true;
// }

// QJSValue EventQueue::callNext()
// {
//     if (empty())
//         return QJSValue();

//     auto &fn = events_.front().second;
//     auto res = fn.call({fn, fn.engine()->newArray(0)});
//     events_.pop_front();
//     --len_;
//     return res;
// }

// bool EventQueue::clear()
// {
//     if (empty())
//         return false;

//     events_.clear();
//     return true;
// }

// bool EventQueue::callAll()
// {
//     if (empty())
//         return false;
//     for (auto &v : events_) {
//         auto &fn = v.second;
//         fn.call({fn, fn.engine()->newArray(0)});
//     }
//     return true;
// }

// bool EventQueue::empty() const
// {
//     return events_.empty();
// }


/// defer function execution until event loop processes next event,
/// processing can be enforced by calling Env::idle()
void Env::defer(QJSValue const& fn)
{
    if (!fn.isCallable()) {
        QString err("Can defer only function, got %1");
        err.arg(fn.toString());
        // TODO engine_.currentContext()->throwError(err);
        return;
    }
    QCoreApplication::postEvent(this, new EnvEvent(fn), Qt::HighEventPriority);
}

/// process all queued events including deferred functions
void Env::idle()
{
    QCoreApplication::processEvents();
}

bool Env::event(QEvent *e)
{
    auto evType = static_cast<EnvEvent::Type>(e->type());
    if (evType != EnvEvent::Deferred)
        return QObject::event(e);

    auto deferred = static_cast<EnvEvent*>(e);
    deferred->call();
    return true;
}

QJSValue Env::extend(QString const& extension)
{
    auto parts = extension.split('.');
    auto len = parts.length();
    if (!len) {
        qWarning() << "Wrong extension name: '" << extension << "'";
        return QJSValue();
    }
    parts.last() = QString("libcutes-%1.so").arg(parts.last());
    auto rel_path = parts.join('/');
    auto full_path = findFile(rel_path);
    if (full_path.isEmpty()) {
        qWarning() << "Can't find extension: '" << rel_path << "'";
        return QJSValue();
    }
    QLibrary lib(full_path);
    if (!lib.load()) {
        qWarning() << "Can't load library: '" << full_path << "'";
        return QJSValue();
    }
    auto fn = reinterpret_cast<cutesRegisterFnType>(lib.resolve(cutesRegisterName()));
    if (!fn) {
        qWarning() << "Can't resolve symbol " << cutesRegisterName()
                   << " in '" << full_path << "'";
        return QJSValue();
    }
    return fn(&engine());
}

void Env::addSearchPath(QString const &path, Position pos)
{
    if (pos == Front)
        path_.push_front(path);
    else
        path_.push_back(path);
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
        return QFileInfo(res).exists();
    };

    auto script = scripts_.top();
    // first - relative to cwd
    if (mkRelative(QDir(script->cwd())))
        return res;

    // search in path
    for (auto &d : path_)
        if (mkRelative(d))
            return res;

    return QString();
}

QString Env::libPath() const
{
    QString res;
    QTextStream s(&res);
    s << QDir(scripts_.top()->cwd()).canonicalPath();
    for (auto &d : path_)
        s << ":" << d;
    return res;
}

QJSValue Env::require(QString const &file_name)
{
    return include(file_name, false);
}

QJSValue Env::include(QString const &file_name, bool is_reload)
{
    QString err_msg;
    try {
        return load(file_name, is_reload);
    } catch (JsError const &e) {
        err_msg = QString("Exception loading 1: %2").arg(file_name, e.msg);
    } catch (Error const &e) {
        err_msg = QString("Exception loading %1: %2").arg(file_name, e.msg);
    } catch (...) {
        err_msg = QString("Unspecified error loading %1").arg(file_name);
    }
    if (!err_msg.isEmpty()) {
        using namespace v8;
        ThrowException(Exception::Error(String::New(err_msg.toUtf8().data())));
    }
    return QJSValue();
}

QJSValue Env::load(QString const &script_name, bool is_reload)
{
    QString file_name = findFile(QFileInfo(script_name).suffix() != "js"
                                  ? script_name + ".js" : script_name);

    if (file_name.isEmpty()) {
        throw Error(QString("Can't find file %1. Search path: %2")
                    .arg(script_name, libPath()));
    }

    Module *script = new Module(this, file_name);

    file_name = script->fileName();
    auto p = modules_.find(file_name);
    if (p != modules_.end() && !is_reload) {
        script->deleteLater();
        return p.value()->exports();
    }

    auto scope = mk_scope
        ([this, script](){
            scripts_.push(script);
        }
        , [this]() {
            scripts_.pop();
        });

    /// qqmlengine is used and to supply "cutes" global to the invoked
    /// module new engine is invoked in the same thread with
    /// non-frozen global object
    if (!module_engine_) {
        module_engine_ = new QJSEngine(this);
        setupEngine(*module_engine_);
        auto self = module_engine_->newQObject(new EnvWrapper(this));
        module_engine_->globalObject().setProperty("cutes", self);

    }
    auto res = script->load(*module_engine_);
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

QVariantMap const& Env::env() const
{
    return env_;
}

void Env::setEnv(QString const &name, QVariant const &value)
{
    env_[name] = value;
}

QStringList const& Env::path() const
{
    return path_;
}

QStringList const& Env::args() const
{
    return args_;
}

QJSEngine &Env::engine()
{
    return engine_;
}

Module::Module(Env *parent, QString const& fname)
    : QObject(parent)
    , info_(fname)
    , exports_(parent->engine().newObject())
    , is_loaded_(false)
    , cwd_(info_.canonicalPath())
{
    setObjectName("script");
}

Module::Module(Env *parent, QString const& fname, QString const& cwd)
    : QObject(parent)
    , info_(fname)
    , exports_(parent->engine().newObject())
    , is_loaded_(false)
    , cwd_(cwd)
{
    setObjectName("script");
}

QJSValue Module::require(QString const& name)
{
    return env()->include(name, false);
}

bool Module::loaded() const
{
    return is_loaded_;
}

QStringList const& Module::args() const
{
    return env()->args();
}

QJSValue Module::exports() const
{
    return exports_;
}

void Module::setExports(QJSValue v)
{
    exports_ = v;
}


QString Module::cwd() const
{
    return cwd_;
}

QString Module::fileName() const
{
    return info_.canonicalFilePath();
}

QJSValue Module::load(QJSEngine &engine)
{
    auto file_name = fileName();
    QFile file(file_name);
    if (!file.open(QFile::ReadOnly))
        throw Error(QString("Can't open %1").arg(file_name));

    const QString prolog = errorConverterTry
        ("(function() { var module = cutes.module"
         ", exports = module.exports"
         ", require = module.require"
         ", __filename = module.filename;");

    const QString epilog = errorConverterCatch(" return exports; }).call(this)\n");
    QString contents;
    contents.reserve(file.size() + prolog.size() + epilog.size());

    int line_nr = 1;

    QTextStream dst(&contents);
    QTextStream input(&file);

    dst << prolog;
    QString first = input.readLine();
    if (!first.startsWith("#!")) {
        dst << first << "\n";
        line_nr = 0;
    }

    while (!input.atEnd())
        dst << input.readLine() << "\n";
    dst << epilog;

    if (isTrace()) trace() << "Load: " << file_name;
    auto res = engine.evaluate(contents, file_name, line_nr);
    if (res.isError()) {
        qWarning() << "Error loading " << file_name << ":" << res.toString();
        if (res.hasProperty("stack"))
            qWarning() << "Stack:" << res.property("stack").toString();
        return res;
    }
    setExports(res);
    is_loaded_ = true;
    return exports();
}

QString asString(QJSValue v)
{
    if (!v.isObject())
        return v.toString();

    QString res;
    QTextStream out(&res);
    QJSValueIterator it(v);
    while (it.hasNext()) {
        it.next();
        out << it.name() << ": " << it.value().toString() << ", ";
    }
    out.flush();
    return res;
}


QJSValue EnvWrapper::include(QString const& fname, bool is_reload)
{
    return env_->include(fname, is_reload);
}

QJSValue EnvWrapper::extend(QString const& fname)
{
    return env_->extend(fname);
}

QJSValue EnvWrapper::actor()
{
    return env_->actor();
}

void EnvWrapper::exit(int status)
{
    return env_->exit(status);
}

void EnvWrapper::defer(QJSValue const &fn)
{
    return env_->defer(fn);
}

void EnvWrapper::idle()
{
    return env_->idle();
}


QJSValue EnvWrapper::module()
{
    return env_->module();
}

QString EnvWrapper::os() const
{
    return env_->os();
}

QVariantMap const& EnvWrapper::env() const
{
    return env_->env();
}

QStringList const& EnvWrapper::path() const
{
    return env_->path();
}


}
