//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef TFormat_H
#define TFormat_H

class tFormat
{
public:
	tFormat( long t )
	{
		int	neg;
		if( t < 0 )	{ neg = 1; t = -t; } else { neg = 0; }

		long	hour = t / (60 * 60), min = (t - hour * 60*60) / 60, sec = t % 60;

		char *p = &sz[sizeof(sz)];
		if( !neg ) --p;
		*--p = 0;
		register int i;
		i = 0; do { *--p = (sec % 10) + '0'; sec /= 10; } while( ++i < 2 ); *--p = ':';
		i = 0; do { *--p = (min % 10) + '0'; min /= 10; } while( ++i < 2 ); *--p = ':';
		i = 0; do { *--p = (hour % 10) + '0'; hour /= 10; } while( ++i < 2 );
		if( neg ) *--p = '-';
	}
	operator const char *() { return sz; }

	char	sz[1+2+1+2+1+2+1];	// Format is [-]HH:MM:SS(eos) = 10 characters at most.
};

#endif // TFormat_H