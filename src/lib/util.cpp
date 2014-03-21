#include "util.hpp"

namespace cutes { namespace js {

QVariantMap ObjectFactory::members()
{
    return members_;
}

QJSValue ObjectFactory::create(QVariant const &v)
{
    auto args = v.toList();
    if (!args.size()) {
        qWarning() << "create: no params provided " << v;
        return QJSValue();
    }

    auto name = args[0];
    
    auto p = names_.find(name.toString());
    if (p == names_.end()) {
        qWarning() << "Can't find class " << name;
        return QJSValue();
    } else {
        auto params = args.size() > 1 ? args[1].toList() : QVariantList();
        return p->second(*engine_, params);
    }
}

void ObjectFactory::addEnums(QMetaObject const &mo, QString const &cls_name)
{
    QVariantMap cls_members;
    for (int i = 0; i < mo.enumeratorCount(); ++i) {
        auto emo = mo.enumerator(i);
        QVariantMap ids;
        for (int j = 0; j < emo.keyCount(); ++j)
            ids[emo.key(j)] = emo.value(j);

        cls_members[emo.name()] = ids;
    }
    members_[cls_name] = cls_members;
}

}}
