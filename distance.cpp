//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
extern "C"
{
#include "TMSHeader.h"
}

#include "distance.h"

static inline double FAbs( const double x )	{ return x > 0.0 ? x : -x; }

static inline double square(const double x)	{ return x * x; }

//
//  The following routines return the distances in km.
//  To convert the distances to miles, use the kmToMiles function.

// Radius of Earth in km
#define RadiusOfEarth	6367.5

// Trig constants
#define	degToRad		(M_PI/180.0)

double Distance::greatCircleDistance( double long1, double lat1, double long2, double lat2 )
{
	if( m_bUseLatLong )	// Flag defined in TMSHeader.h.
	{
		register double dlong, dlat, sin_dlatDiv2, sin_dlongDiv2, sqrtA;

		/* Convert to radians */
		long1	*= degToRad;
		lat1	*= degToRad;
		long2	*= degToRad;
		lat2	*= degToRad;

		/* Do the calculation.  Only compute intermediate results once. */
		dlong = long2 - long1;
		dlat = lat2 - lat1;
		sin_dlatDiv2 = sin(dlat/2.0);
		sin_dlongDiv2 = sin(dlong/2.0);
		sqrtA = sqrt(sin_dlatDiv2 * sin_dlatDiv2 + cos(lat1) * cos(lat2) * sin_dlongDiv2 * sin_dlongDiv2);

		/* Return result in km. */
		return RadiusOfEarth * 2.0 * asin(sqrtA < 1.0 ? sqrtA : 1.0);
	}
	else
	{
		// Assume (long1, lat1) and (long2, lat2) are really (x,y) coords in meters in UTM.
		return sqrt( square(long2 - long1) + square(lat2 - lat1) ) / 1000.0;	// Return result in km.
	}
}

double Distance::longReference = -1.0;
double Distance::latReference = -1.0;
double Distance::longitude_1_degree_distance = -1.0;
double Distance::latitude_1_degree_distance = -1.0;

// Initialize the reference dimensions at the point of the earth we want
// to compute the distances relative to.
// This will be called automatically by FastCreateCircleDistance,
// but it should be called again if you need to dramatically alter
// the reference point on the earth.
void Distance::initFastDimensions(double longRef, double latRef )
{
    // Use the great circle distance to compute the distance of
    // one degree in each direction from the reference point.
    // These distances will be used in the fast linear interpolation.
    longitude_1_degree_distance
        = greatCircleDistance(longRef, latRef, longRef+1.0, latRef);
    latitude_1_degree_distance
        = greatCircleDistance(longRef, latRef, longRef, latRef+1.0);

	longReference = longRef;
	latReference = latRef;
}

// Initialize the directions table.
char	*Distance::szDirections[8] =
{
	"Northeast",
	"North",
	"Northwest",
	"West",
	"Southwest",
	"South",
	"Southeast",
	"East"
};