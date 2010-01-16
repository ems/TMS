//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

long GetTimeAtNode(long TRIPSrecordID, long NODESrecordID, int count, TRIPSDef *pTRIPS)
{
  GenerateTripDef GTResults;
  TRIPSDef LocalTRIPS;
  BOOL bGotNode;
  long timeAtNode;
  int  nI;
  int  rcode2;
  int  position;
//
//  Get the trip if required
//
  if(TRIPSrecordID == NO_RECORD)
  {
    if(pTRIPS == NULL)
      return(NO_TIME);
    memcpy(&LocalTRIPS, pTRIPS, sizeof(TRIPSDef));
  }
  else
  {
    TRIPSKey0.recordID = TRIPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &LocalTRIPS, &TRIPSKey0, 0);
    if(rcode2 != 0)
      return(NO_TIME);
  }
//
//  Generate the trip
//
  GenerateTrip(LocalTRIPS.ROUTESrecordID, LocalTRIPS.SERVICESrecordID,
        LocalTRIPS.directionIndex, LocalTRIPS.PATTERNNAMESrecordID,
        LocalTRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Time at node.  Hunt for it in case we have to pay attention to position
//
  position = 0;
  PATTERNSKey2.ROUTESrecordID = LocalTRIPS.ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = LocalTRIPS.SERVICESrecordID;
  PATTERNSKey2.directionIndex = LocalTRIPS.directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = LocalTRIPS.PATTERNNAMESrecordID;
  PATTERNSKey2.nodeSequence = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  nI = 0;
  bGotNode = FALSE;
  while(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == LocalTRIPS.ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == LocalTRIPS.SERVICESrecordID &&
        PATTERNS.directionIndex == LocalTRIPS.directionIndex &&
        PATTERNS.PATTERNNAMESrecordID == LocalTRIPS.PATTERNNAMESrecordID)
  {
    if(PATTERNS.NODESrecordID == NODESrecordID)
    {
      if(position != count)
        position++;
      else
      {
        bGotNode = TRUE;
        break;
      }
    }
    if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
    {
      nI++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
  timeAtNode = bGotNode ? GTResults.tripTimes[nI] : NO_TIME;

  return(timeAtNode);
}
