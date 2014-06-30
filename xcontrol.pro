# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += plugin release
CONFIG -= thread exceptions qt rtti debug

VERSION = 1.0.0

INCLUDEPATH += ../SDK/CHeaders/XPLM
INCLUDEPATH += ../SDK/CHeaders/Wrappers
INCLUDEPATH += ../SDK/CHeaders/Widgets
QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"

# Defined to use X-Plane SDK 2.0 capabilities - no backward compatibility before 9.0
DEFINES += XPLM200

win32 {
    DEFINES += APL=0 IBM=1 LIN=0
    LIBS += -LE:/projects/SDK/Libraries/Win -LE:/projects/Libs/Win
    LIBS += -lXPLM -lXPWidgets -lusb
    TARGET = win.xpl
}

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    TARGET = lin.xpl
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden
    LIBS += -lusb-1.0
}

macx {
    DEFINES += APL=1 IBM=0 LIN=0
    TARGET = mac.xpl
    QMAKE_LFLAGS += -flat_namespace -undefined suppress

    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # The following line defines for which architectures we build.
    CONFIG += x86 ppc
}

HEADERS		+= session.h
HEADERS		+= tools.h
HEADERS		+= data.h
HEADERS		+= out.h
HEADERS		+= in.h
HEADERS		+= mfdpage.h
HEADERS             += mfdpage_fms.h
HEADERS             += mfdpage_flight.h
HEADERS             += mfdpage_time.h
HEADERS             += mfdpage_weather.h
HEADERS             += mfdpage_std.h
HEADERS             += mfdpage_led.h
HEADERS		+= fms.h
HEADERS             += fms_config.h
HEADERS             += fms_fuel.h
HEADERS             += fms_fp.h
HEADERS             += fms_utils.h
HEADERS		+= config.h
HEADERS		+= gui.h

SOURCES		+= main.cpp
SOURCES		+= session.cpp
SOURCES		+= tools.cpp
SOURCES		+= data.cpp
SOURCES		+= out.cpp
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




