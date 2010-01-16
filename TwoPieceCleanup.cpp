//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

extern "C" {
#include "rc.h"
} // extern "C"

#pragma warning(disable : 4786 4114 4800)

#include "HashFI.h"
#include <set>

//#define AssertTrue( c ) do { if( !(c) ) { int a = 0, b; b = 10 / a; } } while(0)
#define AssertTrue(c)

typedef	HashFI< Piece *, Piece * > PieceCollection;

#define	cond(a, op, b) (a) op (b) ? 1 : (b) op (a) ? 0

struct decreasingEndTimeCmp
{
	int operator()( const Piece *p1, const Piece *p2 ) const
	{ return cond(p1->segmentEnd->end, >, p2->segmentEnd->end) : p1 > p2; }
};
typedef std::set< Piece *, decreasingEndTimeCmp > DecreasingEndTime;

struct increasingStartTimeCmp
{
	int operator()( const Piece *p1, const Piece *p2 ) const
	{ return cond(p1->segmentStart->start, <, p2->segmentStart->start) : p1 < p2; }
};
typedef std::set< Piece *, increasingStartTimeCmp > IncreasingStartTime;

static	void	TwoPieceEndCleanup()
{
	const int		fSmartLookaheadSave = grcp->fSmartLookahead;
	const cron_t	minPieceSizeSave	= grcp->minPieceSize;
	const cron_t	desPieceSizeSave	= grcp->desPieceSize;
	const cron_t	maxPieceSizeSave	= grcp->maxPieceSize;

	grcp->fSmartLookahead = 0;

	// Find all potential pieces that end after endRunAfterTime.
	register	Piece	*p;
	DecreasingEndTime	decreasingEndTime;

	while( (p = unexaminedPieceList) != NULL )
	{
		if( PieceLength(p) < grcp->minLeftover )
		{
	        ChangePieceStatus( p, Invalid );
			continue;
		}
        ChangePieceStatus( p, Valid );
		decreasingEndTime.insert( p );
	}

	// If there are any valid pieces, try to match them up.
	cron_t	costTmp;
	Piece	rgPiece[2];
	DecreasingEndTime::iterator d;
	while( !((d = decreasingEndTime.begin()) == decreasingEndTime.end()) )
	{
		p = *d; decreasingEndTime.erase( d );
		AssertTrue( p->status == Valid && validPieceList != NULL );

		if( p->segmentEnd->end < grcp->endRunAfterTime )
			break;

		if( PieceLength(p) < grcp->minPieceSize2 )
		{
			ChangePieceStatus( p, Invalid );
			continue;
		}

		// This piece is too large.
		// Cut it from the end if we can.
		if( PieceLength(p) > grcp->maxPieceSize2 )
		{
			grcp->minPieceSize = grcp->minPieceSize2;
			grcp->desPieceSize = grcp->desPieceSize2;
			grcp->maxPieceSize = grcp->maxPieceSize2;
			Segment *s = StraightFindBackwardSplit( p->segmentEnd, p->segmentStart, &costTmp );
			if( s == NULL )
			{
				ChangePieceStatus( p, Invalid );
				continue;
			}
			else
			{
				Piece *pNew = NewPiece();

				// Set up the segments of this piece.
				pNew->segmentStart = p->segmentStart;
				pNew->segmentEnd = s->prev;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				p->segmentStart = s;
				AssertTrue( PieceLength(p) >= grcp->minPieceSize2 );
				if( PieceLength(pNew) < grcp->minPieceSize2 && PieceLength(pNew) < grcp->minPieceSize1 )
					pNew->status = Invalid;
				else
				{
					pNew->status = Valid;
					decreasingEndTime.insert( pNew );
				}
				PieceInsert( pNew );
			}
		}

		// This is a potentially valid piece.  Try to find a mate.
		// Attempt to find a match to an existing piece.
		Piece	pSearch;
		Segment	last;
		pSearch.segmentStart = NULL;
		pSearch.segmentEnd = &last;

		DecreasingEndTime::iterator d;
		Piece	*pBest = NULL;
		cron_t	cost, costBest;
		pSearch.segmentEnd->start = p->segmentStart->start;
		pSearch.segmentEnd->end   = p->segmentStart->start;
		for( d = decreasingEndTime.lower_bound(&pSearch); !(d == decreasingEndTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( p->segmentEnd->end - pCur->segmentEnd->end > grcp->maxSpread )
				break;

			// Check if this piece pair is valid.
			rgPiece[0].segmentStart = pCur->segmentStart;
			rgPiece[0].segmentEnd   = pCur->segmentEnd;
			rgPiece[1].segmentStart = p->segmentStart;
			rgPiece[1].segmentEnd   = p->segmentEnd;

			cost = PieceCoster( rgPiece, 2 );
			if( cost >= InfiniteCost )
				continue;

			if( pBest == NULL || cost < costBest )
			{
				costBest = cost;
				pBest = pCur;
			}
		}

		if( pBest != NULL )
		{
			// We found a match!
			PieceRemove( p ); p->status = Allocated;
			PieceRemove( pBest); pBest->status = Allocated;
			decreasingEndTime.erase( pBest );

			Run *run = NewRun();
            ListInsert( run->pieceList, pBest );
            ListInsert( run->pieceList, p );
			continue;
		}

		// We could not find a suitable piece to match with.
		// Try to find a match to a piece that we cut from the beginning or the end.
		// Adjust the desired piece size for the cut.
		grcp->desPieceSize = (grcp->desPieceSize1 + grcp->desPieceSize2) - PieceLength(p);
		if( grcp->desPieceSize < grcp->minPieceSize1 )
			grcp->desPieceSize = grcp->minPieceSize1;
		else if( grcp->desPieceSize > grcp->maxPieceSize1 )
			grcp->desPieceSize = grcp->maxPieceSize1;
		grcp->minPieceSize = grcp->minPieceSize1;
		grcp->maxPieceSize = grcp->maxPieceSize1;

		Segment	*segmentBest = NULL;
		int		backwardBest = 1;
		pSearch.segmentEnd->start = p->segmentEnd->end;
		pSearch.segmentEnd->end   = p->segmentEnd->end;
		for( d = decreasingEndTime.lower_bound(&pSearch); !(d == decreasingEndTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( p->segmentEnd->end - pCur->segmentEnd->end > grcp->maxSpread )
				break;

			Segment *s = StraightFindBackwardSplit( pCur->segmentEnd, pCur->segmentStart, &costTmp );
			if( s != NULL )
			{
				// Check if this piece pair is valid.
				rgPiece[0].segmentStart = s;
				rgPiece[0].segmentEnd   = pCur->segmentEnd;
				rgPiece[1].segmentStart = p->segmentStart;
				rgPiece[1].segmentEnd   = p->segmentEnd;

				cost = PieceCoster( rgPiece, 2 );
				if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
				{
					segmentBest = s;
					backwardBest = 1;
					costBest = cost;
					pBest = pCur;
				}
			}

			s = StraightFindForwardSplit( pCur->segmentStart, pCur->segmentEnd, &costTmp );
			if( s != NULL )
			{
				// Check if this piece pair is valid.
				rgPiece[0].segmentStart = pCur->segmentStart;
				rgPiece[0].segmentEnd   = s;
				rgPiece[1].segmentStart = p->segmentStart;
				rgPiece[1].segmentEnd   = p->segmentEnd;

				const cron_t cost = PieceCoster( rgPiece, 2 );
				if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
				{
					segmentBest = s;
					backwardBest = 0;
					costBest = cost;
					pBest = pCur;
				}
			}
		}

		if( pBest != NULL )
		{
			// We found a match!
			// We need to split this piece.
			Piece *pNew = NewPiece();
			pNew->status = Allocated;

			if( backwardBest )
			{
				// Set up the segments of this piece.
				pNew->segmentStart = segmentBest;
				pNew->segmentEnd = pBest->segmentEnd;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				decreasingEndTime.erase( pBest );
				pBest->segmentEnd = segmentBest->prev;
				AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
				if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
					ChangePieceStatus( pBest, Invalid );
				else
					decreasingEndTime.insert( pBest );
			}
			else
			{
				// Set up the segments of this piece.
				pNew->segmentStart = pBest->segmentStart;
				pNew->segmentEnd = segmentBest;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				pBest->segmentStart = segmentBest->next;
				AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
				if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
				{
					ChangePieceStatus( pBest, Invalid );
					decreasingEndTime.erase( pBest );
				}
			}

			PieceRemove( p ); p->status = Allocated;

			Run *run = NewRun();
            ListInsert( run->pieceList, p );
            ListInsert( run->pieceList, pNew );
			continue;
		}

		// We still could not find a suitable piece to match with.
		// Try to find a match to a piece that we cut from somewhere in the middle.
		Segment	*sEnd, *segmentEndBest = NULL;
		pSearch.segmentEnd->start = p->segmentEnd->end;
		pSearch.segmentEnd->end   = p->segmentEnd->end;
		for( d = decreasingEndTime.lower_bound(&pSearch); !(d == decreasingEndTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( p->segmentEnd->end - pCur->segmentEnd->end > grcp->maxSpread )
				break;

			// Ignore possibilities that have no middle.
			if( pCur->segmentStart == pCur->segmentEnd )
				continue;

#ifdef FIXLATER
			// Need to look at this more - segmentEnd and segmentStart should be in the same list.
			// There should be no way for sEnd to become NULL without first becoming equal to pCur->segmentStart.
			//
//#define CheckValidPiece(pCur)
			do
			{
				Segment	*s;
				for( s = pCur->segmentStart; s; s = s->next )
					if( s == pCur->segmentEnd )
						break;
				if( s != pCur->segmentEnd )
					{ int xxx = 0, yyy = 10, zzz; zzz = yyy / xxx; }
				for( s = pCur->segmentEnd; s; s = s->prev )
					if( s == pCur->segmentStart )
						break;
				if( s != pCur->segmentStart )
					{ int xxx = 0, yyy = 10, zzz; zzz = yyy / xxx; }
			} while(0);

//			CheckValidPiece( pCur );
#endif

			// First, find the first relief point to start from that does not overlap.
			for( sEnd = pCur->segmentEnd; sEnd != pCur->segmentStart; sEnd = sEnd->prev )
			{
				if( !sEnd )
					break;
				if( pCur->segmentEnd->end - sEnd->end >= grcp->minLeftover &&
					sEnd->end <= p->segmentStart->start )
					break;
			}
			if( !sEnd )
				continue;

			// Check all possible relief points to see if we can split in the middle.
			while( BackwardPieceLength(sEnd, pCur->segmentStart) >=
					grcp->minLeftover + grcp->minPieceSize1 )
			{
				Segment *s = StraightFindBackwardSplit( sEnd, pCur->segmentStart, &costTmp );
				if( s != NULL )
				{
					// Check if this piece pair is valid.
					rgPiece[0].segmentStart = s;
					rgPiece[0].segmentEnd   = sEnd;
					rgPiece[1].segmentStart = p->segmentStart;
					rgPiece[1].segmentEnd   = p->segmentEnd;

					const cron_t cost = PieceCoster( rgPiece, 2 );
					if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
					{
						segmentEndBest = sEnd;
						segmentBest = s;
						costBest = cost;
						pBest = pCur;
					}
				}

				if( sEnd == pCur->segmentStart )
					break;
				sEnd = sEnd->prev;
			} 
		}

		if( pBest != NULL )
		{
			// We found a match!
			// We need to split this piece from the middle.

			// Set up the segments of this middle piece.
			Piece *pNew = NewPiece();
			pNew->status = Allocated;
			pNew->segmentStart = segmentBest;
			pNew->segmentEnd = segmentEndBest;
			AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

			// Set up the segments of the leftover piece.
			Piece *pLeftover = NewPiece();
			pLeftover->segmentStart = pBest->segmentStart;
			pLeftover->segmentEnd = segmentBest->prev;
			AssertTrue( PieceLength(pLeftover) >= grcp->minLeftover );
			if( PieceLength(pLeftover) < grcp->minPieceSize1 && PieceLength(pLeftover) < grcp->minPieceSize2 )
				pLeftover->status = Invalid;
			else
			{
				pLeftover->status = Valid;
				decreasingEndTime.insert( pLeftover );
			}
			PieceInsert( pLeftover );

			// Modify the segments of the original piece.
			pBest->segmentStart = segmentEndBest->next;
			AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
			if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
			{
				decreasingEndTime.erase( pBest );
				ChangePieceStatus( pBest, Invalid );
			}

			PieceRemove( p ); p->status = Allocated;

			Run *run = NewRun();
            ListInsert( run->pieceList, p );
            ListInsert( run->pieceList, pNew );
		}
	}

	// Cleanup
	grcp->minPieceSize		= minPieceSizeSave;
	grcp->desPieceSize		= desPieceSizeSave;
	grcp->maxPieceSize		= maxPieceSizeSave;
	grcp->fSmartLookahead	= fSmartLookaheadSave;
}

static	void	TwoPieceStartCleanup()
{
	const int		fSmartLookaheadSave = grcp->fSmartLookahead;
	const cron_t	minPieceSizeSave	= grcp->minPieceSize;
	const cron_t	desPieceSizeSave	= grcp->desPieceSize;
	const cron_t	maxPieceSizeSave	= grcp->maxPieceSize;

	grcp->fSmartLookahead = 0;

	// Find all potential pieces that end after endRunAfterTime.
	register	Piece	*p;
	IncreasingStartTime	increasingStartTime;

	while( (p = unexaminedPieceList) != NULL )
	{
		if( PieceLength(p) < grcp->minLeftover )
		{
	        ChangePieceStatus( p, Invalid );
			continue;
		}
        ChangePieceStatus( p, Valid );
		increasingStartTime.insert( p );
	}

	// If there are any valid pieces, try to match them up.
	cron_t	costTmp;
	Piece	rgPiece[2];
	IncreasingStartTime::iterator d;
	while( !((d = increasingStartTime.begin()) == increasingStartTime.end()) )
	{
		p = *d; increasingStartTime.erase( d );

		if( p->segmentStart->block == 201 )
		{
			int a = 10;
		}

		if( p->segmentStart->start > grcp->startRunBeforeTime )
			break;

		if( PieceLength(p) < grcp->minPieceSize1 )
		{
			ChangePieceStatus( p, Invalid );
			continue;
		}

		// This piece is too large.
		// Cut it from the beginning if we can.
		if( PieceLength(p) > grcp->maxPieceSize1 )
		{
			grcp->minPieceSize = grcp->minPieceSize1;
			grcp->desPieceSize = grcp->desPieceSize1;
			grcp->maxPieceSize = grcp->maxPieceSize1;
			Segment *s = StraightFindForwardSplit( p->segmentStart, p->segmentEnd, &costTmp );
			if( s == NULL )
			{
				ChangePieceStatus( p, Invalid );
				continue;
			}
			else
			{
				Piece *pNew = NewPiece();

				// Set up the segments of this piece.
				pNew->segmentStart = s->next;
				pNew->segmentEnd = p->segmentEnd;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				p->segmentEnd = s;
				AssertTrue( PieceLength(p) >= grcp->minPieceSize1 );
				if( PieceLength(pNew) < grcp->minPieceSize1 && PieceLength(pNew) < grcp->minPieceSize2 )
					pNew->status = Invalid;
				else
				{
					pNew->status = Valid;
					increasingStartTime.insert( pNew );
				}
				PieceInsert( pNew );
			}
		}

		// This is a potentially valid piece.  Try to find a mate.
		// Attempt to find a match to an existing piece.
		Piece	pSearch;
		Segment	first;
		pSearch.segmentStart = &first;
		pSearch.segmentEnd = NULL;

		IncreasingStartTime::iterator d;
		Piece	*pBest = NULL;
		cron_t	cost, costBest;
		pSearch.segmentStart->start = p->segmentEnd->end;
		pSearch.segmentStart->end   = p->segmentEnd->end;
		for( d = increasingStartTime.lower_bound(&pSearch); !(d == increasingStartTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( pCur->segmentStart->start - p->segmentStart->start > grcp->maxSpread )
				break;

			// Check if this piece pair is valid.
			rgPiece[0].segmentStart = p->segmentStart;
			rgPiece[0].segmentEnd   = p->segmentEnd;
			rgPiece[1].segmentStart = pCur->segmentStart;
			rgPiece[1].segmentEnd   = pCur->segmentEnd;

			cost = PieceCoster( rgPiece, 2 );
			if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
			{
				costBest = cost;
				pBest = pCur;
			}
		}

		if( pBest != NULL )
		{
			// We found a match!
			PieceRemove( p ); p->status = Allocated;
			PieceRemove( pBest); pBest->status = Allocated;
			increasingStartTime.erase( pBest );

			Run *run = NewRun();
            ListInsert( run->pieceList, p );
            ListInsert( run->pieceList, pBest );
			continue;
		}

		// We could not find a suitable piece to match with.
		// Try to find a match to a piece that we cut.
		// Adjust the desired piece size for the cut.
		grcp->desPieceSize = (grcp->desPieceSize1 + grcp->desPieceSize2) - PieceLength(p);
		if( grcp->desPieceSize < grcp->minPieceSize2 )
			grcp->desPieceSize = grcp->minPieceSize2;
		else if( grcp->desPieceSize > grcp->maxPieceSize2 )
			grcp->desPieceSize = grcp->maxPieceSize2;
		grcp->minPieceSize = grcp->minPieceSize2;
		grcp->maxPieceSize = grcp->maxPieceSize2;

		Segment	*segmentBest = NULL;
		int		backwardBest = 1;
		pSearch.segmentStart->start = p->segmentStart->start;
		pSearch.segmentStart->end   = p->segmentStart->start;
		for( d = increasingStartTime.lower_bound(&pSearch); !(d == increasingStartTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( pCur->segmentStart->start - p->segmentStart->start > grcp->maxSpread )
				break;

			Segment *s = StraightFindBackwardSplit( pCur->segmentEnd, pCur->segmentStart, &costTmp );
			if( s != NULL )
			{
				// Check if this piece pair is valid.
				rgPiece[0].segmentStart = p->segmentStart;
				rgPiece[0].segmentEnd   = p->segmentEnd;
				rgPiece[1].segmentStart = s;
				rgPiece[1].segmentEnd   = pCur->segmentEnd;

				cost = PieceCoster( rgPiece, 2 );
				if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
				{
					segmentBest = s;
					backwardBest = 1;
					costBest = cost;
					pBest = pCur;
				}
			}

			s = StraightFindForwardSplit( pCur->segmentStart, pCur->segmentEnd, &costTmp );
			if( s != NULL )
			{
				// Check if this piece pair is valid.
				rgPiece[0].segmentStart = p->segmentStart;
				rgPiece[0].segmentEnd   = p->segmentEnd;
				rgPiece[1].segmentStart = pCur->segmentStart;
				rgPiece[1].segmentEnd   = s;

				const cron_t cost = PieceCoster( rgPiece, 2 );
				if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
				{
					segmentBest = s;
					backwardBest = 0;
					costBest = cost;
					pBest = pCur;
				}
			}
		}

		if( pBest != NULL )
		{
			// We found a match!
			// We need to split this piece.
			Piece *pNew = NewPiece();
			pNew->status = Allocated;

			if( backwardBest )
			{
				// Set up the segments of this piece.
				pNew->segmentStart = segmentBest;
				pNew->segmentEnd = pBest->segmentEnd;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				pBest->segmentEnd = segmentBest->prev;
				AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
				if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
				{
					ChangePieceStatus( pBest, Invalid );
					increasingStartTime.erase( pBest );
				}
			}
			else
			{
				// Set up the segments of this piece.
				pNew->segmentStart = pBest->segmentStart;
				pNew->segmentEnd = segmentBest;
				AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

				// Update the given piece to reflect the split.
				increasingStartTime.erase( pBest );
				pBest->segmentStart = segmentBest->next;
				AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
				if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
					ChangePieceStatus( pBest, Invalid );
				else
					increasingStartTime.insert( pBest );
			}

			PieceRemove( p ); p->status = Allocated;

			Run *run = NewRun();
            ListInsert( run->pieceList, pNew );
            ListInsert( run->pieceList, p );
			continue;
		}
	
		// We still could not find a suitable piece to match with.
		// Try to find a match to a piece that we cut from somewhere in the middle.
		Segment	*sStart, *segmentStartBest = NULL;
		pSearch.segmentStart->start = p->segmentStart->start;
		pSearch.segmentStart->end   = p->segmentStart->start;
		for( d = increasingStartTime.lower_bound(&pSearch); !(d == increasingStartTime.end()); ++d )
		{
			Piece *pCur = *d;
			if( pCur->segmentStart->start - p->segmentStart->start > grcp->maxSpread )
				break;

			// Ignore possibilities that have no middle.
			if( pCur->segmentStart == pCur->segmentEnd )
				continue;

			// First, find the first relief point to start from that does not overlap.
			for( sStart = pCur->segmentStart; sStart != pCur->segmentEnd; sStart = sStart->next )
				if( sStart->start - pCur->segmentStart->start >= grcp->minLeftover &&
					sStart->start >= p->segmentEnd->end )
					break;

			// Check all possible relief points to see if we can split in the middle.
			while( ForwardPieceLength(sStart, pCur->segmentEnd) >=
					grcp->minLeftover + grcp->minPieceSize2 )
			{
				Segment *s = StraightFindForwardSplit( sStart, pCur->segmentEnd, &costTmp );
				if( s != NULL )
				{
					// Check if this piece pair is valid.
					rgPiece[0].segmentStart = p->segmentStart;
					rgPiece[0].segmentEnd   = p->segmentEnd;
					rgPiece[1].segmentStart = sStart;
					rgPiece[1].segmentEnd   = s;

					const cron_t cost = PieceCoster( rgPiece, 2 );
					if( cost < InfiniteCost && (pBest == NULL || cost < costBest) )
					{
						segmentStartBest = sStart;
						segmentBest = s;
						costBest = cost;
						pBest = pCur;
					}
				}

				if( sStart == pCur->segmentEnd )
					break;
				sStart = sStart->next;
			} 
		}

		if( pBest != NULL )
		{
			// We found a match!
			// We need to split this piece from the middle.
			// Set up the segments of the middle piece.
			Piece *pNew = NewPiece();
			pNew->status = Allocated;
			pNew->segmentStart = segmentStartBest;
			pNew->segmentEnd = segmentBest;
			AssertTrue( PieceLength(pNew) >= grcp->minLeftover );

			// Set up the segments of the leftover piece.
			Piece *pLeftover = NewPiece();
			pLeftover->segmentStart = segmentBest->next;
			pLeftover->segmentEnd = pBest->segmentEnd;
			AssertTrue( PieceLength(pLeftover) >= grcp->minLeftover );
			if( PieceLength(pLeftover) < grcp->minPieceSize1 && PieceLength(pLeftover) < grcp->minPieceSize2 )
				pLeftover->status = Invalid;
			else
			{
				pLeftover->status = Valid;
				increasingStartTime.insert( pLeftover );
			}
			PieceInsert( pLeftover );

			// Modify the segments of the original piece.
			pBest->segmentEnd = segmentStartBest->prev;
			AssertTrue( PieceLength(pBest) >= grcp->minLeftover );
			if( PieceLength(pBest) < grcp->minPieceSize1 && PieceLength(pBest) < grcp->minPieceSize2 )
			{
				increasingStartTime.erase( pBest );
				ChangePieceStatus( pBest, Invalid );
			}

			PieceRemove( p ); p->status = Allocated;

			Run *run = NewRun();
            ListInsert( run->pieceList, pNew );
            ListInsert( run->pieceList, p );
		}
	}

	// Cleanup
	grcp->minPieceSize		= minPieceSizeSave;
	grcp->desPieceSize		= desPieceSizeSave;
	grcp->maxPieceSize		= maxPieceSizeSave;
	grcp->fSmartLookahead	= fSmartLookaheadSave;
}

extern "C" {
void	PerformTwoPieceCleanup()
{
	if( grcp->startRunBeforeTime < Hour(24)-1 )
		TwoPieceStartCleanup();
	else
		TwoPieceEndCleanup();
}
} // extern "C"