//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "distance.h"

static inline double square(const double x)	{ return x * x; }

//
//  The following routines return the distances in km.
//  To convert the distances to miles, use the kmToMiles function.

// Radius of Earth in km
#define RadiusOfEarth	6378.135

// Trig constants
#define	degToRad		(M_PI/180.0)

double Distance::greatCircleDistance(register double lng1, register double lat1,
									 register double lng2, register double lat2 )
{
	if( m_bUseLatLong )	// Flag defined in TMSHeader.h.
	{
		// Convert to radians.
		lng1	*= degToRad;
		lat1	*= degToRad;
		lng2	*= degToRad;
		lat2	*= degToRad;

		// Compute approximate distance between two points in km. Assumes the Earth is a sphere.
		// TODO: change to ellipsoid approximation, such as
		// http://www.codeguru.com/Cpp/Cpp/algorithms/article.php/c5115/
		register double dlat = sin(0.5 * (lat2 - lat1));
		register double dlng = sin(0.5 * (lng2 - lng1));
		register double x = dlat * dlat + dlng * dlng * cos(lat1) * cos(lat2);
		return RadiusOfEarth * (2.0 * atan2(sqrt(x), sqrt(x < 1.0 ? 1.0 - x : 0.0)));
	}
	else
	{
		// Assume (long1, lat1) and (long2, lat2) are really (x,y) coords in meters in UTM.
		return sqrt( square(lng2 - lng1) + square(lat2 - lat1) ) / 1000.0;	// Return result in km.
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