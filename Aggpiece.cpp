//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <lpgen.H>
#include <stdio.h>
extern "C" {
#include "TMSHeader.h"
}

aggPiece::aggPiece()
{
    init();
}

aggPiece::aggPiece( ReliefPoint *rp )
{
    init();
    addRelief( rp );
}

aggPiece::aggPiece( const aggPiece &p1, const aggPiece &p2 )
{
    init();
    append( p1 );
    append( p2 );
}

void aggPiece::append( const aggPiece &a )
{
    for( int i = 0; i < a.numReliefs; i++ )
        addRelief( a.reliefs[i] );
}

void aggPiece::markAllDone()
{
    for( int i = 0; i < numReliefs; i++ )
        reliefs[i]->done = 1;
}

void aggPiece::normalize()
{
    for( int i = 1; i < numReliefs; i++ )
    {
        if( reliefs[i-1]->getBlockNum() == reliefs[i]->getBlockNum() &&
            reliefs[i-1]->to()->getT() == reliefs[i]->from()->getT() )
        {
            if( i < breakPoint )
                breakPoint--;
            reliefs[i]->pred = reliefs[i-1]->pred;
            for( int j = i; j < numReliefs; j++ )
                reliefs[j-1] = reliefs[j];
            numReliefs--;
            i = 0;
        }
    }
}

static void installPiece( const long r, // runNumber
                          const int pieceNum,
                          const ReliefPoint *from,
                          const ReliefPoint *to )
{
    // Update the first relief point (allowing for duplicate times).
    int j = from->ri;
    m_pRELIEFPOINTS[j].start.runNumber   = r;
    m_pRELIEFPOINTS[j].start.pieceNumber = pieceNum;

    // Update the middle relief points (allowing for duplicate times).
    for( j++;
            !(m_pRELIEFPOINTS[j].flags & RPFLAGS_LASTINBLOCK) &&
            m_pRELIEFPOINTS[j+1].time <= to->getT() &&
            m_pRELIEFPOINTS[j].start.runNumber == NO_RECORD;
         j++ )
    {
        m_pRELIEFPOINTS[j].start.runNumber   = r;
        m_pRELIEFPOINTS[j].start.pieceNumber = pieceNum;

        m_pRELIEFPOINTS[j].end.runNumber     = r;
        m_pRELIEFPOINTS[j].end.pieceNumber   = pieceNum;
    }

    // Update the final relief point.
    m_pRELIEFPOINTS[j].end.runNumber   = r;
    m_pRELIEFPOINTS[j].end.pieceNumber = pieceNum;
}

static int isRunValid( const long cutAsRunType )
{
    // First, check if this is a valid runtype.
    if( cutAsRunType == NO_RUNTYPE || cutAsRunType == UNCLASSIFIED_RUNTYPE )
        return FALSE;

    // Second, check if we are supposed to be cutting this runtype.
    if( (CUTPARMS.runtypes[LOWORD(cutAsRunType)][HIWORD(cutAsRunType)])
                && RUNTYPE[LOWORD(cutAsRunType)][HIWORD(cutAsRunType)].flags & RTFLAGS_INUSE )
        return TRUE;
   return FALSE;
}

void aggPiece::installRun( const LPGen & /*lpg*/ ) const
{
    int         i;
    PROPOSEDRUN pr;   // proposed pieces of the run
    COSTDef     cost; // returned cost structure

    // Populate the RELIEFPOINTS data structure if this run is valid.

    // First, initialize the pieces of the run so we can call the run coster.
    if( numReliefs > MAXPIECES )
        return;
    for( i = 0; i < numReliefs; i++ )
    {
        ReliefPoint *from = reliefs[i]->from();
        ReliefPoint *to   = reliefs[i]->to();

        pr.piece[i].fromTime = from->getT();
        pr.piece[i].fromNODESrecordID = m_pRELIEFPOINTS[from->ri].NODESrecordID;
        pr.piece[i].fromTRIPSrecordID = m_pRELIEFPOINTS[from->ri].TRIPSrecordID;

        pr.piece[i].toTime = to->getT();
        pr.piece[i].toNODESrecordID = m_pRELIEFPOINTS[to->ri].NODESrecordID;
        pr.piece[i].toTRIPSrecordID = m_pRELIEFPOINTS[to->ri].TRIPSrecordID;
    }
    pr.numPieces = numReliefs;

    // Check if this is a valid run.
    const long cutAsRunType = RunCoster(&pr, NO_RUNTYPE, &cost );
    if( !isRunValid(cutAsRunType) )
    {
        // OK, this is not a valid run.
        // Check each piece to see if we can install each piece as a tripper.
        pr.numPieces = 1;
        for( i = 0; i < numReliefs; i++ )
        {
            if( i != 0 )
                pr.piece[0] = pr.piece[i];
            const long cutAsRunType = RunCoster(&pr, NO_RUNTYPE, &cost );
            if( !isRunValid(cutAsRunType) )
                continue;
            // This piece of the run is a tripper, and we are supposed to cut trippers.
            // Install this piece as a tripper.
            const long r = m_GlobalRunNumber++;
            const int pieceNum = 1;
            installPiece( r, pieceNum, reliefs[i]->from(), reliefs[i]->to() );
        }
        return; // This run did not qualify totally so we tried to install as many pieces as possible.
    }

    // Install all the pieces of this run.
    const long r = m_GlobalRunNumber++;
    int pieceNum = 1;
    for( i = 0; i < numReliefs; i++ )
    {
        if( i == breakPoint )
            pieceNum++;
        else if( i > 0 &&
                reliefs[i-1]->getBlockNum() != reliefs[i]->getBlockNum() )
            pieceNum++;

        installPiece( r, pieceNum, reliefs[i]->from(), reliefs[i]->to() );
    }
}

ostream &aggPiece::printRun( ostream &os, const long r )
{
    // Print the run in a machine readable form.
    int pieceNum = 1;
    for( int i = 0; i < numReliefs; i++ )
    {
        if( i == breakPoint )
            pieceNum++;
        else if( i > 0 &&
                 reliefs[i-1]->getBlockNum() != reliefs[i]->getBlockNum() )
            pieceNum++;
        char    szOut[64];
        sprintf( szOut, "%7ld,%7ld,%10ld,%7ld,%7ld\n",
                 r,
                 (long)pieceNum,
                 reliefs[i]->getBlockNum(),
                 reliefs[i]->from()->getT(),
                 reliefs[i]->to()->getT() );
        os << szOut;
    }

    return os;
}

ostream &operator<<( ostream &os, const aggPiece &a )
{
    for( int i = 0; i < a.numReliefs; i++ )
    {
        if( i == a.breakPoint )
            os << "...\t";
        os << asTime(a.reliefs[i]->from()->getT()) << '-';
        os << asTime(a.reliefs[i]->to()->getT()) << " (";
        os << a.reliefs[i]->from()->getBlockNum() << ")\t";
    }
    
    return os;
}

