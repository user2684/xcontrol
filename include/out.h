#ifndef OUT_H
#define OUT_H

#include <string>
#include "tools.h"

using namespace std;

class out_joystick_t
{
public:
    virtual void set_display_brightness(char brightness)=0;
    virtual void set_led_brightness(char brightness)=0;
    virtual void set_textdata(const char* text)=0;
    virtual string get_text(void)=0;
    virtual void set_led(int led, int on)=0;
    virtual void set_time(bool h24, int hour, int minute)=0;
    virtual void set_date(int year, int month, int day)=0;
    virtual void print(void)=0;
    virtual void init(void)=0;
    virtual void deinit(void)=0;
private:
};

class out_t : public tools_t
{
public:
    out_t(void);
    ~out_t(void);
    out_joystick_t* a_joystick;
private:
};


#endif /* OUT_H */
