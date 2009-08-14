#include <lib/base/eerror.h>
#include <lib/system/econfig.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <sys/stat.h>
#include <dirent.h>

eConfig *eConfig::instance;

eConfig::eConfig()
{
	init_eConfig();
}
void eConfig::init_eConfig()
{
	if (!instance)
		instance=this;
	DIR *configdir;
	FILE *f = fopen(CONFIGDIR "/enigma/config", "r");
	if (f)
	{
		char buffer[1024];
		while (1)
		{
			if (!fgets(buffer, 1024, f))
				break;
			if (strlen(buffer) < 4)
				break;
			buffer[strlen(buffer)-1]=0;
			char *key = buffer + 2;
			char *opt = strchr(key, '=');
			if (!opt)
				continue;
			*opt++ = 0;
			
			switch(*buffer)
			{
			case 's': keys_string[key] = opt; break;
			case 'u': keys_uint[key] = strtoul(opt, 0, 0x10); break;
			case 'd':
			{
				char *endptr=0;
				keys_double[key] = strtod(opt, &endptr);
				if ( endptr && *endptr )
				{
					if ( *endptr == ',' )
						*endptr = '.';
					else if (*endptr == '.' )
						*endptr = ',';
					endptr=0;
					keys_double[key] = strtod(opt, &endptr);
					if ( endptr && *endptr )
						eDebug("failed to parse %s %s", key, opt);
				}
				break;
			}
			case 'i':
			{
				if ( sscanf(opt, "%x", &keys_int[key] ) != 1 )
				{
					if (sscanf(opt, "%x", &keys_int[key] ) != 1 )
						eDebug("couldn't parse %s", opt);
				}
				break;
			}              
			}
		}
		fclose(f);
	}
	else   if ((configdir = opendir(CONFIGDIR "/enigma")) == 0)
		  {
			if(mkdir(CONFIGDIR "/enigma", 0777) == -1)
    			{
			eFatal("error while opening configdir or creating - " CONFIGDIR "/enigma");
			}
		  }
		  else
		  {
		  closedir(configdir);
		  }
}

eConfig::~eConfig()
{
	flush();
	if (instance==this)
		instance=0;
}

int eConfig::getKey(const char *key, int &i)
{
	std::map<eString, int>::iterator it = keys_int.find(key);
	if (it == keys_int.end())
		return -1;
	i = it->second;
	return 0;
}

int eConfig::getKey(const char *key, unsigned int &ui)
{
	std::map<eString, unsigned int>::iterator it = keys_uint.find(key);
	if (it == keys_uint.end())
		return -1;
	ui = it->second;
	return 0;
}

int eConfig::getKey(const char *key, double &d)
{
	std::map<eString, double>::iterator it = keys_double.find(key);
	if (it == keys_double.end())
		return -1;
	d = it->second;
	return 0;
}

int eConfig::getKey(const char *key, char * &string)
{
	std::map<eString, eString>::iterator it = keys_string.find(key);
	if (it == keys_string.end())
		return -1;
	string = strdup(it->second.c_str());
	return 0;
}

int eConfig::getKey(const char *key, eString &string)
{
	std::map<eString, eString>::iterator it = keys_string.find(key);
	if (it == keys_string.end())
		return -1;
	string = it->second;
	return 0;
}

int eConfig::setKey(const char *key, const int &i)
{
	keys_int[key] = i;
	return 0;
}

int eConfig::setKey(const char *key, const unsigned int &ui)
{
	keys_uint[key] = ui;
	return 0;
}

int eConfig::setKey(const char *key, const double &d)
{
	keys_double[key] = d;
	return 0;
}

int eConfig::setKey(const char *key, const char *s)
{
	keys_string[key] = s;
	return 0;
}

int eConfig::setKey(const char *key, const eString &string)
{
	keys_string[key] = string;
	return 0;
}

void eConfig::delKey(const char *key)
{
	std::map<eString, int> del_keys;
	
	for (std::map<eString, int>::iterator i(keys_int.begin()); i != keys_int.end(); ++i)		
		if(strncmp(i->first.c_str(), key,  strlen(key))==0)
			del_keys[i->first.c_str()] = 1;
	for (std::map<eString, unsigned int>::iterator i(keys_uint.begin()); i != keys_uint.end(); ++i)
		if(strncmp(i->first.c_str(), key,  strlen(key))==0)
			del_keys[i->first.c_str()] = 1;
	for (std::map<eString, double>::iterator i(keys_double.begin()); i != keys_double.end(); ++i)
		if(strncmp(i->first.c_str(), key,  strlen(key))==0)
			del_keys[i->first.c_str()] = 1;
	for (std::map<eString, eString>::iterator i(keys_string.begin()); i != keys_string.end(); ++i)
		if(strncmp(i->first.c_str(), key,  strlen(key))==0)
			del_keys[i->first.c_str()] = 1;
	
	for (std::map<eString, int>::iterator i(del_keys.begin()); i != del_keys.end(); ++i)
	{
		keys_int.erase(i->first.c_str());
		keys_string.erase(i->first.c_str());
		keys_uint.erase(i->first.c_str());
		keys_double.erase(i->first.c_str());
	}
}

void eConfig::flush()
{
	FILE *f = fopen(CONFIGDIR "/enigma/config", "w");
	if (!f)
	{
		eWarning("couldn't write config!");
		return;
	}

	for (std::map<eString, int>::iterator i(keys_int.begin()); i != keys_int.end(); ++i)
		fprintf(f, "i:%s=%08x\n", i->first.c_str(), i->second);
	for (std::map<eString, unsigned int>::iterator i(keys_uint.begin()); i != keys_uint.end(); ++i)
		fprintf(f, "u:%s=%08x\n", i->first.c_str(), i->second);
	for (std::map<eString, double>::iterator i(keys_double.begin()); i != keys_double.end(); ++i)
		fprintf(f, "d:%s=%lf\n", i->first.c_str(), i->second);
	for (std::map<eString, eString>::iterator i(keys_string.begin()); i != keys_string.end(); ++i)
		fprintf(f, "s:%s=%s\n", i->first.c_str(), i->second.c_str());

	fclose(f);
}

eSimpleConfigFile::eSimpleConfigFile(const char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (f)
	{
		char buffer[256];

		while (fgets(buffer, 255, f))
		{
			if (strlen(buffer) && buffer[strlen(buffer) - 1] == '\n')
			{
				buffer[strlen(buffer) - 1] = 0;
			}

			for (int i = 0; i < strlen(buffer); i++)
			{
				if (buffer[i] == '=')
				{
					buffer[i] = 0;
					config[buffer] = &buffer[i + 1];
					break;
				}
			}
		}
		fclose(f);
	}
}

eString eSimpleConfigFile::getInfo(const char *info)
{
	return config[info];
}
void eSimpleConfigFile::setInfo(const char *info, const char* value)
{
	config[info] = value;
}
void eSimpleConfigFile::Save(const char *filename)
{
	FILE *f = fopen(filename, "w");
	if (!f)
	{
		eWarning("couldn't write config!");
		return;
	}
	for (std::map<eString, eString>::iterator i(config.begin()); i != config.end(); ++i)
		fprintf(f, "%s=%s\n", i->first.c_str(), i->second.c_str());
	fclose(f);
}

eAutoInitP0<eConfig> init_eRCConfig(eAutoInitNumbers::configuration, "Configuration");
