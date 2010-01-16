//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#include <math.h>

#define kmToMiles( k )  ((double)(k) * 0.6213712)
#define FAbs( x )		((x) > 0.0 ? (x) : -(x))
#define Square( x )		((x) * (x))

/* Radius of Earth in km */
#define RadiusOfEarth	6367.5

/* Trig constants */
#define PI				3.14159265358979323846
#define	degToRad		(PI/180.0)

/*
 * Compute the GreatCircleDistance of two points in the earth (assume earth is a sphere).
 * Assume lat/long are inputs in degrees.
 * Return result in km.
 *
 * This routine uses a method which is numerically stable.
 */
double	GreatCircleDistance( double long1, double lat1, double long2, double lat2 )
{
	register	double d;
	if( m_bUseLatLong )
	{
		register double dlong, dlat, sin_dlatDiv2, sin_dlongDiv2, sqrtA;

		/* Convert to radians */
		lat1	*= degToRad;
		long1	*= degToRad;
		lat2	*= degToRad;
		long2	*= degToRad;

		/* Do the calculation.  Only compute intermediate results once. */
		dlong = long2 - long1;
		dlat = lat2 - lat1;
		sin_dlatDiv2 = sin(dlat/2.0);
		sin_dlongDiv2 = sin(dlong/2.0);
		sqrtA = sqrt(sin_dlatDiv2 * sin_dlatDiv2 + cos(lat1) * cos(lat2) * sin_dlongDiv2 * sin_dlongDiv2);
		d = RadiusOfEarth * 2.0 * asin(sqrtA < 1.0 ? sqrtA : 1.0);
	}
	else
	{
		// Assume coords are in UTM, in meters.
		d = sqrt( Square(long2 - long1) + Square(lat2 - lat1) ) / 1000.0;
	}
	return distanceMeasure == PREFERENCES_KILOMETERS ? d : kmToMiles(d);
}

double	RectilinearGreatCircleDistance( double long1, double lat1, double long2, double lat2 )
{
	return GreatCircleDistance(long1, lat1, long2, lat1) + GreatCircleDistance(long2, lat1, long2, lat2);
}

static double latitude_1_degree_distance = -1.0;
static double longitude_1_degree_distance = -1.0;

// Initialize the reference dimensions at the point of the earth we want
// to compute the distances relative to.
// This will be called automatically by FastCreateCircleDistance,
// but it should be called again if you need to dramatically alter
// the reference point on the earth.
void InitFastGreatCircleDimensions(double longRef, // in degrees
                                   double latRef)
{
    // Use the great circle distance to compute the distance of
    // one degree in each direction from the reference point.
    // These distances will be used in the fast linear interpolation.
    longitude_1_degree_distance
        = GreatCircleDistance(longRef, latRef, longRef+1.0, latRef);
    latitude_1_degree_distance
        = GreatCircleDistance(longRef, latRef, longRef, latRef+1.0);
}

// This routine is much faster than that GreatCircleDistance function
// because it avoids any trigonometric functions.
double FastGreatCircleDistance(double long1, double lat1, // in degrees
                               double long2, double lat2 )

{
    register double      dx, dy, d;

    // Use a flat plane interpolation to compute distances
    // (assume that the earth is flat).

    // Make sure the distances have been initialized.
    if( longitude_1_degree_distance <= 0.0 )
        InitFastGreatCircleDimensions(long1, lat1);

    dx = (long2 - long1) * longitude_1_degree_distance;
    dy = (lat2  -  lat1) * latitude_1_degree_distance;

    // Compute the distance between the points in km.
//    d = sqrt(Square(dx) + Square(dy));

    // Compute rectilinear distance instead of straight line distance.
    d = FAbs(dx) + FAbs(dy);

    return(distanceMeasure == PREFERENCES_KILOMETERS ? d : kmToMiles(d));
}
//
//  This routine uses no longer uses the MapInfo distance formula.
//  Instead, it uses the standard great-circle distance formula.
//
double MapInfoDistance(double long1, double lat1, double long2, double lat2 )
{
  return GreatCircleDistance( long1, lat1, long2, lat2 );
}
