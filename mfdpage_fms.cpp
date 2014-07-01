#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

#include "include/mfdpage_fms.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"

using std::map;
using std::string;

mfdpage_fms_t::mfdpage_fms_t(string name)
{
}

mfdpage_fms_t::mfdpage_fms_t(string name,fms_t* fms)
{
	a_fms_ref = fms;
	a_name = name;
}

string mfdpage_fms_t::get_template(string name,std::map<int, std::string> &sources)
{
	int c = 0;
	    if (name == "fms.status")
    {
		sources[c++] = "sim/time/zulu_time_sec"; 
    }
		 if (name == "fms.planner")
    {
		sources[c++] = "sim/time/zulu_time_sec"; 
    }
		 if (name == "fms.scheduler")
    {
		sources[c++] = "sim/time/zulu_time_sec"; 
    }
		if (name == "fms.refresh")
    {
		sources[c++] = "sim/time/zulu_time_sec"; 
		sources[c++] = "sim/flightmodel/position/groundspeed"; 
		sources[c++] = "sim/flightmodel/misc/h_ind";
    }
	return "";
}

void mfdpage_fms_t::do_refresh(object_t* source) {
	if (a_name == "fms.refresh") {
		a_fms_ref->refresh(); // refresh the FMS status
		if (a_fms_ref->a_fp_ref->a_status > 0) { // if the FMS is in a valid status
			if (source->name() == "sim/time/zulu_time_sec") { 
				a_fms_ref->a_current_zulu_time = (float)*source; // just store zulu time
				a_fms_ref->check_position(); // check the current position and track each FMS entry
				a_fms_ref->check_gate_arrival(); // check if FF is zero to track arrival at gate
			} else if (source->name() == "sim/flightmodel/position/groundspeed") { 
				a_fms_ref->check_takeoff((float)*source);  // use GS to track takeoff
				a_fms_ref->check_landing((float)*source); // use GS to track landing
				a_fms_ref->check_gate_departure((float)*source); // use GS to track leaving departure gate
			} 	else if (source->name() == "sim/flightmodel/misc/h_ind") {
				a_fms_ref->a_current_altitude = (float)*source;  // just store altitude that will be used by check_position()
			}
		}
	}
}

int mfdpage_fms_t::refresh_interval(string name) {
    return 1;
}

string mfdpage_fms_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
    char temp[2048] = {};

	if (name == "fms.status") { 
		float zulu_time = (float)*a_datasources[0];	
		memset(temp, 0, 2048);
		if (a_fms_ref->a_fp_ref->a_status > 0) {
			int flying_index = a_fms_ref->flying_index();
			int displayed_index = a_fms_ref->displayed_index(); // get the displayed entry ID
			if (displayed_index >= flying_index) { // displaying an upcoming entry
				// <arrival_apt> <remaining_distance> <fuel_eta>
				// <hh_eta>:<mm_eta> <id_name>
				// <distance>/<remaining>m <hh_time>:<mm_time>
                _snprintf(temp, 2048, "%4s %4.0fm %.5s\n%.5s %.10s\n%3.0fm/%4.0fm %.5s",
					a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_name.c_str(),
					a_fms_ref->remaining_distance(),
					a_fms_ref->a_fuel_ref->remaining().c_str(),
					a_fms_ref->eta_to_entry(displayed_index,zulu_time).c_str(),
					a_fms_ref->entry_name(displayed_index).c_str(),
					a_fms_ref->distance_to_entry(displayed_index),
					a_fms_ref->remaining_distance_from_entry(displayed_index),
					a_fms_ref->time_to_entry(displayed_index).c_str()
				);
			} else { // displaying a previous entry
				// <id_name> <time_actual>
				// <time_expected> (<delay_from_scheduled>/<delay_from_expected>)
				// <altitude_expected> (<altitude_actual>)
                _snprintf(temp, 2048, "%5s %5s\n%5s (%3s/%3s)\n%5s (%5s)",
					a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_name.c_str(),
					minutes2time(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_time_actual).c_str(),
					minutes2time(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_time_scheduled).c_str(),
					delay2string(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_delay_from_scheduled).c_str(),
					delay2string(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_delay_from_expected).c_str(),
					altitude2string(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_altitude_expected).c_str(),
					altitude2string(a_fms_ref->a_fp_ref->a_fp[displayed_index]->a_altitude_actual).c_str()
				);
			}
		} 	else
            _snprintf(temp, 2048, "     STATUS     \nFMS NOT READY\nStatus:%.8s",a_fms_ref->a_fp_ref->status2string(a_fms_ref->a_fp_ref->a_status).c_str());
	}
	else if (name == "fms.planner") { 
		memset(temp, 0, 2048);
		if (a_fms_ref->a_fp_ref->a_status > 0) { 
			// <departure_apt>-><arrival_apt> <total_distance>
			// D:<duration> F:<fuel>
			// <distance>/<remaining>m <hh_time>:<mm_time>
            _snprintf(temp, 2048, "%4s->%4s %4.0fm\nD:%.5s F:%6.0f\nTD:%.13s",
			a_fms_ref->a_fp_ref->a_fp[0]->a_name.c_str(),
			a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_name.c_str(),
			a_fms_ref->a_fp_ref->a_total_distance,
			minutes2time(a_fms_ref->a_fp_ref->a_total_time).c_str(),
			a_fms_ref->a_fuel_ref->a_lb_to_load,
			a_fms_ref->a_fp_ref->tod_string.c_str());		
		} else 
            _snprintf(temp, 2048, "    PLANNER     \nFMS NOT READY\nStatus:%.8s",a_fms_ref->a_fp_ref->status2string(a_fms_ref->a_fp_ref->a_status).c_str());
	}
	else if (name == "fms.scheduler") { // FMS page is handled in a different way than the others
		memset(temp, 0, 2048);
		if (a_fms_ref->a_fp_ref->a_status > 0) {
			// T:<expected_takeoff_time> (<actual_takeoff_time>)
			// C:<expected_top_of_climb_time> D:<expected_top_of_descend_time>
			// L:<expected_landing_time> (<actual_landing_time>)
            _snprintf(temp, 2048, "T:%.5s (%.5s)\nC:%.5s D:%.5s\nL:%.5s (%.5s)",
			minutes2time(a_fms_ref->a_fms_config_ref->a_fms_config["scheduled_takeoff"]).c_str(),
			minutes2time(a_fms_ref->a_fp_ref->a_fp[0]->a_time_actual).c_str(),
			minutes2time(a_fms_ref->a_fp_ref->a_toc->a_time_scheduled).c_str(),
			minutes2time(a_fms_ref->a_fp_ref->a_tod->a_time_scheduled).c_str(),
			minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_scheduled).c_str(),
			minutes2time(a_fms_ref->a_fp_ref->a_fp[a_fms_ref->a_fp_ref->a_arrival_index]->a_time_actual).c_str());
		} else
            _snprintf(temp, 2048, "   SCHEDULER    \nFMS NOT READY\nStatus:%.8s",a_fms_ref->a_fp_ref->status2string(a_fms_ref->a_fp_ref->a_status).c_str());
	}
	string temp_string;
	temp_string.assign(temp);
	return temp_string;
}
