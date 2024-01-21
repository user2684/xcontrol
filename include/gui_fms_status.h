#ifndef GUI_FMS_STATUS_H
#define GUI_FMS_STATUS_H


#include "tools.h"
#include "config.h"
#include "fms.h"

class gui_fms_status_t;

class gui_fms_status_t: public tools_t
{
public:
    gui_fms_status_t(config_t* config_ref,fms_t* fms_ref);
    ~gui_fms_status_t(void);
    static void enable(void);
    static void disable(void);
private:
    static void create(void);
    static void hotkey_handler(void * inRefcon);
    static int  click(XPWidgetMessage inMessage,XPWidgetID inWidget,intptr_t inParam1,intptr_t inParam2);
};

#endif // GUI_FMS_STATUS_H
