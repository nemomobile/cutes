#include "EngineAccess.hpp"
#include "QmlAdapter.hpp"
#include "Actor.hpp"
#include "Env.hpp"
#include "util.hpp"

#include "qt_quick_types.hpp"

namespace cutes {

void setupDeclarative
(QCoreApplication &app, QQuickView &view, QString const &qml_path)
{
    QJSEngine *pengine = view.engine();
    auto script_env = loadEnv(app, *pengine);
    script_env->pushParentScriptPath(qml_path);

    registerDeclarativeTypes(DQUOTESTR(QML_NAMESPACE));
}

}
