#ifndef _CUTES_PRIVATE_QT_QUICK_TYPES_HPP_
#define _CUTES_PRIVATE_QT_QUICK_TYPES_HPP_

#if QT_VERSION < 0x050000

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeExpression>
#include <QtDeclarative>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/qdeclarative.h>
#include <QScriptEngine>

#else

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQuickView>
#include <qqml.h>

typedef QQmlContext QDeclarativeContext;
typedef QQmlExpression QDeclarativeExpression;
typedef QQmlEngine QDeclarativeEngine;
typedef QQuickView QDeclarativeView;

#endif



#endif // _CUTES_PRIVATE_QT_QUICK_TYPES_HPP_
