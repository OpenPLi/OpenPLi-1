#ifndef __multipage_h_
#define __multipage_h_

#include <lib/base/eptrlist.h>
class eWidget;

class eMultipage
{
	ePtrList<eWidget> list;
public:
	eMultipage();
	
	int prev();
	int next();
	void set(eWidget *page);
	void first();
	void addPage(eWidget *page);
	eWidget *getCurrent() { return list.current(); }
	
	int count() { return list.size(); }
	int at();
};

#endif
