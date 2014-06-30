#ifndef MFDPAGE_LED_H
#define MFDPAGE_LED_H

#include <time.h>
#include <map>
#include <string>
#include "tools.h"
#include "mfdpage.h"
#include "fms.h"

class mfdpage_led_t : public tools_t, public mfdpage_type_t
{
public:
    mfdpage_led_t(string name);
	mfdpage_led_t(string name, out_t* outdevice);
    string get_template(string name,std::map<int, std::string> &sources);
	void do_refresh(object_t* source);
	int refresh_interval(string name);
    string refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template);
private:
    void set_led_by_name(int led_debug, const char* dataref, int data, const char* action, ...);
    fms_t* a_fms_ref;
	string a_name;
	int led_debug;
	int a_gear;
    int a_autopilot_engaged;
    int a_weapons_armed;
	map<string,string> config;
	out_t* a_outdevice;
};

#endif /* MFDPAGE_LED_H */
