#include <stdio.h>

#include "libavs.h"

int main()
{
	if ( avsInit(1) != 0 )
	{
		return -1;
	}

	if ( avsSetRoute( epTV, esDIGITALENCODER, emCVBS ) != 0 )
	{
		printf("error\n");
	}

	avsDeInit();

	return 0;
}
