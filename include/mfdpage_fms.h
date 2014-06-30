#ifndef MFDPAGE_FMS_H
#define MFDPAGE_FMS_H

#include <time.h>
#include <map>
#include <string>
#include "tools.h"
#include "mfdpage.h"
#include "fms.h"

class mfdpage_fms_t : public tools_t, public mfdpage_type_t, public fms_utils_t
{
public:
    mfdpage_fms_t(string name);
	mfdpage_fms_t(string name, fms_t* fms);
    string get_template(string name,std::map<int, std::string> &sources);
	void do_refresh(object_t* source);
	int refresh_interval(string name);
    string refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template);
private:
	fms_t* a_fms_ref;
	string a_name;
};

#endif /* MFDPAGE_FMS_H */
