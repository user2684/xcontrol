#include <string>
#include <stdio.h>
#include <map>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "include/SDK/XPLMDisplay.h"
#include "include/SDK/XPLMUtilities.h"
#include "include/SDK/XPLMDataAccess.h"
#include "include/SDK/XPLMMenus.h"
#include "include/SDK/XPLMGraphics.h"
#include "include/SDK/XPWidgets.h"
#include "include/SDK/XPStandardWidgets.h"
#include "include/SDK/XPLMDefs.h"
#include "include/gui_mfd.h"
#include "include/out.h"
#include "include/config.h"


static XPWidgetID mfd_widget;
static XPWidgetID mfd_window;
static XPWidgetID mfd_refresh;
static out_t* a_out_ref;
static XPLMHotKeyID hotkey = NULL;

gui_mfd_t::gui_mfd_t(out_t* out_ref)
{
    a_out_ref = out_ref;
    hotkey = XPLMRegisterHotKey(XPLM_VK_V, xplm_DownFlag + xplm_ShiftFlag + xplm_ControlFlag,"Show the virtual MFD",hotkey_handler,NULL);
}


// handle the creation of the MFD window
void gui_mfd_t::enable(void) {
        if (XPIsWidgetVisible(mfd_widget)) XPHideWidget(mfd_widget);
        else {
            create(300, 600, 250, 200);
            XPShowWidget(mfd_widget);
        }
}

// handle the destruction of the MFD window
void gui_mfd_t::disable(void) {
}

// build the MFD window
void gui_mfd_t::create(int x, int y, int w, int h){
    int x2 = x + w;
    int y2 = y - h;
    int Item =-1;
    // build the MFD window
    XPDestroyWidget(mfd_widget, 1);
    mfd_widget = XPCreateWidget(x, y, x2, y2,1,"Virtual MFD",1,NULL,xpWidgetClass_MainWindow);
    XPSetWidgetProperty(mfd_widget, xpProperty_MainWindowHasCloseBoxes, 1);
    mfd_window = XPCreateWidget(x+30, y-30, x2-30, y2+30,1,	"",0,mfd_widget,xpWidgetClass_SubWindow);
    XPSetWidgetProperty(mfd_window, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    // set the text
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,a_out_ref->a_joystick->get_text(0).c_str(),0,mfd_widget,xpWidgetClass_Caption);
    Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,a_out_ref->a_joystick->get_text(1).c_str(),0,mfd_widget,xpWidgetClass_Caption);
    Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,a_out_ref->a_joystick->get_text(2).c_str(),0,mfd_widget,xpWidgetClass_Caption);
    Item++;
    mfd_refresh = XPCreateWidget(x+60+40, y-(70 + (Item*30))-45, x+115+40, y-(92 + (Item*30))-45,1, "Refresh", 0, mfd_widget,xpWidgetClass_Button);
    XPSetWidgetProperty(mfd_refresh, xpProperty_ButtonType, xpPushButton);
    // Add the callback
    XPAddWidgetCallback(mfd_widget,click);
}

// handle events from the MFD panel
int gui_mfd_t::click(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
    if (inMessage == xpMessage_CloseButtonPushed){
            if(XPIsWidgetVisible(mfd_widget)) XPHideWidget(mfd_widget);
            return 1;
    }
    if (inMessage == xpMsg_PushButtonPressed){
        if (inParam1 == (intptr_t)mfd_refresh){
            create(300, 600, 250, 200);
            return 1;
        }
    }
    return 0;
}

void gui_mfd_t::hotkey_handler(void * inRefcon) {
    enable();
}
