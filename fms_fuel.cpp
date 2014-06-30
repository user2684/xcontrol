#include <map>
#include <string>
#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <fms_fuel.h>
#include "XPLM/XPLMDataAccess.h"
#include <math.h>

using namespace std;
using std::map;
using std::string;

fms_fuel_t::fms_fuel_t(void) {
}

fms_fuel_t::fms_fuel_t(fms_config_t* fms_config) {
	a_fms_config_ref = fms_config;
	a_num_engines = a_lb_to_load = 0;
}

fms_fuel_t::~fms_fuel_t(void) {
}

void fms_fuel_t::load(void) {
	a_num_engines = num_engines();
	a_lb_to_load = lb_to_load();
}

// get the fuel on board
float fms_fuel_t::fuel_on_board(void) {
	XPLMDataRef a_ref = XPLMFindDataRef("sim/flightmodel/weight/m_fuel_total");
	float fuel_kg = XPLMGetDataf(a_ref);
	debug_out(verbose, "fms_fuel: the aircraft has %f kg of fuel", fuel_kg);
	return fuel_kg;
}

// get the fuel flow on the first engine
float fms_fuel_t::fuel_flow(void) {
	XPLMDataRef a_ref = XPLMFindDataRef("sim/flightmodel/engine/ENGN_FF_");
	float fuel_ff;
	XPLMGetDatavf(a_ref,&fuel_ff,0,1); // get fuel flow per engine
	debug_out(verbose, "fms_fuel: each engine is consuming %f kg/s of fuel", fuel_ff);
	return fuel_ff;
}

// get the number of engines
int fms_fuel_t::num_engines(void)
{
	XPLMDataRef a_ref = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
	int engines = XPLMGetDatai(a_ref);
	debug_out(debug, "fms_fuel: the aircraft has %d engines", engines);
	return engines;
}

// get the fuel eta
string fms_fuel_t::remaining(void) {
	float fuel_tot = fuel_on_board();
	int engines = a_num_engines;
	float fuel_ff = fabs(fuel_flow());
	if (fuel_ff < 0.001 || engines == 0) return minutes2time(0);
	float remaining_min = fuel_tot/(fuel_ff*60*engines);
	debug_out(debug, "fms_fuel: fuel ETA is %f min (fuel: %f kg, engines: %d, ff: %f kg/s)", remaining_min,fuel_tot,engines,fuel_ff);
	return minutes2time(remaining_min);
}

// returns the required lb of fuel to load to complete the trip loaded into the FMS based on the config
float fms_fuel_t::lb_to_load(void) {
	float fuel_lb;
	float fuel_cruise = a_fms_config_ref->a_fms_config["total_distance"]/100*a_fms_config_ref->a_fms_config["fuel_every_100nm"];
	fuel_lb = a_fms_config_ref->a_fms_config["fuel_ground"] + a_fms_config_ref->a_fms_config["fuel_clb_dsc"] + fuel_cruise + a_fms_config_ref->a_fms_config["fuel_contingency"];
	debug_out(debug, "fms_fuel: fuel to load %f lb (ground: %f, climb/desc: %f, cruise: %f, contigency: %f)",fuel_lb, a_fms_config_ref->a_fms_config["fuel_ground"],a_fms_config_ref->a_fms_config["fuel_clb_dsc"] ,fuel_cruise,a_fms_config_ref->a_fms_config["fuel_contingency"]);
	return fuel_lb;
}

