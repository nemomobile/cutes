#ifndef _CUTES_JS_OS_HPP_
#define _CUTES_JS_OS_HPP_

#include <cutes/util.hpp>

#include <QFile>
#include <QFileInfo>

namespace cutes { namespace js {

template<> struct Convert<QIODevice::OpenModeFlag> {
    static inline v8::Handle<v8::Value> toV8(QIODevice::OpenModeFlag v)
    {
        return ValueToV8((int)v);
    }
};

class IODevice
{
public:
    typedef IODevice impl_type;

    IODevice(v8::Arguments const&)
    {
    }

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

class ByteArray : public QByteArray
{
public:
    typedef QByteArray impl_type;

    ByteArray(v8::Arguments const &args);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static v8::Handle<v8::Value> toString(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

template <> struct ObjectTraits<QByteArray>
{
    typedef ByteArray js_type;
};

class File : public QFile
{
public:
    typedef QFile impl_type;

    File(v8::Arguments const& args);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static v8::Handle<v8::Value> open(const v8::Arguments &);
    static v8::Handle<v8::Value> write(const v8::Arguments &);

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

class FileInfo : public QFileInfo
{
public:
    typedef QFileInfo impl_type;

    FileInfo(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);

};

}}

#endif // _CUTES_JS_OS_HPP_
