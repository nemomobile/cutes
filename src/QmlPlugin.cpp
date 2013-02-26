#include "EngineAccess.hpp"
#include "QsActor.hpp"
#include "QsEnv.hpp"
#include "QmlAdapter.hpp"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeExpression>
#include <QtDeclarative>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

namespace QsExecute {

class Plugin : public QDeclarativeExtensionPlugin
{
public:
    void initializeEngine(QDeclarativeEngine *engine, const char *)
    {
        QScriptEngine *script_engine = getDeclarativeScriptEngine(*engine->rootContext());
        loadEnv(*QCoreApplication::instance(), *script_engine);
    }

    void registerTypes(char const *uri)
    {
        registerDeclarativeTypes(uri);
    }
};

}

Q_EXPORT_PLUGIN2(QML_NAMESPACE, QsExecute::Plugin);
