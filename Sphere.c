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
#define RadiusOfEarth	6378.135

/* Trig constants */
#ifndef M_PI
#define M_PI				3.14159265358979323846
#endif
#define	degToRad		(M_PI/180.0)

/*
 * Compute the GreatCircleDistance of two points in the earth (assume earth is a sphere).
 * Assume lat/long are inputs in degrees.
 * Return result in km.
 *
 * This routine uses a method which is numerically stable.
 */
double	GreatCircleDistance( double lng1, double lat1, double lng2, double lat2 )
{
	register	double d;
	if( m_bUseLatLong )
	{
		register double dlng, dlat, x;

		// Convert to radians.
		lng1	*= degToRad;
		lat1	*= degToRad;
		lng2	*= degToRad;
		lat2	*= degToRad;

		// Compute approximate distance between two points in km. Assumes the Earth is a sphere.
		// TODO: change to ellipsoid approximation, such as
		// http://www.codeguru.com/Cpp/Cpp/algorithms/article.php/c5115/
		dlat = sin(0.5 * (lat2 - lat1));
		dlng = sin(0.5 * (lng2 - lng1));
		x = dlat * dlat + dlng * dlng * cos(lat1) * cos(lat2);
		d = RadiusOfEarth * (2.0 * atan2(sqrt(x), sqrt(x < 1.0 ? 1.0 - x : 0.0)));
	}
	else
	{
		// Assume coords are in UTM, in meters.
		d = sqrt( Square(lng2 - lng1) + Square(lat2 - lat1) ) / 1000.0;
	}
  
 	return distanceMeasure == 1 ? d : kmToMiles(d);
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
