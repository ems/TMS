//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "rc.h"

long	UnconstrainedPieceCoster( Piece rgPiece[], int numPieces, cron_t *pCost )
{
	 long runType;
    cron_t	weightedCost;
    COSTDef	cost;
    int  i;
    PROPOSEDRUN          pr;   // proposed pieces of the run
    PROPOSEDRUNPIECESDef prTemp;
	Piece				 pTemp;

    // Initialize the pieces of the run.
    Assert( numPieces <= MAXPIECES );
    for( i = 0; i < numPieces; i++ )
    {
        Segment *segmentStart = rgPiece[i].segmentStart;
        Segment *segmentEnd = rgPiece[i].segmentEnd;

        pr.piece[i].fromTime = segmentStart->start;
        pr.piece[i].fromNODESrecordID = segmentStart->ifrom;
        pr.piece[i].fromTRIPSrecordID = m_pRELIEFPOINTS[segmentStart->iFromIndex].TRIPSrecordID;

        pr.piece[i].toTime = segmentEnd->end;
        pr.piece[i].toNODESrecordID = segmentEnd->ito;
        pr.piece[i].toTRIPSrecordID = m_pRELIEFPOINTS[segmentEnd->iToIndex].TRIPSrecordID;
    }

    pr.numPieces = numPieces;
//
//  On a two-piece run, make sure the earlier piece is in the first position
//
    if(numPieces == 2 && pr.piece[0].fromTime > pr.piece[1].fromTime)
    {
      memcpy(&prTemp, &pr.piece[0], sizeof(prTemp));
      memcpy(&pr.piece[0], &pr.piece[1], sizeof(prTemp));
      memcpy(&pr.piece[1], &prTemp, sizeof(prTemp));

	  memcpy(&pTemp, &rgPiece[0], sizeof(Piece) );
	  memcpy(&rgPiece[0], &rgPiece[1], sizeof(Piece) );
	  memcpy(&rgPiece[1], &pTemp, sizeof(Piece) );
    }

	// Check if this is a two piece and the pieces overlap.
	if( numPieces == 2 && rgPiece[0].segmentEnd->end > rgPiece[1].segmentStart->start )
	{
		*pCost = InfiniteCost;
		return UNCLASSIFIED_RUNTYPE;
	}

    // Cost out this run.
    runType = RunCoster(&pr, NO_RUNTYPE, &cost );
    if( runType == UNCLASSIFIED_RUNTYPE )
    {
        TM( 1, "UnconstrainedPieceCoster: runCoster returns impossible run type.\n" );
        *pCost = InfiniteCost;
        return UNCLASSIFIED_RUNTYPE;
    }

    weightedCost = cost.TOTAL.payTime - (numPieces > 1 ? cost.TOTAL.platformTime : 0);
    if(weightedCost < 0L)
      weightedCost = 0L;

    *pCost = weightedCost;
    return runType;
}

cron_t   PieceCoster( Piece rgPiece[], int numPieces )
{
    long    runType;
    int     goodRunType;
    cron_t	cost;

    // Check the run cut time boundaries.
    if( grcp->startRunBeforeTime < rgPiece[0].segmentStart->start )
    {
        TM( 1, "PieceCoster: run violates start before time.\n" );
        return InfiniteCost;
    }
    if( rgPiece[numPieces-1].segmentEnd->end < grcp->endRunAfterTime )
    {
        TM( 1, "PieceCoster: run violates end after time.\n" );
        return InfiniteCost;
    }

    // Cost out this run.
    runType = UnconstrainedPieceCoster( rgPiece, numPieces, &cost );

    // Check if this run is possible at all.
    if( runType == UNCLASSIFIED_RUNTYPE )
    {
        TM( 1, "PieceCoster: runCoster returns impossible run type.\n" );
        return InfiniteCost;
    }

    // Check if the returned type is the type we are looking for.
    if( numPieces == 2 )
    {
        int i = LOWORD( runType );
        int j = HIWORD( runType );
        goodRunType = (CUTPARMS.runtypes[i][j]) &&
              (RUNTYPE[i][j].flags & RTFLAGS_INUSE);
    }
    else
        goodRunType = runType == grcp->runType;

    if( !goodRunType )
    {
        TM(  1, "PieceCoster: runCoster returns incompatible run type.\n");
        TM2( 1, "             (wanted %ld, got %ld)\n",
              grcp->runType, runType );
        return InfiniteCost;
    }

    return cost;
}

/****************************************************************
 *
 * StraightCutDecide attempts to figure out whether to cut
 * a run from the beginning or from the end.
 */
typedef enum
{
    CSForward,
    CSBackward,
    CSMax,
    CSInvalid,
} CutStrategy;

static  cron_t  EvaluateCutStrategy( Piece *piece, CutStrategy cs )
{
    switch( cs )
    {
    case CSForward:
        return EvaluateForwardStrategy(piece);

    case CSBackward:
        return EvaluateBackwardStrategy(piece);
    }

    return InfiniteCost;
}

static  void    ImplementCutStrategy( Piece *piece,
                                      CutStrategy cs )
{
    switch( cs )
    {
    case CSForward:
        TM( 1, "ImplementCutStrategy: cutting forward.\n" );
        StraightCutForward( piece );
        break;

    case CSBackward:
        TM( 1, "ImplementCutStrategy: cutting backward.\n" );
        StraightCutBackward( piece );
        break;

    case CSInvalid:
    default:
        TM( 1, "ImplementCutStrategy: invalidating piece.\n" );
        ChangePieceStatus( piece, Invalid );
        break;
    }
}

void    StraightCutDecide( Piece *piece )
{
    CutStrategy csBest = CSInvalid;             /* best strategy so far */
    cron_t      costBest = InfiniteCost;        /* current best cost */
    cron_t      rgCost[(int)CSMax];
    int         i;

    TM( 1, "\n-------------------------------------------------------------\n" );
    TM( 1, "StraightCutDecide: deciding how to cut this run.\n" );

//
//  Let the user override if he really must...
//
    switch(CUTPARMS.cutDirection)
    {
      case PARAMETERS_FRONT:
        csBest = CSForward;
        break;

      case PARAMETERS_BACK:
        csBest = CSBackward;
        break;

      case PARAMETERS_TMSDECIDES:
        for( i = 0; i < Number(rgCost); i++ )
            rgCost[i] = EvaluateCutStrategy( piece, (CutStrategy)i );

        /* implement the cut strategy with the lowest cost */
        for( i = 0; i < (int)CSMax; i++ )
            if( rgCost[i] < costBest )
            {
                costBest = rgCost[i];
                csBest = (CutStrategy)i;
            }
        break;
    }
    ImplementCutStrategy( piece, csBest );
}

