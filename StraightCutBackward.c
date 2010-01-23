//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "rc.h"

/****************************************************************/
/*
 * StraightFindBackward takes a ending segment and an starting
 * segment and attempts to find a place to cut backward between
 * the segments.
 */
 static Segment  *BackwardFindFeasible( Segment *end, Segment *start, StraightCostFunc feasibleCostFunc )
 {
	// Find a feasible length piece in this block.
	 Segment	*segment;

	Assert( start->block == end->block );
	 for( segment = end; ; segment = segment->prev )
	 {
		 if( BackwardPieceLength(end, segment) < grcp->minPieceSize )
		 {
			 if( segment == start )
				 return NULL;
			 continue;
		 }
		 if( BackwardPieceLength(end, segment) > grcp->maxPieceSize )
			 break;
		 if( feasibleCostFunc(end, segment) < InfiniteCost &&
				(segment == start || BackwardPieceLength(segment, start) >= grcp->minLeftover) )
			 return segment;
		 if( segment == start )
			 break;
	 }
	 return NULL;
 }

static Segment  *BackwardTuneFeasible( Segment *segment,
												Segment *end, Segment *start,
												StraightCostFunc desirableCostFunc )
{
	// Tune this feasible length to get a more desirable length.
	Segment  *segmentBest, *segmentCur;
	cron_t	costBest, cost;
	
	Assert( segment->block == end->block );
	Assert( segment->block == start->block );
	
	segmentBest = segment;
	costBest = desirableCostFunc(end, segmentBest);
	
	// Try making the piece smaller.
	if( segmentBest != end )
	{
		for( segmentCur = segmentBest->next; ; segmentCur = segmentCur->next )
		{
			if( BackwardPieceLength(end, segmentCur) < grcp->minPieceSize )
				break;
			cost = desirableCostFunc( end, segmentCur );
			if( cost < costBest )
			{
				costBest = cost;
				segmentBest = segmentCur;
			}
			if( segmentCur == end )
				break;
		}
	}
	
	// Try making the piece larger.
	if( segment != start )
	{
		for( segmentCur = segment;; )
		{
			segmentCur = segmentCur->prev;
			if( segmentCur == NULL || segmentCur->block != end->block )
				break;
			
			if( BackwardPieceLength(end, segmentCur) > grcp->maxPieceSize )
				break;
			if( BackwardPieceLength(segmentCur, start) < grcp->minLeftover )
				break;
			cost = desirableCostFunc( end, segmentCur );
			if( cost <= costBest )
			{
				costBest = cost;
				segmentBest = segmentCur;
			}
			if( segmentCur == start )
				break;
		}

		// Try the whole potential piece.
		segmentCur = start;
		if( BackwardPieceLength(end, segmentCur) <= grcp->maxPieceSize &&
			(cost = desirableCostFunc(end, segmentCur)) < costBest )
		{
			costBest = cost;
			segmentBest = segmentCur;
		}
	}

	// Return the best found.
	return segmentBest;
}

static Segment *BackwardLeaveFeasibleLeftover(  Segment *segment,
												Segment *end, Segment *start,
												StraightCostFunc feasibleCostFunc,
												cron_t *pcost ) // return
{
	if( segment == start )
		return segment;
	
	// Check that the leftover piece is large enough already.
	if( (*pcost = feasibleCostFunc(end, segment)) != InfiniteCost &&
		segment->start - start->start >= grcp->minLeftover )
		return segment;
	
	// Case 1: Try to extend this piece to take the rest of the block.
	if( (*pcost = feasibleCostFunc(end, start)) < InfiniteCost )
		return end;
	
	// Case 2: Try to shorten this piece to leave minLeftover at the start.
	for( ; segment != end; segment = segment->next )
		if( segment->start - start->start >= grcp->minLeftover )
			break;
		// Check if it was possible to leave minLeftover.
		if( segment->start - start->start < grcp->minLeftover )
			return NULL;
		// Check if the remaining piece is still feasible.
		if( (*pcost = feasibleCostFunc(end, segment)) >= InfiniteCost )
			return NULL;
		// Otherwise, we successfully shortened the piece.
		return segment;
}

Segment  *StraightFindBackwardCut( Segment *segmentEnd, Segment *segmentStart,
											StraightCostFunc feasiblePieceCoster,
											StraightCostFunc desirablePieceCoster,
											cron_t *pcost ) /* RETURN */
{
	 Segment  *segment;

	 *pcost = InfiniteCost;

	 // Step 1:  Find a feasible straight cut of this piece.
	 segment = BackwardFindFeasible(  segmentEnd, segmentStart, feasiblePieceCoster );
	 if( segment == NULL )
		  return NULL;  // There is no way to feasibly cut the given piece.

	 // Step 2: Tune the feasible piece to its most desired size.
	 GetLookaheadCutRange( segmentEnd, segmentStart );
	 segment = BackwardTuneFeasible( segment,
									 segmentEnd, segmentStart,
									 desirablePieceCoster );

	 // Step 3: Ensure that the cutting this piece will not leave something invalid.
	 segment = BackwardLeaveFeasibleLeftover(	segment,
												segmentEnd, segmentStart,
												feasiblePieceCoster, pcost );

	 return segment;
}

Segment *StraightFindBackward( Segment *segmentEnd, Segment *segmentStart,
												cron_t *pcost )
{
	 return StraightFindBackwardCut( segmentEnd, segmentStart,
						StraightFeasiblePieceCoster, StraightDesirablePieceCoster,
						pcost );

}

/****************************************************************/
/*
 * EvaluateBackwardStrategy computes the number of runs overlapped by
 * the piece left over from completely backward cutting a block.
 */
cron_t  EvaluateBackwardStrategy( Piece *piece )
{
	 cron_t      cost;
	 StraightFindBackward( piece->segmentEnd,
								  piece->segmentStart,
								  &cost );
	 return cost;
}

/****************************************************************/
/*
 * BackwardCutRun actually backward cuts a piece given a valid segment
 * to cut it at.
 */
Run     *BackwardCutRun( Piece *piece, Segment *segment )
{
	 Run         *run;
	 Piece       *pieceNew;

	 Assert( piece );
	 Assert( segment );

	 /* the run is valid without cutting */
	 if( segment == piece->segmentStart )
	 {
		  TM( 1, "BackwardCutRun: new run is the same as the piece.\n" );
		  run = NewRun();
		  PieceRemove( piece );
		  piece->status = Allocated;
		  ListInsert( run->pieceList, piece );
		  return run;
	 }

	 TM( 1, "BackwardCutRun: cutting run.\n" );
	 /*
	  * split this piece into 2 - the valid straight
	  * one we are going to use for the run, and the remainder.
	  */
	 pieceNew = NewPiece();
	 pieceNew->segmentStart = piece->segmentStart;
	 pieceNew->segmentEnd = segment->prev;
	 pieceNew->status = Unexamined;
	 ListInsert( unexaminedPieceList, pieceNew );

	 run = NewRun();

	 /* set up the segments of this run */
	 PieceRemove( piece );
	 piece->segmentStart = segment;
	 piece->status = Allocated;
	 ListInsert( run->pieceList, piece );

	 TExec( 1, PrintPiece(piece) );
	 TExec( 1, PrintPiece(pieceNew) );
	 TM( 1, "********************************************\n" );
	 return run;
}

/****************************************************************/
/*
 * StraightCutBackward takes an invalid piece and cuts a straight
 * run from it (if possible) beginning at the end of the
 * given run.  It returns NULL if a valid cut is not possible.
 */
Run     *StraightCutBackward( Piece *piece )
{
	 Segment     *segment;
	 cron_t      cost;

	 TM( 1, "\nStraightCutBackward: attempting to backward cut:\n" );
	 TExec( 1, PrintPiece(piece) );

	 segment = StraightFindBackward( piece->segmentEnd, piece->segmentStart, &cost );
	 if( segment == NULL )
	 {
		  TM( 1, "StraightCutBackward: cannot find a place to cut.\n" );
		  ChangePieceStatus( piece, Invalid );
		  return NULL;
	 }
	 return BackwardCutRun(piece, segment);
}
