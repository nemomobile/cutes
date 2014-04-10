#ifndef _QSEXECUTE_QSENV_HPP_
#define _QSEXECUTE_QSENV_HPP_
/**
 * @file Env.hpp
 * @brief Cutes environment private declarations
 * @author (C) 2012-2014 Jolla Ltd. Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 * @copyright LGPL 2.1 http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */


#include "Actor.hpp"
#include <cutes/env.hpp>
#include <cutes/util.hpp>

#include <QObject>
#include <QString>
#include <QSet>
#include <QJSEngine>
#include <QJSValue>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QStack>
#include <QDebug>
#include <QVariantList>

#include <stdexcept>

class QLibrary;
class QTimer;

namespace cutes {

QString errorConverterTry(QString const &);
QString errorConverterCatch(QString const &);

class ModuleImpl;
class Env;

typedef QMap<QString, QString> StringMap;

class Error : public std::runtime_error
{
public:
    Error(QString const &s);
    virtual ~Error() throw() {}

    QString msg;
};

class Globals;
class Extension;

class Extensions
{
public:
    QSharedPointer<Extension> get(QString const &path);
private:
    QMutex mutex_;
    QMap<QString, QWeakPointer<Extension> > libraries_;
};

class EnvImpl : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QJSValue module READ module);
    Q_PROPERTY(QString os READ os);
    Q_PROPERTY(QVariantMap env READ env);
    Q_PROPERTY(QStringList path READ path);
    Q_PROPERTY(QString engine READ getEngineName);

    typedef Env::Position Position;
public:

    EnvImpl(QObject *, QCoreApplication &, QJSEngine &);
    virtual ~EnvImpl();

    virtual bool event(QEvent *);

    Q_INVOKABLE QJSValue include(QString const&, bool is_reload);
    Q_INVOKABLE QJSValue require(QString const&);
    Q_INVOKABLE QJSValue extend(QString const&);
    Q_INVOKABLE QJSValue actor();
    Q_INVOKABLE void exit(int);
    Q_INVOKABLE void defer(QJSValue);
    Q_INVOKABLE void idle();
    Q_INVOKABLE void setEnv(QString const&, QVariant const&);
    Q_INVOKABLE void fprint(QVariant const &);
    Q_INVOKABLE void print(QVariant const &);
    Q_INVOKABLE void trace(QVariant const &);
    Q_INVOKABLE QJSValue eval(QString const &);
    Q_INVOKABLE QJSValue globals() const;
    Q_INVOKABLE void setInterval(QJSValue, int);
    Q_INVOKABLE void gc();

    QJSValue mkVariadic(QJSValue const &
                        , QJSValue const &
                        , QJSValue const &);
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

    QJSEngine &engine();
    Module* currentModule();
    QJSValue executeModule(QTextStream &, QString const&, size_t);
    QJSValue throwJsError(QString const&);

private:
    static EnvImpl * create(QObject *, QCoreApplication *, QJSEngine *);
    friend class Env;

    EnvImpl(EnvImpl const&);
    QString findFile(QString const &);
    QString libPath() const;
    void fprintInternal(QTextStream &, QVariantList const &, QString const &);
    void fprintVariant(QTextStream &, QVariant const &);
    void fprintImpl(QTextStream &out, QVariantList const &);
    void addToObjectPrototype(QString const&, QJSValue const&);

    QJSValue mkVariadicImpl(QJSValue const &
                            , QJSValue const &members = QJSValue()
                            , QJSValue const &obj = QJSValue());
    QJSValue evaluateLazy(QString const&, QString const&
                          , QJSValue &);
    QJSValue callJsLazy(QString const&, QString const&
                        , QJSValue &, QJSValueList const &);
    void pushModule(Module *);

    static Extensions libraries_;
    QJSEngine &engine_;

    QJSValue obj_proto_enhance_;
    QJSValue cpp_bridge_fn_;
    QJSValue throw_fn_;

    QJSValue this_;
    std::unique_ptr<Globals> globals_;
    friend class Globals;

    QMap<QString, std::pair<QSharedPointer<Extension>, QJSValue> > factories_;
    QMap<QString, Module*> modules_;

    QVariantMap env_;
    QStringList path_;
    QStack<std::pair<Module*, QJSValue> > scripts_;
    QStringList args_;
    QScopedPointer<QTimer> interval_timer_;
    int actor_count_;
    bool is_eval_;
    bool is_event_loop_running_;
    bool is_waiting_exit_;

private slots:
    void actorAcquired();
    void actorReleased();
    void exitImpl(int);
    void eventLoopRunning();
};

class Module : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString id READ fileName);
    Q_PROPERTY(QString filename READ fileName);
    Q_PROPERTY(bool loaded READ loaded);
    Q_PROPERTY(QString cwd READ cwd);
    Q_PROPERTY(QStringList args READ args);
    Q_PROPERTY(QJSValue exports READ exports WRITE setExports);

public:
    Module(EnvImpl *parent, QString const&);
    Module(EnvImpl *parent, QString const&, QString const&);
    virtual ~Module() {}

    Q_INVOKABLE QJSValue require(QString const&);

    QString fileName() const;
    bool loaded() const;
    QString cwd() const;
    QStringList const& args() const;
    QJSValue exports() const;
    void setExports(QJSValue);

    QJSValue load();

    //QJSValue result_;
private:
    EnvImpl* env() { return static_cast<EnvImpl*>(parent()); }
    EnvImpl const* env() const { return static_cast<EnvImpl const*>(parent()); }
    QFileInfo info_;
    QJSValue exports_;
    bool is_loaded_;
    QString cwd_;
};

bool isTrace();

template <typename ... Args> decltype(qDebug()) tracer(Args&&... args) {
    return qDebug(std::forward<Args>(args)...);
}

enum class JSValueConvert
{
    Deep, Shallow
};

enum class JSValueConvertOptions
{
    NoError, AllowError
};

QVariant msgFromValue
(QJSValue const &
 , JSValueConvert convert = JSValueConvert::Deep
 , JSValueConvertOptions options = JSValueConvertOptions::NoError);


} // namespace

#endif // _QSEXECUTE_QSENV_HPP_
