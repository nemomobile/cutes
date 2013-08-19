#include "ActorEvents.hpp"
#include "ActorEvents.moc"
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
{}

Load::Load(QString const &src, QString const& top_script)
    : Event(Event::LoadScript)
    ,  src_(src)
    , top_script_(top_script)
{}

Load::~Load() {}

LoadError::LoadError(QString const &src)
    : Event(Event::LoadException)
    ,  src_(src)
{}

LoadError::~LoadError() {}

EngineException::EngineException(QJSEngine const&, QJSValue const& err)
    : Event(Event::LoadException)
    , exception_(err.toVariant())
{}

Message::Message(QVariant const& data, endpoint_handle ep
                 , Event::Type type)
    : Event(type), data_(data), endpoint_(ep)
{}

Message::~Message() {}

Request::Request(QString const &method_name, QVariant const& data
                 , endpoint_handle ep, Event::Type type)
    : Message(data, ep, type)
    , method_name_(method_name)
{}

Request::~Request() {}

EndpointRemove::EndpointRemove(Endpoint *p)
    : Event(Event::EndpointRemove)
    , endpoint_(p)
{}

EndpointRemove::~EndpointRemove() {}

}
