//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef HashObjFI_H
#define HashObjFI_H

#ifndef mm_H
#include "mm.h"
#endif
#ifndef nextPrime_H
#include "nextPrime.h"
#endif

#include <new.h>

template < class cKey, class cValue >
class HashObjFI
{
private:
  enum { mmHTMax = 128 };

  struct	Element
  {
    cKey	key;
	cValue	value;

	Element( const cKey &aKey, const cValue &aValue ) : key(aKey) , value(aValue) {}

    Element	*bNext;
	Element *next, *prev;

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
	void	addToList( Element *&head )
	{
		if( head )
			head->prev = this;
		next = head;
		prev = NULL;
		head = this;
	}
	void	addToListTail( Element *&head, Element *&tail )
	{
		if( tail )
			tail->next = this;
		else
			head = this;
		next = NULL;
		prev = tail;
		tail = this;
	}
	void	removeFromList( Element *&head )
	{
		if( prev )
			prev->next = next;
		else
			head = next;
		if( next )
			next->prev = prev;
	}

	void	addToSList( Element *&tail )
	{
		if( tail )
		{
			next = tail->next;
			tail->next = this;
		}
		else
			next = this;
		tail = this;
	}
  };

  public:
	typedef cKey keytype;
	typedef cValue valuetype;

  class iterator
  {
  public:
	  iterator( Element *aElement = NULL ) : b(aElement) {}
	iterator( const iterator &i ) { b = i.b; }
    iterator	operator++() // Prefix
	{ b = b->next; return *this; }
    iterator	operator++(int) // Postfix
	{ iterator save(*this); operator++(); return save; }
    iterator	operator--() // Prefix
	{ b = b->prev; return *this; }
    iterator	operator--(int) // Postfix
	{ iterator save(*this); operator--(); return save; }
    iterator	&operator=( const iterator &i ) { b = i.b; return *this; }
    bool		operator==( const iterator &i ) const { return b == i.b; }
    bool		operator!=( const iterator &i ) const { return b != i.b; }

    const cKey	&operator*() const { return b->key; }
    cKey	&operator*() { return b->key; }

    bool		operator!() const { return b == NULL; }

	const cValue	&data() const { return b->value; }
	cValue	&data() { return b->value; }

	operator bool() const { return b != NULL; }
  private:
    Element	*b;
  };

  iterator begin() const { return iterator(getHead()); }
  iterator end() const { return iterator(NULL); }

  HashObjFI( size_t aSize = 0 );
  HashObjFI( const HashObjFI &c );
  ~HashObjFI();
  
  HashObjFI	&operator=( const HashObjFI &pc );
  HashObjFI	&operator|=( const HashObjFI &pc ); // Merge
  HashObjFI &destructiveMerge( HashObjFI &pc ); // Merge clearing the incoming hash table.

  size_t	size() const { processPending(); return numElements; }
  bool		empty() const { return size() == 0; }

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
				b->removeFromList( head );
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
      return iterator(numElements > 0 ? hashFindQuick(i, key) : NULL);
    }
  bool	contains( const cKey &p ) const { return find(p) != end(); }

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
		  {
			e->addToBktList( ht[i] );
			e->addToList( head );
		  }
	  }
	  return iterator(e);
    }

  void 		clear();
  void		rightSize() { processPending(); if( shouldShrink() ) resize(shrinkSize()); }

  size_t	getHTMax() const { return htMax; }

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

  Element	*getHead() const { ((HashObjFI *)this)->rightSize(); return head; }

  Element	**ht;		// HashFI table.
  Element	*head;		// Head of element list.
  Element	*tailPending; // Tail of a singly-linked list to be inserted.
  size_t	htMax;		// maximum size of the HashFI table.
  size_t	numElements;// number of elements in collection.

  Element	*findHead( Element *e, const cKey &p ) const;

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

  size_t	hash( const cKey &key ) const { return key.hash() % htMax; }
  void		resize( size_t htMaxNew );
  bool		shouldGrow() const { return htMax - (htMax>>2) <= numElements; }
  bool		shouldShrink() const { return numElements <= (htMax>>1); }
  size_t	growSize() const { return numElements + (numElements>>1) + 1; }
  size_t	shrinkSize() const { return numElements + (numElements>>2) + 1; }
  void	    addPending();
  void		removeNonFirst( Element *bPrev, const cKey &key );
  void		processPending() const { if( tailPending ) ((HashObjFI *)this)->addPending(); }

#ifdef MMenableMemoryManager
  static	MMallocator	*allocator;
#endif // MMenableMemoryManager
};

#ifdef MMenableMemoryManager
template < class cKey, class cValue >
MMallocator *HashObjFI<cKey, cValue>::allocator = NULL;
#endif // MMenableMemoryManager

template <class cKey, class cValue>
void HashObjFI<cKey, cValue>::resize( size_t htMaxNew )
{
	if( htMaxNew == 0 )
	{
#ifdef MMenableMemoryManager
		if( htMax <= mmHTMax )
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
			MMallocatorFactory::getAllocator( htMax * sizeof(ht[0]) )->freeBuf( ht );
	}
	else
#endif // MMenableMemoryManager
		::operator delete( ht );

	htMax = htMaxNew;
#ifdef MMenableMemoryManager
	if( htMax <= mmHTMax )
		ht = (Element **)MMallocatorFactory::getAllocator( htMax * sizeof(ht[0]) )->allocBuf();
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

template <class cKey, class cValue>
HashObjFI<cKey, cValue>::HashObjFI( size_t aSize )
{
#ifdef MMenableMemoryManager
	if( allocator == NULL )
		allocator = MMallocatorFactory::getAllocator( sizeof(Element) );
#endif // MMenableMemoryManager
	ht = NULL;
	head = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;
	if( aSize > 0 )
		resize( aSize );
}


template <class cKey, class cValue>
HashObjFI<cKey, cValue>::HashObjFI( const HashObjFI &c )
{
	ht = NULL;
	head = NULL;
	tailPending = NULL;
	htMax = 0;
	numElements = 0;

	if( !c.empty() )
		operator=( c );
}

template <class cKey, class cValue>
void HashObjFI<cKey, cValue>::removeNonFirst( register Element *bPrev, const cKey &p )
{
	register Element *b;
	for( b = bPrev->bNext; b != NULL; bPrev = b, b = b->bNext )
		if( b->key == p )
		{
			bPrev->bNext = b->bNext;
			b->removeFromList( head );
			deleteElement( b );
			--numElements;
			return;
		}
}

template <class cKey, class cValue>
HashObjFI<cKey, cValue> &HashObjFI<cKey, cValue>::operator=( const HashObjFI &c )
{
	clear();
	if( c.empty() )
		return *this;
    resize( c.numElements + (c.numElements >> 2) + 1 );

	Element	*tail = NULL;
    register Element *b, *bNew;
	for( b = c.head; b != NULL; b = b->next )
	{
		bNew = newElement( b->key, b->data );
		bNew->addToBktList( ht[hash(bNew->key)] );
		bNew->addToListTail( head, tail );
	}
	numElements = c.numElements;
	return *this;
}


template <class cKey, class cValue>
HashObjFI<cKey, cValue> &HashObjFI<cKey, cValue>::destructiveMerge( HashObjFI &c )
{
	c.processPending();
	if( c.numElements == 0 )
		return *this;

	register Element *e, *eFound, *eNext;
	size_t i;

	for( e = c.head; e != NULL; e = eNext )
	{
		eNext = e->next;

		if( shouldGrow() )
		{
			// If we need to grow, just park the new element on a list.
			// We will merge it with the hash table when we do a search.
			e->addToSList( tailPending );
			numElements++;
		}
		else
		{
			if( (eFound = hashFindQuick(i, e->key)) != NULL )	// key already exists - just update value.
			{
				eFound->value = e->value;
				deleteElement( e );
			}
			else
			{
				e->addToBktList( ht[i] );
				e->addToList( head );
				numElements++;
			}
		}
	}

	c.head = NULL;
	c.numElements = 0;
	c.resize( 0 );

	return *this;
}

template <class cKey, class cValue>
HashObjFI<cKey, cValue> &HashObjFI<cKey, cValue>::operator|=( const HashObjFI &c )
{
	c.processPending();
    register Element *b;
	for( b = c.head; b != NULL; b = b->next )
	    insert( b->key, b->value );
	return *this;
}

template <class cKey, class cValue>
typename HashObjFI<cKey, cValue>::Element *HashObjFI<cKey, cValue>::findHead( register Element *b, const cKey &p ) const
{
  for( b = b->bNext; b != NULL; b = b->bNext )
    if( b->key == p )
      return b;
  return NULL;
}

template <class cKey, class cValue>
void HashObjFI<cKey, cValue>::clear()
{
	// Reset the HashFI table.
	register Element *b, *bNext;

	// Delete all the elements.
	for( b = head; b != NULL; b = bNext )
	{
		bNext = b->next;
		deleteElement( b );
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
			deleteElement( b );
		}
	}

	// Delete the rest of the hash table.
	head = NULL;
	tailPending = NULL;
	numElements = 0;
	resize( 0 );
}

template <class cKey, class cValue>
void HashObjFI<cKey, cValue>::addPending()
{
	// Actually resize the hash table to add the new elements.
	Element *headPending = tailPending->next;
	tailPending->next = NULL;
	tailPending = NULL;

	resize( numElements + (numElements>>1) );

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
			eFound->value = e->value;
			deleteElement( e );
			--numElements;	// correct the numElements.
		}
	}
}

template <class cKey, class cValue>
HashObjFI<cKey, cValue>::~HashObjFI()
{
  clear();
#ifdef MMenableMemoryManager
  if( htMax <= mmHTMax )
  {
	  if( ht != NULL )
		MMallocatorFactory::getAllocator( htMax * sizeof(ht[0]) )->freeBuf( ht );
  }
  else
#endif // MMenableMemoryManager
	  ::operator delete( ht );
}

#endif // HashObjFI_H