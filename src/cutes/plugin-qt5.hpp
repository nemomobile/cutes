#ifndef _CUTES_QML_PLUGIN_QT5_HPP_
#define _CUTES_QML_PLUGIN_QT5_HPP_

#include "util.hpp"

#include "QmlAdapter.hpp"

#include <QtQml/QQmlExtensionPlugin>
#include "qt_quick_types.hpp"

namespace cutes
{

class Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DQUOTESTR(QML_NAMESPACE))
public:
    void initializeEngine(QQmlEngine *engine, const char *)
    {
        auto app = QCoreApplication::instance();
        if (app)
            new EnvImpl(app, *app, *engine);
    }

    void registerTypes(char const* uri) {
        registerDeclarativeTypes(uri);
    }
};

}

#endif // _CUTES_QML_PLUGIN_QT5_HPP_
