#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include "config.h"
#include "tools.h"

using namespace std;

class config_t
{
public:
    static config_t* getInstance();
	void read_config();
	void read_config_main();
	void read_config_fms();
	map<string,string> get_config();
	map<string,string> get_config_fms();
	void set_fms_config(string fl,string gs,string takeoff_time);
    ~config_t()  { instanceFlag = false;   }
private:
    static bool instanceFlag;
    static config_t *single;
    config_t(){}
	 enum dbgtype_e
    {
		verbose,
		debug,
        info,
        warn,
        err
    };
	std::string get_fms_local_path(void);
	int a_debug;
	map<string,string> do_read_config(char* file);
	void debug_out(int type, const char* msg, ...);
	 std::map<std::string, std::string> control_config;
	std::map<std::string, std::string> control_config_fms;
};


#endif /* CONFIG_H */
