//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "mm.h"

#ifdef MMenableMemoryManager

#include <new.h>

static	inline size_t	fixSizeMax( size_t &aSize, size_t &aMax )
{
	// Make sure size can hold a pointer.
	if( aSize < sizeof(MMbuf) )
		aSize = sizeof(MMbuf);

	// Make sure aMax is reasonable.
	if( aMax < 4 )
		aMax = 4;

	// Find the next power of 2 >= aSize * aMax;
	// We do this to reduce fragmentation.  When pools are freed,
	// we know that two consecutive pools can always be merged
	// to form a larger one.
	register size_t	poolSize = aSize * aMax;
	if( (poolSize & (poolSize-1)) != 0 )
	{
		// Multiply by two and clear the non-power-of-2 bits.
		poolSize <<= 1;
		poolSize &= (poolSize - 1);

		// Inline the first few iterations for speed.
		do	// Not really a loop, just allows me to break.
		{
			register size_t t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			if( (t = (poolSize & (poolSize-1))) == 0 ) break; poolSize = t;
			while( (t = (poolSize & (poolSize-1))) != 0 )
				poolSize = t;
		} while(0);
	}

	// Adjust max to the number of objects that can fit in poolSize;
	aMax = poolSize / aSize;
	return poolSize;
}

//--------------------------------------------------------

// newPool and deletePool are used to allocate new memory pools.
// Memory pools are held in a global memory pool list.
static MMpool	*curAlloc, *freeList;

inline void *newPool()
{
	void	*b;

	if( freeList != NULL )
	{
		b = (void *)freeList;
		freeList = freeList->next;
	}
	else
	{
		if( curAlloc == NULL || curAlloc->isFull() )
		{
			size_t	size = sizeof(MMpool);
			size_t	iMax = 128;
			size_t  poolSize = fixSizeMax(size, iMax);
			curAlloc = new MMpool( size, iMax, poolSize );
		}
		b = curAlloc->allocBufFromPool();
	}
	return b;
}

inline void deletePool( void *b )
{
	// Put the pool back on the free list.
	((MMpool *)b)->next = freeList;
	freeList = (MMpool *)b;
}

static MMpool	*header;

MMallocator::MMallocator( size_t aSize, size_t aMax )
{
	size = aSize;
	iMax = aMax;

	poolSize = fixSizeMax( size, iMax );
	nonFullList = NULL;

	if( header == NULL )
		header = new (newPool()) MMpool;
	root = nullNode = new (newPool()) MMpool;
	nullNode->left = nullNode->right = nullNode;
}

MMallocator::~MMallocator()
{
	// Warning!  When an allocator is deleted, all pools are freed, but
	// none of the destructors will be called on its objects.
	// Also, it is a *really* bad idea to call this with shared allocators.
	// Normally, this is never called, but it may be useful when managing
	// memory in a collection.
	while( root != nullNode )
		deleteRoot(0);
	deletePool( (void *)nullNode );
}

void MMallocator::splay( register void *x, MMpool *&t )
{
    register MMpool *leftTreeMax, *rightTreeMin;

    header->left = header->right = nullNode;
    leftTreeMax = rightTreeMin = header;

    nullNode->pool = (char *)x;   // Guarantee a match.
	nullNode->poolEnd = nullNode->pool + 1;
	
    for( ;; )
	{
        if( t->gt(x) )
        {
            if( t->left->gt(x) )
                t = rotateWithLeftChild( t );
            if( t->left == nullNode )
                break;
            rightTreeMin->left = t;
            rightTreeMin = t;
            t = t->left;
        }
        else if( t->lt(x) )
        {
            if( t->right->lt(x) )
                t = rotateWithRightChild( t );
            if( t->right == nullNode )
                break;
            leftTreeMax->right = t;
            leftTreeMax = t;
            t = t->right;
        }
        else
            break;
	}

    leftTreeMax->right = t->left;
    rightTreeMin->left = t->right;
    t->left = header->right;
    t->right = header->left;
}

void MMallocator::insert( MMpool *newNode )
{
	// Assume the new pool is empty and put it on the nonFullList.
	newNode->putOnList( nonFullList );

	// Insert the pool into the splay tree.
	if( root == nullNode )
    {
        newNode->left = newNode->right = nullNode;
    }
    else
    {
		void	*b = (void *)newNode->pool;
        splay( b, root );
        if( root->gt(b) )
        {
            newNode->left = root->left;
            newNode->right = root;
            root->left = nullNode;
        }
        else if( root->lt(b) )
        {
            newNode->right = root->right;
            newNode->left = root;
            root->right = nullNode;
        }
        else
            return;
    }
    root = newNode;
}

void *MMallocator::allocBufFromNewPool()
{
	// Get a new pool from the pool list.
	MMpool *pool = new (newPool()) MMpool( size, iMax, poolSize );

	// Insert this pool into the splay tree.
	insert( pool );

	// Return a buffer allocated from this pool.
	return pool->allocBufFromPool();
}

void MMallocator::deleteRoot( const int leaveLastOne )
{
	// Leave the last pool in the allocator if specified.
	if( leaveLastOne && root->left == nullNode && root->right == nullNode )
		return;

	// Assume the root is empty and remove the root from the nonFullList.
	root->removeFromList( nonFullList );
			
	// Remove the root from the splay tree.
	register MMpool *newTree;
	if( root->left == nullNode )
		newTree = root->right;
	else
	{
		// Find the maximum in the left subtree
		// Splay it to the root and attach the right child
		newTree = root->left;
		splay( (void *)root->pool, newTree );
		newTree->right = root->right;
	}

	// Reclaim the pool memory.
	root->~MMpool();
	deletePool( root );

	root = newTree;
}

void MMallocator::freeBufFromNonRoot( void *b )
{
	// Find the pool that contains b in the splay tree.
    // If it is found, it will be at the root
    splay( b, root );

	// Free b from the splay tree root.
	if( root->contains(b) )
	{
		root->freeBufFromPool( b );
		if( root->wasFull() )
		{
			// Put the pool back on the nonFullList if it was full before.
			root->putOnList( nonFullList );
		}
		else if( root->isEmpty() )
		{
			// Delete the pool entirely.
			deleteRoot();
		}
	}
	else
	{
		// We do not have this buffer in our pools.
		// Assume it came from somewhere else.
		::operator delete( b );
	}
}

//------------------------------------------------------------------------------

MMallocator *MMallocatorFactory::getAllocator( size_t size, size_t iMax )
{
	static MMallocator	**allocators;
	static size_t		allocMax;

	register size_t	h;

	if( allocMax == 0 )
	{
		// Initialize the allocators hash table.
		allocMax = 511; // Create space for 511 allocators.
		allocators = new MMallocator * [allocMax];
		for( h = 0; h < allocMax; h++ )
			allocators[h] = NULL;
	}

	// Adjust the size since all our pools are powers of two.
	fixSizeMax( size, iMax );

	// Find the requested allocator in the hash table.
	h = ((size*7) * iMax) % allocMax;
	while( allocators[h] != NULL
		&& allocators[h]->size != size && allocators[h]->iMax != iMax )
		if( ++h > allocMax )
			h = 0;

	// If we find one, return it.
	if( allocators[h] != NULL )
		return allocators[h];

	// If not, allocate a new one.
	return allocators[h] = new MMallocator( size, iMax );
}

#endif // MMenableMemoryManager