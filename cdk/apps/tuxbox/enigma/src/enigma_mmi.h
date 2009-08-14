#ifndef __ENIGMA_MMI_H_
#define __ENIGMA_MMI_H_

#include <lib/gui/listbox.h>
#include <lib/base/message.h>
#include <lib/base/buffer.h>

class eNumber;
class eLabel;
class eTextInputField;
class eWindow;
class eStatusBar;

struct eMMIMsg
{
	char *data;
	int len;
	eMMIMsg()
	{
	}
	eMMIMsg(char* data, int len)
		: data(data), len(len)
	{
	}
};

class eMMIEnqWindow : public eWindow
{
	eLabel *title;
	eNumber *input;
	int num;
	int eventHandler( const eWidgetEvent &e );
	void okPressed(int*);
	void init_eMMIEnqWindow( const eString &titleBarText, const eString &text, bool blind );
public:
	eMMIEnqWindow( const eString& titleBarText, const eString &windowText, int num, bool blind );
	eString getAnswer();
};

class eMMIListWindow : public eListBoxWindow<eListBoxEntryText>
{
	eLabel *title, *subtitle, *bottomText;
	int eventHandler( const eWidgetEvent &e );
	void init_eMMIListWindow(const eString & titleBarText, const eString &titleTextT, const eString &subtitleTextT, const eString &bottomTextT, std::list< std::pair< eString, int> > &entrys);
public:
	void entrySelected( eListBoxEntryText* e );
	eMMIListWindow(const eString &titleBarText, const eString &title, const eString &subtitle, const eString &bottomText, std::list< std::pair< eString, int> > &entrys );
	int getSelected() { return (int) list.getCurrent()->getKey(); }
};

class enigmaMMI : public eWindow
{
	void init_enigmaMMI();
protected:
	eIOBuffer buffer;
	eFixedMessagePump<eMMIMsg> mmi_messages;
	eWidget *open;
	Connection conn;
	eLabel *lText;
	eTimer responseTimer, delayTimer, closeTimer;
	const char *scheduledData;
	void closeMMI();
	enum AnswerType { ENQAnswer, MENUAnswer, LISTAnswer };
	enigmaMMI();
public:
	bool connected() { return conn.connected(); }
	int eventHandler( const eWidgetEvent &e );
	virtual bool handleMMIMessage(const char *data);
	void gotMMIData(const char* data, int);
	void handleMessage( const eMMIMsg &msg );
	void showWaitForAnswer(int ret);
	void hideWaitForAnswer();
	virtual void beginExec() { }
	virtual void endExec() { }
	virtual void sendAnswer( AnswerType ans, int param, unsigned char *data )=0;
	void haveScheduledData();
};

#endif //__ENIGMA_MMI_H_
