#ifndef __eservicegrid_h
#define __eservicegrid_h

#include "ewidget.h"

class eServiceGrid: public eWidget
{
	eService *(*grid);
	eSize gridsize;
	eSize elemsize;
	
	eTextPara *para;
	
	void validate();
	
	void createGrid(eSize gridsize);
	eService **allocateGrid(eSize size);
	
public:
	eServiceGrid(eWidget *parent);
	
	void setGridSize(int gx, int gy);
	void addService(eService *service);
	
};

#endif
