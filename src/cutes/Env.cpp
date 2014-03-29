#include <cutes/env.hpp>
#include "Env.hpp"

namespace cutes {

Env::Env(QObject *parent, QCoreApplication *app, QJSEngine *eng)
    : QObject(parent)
    , impl_(EnvImpl::create(this, app, eng))
{
}

Env::Env(Env &&from)
    : impl_(from.impl_)
{
    impl_->setParent(this);
}

Env::~Env()
{
}

QJSValue Env::require(QString const &name)
{
    return impl_->require(name);
}

QJSValue Env::extend(QString const &name)
{
    return impl_->extend(name);
}

QJSValue Env::actor()
{
    return impl_->actor();
}

void Env::exit(int rc)
{
    return impl_->exit(rc);
}

void Env::defer(QJSValue const &fn)
{
    return impl_->defer(fn);
}

void Env::idle()
{
    return impl_->idle();
}

void Env::setEnv(QString const &n, QVariant const &v)
{
    return impl_->setEnv(n, v);
}

void Env::trace(QVariant const &v)
{
    return impl_->trace(v);
}

QJSValue Env::eval(QString const &code)
{
    return impl_->eval(code);
}

QJSValue Env::globals() const
{
    return impl_->globals();
}

QJSValue Env::mkVariadic(QJSValue const &fn
                         , QJSValue const &members
                         , QJSValue const &obj)
{
    return impl_->mkVariadic(fn, members, obj);
}


QJSValue Env::module()
{
    return impl_->module();
}

QString Env::os() const
{
    return impl_->os();
}

QVariantMap const& Env::env() const
{
    return impl_->env();
}

QStringList const& Env::path() const
{
    return impl_->path();
}

QString Env::getEngineName() const
{
    return impl_->getEngineName();
}


bool Env::shouldWait()
{
    return impl_->shouldWait();
}

QStringList const& Env::args() const
{
    return impl_->args();
}

QJSValue Env::load(QString const &name, bool is_reload)
{
    return impl_->load(name, is_reload);
}

void Env::addSearchPath(QString const &path, Position pos)
{
    return impl_->addSearchPath(path, pos);
}

void Env::pushParentScriptPath(QString const &s)
{
    return impl_->pushParentScriptPath(s);
}

bool Env::addGlobal(QString const &name, QJSValue const &value)
{
    return impl_->addGlobal(name, value);
}


QJSEngine *Env::engine()
{
    return &(impl_->engine());
}

EnvImpl * Env::getImpl()
{
    return impl_;
}

QJSValue Env::include(QString const &name, bool is_reload)
{
    return impl_->include(name, is_reload);
}

}
