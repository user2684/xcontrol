######################################
# Init QT for generating a shared library without any Qt functionality
######################################
TEMPLATE = lib
QT -= gui core
CONFIG += plugin release
CONFIG -= thread exceptions qt rtti

######################################
# Define the target Architecture
######################################
CONFIG += 64bit

######################################
# Define global variables
######################################
INCLUDEPATH += ./include
DEFINES += XPLM200

######################################
# Win32 platform specific variables
######################################
win32 {
    DEFINES += APL=0 IBM=1 LIN=0 _CRT_SECURE_NO_WARNINGS
    LIBS += -L../../../lib/win
    TARGET = win.xpl
    32bit:LIBS += -lXPLM -lXPWidgets -llibusb0
    64bit:LIBS += -lXPLM_64 -lXPWidgets_64 -llibusb0_64
}

######################################
# *unix platform common variables
######################################
unix|macx {
    QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
    32bit:QMAKE_CXXFLAGS += -m32
    32bit:QMAKE_LFLAGS += -m32
    64bit:QMAKE_CXXFLAGS += -m64
    64bit:QMAKE_LFLAGS += -m64
}

######################################
# Linux platform specific variables
######################################
unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    32bit:TARGET = lin_32.xpl
    64bit:TARGET = lin_64.xpl
    QMAKE_CXXFLAGS += -fvisibility=hidden
    LIBS += -L../Libs -lusb-1.0
}

######################################
# Mac OSX platform specific variables
######################################
macx {
    DEFINES += APL=1 IBM=0 LIN=0
    TARGET = mac.xpl
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    CONFIG += x86 ppc
}

######################################
# Source Files - Headers
######################################
HEADERS		+= include/*.h

######################################
# Source Files - CPP
######################################
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
