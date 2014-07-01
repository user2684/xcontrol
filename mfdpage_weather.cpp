#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <math.h>

#include "include/mfdpage_weather.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"


using std::map;
using std::string;

mfdpage_weather_t::mfdpage_weather_t(string name)
{
	a_name = name;
}

string mfdpage_weather_t::get_template(string name,std::map<int, std::string> &sources)
{
	int c= 0;
	if (name == "weather.wind")
    {
		c=0;	    
		sources[c++] = "sim/weather/wind_altitude_msl_m[0]";
		sources[c++] = "sim/weather/wind_direction_degt[0]";
        sources[c++] = "sim/weather/wind_speed_kt[0]";
		sources[c++] = "sim/weather/turbulence[0]";

		sources[c++] = "sim/weather/wind_altitude_msl_m[1]";
		sources[c++] = "sim/weather/wind_direction_degt[1]";
        sources[c++] = "sim/weather/wind_speed_kt[1]";
		sources[c++] = "sim/weather/turbulence[1]";

		sources[c++] = "sim/weather/wind_altitude_msl_m[2]";
		sources[c++] = "sim/weather/wind_direction_degt[2]";
        sources[c++] = "sim/weather/wind_speed_kt[2]";
		sources[c++] = "sim/weather/turbulence[2]";

		return "W:F%%03.0f %%03.0f/%%02.0f:%%01.0fT\nW:F%%03.0f %%03.0f/%%02.0f:%%01.0fT\nW:F%%03.0f %%03.0f/%%02.0f:%%01.0fT";
        // W:F<fl1> <wind_dir>/<wind_speed>:<turbulence>T
		// W:F<fl2> <wind_dir>/<wind_speed>:<turbulence>T
		// W:F<fl3> <wind_dir>/<wind_speed>:<turbulence>T
    }
		if (name == "weather.cloud")
    {
		c=0;
	    sources[c++] = "sim/weather/cloud_base_msl_m[0]";
		sources[c++] = "sim/weather/cloud_tops_msl_m[0]";
		sources[c++] = "sim/weather/cloud_type[0]";
				
	    sources[c++] = "sim/weather/cloud_base_msl_m[1]";
		sources[c++] = "sim/weather/cloud_tops_msl_m[1]";
		sources[c++] = "sim/weather/cloud_type[1]";
		
		sources[c++] = "sim/weather/cloud_base_msl_m[2]";
		sources[c++] = "sim/weather/cloud_tops_msl_m[2]";   
        sources[c++] = "sim/weather/cloud_type[2]";
		
		return "C:F%%03.0f-%%03.0f %%5s\nC:F%%03.0f-%%03.0f %%5s\nC:F%%03.0f-%%03.0f %%5s";
		// C:F<fl1_base>-<fl1_top> <clouds>
		// C:F<fl2_base>-<fl2_top> <clouds>
		// C:F<fl3_base>-<fl3_top> <clouds>
    }
			if (name == "weather.condition")
    {
		c=0;
		sources[c++] = "sim/weather/temperature_sealevel_c";
		sources[c++] = "sim/weather/temperature_le_c";
		sources[c++] = "sim/weather/temperature_ambient_c";

		sources[c++] = "sim/weather/wind_direction_degt[0]";
        sources[c++] = "sim/weather/wind_speed_kt[0]";
		sources[c++] = "sim/cockpit2/gauges/indicators/wind_heading_deg_mag";
		sources[c++] = "sim/cockpit2/gauges/indicators/wind_speed_kts";
		
		sources[c++] = "sim/weather/visibility_reported_m";
		sources[c++] = "sim/weather/rain_percent";
		sources[c++] = "sim/weather/runway_friction";

		return "T:%%3.0fC %%3.0fC %%3.0fC\nW:%%03.0f/%%02.0f  %%03.0f/%%02.0f\nV:%%02.0fk R:%%02.0f RWY:%%1s";
		// T:<ground_temp>C <wing_temp>C <outside_temp>C
		// W:<ground_wind_dir>/<ground_wind_speed> <outside_wind_dir>/<outside_wind_speed>
    }
	return "";
}

void mfdpage_weather_t::do_refresh(object_t* source) {
}

int mfdpage_weather_t::refresh_interval(string name) {
    return 10;
}

string mfdpage_weather_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
    char temp[2048] = {};
	 size_t p = std::string::npos;
    string t2 = a_template.c_str();
     //int i;
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
				if (std::string::npos != refName.find("sim/weather/cloud_type")){
					if (f == 0) cloud = "CAVOK";
					if (f == 1) cloud = " FEW ";
					if (f == 2) cloud = " SCT ";
					if (f == 3) cloud = " BKN ";
					if (f == 4) cloud = " OVC ";
					if (f == 5) cloud = " STR ";
                    _snprintf(temp, 2048, t2.c_str(),cloud.c_str());
					break;
				}
                _snprintf(temp, 2048, t2.c_str(),f);
				break;
			case xplmType_Float:
				f = (float)*a_datasources[(*it).first];
				if (refName == "sim/weather/visibility_reported_m")  { f = f/1000;  }		
				if (refName == "sim/weather/runway_friction"){
					if (f == 0) runaway = "0";
					if (f == 1) runaway = "M";
					if (f == 2) runaway = "H";
                    _snprintf(temp, 2048, t2.c_str(),runaway.c_str());
					break;
				}	
				if (refName == "sim/weather/rain_percent"){ f = f*10;}	
				if (std::string::npos != refName.find("sim/weather/cloud_base_msl_m") || std::string::npos != refName.find("sim/weather/cloud_tops_msl_m") || std::string::npos != refName.find("sim/weather/wind_altitude_msl_m")){
					f = f*3.2808399/100;
				}
                _snprintf(temp, 2048, t2.c_str(),f);
				break;
			case xplmType_Double:
				d = (double)*a_datasources[(*it).first];
                _snprintf(temp, 2048, t2.c_str(),d);
				break;
			case xplmType_Data:
                _snprintf(temp, 2048, t2.c_str(),(const char*)*(a_datasources[(*it).first]));
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
