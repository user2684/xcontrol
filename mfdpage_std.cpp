#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <math.h>
#include "include/mfdpage_std.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"


using std::map;
using std::string;

mfdpage_std_t::mfdpage_std_t(string name)
{
	a_name = name;
}

string mfdpage_std_t::get_template(string name,std::map<int, std::string> &sources)
{
	int c= 0;
	    if (name == "std.radio.com")
    {
        sources[0] = "sim/cockpit/radios/com1_freq_hz";
        sources[1] = "sim/cockpit/radios/com2_freq_hz";
        sources[2] = "sim/cockpit/radios/com1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/com2_stdby_freq_hz";
        return "Com1      Com2\n%%06.2f Ac %%06.2f\n%%06.2f Sb %%06.2f";
    }
    if (name == "std.radio.nav")
    {
        sources[0] = "sim/cockpit/radios/nav1_freq_hz";
        sources[1] = "sim/cockpit/radios/nav2_freq_hz";
        sources[2] = "sim/cockpit/radios/nav1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/nav2_stdby_freq_hz";
        return " Nav1      Nav2\n%%06.2f Ac %%06.2f\n%%06.2f Sb %%06.2f";
    }
    if (name == "std.radio.adf")
    {
        sources[0] = "sim/cockpit/radios/adf1_freq_hz";
        sources[1] = "sim/cockpit/radios/adf2_freq_hz";
        sources[2] = "sim/cockpit/radios/adf1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/adf2_stdby_freq_hz";
        return " Adf1      Adf2\n%%03d Hz Ac %%03d Hz\n%%03d Hz Sb %%03d Hz";
    }
    if (name == "std.navigation")
    {
        sources[0] = "sim/flightmodel/position/psi";
        sources[1] = "sim/flightmodel/position/indicated_airspeed2";
        sources[2] = "sim/flightmodel/misc/h_ind";
        sources[3] = "sim/flightmodel/position/vh_ind_fpm2";
        return "hdg:   %%03d\nspd:  %%03d\nfl :   %%03d %%03d";
    }
		if (name == "welcome")
    {
		sources[c++] = "sim/time/zulu_time_sec";
		return "\n    WELCOME     \n Press any key \n    to start    ";
    }
	return "";
}

void mfdpage_std_t::do_refresh(object_t* source) {
}

int mfdpage_std_t::refresh_interval(string name) {
	if (name == "welcome") return 1000;
	if (name == "std.navigation") return 2;
    return 10;
}

string mfdpage_std_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
    char temp[2048] = {};
	 size_t p = std::string::npos;
    string t2 = a_template.c_str();
	int i;
    float f;
    int count;
    count=0;

    for (map<int, object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        count+=1;

		
        if ((p = t2.find('%')) != std::string::npos)
        {
            t2.erase(p, 1);
        }
        memset(temp, 0, 2048);
		
		if (a_name == "std.navigation") {
            if (count <=2) {
                i = (int)*a_datasources[(*it).first];
            } else {
                i = (int)*a_datasources[(*it).first]/100;
            }
            _snprintf(temp, 2048, t2.c_str(), i);
        } else if (a_name == "std.radio.adf") {
            i = (int)*a_datasources[(*it).first];
            _snprintf(temp, 2048, t2.c_str(), i);
        } else {
            //now "std.radio.com" or "std.radio.nav"
            f = (int)*a_datasources[(*it).first]/100.0f;
            _snprintf(temp, 2048, t2.c_str(), f);
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
