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

typedef v8::Handle<v8::Value> VHandle;

template <typename T>
struct ObjectTraits
{
};

static inline v8::Local<v8::Value> Get
(v8::Handle<v8::Object> v, char const *name)
{
    return v->Get(v8::String::New(name));
}

static inline void Set
(v8::Handle<v8::Object> o, char const *name, VHandle v)
{
    o->Set(v8::String::New(name), v);
}

VHandle callConvertException(const v8::Arguments &, v8::InvocationCallback);

template <typename ResT>
ResT *QObjFromV8This(const v8::Arguments &args)
{
    return reinterpret_cast<ResT*>
        (v8::External::Unwrap(args.This()->GetInternalField(0)));
}

template <typename T>
static inline VHandle objToV8(T const& v)
{
    v8::HandleScope hscope;
    typedef typename ObjectTraits<T>::js_type obj_type;
    VHandle p = v8::External::New(new T(v));
    auto ctor = obj_type::cutesCtor_->GetFunction();
    return hscope.Close(ctor->NewInstance(1, &p));
}

template <typename T> struct Convert
{
    static inline VHandle toV8(T const& v)
    {
        return objToV8<T>(v);
    }
};

template <typename T>
inline VHandle ValueToV8(T const& v)
{
    return Convert<T>::toV8(v);
};

template <typename T>
inline T ValueFromV8(QV8Engine *e, VHandle h)
{
    return Convert<T>::fromV8(e, h);
}

template<> struct Convert<QString> {
    static inline QString fromV8(QV8Engine *, VHandle v)
    {
        return QJSConverter::toString(v->ToString());
    }

    static inline VHandle toV8(QString const& v)
    {
        return QJSConverter::toString(v);
    }
};

template<> struct Convert<int> {
    static inline int fromV8(QV8Engine *, VHandle v)
    {
        using namespace v8;
        if (!v->IsNumber()) {
            throw std::invalid_argument("Not a number");
            return 0;
        }
        return v->ToInteger()->Value();
    }

    static inline VHandle toV8(int v)
    {
        return v8::Integer::New(v);
    }
};

template<> struct Convert<long long> {
    static inline VHandle toV8(long long v)
    {
        return v8::Number::New(v);
    }
};

template<> struct Convert<bool> {
    static inline VHandle toV8(bool v)
    {
        return v8::Boolean::New(v);
    }
};

template<> struct Convert<QVariant> {
    static inline QVariant fromV8(QV8Engine *e, VHandle v)
    {
        return e->variantFromJS(v);
    }
};

template<> struct Convert<QStringList> {
    static inline QStringList fromV8(QV8Engine *, VHandle v)
    {
        using namespace v8;
        if (!v->IsArray()) {
            throw std::invalid_argument("Not an array");
            return QStringList();
        }
        return QJSConverter::toStringList(Handle<Array>::Cast(v));
    }
};

template <typename T>
T Arg(v8::Arguments const& args, unsigned i)
{
    return ValueFromV8<T>(V8ENGINE(), args[i]);
}


template <typename T>
void v8DeleteCppObj(v8::Persistent<v8::Value>, void* p)
{
    delete reinterpret_cast<T*>(p);
}

std::pair<bool, VHandle> copyCtor(const v8::Arguments &args);

template <typename T, typename BaseT>
static VHandle v8Ctor(const v8::Arguments &args)
{
    auto res = copyCtor(args);
    if (res.first)
        return res.second;

    using namespace v8;
    auto self = args.This();
    T *p = new T(args);
    Local<External> external = External::New(static_cast<BaseT*>(p));
    auto ph = Persistent<Value>::New(self);
    ph.MakeWeak(p, &v8DeleteCppObj<T>);
    self->SetInternalField(0, external);
    return self;
}

template <typename T>
static void v8EngineAdd(QV8Engine *v8e, char const *name)
{
    using namespace v8;
    HandleScope hscope;

    Handle<FunctionTemplate> ctor 
        = FunctionTemplate::New(v8Ctor<T, typename T::impl_type>
                                , External::New(v8e));

    T::cutesCtor_ = Persistent<FunctionTemplate>::New(ctor);

    Handle<ObjectTemplate> tpl = ctor->InstanceTemplate();
    tpl->SetInternalFieldCount(1);

    tpl->Set("cutesClass__", v8::String::New(name));
    
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

static inline TemplateInitializer
setupTemplate(QV8Engine *engine, v8::Handle<v8::Template> target)
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

template <typename ResT, typename ObjT>
struct FnWrapper
{
    template <typename FnT, FnT fn>
    static inline VHandle param0(const v8::Arguments &args)
    {
        return callConvertException(args, [](const v8::Arguments &args) -> VHandle {
                auto self = QObjFromV8This<ObjT>(args);
                return ValueToV8((self->*fn)());
            });
    }

    template <typename ParamT, typename FnT, FnT fn>
    static inline VHandle param1(const v8::Arguments &args)
    {
        return callConvertException(args, [](const v8::Arguments &args) -> VHandle {
                auto self = QObjFromV8This<ObjT>(args);
                auto p0 = Arg<ParamT>(args, 0);
                return ValueToV8((self->*fn)(p0));
            });
    }
};

template <typename ObjT>
struct FnWrapper<void, ObjT>
{
    template <typename FnT, FnT fn>
    static inline VHandle param0(const v8::Arguments &args)
    {
        return callConvertException(args, [](const v8::Arguments &args) -> VHandle {
                auto self = QObjFromV8This<ObjT>(args);
                (self->*fn)();
                return v8::Undefined();
            });
    }

    template <typename ParamT, typename FnT, FnT fn>
    static inline VHandle param1(const v8::Arguments &args)
    {
        return callConvertException(args, [](const v8::Arguments &args) -> VHandle {
                auto self = QObjFromV8This<ObjT>(args);
                auto p0 = Arg<ParamT>(args, 0);
                (self->*fn)(p0);
                return v8::Undefined();
            });
    }
};

template <typename ResT, typename ObjT, typename ParamT, typename FnT, FnT fn>
inline VHandle fnWithParam(const v8::Arguments &args)
{
    return FnWrapper<ResT, ObjT>::template param1<ParamT, FnT, fn>(args);
}

template <typename ResT, typename ObjT, typename FnT, FnT fn>
static VHandle fnWOParams(const v8::Arguments &args)
{
    return FnWrapper<ResT, ObjT>::template param0<FnT, fn>(args);
}

#define CUTES_CONST(name, cls) cutes::js::Const(#name, cls::name)

#define CUTES_FN(name, cls) cutes::js::Callback(#name, &cls::name)

#define CUTES_GET_CONST(name, res, type, obj_type)   \
    cutes::js::Callback(#name, fnWOParams<res, type,                 \
                        res(obj_type::*)() const, &obj_type::name>)

#define CUTES_GET(name, res, type, obj_type)  \
    cutes::js::Callback(#name, fnWOParams                                   \
                            <res, type, res(obj_type::*)(), &obj_type::name>)

#define CUTES_VOID_FN(name, type, obj_type)                                      \
    cutes::js::Callback(#name, fnWOParams<void, type,                               \
                                          void(obj_type::*)(), &obj_type::name>)

#define CUTES_FN_PARAM(name, res, type, obj_type, param_t, param_sig)    \
    cutes::js::Callback(#name, fnWithParam                                  \
                            <res, type, param_t,                            \
                             res (obj_type::*)(param_sig),                  \
                             &obj_type::name>)

#define CUTES_FN_PARAM_CONST(name, res, type, obj_type, param_t, param_sig)    \
    cutes::js::Callback(#name, fnWithParam                                     \
                            <res, type, param_t,                               \
                             res (obj_type::*)(param_sig) const,               \
                             &obj_type::name>)

}}

extern "C" void registerLibrary(QV8Engine *);

#endif // _CUTES_JS_UTIL_HPP_
