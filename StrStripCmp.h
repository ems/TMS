#ifndef StrStripCmp_H
#define StrStipCmp_H

#include <ctype.h>

//----------------------------------------------------------------------------------------
// Compare strings while ignoring leading spaces, consecutive spaces and trailing spaces.
//
inline int StrStripCmp( const char *x, const char *y )
{
	// Skip leading spaces.
	register const char *xCur = x;
	while( isspace(*xCur) )
		++xCur;

	register const char *yCur = y;
	while( isspace(*yCur) )
		++yCur;

	// Try to find a non-equal character.
	// Also skip consecutive strings of spaces.
	for( ; *xCur && *yCur && *xCur == *yCur; )
	{
		do ++xCur; while( isspace(*(xCur-1)) && isspace(*xCur) );
		do ++yCur; while( isspace(*(yCur-1)) && isspace(*yCur) );
	}

	// Check if the strings are equal due to trailing spaces.
	if( !*yCur )
	{
		while( isspace(*xCur) )
			++xCur;
	}

	if( !*xCur )
	{
		while( isspace(*yCur) )
			++yCur;
	}

	return *xCur < *yCur ? -1 : *yCur < *xCur ? 1 : 0;
}

#endif // StrStripCmp_H

