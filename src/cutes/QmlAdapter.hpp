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
    qmlRegisterType<Adapter>(uri, 1, 1, "CutesAdapter");
}

void setupDeclarative(QCoreApplication &, QDeclarativeView &, QString const &);
void setupEngine(QCoreApplication &, QJSEngine &, QString const &);

QJSEngine *getDeclarativeScriptEngine(QDeclarativeContext &);
QJSEngine *getDeclarativeScriptEngine(QDeclarativeEngine *decl_eng);

}

#endif // _QTSCRIPT_QML_ADAPTER_H_
