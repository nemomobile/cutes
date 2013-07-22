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
