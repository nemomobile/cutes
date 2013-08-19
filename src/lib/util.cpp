/*
 * Support for cutes on Qt5V8
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

#include <cutes/util.hpp>

#include <QtQml/private/qjsvalue_p.h>
#include <QtQml/private/qjsvalue_impl_p.h>

#include <QtQml/private/qv8engine_p.h>
#include <QtQml/private/qv8engine_impl_p.h>
#include <QtQml/private/qjsvalueiterator_impl_p.h>

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

QJSValue toQJSValue(QJSEngine &engine, QVariant const& v)
{
    v8::HandleScope hscope;
    auto v8e = engine.handle();
    return toQJSValue(engine, v8e->fromVariant(v));
}

QJSValue toQJSValue(QJSEngine &engine, v8::Handle<v8::Value> src)
{
    v8::HandleScope hscope;
    return QJSValuePrivate::get(new QJSValuePrivate(engine.handle(), src));
}

v8::Handle<v8::Value> fromQJSValue(QJSValue const& v)
{
    return QJSValuePrivate::get(v)->handle();
}

namespace js {

v8::Handle<v8::Value> callConvertException
(const v8::Arguments &args, v8::InvocationCallback fn)
{
    try {
        return fn(args);
    } catch (std::exception const &e) {
        using namespace v8;
        qWarning() << "Exception (" << e.what()
                   << ") invoking native fn. Args("
                   << args.Length() << "):";
        for (int i = 0; i < args.Length(); ++i) {
            v8::String::Utf8Value cs(args[i]);
            qWarning() << " " << *cs << ",";
        }
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

char charFromV8(QV8Engine *, VHandle v)
{
    v8::String::AsciiValue cs(v);
    if (!cs.length())
        throw std::invalid_argument("String is not convertible to ascii");
    if (cs.length() > 1)
        throw std::invalid_argument("Expecting char but string length > 1");
    return cs.length() ? (*cs)[0] : 0;
}

QStringList QStringListFromV8(QV8Engine *, VHandle v)
{
    using namespace v8;
    if (!v->IsArray()) {
        throw std::invalid_argument("Not an array");
        return QStringList();
    }
    return QJSConverter::toStringList(Handle<Array>::Cast(v));
}

VHandle QStringListToV8(QStringList const &src)
{
    using namespace v8;
    auto res = Array::New(src.length());
    int i = 0;
    for (auto const &v : src)
        res->Set(i++, ValueToV8<QString>(v));
    return res;
}

}}
