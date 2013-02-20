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


namespace QsExecute
{

class Event : public QEvent
{
public:
    enum Type {
        LoadScript = QEvent::User,
        Message, /// plain message, actor invoked as function
        Request, /// actor method invokation
        Reply,
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

class Load : public Event
{
public:
    Load(QString const&, QString const&);
    virtual ~Load() {}

    QString src_;
    QString top_script_;
};

class Message : public Event
{
public:
    Message(QVariant const&, QScriptValue const&,
            Event::Type type);
    virtual ~Message() {}

    QVariant data_;
    QScriptValue cb_;
};

class Request : public Message
{
public:
    Request(QString const&, QVariant const&,
            QScriptValue const&, Event::Type type);
    virtual ~Request() {}

    QString method_name_;
};

class EngineException : public Event
{
public:
    EngineException(QScriptEngine const&);
    virtual ~EngineException() {}

    QVariant exception_;
    QStringList backtrace_;
};

class Actor;

class Engine;
class MessageContext : public QObject
{
    Q_OBJECT;
public:
    MessageContext(Engine *, QScriptValue);
    virtual ~MessageContext();

    Q_INVOKABLE void reply(QScriptValue);
private:
    Engine *engine_;
    QScriptValue cb_;
};

class Actor;
class Engine : public QObject
{
    Q_OBJECT;
public:
    Engine(Actor*);
    virtual ~Engine();

    void run();
    virtual bool event(QEvent *);

    void reply(QVariant const &, QScriptValue const &, Event::Type);
    void error(QVariant const &, QScriptValue const & = QScriptValue());

signals:
    void onQuit();

private:
    void load(Load *);
    void processMessage(Message *);
    void processRequest(Request *);
    void toActor(Event*);

    Actor *actor_;
    QScriptEngine *engine_;
    QScriptValue handler_;
    QWaitCondition cond_;
    QMutex mutex_;
};

class WorkerThread : public QThread
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
    Q_PROPERTY(QUrl source READ source WRITE setSource);
public:
    Actor(QScriptEngine *engine = nullptr);
    virtual ~Actor();

    QUrl source() const;
    void setSource(QUrl const&);

    Q_INVOKABLE void send(QScriptValue, QScriptValue cb = QScriptValue());
    Q_INVOKABLE void request
    (QString const&, QScriptValue, QScriptValue cb = QScriptValue());

    virtual bool event(QEvent *);

signals:
    void error(QVariant const& error);
    void acquired();
    void released();

protected:
    QUrl src_;
    void reply(Message*);

private:
    QScriptEngine *engine_;
    int unreplied_count_;
    QScopedPointer<WorkerThread> worker_;
};

}

#endif // _QS_ACTOR_HPP_
