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
#include "include/gui.h"
#include "include/fms.h"
#include "include/fms_fp.h"
#include "include/config.h"

#if IBM
#define snprintf _snprintf
#endif

using namespace std;
using std::map;
using std::string;

static XPWidgetID fms_option_widget;
static XPWidgetID fms_option_window;
static XPWidgetID fms_status_widget;
static XPWidgetID fms_status_window;
static XPWidgetID Text_fl;
static XPWidgetID Text_gs;
static XPWidgetID Text_takeoff;
static XPWidgetID fms_option_apply;
static XPWidgetID fms_status_prev;
static XPWidgetID fms_status_next;
static XPWidgetID fms_status_refresh;
static XPLMMenuID myMenu;
static int mySubMenuItem;
static int a_menu_created;
static int a_fms_status_page;
static fms_t* a_fms_ref;
static config_t* a_config_ref;
static mfdpages_t* a_mfdpages_ref;
static fms_utils_t* a_fms_utils_ref;
static XPLMHotKeyID fms_status_hotkey = NULL;
static out_t* a_out_ref;
static XPWidgetID mfd_widget;
static XPWidgetID mfd_window;
static XPWidgetID mfd_refresh;

gui_t::gui_t(void)
{
}

gui_t::gui_t(mfdpages_t* mfdpages,fms_t* fms_ref,out_t* out_ref){
	a_fms_ref = fms_ref;
	a_mfdpages_ref = mfdpages;
    a_out_ref = out_ref;
	a_config_ref = config_t::getInstance();
	a_menu_created = 0;
}

gui_t::~gui_t(void) {
	disable();
}

// enagle the UI
void gui_t::enable(void) {
	// Create a Custom Menu within the Plugin Menu
    if (a_menu_created == 0) mySubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "X Control", 0, 1);
    myMenu = XPLMCreateMenu("X Control", XPLMFindPluginsMenu(), mySubMenuItem, menu_handler, 0);
    a_fms_status_page = 0;
    a_fms_utils_ref = new fms_utils_t();
    XPLMAppendMenuItem(myMenu,"FMS Settings",(void*)1,1);
    XPLMAppendMenuSeparator(myMenu);
    XPLMAppendMenuItem(myMenu,"Show FMS Status",(void*)2,1);
    XPLMAppendMenuItem(myMenu,"Show Virtual MFD",(void*)3,1);
    XPLMAppendMenuSeparator(myMenu);
    XPLMAppendMenuItem(myMenu,"Reset the plugin",(void*)4,1);
    fms_status_hotkey = XPLMRegisterHotKey(XPLM_VK_B, xplm_DownFlag + xplm_ShiftFlag + xplm_ControlFlag,"Show control FMS Status",fms_status_hotkey_handler,NULL);
	a_menu_created = 1;
}

// disable the UI
void gui_t::disable(void) {
	XPLMDestroyMenu(myMenu);
	XPLMUnregisterHotKey(fms_status_hotkey);
}

// handle the plugin menu events
void gui_t::menu_handler(void *inMenuRef, void *inItemRef) {
	intptr_t menuItem = (intptr_t)inItemRef;
	if(menuItem == 1) { // customize the FMS
		fms_option_window_handler();
	} 	else if(menuItem == 2) { // Show FMS status
		fms_status_window_handler();
    } 	else if(menuItem == 3) { // Show the MFD
        mfd_window_handler();
    } else if(menuItem == 4) { // reset the plugin
		a_fms_ref->reset(); // reset the FMS
        a_mfdpages_ref->load(); // recreate the MFD pages
	} 	
}

void	gui_t::fms_status_hotkey_handler(void * inRefcon) {
	fms_status_window_handler();
}

// handle the creation of the option window
void	gui_t::fms_option_window_handler(void) {
		if (XPIsWidgetVisible(fms_option_widget)) XPHideWidget(fms_option_widget);
		else {
			fms_option_create_window(300, 600, 300, 350);
			XPShowWidget(fms_option_widget);
		}
}
// build the FMS Options window
void gui_t::fms_option_create_window(int x, int y, int w, int h){
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
	fms_option_widget = XPCreateWidget(x, y, x2, y2,1,"FMS Options",1,NULL,xpWidgetClass_MainWindow);
	XPSetWidgetProperty(fms_option_widget, xpProperty_MainWindowHasCloseBoxes, 1);
	fms_option_window = XPCreateWidget(x+30, y-30, x2-30, y2+30,1,	"",0,fms_option_widget,xpWidgetClass_SubWindow);
	XPSetWidgetProperty(fms_option_window, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
	// build the labels and the text box widgets
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Requested Flight Level (FL)",0,fms_option_widget,xpWidgetClass_Caption);
	Item++;
	Text_fl = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_fl, 0, fms_option_widget,xpWidgetClass_TextField);
	Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Requested Ground Speed (kts)",0,fms_option_widget,xpWidgetClass_Caption);
	Item++;
	Text_gs = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_gs, 0, fms_option_widget,xpWidgetClass_TextField);
	Item++;
    XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),1,"Expected Take-off Zulu Time (HH:MM)",0,fms_option_widget,xpWidgetClass_Caption);
	Item++;
	Text_takeoff = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, buffer_takeoff, 0, fms_option_widget,xpWidgetClass_TextField);
	// build the buttons
	Item++;
	fms_option_apply = XPCreateWidget(x+120, y-(70 + (Item*30)), x+210, y-(92 + (Item*30)),1, "Apply Data", 0, fms_option_widget,xpWidgetClass_Button);
	XPSetWidgetProperty(fms_option_apply, xpProperty_ButtonType, xpPushButton);
	// Add the callback
	XPAddWidgetCallback(fms_option_widget, fms_option_handler);
}


// handle events from the FMS options panel
int gui_t::fms_option_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
	if (inMessage == xpMessage_CloseButtonPushed){
			if(XPIsWidgetVisible(fms_option_widget)) XPHideWidget(fms_option_widget);
			return 1;
	}
	if (inMessage == xpMsg_PushButtonPressed){
		if (inParam1 == (intptr_t)fms_option_apply){
			char buffer_fl[512],buffer_gs[512],buffer_takeoff[512];
			XPGetWidgetDescriptor(Text_fl, buffer_fl, 512);
			XPGetWidgetDescriptor(Text_gs, buffer_gs, 512);
			XPGetWidgetDescriptor(Text_takeoff, buffer_takeoff, 512);
			string fl_s = string(buffer_fl);
			string gs_s = string(buffer_gs);
			string takeoff_s = string(buffer_takeoff);
			a_config_ref->set_fms_config(fl_s,gs_s,takeoff_s);
			a_fms_ref->a_fp_ref->a_status = 99; // force refresh
			XPHideWidget(fms_option_widget);
			return 1;
		}
	}
	return 0;
}

// handle the creation of the status window
void gui_t::fms_status_window_handler(void) {
		if (XPIsWidgetVisible(fms_status_widget)) XPHideWidget(fms_status_widget);
		else {
			fms_status_create_window();
			XPShowWidget(fms_status_widget);
		}
}

// build the FMS Status window
void gui_t::fms_status_create_window(void){
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
	XPDestroyWidget(fms_status_widget, 1);
	fms_status_widget = XPCreateWidget(x1,y1,x2,y2,1,"FMS Status",1,NULL,xpWidgetClass_MainWindow);
	XPSetWidgetProperty(fms_status_widget, xpProperty_MainWindowHasCloseBoxes, 1);
	x1 += 30; x2 -=30; y1 -=30; y2 +=30;
	fms_status_window = XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_SubWindow);
	XPSetWidgetProperty(fms_status_window, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
	// build the buttons
	fms_status_next = XPCreateWidget(x2-100,y2,x2-50,y2-20,1, "Next ->", 0, fms_status_widget,xpWidgetClass_Button);
	XPSetWidgetProperty(fms_status_next, xpProperty_ButtonType, xpPushButton);
	XPAddWidgetCallback(fms_status_next, fms_option_handler);
	fms_status_prev = XPCreateWidget(x2-200,y2,x2-150,y2-20,1, "<- Prev", 0, fms_status_widget,xpWidgetClass_Button);
	XPSetWidgetProperty(fms_status_prev, xpProperty_ButtonType, xpPushButton);
	XPAddWidgetCallback(fms_status_prev, fms_option_handler);
	fms_status_refresh = XPCreateWidget(x2-300,y2,x2-250,y2-20,1, "Refresh", 0, fms_status_widget,xpWidgetClass_Button);
	XPSetWidgetProperty(fms_status_refresh, xpProperty_ButtonType, xpPushButton);
	XPAddWidgetCallback(fms_status_refresh, fms_option_handler);
	// build the trip summary
	if (a_fms_ref->a_fp_ref->a_status > 0) { 
        snprintf(temp, 2048, "%s (at %5s) -> %s (at %5s)",
			a_fms_ref->a_fp_ref->a_fp[0]->a_name.c_str(),
			a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[0]->a_time_scheduled).c_str(),
			a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_name.c_str(),
			a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_scheduled).c_str()
			);
		y1 -= h;  y2= y1-box_h;
		XPCreateWidget(x1,y1, x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Distance: %.0f nm. Expected duration: %5s",
			a_fms_ref->a_fp_ref->a_total_distance,
			a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_total_time).c_str()
			);
		y1 -= h;  y2= y1-box_h;
		XPCreateWidget(x1,y1, x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Left departure gate at: %s",a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[0]->a_time_at_gate).c_str());
		y1 -= h;  y2= y1-box_h;
		XPCreateWidget(x1,y1, x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
        snprintf(temp, 2048, "Arrived at the gate at: %s",a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_at_gate).c_str());
		y1 -= h;  y2= y1-box_h;
		XPCreateWidget(x1,y1, x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
		// build the legend
		y1 -= h;
		int x1_orig = x1;
		x2 = x1+box_l/2;
		y1 -= h;
		y2 = y1-box_h;
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Curr",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l;
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Name",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Status",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Leg Dist",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Dist Dep",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l+30; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Dist Arr",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Leg Time",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Time Dep",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l+30; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Time Arr",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Actual",0,fms_status_widget,xpWidgetClass_Caption);		
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Sched",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Delay",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Expect",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"Delay",0,fms_status_widget,xpWidgetClass_Caption);
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"FL Exp",0,fms_status_widget,xpWidgetClass_Caption);			
		x1 = x2;  x2 = x1 +box_l; 
		XPCreateWidget(x1,y1,x2,y2,1,(char*)"FL Act",0,fms_status_widget,xpWidgetClass_Caption);
		// cycle over the items
		fms_fp_item_t* prev_item = new fms_fp_item_t();
		if (a_fms_status_page < 0) a_fms_status_page = 0;
		int current_entry_index = a_fms_ref->flying_index();
		for (int i = items_per_page*a_fms_status_page; i < (items_per_page*a_fms_status_page) + items_per_page; i++) {
			x1 = x1_orig;
			x2 = x1+box_l/2;
			y1 -= h;
			y2 = y1-box_h;
			if (i > a_fms_ref->a_fp_ref->a_arrival_index) break; // reached the end of the fp
			fms_fp_item_t * item = a_fms_ref->a_fp_ref->a_fp[i];
			if (prev_item->a_flight_status == "climb" && item->a_flight_status == "cruise") { // TOC
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"Top of Climb",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
                snprintf(temp, 2048,"%.0fnm",a_fms_ref->a_fms_config_ref->a_fms_config["climb_distance"]);
				XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l+30; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fms_config_ref->a_fms_config["climb_time"]).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l+30; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_toc->a_time_scheduled).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);			
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);			
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x1_orig;
				x2 = x1+box_l/2;
				y1 -= h;
				y2 = y1-box_h;
			}
			if (prev_item->a_flight_status == "cruise" && item->a_flight_status == "desc") { //TOD
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"Top of Desc",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l;
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l+30; 
                snprintf(temp, 2048,"%.0fnm",a_fms_ref->a_fms_config_ref->a_fms_config["desc_distance"]);
				XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l+30; 
				XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fms_config_ref->a_fms_config["desc_time"]).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(a_fms_ref->a_fp_ref->a_tod->a_time_scheduled).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);			
				x1 = x2;  x2 = x1 +box_l; 
				XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
				x1 = x1_orig;
				x2 = x1+box_l/2;
				y1 -= h;
				y2 = y1-box_h;
			}
			if (item->a_index == current_entry_index) XPCreateWidget(x1,y1,x2,y2,1,"->",0,fms_status_widget,xpWidgetClass_Caption);
			else XPCreateWidget(x1,y1,x2,y2,1,"",0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,item->a_name.c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,item->a_flight_status.c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_prev);
			XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_departure);
			XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l+30; 
            snprintf(temp, 2048,"%.0fnm",item->a_distance_from_arrival);
			XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_prev).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_departure).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l+30; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_from_arrival).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_actual).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_scheduled).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->delay2string(item->a_delay_from_scheduled).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->minutes2time(item->a_time_expected).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->delay2string(item->a_delay_from_expected).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->altitude2string(item->a_altitude_expected).c_str(),0,fms_status_widget,xpWidgetClass_Caption);			
			x1 = x2;  x2 = x1 +box_l; 
			XPCreateWidget(x1,y1,x2,y2,1,a_fms_utils_ref->altitude2string(item->a_altitude_actual).c_str(),0,fms_status_widget,xpWidgetClass_Caption);
			prev_item = item;
		}
	} else { 
        snprintf(temp, 2048, "FMS NOT READY. Status:%.8s",a_fms_ref->a_fp_ref->status2string(a_fms_ref->a_fp_ref->a_status).c_str());
		XPCreateWidget(x1,y1,x2,y2,1,temp,0,fms_status_widget,xpWidgetClass_Caption);
	} 
	// Add the callback
	XPAddWidgetCallback(fms_status_widget, fms_status_handler);
}

// handle events from the FMS status panel
int gui_t::fms_status_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
	if (inMessage == xpMessage_CloseButtonPushed){
			if(XPIsWidgetVisible(fms_status_widget)) XPHideWidget(fms_status_widget);
			return 1;
	}
	if (inMessage == xpMsg_PushButtonPressed){
		if (inParam1 == (intptr_t)fms_status_next){
			a_fms_status_page++;
			fms_status_create_window();
			return 1;
		}
		if (inParam1 == (intptr_t)fms_status_prev){
			a_fms_status_page--;
			fms_status_create_window();
			return 1;
		}
		if (inParam1 == (intptr_t)fms_status_refresh){
			fms_status_create_window();
			return 1;
		}
	}
	return 0;
}

// handle the creation of the MFD window
void	gui_t::mfd_window_handler(void) {
        if (XPIsWidgetVisible(mfd_widget)) XPHideWidget(mfd_widget);
        else {
            mfd_create_window(300, 600, 250, 200);
            XPShowWidget(mfd_widget);
        }
}
        
// build the MFD window
void gui_t::mfd_create_window(int x, int y, int w, int h){
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
    XPAddWidgetCallback(mfd_refresh, mfd_handler);
    // Add the callback
    XPAddWidgetCallback(mfd_widget, mfd_handler);
}

// handle events from the MFD panel
int gui_t::mfd_handler(XPWidgetMessage inMessage,XPWidgetID inWidget,long inParam1,long inParam2) {
    if (inMessage == xpMessage_CloseButtonPushed){
            if(XPIsWidgetVisible(mfd_widget)) XPHideWidget(mfd_widget);
            return 1;
    }
    if (inMessage == xpMsg_PushButtonPressed){
        if (inParam1 == (intptr_t)mfd_refresh){
            mfd_create_window(300, 600, 250, 200);
            return 1;
        }
    }
    return 0;
}
