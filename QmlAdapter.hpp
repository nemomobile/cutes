#ifndef _QTSCRIPT_QML_ADAPTER_H_
#define _QTSCRIPT_QML_ADAPTER_H_

#include <QCoreApplication>
#include <QDeclarativeView>

namespace QsExecute {

void setupDeclarative(QCoreApplication &, QDeclarativeView &, QString const &);
QScriptEngine *getDeclarativeScriptEngine(QDeclarativeContext &);

}

#endif // _QTSCRIPT_QML_ADAPTER_H_
