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
#include "include/gui_fms_option.h"
#include "include/out.h"
#include "include/config.h"
#include "include/fms.h"


static XPWidgetID widget;
static XPWidgetID window;
static XPWidgetID Text_fl;
static XPWidgetID Text_gs;
static XPWidgetID Text_takeoff;
static XPWidgetID button_apply;
static config_t* a_config_ref;
static fms_t* a_fms_ref;

gui_fms_option_t::gui_fms_option_t(config_t* config_ref,fms_t* fms_ref)
{
    a_config_ref = config_ref;
    a_fms_ref = fms_ref;
}


// handle the creation of the option window
void gui_fms_option_t::enable(void) {
        if (XPIsWidgetVisible(widget)) XPHideWidget(widget);
        else {
            create(300, 600, 300, 350);
            XPShowWidget(widget);
        }
}

// handle the destruction of the option window
void gui_fms_option_t::disable(void) {
}

// build the FMS Options window
void gui_fms_option_t::create(int x, int y, int w, int h){
    int x2 = x + w;
    int y2 = y - h;
    int Item =0;
    // retrieve default FL and GS
    char buffer_fl[512],buffer_gs[512],buffer_takeoff[512];
    map<string,string> config=a_config_ref->get_config_fms();
    sprintf(buffer_fl, "%i",::atoi(config["cruise_fl"].c_str()));
    sprintf(buffer_gs, "%i",::atoi(config["cruise_gs"].c_str()));
    sprintf(buffer_takeoff, "%s",config["scheduled_takeoff"].c_str());
    // build the option window
    widget = XPCreateWidget(x, y, x2, y2,1,"FMS Options",1,NULL,xpWidgetClass_MainWindow);
    XPSetWidgetProperty(widget, xpProperty_MainWindowHasCloseBoxes, 1);
    window = XPCreateWidget(x+30, y-30, x2-30, y2+30,1,	"",0,widget,xpWidgetClass_SubWindow);
    XPSetWidgetProperty(window, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    // build the labels and the text box widgets
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Requested Flight Level (FL)",0,widget,xpWidgetClass_Caption);
    Item++;
    Text_fl = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_fl, 0, widget,xpWidgetClass_TextField);
    Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Requested Ground Speed (kts)",0,widget,xpWidgetClass_Caption);
    Item++;
    Text_gs = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_gs, 0, widget,xpWidgetClass_TextField);
    Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Expected Take-off Zulu Time (HH:MM)",0,widget,xpWidgetClass_Caption);
    Item++;
    Text_takeoff = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_takeoff, 0, widget,xpWidgetClass_TextField);
    // build the buttons
    Item++;
    button_apply = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, "Apply Data", 0, widget,xpWidgetClass_Button);
    XPSetWidgetProperty(button_apply, xpProperty_ButtonType, xpPushButton);
    // Add the callback
    XPAddWidgetCallback(widget, click);
}


// handle events from the FMS options panel
int gui_fms_option_t::click(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
    if (inMessage == xpMessage_CloseButtonPushed){
            if(XPIsWidgetVisible(widget)) XPHideWidget(widget);
            return 1;
    }
    if (inMessage == xpMsg_PushButtonPressed){
        if (inParam1 == (intptr_t)button_apply){
            char buffer_fl[512],buffer_gs[512],buffer_takeoff[512];
            XPGetWidgetDescriptor(Text_fl, buffer_fl, 512);
            XPGetWidgetDescriptor(Text_gs, buffer_gs, 512);
            XPGetWidgetDescriptor(Text_takeoff, buffer_takeoff, 512);
            string fl_s = string(buffer_fl);
            string gs_s = string(buffer_gs);
            string takeoff_s = string(buffer_takeoff);
            a_config_ref->set_fms_config(fl_s,gs_s,takeoff_s);
            a_fms_ref->a_fp_ref->a_status = 99; // force refresh
            XPHideWidget(widget);
            return 1;
        }
    }
    return 0;
}
