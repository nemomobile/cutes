#include "EngineAccess.hpp"
#include "QmlAdapter.hpp"
#include "QsActor.hpp"
#include "QsEnv.hpp"

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

QScriptEngine *getDeclarativeScriptEngine(QDeclarativeEngine *decl_eng)
{
    return decl_eng
        ? getDeclarativeScriptEngine(*decl_eng->rootContext())
        : nullptr;
}

void setupDeclarative
(QCoreApplication &app, QDeclarativeView &view, QString const &qml_path)
{
    QScriptEngine *pengine = getDeclarativeScriptEngine(*view.rootContext());

    auto old = pengine->globalObject();
    auto global = pengine->newObject();
    global.setPrototype(old);

    auto script_env = loadEnv(app, *pengine, global);
    pengine->setGlobalObject(global);
    script_env->pushParentScriptPath(qml_path);

    qmlRegisterType<QsExecute::Actor>("Mer.QtScript", 1, 1, "QtScriptActor");
}

}
