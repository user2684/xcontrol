#ifndef OUT_H
#define OUT_H

#include <string>
#include "tools.h"

using namespace std;

struct usb_dev_handle;

class out_t : public tools_t
{
public:
    out_t(void);
    ~out_t(void);

    void set_textdata(const char* text);
    void print(const char* text, ...);
    void print();
    void time(bool h24, char hour, char minute);
    void date(int year, int month, int day);
    void display_brightness(char brightness);
    void led_brightness(char brightness);
    void set_led(int led, int on);
    void set_led_by_name(int led_debug, const char* dateref, int data,const char* action, ...);
    void clear(void);
    int led_debug;
	struct libusb_device_handle* a_joydev;
	
private:
    void setbrightness(bool mfd, char brightness);
    void setled(int led, int on);
    void settext(int line, const char *text, int length);
    void settime(int h24, int hour, int minute);
    void setdate(int year, int month, int day);
    void cleartext(int line);
    usb_dev_handle* a_usbhdl;
    int product;
    std::string a_textdata;

};

#endif /* OUT_H */
