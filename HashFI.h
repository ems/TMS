//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef HashFI_H
#define HashFI_H

#ifndef mm_H
#include "mm.h"
#endif
#ifndef nextPrime_H
#include "nextPrime.h"
#endif

class HashFIBase
{
public:
	enum { mmHTMax = 128 };
	
	struct	Element
	{
		unsigned long key, data;
		
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
		
		MMdeclare();
	};
	
	HashFIBase( size_t aSize = 0 );
	HashFIBase( const HashFIBase &c );
	~HashFIBase();
	
	HashFIBase	&operator=( const HashFIBase &pc );
	HashFIBase	&operator|=( const HashFIBase &pc ); // Merge
	
	HashFIBase	&operator-=( const HashFIBase &pc ); // Remove all in collection.

	bool		operator==( const HashFIBase &pc ) const; // Check element-wise equality.
	bool		operator!=( const HashFIBase &pc ) const { return !operator==(pc); }

	size_t	size() const { processPending(); return numElements; }
	bool	empty() const { return size() == 0; }
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
					b->removeFromList( head );
					delete b;
					--numElements;
				}
				else
					removeNonFirst( b, p );
			}
		}
	}
	Element	*find( const unsigned long key ) const
    {
		processPending();
		size_t i;
		return numElements > 0 ? hashFindQuick(i, key) : NULL;
    }
	
	Element	*insert( const unsigned long key, const unsigned long data  )
    {
		register Element *e;
		size_t i;
		if( (e = hashFindQuick(i, key)) != NULL )	// key already exists - just update data.
			e->data = data;
		else
		{
			// Add a new element to the hash table.
			// If we need to grow, just park the new element on a list.
			// We will merge it with the hash table when we do a search.
			e = new Element;
			e->key = key;
			e->data = data;
			++numElements;
			if( shouldGrow() )
				e->addToSList( tailPending );
			else
			{
				e->addToBktList( ht[i] );
				e->addToList( head );
			}
		}
		return e;
    }
	
	void 	clear();
	void	rightSize() const { processPending(); if( shouldShrink() ) ((HashFIBase *)this)->resize(shrinkSize()); }
	
	Element	*getHead() const { rightSize(); return head; }
	void	setHead( Element *headNew ) { head = headNew; }
	void	resize( size_t htMaxNew );
	
protected:
	Element	**ht;		// HashFI table.
	Element	*head;		// Head of element list.
	Element	*tailPending; // Tail of a singly-linked list to be inserted.
	size_t	htMax;		// maximum size of the HashFI table.
	size_t	numElements;// number of elements in collection.
	
	Element	*findHead( Element *e, const unsigned long p ) const;
	
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
	
	size_t	hash( const unsigned long p ) const { return (size_t)(((unsigned long)p) % htMax); }
	bool	shouldGrow() const { return htMax - (htMax>>2) <= numElements; }
	bool	shouldShrink() const { return numElements <= (htMax>>2); }
	size_t	growSize() const { return (numElements<<1) + 1; }
	size_t	shrinkSize() const { return numElements + (numElements>>1) + 1; }
	void	removeNonFirst( Element *b, unsigned long key );
	void	addPending();
	void	processPending() const { if( tailPending ) const_cast<HashFIBase *>(this)->addPending(); }
};

template < class cKey, class cData >
class	HashFI
{
public:
	typedef cKey keytype;
	typedef cData datatype;

  class iterator
  {
  public:
    iterator( HashFIBase::Element *aElement = NULL ) { b = aElement; }
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
    bool	operator==( const iterator &i ) const { return b == i.b; }
    bool	operator!=( const iterator &i ) const { return b != i.b; }
    cKey	operator*() const { return (cKey)b->key; }
    cKey	operator()() const { return (cKey)b->key; }
    bool	operator!() const { return b == NULL; }
	const cData	data() const { return (cData)b->data; }
	cData	&data() { return (cData &)b->data; }
	operator bool() const { return b != NULL; }
  private:
    HashFIBase::Element	*b;
  };

  HashFI( size_t aSize = 0 ) : pc(aSize) {}
  HashFI( const HashFI &c ) : pc(c.pc) {}

  iterator begin() const { return iterator(pc.getHead()); }
  iterator end() const { return iterator(NULL); }

  HashFI &operator=( const HashFI &c ) { pc = c.pc; return *this; }
  HashFI &operator|=( const HashFI &c ) { pc |= c.pc; return *this; }
  HashFI &operator-=( const HashFI &c ) { pc -= c.pc; return *this; }
  bool operator==( const HashFI &c ) { return pc == c.pc; }
  bool operator!=( const HashFI &c ) { return pc != c.pc; }

  iterator	insert( const cKey key, const cData data )
  { return iterator(pc.insert( (const unsigned long)key, (const unsigned long)data)); }
  iterator  insertKey( const cKey key )
  { return iterator(pc.insert( (const unsigned long)key, (const unsigned long)key)); }
  iterator	insert( const iterator i )
  { return iterator(pc.insert( (const unsigned long)*i, (const unsigned long)i.data() )); }
  void	remove( const cKey key ) { pc.remove( (const unsigned long)key ); }
  iterator	find( const cKey key ) const
  { return iterator(pc.find( (const unsigned long)key)); }
  int contains( const cKey key ) const
  { return pc.find((const unsigned long)key) != NULL; }
  cData operator[]( const cKey key )
  { return pc.find((const unsigned long)key)->data; }
  cData get( const cKey key, const cData aDefault ) 
  { iterator i = find(key); return i != end() ? i.data() : aDefault; }

  size_t   size() const { return pc.size(); }
  bool		empty() const { return pc.empty(); }
  void		clear() { pc.clear(); }
  void		rightSize() { pc.rightSize(); }
  void		resize( const size_t size ) { pc.clear(); pc.resize(size*2); }

private:
  HashFIBase	pc;
};

#endif // HashFI_H