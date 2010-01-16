//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "qtree.h"
#include "dheap.h"
#include <limits.h>

MMinitMax( QTree::Leaf, 512 );
MMinitMax( QTree::Parent, 128 );

void	QTree::initRange( coord_t xMin, coord_t yMin, coord_t xMax, coord_t yMax )
{
	clear();
	if( xMin > xMax ) { coord_t t = xMin; xMin = xMax; xMax = t; }
	if( yMin > yMax ) { coord_t t = yMin; yMin = yMax; yMax = t; }
	const coord_t	xDiff = xMax - xMin;
	const coord_t	yDiff = yMax - yMin;
	const coord_t	maxDiff = xDiff > yDiff ? xDiff : yDiff;

	// Find a power of 2 greater than maxDiff.
	coord_t size = 1;
	while( size < maxDiff )
		size <<= 1;

	root->square = Square(Point(xMin, yMin), size);
	root->reset();
}

QTree::QTree()
{
	root = new Parent( NULL, Square(Point(0,0), 1) );
	const	coord_t	maxSpread = (1<<28);
	initRange( -maxSpread/2, -maxSpread/2, maxSpread/2, maxSpread/2);
}

QTree::QTree( coord_t xMin, coord_t yMin, coord_t xMax, coord_t yMax )
{
	root = new Parent( NULL, Square(Point(0,0), 1) );
	initRange( xMin, yMin, xMax, yMax );
}

QTree::~QTree()
{
	clear();
	delete root;
}

void QTree::insert( const Point &point, const ident_t id )
{
	register Parent::Quadrant q;
	register Parent *cur = root;
	register Node *child;

	for( ;; )
	{
		q = cur->quadrantContaining(point);
		if( (child = cur->getChild(q)) == NULL )
		{
			// Insert this point as a new leaf.
			cur->setChild( q, new Leaf(cur, cur->childSquare(q), point, id) );
			break;
		}
		else if( child->type == Node::ParentType )
		{
			// This is a parent node.  Continue searching.
			cur = (Parent *)child;
		}
		else
		{
			// There is a leaf here already.
			// Keep subdividing the space until the current child and the new point
			// are not in the same square.

			Leaf	*leaf = (Leaf *)child;

			// Ignore duplicate points.
			if( leaf->point == point )
				break;

			register Parent *newChild;
			register Parent::Quadrant qLeaf;
			for( ;; )
			{
				cur->setChild( q, newChild = new Parent(cur, cur->childSquare(q)) );
				cur = newChild;
				if( (q = cur->quadrantContaining(point)) !=
					(qLeaf = cur->quadrantContaining(leaf->point)) )
				{
					cur->setChild( q, new Leaf(cur, cur->childSquare(q), point, id) );
					cur->setChild( qLeaf, leaf );
					break;
				}
			}
			break;
		}
	}
}

void QTree::resetFlags()
{
	// Reset the searched flags.
	register Node *child;
	register Parent *cur = root;

	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			cur->searched = 0;
			cur->ichild = 0;
			if( cur->parent == NULL )
				return;
			cur = (Parent *)cur->parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			if( ((Parent *)child)->searched )
			{
				if( ((Parent *)child)->ichild == 4 )
				{
					// This node's subtree was searched.
					// Recurse to the child.
					cur = (Parent *)child;
					cur->ichild = 0;
				}
				else
				{
					// This node was not searched because it did not intersect the boundingSquare.
					// No need to recurse to the child - just reset the flag.
					((Parent *)child)->searched = 0;
				}
			}
		}
	}
}

bool QTree::findNearest( const Point &search, Point &find, ident_t &id )
{
	register Parent::Quadrant q;
	register Parent *cur = root;
	register Node *child;
	coord_t	dMin;
	bool	found;

	// First, find the quadrant that contains the point.
	for( ;; )
	{
		q = cur->quadrantContaining(search);
		if( (child = cur->getChild(q)) == NULL )
		{
			// Set lower bound so we get all the other children.
			dMin = cur->square.size + cur->square.size/2;
			found = false;
			break;
		}
		else if( child->type == Node::ParentType )
		{
			// This is a parent node.  Continue searching.
			cur = (Parent *)child;
		}
		else
		{
			// We found the leaf that contains the point.
			// Set the best lower bound.
			find = ((Leaf *)child)->point;
			id = ((Leaf *)child)->id;
			dMin = search.distance( find );
			if( dMin == 0 ) // Don't argue with perfection.
				return true;
			found = true;
			break;
		}
	}

	Square	boundingSquare( Point(search.x - dMin/2, search.y - dMin/2), dMin );
	register coord_t	d;

	// Use a branch and bound technique to find the nearest point.
	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			// Move up to the parent.
			cur->searched = 1;
			if( cur->parent == NULL )
				break;
			cur = (Parent *)cur->parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			if( ((Parent *)child)->searched )
				continue;

			if( ((Parent *)child)->square.intersects(boundingSquare) )
			{
				// Dive into the child as we have not searched it before.
				cur = (Parent *)child;
				cur->ichild = 0;
				continue;
			}

			// Otherwise, mark this child as searched.
			((Parent *)child)->ichild = 0;
			((Parent *)child)->searched = 1;
			continue;
		}

		// Check if this leaf contains a closer point.
		if( (d = search.distance( ((Leaf *)child)->point )) < dMin )
		{
			find = ((Leaf *)child)->point;
			id = ((Leaf *)child)->id;
			dMin = d;
			boundingSquare.center( search, dMin );
			found = true;
		}
	}

	resetFlags();

	return found;
}

size_t QTree::findNearestN( const Point &search, PointID *pointIDs, const size_t maxPointIDs )
{
	if( maxPointIDs == 0 )
		return 0;

	QHeapElement	*heap = new QHeapElement[maxPointIDs];
	size_t			N = 0;
	QEdecreasingD	cmpD;

	register Parent::Quadrant q;
	register Parent *cur = root;
	register Node *child;

	// First, find the quadrant that contains the point.
	for( ;; )
	{
		q = cur->quadrantContaining(search);
		if( (child = cur->getChild(q)) == NULL )
		{
			// The child is found in an empty square.
			break;
		}
		else if( child->type == Node::ParentType )
		{
			// This is a parent node.  Continue searching.
			cur = (Parent *)child;
		}
		else
		{
			// We found the leaf that contains the point.
			break;
		}
	}

	Square	boundingSquare( search, 1 );
	register coord_t	d;

	// Use a branch and bound technique to find the nearest k points.
	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			// Move up to the parent.
			cur->searched = 1;
			if( cur->parent == NULL )
				break;
			cur = (Parent *)cur->parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			if( ((Parent *)child)->searched )
				continue;

			if( N < maxPointIDs || ((Parent *)child)->square.intersects(boundingSquare) )
			{
				// Dive into the child as we have not searched it before.
				cur = (Parent *)child;
				cur->ichild = 0;
				continue;
			}

			// Otherwise, mark this child as searched.
			((Parent *)child)->ichild = 0;
			((Parent *)child)->searched = 1;
			continue;
		}

		if( N < maxPointIDs )
		{
			// There are not enough points.  Add another one.
			heap[N].leaf = (Leaf *)child;
			heap[N].d = search.distance( ((Leaf *)child)->point );
			if( ++N == maxPointIDs )
			{
				DHeapBuild( heap, N, cmpD );
				boundingSquare.center( search, heap[0].d );
			}
		}
		else if( (d = search.distance( ((Leaf *)child)->point )) < heap[0].d )
		{
			// We found a closer point.  Replace the max point with the new one.
			heap[0].leaf = (Leaf *)child;
			heap[0].d = d;

			DHeapDemote( heap, N, (unsigned int)0, cmpD );

			// Update the bounding square with the next largest distance.
			boundingSquare.center( search, heap[0].d );
		}
	}

	resetFlags();

	if( N == 0 )
		return 0;	// Nothing matches.

	// Fill in the return array by increasing distance from the search point.
	// This means we have to copy in reverse order from the top of the heap.
	if( N != maxPointIDs )
		DHeapBuild( heap, N, cmpD );
	const size_t nSave = N;
	register	size_t j = N;
	do
	{
		--j;
		pointIDs[j].point = heap[0].leaf->point;
		pointIDs[j].id    = heap[0].leaf->id;
		DHeapRemoveTop( heap, N, cmpD );
	} while( j != 0 );

	delete [] heap;
	return nSave;
}

size_t QTree::findInRadius( const Point &search, const coord_t radius, PointID *pointIDs, const size_t maxPointIDs )
{
	if( maxPointIDs == 0 )
		return 0;

	register Node *child;
	register Parent *cur = root;
	size_t	numPointIDs = 0;

	const Square	boundingSquare( Point(search.x - radius/2, search.y - radius/2), radius );

	// Search all regions that intersect the boundingSquare.
	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			// Move up to the parent.
			cur->searched = 1;
			if( cur->parent == NULL )
				break;
			cur = (Parent *)cur->parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			if( ((Parent *)child)->searched )
				continue;
				
			if( ((Parent *)child)->square.intersects(boundingSquare) )
			{
				// Dive into the child as we have not searched it before.
				cur = (Parent *)child;
				cur->ichild = 0;
				continue;
			}

			// Otherwise, mark this child as searched.
			((Parent *)child)->ichild = 0;
			((Parent *)child)->searched = 1;
			continue;;
		}

		// Check if this leaf contains a node in the radius.
		if( search.distance( ((Leaf *)child)->point ) <= radius )
		{
			if( numPointIDs < maxPointIDs )
			{
				pointIDs[numPointIDs].point = ((Leaf *)child)->point;
				pointIDs[numPointIDs].id    = ((Leaf *)child)->id;
				numPointIDs++;
			}
		}
	}

	resetFlags();

	return numPointIDs;
}

void QTree::clear()
{
	// Delete everything but the root.
	register Node *child;
	register Parent *cur = root;

	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			if( cur->parent == NULL )
				break;
			Parent	*parent = (Parent *)cur->parent;
			delete cur;
			cur = parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			// Recurse to the child.
			cur = (Parent *)child;
			cur->ichild = 0;
		}
		else
			delete (Leaf *)child;
	}

	root->reset();
}

#ifdef DIAGNOSTICS

static	const	int	lmargin = 72 / 2;
static	const	int	rmargin = (int)((8.5*72) - lmargin);
static	const	int	tmargin = lmargin;
static	const	int	psize = rmargin - lmargin;
static	const	double	scale = psize / 1024.0;

static	void	moveTo( ostream &os, coord_t x, coord_t y )
{
	os << ((x * scale) + lmargin) << ' ' << (((1024 - y) * scale) + tmargin) << " moveto\n";
}

static	void	lineTo( ostream &os, coord_t x, coord_t y )
{
	os << ((x * scale) + lmargin) << ' ' << (((1024 - y) * scale) + tmargin) << " lineto\n";
}

void QTree::print( ostream &os )
{
	os << "%%PS-Adobe\n";

	// Print everything.
	register Node *child;
	register Parent *cur = root;

	cur->ichild = 0;

	for( ;; )
	{
		while( cur->ichild < 4 && (child = cur->getChild(cur->ichild)) == NULL )
			cur->ichild++;

		if( cur->ichild == 4 )
		{
			moveTo( os, cur->square.xMin(), cur->square.yMin() );
			lineTo( os, cur->square.xMax(), cur->square.yMin() );
			lineTo( os, cur->square.xMax(), cur->square.yMax() );
			lineTo( os, cur->square.xMin(), cur->square.yMax() );
			lineTo( os, cur->square.xMin(), cur->square.yMin() );

			moveTo( os, cur->square.xMin() + cur->square.size/2, cur->square.yMin() );
			lineTo( os, cur->square.xMin() + cur->square.size/2, cur->square.yMax() );
			moveTo( os, cur->square.xMin(), cur->square.yMin() + cur->square.size/2);
			lineTo( os, cur->square.xMax(), cur->square.yMin() + cur->square.size/2);

			os << "stroke\n";
			if( cur->parent == NULL )
				break;
			cur = (Parent *)cur->parent;
			continue;
		}

		cur->ichild++;
		if( child->type == Node::ParentType )
		{
			// Recurse to the child.
			cur = (Parent *)child;
			cur->ichild = 0;
		}
		else
		{
			moveTo( os, ((Leaf *)child)->point.x-2, ((Leaf *)child)->point.y-2 );
			lineTo( os, ((Leaf *)child)->point.x+2, ((Leaf *)child)->point.y+2 );
			lineTo( os, ((Leaf *)child)->point.x+2, ((Leaf *)child)->point.y-2 );
			lineTo( os, ((Leaf *)child)->point.x-2, ((Leaf *)child)->point.y+2 );
			os << "stroke\n";
		}
	}

	os << "showpage\n";
}

#endif // DIAGNOSTICS
