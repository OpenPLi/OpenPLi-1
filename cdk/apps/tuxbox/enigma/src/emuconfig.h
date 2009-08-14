#ifndef _emuconfig_h
#define _emuconfig_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <callablemenu.h>

#include <common.h>

class eLabel;
class eButton;
class eStatusBar;

class eEmuConfig : public eWindow, public eCallableMenu
{
	private:
		struct selectListEntry : public std::unary_function<const eListBoxEntryText&, void>
		{
			const void *key;
			const char *szKey;
			eComboBox *lb;
			typedef enum
			{
				TEXTSEARCH,
				OTHER
			} eSearchMethod;

			eSearchMethod searchmethod;

			selectListEntry(const char* szKey, eComboBox* lb): key(0), szKey(szKey), lb(lb)
			{
				searchmethod = TEXTSEARCH;
			}
			selectListEntry(const void* key, eComboBox* lb): key(key), szKey(0), lb(lb)
			{
				searchmethod = OTHER;
			}

			bool operator()(const eListBoxEntryText& s)
			{
				switch (searchmethod)
				{
					case TEXTSEARCH:
						if (szKey)
						{
							if (s.getText() == szKey)
							{
								lb->setCurrent(&s);
								return 1;
							}
						}
						break;
					case OTHER:
						if (key == (void *)s.getKey())
						{
							lb->setCurrent(&s);
							return 1;
						}
						break;
				}
				return 0;
			}
		};

		int emudSocket;

		int activecardserver;

		eLabel *lb_defemu;
		eComboBox *lbx_Emulator, *lbx_stick_serv, *lbx_stick_prov, *lbx_cardserver;

		eLabel *lb_service;
		eLabel *lb_provider;
		eLabel *lb_caids;
		eLabel *lb_caids2;
		eLabel *lb_cardserver;
		eCheckbox *chk_reconnect;
		eCheckbox *chk_twoservices;
		eButton *bt_save;
		eButton *bt_saveandrestart;
		eButton *bt_restartcardserver;
		eButton *bt_list;
		eButton *bt_cardinfo;
		eStatusBar *sbStatus;

		void enumSettings();
		void saveSettings(bool restart = false);
		void saveCardserver();
		void stickService();
		void stickProvider();
		void selected(eListBoxEntryText *l);
		void restartCardserver();
		void getCardInfo();

		void populateList();
		char * sendMessage(char *msg);

		int writeToSocket(int socket, const void *data, int size);
		int readFromSocket(int socket, void *data, int size);
		int commandExchange(int socket, int cmd, const void *txdata, int txsize, void *rxdata, int rxsize);

	public:
		int transferEmudCommand(int cmd, const void *txdata = NULL, int txsize = 0, void *rxdata = NULL, int rxsize = 0);
		eEmuConfig();
		~eEmuConfig();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class eEmuStickyList: public eWindow
{
	protected:
		eListBox<eListBoxEntryText> *listb;

	public:
		eEmuStickyList(eEmuConfig *eEmuConf);
		~eEmuStickyList();
};

#endif
