#ifdef ENABLE_FLASHTOOL
/**********************************************
*
*	$Revision: 1.4 $
*
**********************************************/

#ifndef __flashtool_h__
#define __flashtool_h__

#include <lib/gui/ewidget.h>
#include <lib/gui/listbox.h>


class eFlashtool: public eListBoxWindow<eListBoxEntryText>
{
private:
	int flashimage;
	int mtd;

	void sel_item(eListBoxEntryText *sel);
	bool erase();
	bool readmtd(char destination[]);
public:
	void programm(char filename[]);

	eFlashtool(int direction);
	~eFlashtool();
};


class eFlashtoolMain: public eListBoxWindow<eListBoxEntryText>
{
private:
	void sel_item(eListBoxEntryText *sel);
	void init_eFlashtoolMain();
public:
	eFlashtoolMain();
	~eFlashtoolMain();
};


class eFlashtoolImageView: public eListBoxWindow<eListBoxEntryText>
{
private:
	int fmtdnr;
	char buffer[100];

	void sel_item(eListBoxEntryText *sel);
public:
	~eFlashtoolImageView();
	eFlashtoolImageView(char folder[]);
	char* getFilename();
};


class eFlashtoolSource: public eListBoxWindow<eListBoxEntryText>
{
private:
	bool flash;
	char buffer[100];

	void sel_item(eListBoxEntryText *sel);
public:
	~eFlashtoolSource();
	eFlashtoolSource(int direction);
	char* getDestination();
};


#endif /* __flashtool_h__ */
#endif // ENABLE_FLASHTOOL
