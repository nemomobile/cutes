#ifndef _QS_ACTOR_HPP_
#define _QS_ACTOR_HPP_
/*
 * QtScript Actor component
 *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jolla.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */

#include <QObject>
#include <QVariant>
#include <QString>
#include <QScriptValue>
#include <QThread>
#include <QEvent>
#include <QWaitCondition>
#include <QMutex>
#include <QStringList>
#include <QDeclarativeEngine>
#include <QSharedPointer>

namespace QsExecute
{

class Event : public QEvent
{
public:
    enum Type {
        LoadScript = QEvent::User,
        Message, /// plain message, actor invoked as function
        Request, /// actor method invokation
        Progress,
        Return,
        QuitThread,
        LoadException,
        Error
    };

    Event();
    virtual ~Event();

protected:
    Event(Type t);

};

class Actor;

class Engine;
class Load;
class Message;
class Request;

class Endpoint;
typedef QSharedPointer<Endpoint> endpoint_ptr;

class Actor;
class Engine : public QObject
{
    Q_OBJECT;
public:
    Engine(Actor*);
    virtual ~Engine();

    void run();
    virtual bool event(QEvent *);

    void reply(QVariant const&, endpoint_ptr, Event::Type);
    void error(QVariant const&, endpoint_ptr);

signals:
    void onQuit();

private:
    void load(Load *);
    void processMessage(Message *);
    void processRequest(Request *);
    void processResult(QScriptValue &, endpoint_ptr);
    void toActor(Event*);

    Actor *actor_;
    QScriptEngine *engine_;
    QScriptValue handler_;
    QWaitCondition cond_;
    QMutex mutex_;
};

class WorkerThread : protected QThread
{
    Q_OBJECT;
public:
    WorkerThread(Actor *, QString const &, QString const &);
    virtual ~WorkerThread();

    void run();
    void send(Message*);

private:
    Actor *actor_;
    QWaitCondition cond_;
    QMutex mutex_;
    QScopedPointer<Engine> engine_;
};


class Actor : public QObject
{
    Q_OBJECT;
public:
    Actor(QScriptEngine *engine = nullptr);
    virtual ~Actor();

protected:
    QString source() const;
    void setSource(QString const&);

    Q_INVOKABLE void send
    (QScriptValue const&
     , QScriptValue const& on_reply = QScriptValue()
     , QScriptValue const& on_error = QScriptValue()
     , QScriptValue const& on_progress = QScriptValue());

    Q_INVOKABLE void request
    (QString const&, QScriptValue const&
     , QScriptValue const& on_reply = QScriptValue()
     , QScriptValue const& on_error = QScriptValue()
     , QScriptValue const& on_progress = QScriptValue());

    Q_INVOKABLE void wait();
    Q_INVOKABLE void reload();

    virtual bool event(QEvent *);

signals:
    void error(QVariant const& error);
    void acquired();
    void released();

protected:
    QString src_;
    void progress(Message*);
    void reply(Message*);
    void error(Message*);
    WorkerThread *worker();

    mutable QScriptEngine *engine_;
private:

    void acquire();
    void release();
    void callback(Message*, QScriptValue&);
    void execute(std::function<void()>);

    int unreplied_count_;
    QScopedPointer<WorkerThread> worker_;
};

class DeclarativeActor : public Actor
{
    Q_OBJECT;
    // uses QUrl to allow declarative engine to resolve full path
    Q_PROPERTY(QUrl source READ source WRITE setSource);
public:
    DeclarativeActor(QScriptEngine *engine = nullptr);
    virtual ~DeclarativeActor() {}

    QUrl source() const;
    void setSource(QUrl const&);
};

class QtScriptActor : public Actor
{
    Q_OBJECT;
    // in QtScript using simple string as a source name
    Q_PROPERTY(QString source READ source WRITE setSource);
public:
    QtScriptActor(QScriptEngine *engine = nullptr);
    virtual ~QtScriptActor() {}
};

class QtScriptAdapter : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(QUrl qml READ qml WRITE setQml);
public:
    QtScriptAdapter() {}
    virtual ~QtScriptAdapter() {}

    QUrl qml() const;
    void setQml(QUrl const&);

private:
    QUrl qml_;
};

}

#endif // _QS_ACTOR_HPP_
