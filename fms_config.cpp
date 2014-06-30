#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "tools.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fms_config.h>

using namespace std;
using std::map;
using std::string;

fms_config_item_t::fms_config_item_t(void)
{
}

fms_config_t::fms_config_t(void)
{
	config_t * a_config = config_t::getInstance();
	a_config->read_config_fms();
}

fms_config_t::~fms_config_t(void)
{
	a_fms_config.clear(); 
}

// load the fms profiles
void fms_config_t::load(void) {
	a_fms_config.clear(); // clear the fms configuration
	config_t * a_config = config_t::getInstance();
	a_config=a_config->get_config_fms();
	load_config();
	load_profiles();
	calculate_climb_desc();
}

// load the fms configuration from file
void fms_config_t::load_config(void) {
	a_fms_config["cruise_gs"] = ::atof(a_config["cruise_gs"].c_str());
	a_fms_config["cruise_fl"] = ::atof(a_config["cruise_fl"].c_str());
	a_fms_config["fuel_ground"] = ::atof(a_config["fuel_ground"].c_str());
	a_fms_config["fuel_clb_dsc"] = ::atof(a_config["fuel_clb_dsc"].c_str());
	a_fms_config["fuel_every_100nm"] = ::atof(a_config["fuel_every_100nm"].c_str());
	a_fms_config["fuel_contingency"] = ::atof(a_config["fuel_contingency"].c_str());
	a_fms_config["voice"] = ::atof(a_config["voice"].c_str());
	a_fms_config["scheduled_takeoff"] = time2minutes(a_config["scheduled_takeoff"]);
	a_fms_config["total_distance"] = ::atof(a_config["total_distance"].c_str());
	// set default values if no configuration is provided
	debug_out(debug, "fms_config: requested cruise FL: %f, requested cruise GS: %f, scheduled takeoff time: %s ",a_fms_config["cruise_fl"],a_fms_config["cruise_gs"],minutes2time(a_fms_config["scheduled_takeoff"]).c_str());
}

// read the FMS configuration and parse it
void fms_config_t::load_profiles(void) {
	string s;
	a_profiles.clear();
	load_profile("climb_fl_time");
	load_profile("climb_fl_distance");
	load_profile("desc_fl_time");
	load_profile("desc_fl_distance");
	reverse_profile("climb_fl_distance","climb_distance_fl");
	reverse_profile("desc_fl_distance","desc_distance_fl");
	combine_profile("climb_fl_distance","climb_fl_time","climb_distance_time");
	combine_profile("desc_fl_distance","desc_fl_time","desc_distance_time");
}

// load a profile (e.g. mapping between FL and time) into the fiven profile_to_load
void fms_config_t::load_profile(string profile_to_load) {
	debug_out(verbose, "fms_config: loading profile %s",profile_to_load.c_str());
	string profile_string = a_config[profile_to_load];
	debug_out(verbose, "fms_config: parsing profile string %s",profile_string.c_str());
    stringstream stream(profile_string);
	int key=0; string s;
	a_profiles[profile_to_load] = new fms_config_item_t();
    while( getline(stream, s, ',') )  {
		a_profiles[profile_to_load]->a_profile[key] = ::atof(s.c_str());
		debug_out(verbose, "fms_config: %s : loaded item %i with value %f",profile_to_load.c_str(),key,a_profiles[profile_to_load]->a_profile[key]);
		key += 10;
	}
}

// generate a profile named profile_to_save reversing the key->value pairs for the given profile
void fms_config_t::reverse_profile(string profile_to_reverse,string profile_to_save) {
	debug_out(verbose, "fms_config: reversing profile %s into %s",profile_to_reverse.c_str(),profile_to_save.c_str());
	a_profiles[profile_to_save] = new fms_config_item_t();
	for (map<int, float>::iterator it = a_profiles[profile_to_reverse]->a_profile.begin(); it != a_profiles[profile_to_reverse]->a_profile.end(); ++it) {
		float key = (float)(*it).first;
		int value = (int)(*it).second;
		a_profiles[profile_to_save]->a_profile[value] = key;
		debug_out(verbose, "fms_config: %s : loaded item %i with value %f",profile_to_save.c_str(),value,key);
	}
}

// generate a profile named profile_to_save combining the values of the two given profiles with the same keys
void fms_config_t::combine_profile(string profile_key,string profile_value,string profile_to_save) {
	debug_out(verbose, "fms_config: combining profiles using keys from %s and values from %s into %s",profile_key.c_str(),profile_value.c_str(),profile_to_save.c_str());
	a_profiles[profile_to_save] = new fms_config_item_t();
	for (map<int, float>::iterator it = a_profiles[profile_key]->a_profile.begin(); it != a_profiles[profile_key]->a_profile.end(); ++it) {
		float key = (float)(*it).first; // e.g. FL
		int value = (int)(*it).second; //e.g. distance
		a_profiles[profile_to_save]->a_profile[value] = a_profiles[profile_value]->a_profile[key];
		debug_out(verbose, "fms_config: %s : loaded item %i with value %f (%f)",profile_to_save.c_str(),value,a_profiles[profile_value]->a_profile[key],key);
	}
}

// calculate a FMS profile
void fms_config_t::calculate_climb_desc(void) {
	int req_fl = a_fms_config["cruise_fl"]/10;
	a_fms_config["climb_time"] = value_from_profile("climb_fl_time",req_fl);
	a_fms_config["climb_distance"] = value_from_profile("climb_fl_distance",req_fl);
	a_fms_config["desc_time"] = value_from_profile("desc_fl_time",req_fl);
	a_fms_config["desc_distance"] = value_from_profile("desc_fl_distance",req_fl);
	debug_out(debug, "fms_config: estimated climb time: %f min, distance: %f nm",a_fms_config["climb_time"],a_fms_config["climb_distance"]);
	debug_out(debug, "fms_config: estimated desc time: %f min, distance: %f nm",a_fms_config["desc_time"],a_fms_config["desc_distance"]);
}

// returns the expected value based on a key->value map for the requested key
float fms_config_t::value_from_profile(string profile, int req_key) {
	float result =0;
	int prev_key = 0;
	float prev_value =0;
	debug_out(verbose, "fms_config: requesting value for key %i from profile %s",req_key,profile.c_str());
	if (req_key == 0) return 0;
	for (map<int,float>::iterator it = a_profiles[profile]->a_profile.begin(); it != a_profiles[profile]->a_profile.end(); ++it) { // cycle through the profile
		int key = (*it).first;
		float value = (*it).second;
		if (req_key > key) { //looking for the upperbound, if req_fl > fl, continue (e.g. FL370 > Fl100) but stores the current fl and value
			debug_out(verbose, "fms_config: skipping key %i, requested key %i",key,req_key);	
			prev_key = key;
			prev_value = value;
			continue;
		} else { // we identified the upper bound (e.g. FL370 < FL400)
			int factor = req_key - prev_key; // e.g. 7 for FL370
			result = (value - prev_value)*factor/(key - prev_key)+prev_value; // value for each FL001 * 7 + previous value
			debug_out(verbose, "fms_config: upperbound found: key %i, result: %f (factor: %i, value: %f, prev_key: %i, prev_value: %f)",key,result,factor,value,prev_key,prev_value);	
			break;
		}
	}
	debug_out(verbose, "fms_config: key: %i, value: %f for profile %s",req_key,result,profile.c_str());	
	return result;
}

