#include <lib/gui/eservicegrid.h>

eServiceGrid::eServiceGrid(eWidget *parent): eWidget(parent)
{
	para=0;
	grid=0;
	
	elemsize=eSize(16, 16);
}

void eServiceGrid::createGrid(eSize gs)
{
	gridsize=gs;
	if (grid)
		delete[] grid;
	if (!(gridsize.x() && gridsize.y()))
		return;
	grid=new (eService*)[gridsize.x()*gridsize.y()];
}

eService **eServiceGrid::allocateGrid(eSize size)
{
	for (int y=0; y<gridsize.height(); y++)
		for (int x=0; x<gridsize.width(); x++)
		{
			int ok=1;
			for (int cy=0; ok && cy<size.height(); cy++)
				for (int cx=0; ok && cx<size.width(); cx++)
					if (grid[gridsize.width()*(y+cy)+x+cx])
					{
						ok=0;
						break;
					}
			if (ok)
				return grid+gridsize.width()*(y+cy)+x+cx;
		}
	return 0;
}

void eServiceGrid::setGridSize(int gx, int gy)
{
	createGrid(eSize(gx, gy));
}

void eServiceGrid::addService(eService *service)
{
	eTextPara temp;
	temp.setFont(font);
	temp.renderText(service->service_name);
	eSize size=temp.getExtends();
	eSize gsize=eSize((size.width()+elemsize.width()-1)/elemsize.width(), (size.height()+elemsize.height()-1)/elemsize.height());
	eService **sp=allocateGrid(gsize);
	if (!sp)
		return;
	for (int cy=0; ok && cy<gsize.height(); cy++)
		for (int cx=0; ok && cx<gsize.width(); cx++)
			sp[cy*gridsize.width()+cx]=service;
}


void eServiceGrid::validate()
{
	eService **gp=grid;
	for (int y=0; y<gridsize.height(); y++)
		for (int x=0; x<gridsize.width(); x++)
		{
			if (*gp)
			{
				para->setCursor(x*elemsize.width(), y*elemsize.height());
				para->renderString((*gp)->service_name);
			}
			*gp++;
		}
}
