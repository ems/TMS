//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef Boundaries_H
#define Boundaries_H

#include <stdlib.h>
#include <limits.h>
#include "insectc.h"
#include "List.h"
#include "Hash.h"
#include "HashObjFI.h"

typedef unsigned long ident_t;
typedef int	coord_t;

class ITtree;
class ITline;
class ITpoint;
class ITnode;
class ITinterval;

typedef SList< ITline * >				ITlineCollection;
typedef HashObjFI< ITpoint, ident_t >	ITpointIDCollection;

#define	tiMin	(INT_MIN / 64)
#define	tiMax	(INT_MAX / 64)

class ITinterval
{
public:
	ITinterval( coord_t aMin, coord_t aMax )
	{
		if( aMin > aMax )	{ m_min = aMax; m_max = aMin; }
		else				{ m_min = aMin; m_max = aMax; }
	}
	ITinterval()
	{ reset(); }
	void reset()
	{
		m_min = tiMax;
		m_max = tiMin;
	}
	ITinterval &operator|=( const ITinterval &t )
	{
		if( t.m_min < m_min ) m_min = t.m_min;
		if( t.m_max > m_max ) m_max = t.m_max;
		return *this;
	}
	ITinterval &operator|=( const coord_t t )
	{
		if( t < m_min ) m_min = t;
		if( t > m_max ) m_max = t;
		return *this;
	}
	ITinterval &operator=( const ITinterval &t )
	{
		m_min = t.m_min;
		m_max = t.m_max;
		return *this;
	}
	int operator==( const ITinterval &ti ) const
	{ return m_min == ti.m_min && m_max == ti.m_max; }
	int contains( const coord_t t ) const
	{ return m_min <= t && t <= m_max; }
	int intersects( const ITinterval &ti ) const
	{ return !(ti.m_max < m_min || m_max < ti.m_min); }
	coord_t overlap( const ITinterval &ti ) const
	{
		return intersects(ti)
			? (m_max < ti.m_max ? m_max : ti.m_max) - (m_min > ti.m_min ? m_min : ti.m_min)
			: 0;
	}
	int operator!() const	// returns True if the interval is empty.
	{ return m_min > m_max; }
	int operator()() const	// returns True if the interval is not empty.
	{ return m_min <= m_max; }

	coord_t	imin() const { return m_min; }
	coord_t	imax() const { return m_max; }

	coord_t	length() const { return m_max - m_min; }
private:
	coord_t	m_min, m_max;
};

//--------------------------------------------------------------------------------------

struct ITintervalID
{
	ident_t		id;
	ITinterval	domain;
};

//--------------------------------------------------------------------------------------

class ITnode
{
public:
	ident_t	id;
	ITnode	*next;

private:
	friend class ITtree;
	ITnode()
	{
		tmax = INT_MIN;
		left = right = NULL;
	}

	void	set( const ITintervalID &ind )
	{
		id = ind.id;
		domain = ind.domain;
	}

	MMdeclare();

	void	update()
	{
		tmax = domain.imax();
		if( right != NULL && tmax < right->tmax )
			tmax = right->tmax;
	}

	ITinterval	domain;
	coord_t		tmax;
	ITnode	*left, *right;

	void		addToList( ITnode *&head )
	{
		next = head;
		head = this;
	}
};


//--------------------------------------------------------------------------------------

class ITtree
{
public:
	ITtree() : root(NULL) {}
	~ITtree();

	void	build( ITintervalID *array, int n );

	ITnode *findAllIntersecting( const coord_t t );
	ITnode *findAllIntersecting( const ITinterval &ti );

private:
	struct intervalIDIncreasingCmp
	{
		int operator()( const ITintervalID &i1, const ITintervalID &i2 ) const
		{ return i1.domain.imin() < i2.domain.imin(); }
	};
	void	updateAll();

	ITnode	*root;
};

//--------------------------------------------------------------------------------------

class ITpoint
{
	friend class ITline;
	friend class ITboundaries;
public:
	ITpoint( const coord_t aX = 0, const coord_t aY = 0 ) : x(aX), y(aY) {}
	ITpoint( const ITpoint &itp ) : x(itp.x), y(itp.y) {}
	ITpoint &operator=( const ITpoint &p ) { x = p.x; y = p.y; return *this; }

	int operator==( const ITpoint &p ) const { return x == p.x && y == p.y; }

	coord_t	getX() const { return x; }
	coord_t	getY() const { return x; }

	size_t	hash() const { return (x<<1) ^ y; }

	MMdeclare();

private:
	coord_t	x, y;
};

//--------------------------------------------------------------------------------------

class ITline
{
public:
	ITline( const ITpoint &aP1, const ITpoint &aP2 ) : p1(aP1), p2(aP2) {}
	ITline( const ITline &aL ) : p1(aL.p1), p2(aL.p2) {}
	ITline	&operator=( const ITline &aL )
	{
		p1 = aL.p1;
		p2 = aL.p2;
		return *this;
	}

	MMdeclare();

	const ITpoint	&getP1() const { return p1; }
	const ITpoint	&getP2() const { return p2; }
	int	intersects( const ITline &i ) const
	{
		coord_t	x, y;
		return lines_intersect(p1.x, p1.y, p2.x, p2.y, i.p1.x, i.p1.y, i.p2.x, i.p2.y, x, y)
					== DO_INTERSECT;
    }
	ITpoint	p1, p2;
};

//--------------------------------------------------------------------------------------

class ITboundaries
{
public:
	ITboundaries() : ownsLines(1) {}
	~ITboundaries() { clearLines(); }

	void	addLine( const ITline &line ) { lines.insert(new ITline(line)); }
	void	init();

	int		removeBlocked( const ITpoint &p, ITpointIDCollection &pc /* passed in and returned */ );

private:
	void	setLines( ITlineCollection &aLines ) { clearLines(); lines = aLines; ownsLines = 0; init(); }
	void	clearLines();

	ITlineCollection	lines;
	ITtree				yTree, xTree;

	int		getXYLines( const ITinterval &xi, const ITinterval &yi );
	ITlineCollection	xyLines;	// Lines intersecting a domain search.

	int	ownsLines;	// True if this object owns its lines.
};

#endif // Boundaries_H