#include <algorithm>
#include <cctype>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "include/libusb-1.0/libusb.h"
#include "include/out.h"

extern const char* version;

#define STANDARD_MSG  "   Saitek \n     Flight\n Control System"
#define STANDARD_MSG_PRO " Saitek  Pro\n     Flight\n Control System"
#define WELCOME_MSG   "   control\n version %s\n   Loading...  ", version

#if defined (__ppc__)
#define __bswap_constant_16(x)                   \
    ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#define _SWAB16(x)                               \
    (__extension__                                  \
    ({ register unsigned short int __v, __x = (x);  \
    if (__builtin_constant_p (__x))                 \
        __v = __bswap_constant_16 (__x);            \
    else                                            \
        __asm__ ("rorw $8, %w0"                     \
                : "=r" (__v)                        \
                : "0" (__x)                         \
                : "cc");                            \
                __v; }))
#elif defined (__i386__)
#ifdef __bswap_constant_16
#undef __bswap_constant_16
#endif
#define __bswap_constant_16(x) x
#define _SWAB16(x) x
#endif /* __ppc__ */

enum vendors_e
{
    _vendor  = 0x06A3
};

enum products_e
{
    _standard_device  = 0x0255,
    _standard_device2 = 0x075C,
    _pro_device       = 0x0762,
    yoke_device          = 0x0BAC
};


out_t::out_t(void)
{
	int r;
	a_joydev = NULL;
	int devices [4] = { _standard_device, _standard_device2, _pro_device, yoke_device}; 
	
	debug_out(debug, "out: searching for joystick");
	r = libusb_init(NULL);
	if (r < 0) {
		debug_out(err, "out: failed to initialize libusb: %d",r);
		goto out;
	}
	debug_out(debug, "out: initialized libusb");
	
	debug_out(debug, "out: probing for supported joystick");
	for (int i = 0; i<=3; i++) {
			debug_out(verbose, "out: searching for Product ID: 0x%.4X",devices[i]);
			a_joydev = libusb_open_device_with_vid_pid(NULL, _vendor, devices[i]);
			if (a_joydev != NULL) {
				debug_out(info, "out: found Saitek  Flight Control System  (Product ID: 0x%.4X)",devices[i]);
				product = devices[i];
				break;
			}	
	}
	if (a_joydev == NULL) {
		debug_out(err, "out: cannot find any supported device");
		goto out;
	}	
	
	r = libusb_claim_interface(a_joydev, 0);
	if (r < 0) {
		debug_out(warn, "out: cannot claim usb interface (error: %d), running as an unprovileged user",r);
		//goto out; // On MacOSX returns LIBUSB_ERROR_ACCESS (-3)
	}
	debug_out(debug, "out: interface claimed");
	debug_out(debug, "out: setting initial brightness and welcome message");
	display_brightness(0x7F);
	led_brightness(0x7F);
	print(WELCOME_MSG);
	goto success;
	
	out:
	debug_out(err, "out: error initializing the joystick, disconnecting");
	if (a_joydev) libusb_close(a_joydev);
	libusb_exit(NULL);

	success:
	return ;
}

out_t::~out_t(void)
{
	debug_out(debug, "out: destroying Joystick connection");
    switch (product)
    {
    case _standard_device:
    case _standard_device2:
        print(STANDARD_MSG);
        break;
    case _pro_device:
        print(STANDARD_MSG_PRO);
        break;
    case yoke_device:
    default:
        print("");
        break;
    }
    time(true, 0, 0);
    date(0, 0, 0);
    display_brightness(0x00);
    led_brightness(0x00);
    if (a_joydev) libusb_release_interface(a_joydev, 0);
	if (a_joydev) libusb_close(a_joydev);
	libusb_exit(NULL);
    debug_out(info, "out: joystick disconnected cleanly");
}

void out_t::display_brightness(char brightness)
{
    try
    {
        setbrightness(true, brightness);
    }
    catch (const char* reason)
    {
        debug_out(err, "out: %s of multifunction display", reason);
    }
}

void out_t::led_brightness(char brightness)
{
    try
    {
        setbrightness(false, brightness);
    }
    catch (const char* reason)
    {
        debug_out(err, "out: %s of LED's", reason);
    }
}

void out_t::set_led_by_name(int led_debug, const char* dataref, int data, const char* action, ...) {
    string str=action;
    std::transform(str.begin(),str.end(),str.begin(), ::tolower);
    int pos=str.find(",",0);
    if ( pos <= 0) {
        debug_out(err,"out: unknown or empty action: %s = %d -> %s",dataref,data,action);
        return;
    }
    string led = str.substr(0,pos);
    string colors = str.substr(pos+1);

    if (led_debug)
        debug_out(warn,"out: %s: turn led %s with colors: %s and value %d",dataref,led.c_str(),colors.c_str(),data);

    map<int, string> ord_colors;
    int c=0;
    pos=colors.find(",",0);
    while ( pos > 0 ) {
        ord_colors[c++]=colors.substr(0,pos);
        colors=colors.substr(pos+1);
        pos=colors.find(",",0);
    }

    string color;
    ord_colors[c++]=colors.substr(0,pos);
    if ( data > (int) ord_colors.size()-1 ){
	 debug_out(warn,"out: no color defined for %s = %d, using last defined color: %s",dataref,data,action);
        color=ord_colors[ord_colors.size()-1 ];
    } else {
        try {
            color=ord_colors[data];
        } catch (const char* reason) {
            debug_out(err,"out: no color defined for value %d",led.c_str(),colors.c_str(),data);
            return;
        }
    }

    int led_base_number=0;
    if (led=="a") led_base_number=2;
    else if (led=="b") led_base_number=4;
    else if (led=="d") led_base_number=6;
    else if (led=="e") led_base_number=8;
    else if (led=="t1" || led == "t2") led_base_number=10;
    else if (led=="t3" || led == "t4") led_base_number=12;
    else if (led=="t5" || led == "t6") led_base_number=14;
    else if (led=="i") led_base_number=18;
    else if (led=="1") led_base_number=0;
    else if (led=="hl") led_base_number=16;
    else if (led=="th") led_base_number=20;
    else {
        debug_out(err,"out: unknown led: %s",led.c_str());
        return;
    }
    if (color=="red") {
        setled(led_base_number,1);
        setled(led_base_number+1,0);
    } else if (color=="green") {
        setled(led_base_number,0);
        setled(led_base_number+1,1);
    } else if (color=="orange" || color=="on") {
        setled(led_base_number,1);
        setled(led_base_number+1,1);
    } else if (color=="off") {
        setled(led_base_number,0);
        setled(led_base_number+1,0);
    } else {
        debug_out(err,"out: no such color: %s",color.c_str());
    }
}
void out_t::set_led(int led, int on)
{
    try
    {
        setled(led,on);
    }
    catch (const char* reason)
    {
        debug_out(err, "out: %s", reason);
    }
}

void out_t::set_textdata(const char* text)
{
    if (!text) return;
    a_textdata.assign(text);
}

void out_t::print()
{
    print(a_textdata.c_str());
}

void out_t::print(const char* t, ...)
{
    int n_lf = 0;
    // the display only supports 48 characters, we allow 2 additional newline
    // characters and a terminating null byte, any additional characters are discarded
    char text[51] = {};
    if (!t || product == yoke_device) return;
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
            settext(n_lf, line, 16);
        }
        catch (const char* reason)
        {
            debug_out(err, "out: %s (%s)", reason, line);
        }
        n_lf++;
        token = strtok(0, "\n");
    }
}

void out_t::clear(void)
{
    if (product == yoke_device) return;
    try
    {
        cleartext(0);
        cleartext(1);
        cleartext(2);
    }
    catch (const char* reason)
    {
        debug_out(err, reason);
    }
}

void out_t::time(bool h24, char hour, char minute)
{
    try
    {
        settime(h24, hour, minute);
    }
    catch (const char* reason)
    {
        debug_out(err, "out: %s", reason);
    }
}

void out_t::date(int year, int month, int day)
{
    if (product == yoke_device) return;
    try
    {
        setdate(year, month, day);
    }
    catch (const char* reason)
    {
        debug_out(err, "out: %s", reason);
    }
}


/* private members */

void out_t::setbrightness(bool mfd, char brightness)
{
    int res = 0;
	
    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          brightness, mfd?0xB1:0xB2, 0, 0, 100);
    if (res < 0)
        throw "could not set brightness";
}

void out_t::setled(int led, int on) {
    int res = 0;
    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          on | (led<<8), 0xb8, 0, 0, 100);
    if (res > 0)
        throw "could not set led";
}

void out_t::settext(int line, const char *text, int length)
{
    int res = 0;
    if (!text) return;
    unsigned char line_writectl[3] = {0xD1, 0xD2, 0xD4};
    cleartext(line);
    while (length >= 1)
    {
        unsigned short charpair;
        if (length == 1) charpair = (0 << 8) + *text;
        else charpair = *(unsigned short*) text;
        //charpair = __bswap_constant_16(charpair);
        res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                              charpair, line_writectl[line], 0, 0, 100);
        if (res < 0)
            throw "could not set textline";
        length -= 2;
        text += 2;
    }
}

void out_t::cleartext(int line)
{
    int res = 0;
    unsigned char line_clearctl[3] = {0xD9, 0xDA, 0xDC};

    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          0x00, line_clearctl[line], 0, 0, 100);
    if (res < 0)
        throw "could not clear textline";
}

void out_t::settime(int h24, int hour, int minute)
{
    int res = 0;

    unsigned short timedata = minute | (hour<<8) | (h24?0x8000:0);
    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          timedata, 0xC0, 0, 0, 100);
    if (res < 0)
        throw "could not set time";
}

void out_t::setdate(int year, int month, int day)
{
    int res = 0;

    unsigned short datedata = day | (month<<8);
    unsigned short yeardata = year;
    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          datedata, 0xC4, 0, 0, 100);
    if (res < 0)
        throw "could not set day and month";
    res = libusb_control_transfer(a_joydev, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 0x91,
                          yeardata, 0xC8, 0, 0, 100);
    if (res < 0)
        throw "could not set year";
}
