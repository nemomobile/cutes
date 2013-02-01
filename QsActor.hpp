#ifndef _QS_ACTOR_HPP_
#define _QS_ACTOR_HPP_
/*
 * QtScript Actor component
 *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jolla.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */

#include <QObject>
#include <QVariant>
#include <QString>
#include <QScriptValue>
#include <QThread>
#include <QEvent>
#include <QWaitCondition>
#include <QMutex>

namespace QsExecute
{

class EngineEvent : public QEvent
{
public:
    enum Type {
        LoadScript = QEvent::User,
        ProcessMessage,
        QuitThread
    };

    EngineEvent();
    EngineEvent(QString const&);
    EngineEvent(QVariant const&, QScriptValue);
    virtual ~EngineEvent();

    QString src_;
    QVariant data_;
    QScriptValue cb_;
};

class Actor;

class Engine : public QObject
{
    Q_OBJECT;
public:
    Engine();
    virtual ~Engine();

    void run();
    virtual bool event(QEvent *);
    void startEngine();

signals:
    void onReply(QVariant, QScriptValue);
    void onQuit();

private:
    void load(QString const&);
    void processMessage(QVariant &, QScriptValue &);
    QScriptEngine *engine_;
    QScriptValue handler_;
    QWaitCondition cond_;
    QMutex mutex_;
};

class EngineThread : public QThread
{
    Q_OBJECT;
public:
    EngineThread(Actor *parent, QString const &src);
    virtual ~EngineThread();

    void run();
    void sendMessage(QScriptValue, QScriptValue);

private:
    Actor *actor_;
    QWaitCondition cond_;
    QMutex mutex_;
    QScopedPointer<Engine> engine_;
};

class Actor : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(QString source READ source WRITE setSource);
public:
    Actor(QObject *parent = nullptr);
    virtual ~Actor();

    QString source() const;
    void setSource(QString const &);

    Q_INVOKABLE void sendMessage(QScriptValue, QScriptValue);

    Q_INVOKABLE void exitEngine();

signals:
    void message(QScriptValue);

public slots:
    void onReply(QVariant, QScriptValue);

protected:
    QString src_;

private:

    QScopedPointer<EngineThread> engine_;
};

}

#endif // _QS_ACTOR_HPP_
