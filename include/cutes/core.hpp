#ifndef _CUTES_JS_OS_HPP_
#define _CUTES_JS_OS_HPP_

#include <cutes/util.hpp>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

namespace cutes { namespace js {

class IODevice
{
public:
    typedef IODevice base_type; // QIODevice is abstract
    typedef QIODevice impl_type;

    IODevice(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

CUTES_FLAG_CONVERTIBLE_INT(QIODevice::OpenModeFlag);

class ByteArray : public QByteArray
{
public:
    typedef QByteArray base_type;
    typedef QByteArray impl_type;

    ByteArray(v8::Arguments const &args);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle toString(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

template <> struct ObjectTraits<QByteArray>
{
    typedef ByteArray js_type;
};

template<> struct Convert<QByteArray> {
    static inline QByteArray fromV8(QV8Engine *e, VHandle v)
    {
        return v->IsString()
            ? ValueFromV8<QString>(e, v).toUtf8()
            : *cutesObjFromV8<QByteArray>(e, v);
    }
    static inline VHandle toV8(QByteArray const& v)
    {
        return objToV8<QByteArray>(v);
    }
};

class File : public QFile
{
public:
    typedef QFile base_type;
    typedef QFile impl_type;

    File(v8::Arguments const& args);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle open(const v8::Arguments &);

    static void v8Setup(QV8Engine *v8e
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

class FileInfo : public QFileInfo
{
public:
    typedef QFileInfo base_type;
    typedef QFileInfo impl_type;

    FileInfo(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);

};

CUTES_CONVERTIBLE_COPYABLE(QFileInfo, FileInfo);

class Dir : public QDir
{
public:
    typedef QDir base_type;
    typedef QDir impl_type;

    Dir(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle rootPath(const v8::Arguments &);
    static VHandle homePath(const v8::Arguments &);
    static VHandle entryInfoList(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

CUTES_CONVERTIBLE_COPYABLE(QDir, Dir);
CUTES_FLAG_CONVERTIBLE_INT(QDir::Filter);
CUTES_FLAG_CONVERTIBLE_INT(QDir::SortFlag);

class Process : public QProcess
{
public:
    typedef QProcess base_type;
    typedef QProcess impl_type;

    Process(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle start(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

}}

#endif // _CUTES_JS_OS_HPP_
