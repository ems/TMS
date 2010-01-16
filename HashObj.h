//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef HashObj_H
#define HashObj_H

#ifndef mm_H
#include "mm.h"
#endif
#ifndef nextPrime_H
#include "nextPrime.h"
#endif

#include <new.h>

template <class cKey, class cValue>
class HashObj
{
private:
	enum { mmHTMax = 128 };

  struct	Element
  {
	  Element( const cKey &aKey, const cValue &aValue ) : key(aKey), value(aValue) {}
    cKey	key;
	cValue	value;

    Element	*bNext;

    void	addToBktList( Element *&head )
      {
		bNext = head;
		head = this;
      }
    void	removeFromBktList( Element *&head, Element *bPrev )
      {
		if( bPrev )
		  bPrev->bNext = bNext;
		else
		  head = bNext;
      }
	void	addToSList( Element *&tail )
	{
		if( tail )
		{
			bNext = tail->bNext;
			tail->bNext = this;
		}
		else
			bNext = this;
		tail = this;
	}
  };

public:
	typedef cKey keytype;
	typedef cValue valuetype;
	class iterator;
	friend class iterator;

  class iterator
  {
  public:
	  iterator( const HashObj *aPC = NULL, Element *aElement = NULL ) : pc(aPC), b(aElement) {}
	  iterator( const iterator &i ) : pc(i.pc), b(i.b) {}
    iterator	operator++() // Prefix
	{ b = (b->bNext) ? b->bNext : pc->findNextEntry( b->key ); return *this; }
    iterator	operator++(int) // Postfix
	{ iterator save(*this); operator++(); return save; }
    iterator	&operator=( const iterator &i ) { pc = i.pc; b = i.b; return *this; }
    int		operator==( const iterator &i ) const { return b == i.b && pc == i.pc; }
    int		operator!=( const iterator &i ) const { return b != i.b || pc != i.pc; }

    const cKey	&operator*() const { return b->key; }
    cKey	&operator*() { return b->key; }

    bool	operator!() const { return b == NULL; }

	const cValue	&value() const { return b->value; }
	cValue	&value() { return b->value; }

	const cValue	&data() const { return value(); }
	cValue	&data() { return b->value; }

	operator bool() const { return b != NULL; }
  private:
	  const HashObj	*pc;
      Element	*b;
  };

  HashObj &operator=( const HashObj &c );
  HashObj &operator|=( const HashObj &c );

  iterator begin() const { return iterator(this, findFirstEntry()); }
  iterator end() const { return iterator(this, NULL); }
  
  HashObj( size_t aSize = 0 );
  HashObj( HashObj &c );
  ~HashObj() { clear(); }
  void		remove( const cKey &p )
  {
	// Remove fast.
  	if( size() != 0 )
	{
		size_t i = hash(p);
		register Element *b;
		if( (b = ht[i]) != NULL )
		{
			if( b->key == p )
			{
				ht[i] = b->bNext;
				deleteElement( b );
				--numElements;
			}
			else
				removeNonFirst( b, p );
		}
	}
  }

  void		remove( const iterator &i ) { remove(*i); }

  iterator	find( const cKey &key ) const
    {
	  processPending();
	  size_t i;
      return iterator(this, numElements > 0 ? hashFindQuick(i, key) : NULL);
    }
  bool contains( const cKey &key ) const { return find(key) != end(); }
  iterator	insert( const cKey &key, const cValue &value  )
    {
	  register Element *e;
	  size_t i;
	  if( (e = hashFindQuick(i, key)) != NULL )	// key already exists - just update value.
		  e->value = value;
	  else
	  {
		  // Add a new hash entry.
		  // If we need to grow, just park the new element on a list.
		  // We will merge it with the hash table when we do a search.
		  e = newElement( key, value );
		  ++numElements;
		  if( shouldGrow() )
			e->addToSList( tailPending );
		  else
			e->addToBktList( ht[i] );
	  }
	  return iterator(this, e);
    }

  size_t	size() const { processPending(); return numElements; }
  bool		empty() const { return size() == 0; }
  size_t	getHTMax() const { return htMax; }

  void		rightSize() { processPending(); if( shouldShrink() ) resize(shrinkSize()); }
  void		resize( size_t htMaxNew );
  void 		clear();

private:
	Element *newElement( const cKey &key, const cValue &value )
	{
#ifdef MMenableMemoryManager
		return new (allocator->allocBuf()) Element(key, value);
#else
		return new Element(key, value);
#endif // MMenableMemoryManager
	}

	void deleteElement( Element *e )
	{
#ifdef MMenableMemoryManager
		e->~Element();
		allocator->freeBuf( (void *)e );
#else
		delete e;
#endif // MMenableMemoryManager
	}

	size_t	hash( const cKey &p ) const { return p.hash() % htMax; }

  Element	*findHead( Element *e, const cKey &p ) const;
  Element	*findNextEntry( const cKey &key ) const
  {
	  Element *e = NULL;
	  register size_t i = hash(key);
	  for( ++i; i < htMax; ++i )
		  if( ht[i] != NULL )
		  {
			  e = ht[i];
			  break;
		  }
		return e;
  }
  Element	*findFirstEntry() const
  {
//	  processPending();
	  ((HashObj *)this)->rightSize();
	  Element *e = NULL;
	  if( numElements != 0 )
	  {
		register size_t i;
		for( i = 0; i < htMax; ++i )
		  if( ht[i] != NULL )
		  {
			  e = ht[i];
			  break;
		  }
	  }
	  return e;
  }

  Element	**ht;		// hash table.
  Element	*tailPending; // list of items added but not put into the ht yet.
  size_t	htMax;		// maximum size of the hash table.
  size_t	numElements;// number of elements in collection.

  Element	*hashFindQuick( size_t &i, const cKey &key ) const
  {
      register Element   *e;
	  if( htMax == 0 )
		  e = NULL;
	  else if( (e = ht[i = hash(key)]) != NULL && !(e->key == key) )
	  {
		 if( (e = e->bNext) != NULL && !(e->key == key) )
			e = findHead(e, key);
	  }
	  return e;
  }

  bool		shouldGrow() const { return htMax - (htMax>>2) <= numElements; }
  bool		shouldShrink() const { return numElements <= (htMax>>1); }
  size_t	growSize() const { return numElements + (numElements>>1) + 1; }
  size_t	shrinkSize() const { return numElements + (numElements>>2) + 1; }

  void		removeNonFirst( Element *bPrev, const cKey &key );
  void	    addPending();
  void		processPending() const { if( tailPending ) ((HashObj *)(this))->addPending(); }

#ifdef MMenableMemoryManager
  static	MMallocator *allocator;
#endif // MMenableMemoryManager
};

#ifdef MMenableMemoryManager
template <class cKey, class cValue>
MMallocator *HashObj<cKey, cValue>::allocator = NULL;
#endif // MMenableMemoryManager

template < class cKey, class cValue >
void HashObj<cKey, cValue>::resize( size_t htMaxNew )
{
	if( htMaxNew == 0 )
	{
		// Assume the hash table is already empty.
#ifdef MMenableMemoryManager
		if( htMax < mmHTMax )
		{
			if( ht != NULL )
				MMallocatorFactory::getAllocator( htMax * sizeof(ht[0]) )->freeBuf( ht );
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
		htNew = (Element **)MMallocatorFactory::getAllocator( htMaxNew * sizeof(htNew[0]) )->allocBuf();
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
			MMallocatorFactory::getAllocator( htMaxOld * sizeof(ht[0]) )->freeBuf( ht );
	}
	else
#endif // MMenableMemoryManager
		::operator delete( ht );
	ht = htNew;
}

template < class cKey, class cValue >
HashObj<cKey, cValue>::HashObj( size_t aSize )
{
#ifdef MMenableMemoryManager
	if( allocator == NULL )
		allocator = MMallocatorFactory::getAllocator( sizeof(Element) );
#endif

	ht = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;
	if( aSize > 0 )
		resize( aSize );
}

template < class cKey, class cValue >
HashObj<cKey, cValue>::HashObj( HashObj &c )
{
	ht = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;

	operator=( c );
}

template < class cKey, class cValue >
HashObj<cKey, cValue> &HashObj<cKey, cValue>::operator=( const HashObj &c )
{
	clear();
	c.processPending();
	if( c.numElements == 0 )
		return *this;
    resize( c.numElements + (c.numElements >> 2) + 1 );

    register typename HashObj::Element *b;
	register size_t i;
	for( i = 0; i < c.htMax; i++ )
		for( b = c.ht[i]; b != NULL; b = b->bNext )
		    insert( b->key, b->value );
	return *this;
}

template < class cKey, class cValue >
HashObj<cKey, cValue> &HashObj<cKey, cValue>::operator|=( const HashObj &c )
{
	c.processPending();

    register typename HashObj::Element *b;
	register size_t i;
	for( i = 0; i < c.htMax; i++ )
		for( b = c.ht[i]; b != NULL; b = b->bNext )
		    insert( b->key, b->value );
	return *this;
}

template < class cKey, class cValue >
typename HashObj<cKey, cValue>::Element *HashObj<cKey, cValue>::findHead( register typename HashObj::Element *b, const cKey &p ) const
{
  for( b = b->bNext; b != NULL; b = b->bNext )
    if( b->key == p )
      return b;
  return NULL;
}

template <class cKey, class cValue>
void HashObj<cKey, cValue>::removeNonFirst( register typename HashObj::Element *bPrev, const cKey &p )
{
  register typename HashObj::Element *b;
  for( b = bPrev->bNext; b != NULL; bPrev = b, b = b->bNext )
    if( b->key == p )
      {
        bPrev->bNext = b->bNext;
		deleteElement( b );
        --numElements;
        return;
      }
}

template < class cKey, class cValue >
void HashObj<cKey, cValue>::clear()
{
	if( numElements != 0 )
	{
		// Reset the hash table.
		register size_t i;
		register typename HashObj::Element *b, *bNext;

		// Delete all the elements and reset all the ht entries.
		for( i = 0; i < htMax; i++ )
		{
			for( b = ht[i]; b != NULL; b = bNext )
			{
				bNext = b->bNext;
				deleteElement( b );
			}
		}

		// Delete any elements in the pending list.
		if( tailPending )
		{
			typename HashObj::Element *headPending = tailPending->bNext;
			tailPending->bNext = NULL;
			tailPending = NULL;

			for( b = headPending; b != NULL; b = bNext )
			{
				bNext = b->bNext;
				deleteElement( b );
			}
		}

		numElements = 0;
	}
	resize( 0 );
}

template < class cKey, class cValue >
void HashObj<cKey, cValue>::addPending()
{
	// Actually resize the hash table to add the new elements.
	typename HashObj::Element *headPending = tailPending->bNext;
	tailPending->bNext = NULL;
	tailPending = NULL;

	resize( numElements + (numElements>>1) + 1 );

	register typename HashObj::Element *e, *eFound, *eNext;
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
			// Just update the value.
			eFound->value = e->value;
			deleteElement( e );
			--numElements;	// correct the numElements.
		}
	}
}

#endif // HashObj_H