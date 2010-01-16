//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "Hash.h"
#include <limits.h>

MMinitMax( HashBase::Element, 4096 );

#ifdef MMenableMemoryManager
static	MMallocator	*htAllocators[HashBase::mmHTMax];
#define getAllocator(size)	(htAllocators[(size)] != NULL ? htAllocators[(size)] : htAllocators[(size)] = MMallocatorFactory::getAllocator((size) * sizeof(ht[0])))
#endif

void HashBase::resize( size_t htMaxNew )
{
	if( htMaxNew == 0 )
	{
		// Assume the hash table is already empty.
#ifdef MMenableMemoryManager
		if( htMax < mmHTMax )
		{
			if( ht != NULL )
				getAllocator(htMax)->freeBuf( ht );
		}
		else
#endif // MMenableMemoryManager
		  ::operator delete( ht );
		htMax = 0;
		ht = NULL;
		return;
	}
	if( htMaxNew <= 3 )
		htMaxNew = 3;
	htMaxNew = nextPrime( htMaxNew );
	if( htMaxNew == htMax )
		return;

	Element **htNew;

#ifdef MMenableMemoryManager
	if( htMaxNew <= mmHTMax )
		htNew = (Element **)getAllocator(htMaxNew)->allocBuf();
	else
#endif // MMenableMemoryManager
		htNew = (Element **)::operator new( htMaxNew * sizeof(ht[0]) );

	// Unroll the initialization loop for more speed.
	register Element **pe = htNew;
	for( register size_t m8 = htMaxNew >> 3; m8 > 0; --m8 )
	{
		*pe++ = NULL; *pe++ = NULL; *pe++ = NULL; *pe++ = NULL;
		*pe++ = NULL; *pe++ = NULL; *pe++ = NULL; *pe++ = NULL;
	}
	switch( htMaxNew & 7 )
	{
	case 7: *pe++ = NULL;
	case 6: *pe++ = NULL;
	case 5: *pe++ = NULL;
	case 4: *pe++ = NULL;
	case 3: *pe++ = NULL;
	case 2: *pe++ = NULL;
	case 1: *pe++ = NULL;
	}

	const size_t htMaxOld = htMax;
	htMax = htMaxNew;
    register Element *b, *bNext;
	for( register size_t i = 0; i < htMaxOld; i++ )
		for( b = ht[i]; b != NULL; b = bNext )
		{
			bNext = b->bNext;
			b->addToBktList( htNew[hash(b->key)] );
		}

#ifdef MMenableMemoryManager
	if( htMaxOld < mmHTMax )
	{
		if( ht != NULL )
			getAllocator(htMaxOld)->freeBuf( ht );
	}
	else
#endif // MMenableMemoryManager
		::operator delete( ht );
	ht = htNew;
}

HashBase::HashBase( size_t aSize )
{
	ht = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;
	if( aSize > 0 )
		resize( aSize );
}

HashBase::HashBase( const HashBase &c )
{
	ht = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;

	operator=( c );
}

HashBase &HashBase::operator=( const HashBase &c )
{
	clear();
	c.processPending();
	if( c.numElements == 0 )
		return *this;
    resize( c.numElements + (c.numElements >> 2) + 1 );

    register Element *b;
	register size_t i;
	for( i = 0; i < c.htMax; i++ )
		for( b = c.ht[i]; b != NULL; b = b->bNext )
		    insert( b->key, b->data );
	return *this;
}

HashBase &HashBase::operator|=( const HashBase &c )
{
	c.processPending();

    register Element *b;
	register size_t i;
	for( i = 0; i < c.htMax; i++ )
		for( b = c.ht[i]; b != NULL; b = b->bNext )
		    insert( b->key, b->data );
	return *this;
}

HashBase::Element *HashBase::findHead( register HashBase::Element *b, const unsigned long p ) const
{
  for( b = b->bNext; b != NULL; b = b->bNext )
    if( b->key == p )
      return b;
  return NULL;
}

void HashBase::removeNonFirst( register Element *bPrev, register unsigned long p )
{
	register Element *b;
	for( b = bPrev->bNext; b != NULL; bPrev = b, b = b->bNext )
		if( b->key == p )
		{
			bPrev->bNext = b->bNext;
			delete b;
			--numElements;
			return;
		}
}

void HashBase::clear()
{
	if( numElements != 0 )
	{
		// Reset the hash table.
		register size_t i;
		register Element *b, *bNext;

		// Delete all the elements and reset all the ht entries.
		for( i = 0; i < htMax; i++ )
		{
			for( b = ht[i]; b != NULL; b = bNext )
			{
				bNext = b->bNext;
				delete b;
			}
		}

		// Delete any elements in the pending list.
		if( tailPending )
		{
			Element *headPending = tailPending->bNext;
			tailPending->bNext = NULL;
			tailPending = NULL;

			for( b = headPending; b != NULL; b = bNext )
			{
				bNext = b->bNext;
				delete b;
			}
		}

		numElements = 0;
	}
	resize( 0 );
}

void HashBase::addPending()
{
	// Actually resize the hash table to add the new elements.
	Element *headPending = tailPending->bNext;
	tailPending->bNext = NULL;
	tailPending = NULL;

	if( shouldGrow() )
		resize( growSize() );

	register Element *e, *eFound, *eNext;
	size_t i;

	for( e = headPending; e != NULL; e = eNext )
	{
		eNext = e->bNext;

		if( (eFound = hashFindQuick(i, e->key)) == NULL )
		{
			// This is a unique key.  Add the element into the collection.
			e->addToBktList( ht[i] );
		}
		else
		{
			// The key is already in the collection.
			// Just update the data.
			eFound->data = e->data;
			delete e;
			--numElements;	// correct the numElements.
		}
	}
}
