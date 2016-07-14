QT += core network
QT -= gui

TARGET = OSCGrabber
CONFIG += console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_LFLAGS += -stdlib=libc++

QMAKE_CXXFLAGS += -std=gnu++0x
QMAKE_LFLAGS += -std=gnu++0x

INCLUDEPATH += ./liblo/include
LIBS += -L./liblo/lib/ -llo


TEMPLATE = app

SOURCES += main.cpp

