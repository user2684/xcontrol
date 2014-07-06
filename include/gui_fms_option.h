#ifndef GUI_FMS_OPTION_H
#define GUI_FMS_OPTION_H

#include "tools.h"
#include "config.h"
#include "fms.h"

class gui_fms_option_t;

class gui_fms_option_t: public tools_t
{
public:
    gui_fms_option_t(config_t* config_ref,fms_t* fms_ref);
    ~gui_fms_option_t(void);
    static void enable(void);
    static void disable(void);
private:
    static void create(int x, int y, int w, int h);
    static int	click(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2);
};


#endif // GUI_FMS_OPTION_H
