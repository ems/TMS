//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
extern "C" {
#include "TMSHeader.h"
}

#include <lpgen.H>

static void PerformCutCrew(void)
{
    int         i, j;

    LPGen       rcSolve;

    long        maxPieceSize = 0L;
    for( i = 0; i < NUMRUNTYPES; i++ )
         for( j = 0; j < NUMRUNTYPESLOTS; j++ )
              if( CUTPARMS.runtypes[i][j] && (RUNTYPE[i][j].flags & RTFLAGS_INUSE) )
              {
                  RunDef        *rd = new RunDef;

                  rd->runType = MAKELONG(i,j);

                  rd->workMin   =RUNTYPE[i][j].minPlatformTime;
                  rd->workMax   =RUNTYPE[i][j].maxPlatformTime;
                  rd->spreadMax =RUNTYPE[i][j].maxSpreadTime;
                  // Fix the on and off time... FIXLATER

                  for( int p = 0; p < RUNTYPE[i][j].numPieces; p++ )
                  {
                      rd->addWork( RUNTYPE[i][j].PIECE[p].minPieceSize,
                                   RUNTYPE[i][j].PIECE[p].maxPieceSize,
                                   RUNTYPE[i][j].PIECE[p].desPieceSize );
                      if( p != RUNTYPE[i][j].numPieces - 1 )
                          rd->addBreak( RUNTYPE[i][j].PIECE[p].minBreakTime,
                                        RUNTYPE[i][j].PIECE[p].maxBreakTime,
                                        RUNTYPE[i][j].PIECE[p].maxBreakTime );

                      if( RUNTYPE[i][j].PIECE[p].maxPieceSize > maxPieceSize )
                          maxPieceSize = RUNTYPE[i][j].PIECE[p].maxPieceSize;
                  }

                  rcSolve.addRunDef( rd );
              }

    // Add the blocks to the solve object.
    long dayStart = 3L * 24L * 60L * 60L;
    long dayEnd = 0L;
    long lastBlockNum = -1L;
    RBlock      *block = NULL;
    for( i = 0; i < m_numRELIEFPOINTS; i++ )
    {
        // Skip matched relief points
        int isMatched;
        if( i < m_numRELIEFPOINTS - 1 &&
            m_pRELIEFPOINTS[i].blockNumber == m_pRELIEFPOINTS[i+1].blockNumber)
             isMatched = (IsFromReliefPointMatched(m_pRELIEFPOINTS[i])
                          && IsToReliefPointMatched(m_pRELIEFPOINTS[i+1]) );
        else
             isMatched = IsToReliefPointMatched(m_pRELIEFPOINTS[i]);

        if( isMatched )
        {
            // Reset the logical block number.
            // This creates a logical "block" for each contiguous
            // set of relief points.
            lastBlockNum = -1L;
            continue;
        }
        if( m_pRELIEFPOINTS[i].blockNumber != lastBlockNum )
        {
            block = new RBlock( m_pRELIEFPOINTS[i].blockNumber );
            rcSolve.addBlock( block );
            lastBlockNum = m_pRELIEFPOINTS[i].blockNumber;
        }
        block->addRelief( m_pRELIEFPOINTS[i].time, i );
        if( m_pRELIEFPOINTS[i].time < dayStart )
            dayStart = m_pRELIEFPOINTS[i].time;
        if( m_pRELIEFPOINTS[i].time > dayEnd )
            dayEnd = m_pRELIEFPOINTS[i].time;
    }

    // For feasibility, add a tripper run type.
    RunDef *rdFill = new RunDef( maxPieceSize );
    rdFill->isPenalty = 1;
    rdFill->addWork( CUTPARMS.minLeftover, maxPieceSize, maxPieceSize );
    rcSolve.addRunDef( rdFill );

    // The last argument is the bucket size (time resolution) during the day.
    // Pick a bucketSize that leads to a "reasonable" number of patterns to try.
    const long idealNumPatterns = 4096L;
    long bucketSize;
    const long minute = 60L;
    const long hour = 60L * 60L;
//    static const long bucketSizes[] =
//    {
//      2.0*hour,
//      1.5*hour,
//      1.0*hour,
//      45L*minute,
//      30L*minute,
//      20L*minute,
//      15L*minute,
//      10L*minute,
//      5L *minute
//    };
//    int left = 0, right = sizeof(bucketSizes) / sizeof(bucketSizes[0]) - 1;
//    while( left < right )
//    {
//        int middle = (left + right) / 2;
//        bucketSize = bucketSizes[middle];
//        long numPatterns = rcSolve.getNumPatterns( dayStart, dayEnd, bucketSize );
//        if( numPatterns < idealNumPatterns )
//            left = middle + 1;
//        else
//            right = middle;
//    }
//    bucketSize = bucketSizes[left];
//    if( rcSolve.getNumPatterns(dayStart, dayEnd, bucketSize) > idealNumPatterns )
//        if( left > 0 )
//            bucketSize = bucketSizes[left - 1];
//

    bucketSize = 1L * hour;
    rcSolve.solveLP( dayStart, dayEnd, CUTPARMS.minLeftover, bucketSize );
    if( rcSolve.getIdealRuncutCost() < INFINITY )
    {
        // Problem is feasible.
        rcSolve.cutBlocks();
        rcSolve.cutRuns();  // this will install the solution into RELIEFPOINTS
    }
}

extern "C" {
void CutCrew( void )
{
  int runsCut = 0;
  int totalCut = 0;
  // This is super bogus - keep cutting crews until we don't find any more runs.
  long  GlobalRunNumberLast;
  for( ;; )
  {
    if(StatusBarAbort())
      break;
    GlobalRunNumberLast = m_GlobalRunNumber;
    PerformCutCrew();
    runsCut = m_GlobalRunNumber - GlobalRunNumberLast;
    totalCut += runsCut;
    sprintf(tempString, "Runs cut this pass: %d, Total cut: %d", runsCut, totalCut);
    StatusBarText(tempString);

    if( GlobalRunNumberLast == m_GlobalRunNumber )
      break;
  }
}
} // extern "C"
