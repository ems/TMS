//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef List_H
#define List_H

#ifndef mm_H
#	include "mm.h"
#endif

#include <stdlib.h>

//----------------------------------------------------------------------
class DListBase
{
public:
  class Element
  {
  public:
    unsigned long data;
    Element	*next, *prev;

    MMdeclare();

    void addToList( Element *&head )
      {
		if( head )
		  head->prev = this;

		next = head;
		prev = NULL;
		head = this;
      }

    void addToListTail( Element *&head, Element *&tail )
      {
		if( tail )
			tail->next = this;
		else
			head = this;

		prev = tail;
		next = NULL;
		tail = this;
      }

    void removeFromList( Element *&head )
      {
		if( prev )
		  prev->next = next;
		else
		  head = next;
		if( next )
		  next->prev = prev;
      }
  };

  DListBase() { head = NULL; numElements = 0; }
  DListBase( const DListBase &list ) { head = NULL; operator=(list); }
  ~DListBase() { clear(); }

  DListBase &operator=( const DListBase &list );

  Element *insert( const unsigned long data )
    {
      register Element *e = new Element;
      e->data = data;
      e->addToList( head );
	  numElements++;
      return e;
    }
  void remove( Element *e ) { e->removeFromList(head); delete e; numElements--; }
  void clear();
  size_t size() const { return numElements; }
  int empty() const { return head == NULL; }

  size_t	numElements;
  Element	*head;
};

template < class Data >
class DList
{
public:
  class iterator
  {
  public:
	iterator() {}
    iterator( DListBase::Element *element ) { e = element; }
    iterator( const iterator &i ) { e = i.e; }
    iterator &operator=( const iterator i ) { e = i.e; return *this; }

    iterator operator++() { e = e->next; return *this; }
    iterator operator++(int) { iterator s(*this); e = e->next; return s; }

    iterator operator--() { e = e->prev; return *this; }
    iterator operator--(int) { iterator s(*this); e = e->prev; return s; }

    Data  operator*() const { return (Data)e->data; }
    Data  data() const { return (Data)e->data; }

    int operator==( const iterator &i ) const { return e == i.e; }
    int operator!=( const iterator &i ) const { return e != i.e; }

    DListBase::Element *e;
  };

  typedef Data datatype;
  iterator begin() const { return iterator(list.head); }
  iterator end() const { return iterator(NULL); }

  iterator insert( const Data d ) { return iterator(list.insert((const unsigned long)d)); }
  iterator remove( iterator i ) { DListBase::Element *e = i.e; ++i; list.remove(e); return i; }
  iterator remove() { list.remove(list.head); return iterator(list.head); }

  size_t size() const { return list.size(); }
  int	empty() const { return list.empty(); }
  void clear() { list.clear(); }

  DList() {}
  DList( const DList &dList ) : list(dList.list) {}
  DList &operator=( const DList &dlist ) { list = dlist.list; return *this; }

private:
  DListBase	list;
};

//----------------------------------------------------------------------

class SListBase
{
public:
  class Element
  {
  public:
    unsigned long data;
    Element	*next;

    MMdeclare();

    void addToList( Element *&head )
      {
		next = head;
		head = this;
      }

    void addToListTail( Element *&head, Element *&tail )
      {
		if( tail )
			tail->next = this;
		else
			head = this;
		next = NULL;
      }
  };

  SListBase() { head = NULL; numElements = 0; }
  SListBase( const SListBase &list ) { head = NULL; operator=(list); }
  ~SListBase() { clear(); }

  SListBase &operator=( const SListBase &list );

  Element *insert( const unsigned long data )
    {
      register Element *e = new Element;
      e->data = data;
      e->addToList( head );
	  numElements++;
      return e;
    }

  void remove() { Element *e = head; head = head->next; delete e; numElements--; }
  void clear();
  size_t size() const { return numElements; }
  int empty() const { return head == NULL; }

  Element	*head;
  size_t	numElements;
};

template < class Data >
class SList
{
public:
  class iterator
  {
  public:
	iterator() {}
    iterator( SListBase::Element *element ) { e = element; }
    iterator( const iterator &i ) { e = i.e; }
    iterator &operator=( const iterator i ) { e = i.e; return *this; }

    iterator operator++() { e = e->next; return *this; }
    iterator operator++(int) { iterator s(*this); e = e->next; return s; }

    Data  operator*() const { return (Data)e->data; }
    Data  data() const { return (Data)e->data; }

    int operator==( const iterator &i ) const { return e == i.e; }
    int operator!=( const iterator &i ) const { return e != i.e; }

    SListBase::Element *e;
  };

  typedef Data datatype;
  iterator begin() const { return iterator(list.head); }
  iterator end() const { return iterator(NULL); }

  iterator insert( const Data d ) { return iterator(list.insert((const unsigned long)d)); }
  Data remove() { Data d = (Data)list.head->data; list.remove(); return d; }
  void clear() { list.clear(); }
  size_t	size() const { return list.size(); }
  int		empty() const { return list.empty(); }

  SList() {}
  SList( const SList &sList ) : list(sList.list) {}
  SList &operator=( const SList &slist ) { list = slist.list; return *this; }

private:
  SListBase	list;
};

#endif // List_H
