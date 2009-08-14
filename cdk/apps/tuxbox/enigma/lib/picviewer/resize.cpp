#ifndef DISABLE_FILE
#include <lib/picviewer/pictureviewer.h>

unsigned char *simple_rotate(unsigned char *orgin, int ox, int oy, int rotate);

unsigned char * simple_resize(unsigned char * orgin, int ox, int oy, int dx, int dy, int rotate)
{
	eDebug("picviewer: simple_resize: buf=%lx oxy=%d,%d dxy=%d,%d rotate=%d", orgin, ox, oy, dx, dy, rotate);
	unsigned char *cr, *p, *l;
	int i, j, k, ip;
	cr = new unsigned char[dx * dy * 3]; 
	if (cr == NULL)
	{
		eDebug("picviewer: simple_resize: Error: malloc");
		return orgin;
	}
	l = cr;

	for (j = 0; j < dy; j++,l += dx * 3)
	{
		p = orgin + (j * oy / dy * ox * 3);
		for (i = 0, k = 0; i < dx; i++, k += 3)
		{
			ip = i * ox / dx * 3;
			l[k] = p[ip];
			l[k+1] = p[ip + 1];
			l[k+2] = p[ip + 2];
		}
	}
	// eDebug("picviewer: simple_resize done");
	return cr;
}

unsigned char * color_average_resize(unsigned char * orgin, int ox, int oy, int dx, int dy, int rotate)
{
	eDebug("picviewer: color_average_resize: buf=%lx oxy=%d,%d dxy=%d,%d rotate=%d", orgin, ox, oy, dx, dy, rotate);
	unsigned char *cr, *p, *q;
	int i, j, k, l, xa, xb, ya, yb;
	int sq, r, g, b;

	cr = new unsigned char[dx * dy * 3];
	if (cr == NULL)
	{
		eDebug("picviewer: color_average_resize: Error: malloc");
		return orgin;
	}
	p = cr;

	for (j = 0; j < dy; j++)
	{
		for (i = 0; i < dx; i++, p += 3)
		{
			xa = i * ox / dx;
			ya = j * oy / dy;
			xb = (i + 1) * ox / dx; 
			if (xb >= ox)
				xb = ox - 1;
			yb = (j + 1) * oy / dy; 
			if (yb >= oy)
				yb = oy - 1;
			for (l = ya, r = 0, g = 0, b = 0, sq = 0; l <= yb; l++)
			{
				q = orgin + ((l * ox + xa) * 3);
				for (k = xa; k <= xb; k++, q += 3, sq++)
				{
					r += q[0]; g += q[1]; b += q[2];
				}
			}
			p[0] = r / sq; p[1] = g / sq; p[2] = b / sq;
		}
	}

	if (rotate)
	{
		unsigned char *rot;
		rot = simple_rotate(cr, dx, dy, rotate);

		if (rot != cr && rot != NULL) 
		{ // rotate() managed to do something...
			delete [] cr;
			cr = rot;	
		}
	}
	// eDebug("picviewer: color_average_resize done");
	return cr;
}

unsigned char *simple_rotate(unsigned char *orgin, int ox, int oy, int rotate)
{
	eDebug("picviewer: simple_rotate: buf=%lx oxy=%d,%d rotate=%d", orgin, ox, oy, rotate);
	unsigned char *cr, *p, *q;
	int i, j;

	if (rotate == 0 || rotate > 3)
		return(orgin);

	cr = new unsigned char[ox * oy * 3];
	if (cr == NULL)
	{
		eDebug("picviewer: simple_rotate: Error: malloc");
		return orgin;
	}

	q = orgin;
	switch (rotate)
	{
		case 1: // rotate 90 degrees clockwise
			for (j = 0; j < oy; j++)
			{
				for (i = 0; i < ox; i++)
				{
					p = cr + ((i + 1)*oy - j - 1)*3;
					p[0] = *(q++);
					p[1] = *(q++);
					p[2] = *(q++);
				}
			}
			break;
		case 2: // rotate 180 degrees clockwise
			p = cr + ox*oy*3 - 3;
			for (j = 0; j < oy; j++)
			{
				for (i = 0; i < ox; i++)
				{
					p[0] = *(q++);
					p[1] = *(q++);
					p[2] = *(q++);
					p -= 3;
				}
			}
			break;
		case 3: // rotate 270 degrees clockwise
			for (j = 0; j < oy; j++)
			{
				for (i = 0; i < ox; i++)
				{
					p = cr + ((ox - i - 1)*oy + j)*3;
					p[0] = *(q++);
					p[1] = *(q++);
					p[2] = *(q++);
				}
			}
			break;
	}

	eDebug("picviewer: simple_rotate done");
	return cr;
}

#endif
