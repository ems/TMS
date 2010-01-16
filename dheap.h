//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef dheap_H
#define dheap_H

#include <stdlib.h>

// This is implemented as a base 4 heap.
// It runs about twice as fast as a base 2 heap.

template < class Element >
inline void DHeapAssign( Element *h, const size_t i, Element &e )
{
	h[i] = e;
#ifdef DHeapSetIndex
	// SetIndex must be defined if you want to store the heap index with the element.
	// HACK HACK HACK HACK HACK HACK HACK HACK HACK
	SetIndex( e, i );
#endif
}

inline size_t DHeapParent( const size_t x ) { return (x-1) >> 2; }
inline size_t DHeapChild(  const size_t x ) { return (x << 2) + 1; }

template <class Element, class Cmp>
inline size_t	DHeapTopChild( Element *h, const size_t N, register size_t ci, const Cmp cmp )
{
	// Initialize the first child as the top.
	register Element *pTop = h + ci, *p = pTop;

	// Unroll the comparison loop to find the top child.
	if( ci + 3 < N )
	{
		// Optimize for the non-terminal case
		// (this happens most of the time).
		if( cmp(*++p, *pTop) ) pTop = p;
		if( cmp(*++p, *pTop) ) pTop = p;
		if( cmp(*++p, *pTop) ) pTop = p;
	}
	else
	{
		// Handle the case when we are at the end of the array.
		if( ci + 1 < N )
		{
			if( cmp(*++p, *pTop) ) pTop = p;
			if( ci + 2 < N && cmp(*++p, *pTop) ) pTop = p;
		}
	}

	return pTop - h;
}

template <class Element, class Cmp>
void	DHeapSiftDown( Element *h, const size_t N, register size_t x, const Cmp cmp )
{
	Element	i = h[x];
	register size_t	c, ci;
	while( (ci = DHeapChild(x)) < N && cmp(h[c = DHeapTopChild(h, N, ci, cmp)],i) )
	{
		DHeapAssign( h, x, h[c] );
		x = c;
	}
	DHeapAssign( h, x, i );
}

template <class Element, class Cmp>
void	DHeapSiftUp( Element *h, register size_t x, const Cmp cmp )
{
	Element	i = h[x];
	register size_t p = x;
	while( cmp(i, h[p = DHeapParent(p)]) )
	{
		DHeapAssign( h, x, h[p] );
		if( (x = p) == 0 )
			break;
	}
	DHeapAssign( h, x, i );
}

template <class Element, class Cmp>
inline void	DHeapPromote( Element *h, register size_t x, const Cmp cmp )
{
	if( x != 0 )
		DHeapSiftUp( h, x, cmp );
}

template <class Element, class Cmp>
inline void	DHeapDemote( Element *h, const size_t N, register size_t x, const Cmp cmp )
{
	if( x != N-1 )
		DHeapSiftDown( h, N, x, cmp );
}

template <class Element, class Cmp>
inline void	DHeapInsert( Element *h, size_t N, const Cmp cmp )
{
	// Assume a new element has been added to the end of the heap.
	// Make sure the last element is indexed and sift it up.
	--N;
	DHeapAssign( h, N, h[N] );
	if( N != 0 )
		DHeapSiftUp( h, N, cmp );
}

template <class Element, class Cmp>
inline void	DHeapRemove( Element *h, size_t &N, register size_t i, const Cmp cmp )
{
	if( i != --N )
	{
		register int siftDown = cmp(h[i], h[N]);
		DHeapAssign( h, i, h[N] );
		if( siftDown )
			DHeapSiftDown( h, N, i, cmp );
		else
			DHeapSiftUp( h, i, cmp );
	}
}

template <class Element, class Cmp>
inline void	DHeapRemoveTop( Element *h, size_t &N, const Cmp cmp )
{
	if( 0 != --N )
	{
		DHeapAssign( h, (unsigned int)0, h[N] );
		DHeapSiftDown( h, N, (unsigned int)0, cmp );
	}
}

template <class Element, class Cmp>
void	DHeapBuild( Element *h, const size_t N, const Cmp cmp )
{
	if( N == 0 )
		return;
	register size_t x = N;
#ifdef DHeapSetIndex
	// SetIndex must be defined if you want to store the heap index with the element.
	// HACK HACK HACK HACK HACK HACK HACK HACK HACK
	do SetIndex( h[--x], x ); while( x != 0 );
	x = N;
#endif
	do DHeapSiftDown( h, N, --x, cmp ); while( x != 0 );
}

#endif // dheap_H