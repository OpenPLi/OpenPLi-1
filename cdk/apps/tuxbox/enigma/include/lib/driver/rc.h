#ifndef __rc_h
#define __rc_h

#include <config.h>
#include <unistd.h>
#include <list>
#include <map>

#include <lib/base/ebase.h>
#include <lib/base/estring.h>
#include <libsig_comp.h>
#include <linux/input.h>
#include <lib/driver/input_fake.h>

class eRCInput;
class eRCDriver;
class eRCKey;

	// the offset of plain ascii codes coming out of the console (only when in keyboard mode)
#define KEY_ASCII     0x1000

/**
 * \brief A remote control.
 *
 * Handles one remote control. Gets codes from a \ref eRCDriver. Produces events in \ref eRCInput.
 */
class eRCDevice: public Object
{
protected:
	eRCInput *input;
	eRCDriver *driver;
	eString id;
public:
	/**
	 * \brief Constructs a new remote control.
	 *
	 * \param id The identifier of the RC, for use in settings.
	 * \param input The \ref eRCDriver where this remote gets its codes from.
	 */
	eRCDevice(const eString &id, eRCDriver *input);
	~eRCDevice();
	/**
	 * \brief Handles a device specific code.
	 *
	 * Generates events in \ref eRCInput. code is highly device- and driver dependant.
	 * For Example, it might be 16bit codes with one bit make/break or special codes
	 * for repeat.
	 */
	virtual void handleCode(int code)=0;
	/**
	 * \brief Get user readable description.
	 * \result The description.
	 */
	virtual const char *getDescription() const=0;
	/**
	 * \brief Get a description for a specific key.
	 * \param key The key to get the description for.
	 * \result User readable description of given key.
	 */
	const eString getIdentifier() const { return id; }
	/**
	 * \brief Get the identifier for this device.
	 * \result User readable identifier of device.
	 */
	virtual const char *getKeyDescription(const eRCKey &key) const=0;
	/**
	 * \brief Get an input device keycode.
	 *
	 * \param key The key to get the input code for.
	 * \result The Linux input device keycode
	 */
	virtual int getKeyCompatibleCode(const eRCKey &key) const;
	const eRCDriver *getDriver() { return driver; }
};

/**
 * Receives codes from one or more remote controls.
 */
class eRCDriver: public Object
{
protected:
	std::list<eRCDevice*> listeners;
	eRCInput *input;
	int enabled;
public:
	/**
	 * \brief Constructs a driver.
	 *
	 * \param input The RCInput to bind this driver to.
	 */
	eRCDriver(eRCInput *input);
	/**
	 * \brief Get pointer to key-consumer.
	 */
	eRCInput *getInput() const { return input; }
	/**
	 * \brief Adds a code lister
	 */
	void addCodeListener(eRCDevice *dev)
	{
		listeners.push_back(dev);
	}
	void removeCodeListener(eRCDevice *dev)
	{
		listeners.remove(dev);
	}
	~eRCDriver();
	
	void enable(int en) { enabled=en; }

	virtual void flushBuffer() const {};
	virtual void lock() const {};
	virtual void unlock() const {};
};

#if HAVE_DVB_API_VERSION < 3
class eRCShortDriver: public eRCDriver
{
protected:
	int handle;
	eSocketNotifier *sn;
	void keyPressed(int);
	void init_eRCShortDriver(const char *filename);
public:
	eRCShortDriver(const char *filename);
	~eRCShortDriver();
	void flushBuffer() const
	{
		__u16 buf;
		if (handle != -1)
			while ( ::read(handle, &buf, 2) == 2 );
	}
	void lock() const
	{
		if ( sn )
			sn->stop();
	}
	void unlock() const
	{
		if ( sn )
			sn->start();
	}
};
#endif

class eRCInputEventDriver: public eRCDriver
{
	void init_eRCInputEventDriver(const char *filename);
protected:
	int handle;
	eSocketNotifier *sn;
	void keyPressed(int);
public:
	eString getDeviceName();
	eRCInputEventDriver(const char *filename);
	~eRCInputEventDriver();
	void flushBuffer() const
	{
		struct input_event ev;
		if (handle != -1)
			while ( ::read(handle, &ev, sizeof(struct input_event)) == sizeof(struct input_event) );
	}
	void lock() const
	{
		if ( sn )
			sn->stop();
	}
	void unlock() const
	{
		if ( sn )
			sn->start();
	}
};

class eRCKey
{
public:
	eRCDevice *producer;
	int code, flags;
	eString picture;

	eRCKey(eRCDevice *producer, int code, int flags, eString picture=""):
		producer(producer), code(code), flags(flags), picture(picture)
	{
	}
	enum
	{
		flagBreak=1,
		flagRepeat=2
	};
	
	bool operator<(const eRCKey &r) const
	{
		if (producer > r.producer)
			return 0;
		if (producer < r.producer)
			return 1;
		
		if (code > r.code)
			return 0;
		if (code < r.code)
			return 1;
			
		if (flags > r.flags)
			return 0;
		if (flags < r.flags)
			return 1;

		return 0;
	}
};

class eRCConfig
{
public:
	eRCConfig();
	~eRCConfig();
	void reload();
	void save();
	void set(int delay, int repeat);
	int rdelay, // keypress delay after first keypress to begin of repeat (in ms)
		rrate;		// repeat rate (in ms)
};

class eRCInput: public Object
{
	int locked;	
	int handle;
	static eRCInput *instance;
	int keyboardMode;
public:
	struct lstr
	{
		bool operator()(const eString &a, const eString &b) const
		{
			return a<b; 
		}
	};

protected:
	std::map<eString,eRCDevice*,lstr> devices;
public:
	Signal1<void, const eRCKey&> keyEvent;
	eRCInput();
	~eRCInput();
	
	int lock();
	void unlock();
	int islocked() { return locked; }
	void close();
	bool open();

	void setFile(int handle);
	
	
	/* This is only relevant for "keyboard"-styled input devices,
	   i.e. not plain remote controls. It's up to the input device
	   driver to decide wheter an input device is a keyboard or
	   not.
	   
	   kmNone will ignore all Ascii Characters sent from the 
	   keyboard/console driver, only give normal keycodes to the
	   application.
	   
	   kmAscii will filter out all keys which produce ascii characters,
	   and send them instead. Note that Modifiers like shift will still
	   be send. Control keys which produce escape codes are send using
	   normal keycodes. 
	   
	   kmAll will ignore all keycodes, and send everything as ascii,
	   including escape codes. Pretty much useless, since you should
	   lock the console and pass this as the console fd for making the
	   tc* stuff working.
	*/
	
	enum { kmNone, kmAscii, kmAll };
	void setKeyboardMode(int mode);
	int  getKeyboardMode() { return keyboardMode; }
	void loadKeyboardMapping();

	void keyPressed(const eRCKey &key)
	{
		/*emit*/ keyEvent(key);
	}
	
	void addDevice(const eString&, eRCDevice *dev);
	void removeDevice(const eString &id);
	eRCDevice *getDevice(const eString &id);
	std::map<eString,eRCDevice*,lstr> &getDevices();

	static eRCInput *getInstance() { return instance; }
	
	eRCConfig config;
};

#endif
