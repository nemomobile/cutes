// -----------------------------------------------------------------------------
// THIS IS AUTOMATICALLY GENERATED FILE! DO NOT EDIT IT DIRECTLY!
// Original source file is core.cpp.in
// -----------------------------------------------------------------------------
/**
 * @file core.cpp
 *
 * @author (C) 2013 Jolla Ltd. Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 * @copyright LGPL 2.1 http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 *
 */
#include "util.hpp"

#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QByteArray>
#include <QMetaType>
#include <QMutex>
#include <QDateTime>

namespace cutes { namespace js {


class ByteArray;
template <>
struct JsTraits<QByteArray>
{
    typedef ByteArray js_type;
};
class ByteArray : public JsObject<ByteArray, QByteArray>
{

    Q_OBJECT
public:
    typedef JsObject<ByteArray, QByteArray> base_type;
    static QString className() { return "ByteArray"; }

    virtual ~ByteArray () {}

    ByteArray(QJSEngine &engine, QVariantList const &params);

    ByteArray(QJSEngine &e, QByteArray &&src);

    Q_INVOKABLE QJSValue mid(int p0, int p1);
    Q_INVOKABLE QJSValue left(int p0);
    Q_INVOKABLE QJSValue right(int p0);
    Q_INVOKABLE void append(QString const& p0);
    Q_INVOKABLE int indexOf(QString const& p0, int p1) const;
    Q_INVOKABLE int length() const;
    Q_INVOKABLE QString asString() const
    {
        QString res(*impl_);
        return res;
    }

    Q_INVOKABLE QVariantList split(char sep) const
    {
        return convert<QVariantList>(impl_->split(sep));
    }

    static ctors_type ctors_;
};

ByteArray::base_type::ctors_type ByteArray::ctors_ = {{
        [](QJSEngine &, QVariantList const &) {
            return std::make_shared<QByteArray>();
        },
        [](QJSEngine &, QVariantList const &values) {
            auto v = values[0];
            if (hasType(v, QMetaType::QString)) {
                auto res =  std::make_shared<QByteArray>();
                res->append(v.toString());
                return res;
            }
            return std::make_shared<QByteArray>();
        }
    }};


class Process;
template <>
struct JsTraits<QProcess>
{
    typedef Process js_type;
};
class Process : public JsObject<Process, QProcess>
{

    Q_OBJECT
public:
    typedef JsObject<Process, QProcess> base_type;
    static QString className() { return "Process"; }

    virtual ~Process () {}

    Process(QJSEngine &engine, QVariantList const &params);

    Q_ENUMS(ExitStatus)
    enum ExitStatus {
        NormalExit = QProcess::NormalExit,CrashExit = QProcess::CrashExit
    };

    Q_ENUMS(ProcessState)
    enum ProcessState {
        NotRunning = QProcess::NotRunning,Starting = QProcess::Starting,Running = QProcess::Running
    };

    Q_ENUMS(ProcessError)
    enum ProcessError {
        FailedToStart = QProcess::FailedToStart,Crashed = QProcess::Crashed,Timedout = QProcess::Timedout,WriteError = QProcess::WriteError,ReadError = QProcess::ReadError,UnknownError = QProcess::UnknownError
    };


    Q_INVOKABLE void start(QString const & p0, QStringList const & p1);
    Q_INVOKABLE bool waitForFinished(int p0);
    Q_INVOKABLE bool waitForStarted(int p0);
    Q_INVOKABLE void setWorkingDirectory(QString const& p0);
    Q_INVOKABLE int exitCode() const;
    Q_INVOKABLE int exitStatus() const;
    Q_INVOKABLE int error() const;
    Q_INVOKABLE int state() const;
    Q_INVOKABLE qint64 pid() const;

    Q_INVOKABLE QJSValue readAllStandardOutput();
    Q_INVOKABLE QJSValue readAllStandardError();

    Q_INVOKABLE qint64 write(QByteArray const& p0);
    Q_INVOKABLE void closeWriteChannel();
    static ctors_type ctors_;
};

Process::base_type::ctors_type Process::ctors_;


class File;
template <>
struct JsTraits<QFile>
{
    typedef File js_type;
};
class File : public JsObject<File, QFile>
{

    Q_OBJECT
public:
    typedef JsObject<File, QFile> base_type;
    static QString className() { return "File"; }

    virtual ~File () {}

    File(QJSEngine &engine, QVariantList const &params);

    Q_ENUMS(OpenMode)
    enum OpenMode {
        NotOpen = QIODevice::NotOpen,ReadOnly = QIODevice::ReadOnly,WriteOnly = QIODevice::WriteOnly,ReadWrite = QIODevice::ReadWrite,Append = QIODevice::Append,Truncate = QIODevice::Truncate,Text = QIODevice::Text,Unbuffered = QIODevice::Unbuffered
    };


    Q_ENUMS(Permission)
    enum Permission {
        ReadOwner = QFileDevice::ReadOwner,WriteOwner = QFileDevice::WriteOwner,ExeOwner = QFileDevice::ExeOwner,ReadUser = QFileDevice::ReadUser,WriteUser = QFileDevice::WriteUser,ExeUser = QFileDevice::ExeUser,ReadGroup = QFileDevice::ReadGroup,WriteGroup = QFileDevice::WriteGroup,ExeGroup = QFileDevice::ExeGroup,ReadOther = QFileDevice::ReadOther,WriteOther = QFileDevice::WriteOther,ExeOther = QFileDevice::ExeOther
    };


    Q_INVOKABLE void close();
    Q_INVOKABLE bool isOpen() const;
    Q_INVOKABLE int permissions();
    Q_INVOKABLE bool open(int p0);
    Q_INVOKABLE QJSValue readAll();
    Q_INVOKABLE qint64 write(QVariant const &);
    static ctors_type ctors_;
};

File::base_type::ctors_type File::ctors_ = {{
        [](QJSEngine &, QVariantList const &) {
            return std::make_shared<QFile>();
        },
        [](QJSEngine &, QVariantList const &values) {
            auto v = values[0];
            if (hasType(v, QMetaType::QString)) {
                return std::make_shared<QFile>(v.toString());
            }
            return std::make_shared<QFile>();
        }
    }};


class FileInfo;
template <>
struct JsTraits<QFileInfo>
{
    typedef FileInfo js_type;
};
class FileInfo : public JsObject<FileInfo, QFileInfo>
{

    Q_OBJECT
public:
    typedef JsObject<FileInfo, QFileInfo> base_type;
    static QString className() { return "FileInfo"; }

    virtual ~FileInfo () {}

    FileInfo(QJSEngine &engine, QVariantList const &params);

    FileInfo(QJSEngine &e, QFileInfo &&src);
    Q_INVOKABLE bool exists() const;
    Q_INVOKABLE bool isFile() const;
    Q_INVOKABLE bool caching() const;
    Q_INVOKABLE bool isSymLink() const;
    Q_INVOKABLE bool isDir() const;
    Q_INVOKABLE bool isExecutable() const;
    Q_INVOKABLE bool isWritable() const;
    Q_INVOKABLE bool isReadable() const;
    Q_INVOKABLE bool isAbsolute() const;
    Q_INVOKABLE bool isRelative() const;
    Q_INVOKABLE bool isRoot() const;
    Q_INVOKABLE bool isHidden() const;

    Q_INVOKABLE QDateTime lastModified() const;

    Q_INVOKABLE QString absoluteFilePath() const;
    Q_INVOKABLE QString absolutePath() const;
    Q_INVOKABLE QString baseName() const;
    Q_INVOKABLE QString canonicalFilePath() const;
    Q_INVOKABLE QString canonicalPath() const;
    Q_INVOKABLE QString completeBaseName() const;
    Q_INVOKABLE QString completeSuffix() const;
    Q_INVOKABLE QString fileName() const;
    Q_INVOKABLE QString filePath() const;
    Q_INVOKABLE QString group() const;
    Q_INVOKABLE QString owner() const;
    Q_INVOKABLE QString path() const;
    Q_INVOKABLE QString symLinkTarget() const;
    Q_INVOKABLE QString suffix() const;

    Q_INVOKABLE int permissions() const;
    Q_INVOKABLE unsigned groupId() const;
    Q_INVOKABLE int ownerId() const;

    Q_INVOKABLE QJSValue dir() const;
    Q_INVOKABLE qint64 size() const;
    static ctors_type ctors_;
};

FileInfo::base_type::ctors_type FileInfo::ctors_ = {{
        [](QJSEngine &, QVariantList const &) {
            return std::make_shared<QFileInfo>();
        },
        [](QJSEngine &, QVariantList const &values) {
            auto v = values[0];
            if (hasType(v, QMetaType::QString)) {
                return std::make_shared<QFileInfo>(v.toString());
            }
            return std::make_shared<QFileInfo>();
        }
    }};


class Dir;
template <>
struct JsTraits<QDir>
{
    typedef Dir js_type;
};
class Dir : public JsObject<Dir, QDir>
{

    Q_OBJECT
public:
    typedef JsObject<Dir, QDir> base_type;
    static QString className() { return "Dir"; }

    virtual ~Dir () {}

    Dir(QJSEngine &engine, QVariantList const &params);

    Dir(QJSEngine &e, QDir &&src);
    Q_ENUMS(Filter)
    enum Filter {
        Dirs = QDir::Dirs,AllDirs = QDir::AllDirs,Files = QDir::Files,Drives = QDir::Drives,NoSymLinks = QDir::NoSymLinks,NoDotAndDotDot = QDir::NoDotAndDotDot,NoDot = QDir::NoDot,NoDotDot = QDir::NoDotDot,AllEntries = QDir::AllEntries,Readable = QDir::Readable,Writable = QDir::Writable,Executable = QDir::Executable,Modified = QDir::Modified,Hidden = QDir::Hidden,System = QDir::System,CaseSensitive = QDir::CaseSensitive,NoFilter = QDir::NoFilter
    };

    Q_ENUMS(SortFlag)
    enum SortFlag {
        Name = QDir::Name,Time = QDir::Time,Size = QDir::Size,Unsorted = QDir::Unsorted,SortByMask = QDir::SortByMask,DirsFirst = QDir::DirsFirst,Reversed = QDir::Reversed,IgnoreCase = QDir::IgnoreCase,DirsLast = QDir::DirsLast,LocaleAware = QDir::LocaleAware,Type = QDir::Type,NoSort = QDir::NoSort
    };


    Q_INVOKABLE QString homePath();
    Q_INVOKABLE QString rootPath();

    Q_INVOKABLE bool mkdir(QString const& p0);
    Q_INVOKABLE QString relativeFilePath(QString const& p0);
    Q_INVOKABLE QString filePath(QString const& p0);
    Q_INVOKABLE bool rename(QString const& p0, QString const& p1);
    Q_INVOKABLE bool exists() const;
    Q_INVOKABLE bool isRoot() const;
    Q_INVOKABLE QString dirName() const;
    Q_INVOKABLE QString path() const;
    Q_INVOKABLE bool cdUp();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QStringList entryList(QStringList const& p0, int p1, int p2);
    Q_INVOKABLE QVariantList entryInfoList(const QStringList & nameFilters, int filters, int sort) const
    {
        return convert<QVariantList>
            (impl_->entryInfoList
             (nameFilters, (QDir::Filter)filters, (QDir::SortFlag)sort));
    }

    static ctors_type ctors_;
};

Dir::base_type::ctors_type Dir::ctors_ = {{
        [](QJSEngine &, QVariantList const &) {
            return std::make_shared<QDir>();
        },
        [](QJSEngine &, QVariantList const &values) {
            auto v = values[0];
            if (hasType(v, QMetaType::QString)) {
                return std::make_shared<QDir>(v.toString());
            }
            return std::make_shared<QDir>();
        }
    }};


class Mutex;
template <>
struct JsTraits<QMutex>
{
    typedef Mutex js_type;
};
class Mutex : public JsObject<Mutex, QMutex>
{

    Q_OBJECT
public:
    typedef JsObject<Mutex, QMutex> base_type;
    static QString className() { return "Mutex"; }

    virtual ~Mutex () {}

    Mutex(QJSEngine &engine, QVariantList const &params);

    Q_INVOKABLE void tryLock(int p0);
    Q_INVOKABLE void lock();
    Q_INVOKABLE void unlock();
    static ctors_type ctors_;
};

Mutex::base_type::ctors_type Mutex::ctors_;


ByteArray::ByteArray(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

ByteArray::ByteArray(QJSEngine &e, QByteArray &&src)
    : JsObject(e, std::make_shared<QByteArray>(std::move(src)))
{
}

QJSValue ByteArray::mid(int p0, int p1)
{
    return convert<QJSValue>(ByteArray(engine_, impl_->mid(p0, p1)));
}


QJSValue ByteArray::left(int p0)
{
    return convert<QJSValue>(ByteArray(engine_, impl_->left(p0)));
}


QJSValue ByteArray::right(int p0)
{
    return convert<QJSValue>(ByteArray(engine_, impl_->right(p0)));
}


void ByteArray::append(QString const& p0)
{
    impl_->append(p0);
}

int ByteArray::indexOf(QString const& p0, int p1) const
{
    return impl_->indexOf(p0, p1);
}

int ByteArray::length() const
{
    return impl_->length();
}

Process::Process(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

void Process::start(QString const & p0, QStringList const & p1)
{
    impl_->start(p0, p1);
}

bool Process::waitForFinished(int p0)
{
    return impl_->waitForFinished(p0);
}

bool Process::waitForStarted(int p0)
{
    return impl_->waitForStarted(p0);
}

void Process::setWorkingDirectory(QString const& p0)
{
    impl_->setWorkingDirectory(p0);
}

int Process::exitCode() const
{
    return impl_->exitCode();
}

int Process::exitStatus() const
{
    return impl_->exitStatus();
}

int Process::error() const
{
    return impl_->error();
}

int Process::state() const
{
    return impl_->state();
}

qint64 Process::pid() const
{
    return impl_->pid();
}

QJSValue Process::readAllStandardOutput()
{
    return convert<QJSValue>(ByteArray(engine_, impl_->readAllStandardOutput()));
}


QJSValue Process::readAllStandardError()
{
    return convert<QJSValue>(ByteArray(engine_, impl_->readAllStandardError()));
}


qint64 Process::write(QByteArray const& p0)
{
    return impl_->write(p0);
}

void Process::closeWriteChannel()
{
    impl_->closeWriteChannel();
}

File::File(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

void File::close()
{
    impl_->close();
}

bool File::isOpen() const
{
    return impl_->isOpen();
}

int File::permissions()
{
    return impl_->permissions();
}

bool File::open(int p0)
{
    return impl_->open((QIODevice::OpenModeFlag)p0);
}

QJSValue File::readAll()
{
    return convert<QJSValue>(ByteArray(engine_, impl_->readAll()));
}


qint64 File::write(QVariant const &p0)
{
    auto p = castCutesObj<ByteArray>(p0);
    if (!p)
        return 0;
    return impl_->write(*(p->impl_));
}

FileInfo::FileInfo(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

FileInfo::FileInfo(QJSEngine &e, QFileInfo &&src)
    : JsObject(e, std::make_shared<QFileInfo>(std::move(src)))
{
}

bool FileInfo::exists() const
{
    return impl_->exists();
}

bool FileInfo::isFile() const
{
    return impl_->isFile();
}

bool FileInfo::caching() const
{
    return impl_->caching();
}

bool FileInfo::isSymLink() const
{
    return impl_->isSymLink();
}

bool FileInfo::isDir() const
{
    return impl_->isDir();
}

bool FileInfo::isExecutable() const
{
    return impl_->isExecutable();
}

bool FileInfo::isWritable() const
{
    return impl_->isWritable();
}

bool FileInfo::isReadable() const
{
    return impl_->isReadable();
}

bool FileInfo::isAbsolute() const
{
    return impl_->isAbsolute();
}

bool FileInfo::isRelative() const
{
    return impl_->isRelative();
}

bool FileInfo::isRoot() const
{
    return impl_->isRoot();
}

bool FileInfo::isHidden() const
{
    return impl_->isHidden();
}

QDateTime FileInfo::lastModified() const
{
    return impl_->lastModified();
}

QString FileInfo::absoluteFilePath() const
{
    return impl_->absoluteFilePath();
}

QString FileInfo::absolutePath() const
{
    return impl_->absolutePath();
}

QString FileInfo::baseName() const
{
    return impl_->baseName();
}

QString FileInfo::canonicalFilePath() const
{
    return impl_->canonicalFilePath();
}

QString FileInfo::canonicalPath() const
{
    return impl_->canonicalPath();
}

QString FileInfo::completeBaseName() const
{
    return impl_->completeBaseName();
}

QString FileInfo::completeSuffix() const
{
    return impl_->completeSuffix();
}

QString FileInfo::fileName() const
{
    return impl_->fileName();
}

QString FileInfo::filePath() const
{
    return impl_->filePath();
}

QString FileInfo::group() const
{
    return impl_->group();
}

QString FileInfo::owner() const
{
    return impl_->owner();
}

QString FileInfo::path() const
{
    return impl_->path();
}

QString FileInfo::symLinkTarget() const
{
    return impl_->symLinkTarget();
}

QString FileInfo::suffix() const
{
    return impl_->suffix();
}

int FileInfo::permissions() const
{
    return impl_->permissions();
}

unsigned FileInfo::groupId() const
{
    return impl_->groupId();
}

int FileInfo::ownerId() const
{
    return impl_->ownerId();
}

QJSValue FileInfo::dir() const
{
    return convert<QJSValue>(Dir(engine_, impl_->dir()));
}


qint64 FileInfo::size() const
{
    return impl_->size();
}

Dir::Dir(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

Dir::Dir(QJSEngine &e, QDir &&src)
    : JsObject(e, std::make_shared<QDir>(std::move(src)))
{
}

QString Dir::homePath()
{
    return impl_->homePath();
}

QString Dir::rootPath()
{
    return impl_->rootPath();
}

bool Dir::mkdir(QString const& p0)
{
    return impl_->mkdir(p0);
}

QString Dir::relativeFilePath(QString const& p0)
{
    return impl_->relativeFilePath(p0);
}

QString Dir::filePath(QString const& p0)
{
    return impl_->filePath(p0);
}

bool Dir::rename(QString const& p0, QString const& p1)
{
    return impl_->rename(p0, p1);
}

bool Dir::exists() const
{
    return impl_->exists();
}

bool Dir::isRoot() const
{
    return impl_->isRoot();
}

QString Dir::dirName() const
{
    return impl_->dirName();
}

QString Dir::path() const
{
    return impl_->path();
}

bool Dir::cdUp()
{
    return impl_->cdUp();
}

void Dir::refresh()
{
    impl_->refresh();
}

QStringList Dir::entryList(QStringList const& p0, int p1, int p2)
{
    return impl_->entryList(p0, (QDir::Filter)p1, (QDir::SortFlag)p2);
}

Mutex::Mutex(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{
}

void Mutex::tryLock(int p0)
{
    impl_->tryLock(p0);
}

void Mutex::lock()
{
    impl_->lock();
}

void Mutex::unlock()
{
    impl_->unlock();
}
void ObjectFactory::init()
{
    addClass<ByteArray>("ByteArray");
    addClass<Process>("Process");
    addClass<Mutex>("Mutex");
    addClass<File>("File");
    addClass<FileInfo>("FileInfo");
    addClass<Dir>("Dir");
}

}}


extern "C" QJSValue cutesRegister(QJSEngine *engine)
{
    using cutes::js::ObjectFactory;
    auto p = cor::make_unique<ObjectFactory>(engine);
    return engine->newQObject(p.release());
}

#include "core.moc"
