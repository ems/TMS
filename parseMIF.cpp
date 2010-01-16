//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "parseMIF.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

MMinit( ParseMIF::Line );

static	const	char	*wSpace = " \t\r\n";

static	int	strcasecmp( register const char *s1, register const char *s2 )
{
	register int d = 0;
	while( *s1 && *s2 && (d = tolower((unsigned char)*s1) - tolower((unsigned char)*s2)) == 0 )
		++s1, ++s2;
	return d < 0 ? -1 : d > 0 ? 1 : 0;
}

void	ParseMIF::getLines( int numPoints, istream &is )
{
	// Parse a list of points and turn them into lines.
	char	szIn[512], *p;
	Line	*line, *linePrev = NULL;

	// Get the first point.
	line = new Line; line->addToList(lHead);
	if( !is.getline(szIn, sizeof(szIn)) ) return;
	if( (p = strtok(szIn, wSpace)) == NULL ) return;
	line->long1 = atof(p);
	if( (p = strtok(NULL, wSpace)) == NULL ) return;
	line->lat1 = atof(p);

	// Get the intervening points.
	while( --numPoints > 1 )
	{
		if( !is.getline(szIn, sizeof(szIn)) ) return;
		linePrev = line; line = new Line; line->addToList(lHead);
		if( (p = strtok(szIn, wSpace)) == NULL ) return;
		line->long1 = atof(p); linePrev->long2 = line->long1;
		if( (p = strtok(NULL, wSpace)) == NULL ) return;
		line->lat1  = atof(p); linePrev->lat2  = line->lat1;
	}

	// Get the last point.
	if( !is.getline(szIn, sizeof(szIn)) ) return;
	if( (p = strtok(szIn, wSpace)) == NULL ) return;
	line->long2 = atof(p);
	if( (p = strtok(NULL, wSpace)) == NULL ) return;
	line->lat2  = atof(p);
}

ParseMIF::ParseMIF( istream &is ) : lHead(NULL)
{
	// Extract all Line, Pline and Region tokens from the MapInfo .MAP file.
	// Convert them all to lines.
	char	szIn[512], *p;

	while( is.getline(szIn, sizeof(szIn)) )
	{
		if( (p = strtok(szIn, wSpace)) == NULL ) continue;

		if( strcasecmp(p, "Line") == 0 )
		{
			Line	*line = new Line; line->addToList(lHead);

			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			line->long1 = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			line->lat1 = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			line->long2 = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			line->lat2 = atof( p );
		}
		else if( strcasecmp(p, "Pline") == 0 )
		{
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			int	numPoints = atoi(p);
			if( numPoints <= 0 ) continue;
			getLines( numPoints, is );
		}
		else if( strcasecmp(p, "Region") == 0 )
		{
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			int numPolygons  = atoi(p);
			while( numPolygons-- > 0 )
			{
				// Get the number of points line.
				if( !is.getline(szIn, sizeof(szIn)) ) break;
				if( (p = strtok(szIn, wSpace)) == NULL ) continue;
				int	numPoints = atoi(p);
				if( numPoints <= 0 ) continue;
				getLines( numPoints, is );
			}
		}
		else if( strcasecmp(p, "Rect") == 0 )
		{
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			const double	long1 = atof( p );;
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			const double	lat1  = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			const double	long2 = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			const double	lat2  = atof( p );

			(new Line(lat1, long1, lat2, long1))->addToList(lHead);
			(new Line(lat2, long1, lat2, long2))->addToList(lHead);
			(new Line(lat2, long2, lat1, long2))->addToList(lHead);
			(new Line(lat1, long2, lat1, long1))->addToList(lHead);
		}
		else if( strcasecmp(p, "Elipse") == 0 )
		{
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			double	long1 = atof( p );;
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			double	lat1  = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			double	long2 = atof( p );
			if( (p = strtok(NULL, wSpace)) == NULL ) continue;
			double	lat2  = atof( p );

			// Approximate an elipse with a bunch of lines.
			const int		numLines = 32;

			// Find the center and dimensions of the elipse.
			const double	radMult = (2.0 * 3.14159265358979323846) / (double)numLines;
			const double	x = (long1 + long2) / 2.0, y = (lat1 + lat2) / 2.0;
			const double	xFactor = (long2 - long1) / 2.0, yFactor = (lat2 - lat1) / 2.0;

			// Draw a set of connecting lines to form the elipse.
			long1 = x + xFactor;
			lat1 = y;
			for( int i = 1; i < numLines; ++i )
			{
				long2 = x + cos(radMult * i) * xFactor;
				lat2 = y + sin(radMult * i) * yFactor;
				(new Line(lat1, long1, lat2, long2))->addToList(lHead);
				long1 = long2;
				lat1 = lat2;
			}

			// Join up the last point to the first.
			long2 = x + xFactor;
			lat2 = y;
			(new Line(lat1, long1, lat2, long2))->addToList(lHead);
		}
	}
}

ParseMIF::~ParseMIF()
{
	register Line *line, *lNext;
	for( line = lHead; line != NULL; line = lNext )
	{
		lNext = line->next;
		delete line;
	}
}