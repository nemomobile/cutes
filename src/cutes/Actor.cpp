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
#include <QMap>

namespace cutes {

static endpoint_ptr endpoint(QJSValue const& ep)
{
    QJSValue on_reply, on_error, on_progress;
    if (ep.isObject()) {
        on_reply = ep.property("on_reply");
        on_error = ep.property("on_error");
        on_progress = ep.property("on_progress");
    } else if (ep.isCallable()) {
        on_reply = ep;
    } else {
        throw Error(QString("Wrong endpoint is passed, should be a function"
                            " or endpoint objects: ") + ep.toString());
    }
    return endpoint_ptr(new Endpoint(on_reply, on_error, on_progress));
}

Actor::Actor(QJSEngine *engine)
    : engine_(engine)
    , unreplied_count_(0)
    , cookie_(0)
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
        qWarning() << "Error executing request/send:" << e.msg;
        // TODO throw js error
    } catch (...) {
        qWarning() << "Some error executing request/send";
        // TODO throw js error
    }
}

void Actor::reload()
{
    auto fn = [this]() {
        if (!engine_) {
            qWarning() << "Can't get script engine."
            << " Not qml engine and missing initialization?";
            return;
        }
        auto engine = qmlEngine(this);
        auto env = engine->rootContext()->contextProperty("cutes").value<Env*>();
        if (!env) {
            qWarning() << "No env set in context?";
            return;
        }
        // cwd should be set to the same directory as for main engine
        auto script = env->current_module();

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
}

QUrl QmlActor::source() const
{
    return Actor::source();
}

void QmlActor::setSource(QUrl const& src)
{
    // if Actor is created by QtScript environment it should have
    // engine_ set while declarative view does not provide direct
    // access to the engine and it will be initialized on first call
    // to this method
    if (!engine_)
        engine_ = qmlEngine(this);

    if (!engine_) {
        qWarning() << "CutesActor have not engine assigned. No go...";
        return;
    }
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

endpoint_handle Actor::endpoint_new(QJSValue cb)
{
        auto ep = endpoint(cb);
        endpoints_.insert(ep.data(), ep);
        endpoint_handle h(ep.data(), [this](Endpoint *p) {
                QCoreApplication::postEvent(this, new EndpointRemove(p));
            });
        return h;
}

void Actor::send(QJSValue const &msg, QJSValue cb)
{
    auto fn = [&]() {
        acquire();
        worker()->send
        (new Message(msg.toVariant(), endpoint_new(cb), Event::Message));
    };
    execute(fn);
}

void Actor::request(QString const &method, QVariant msg, QJSValue cb)
{
    auto fn = [&]() {
        acquire();
        worker_->send(new Request(method, msg, endpoint_new(cb), Event::Request));
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
    params.push_back(cutes::toQJSValue(*engine_, msg->data_));
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
    auto err = engine_
        ? cutes::toQJSValue(*engine_, reply->data_)
        : cutes::toQJSValue(reply->data_);
    params.push_back(err);
    cb.callWithInstance(cb, params);
}

Event::Event()
    : QEvent(static_cast<QEvent::Type>(Event::QuitThread))
{
}

Event::Event(Event::Type t)
    : QEvent(static_cast<QEvent::Type>(t))
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

void Engine::load(Load *msg)
{
    engine_ = new QJSEngine(this);
    try {
        auto script_env = loadEnv(*QCoreApplication::instance(), *engine_);
        script_env->pushParentScriptPath(msg->top_script_);
        handler_ = script_env->load(msg->src_, false);
        if (!(handler_.isCallable() || handler_.isObject())) {
            qWarning() << "Not a function or object but "
                       << handler_.toString();
            if (handler_.isError()) {
                qWarning() << "Exception while loading:" << handler_.toString();
                toActor(new LoadError(msg->src_));
            }
        }
    } catch (Error const &e) {
        qWarning() << "Failed to eval:" << msg->src_;
        qWarning() << e.msg;
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

void Engine::processResult(QJSValue ret, endpoint_handle ep)
{
    if (!ret.isError()) {
        reply(ret.toVariant(), ep, Event::Return);
    } else {
        QVariantMap err;
        for (auto p : {"message", "stack", "arguments"
                    , "type", "isWrapped", "originalError"})
            err[p] = ret.property(p).toVariant();

        error(err, ep);
    }
}

QJSValue Engine::callConvertError(QJSValue const &fn
                                  , QJSValue const &instance
                                  , QJSValueList const &params)
{
    if (convert_error_.isUndefined()) {
        auto code = errorConverterTry("var res = function(fn, obj) {")
            + "return fn.apply(obj, [].slice.call(arguments, 2));"
            + errorConverterCatch("}; res;");
        if (!engine_) {
            qWarning() << "Engine is null!";
            return convert_error_;
        }
        convert_error_ = engine_->evaluate(code);
        if (!convert_error_.isCallable()) {
            qWarning() << "Can't evaluate properly error conversion code: "
                       << "expected conversion function, got "
                       << convert_error_.toString();
            convert_error_ = QJSValue();
            return convert_error_;
        }
    }

    QJSValueList converter_params(params);
    converter_params.push_front(instance);
    converter_params.push_front(fn);
    return convert_error_.callWithInstance(convert_error_, converter_params);
}

void Engine::processMessage(Message *msg)
{
    QJSValue ret;

    if (handler_.isCallable()) {
        QJSValueList params;
        params.push_back(cutes::toQJSValue(*engine_, msg->data_));
        params.push_back(engine_->newQObject
                         (new MessageContext(this, msg->endpoint_)));
        ret = callConvertError(handler_, handler_, params);
    } else if (!(handler_.isNull() && handler_.isUndefined())) {
        qWarning() << "Handler is not a function but "
                 << (handler_.toString());
    } else {
        qWarning() << "No handler";
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
            params.push_back(engine_->toScriptValue(req->data_));
            params.push_back(engine_->newQObject(ctx));
            ret = callConvertError(method, handler_, params);
            ctx->disable();
            ctx->deleteLater();
        } else if (method.isUndefined() || method.isNull()) {
            qWarning() << "Actor does not have method" << req->method_name_;
        } else {
            qWarning() << "Actor property " << req->method_name_
                     << " is not a method but "
                     << method.toString();
        }
    } else {
        qWarning() << "Handler is not an object";
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
    case Event::EndpointRemove: {
        EndpointRemove *rm = static_cast<EndpointRemove*>(e);
        endpoints_.remove(rm->endpoint_);
        res = true;
        break;
    }
    default:
        res = QObject::event(e);
        break;
    }
    if (is_release)
        release();
    return res;
}

MessageContext::MessageContext(Engine *engine, endpoint_handle ep)
    : QObject(engine)
    , engine_(engine)
    , endpoint_(ep)
{}
MessageContext::~MessageContext() {}

void MessageContext::disable()
{
    endpoint_.reset();
}

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
        qWarning() << "MessageContext is disabled, " <<
            "are you using it outside processing function?";
}

void Engine::reply
(QVariant const &data, endpoint_handle ep, Event::Type type)
{
    toActor(new Message(data, ep, type));
}

void Engine::error(QVariant const &data, endpoint_handle ep)
{
    toActor(new Message(data, ep, Event::Error));
}


QUrl Adapter::qml() const
{
    return qml_;
}

void Adapter::setQml(QUrl const& url)
{
    qml_ = url;
    auto env = getEnv();
    if (!env) {
        qWarning() << "Adapter:Env is null!";
        return;
    }
    env->pushParentScriptPath(url.path());
}

Env * Adapter::getEnv() const
{
    auto engine = qmlEngine(this);

    if (!engine) {
        qWarning() << "Adapter.engine is null!";
        return nullptr;
    }
    auto env = engine->rootContext()->contextProperty("cutes").value<Env*>();
    return env;
}


}
