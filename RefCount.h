//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef RefCount_H
#define RefCount_H

#ifndef _INC_STDLIB
#	include <stdlib.h>
#endif
#ifndef mm_H
#	include "mm.h"
#endif

#ifdef MMenableMemoryManager
extern MMallocator	*refAllocator;
#endif // MMenableMemoryManager

template < class T >
class Ref
{
	struct RefCount
	{
		int		count;
		T		*data;

		RefCount() : count(0), data(NULL) {}
		RefCount( T *aData ) : count(1), data(aData) {}
		~RefCount() { if(data) delete data; }
	};

public:
	Ref() { rc = NULL; }
	Ref( T *data ) { rc = NULL; operator=(data); }
	Ref( const Ref &r ) { rc = NULL; operator=(r); }
	~Ref()
	{
		if( rc && --rc->count == 0 )
			deleteRefCount( rc );
	}
	bool operator()() const { return rc != NULL; }
	bool operator!() const { return rc == NULL; }

	Ref	&operator=( const Ref &rIn )
	{
		Ref &r = const_cast<Ref &>(rIn);
		if( rc && --rc->count == 0 )
			deleteRefCount( rc );
		if( (rc = r.rc) != NULL )
			rc->count++;
		return *this;
	}
	Ref &operator=( T *data )
	{
		if( rc && --rc->count == 0 )
			deleteRefCount( rc );
		if( data != NULL )
			rc = newRefCount( data );
		else
			rc = NULL;
		return *this;
	}
	T &operator*() const { return *(rc->data); }
	T *operator->() const { return rc->data; }
	T *data() const { return rc->data; }

	bool	operator==( const Ref &rIn ) const
	{
		return (rc == NULL && rIn.rc == NULL) || rc->data == rIn->rc->data || *(rc->data) == *(rIn->rc->data);
	}

private:
	RefCount	*rc;
	RefCount	*newRefCount( T *data )
	{
#ifdef MMenableMemoryManager
		return new (refAllocator->allocBuf()) RefCount(data);
#else
		return new RefCount(data);
#endif // MMenableMemoryManager
	}
	void deleteRefCount( RefCount *rc )
	{
#ifdef MMenableMemoryManager
		rc->~RefCount();
		refAllocator->freeBuf(rc);
#else
		delete rc;
#endif // MMenableMemoryManager
	}
};

#endif // RefCount_H