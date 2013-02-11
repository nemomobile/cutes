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

#include <stdexcept>

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
    JsError(QScriptEngine &engine, QString const &file);
    static QString errorMessage(QScriptEngine &engine, QString const &file);
};

QScriptValue findProperty(QScriptValue const&, QStringList const &);


class Script;

class Env : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QObject * script READ script);
    Q_PROPERTY(QString os READ os);
    Q_PROPERTY(QScriptValue env READ env);
    Q_PROPERTY(QScriptValue path READ path);

public:

    enum Position {
        Front,
        Back
    };

    Env(QCoreApplication &, QScriptEngine &, QScriptValue &);
    virtual ~Env() {}

    Q_INVOKABLE QScriptValue include(QString const&, bool is_reload = false);
    Q_INVOKABLE QScriptValue extend(QString const&);
    Q_INVOKABLE QScriptValue actor();
    Q_INVOKABLE void exit(int);

    QObject * script() const;
    QString os() const;
    QScriptValue env() const;
    QScriptValue path() const;

    bool shouldWait();
    QScriptValue args() const;
    QScriptValue load(QString const &, bool is_reload = false);
    void addSearchPath(QString const &, Position);
    void pushParentScriptPath(QString const&);
    
private:
    Env(Env const&);
    QString findFile(QString const &);

    QScriptEngine &engine_;
    QList<QDir> lib_path_;
    QMap<QString, QScriptValue> modules_;

    QScriptValue env_;
    QStringList path_;
    QStack<Script*> scripts_;
    QStringList args_;
    int actor_count_;
    bool is_waiting_exit_;
                         
private slots:
    void actorAcquired();
    void actorReleased();
};

class Script : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString fileName READ fileName);
    Q_PROPERTY(QString cwd READ cwd);
    Q_PROPERTY(QScriptValue args READ args);

public:
    Script(Env *parent, QString const&);
    virtual ~Script() {}

    QString fileName() const;
    QString cwd() const;
    QScriptValue args() const;

private:
    Env* env() { return static_cast<Env*>(parent()); }
    Env const* env() const { return static_cast<Env const*>(parent()); }
    QFileInfo info_;
};

Env *loadEnv(QCoreApplication &app, QScriptEngine &engine, QScriptValue global);
Env *loadEnv(QCoreApplication &app, QScriptEngine &engine);

}


#endif // _QSEXECUTE_QSENV_HPP_
