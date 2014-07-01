#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <math.h>

#include "include/mfdpage_flight.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"

#if IBM
#define snprintf _snprintf
#endif

using std::map;
using std::string;

mfdpage_flight_t::mfdpage_flight_t(string name)
{
	a_name = name;
}

string mfdpage_flight_t::get_template(string name,std::map<int, std::string> &sources)
{
	int c=0;
		if (name == "flight.combined")
    {
        sources[c++] = "sim/flightmodel/position/indicated_airspeed2";
		sources[c++] = "sim/weather/speed_sound_ms";
		sources[c++] = "sim/cockpit/radios/transponder_mode";
		
		sources[c++] = "sim/flightmodel/misc/h_ind";
		sources[c++] = "sim/flightmodel/position/vh_ind_fpm2";
		sources[c++] = "sim/cockpit/switches/gear_handle_status";
		
		sources[c++] = "sim/cockpit2/radios/indicators/nav1_nav_id";
		sources[c++] = "sim/cockpit/radios/nav1_dme_dist_m";
		sources[c++] = "sim/flightmodel/controls/wing1l_fla1def";
		
		return "S:%%03.0f M:%%04.02f T:%%s\nFL%%03.0f V:%%3.0f00 %%2s\n%%3s %%5.1fnm F:%%02.0f";
		// S:<IAS> M:<match_speed> T:<trasponder>  
		// FL:<FL> V:<vs>00 <gear>
		//  <nav1_id> <nav1_dme>nm F:<flap>
    }
		if (name == "flight.high")
    {
		c=0;
        sources[c++] = "sim/flightmodel/position/indicated_airspeed2";
		sources[c++] = "sim/flightmodel/position/true_airspeed";
		sources[c++] = "sim/flightmodel/position/groundspeed";
		sources[c++] = "sim/weather/speed_sound_ms";
		
		sources[c++] = "sim/flightmodel/misc/h_ind";
		sources[c++] = "sim/weather/temperature_sealevel_c";
        sources[c++] = "sim/cockpit2/gauges/indicators/wind_speed_kts";
		
		sources[c++] = "sim/cockpit2/radios/indicators/gps_nav_id";
		sources[c++] = "sim/cockpit2/radios/indicators/gps_dme_distance_nm";
		sources[c++] = "sim/cockpit2/radios/indicators/gps_dme_time_min";
		
		return "%%03.0f %%03.0f/%%03.0f %%04.02f\nFL%%03.0f T:%%03.0f W:%%02.0f\n%%5s %%3.0fm %%3.0fm";
		// <IAS> <TAS>/<GS> <mach>
		// FL:<FL> T:<temp> W:<wind_speed>
		// <cur_nav_id> <nav_dme>nm <hh>:<mm>);
    }
		if (name == "flight.low")
    {
		c=0;
        sources[c++] = "sim/flightmodel/position/indicated_airspeed2";
		sources[c++] = "sim/weather/rain_percent";
		sources[c++] = "sim/weather/runway_friction";
		sources[c++] = "sim/weather/visibility_reported_m";
		sources[c++] = "sim/cockpit/radios/transponder_mode";
		
		sources[c++] = "sim/flightmodel/misc/h_ind";
		sources[c++] = "sim/flightmodel/position/vh_ind_fpm2";
		sources[c++] = "sim/flightmodel/controls/wing1l_fla1def";
		
		sources[c++] = "sim/cockpit2/radios/indicators/nav1_nav_id";
		sources[c++] = "sim/cockpit/radios/nav1_dme_dist_m";
		sources[c++] = "sim/weather/wind_direction_degt[0]";
        sources[c++] = "sim/weather/wind_speed_kt[0]";
		
		return "%%03.0f R:%%1.0f-%%s V:%%2.0f %%s\nFL%%03.0f V:%%03.0f F:%%02.0f\n%%3s %%4.1f  %%03.0f/%%02.0f";
        // <IAS> R:<rain_perc>-<runway_friction> V:<visibility> <trasponder>
		// FL:<FL> V:<vs> F:<flap>
		// <cur_nav_id> <nav_dme> <wind_dir>/<wind_speed>
    }
	return "";
}

void mfdpage_flight_t::do_refresh(object_t* source) {
}

int mfdpage_flight_t::refresh_interval(string name) {
    return 2;
}

string mfdpage_flight_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
    char temp[2048] = {};
	 size_t p = std::string::npos;
    string t2 = a_template.c_str();
    float f;
	double d;
    int count;
	int refType;
	std::string transponder;
	std::string cloud;
	std::string rain;
	std::string runaway;
	std::string refName;
    count=0;

    for (map<int, object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        count+=1;
		refType = (*a_datasources[(*it).first]).getType();
		refName = (*a_datasources[(*it).first]).name();
		
        if ((p = t2.find('%')) != std::string::npos)
        {
            t2.erase(p, 1);
        }
        memset(temp, 0, 2048);
		
		  switch (refType)
		{
			case xplmType_Int:
				f = (int)*a_datasources[(*it).first];	
				if (refName == "sim/cockpit/radios/transponder_mode")  { 
					if (f == 0) transponder = "0";
					if (f == 1) transponder = "S";
					if (f == 2) transponder = "C";
					// check whether we are squawking ident right now
					//float ident = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/transponder_id"));
					//if (ident) transponder = "I";
                    snprintf(temp, 2048, t2.c_str(),transponder.c_str());
					break;
				}
				if (refName == "sim/cockpit/switches/gear_handle_status"){
					string gear;
					if ((int)f == 0) gear = "UP";
					if (f == 1) gear = "DW";
                    snprintf(temp, 2048, t2.c_str(),gear.c_str());
					break;
				}	
                snprintf(temp, 2048, t2.c_str(),f);
				break;
			case xplmType_Float:
				f = (float)*a_datasources[(*it).first];
				if (refName == "sim/weather/runway_friction"){
					if (f == 0) runaway = "0";
					if (f == 1) runaway = "M";
					if (f == 2) runaway = "H";
                    snprintf(temp, 2048, t2.c_str(),runaway.c_str());
					break;
				}	
				if (refName == "sim/weather/visibility_reported_m")  { f = f/1000;  }		
				if (refName == "sim/flightmodel/misc/h_ind")  { f = f/100;  }
				if (refName == "sim/flightmodel/position/vh_ind_fpm2")  { f = f/100;  }		
				if (refName == "sim/flightmodel/controls/flaprqst")  { f = f*100;  } 
				if (refName == "sim/flightmodel/controls/flaprat")  { f = f*100;	}		
				if (refName == "sim/flightmodel/position/indicated_airspeed2")  { if (f <0) f = 0;	}		
				if (refName == "sim/flightmodel/position/true_airspeed" || refName == "sim/flightmodel/position/groundspeed")  { a_gs = f; f = f*1.944;	}
				if (refName == "sim/weather/speed_sound_ms") { 
					float tas = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/true_airspeed"));
					if (tas == 0 ) f = 0;
					else f = fabs(tas / f);
				}	
                snprintf(temp, 2048, t2.c_str(),f);
				break;
			case xplmType_Double:
				d = (double)*a_datasources[(*it).first];
                snprintf(temp, 2048, t2.c_str(),d);
				break;
			case xplmType_Data:
                snprintf(temp, 2048, t2.c_str(),(const char*)*(a_datasources[(*it).first]));
				break;
		}
		
        t2 = temp;
        p = 0;
        while ((p = t2.find('%', p)) != std::string::npos)
        {
            t2.replace(p, 1, "%%");
            p += 2;
        }
	}	
	string temp_string;
	temp_string.assign(temp);
	return temp_string;
}
