#ifndef _QMLSCRIPT_PRIVATE_ENGINE_HPP_
#define _QMLSCRIPT_PRIVATE_ENGINE_HPP_

#include <QJSEngine>

namespace QsExecute {

class EngineAccess : public QObject
{
    Q_OBJECT;
public:
    EngineAccess(QJSEngine **e);
    virtual ~EngineAccess();

    Q_INVOKABLE void setEngine(QJSValue val);

private:
    QJSEngine** engine_;

};

}
#endif // _QMLSCRIPT_PRIVATE_ENGINE_HPP_
