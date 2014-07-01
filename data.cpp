#include <sstream>

#include "include/SDK/XPLMProcessing.h"
#include "include/SDK/XPLMDataAccess.h"
#include "include/data.h"
#include "include/mfdpage.h"

using std::set;

data_t::data_t(void)
{
}

data_t::~data_t(void)
{
    for (set<object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        delete *it;
    }
    debug_out(debug, "data: deleted data connection (self: %p)", this);
}

const float& data_t::upd_interval(void)
{
    return a_interval;
}

void data_t::connect(float interval)
{
    a_interval = interval;
    XPLMRegisterFlightLoopCallback(update, interval, this);
    debug_out(debug, "data: engaged data connection (self: %p)", this);
}

void data_t::disconnect()
{
    XPLMUnregisterFlightLoopCallback(update, this);
    debug_out(debug, "data: deactivating data connection with x-plane (self: %p)", this);
}

void data_t::add_listener(listener_t* listener)
{
    if (!listener) return;
    if (a_listeners.insert(listener).second)
    {
        debug_out(verbose, "data: added listener: {%s} (self: %p)", listener->name().c_str(), listener);
    }
}

void data_t::remove_listener(listener_t* listener)
{
    if (!listener) return;
    if (a_listeners.erase(listener))
    {
        debug_out(verbose, "data: removed listener: {%s} (self: %p)", listener->name().c_str(), listener);
    }
}

object_t* data_t::add_datasource(const char* ref)
{
    if (!ref) return 0;
    datasource_t* source = 0;
	source = new datasource_t(ref);
	for (set<object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it) {
		if ((*it)->name().c_str() == source->name()) { // check if the datasource has already been added
			debug_out(verbose, "data: datasource '%s' already added", source->name().c_str());
			return *it;
		}
    }
     if (a_datasources.insert(source).second) {
            debug_out(verbose, "data: added datasource '%s' (self: %p)", source->name().c_str(), source);
    }
    return source;
}

void data_t::remove_datasource(object_t* source)
{
    if (!source) return;
    if (a_datasources.erase(source))
    {
        debug_out(verbose, "data: removed datasource '%s' (self: %p)", source->name().c_str(), source);
        delete source;
    }
}

void data_t::remove_datasource(const char* ref)
{
    if (!ref) return;
    for (set<object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it)->name() == ref)
            remove_datasource(*it);
    }
}


/* private members */

void data_t::refresh_datasources(void) // firstly called by xplane
{
    a_wantsupdate.clear();
    for (set<object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it)->refresh()) // refresh each datasource value
        {
            debug_out(verbose, "data: '%s' type %d reported new data (%s)", (*it)->name().c_str(), (*it)->getType(),(const char*)*(*it));
            refresh_listeners(*it); // refresh all the listeners with this datasource
        }
    }
    for (set<listener_t*>::iterator it = a_wantsupdate.begin(); it != a_wantsupdate.end(); ++it) // populated by refresh_listeners
    {
        (*it)->final_refresh(); 
    }
}

void data_t::refresh_listeners(object_t* updated_source)
{
    int not_orphaned = 0;
    for (set<listener_t*>::iterator it = a_listeners.begin(); it != a_listeners.end(); ++it) // for each page
    {
        if ((*it)->has_object(updated_source)) // if the page is expecting updates for this datasource
        {
            not_orphaned++;
            if ((*it)->refresh(updated_source)) // mfdpage refresh
                a_wantsupdate.insert(*it); // if returned true (visible), schedule it for the final_refresh
        }
    }
    if (!not_orphaned) // nobody wants this datasource
        remove_datasource(updated_source);
}

// static member, called by X-Plane
float data_t::update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg)
{
    data_t* me = reinterpret_cast<data_t*>(arg);
    me->refresh_datasources();
    return me->upd_interval();
}


/*
** datasource_t
*/

datasource_t::datasource_t(const char* ref) : a_name(ref), a_newdata(true)
{
    a_ref = XPLMFindDataRef(ref);
    if (!a_ref)
        throw "data source not found";

    a_reftype = XPLMGetDataRefTypes(a_ref);
    if (!a_reftype)
        throw "data type not applicable";
    a_dint = 0;
    a_dfloat = 0;
    a_ddouble = 0;
    a_dstring.clear();
}

datasource_t::~datasource_t(void)
{
    debug_out(verbose, "data: deleted data source '%s' (self: %p)", a_name.c_str(), this);
}

const std::string& datasource_t::name(void)
{
    return a_name;
}

bool datasource_t::newdata_available(void)
{
    return a_newdata;
}

void datasource_t::mark_dirty(void)
{
    a_isdirty = true;
}

// we use standard casts for now, i.e. no exact rounding

datasource_t::operator int(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

datasource_t::operator double(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

datasource_t::operator float(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

datasource_t::operator const std::string&(void)
{
    std::ostringstream os;
    switch (a_reftype)
    {
    case xplmType_Int:
        os << a_dint;
        break;
    case xplmType_Float:
        os << a_dfloat;
        break;
    case xplmType_Double:
        os << a_ddouble;
        break;
	case xplmType_Data:
		return a_dstring;
		break;
    }
    a_dstring.assign(os.str());
    return a_dstring;
}

datasource_t::operator const char*(void)
{
    std::ostringstream os;
    switch (a_reftype)
    {
    case xplmType_Int:
        os << a_dint;
        break;
    case xplmType_Float:
        os << a_dfloat;
        break;
    case xplmType_Double:
        os << a_ddouble;
        break;
	case xplmType_Data:
		return a_dstring.c_str();
		break;
    }
    a_dstring.assign(os.str());
    return a_dstring.c_str();
}

/* private members */

bool datasource_t::refresh(void)
{
    int   dint;
    float  dfloat;
    double  ddouble;
    char	tdstring[256];
	std::string dstring;
    a_newdata = false;

    switch (a_reftype)
    {
    case xplmType_Int:
        if (((dint = XPLMGetDatai(a_ref)) != a_dint) || a_isdirty)
        {
            a_dint = dint;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_Float:
        if (a_name == "sim/time/local_time_sec" || a_name == "sim/time/zulu_time_sec")
        {
            if (((dint = XPLMGetDataf(a_ref)) != (int)a_dfloat) || a_isdirty)
            {
                a_dfloat = dint;
                a_isdirty = false;
                return (a_newdata = true);
            }
            else return false;
        }
        else if (((dfloat = XPLMGetDataf(a_ref)) != a_dfloat) || a_isdirty)
        {
            a_dfloat = dfloat;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_Double:
        if (((ddouble = XPLMGetDatad(a_ref)) != a_ddouble) || a_isdirty)
        {
            a_ddouble = ddouble;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_FloatArray:
        return false;
    case xplmType_IntArray:
        return false;
    case xplmType_Data:
			XPLMGetDatab(a_ref, tdstring, 0, sizeof(tdstring)-1);
			dstring.assign(tdstring);
        if (dstring != a_dstring || a_isdirty)
        {
            a_dstring = dstring;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_Unknown:
        return false;
    default:
        return false;
    }
}


int datasource_t::getType(void)
{
	return a_reftype;
}
