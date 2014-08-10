#include "include/SDK/XPLMUtilities.h"
#include "include/mfdpage.h"
#include "include/in.h"

using std::set;

in_t::in_t(void) : a_currentpage(0)
{
    a_cmd_pagecycle_1 = XPLMCreateCommand("sim/instruments/xcontrol_cycle_button_1", "cycles through button 1 datapages on the display");
    if (!a_cmd_pagecycle_1)  debug_out(err,"in: wasn't able to create custom xcontrol_cycle_button_1 X-Plane commands");
    XPLMRegisterCommandHandler(a_cmd_pagecycle_1, dispatch_command, 0, this);
    a_cmd_pagecycle_2 = XPLMCreateCommand("sim/instruments/xcontrol_cycle_button_2", "cycles through button 2 datapages on the display");
    if (!a_cmd_pagecycle_2)  debug_out(err, "in: wasn't able to create custom xcontrol_cycle_button_2 X-Plane commands");
    XPLMRegisterCommandHandler(a_cmd_pagecycle_2, dispatch_command, 0, this);
    a_cmd_pagecycle_3 = XPLMCreateCommand("sim/instruments/xcontrol_cycle_button_3", "cycles through button 3 datapages on the display");
    if (!a_cmd_pagecycle_3)  debug_out(err, "in: wasn't able to create custom xcontrol_cycle_button_3 X-Plane commands");
    XPLMRegisterCommandHandler(a_cmd_pagecycle_3, dispatch_command, 0, this);
    a_cmd_pagecycle_4 = XPLMCreateCommand("sim/instruments/xcontrol_cycle_button_4", "cycles through button 4 datapages on the display");
    if (!a_cmd_pagecycle_4)  debug_out(err, "in: wasn't able to create custom xcontrol_cycle_button_4 X-Plane commands");
    XPLMRegisterCommandHandler(a_cmd_pagecycle_4, dispatch_command, 0, this);
}

in_t::~in_t(void)
{
	delete_all_pages();
}

void in_t::delete_all_pages(void)
{
	if (! a_pages.empty()) {
		for (set<mfdpage_t*>::iterator it = a_pages.begin(); it != a_pages.end(); ++it) delete *it;
		a_pages.clear();
        debug_out(debug, "in: deleted input handler (self: %p)", this);
	}
}

void in_t::add_page(mfdpage_t* page)
{
    if (!page) return;
    if (a_pages.insert(page).second)
    {
        a_currentpage = page;
        debug_out(debug, "in: added page to inputhandler: {%s} (self: %p)", page->name().c_str(), page);
    }
    for (set<mfdpage_t*>::iterator it = a_pages.begin(); it != a_pages.end(); ++it)
    {
        (*it)->set_active(false);
    }
    a_currentpage->set_active(true);
}

void in_t::handle_pagecycle(int button)
{	
    set<mfdpage_t*>::iterator it = a_pages.find(a_currentpage);
    debug_out(verbose,"in: handling page change from button %i",button);
    if (it == a_pages.end()) {
          debug_out(debug,"in: cannot find current page");
          return; // not found
     }
     debug_out(debug,"in: setting current page {%s} as inactive",a_currentpage->name().c_str());
     // This can be used to make the plugin crashing on purpose
     //     *((unsigned int*)0) = 0xDEAD;
     a_currentpage->set_active(false); // found, set inactive
	 
	 std::set<mfdpage_t*> a_pages_button; // temp set with only the pages belonging to the current button
     for (set<mfdpage_t*>::iterator it = a_pages.begin(); it != a_pages.end(); ++it) // cycle through all the pages
    {
        if ((*it)->a_button == button) a_pages_button.insert(*it);
    }
		
    set<mfdpage_t*>::iterator b_it = a_pages_button.find(a_currentpage); // find the current page in the current button set
    if (b_it == a_pages.end()) {
        debug_out(debug,"in: the data page requested belongs to a different button");
        b_it = a_pages_button.begin(); // currentpage is belonging to a different button, show the first item
    }
    else ++b_it;
    if (b_it == a_pages_button.end()) {
        debug_out(debug,"in: no more data pages to show for the current button, reverting to the first one");
        b_it = a_pages_button.begin();
    }
    
    a_currentpage = *b_it;
    a_currentpage->set_active(true);

    debug_out(debug, "in: switching datapage to: {%s}, button %d", a_currentpage->name().c_str(),button);
}

int in_t::dispatch_command(XPLMCommandRef cmd, XPLMCommandPhase phase, void *arg)
{
    in_t* me = reinterpret_cast<in_t*>(arg);
    if (cmd == me->a_cmd_pagecycle_1) {
        if (phase == xplm_CommandBegin) me->handle_pagecycle(1);
	}
	if (cmd == me->a_cmd_pagecycle_2) {
        if (phase == xplm_CommandBegin) me->handle_pagecycle(2);
	}
	if (cmd == me->a_cmd_pagecycle_3) {
        if (phase == xplm_CommandBegin) me->handle_pagecycle(3);
	}
	if (cmd == me->a_cmd_pagecycle_4) {
        if (phase == xplm_CommandBegin) me->handle_pagecycle(4);
	}
    return 0;
}
