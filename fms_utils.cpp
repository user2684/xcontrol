#include <stdio.h>
#include <stdarg.h>
#include "fms_utils.h"
#include <iostream>
#include <fstream>
#include <math.h>
#define pi 3.14159265358979323846

using namespace std;
using std::map;
using std::string;

fms_utils_t::fms_utils_t(void)
{
}

// convert a time string into minutes
float fms_utils_t::time2minutes(string time) {
	int hrs = 0;
	int mins = 0;
	sscanf(time.c_str(), "%d:%d", &hrs, &mins);
	float minutes = hrs*60+mins;
	//debug_out(verbose,"fms_utils: converted time %s into %f minutes",time.c_str(),minutes);
	return minutes;
}

// convert a number of minutes since 00:00 into a string
string fms_utils_t::minutes2time(float minutes) {
	if (minutes == -1 || minutes == 0) return "__:__";
    int hours = minutes/60;
    int mins = minutes-(hours*60);
	if (hours > 23) hours -= 24;
	if (hours < 0) hours += 24;
	char buffer[130];
	string time;
	snprintf(buffer, 130, "%02i:%02i", hours,mins);
	time.assign(buffer);
	//debug_out(verbose,"fms_utils: converted minutes %f into time %s",minutes,time.c_str());
	return time;
}

// convert zulu time (secs) in a time string
string fms_utils_t::zulu2time(float zulu_time){
	return  minutes2time(zulu2minutes(zulu_time));
}

// convert zulu time (secs) into minutes
float fms_utils_t::zulu2minutes(float zulu_time){
	if (zulu_time == 0) return 0;
	int secs = (int) zulu_time;
    int hours = secs/3600;
    int mins = (secs%3600)/60;
	int minutes = hours*60+mins;
	return  minutes;
}

// convert nautical miles into kilometers
float fms_utils_t::nm2km(float nm) {
	return nm*1.852;
}

// convert knotes per second into kilometer per minute
float fms_utils_t::kts2kmm(float kts) {
	return kts*0.03087;
}

// convert m/s into knotes 
float fms_utils_t::ms2kts(float ms) {
	return ms*1.944;
}

// converts decimal degrees to radians 
double fms_utils_t::deg2rad(double deg) {
  return (deg * pi / 180);
}
 
// converts radians to decimal degrees
double fms_utils_t::rad2deg(double rad) {
  return (rad * 180 / pi);
}

// convert a given altitude to a nice string
string fms_utils_t::altitude2string(float altitude) {
	char temp[2048] = {};
	string result;
	memset(temp, 0, 2048);
	if (altitude <100 ) return "-";
	if (altitude > 7000) snprintf(temp, 2048, "FL%2.0f0",altitude/1000); // assuming TA = FL70
	else snprintf(temp, 2048, "%2.0f00f",altitude/100); // below TA
	result.assign(temp);
	if (result == "00") return "-";
	return result;
}

// convert a time delay in a nice string
string fms_utils_t::delay2string(float minutes) {
	char temp[2048] = {};
	string result;
	memset(temp, 0, 2048);
	if (minutes == 0 || minutes > 99 || minutes < -99) return "-";
	if (minutes >= 1) snprintf(temp, 2048, "+%.0f",minutes);
	else if (minutes <= -1) snprintf(temp, 2048, "-%.0f",fabs(minutes));
	else snprintf(temp, 2048, "0");
	result.assign(temp);
	return result;
}
