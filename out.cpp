#include "include/out.h"
#include "include/out_saitek.h"

out_t::out_t(void)
{
    a_joystick = new out_saitek_t();
    a_joystick->init();
}

out_t::~out_t(void)
{
    a_joystick->deinit();
}

