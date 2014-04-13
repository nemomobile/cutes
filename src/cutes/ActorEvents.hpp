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

#include "Actor.hpp"
#include <cutes/util.hpp>

#include <QObject>
#include <QVariant>
#include <QString>
#include <QJSValue>


namespace cutes
{

class Endpoint : public QObject
{
    Q_OBJECT;
public:
    Endpoint(QJSValue const&, QJSValue const&, QJSValue const&
             , std::unique_ptr<ActorHolder>);
    ~Endpoint();

    QJSValue on_reply_;
    QJSValue on_error_;
    QJSValue on_progress_;
    std::unique_ptr<ActorHolder> actor_holder_;
};

class Load : public Event
{
public:
    Load(QString const&, QString const&, std::unique_ptr<ActorHolder>);
    Load(Load &&);
    virtual ~Load();

    QString src_;
    QString top_script_;
    std::unique_ptr<ActorHolder> actor_holder_;
};

class LoadError : public Event
{
public:
    LoadError(QVariant const&, std::unique_ptr<ActorHolder>);
    virtual ~LoadError();

    QVariant info_;
    std::unique_ptr<ActorHolder> actor_holder_;
};

class EndpointRemove : public Event
{
public:
    EndpointRemove(Endpoint*);
    virtual ~EndpointRemove();

    Endpoint *endpoint_;
};

class Message : public Event
{
public:
    Message(QVariant const&, endpoint_handle, Event::Type);
    virtual ~Message();

    QVariant data_;
    endpoint_handle endpoint_;
};

class Request : public Message
{
public:
    Request(QString const&, QVariant const&, endpoint_handle, Event::Type);
    virtual ~Request();

    QString method_name_;
};

class MessageContext : public QObject
{
    Q_OBJECT;

    struct Deleter {
        void operator ()(MessageContext *self)
        {
            self->disable();
            self->deleteLater();
        }
    };

    class Handle {
    public:
        Handle(Engine *e, endpoint_handle reply_ep)
            : p_(new MessageContext(e, reply_ep))
        {}

        QJSValue jsObject(QJSEngine &e)
        {
            return getCppOwnedJSValue(e, p_.get());
        }
    private:
        std::unique_ptr<MessageContext, Deleter> p_;
    };

public:
    ~MessageContext();

    Q_INVOKABLE void reply(QJSValue);
    void disable();

    static Handle create(Engine *e, endpoint_handle reply_ep)
    {
        return Handle(e, reply_ep);
    }

private:
    MessageContext(Engine *, endpoint_handle);
    Engine *engine_;
    endpoint_handle endpoint_;
};

}

#endif // _QS_ACTOR_EVENTS_HPP_
