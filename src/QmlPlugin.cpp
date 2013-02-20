#include "EngineAccess.hpp"
#include "QmlAdapter.hpp"
#include "QsActor.hpp"
#include "QsEnv.hpp"

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
        qDebug() << "REG";
        qmlRegisterType<Actor>(uri, 1, 1, "QtScriptActor");
        qmlRegisterType<QtScriptAdapter>(uri, 1, 1, "QtScriptAdapter");
    }
};

}

Q_EXPORT_PLUGIN2(Mer.QtScript, QsExecute::Plugin);
