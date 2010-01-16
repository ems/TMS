//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef rbtreemap_H
#define rbtreemap_H
#include <stdlib.h>
#include <new.h>
#include <string.h>
#ifndef mm_H
#	include "mm.h"
#endif
#ifndef HashFI_H
#	include "HashFI.h"
#endif

// This class behaves similarly to the stl map template, but with some simplifications.
// Additionally, the following efficient convenience functions have been added for
// more query control:
//
// iterator findPredecessor( const Key &key )	// finds the element just preceding key
// iterator findEqPredecessor( const Key &key )	// finds the element matching or just preceding key
// iterator findSuccessor( const Key &key )		// finds the element just succeding key
// iterator findEqSuccessor( const Key &key )	// finds the element matching or just preceding key

template <class Key, class Value, class Compare>
class RBTreeMap
{
public:
	struct data_type
	{
		Key		first;
		Value	second;
		data_type( const data_type &dt ) : first(dt.first), second(dt.second) {}
		data_type( const Key &key, const Value &value ) : first(key), second(value) {}
		data_type &operator=( const data_type &dt ) { first = dt.first; second = dt.second; return *this; }
	};

private:
	int CompLT(const Key &a, const Key &b) const { return cmp(a, b); }
	int CompLT(const data_type &a, const data_type &b) const { return cmp(a.first, b.first); }

	enum NodeColor { Black, Red };
	struct Node
	{
		Node( const data_type &dt ) : Data(dt) {}
		Node *left;         /* left child */
		Node *right;        /* right child */
		Node *parent;       /* parent */
		NodeColor color;    /* node color (black, red) */
		data_type Data;     /* data stored in node */
	};

public:
	class iterator;
	friend iterator;
	class iterator
	{
	public:
		iterator() : tree(NULL), n(NULL) {}
		iterator( const iterator &i ) : tree(i.tree), n(i.n) {}
		iterator &operator++() { n = tree->successor(n); return *this; }
		iterator &operator--() { n = tree->predecessor(n); return *this; }
		iterator operator++(int) { iterator i(tree,n); operator++(); return i; }
		iterator operator--(int) { iterator i(tree,n); operator--(); return i; }
		int operator==( const iterator &i ) const { return n == i.n; }
		int operator!=( const iterator &i ) const { return n != i.n; }
		const data_type &operator*() const { return n->Data; }

		iterator( const RBTreeMap *aTree, Node *aN ) : tree(aTree), n(aN) {}

		const RBTreeMap	*tree;
		Node	*n;
	};

	struct insert_pair
	{
		iterator	first;
		int			second;
		insert_pair( iterator i, int added ) : first(i), second(added) {}
	};

	RBTreeMap() { init(); }
	RBTreeMap( const RBTreeMap &t ) { init(); operator=(t); }
	~RBTreeMap() { clear(); }

	iterator begin() const { return iterator(this, minNode); }
	iterator end() const { return iterator(this, NIL); }

	RBTreeMap &operator=( const RBTreeMap &t )
	{
		clear();
		if( !t.empty() )
		{
			HashFI< Node *, Node * > map;
			iterator i;
			for( i = t.begin(); i != t.end(); ++i )
				map.insert( i.n, newNode(i.n->Data) );
			HashFI< Node *, Node * >::iterator n;
			for( n = map.begin(); n != map.end(); ++n )
			{
				Node	*nOld = *n, *nNew = n.data();
				nNew->color = nOld->color;
				nNew->parent = (nOld->parent == NULL ? NULL : map.find(nOld->parent).data());
				nNew->left = (nOld->left == t.NIL ? NIL : map.find(nOld->left).data());
				nNew->right = (nOld->right == t.NIL ? NIL : map.find(nOld->right).data());
			}
			root = map.find(t.root).data();
			minNode = map.find(t.minNode).data();
		}
		return *this;
	}

	insert_pair insert(const data_type &Data)
	{
		register Node *current = root, *parent = 0, *X;

	   /***********************************************
		*  allocate node for Data and insert in tree  *
		***********************************************/

		/* find where node belongs */
		while (current != NIL)
		{
			parent = current;
			if( CompLT(Data, current->Data) )
				current = current->left;
			else if( CompLT(current->Data, Data) )
				current = current->right;
			else
				return insert_pair(iterator(this, current),0);
	    }

		/* setup new node */
		X = newNode( Data );
		X->parent = parent;
		X->left = NIL;
		X->right = NIL;
		X->color = Red;

		/* insert node in tree */
		if(parent)
		{
			if(CompLT(Data, parent->Data))
			{
				if( parent == minNode )
					minNode = X;
				parent->left = X;
			}
			else
				parent->right = X;
		}
		else
			root = minNode = X;

		insertFixup(X);
		return insert_pair(iterator(this,X), 1);
	}
	insert_pair insert(const Key &key, const Value &value)
	{ return insert( data_type(key,value) ); }

	iterator find( const Key &key ) const
	{
	   /*******************************
		*  find node containing Data  *
		*******************************/
		register Node *current = root;
		while(current != NIL)
			if( CompLT(key, current->Data.first) )
				current = current->left;
			else if( CompLT(current->Data.first, key) )
				current = current->right;
			else
				break;

		return iterator(this, current);
	}

	iterator findPredecessor( const Key &key ) const
	{
		register Node *current = root, *parent = current;
		while(current != NIL)
		{
			parent = current;
			if( CompLT(key, current->Data.first) )
				current = current->left;
			else if( CompLT(current->Data.first, key) )
				current = current->right;
			else
				break;
		}

		return parent == NIL ? end() :
			CompLT(parent->Data.first, key) ? iterator(this, parent) :
			iterator(this, predecessor(parent));
	}

	iterator findEqPredecessor( const Key &key ) const
	{
		register Node *current = root, *parent = current;
		while(current != NIL)
		{
			parent = current;
			if( CompLT(key, current->Data.first) )
				current = current->left;
			else if( CompLT(current->Data.first, key) )
				current = current->right;
			else
				return iterator(this, current);
		}

		return parent == NIL ? end() :
			CompLT(parent->Data.first, key) ? iterator(this, parent) :
			iterator(this, predecessor(parent));
	}

	iterator findSuccessor( const Key &key ) const
	{
		register Node *current = root, *parent = current;
		while(current != NIL)
		{
			parent = current;
			if( CompLT(key, current->Data.first) )
				current = current->left;
			else if( CompLT(current->Data.first, key) )
				current = current->right;
			else
				break;
		}

		return parent == NIL ? end() :
			CompLT(key, parent->Data.first) ? iterator(this, parent) :
			iterator(this, successor(parent));
	}

	iterator findEqSuccessor( const Key &key ) const
	{
		register Node *current = root, *parent = current;
		while(current != NIL)
		{
			parent = current;
			if( CompLT(key, current->Data.first) )
				current = current->left;
			else if( CompLT(current->Data.first, key) )
				current = current->right;
			else
				return iterator(this, current);
		}

		return parent == NIL ? end() :
			CompLT(key, parent->Data.first) ? iterator(this, parent) :
			iterator(this, successor(parent));
	}

	void	erase( const Key &key ) { iterator i = find(key); if( i != end() ) DeleteNode( i.n ); }
	void	erase( const iterator i ) { DeleteNode( i.n ); }
	void	erase( const iterator start, const iterator finish )
	{
		register Node *n = start.n, *nNext;
		while( n != finish.n )
		{
			nNext = successor(n);
			DeleteNode( n );
			n = nNext;
		}
	}
	void	remove( const Key &key ) { erase(key); }
	void	remove( const iterator i ) { erase(i); }
	int		empty() const { return numElements == 0; }

	void	clear()
	{
		if( empty() )
			return;
		register Node	*n = root, *parent;
		for( ;; )
		{
			// Find a leaf.
			while( n->left != NIL || n->right != NIL )
			{
				while( n->left  != NIL )	n = n->left;
				while( n->right != NIL )	n = n->right;
			}

			// Delete the leaf.
			parent = n->parent;

			deleteNode( n );
			if( parent == NULL )
				break;
			(parent->left == n ? parent->left : parent->right) = NIL;

			// Follow the parent.
			n = parent;
		}
		root = minNode = NIL;
	}

	size_t	size() const { return numElements; }

private:
	void init()
	{
#ifdef MMenableMemoryManager
		if( allocator == NULL )
		{
			allocator = MMallocatorFactory::getAllocator( sizeof(Node) );
			NIL = (Node *)allocator->allocBuf();
		}
#else
		if( NIL == NULL )
			NIL = (Node *)operator new( sizeof(Node) );
#endif
		memset( NIL, -1, sizeof(Node) );
		NIL->left = NIL->right = NIL;
		NIL->parent = NULL;
		NIL->color = Black;
	
		root = minNode = NIL;
		numElements = 0;
	}

	void insertFixup(register Node *X)
	{

	   /*************************************
		*  maintain red-black tree balance  *
		*  after inserting node X           *
		*************************************/

		/* check red-black properties */
		while (X != root && X->parent->color == Red)
		{
			/* we have a violation */
			if (X->parent == X->parent->parent->left)
			{
				Node *Y = X->parent->parent->right;
				if (Y->color == Red)
				{
					/* uncle is red */
					X->parent->color = Black;
					Y->color = Black;
					X->parent->parent->color = Red;
					X = X->parent->parent;
				}
				else
				{
					/* uncle is black */
					if (X == X->parent->right)
					{
						/* make X a left child */
						X = X->parent;
						rotateLeft(X);
					}
					/* recolor and rotate */
					X->parent->color = Black;
					X->parent->parent->color = Red;
					rotateRight(X->parent->parent);
				}
			}
			else
			{
				/* mirror image of above code */
				Node *Y = X->parent->parent->left;
				if (Y->color == Red)
				{
					/* uncle is red */
					X->parent->color = Black;
					Y->color = Black;
					X->parent->parent->color = Red;
					X = X->parent->parent;
				}
				else
				{
					/* uncle is black */
					if (X == X->parent->left)
					{
						X = X->parent;
						rotateRight(X);
					}
					X->parent->color = Black;
					X->parent->parent->color = Red;
					rotateLeft(X->parent->parent);
				}
			}
		}
		root->color = Black;
	}

	void rotateLeft(register Node *X)
	{
	   /**************************
		*  rotate Node X to left *
		**************************/

		register Node *Y = X->right;

		/* establish X->right link */
		X->right = Y->left;
		if (Y->left != NIL) Y->left->parent = X;

		/* establish Y->parent link */
		if (Y != NIL) Y->parent = X->parent;
		if (X->parent) {
			if (X == X->parent->left)
				X->parent->left = Y;
			else
				X->parent->right = Y;
		} else {
			root = Y;
		}

		/* link X and Y */
		Y->left = X;
		if (X != NIL) X->parent = Y;
	}


	void rotateRight(register Node *X)
	{

	   /****************************
		*  rotate Node X to right  *
		****************************/

		register Node *Y = X->left;

		/* establish X->left link */
		X->left = Y->right;
		if (Y->right != NIL) Y->right->parent = X;

		/* establish Y->parent link */
		if (Y != NIL) Y->parent = X->parent;
		if (X->parent) {
			if (X == X->parent->right)
				X->parent->right = Y;
			else
				X->parent->left = Y;
		} else {
			root = Y;
		}

		/* link X and Y */
		Y->right = X;
		if (X != NIL) X->parent = Y;
	}

	void DeleteNode(register Node *Z)
	{
		register Node *X, *Y;

	   /*****************************
		*  delete node Z from tree  *
		*****************************/

		if (!Z || Z == NIL) return;

		if( Z == minNode )
			minNode = successor(minNode);

		if (Z->left == NIL || Z->right == NIL) {
			/* Y has a NIL node as a child */
			Y = Z;
		} else {
			/* find tree successor with a NIL node as a child */
			Y = Z->right;
			while (Y->left != NIL) Y = Y->left;
		}

		/* X is Y's only child */
		if (Y->left != NIL)
			X = Y->left;
		else
			X = Y->right;

		/* remove Y from the parent chain */
		X->parent = Y->parent;
		if (Y->parent)
			if (Y == Y->parent->left)
				Y->parent->left = X;
			else
				Y->parent->right = X;
		else
			root = X;

		if (Y != Z) Z->Data = Y->Data;
		if (Y->color == Black)
			DeleteFixup (X);
		deleteNode(Y);
	}

	void DeleteFixup(register Node *X)
	{
	   /*************************************
		*  maintain red-black tree balance  *
		*  after deleting node X            *
		*************************************/

		while (X != root && X->color == Black) {
			if (X == X->parent->left) {
				register Node *W = X->parent->right;
				if (W->color == Red) {
					W->color = Black;
					X->parent->color = Red;
					rotateLeft (X->parent);
					W = X->parent->right;
				}
				if (W->left->color == Black && W->right->color == Black) {
					W->color = Red;
					X = X->parent;
				} else {
					if (W->right->color == Black) {
						W->left->color = Black;
						W->color = Red;
						rotateRight (W);
						W = X->parent->right;
					}
					W->color = X->parent->color;
					X->parent->color = Black;
					W->right->color = Black;
					rotateLeft (X->parent);
					X = root;
				}
			} else {
				register Node *W = X->parent->left;
				if (W->color == Red) {
					W->color = Black;
					X->parent->color = Red;
					rotateRight (X->parent);
					W = X->parent->left;
				}
				if (W->right->color == Black && W->left->color == Black) {
					W->color = Red;
					X = X->parent;
				} else {
					if (W->left->color == Black) {
						W->right->color = Black;
						W->color = Red;
						rotateLeft (W);
						W = X->parent->left;
					}
					W->color = X->parent->color;
					X->parent->color = Black;
					W->left->color = Black;
					rotateRight (X->parent);
					X = root;
				}
			}
		}
		X->color = Black;
	}


	Node *minimum( register Node *sc ) const
	{
		while ( sc->left != NIL )
			sc = sc->left;
		return sc;
	}

	Node *maximum( register Node *sc ) const
	{
		while ( sc->right != NIL )
			sc = sc->right;
		return sc;
	}

	Node *predecessor( register Node *x ) const
	{
		register Node *y;
		if (x->left != NIL )
			y = maximum( x->left );
		else
		{
			y = x->parent;
			while (y && x == y->left)
			{
				x = y;
				y = y->parent;
			}
		}
		return y == NULL ? NIL : y;
	}

	Node *successor( register Node *x ) const
	{
		register Node *y;
		if ( x->right != NIL )
			y = minimum( x->right );
		else
		{
			y = x->parent;
			while (y && x == y->right)
			{
				x = y;
				y = y->parent;
			}
		}
		return y == NULL ? NIL : y;
	}

private:
	static Node *NIL;
	Node *root;               /* root of red-black tree */
	Node *minNode;
	size_t	numElements;
#ifdef MMenableMemoryManager
	static MMallocator *allocator;
#endif

	Node	*newNode( const data_type &dt )
	{
		numElements++;
#ifdef MMenableMemoryManager
		return new (allocator->allocBuf()) Node(dt);
#else
		return new Node(dt);
#endif
	}
	void	deleteNode( Node *n )
	{
		numElements--;
#ifdef MMenableMemoryManager
		n->~Node(); allocator->freeBuf( (void *)n );
#else
		delete n;
#endif
	}

	Compare	cmp;
};

#ifdef MMenableMemoryManager
template <class Key, class Value, class Compare>
MMallocator *RBTreeMap<Key, Value, Compare>::allocator = NULL;
#endif

template <class Key, class Value, class Compare>
RBTreeMap<Key, Value, Compare>::Node *RBTreeMap<Key, Value, Compare>::NIL = NULL;

#endif // rbtreemap_H