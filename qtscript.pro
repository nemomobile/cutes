include( ../../common.pri)
TEMPLATE = app
TARGET = qtscript
DESTDIR = ./
QT = core script declarative gui
HEADERS += QsExecute.hpp EngineAccess.hpp QmlAdapter.hpp QsActor.hpp
SOURCES += QsExecute.cpp main.cpp QmlAdapter.cpp QsActor.cpp
