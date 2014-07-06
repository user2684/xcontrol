#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/tools.h"
#include "include/fms.h"
#include "include/SDK/XPLMNavigation.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"
#include "include/SDK/XPLMUtilities.h"

#if IBM
#define snprintf _snprintf
#endif

using namespace std;
using std::map;
using std::string;

fms_t::fms_t(void)
{
	reset();
}

fms_t::~fms_t(void)
{
	delete a_fms_config_ref;
	delete a_fp_ref;
	delete a_fuel_ref;
}

void fms_t::reset(void) 
{
	debug_out(debug, "fms: resetting the FMS");
	a_fms_config_ref = new fms_config_t();
	a_fp_ref = new fms_fp_t(a_fms_config_ref); 
	a_fuel_ref = new fms_fuel_t(a_fms_config_ref);
	a_current_zulu_time = a_current_altitude = 0;
	a_current_entry_index = -1;
	a_flight_status = "departure";
}

void fms_t::load() 
{
	debug_out(debug, "fms: loading the FMS");
	a_fms_config_ref->load(); // load the config
	a_fp_ref->load();  // load the FP
	a_fuel_ref->load(); // load the fuel (which requires the FP to be loaded)
}

void fms_t::refresh(void) 
{
	a_fp_ref->a_status = a_fp_ref->has_changed(a_flight_status);
	if (a_fp_ref->a_status > 1) { // the FP has changed and it is valid
		debug_out(info, "fms: reloading the FMS (status: %s)",a_fp_ref->status2string(a_fp_ref->a_status).c_str());
		load();
		// voice report
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "%.0f miles,  %s minutes",
			a_fp_ref->a_total_distance,
			minutes2time(a_fp_ref->a_total_time).c_str()
		);
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
	}
}

// return the displayed entry ID
int fms_t::displayed_index(void) 
{
	int index = XPLMGetDisplayedFMSEntry();
	if (index > a_fp_ref->a_arrival_index) index = a_fp_ref->a_arrival_index;
	debug_out(verbose, "fms: current displayed entry index is %d", index);
	return index;
}

// return the displayed entry name
string fms_t::entry_name(int index) 
{
	debug_out(verbose, "fms: current displayed entry name is %s",a_fp_ref->a_fp[index]->a_full_name.c_str());
	return a_fp_ref->a_fp[index]->a_full_name;
}

// return the displayed entry type
string fms_t::entry_type(int index) 
{
	debug_out(verbose, "fms: current displayed entry type is %s",a_fp_ref->a_fp[index]->a_type_string.c_str());
	return a_fp_ref->a_fp[index]->a_type_string;
}

// get the index of the FMS entry we are flying to
int fms_t::flying_index(void)
{
	int index = XPLMGetDestinationFMSEntry();
	debug_out(verbose, "fms: we are flying to entry %s (%d)",a_fp_ref->a_fp[index]->a_name.c_str(),index);
	return index;
}

// get the distance to the FMS entry we are flying to
float fms_t::distance_to_current(void)
{
	XPLMDataRef a_ref = XPLMFindDataRef("sim/cockpit2/radios/indicators/gps_dme_distance_nm");
	float d = XPLMGetDataf(a_ref);
	debug_out(debug, "fms: distance to the entry we are currently flying to is %f nm", d);
	return d;
}

// get the speed to the FMS entry we are flying to
float fms_t::speed_to_current(void)
{
	XPLMDataRef a_ref = XPLMFindDataRef("sim/cockpit2/radios/indicators/gps_dme_speed_kts");
	float s = XPLMGetDataf(a_ref);
	debug_out(debug, "fms: speed to the entry we are currently flying to is %f kts", s);
	return s;
}

// ETA to reach the current entry
float fms_t::time_to_current(void)
{
	float t = 0;
	float distance_km = nm2km(distance_to_current());
	float speed_kmm = kts2kmm(speed_to_current());
	if (speed_kmm == 0) return t;
	t = distance_km/speed_kmm; // in minutes
	debug_out(debug, "fms: time to reach the current entry is %f min (distance: %f km, speed: %f kmm)",t,distance_km,speed_kmm);
	return t;
}


// get the distance to reach a given entry
float fms_t::distance_to_entry(int req_index)
{
	float d = 0;
	int current_index = flying_index();
	fms_fp_item_t* current_entry = a_fp_ref->a_fp[current_index]; // entry we are flying to
	fms_fp_item_t* req_entry = a_fp_ref->a_fp[req_index]; // requested entry
	d = distance_to_current(); // distance from the actual position to the current FMS entry
	if (req_index == current_index) { // requesting the distance to our current destination, return it
		debug_out(debug, "fms: distance to %s (to which we are flying) is %f nm",req_entry->a_name.c_str(),d);
		return d; 
	} 
	// add to the distance_to_current, the remaining distance
	d += req_entry->a_distance_from_departure - current_entry->a_distance_from_departure;
	debug_out(debug, "fms: distance to %s is %f nm",req_entry->a_name.c_str(),d);
	return d;
}

// get the time to reach a given entry
float fms_t::minutes_to_entry(int req_index)
{
	float t = 0;
	int current_index = flying_index();
	fms_fp_item_t* current_entry = a_fp_ref->a_fp[current_index]; // entry we are flying to
	fms_fp_item_t* req_entry = a_fp_ref->a_fp[req_index]; // requested entry
	if (a_flight_status == "departure") t = current_entry->a_time_from_departure; // on ground, use scheduled time as time to current
	else t = time_to_current(); // time from the actual position to the current FMS entry
	if (req_index == current_index) { // requesting the time to our current destination, return it
		debug_out(debug, "fms: time to %s (to which we are flying) is %f min", req_entry->a_name.c_str(),t);
		return t;
	}
	// add to the time_to_current, the remaining time
	t += req_entry->a_time_from_departure - current_entry->a_time_from_departure;
	debug_out(debug, "fms: time to %s is %f min",req_entry->a_name.c_str(),t);
	return t;
}

// get the distance from a given entry to the arrival
float fms_t::remaining_distance_from_entry(int req_index) {
	fms_fp_item_t* req_entry = a_fp_ref->a_fp[req_index]; // requested entry
	debug_out(debug, "fms: distance from %s to arrival is %f nm",req_entry->a_name.c_str(),req_entry->a_distance_from_arrival);
	return req_entry->a_distance_from_arrival;
}

// get the time to reach a given entry
string fms_t::time_to_entry(int to_index)
{
	float minutes = minutes_to_entry(to_index);
	return minutes2time(minutes);
}

// get ETA to a given entry
string fms_t::eta_to_entry(int index,float zulu_time) 
{
	string eta = minutes2time((zulu_time /60)+ minutes_to_entry(index));
	debug_out(debug, "fms: ETA to entry %d is %s",index,eta.c_str());
	return eta;
}

// get the remaining distance to destination
float fms_t::remaining_distance(void) 
{
	return distance_to_entry(a_fp_ref->a_arrival_index);
}

// get the remaining time to destination
float fms_t::remaining_time(void) 
{
	return minutes_to_entry(a_fp_ref->a_arrival_index);
}

// get the percentage of the flew distance
float fms_t::percentage(void)
{
	if (a_fp_ref->a_total_time == 0) return 0;
	float percentage_left = remaining_time()*100/a_fp_ref->a_total_time;
	float percentage = 100-percentage_left;
	if (percentage > 100) percentage = 100;
	debug_out(debug, "fms: percentage of flight %f %",percentage);
	return percentage;
}

// check departure gate
void fms_t::check_gate_departure(float speed) {
	if (a_current_zulu_time == 0) return; // no zulu time available yet
	if (a_flight_status != "departure") return; // avoid checking when on cruise, save CPU cycles
	speed = ms2kts(speed);
	if (a_fp_ref->a_fp[0]->a_time_at_gate == -1 && speed > 5) {
		a_fp_ref->a_fp[0]->a_time_at_gate = zulu2minutes(a_current_zulu_time);
		debug_out(info,"fms: leaving the gate at %s",minutes2time(a_fp_ref->a_fp[0]->a_time_at_gate).c_str());
		// voice report
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "We are leaving departure gate in %s now at %s and we expect to take off at %s. The distance to %s will be %.0f miles and we expect to land at %s, %s minutes after our departure",
			a_fp_ref->a_fp[0]->a_full_name.c_str(),
			minutes2time(a_fp_ref->a_fp[0]->a_time_at_gate).c_str(),
			minutes2time(a_fp_ref->a_fp[0]->a_time_scheduled).c_str(),
			a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_full_name.c_str(),
			a_fp_ref->a_total_distance,
			minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_scheduled).c_str(),
			minutes2time(a_fp_ref->a_total_time).c_str()
		);	
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
		return;
	} 
}

// saves the actual takeoff time
void fms_t::check_takeoff(float speed) {
	if (a_current_zulu_time == 0) return; // no zulu time available yet
	if (a_fp_ref->a_fp[0]->a_time_actual != -1) return; // takeoff already set
	speed = ms2kts(speed);
	if (a_flight_status == "departure" && speed > 80) { // not departed yet and speed > 80
		a_fp_ref->a_fp[0]->a_time_actual = zulu2minutes(a_current_zulu_time);
		a_fp_ref->a_fp[0]->a_delay_from_scheduled = a_fp_ref->a_fp[0]->a_time_actual - a_fp_ref->a_fp[0]->a_time_scheduled;
		a_fp_ref->update_expected_time(a_fp_ref->a_fp[0]->a_time_actual);
		a_flight_status = "cruise";
		debug_out(info,"fms: takeoff recorded at %s, flight status set to cruise",minutes2time(a_fp_ref->a_fp[0]->a_time_actual).c_str());
		// voice report
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "Reporting take off at %s, delay %s minutes. Landing expected at %s",
			minutes2time(a_fp_ref->a_fp[0]->a_time_actual).c_str(),
			delay2string(a_fp_ref->a_fp[0]->a_delay_from_scheduled).c_str(),
			minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_expected).c_str()
		);
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
	} 
}

// saves the actual landing time
void fms_t::check_landing(float speed) {
	if (a_current_zulu_time == 0) return; // no zulu time available yet
	if (a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual != -1) return;
	speed = ms2kts(speed);
	if (a_flight_status == "cruise" && speed < 80) {
		a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual = zulu2minutes(a_current_zulu_time);
		a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_delay_from_scheduled = a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual - a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_scheduled;
		a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_delay_from_expected = a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual - a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_expected;
		float actual_duration = a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual - a_fp_ref->a_fp[0]->a_time_actual;
		a_flight_status = "arrival";
		debug_out(info,"fms: landing recorded at %s, flight status set to arrival",minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual).c_str());
		// voice report
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "Reporting landing at %s, delay %s minutes. Flight duration %s, expected %s",
			minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_actual).c_str(),
			delay2string(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_delay_from_scheduled).c_str(),
			minutes2time(actual_duration).c_str(),
			minutes2time(a_fp_ref->a_total_time).c_str()
		);
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
	} 
}

// check arrival gate
void fms_t::check_gate_arrival(void) {
	if (a_current_zulu_time == 0) return; // no zulu time available yet
	if (a_flight_status != "arrival") return; // avoid checking when on cruise, save CPU cycles
	if (a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_at_gate  == -1 && a_fuel_ref->fuel_flow() < 0.001) {
		a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_at_gate = zulu2minutes(a_current_zulu_time);
		debug_out(info,"fms: arriving at the gate at %s",minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_at_gate).c_str());
		// voice report
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "Arriving at gate at %s",minutes2time(a_fp_ref->a_fp[a_fp_ref->a_arrival_index]->a_time_at_gate).c_str());
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
	} 
}

// check the current position
void fms_t::check_position(void) {
	if (a_flight_status != "cruise") return; // avoid checking when not on cruise
	int current_entry_index = flying_index(); // retrieve the current index
	int changed = 0;
	if (current_entry_index != a_current_entry_index) changed = 1; // the index changed, we are now flying to a different entry
	if (a_current_entry_index == -1)  { }// no previous FMS entry set yet
	if (! changed) return; // nothing to do 
	if (changed && (a_current_entry_index == 0 || a_current_entry_index == a_fp_ref->a_arrival_index)) { } // do not record position at departure/arrival
	if (changed && (a_fp_ref->a_fp[a_current_entry_index]->a_time_actual != -1)) { } // position already recorded
	if (changed && current_entry_index != 1 && current_entry_index != a_current_entry_index+1) {  // we are not flying to the next entry, this is a FLY_DIRECT
		debug_out(info,"fms: flying direct to %s",entry_name(current_entry_index).c_str());
		char voice_1[2048] = "";
        snprintf(voice_1, 2048, "Reporting direct flight to %s",entry_name(current_entry_index).c_str());
		if (a_fms_config_ref->a_fms_config["voice"] >0)XPLMSpeakString(voice_1);
	}
	if (current_entry_index == a_current_entry_index+1)  {  // the index has increased, we are now flying to the next entry
		a_fp_ref->a_fp[a_current_entry_index]->a_time_actual = zulu2minutes(a_current_zulu_time);
		a_fp_ref->a_fp[a_current_entry_index]->a_altitude_actual = a_current_altitude;
		a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_scheduled = a_fp_ref->a_fp[a_current_entry_index]->a_time_actual - a_fp_ref->a_fp[a_current_entry_index]->a_time_scheduled;
		a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_expected = a_fp_ref->a_fp[a_current_entry_index]->a_time_actual - a_fp_ref->a_fp[a_current_entry_index]->a_time_expected;
		debug_out(info,"fms: recording over %s at %s (delay from schedule: %s, delay from expected: %s), altitude %s (expected %s)",
			entry_name(a_current_entry_index).c_str(),
			minutes2time(a_fp_ref->a_fp[a_current_entry_index]->a_time_actual).c_str(),
			delay2string(a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_scheduled).c_str(),
			delay2string(a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_expected).c_str(),
			altitude2string(a_current_altitude).c_str(),
			altitude2string(a_fp_ref->a_fp[a_current_entry_index]->a_altitude_expected).c_str()
		);
		// voice report
		char voice_1[2048] = ""; char voice_2[2048] = "";
        snprintf(voice_1, 2048, "Reporting over %s at %s, delay %s minutes",
			entry_name(a_current_entry_index).c_str(),
			minutes2time(a_fp_ref->a_fp[a_current_entry_index]->a_time_actual).c_str(),
			delay2string(a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_scheduled).c_str()
		);
        snprintf(voice_2, 2048, "Reporting over %s at %s, scheduled at %s, delay from schedule %s minutes, delay from expected %s minutes, altitude %s, expected altitude %s",
			entry_name(a_current_entry_index).c_str(),
			minutes2time(a_fp_ref->a_fp[a_current_entry_index]->a_time_actual).c_str(),
			minutes2time(a_fp_ref->a_fp[a_current_entry_index]->a_time_scheduled).c_str(),
			delay2string(a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_scheduled).c_str(),
			delay2string(a_fp_ref->a_fp[a_current_entry_index]->a_delay_from_expected).c_str(),
			altitude2string(a_current_altitude).c_str(),
			altitude2string(a_fp_ref->a_fp[a_current_entry_index]->a_altitude_expected).c_str()
		);
		if ((int) a_fms_config_ref->a_fms_config["voice"] == 1) XPLMSpeakString(voice_1);
		if ((int) a_fms_config_ref->a_fms_config["voice"] == 2) XPLMSpeakString(voice_2);
	}
	a_current_entry_index = current_entry_index;
}
