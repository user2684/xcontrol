#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

#include "include/mfdpage_led.h"
#include "include/config.h"
#include "include/out.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"

using std::map;
using std::string;

mfdpage_led_t::mfdpage_led_t(string name)
{
	a_name = name;
}

mfdpage_led_t::mfdpage_led_t(string name, out_t* outdevice)
{
	a_name = name;
	a_outdevice = outdevice;
}

string mfdpage_led_t::get_template(string name,std::map<int, std::string> &sources)
{
		int count = 0;
		config_t * a_config = config_t::getInstance();
		config=a_config->get_config();
		//add datarefs from config file
        for (map<string, string>::iterator it = config.begin(); it != config.end(); ++it) {
            debug_out(verbose, "mfdpage_led: configuration map: %s = '%s'", it->first.c_str(), it->second.c_str());
            int dataref=it->first.find("sim/",0);
            if (dataref >= 0 ) {
                //real dataref
                sources[count++]=it->first.c_str();
                debug_out(verbose, "mfdpage_led: added dataref: %s", it->first.c_str());
            } else {
                //predefined or settings
                debug_out(verbose, "mfdpage_led: setting or predefined: %s",it->first.c_str());
                if (it->first ==  "brightness_mfd") {
                    a_outdevice->a_joystick->set_display_brightness(atoi(it->second.c_str()));
                } else if (it->first ==  "brightness_led") {
                    a_outdevice->a_joystick->set_led_brightness(atoi(it->second.c_str()));
                } else if (it->first ==  "debug") {
                    led_debug=atoi(it->first.c_str());
                } else if (it->first ==  "gear") {
                     sources[count++]="sim/cockpit/switches/gear_handle_status";
                     sources[count++]="sim/cockpit/warnings/annunciators/gear_unsafe";
                } else if (it->first == "autopilot") {
                     sources[count++]="sim/cockpit/autopilot/autopilot_mode";
                     sources[count++]="sim/cockpit/warnings/annunciators/autopilot_disconnect";
                } else if (it->first ==  "fire") {
                     sources[count++]="sim/cockpit/weapons/guns_armed";
                     sources[count++]="sim/cockpit/weapons/rockets_armed";
                     sources[count++]="sim/cockpit/weapons/missiles_armed";
                     sources[count++]="sim/cockpit/weapons/bombs_armed";
                } else if (it->first ==  "mfdpages_button1"|| it->first ==  "mfdpages_button2" || it->first ==  "mfdpages_button3" || it->first ==  "mfdpages_button4") {
                } else {
                    debug_out(err,"mfdpage_led: unknown setting or predefined configuration item: %s", it->first.c_str());
                }
            }
        }
		return "";
}

void mfdpage_led_t::do_refresh(object_t* source) {    
	string action;
	if (source->name() == "sim/cockpit/switches/gear_handle_status" ) {
        //keep state
        a_gear = (int) *source;
        return;
    } else if (source->name() == "sim/cockpit/warnings/annunciators/gear_unsafe" ) {
        //go to orange if unsafe, go to gear handle state otherwise
        //do we have a predefined action for gear?
        try {
            action = config["gear"];
        } catch (const char* reason) {
            return;
        }
        int i = *source;
        if (i != 0) {
            //gear not safe turn orange
            action+=",orange";
            set_led_by_name(led_debug,source->name().c_str(),0,action.c_str());
        } else {
            //gear now safe
            if (a_gear == 0) {
                //retracted
                action+=",off";
                set_led_by_name(led_debug,source->name().c_str(),0,action.c_str());
            } else {
                //extended
                action+=",green";
                set_led_by_name(led_debug,source->name().c_str(),0,action.c_str());
            }
        }
        return ;
    } else if (source->name() == "sim/cockpit/autopilot/autopilot_mode") {
        int i = (int) *source;
        try {
            action = config["autopilot"];
        } catch (const char* reason) {
            return ;
        }
        a_autopilot_engaged=i;
        action+=",off,orange,green";
        set_led_by_name(led_debug,source->name().c_str(),i,action.c_str());
        return ;
    } else if (source->name() == "sim/cockpit/warnings/annunciators/autopilot_disconnect") {
        int i = (int) *source;
        try {
            action = config["autopilot"];
        } catch (const char* reason) {
            return ;
        }
        if (i == 1 ) {
            action+=",red";
            set_led_by_name(led_debug,source->name().c_str(),0,action.c_str());
        } else {
            action+=",off,orange,green";
            set_led_by_name(led_debug,source->name().c_str(),a_autopilot_engaged,action.c_str());
        }
        return ;
    } else if (source->name() == "sim/cockpit/weapons/guns_armed" ||
            source->name() == "sim/cockpit/weapons/rockets_armed" ||
            source->name() == "sim/cockpit/weapons/missiles_armed" ||
            source->name() == "sim/cockpit/weapons/bombs_armed") {
        int i = (int) *source;
        action = config["fire"];
        action+=",off,on";
        set_led_by_name(led_debug,source->name().c_str(),i,action.c_str());
    } else {
        //unhandled or custom action from config file
        try {
            action = config[source->name()];
            int i = (int) *source;
            set_led_by_name(led_debug,source->name().c_str(),i,action.c_str());
        } catch (const char* e) {
            //unhandled data refs
            //use this to check on registered datarefs that have changed
            float f = *source;
            debug_out(warn,"mfdpage_led: unhandled data ref: %s: %6.2f", source->name().c_str(),f);
        }
        /*
        float f = *source;
        debug_out(err,"unhandled data ref: %s: %6.2f", source->name().c_str(),f);
        */
        return ;
    }
}

int mfdpage_led_t::refresh_interval(string name) {
    return 2;
}

string mfdpage_led_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
    return "";
}

void mfdpage_led_t::set_led_by_name(int led_debug, const char* dataref, int data, const char* action, ...) {
    string str=action;
    std::transform(str.begin(),str.end(),str.begin(), ::tolower);
    int pos=str.find(",",0);
    if ( pos <= 0) {
        debug_out(err,"mfdpage_led:unknown or empty action: %s = %d -> %s",dataref,data,action);
        return;
    }
    string led = str.substr(0,pos);
    string colors = str.substr(pos+1);

    if (led_debug)
        debug_out(warn,"mfdpage_led:%s: turn led %s with colors: %s and value %d",dataref,led.c_str(),colors.c_str(),data);

    map<int, string> ord_colors;
    int c=0;
    pos=colors.find(",",0);
    while ( pos > 0 ) {
        ord_colors[c++]=colors.substr(0,pos);
        colors=colors.substr(pos+1);
        pos=colors.find(",",0);
    }

    string color;
    ord_colors[c++]=colors.substr(0,pos);
    if ( data > (int) ord_colors.size()-1 ){
     debug_out(warn,"mfdpage_led:no color defined for %s = %d, using last defined color: %s",dataref,data,action);
        color=ord_colors[ord_colors.size()-1 ];
    } else {
        try {
            color=ord_colors[data];
        } catch (const char* reason) {
            debug_out(err,"mfdpage_led:no color defined for value %d",led.c_str(),colors.c_str(),data);
            return;
        }
    }

    int led_base_number=0;
    if (led=="a") led_base_number=2;
    else if (led=="b") led_base_number=4;
    else if (led=="d") led_base_number=6;
    else if (led=="e") led_base_number=8;
    else if (led=="t1" || led == "t2") led_base_number=10;
    else if (led=="t3" || led == "t4") led_base_number=12;
    else if (led=="t5" || led == "t6") led_base_number=14;
    else if (led=="i") led_base_number=18;
    else if (led=="1") led_base_number=0;
    else if (led=="hl") led_base_number=16;
    else if (led=="th") led_base_number=20;
    else {
        debug_out(err,"mfdpage_led:unknown led: %s",led.c_str());
        return;
    }
    if (color=="red") {
        a_outdevice->a_joystick->set_led(led_base_number,1);
        a_outdevice->a_joystick->set_led(led_base_number+1,0);
    } else if (color=="green") {
        a_outdevice->a_joystick->set_led(led_base_number,0);
        a_outdevice->a_joystick->set_led(led_base_number+1,1);
    } else if (color=="orange" || color=="on") {
        a_outdevice->a_joystick->set_led(led_base_number,1);
        a_outdevice->a_joystick->set_led(led_base_number+1,1);
    } else if (color=="off") {
        a_outdevice->a_joystick->set_led(led_base_number,0);
        a_outdevice->a_joystick->set_led(led_base_number+1,0);
    } else {
        debug_out(err,"mfdpage_led:no such color: %s",color.c_str());
    }
}
