#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include <math.h>

#include "include/out.h"
#include "include/data.h"
#include "include/fms.h"
#include "include/in.h"
#include "include/config.h"
#include "include/mfdpage.h"
#include "include/mfdpage_fms.h"
#include "include/mfdpage_flight.h"
#include "include/mfdpage_weather.h"
#include "include/mfdpage_std.h"
#include "include/mfdpage_time.h"
#include "include/mfdpage_led.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"

using std::map;
using std::string;

// mfdpages
mfdpages_t::mfdpages_t(out_t* out,in_t* in,data_t* dataconnection,fms_t* fms_ref)
{
    a_in = in;
    a_out = out;
	a_dataconnection = dataconnection;
	a_fms_ref = fms_ref;
}

mfdpages_t::~mfdpages_t(void)
{
    debug_out(debug, "mfdpages: deleting all the mfd pages");
    a_in->delete_all_pages();
}

void mfdpages_t::load(void)
{
    a_in->delete_all_pages(); // delete all the existing pages
	config_t * a_config = config_t::getInstance();
     map<string,string> config=a_config->get_config();
	 map<string,int> buttons;
	 buttons["mfdpages_button1"] = 1; buttons["mfdpages_button2"] = 2;
	 buttons["mfdpages_button3"] = 3; buttons["mfdpages_button4"] = 4;
	 // Add any custom page
	for (map<string,int>::iterator it = buttons.begin(); it != buttons.end(); ++it) {
		string button_config = (*it).first;
		int button_num = (*it).second;
		string s;
		string pages = config[button_config];
		stringstream stream(pages);
		while( getline(stream, s, ',') ) {
            if (s.find("fms.") != std::string::npos) a_in->add_page(new mfdpage_t(s.c_str(), a_out, a_dataconnection,button_num,"fms",a_fms_ref));
            else if (s.find("flight.") != std::string::npos) a_in->add_page(new mfdpage_t(s.c_str(), a_out, a_dataconnection,button_num,"flight",NULL));
            else if (s.find("weather.") != std::string::npos) a_in->add_page(new mfdpage_t(s.c_str(), a_out, a_dataconnection,button_num,"weather",NULL));
            else if (s.find("std.") != std::string::npos) a_in->add_page(new mfdpage_t(s.c_str(), a_out, a_dataconnection,button_num,"std",NULL));
			else debug_out(warn,"mfdpages: unknown mfdpage class for %s",s.c_str());
		}
	}
	// Add the time page
    mfdpage_t* time_page = new mfdpage_t("time", a_out, a_dataconnection,0,"time",NULL);
    a_in->add_page(time_page);
	time_page->set_active(true);
	// Add the led (invisible) page
    mfdpage_t* led_page = new mfdpage_t("led", a_out, a_dataconnection,0,"led",NULL);
    a_in->add_page(led_page);
	led_page->set_active(true);
	// Add the FMS refresh (invisible) page
    mfdpage_t* fms_refresh_page = new mfdpage_t("fms.refresh", a_out, a_dataconnection,0,"fms",a_fms_ref);
    a_in->add_page(fms_refresh_page);
	fms_refresh_page->set_active(true);
	// Add the welcome page
    mfdpage_t* welcome_page = new mfdpage_t("welcome", a_out, a_dataconnection,0,"std",NULL);
    a_in->add_page(welcome_page);
	welcome_page->set_active(true);
}
////////////////////////////
// mfdpage
mfdpage_t::mfdpage_t(const char* name, out_t* outputdevice, data_t* dataconnection, int button,string type,fms_t* fms_ref)
        : a_visible(false), a_outdevice(outputdevice), a_dataconnection(dataconnection)
{
	a_button = button;
	a_type = type;
	a_fms_ref = fms_ref;
    if (!name || !strlen(name)) a_name.assign(" -unnamed page- ");
    else a_name.assign(name);
	a_dataconnection->add_listener(this);
    map<int, string> sources;
	if (type == "fms") a_page = new mfdpage_fms_t(a_name,a_fms_ref);
	if (type == "flight") a_page = new mfdpage_flight_t(a_name);
	if (type == "weather") a_page = new mfdpage_weather_t(a_name);
	if (type == "std") a_page = new mfdpage_std_t(a_name);
	if (type == "time") a_page = new mfdpage_time_t(a_name);
	if (type == "led") a_page = new mfdpage_led_t(a_name,outputdevice);
	a_template = a_page->get_template(name,sources);
	set_datasources(&sources);
	debug_out(info,"mfdpage: created page %s for button %d, class %s, refresh %d seconds",a_name.c_str(),button,type.c_str(),a_page->refresh_interval(name));
	last_page_update = 0;
}

mfdpage_t::~mfdpage_t(void)
{
	remove_datasources();
    a_dataconnection->remove_listener(this);
    debug_out(debug, "mfdpage: deleted mfdpage {%s} (self: %p)", a_name.c_str(), this);
}

const std::string& mfdpage_t::name(void)
{
    return a_name;
}

void mfdpage_t::set_active(bool active)
{
    a_visible = active;
    if (active) {
        for (map<int, object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
        {
            (*it).second->mark_dirty();
        }
     } else { // reset the timestamp for last update so to refresh when cycled back to the page
        last_page_update = 0;
    }
}

void mfdpage_t::set_datasources(std::map<int, std::string>* sources)
{
    if (!sources) return;
    a_datasources.clear();
    for (map<int, string>::iterator it = sources->begin(); it != sources->end(); ++it)
    {
        object_t* src;
        src = a_dataconnection->add_datasource((*it).second.c_str());
        if (src) a_datasources[(*it).first] = src;
    }
}

void mfdpage_t::remove_datasources(void)
{
    for (map<int, object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        a_dataconnection->remove_datasource((*it).second);
    }
	a_datasources.clear();
}

bool mfdpage_t::has_object(object_t* source)
{
    for (map<int, object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it).second->name().c_str() == source->name()) {
            return true;
		}
    }
    return false;
}

bool mfdpage_t::refresh(object_t* source)
{
    debug_out(all,"mfdpage: page %s (%d) received the updated '%s': %s",a_name.c_str(),a_visible,source->name().c_str(),(const char*)(*source));
	a_page->do_refresh(source);
	if (a_type == "time" || a_type == "led") return true; // time is always visible
    if (!a_visible) return false;
    return true;
}

void mfdpage_t::final_refresh(void)
{
	time_t now = time(NULL);
    debug_out(all, "mfdpage: requested joystick refresh for %s", a_name.c_str());
	if (now < (last_page_update + a_page->refresh_interval(a_name)) ) return; // not the time to update
    last_page_update=now;
    debug_out(debug, "mfdpage: refreshing joystick display (%s)", a_name.c_str());
    a_data = a_page->refresh_template(a_name,a_datasources,a_template);
	if (a_type == "time") {
            debug_out(verbose, "mfdpage: updating time %02d:%02d - %d/%d/%d", a_page->a_time["hour"],a_page->a_time["minute"],a_page->a_date["year"], a_page->a_date["month"], a_page->a_date["day"]);
            a_outdevice->a_joystick->set_time(true, a_page->a_time["hour"],a_page->a_time["minute"]);
            a_outdevice->a_joystick->set_date(a_page->a_date["year"], a_page->a_date["month"], a_page->a_date["day"]);
	} else { // print something on the MFD
        a_outdevice->a_joystick->set_textdata(a_data.c_str());
        a_outdevice->a_joystick->print();
	}	
}
