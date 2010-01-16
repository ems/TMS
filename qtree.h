//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef qtree_H
#define qtree_H

#ifndef mm_H
#	include "mm.h"
#endif

//#define	DIAGNOSTICS
#ifdef DIAGNOSTICS
#	include <iostream.h>
#endif

typedef int coord_t;
typedef unsigned long ident_t;

class QTree
{
public:
	class Point
	{
	public:
		Point( coord_t aX = 0, coord_t aY = 0) { x = aX; y = aY; }
		Point &operator=( const Point &p ) { x = p.x; y = p.y; return *this; }
		bool operator==( const Point &p ) const { return x == p.x && y == p.y; }
		bool operator!=( const Point &p ) const { return x != p.x || y != p.y; }
		coord_t	distance( const Point &p ) const
		{
			// Manhatten (rectilinear) distance.
			coord_t	xDiff = p.x - x;
			if( xDiff < 0 ) xDiff = -xDiff;
			coord_t yDiff = p.y - y;
			if( yDiff < 0 ) yDiff = -yDiff;
			return xDiff + yDiff;
		}
		size_t	hash() const { return (size_t)(x * y); }

		coord_t	x, y;
	};
	struct PointID
	{
		Point	point;
		ident_t	id;

		PointID() : point(0,0), id(0) {}
		PointID( const Point &aPoint, const ident_t aID ) : point(aPoint), id(aID) {}
		PointID( const PointID &p ) : point(p.point), id(p.id) {}
		void	set( const Point &aPoint, const ident_t aID ) { point = aPoint; id = aID; }

		bool operator==( const PointID &p ) const { return point == p.point && id == p.id; }
	};

	QTree();
	QTree( coord_t xMin, coord_t yMin, coord_t xMax, coord_t yMax );
	~QTree();

	void	insert( const Point &point, const ident_t id );
	bool	findNearest( const Point &search, Point &find, ident_t &id );
	bool	contains( const Point &search )
	{
		Point	find;
		ident_t	id;
		return findNearest(search, find, id) && find == search;
	}
	size_t	findNearestN( const Point &search, PointID *pointIDs, const size_t maxPointIDs ); // returns actual points found.
	size_t	findInRadius( const Point &search, const coord_t radius, PointID *pointIDs, const size_t maxPointIDs );
	void	clear();
	void	initRange( coord_t xMin, coord_t yMin, coord_t xMax, coord_t yMax );
#ifdef DIAGNOSTICS
	void	print( ostream &os );
#endif

private:
	class Square
	{
	public:
		Square( const Square &s ) : point(s.point), size(s.size) {}
		Square( const Point &aPoint, const coord_t aSize ) : point(aPoint), size(aSize) {}
		void	center( const Point &aPoint, const coord_t aSize )
		{
			point = aPoint;
			point.x -= aSize/2;
			point.y -= aSize/2;
			size = aSize;
		}
		Square &operator=( const Square &s )
		{ point = s.point; size = s.size; return *this; }
		coord_t	xMin() const { return point.x; }
		coord_t xMax() const { return point.x + size; }
		coord_t yMin() const { return point.y; }
		coord_t yMax() const { return point.y + size; }
		bool between( const coord_t min, const coord_t x, const coord_t max ) const
		{ return min <= x && x < max; }
		bool contains( const Point &p ) const
		{ return between(xMin(), p.x, xMax()) && between(yMin(), p.y, yMax()); }
		bool intersects( const Square &s ) const
		{
			// Check for trivial rejection.
			return !(s.point.x + s.size <= point.x ||
					point.x + size < s.point.x ||
					s.point.y + s.size <= point.y ||
					point.y + size < s.point.y );
		}
		Point	point;
		coord_t	size;
	};

	class Node
	{
	public:
		enum NodeType { ParentType = 0, LeafType };

		Node( Node *aParent, const Square &aSquare, NodeType aType ) :
			parent(aParent), square(aSquare), type(aType) {}

		const NodeType	type;

		Node		*parent;
		Square		square;
	};

	class Leaf : public Node
	{
	public:
		Leaf( Node *parent, const Square &square, const Point &aPoint, const ident_t aID ) :
			Node(parent, square, Node::LeafType), point(aPoint), id(aID) {}
		MMdeclare();
		Point	point;
		ident_t	id;
	};

	class Parent : public Node
	{
	public:
		enum Quadrant { Invalid = -1, NW, NE, SW, SE };

		Parent( Parent *parent, const Square &square ) :
			Node(parent, square, Node::ParentType)
		{ reset(); }
		MMdeclare();

		void	reset()
		{
			children[0] = NULL;
			children[1] = NULL;
			children[2] = NULL;
			children[3] = NULL;
			ichild = 0;
			searched = 0;
		}

		Quadrant	quadrantContaining( const Point &p ) const
		{
			Square	sCur( square );
			coord_t	hSize = (square.size >> 1);
			sCur.size = hSize;

			Quadrant	q;
			if( sCur.contains(p) ) q = NW;
			else if( sCur.point.x += hSize, sCur.contains(p) ) q = NE;
			else if( sCur.point.x = square.point.x, sCur.point.y += hSize, sCur.contains(p) ) q = SW;
			else q = SE;
			return q;
		}
		Square	childSquare( const Parent::Quadrant q ) const
		{
			Square	sCur( square );
			coord_t	hSize = (square.size >> 1);
			sCur.size = hSize;

			// Default is for NW.
			if(      q == NE ) sCur.point.x += hSize;
			else if( q == SW ) sCur.point.y += hSize;
			else if( q == SE ) { sCur.point.x += hSize; sCur.point.y += hSize; }
			return sCur;
		}

		void	setChild( const Parent::Quadrant q, Node *n ) { children[q] = n; }
		Node	*getChild( const int q ) const { return children[q]; }

		Node	*children[4];

		unsigned	ichild : 8;
		unsigned	searched : 1;
	};

	class QHeapElement
	{
	public:
		coord_t	d;
		Leaf	*leaf;

		QHeapElement( const QHeapElement &e ) : d(e.d), leaf(e.leaf) {}
		QHeapElement() {}
		QHeapElement &operator=( const QHeapElement &e ) { d = e.d; leaf = e.leaf; return *this; }
	};
	struct QEdecreasingD
	{
		int operator()( const QHeapElement &e1, const QHeapElement &e2 ) const
		{ return e1.d > e2.d; }
	};
	void	resetFlags();

	Parent	*root;
};

#endif