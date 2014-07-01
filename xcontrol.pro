# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += plugin release
CONFIG -= thread exceptions qt rtti

VERSION = 1.0.0

INCLUDEPATH += ./include
DEFINES += XPLM200

win32 {
    DEFINES += APL=0 IBM=1 LIN=0 _CRT_SECURE_NO_WARNINGS
    LIBS += -L../../../xcontrol/lib/win/64 -lXPLM -lXPWidgets -llibusb0
    TARGET = win.xpl
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    QMAKE_CXXFLAGS += -fvisibility=hidden
    LIBS += -lusb-1.0
    QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
}

macx {
    DEFINES += APL=1 IBM=0 LIN=0
    TARGET = mac.xpl
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    CONFIG += x86 ppc
    QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
}

HEADERS		+= include/*.h

SOURCES		+= main.cpp
SOURCES		+= session.cpp
SOURCES		+= tools.cpp
SOURCES		+= data.cpp
SOURCES		+= out.cpp
SOURCES		+= out_saitek.cpp
SOURCES		+= in.cpp
SOURCES		+= mfdpage.cpp
SOURCES             += mfdpage_fms.cpp
SOURCES             += mfdpage_flight.cpp
SOURCES             += mfdpage_time.cpp
SOURCES             += mfdpage_weather.cpp
SOURCES             += mfdpage_std.cpp
SOURCES             += mfdpage_led.cpp
SOURCES		+= fms.cpp
SOURCES             += fms_config.cpp
SOURCES             += fms_fuel.cpp
SOURCES             += fms_fp.cpp
SOURCES             += fms_utils.cpp
SOURCES		+= config.cpp
SOURCES		+= gui.cpp
