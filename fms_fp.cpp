#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "include/tools.h"
#include "include/fms.h"
#include "include/SDK/XPLMNavigation.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"
#include "include/fms_fp.h"

#if IBM
#define snprintf _snprintf
#endif

using namespace std;
using std::map;
using std::string;

fms_fp_item_t::fms_fp_item_t(void)
{
	a_index  = 0;
	a_distance_from_prev = a_distance_from_next = 0;
	a_distance_from_departure =  a_distance_from_arrival = 0;
	a_time_from_prev = a_time_from_next = 0;
	a_time_from_departure = a_time_from_arrival = -1;
	a_time_scheduled = a_time_expected = a_time_actual = -1;
	a_altitude_expected = a_altitude_actual = 0;
	a_delay_from_scheduled = a_delay_from_expected = 0;
	a_time_at_gate = -1;
}

fms_fp_t::fms_fp_t(void)
{
}

fms_fp_t::fms_fp_t(fms_config_t* fms_config) {
	a_status = -4;
	a_fms_config_ref = fms_config;
	a_fp.clear();
	a_fp_cache.clear();
	a_toc = new fms_fp_item_t();
	a_tod = new fms_fp_item_t();
}

fms_fp_t::~fms_fp_t(void)
{
	a_fp.clear();
	a_fp_cache.clear();
}

// load the configured flight plan
void fms_fp_t::load(void)
{
	// FMS config integrity checks
	if (a_fms_config_ref->a_fms_config["cruise_gs"] == 0 || a_fms_config_ref->a_fms_config["cruise_fl"] == 0 ) {
		a_fms_config_ref->a_fms_config["missing_config"] = 1;
		debug_out(err,"fms_fp: FMS configuration is missing");
		return;
	}
	if (a_fms_config_ref->a_fms_config["cruise_fl"] > 600 ) {
		a_fms_config_ref->a_fms_config["wrong_config"] = 1;
		debug_out(err,"fms_fp: FMS configured FL is too high");
		return;
	}
	// load the FP
	string status = status2string(a_status);
	a_fp.clear();
	a_total_distance = 0;
	a_total_time = 0;
	a_count = num_entries();
	a_arrival_index = a_count-1;
	debug_out(debug,"fms_fp: loading the %d entries of the flight plan", a_count);
	fms_fp_item_t* prev_item = new fms_fp_item_t();
	// first cycle, set basic information and build the FP structure
	for (int index = 0; index < a_count; index++) {
		fms_fp_item_t* item = new fms_fp_item_t();
		load_entry(index,item); // Retrieve basic information for the current entry
		if (item->a_name == "") {// something went wrong, ignore the entry
			debug_out(warn,"fms_fp: Ignoring entry %d",index);
			continue; 
		}
		load_distance(prev_item,item); // calculate the distances between items and set a_total_distance
		a_fp[index] = item; // save the item into the flight plan structure
		prev_item = item;
	}
	// set the total distance to the global config class
	a_fms_config_ref->a_fms_config["total_distance"] = a_total_distance;
	// second cycle, we now have distance_from_departure
	for (map<int, fms_fp_item_t*>::iterator it = a_fp.begin(); it != a_fp.end(); ++it) {
		fms_fp_item_t* item = (*it).second;
		item->a_distance_from_arrival = a_total_distance - item->a_distance_from_departure;
		// the following need a_total_distance and a_distance_from_arrival defined
		load_flight_status(item); // define the expected flight status (climb/cruise/desc) at the item
		load_time(prev_item,item); // calculate expected elapsed time
		//load_time_scheduled(item); // calculate the scheduled time on the entry
		prev_item = item;
	}
	// third cycle, we now have time_from_departure
	for (map<int, fms_fp_item_t*>::iterator it = a_fp.begin(); it != a_fp.end(); ++it) {
		fms_fp_item_t* item = (*it).second;
		if (item->a_time_from_departure == -1) item->a_time_from_departure = a_total_time - item->a_time_from_arrival;
		if (item->a_time_from_arrival  == -1) item->a_time_from_arrival = a_total_time - item->a_time_from_departure;
		item->a_distance_from_arrival = a_total_distance - item->a_distance_from_departure;
		if (item->a_index !=0) debug_out(debug, "fms_fp: LEG %d: %s-%s %f nm (%f nm/%f nm), %s (%s/%s). Expect %s in %s at %s, alt %s", 
			item->a_index,
			prev_item->a_name.c_str(),
			item->a_name.c_str(),
			item->a_distance_from_prev,
			item->a_distance_from_departure,
			item->a_distance_from_arrival,
			minutes2time(item->a_time_from_prev).c_str(),
			minutes2time(item->a_time_from_departure).c_str(),
			minutes2time(item->a_time_from_arrival).c_str(),
			item->a_name.c_str(),
			item->a_flight_status.c_str(),
			minutes2time(item->a_time_scheduled).c_str(),
			altitude2string(item->a_altitude_expected).c_str()
			);
		if (status == "ADD_ENTRY") load_cache(item); // load actual_time the information from the cache
		a_fp_cache[item->a_name] = item; // saving item to cache
		prev_item = item;
	}
	a_fp[0]->a_time_scheduled = a_fms_config_ref->a_fms_config["scheduled_takeoff"];
	a_toc->a_time_scheduled = a_fms_config_ref->a_fms_config["scheduled_takeoff"]+a_fms_config_ref->a_fms_config["climb_time"];
	a_tod->a_time_scheduled = a_fms_config_ref->a_fms_config["scheduled_takeoff"]+a_total_time-a_fms_config_ref->a_fms_config["desc_time"];
	if (a_toc->a_time_scheduled > a_tod->a_time_scheduled) { // TOC is after TOD, altitude cannot be reached
		a_fms_config_ref->a_fms_config["wrong_config"] = 1;
		debug_out(err,"fms_fp: FMS configured FL cannot be reached");
		return;
	}
	debug_out(info, "fms_fp: Loaded flight plan %s->%s (%d),  distance %.0f nm,  duration %s, departure at %s, arrival at %s, TOD %s",
			a_fp[0]->a_name.c_str(),
			a_fp[a_arrival_index]->a_name.c_str(),
			a_count,
			a_total_distance,
			minutes2time(a_total_time).c_str(),
			minutes2time(a_fms_config_ref->a_fms_config["scheduled_takeoff"]).c_str(),
			minutes2time(a_fp[a_arrival_index]->a_time_scheduled).c_str(),
			tod_string.c_str()
		);
	save(); // save the fp
	a_fms_config_ref->a_fms_config["wrong_config"] = 0;
}

// get the number of entries in the FMS
int fms_fp_t::num_entries(void) {
	return XPLMCountFMSEntries();
}

// load into a given entry information regarding actual_time (changed: ADD_ENTRY)
void fms_fp_t::load_cache(fms_fp_item_t* item) {
		if (a_fp_cache[item->a_name]) {
			item->a_time_actual = a_fp_cache[item->a_name]->a_time_actual;
			item->a_time_at_gate = a_fp_cache[item->a_name]->a_time_at_gate;
			item->a_altitude_actual = a_fp_cache[item->a_name]->a_altitude_actual;
			item->a_delay_from_scheduled = a_fp_cache[item->a_name]->a_delay_from_scheduled;
			debug_out(debug, "fms_fp: retrieving item %s from cache", item->a_name.c_str());
		}
}

// load basic information for the entry
void fms_fp_t::load_entry(int index,fms_fp_item_t* item) {
		char tmp_name[80];
		char tmp_full_name[80];
		XPLMNavRef tmp_ref;
		XPLMGetFMSEntryInfo(index, &item->a_type, tmp_name, &tmp_ref, &item->a_altitude, &item->a_lat, &item->a_lon);
		XPLMGetNavAidInfo(tmp_ref, NULL, NULL, NULL, NULL, NULL, NULL, NULL, tmp_full_name, NULL);	
		item->a_index = index;
		item->a_name.assign(tmp_name);
		item->a_full_name.assign(tmp_full_name);
		item->a_type_string.assign(load_entry_type(item->a_type));
		debug_out(verbose, "fms_fp: creating FP item %d with name %s (%s) type %d lat %f lon %f alt %d", index,item->a_name.c_str(),item->a_full_name.c_str(),item->a_type,item->a_lat,item->a_lon,item->a_altitude);
}

string fms_fp_t::load_entry_type(XPLMNavType type) {
	std::string s;
	if (type ==xplm_Nav_Airport) s = "APT";
	else if (type ==xplm_Nav_NDB) s = "NDB";
	else if (type ==xplm_Nav_VOR) s = "VOR";
	else if (type ==xplm_Nav_ILS) s = "ILS";
	else if (type ==xplm_Nav_NDB) s = "NDB";
	else if (type ==xplm_Nav_Localizer) s = "LOC";
	else if (type ==xplm_Nav_Fix) s = "FIX";
	else if (type ==xplm_Nav_DME) s = "DME";
	else if (type ==xplm_Nav_LatLon) s = "LAT";
	else s = "UNK";
	return s;
}

// set the distances between two entries. Calculate total distance
void fms_fp_t::load_distance(fms_fp_item_t* prev_item,fms_fp_item_t* item) {
	if (item->a_index == 0) return; // first entry, skipping
	float leg_d = distance_between(prev_item,item,3);
	a_total_distance += leg_d;
	item->a_distance_from_departure = a_total_distance;
	item->a_distance_from_prev = leg_d;
	prev_item->a_distance_from_next = leg_d;
	debug_out(verbose, "fms_fp: distance between entry %d (%s) and entry %d (%s) is %f, cumulative distance %f",prev_item->a_index,prev_item->a_name.c_str(),item->a_index,item->a_name.c_str(),leg_d,a_total_distance);
}		

// set the item flight status based on the information of the FMS
void fms_fp_t::load_flight_status(fms_fp_item_t* item) {
	if (item->a_distance_from_departure < a_fms_config_ref->a_fms_config["climb_distance"]) item->a_flight_status = "climb";
	else if (item->a_distance_from_arrival < a_fms_config_ref->a_fms_config["desc_distance"]) item->a_flight_status = "desc";
	else item->a_flight_status = "cruise";
	debug_out(debug,"fms_fp: expecting flight status %s at entry %s",item->a_flight_status.c_str(),item->a_name.c_str());
}		

//  calculate time from prev, time to next, time from departure, time from arrival
void fms_fp_t::load_time(fms_fp_item_t* prev_item, fms_fp_item_t* item) {
	if (item->a_index == 0) { // first entry, skipping
		item->a_time_from_departure = 0;
	}
	if (item->a_flight_status == "climb") { // before top of climb, use the profiles
		item->a_time_from_departure = a_fms_config_ref->value_from_profile("climb_distance_time",item->a_distance_from_departure); //time from departure is calculated using the profile
		item->a_time_from_prev = item->a_time_from_departure - prev_item->a_time_from_departure; // time from prev is the difference between the current and previous time from departure
		item->a_altitude_expected = a_fms_config_ref->value_from_profile("climb_distance_fl",item->a_distance_from_departure)*1000; //expected altitude is calculated using the profile
	} else if (item->a_flight_status == "cruise") {
		item->a_altitude_expected = a_fms_config_ref->a_fms_config["cruise_fl"]*100; // expected altitude is cruise FL
		if (prev_item->a_flight_status == "climb") { // first item after the TOC
			float time_before_toc = a_fms_config_ref->a_fms_config["climb_time"]  - prev_item->a_time_from_departure; // time to cover the distance between prev item and TOD
			float time_after_toc = (item->a_distance_from_departure - a_fms_config_ref->a_fms_config["climb_distance"])*60 / a_fms_config_ref->a_fms_config["cruise_gs"]; // time to cover the distance between TOD and current item
			item->a_time_from_prev = time_before_toc + time_after_toc; // time from prev is the sum of the two legs
			item->a_time_from_departure = prev_item->a_time_from_departure + item->a_time_from_prev; // time from departure is the prev item time from departure + the time for the current leg
			debug_out(debug,"fms_fp: just after TOC, time before is %f, time after is %f",time_before_toc,time_after_toc);
		} else { // enruoute
			item->a_time_from_prev = item->a_distance_from_prev*60 / a_fms_config_ref->a_fms_config["cruise_gs"]; // time from prev is t=s/v where speed is the expected GS on cruise
			item->a_time_from_departure = prev_item->a_time_from_departure + item->a_time_from_prev; // time from departure is the prev item time from departure + the time for the current leg
		}
	}	else if (item->a_flight_status == "desc") {
		if (prev_item->a_flight_status == "desc") { // descending, prev item is on the desc path
			item->a_time_from_arrival = a_fms_config_ref->value_from_profile("desc_distance_time",item->a_distance_from_arrival); // we need time for arrival calculated using the profile to calculate time from arrival
			item->a_time_from_prev = prev_item->a_time_from_arrival  - item->a_time_from_arrival; // time from prev is the difference between the current and previous time from arrival
			item->a_time_from_departure = prev_item->a_time_from_departure + item->a_time_from_prev; // time from departure is the prev item time from departure + the time for the current leg	
		} else { // prev item can be cruise or climb
			// set the TOD string
			float distance_to_tod = a_fms_config_ref->a_fms_config["desc_distance"] - item->a_distance_from_arrival;
			char buffer[130];
            snprintf(buffer, 130, "%3.0fm to %.5s", distance_to_tod,item->a_name.c_str());
			tod_string.assign(buffer);
			debug_out(debug, "fms_fp: TOD is %s",tod_string.c_str());
			if (prev_item->a_flight_status == "cruise") { // first item after the TOD
				// time from prev = time before TOD (on cruise) + time after TOD (on desc)
				float time_before_tod = (prev_item->a_distance_from_arrival - a_fms_config_ref->a_fms_config["desc_distance"])*60 / a_fms_config_ref->a_fms_config["cruise_gs"]; // time between prev item and TOD is like when on cruise
				float time_after_tod =  a_fms_config_ref->a_fms_config["desc_time"] - a_fms_config_ref->value_from_profile("desc_distance_time",item->a_distance_from_arrival); // time to descend from TOD to current item
				item->a_time_from_prev = time_before_tod + time_after_tod; // time from prev is the sum of the two legs
				debug_out(debug,"fms_fp: just after TOD, time before is %f, time after is %f",time_before_tod,time_after_tod);
			} else if (prev_item->a_flight_status == "climb") { // first item after the TOD and TOC
				// time from prev = time before TOC (on climb) + time between TOD and TOC (on cruise) + time after TOD (on desc)
				float time_before_toc = a_fms_config_ref->a_fms_config["climb_time"]  - prev_item->a_time_from_departure; // time to cover the distance between prev item and TOD
				float time_after_tod =  a_fms_config_ref->a_fms_config["desc_time"] - a_fms_config_ref->value_from_profile("desc_distance_time",item->a_distance_from_arrival); // time to descend from TOD to current item
				float time_on_cruise = ((prev_item->a_distance_from_arrival - (a_fms_config_ref->a_fms_config["climb_distance"] - prev_item->a_distance_from_departure)) - a_fms_config_ref->a_fms_config["desc_distance"])*60 / a_fms_config_ref->a_fms_config["cruise_gs"];
				item->a_time_from_prev = time_before_toc + time_on_cruise + time_after_tod; // time from prev is the sum of the two legs
				debug_out(debug,"fms_fp: after TOC and TOD, time to TOC is %f, time on cruise is %f, time after TOD is %f",time_before_toc,time_on_cruise,time_after_tod);
			} 
		}
		// the following are in common with all "desc" items
		item->a_time_from_departure = prev_item->a_time_from_departure + item->a_time_from_prev; // time from departure is the prev item time from departure + the time for the current leg
		item->a_time_from_arrival = a_fms_config_ref->value_from_profile("desc_distance_time",item->a_distance_from_arrival);
		item->a_altitude_expected = a_fms_config_ref->value_from_profile("desc_distance_fl",item->a_distance_from_arrival)*1000; //expected altitude is calculated using the profil
	}
	prev_item->a_time_from_next = item->a_time_from_prev;
	a_total_time += item->a_time_from_prev;
	debug_out(debug,"fms_fp: expected time from %s (%s) to %s (%s) is %f (from dep: %f, from arrival: %f), so far %f",prev_item->a_name.c_str(),prev_item->a_flight_status.c_str(),item->a_name.c_str(),item->a_flight_status.c_str(),item->a_time_from_prev,item->a_time_from_departure,item->a_time_from_arrival,a_total_time);
	item->a_time_scheduled = a_fms_config_ref->a_fms_config["scheduled_takeoff"]+item->a_time_from_departure;
	debug_out(debug,"fms_fp: scheduled time over %s at %s",item->a_name.c_str(),minutes2time(item->a_time_scheduled).c_str());
	debug_out(debug,"fms_fp: expected altitude over %s is %s",item->a_name.c_str(),altitude2string(item->a_altitude_expected).c_str());
}		

// save the configured flight plane to disk
void fms_fp_t::save(void)
{
	FILE * fp;
    fp = fopen ("Output/FMS plans/_xcontrol.fms","w");
    debug_out(debug, "fms_fp: saving flight plan to Output/FMS plans/_xcontrol.fms");
	if (fp==NULL) {
		debug_out(err, "fms_fp: unable to open file for saving");
		return;
	}
	fprintf(fp, "I\r\n3 version\r\n1\r\n"); // fp header
	int num_entries = a_count-1;
	fprintf(fp, "%d\r\n",num_entries); // number of entries -1 
	for (map<int, fms_fp_item_t*>::iterator it = a_fp.begin(); it != a_fp.end(); ++it) {
		fms_fp_item_t* item = (*it).second;
		// handle entry type
		int type;
		if (item->a_type ==xplm_Nav_Airport) type = 1;
		else if (item->a_type ==xplm_Nav_NDB) type = 2;
		else if (item->a_type ==xplm_Nav_VOR) type = 3;
		else if (item->a_type ==xplm_Nav_Fix) type = 11;
		else if (item->a_type ==xplm_Nav_LatLon) type = 28;
		else  type = 28;
		std::string name;
		// handle entry name
		if (item->a_type ==xplm_Nav_LatLon) { // must be in the format +00.000_+000.000
			char lat[2048] = {};
			memset(lat, 0, 2048);
            if (item->a_lat < 0) snprintf(lat, 2048, "-%06.03f",fabs(item->a_lat));
            else snprintf(lat, 2048, "+%06.03f",fabs(item->a_lat));
			char lon[2048] = {};
			memset(lon, 0, 2048);
            if (item->a_lon < 0) snprintf(lon, 2048, "-%07.03f",fabs(item->a_lon));
            else snprintf(lon, 2048, "+%07.03f",fabs(item->a_lon));
			char latlon[2048] = {};
			memset(latlon, 0, 2048);
            snprintf(latlon, 2048, "%s_%s",lat,lon);
			name.assign(latlon);
		} else name = item->a_name;
		fprintf(fp, "%d %s %d.000000 %.6f %.6f\r\n",type,name.c_str(),(int)item->a_altitude,item->a_lat,item->a_lon);
	}
	for (int i = 0; i<= 10; i++) fprintf(fp, "0 ---- 0.000000 0.000000 0.000000\r\n"); // add padding
	fflush(fp);
	fclose(fp);
}


// calculate the distance between two FP items
float fms_fp_t::distance_between(fms_fp_item_t* from, fms_fp_item_t* to,int unit)
{
	if (from->a_name == to->a_name) return 0; // same entry, returns 0
	double d = calculate_distance((double) from->a_lat, (double) from->a_lon, (double) to->a_lat, (double) to->a_lon, unit);
	debug_out(verbose, "fms_fp: distance between  (%f,%f) and (%f,%f) in %d is %f", from->a_lat, from->a_lon, to->a_lat, to->a_lon, unit,(float)d);
	return (float) d;
}

// calculate the distance between to lat/lon coordinates 
double fms_fp_t::calculate_distance(double lat1, double lon1, double lat2, double lon2,int unit) {
  double theta, dist;
  theta = lon1 - lon2;
  dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
  dist = acos(dist);
  dist = rad2deg(dist);
  dist = dist * 60 * 1.1515;
  switch(unit) {
    case 1: //Miles
      break;
    case 2: //Km
      dist = dist * 1.609344;
      break;
    case 3://Nn
      dist = dist * 0.8684;
      break;
  }
  return (dist);
}

// return true if the flight plan has been changed
int fms_fp_t::has_changed(string flight_status) {
	if (a_fms_config_ref->a_fms_config["missing_config"]) return -5;
	int count = num_entries();
	if (count < 2)  return -1; // FP not set
	//  Load the dept/arrival currently set in the FP
	fms_fp_item_t* departure = new fms_fp_item_t();
	fms_fp_item_t* arrival = new fms_fp_item_t();
	load_entry(0,departure);
	load_entry(count-1,arrival);
	if (departure->a_type != xplm_Nav_Airport) return -2; // departure entry not an airport
	if (arrival->a_type != xplm_Nav_Airport) return -3; // arrival entry not an airport
	// At this point the FP currently set is valid
	if (a_count < 2 || a_fp.empty()) return 2; // a valid FP is set but not loaded yet
	if (a_status == 99) return 5; // configuration change, force reload
	if (a_fms_config_ref->a_fms_config["wrong_config"]) return -6;
	// nothing has changed: same departure/arrival ariport and num entries
	if (departure->a_name == a_fp[0]->a_name && arrival->a_name == a_fp[a_arrival_index]->a_name && count == a_count) return 1;
	// An entry has been added/deleted: same departure/arrival ariport, but num entries changed
	if (departure->a_name == a_fp[0]->a_name && arrival->a_name == a_fp[a_arrival_index]->a_name && flight_status == "cruise" && count != a_count) return 3;
	return 4; // something else has changed
}

string fms_fp_t::status2string(int status) {
	if (status == -1) return "INVALID";
	if (status == -2) return "DEP_APT";
	if (status == -3) return "ARR_APT";
	if (status == -4) return "NO_LOAD";
	if (status == -5) return "NO_CONF";
	if (status == -6) return "CONF_ER";
	if (status == 1) return "NOT_CHANGED";
	if (status == 2) return "LOADED";
	if (status == 3) return "ADD_ENTRY";
	if (status == 4) return "CHANGED";
	if (status == 5) return "CONF_CHG";
	return "UNKNOWN";
}

// update the expected time for each FP entry
void fms_fp_t::update_expected_time(float takeoff_time) {
	for (int i = 1; i<=a_arrival_index; i++ ) {
		a_fp[i]->a_time_expected = takeoff_time + a_fp[i]->a_time_from_departure; 
		debug_out(debug,"fms_fp: updating expected time for %s at %s",a_fp[i]->a_name.c_str(),minutes2time(a_fp[i]->a_time_expected).c_str());
	}
}
