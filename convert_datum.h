
#ifndef convert_datum_H
#define convert_datum_H
/*
 * convert_datum.h
 * Peter Daly
 * MIT Ocean Acoustics
 * Copyright (C) 1998 Massachusetts Institute of Technology
 * All Rights Reserved
 */
#define GRID_ZONE_LENGTH 4
#define LOWER_EPS_LIMIT 1e-14	/* kind of arbitrary... */

#define CLARKE_1866_DATUM 0
#define GRS_80_DATUM      1
#define WGS_84_DATUM      2

class UTM
{
public:
	UTM() { set(0.0, 0.0, ""); }
	UTM( const double aX, const double aY, register const char *aGrid_zone ) { set(aX, aY, aGrid_zone); }
	UTM( const UTM &u )		{ set(u.x, u.y, u.grid_zone); }

	void	set( const double aX, const double aY, register const char *aGrid_zone )
	{
		x = aX;
		y = aY;
		for( register char *p = grid_zone; *aGrid_zone; )
			*p++ = *aGrid_zone++;
		*p = 0;
	}

	bool	valid() const { return grid_zone[0] != 0; }
  char grid_zone[GRID_ZONE_LENGTH];
  double x, y;
};

class LL
{
public:
  double latitude, longitude;
};

void get_grid_zone (LL*, char *, double *);
int get_lambda0 (char *, double *);
int ll2utm (LL *, UTM *, unsigned char);
int utm2ll (UTM *, LL *, unsigned char);

inline int ll2utm( const LL &ll, UTM &utm, unsigned char d = WGS_84_DATUM )
{
	return ll2utm( const_cast<LL *>(&ll), &utm, d );
}
inline int utm2ll( const UTM &utm, LL &ll, unsigned char d = WGS_84_DATUM )
{
	return utm2ll( const_cast<UTM *>(&utm), &ll, d );
}

#endif // convert_datum_H

