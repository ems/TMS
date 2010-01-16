//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef Timer_H
#define Timer_H

#include <time.h>
#include <sys/timeb.h>

class Timer
{
public:
	Timer	&reset()
	{
		struct timeb n;
		ftime(&n);
		d = n.time + n.millitm / 1000.0;
		return *this;
	}

	Timer() { reset(); }
	Timer( const Timer &ti ) { d = ti.d; }
	Timer( const double aD ) { d = aD; }

	double elapsed() const { return Timer() - *this; }
	double operator-( const Timer &t ) const { return d - t.d; }

private:
	double	d;
};

#endif // Timer_H