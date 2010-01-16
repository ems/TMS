//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef mm_H
#define mm_H

#include <stdlib.h>

// If mmEnableMemoryManager is defined, the fast memory manager will be
// used.  Otherwise, normal ::new and ::delete will be used.
#define MMenableMemoryManager
//#undef MMenableMemoryManager

#ifdef MMenableMemoryManager
// If MMsharedAllocators is defined, objects of the same size will share
// the same allocator.
#define MMsharedAllocators

// This is the default number of objects what will be allocated in a pool.
//#define MMdefaultAllocationMax	128
//#define MMdefaultAllocationMax	256
#define MMdefaultAllocationMax	1024

class MMallocator;
class MMallocatorFactory;
class MMpool;

// MMbuf is a buffer managed inside a pool.
class MMbuf
{
private:
	friend class MMpool;
	friend class MMallocator;

	MMbuf	*next;
};

// MMpool is a set of buffers managed together.
class MMpool
{
	friend class MMallocator;
	friend void *newPool();
	friend void deletePool( void *b );

public:
	MMpool( size_t aSize, size_t aMax, size_t poolSize )
	{
		iCur = 0;
		count = 0;

		next = prev = left = right = NULL;
		freeList = NULL;

		size = aSize;
		iMax = aMax;

		pool = (char *)::operator new( poolSize );
		poolEnd = pool + poolSize;
	}

	// null constructor
	MMpool()
	{
		// Null constructor.
		// Used for header and nullNode in splay tree.
		iCur = 0;
		count = 0;
		next = prev = left = right = NULL;

		freeList = NULL;

		size = iMax = 0;
		pool = poolEnd = NULL;
	}

	~MMpool() { if( size != 0 ) { ::operator delete ((void *)pool); size = 0; } }

	void	*allocBufFromPool()
	{
		void	*b;
		if( freeList != NULL )
		{
			b = (void *)freeList;
			freeList = freeList->next;
			++count;
		}
		else if( iCur < iMax )
		{
			b = (void *)(pool + (size * iCur++));
			++count;
		}
		else
			b = NULL;
		return b;
	}

	void	freeBufFromPool( void *b )
	{
		((MMbuf *)b)->next = freeList;
		freeList = (MMbuf *)b;
		--count;
	}

private:
	int		isFull() const { return count == iMax; }
	int		isEmpty() const { return count == 0; }
	int		wasFull() const { return count == iMax-1; }

	size_t	size, count, iCur, iMax;
	MMbuf	*freeList;
	char	*pool, *poolEnd;

	int		contains( const void *b ) const
	{ return pool <= (const char *)b && (const char *)b < poolEnd; }

	MMpool	*next, *prev;
	void	putOnList( MMpool *&list )
	{
		if( list != NULL )
			list->prev = this;
		next = list;
		prev = NULL;
		list = this;
	}
	void	removeFromList( MMpool *&list )
	{
		if( prev != NULL )
			prev->next = next;
		else
			list = next;
		if( next != NULL )
			next->prev = prev;
	}

	// Splay tree stuff.
	MMpool	*left, *right;

	int		lt( const void *b ) const
	{ return poolEnd <= (const char *)b; }
	int		eq( const void *b ) const
	{ return pool <= (const char *)b && (const char *)b < poolEnd; }
	int		gt( const void *b ) const
	{ return pool > (const char *)b; }
};

// MMallocator manages a set of pools.
class MMallocator
{
	friend class MMallocatorFactory;
public:
	MMallocator( size_t aSize, size_t aMax = MMdefaultAllocationMax );
	~MMallocator();

	void	*allocBuf()
	{
		void	*b;
		if( nonFullList != NULL )
		{
			b = nonFullList->allocBufFromPool();

			// Remove the pool from the nonFullList if it is full.
			if( nonFullList->isFull() )
				nonFullList->removeFromList( nonFullList );
		}
		else
			b = allocBufFromNewPool();

		return b;
	}

	void	freeBuf( void *b )
	{
		if( root->contains(b) )
		{
			root->freeBufFromPool( b );
			if( root->wasFull() )
			{
				// Put the pool back on the nonFullList if it was full before.
				root->putOnList( nonFullList );
			}
			else if( root->isEmpty() )
				deleteRoot();
		}
		else
			freeBufFromNonRoot( b );
	}

private:
	void	*allocBufFromNewPool();
	void	freeBufFromNonRoot( void *b );

	size_t	size, iMax, poolSize;

	// The nonFullList keeps track of pools that are not full.  This makes
	// it easy to find a pool to allocate from.
	MMpool	*nonFullList;

	// All the pools are kept in a splay tree.  This preserves more locality
	// in the search and improves performance with clustered deletes.
	MMpool	*root;		// root of the splay tree.
	MMpool	*nullNode;  // a node that can safely terminate a tree pointer.

	// Splay tree manipulations
	void deleteRoot( const int leaveLastOne = 1 );
	void insert( MMpool *p );
	void splay( void *x, MMpool * &t );
	MMpool *rotateWithLeftChild( register MMpool *k2 )
	{
		register MMpool *k1 = k2->left;
		k2->left = k1->right;
		k1->right = k2;
		return k1;
	}
	MMpool *rotateWithRightChild( register MMpool *k1 )
	{
		register MMpool *k2 = k1->right;
		k1->right = k2->left;
		k2->left = k1;
		return k2;
	}
};

class MMallocatorFactory
{
public:
	static MMallocator *getAllocator( size_t size, size_t iMax = MMdefaultAllocationMax );
};

#else
class MMallocator { int dummy; };
class MMallocatorFactory { int dummy; };
#endif // MMenableMemoryManager

#ifdef MMenableMemoryManager
#	define MMdeclare() \
		void *operator new( size_t ) { return mma->allocBuf(); } \
		void operator delete( void *b ) { mma->freeBuf(b); } \
		static MMallocator *mma

#	ifdef MMsharedAllocators
#		define MMinit( T ) \
			MMallocator *T::mma = MMallocatorFactory::getAllocator( sizeof(T) )
#		define MMinitMax( T, max ) \
			MMallocator *T::mma = MMallocatorFactory::getAllocator( sizeof(T), max )
#	else
#		define MMinit( T ) \
			MMallocator *T::mma = new MMallocator( sizeof(T) )
#		define MMinitMax( T, max ) \
			MMallocator *T::mma = new MMallocator( sizeof(T), max )
#	endif
#else // !MMenableMemoryManager
#	define MMdeclare()			static int MMenableMemoryManagerDummy
#	define MMinit( T )			int T::MMenableMemoryManagerDummy = 0
#	define MMinitMax( T, max )	int T::MMenableMemoryManagerDummy = 0
#endif

#endif // mm_H