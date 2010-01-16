//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef ParseStream_H
#define ParseStream_H

#include <istream>
#include <ctype.h>

using namespace std;

class ParseStream
{
	// This class provides a simple wrapper around a C++ stream so that getChar operations can
	// be done extremely fast.  All C++ streams call a function to get a char and this results
	// in extremely slow parsers.  This wrapper class allows the convenience of C++ with the
	// performance of C for getting characters from a file one at at time.
	//
	// In keeping with this stripped down approach, it does not support reading any other data
	// type other than chars.  It assumes that your lexer and parser will handle other data
	// types according to your grammar.
	//
	// For convenience, it also properly keeps track of the line number.
	//
	// Unlike a pure C implementation, get() returns '\0' on the end of file.  This makes it
	// easier to write while( (c = ps.get()) )... to get a loop on all chars in the file.
	// Also, putback and unget are only guaranteed to work for one char only.
public:
	ParseStream( istream &aIS ) : is(aIS), p(buf), lineNo(1) { *p = 0; }

	operator const void *()			{ return (void *)(*p || is); }
	int	good() const				{ return (*p || is); }

	char		get()				{ return rawGet(); }
	ParseStream	&get( char &c )		{ c = rawGet(); return *this; }
	ParseStream	&putback( char c )	{ rawPutback(c); return *this; }
	ParseStream	&unget()			{ rawUnget(); return *this; }
	size_t		line() const		{ return lineNo; }
	void		eatwhite()
	{
		while( isspace(rawGet()) ) continue;
		rawUnget();
	}

private:
	char	rawGet()
	{
		char c;
		if( *p == 0 )
		{
			getBuf();
			if( (c = *p) != 0 )
				++p;
		}
		else
			c = *p++;
		if( c == '\n' )
			++lineNo;
		return c;
	}
	void	rawPutback( char c )
	{
		if( p != buf && (*--p = c) == '\n' )
			--lineNo;
	}
	void	rawUnget()
	{
		if( p != buf && *--p == '\n' )
			--lineNo;
	}

	void	getBuf()
	{
		p = buf;
		if( is )
		{
			is.read( buf, sizeof(buf) - 1 );
			buf[is.gcount()] = 0;
		}
		else
			*p = 0;
	}

	istream	&is;			// stream we are managing.
	char	buf[256], *p;	// buffer and pointer to next char.
	size_t	lineNo;			// current line number.
};

#endif // ParseStream_H