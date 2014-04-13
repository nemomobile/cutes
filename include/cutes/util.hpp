#ifndef _CUTES_V4_UTIL_HPP_
#define _CUTES_V4_UTIL_HPP_

#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
#include <QVariantList>
#include <QStringList>
#include <QMetaObject>
#include <QDebug>

#include <memory>

namespace cutes {

static inline char const *cutesRegisterName()
{
    return "cutesRegister";
}

typedef QJSValue (*cutesRegisterFnType)(QJSEngine *);


template <typename T>
static inline QJSValue getCppOwnedJSValue(QJSEngine &e, T *p)
{
    auto res = e.newQObject(p);
    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
    return res;
}

namespace js {

template <class QtT>
struct JsTraits;

template <typename T>
QJSValue create_object(QJSEngine &engine, QVariantList const &args)
{
    try {
        return engine.newQObject(new T(engine, args));
    } catch (std::exception const &e) {
        qWarning() << "Creating object: C++ exception " << e.what();
        return QJSValue();
    }
}

static inline bool hasType(QVariant const &v, QMetaType::Type t)
{
    return static_cast<QMetaType::Type>(v.type()) == t;
}

template <typename T>
T convert(QVariant const &v)
{
    return v.value<T>();
}

template<>
inline QStringList convert(QVariant const &v)
{
    return v.toStringList();
}

template <typename To>
struct Converter
{
    static To convert(QJSEngine &engine, To &&src)
    {
        return src;
    }
};

template <typename To, typename From>
To convert(QJSEngine &engine, From &&src)
{
    return Converter<To>::template convert<>(engine, std::move(src));
}

template <>
struct Converter<QJSValue>
{
    template <typename From>
    static QJSValue convert(QJSEngine &engine, From &&src)
    {
        try {
            return engine.newQObject(new From(std::move(src)));
        } catch (std::exception const &e) {
            qDebug() << "Creating object: C++ exception " << e.what();
            return QJSValue();
        }
    }

    template <typename From>
    static QJSValue convert(QJSEngine &engine, QList<From> &&src)
    {
        typedef typename JsTraits<From>::js_type js_type;
        try {
            auto res = engine.newArray(src.size());
            for (size_t i = 0; !src.isEmpty(); ++i) {
                auto v = engine.newQObject(new js_type(engine, src.takeFirst()));
                res.setProperty(i, v);
            }
            return res;
        } catch (std::exception const &e) {
            qDebug() << "Creating object: C++ exception " << e.what();
            return QJSValue();
        }
    }
};

template <typename To, typename From>
To convert(QJSEngine &engine, From const &src)
{
    return Converter<To>::template convert<From>(engine, src);
}

template <typename T>
T * cast(QJSValue const &v)
{
    auto obj = v.toQObject();
    return obj ? qobject_cast<T*>(obj) : nullptr;
}

class AJsObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cutesClass__ READ getClassName)
public:
    AJsObject(QJSEngine &e)
        : QObject(nullptr)
        , engine_(e)
    {
    }

    AJsObject(AJsObject const &src)
        : QObject(nullptr)
        , engine_(src.engine_)
    {}

    template <typename To, typename From>
    To convert(From &&src) const
    {
        return Converter<To>::template convert<>(engine_, std::move(src));
    }

    QJSValue cutesObjImpl(QVariant const &src) const
    {
        auto m = src.toMap();
        return engine_.toScriptValue(m["impl__"]);
    }

    template <typename T>
    T* castCutesObj(QVariant const &src) const
    {
        return cast<T>(cutesObjImpl(src));
    }

    virtual QString getClassName() const =0;

protected:
    QJSEngine &engine_;
};


template <typename T, typename ImplT>
class JsObject : public AJsObject
{
public:
    typedef ImplT impl_type;
    typedef std::shared_ptr<impl_type> impl_ptr; 
    typedef std::function<impl_ptr (QJSEngine &, QVariantList const &)> ctor_type;
    typedef std::vector<ctor_type> ctors_type;

    JsObject(QJSEngine &e, QVariantList const &params)
        : AJsObject(e)
        , impl_(ctor_(params))
    { }

    JsObject(QJSEngine &e, impl_ptr p)
        : AJsObject(e)
        , impl_(p)
    {}

    JsObject(JsObject &&from)
        : AJsObject(from.engine_)
        , impl_(from.impl_)
    {}

    virtual ~JsObject() {
    }

    virtual QString getClassName() const
    {
        return T::className();
    };

    impl_ptr impl_;

protected:

    std::shared_ptr<impl_type> ctor_(QVariantList const &values)
    {
        auto &ctors = T::ctors_;
        size_t s = values.size();
        if (!s) {
            return std::make_shared<impl_type>();
        } else if (s < ctors.size()) {
            return ctors[s](engine_, values);
        } else {
            qWarning() << "Unexpected amount of arguments " << s
                       << " for ctor of " << getClassName();
            return nullptr;
        }
    }
};

typedef std::function<QJSValue (QJSEngine &, QVariantList const &)> obj_ctor_type;

class ObjectFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap members READ getMembers)
public:
    ObjectFactory(QJSEngine *e)
        : engine_(e)
    {
        init();
    }

    Q_INVOKABLE QJSValue create(QVariant const &);
    Q_INVOKABLE QVariantMap const & getMembers();
private:

    void init();
    void addEnums(QMetaObject const &mo, QString const &cls_name);

    template <typename T>
    void addClass(QString const &cls_name)
    {
        addEnums(T::staticMetaObject, cls_name);
        names_.insert(std::make_pair(cls_name, create_object<T>));
    }

    QJSEngine *engine_;
    std::map<QString, obj_ctor_type> names_;
    QVariantMap members_;
};


} // namespace js
} // namespace cutes

extern "C" QJSValue cutesRegister(QJSEngine *);

#endif // _CUTES_V4_UTIL_HPP_
