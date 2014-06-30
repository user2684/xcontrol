#ifndef DATA_H
#define DATA_H

#include <set>
#include <string>
#include "tools.h"

class mfdpage_t;

class datasource_t : public object_t, public tools_t
{
public:
    datasource_t(const char* ref);
    ~datasource_t(void);

    const std::string& name(void);
    bool newdata_available(void);
    void mark_dirty(void);
    operator int(void);
    operator float(void);
    operator double(void);
    operator const std::string&(void);
    operator const char*(void);
    bool refresh(void);
	int getType(void);

private:
    std::string a_name;
    void*  a_ref;
    int   a_reftype;
    bool  a_newdata;
    bool  a_isdirty;
    int   a_dint;
    float  a_dfloat;
    double  a_ddouble;
    std::string a_dstring;
};

class data_t : public provider_t, public tools_t
{
public:
    data_t(void);
    ~data_t(void);

    void add_listener(listener_t* listener);
    void remove_listener(listener_t* listener);
    object_t* add_datasource(const char* ref);
    void remove_datasource(object_t* source);
    void remove_datasource(const char* ref);
    void connect(float interval);
    void disconnect();
    const float& upd_interval(void);

private:
    static float update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg);
    void refresh_datasources(void);
    void refresh_listeners(object_t* updated_source);
    std::set<listener_t*> a_listeners;
    std::set<object_t*> a_datasources;
    std::set<listener_t*> a_wantsupdate;
    float a_interval;
};

#endif /* DATA_H */
