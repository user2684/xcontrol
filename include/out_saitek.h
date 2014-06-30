#ifndef OUT_SAITEK_H
#define OUT_SAITEK_H

#include <string>
#include "tools.h"
#include "out.h"

using namespace std;

struct usb_dev_handle;

class out_saitek_t : public tools_t, public out_joystick_t
{
public:
    out_saitek_t(void);
    ~out_saitek_t(void);
    void init(void);
    void deinit(void);
    void set_display_brightness(char brightness);
    void set_led_brightness(char brightness);
    void set_textdata(const char* text);
    string get_text(void);
    void set_led(int led, int on);
    void set_time(bool h24, int hour, int minute);
    void set_date(int year, int month, int day);
    void print(void);
private:
    bool search(void);
    bool attach(void);
    bool detach(void);
    void print(const char* text, ...);
    void print_line(int line, const char *text, int length);
    void clear_line(int line);
    int send_usb(int index, int value);
    std::string a_textdata;
    bool a_attached;
    struct usb_device* a_joydev;
    usb_dev_handle* a_usbhdl;
    int product;
};


#endif /* OUT_SAITEK_H */
