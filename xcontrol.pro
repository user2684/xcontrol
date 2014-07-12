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
#
# QT Creator Build chain: msvc32, msvc64
# QT Creator build directory: build\working
# QT Creator Custom process step (msvc32): cmd.exe /c copy /y release\win_32.xpl.dll ..\release\32\win.xpl
# QT Creator Custom process step (msvc64): cmd.exe /c copy /y release\win_64.xpl.dll ..\release\64\win.xpl
######################################
win32 {
    DEFINES += APL=0 IBM=1 LIN=0 _CRT_SECURE_NO_WARNINGS
    32bit:TARGET = win_32.xpl
    64bit:TARGET = win_64.xpl
    LIBS += -L../lib
    32bit:LIBS += -lXPLM -lXPWidgets -llibusb0
    64bit:LIBS += -lXPLM_64 -lXPWidgets_64 -llibusb0_64
}

######################################
# *unix platform common variables
######################################
unix|macx {
    QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
    32bit {
        QMAKE_CFLAGS += -m32
        QMAKE_CXXFLAGS += -m32
        QMAKE_LFLAGS += -m32
    }
    64bit {
        QMAKE_CFLAGS += -m64
        QMAKE_CXXFLAGS += -m64
        QMAKE_LFLAGS += -m64
    }

}

######################################
# Linux platform specific variables
#
# QT Creator Build chain: gcc
# QT Creator build directory: build/working
# QT Creator Custom process step: cp liblin_32.xpl.so ../release/32/lin.xpl
# QT Creator Custom process step: cp liblin_64.xpl.so ../release/64/lin.xpl
######################################
unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1
    32bit:TARGET = lin_32.xpl
    64bit:TARGET = lin_64.xpl
    QMAKE_CXXFLAGS += -fvisibility=hidden
    # Libusb0 source files
    INCLUDEPATH         += include/libusb0
    SOURCES             += libusb0/usb.c
    SOURCES             += libusb0/error.c
    SOURCES             += libusb0/linux.c
    SOURCES             += libusb0/descriptors.c
    # Libusb-1.0 source files
    # LIBS += -ludev
    # SOURCES        += libusb-1.0/os/linux_usbfs.c libusb-1.0/os/linux_udev.c
}

######################################
# Mac OSX platform specific variables
#
# QT Creator Build chain: xcode
# QT Creator build directory: build/working
# QT Creator Custom process step: cp libmac_32.xpl.dylib ../release/32/mac.xpl
# QT Creator Custom process step: cp libmac_64.xpl.dylib ../release/64/mac.xpl
######################################
macx {
    DEFINES += APL=1 IBM=0 LIN=0
    32bit:TARGET = mac_32.xpl
    64bit:TARGET = mac_64.xpl
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    CONFIG += x86 ppc
    # Libusb-1.0 source files
    INCLUDEPATH         += include/libusb-1.0
    INCLUDEPATH         += include/libusb-1.0/os
    SOURCES             += libusb-1.0/core.c
    SOURCES             += libusb-1.0/descriptor.c
    SOURCES             += libusb-1.0/io.c
    SOURCES             += libusb-1.0/strerror.c
    SOURCES             += libusb-1.0/sync.c
    SOURCES             += libusb-1.0/hotplug.c
    SOURCES             += libusb-1.0/os/threads_posix.c
    SOURCES             += libusb-1.0/os/poll_posix.c
    SOURCES             += libusb-1.0/os/darwin_usb.c
}

######################################
# Source Files - Headers
######################################
HEADERS		+= include/*.h
HEADERS		+= include/libusb-1.0/*.h
HEADERS		+= include/libusb0/*.h
HEADERS		+= include/libusb-win32/*.h
HEADERS		+= include/SDK/*.h
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
SOURCES             += gui_mfd.cpp
SOURCES             += gui_fms_option.cpp
SOURCES             += gui_fms_status.cpp
