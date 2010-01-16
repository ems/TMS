//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef distance_H
#define distance_H

// Use this to get m_bUseLatLong.
extern "C"
{
#include "TMSHeader.h"
}

#include <math.h>

#ifndef M_PI
#define M_PI				3.14159265358979323846
#endif // M_PI

#define FAST_DISTANCE

class Distance
{
public:
	static double kmToMiles( const double k )	{ return k * 0.6213712; }
	static double milesToKM( const double m )	{ return m * (1.0/0.6213712); }

	static double fastRectilinearDistance( double long1, double lat1, double long2, double lat2 )
	{
		// Use a flat plane interpolation to compute distances
		// (assume that the earth is flat).

		// Make sure the distances have been initialized.
		if( longitude_1_degree_distance <= 0.0 )
			initFastDimensions(long1, lat1);

#ifdef FAST_DISTANCE
		double dx = (long2 - long1) * longitude_1_degree_distance;
		if( dx < 0 ) dx = -dx;
		double dy = (lat2  -  lat1) * latitude_1_degree_distance;
		if( dy < 0 ) dy = -dy;

		return dx + dy;
#else
		return rectilinearGreatCircleDistance( long1, lat1, long2, lat2 );
#endif
	}

	static	void	CoordFromLongLat( int &x, int &y, double longIn, double latIn )
	{
		// Make sure the distances have been initialized.
		if( longitude_1_degree_distance <= 0.0 )
			initFastDimensions(longIn, latIn);

#ifdef FAST_DISTANCE
		// Convert to meters.
		x = (int)((longIn - longReference) * longitude_1_degree_distance * 1000.0);
		y = (int)((latIn  -  latReference) * latitude_1_degree_distance  * 1000.0);
#else
		x = (int)(greatCircleDistance(longReference, latReference, longReference, latIn) * 1000.0);
		y = (int)(greatCircleDistance(longReference, latReference, longIn, latReference) * 1000.0);
#endif
	}

	enum CompassPoint { Northeast, North, Northwest, West, Southwest, South, Southeast, East, Directly };
	static	CompassPoint compassPoint( int x1, int y1, int x2, int y2 )
	{
		if( !m_bUseLatLong )
		{
			register int t;
			t = x1; x1 = y1; y1 = t;
			t = x2; x2 = y2; y2 = t;
		}

		if( x1 == x2 && y1 == y2 ) return Directly;
		register double a = atan2( (double)y2 - y1, (double)x2 - x1 ) - M_PI/8.0;
		if( a < 0.0 ) a += 2.0 * M_PI;
		if( a < 0.0 ) a += 2.0 * M_PI;
		return (CompassPoint)(int)(a / (M_PI/4.0));
	}

	static const char	*compassDirection( int x1, int y1, int x2, int y2 )
	{
		const CompassPoint	cp = compassPoint( x1, y1, x2, y2 );
		return cp == Directly ? "" : szDirections[cp];
	}
	static const char	*compassDirection( double long1, double lat1, double long2, double lat2 )
	{
		int	x1, y1, x2, y2;
		CoordFromLongLat( x1, y1, long1, lat1 );
		CoordFromLongLat( x2, y2, long2, lat2 );
		return compassDirection( x1, y1, x2, y2 );
	}

	static double fastGreatCircleDistance( double long1, double lat1, double long2, double lat2 )
	{
#ifdef FAST_DISTANCE
		// Use a flat plane interpolation to compute distances
		// (assume that the earth is flat).

		// Make sure the distances have been initialized.
		if( longitude_1_degree_distance <= 0.0 )
			initFastDimensions(long1, lat1);

		double dx = (long2 - long1) * longitude_1_degree_distance;
		double dy = (lat2  -  lat1) * latitude_1_degree_distance;

		return sqrt( dx*dx + dy*dy );
#else
		return greatCircleDistance( long1, lat1, long2, lat2 );
#endif
	}

	static double greatCircleDistance( double long1, double lat1, double long2, double lat2 );

	static double rectilinearGreatCircleDistance( double long1, double lat1, double long2, double lat2 )
	{
		return	greatCircleDistance(long1, lat1, long2, lat1) +
				greatCircleDistance(long2, lat1, long2, lat2);
	}

	static void resetFastDimensions()
	{
		latitude_1_degree_distance = longitude_1_degree_distance = -1.0;
	}

private:
	static void	initFastDimensions( double longRef, double latRef );

	static double longReference, latReference;
	static double longitude_1_degree_distance;
	static double latitude_1_degree_distance;
	static char	*szDirections[8];
};

#endif // distance_H