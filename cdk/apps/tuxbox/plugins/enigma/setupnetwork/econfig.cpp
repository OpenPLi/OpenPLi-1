#include <econfig.h>

eConfig::eConfig()
{
	FILE *f = fopen("/var/tuxbox/config/enigma/config", "r");
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
			case 'i':
			{
				if ( sscanf(opt, "%x", &keys_int[key] ) != 1 )
				{
					if (sscanf(opt, "%x", &keys_int[key] ) != 1 )
						printf("couldn't parse %s", opt);
				}
				break;
			}             
			}
		}
		fclose(f);
	} else {
		printf("No config could be opened\n");
	}
	
}

eConfig::~eConfig()
{
}

int eConfig::getKey(const char *key, int &i)
{
	std::map<std::string, int>::iterator it = keys_int.find(key);
	if (it == keys_int.end())
		return -1;
	i = it->second;
	return 0;
}

int eConfig::getKey(const char *key, unsigned int &ui)
{
	std::map<std::string, unsigned int>::iterator it = keys_uint.find(key);
	if (it == keys_uint.end())
		return -1;
	ui = it->second;
	return 0;
}
int eConfig::getKey(const char *key, char * &string)
{
	std::map<std::string, std::string>::iterator it = keys_string.find(key);
	if (it == keys_string.end())
		return -1;
	string = strdup(it->second.c_str());
	return 0;
}
