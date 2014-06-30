#include "include/out.h"
#include "include/in.h"
#include "include/data.h"
#include "include/mfdpage.h"
#include "include/session.h"
#include "include/fms.h"
#include "include/gui.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

using namespace std;
using std::map;
using std::string;
extern const char* version;

session_t::session_t(void) : a_joystickout(0), a_joystickin(0), a_dataconnection(0)
{
	debug_out(info, "session: Saitek  Control Enhanced plugin version %s",version);
	debug_out(debug, "session: instantiating connection to the Joystick");
    a_joystickout = new out_t();
	if (a_joystickout->a_joydev == NULL) { // joystick not found
		a_joy = 0;
		debug_out(warn, "session: no joystick found or error connecting to it");
	} else  { // joystick found
		a_joy = 1;
		debug_out(debug, "session: configuring buttons in x-plane");
		a_joystickin = new in_t();
		debug_out(info, "session: Establishing data connection");
		a_dataconnection = new data_t();
		debug_out(debug, "session: Configuring the FMS");
		a_fms = new fms_t();
		debug_out(debug, "session: Creating MFD pages");
		a_mfdpages = new mfdpages_t(a_joystickout,a_joystickin,a_dataconnection,a_fms);
		a_mfdpages->load(); // create default pages
	}
	// create the UI regardless
	a_gui = new gui_t(a_mfdpages,a_fms);
}

session_t::~session_t(void)
{
	debug_out(debug, "session: deleting session");
	delete a_mfdpages;
    delete a_joystickin;
    delete a_dataconnection;
	delete a_gui;
	delete a_fms;
    if (a_joy) delete a_joystickout;
	debug_out(info, "session: exited");
}

int session_t::enable(void)
{
    a_dataconnection->connect(0.5f);
	a_gui->enable(a_joy);
    return 1;
}

void session_t::disable(void)
{
    a_dataconnection->disconnect();
	a_gui->disable();
}

