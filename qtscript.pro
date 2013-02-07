include( ../../common.pri)
TEMPLATE = app
TARGET = qtscript
DESTDIR = ./
QT = core script declarative gui
HEADERS += EngineAccess.hpp QmlAdapter.hpp QsActor.hpp QsEnv.hpp
SOURCES += main.cpp QmlAdapter.cpp QsActor.cpp QsEnv.cpp
