#include <algorithm>
#include <cctype>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "include/out_saitek.h"
#include "include/version.h"

#if IBM
#define snprintf _snprintf
#endif


#if IBM // Use libusb-win32
#include "include/libusb-win32/lusb0_usb.h"
#elif LIN // Use libusb0
#include "include/libusb0/usb.h"
#else // Use libusb-1.0
#include "include/libusb-1.0/libusb.h"
#endif

// define the messages to display when the plugin load/unload

#define OFFLINE_MSG  "   Saitek X52\n     Flight\n Control System"
#define ONLINE_MSG   "X Control Plugin\nVersion 1.0.0b \nLoading...   ", plugin_version

// Define supported joystick IDs
enum saitek_vendors
{
    saitek_vendor  = 0x06A3
};

enum saitek_products
{
    x52_standard_device1 = 0x0255,
    x52_standard_device2 = 0x075C,
    x52_pro_device       = 0x0762,
    x52_other_device     = 0x0BAC
};


out_saitek_t::out_saitek_t(void)
{
    a_attached = false;
    a_joydev = NULL;
    a_usb_detached = false;
}

// Init the library
void out_saitek_t::init(void)
{
    debug_out(debug, "out_saitek: searching for a supported Saitek joystick");
    bool found = search();
    if (found) { // if a joystick was found, attach to it
       a_attached = attach();
    }
}

// Unload the library
void out_saitek_t::deinit(void)
{
   if (a_attached) detach();
   a_joydev = NULL;
   a_attached = false;
}

// search for a supported joystick
bool out_saitek_t::search(void)
{
    int supported_devices [4] = { x52_standard_device1, x52_standard_device2, x52_pro_device, x52_other_device};
#if IBM || LIN // libusb v0.1
    usb_bus* bus               = 0;
    struct usb_device* dev     = 0;
    usb_device_descriptor* dsc = 0;

    debug_out(debug,"out_saitek: libusb v0.1 in use");
    usb_init();
    debug_out(debug, "out_saitek: probing for a supported joystick");
    int busses = usb_find_busses();
    int devices = usb_find_devices();
    debug_out(debug, "out_saitek: enumerated %d busses and %d devices",busses,devices);
    // cycle through all the devices looking for a supported joystick
    for (bus = usb_busses; bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            dsc = &dev->descriptor;
            if (dsc->idVendor != saitek_vendor) continue;
            for (int i = 0; i<=3; i++) {
                debug_out(verbose, "out_saitek: searching for Product ID: 0x%.4X",supported_devices[i]);
                if (dsc->idProduct == supported_devices[i]) {
                    debug_out(info, "out_saitek: found Saitek X52 Flight Control System  (Product ID: 0x%.4X)",supported_devices[i]);
                    a_product = supported_devices[i];
                    a_joydev = dev;
                }
                if (a_joydev) break;
            }
            if (a_joydev) break;
        }
        if (a_joydev) break;
    }
#else // libusb v1.0
    debug_out(debug,"out_saitek: libusb v1.0 in use");
    int r = libusb_init(NULL);
    if (r < 0) {
        debug_out(err, "out_saitek: failed to initialize libusb: %s",usb_error(r));
        return false;
    }
    debug_out(debug, "out_saitek: probing for a supported joystick");
    for (int i = 0; i<=3; i++) {
            debug_out(verbose, "out_saitek: searching for Product ID: 0x%.4X",supported_devices[i]);
            a_joydev = libusb_open_device_with_vid_pid(NULL, saitek_vendor, supported_devices[i]);
            if (a_joydev != NULL) {
                debug_out(info, "out_saitek: found Saitek X52 Flight Control System  (Product ID: 0x%.4X)",supported_devices[i]);
                a_product = supported_devices[i];
                break;
            }
    }
#endif
    if (a_joydev == NULL) {
        debug_out(err, "out_saitek: cannot find any supported device");
        return false;
    }
    return true; // joystick found
}

// Attach to the device and initialize it
bool out_saitek_t::attach(void)
{
#if IBM || LIN // libusb v0.1
    a_usbhdl = usb_open(a_joydev);
    if (!a_usbhdl) {
        debug_out(warn,"out_saitek: could not open joystick");
        a_joydev = NULL;
        return false;
    }
#else // libusb v1.0
#endif

    debug_out(debug, "out_saitek: setting initial brightness and welcome message");
    set_display_brightness(0x7F);
    set_led_brightness(0x7F);
    print(ONLINE_MSG);
    return true;
}

// Detach from the device
bool out_saitek_t::detach(void)
{
    if (!a_attached)return false;
    debug_out(info, "out_saitek: detaching joystick connection");
    print(OFFLINE_MSG);
    set_time(true, 0, 0);
    set_date(0, 0, 0);
    set_display_brightness(0x00);
    set_led_brightness(0x00);
#if IBM || LIN // libusb v0.1
    int res = usb_close(a_usbhdl);
    if (res < 0) debug_out(err,"out_saitek: error while detaching the joystick: %s",usb_error(res));
#else // libusb v1.0
    libusb_close(a_joydev);
    libusb_exit(NULL);
#endif
    debug_out(info,"out_saitek: joystick disconnected cleanly");
    return true;
}

// Set the display brightness
void out_saitek_t::set_display_brightness(char brightness) {
    if (!a_attached) return;
    int res = 0;
    bool mfd = true; // Turn the lights on for the MFD
    res = send_usb(mfd?0xB1:0xB2,brightness);
    if (res < 0) debug_out(err,"out_saitek: error while setting brightness %c on display: %s",brightness,usb_error(res));
}

// Set the led brightness
void out_saitek_t::set_led_brightness(char brightness)
{
    if (!a_attached) return;
    int res = 0;
    bool mfd = false; // Do not turn the lights on for the MFD
    res = send_usb(mfd?0xB1:0xB2,brightness);
    if (res < 0) debug_out(err,"out_saitek: error while setting brightness %c on LEDs: %s",brightness,usb_error(res));
}

// Set the text for the MFD and buffer it
void out_saitek_t::set_textdata(const char* text)
{
    a_textdata.assign(text);
}

// return the MFD text
string out_saitek_t::get_text(int line)
{
    return a_display[line];
}

// print on the MFD the text that has been set
void out_saitek_t::print()
{
    print(a_textdata.c_str());
}

// Turn on a led
void out_saitek_t::set_led(int led, int on)
{
    if (!a_attached) return;
    int res = send_usb(0xB8,on | (led<<8));
    if (res < 0) debug_out(err,"out_saitek: cannot set let %d on %d: %s",led,on,usb_error(res));
}

// Set the time on the MFD
void out_saitek_t::set_time(bool h24, int hour, int minute)
{
    if (!a_attached) return;
    unsigned short timedata = minute | (hour<<8) | (h24?0x8000:0);
    int res = send_usb(0xC0,timedata);
    if (res < 0) debug_out(err,"out_saitek: cannot set time to %c:%c: %s",hour,minute,usb_error(res));
}

// Set the date on the MFD
void out_saitek_t::set_date(int year, int month, int day)
{
    if (!a_attached) return;
    int res;
    if (a_product == x52_other_device) return;
    unsigned short datedata = day | (month<<8);
    unsigned short yeardata = year;
    res = send_usb(0xC4,datedata);
    if (res < 0) debug_out(err,"out_saitek: cannot set day to %d and month to %d: %s",day,month,usb_error(res));
    res = send_usb(0xC8,yeardata);
    if (res < 0) debug_out(err,"out_saitek: cannot set year to %d: %s",year,usb_error(res));
}

// print on the MFD the given text
void out_saitek_t::print(const char* t, ...)
{
    int n_lf = 0;
    // the display only supports 48 characters, we allow 2 additional newline
    // characters and a terminating null byte, any additional characters are discarded
    char text[51] = {};
    if (!t || a_product == x52_other_device) return;
    if (!strlen(t)) return;
    va_list ap;
    va_start(ap, t);
    snprintf(text, 51, t, ap);
    va_end(ap);
    char* token = strtok(text, "\n");
    while (token && (n_lf < 3))
    {
        char line[17] = {};
        strncpy(line, token, 16);
        line[16] = 0;
        string line_string;
        line_string.assign(line);
        a_display[n_lf] = line_string;
        if (a_attached) print_line(n_lf, line, 16); // print only if the joystick is attached
        n_lf++;
        token = strtok(0, "\n");
    }
}

// Print the given text on the given line
void out_saitek_t::print_line(int line, const char *text, int length)
{
    int res;
    if (!text) return;
    debug_out(debug,"out_saitek: printing line %d: '%s'",line,text);
    unsigned char line_writectl[3] = {0xD1, 0xD2, 0xD4};
    clear_line(line);
    while (length >= 1)
    {
        unsigned short charpair;
        if (length == 1) charpair = (0 << 8) + *text;
        else charpair = *(unsigned short*) text;
        res = send_usb(line_writectl[line],charpair);
        if (res < 0) debug_out(err,"out_saitek: cannot print line %d with text '%s': %s",line,text,usb_error(res));
        length -= 2;
        text += 2;
    }
}

// clear a given line
void out_saitek_t::clear_line(int line)
{
    unsigned char line_clearctl[3] = {0xD9, 0xDA, 0xDC};
    int res = send_usb(line_clearctl[line],0x00);
    if (res < 0) debug_out(err,"out_saitek: cannot clear line %d: %s",line,usb_error(res));
}

// Send a message to the USB device
int out_saitek_t::send_usb(int index, int value)
{
    // requesttype: instruct for outboud connection
    // request: Saitek only uses endpoint 0x91
    // message: Saitek uses index and value to communicate with the joystick
#if IBM || LIN // libusb v0.1
    int requesttype = USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT;
#else // libusb v1.0
    int requesttype = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
#endif
    int request = 0x91;
    int timeout = 100;
    debug_out(verbose,"out_saitek: sending to usb requesttype: 0x%x,request: 0x%x,index: 0x%x,value: 0x%x",requesttype,request,index,value);
#if IBM || LIN // libusb v0.1
    return usb_control_msg(a_usbhdl,requesttype,request,value,index,0,0,timeout);
#else // libusb v1.0
    return libusb_control_transfer(a_joydev,requesttype,request,value,index,0,0,timeout);
#endif
}

// return the USB error
char* out_saitek_t::usb_error(int res)
{
#if IBM || LIN // libusb v0.1
    return usb_strerror();
#else // libusb v1.0
    return (char*) libusb_strerror((enum libusb_error)res);
#endif
}
