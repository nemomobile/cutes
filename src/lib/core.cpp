#include <cutes/core.hpp>

namespace cutes { namespace js {

v8::Persistent<v8::FunctionTemplate> IODevice::cutesCtor_;

IODevice::IODevice(v8::Arguments const&)
{
}

#define IODEVICE_CONST(name) CUTES_CONST(name, QIODevice)

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
    : base_type(Arg<QByteArray>(args, 0))
{
}

VHandle ByteArray::toString(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = cutesObjFromThis<base_type>(args);
            return ValueToV8(QString(*self));
        });
}

void ByteArray::v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, obj)
        << CUTES_FN(toString, ByteArray);
}

v8::Persistent<v8::FunctionTemplate> File::cutesCtor_;

File::File(v8::Arguments const& args)
    : base_type(Arg<QString>(args, 0))
{
}

VHandle File::open(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = cutesObjFromThis<base_type>(args);
            auto p0 = Arg<int>(args, 0);
            auto mode = static_cast<QIODevice::OpenModeFlag>(p0);
            return ValueToV8(self->open(mode));
        });
}

void File::v8Setup(QV8Engine *v8e
                   , v8::Handle<v8::FunctionTemplate>
                   , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, obj)
        << CUTES_FN(open, File)
        << CUTES_FN_PARAM(write, qint64, QFile, QIODevice
                             , QByteArray, const QByteArray&)
        << CUTES_VOID_FN(close, QFile, QIODevice)
        << CUTES_GET_CONST(isOpen, bool, QFile, QIODevice)
        << CUTES_GET(readAll, QByteArray, QFile, QIODevice)
        ;
}

v8::Persistent<v8::FunctionTemplate> FileInfo::cutesCtor_;

FileInfo::FileInfo(v8::Arguments const& args)
    : base_type(Arg<QString>(args, 0))
{
}

#define BOOL_QUERY_(name) \
    CUTES_GET_CONST(name, bool, QFileInfo, QFileInfo)

#define STR_QUERY_(name) \
    CUTES_GET_CONST(name, QString, QFileInfo, QFileInfo)

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
        << CUTES_GET_CONST(dir, QDir, QFileInfo, QFileInfo)
        ;
}

#undef BOOL_QUERY_
#undef STR_QUERY_

v8::Persistent<v8::FunctionTemplate> Dir::cutesCtor_;

Dir::Dir(v8::Arguments const &args)
    : base_type(Arg<QString>(args, 0))
{
}

VHandle Dir::entryInfoList(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = cutesObjFromThis<base_type>(args);
            auto p0 = Arg<QStringList>(args, 0);
            auto p1 = args.Length() > 1
                ? Arg<QDir::Filter>(args, 1)
                : QDir::NoFilter;
            auto p2 = args.Length() > 2
                ? Arg<QDir::SortFlag>(args, 2)
                : QDir::NoSort;
            return ValueToV8(self->entryInfoList(p0, p1, p2));
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
    CUTES_GET_CONST(name, bool, QDir, QDir)

#define STR_QUERY_(name) \
    CUTES_GET_CONST(name, QString, QDir, QDir)

#define CONST_(name) CUTES_CONST(name, QDir)

void Dir::v8Setup(QV8Engine *v8e
                  , v8::Handle<v8::FunctionTemplate> cls
                  , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, cls)
        << CONST_(Dirs)
        << CONST_(AllDirs)
        << CONST_(Files)
        << CONST_(Drives)
        << CONST_(NoSymLinks)
        << CONST_(NoDotAndDotDot)
        << CONST_(NoDot)
        << CONST_(NoDotDot)
        << CONST_(AllEntries)
        << CONST_(Readable)
        << CONST_(Writable)
        << CONST_(Executable)
        << CONST_(Modified)
        << CONST_(Hidden)
        << CONST_(System)
        << CONST_(CaseSensitive)
        << CONST_(NoFilter)

        << CONST_(Name)
        << CONST_(Time)
        << CONST_(Size)
        << CONST_(Unsorted)
        << CONST_(SortByMask)
        << CONST_(DirsFirst)
        << CONST_(Reversed)
        << CONST_(IgnoreCase)
        << CONST_(DirsLast)
        << CONST_(LocaleAware)
        << CONST_(Type)
        << CONST_(NoSort)

        << CUTES_FN(homePath, Dir)
        << CUTES_FN(rootPath, Dir)
        ;
    setupTemplate(v8e, obj)
        << CUTES_FN_PARAM_CONST(mkdir, bool, QDir, QDir, QString, const QString&)
        << BOOL_QUERY_(exists)
        << BOOL_QUERY_(isRoot)
        << STR_QUERY_(dirName)
        << STR_QUERY_(path)
        << CUTES_GET(cdUp, bool, QDir, QDir)
        << CUTES_FN(entryInfoList, Dir)
        ;
}

#undef BOOL_QUERY_
#undef STR_QUERY_
#undef CONST_

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

VHandle Process::start(const v8::Arguments &args)
{
    return callConvertException
        (args, [](const v8::Arguments &args) -> VHandle {
            auto self = cutesObjFromThis<base_type>(args);
            auto p0 = Arg<QString>(args, 0);
            auto p1 = Arg<QStringList>(args, 1);
            auto p2 = (args.Length() == 3)
                ? Arg<QIODevice::OpenModeFlag>(args, 2)
                : QIODevice::ReadWrite;
            self->start(p0, p1, p2);
            return v8::Undefined();
        });
}

#define QUERY_(name, type)                          \
    CUTES_GET_CONST(name, type, QProcess, QProcess)

#define SIMPLE_(name, type)\
    CUTES_GET(name, type, QProcess, QProcess)

void Process::v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate> cls
                        , v8::Handle<v8::ObjectTemplate> obj)
{
    setupTemplate(v8e, cls)
        << CUTES_CONST(NormalExit, QProcess)
        << CUTES_CONST(CrashExit, QProcess)
        << CUTES_CONST(NotRunning, QProcess)
        << CUTES_CONST(Starting, QProcess)
        << CUTES_CONST(Running, QProcess)
        ;
    setupTemplate(v8e, obj)
        << CUTES_FN_PARAM(waitForFinished, bool, QProcess, QProcess
                          , int, int)
        << CUTES_FN_PARAM(waitForStarted, bool, QProcess, QProcess
                          , int, int)
        << CUTES_FN_PARAM(setWorkingDirectory, void, QProcess, QProcess
                          , QString, QString const&)
        << CUTES_FN(start, Process)
        << SIMPLE_(readAllStandardOutput, QByteArray)
        << SIMPLE_(readAllStandardError, QByteArray)
        << QUERY_(exitCode, int)
        << QUERY_(exitStatus, QProcess::ExitStatus)
        ;
}

#undef QUERY_
#undef SIMPLE_

extern "C" void registerLibrary(QV8Engine *v8e)
{
    using namespace cutes::js;
    v8::HandleScope hscope;
    auto global = v8e->global();
    v8EngineAdd(v8e, global)
            << v8Class<File>("QFile")
            << v8Class<FileInfo>("QFileInfo")
            << v8Class<IODevice>("QIODevice")
            << v8Class<ByteArray>("QByteArray")
            << v8Class<Dir>("QDir")
            << v8Class<Process>("QProcess");
}

}}
