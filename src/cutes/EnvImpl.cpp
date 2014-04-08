/**
 * @file Env.cpp
 * @brief Cutes environment implementation
 * @author (C) 2012-2014 Jolla Ltd. Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 * @copyright LGPL 2.1 http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */

#include "QmlAdapter.hpp"

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

Q_DECLARE_METATYPE(cutes::EnvImpl*);

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

static char const *std_vars_ =
    "var module = cutes.module"
    ", require = module.require"
    ", exports = module.exports"
    ", require = module.require"
    ", globals = cutes.globals"
    ", process = {}"
    ", __filename = module.filename;";

QString errorConverterTry(QString const &prolog)
{
    return QString(error_converter_try).arg(prolog);
}

QString errorConverterCatch(QString const &epilog)
{
    return QString(error_converter_catch).arg(epilog);
}

const char *os_name =
#ifdef Q_OS_WIN32
    "windows"
#elif defined(Q_OS_LINUX)
    "linux"
#elif defined(Q_OS_MAC)
    "macos"
#elif defined(Q_OS_UNIX)
    "unix"
#else
    "unknown"
#endif
    ;

static bool is_trace = false;

bool isTrace()
{
    return is_trace;
}

Error::Error(QString const &s)
    : std::runtime_error(s.toStdString()),
      msg(s)
{ }

class Globals
{
public:
    Globals(EnvImpl *e) : env_(e)
    {}

    void init();
    QString asParamNames() const;
    void pushBack(QJSValueList &) const;
    void setProperties(QJSValue &) const;
private:
    EnvImpl *env_;
    static const std::vector<char const*> names_;
    QMap<QString, QJSValue> globals_;
};

const std::vector<char const*> Globals::names_ = {{
        "print", "fprint"
    }};

void Globals::init()
{
    auto js_env = env_->this_;
    auto add_wrapper_to_global = [this, &js_env](char const *name) {
        auto prop = js_env.property(name);
        globals_.insert(name, env_->mkVariadicImpl(prop));
    };
    globals_.insert("cutes", js_env);
    std::for_each(names_.begin(), names_.end(), add_wrapper_to_global);
}

QString Globals::asParamNames() const
{
    QStringList keys(globals_.keys());
    return keys.join(", ");
}

void Globals::pushBack(QJSValueList &tgt) const
{
    for(auto it = globals_.begin(); it != globals_.end(); ++it)
        tgt.push_back(it.value());
}

void Globals::setProperties(QJSValue &tgt) const
{
    for(auto it = globals_.begin(); it != globals_.end(); ++it)
        tgt.setProperty(it.key(), it.value());
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

void EnvImpl::fprintVariant(QTextStream &out, QVariant const &v)
{
    auto t = (QMetaType::Type)v.type();
    auto printMapItem = [&out, this](QVariantMap::const_iterator it) {
        auto key = it.key();
        fprintVariant(out, QVariant(!key.isEmpty() ? key : QString("?")));
        out << ": ";
        fprintVariant(out, it.value());
    };
    switch (t) {
    case QMetaType::QString:
        out << '"' << v.toString() << '"';
        break;
    case QMetaType::QVariantMap: {
        out << "{";
        auto m = v.toMap();
        auto it = m.begin();
        if (it != m.end())
            printMapItem(it++);
        for (; it != m.end(); ++it) {
            out << ", ";
            printMapItem(it);
        }
        out << "}";
        break;
    }
    case QMetaType::QVariantList:
        out << "[";
        fprintInternal(out, v.toList(), ", ");
        out << "]";
        break;
    default:
        out << v.toString();
        break;
    }
}

void EnvImpl::fprintInternal(QTextStream &out, QVariantList const &l
                             , QString const &delim)
{
    if (l.empty())
        return;

    auto it = l.begin();
    fprintVariant(out, *it++);
    for (; it != l.end(); ++it) {
        out << delim;
        fprintVariant(out, *it);
    }
}

void EnvImpl::fprintImpl(QTextStream &out, QVariantList const &l)
{
    fprintInternal(out, l, " ");
    out << '\n';
}

void EnvImpl::fprint(QVariant const &data)
{
    auto l = data.toList();
    if (l.empty())
        return;

    auto out_id = l.front();
    l.pop_front();
    FILE *f;
    if (static_cast<QMetaType::Type>(out_id.type()) == QMetaType::QString) {
        auto s = out_id.toString();
        f = (s == "stdout"
             ? stdout : (s == "stderr"
                         ? stderr : nullptr));
    } else if (out_id.canConvert<unsigned>()) {
        auto i = out_id.toUInt();
        f = (i == STDOUT_FILENO
             ? stdout : (i == STDERR_FILENO
                         ? stderr : nullptr));
    } else {
        qWarning() << "Wrong stream id" << out_id;
        return;
    }
    QTextStream out(f);
    fprintImpl(out, l);
}

void EnvImpl::print(QVariant const &data)
{
    auto l = data.toList();
    if (l.empty())
        return;

    QTextStream out(stdout);
    fprintImpl(out, l);
}

void EnvImpl::trace(QVariant const &data)
{
    if (isTrace()) {
        auto l = data.toList();
        if (l.empty())
            return;
        QTextStream out(stderr);
        fprintImpl(out, l);
    }
}


EnvImpl::EnvImpl(QObject *parent, QCoreApplication &app, QJSEngine &engine)
    : QObject(parent)
    , engine_(engine)
    , this_(engine.newQObject(this))
    , globals_(new Globals(this))
    , actor_count_(0)
    , is_eval_(false)
    , is_waiting_exit_(false)
{
    if (isTrace()) tracer() << "New js environment for " << &engine
                            << " in " << QThread::currentThreadId();
    setObjectName("cutes");
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    args_ = app.arguments();
    if (args_.size())
        args_.pop_front(); // remove interpreter name

    // to allow safe access to top w/o checking
    auto m = new Module(this, "", QDir::currentPath());
    pushModule(m);

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

    if (isTrace()) tracer() << "Path:" << path_;
    app.setLibraryPaths(path_);

    auto qml_engine = dynamic_cast<QQmlEngine*>(&engine_);
    if (qml_engine) {
        /// if qmlengine is used it is impossible to modify global object,
        /// so cutes is added to qml context
        if (isTrace()) tracer() << qml_engine->rootContext() <<  " Set cutes to "
                                << this_.toString() << " for " << this;
        qml_engine->rootContext()->setContextProperty("cutes", this);
    } else {
        auto c = engine_.globalObject().property("cutes");
        if (c.isUndefined())
            engine_.globalObject().setProperty("cutes", this_);
    }

    globals_->init();
}

EnvImpl::~EnvImpl()
{
}

Module* EnvImpl::currentModule()
{
    return scripts_.top().first;
}

QJSValue EnvImpl::module()
{
    auto &m = scripts_.top();
    return m.second;
}

QString EnvImpl::getEngineName() const
{
    return "qt5";
}

bool EnvImpl::addGlobal(QString const &name, QJSValue const &value)
{
    auto qml_engine = dynamic_cast<QQmlEngine*>(&engine_);
    if (qml_engine) {
        // TODO try to understand the best way to do it
        qDebug() << "Can't add global " << name
                 << " to QQmlEngine: global object is frozen";
        return false;
    }
    engine_.globalObject().setProperty(name, value);
    return true;
}

QJSValue EnvImpl::actor()
{
    auto actor = new StdActor(&engine_);
    connect(actor, SIGNAL(acquired()),
            this, SLOT(actorAcquired()));
    connect(actor, SIGNAL(released()),
            this, SLOT(actorReleased()));
    QQmlEngine::setObjectOwnership(actor, QQmlEngine::CppOwnership);
    return engine_.newQObject(actor);
}

void EnvImpl::actorAcquired()
{
    ++actor_count_;
}

void EnvImpl::actorReleased()
{
    if (--actor_count_ == 0 && is_waiting_exit_) {
        if (isTrace()) tracer() << "Quit from waiting application";
        QCoreApplication::quit();
    }
}

QJSValue EnvImpl::evaluateLazy(QString const &code, QString const &, QJSValue &fn)
{
    if (fn.isUndefined()) {
        auto res = engine().evaluate(code);
        if (res.isError()) {
            qWarning() << "Error trying to evaluate js function"
                       << res.toString();
            return fn;
        }
        fn = res;
    }
    return fn;
}

QJSValue EnvImpl::callJsLazy
(QString const &code, QString const &name
 , QJSValue &refval, QJSValueList const &params)
{
    QJSValue res;
    QJSValue fn = evaluateLazy(code, name, refval);
    if (!fn.isCallable())
        return res;

    res = fn.call(params);
    if (isTrace()) tracer() << "callJsLazy " << name << " result: " << res.toString();
    if (res.isError())
        qWarning() << "Error returned by js function" << res.toString();
    return res;
}

void EnvImpl::addToObjectPrototype(QString const &name, QJSValue const &v)
{
    static const QString code =
        "(function() {\n"
        "    return function(name, obj) {\n"
        "        Object.prototype[name] = obj;\n"
        "    };\n"
        "}).call(this)\n";

    QJSValueList params;
    params.push_back(name);
    params.push_back(v);

    callJsLazy(code, "proto", obj_proto_enhance_, params);
}

QJSValue EnvImpl::throwJsError(QString const &msg)
{
    static const QString code = "(function(){ return function(data) {"
        "throw new Error(data); }}).call(this)";
    auto fn =  evaluateLazy(code, "throw_error", throw_fn_);
    if (!fn.isCallable())
        return QJSValue();

    QJSValueList params;
    params.push_back(msg);
    return fn.callAsConstructor(params);
}

/**
 * convert function fn accepting array as the argument to the function
 * accepting variable number of arguments. This is done because I do
 * not see a way how to implement native js function accepting
 * variable argument list w/o using private headers.
 *
 * Also members object members will be added as resulting function
 * members (to resemble class members).
 *
 * @param fn function to be wrapped
 * @param members members to be added to the resulting function
 *
 * @return resulting variadic function
 */
QJSValue EnvImpl::mkVariadicImpl(QJSValue const &fn
                             , QJSValue const &members
                             , QJSValue const &obj)
{
        static const QString code =
            "(function() { "
            "return function(fn, members, obj) {"
            "    var res = function() {"
            "        return fn.apply(obj, [[].slice.call(arguments)]);"
            "    };"
            "    if (members) {"
            "        for (var m in members) {"
            "            res[m] = members[m];"
            "        }"
            "    }"
            "    return res;"
            "}; }).call(this)\n";

    QJSValueList params;
    params.push_back(fn);
    params.push_back(members);
    params.push_back(obj);
    return callJsLazy(code, "bridge", cpp_bridge_fn_, params);
}

/// javascript wrapper for mkVariadicImpl
QJSValue EnvImpl::mkVariadic(QJSValue const &fn
                         , QJSValue const &members
                         , QJSValue const &obj)
{
    return mkVariadicImpl(fn, members, obj);
}

bool EnvImpl::shouldWait()
{
    idle();
    if (actor_count_) {
        is_waiting_exit_ = true;
    }
    if (isTrace()) tracer() << "shouldWait: " << is_waiting_exit_;
    return is_waiting_exit_;
}

void EnvImpl::exit(int rc)
{
    if (isTrace()) tracer() << "exit(" << rc << ")";
    auto app = QCoreApplication::instance();
    if (app) {
        Actor::quitAll();
        app->exit(rc);
    } else {
        qWarning() << "No QCoreApplication instance";
    }
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
/// processing can be enforced by calling EnvImpl::idle()
void EnvImpl::defer(QJSValue fn)
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
void EnvImpl::idle()
{
    QCoreApplication::processEvents();
}

bool EnvImpl::event(QEvent *e)
{
    auto evType = static_cast<EnvEvent::Type>(e->type());
    if (evType != EnvEvent::Deferred)
        return QObject::event(e);

    auto deferred = static_cast<EnvEvent*>(e);
    deferred->call();
    return true;
}

QJSValue EnvImpl::extend(QString const &extension)
{
    if (isTrace())
        qDebug() << "Extending v4 with " << extension;
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
    if (isTrace())
        tracer() << "Using " << full_path << " to extend";
    auto lib = new QLibrary(full_path, this);
    if (!lib->load()) {
        qWarning() << "Can't load library: '" << full_path;
        qWarning() << "Reason: " << lib->errorString();
        return QJSValue();
    }

    auto fn = reinterpret_cast<cutesRegisterFnType>
        (lib->resolve(cutesRegisterName()));
    if (!fn) {
        qWarning() << "Can't resolve symbol " << cutesRegisterName()
                   << " in '" << full_path << "'";
        return QJSValue();
    }
    auto obj = fn(&engine());
    libraries_.insert(extension, std::make_pair(lib, obj));
    return mkVariadicImpl(obj.property("create"), obj.property("members"));
}

void EnvImpl::addSearchPath(QString const &path, Position pos)
{
    if (isTrace()) tracer() << "Add search path " << path;
    if (pos == Env::Front)
        path_.push_front(path);
    else
        path_.push_back(path);
}

void EnvImpl::pushParentScriptPath(QString const &file_name)
{
    auto m = new Module(this, file_name);
    pushModule(m);
}

QString EnvImpl::findFile(QString const &file_name)
{
    if (isTrace()) tracer() << "Find " << file_name;
    if (QFileInfo(file_name).isAbsolute())
        return file_name;

    QString res;

    auto mkRelative = [&res, &file_name](QDir const& dir) {
        res = dir.filePath(file_name);
        return QFileInfo(res).exists();
    };

    auto script = scripts_.top().first;
    // first - relative to cwd
    if (mkRelative(QDir(script->cwd())))
        return res;

    // search in path
    for (auto &d : path_) {
        if (isTrace()) tracer() << "Find in " << d;
        if (mkRelative(d))
            return res;
    }
    return QString();
}

QString EnvImpl::libPath() const
{
    QString res;
    QTextStream s(&res);
    s << QDir(scripts_.top().first->cwd()).canonicalPath();
    for (auto &d : path_)
        s << ":" << d;
    return res;
}

QJSValue EnvImpl::require(QString const &file_name)
{
    return include(file_name, false);
}

QJSValue EnvImpl::include(QString const &file_name, bool is_reload)
{
    QString err_msg;
    try {
        return load(file_name, is_reload);
    } catch (Error const &e) {
        err_msg = QString("Exception loading %1: %2").arg(file_name, e.msg);
    } catch (...) {
        err_msg = QString("Unspecified error loading %1").arg(file_name);
    }
    // TODO Throw Js error
    return throwJsError(!err_msg.isEmpty() ? err_msg : "");
}

QJSValue EnvImpl::load(QString const &script_name, bool is_reload)
{
    QString file_name = findFile(QFileInfo(script_name).suffix() != "js"
                                  ? script_name + ".js" : script_name);

    if (file_name.isEmpty()) {
        throw Error(QString("Can't find file %1. Search path: %2")
                    .arg(script_name, libPath()));
    }

    Module *script = new Module(this, file_name);
    QQmlEngine::setObjectOwnership(script, QQmlEngine::CppOwnership);

    file_name = script->fileName();
    auto p = modules_.find(file_name);
    if (p != modules_.end() && !is_reload) {
        script->deleteLater();
        return p.value()->exports();
    }

    auto scope = mk_scope
        ([this, script](){
            pushModule(script);
        }
        , [this]() {
            scripts_.pop();
        });

    auto res = script->load();
    if (p != modules_.end()) {
        p.value()->deleteLater();
    }
    modules_[file_name] = script;
    return res;
}

void EnvImpl::pushModule(Module *m)
{
    QQmlEngine::setObjectOwnership(m, QQmlEngine::CppOwnership);
    scripts_.push(std::make_pair(m, engine_.newQObject(m)));
}

QJSValue EnvImpl::eval(QString const &line)
{
    if (!is_eval_) {
        auto global = engine_.globalObject();
        globals_->setProperties(global);
        auto s = engine_.evaluate(std_vars_);
        if (s.isError())
            qWarning() << "Can't evaluate std vars: " << s.toString();
        is_eval_ = true;
    }
    return engine_.evaluate(line);
}

QJSValue EnvImpl::globals() const
{
    return engine_.globalObject();
}

QString EnvImpl::os() const
{
    return QString(os_name);
}

QVariantMap const& EnvImpl::env() const
{
    return env_;
}

void EnvImpl::setEnv(QString const &name, QVariant const &value)
{
    env_[name] = value;
}

QStringList const& EnvImpl::path() const
{
    return path_;
}

QStringList const& EnvImpl::args() const
{
    return args_;
}

QJSEngine &EnvImpl::engine()
{
    return engine_;
}

Module::Module(EnvImpl *parent, QString const& fname)
    : QObject(parent)
    , info_(fname)
    , exports_(parent->engine().newObject())
    , is_loaded_(false)
    , cwd_(info_.canonicalPath())
{
    setObjectName("script");
}

Module::Module(EnvImpl *parent, QString const& fname, QString const& cwd)
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

QJSValue EnvImpl::executeModule(QTextStream &input
                                , QString const& file_name
                                , size_t reserve_size)
{
    static const QString prologFmt("(function(%1) { %2");
    const QString prolog = errorConverterTry
        (prologFmt.arg(globals_->asParamNames(), std_vars_));

    static const QString epilog = errorConverterCatch(" return exports; })\n");
    QString contents;
    contents.reserve(reserve_size + prolog.size() + epilog.size());

    int line_nr = 1;
    QTextStream dst(&contents);
    dst << prolog;
    QString first = input.readLine();
    if (!first.startsWith("#!")) {
        dst << first << "\n";
        line_nr = 0;
    }
    while (!input.atEnd())
        dst << input.readLine() << "\n";
    dst << epilog;

    auto res = engine_.evaluate(contents, file_name, line_nr);
    if (res.isError()) {
        qWarning() << "Error loading " << file_name << ":" << res.toString();
        if (res.hasProperty("stack"))
            qWarning() << "Stack:" << res.property("stack").toString();
        return res;
    }

    QJSValueList params;
    globals_->pushBack(params);
    res = res.call(params);
    if (res.isError()) {
        if (isTrace()) tracer() << "Caught error " << res.toString();
        QVariant msg = res.toString();
        if (res.isQObject() || res.isObject()) {
            QTextStream out(stderr);
            auto v = msgFromValue(res, JSValueConvert::Deep
                                  , JSValueConvertOptions::AllowError);
            out << "Exception evaluating " << file_name << ": ";
            fprintVariant(out, v);
            out << endl;
        } else {
            qWarning() << "Error evaluating " << file_name << ":" << res.toString();
        }
    }

    return res;
}

QJSValue Module::load()
{
    auto file_name = fileName();
    if (isTrace()) tracer() << "Load: " << file_name;
    QFile file(file_name);
    if (!file.open(QFile::ReadOnly))
        throw Error(QString("Can't open %1").arg(file_name));

    QTextStream input(&file);
    auto res = env()->executeModule(input, file_name, file.size());
    setExports(res);
    is_loaded_ = true;
    return exports();
}

EnvImpl * EnvImpl::create(QObject *env, QCoreApplication *app, QJSEngine *eng)
{
    if (!(app && eng))
        throw Error("Wrong app or engine was passed to Env ctor");

    return new EnvImpl(env, *app, *eng);
}


QUrl Adapter::qml() const
{
    return qml_;
}

void Adapter::setQml(QUrl const& url)
{
    if (isTrace()) tracer() << "Base url " << url;
    qml_ = url;
    auto env = getEnv();
    if (!env) {
        qWarning() << "Adapter:Env is null!";
        return;
    }
    env->pushParentScriptPath(url.path());
}

Adapter::Adapter()
{
    qRegisterMetaType<EnvImpl*>("EnvImpl");
}

QVariant Adapter::getEnvObj() const
{
    auto engine = qmlEngine(this);

    if (!engine) {
        qWarning() << "Adapter.engine is null!";
        return QVariant();
    }
    auto prop = engine->rootContext()->contextProperty("cutes");
    if (isTrace()) tracer() << engine->rootContext() << " Get adapter env: " << prop.toString();
    return prop;
}

EnvImpl * Adapter::getEnv() const
{
    return getEnvObj().value<EnvImpl*>();
}

}
