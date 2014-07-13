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
#include "include/gui_mfd.h"
#include "include/gui_fms_option.h"
#include "include/gui_fms_status.h"

using namespace std;
using std::map;
using std::string;

static XPLMMenuID myMenu;
static int mySubMenuItem;
static int a_menu_created;
static int a_fms_status_page;
static fms_t* a_fms_ref;
static config_t* a_config_ref;
static mfdpages_t* a_mfdpages_ref;
static out_t* a_out_ref;
static gui_mfd_t* gui_mfd_ref;
static gui_fms_option_t* gui_fms_option_ref;
static gui_fms_status_t* gui_fms_status_ref;

gui_t::gui_t(void)
{
}

gui_t::gui_t(mfdpages_t* mfdpages,fms_t* fms_ref,out_t* out_ref){
	a_fms_ref = fms_ref;
	a_mfdpages_ref = mfdpages;
    a_out_ref = out_ref;
	a_config_ref = config_t::getInstance();
	a_menu_created = 0;
    gui_mfd_ref = new gui_mfd_t(a_out_ref);
    gui_fms_option_ref = new gui_fms_option_t(a_config_ref,a_fms_ref);
    gui_fms_status_ref = new gui_fms_status_t(a_config_ref,a_fms_ref);
}

gui_t::~gui_t(void) {
	disable();
}

// enagle the UI
void gui_t::enable(void) {
	// Create a Custom Menu within the Plugin Menu
    if (a_menu_created == 0) mySubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "X-Control Enhanced", 0, 1);
    myMenu = XPLMCreateMenu("X-Control Enhanced", XPLMFindPluginsMenu(), mySubMenuItem, menu_handler, 0);
    a_fms_status_page = 0;
    XPLMAppendMenuItem(myMenu,"FMS Settings",(void*)1,1);
    XPLMAppendMenuSeparator(myMenu);
    XPLMAppendMenuItem(myMenu,"Show FMS Status (CTRL+Shift+B)",(void*)2,1);
    XPLMAppendMenuItem(myMenu,"Show Virtual MFD (CTRL+Shift+V)",(void*)3,1);
    XPLMAppendMenuSeparator(myMenu);
    XPLMAppendMenuItem(myMenu,"Reset the plugin",(void*)4,1);
	a_menu_created = 1;
}

// disable the UI
void gui_t::disable(void) {
    gui_fms_option_ref->disable();
    gui_fms_status_ref->disable();
    gui_mfd_ref->disable();
	XPLMDestroyMenu(myMenu);
}

// handle the plugin menu events
void gui_t::menu_handler(void *inMenuRef, void *inItemRef) {
	intptr_t menuItem = (intptr_t)inItemRef;
	if(menuItem == 1) { // customize the FMS
        gui_fms_option_ref->enable();
	} 	else if(menuItem == 2) { // Show FMS status
        gui_fms_status_ref->enable();
    } 	else if(menuItem == 3) { // Show the MFD
        gui_mfd_ref->enable();
    } else if(menuItem == 4) { // reset the plugin
		a_fms_ref->reset(); // reset the FMS
        a_mfdpages_ref->load(); // recreate the MFD pages
	} 	
}
