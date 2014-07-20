#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <map>


using namespace std;

class tools_t
{
public:
    tools_t(void);
    void debug_out(int type, const char* msg, ...);
    static void debug_out(const char* msg, ...);
    enum dbgtype_e
    {
        all,
        verbose,
		debug,
        info,
        warn,
        err
    };
private:
	int a_debug;	
};

class object_t
{
public:
    virtual ~object_t(void){};
    virtual const std::string& name(void)=0;
    virtual bool refresh(void)=0;
    virtual void mark_dirty(void)=0;
    virtual operator int(void)=0;
    virtual operator float(void)=0;
    virtual operator double(void)=0;
    virtual operator const std::string&(void)=0;
    virtual operator const char*(void)=0;
	virtual int getType(void)=0;

};

class provider_t
{
public:
    virtual ~provider_t(void){};
private:
private:
    virtual void refresh_datasources(void)=0;
    virtual void refresh_listeners(object_t* updated_source)=0;
};

class listener_t
{
public:
    virtual ~listener_t(void){};
    virtual const std::string& name(void)=0;
    virtual bool has_object(object_t* obj)=0;
    virtual bool refresh(object_t* obj)=0;
    virtual void final_refresh(void)=0;
};

#endif /* TOOLS_H */
