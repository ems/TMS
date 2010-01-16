//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef Trapezoid_H

//#define Trapezoid_Debug

#pragma warning( disable : 4786 )

#include <stdlib.h>
#include <limits.h>
#include <float.h>
#ifdef Trapezoid_Debug
#include <iostream>
using namespace std;
#endif
#include <assert.h>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

const double tolDefault = 0.000001;

inline bool eqZeroTol( const double x, const double tol = tolDefault )
{ return (x >= 0.0 ? x <= tol: -x <= tol); }

inline bool eqTol( const double a, const double b, const double tol = tolDefault )
{ return eqZeroTol( b - a, tol ); }

inline bool ltTol( const double a, const double b, const double tol = tolDefault )
{ return (b - a) > tol; }

inline bool leTol( const double a, const double b, const double tol = tolDefault )
{
	register double x = b - a;
	return x < 0.0 ? -x <= tol : true;
}

inline bool gtTol( const double a, const double b, const double tol = tolDefault )
{ return ltTol(b, a, tol); }

inline bool geTol( const double a, const double b, const double tol = tolDefault )
{ return leTol(b, a, tol); }

template <class T> inline bool between( const T min, const T x, const T max ) { return min < x && x < max; }
inline bool betweenTol( const double min, const double x, const double max, const double tol = tolDefault )
{ return ltTol(min,x,tol) && ltTol(x,max,tol); }

class Trapezoidal
{
public:
	typedef	double	numType;

	enum IntersectStatus { DONT_INTERSECT, DO_INTERSECT, COLLINEAR };
	static IntersectStatus lines_intersect( const numType x1, const numType y1, const numType x2, const numType y2,
											const numType x3, const numType y3, const numType x4, const numType y4,
											numType &x, numType &y );
public:
	class Point
	{
	public:
		Point( const Point &p ) : x(p.x), y(p.y) {}
		Point( numType aX = 0, numType aY = 0) : x(aX), y(aY) {}
		numType x, y;

		bool	operator<( const Point &p ) const { return x < p.x ? true : p.x < x ? false : y < p.y; }
		bool	operator==( const Point &p ) const { return x == p.x && y == p.y; }

		bool	eqTol( const Point &p, const double tol = tolDefault ) const
		{ return ::eqTol(x, p.x, tol) && ::eqTol(y, p.y, tol); }

#ifdef Trapezoid_Debug
		void	printOn( ostream &os ) const { os << '(' << x << ',' << y << ')'; }
#endif // Trapezoid_Debug
	};

	typedef std::vector<Point *> PPointVector;
	typedef std::map<Point, Point *> PointPMap;

	class Segment
	{
	public:
		Segment( const Point *aFrom, const Point *aTo ) : from(aFrom), to(aTo) {}
		const Point	*from, *to;

		numType	xMax() const { return from->x > to->x ? from->x : to->x; }
		numType	yMax() const { return from->y > to->y ? from->y : to->y; }
		numType	xMin() const { return from->x < to->x ? from->x : to->x; }
		numType	yMin() const { return from->y < to->y ? from->y : to->y; }

		numType	getY( const numType x ) const
		{
			// Assume Segment is not vertical.
			return from->y + (to->y - from->y) / (to->x - from->x) * (x - from->x);
		}

		bool	intersect( const Segment &seg, Point &p ) const
		{
			// Trivial regection.
			if( ltTol(seg.xMax(), xMin()) || ltTol(xMax(), seg.xMin()) ||
				ltTol(seg.yMax(), yMin()) || ltTol(yMax(), seg.yMin()) )
				return false;

			register double	s, t, num, denom;

			denom = from->x		* (seg.to->y - seg.from->y) +
					to->x		* (seg.from->y - seg.to->y) +
					seg.to->x	* (to->y - from->y) +
					seg.from->x	* (from->y - to->y);

			if( denom == 0.0 )	// parallel
				return false;

			num =	from->x		* (seg.to->y - seg.from->y) +
					seg.from->x	* (from->y - seg.to->y) +
					seg.to->x	* (seg.from->y - from->y);
			s = num / denom;

			num = -(from->x		* (seg.from->y - to->y) +
					to->x		* (from->y - seg.from->y) +
					seg.from->x	* (to->y - from->y));
			t = num / denom;

			// Check that the segments intersect within their from and to points.
			const double paraTol = 0.0000001;
			if( leTol(0,s,paraTol) && leTol(s,1,paraTol) && leTol(0,t,paraTol) && leTol(t,1,paraTol) )
			{
				if( s < 0.0 ) s = 0.0; else if( s > 1.0 ) s = 1.0;
				p.x = from->x + s * (to->x - from->x);
				p.y = from->y + s * (to->y - from->y);
				return true;
			}

			return false;
		}

		bool	operator<( const Segment &s ) const
		{ return *from < *s.from ? true : *s.from < *from ? false : *to < *s.to; }

		bool	operator==( const Segment &s ) const
		{ return *from == *s.from && *to == *s.to; }

		bool	eqTol( const Segment &s, const double tol = tolDefault ) const
		{ return from->eqTol(*s.from) && to->eqTol(*s.to); }

		bool	isVertical() const	{ return from->x == to->x; }

		void	makeCanonical() { if( !(*from < *to) ) std::swap(from, to); }

#ifdef Trapezoid_Debug
		void	printOn( ostream &os ) const	{ from->printOn(os); os << ' '; to->printOn(os); }
#endif // Trapezoid_Debug
	};

	typedef std::set<Segment>	SegmentSet;

	class Trapezoid
	{
	public:
		Trapezoid( Trapezoid *aParent, const Segment &aTop, const Segment &aBottom, const numType xMinIn, const numType xMaxIn )
			: parent(aParent), top(aTop), bottom(aBottom), 
			childCur(0)
		{
			// corner[2] +---------------+ corner[1]
			//           |               |
			//           |               |
			//           |               |
			// corner[3] +---------------+ corner[0]

			corner[0] = Point( xMaxIn, bottom.getY(xMaxIn) );
			corner[1] = Point( xMaxIn, top.getY(xMaxIn) );
			corner[2] = Point( xMinIn, top.getY(xMinIn) );
			corner[3] = Point( xMinIn, bottom.getY(xMinIn) );

			child[0] = NULL;
			child[1] = NULL;
			child[2] = NULL;
			child[3] = NULL;
		}

		Trapezoid		*parent;
		const Segment	&top, &bottom;
		Point			corner[4];
		Trapezoid		*child[4];
		bool			hasChildren() const { return child[0] != NULL; }
		int				childCur;

		bool contains( const Point &p ) const
		{
			// If a point is on a line, we consider it an intersection.
			return ltTol(xMin(), p.x) && ltTol(p.x, xMax()) && ltTol(bottom.getY(p.x), p.y) && ltTol(p.y, top.getY(p.x));
		}

		Segment getRight() const { return Segment(&corner[0], &corner[1]); }
		Segment getTop() const { return Segment(&corner[2], &corner[1]); }
		Segment getLeft() const { return Segment(&corner[3], &corner[2]); }
		Segment getBottom() const { return Segment(&corner[3], &corner[0]); }

		bool intersectLeft( const Segment &s, Point &p ) const	{ return getLeft().intersect(s, p); }
		bool intersectRight( const Segment &s, Point &p ) const	{ return getRight().intersect(s, p); }
		bool intersectTop( const Segment &s, Point &p ) const	{ return getTop().intersect(s, p); }
		bool intersectBottom( const Segment &s, Point &p ) const{ return getBottom().intersect(s, p); }

		numType		xMin() const { return corner[2].x; }
		numType		xMax() const { return corner[0].x; }
		numType		yMin() const { return corner[0].y < corner[3].y ? corner[0].y : corner[3].y; }
		numType		yMax() const { return corner[1].y > corner[2].y ? corner[1].y : corner[2].y; }

		bool intersect( const Segment &s ) const
		{
#ifdef FIXLATER
			// Check trivial rejection.
			if( s.xMax() < xMin() || xMax() < s.xMin() || s.yMax() < yMin() || yMax() < s.yMin() )
				return false;

			// Check for point inclusion.
			if( contains(*s.from) || contains(*s.to) )
				return true;

			// Check right side intersection.
			if( between(s.xMin(), xMax(), s.xMax()) && between(corner[0].y, s.getY(xMax()), corner[1].y) )
				return true;

			// Check left side intersection.
			if( between(s.xMin(), xMin(), s.xMax()) && between(corner[3].y, s.getY(xMin()), corner[2].y) )
				return true;

			// Check top and bottom.
			Point	p;
			if( intersectTop(s, p) && !corner[1].eqTol(p) && !corner[2].eqTol(p) )
				return true;
			if( intersectBottom(s, p) && !corner[0].eqTol(p) && !corner[3].eqTol(p) )
				return true;
			return false;
#endif
			// Check for point inclusion.
			if( contains(*s.from) || contains(*s.to) )
				return true;

			Point	p;
			return intersectRight(s, p) || intersectTop(s, p) || intersectLeft(s, p) || intersectBottom(s, p);
		}

		enum CutType { ThreePiecesOnRight, ThreePiecesOnLeft, TwoPieces, FourPieces, NoCut };
		void	subdivide( const Segment &s );

#ifdef Trapezoid_Debug
		void	printOn( ostream &os, const int level ) const;
		void	printPDF( const int level ) const;
#endif
	};


public:
	typedef std::vector<Point>	PointVector;
	
	Trapezoidal() :
	  // Initialize the root trapezoid.  This must be large enough to contain all segments.
	  p0(rootRangeMin, rootRangeMax), p1(rootRangeMax, rootRangeMax), p2(rootRangeMin, rootRangeMin), p3(rootRangeMax, rootRangeMin),
	  topUniverse(&p0,&p1), bottomUniverse(&p2,&p3),
	  root( NULL, topUniverse, bottomUniverse, topUniverse.xMin(), topUniverse.xMax() ),
	  reInitialize(true)
	  {}

	~Trapezoidal() { clear(); }

	bool	empty() const { return segments.empty(); }

	void	addSegment( const Segment &sIn );
	void	addSegment( const Point &from, const Point &to ) { addSegment( Segment(&from, &to) ); }

	bool	crossesExistingSegment( const Segment &s );
	bool	crossesExistingSegment( const Point &from, const Point &to ) { return crossesExistingSegment( Segment(&from, &to) ); }

	struct PointData
	{
		Point	p;
		void	*data;
	};
	typedef std::vector<PointData>	PointDataVector;
	void	filterCrossing( const Point &origin, PointDataVector &destinations );

#ifdef Trapezoid_Debug
	void	printOn( ostream &os );
	typedef std::vector<Segment> SegmentVector;
	void	printPDF( const SegmentVector &sv1 = SegmentVector(), const SegmentVector &sv2 = SegmentVector() );
#endif

protected:

	Trapezoidal &operator=( const Trapezoidal &t );	// prevent this from being called.

	void	reset();
	void	clear();
	void	permutePoints( const bool subtract = false );
	void	unpermutePoints() { permutePoints(true); }
	void	insertSegment( const Segment &s );
	void	insertSegments();

	// These points and Segments are used to initialize the root trapezoid.
	const Point	p0, p1, p2, p3;
	const Segment	topUniverse, bottomUniverse;
	Trapezoid	root;

	SegmentSet		segments;
	PPointVector	pPointVector;	// Used to keep track of the permuted points.
	PointPMap		pointPMap;		// Used to map unpermuted points to the permuted ones.

	bool		reInitialize;		// Flag indicating that 

	static const double rootRangeMin, rootRangeMax;
};

#ifdef Trapezoid_Debug
inline ostream &operator<<( ostream &os, const Trapezoidal::Point &p )
{ p.printOn(os); return os; }

inline ostream &operator<<( ostream &os, const Trapezoidal::Segment &s )
{ s.printOn(os); return os; }

inline ostream &operator<<( ostream &os, const Trapezoidal::Trapezoid &t )
{ t.printOn(os, 0); return os; }

inline ostream &operator<<( ostream &os, Trapezoidal &t )
{ t.printOn(os); return os; }
#endif

#endif // Trapezoid_H
