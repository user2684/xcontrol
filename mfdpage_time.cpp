#include <string.h>
#include <time.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

#include "include/mfdpage_time.h"
#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"

using std::map;
using std::string;

mfdpage_time_t::mfdpage_time_t(string name)
{
	a_name = name;
    a_hours = 0;
    a_mins = 0;
    a_secs = 0;
}

string mfdpage_time_t::get_template(string name,std::map<int, std::string> &sources)
{
	sources[0] = "sim/time/zulu_time_sec";
	return "";
}

void mfdpage_time_t::do_refresh(object_t* source) {
	if (source->name() == "sim/time/zulu_time_sec") {
	   float currsecs = *source;
        a_secs = (int) currsecs;
    }
}

int mfdpage_time_t::refresh_interval(string name) {
    return 10;
}

string mfdpage_time_t::refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)
{
	a_hours = a_secs/3600;
	a_mins = (a_secs%3600)/60;
	a_secs = (a_secs%3600)%60;
	a_time_t = time(NULL);
    struct tm * today = localtime(&a_time_t);
	a_time["hour"] = a_hours;
	a_time["minute"] = a_mins;
    a_date["year"] = today->tm_year+1900;
    a_date["month"] = today->tm_mon+1;
    a_date["day"] = today->tm_mday;
    return "";
}
