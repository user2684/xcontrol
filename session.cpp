#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

#include "include/out.h"
#include "include/in.h"
#include "include/data.h"
#include "include/mfdpage.h"
#include "include/session.h"
#include "include/fms.h"
#include "include/gui.h"

using namespace std;
using std::map;
using std::string;
extern const char* version;

session_t::session_t(void) : a_out(0), a_in(0), a_data(0)
{
    debug_out(info, "session: X Control Enhanced plugin version %s",version);
	debug_out(debug, "session: instantiating connection to the Joystick");
    a_out = new out_t();
    debug_out(debug, "session: configuring buttons in x-plane");
    a_in = new in_t();
    debug_out(info, "session: establishing data connection");
    a_data = new data_t();
    debug_out(debug, "session: configuring the FMS");
    a_fms = new fms_t();
    debug_out(debug, "session: creating MFD pages");
    a_mfdpages = new mfdpages_t(a_out,a_in,a_data,a_fms);
    a_mfdpages->load(); // create default pages
    a_gui = new gui_t(a_mfdpages,a_fms,a_out);
}

session_t::~session_t(void)
{
	debug_out(debug, "session: deleting session");
	delete a_mfdpages;
    delete a_in;
    delete a_data;
	delete a_gui;
	delete a_fms;
    delete a_out;
	debug_out(info, "session: exited");
}

int session_t::enable(void)
{
    a_data->connect(0.5f);
    a_gui->enable();
    return 1;
}

void session_t::disable(void)
{
    a_data->disconnect();
	a_gui->disable();
}

