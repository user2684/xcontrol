#ifndef GUI_H
#define GUI_H

#include "tools.h"
#include "fms.h"
#include "mfdpage.h"
#include "out.h"

using namespace std;
class gui_t;

class gui_t: public tools_t, public fms_utils_t
{
public:
    gui_t(void);
    gui_t(mfdpages_t* mfdpages, fms_t* fms_ref, out_t* out_ref);
	~gui_t(void);
    static void enable(void);
	static void disable(void);
private:
	static void menu_handler(void *inMenuRef, void *inItemRef);
};

#endif /* GUI_H */
