#ifndef _QTSCRIPT_QML_ADAPTER_H_
#define _QTSCRIPT_QML_ADAPTER_H_

#include "QsEnv.hpp"

#include <QCoreApplication>
#include "qt_quick_types.hpp"

namespace QsExecute {

#define QML_NAMESPACE Mer.QtScript
static inline void registerDeclarativeTypes(char const *uri)
{
    qmlRegisterType<DeclarativeActor>(uri, 1, 1, "QtScriptActor");
    qmlRegisterType<QtScriptAdapter>(uri, 1, 1, "QtScriptAdapter");
}

void setupDeclarative(QCoreApplication &, QDeclarativeView &, QString const &);
QJSEngine *getDeclarativeScriptEngine(QDeclarativeContext &);
QJSEngine *getDeclarativeScriptEngine(QDeclarativeEngine *decl_eng);

}

#endif // _QTSCRIPT_QML_ADAPTER_H_
