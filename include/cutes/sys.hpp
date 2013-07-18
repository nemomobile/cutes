#ifndef _CUTES_JS_SYS_HPP_
#define _CUTES_JS_SYS_HPP_

#include <cutes/util.hpp>

#include <QFile>
#include <QFileInfo>

namespace cutes { namespace js {

class IODevice
{
public:
    typedef IODevice base_type;

    IODevice(v8::Arguments const& args)
    {
    }

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
    {
        setupTemplate(v8e, cls)
            << Const("NotOpen", QIODevice::NotOpen)
            << Const("NotOpen", QIODevice::NotOpen)
            << Const("ReadOnly", QIODevice::ReadOnly)
            << Const("WriteOnly", QIODevice::WriteOnly)
            << Const("ReadWrite", QIODevice::ReadWrite)
            << Const("Append", QIODevice::Append)
            << Const("Truncate", QIODevice::Truncate)
            << Const("Text", QIODevice::Text)
            << Const("Unbuffered", QIODevice::Unbuffered);
    }

};



class File : public QFile
{
public:
    typedef QFile base_type;

    File(v8::Arguments const& args)
        : base_type(Arg<QString>(args, 0))
    {
    }

    virtual ~File() { std::cerr << "~File"; }

    static v8::Handle<v8::Value> open(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                auto p0 = Arg<int>(args, 0);
                auto mode = static_cast<QIODevice::OpenModeFlag>(p0);
                return ValueToV8(self->open(mode));
            });
    }

    static v8::Handle<v8::Value> isOpen(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                return ValueToV8(self->isOpen());
            });
    }

    static v8::Handle<v8::Value> write(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                auto p0 = Arg<QByteArray>(args, 0);
                return ValueToV8(self->write(p0));
            });
    }

    static v8::Handle<v8::Value> close(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                self->close();
                return v8::Undefined();
            });
    }

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
    {
        setupTemplate(v8e, obj)
            << Callback("open", &File::open)
            << Callback("close", &File::close)
            << Callback("write", &File::write)
            << Callback("isOpen", &File::isOpen);
    }

};

class FileInfo : public QFileInfo
{
public:
    typedef QFileInfo base_type;

    FileInfo(v8::Arguments const& args)
        : base_type(Arg<QString>(args, 0))
    {
    }

    static v8::Handle<v8::Value> exists(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                return ValueToV8(self->exists());
            });
    }

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
    {
        setupTemplate(v8e, obj)
            << Callback("exists", &FileInfo::exists);
    }

};

}}

#endif // _CUTES_JS_SYS_HPP_
