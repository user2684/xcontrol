
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
#include "include/gui_fms_status.h"
#include "include/config.h"
#include "include/fms.h"

#if IBM
#define snprintf _snprintf
#endif

static XPWidgetID widget;
static XPWidgetID window;
static XPWidgetID button_prev;
static XPWidgetID button_next;
static XPWidgetID button_refresh;
static int status_page;
static config_t* a_config_ref;
static fms_t* a_fms_ref;
static fms_utils_t* a_fms_utils_ref;
static XPLMHotKeyID hotkey = NULL;

gui_fms_status_t::gui_fms_status_t(config_t* config_ref,fms_t* fms_ref)
{
    a_config_ref = config_ref;
    a_fms_ref = fms_ref;
    a_fms_utils_ref = new fms_utils_t();
    hotkey = XPLMRegisterHotKey(XPLM_VK_B, xplm_DownFlag + xplm_ShiftFlag + xplm_ControlFlag,"Show the FMS Status",hotkey_handler,NULL);
}


// handle the creation of the status window
void gui_fms_status_t::enable(void) {
        if (XPIsWidgetVisible(widget)) XPHideWidget(widget);
        else {
            create();
            XPShowWidget(widget);
        }
}

// handle the destruction of the status window
void gui_fms_status_t::disable(void) {
    XPLMUnregisterHotKey(hotkey);
}

// build the FMS Status window
void gui_fms_status_t::create(void){
    int width;
    int height;
    XPLMGetScreenSize(&width, &height);
    int x_margin = 100;
    int y_margin = 100;
    int x1 = x_margin;
    int y1 = height - x_margin;
    int x2 = width-y_margin;
    int y2 = y_margin;
    int box_l = 60;
    int box_h = 20;
    int h = 15;
    int items_per_page = (height-(2*y_margin)-(9*h)-60)/h;
    char temp[2048] = {};
    memset(temp, 0, 2048);
    // build the status window
    XPDestroyWidget(widget, 1);
    widget = XPCreateWidget(x1,y1,x2,y2,1,"FMS Status",1,NULL,xpWidgetClass_MainWindow);
    XPSetWidgetProperty(widget, xpProperty_MainWindowHasCloseBoxes, 1);
    x1 += 30; x2 -=30; y1 -=30; y2 +=30;
    window = XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_SubWindow);
    XPSetWidgetProperty(window, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    // build the buttons
    button_next = XPCreateWidget(x2-100,y2,x2-50,y2-20,1, "Next ->", 0, widget,xpWidgetClass_Button);
    XPSetWidgetProperty(button_next, xpProperty_ButtonType, xpPushButton);
    XPAddWidgetCallback(button_next, click);
    button_prev = XPCreateWidget(x2-200,y2,x2-150,y2-20,1, "<- Prev", 0, widget,xpWidgetClass_Button);
    XPSetWidgetProperty(button_prev, xpProperty_ButtonType, xpPushButton);
    XPAddWidgetCallback(button_prev, click);
    button_refresh = XPCreateWidget(x2-300,y2,x2-250,y2-20,1, "Refresh", 0, widget,xpWidgetClass_Button);
    XPSetWidgetProperty(button_refresh, xpProperty_ButtonType, xpPushButton);
    XPAddWidgetCallback(button_refresh, click);
    // build the trip summary
    if (a_fms_ref->a_fp_ref->a_status > 0) {
        snprintf(temp, 2048, "%s (at %5s) -> %s (at %5s)",
            a_fms_ref->a_fp_ref->a_fp[0]->a_name.c_str(),
            a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[0]->a_time_scheduled).c_str(),
            a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_name.c_str(),
            a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_scheduled).c_str()
            );
        y1 -= h;  y2= y1-box_h;
        XPCreateWidget(x1,y1, x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Distance: %.0f nm. Expected duration: %5s",
            a_fms_ref->a_fp_ref->a_total_distance,
            a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_total_time).c_str()
            );
        y1 -= h;  y2= y1-box_h;
        XPCreateWidget(x1,y1, x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Left departure gate at: %s",a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[0]->a_time_at_gate).c_str());
        y1 -= h;  y2= y1-box_h;
        XPCreateWidget(x1,y1, x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Arrived at the gate at: %s",a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_at_gate).c_str());
        y1 -= h;  y2= y1-box_h;
        XPCreateWidget(x1,y1, x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
        // build the legend
        y1 -= h;
        int x1_orig = x1;
        x2 = x1+box_l/2;
        y1 -= h;
        y2 = y1-box_h;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Curr",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Name",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Status",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Leg Dist",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Dist Dep",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l+30;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Dist Arr",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Leg Time",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Time Dep",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l+30;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Time Arr",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Actual",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Sched",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Delay",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Expect",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"Delay",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"FL Exp",0,widget,xpWidgetClass_Caption);
        x1 = x2;  x2 = x1 +box_l;
        XPCreateWidget(x1,y1,x2,y2,1,(char*)"FL Act",0,widget,xpWidgetClass_Caption);
        // cycle over the items
        fms_fp_item_t* button_prev_item = new fms_fp_item_t();
        if (status_page < 0) status_page = 0;
        int current_entry_index = a_fms_ref->flying_index();
        for (int i = items_per_page*status_page; i < (items_per_page*status_page) + items_per_page; i++) {
            x1 = x1_orig;
            x2 = x1+box_l/2;
            y1 -= h;
            y2 = y1-box_h;
            if (i > a_fms_ref->a_fp_ref->a_arrival_index) break; // reached the end of the fp
            fms_fp_item_t * item = a_fms_ref->a_fp_ref->a_fp[i];
            if (button_prev_item->a_flight_status == "climb" && item->a_flight_status == "cruise") { // TOC
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"Top of Climb",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                snprintf(temp, 2048,"%.0fnm",a_fms_ref->a_fms_config_ref->a_fms_config["climb_distance"]);
                XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l+30;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fms_config_ref->a_fms_config["climb_time"]).c_str(),0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l+30;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_toc->a_time_scheduled).c_str(),0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x1_orig;
                x2 = x1+box_l/2;
                y1 -= h;
                y2 = y1-box_h;
            }
            if (button_prev_item->a_flight_status == "cruise" && item->a_flight_status == "desc") { //TOD
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"Top of Desc",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l+30;
                snprintf(temp, 2048,"%.0fnm",a_fms_ref->a_fms_config_ref->a_fms_config["desc_distance"]);
                XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l+30;
                XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fms_config_ref->a_fms_config["desc_time"]).c_str(),0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_tod->a_time_scheduled).c_str(),0,widget,xpWidgetClass_Caption);
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x2;  x2 = x1 +box_l;
                XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
                x1 = x1_orig;
                x2 = x1+box_l/2;
                y1 -= h;
                y2 = y1-box_h;
            }
            if (item->a_index == current_entry_index) XPCreateWidget(x1,y1,x2,y2,1,"->",0,widget,xpWidgetClass_Caption);
            else XPCreateWidget(x1,y1,x2,y2,1,"",0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,item->a_name.c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,item->a_flight_status.c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_prev);
            XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_departure);
            XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l+30;
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_arrival);
            XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_prev).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_departure).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l+30;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_arrival).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_actual).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_scheduled).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->delay2string(item->a_delay_from_scheduled).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_expected).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->delay2string(item->a_delay_from_expected).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->altitude2string(item->a_altitude_expected).c_str(),0,widget,xpWidgetClass_Caption);
            x1 = x2;  x2 = x1 +box_l;
            XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->altitude2string(item->a_altitude_actual).c_str(),0,widget,xpWidgetClass_Caption);
            button_prev_item = item;
        }
    } else {
        snprintf(temp, 2048, "FMS NOT READY. Status:%.8s",a_fms_ref->a_fp_ref->status2string(a_fms_ref->a_fp_ref->a_status).c_str());
        XPCreateWidget(x1,y1,x2,y2,1,temp,0,widget,xpWidgetClass_Caption);
    }
    // Add the callback
    XPAddWidgetCallback(widget, click);
}

// handle events from the FMS status panel
int gui_fms_status_t::click(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
    if (inMessage == xpMessage_CloseButtonPushed){
            if(XPIsWidgetVisible(widget)) XPHideWidget(widget);
            return 1;
    }
    if (inMessage == xpMsg_PushButtonPressed){
        if (inParam1 == (intptr_t)button_next){
            status_page++;
            create();
            return 1;
        }
        if (inParam1 == (intptr_t)button_prev){
            status_page--;
            create();
            return 1;
        }
        if (inParam1 == (intptr_t)button_refresh){
            create();
            return 1;
        }
    }
    return 0;
}

void gui_fms_status_t::hotkey_handler(void * inRefcon) {
    enable();
}
