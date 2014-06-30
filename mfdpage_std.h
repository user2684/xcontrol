#ifndef MFDPAGE_STD_H
#define MFDPAGE_STD_H

#include <time.h>
#include <map>
#include <string>
#include "mfdpage.h"
#include "tools.h"

class mfdpage_std_t : public tools_t, public mfdpage_type_t
{
public:
    mfdpage_std_t(string name);
    string get_template(string name,std::map<int, std::string> &sources);
	void do_refresh(object_t* source);
	int refresh_interval(string name);
    string refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template);
private:
	string a_name;
};

#endif /* MFDPAGE_STD_H */
