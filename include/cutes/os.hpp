#ifndef _CUTES_JS_OS_HPP_
#define _CUTES_JS_OS_HPP_

#include <cutes/util.hpp>

#include <QFile>
#include <QFileInfo>

namespace cutes { namespace js {

class IODevice
{
public:
    typedef IODevice base_type;

    IODevice(v8::Arguments const&)
    {
    }

    virtual ~IODevice() {}

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate>)
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

    virtual ~File() {}

    static v8::Handle<v8::Value> open(const v8::Arguments &args)
    {
        return callConvertException([&args]() {
                auto self = QObjFromV8This<base_type>(args);
                auto p0 = Arg<int>(args, 0);
                auto mode = static_cast<QIODevice::OpenModeFlag>(p0);
                return ValueToV8(self->open(mode));
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

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate> obj)
    {
        setupTemplate(v8e, obj)
            << Callback("open", &File::open)
            << Callback("write", &File::write)
            << Callback("close", simpleFnVoid<QFile, void(QIODevice::*)()
                        , &QIODevice::close>)
            << CUTES_JS_QUERY(isOpen, QFile, QIODevice, bool)
            ;
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

    virtual ~FileInfo() {}

#define BOOL_QUERY_(name) \
    CUTES_JS_QUERY(name, QFileInfo, QFileInfo, bool)

#define STR_QUERY_(name) \
    CUTES_JS_QUERY(name, QFileInfo, QFileInfo, QString)

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate> obj)
    {
        setupTemplate(v8e, obj)
            << BOOL_QUERY_(exists)
            << BOOL_QUERY_(isFile)
            << BOOL_QUERY_(isSymLink)
            << BOOL_QUERY_(isDir)
            << BOOL_QUERY_(isExecutable)
            << BOOL_QUERY_(isWritable)
            << BOOL_QUERY_(isReadable)
            << BOOL_QUERY_(isAbsolute)
            << BOOL_QUERY_(isRelative)
            << BOOL_QUERY_(isRoot)
            << BOOL_QUERY_(isHidden)
            << STR_QUERY_(owner)
            << STR_QUERY_(group)
            << STR_QUERY_(path)
            << STR_QUERY_(fileName)
            << STR_QUERY_(filePath)
            << STR_QUERY_(canonicalFilePath)
            << STR_QUERY_(canonicalPath)
            << STR_QUERY_(baseName)
            << STR_QUERY_(completeBaseName)
            << STR_QUERY_(completeSuffix)
            << STR_QUERY_(absoluteFilePath)
            << STR_QUERY_(absolutePath)
            ;
    }

#undef BOOL_QUERY_
#undef STR_QUERY_

};

}}

#endif // _CUTES_JS_OS_HPP_
