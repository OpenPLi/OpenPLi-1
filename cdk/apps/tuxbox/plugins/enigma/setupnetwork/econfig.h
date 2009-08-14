#ifndef __econfig_h
#define __econfig_h

#include <map>
#include <string>

class eConfig // : public NConfig
{
private:
	std::map<std::string, int> keys_int;
	std::map<std::string, std::string> keys_string;
	std::map<std::string, unsigned int> keys_uint;
	
public:
	
	int getKey(const char *, int &);
	int getKey(const char *, unsigned int &);
	int getKey(const char *, char * &string);
	
	eConfig();
	~eConfig();
};

#endif
