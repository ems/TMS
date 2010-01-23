//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <math.h>
#include <match.H>
extern "C" {
#include <rc.h>
}

#define	AddToList(h,n)	((n)->next = (h), (h) = (n))

extern "C" {

void MatchPieces()
{
    Piece       rgPiece[2];

    // Count the number of pieces to match.
    Piece	*piece;
    int         numPiece = 0;
    ForAllListDo( validPieceList, piece )
        numPiece++;
    if( numPiece < 1 )
        return;

    // Create a match graph large enough to hold the penalty pairs.
    MatchGraph  mg( numPiece * 2L );

    // Initialize the match graph with labels and penalty arcs.
    numPiece = 0;
    ForAllListDo( validPieceList, piece )
    {
		mg.setNodeLabel( numPiece*2L+1L, (unsigned long)piece );
        mg.AddEdge( numPiece*2L+1L, numPiece*2L+2L, 48L*60L*60L );
        numPiece++;
    }

    // Set up the valid match possibilities.
    bool         addedMatchEdge = false;
    for( int fromC = 0; fromC < numPiece; fromC++ )
    {
        for( int toC = fromC+1; toC < numPiece; toC++ )
        {
            // Get the pieces from the node labels.
            Piece *fromPiece = (Piece *)mg.getNodeLabel( fromC*2L+1L );
            Piece *toPiece   = (Piece *)mg.getNodeLabel( toC*2L+1L );

            // Check that we have the pieces the right way round.
            if( fromPiece->segmentStart->start > toPiece->segmentStart->start )
            {
                Piece *t = fromPiece;
                fromPiece = toPiece;
                toPiece = t;
            }

            // Check for overlapping pieces.
            if( fromPiece->segmentEnd->end > toPiece->segmentStart->start )
                continue;

			// Check for pieces that would form single runs together.
			if( !grcp->fTwoPieceCanBeOne &&
				fromPiece->segmentEnd->end == toPiece->segmentStart->start &&
				fromPiece->segmentEnd->block == toPiece->segmentStart->block )
				continue;

            // Check if this piece pair is valid.
            rgPiece[0].segmentStart = fromPiece->segmentStart;
            rgPiece[0].segmentEnd   = fromPiece->segmentEnd;
            rgPiece[1].segmentStart = toPiece->segmentStart;
            rgPiece[1].segmentEnd   = toPiece->segmentEnd;

            const cron_t cost = PieceCoster( rgPiece, 2 );
            if( cost < InfiniteCost )
            {
                // Add the cost edge.
                mg.AddEdge( fromC*2L+1L, toC*2L+1L, cost );

                // Add the penalty relief edge.
                mg.AddEdge( fromC*2L+2L, toC*2L+2L, 0L );

                addedMatchEdge = true;
            }
        }
    }
    if( !addedMatchEdge )
        return;

    // Perform the minimum cost matching.
    mg.Weighted_Match( 0 );

    // Install the matched pieces into the appropriate data structures.
    for( int i = 0; i < numPiece; i++ )
    {
        Piece *fromPiece = (Piece *)mg.getNodeLabel( i*2L+1L );

        const long mate = mg.Mate( i*2L+1L );
        if( (mate & 1L) == 0L )
        {
            // This piece was not matched.
			if( fromPiece->status != Invalid )
				ChangePieceStatus( fromPiece, Invalid );
        }
        else
        {
            // This piece has a match.
            Piece *toPiece = (Piece *)mg.getNodeLabel( mate );
            if( fromPiece->segmentStart->start < toPiece->segmentStart->start &&
                fromPiece->status != Allocated && toPiece->status != Allocated )
            {
                PieceRemove( fromPiece );
                fromPiece->status = Allocated;

                PieceRemove( toPiece );
                toPiece->status = Allocated;

                Run *run = NewRun();
                ListInsert( run->pieceList, toPiece );
                ListInsert( run->pieceList, fromPiece );
            }
        }
    }
}

} // extern "C"
