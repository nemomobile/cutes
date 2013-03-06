#ifndef _QSEXECUTE_QSENV_HPP_
#define _QSEXECUTE_QSENV_HPP_

#include "QsActor.hpp"

#include <QObject>
#include <QString>
#include <QSet>
#include <QScriptEngine>
#include <QScriptValue>
#include <QStringList>
#include <QtScript>
#include <QCoreApplication>
#include <QScriptEngineAgent>

#include <stdexcept>

// workaround for stupid moc namespaces handling
namespace QsExecute {
class Module;
class Env;
}

typedef QsExecute::Env QsExecuteEnv;
typedef QsExecute::Module QsExecuteModule;

namespace QsExecute {

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

QScriptValue findProperty(QScriptValue const&, QStringList const &);

class Agent : public QScriptEngineAgent
{
public:
    Agent(Env *);

    virtual void exceptionThrow
    (qint64 scriptId, const QScriptValue & exception, bool hasHandler);

private:
    Env *env_;
};

class Global : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QsExecuteModule * module READ module);
    Q_PROPERTY(QObject * qtscript READ qtscript);
    Q_PROPERTY(QScriptValue exports READ exports WRITE setExports);

public:

    Global(QCoreApplication &, QScriptEngine &, QScriptValue &);
    virtual ~Global() {}

    QScriptValue exports() const;
    void setExports(QScriptValue);

    QObject * qtscript() const;
    Module * module() const;

    Env *env() const;
private:
    Global(Global const&);
    Env *env_;
};

class Env : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QsExecuteModule * module READ module);
    Q_PROPERTY(QString os READ os);
    Q_PROPERTY(QScriptValue env READ env);
    Q_PROPERTY(QScriptValue path READ path);
    Q_PROPERTY(bool debug READ getDebug WRITE setDebug);
    Q_PROPERTY(QStringList backtrace READ getBacktrace);

public:

    enum Position {
        Front,
        Back
    };

    Env(Global *parent, QCoreApplication &, QScriptEngine &);
    virtual ~Env() {}

    Q_INVOKABLE QScriptValue include(QString const&, bool is_reload = false);
    Q_INVOKABLE QScriptValue extend(QString const&);
    Q_INVOKABLE QScriptValue actor();
    Q_INVOKABLE void exit(int);

    Module *module() const;
    QString os() const;
    QScriptValue env() const;
    QScriptValue path() const;
    QStringList const& getBacktrace() const;
    void setBacktrace(QStringList const&);

    bool getDebug() const;
    void setDebug(bool);


    bool shouldWait();
    QScriptValue args() const;
    QScriptValue load(QString const &, bool is_reload = false);
    void addSearchPath(QString const &, Position);
    void pushParentScriptPath(QString const&);

    QScriptEngine &engine();
private:
    Env(Env const&);
    QString findFile(QString const &);

    QScriptEngine &engine_;
    QList<QDir> lib_path_;
    QMap<QString, Module*> modules_;
    Agent *agent_;

    QStringList backtrace_;
    QScriptValue env_;
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
    Q_PROPERTY(QScriptValue args READ args);
    Q_PROPERTY(QScriptValue exports READ exports WRITE setExports);

public:
    Module(Env *parent, QString const&);
    virtual ~Module() {}

    Q_INVOKABLE QScriptValue require(QString const&, bool is_reload = false);

    QString fileName() const;
    bool loaded() const;
    QString cwd() const;
    QScriptValue args() const;
    QScriptValue exports() const;
    void setExports(QScriptValue);

    QScriptValue load(QScriptEngine &);

    QScriptValue result_;
private:
    Env* env() { return static_cast<Env*>(parent()); }
    Env const* env() const { return static_cast<Env const*>(parent()); }
    QFileInfo info_;
    QScriptValue exports_;
    bool is_loaded_;
};

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine, QScriptValue global);
Env *loadEnv(QCoreApplication &app, QScriptEngine &engine);

} // namespace

template <typename T>
QScriptValue anyToScriptValue(QScriptEngine *engine, T* const &in)
{
    return engine->newQObject(in);
}

template <typename T>
void anyFromScriptValue(const QScriptValue &object, T* &out)
{
    out = qobject_cast<T*>(object.toQObject());
}

#endif // _QSEXECUTE_QSENV_HPP_
