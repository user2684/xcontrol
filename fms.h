#ifndef FMS_H
#define FMS_H

#include <map>
#include <string>
#include <fms_fp.h>
#include <fms_config.h>
#include <fms_utils.h>
#include <fms_fuel.h>

class fms_t;

class fms_t: public tools_t, public fms_utils_t
{
public:
	fms_t(void);
	~fms_t(void);
	void refresh(void);
	void reset(void);
	void load(void);
	int displayed_index(void);
	string entry_name(int index);
	string entry_type(int index);
	float distance_to_entry(int index);
	float remaining_distance_from_entry(int index);
	float minutes_to_entry(int index);
	string time_to_entry(int index);
	string eta_to_entry(int index,float zulu_time);
	float remaining_distance(void);
	float remaining_time(void);
	int flying_index(void);
	float percentage(void);
	void check_takeoff(float speed);
	void check_landing(float speed);
	void check_position(void);
	void check_gate_departure(float speed);
	void check_gate_arrival(void);
	float a_current_zulu_time;
	float a_current_altitude;
	int a_current_entry_index;
	fms_fp_t* a_fp_ref;
	fms_fuel_t* a_fuel_ref;
	fms_config_t* a_fms_config_ref;
private:
	float distance_to_current(void);
	float speed_to_current(void);
	float time_to_current(void);
	string a_flight_status;
};


#endif /* FMS_H */
