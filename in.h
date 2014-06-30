#ifndef IN_H
#define IN_H

#include <set>
#include "tools.h"

class mfdpage_t;

class in_t : public tools_t
{
public:
    in_t(void);
    ~in_t(void);
    void handle_pagecycle(int button);
    void add_page(mfdpage_t* page);
	void delete_all_pages(void);
    static int dispatch_command(void* cmd, int phase, void *arg);
    void* a_cmd_pagecycle_1;
	void* a_cmd_pagecycle_2;
	void* a_cmd_pagecycle_3;
	void* a_cmd_pagecycle_4;
private:
	std::set<mfdpage_t*> a_pages;
    mfdpage_t* a_currentpage;
};

#endif /* IN_H */
