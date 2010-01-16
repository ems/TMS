//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "rc.h"

// This is the exact same logic used in strcutf.c in StraightFindCutForward.
// However, we call a local cost function.

static cron_t FeasiblePieceCoster( Segment *start, Segment *end )
{
	if( start->start > end->start )
	{
		Segment *t = start; start = end; end = t;
	}
	if( ForwardPieceLength(start, end) < grcp->minPieceSize ||
		 ForwardPieceLength(start, end) > grcp->maxPieceSize )
		 return InfiniteCost;

	// Penalize on the difference from the global desired piece size.
	return Abs(ForwardPieceLength(start, end) - grcp->desPieceSize);
}

static cron_t DesirablePieceCoster( Segment *start, Segment *end )
{
	cron_t	feasibleCost = FeasiblePieceCoster( start, end );
	if( !grcp->fSmartLookahead || feasibleCost >= InfiniteCost )
		 return feasibleCost;

	if( start->start > end->start )
	{
		Segment *t = start; start = end; end = t;
	}
	// Penalize on the difference from the local desired piece size.
	return Abs( lrcp->desPieceSize - ForwardPieceLength(start, end) );
}

Segment	*StraightFindForwardSplit( Segment *start, Segment *end, cron_t *pCost )
{
	 return StraightFindForwardCut( start, end,
										FeasiblePieceCoster, DesirablePieceCoster,
										pCost );
}

Segment	*StraightFindBackwardSplit( Segment *start, Segment *end, cron_t *pCost )
{
	 return StraightFindBackwardCut( start, end,
										FeasiblePieceCoster, DesirablePieceCoster,
										pCost );
}

void SplitPiece( Piece *piece )
{
	 Segment     *segment;
	 cron_t		 cost;

	 if( piece == NULL )
		  return;

	 // Attempt to split this piece into chunks roughly the size of
	 // 1/2 of a 2-piece run..
	 segment = StraightFindForwardCut( piece->segmentStart, piece->segmentEnd,
												FeasiblePieceCoster, DesirablePieceCoster,
												&cost );

	 // Classify the piece according to how it was cut.
	 if( segment == NULL )
	 {
		  // This piece cannot be cut further.
		  ChangePieceStatus( piece, Invalid );
	 }
	 else if( segment == piece->segmentEnd )
	 {
		  // This piece is fine the way it is.
		  ChangePieceStatus( piece, Valid );
	 }
	 else
	 {
		  // We need to split this piece.
		  Piece *pieceNew = NewPiece();
		  pieceNew->status = Valid;
		  PieceInsert( pieceNew );

		  // Set up the segments of this piece.
		  pieceNew->segmentStart = piece->segmentStart;
		  pieceNew->segmentEnd = segment;

		  // Update the given piece to reflect the split.
		  // Leave the leftover piece on the unexamined piece list.
		  piece->segmentStart = segment->next;
	 }
}

