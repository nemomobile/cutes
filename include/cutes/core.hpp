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
    typedef IODevice impl_type; // QIODevice is abstract
    typedef QIODevice qt_type;

    IODevice(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

class ByteArray : public QByteArray
{
public:
    typedef QByteArray impl_type;
    typedef QByteArray qt_type;

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
        return ValueFromV8<QString>(e, v).toUtf8();
    } 
    static inline VHandle toV8(QByteArray const& v)
    {
        return objToV8<QByteArray>(v);
    }
};

class File : public QFile
{
public:
    typedef QFile impl_type;
    typedef QFile qt_type;

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
    typedef QFileInfo impl_type;
    typedef QFileInfo qt_type;

    FileInfo(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);

};

template <> struct ObjectTraits<QFileInfo>
{
    typedef FileInfo js_type;
};

class Dir : public QDir
{
public:
    typedef QDir impl_type;
    typedef QDir qt_type;

    Dir(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle rootPath(const v8::Arguments &);
    static VHandle homePath(const v8::Arguments &);
    static VHandle entryInfoList(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

template <> struct ObjectTraits<QDir>
{
    typedef Dir js_type;
};

class Process : public QProcess
{
public:
    typedef QProcess impl_type;
    typedef QProcess qt_type;

    Process(v8::Arguments const&);

    static v8::Persistent<v8::FunctionTemplate> cutesCtor_;

    static VHandle start(const v8::Arguments &);

    static void v8Setup(QV8Engine *
                        , v8::Handle<v8::FunctionTemplate>
                        , v8::Handle<v8::ObjectTemplate>);
};

}}

#endif // _CUTES_JS_OS_HPP_
