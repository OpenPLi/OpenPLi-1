#include <lib/system/math.h>

/*----------------------------------------------------------------------------*/
double factorial_div( double value, int x)
{
	if(!x)
		return 1;
	else
	{
		while( x > 1)
		{
			value = value / x--;
		}
	}
	return value;
}

/*----------------------------------------------------------------------------*/
double powerd( double x, int y)
{
	int i=0;
	double ans=1.0;

	if(!y)
		return 1.000;
	else
	{
		while( i < y)
		{
			i++;
			ans = ans * x;
		}
	}
	return ans;
}

/*----------------------------------------------------------------------------*/
double SIN( double x)
{
	int i=0;
	int j=1;
	int sign=1;
	double y1 = 0.0;
	double diff = 1000.0;

	if (x < 0.0)
	{
		x = -1 * x;
		sign = -1;
	}

	while ( x > 360.0*M_PI/180)
	{
		x = x - 360*M_PI/180;
	}

	if( x > (270.0 * M_PI / 180) )
	{
		sign = sign * -1;
		x = 360.0*M_PI/180 - x;
	}
	else if ( x > (180.0 * M_PI / 180) )
	{
		sign = sign * -1;
		x = x - 180.0 *M_PI / 180;
	}
	else if ( x > (90.0 * M_PI / 180) )
	{
		x = 180.0 *M_PI / 180 - x;
	}

	while( powerd( diff, 2) > 1.0E-16 )
	{
		i++;
		diff = j * factorial_div( powerd( x, (2*i -1)) ,(2*i -1));
		y1 = y1 + diff;
		j = -1 * j;
	}
	return ( sign * y1 );
}

double COS(double x)
{
	return SIN(90 * M_PI / 180 - x);
}

/*----------------------------------------------------------------------------*/
double ATAN( double x)
{
	int i=0; /* counter for terms in binomial series */
	int j=1; /* sign of nth term in series */
	int k=0;
	int sign = 1; /* sign of the input x */
	double y = 0.0; /* the output */
	double deltay = 1.0; /* the value of the next term in the series */
	double addangle = 0.0; /* used if arctan > 22.5 degrees */

	if (x < 0.0)
	{
		x = -1 * x;
		sign = -1;
	}

	while( x > 0.3249196962 )
	{
		k++;
		x = (x - 0.3249196962) / (1 + x * 0.3249196962);
	}
	addangle = k * 18.0 *M_PI/180;

	while( powerd( deltay, 2) > 1.0E-16 )
	{
		i++;
		deltay = j * powerd( x, (2*i -1)) / (2*i -1);
		y = y + deltay;
		j = -1 * j;
	}
	return (sign * (y + addangle) );
}

double ASIN(double x)
{
	return 2 * ATAN( x / (1 + std::sqrt(1.0 - x*x)));
}

double Radians( double number )
{
	return number*M_PI/180;
}

double Deg( double number )
{
	return number*180/M_PI;
}

double Rev( double number )
{
	return number - std::floor( number / 360.0 ) * 360;
}

