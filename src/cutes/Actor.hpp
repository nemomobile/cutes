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
#include <QJSValue>
#include <QThread>
#include <QEvent>
#include <QWaitCondition>
#include <QMutex>
#include <QStringList>
#if QT_VERSION < 0x050000
#include <QDeclarativeEngine>
#else
#include <QQmlEngine>
#endif
#include <QSharedPointer>

#include <memory>

namespace cutes
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
        Error,
        EndpointRemove
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
// handle does not delete endpoint directly but sends request for
// removal to the hosting Env it (QJSValue should be deleted in the
// corresponding Isolate)
typedef std::shared_ptr<Endpoint> endpoint_handle;

class Actor;
class Engine : public QObject
{
    Q_OBJECT;
public:
    Engine(Actor*);
    virtual ~Engine();

    void run();
    virtual bool event(QEvent *);

    void reply(QVariant const&, endpoint_handle, Event::Type);
    void error(QVariant const&, endpoint_handle);

signals:
    void onQuit();

private:
    void load(Load *);
    void processMessage(Message *);
    void processRequest(Request *);
    void processResult(QJSValue, endpoint_handle);
    void toActor(Event*);

    Actor *actor_;
    QJSEngine *engine_;
    QJSValue handler_;
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
    Actor(QJSEngine *engine = nullptr);
    virtual ~Actor();

protected:
    QString source() const;
    void setSource(QString const&);

    Q_INVOKABLE void send(QJSValue const&, QJSValue callbacks);
    Q_INVOKABLE void request(QString const&, QVariant, QJSValue callbacks);
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

    mutable QJSEngine *engine_;
private:

    void acquire();
    void release();
    void callback(Message*, QJSValue&);
    void execute(std::function<void()>);
    endpoint_handle endpoint_new(QJSValue);

    int unreplied_count_;
    QScopedPointer<WorkerThread> worker_;
    QMap<Endpoint*, endpoint_ptr> endpoints_;
    long cookie_;
};

class QmlActor : public Actor
{
    Q_OBJECT;
    // uses QUrl to allow declarative engine to resolve full path
    Q_PROPERTY(QUrl source READ source WRITE setSource);
public:
    QmlActor(QJSEngine *engine = nullptr);
    virtual ~QmlActor() {}

    QUrl source() const;
    void setSource(QUrl const&);
};

class StdActor : public Actor
{
    Q_OBJECT;
    // in QtScript using simple string as a source name
    Q_PROPERTY(QString source READ source WRITE setSource);
public:
    StdActor(QJSEngine *engine = nullptr);
    virtual ~StdActor() {}
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
