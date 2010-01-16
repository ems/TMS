//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static   int numRuns = 0;

static long AddReliefPointToRun( RPSEDef *se, long reliefPoint, long runIndexHint )
{
  BOOL  found = FALSE;
  int   nJ, thisRun, thisPiece;
  RELIEFPOINTSDef *relief = &m_pRELIEFPOINTS[reliefPoint];
  RUNLISTDef *RLRun;

//
//  First find the run in the runlist.
//
  // Check if it is the same run number as last time.
  // This happens a lot since consecutive relief points occur together.
  // This saves a lot of time.
  if( runIndexHint < numRuns && RUNLIST[runIndexHint].runNumber == se->runNumber )
  {
	  thisRun = runIndexHint;
	  found = TRUE;
      RLRun = &RUNLIST[thisRun];
  }
  else
  {
	  // Find the run number in the RUNLIST.
	  // This will only execute if we did not get a match with the runIndexHint.
	  for( nJ = 0; nJ < numRuns; nJ++ )
	  {
	  	if(RUNLIST[nJ].runNumber == se->runNumber)
		  {
  		  thisRun = runIndexHint = nJ;
  		  found = TRUE;
  		  RLRun = &RUNLIST[thisRun];
  		  break;
  		}
	  }
  }
//
//  Nope.  Bump up numRuns and initialize this portion of runlist
//
  if(!found)
  {
    thisRun = runIndexHint = numRuns++;
    RLRun = &RUNLIST[thisRun];
    RLRun->runNumber = se->runNumber;
    RLRun->run.numPieces = 0;
    for(thisPiece = 0; thisPiece < MAXPIECES; thisPiece++)
    {
      RLRun->recordID[thisPiece] = NO_RECORD;
      RLRun->run.piece[thisPiece].fromTime = NO_TIME;
      RLRun->run.piece[thisPiece].fromNODESrecordID = NO_RECORD;
      RLRun->run.piece[thisPiece].fromTRIPSrecordID = NO_RECORD;
      RLRun->run.piece[thisPiece].toTime = NO_TIME;
      RLRun->run.piece[thisPiece].toNODESrecordID = NO_RECORD;
      RLRun->run.piece[thisPiece].toTRIPSrecordID = NO_RECORD;
      RLRun->run.piece[thisPiece].prior.startTime = NO_TIME;
      RLRun->run.piece[thisPiece].prior.endTime = NO_TIME;
      RLRun->run.piece[thisPiece].after.startTime = NO_TIME;
      RLRun->run.piece[thisPiece].after.endTime = NO_TIME;
      RLRun->run.piece[thisPiece].flags = 0L;
    }
  }
//
//  Set thisPiece to the pieceNumber from the relief point
//  Assume the piece numbers start at 1.
//
  thisPiece = se->pieceNumber - 1;
  if( se->pieceNumber > RLRun->run.numPieces )
    RLRun->run.numPieces = se->pieceNumber;
//
//  Start looking.  If the fromTime on thisRun and thisPiece is NO_TIME,
//  it means that this is the first entry, so drop all the crap here.
//

  if(RLRun->run.piece[thisPiece].fromTime == NO_TIME)
  {
    RLRun->recordID[thisPiece] = se->recordID;
    RLRun->run.piece[thisPiece].fromTime = relief->time;
    RLRun->run.piece[thisPiece].fromNODESrecordID = relief->NODESrecordID;
    RLRun->run.piece[thisPiece].fromTRIPSrecordID = relief->TRIPSrecordID;
  }
  else
  {
    RLRun->run.piece[thisPiece].toTime = relief->time;
    RLRun->run.piece[thisPiece].toNODESrecordID = relief->NODESrecordID;
    RLRun->run.piece[thisPiece].toTRIPSrecordID = relief->TRIPSrecordID;
  }
  RLRun->run.piece[thisPiece].prior.startTime = se->prior.startTime;
  RLRun->run.piece[thisPiece].prior.endTime = se->prior.endTime;
  RLRun->run.piece[thisPiece].after.startTime = se->after.startTime;
  RLRun->run.piece[thisPiece].after.endTime = se->after.endTime;
  RLRun->run.piece[thisPiece].flags = se->flags;

  return runIndexHint;
}

int AssembleRuns(void)
{
  long  nI, runIndexHint = 0;

  numRuns = 0;
//
//  Careen through RELIEFPOINTS and RUNLIST to split out the data
//
  for(nI = 0; nI < m_numRELIEFPOINTS; nI++)
  {
    if( (m_pRELIEFPOINTS[nI].flags & RPFLAGS_INUSE) && IsReliefPointInRun(m_pRELIEFPOINTS[nI]) )
    {
      if( m_pRELIEFPOINTS[nI].start.runNumber != NO_RECORD &&
            m_pRELIEFPOINTS[nI].start.runNumber != SECURELOCATION_RUNNUMBER)
          runIndexHint = AddReliefPointToRun( &m_pRELIEFPOINTS[nI].start, nI, runIndexHint );
      if( m_pRELIEFPOINTS[nI].end.runNumber != NO_RECORD  &&
            m_pRELIEFPOINTS[nI].end.runNumber != SECURELOCATION_RUNNUMBER)
          runIndexHint = AddReliefPointToRun( &m_pRELIEFPOINTS[nI].end, nI, runIndexHint );
    }
  }

  return numRuns;
}
