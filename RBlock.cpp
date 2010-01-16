//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <lpgen.H>
#include <limits.h>

ReliefPoint::ReliefPoint( const tod_t aT, const long aRi )
{
    t = aT;
    pred = NULL;
    next = NULL;
    nextPiece = NULL;

    i = NULL;
    cost = INFINITY;
    done = 0;
    ri = aRi;
}

ReliefPoint::~ReliefPoint()
{
}

#define Square( x )	((x) * (x))
inline double ReliefPoint::getBoundaryCost( const tod_t multiple ) const
{
    const double denominator = Square(24.0 * 60.0 * 60.0) * 2.0;
    const double n = t - nearestMult(t, multiple);
    return Square(n) / denominator;
}

//------------------------------------------------------------

RBlock::RBlock( const long aNum )
{
    num = aNum;

    cost = 0.0;
    head = tail = NULL;
    next = NULL;
}

RBlock::~RBlock()
{
    ReliefPoint	*r, *rNext;
    for( r = head; r != NULL; r = rNext )
    {
	    rNext = r->next;
	    delete r;
    }
}

void RBlock::putBack( LPGen &g )
{
    ReliefPoint	*to;

    // Put back the pieces used in the last shortest path calculation.
    for( to = tail; to != NULL; to = to->pred )
	    g.incCount( to->i );
}

double RBlock::solve( LPGen &g, const tod_t minLeftoverPiece, const tod_t multiple )
{
    cost = 0.0;

    if( head == NULL )
	    return cost;

    register ReliefPoint	*from, *to;
    int	badPiece = 0;

    // Set the initial cost to 0 in the shortest path.
    head->cost = 0.0;
    head->pred = NULL;

    // Construct the shortest path graph using dynamic programming
    // with implicit arc generation.
    for( to = head->next; to != NULL; to = to->next )
    {
	    to->cost = INFINITY;
	    to->pred = NULL;
	    for( from = head; from != to; from = from->next )
	    {
	        if( to->getT() - from->getT() < minLeftoverPiece )
		        break;
	        const double tCost = g.getCost( from->getT(), to->getT() );
	        double pieceCost;
	        if( tCost >= INFINITY )
	        {
		        pieceCost = Square((double)to->getT() - (double)from->getT());
		        badPiece = 1;
	        }
	        else
	        {
		        pieceCost = tCost;
		        pieceCost += from->getBoundaryCost(multiple);
		        pieceCost += to->getBoundaryCost(multiple);
	        }

	        if( from->cost + pieceCost < to->cost )
	        {
		        to->cost = from->cost + pieceCost;
		        to->pred = from;
	        }
	    }
    }

    // Follow the path from the last relief point back to the first on
    // the shortest path.
    // Consume the pieces as we go.
    for( to = tail; to->pred != NULL; to = to->pred )
    {
	    from = to->pred;
	    g.decCount( to->i, from->getT(), to->getT() );
	    cost += to->cost;
    }

    // Link all the bad cuts together if necessary.
    if( badPiece )
	    for( to = tail; to->pred != NULL; to = to->pred )
	        for( from = to->pred; from->pred != NULL; )
		        if( to->i == NULL && from->i == NULL )
		        {
		            to->pred = from->pred;
		            from = from->pred;
		        }
		        else
		            break;
    
    return cost;
}

ostream &operator<<( ostream &os, const RBlock &rb )
{
    ReliefPoint	*from, *to;

    for( to = rb.tail; to->pred != NULL; to = to->pred )
    {
	    from = to->pred;
	    os << ((to->i != NULL) ? "good\t" : "bad\t");
	    os << asTime(from->getT()) << '\t';
	    os << asTime(to->getT()) << '\t';
	    os << asTime(to->getT() - from->getT()) << '\n';
    }
    os << '\n';
    return os;
}
