//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "rc.h"

/****************************************************************/
/*
 * StraightFindForward takes a starting segment and an ending
 * segment and attempts to find a place to cut forward between
 * the segments.
 */

cron_t StraightFeasiblePieceCoster( Segment *start, Segment *end )
{
	Piece	rgPiece[1];

	if( start->start > end->start )
	{
		Segment *t = start;
		start = end;
		end = t;
	}

	if( ForwardPieceLength(start, end) < grcp->minPieceSize ||
		 ForwardPieceLength(start, end) > grcp->maxPieceSize )
		 return InfiniteCost;
	rgPiece[0].segmentStart = start;
	rgPiece[0].segmentEnd = end;
	return PieceCoster( rgPiece, 1 );
}

cron_t StraightDesirablePieceCoster( Segment *start, Segment *end )
{
	cron_t	feasibleCost = StraightFeasiblePieceCoster( start, end );
	if( feasibleCost >= InfiniteCost )
		 return feasibleCost;

  // Unconditionally check how close we are to the desired piece size.
  // If grcp->fSmartLookahead is FALSE, this will be the same as
  // checking against the desired piece size.
  // If grcp->fSmartLookahead is TRUE, this will check against the
  // best adjusted piece size so that full pieces will be cut.
	if( start->start > end->start )
	{
		Segment *t = start;
		start = end;
		end = t;
	}

	// Penalize on the distance from the local desired piece size.
	return Abs( lrcp->desPieceSize - ForwardPieceLength(start, end) );
}

static Segment  *ForwardFindFeasible( Segment *start, Segment *end,
													StraightCostFunc feasibleCostFunc )
{
	// Find a feasible length piece in this block.
	Segment	*segment;
	for( segment = start;; segment = segment->next )
	{
		if( ForwardPieceLength(start, segment) < grcp->minPieceSize )
		{
			if( segment == end )
				return NULL;
			continue;
		}
		if( ForwardPieceLength(start, segment) > grcp->maxPieceSize )
			 return NULL;

		if( feasibleCostFunc(start, segment) < InfiniteCost &&
			(segment == end || ForwardPieceLength(segment, end) >= grcp->minLeftover))
			return segment;

		if( segment == end )
			 break;
	}
	return NULL;
}

static Segment  *ForwardTuneFeasible( Segment *segment,
												Segment *start, Segment *end,
												StraightCostFunc desirableCostFunc )
{
	// Tune this feasible length to get a more desirable length.
	Segment  *segmentBest, *segmentCur;
	cron_t	costBest, cost;

	Assert( segment->block == end->block );
	Assert( segment->block == start->block );

	segmentBest = segment;
	costBest = desirableCostFunc(start, segmentBest);

	// Try making the piece smaller.
	if( segment != start )
	{
		for( segmentCur = segment;; )
		{
			segmentCur = segmentCur->prev;

			if( ForwardPieceLength(start, segmentCur) < grcp->minPieceSize )
				break;
			cost = desirableCostFunc( start, segmentCur );
			if( cost < costBest )
			{
				costBest = cost;
				segmentBest = segmentCur;
			}
			if( segmentCur == start )
				break;
		}
	}

	// Try making the piece larger.
	if( segment != end )
	{
		for( segmentCur = segment;; )
		{
			segmentCur = segmentCur->next;
			
			if( ForwardPieceLength(start, segmentCur) > grcp->maxPieceSize )
				break;
			if( ForwardPieceLength(segmentCur, end) < grcp->minLeftover )
				break;
			cost = desirableCostFunc( start, segmentCur );
			if( cost <= costBest )
			{
				costBest = cost;
				segmentBest = segmentCur;
			}
			if( segmentCur == end )
				break;
		}

		// Try the whole potential piece.
		segmentCur = end;
		if( ForwardPieceLength(start, segmentCur) <= grcp->maxPieceSize &&
			(cost = desirableCostFunc(start, segmentCur)) < costBest )
		{
			costBest = cost;
			segmentBest = segmentCur;
		}
	}

	// Return the best found.
	return segmentBest;
}

static Segment *ForwardLeaveFeasibleLeftover(	Segment *segment,
												Segment *start, Segment *end,
												StraightCostFunc feasibleCostFunc,
												cron_t *pcost ) // return
{
	if( segment == end )
		return segment;
	
	// Check that the leftover piece is large enough already.
	if( (*pcost = feasibleCostFunc(start, segment)) != InfiniteCost &&
		end->end - segment->end >= grcp->minLeftover )
		return segment;
	
	// Case 1: Try to extend this piece to take the rest of the block.
	if( (*pcost = feasibleCostFunc(start, end)) < InfiniteCost )
		return end;
	
	// Case 2: Try to shorten this piece to leave minLeftover at the end.
	for( ; segment != start; segment = segment->prev )
		if( end->end - segment->end >= grcp->minLeftover )
			break;
		// Check if it was possible to leave minLeftover.
		if( end->end - segment->end < grcp->minLeftover )
			return NULL;
		// Check if the remaining piece is still feasible.
		if( (*pcost = feasibleCostFunc(start, segment)) >= InfiniteCost )
			return NULL;
		// Otherwise, we successfully shortened the piece.
		return segment;
}

int     GetLookaheadCutRange( Segment *segmentStart,
										Segment *segmentEnd )
{
	 // This function sets the values in lrcp (LocalRunCutParameters).
	 //
	 // Try to look ahead a bit to cut the whole piece as straights.
	 // This involves dividing the whole length of the piece
	 // by the desired piece size and choosing the "closest" number
	 // of straights.

	 // Using this projected number of straights, change the min
	 // and max piecesize constraints so that the straight run cutter
	 // will attempt to get a run (longer or shorter) than the
	 // usual length.

	 // This "sets us up" for the next straight runcuts - we have
	 // "looked ahead" so that we have cut at a place that is likely
	 // to allow us to cut full straights in the future.

	 // Only return TRUE if a lookahead cut will work and is
	 // different from the normal cut ranges.

	 cron_t lenTotal;
	 int straights;

	 // Handle reversed arguments.
	 if( segmentStart->start > segmentEnd->start )
	 {
		  Segment *s = segmentStart;
		  segmentStart = segmentEnd;
		  segmentEnd = s;
	 }

	 lrcp->minPieceSize = grcp->minPieceSize;
	 lrcp->desPieceSize = grcp->desPieceSize;
	 lrcp->maxPieceSize = grcp->maxPieceSize;

	 // If the user prefers not to use the lookahead feature, just
	 // use the defaults.
	 if( !grcp->fSmartLookahead )
		  return FALSE;

	 lenTotal = ForwardPieceLength(segmentStart, segmentEnd);
	 straights = (int)((double)lenTotal / (double)grcp->desPieceSize + 0.5);
	 if( straights > 1 )
	 {
		  cron_t lenMax = lenTotal - (straights-1) * grcp->minPieceSize;
		  cron_t lenMin = lenTotal - (straights-1) * grcp->maxPieceSize;
		  cron_t lenDes = (cron_t)((double)lenTotal / (double)straights);

		  lenMin = Max(lenMin, grcp->minPieceSize);
		  lenMax = Min(lenMax, grcp->maxPieceSize);
		  lenDes = Max(lenDes, lenMin);
		  lenDes = Min(lenDes, lenMax);

		  // Check if the ranges are valid.
		  if( lenMin == grcp->maxPieceSize || lenMax == grcp->minPieceSize ||
				lenMin > lenMax || lenDes < lenMin || lenDes > lenMax )
				return FALSE;

		  lrcp->minPieceSize = lenMin;
		  lrcp->desPieceSize = lenDes;
		  lrcp->maxPieceSize = lenMax;
		  return TRUE;
	 }

	 return FALSE;
}

Segment  *StraightFindForwardCut( Segment *segmentStart, Segment *segmentEnd,
											StraightCostFunc feasiblePieceCoster,
											StraightCostFunc desirablePieceCoster,
											cron_t *pcost ) /* RETURN */
{
	 Segment  *segment;

	 *pcost = InfiniteCost;

	 // Step 1:  Find a feasible straight cut of this piece.
	 segment = ForwardFindFeasible(  segmentStart, segmentEnd,
												feasiblePieceCoster );
	 if( segment == NULL )
		  return NULL;  // There is no way to feasibly cut the given piece.

	 // Step 2: Tune the feasible piece to its most desired size.
	 GetLookaheadCutRange( segmentStart, segmentEnd );
	 segment = ForwardTuneFeasible(  segment,
												segmentStart, segmentEnd,
												desirablePieceCoster );

	 // Step 3: Ensure that the cutting this piece will not leave something invalid.
	 segment = ForwardLeaveFeasibleLeftover(  segment,
															segmentStart, segmentEnd,
															feasiblePieceCoster, pcost );

	 return segment;
}

Segment *StraightFindForward( Segment *segmentStart, Segment *segmentEnd,
												cron_t *pcost )
{
	 return StraightFindForwardCut( segmentStart, segmentEnd,
						StraightFeasiblePieceCoster, StraightDesirablePieceCoster,
						pcost );

}
/****************************************************************/
/*
 * EvaluateForwardStrategy computes the number of runs overlapped by
 * the piece left over from completely forward cutting a block.
 */
cron_t  EvaluateForwardStrategy( Piece *piece )
{
	 cron_t      cost;
	 StraightFindForward( piece->segmentStart,
								 piece->segmentEnd,
								 &cost );
	 return cost;
}

/****************************************************************/
/*
 * ForwardCutRun actually forward cuts a run given a valid segment
 * to cut it at.
 */
Run     *ForwardCutRun( Piece *piece, Segment *segment )
{
	 Run         *run;
	 Piece       *pieceNew;

	 Assert( piece );
	 Assert( segment );

	 /* the run is valid without cutting */
	 if( segment == piece->segmentEnd )
	 {
		  TM( 1, "ForwardCutRun: new run is the same as the old run.\n" );
		  run = NewRun();
		  PieceRemove( piece );
		  piece->status = Allocated;
		  ListInsert( run->pieceList, piece );
		  return run;
	 }

	 TM( 1, "ForwardCutRun: cutting run.\n" );
	 /*
	  * split this piece into 2 runs - the valid straight
	  * one we are going to use for the run, and the remainder.
	  */
	 pieceNew = NewPiece();

	 pieceNew->segmentStart = segment->next;
	 pieceNew->segmentEnd = piece->segmentEnd;
	 pieceNew->status = Unexamined;
	 ListInsert( unexaminedPieceList, pieceNew );

	 run = NewRun();

	 /* set up the segments of this run */
	 PieceRemove( piece );
	 piece->segmentEnd = segment;
	 piece->status = Allocated;
	 ListInsert( run->pieceList, piece );

	 TExec( 1, PrintPiece(piece) );
	 TExec( 1, PrintPiece(pieceNew) );
	 TM( 1, "********************************************\n" );
	 return run;
}

/****************************************************************/
/*
 * StraightCutForward takes a piece and cuts a straight
 * run from it (if possible) beginning at the start of the
 * given piece.  It returns NULL if a valid cut is not possible.
 */
Run     *StraightCutForward( Piece *piece )
{
	 Segment     *segment;
	 cron_t      cost;

	 TM( 1, "\nStraightCutForward: attempting to forward cut:\n" );
	 TExec( 1, PrintPiece(piece) );

	 segment = StraightFindForward( piece->segmentStart, piece->segmentEnd, &cost );
	 if( segment == NULL )
	 {
		  TM( 1, "StraightCutForward: cannot find a place to cut.\n" );
		  ChangePieceStatus( piece, Invalid );
		  return NULL;
	 }
	 return ForwardCutRun(piece, segment);
}
