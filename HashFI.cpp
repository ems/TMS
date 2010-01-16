//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "HashFI.h"
#include <limits.h>

MMinitMax( HashFIBase::Element, 4096 );

#ifdef MMenableMemoryManager
static	MMallocator	*htFIallocators[HashFIBase::mmHTMax];
#define getAllocator(size)	(htFIallocators[(size)] != NULL ? htFIallocators[(size)] : htFIallocators[size] = MMallocatorFactory::getAllocator((size) * sizeof(ht[0])))
#endif

void HashFIBase::resize( size_t htMaxNew )
{
	if( htMaxNew == 0 )
	{
#ifdef MMenableMemoryManager
		if( htMax <= mmHTMax )
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
    if( htMaxNew < 3 )
        htMaxNew = 3;
    else
		htMaxNew = nextPrime( htMaxNew );
    if( htMaxNew == htMax )
		return;

#ifdef MMenableMemoryManager
	if( htMax <= mmHTMax )
	{
		if( ht != NULL )
			getAllocator(htMax)->freeBuf( ht );
	}
	else
#endif // MMenableMemoryManager
		::operator delete( ht );

	htMax = htMaxNew;
#ifdef MMenableMemoryManager
	if( htMax <= mmHTMax )
		ht = (Element **)getAllocator(htMax)->allocBuf();
	else
#endif // MMenableMemoryManager
		ht = (Element **)::operator new( htMax * sizeof(ht[0]) );

	// Unroll the initialization loop for better perfromance.
    register Element **pe = &ht[0];
	for( register size_t m8 = htMax >> 3; m8 != 0; --m8 )
	{
		*pe++ = NULL; *pe++ = NULL; *pe++ = NULL; *pe++ = NULL;
		*pe++ = NULL; *pe++ = NULL; *pe++ = NULL; *pe++ = NULL;
	}
	switch( htMax & 7 )
	{
	case 7:	*pe++ = NULL;
	case 6:	*pe++ = NULL;
	case 5:	*pe++ = NULL;
	case 4:	*pe++ = NULL;
	case 3:	*pe++ = NULL;
	case 2:	*pe++ = NULL;
	case 1:	*pe++ = NULL;
	}

    register Element *b;
	for( b = head; b != NULL; b = b->next )
		b->addToBktList( ht[hash(b->key)] );
}

HashFIBase::HashFIBase( size_t aSize )
{
	ht = NULL;
	head = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;
	if( aSize > 0 )
		resize( aSize );
}

HashFIBase::HashFIBase( const HashFIBase &c )
{
	ht = NULL;
	head = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;

	operator=( c );
}

HashFIBase &HashFIBase::operator=( const HashFIBase &c )
{
	clear();
	c.processPending();
	if( c.numElements == 0 )
		return *this;
    resize( c.numElements + (c.numElements >> 2) + 1 );

	Element	*tail = NULL;
    register Element *b, *bNew;
	for( b = c.head; b != NULL; b = b->next )
	{
		bNew = new Element;
		bNew->key = b->key;
		bNew->data = b->data;
		bNew->addToBktList( ht[hash(bNew->key)] );
		bNew->addToListTail( head, tail );
	}
	numElements = c.numElements;
	return *this;
}

HashFIBase &HashFIBase::operator|=( const HashFIBase &c )
{
	c.processPending();
    register Element *b;
	for( b = c.head; b != NULL; b = b->next )
	    insert( b->key, b->data );
	return *this;
}

HashFIBase &HashFIBase::operator-=( const HashFIBase &c )
{
	c.processPending();
	register Element *b;
	for( b = c.head; b != NULL; b = b->next )
		remove( b->key );
	return *this;
}

bool	HashFIBase::operator==( const HashFIBase &c ) const
{
	if( size() != c.size() )
		return false;
	register Element *b, *bFound;
	for( b = head; b != NULL; b = b->next )
		if( !(bFound = c.find(b->key)) || b->data != bFound->data )
			return false;
	return true;
}

HashFIBase::Element *HashFIBase::findHead( register HashFIBase::Element *b, const unsigned long p ) const
{
  for( b = b->bNext; b != NULL; b = b->bNext )
    if( b->key == p )
      return b;
  return NULL;
}

void HashFIBase::removeNonFirst( register Element *bPrev, register unsigned long p )
{
  register Element *b;
  for( b = bPrev->bNext; b != NULL; bPrev = b, b = b->bNext )
    if( b->key == p )
      {
        bPrev->bNext = b->bNext;
		b->removeFromList( head );
		delete b;
        --numElements;
        return;
      }
}

void HashFIBase::clear()
{
	// Reset the HashFI table.
	register Element *b, *bNext;

	// Delete all the elements.
	for( b = head; b != NULL; b = bNext )
	{
		bNext = b->next;
		delete b;
	}

	// Delete any elements in the pending list.
	if( tailPending )
	{
		Element *headPending = tailPending->next;
		tailPending->next = NULL;
		tailPending = NULL;

		for( b = headPending; b != NULL; b = bNext )
		{
			bNext = b->next;
			delete b;
		}
	}

	// Delete the rest of the hash table.
	head = NULL;
	tailPending = NULL;
	numElements = 0;
	resize( 0 );
}

void HashFIBase::addPending()
{
	// Actually resize the hash table to add the new elements.
	Element *headPending = tailPending->next;
	tailPending->next = NULL;
	tailPending = NULL;

	if( shouldGrow() )
		resize( growSize() );

	register Element *e, *eFound, *eNext;
	size_t i;

	for( e = headPending; e != NULL; e = eNext )
	{
		eNext = e->next;

		if( (eFound = hashFindQuick(i, e->key)) == NULL )
		{
			// This is a unique key.  Add the element into the collection.
			e->addToBktList( ht[i] );
			e->addToList( head );
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

HashFIBase::~HashFIBase()
{
  clear();
#ifdef MMenableMemoryManager
  if( htMax <= mmHTMax )
  {
	  if( ht != NULL )
		getAllocator(htMax)->freeBuf( ht );
  }
  else
#endif // MMenableMemoryManager
	  ::operator delete( ht );
}
