//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef parseMIF_H
#define parseMIF_H

#include <fstream>
using namespace std;

#include "mm.h"

// Parse a mapinfo MIF file and extract all the lines.

class ParseMIF
{
public:
	class Line
	{
	public:
		Line( const double aLat1 = 0.0, const double aLong1 = 0.0, const double aLat2 = 0.0, const double aLong2 = 0.0 ) :
			lat1(aLat1), long1(aLong1), lat2(aLat2), long2(aLong2), next(NULL) {}
		Line( const Line &l ) : next(NULL) { operator=(l); }
		Line &operator=( const Line &l ) { lat1 = l.lat1; long1 = l.long1; lat2 = l.lat2; long2 = l.long2; return *this; }
		MMdeclare();

		double	lat1, long1, lat2, long2;
		Line	*next;

	private:
		friend class ParseMIF;
		void	addToList( Line *&head )
		{
			next = head;
			head = this;
		}
	};

	ParseMIF( istream &is );
	~ParseMIF();

	Line	*getLHead() const { return lHead; }

private:
	void	getLines( int numPoints, istream &is );
	Line	*lHead;
};

#endif // parseMIF_H