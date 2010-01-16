#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>		/* isdigit() */
#include "convert_datum.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2	1.57079632679489661923
#endif

/*
 * Peter Daly
 * MIT Ocean Acoustics
 * pmd@mit.edu
 * 25-MAY-1998
 * 
 * These routines convert UTM to Lat/Longitude and vice-versa,
 * using the WGS-84 (GPS standard) or Clarke 1866 Datums.
 * 
 * The formulae for these routines were originally taken from
 * Chapter 10 of "GPS: Theory and Practice," by B. Hofmann-Wellenhof,
 * H. Lictenegger, and J. Collins. (3rd ed) ISBN: 3-211-82591-6,
 * however, several errors were present in the text which
 * made their formulae incorrect.
 *
 * Instead, the formulae for these routines was taken from
 * "Map Projections: A Working Manual," by John P. Snyder
 * (US Geological Survey Professional Paper 1395)
 *
 * Copyright (C) 1998 Massachusetts Institute of Technology
 *               All Rights Reserved
 *
 * RCS ID: $Id: convert_datum.c,v 1.3 1999/03/25 17:26:22 pmd Exp pmd $
 */


#ifdef DEBUG
int
main (int argc, char **argv)
{
  UTM utm;
  LL ll;

  ll.latitude = 34.0 + 6/60.0 + 41.03/3600.0;
  ll.longitude = -(119 + 19/60.0 + 49.90/3600.0);

  utm.x = utm.y = 0.0;

  ll2utm (&ll, &utm, WGS_84_DATUM);

  printf ("ll: %f %f utm: [%s] %f,%f\n",
	  ll.latitude, ll.longitude,
	  utm.grid_zone, utm.x, utm.y);

  utm2ll (&utm, &ll, WGS_84_DATUM);

  printf ("ll: %f %f utm: [%s] %f,%f\n",
	  ll.latitude, ll.longitude,
	  utm.grid_zone, utm.x, utm.y);

  return(0);
}
#endif

void
get_grid_zone (LL *ll, char grid_zone[GRID_ZONE_LENGTH], double *lambda0)
{
  unsigned int zone_long;
  unsigned char zone_lat;

  /* Solve for the grid zone, returns the central meridian */

  /* First, let's take care of the polar regions */

  if (ll->latitude < -80) {
    if (ll->longitude < 0) {
      strcpy (grid_zone, "30A");
      *lambda0 = 0 * M_PI / 180.0;
    } else {
      strcpy (grid_zone, "31B");
      *lambda0 = 0 * M_PI / 180.0;
    }
    return;

  } else if (ll->latitude > 84) {
    if (ll->longitude < 0) {
      strcpy (grid_zone, "30Y");
      *lambda0 = 0 * M_PI / 180.0;
    } else {
      strcpy (grid_zone, "31Z");
      *lambda0 = 0 * M_PI / 180.0;
    }
    return;
  }

  /* Now the special "X" grid */

  if (ll->latitude > 72 && ll->longitude > 0 && ll->longitude < 42) {
    if (ll->longitude < 9) {
      *lambda0 = 4.5;
      strcpy (grid_zone, "31X");
    } else if (ll->longitude < 21) {
      *lambda0 = 15 * M_PI / 180.0;
      strcpy (grid_zone, "33X");
    } else if (ll->longitude < 33) {
      *lambda0 = 27 * M_PI / 180.0;
      strcpy (grid_zone, "35X");
    } else if (ll->longitude < 42) {
      *lambda0 = 37.5 * M_PI / 180.0;
      strcpy (grid_zone, "37X");
    }
  
    return;
  }

  /* Handle the special "V" grid */

  if (ll->latitude > 56 && ll->latitude < 64 &&
      ll->longitude > 0 && ll->longitude < 12) {
    if (ll->longitude < 3) {
      *lambda0 = 1.5 * M_PI / 180.0;
      strcpy (grid_zone, "31V");
    } else if (ll->longitude < 12) {
      *lambda0 = 7.5 * M_PI / 180.0;
      strcpy (grid_zone, "32V");
    }
      
    return;
  }

  /* The remainder of the grids follow the standard rule */

  zone_long = (unsigned int) ((ll->longitude - (-180.0)) / 6.0) + 1;
  *lambda0 = ((zone_long - 1) * 6.0 + (-180.0) + 3.0) * M_PI / 180.0;

  zone_lat = (unsigned char) ((ll->latitude - (-80.0)) / 8.0) + 'C';
  if (zone_lat > 'H')
    zone_lat++;
  if (zone_lat > 'N')
    zone_lat++;

  if (ll->latitude > 80)
    zone_lat = 'X';

  grid_zone[0] = ((unsigned char) (zone_long / 10)) + '0';
  grid_zone[1] = (zone_long % 10) + '0';
  grid_zone[2] = zone_lat;
  grid_zone[3] = 0;

  /* All done */
}

int
get_lambda0 (char grid_zone[GRID_ZONE_LENGTH], double *lambda0)
{
  unsigned int zone_long;
  unsigned char zone_lat;

  /* given the grid zone, sets the central meridian, lambda0 */

  /* Check the grid zone format */

  if (!isdigit(grid_zone[0]) || !isdigit(grid_zone[1])) {
#ifdef DEBUG
    fprintf (stderr, "Invalid grid zone format: [%s]\n", grid_zone);
#endif
    return(-1);
  }

  zone_long = (grid_zone[0] - '0') * 10 + (grid_zone[1] - '0');
  zone_lat = grid_zone[2];

  /* Take care of special cases */

  switch(zone_lat) {
  case 'A': case 'B': case 'Y': case 'Z':
    *lambda0 = 0;
    return(0);
    break;
  case 'V':
    switch (zone_long) {
    case 31:
      *lambda0 = 1.5 * M_PI / 180.0;
      return(0);
      break;
    case 32:
      *lambda0 = 7.5 * M_PI / 180.0;
      return(0);
      break;
    break;
    }
  case 'X':
    switch (zone_long) {
    case 31:
      *lambda0 = 4.5 * M_PI / 180.0;
      return(0);
      break;
    case 33:
      *lambda0 = 15 * M_PI / 180.0;
      return(0);
      break;
    case 35:
      *lambda0 = 27 * M_PI / 180.0;
      return(0);
      break;
    case 37:
      *lambda0 = 37.5 * M_PI / 180.0;
      return(0);
      break;
    case 32: case 34: case 36:
#ifdef DEBUG
      fprintf (stderr, "Zone %02d%c does not exist!\n", zone_long, zone_lat);
#endif
      return(-1);
      break;
    }
    break;
  }

  /* Now handle standard cases */

  *lambda0 = ((zone_long - 1) * 6.0 + (-180.0) + 3.0) * M_PI / 180.0;

  /* All done */

  return(0);
}

int
ll2utm(LL *ll, UTM *utm, unsigned char datum)
{
  double a, b, f, e, e2, e4, e6;
  double phi, lambda, lambda0, phi0, k0;
  double t, rho, m, x, y, k, mm, mm0;
  double aa, aa2, aa3, aa4, aa5, aa6;
  double ep2, nn, tt, cc;
  char grid_zone[GRID_ZONE_LENGTH];

  /* Converts lat/long to UTM, using the specified datum */

  switch (datum) {
  case CLARKE_1866_DATUM:
    a = 6378206.4;
    b = 6356583.8;
    break;
  case GRS_80_DATUM:
    a = 6378137;
    b = 6356752.3;
    break;
  case WGS_84_DATUM:
    a = 6378137.0;		/* semimajor axis of ellipsoid (meters) */
    b = 6356752.31425;		/* semiminor axis of ellipsoid (meters) */
    break;
  default:
#ifdef DEBUG
   fprintf (stderr, "Unknown datum: %d\n", datum);
#endif
   return(-1);
  }

  /* Calculate flatness and eccentricity */

  f = 1 - (b/a);
  e2 = 2*f - f*f;
  e = sqrt(e2);
  e4 = e2*e2;
  e6 = e4*e2;

  /* Convert latitude/longitude to radians */
  
  phi = (ll->latitude) * M_PI / 180.0;
  lambda = (ll->longitude) * M_PI / 180.0;

  /* Figure out the UTM zone, as well as lambda0 */

  (void)get_grid_zone (ll, grid_zone, &lambda0);
  phi0 = 0.0;

  /* See if this will use UTM or UPS */

  if (ll->latitude > 84.0) {

    /* use Universal Polar Stereographic Projection (north polar aspect) */

    k0 = 0.994;

    t = sqrt(((1-sin(phi))/(1+sin(phi))) *
	     pow((1+e*sin(phi))/(1-e*sin(phi)), e));
    rho = 2 * a * k0 * t / sqrt(pow(1+e,1+e) * pow(1-e,1-e));
    m = cos(phi) / sqrt (1 - e2 * sin(phi)*sin(phi));

    x = rho * sin(lambda - lambda0);
    y = -rho * cos(lambda - lambda0);
    k = rho * a * m;

    /* Apply false easting/northing */

    x += 2000000;
    y += 2000000;

  } else if (ll->latitude < -80.0) {

    /* use Universal Polar Stereographic Projection (south polar aspect) */

    phi = -phi;
    lambda = -lambda;
    lambda0 = -lambda0;

    k0 = 0.994;

    t = sqrt(((1-sin(phi))/(1+sin(phi))) *
	     pow((1+e*sin(phi))/(1-e*sin(phi)), e));
    rho = 2 * a * k0 * t / sqrt(pow(1+e,1+e) * pow(1-e,1-e));
    m = cos(phi) / sqrt (1 - e2 * sin(phi)*sin(phi));

    x = rho * sin(lambda - lambda0);
    y = -rho * cos(lambda - lambda0);
    k = rho * a * m;

    x = -x;
    y = -y;

    /* Apply false easting/northing */

    x += 2000000;
    y += 2000000;

  } else {

    /* Use UTM */

    /* set scale on central median (0.9996 for UTM) */
    
    k0 = 0.9996;

    mm = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi -
	      (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi) +
	      (15*e4/256 + 45*e6/1024) * sin(4*phi) -
	      (35*e6/3072) * sin(6*phi));

    mm0 = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi0 -
	       (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi0) +
	       (15*e4/256 + 45*e6/1024) * sin(4*phi0) -
	       (35*e6/3072) * sin(6*phi0));

    aa = (lambda - lambda0) * cos(phi);
    aa2 = aa*aa;
    aa3 = aa2*aa;
    aa4 = aa2*aa2;
    aa5 = aa4*aa;
    aa6 = aa3*aa3;

    ep2 = e2 / (1 - e2);
    nn = a / sqrt(1 - e2*sin(phi)*sin(phi));
    tt = tan(phi) * tan(phi);
    cc = ep2 * cos(phi) * cos(phi);

    k = k0 * (1 + (1+cc)*aa2/2 + (5-4*tt+42*cc+13*cc*cc-28*ep2)*aa4/24.0
	      + (61-148*tt+16*tt*tt)*aa6/720.0);
    x = k0 * nn * (aa + (1-tt+cc)*aa3/6 +
		   (5-18*tt+tt*tt+72*cc-58*ep2)*aa5/120.0);
    y = k0 * (mm - mm0 + nn * tan(phi) * 
	      (aa2/2 + (5-tt+9*cc+4*cc*cc)*aa4/24.0
	       + (61 - 58*tt + tt*tt + 600*cc - 330*ep2)*aa6/720));

    /* Apply false easting and northing */

    x += 500000.0;
    if (y < 0.0)
      y += 10000000;
  }

  /* Set entries in UTM structure */

  memcpy (utm->grid_zone, grid_zone, GRID_ZONE_LENGTH);
  utm->x = x;
  utm->y = y;

  /* done */

  return(0);
}

int
utm2ll (UTM *utm, LL *ll, unsigned char datum)
{
  double a, b, f, e, e2, e4, e6, e8;
  double lambda0, x, y, k0, rho, t, chi, phi, phi1, phit;
  double lambda, phi0, e1, e12, e13, e14;
  double mm, mm0, mu, ep2, cc1, tt1, nn1, rr1;
  double dd, dd2, dd3, dd4, dd5, dd6;

  unsigned int zone_long;
  unsigned char zone_lat;

  /* Converts UTM to lat/long, using the specified datum */

  switch (datum) {
  case CLARKE_1866_DATUM:
    a = 6378206.4;
    b = 6356583.8;
    break;
  case GRS_80_DATUM:
    a = 6378137;
    b = 6356752.3;
    break;
  case WGS_84_DATUM:
    a = 6378137.0;		/* semimajor axis of ellipsoid (meters) */
    b = 6356752.31425;		/* semiminor axis of ellipsoid (meters) */
    break;
  default:
#ifdef DEBUG
   fprintf (stderr, "Unknown datum: %d\n", datum);
#endif
   return(-1);
  }

  /* Calculate flatness and eccentricity */

  f = 1 - (b/a);
  e2 = 2*f - f*f;
  e = sqrt(e2);
  e4 = e2*e2;
  e6 = e4*e2;
  e8 = e4*e4;

  /* Given the UTM grid zone, generate a baseline lambda0 */

  if (get_lambda0 (utm->grid_zone, &lambda0) < 0) {
#ifdef DEBUG
    fprintf (stderr, "unable to translate UTM to LL\n");
#endif
    return(-1);
  }

  zone_long = (utm->grid_zone[0] - '0') * 10 + (utm->grid_zone[1] - '0');
  zone_lat = utm->grid_zone[2];

  /* Take care of the polar regions first. */

  switch(zone_lat) {
  case 'Y': case 'Z':		/* north polar aspect */

    /* Subtract the false easting/northing */

    x = utm->x - 2000000;
    y = utm->y - 2000000;

    /* Solve for inverse equations */

    k0 = 0.994;
    rho = sqrt (x*x + y*y);
    t = rho * sqrt(pow(1+e,1+e) * pow(1-e,1-e)) / (2*a*k0);

    /* Solve for latitude and longitude */

    chi = M_PI_2 - 2 * atan(t);
    phit = chi + (e2/2 + 5*e4/24 + e6/12 + 13*e8/360) * sin(2*chi) +
      (7*e4/48 + 29*e6/240 + 811*e8/11520) * sin(4*chi) +
      (7*e6/120 + 81*e8/1120) * sin(6*chi) +
      (4279*e8/161280) * sin(8*chi);

    do {
      phi = phit;
      phit = M_PI_2 - 2*atan(t*pow((1-e*sin(phi))/(1+e*sin(phi)),e/2));
    } while (fabs(phi-phit) > LOWER_EPS_LIMIT);

    lambda = lambda0 + atan2(x,-y);
    break;

  case 'A': case 'B':		/* south polar aspect */

    /* Subtract the false easting/northing */

    x = -(utm->x - 2000000);
    y = -(utm->y - 2000000);

    /* Solve for inverse equations */

    k0 = 0.994;
    rho = sqrt (x*x + y*y);
    t = rho * sqrt(pow(1+e,1+e) * pow(1-e,1-e)) / (2*a*k0);

    /* Solve for latitude and longitude */

    chi = M_PI_2 - 2 * atan(t);
    phit = chi + (e2/2 + 5*e4/24 + e6/12 + 13*e8/360) * sin(2*chi) +
      (7*e4/48 + 29*e6/240 + 811*e8/11520) * sin(4*chi) +
      (7*e6/120 + 81*e8/1120) * sin(6*chi) +
      (4279*e8/161280) * sin(8*chi);

    do {
      phi = phit;
      phit = M_PI_2 - 2*atan(t*pow((1-e*sin(phi))/(1+e*sin(phi)),e/2));
    } while (fabs(phi-phit) > LOWER_EPS_LIMIT);

    phi = -phi;
    lambda = -(-lambda0 + atan2(x,-y));

    break;

  default:

    /* Now take care of the UTM locations */

    k0 = 0.9996;

    /* Remove false eastings/northings */

    x = utm->x - 500000;
    y = utm->y;

    if (zone_lat > 'B' && zone_lat < 'N')   /* southern hemisphere */
      y -= 10000000;

    /* Calculate the footpoint latitude */

    phi0 = 0.0;
    e1 = (1 - sqrt(1-e2))/(1 + sqrt(1-e2));
    e12 = e1*e1;
    e13 = e1*e12;
    e14 = e12*e12;

    mm0 = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi0 -
	       (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi0) +
	       (15*e4/256 + 45*e6/1024) * sin(4*phi0) -
	       (35*e6/3072) * sin(6*phi0));
    mm = mm0 + y/k0;
    mu = mm/(a*(1-e2/4-3*e4/64-5*e6/256));

    phi1 = mu + (3*e1/2 - 27*e13/32) * sin(2*mu) +
      (21*e12/16 - 55*e14/32) * sin(4*mu) +
      (151*e13/96) * sin(6*mu) +
      (1097*e14/512) * sin(8*mu);

    /* Now calculate lambda and phi */

    ep2 = e2/(1-e2);
    cc1 = ep2*cos(phi1)*cos(phi1);
    tt1 = tan(phi1)*tan(phi1);
    nn1 = a / sqrt(1-e2*sin(phi1)*sin(phi1));
    rr1 = a * (1-e2)/pow(1-e2*sin(phi1)*sin(phi1), 1.5);
    dd = x / (nn1 * k0);

    dd2 = dd*dd;
    dd3 = dd*dd2;
    dd4 = dd2*dd2;
    dd5 = dd3*dd2;
    dd6 = dd4*dd2;

    phi = phi1 - (nn1*tan(phi1)/rr1) *
      (dd2/2 - (5+3*tt1+10*cc1-4*cc1*cc1-9*ep2)*dd4/24 +
       (61+90*tt1+298*cc1+45*tt1*tt1-252*ep2-3*cc1*cc1)*dd6/720);
    lambda = lambda0 +
      (dd - (1+2*tt1+cc1)*dd3/6 +
       (5-2*cc1+28*tt1-3*cc1*cc1+8*ep2+24*tt1*tt1)*dd5/120)/ cos(phi1);
  }

  /* Convert phi/lambda to degrees */
  
  ll->latitude = phi * 180.0 / M_PI;
  ll->longitude = lambda * 180.0 / M_PI;
  
  /* All done */

  return(0);
}
