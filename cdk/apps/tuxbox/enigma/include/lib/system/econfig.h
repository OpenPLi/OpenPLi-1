#ifndef __econfig_h
#define __econfig_h

// #include <lib/system/nconfig.h>

#include <map>
#include <lib/base/estring.h>

class eConfig // : public NConfig
{
	static eConfig *instance;
	
	std::map<eString, int> keys_int;
	std::map<eString, eString> keys_string;
	std::map<eString, unsigned int> keys_uint;
	std::map<eString, double> keys_double;
	void init_eConfig();	
public:
	static eConfig *getInstance() { return instance; }
	
	int getKey(const char *, int &);
	int getKey(const char *, unsigned int &);
	int getKey(const char *, double &);
	int getKey(const char *, char * &string);
	int getKey(const char *, eString &);
	
	int setKey(const char *, const int &);
	int setKey(const char *, const unsigned int &);
	int setKey(const char *, const double &);
	int setKey(const char *, const char *);
	int setKey(const char *, const eString &);
	
	void delKey(const char *);
	
	void flush();
	
	eConfig();
	~eConfig();
};

class eSimpleConfigFile
{
protected:
	std::map<eString, eString> config;

public:
	eSimpleConfigFile(const char *filename);
	eString getInfo(const char *info);
	void setInfo(const char *info, const char* value);
	void Save(const char *filename);
};

#endif
