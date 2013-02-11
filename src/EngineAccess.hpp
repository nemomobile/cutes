#ifndef _QMLSCRIPT_PRIVATE_ENGINE_HPP_
#define _QMLSCRIPT_PRIVATE_ENGINE_HPP_

#include <QtScript/QScriptEngine>

namespace QsExecute {

class EngineAccess : public QObject
{
    Q_OBJECT;
public:
    EngineAccess(QScriptEngine **e);
    virtual ~EngineAccess();

    Q_INVOKABLE void setEngine(QScriptValue val);

private:
    QScriptEngine** engine_;

};

}
#endif // _QMLSCRIPT_PRIVATE_ENGINE_HPP_
