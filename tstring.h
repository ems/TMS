//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//


#ifndef tstring_H
#define tstring_H

#include <string.h>
#include "mm.h"

// A tiny memory-managed string class.
class TString
{
public:
	TString( const char *aS = NULL )	{ s = NULL; assign(aS); }
	TString( const TString &aS )		{ s = NULL; assign(aS.s); }

	~TString() { freeStr(); }

	TString	&assign( const char *aS = NULL )
	{
		if( s != aS )
		{
			freeStr();
			register size_t len;
			if( aS != NULL && (len = strlen(aS)) != 0 )
			{
#ifdef MMenableMemoryManager
				MMallocator *a = getAllocator(len+1);
				s = (a != NULL ? (char *)a->allocBuf() : (char *)operator new(len + 1));
#else
				s = (char *)operator new(len+1);
#endif // MMenableMemoryManager
				memcpy( s, aS, len );
				s[len] = 0;
			}
		}
		return *this;
	}

	TString &assign( const TString &aS )	{ return assign(aS.s); }
	TString &operator=( const TString &aS ) { return assign(aS.s); }
	TString &operator=( const char *aS )	{ return assign(aS); }

	bool operator==( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) == 0; }
	bool operator!=( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) != 0; }
	bool operator< ( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) < 0; }
	bool operator<=( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) <= 0; }
	bool operator> ( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) > 0; }
	bool operator>=( const TString &aS ) const { return strcmp(getStr(), aS.getStr()) >= 0; }

	bool operator==( const char *aS ) const { return strcmp(getStr(), aS) == 0; }
	bool operator!=( const char *aS ) const { return strcmp(getStr(), aS) != 0; }
	bool operator< ( const char *aS ) const { return strcmp(getStr(), aS) < 0; }
	bool operator<=( const char *aS ) const { return strcmp(getStr(), aS) <= 0; }
	bool operator> ( const char *aS ) const { return strcmp(getStr(), aS) > 0; }
	bool operator>=( const char *aS ) const { return strcmp(getStr(), aS) >= 0; }

	const char *getStr() const { return s != NULL ? s : ""; }
	bool empty() const { return *getStr() == 0; }

	size_t	hash() const
	{
		register size_t h = 0;
		register const char *p;
		for( p = getStr(); *p != 0; ++p )
			h = ((h<<1) ^ *p);
		return h;
	}

	size_t	length() const { return strlen(getStr()); }

	void	trim();

private:
	char	*s;

#ifdef MMenableMemoryManager
	enum { numAllocators = 20 };

	MMallocator	*getAllocator( const int len )
	{
		const int i = (len + sizeof(char *)) / sizeof(char *);
		MMallocator	*a;
		if( i < numAllocators )
		{
			if( allocators[i] == NULL )
				allocators[i] = new MMallocator(i * sizeof(char *));
			a = allocators[i];
		}
		else
			a = NULL;
		return a;
	}
#endif // MMenableMemoryManager
	void	freeStr()
	{
		if( s != NULL )
		{
#ifdef MMenableMemoryManager
			MMallocator *a = getAllocator(strlen(s)+1);
			if( a != NULL ) a->freeBuf(s);
			else operator delete(s);
#else
			operator delete(s);
#endif // MMenableMemoryManager
			s = NULL;
		}
	}
#ifdef MMenableMemoryManager
	static MMallocator	*allocators[numAllocators];
#endif // MMenableMemoryManager
};

#endif // tstring_H