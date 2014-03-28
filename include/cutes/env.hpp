#ifndef _CUTES_ENV_HPP_
#define _CUTES_ENV_HPP_
/**
 * @file env.hpp
 * @brief Cutes environment public declarations
 * @author (C) 2012-2014 Jolla Ltd. Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 * @copyright LGPL 2.1 http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */

#include <QJSValue>
#include <QString>
#include <QVariantMap>
#include <QStringList>

class QJSEngine;
class QCoreApplication;

namespace cutes {

class EnvImpl;
class Module;

class Env : public QObject
{
    Q_OBJECT

public:

    enum Position {
        Front,
        Back
    };

    Env(QObject *, QCoreApplication *, QJSEngine *);
    Env(Env &&);
    virtual ~Env();

    Env() =delete;
    Env(Env const&) =delete;
    Env operator =(Env const&) =delete;

    virtual bool event(QEvent *);

    QJSValue include(QString const&, bool is_reload);
    QJSValue require(QString const&);
    QJSValue extend(QString const&);
    QJSValue actor();
    void exit(int);
    void defer(QJSValue const&);
    void idle();
    void setEnv(QString const&, QVariant const&);
    void trace(QVariant const &);
    QJSValue eval(QString const &);
    QJSValue globals() const;
    QJSValue mkVariadic(QJSValue const &, QJSValue const &, QJSValue const &);

    QJSValue module();
    QString os() const;
    QVariantMap const& env() const;
    QStringList const& path() const;
    QString getEngineName() const;

    bool shouldWait();
    QStringList const& args() const;
    QJSValue load(QString const &, bool is_reload);
    void addSearchPath(QString const &, Position);
    void pushParentScriptPath(QString const&);
    bool addGlobal(QString const &, QJSValue const &);

    QJSEngine *engine();

    EnvImpl * getImpl();
private:
    EnvImpl *impl_;
};

}

#endif //_CUTES_ENV_HPP_

