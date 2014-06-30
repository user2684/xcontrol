#ifndef GUI_H
#define GUI_H

#include <string>
#include <map>
#include "include/SDK/XPLMDisplay.h"
#include "include/SDK/XPLMUtilities.h"
#include "include/SDK/XPLMDataAccess.h"
#include "include/SDK/XPLMMenus.h"
#include "include/SDK/XPLMGraphics.h"
#include "include/SDK/XPWidgets.h"
#include "include/SDK/XPStandardWidgets.h"
#include "include/SDK/XPLMDefs.h"
#include "tools.h"
#include "fms.h"
#include "mfdpage.h"
#include "out.h"

using namespace std;
class gui_t;

class gui_t: public tools_t, public fms_utils_t
{
public:
    gui_t(void);
    gui_t(mfdpages_t* mfdpages, fms_t* fms_ref, out_t* out_ref);
	~gui_t(void);
    static void enable(void);
	static void disable(void);
private:
	static void menu_handler(void *inMenuRef, void *inItemRef);
	static void fms_status_hotkey_handler(void * inRefcon);
	static void fms_option_window_handler(void);
	static void fms_option_create_window(int x, int y, int w, int h);
	static int	fms_option_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2);
	static void fms_status_window_handler(void);
	static void fms_status_create_window(void);
	static int	fms_status_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2);
    static void mfd_window_handler(void);
    static void mfd_create_window(int x, int y, int w, int h);
    static int	mfd_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2);

};

#endif /* GUI_H */
