#include <cutes/core.hpp>

namespace cutes { namespace js {

v8::Persistent<v8::FunctionTemplate> IODevice::cutesCtor_;

IODevice::IODevice(v8::Arguments const&)
{
}

template<> struct Convert<QIODevice::OpenModeFlag> {
    static inline QIODevice::OpenModeFlag fromV8(QV8Engine *e, VHandle v)
    {
        return (QIODevice::OpenModeFlag)ValueFromV8<int>(e, v);
    } 
    static inline VHandle toV8(QIODevice::OpenModeFlag const& v)
    {
        return ValueToV8((int)v);
    }
};

#define IODEVICE_CONST(name) CUTES_JS_CONST(name, QIODevice)

void IODevice::v8Setup(QV8Engine *v8e
                       , v8::Handle<v8::FunctionTemplate> cls
                       , v8::Handle<v8::ObjectTemplate>)
{
    setupTemplate(v8e, cls)
        << IODEVICE_CONST(NotOpen)
        << IODEVICE_CONST(ReadOnly)
        << IODEVICE_CONST(WriteOnly)
        << IODEVICE_CONST(ReadWrite)
        << IODEVICE_CONST(Append)
        << IODEVICE_CONST(Truncate)
        << IODEVICE_CONST(Text)
        << IODEVICE_CONST(Unbuffered);
}

#undef IODEVICE_CONST

v8::Persistent<v8::FunctionTemplate> ByteArray::cutesCtor_;

ByteArray::ByteArray(v8::Arguments const &args)
    : impl_type(Arg<QByteArray>(args, 0))
{
}

VHandle ByteArray::toString(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            return ValueToV8(QString(*self));
        });
}

void ByteArray::v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, obj)
        << CUTES_JS_FN(toString, ByteArray);
}

v8::Persistent<v8::FunctionTemplate> File::cutesCtor_;

File::File(v8::Arguments const& args)
    : impl_type(Arg<QString>(args, 0))
{
}

VHandle File::open(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<int>(args, 0);
            auto mode = static_cast<QIODevice::OpenModeFlag>(p0);
            return ValueToV8(self->open(mode));
        });
}

VHandle File::write(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<QByteArray>(args, 0);
            return ValueToV8(self->write(p0));
        });
}

void File::v8Setup(QV8Engine *v8e
                   , v8::Handle<v8::FunctionTemplate>
                   , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, obj)
        << CUTES_JS_FN(open, File)
        << CUTES_JS_FN(write, File)
        << CUTES_JS_FN_VOID(close, QFile, QIODevice)
        << CUTES_JS_QUERY(isOpen, QFile, QIODevice, bool)
        << CUTES_JS_FN_RES(readAll, QFile, QIODevice, QByteArray)
        ;
}

v8::Persistent<v8::FunctionTemplate> FileInfo::cutesCtor_;

FileInfo::FileInfo(v8::Arguments const& args)
    : impl_type(Arg<QString>(args, 0))
{
}

#define BOOL_QUERY_(name) \
    CUTES_JS_QUERY(name, QFileInfo, QFileInfo, bool)

#define STR_QUERY_(name) \
    CUTES_JS_QUERY(name, QFileInfo, QFileInfo, QString)

void FileInfo::v8Setup(QV8Engine *v8e
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
        << CUTES_JS_QUERY(dir, QFileInfo, QFileInfo, QDir)
        ;
}

#undef BOOL_QUERY_
#undef STR_QUERY_

v8::Persistent<v8::FunctionTemplate> Dir::cutesCtor_;

Dir::Dir(v8::Arguments const &args)
    : impl_type(Arg<QString>(args, 0))
{
}

VHandle Dir::mkdir(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<QString>(args, 0);
            return ValueToV8(self->mkdir(p0));
        });
}

VHandle Dir::homePath(const v8::Arguments &)
{
    return ValueToV8(QDir::homePath());
}

VHandle Dir::rootPath(const v8::Arguments &)
{
    return ValueToV8(QDir::rootPath());
}

#define BOOL_QUERY_(name) \
    CUTES_JS_QUERY(name, QDir, QDir, bool)

#define STR_QUERY_(name) \
    CUTES_JS_QUERY(name, QDir, QDir, QString)

void Dir::v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, cls)
        << CUTES_JS_FN(homePath, Dir)
        << CUTES_JS_FN(rootPath, Dir)
        ;
    setupTemplate(v8e, obj)
        << CUTES_JS_FN(mkdir, Dir)
        << BOOL_QUERY_(exists)
        << STR_QUERY_(dirName)
        << STR_QUERY_(path)
        << CUTES_JS_FN_RES(cdUp, QDir, QDir, bool)
        ;
}

#undef BOOL_QUERY_
#undef STR_QUERY_


template<> struct Convert<QProcess::ExitStatus> {
    static inline VHandle toV8(QProcess::ExitStatus v)
    {
        return ValueToV8((int)v);
    }
};

template<> struct Convert<QProcess::ProcessState> {
    static inline VHandle toV8(QProcess::ProcessState v)
    {
        return ValueToV8((int)v);
    }
};

v8::Persistent<v8::FunctionTemplate> Process::cutesCtor_;

Process::Process(v8::Arguments const &)
{
}

VHandle Process::waitForStarted(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<int>(args, 0);
            return ValueToV8(self->waitForStarted(p0));
        });
}

VHandle Process::waitForFinished(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<int>(args, 0);
            return ValueToV8(self->waitForFinished(p0));
        });
}

VHandle Process::start(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = QObjFromV8This<impl_type>(args);
            auto p0 = Arg<QString>(args, 0);
            auto p1 = Arg<QStringList>(args, 1);
            auto p2 = Arg<QIODevice::OpenModeFlag>(args, 2);
            self->start(p0, p1, p2);
            return v8::Undefined();
        });
}

#define QUERY_(name, type)                          \
    CUTES_JS_QUERY(name, QProcess, QProcess, type)

#define SIMPLE_(name, type)\
    CUTES_JS_FN_RES(name, QProcess, QProcess, type)

void Process::v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, cls)
        << CUTES_JS_CONST(NormalExit, QProcess)
        << CUTES_JS_CONST(CrashExit, QProcess)
        << CUTES_JS_CONST(NotRunning, QProcess)
        << CUTES_JS_CONST(Starting, QProcess)
        << CUTES_JS_CONST(Running, QProcess)
        ;
    setupTemplate(v8e, obj)
        << CUTES_JS_FN(waitForFinished, Process)
        << CUTES_JS_FN(waitForStarted, Process)
        << CUTES_JS_FN(start, Process)
        << SIMPLE_(readAllStandardOutput, QByteArray)
        << SIMPLE_(readAllStandardError, QByteArray)
        << QUERY_(exitCode, int)
        << QUERY_(exitStatus, QProcess::ExitStatus)
        ;
}

#undef QUERY_
#undef SIMPLE_

}}
