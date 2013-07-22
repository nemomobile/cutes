#include <cutes/util.hpp>

namespace cutes {

QJSValue toQJSValue(QVariant const& v)
{
    QJSValue res;
    switch (v.type()) {
    case QVariant::Bool:
        return QJSValue(v.toBool());
    case QVariant::String:
        return QJSValue(v.toString());
    case QVariant::Double:
        return QJSValue(v.toDouble());
    case QVariant::Int:
        return QJSValue(v.toInt());
    case QVariant::UInt:
        return QJSValue(v.toUInt());
    default:
        return QJSValue();
    }
}

namespace js {

v8::Handle<v8::Value> callConvertException
(const v8::Arguments &args, v8::InvocationCallback fn)
{
    try {
        return fn(args);
    } catch (std::exception const &e) {
        using namespace v8;
        ThrowException(Exception::Error(String::New(e.what())));
        return Handle<Value>();
    }
}


std::pair<bool, VHandle> copyCtor(const v8::Arguments &args)
{
    using namespace v8;
    Local<External> external;
    auto self = args.This();
    
    if (!args.IsConstructCall()) {
        ThrowException(Exception::Error(String::New("Call function as ctor")));
        return {true, VHandle()};
    }

    if (args[0]->IsExternal()) {
        external = Local<External>::Cast(args[0]);
        self->SetInternalField(0, external);
        return {true, self};
    }
    return {false, VHandle()};
}

}}
