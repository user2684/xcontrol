#include <stdio.h>
#include <stdarg.h>
#include "include/tools.h"
#include "include/config.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using std::map;
using std::string;

tools_t::tools_t(void)
{

	//debug_out(info,"Initializing configuration files");
}

void tools_t::debug_out(int type, const char* msg, ...)
{
	config_t * a_config = config_t::getInstance();
	 map<string,string> config=a_config->get_config();	
	a_debug = ::atoi(config["debug"].c_str());
	if ((type == debug) && (a_debug <1)) return;
	if ((type == verbose) && (a_debug<2)) return;
	FILE * log;
	log = fopen ("Resources/plugins/control/log.txt","a");
    va_list ap;
    va_start(ap, msg);
    switch (type)
    {
	case verbose:
        fprintf(log, "verbose: ");
        break;
	 case debug:
        fprintf(log, "debug: ");
        break;
    case info:
        fprintf(log, "info: ");
        break;
    case warn:
        fprintf(log, "warn: ");
        break;
    case err:
    default:
        fprintf(log, "err: ");
        break;
    }
    vfprintf(log, msg, ap);
    fprintf(log, "\n");
    va_end(ap);
    fflush(log);
	fclose(log);
}

void tools_t::debug_out(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
	FILE * log;
	log = fopen ("Resources/plugins/control/log.txt","a");
    fprintf(log, "[control]: ");
    vfprintf(log, msg, ap);
    fprintf(log, "\n");
    va_end(ap);
    fflush(log);
	fclose(log);
}



