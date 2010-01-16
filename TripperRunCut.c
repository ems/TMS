//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "rc.h"

static void TripperRunCut( Piece *piece )
{
    cron_t      cost;
    Piece       rgPiece[1];

    TM( 1, "\nTripperRunCut: attempting to cut:\n" );
    TExec( 1, PrintPiece(piece) );

    // Attempt to classify the whole piece as a tripper run.
    // Just call the coster on the whole piece and see what
    // happens...
    rgPiece[0].segmentStart = piece->segmentStart;
    rgPiece[0].segmentEnd = piece->segmentEnd;
    cost = PieceCoster( rgPiece, 1 );
    if( cost < InfiniteCost )
    {
        Run     *run = NewRun();

        PieceRemove( piece );
        piece->status = Allocated;
        ListInsert( run->pieceList, piece );
    }
    else
    {
        TM( 1, "TripperRunCut: piece cannot be a tripper.\n" );
        ChangePieceStatus( piece, Invalid );
    }
}

void PerformTripperRunCut(void)
{
    while( unexaminedPieceList != NULL )
        TripperRunCut( unexaminedPieceList );
}
