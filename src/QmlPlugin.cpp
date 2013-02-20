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
        qDebug() << engine->rootContext()->resolvedUrl(QUrl("."));
        for (auto v : engine->importPathList()) {
        qDebug() << "PATH:" << v;
        }
        qDebug() << "BASE:" << engine->baseUrl();
        loadEnv(*QCoreApplication::instance(), *script_engine);
    }

    void registerTypes(char const *uri)
    {
        qmlRegisterType<QsExecute::Actor>(uri, 1, 1, "QtScriptActor");
    }
};

}

Q_EXPORT_PLUGIN2(Mer.QtScript, QsExecute::Plugin);
