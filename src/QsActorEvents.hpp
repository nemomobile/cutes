#ifndef _QS_ACTOR_EVENTS_HPP_
#define _QS_ACTOR_EVENTS_HPP_
/*
 * QtScript Actor events
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

#include "QsActor.hpp"

#include <QObject>
#include <QVariant>
#include <QString>
#include <QJSValue>


namespace QsExecute
{

class Endpoint : public QObject
{
    Q_OBJECT;
public:
    Endpoint(QJSValue const&, QJSValue const&, QJSValue const&);

    QJSValue on_reply_;
    QJSValue on_error_;
    QJSValue on_progress_;
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
    Message(QVariant const&, endpoint_ptr, Event::Type);
    virtual ~Message() {}

    QVariant data_;
    endpoint_ptr endpoint_;
};

class Request : public Message
{
public:
    Request(QString const&, QVariant const&, endpoint_ptr, Event::Type);
    virtual ~Request() {}

    QString method_name_;
};

class MessageContext : public QObject
{
    Q_OBJECT;
public:
    MessageContext(Engine *, endpoint_ptr);
    virtual ~MessageContext();

    Q_INVOKABLE void reply(QJSValue);

    void disable();
private:
    Engine *engine_;
    endpoint_ptr endpoint_;
};

class EngineException : public Event
{
public:
    EngineException(QJSEngine const&);
    virtual ~EngineException() {}

    QVariant exception_;
    QStringList backtrace_;
};

}

#endif // _QS_ACTOR_EVENTS_HPP_
