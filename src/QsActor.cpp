/*
 * QtScript Actor implementation
 *
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
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

#include "QsActor.hpp"
#include "QsActorEvents.hpp"
#include "QsEnv.hpp"
#include "QmlAdapter.hpp"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QScriptEngine>
#include <QDebug>
#include <QCoreApplication>
#include <QDeclarativeContext>

namespace QsExecute {

Endpoint::Endpoint(QScriptValue const& on_reply
                   , QScriptValue const& on_error)
    : on_reply_(on_reply)
    , on_error_(on_error)
{
}

static inline endpoint_ptr endpoint
(QScriptValue const& on_reply, QScriptValue const& on_error)
{
    return endpoint_ptr(new Endpoint(on_reply, on_error));
}

Actor::Actor(QScriptEngine *engine)
    : engine_(engine)
    , unreplied_count_(0)
{
}

Actor::~Actor()
{
}

QString Actor::source() const
{
    return src_;
}

void Actor::setSource(QString const& src)
{
    if (src == src_)
        return;

    if (!engine_) {
        qDebug() << "Can't get script engine."
                 << " Not qml engine and missing initialization?";
        return;
    }
    // cwd should be set to the same directory as for main engine
    auto script = static_cast<Module*>
        (findProperty
         (engine_->globalObject(), {"qtscript", "module"}).toQObject());

    worker_.reset(new WorkerThread(this, src, script->fileName()));

    src_ = src;
}

DeclarativeActor::DeclarativeActor(QScriptEngine *engine)
    : Actor(engine)
{}

QUrl DeclarativeActor::source() const
{
    return Actor::source();
}

void DeclarativeActor::setSource(QUrl const& src)
{
    // if Actor is created by QtScript environment it should have
    // engine_ set while declarative view does not provide direct
    // access to the engine and it will be initialized on first call
    // to this method
    if (!engine_)
        engine_ = getDeclarativeScriptEngine(qmlEngine(this));
    Actor::setSource(src.path());
}

QtScriptActor::QtScriptActor(QScriptEngine *engine)
    : Actor(engine)
{}

// QUrl QtScriptActor::source() const
// {
//     return Actor::source();
// }

// void QtScriptActor::setSource(QUrl const& src)
// {
//     Actor::setSource(src.path());
// }

WorkerThread *Actor::worker()
{
    if (!worker_)
        throw Error("Non-initialized worker");
    return worker_.data();
}

void WorkerThread::send(Message *msg)
{
    QCoreApplication::postEvent(engine_.data(), msg);
}

void Actor::send
(QScriptValue const &msg, QScriptValue const& on_reply
 , QScriptValue const& on_error)
{
    try {
        if (!unreplied_count_)
            emit acquired();

        ++unreplied_count_;
        worker()->send(new Message
                       (msg.toVariant(), endpoint(on_reply, on_error)
                        , Event::Message));
    } catch (Error const &e) {
        msg.engine()->currentContext()->throwError(e.msg);
    } catch (...) {
        msg.engine()->currentContext()->throwError("Unhandled error");
    }
}

void Actor::request
(QString const &method_name, QScriptValue const &msg
 , QScriptValue const& on_reply, QScriptValue const& on_error)
{
    if (!unreplied_count_)
        emit acquired();

    ++unreplied_count_;
    worker_->send(new Request
                  (method_name, msg.toVariant()
                   , endpoint(on_reply, on_error)
                   , Event::Request));
}

void Actor::reply(Message *reply)
{
    auto &cb = reply->endpoint_->on_reply_;
    if (!cb.isValid())
        return;
    auto params = cb.engine()->newArray(1);
    auto data = cb.engine()->toScriptValue(reply->data_);
    params.setProperty(0, data);
    if (cb.isFunction())
        cb.call(QScriptValue(), params);
}

void Actor::error(Message *reply)
{
    auto &cb = reply->endpoint_->on_error_;
    if (!cb.isValid()) {
        emit error(reply->data_);
        return;
    }
    auto params = cb.engine()->newArray(1);
    auto data = cb.engine()->toScriptValue(reply->data_);
    params.setProperty(0, data);
    if (cb.isFunction())
        cb.call(QScriptValue(), params);
}

Event::Event()
    : QEvent(static_cast<QEvent::Type>(Event::QuitThread))
{
}

Event::Event(Event::Type t)
    : QEvent(static_cast<QEvent::Type>(t))
{ }

Load::Load(QString const &src, QString const& top_script)
    : Event(Event::LoadScript)
    ,  src_(src)
    , top_script_(top_script)
{ }

Event::~Event() {}

WorkerThread::WorkerThread
(Actor *actor, QString const &src, QString const& top_script)
    : QThread(nullptr)
    , actor_(actor)
{
    mutex_.lock();
    start();
    cond_.wait(&mutex_);
    QCoreApplication::postEvent(engine_.data(), new Load(src, top_script));
    connect(engine_.data(), SIGNAL(onQuit()), this, SLOT(quit()));
}

void Engine::toActor(Event *ev)
{
    QCoreApplication::postEvent(actor_, ev);
}

EngineException::EngineException(QScriptEngine const& engine)
    : Event(Event::LoadException)
    , exception_(engine.uncaughtException().toVariant())
    , backtrace_(engine.uncaughtExceptionBacktrace())
{}

Message::Message(QVariant const& data, endpoint_ptr ep
                 , Event::Type type)
    : Event(type), data_(data), endpoint_(ep)
{}

Request::Request(QString const &method_name, QVariant const& data
                 , endpoint_ptr ep, Event::Type type)
    : Message(data, ep, type)
    , method_name_(method_name)
{}

void Engine::load(Load *msg)
{
    engine_ = new QScriptEngine(this);
    try {
        auto script_env = loadEnv(*QCoreApplication::instance(), *engine_);
        script_env->pushParentScriptPath(msg->top_script_);
        handler_ = script_env->load(msg->src_);
        if (!handler_.isFunction()) {
            qDebug() << "Not a function";
            if (handler_.isError())
                error(handler_.toVariant()
                      , endpoint(QScriptValue(), QScriptValue()));
        }
    } catch (Error const &e) {
        qDebug() << "Failed to eval:" << msg->src_;
        qDebug() << e.msg;
        if (engine_->hasUncaughtException()) {
            toActor(new EngineException(*engine_));
            engine_->clearExceptions();
        }
    }
}

Engine::Engine(Actor *actor) : actor_(actor) {}
Engine::~Engine() {}

WorkerThread::~WorkerThread()
{
    if (engine_)
        QCoreApplication::instance()->notify(engine_.data(), new Event());

    wait();
}

void WorkerThread::run()
{
    engine_.reset(new Engine(actor_));
    mutex_.lock();
    cond_.wakeAll();
    mutex_.unlock();
    exec();
}

void Engine::processResult(QScriptValue &ret, endpoint_ptr ep)
{
    if (engine_->hasUncaughtException()) {
        error(engine_->uncaughtException().toVariant(), ep);
    } else if (!ret.isError()) {
        reply(ret.toVariant(), ep, Event::Return);
    } else {
        error(ret.toVariant(), ep);
    }
}

void Engine::processMessage(Message *msg)
{
    QScriptValue ret;

    if (handler_.isFunction()) {
        auto params = engine_->newArray(2);
        params.setProperty(0, engine_->toScriptValue(msg->data_));
        params.setProperty(1, engine_->newQObject
                           (new MessageContext(this, msg->endpoint_)));
        ret = handler_.call(handler_, params);
    } else if (handler_.isValid()) {
        auto cls = handler_.scriptClass();
        qDebug() << "Handler is not a function but "
                 << (cls ? cls->name() : "unknown value");
    } else {
        qDebug() << "No handler";
    }
    processResult(ret, msg->endpoint_);
}

void Engine::processRequest(Request *req)
{
    QScriptValue ret;

    if (handler_.isObject()) {
        auto method = handler_.property(req->method_name_);
        if (method.isFunction()) {
            auto params = engine_->newArray(2);
            params.setProperty(0, engine_->toScriptValue(req->data_));
            params.setProperty(1, engine_->newQObject
                               (new MessageContext(this, req->endpoint_)));
            ret = method.call(handler_, params);
        } else if (method.isUndefined()) {
            qDebug() << "Actor does not have method" << req->method_name_;
        } else {
            qDebug() << "Actor property " << req->method_name_
                     << " is not a method but "
                     << method.scriptClass()->name();
        }
    } else {
        qDebug() << "Handler is not an object";
    }
    processResult(ret, req->endpoint_);
}

bool Engine::event(QEvent *e)
{
    switch (static_cast<Event::Type>(e->type())) {
    case (Event::LoadScript):
        load(static_cast<Load*>(e));
        return true;
    case (Event::Message):
        processMessage(static_cast<Message*>(e));
        return true;
    case (Event::Request):
        processRequest(static_cast<Request*>(e));
        return true;
    case (Event::QuitThread):
        emit onQuit();
        return true;
    default:
        return QObject::event(e);
    }
    return false;
}

bool Actor::event(QEvent *e)
{
    EngineException *ex;
    bool res;
    switch (static_cast<Event::Type>(e->type())) {
    case (Event::Reply):
        reply(static_cast<Message*>(e));
        res = true;
        break;
    case (Event::Return):
        --unreplied_count_;
        reply(static_cast<Message*>(e));
        res = true;
        break;
    case (Event::LoadException):
        ex = static_cast<EngineException*>(e);
        emit error(ex->exception_);
        res = true;
        break;
    case (Event::Error):
        --unreplied_count_;
        error(static_cast<Message*>(e));
        res = true;
        break;
    default:
        res = QObject::event(e);
        break;
    }
    if (!unreplied_count_)
        emit released();
    return res;
}


MessageContext::MessageContext(Engine *engine, endpoint_ptr ep)
    : QObject(engine)
    , engine_(engine)
    , endpoint_(ep)
{}

MessageContext::~MessageContext() {}

void MessageContext::reply(QScriptValue data)
{
    engine_->reply(data.toVariant(), endpoint_, Event::Reply);
}

void Engine::reply
(QVariant const &data, endpoint_ptr ep, Event::Type type)
{
    toActor(new Message(data, ep, type));
}

void Engine::error(QVariant const &data, endpoint_ptr ep)
{
    toActor(new Message(data, ep, Event::Error));
}


QUrl QtScriptAdapter::qml() const
{
    return qml_;
}

void QtScriptAdapter::setQml(QUrl const& url)
{
    qml_ = url;
    auto engine = getDeclarativeScriptEngine(qmlEngine(this));

    auto env = static_cast<Env*>
        (findProperty(engine->globalObject(), {"qtscript"}).toQObject());
    env->pushParentScriptPath(url.path());
}


}
