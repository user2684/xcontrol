#ifndef MFDPAGE_TIME_H
#define MFDPAGE_TIME_H

#include <time.h>
#include <map>
#include <string>
#include "tools.h"
#include "mfdpage.h"
#include "fms.h"

class mfdpage_time_t : public tools_t, public mfdpage_type_t, public fms_utils_t
{
public:
    mfdpage_time_t(string name);
    string get_template(string name,std::map<int, std::string> &sources);
	void do_refresh(object_t* source);
	int refresh_interval(string name);
    string refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template);
private:
	fms_t* a_fms_ref;
	string a_name;
	char a_hours;
    char a_mins;
    int  a_secs;
    time_t a_time_t;
    struct tm a_tr;
};

#endif /* MFDPAGE_TIME_H */
