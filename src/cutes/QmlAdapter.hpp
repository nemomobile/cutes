#ifndef _QTSCRIPT_QML_ADAPTER_H_
#define _QTSCRIPT_QML_ADAPTER_H_

#include "Env.hpp"

#include <QCoreApplication>
#include "qt_quick_types.hpp"

namespace cutes {

#define QML_NAMESPACE Mer.Cutes
static inline void registerDeclarativeTypes(char const *uri)
{
    qmlRegisterType<QmlActor>(uri, 1, 1, "CutesActor");
}

void setupDeclarative(QCoreApplication &, QDeclarativeView &, QString const &);
QJSEngine *getDeclarativeScriptEngine(QDeclarativeContext &);
QJSEngine *getDeclarativeScriptEngine(QDeclarativeEngine *decl_eng);

}

#endif // _QTSCRIPT_QML_ADAPTER_H_
