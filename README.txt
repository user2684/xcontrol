=====================================
X-Control Enhanced Plugin
=====================================
The source code is available on GitHub at the following location: https://github.com/user2684/xcontrol

Requirements
=====================================
The plugin can be built using QT Creator and qmake across the different platform and compiles with:
- msvc 2013 on Windows (http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx)
- XCode on Mac OSX (https://developer.apple.com/xcode/)
- gcc on Linux (https://gcc.gnu.org/)

Compiling the plugin
=====================================
In order to compile the code, accomplish the following steps:
- Install QT Creator and the relevant QT libraries (http://qt-project.org/downloads)
- Install the compiler for the platform of choice
- Open the "xcontrol.pro" project file
- Configure the build chain based on the instruction available in the project file 
- Set the target architecture in the project file.
- Build the project
- When building for different architectures using the same build chain, remember to clean all before rebuilding.

in alternative:

```sh
qmake && make
```

Supported joysticks
=====================================
The plugin supports the following Saitek X52 and Saitek X52 Pro joystick models:
- IdVendor: 0x06A3, Id Product 0x0255
- IdVendor: 0x06A3, Id Product 0x075C
- IdVendor: 0x06A3, Id Product 0x0762
- IdVendor: 0x06A3, Id Product 0x0BAC

Supported USB Libraries
=====================================
In order to communicate with the joystick, the plugin uses the following libraries:
- libusb-1.0 in Max OSX (http://www.libusb.org/). The library is compiled within the plugin and does not require any further installation. 
- libusb-0.1 in Linux (http://www.libusb.org/). The library is compiled within the plugin and does not require any further installation. udev is required.
- libusb-win32 filter driver in Windows (http://sourceforge.net/p/libusb-win32/wiki/Home/). libusb-win32 is the only library that provides a filter driver which would still make the joystick's original functions available while allowing the plugin to interact with it

Directory Structure
=====================================
- Root directory: contains the plugin source code
- include: contains the plugin include files
- include/libusb-1.0: contains libusb-1.0 include files
- include/libusb-win32: contains libusb-win32 include files
- include/libusb0: contains libusb include files
- include/SDK: contains X-Plane SDK include files
- libusb-1.0: contains libusb-1.0 source code
- libusb0: contains libusb source code
- build/lib: contains libraries used when dinamically linking the plugin at compilation time
- build/release: contains the files and directories to be distributed
- build/working: builds the plugin within this directory
