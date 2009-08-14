#include <errno.h>
#include <lib/gui/multipage.h>
#include <lib/gui/ewidget.h>

eMultipage::eMultipage()
{
}

int eMultipage::prev()
{
	if (list.current() == list.begin())
		return -ENOENT;
	if (list.current() != list.end())
		list.current()->hide();
	list.prev();
	list.current()->show();
 return 0;
}

int eMultipage::next()
{
	if (list.current() == *--list.end())
		return -ENOENT;
	list.current()->hide();
	list.next();
	if (list.current() == list.end())
		return 0;
	list.current()->show();
	return 0;
}

void eMultipage::set(eWidget *widget)
{
	if (list.current() == widget)
		return;
	if (list.current() != list.end())
		list.current()->hide();
	list.setCurrent(widget);
	if (list.current() != list.end())
		list.current()->show();
}

void eMultipage::first()
{
	if (list.current() == list.begin())
		return;
	if (list.current() != list.end())
		list.current()->hide();
	list.first();
	if (list.current() != list.end())
		list.current()->show();
}

int eMultipage::at()
{
	int num=0;
	for (ePtrList<eWidget>::iterator i(list.begin()); (i != list.end()) && (i != list.current()); ++i, ++num)	;
	return num;
}

void eMultipage::addPage(eWidget *page)
{
	list.push_back(page);
}
