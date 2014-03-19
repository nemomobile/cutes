#ifndef _CUTES_PRIVATE_UTIL_HPP_
#define _CUTES_PRIVATE_UTIL_HPP_

#include <QVariant>
#include <QJSEngine>
#include <QVariantList>
#include <QStringList>
#include <QDebug>
#include <QMetaObject>
#include <QMetaEnum>

#include <cor/util.hpp>
#include <cutes/v4/util.hpp>


namespace cutes { namespace js {

template <class QtT>
struct JsTraits;

template <typename T>
QJSValue create_object(QJSEngine &engine, QVariantList const &args)
{
    try {
        return engine.newQObject(new T(engine, args));
    } catch (std::exception const &e) {
        std::cerr << "Creating object: C++ exception " << e.what() << std::endl;
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
            std::cerr << "Creating object: C++ exception " << e.what() << std::endl;
            return QJSValue();
        }
    }
};

template <>
struct Converter<QVariantList>
{
    template <typename From>
    static QVariantList convert(QJSEngine &engine, QList<From> &&src)
    {
        typedef typename JsTraits<From>::js_type js_type;
        QVariantList res;
        try {
            while (!src.isEmpty()) {
                auto converted = js_type(engine, src.takeFirst());
                auto v = cutes::js::convert<QJSValue>(engine, std::move(converted));
                res.push_back(v.toVariant());
            }
        } catch (std::exception const &e) {
            qWarning() << "Creating object: C++ exception " << e.what();
        } catch (...) {
            qWarning() << "Got unknown exception converting";
        }
        return res;
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
        : engine_(e)
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
public:
    ObjectFactory(QJSEngine *e)
        : engine_(e)
    {
        init();
    }

    Q_INVOKABLE QJSValue create(QVariant const &);
    Q_INVOKABLE QVariantMap members();
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


}}

#endif // _CUTES_PRIVATE_UTIL_HPP_
