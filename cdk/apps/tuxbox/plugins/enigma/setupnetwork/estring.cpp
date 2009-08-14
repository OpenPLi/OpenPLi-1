#include <lib/base/estring.h>
#include <ctype.h>
#include <limits.h>
#include <lib/system/elock.h>

///////////////////////////////////////// eString sprintf /////////////////////////////////////////////////
eString& eString::sprintf(char *fmt, ...)
{
// Implements the normal sprintf method, to use format strings with eString
// The max length of the result string is 1024 char.
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	std::vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	assign(buf);
	return *this;
}

/////////////////////////////////////// eString upper() ////////////////////////////////////////////////
eString& eString::upper()
{
//	convert all lowercase characters to uppercase, and returns a reference to itself
	for (iterator it = begin(); it != end(); it++)
		switch(*it)
		{
			case 'a' ... 'z' :
				*it -= 32;
			break;

			case 'ä' :
				*it = 'Ä';
			break;
			
			case 'ü' :
				*it = 'Ü';
			break;
			
			case 'ö' :
				*it = 'Ö';
			break;
		}

	return *this;
}

eString& eString::strReplace(const char* fstr, const eString& rstr)
{
//	replace all occurrence of fstr with rstr and, and returns a reference to itself
	unsigned int index=0;
	unsigned int fstrlen = strlen(fstr);
	int rstrlen=rstr.size();

	while ( ( index = find(fstr, index) ) != npos )
	{
		replace(index, fstrlen, rstr);
		index+=rstrlen;
	}
	
	return *this;
}

