#ifndef FMS_CONFIG_H
#define FMS_CONFIG_H

#include <map>
#include <string>
#include <fms_utils.h>

class fms_config_item_t;
class fms_config_t;

class fms_config_t: public tools_t, public fms_utils_t
{
public:
	fms_config_t(void);
	~fms_config_t(void);
	void load(void);
	float value_from_profile(string profile, int req_key);
	std::map<string, float> a_fms_config;
private:
	std::map<string, fms_config_item_t *> a_profiles;
	map<string,string> a_config;
	void load_config(void);
	void load_profiles(void);
	void load_profile(string profile_to_load);
	void reverse_profile(string profile_to_reverse,string profile_to_save);
	void combine_profile(string profile_key,string profile_value,string profile_to_save);
	void calculate_climb_desc(void);
};

class fms_config_item_t: public tools_t
{
public:
	fms_config_item_t(void);
	std::map<int, float> a_profile;
private:
};


#endif /* FMS_CONFIG_H */
