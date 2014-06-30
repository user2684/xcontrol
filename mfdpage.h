#ifndef MFDPAGE_H
#define MFDPAGE_H

#include <time.h>
#include <map>
#include <string>
#include "tools.h"
#include "fms.h"
#include "in.h"

class out_t;
class data_t;
class datasource_t;
class mfdpage_type_t;

class mfdpages_t : public tools_t
{
public:
	mfdpages_t(out_t* joystickout,in_t* joystickin,data_t* dataconnection,fms_t* fms_ref);
	~mfdpages_t(void);
	void load(void);
private:
    out_t* a_joystickout;
    in_t* a_joystickin;
    data_t* a_dataconnection;
	fms_t* a_fms_ref;
};

class mfdpage_type_t
{
public:
    virtual string get_template(string name,std::map<int, std::string> &sources)=0;
	virtual void do_refresh(object_t* source)=0;
    virtual string refresh_template(string name,std::map<int, object_t*> a_datasources,string a_template)=0;
	virtual int refresh_interval(string name)=0;
	std::map<string,char> a_time;
	std::map<string,int> a_date;
    time_t last_update;
private:
};

class mfdpage_t : public listener_t, public tools_t
{
public:
    mfdpage_t(const char* name, out_t* outputdevice, data_t* dataconnection, int button,string type,fms_t* fms_ref);
    ~mfdpage_t(void);
    const std::string& name(void);
    void set_datasources(std::map<int, std::string>* sources);
	void remove_datasources(void);
    void set_active(bool active);
    bool has_object(object_t* source);
    bool refresh(object_t* source);
    void final_refresh(void);
    time_t last_page_update;
	int a_button;
private:
    bool a_visible;
	string a_type;
    out_t* a_outdevice;
    data_t* a_dataconnection;
    std::string a_data;
    std::string a_name;
    std::string a_template;
    std::map<int, object_t*> a_datasources;
	mfdpage_type_t* a_page;
	fms_t* a_fms_ref;
};


#endif /* MFDPAGE_H */
