#ifndef _QTSCRIPT_QML_ADAPTER_H_
#define _QTSCRIPT_QML_ADAPTER_H_

#include <QCoreApplication>
#include <QDeclarativeView>
#include <QDeclarativeEngine>

namespace QsExecute {

void setupDeclarative(QCoreApplication &, QDeclarativeView &, QString const &);
QScriptEngine *getDeclarativeScriptEngine(QDeclarativeContext &);
QScriptEngine *getDeclarativeScriptEngine(QDeclarativeEngine *decl_eng);

}

#endif // _QTSCRIPT_QML_ADAPTER_H_
