//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef PairingHeap_H
#define PairingHeap_H

#ifndef Hash_H
#	include "Hash.h"
#endif

struct PairNode
{
    void		*element;
    PairNode    *leftChild;
    PairNode    *nextSibling;
    PairNode    *prev;

    PairNode( void *theElement ) : element( theElement ),
      leftChild( NULL ), nextSibling( NULL ), prev( NULL ) {}

	MMdeclare();
};

void PairingHeapTreeArrayResize( PairNode **&treeArray, int &treeArraySize );

template <class T, class Cmp>
class PairingHeap
{
  public:
	typedef Hash<T *, PairNode *> HashTable;

	PairingHeap( size_t aSize = 3 ) : ht(aSize) { root = highestNode = NULL; }
	PairingHeap( Cmp aCmp, size_t aSize = 3 ) : cmp(aCmp), ht(aSize) { root = highestNode = NULL; }
    ~PairingHeap() { clear(); }

    T *top() const { return root ? (T *)root->element : NULL; }
	T *extractTop()
	{
		T			*key;
		PairNode	*oldRoot = removeTopNode();
		if( oldRoot != NULL )
		{
			key = (T *)oldRoot->element;
			delete oldRoot;
			ht.remove( key );
		}
		else
			key = NULL;
		return key;
	}

	size_t size() { return ht.size(); }
	bool	empty() { return ht.size() == 0; }

	void	insert( T *x )
	{
		if( !ht.contains(x) )
		{
			PairNode	*n = new PairNode( (void *)x );
			ht.insert( x, n );
			insert( n );
		}
	}

	PairingHeap &operator=( PairingHeap &rhs )
	{
		if( this != &rhs )
			clone( rhs );
		return *this;
	}

	PairingHeap &operator|=( PairingHeap &rhs );

	void	promoteKey( T *x )
	{
		HashTable::iterator i = ht.find(x);
		if( !(i == ht.end()) )
			promoteKey( i.data() );
		else
		{
			PairNode	*n = new PairNode( (void *)x );
			ht.insert( x, n );
			insert( n );
		}
	}
	void	demoteKey( T *x )
	{
		HashTable::iterator i = ht.find(x);
		if( !(i == ht.end()) )
			demoteKey( i.data() );
		else
		{
			PairNode	*n = new PairNode( (void *)x );
			ht.insert( x, n );
			insert( n );
		}
	}
	void	remove( T *x )
	{
		HashTable::iterator i = ht.find(x);
		if( !(i == ht.end()) )
		{
			remove( i.data() );
			delete i.data();
			ht.remove( *i );
		}
	}
	bool	contains( T *x ) { return ht.contains(x); }

	HashTable	&getHT() { return ht; }

    void clear();

  private:
    void	insert( PairNode *newNode )
	{
		if( root == NULL )
			root = newNode;
		else
			compareAndLink( root, newNode );
	}
    void promoteKey( PairNode *p )
	{
		if( p != root )
		{
			if( p->nextSibling != NULL )
				p->nextSibling->prev = p->prev;
			if( p->prev->leftChild == p )
				p->prev->leftChild = p->nextSibling;
			else
				p->prev->nextSibling = p->nextSibling;

			p->nextSibling = NULL;
			compareAndLink( root, p );
		}
	}
    void demoteKey( PairNode *p )
	{
		remove( p );
		insert( p );
	}
	void	remove( PairNode *p )
	{
		// Promote this node to the top of the heap.
		highestNode = p;
		promoteKey( p );
		highestNode = NULL;
		removeTopNode(); // Now, remove it.
	}
	PairNode	*removeTopNode()
	{
		PairNode *oldRoot = root;
		if( root != NULL )
		{
			if( root->leftChild == NULL )
				root = NULL;
			else
				root = combineSiblings( root->leftChild );
		}
		return oldRoot;
	}

    PairNode *root;

    void compareAndLink( register PairNode * & first, register PairNode *second )
	{
		if( second != NULL )
		{
			if( higherPriority(second, first) )
			{
				// Attach first as leftmost child of second
				second->prev = first->prev;
				first->prev = second;
				first->nextSibling = second->leftChild;
				if( first->nextSibling != NULL )
					first->nextSibling->prev = first;
				second->leftChild = first;
				first = second;
			}
			else
			{
				// Attach second as leftmost child of first
				second->prev = first;
				first->nextSibling = second->nextSibling;
				if( first->nextSibling != NULL )
					first->nextSibling->prev = first;
				second->nextSibling = first->leftChild;
				if( second->nextSibling != NULL )
					second->nextSibling->prev = second;
				first->leftChild = second;
			}
		}
	}
    PairNode *combineSiblings( PairNode *firstSibling );
	void	clone( PairingHeap &h );

	PairNode	*highestNode;
	bool	higherPriority( PairNode *n1, PairNode *n2 )
	{
		return highestNode == NULL ? cmp(*((T *)n1->element), *((T *)n2->element))
		: (n1==highestNode ? true : n2==highestNode ? false : cmp(*((T *)n1->element), *((T *)n2->element)));
	}

	Cmp			cmp;
	HashTable	ht;

	static	PairNode	**treeArray;
	static	int			treeArraySize;
};

template <class T, class Cmp>
PairNode	**PairingHeap<T, Cmp>::treeArray = NULL;

template <class T, class Cmp>
int	PairingHeap<T, Cmp>::treeArraySize = 0;

template <class T, class Cmp>
void PairingHeap<T, Cmp>::clear()
{
	if( root != NULL )
	{
		for( HashTable::iterator i = ht.begin(); !(i == ht.end()); ++i )
			delete i.data();
		ht.clear();
		root = NULL;
	}
}

template <class T, class Cmp>
void PairingHeap<T, Cmp>::clone( PairingHeap<T, Cmp> &h )
{
	clear();
	for( HashTable::iterator i = h.ht.begin(); !(i == h.ht.end()); ++i )
		insert( *i );
}

template <class T, class Cmp>
PairingHeap<T, Cmp> &PairingHeap<T, Cmp>::operator|=( PairingHeap<T, Cmp> &h )
{
	for( HashTable::iterator i = h.ht.begin(); !(i == h.ht.end()); ++i )
		insert( *i );
	return *this;
}

template <class T, class Cmp>
PairNode *PairingHeap<T, Cmp>::combineSiblings( register PairNode *firstSibling )
{
    if( firstSibling->nextSibling == NULL )
        return firstSibling;

    // Store the subtrees in an array
    register int numSiblings = 0;
    for( ; firstSibling != NULL; numSiblings++ )
    {
        if( numSiblings == treeArraySize )
            PairingHeapTreeArrayResize( treeArray, treeArraySize );
        treeArray[numSiblings] = firstSibling;
        firstSibling->prev->nextSibling = NULL;  // break links
        firstSibling = firstSibling->nextSibling;
    }
    if( numSiblings == treeArraySize )
        PairingHeapTreeArrayResize( treeArray, treeArraySize );
    treeArray[numSiblings] = NULL;

    // Combine subtrees two at a time, going left to right
    register int i = 0;
    for( ; i + 1 < numSiblings; i += 2 )
        compareAndLink( treeArray[i], treeArray[i+1] );

	i -= 2;

    // i has the result of last compareAndLink.
    // If an odd number of trees, get the last one.
    if( i >= 0 && i == numSiblings - 3 )
        compareAndLink( treeArray[i], treeArray[i+2] );

    // Now go right to left, merging last tree with
    // next to last. The result becomes the new last.
    for( ; i >= 2; i -= 2 )
        compareAndLink( treeArray[i-2], treeArray[i] );
    return treeArray[0];
}

#endif // PairingHeap_H
