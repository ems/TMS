//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <tod.h>
extern "C" {
#include <TMSHeader.h>
#include <rc.h>
}
#include <math.h>
#include <match.H>
#include <fstream>
#include <lpgen.H>

using namespace std;

//#define PRINT_MATCH_STATUS
//#define PRINT_MATCH_RESULT

inline static int InRange( tod_t min, tod_t x, tod_t max )
{
    return min <= x && x <= max;
}

int LPGen::isValidRun( const aggPiece &a )
{
    long cost;
    if( a.breakPoint < 0 )
	return isValidOnePiece( a.from(), a.to() );
    else
	return getPairCost( cost,
			    a.from(), a.reliefs[a.breakPoint-1]->to(),
 			    a.reliefs[a.breakPoint]->from(), a.to() );
 			    
}
 
int LPGen::isPotentialTwoPiece( ReliefPoint *from, ReliefPoint *to )
{
    RunDef	*rd;
    int		reason = 0;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
	if( rd->getNumWorking() != 2 )
	    continue;
	RunComponent	*rc;
	for( rc = rd->cHead; rc != NULL; rc = rc->next )
	    if( rc->lenMin > to->getT() - from->getT() )
		reason = -1;	// too small
	    else if( rc->lenMax < to->getT() - from->getT() )
		reason = 1;	// too large
	    else
		return 0;	// Just right.
    }
    return reason;
}

int LPGen::getShortPairCost( long &cost,
			     ReliefPoint *from1, ReliefPoint *to1,
			     ReliefPoint *from2, ReliefPoint *to2 )
{
    if( isPotentialTwoPiece(from1, to2) != 0 ) return 0;

    if( !ReliefIsPossible(m_pRELIEFPOINTS[to1->ri].TRIPSrecordID,
                          m_pRELIEFPOINTS[to1->ri].NODESrecordID,
                          m_pRELIEFPOINTS[to1->ri].SGRPSERVICESrecordID,
                          m_pRELIEFPOINTS[to1->ri].time,
                          m_pRELIEFPOINTS[from2->ri].TRIPSrecordID,
                          m_pRELIEFPOINTS[from2->ri].NODESrecordID,
                          m_pRELIEFPOINTS[from2->ri].time) )
        return 0;

    RunDef	*rd;
    int		reason = 0;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
	if( rd->getNumWorking() != 2 )
	    continue;
	RunComponent	*rc = rd->cHead;
        cost = Abs( rc->lenDes - (to2->to()->getT() - from1->from()->getT()) );
        return 1;
    }
    return 0;
}

int LPGen::getSlagMatchCost( long &cost,
			     ReliefPoint *from1, ReliefPoint *to1,
			     ReliefPoint *from2, ReliefPoint *to2 )
{
    if( isPotentialTwoPiece(from1, to2) > 0 ) return 0;
    if( !ReliefIsPossible(m_pRELIEFPOINTS[to1->ri].TRIPSrecordID,
                          m_pRELIEFPOINTS[to1->ri].NODESrecordID,
                          m_pRELIEFPOINTS[to1->ri].SGRPSERVICESrecordID,
                          m_pRELIEFPOINTS[to1->ri].time,
                          m_pRELIEFPOINTS[from2->ri].TRIPSrecordID,
                          m_pRELIEFPOINTS[from2->ri].NODESrecordID,
                          m_pRELIEFPOINTS[from2->ri].time) )
        return 0;

    // Just penalize the difference between start and end.
    cost = from2->getT() - to1->getT();
    return 1;
}

int LPGen::getPairCost( long &cost,
			ReliefPoint *from1, ReliefPoint *to1,
			ReliefPoint *from2, ReliefPoint *to2 )
{
    RunDef	*rd;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
	if( rd->getNumWorking() != 2 )
	    continue;

	// Check the spread constraints.
	if( rd->spreadMax && to2->getT() - from1->getT() > rd->spreadMax ) continue;

	// Check the starting constraints.
	if( rd->mustStartBefore && from1->getT() > rd->mustStartBefore ) continue;
	if( rd->mustStartAfter  && from1->getT() < rd->mustStartAfter  ) continue;

	// Check the ending constraints.
	if( rd->mustEndBefore && to2->getT() >= rd->mustEndBefore ) continue;
	if( rd->mustEndAfter  && to2->getT() <  rd->mustEndAfter  ) continue;

	const tod_t	len1 = to1->getT() - from1->getT();
	const tod_t	lenB = from2->getT() - to1->getT();
	const tod_t	len2 = to2->getT() - from2->getT();

	RunComponent	*rc1 = rd->cHead;
	RunComponent	*rcB = rc1->next;
	RunComponent	*rc2 = rcB->next;

	// Check if each piece is in range.
	if( !InRange(rc1->lenMin, len1, rc1->lenMax) ) continue;
	if( !InRange(rcB->lenMin, lenB, rcB->lenMax) ) continue;
	if( !InRange(rc2->lenMin, len2, rc2->lenMax) ) continue;

	const tod_t workTotal = len1 + len2;

	// Check if the total worktime is in range.
	if( rd->workMin && workTotal < rd->workMin )   continue;
	if( rd->workMax && workTotal > rd->workMax )   continue;
	
	if( workTotal < 7L*60*60 )
	    cost = 7L*60L*60L;
	else
	    cost = workTotal;
	return 1;
    }
    return 0;
}

int	LPGen::isValidOnePiece( ReliefPoint *from, ReliefPoint *to )
{
    RunDef	*rd;
    for( rd = rdHead; rd != NULL; rd = rd->next )
    {
	if( rd->getNumWorking() != 1 )
	    continue;

	if( rd->isPenalty ) continue;

	// Check the spread constraints.
	if( rd->spreadMax && to->getT() - from->getT() > rd->spreadMax ) continue;

	// Check the starting constraints.
	if( rd->mustStartBefore && from->getT() > rd->mustStartBefore ) continue;
	if( rd->mustStartAfter  && from->getT() < rd->mustStartAfter  ) continue;

	// Check the ending constraints.
	if( rd->mustEndBefore && to->getT() >= rd->mustEndBefore ) continue;
	if( rd->mustEndAfter  && to->getT() <  rd->mustEndAfter  ) continue;

	const tod_t	len = to->getT() - from->getT();
	RunComponent	*rc = rd->cHead;

	// Check if each piece is in range.
	if( !InRange(rc->lenMin, len, rc->lenMax) ) continue;

	const tod_t workTotal = len;

	// Check if the total worktime is in range.
	if( rd->workMin && workTotal < rd->workMin )   continue;
	if( rd->workMax && workTotal > rd->workMax )   continue;

	return 1;
    }
    return 0;
}

//------------------------------------------------------------------------
void	LPGen::cutRuns()
{
    RBlock	*rb;
    ReliefPoint	*from, *to;

    ReliefPoint	*onePieceHead = NULL;
    long	numOnePiece = 0;

    ReliefPoint	*twoPieceHead = NULL;
    long	numTwoPiece = 0;

    ReliefPoint	*multiPieceHead = NULL;
    long	numMultiPiece = 0;

    ReliefPoint *doneHead = NULL;

    aggPiece	*aggToDo = NULL;

    // First, extract all the one piece runs.
    for( rb = rbHead; rb != NULL; rb = rb->next )
	for( to = rb->tail; to->from() != NULL; to = to->from() )
	    if( to->i != NULL )
	    {
		from = to->from();
		switch( (to->i)->owner->owner->getNumWorking() )
		{
		case 2:
		    to->addToPieceList( twoPieceHead );
		    numTwoPiece++;
		    break;
		default:
		    if( isValidOnePiece(from, to) )
		    {
			to->addToPieceList( onePieceHead );
			numOnePiece++;
		    }
		    else if( isPotentialTwoPiece(from, to) == 0 )
		    {
			to->addToPieceList( twoPieceHead );
			numTwoPiece++;
		    }
		    else
		    {
			to->addToPieceList( multiPieceHead );
			numMultiPiece++;
		    }
		    break;
		}
	    }
	    else
		to->addToPieceList( multiPieceHead );

#ifdef PRINT_MATCH_RESULT
    cout << "One piece runs:\n";
#endif
    for( to = onePieceHead; to != NULL; to = to->nextPiece )
    {
	from = to->from();
	to->done = 1;

#ifdef PRINT_MATCH_RESULT
	cout << from->getBlockNum() << '\t';
	cout << asTime(from->getT()) << '-' << asTime(to->getT());
	cout << ' '<< asTime(to->getT() - from->getT()) << '\n';
#endif

	aggPiece ap( to );
	ap.installRun( *this );
    }
    doneHead = onePieceHead;
    onePieceHead = NULL;

    aggPiece	*a;
    // Put all slag and potential 2 piece chunks onto
    // the aggregate piece list.
    for( to = twoPieceHead; to != NULL; to = to->nextPiece )
    {
	a = new aggPiece( to );
	a->addToList( aggToDo );
    }
    for( to = multiPieceHead; to != NULL; to = to->nextPiece )
    {
	a = new aggPiece( to );
	a->addToList( aggToDo );
    }

    // Match up the little pieces until everything is about the size
    // of a potential 2 piece.
    do
    {
#ifdef PRINT_MATCH_STATUS
	cout << "Matching slag...\n";
#endif
    } while( matchAgg(aggToDo, &LPGen::getSlagMatchCost) );


    // Now, do a matching on all the potential aggregate pairs.
    matchAgg( aggToDo, &LPGen::getPairCost );

    // Normalize all the potential pieces.
    for( a = aggToDo; a != NULL; a = a->next )
	a->normalize();

#ifdef PRINT_MATCH_RESULT
    cout << "\nFinal matched runs:\n";
#endif
    for( a = aggToDo; a != NULL; a = a->next )
    {
#ifdef PRINT_MATCH_RESULT
	cout << (isValidRun(*a) ? "good" : "bad") << '\t';
	cout << *a << '\n';
#endif
	a->installRun( *this );
    }

    // Cleanup.
    aggPiece	*aNext;
    for( a = aggToDo; a != NULL; a = aNext )
    {
	aNext = a->next;
	delete a;
    }

	long savings;
	RuncutImprove(&savings);
}

int	LPGen::matchAgg( aggPiece *&toDo, MatchCostFunc mcf )
{
    // First, count the number of aggregate pieces.
    aggPiece *a, *aNext;
    long numAggPiece = 0;
    for( a = toDo; a != NULL; a = a->next )
	numAggPiece++;

    if( numAggPiece == 0 )
	return 0;    // Nothing to do.

    // Now, create a match graph for all of the penalty pairs.
    MatchGraph	mg( numAggPiece * 2 );
    for( a = toDo, numAggPiece = 0;
	 a != NULL;
	 a = a->next, numAggPiece++ )
    {
	mg.setNodeLabel( numAggPiece*2+1, (unsigned long)a );
	mg.AddEdge( numAggPiece*2+1, numAggPiece*2+2, 48L*60L*60L );
    }

    // Now, add all the other short edges in the graph.
    aggPiece	*p1, *p2;
    long	p1c, p2c;
    int		addedMatchEdge = 0;
    for( p1c = 0, p1 = toDo; p1 != NULL; p1 = p1->next, p1c++ )
    {
	ReliefPoint	*to1 = p1->to();
	ReliefPoint	*from1 = p1->from();
	for( p2c = 0, p2 = toDo; p2 != NULL; p2 = p2->next, p2c++ )
	{
	    ReliefPoint	*to2 = p2->to();
	    ReliefPoint	*from2 = p2->from();

	    // Check that the first piece occurs before the second piece.
	    // This avoids duplicate edges in the match graph.
	    if( from1->getT() > from2->getT() )
		continue;
	    // Check if the pieces overlap.
	    if( to1->getT() > from2->getT() )
		continue;
	    // Check if there are more than 3 pieces in the run.
//	    if( p1->numReliefs + p2->numReliefs > 3 )
//		continue;

	    long cost;
	    if( (this->*mcf)(cost, from1, to1, from2, to2) )
	    {
                // Add the cost edge and the penalty relief edge.
		mg.AddEdge( p1c*2+1, p2c*2+1, cost );
		mg.AddEdge( p1c*2+2, p2c*2+2, 0L );
		addedMatchEdge = 1;
	    }
	}
    }
    if( !addedMatchEdge )
	return 0;

    // Solve the minimum cost matching problem.
    mg.Weighted_Match( 0 );

    aggPiece	*matched = NULL;
    int	matchedSomething = 0;
    for( a = toDo, numAggPiece = 0;
	 a != NULL;
	 a = a->next, numAggPiece++ )
    {
	ReliefPoint	*from1 = a->from();
	const long mate = mg.Mate( numAggPiece*2+1 );
	if( (mate & 1) == 0 )
	{
	    // This piece was not matched.
	}
	else
	{
	    // This piece was matched.
	    aggPiece	*a2 = (aggPiece *)mg.getNodeLabel( mate );
	    ReliefPoint	*from2 = a2->from();

	    if( from1->getT() < from2->getT() )
	    {
		a->done = a2->done = 1;
		aggPiece *m = new aggPiece( *a, *a2 );
		m->addToList( matched );
		m->breakPoint = a->numReliefs;
	    }
	    matchedSomething = 1;
	}
    }

    if( !matchedSomething )
	return 0;

    // Clean all the matched pieces out of the doDo list.
    for( a = toDo; a != NULL; a = aNext )
    {
	aNext = a->next;
	if( a->done )
	{
	    a->removeFromList( toDo );
	    delete a;
	}
    }

    // Add all the new aggregated pieces into the toDo list.
    for( a = matched; a != NULL; a = aNext )
    {
	aNext = a->next;
	a->removeFromList( matched );
	a->addToList( toDo );
    }

    return 1;
}
