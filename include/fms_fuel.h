#ifndef FMS_FUEL_H
#define FMS_FUEL_H

#include <string>
#include <fms_utils.h>
#include <fms_config.h>

class fms_fuel_t;

class fms_fuel_t: public tools_t,public fms_utils_t
{
public:
	fms_fuel_t(void);
	fms_fuel_t(fms_config_t* fms_config);
	~fms_fuel_t(void);
	void load(void);
	string remaining(void);
	float fuel_flow(void);
	int a_num_engines;
	float a_lb_to_load;
private:
	fms_config_t* a_fms_config_ref;
	int num_engines(void);
	float lb_to_load(void);
	float fuel_on_board(void);
};

#endif /* FMS_FUEL_H */
