#ifndef _QSEXECUTE_QSENV_HPP_
#define _QSEXECUTE_QSENV_HPP_

#include "Actor.hpp"

#include <QObject>
#include <QString>
#include <QSet>
#include <QJSEngine>
#include <QJSValue>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QStack>

#include <stdexcept>

// workaround for stupid moc namespaces handling
// namespace cutes {
// class Module;
// class Env;
// }

// typedef QsExecute::Env QsExecuteEnv;
// typedef QsExecute::Module QsExecuteModule;

namespace cutes {

class Module;
class Env;

typedef QMap<QString, QString> StringMap;

class Error : public std::runtime_error
{
public:
    Error(QString const &s);
    virtual ~Error() throw() {}

    QString msg;
};

class JsError : public Error
{
public:
    JsError(Env *env, QString const &file);
private:
    static QString errorMessage(Env *env, QString const &file);
};

QJSValue findProperty(QJSValue const&, QStringList const &);

// class Agent : public QJSEngineAgent
// {
// public:
//     Agent(Env *);

//     virtual void exceptionThrow
//     (qint64 scriptId, const QJSValue & exception, bool hasHandler);

// private:
//     Env *env_;
// };

// class Global : public QObject
// {
//     Q_OBJECT;

//     Q_PROPERTY(QsExecuteModule * module READ module);
//     Q_PROPERTY(QObject * qtscript READ qtscript);
//     Q_PROPERTY(QJSValue exports READ exports WRITE setExports);

// public:

//     Global(QCoreApplication &, QJSEngine &, QJSValue &);
//     virtual ~Global() {}

//     QJSValue exports() const;
//     void setExports(QJSValue);

//     QObject * qtscript() const;
//     Module * module() const;

//     Env *env() const;
// private:
//     Global(Global const&);
//     Env *env_;
// };

// typedef std::pair<unsigned long, QJSValue> Deferred;

// class EventQueue : public QObject
// {
//     Q_OBJECT;
// public:
//     EventQueue(unsigned long);

//     unsigned long enqueue(QJSValue const &);
//     bool remove(unsigned long);
//     QJSValue callNext();
//     bool empty() const;
//     bool clear();
//     bool callAll();
// private:
//     unsigned long serial_;
//     unsigned long max_len_;
//     unsigned long len_;
//     std::list<Deferred> events_;
// };

class Env : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(Module * module READ module);
    Q_PROPERTY(QString os READ os);
    Q_PROPERTY(StringMap env READ env);
    Q_PROPERTY(QStringList path READ path);

public:

    enum Position {
        Front,
        Back
    };

    Env(QObject *, QCoreApplication &, QJSEngine &);
    virtual ~Env() {}

    virtual bool event(QEvent *);

    Q_INVOKABLE QJSValue include(QString const&, bool is_reload = false);
    Q_INVOKABLE QJSValue extend(QString const&);
    Q_INVOKABLE QJSValue actor();
    Q_INVOKABLE void exit(int);
    Q_INVOKABLE void print(QString const&);
    // Q_INVOKABLE void defer(QJSValue const&);
    // Q_INVOKABLE void idle();

    Module *module() const;
    QString os() const;
    StringMap const& env() const;
    QStringList const& path() const;

    bool shouldWait();
    QStringList const& args() const;
    QJSValue load(QString const &, bool is_reload = false);
    void addSearchPath(QString const &, Position);
    void pushParentScriptPath(QString const&);

    QJSEngine &engine();
private:
    Env(Env const&);
    QString findFile(QString const &);

    QJSEngine &engine_;
    QMap<QString, Module*> modules_;

    StringMap env_;
    QStringList path_;
    QStack<Module*> scripts_;
    QStringList args_;
    int actor_count_;
    bool is_waiting_exit_;

private slots:
    void actorAcquired();
    void actorReleased();
};

class Module : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString id READ fileName);
    Q_PROPERTY(QString filename READ fileName);
    Q_PROPERTY(bool loaded READ loaded);
    Q_PROPERTY(QString cwd READ cwd);
    Q_PROPERTY(QStringList args READ args);
    Q_PROPERTY(QJSValue exports READ exports WRITE setExports);

public:
    Module(Env *parent, QString const&);
    Module(Env *parent, QString const&, QString const&);
    virtual ~Module() {}

    Q_INVOKABLE QJSValue require(QString const&, bool is_reload = false);

    QString fileName() const;
    bool loaded() const;
    QString cwd() const;
    QStringList const& args() const;
    QJSValue exports() const;
    void setExports(QJSValue);

    QJSValue load(QJSEngine &);

    QJSValue result_;
private:
    Env* env() { return static_cast<Env*>(parent()); }
    Env const* env() const { return static_cast<Env const*>(parent()); }
    QFileInfo info_;
    QJSValue exports_;
    bool is_loaded_;
    QString cwd_;
};

Env *loadEnv(QCoreApplication &app, QJSEngine &engine, QJSValue global);
Env *loadEnv(QCoreApplication &app, QJSEngine &engine);


template <typename T>
QJSValue anyToScriptValue(QJSEngine *engine, T* const &in)
{
    return engine->newQObject(in);
}

template <typename T>
void anyFromScriptValue(const QJSValue &object, T* &out)
{
    out = qobject_cast<T*>(object.toQObject());
}

// template <typename T>
// void anyMetaTypeRegister(QJSEngine *engine)
// {
//     qScriptRegisterMetaType
//         (engine, anyToScriptValue<T>, anyFromScriptValue<T>);
// }

} // namespace

#endif // _QSEXECUTE_QSENV_HPP_
