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

#include "Actor.hpp"
#include "ActorEvents.hpp"
#include "Env.hpp"
#include "QmlAdapter.hpp"

#include <cutes/util.hpp>
#include "qt_quick_types.hpp"
#include <QDebug>
#include <QCoreApplication>

namespace cutes {

Endpoint::Endpoint(QJSValue const& on_reply
                   , QJSValue const& on_error
                   , QJSValue const& on_progress)
    : on_reply_(on_reply)
    , on_error_(on_error)
    , on_progress_(on_progress)
{
}

static inline endpoint_ptr endpoint
(QJSValue const& on_reply
 , QJSValue const& on_error
 , QJSValue const& on_progress)
{
    return endpoint_ptr(new Endpoint(on_reply, on_error, on_progress));
}

Actor::Actor(QJSEngine *engine)
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

void Actor::execute(std::function<void ()> fn)
{
    try{
        fn();
    } catch (Error const &e) {
        // if (engine_)
        //     engine_->currentContext()->throwError(e.msg);
        // else
        //     qDebug() << "Error " << e.msg;
    } catch (...) {
        // if (engine_)
        //     engine_->currentContext()->throwError("Unhandled error");
        // else
        //     qDebug() << "Error " << "Unhandled error";
    }
}

void Actor::reload()
{
    auto fn = [this]() {
        if (!engine_) {
            qDebug() << "Can't get script engine."
            << " Not qml engine and missing initialization?";
            return;
        }
        // cwd should be set to the same directory as for main engine
        auto script = static_cast<Module*>
        (findProperty
         (engine_->globalObject(), {"cutes", "module"}).toQObject());

        worker_.reset(new WorkerThread(this, src_, script->fileName()));
    };
    execute(fn);
}

void Actor::setSource(QString const& src)
{
    if (src == src_)
        return;

    src_ = src;
    reload();
}

QmlActor::QmlActor(QJSEngine *engine)
    : Actor(engine)
{
    // if Actor is created by QtScript environment it should have
    // engine_ set while declarative view does not provide direct
    // access to the engine and it will be initialized on first call
    // to this method
    if (!engine_)
        engine_ = qmlEngine(this);
}

QUrl QmlActor::source() const
{
    return Actor::source();
}

void QmlActor::setSource(QUrl const& src)
{
    Actor::setSource(src.path());
}

StdActor::StdActor(QJSEngine *engine)
    : Actor(engine)
{}

// QUrl StdActor::source() const
// {
//     return Actor::source();
// }

// void StdActor::setSource(QUrl const& src)
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

void Actor::acquire()
{
    if (!unreplied_count_)
        emit acquired();

    ++unreplied_count_;
}

void Actor::release()
{
    if (--unreplied_count_ == 0)
        emit released();
}

void Actor::send
(QJSValue const &msg
 , QJSValue const& on_reply
 , QJSValue const& on_error
 , QJSValue const& on_progress)
{
    auto fn = [&]() {
        acquire();
        worker()->send(new Message
                       (msg.toVariant()
                        , endpoint(on_reply, on_error, on_progress)
                        , Event::Message));
    };
    execute(fn);
}
void Actor::request
(QString const &method_name, QJSValue const &msg
 , QJSValue const& on_reply
 , QJSValue const& on_error
 , QJSValue const& on_progress)
{
    auto fn = [&]() {
        acquire();
        worker_->send(new Request
                      (method_name, msg.toVariant()
                       , endpoint(on_reply, on_error, on_progress)
                       , Event::Request));
    };
    execute(fn);
}

void Actor::reply(Message *msg)
{
    callback(msg, msg->endpoint_->on_reply_);
}
void Actor::progress(Message *msg)
{
    callback(msg, msg->endpoint_->on_progress_);
}

void Actor::callback(Message *msg, QJSValue& cb)
{
    if (!cb.isCallable())
        return;
    auto params = QJSValueList();
    params.push_back(cutes::toQJSValue((msg->data_)));
    cb.callWithInstance(cb, params);
}

void Actor::error(Message *reply)
{
    auto &cb = reply->endpoint_->on_error_;
    if (!cb.isCallable()) {
        emit error(reply->data_);
        return;
    }
    auto params = QJSValueList();
    params.push_back(cutes::toQJSValue((reply->data_)));
    cb.callWithInstance(cb, params);
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
    mutex_.unlock();
}

void Engine::toActor(Event *ev)
{
    QCoreApplication::postEvent(actor_, ev);
}

EngineException::EngineException(QJSEngine const&, QJSValue const& err)
    : Event(Event::LoadException)
    , exception_(err.toVariant())
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
    engine_ = new QJSEngine(this);
    try {
        auto script_env = loadEnv(*QCoreApplication::instance(), *engine_);
        script_env->pushParentScriptPath(msg->top_script_);
        handler_ = script_env->load(msg->src_);
        if (!(handler_.isCallable() || handler_.isObject())) {
            qDebug() << "Not a function or object";
            if (handler_.isError()) {
                error(handler_.toVariant()
                      , endpoint(QJSValue()
                                 , QJSValue()
                                 , QJSValue()));
            } else {
                // auto cls = handler_.scriptClass();
                // qDebug() << "Handler is "
                //          << (cls ? cls->name() : "unknown value");
            }
        }
    } catch (Error const &e) {
        qDebug() << "Failed to eval:" << msg->src_;
        qDebug() << e.msg;
        // TODO toActor(new EngineException(*engine_));
    }
}

Engine::Engine(Actor *actor) : actor_(actor) {}
Engine::~Engine() {}

WorkerThread::~WorkerThread()
{
    quit();
    wait();
}

void WorkerThread::run()
{
    engine_.reset(new Engine(actor_));
    mutex_.lock();
    cond_.wakeAll();
    mutex_.unlock();
    exec();
    engine_.reset(nullptr);
}

void Engine::processResult(QJSValue &ret, endpoint_ptr ep)
{
    QVariant err;
    if (ret.isError()) {
        err = ret.toVariant();
    } else {
        reply(ret.toVariant(), ep, Event::Return);
        return;
    }
    qDebug() << "Actor error: " << err << ", sending to source";
    error(err, ep);
}

void Engine::processMessage(Message *msg)
{
    QJSValue ret;

    if (handler_.isCallable()) {
        QJSValueList params;
        params.push_back(cutes::toQJSValue(msg->data_));
        params.push_back(engine_->newQObject
                         (new MessageContext(this, msg->endpoint_)));
        ret = handler_.callWithInstance(handler_, params);
    } else if (!(handler_.isNull() && handler_.isUndefined())) {
        qDebug() << "Handler is not a function but "
                 << (handler_.toString());
    } else {
        qDebug() << "No handler";
    }
    processResult(ret, msg->endpoint_);
}

void Engine::processRequest(Request *req)
{
    QJSValue ret;

    if (handler_.isObject()) {
        auto method = handler_.property(req->method_name_);
        if (method.isCallable()) {
            MessageContext *ctx = new MessageContext(this, req->endpoint_);
            QJSValueList params;
            params.push_back(cutes::toQJSValue(req->data_));
            params.push_back(engine_->newQObject(ctx));
            ret = method.callWithInstance(handler_, params);
            ctx->disable();
            ctx->deleteLater();
        } else if (method.isUndefined() || method.isNull()) {
            qDebug() << "Actor does not have method" << req->method_name_;
        } else {
            qDebug() << "Actor property " << req->method_name_
                     << " is not a method but "
                     << method.toString();
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
    bool res, is_release = false;
    switch (static_cast<Event::Type>(e->type())) {
    case (Event::Progress):
        progress(static_cast<Message*>(e));
        res = true;
        break;
    case (Event::Return):
        is_release = true;
        reply(static_cast<Message*>(e));
        res = true;
        break;
    case (Event::LoadException):
        ex = static_cast<EngineException*>(e);
        emit error(ex->exception_);
        res = true;
        break;
    case (Event::Error):
        is_release = true;
        error(static_cast<Message*>(e));
        res = true;
        break;
    default:
        res = QObject::event(e);
        break;
    }
    if (is_release)
        release();
    return res;
}


MessageContext::MessageContext(Engine *engine, endpoint_ptr ep)
    : QObject(engine)
    , engine_(engine)
    , endpoint_(ep)
{}

void MessageContext::disable()
{
    endpoint_.clear();
}

MessageContext::~MessageContext() {}

void Actor::wait()
{
    if (!unreplied_count_)
        return;

    QEventLoop loop;
    QObject::connect(this, SIGNAL(released()), &loop, SLOT(quit()));
    loop.exec();
}

void MessageContext::reply(QJSValue data)
{
    if (engine_)
        engine_->reply(data.toVariant(), endpoint_, Event::Progress);
    else
        qDebug() << "MessageContext is disabled, " <<
            "are you using it outside processing function?";
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
    auto engine = qmlEngine(this);

    auto env = static_cast<Env*>
        (findProperty(engine->globalObject(), {"cutes"}).toQObject());
    env->pushParentScriptPath(url.path());
}


}
