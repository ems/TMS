//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef Hash_H
#define Hash_H

#ifndef mm_H
#include "mm.h"
#endif
#ifndef nextPrime_H
#include "nextPrime.h"
#endif

class HashBase
{
public:
  enum { mmHTMax = 128 };

  struct	Element
  {
    unsigned long key, data;

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

	MMdeclare();
  };

  HashBase( size_t aSize = 0 );
  HashBase( const HashBase &c );
  ~HashBase() { clear(); }
  size_t	size() const { processPending(); return numElements; }
  bool		empty() const { return size() == 0; }
  size_t	hash( const unsigned long p ) const { return (size_t)(((unsigned long)p) % htMax); }
  size_t	getHTMax() const { return htMax; }

  void		remove( const unsigned long p )
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
				delete b;
				--numElements;
			}
			else
				removeNonFirst( b, p );
		}
	}
  }
  Element	*findHead( Element *e, const unsigned long p ) const;
  Element	*find( const unsigned long key ) const
    {
	  processPending();
	  size_t i;
      return numElements > 0 ? hashFindQuick(i, key) : NULL;
    }
  Element	*findNextEntry( const unsigned long key ) const
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
	  // ProcessPending
	  ((HashBase *)this)->rightSize();
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
  Element	*insert( const unsigned long key, const unsigned long data  )
    {
	  register Element *e;
	  size_t i;
	  if( (e = hashFindQuick(i, key)) != NULL )	// key already exists - just update data.
		  e->data = data;
	  else
	  {
		  // Create a new hash entry.
		  // If we need to grow, just park the new element on a list.
		  // We will merge it with the hash table when we do a search.
		  e = new Element;
		  e->key = key;
		  e->data = data;
		  ++numElements;
		  if( shouldGrow() )
			e->addToSList( tailPending );
		  else
			e->addToBktList( ht[i] );
	  }
	  return e;
    }

  Element	**ht;		// hash table.
  Element	*tailPending; // list of items added but not put into the ht yet.
  size_t	htMax;		// maximum size of the hash table.
  size_t	numElements;// number of elements in collection.

  Element	*hashFindQuick( size_t &i, const unsigned long key ) const
  {
      register Element   *e;
	  if( htMax == 0 )
		  e = NULL;
	  else if( (e = ht[i = hash(key)]) != NULL && e->key != key )
	  {
		 if( (e = e->bNext) != NULL && e->key != key )
			e = findHead(e, key);
	  }
	  return e;
  }

  void		rightSize() { processPending(); if( shouldShrink() ) resize(shrinkSize()); }

  bool		shouldGrow() const { return htMax - (htMax>>2) <= numElements; }
  bool		shouldShrink() const { return numElements <= (htMax>>1); }
  size_t	growSize() const { return numElements + (numElements>>1) + 1; }
  size_t	shrinkSize() const { return numElements + (numElements>>2) + 1; }
  void		resize( size_t htMaxNew );
  void 		clear();
  HashBase	&operator=( const HashBase &pc );
  HashBase	&operator|=( const HashBase &pc ); // Merge

  void		removeNonFirst( Element *b, unsigned long key );
  void	    addPending();
  void		processPending() const { if( tailPending ) const_cast<HashBase *>(this)->addPending(); }
};

template < class cKey, class cData >
class	Hash
{
public:
	typedef cKey keytype;
	typedef cData datatype;

  class iterator
  {
  public:
	  iterator( const HashBase *aPC = NULL, HashBase::Element *aElement = NULL ) : pc(aPC), b(aElement) {}
	  iterator( const iterator &i ) : pc(i.pc), b(i.b) {}
    iterator	operator++() // Prefix
	{ b = (b->bNext) ? b->bNext : pc->findNextEntry( b->key ); return *this; }
    iterator	operator++(int) // Postfix
	{ iterator save(*this); operator++(); return save; }
    iterator	&operator=( const iterator &i ) { pc = i.pc; b = i.b; return *this; }
    bool		operator==( const iterator &i ) const { return b == i.b && pc == i.pc; }
    bool		operator!=( const iterator &i ) const { return b != i.b || pc != i.pc; }
    cKey	operator*() const { return (cKey)b->key; }
    cKey	operator()() const { return (cKey)b->key; }
    bool		operator!() const { return b == NULL; }
	cData	data() const { return (cData)b->data; }
	operator bool() const { return b != NULL; }
  private:
	  const HashBase			*pc;
      HashBase::Element	*b;
  };

  Hash( size_t aSize = 0 ) : pc(aSize) {}
  Hash( const Hash &c ) : pc(c.pc) {}

  Hash &operator=( const Hash &c ) { pc = c.pc; return *this; }
  Hash &operator|=( const Hash &c ) { pc |= c.pc; return *this; }

  iterator begin() const { return iterator(&pc, pc.findFirstEntry()); }
  iterator end() const { return iterator(&pc, NULL); }

  iterator	insert( const cKey key, const cData data )
  { return iterator(&pc,pc.insert( (const unsigned long)key, (const unsigned long)data)); }
  iterator  insertKey( const cKey key )
  { return iterator(&pc,pc.insert( (const unsigned long)key, (const unsigned long)key)); }
  iterator  insert( const iterator i )
  { return iterator(&pc,pc.insert( (const unsigned long)*i, (const unsigned long)i.data())); }
  void	remove( const cKey key )
  { pc.remove( (const unsigned long)key ); }
  void	remove( iterator &i )
  { pc.remove( (const unsigned long)*i ); }
  iterator	find( const cKey key ) const
  { return iterator(&pc, pc.find( (const unsigned long)key )); }
  cData operator[]( const cKey key ) const
  { return (cData)(pc.find((const unsigned long)key)->data); }
  cData get( const cKey key, const cData aDefault ) const
  { HashBase::Element *e = pc.find((const unsigned long)key); return e ? (cData)(e->data) : aDefault; }
  bool	contains( const cKey key ) const
  { return pc.find( (const unsigned long)key ) != NULL; }
  size_t   size() const { return pc.size(); }
  bool		empty() const { return pc.empty(); }
  void		clear() { pc.clear(); }
  void		rightSize() { pc.rightSize(); }

private:
  HashBase	pc;
};

#endif // Hash_H