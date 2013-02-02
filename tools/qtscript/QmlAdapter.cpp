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

QScriptEngine *getDeclarativeScriptEngine(QDeclarativeContext &ctx)
{
    QScriptEngine *pengine;

    ctx.setContextProperty("__engineAccess", new EngineAccess(&pengine));
    QDeclarativeExpression expr(&ctx, ctx.contextObject(),
                                "__engineAccess.setEngine(this)");
    expr.evaluate();
    return pengine;
}

void setupDeclarative
(QCoreApplication &app, QDeclarativeView &view, QString const &cwd)
{
    QScriptEngine *pengine = getDeclarativeScriptEngine(*view.rootContext());

    auto old = pengine->globalObject();
    auto global = pengine->newObject();
    global.setPrototype(old);

    QsExecute::setupEngine(app, *pengine, global);
    pengine->setGlobalObject(global);

    // add current working directory to env
    auto script = findProperty(global, {"qtscript", "script"});
    script.setProperty("cwd", pengine->toScriptValue(cwd));

    view.engine()->setBaseUrl(cwd);

    qmlRegisterType<QsExecute::Actor>("Mer.QtScript", 1, 0, "QtScriptActor");
}

}
