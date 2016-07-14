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

## LiveOSC Files
DISTFILES += LiveOSC_mod/__init__.py
DISTFILES += LiveOSC_mod/LiveOSC.py
DISTFILES += LiveOSC_mod/LiveOSCCallbacks.py
DISTFILES += LiveOSC_mod/LiveUtils.py
DISTFILES += LiveOSC_mod/Logger.py
DISTFILES += LiveOSC_mod/LogServer.py
DISTFILES += LiveOSC_mod/OSC.py
DISTFILES += LiveOSC_mod/OSCAPI.txt
DISTFILES += LiveOSC_mod/RemixNet.py
DISTFILES += LiveOSC_mod/socket_live8.py
DISTFILES += LiveOSC_mod/struct.py
