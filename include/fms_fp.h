#ifndef FMS_FP_H
#define FMS_FP_H

#include <map>
#include <string>
#include "include/SDK/XPLMNavigation.h"
#include "include/fms_utils.h"
#include "include/fms_config.h"

class fms_fp_t;
class fms_fp_item_t;

class fms_fp_t: public tools_t, fms_utils_t
{
public:
	fms_fp_t(void);
	fms_fp_t(fms_config_t* fms_config);
	~fms_fp_t(void);
	void load(void);
	string status2string(int invalid);
	int has_changed(string flight_status);
	void update_expected_time(float takeoff_time);
	std::map<int, fms_fp_item_t *> a_fp;
	float a_total_distance;
	float a_total_time;
	string tod_string;
	fms_fp_item_t* a_toc;
	fms_fp_item_t* a_tod;
	int a_arrival_index;
	int a_status;
private:
	fms_config_t* a_fms_config_ref;
	std::map<string, fms_fp_item_t *> a_fp_cache;
	int a_count;
	void save(void);
	// helpers for the load function
	int num_entries(void);
	void load_entry(int index,fms_fp_item_t* item);
	string load_entry_type(XPLMNavType type);
	void load_cache(fms_fp_item_t* item);
	void load_distance(fms_fp_item_t* prev_item,fms_fp_item_t* item);
	void load_flight_status(fms_fp_item_t* item);
	void load_time(fms_fp_item_t* prev_item, fms_fp_item_t* item);
	// helpers for calculating distances
	float distance_between(fms_fp_item_t* from, fms_fp_item_t* to,int unit);
	double calculate_distance(double lat1, double lon1, double lat2, double lon2,int unit);
};


class fms_fp_item_t: public tools_t
{
public:
	fms_fp_item_t(void);
	std::string a_name;
	std::string a_full_name;
	int a_index;
	float a_lat;
	float a_lon;
	int a_altitude;
	XPLMNavType a_type;
	string a_type_string;
	float a_distance_from_prev;
	float a_distance_from_next;
	float a_distance_from_departure;
	float a_distance_from_arrival;
	float a_time_from_prev;
	float a_time_from_next;
	float a_time_from_departure;
	float a_time_from_arrival;
	string a_flight_status;
	float a_time_scheduled;
	float a_time_expected;
	float a_time_actual;
	float a_time_at_gate;
	float a_delay_from_scheduled;
	float a_delay_from_expected;
	float a_altitude_expected;
	float a_altitude_actual;
	private:
};

#endif /* FMS_FP_H */
