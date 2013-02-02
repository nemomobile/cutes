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
#include "QsExecute.hpp"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QScriptEngine>
#include <QDebug>
#include <QCoreApplication>
#include <QDeclarativeContext>

namespace QsExecute {

Actor::Actor(QObject *parent)
        : QObject(parent)
{
}

Actor::~Actor()
{
}

QString Actor::source() const
{
    return src_;
}

void Actor::setSource(QString const &src)
{
    if (src == src_)
        return;

    auto cwd = engine()->baseUrl().toLocalFile();
    worker_.reset(new WorkerThread(this, src, cwd));

    src_ = src;
}

void WorkerThread::sendMessage(QScriptValue m, QScriptValue cb)
{
    try {
        QCoreApplication::postEvent
            (engine_.data(), new Message(m.toVariant(), cb));
    } catch (...) {
        qDebug() << "Caught exception processing sendMessage"
                 << m.toString() << cb.toString();
    }
}

void Actor::sendMessage(QScriptValue m, QScriptValue cb)
{
    worker_->sendMessage(m, cb);
}

void Actor::reply(Message *reply)
{
    auto &cb = reply->cb_;
    auto params = cb.engine()->newArray(1);
    auto data = cb.engine()->toScriptValue(reply->data_);
    params.setProperty(0, data);
    if (cb.isFunction())
        cb.call(QScriptValue(), params);
    emit message(data);
}

Event::Event()
    : QEvent(static_cast<QEvent::Type>(Event::QuitThread))
{
}

Event::Event(Event::Type t)
    : QEvent(static_cast<QEvent::Type>(t))
{ }

Load::Load(QString const &src, QString const &cwd)
    : Event(Event::LoadScript),
      src_(src), cwd_(cwd)
{ }

Event::~Event() {}

WorkerThread::WorkerThread
(Actor *parent, QString const &src, QString const &cwd)
    : QThread(nullptr)
    , actor_(parent)
{
    mutex_.lock();
    start();
    cond_.wait(&mutex_);
    QCoreApplication::postEvent(engine_.data(), new Load(src, cwd));
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
{
}

Message::Message(QVariant const& data, QScriptValue const& cb)
    : Event(ProcessMessage), data_(data), cb_(cb)
{
}

void Engine::load(Load *msg)
{
    engine_ = new QScriptEngine(this);
    try {
        auto load = QsExecute::setupEngine
            (*QCoreApplication::instance(), *engine_, engine_->globalObject());
        auto script = findProperty
            (engine_->globalObject(), {"qtscript", "script"});
        script.setProperty("cwd", engine_->toScriptValue(msg->cwd_));
        handler_ = load(msg->src_, *engine_);
        if (!handler_.isFunction()) {
            qDebug() << "Not a function";
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

void Engine::processMessage(Message *msg)
{
    auto &cb = msg->cb_;
    QVariant res;

    if (handler_.isFunction()) {
        auto params = engine_->newArray(2);
        params.setProperty(0, engine_->toScriptValue(msg->data_));
        params.setProperty(1, engine_->newQObject(new MessageContext(this, cb)));
        res = handler_.call(QScriptValue(), params).toVariant();
    }
    reply(res, cb);
}

bool Engine::event(QEvent *e)
{
    switch (static_cast<Event::Type>(e->type())) {
    case (Event::LoadScript):
        load(static_cast<Load*>(e));
        return true;
    case (Event::ProcessMessage):
        processMessage(static_cast<Message*>(e));
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
    switch (static_cast<Event::Type>(e->type())) {
    case (Event::ProcessMessage):
        reply(static_cast<Message*>(e));
        return true;
    case (Event::LoadException):
        ex = static_cast<EngineException*>(e);
        emit error(ex->exception_);
        return true;
    default:
        return QObject::event(e);
    }
    return false;
}


MessageContext::MessageContext(Engine *engine, QScriptValue cb)
    : QObject(engine), engine_(engine), cb_(cb)
{}

MessageContext::~MessageContext() {}

void MessageContext::reply(QScriptValue data)
{
    engine_->reply(data.toVariant(), cb_);
}

void Engine::reply(QVariant const &data, QScriptValue const &cb)
{
    toActor(new Message(data, cb));
}

QDeclarativeEngine *Actor::engine()
{
    return qmlEngine(this);
}


}
