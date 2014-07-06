#ifndef GUI_MFD_H
#define GUI_MFD_H

#include "tools.h"
#include "out.h"

class gui_mfd_t;

class gui_mfd_t: public tools_t
{
public:
    gui_mfd_t(out_t* out_ref);
    ~gui_mfd_t(void);
    static void enable(void);
    static void disable(void);
private:
    static void create(int x, int y, int w, int h);
    static int	click(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2);
    static void hotkey_handler(void * inRefcon);
};
#endif // GUI_MFD_H
