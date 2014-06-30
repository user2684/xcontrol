#ifndef FMS_UTILS_H
#define FMS_UTILS_H

#include <string>
#include <map>

using namespace std;

class fms_utils_t
{
public:
    fms_utils_t(void);
	float time2minutes(string time);
	string minutes2time(float minutes);
	string zulu2time(float zulu_time);
	float zulu2minutes(float zulu_time);
	float kts2kmm(float kts);
	float ms2kts(float ms);
	float nm2km(float nm);
	double deg2rad(double deg);
	double rad2deg(double rad);
	string altitude2string(float altitude);
	string delay2string(float minutes);
private:
};


#endif /* FMS_UTILS_H */
