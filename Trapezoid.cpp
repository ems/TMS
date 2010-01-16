//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "Trapezoid.h"

#ifdef Trapezoid_Debug
const double Trapezoidal::rootRangeMin = -4;
const double Trapezoidal::rootRangeMax = 4;
#else
// Maximum range we can handle.  This should be as small as possible to minimize numeric overflow.
const double Trapezoidal::rootRangeMin = -2000000.0;
const double Trapezoidal::rootRangeMax = 2000000.0;
#endif

void	Trapezoidal::Trapezoid::subdivide( const Segment &s )
{
	Point	p0, p1;
	CutType	cut = NoCut;
	bool	containsPoint = false;

	do
	{
		// Deal with the case when the end of the segment is in the trapezoid.
		if( contains(*s.from) )
		{
			containsPoint = true;
			p0 = *s.from;
			if( contains(*s.to) )	// Check if both endpoints are in the trapezoid.
			{
				p1 = *s.to;
				cut = FourPieces;
				break;
			}
		}
		else if( contains(*s.to) )
		{
			containsPoint = true;
			p0 = *s.to;
		}

		if( containsPoint )
		{
			if( intersectRight(s, p1) )	{ cut = ThreePiecesOnRight;	break; }
			if( intersectLeft(s, p1) )	{ cut = ThreePiecesOnLeft;	break; }
			if( intersectTop(s, p1) )	{ cut = FourPieces; break; }
			if( intersectBottom(s, p1) ){ cut = FourPieces; break; }
			break;
		}

		// The segment intersects with two edge of the trapezoid.
		// Make sure we handle degeneracies.
		if( intersectTop(s, p0) )
		{
			if( intersectRight(s, p1) && !p0.eqTol(p1) )	{ cut = ThreePiecesOnRight; break; }
			if( intersectLeft(s, p1) && !p0.eqTol(p1) )		{ cut = ThreePiecesOnLeft;	break; }
			if( intersectBottom(s,p1) && !p0.eqTol(p1) )	{ cut = FourPieces;			break; }
			break;
		}
		if( intersectBottom(s, p0) )
		{
			if( intersectRight(s, p1) && !p0.eqTol(p1) )	{ cut = ThreePiecesOnRight; break; }
			if( intersectLeft(s, p1) && !p0.eqTol(p1) )		{ cut = ThreePiecesOnLeft;	break; }
			break;
		}

		// Make sure this is a two-piece cut.
		if( intersectRight(s, p0) && intersectLeft(s, p1) )
			cut = TwoPieces;

	} while(0);	// Not a loop - just a way to break without a goto.

	switch( cut )
	{
	case ThreePiecesOnRight:
		child[0] = new Trapezoid(this, s, bottom, p0.x, xMax());	// Right
		child[1] = new Trapezoid(this, top, s, p0.x, xMax());		// Top
		child[2] = new Trapezoid(this, top, bottom, xMin(), p0.x);	// Left
		break;
	case ThreePiecesOnLeft:
		child[0] = new Trapezoid(this, top, bottom, p0.x, xMax());	// Right
		child[1] = new Trapezoid(this, top, s, xMin(), p0.x);		// Top
		child[2] = new Trapezoid(this, s, bottom, xMin(), p0.x);	// Bottom
		break;
	case TwoPieces:
		child[0] = new Trapezoid(this, top, s, xMin(), xMax());		// Top
		child[1] = new Trapezoid(this, s, bottom, xMin(), xMax());	// Bottom
		break;
	case FourPieces:
		{
			numType pxMin, pxMax;
			if( p0.x < p1.x )	{ pxMin = p0.x; pxMax = p1.x; }
			else				{ pxMin = p1.x; pxMax = p0.x; }
			child[0] = new Trapezoid(this, top, bottom, pxMax, xMax());	// Right
			child[1] = new Trapezoid(this, top, s, pxMin, pxMax );		// Top
			child[2] = new Trapezoid(this, top, bottom, xMin(), pxMin );// Left
			child[3] = new Trapezoid(this, s, bottom, pxMin, pxMax );	// Bottom
		}
		break;
	case NoCut:
		break;
	}
}

void	Trapezoidal::addSegment( const Segment &sIn )
{
	// Skip degenerate line.
	if( sIn.from == sIn.to )
		return;

	if( !reInitialize )
		unpermutePoints();

	// Put this line in canonical format.
	Segment	sCheck( sIn );
	sCheck.makeCanonical();

	// Check for duplicate segments.
	if( segments.find(sCheck) != segments.end() )
		return;

	// Keep track of the individual points.
	PointPMap::const_iterator fromIn = pointPMap.find(*sCheck.from);
	if( fromIn == pointPMap.end() )
	{
		Point	*pp = new Point(*sCheck.from);
		pPointVector.push_back( pp );
		fromIn = pointPMap.insert(std::make_pair(Point(*sCheck.from), pp)).first;
	}
	PointPMap::const_iterator toIn = pointPMap.find(*sCheck.to);
	if( toIn == pointPMap.end() )
	{
		Point	*pp = new Point(*sCheck.to);
		pPointVector.push_back( pp );
		toIn = pointPMap.insert(std::make_pair(Point(*sCheck.to), pp)).first;
	}

	segments.insert( Segment(fromIn->second, toIn->second) );

	reInitialize = true;
}

void	Trapezoidal::reset()
{
	register Trapezoid	*trapCur = &root, *child, *parent;
	trapCur->childCur = 0;

	// Do a DFS on the tree, deleting children as we move up.
	for( ;; )
	{
		while( trapCur->childCur < 4 && (child = trapCur->child[trapCur->childCur++]) )
		{
			// Follow this child.
			trapCur = child;
			trapCur->childCur = 0;
		}

		parent = trapCur->parent;
		if( parent )
			delete trapCur;
		else
			break;
		trapCur = parent;
	}

	root.child[0] = NULL;
	root.child[1] = NULL;
	root.child[2] = NULL;
	root.child[3] = NULL;
}

void Trapezoidal::clear()
{
	reset();
	for( register PPointVector::iterator p = pPointVector.begin(), pEnd = pPointVector.end(); p != pEnd; ++p )
		delete *p;
	pPointVector.clear();
	pointPMap.clear();
	segments.clear();
}

struct PermutePoint
{
	PermutePoint( const bool aSubtract = false ) : subtract(aSubtract) {}
	PermutePoint( const PermutePoint &p ) : subtract(p.subtract) {}

	enum { srandSeed = 0xeded };

	const bool	subtract;

	static const double maxPermutePoint;

	void operator()( Trapezoidal::Point &p ) const
	{
		const double	dx = (((double)rand() / (double)RAND_MAX - 0.5) * maxPermutePoint);
		const double	dy = (((double)rand() / (double)RAND_MAX - 0.5) * maxPermutePoint);
		if( subtract )
		{
			p.x -= (Trapezoidal::numType)dx;
			p.y -= (Trapezoidal::numType)dy;
		}
		else
		{
			p.x += (Trapezoidal::numType)dx;
			p.y += (Trapezoidal::numType)dy;
		}
	}
};
const double PermutePoint::maxPermutePoint = 1.0 / 500.0;

void	Trapezoidal::permutePoints( const bool subtract )
{
	// Make sure none of the points are perfectly vertical or co-linear.
	srand( PermutePoint::srandSeed );
	PermutePoint	pp( subtract );
	for( register PPointVector::iterator p = pPointVector.begin(), pEnd = pPointVector.end(); p != pEnd; ++p )
		pp( **p );
}

void	Trapezoidal::insertSegment( const Segment &s )
{
	// Check if this is the first child.
	if( !root.hasChildren() )
	{
		root.subdivide( s );
		return;
	}

	register Trapezoid	*trapCur = &root, *c;
	trapCur->childCur = 0;

	// Do a DFS on the tree following any path with a child that intersects the Segment.
	do
	{
		while( trapCur->childCur < 4 && (c = trapCur->child[trapCur->childCur++]) )
		{
			if( !c->intersect(s) )
				continue;

			if( !c->hasChildren() )
				c->subdivide( s );
			else
			{
				// Follow this child.
				trapCur = c;
				trapCur->childCur = 0;
			}
		}
	} while( trapCur = trapCur->parent );
}

void	Trapezoidal::insertSegments()
{
	// Clear the init flag.
	reInitialize = false;

	// Empty the current trapezoidal decomposition.
	reset();

	// Make sure there are no vertical or co-linear points.
	permutePoints();

	// Randomize the order of the segments before inserting them.
	// This creates a more balanced search tree.
	std::vector<const Segment *>	vSegments;
	vSegments.reserve( segments.size() );

	for( SegmentSet::const_iterator sItr = segments.begin(); sItr != segments.end(); ++sItr )
		vSegments.push_back( &(*sItr) );

	std::random_shuffle( vSegments.begin(), vSegments.end() );

	// Add the randomized segments to the trapezoidal decomposition.
	for( std::vector<const Segment *>::iterator i = vSegments.begin(); i != vSegments.end(); ++i )
		insertSegment( **i );
}
	
bool	Trapezoidal::crossesExistingSegment( const Segment &s )
{
	// Determine if the given Segment crosses an existing Segment in O(logN) steps
	// (where N is the number of Segments).

	if( reInitialize )
		insertSegments();

	if( !root.hasChildren() )
		return false;

	register Trapezoid	*trapCur = &root, *child;
	trapCur->childCur = 0;

	Point		p;

	// Do a DFS on the tree following any path with a child that intersects the Segment.
	do
	{
		while( trapCur->childCur < 4 && (child = trapCur->child[trapCur->childCur++]) )
		{
			// If the top or bottom of the trapezoid intersect the segment, we found a crossing.
			if( child->intersectTop(s, p) || child->intersectBottom(s, p) )
				return true;

			// At this point, either the segment:
			//		- intersects the left or right boundaries of the trapezoid.
			//		- is contained by the trapezoid.
			//		- is outside the trapezoid entirely.
			if( !child->hasChildren() )
			{
				// Check if the entire segment is contained in a leaf trapezoid.
				if( child->contains(*s.from) && child->contains(*s.to) )
					return false;
			}
			else if(child->contains(*s.from) || child->contains(*s.to) ||
					child->intersectLeft(s, p) || child->intersectRight(s, p) )
			{
				// At this point, we have determined that the segment might intersect with some child trapezoid.
				// Explore this possibility.
				trapCur = child;
				trapCur->childCur = 0;
			}
		}
	} while( trapCur = trapCur->parent );

	// We fathomed the tree and did not find anything - the given segment does not cross any existing segment.
	return false;
}

class TestSegmentCrossing
{
public:
	TestSegmentCrossing( Trapezoidal &aT, const Trapezoidal::Point &aFrom ) : t(aT), from(aFrom) {}
	TestSegmentCrossing( const TestSegmentCrossing &tlc ) : t(tlc.t), from(tlc.from) {}

	Trapezoidal					&t;
	const Trapezoidal::Point	&from;

	bool	operator()( const Trapezoidal::PointData &to ) const
	{ return t.crossesExistingSegment(Trapezoidal::Segment(&from, &to.p)); }
};

void	Trapezoidal::filterCrossing( const Point &origin, PointDataVector &destinations )
{
	if( root.hasChildren() )
		destinations.erase( std::remove_if(destinations.begin(), destinations.end(), TestSegmentCrossing(*this, origin)),
							destinations.end() );
}
