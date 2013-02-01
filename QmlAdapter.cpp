#include "EngineAccess.hpp"
#include "QmlAdapter.hpp"
#include "QsExecute.hpp"
#include "QsActor.hpp"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeExpression>
#include <QtDeclarative>

namespace QsExecute {

EngineAccess::EngineAccess(QScriptEngine **e) : engine_(e) {}
EngineAccess::~EngineAccess() {}

void EngineAccess::setEngine(QScriptValue val)
{
    *engine_ = val.engine();
}

void setupDeclarative(QCoreApplication &app, QDeclarativeView &view)
{
    QScriptEngine *pengine;
    QDeclarativeContext *ctx = view.rootContext();

    ctx->setContextProperty("__engineAccess", new EngineAccess(&pengine));
    QDeclarativeExpression expr(ctx, ctx->contextObject(),
                                "__engineAccess.setEngine(this)");
    expr.evaluate();

    auto old = pengine->globalObject();
    auto global = pengine->newObject();
    global.setPrototype(old);

    QsExecute::setupEngine(app, *pengine, global);
    pengine->setGlobalObject(global);
    qmlRegisterType<QsExecute::Actor>("Mer.QtScript", 1, 0, "QtScriptActor");
}

}
