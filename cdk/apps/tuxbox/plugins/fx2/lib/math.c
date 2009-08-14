
#define tolower(a)	((a)|32)

int	_atoi( const char *str )
{
	int		val;
	int		base, c;
	char	*sp;
	char	ineg=0;

	val=0;
	sp=(char*)str;
	if ((*sp == '0' ) && (*(sp+1) == 'x' ))
	{
		base=16;
		sp+=2;
	}
	else if ( *sp == '0' )
	{
		base = 8;
		sp++;
	}
	else
	{
		base = 10;
		if ( *sp == '-' )
		{
			ineg=1;
			sp++;
		}
	}
	for (; (*sp != 0); sp++)
	{
		c = (*sp > '9') ? (tolower(*sp) - 'a' + 10) : (*sp - '0');
		if ((c < 0) || (c >= base))
			break;
		val = (val*base)+c;
	}
	return ineg ? -val:val;
}
