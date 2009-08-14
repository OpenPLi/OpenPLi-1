#include <lib/gui/emessage.h>

class enigmaVCR : public eMessageBox
{
		static enigmaVCR *instance;
		void init_enigmaVCR();
	public:
		static enigmaVCR* getInstance() { return instance; }
		enigmaVCR(eString string, eString caption);
		~enigmaVCR();
		int eventHandler(const eWidgetEvent &event);
		void volumeUp();
		void volumeDown();
		void switchBack();
};
