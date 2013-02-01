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

    if (!engine_) {
        engine_.reset(new EngineThread(this, src));
    }

    src_ = src;
}

void Actor::exitEngine()
{
    if (engine_)
        QCoreApplication::postEvent(engine_.data(), new EngineEvent());
}

void EngineThread::sendMessage(QScriptValue m, QScriptValue cb)
{
    try {
        QCoreApplication::postEvent
            (engine_.data(), new EngineEvent(m.toVariant(), cb));
    } catch (...) {
        qDebug() << "Caught exception processing sendMessage"
                 << m.toString() << cb.toString();
    }
}

void Actor::sendMessage(QScriptValue m, QScriptValue cb)
{
    engine_->sendMessage(m, cb);
}

void Actor::onReply(QVariant var, QScriptValue cb)
{
    auto params = cb.engine()->newArray(1);
    auto data = cb.engine()->toScriptValue(var);
    params.setProperty(0, data);
    if (cb.isFunction())
        cb.call(QScriptValue(), params);
    emit message(data);
}

EngineEvent::EngineEvent()
    : QEvent(static_cast<QEvent::Type>(EngineEvent::QuitThread))
{
}

EngineEvent::EngineEvent(QString const &src)
    : QEvent(static_cast<QEvent::Type>(EngineEvent::LoadScript)),
      src_(src)
{
}

EngineEvent::EngineEvent(QVariant const &data, QScriptValue cb)
    : QEvent(static_cast<QEvent::Type>(EngineEvent::ProcessMessage)),
      data_(data), cb_(cb)
{
}

EngineEvent::~EngineEvent() {}

EngineThread::EngineThread(Actor *parent, QString const &src)
    : QThread(nullptr)
    , actor_(parent)
{
    mutex_.lock();
    start();
    cond_.wait(&mutex_);
    QCoreApplication::postEvent(engine_.data(), new EngineEvent(src));
    connect(engine_.data(), SIGNAL(onReply(QVariant, QScriptValue))
            , actor_, SLOT(onReply(QVariant, QScriptValue)));
    connect(engine_.data(), SIGNAL(onQuit())
            , this, SLOT(quit()));
}

void Engine::load(QString const &src)
{
    engine_ = new QScriptEngine(this);
    auto load = QsExecute::setupEngine(*QCoreApplication::instance(),
                                       *engine_, engine_->globalObject());
    int rc = EXIT_SUCCESS;

    try {
        handler_ = load(src, *engine_);
        if (engine_->uncaughtException().isValid())
            rc = EXIT_FAILURE;
        if (!handler_.isFunction()) {
            qDebug() << "Not a function";
        }
    } catch (Error const &e) {
        qDebug() << "Failed to eval:" << src;
        qDebug() << e.msg;
        rc = EXIT_FAILURE;
    }
}

Engine::Engine() {}
Engine::~Engine() {}

EngineThread::~EngineThread()
{
    if (engine_)
        QCoreApplication::instance()->notify(engine_.data(), new EngineEvent());

    wait();
}

void EngineThread::run()
{
    engine_.reset(new Engine());
    mutex_.lock();
    cond_.wakeAll();
    mutex_.unlock();
    exec();
}

void Engine::processMessage(QVariant &data, QScriptValue &cb)
{
    QVariant res;
    if (handler_.isFunction()) {
        auto params = engine_->newArray(1);
        params.setProperty(0, engine_->toScriptValue(data));
        res = handler_.call(QScriptValue(), params).toVariant();
    }
    emit onReply(res, cb);
}

bool Engine::event(QEvent *e)
{
    EngineEvent *self;
    switch (static_cast<EngineEvent::Type>(e->type())) {
    case (EngineEvent::LoadScript):
        self = static_cast<EngineEvent*>(e);
        load(self->src_);
        return true;
    case (EngineEvent::ProcessMessage):
        self = static_cast<EngineEvent*>(e);
        processMessage(self->data_, self->cb_);
        return true;
    case (EngineEvent::QuitThread):
        emit onQuit();
        return true;
    default:
        return QObject::event(e);
    }
    return false;
}

}
