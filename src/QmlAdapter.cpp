#include "EngineAccess.hpp"
#include "QmlAdapter.hpp"
#include "QsActor.hpp"
#include "QsEnv.hpp"

#include "qt_quick_types.hpp"

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
    auto script_env = loadEnv(app, *pengine);
    script_env->pushParentScriptPath(qml_path);

    registerDeclarativeTypes("Mer.QtScript");
}

}
