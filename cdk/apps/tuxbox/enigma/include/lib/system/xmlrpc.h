#ifndef DISABLE_NETWORK

#ifndef __xmlrpc_h_
#define __xmlrpc_h_

#include <asm/types.h>
#include <map>
#include <vector>
#include <xmltree.h>

#include <lib/base/estring.h>
#include <lib/base/eptrlist.h>
#include <lib/system/httpd.h>

#define INSERT(KEY,VALUE) insert(std::pair<eString, eXMLRPCVariant*>(KEY,VALUE))

class eXMLRPCVariant
{
	union
	{
		std::map<eString,eXMLRPCVariant*> *_struct;
		std::vector<eXMLRPCVariant> *_array;
		__s32 *_i4;
		bool *_boolean;
		eString *_string;
		double *_double;
//	QDateTime *_datetime;
//	QByteArray *_base64;
	};
	void init_eXMLRPCVariant(const eXMLRPCVariant &c);
public:
	enum XMLRPCType { STRUCT, ARRAY, I4, BOOLEAN, STRING, DOUBLE };
	eXMLRPCVariant(std::map<eString,eXMLRPCVariant*> *_struct);
	eXMLRPCVariant(std::vector<eXMLRPCVariant> *_array);
	eXMLRPCVariant(__s32 *_i4);
	eXMLRPCVariant(bool *_boolean);
	eXMLRPCVariant(eString *_string);
	eXMLRPCVariant(double *_double);
//	eXMLRPCVariant(QDateTime *_datetime);
//	eXMLRPCVariant(QByteArray *_base64);
	eXMLRPCVariant(const eXMLRPCVariant &c);
	~eXMLRPCVariant();

	std::map<eString,eXMLRPCVariant*> *getStruct();
	std::vector<eXMLRPCVariant> *getArray();
	__s32 *getI4();
	bool *getBoolean();
	eString *getString();
	double *getDouble();
//	inline QDateTime *getDatetime();
//	inline QByteArray *getBase64();

	void toXML(eString &);
private:
	XMLRPCType type;
public:
	XMLRPCType getType() { return type; }
};

inline eXMLRPCVariant::eXMLRPCVariant(std::map<eString,eXMLRPCVariant*> *__struct)
	:_struct(__struct), type( STRUCT )
{
}

inline eXMLRPCVariant::eXMLRPCVariant(std::vector<eXMLRPCVariant> *__array)
	:_array( __array ), type( ARRAY )
{
}

inline eXMLRPCVariant::eXMLRPCVariant(__s32 *__i4)
	:_i4(__i4), type(I4)
{
}

inline eXMLRPCVariant::eXMLRPCVariant(bool *__boolean)
	:_boolean(__boolean), type(BOOLEAN)
{
}

inline eXMLRPCVariant::eXMLRPCVariant(eString *__string)
	:_string(__string), type(STRING)
{
}

inline eXMLRPCVariant::eXMLRPCVariant(double *__double)
	:_double(__double), type(DOUBLE)
{
}

/*inline eXMLRPCVariant::eXMLRPCVariant(QDateTime *__datetime)
	:_datetime(__datetime), type(DATETIME)
{
} */

/*inline eXMLRPCVariant::eXMLRPCVariant(QByteArray *__base64)
	:_base64(__base64), type(BASE64)
{
} */

inline std::map<eString,eXMLRPCVariant*> *eXMLRPCVariant::getStruct()
{
	return _struct;
}

inline std::vector<eXMLRPCVariant> *eXMLRPCVariant::getArray()
{
	return _array;
}

inline __s32 *eXMLRPCVariant::getI4()
{
	return _i4;
}

inline bool *eXMLRPCVariant::getBoolean()
{
	return _boolean;
}

inline eString *eXMLRPCVariant::getString()
{
	return _string;
}

inline double *eXMLRPCVariant::getDouble()
{
	return _double;
}

/*inline QDateTime *eXMLRPCVariant::getDatetime()
{
	return _datetime;
} */

/*inline QByteArray *eXMLRPCVariant::getBase64()
{
	return _base64;
} */

class eXMLRPCResponse: public eHTTPDataSource
{
	XMLTreeParser parser;
	eString result;
	int size;
	int wptr;
	int doCall();
public:
	eXMLRPCResponse(eHTTPConnection *c);
	~eXMLRPCResponse();
	
	int doWrite(int);
	void haveData(void *data, int len);
};

void xmlrpc_initialize(eHTTPD *httpd);
void xmlrpc_addMethod(eString methodName, int (*)(std::vector<eXMLRPCVariant>&, ePtrList<eXMLRPCVariant>&));
void xmlrpc_fault(ePtrList<eXMLRPCVariant> &res, int faultCode, eString faultString);
int xmlrpc_checkArgs(eString args, std::vector<eXMLRPCVariant>&, ePtrList<eXMLRPCVariant> &res);

class eHTTPXMLRPCResolver: public eHTTPPathResolver
{
public:
	eHTTPXMLRPCResolver();
	eHTTPDataSource *getDataSource(eString request, eString path, eHTTPConnection *conn);
};

#endif

#endif //DISABLE_NETWORK
