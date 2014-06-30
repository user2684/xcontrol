#include <algorithm>
#include <cctype>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "include/libusb/lusb0_usb.h"
#include "include/out_saitek.h"

extern const char* version;

#define OFFLINE_MSG  "   Saitek X52\n     Flight\n Control System"
#define ONLINE_MSG   "   xcontrol  \n \n ver %s", version

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
}

void out_saitek_t::init(void)
{
    debug_out(debug, "out_saitek: searching for a supported Saitek joystick");
    bool found = search();
    if (found) {
       a_attached = attach();
    }
}

void out_saitek_t::deinit(void)
{
   if (a_attached) detach();
   a_joydev = NULL;
   a_attached = false;
}

bool out_saitek_t::search(void)
{
    int supported_devices [4] = { x52_standard_device1, x52_standard_device2, x52_pro_device, x52_other_device};

    usb_bus* bus               = 0;
    struct usb_device* dev     = 0;
    usb_device_descriptor* dsc = 0;

    debug_out(debug,"out_saitek: libusb v0.1 in use");
    usb_init();
    int busses = usb_find_busses();
    int devices = usb_find_devices();
    debug_out(debug, "out_saitek: enumerated %d busses and %d devices",busses,devices);

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
                    product = supported_devices[i];
                    a_joydev = dev;
                }
                if (a_joydev) break;
            }
            if (a_joydev) break;
        }
        if (a_joydev) break;
    }
    if (a_joydev == NULL) {
        debug_out(err, "out_saitek: cannot find any supported device");
        return false;
    }
    return true;
}

bool out_saitek_t::attach(void)
{
    a_usbhdl = usb_open(a_joydev);
    if (!a_usbhdl) {
        debug_out(warn,"out_saitek: could not open joystick");
        a_joydev = NULL;
        return false;
    }
    debug_out(debug, "out_saitek: setting initial brightness and welcome message");
    set_display_brightness(0x7F);
    set_led_brightness(0x7F);
    print(ONLINE_MSG);
    return true;
}

bool out_saitek_t::detach(void)
{

    debug_out(info, "out_saitek: detaching joystick connection");
    print(OFFLINE_MSG);
    set_time(true, 0, 0);
    set_date(0, 0, 0);
    set_display_brightness(0x00);
    set_led_brightness(0x00);
    int res = usb_close(a_usbhdl);
    if (res < 0) debug_out(err,"out_saitek: error while detaching the joystick: %d",res);
    debug_out(info,"out_saitek: joystick disconnected cleanly");
    return true;
}

void out_saitek_t::set_display_brightness(char brightness) {
    if (!a_attached) return;
    int res = 0;
    bool mfd = true;
    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,brightness, mfd?0xB1:0xB2, 0, 0, 100);
    if (res < 0) debug_out(err,"out_saitek: error while setting brightness %c on display: %d",brightness,res);
}

void out_saitek_t::set_led_brightness(char brightness)
{
    if (!a_attached) return;
    int res = 0;
    bool mfd = false;
    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,brightness, mfd?0xB1:0xB2, 0, 0, 100);
    if (res < 0) debug_out(err,"out_saitek: error while setting brightness %c on LEDs: %d",brightness,res);
}

void out_saitek_t::set_textdata(const char* text)
{
    if (!a_attached) return;
    if (!text) return;
    a_textdata.assign(text);
}

string out_saitek_t::get_text(void)
{
    return a_textdata;
}

void out_saitek_t::print()
{
    if (!a_attached) return;
    print(a_textdata.c_str());
}

void out_saitek_t::print(const char* t, ...)
{
    int n_lf = 0;
    // the display only supports 48 characters, we allow 2 additional newline
    // characters and a terminating null byte, any additional characters are discarded
    char text[51] = {};
    if (!t || product == x52_other_device) return;
    //clear();
    if (!strlen(t)) return;
    va_list ap;
    va_start(ap, t);
    vsnprintf(text, 51, t, ap);
    va_end(ap);

    char* token = strtok(text, "\n");
    while (token && (n_lf < 3))
    {
        char line[17] = {};
        strncpy(line, token, 16);
        line[16] = 0;
        try
        {
            print_line(n_lf, line, 16);
        }
        catch (const char* reason)
        {
            debug_out(err,"out_saitek_t: internal error printing %s: %s",line,reason);
        }
        n_lf++;
        token = strtok(0, "\n");
    }
}

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
        if (res < 0) debug_out(err,"out_saitek: cannot print line %d with text '%s': %d",line,text,res);
        length -= 2;
        text += 2;
    }
}

void out_saitek_t::clear_line(int line)
{
    unsigned char line_clearctl[3] = {0xD9, 0xDA, 0xDC};
    int res = send_usb(line_clearctl[line],0x00);
    if (res < 0) debug_out(err,"out_saitek: cannot clear line %d: %d",line,res);
}


void out_saitek_t::set_led(int led, int on)
{
    int res = send_usb(0xB8,on | (led<<8));
    if (res < 0) debug_out(err,"out_saitek: cannot set let %d on %d: %d",led,on,res);
}

void out_saitek_t::set_time(bool h24, int hour, int minute)
{
    unsigned short timedata = minute | (hour<<8) | (h24?0x8000:0);
    int res = send_usb(0xC0,timedata);
    if (res < 0) debug_out(err,"out_saitek: cannot set time to %c:%c: %d",hour,minute,res);
}

void out_saitek_t::set_date(int year, int month, int day)
{
    int res;
    if (product == x52_other_device) return;
    unsigned short datedata = day | (month<<8);
    unsigned short yeardata = year;
    res = send_usb(0xC4,datedata);
    if (res < 0) debug_out(err,"out_saitek: cannot set day to %d and month to %d: %d",day,month,res);
    res = send_usb(0xC8,yeardata);
    if (res < 0) debug_out(err,"out_saitek: cannot set your to %d: %d",year,res);
}

int out_saitek_t::send_usb(int index, int value)
{
    int requesttype = USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT;
    int request = 0x91;
    int timeout = 100;
    return usb_control_msg(a_usbhdl,requesttype,request,value,index,0,0,timeout);

}

