#ifndef __media_mapping_h
#define __media_mapping_h

#include <lib/base/estring.h>
#include <lib/system/econfig.h>

class eMediaMapping
{
	public:
	enum mediaTypeEnum
	{
		mediaMovies = 0,
		mediaTimeshifts = 1,
		maxMediaTypeEnum
	};
	
	static eMediaMapping *getInstance()
	{
		return (instance) ? instance : (instance = new eMediaMapping());
	}

	bool getStorageLocation(
		eMediaMapping::mediaTypeEnum mediaType,
		eString& storageLocation);
	void setStorageLocation(
		eMediaMapping::mediaTypeEnum mediaType,
		const eString& storageLocation);

	~eMediaMapping();

	private:
	eMediaMapping();
	void init();
	
	static eMediaMapping *instance;
	eConfig *econfig;
};

#endif
