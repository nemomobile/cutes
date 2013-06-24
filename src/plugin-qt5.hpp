#ifndef _CUTES_QML_PLUGIN_QT5_HPP_
#define _CUTES_QML_PLUGIN_QT5_HPP_

#include "QmlAdapter.hpp"

#include <QtQml/QQmlExtensionPlugin>
#include "qt_quick_types.hpp"

namespace QsExecute
{

#define STRINGIFY(x) #x
#define DQUOTESTR(x) STRINGIFY(x)

class Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DQUOTESTR(QML_NAMESPACE))
public:
    void initializeEngine(QDeclarativeEngine *engine, const char *)
    {
        QScriptEngine *script_engine = getDeclarativeScriptEngine(*engine->rootContext());
        loadEnv(*QCoreApplication::instance(), *script_engine);
    }

    void registerTypes(char const* uri) {
        registerDeclarativeTypes(uri);
    }
};

}

#endif // _CUTES_QML_PLUGIN_QT5_HPP_
