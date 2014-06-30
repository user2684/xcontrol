#include <stdio.h>
#include <stdarg.h>
#include "tools.h"
#include <iostream>
#include <fstream>
#include <string>
#include "XPLM/XPLMPlanes.h"

using namespace std;
using std::map;
using std::string;

bool config_t::instanceFlag = false;
config_t* config_t::single = NULL;
config_t* config_t::getInstance()
{
    if(! instanceFlag) // singleton class
    {
        single = new config_t();
        instanceFlag = true;
		single->read_config(); // read the configuration files the first time is instantiated
		//a_tools = new tools_t();
        return single;
    }
    else
    {
        return single;
    }
}

// read all the configuration files
void config_t::read_config(void) {
	read_config_main();
	read_config_fms();
}

// read the main config
void config_t::read_config_main(void) {
	control_config = do_read_config((char *)"Resources/plugins/control/control.ini"); // load the main config file
}

// read the fms config
void config_t::read_config_fms(void) {
	string aircraft_path = get_fms_local_path(); 
	if (aircraft_path.length() > 0) control_config_fms = do_read_config((char*)aircraft_path.c_str()); // try loading the fms.ini file within the local aircraft folder
	// if not found, load the fms.ini at the plugin directory
	if (control_config_fms.empty()) control_config_fms = do_read_config((char *)"Resources/plugins/control/fms.ini");
}

// Build the fms.ini path relative to the loaded aircraft
std::string config_t::get_fms_local_path(void) {
	char XpFileName[256], XpAircraftPath[512];
	string path;
	int	planeCount;
	XPLMCountAircraft(&planeCount, 0, 0);
	if (planeCount > 0) {
		XPLMGetNthAircraftModel(0, XpFileName, XpAircraftPath);
		path = string (XpAircraftPath);
		string filename = string (XpFileName);
		size_t f = path.find(filename);
		path.replace(f, filename.length(),"fms.ini");
		debug_out(debug, "config: Current aircraft name, path and fms path: %s - %s - %s",XpFileName,XpAircraftPath,path.c_str());
	} else {
		debug_out(debug, "config: No aircraft is loaded");
	}
	return path;
}

// Read and parse a configuration file
map<string,string> config_t::do_read_config(char* file) {
    debug_out(debug,"config: Reading configuration file %s",file);
    string line;
    char _buff[1024];
    int line_count=0;
    map<string,string> config;
    ifstream cfg(file);
    if ( cfg.good() ) {
        while ( !cfg.eof()) {
            line_count++;
            cfg.getline(_buff, 1024);
            line=_buff;
            //ignore remarks
            int found=line.find("#",0);
            if (found >= 0) line.resize(found);
            if (line.length() > 0 ){
                //remove blanks
                found=line.find(" ",0);
                while ( found >= 0 ) {
                    line.replace(found,1,"");
                    found=line.find(" ",0);
                }
                found=line.find("=");
                if (found >= 0) {
                    string key=line.substr(0,found);
                    string val=line.substr(found+1);
                    config[key]=val;
                } else {
                    debug_out(warn, "config: Invalid format in configuration file %s in line %d: %s",file,line_count,line.c_str());
                }
            }
        }
        cfg.close();
    } else {
        debug_out(debug,"config: Cannot open configuration file %s",file);
    }
    return config;
}

// returns the config array
map<string,string> config_t::get_config(void) {
	return control_config;
}	

// returns the config array for the fms
map<string,string> config_t::get_config_fms(void) {
	return control_config_fms;
}	

// manually set the fms FL and GS from the plugin window
void config_t::set_fms_config(string fl,string gs,string takeoff_time) {
	debug_out(info, "config: setting new FMS options: FL %s, GS %s, takeoff time: %s",fl.c_str(),gs.c_str(),takeoff_time.c_str());
	control_config_fms["cruise_fl"] = fl;
	control_config_fms["cruise_gs"] = gs;
	control_config_fms["scheduled_takeoff"] = takeoff_time;
}	

// duplicated debug_out function since tools cannot be imported here
void config_t::debug_out(int type, const char* msg, ...)
{
	 map<string,string> config=get_config();	
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

