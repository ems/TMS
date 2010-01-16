//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "rc.h"

static void IllegalRunCut( Piece *piece )
{
    Piece       rgPiece[1];

    TM( 1, "\nIllegalRunCut: attempting to cut:\n" );
    TExec( 1, PrintPiece(piece) );

    rgPiece[0].segmentStart = piece->segmentStart;
    rgPiece[0].segmentEnd = piece->segmentEnd;
    PieceCoster( rgPiece, 1 );
    {
        Run     *run = NewRun();

        PieceRemove( piece );
        piece->status = Allocated;
        ListInsert( run->pieceList, piece );
    }
}

void PerformIllegalRunCut(void)
{
    while( unexaminedPieceList != NULL )
        IllegalRunCut( unexaminedPieceList );
}
