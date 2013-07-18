#ifndef _CUTES_JS_UTIL_HPP_
#define _CUTES_JS_UTIL_HPP_

#include <QtQml/private/qv8engine_p.h>
#include <QtQml/private/qjsconverter_p.h>
#include <QtQml/private/qjsconverter_impl_p.h>

#include <QString>
#include <QVariant>

#include <functional>
#include <stdexcept>

namespace cutes { namespace js {

static inline v8::Local<v8::Value> Get(v8::Handle<v8::Object> v, char const *name)
{
    return v->Get(v8::String::New(name));
}

static inline void Set
(v8::Handle<v8::Value> o, char const *name, v8::Handle<v8::Value> v)
{
    return o->Set(v8::String::New(name), v);
}

template <typename FnT>
v8::Handle<v8::Value> callConvertException(FnT fn)
{
    try {
        return fn();
    } catch (std::exception const &e) {
        using namespace v8;
        ThrowException(Exception::Error(String::New(e.what())));
        return Handle<Value>();
    }
}

template <typename ResT>
ResT *QObjFromV8This(const v8::Arguments &args)
{
    return reinterpret_cast<ResT*>
        (v8::External::Unwrap(args.This()->GetInternalField(0)));
}

template <typename T>
T ValueFromV8(QV8Engine *, v8::Handle<v8::Value>);

template <>
QString ValueFromV8<QString>(QV8Engine *, v8::Handle<v8::Value> v)
{
    return QJSConverter::toString(v->ToString());
}

template <>
int ValueFromV8<int>(QV8Engine *, v8::Handle<v8::Value> v)
{
    using namespace v8;
    if (!v->IsNumber()) {
        throw std::invalid_argument("Not a number");
        return 0;
    }
    return v->ToInteger()->Value();
}

template <>
QVariant ValueFromV8<QVariant>(QV8Engine *e, v8::Handle<v8::Value> v)
{
    return e->variantFromJS(v);
}

template <>
QByteArray ValueFromV8<QByteArray>(QV8Engine *e, v8::Handle<v8::Value> v)
{
    return ValueFromV8<QString>(e, v).toUtf8();
}

inline v8::Handle<v8::String> ValueToV8(QString const& v)
{
    return QJSConverter::toString(v);
}

inline v8::Handle<v8::Integer> ValueToV8(int v)
{
    return v8::Integer::New(v);
}

inline v8::Handle<v8::Number> ValueToV8(long long v)
{
    return v8::Number::New(v);
}

inline v8::Handle<v8::Boolean> ValueToV8(bool v)
{
    return v8::Boolean::New(v);
}

inline v8::Handle<v8::Value> ValueToV8(void)
{
    return v8::Undefined();
}

template <typename T>
void v8DeleteCppObj(v8::Persistent<v8::Value>, void* p)
{
    qDebug() << "Deleting " << p;
    delete reinterpret_cast<T*>(p);
}

template <typename T, typename BaseT>
static v8::Handle<v8::Value> v8Ctor(const v8::Arguments &args)
{
    using namespace v8;
    Local<External> external;
    auto self = args.This();
    
    if (!args.IsConstructCall())
        V8THROW_ERROR("Call function as ctor");

    if (args[0]->IsExternal()) {
        external = Local<External>::Cast(args[0]);
    } else {
        T *p = new T(args);
        external = External::New(static_cast<BaseT*>(p));
        auto ph = Persistent<Value>::New(args.This());
        ph.MakeWeak(p, &v8DeleteCppObj<T>);
    }
    args.This()->SetInternalField(0, external);
    return args.This();
}

template <typename T>
static void v8EngineAdd(QV8Engine *v8e, char const *name)
{
    using namespace v8;
    HandleScope hscope;

    Handle<FunctionTemplate> ctor 
        = FunctionTemplate::New(v8Ctor<T, typename T::base_type>
                                , External::New(v8e));

    Handle<ObjectTemplate> tpl = ctor->InstanceTemplate();
    tpl->SetInternalFieldCount(1);

    Set(tpl, "cutesClass__", v8::String::New(name));
    
    T::v8Setup(v8e, ctor, tpl);
    v8e->global()->Set(v8::String::New(name), ctor->GetFunction());
}

class TemplateInitializer
{
public:
    TemplateInitializer(QV8Engine *v8e, v8::Handle<v8::Template> target)
        : engine_(v8e), target_(target)
    {}

    template <typename FnT>
    TemplateInitializer const& setFn(char const *name, FnT fn) const
    {
        target_->Set(name, V8FUNCTION(fn, engine_));
        return *this;
    }

    template <typename T>
    TemplateInitializer const& setConst(char const *name, T v) const
    {
        target_->Set(name, ValueToV8(v));
        return *this;
    }

private:
    QV8Engine *engine_;
    v8::Handle<v8::Template> target_;
};

TemplateInitializer setupTemplate(QV8Engine *engine, v8::Handle<v8::Template> target)
{
    TemplateInitializer self(engine, target);
    return self;
}

template <typename FnT>
struct FunctionInfo
{
    FunctionInfo(char const *name_, FnT fn_) : name(name_), fn(fn_) {}
    char const *name;
    FnT fn;
};

template <typename FnT>
FunctionInfo<FnT> Callback(char const *name, FnT fn)
{
    return FunctionInfo<FnT>(name, fn);
}

template <typename T>
struct ConstInfo
{
    ConstInfo(char const *name_, T v_) : name(name_), v(v_) {}
    char const *name;
    T v;
};

template <typename T>
ConstInfo<T> Const(char const *name, T v)
{
    return ConstInfo<T>(name, v);
}

template <typename FnT>
TemplateInitializer const& operator << (TemplateInitializer const& dst
                                   , FunctionInfo<FnT> const &fn)
{
    return dst.setFn(fn.name, fn.fn);
}

template <typename T>
TemplateInitializer const& operator << (TemplateInitializer const& dst
                                   , ConstInfo<T> const &v)
{
    return dst.setConst(v.name, v.v);
}

}}

#endif // _CUTES_JS_UTIL_HPP_
