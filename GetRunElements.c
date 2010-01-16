//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

int GetRunElements(HWND hWnd, RUNSDef *pRUN, PROPOSEDRUNDef *pPROPOSEDRUN, COSTDef *pCOST, BOOL bReposition)
{
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  float distance;
  BOOL  bFound;
  BOOL  bStop;
  long  cutAsRuntype;
  long  runNumber;
  long  divisionRecordID;
  long  serviceRecordID;
  long  time;
  int   lastPiece;
  int   counter;
  int   nI;
  int   pieceNumber;
  int   rcode2;

//
//  Reposition the key
//
  rcode2 = 0;
  if(bReposition)
  {
    btrieve(B_GETPOSITION, TMS_RUNS, pRUN, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, pRUN, &RUNSKey1, 1);
  }
//
//  Get all of the data for this run
//
  memset(&RUNSVIEW, 0x00, sizeof(RUNSVIEWDef));
  runNumber = pRUN->runNumber;
  divisionRecordID = pRUN->DIVISIONSrecordID;
  serviceRecordID = pRUN->SERVICESrecordID;
  pieceNumber = 0;
  bStop = FALSE;
  while(rcode2 == 0 &&
        pRUN->DIVISIONSrecordID == divisionRecordID &&
        pRUN->SERVICESrecordID == serviceRecordID &&
        pRUN->runNumber == runNumber &&
        pieceNumber < MAXPIECES)
  {
    RUNSVIEW[pieceNumber].runNumber = runNumber;
    RUNSVIEW[pieceNumber].runOnTime = NO_TIME;
    RUNSVIEW[pieceNumber].runOffTime = NO_TIME;
    TRIPSKey0.recordID = pRUN->start.TRIPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    if(rcode2 != 0)
    {
      RUNSVIEW[pieceNumber].runOnTRIPSrecordID = NO_RECORD;
      bStop = TRUE;
      break;
    }
    RUNSVIEW[pieceNumber].blockNumber = TRIPS.standard.blockNumber;
    RUNSVIEW[pieceNumber].ROUTESrecordID = TRIPS.ROUTESrecordID;
    RUNSVIEW[pieceNumber].onTripSERVICESrecordID = TRIPS.SERVICESrecordID;
//
//  Generate the start trip
//
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullout?
//
    if(TRIPS.standard.POGNODESrecordID == pRUN->start.NODESrecordID)
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
      GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
      GCTData.timeOfDay = GTResults.firstNodeTime;
      RUNSVIEW[pieceNumber].runOnTime =
            GTResults.firstNodeTime - GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
    }
//
//  Nope - Find the node on the pattern
//
    else
    {
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if((bFound = PATTERNS.NODESrecordID == pRUN->start.NODESrecordID) == TRUE)
          break;
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          counter++;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      RUNSVIEW[pieceNumber].runOnTime = bFound ? GTResults.tripTimes[counter] : NO_TIME;
    }
//
//  Generate the end trip
//
    TRIPSKey0.recordID = pRUN->end.TRIPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    if(rcode2 != 0)
    {
      RUNSVIEW[pieceNumber].runOffTRIPSrecordID = NO_RECORD;
      bStop = TRUE;
      break;
    }
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullin?
//
    if(TRIPS.standard.PIGNODESrecordID == pRUN->end.NODESrecordID)
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
      GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
      GCTData.timeOfDay = GTResults.lastNodeTime;
      RUNSVIEW[pieceNumber].runOffTime =
            GTResults.lastNodeTime + GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
    }
//
//  Nope - Find the node on the pattern
//
    else
    {
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if((bFound = PATTERNS.NODESrecordID == pRUN->end.NODESrecordID) == TRUE)
          break;
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          counter++;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      RUNSVIEW[pieceNumber].runOffTime = bFound ? GTResults.tripTimes[counter] : NO_TIME;
    }
//
//  Set up a couple of extra RUNSVIEW elements for the benefit of the reports
//
    RUNSVIEW[pieceNumber].runOnNODESrecordID = pRUN->start.NODESrecordID;
    RUNSVIEW[pieceNumber].runOnTRIPSrecordID = pRUN->start.TRIPSrecordID;
    RUNSVIEW[pieceNumber].runOffNODESrecordID = pRUN->end.NODESrecordID;
    RUNSVIEW[pieceNumber].runOffTRIPSrecordID = pRUN->end.TRIPSrecordID;
//
//  And for any extraboard displays
//
    RUNSVIEW[pieceNumber].flags = pRUN->flags;
    if(pRUN->flags & RUNS_FLAG_EXTRABOARDPRIOR)
    {
      RUNSVIEW[pieceNumber].startOfPieceExtraboardStart = pRUN->prior.startTime;
      RUNSVIEW[pieceNumber].startOfPieceExtraboardEnd = pRUN->prior.endTime;
    }
    else
    {
      RUNSVIEW[pieceNumber].startOfPieceExtraboardStart = NO_TIME;
      RUNSVIEW[pieceNumber].startOfPieceExtraboardEnd = NO_TIME;
    }
    if(pRUN->flags & RUNS_FLAG_EXTRABOARDAFTER)
    {
      RUNSVIEW[pieceNumber].endOfPieceExtraboardStart = pRUN->after.startTime;
      RUNSVIEW[pieceNumber].endOfPieceExtraboardEnd = pRUN->after.endTime;
    }
    else
    {
      RUNSVIEW[pieceNumber].endOfPieceExtraboardStart = NO_TIME;
      RUNSVIEW[pieceNumber].endOfPieceExtraboardEnd = NO_TIME;
    }
//  
//  Set up PROPOSEDRUN
//
    pPROPOSEDRUN->piece[pieceNumber].fromTime = RUNSVIEW[pieceNumber].runOnTime;
    pPROPOSEDRUN->piece[pieceNumber].fromNODESrecordID = pRUN->start.NODESrecordID;
    pPROPOSEDRUN->piece[pieceNumber].fromTRIPSrecordID = pRUN->start.TRIPSrecordID;
    pPROPOSEDRUN->piece[pieceNumber].toTime = RUNSVIEW[pieceNumber].runOffTime;
    pPROPOSEDRUN->piece[pieceNumber].toNODESrecordID = pRUN->end.NODESrecordID;
    pPROPOSEDRUN->piece[pieceNumber].toTRIPSrecordID = pRUN->end.TRIPSrecordID;
    pPROPOSEDRUN->piece[pieceNumber].prior.startTime = pRUN->prior.startTime;
    pPROPOSEDRUN->piece[pieceNumber].prior.endTime = pRUN->prior.endTime;
    pPROPOSEDRUN->piece[pieceNumber].after.startTime = pRUN->after.startTime;
    pPROPOSEDRUN->piece[pieceNumber].after.endTime = pRUN->after.endTime;
//
//  And the runtype
//
    cutAsRuntype = pRUN->cutAsRuntype;
//
//  Get the next run record
//
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, pRUN, &RUNSKey1, 1);
    pieceNumber++;
  }
//
//  Check the integrity of PROPOSEDRUN
//
  if(pieceNumber == 0)
    bStop = TRUE;
  if(!bStop)
  {
    for(nI = 0; nI < pieceNumber; nI++)
    {
      if(pPROPOSEDRUN->piece[nI].fromTime == NO_TIME ||
            pPROPOSEDRUN->piece[nI].fromNODESrecordID == NO_RECORD ||
            pPROPOSEDRUN->piece[nI].fromTRIPSrecordID == NO_RECORD ||
            pPROPOSEDRUN->piece[nI].toTime == NO_TIME ||
            pPROPOSEDRUN->piece[nI].toNODESrecordID == NO_RECORD ||
            pPROPOSEDRUN->piece[nI].toTRIPSrecordID == NO_RECORD)
      {
        bStop = TRUE;
        break;
      }
    }
  }
//
//  Cost the run
//
  if(!bStop)
  {
    pPROPOSEDRUN->numPieces = pieceNumber;
    RunCoster(pPROPOSEDRUN, cutAsRuntype, pCOST);
//
//  And fill out the structure
//
    for(nI = 0; nI < pPROPOSEDRUN->numPieces; nI++)
    {
//
//  Runtype
//
      RUNSVIEW[nI].runType = nI == 0 ? cutAsRuntype : NO_RECORD;
//
//  Platform time
//
      RUNSVIEW[nI].platformTime = pCOST->PIECECOST[nI].platformTime;
//
//  Spread Overtime
//
      RUNSVIEW[nI].spreadOT = nI == pPROPOSEDRUN->numPieces - 1 ?
            pPROPOSEDRUN->numPieces == 1 ? 0 : pCOST->spreadOvertime : NO_TIME;
//
//  Spread Time
//
      RUNSVIEW[nI].spreadTime = nI == pPROPOSEDRUN->numPieces - 1 ? pCOST->spreadTime : NO_TIME;
//
//  Overtime
//
      RUNSVIEW[nI].overTime = nI == pPROPOSEDRUN->numPieces - 1 ? pCOST->TOTAL.overTime : NO_TIME;
//
//  Premium time
//
      RUNSVIEW[nI].premiumTime = pPROPOSEDRUN->numPieces == 1 ||
            nI == pPROPOSEDRUN->numPieces - 1 ?
            pCOST->TOTAL.premiumTime : pCOST->PIECECOST[nI].premiumTime;
//
//  MakeUp time
//
      RUNSVIEW[nI].makeUpTime = nI == pPROPOSEDRUN->numPieces - 1 ? pCOST->TOTAL.makeUpTime : NO_TIME;
//
//  Paid breaks
//
      RUNSVIEW[nI].paidBreaks = nI == pPROPOSEDRUN->numPieces - 1 ? pCOST->TOTAL.paidBreak : NO_TIME;
//
//  Pay time
//
      RUNSVIEW[nI].payTime = nI == pPROPOSEDRUN->numPieces - 1 ? pCOST->TOTAL.payTime : NO_TIME;
//
//  Start and End of Piece Travel times
//
      RUNSVIEW[nI].startOfPieceTravel = pCOST->TRAVEL[nI].startTravelTime;
      RUNSVIEW[nI].endOfPieceTravel = pCOST->TRAVEL[nI].endTravelTime;
      RUNSVIEW[nI].travelTime = RUNSVIEW[nI].startOfPieceTravel + RUNSVIEW[nI].endOfPieceTravel;
//
//  Start and End of Piece Travel locations
//
      if(pCOST->TRAVEL[nI].startNODESrecordID == NO_RECORD)
      {
        RUNSVIEW[nI].startOfPieceNODESrecordID = pPROPOSEDRUN->piece[nI].fromNODESrecordID;
      }
      else
      {
        RUNSVIEW[nI].startOfPieceNODESrecordID = pCOST->TRAVEL[nI].startNODESrecordID;
      }
      if(pCOST->TRAVEL[nI].endNODESrecordID == NO_RECORD)
      {
        RUNSVIEW[nI].endOfPieceNODESrecordID = pPROPOSEDRUN->piece[nI].toNODESrecordID;
      }
      else
      {
        RUNSVIEW[nI].endOfPieceNODESrecordID = pCOST->TRAVEL[nI].endNODESrecordID;
      }
//
//  Report and Turnin
//
      RUNSVIEW[nI].reportTime = pCOST->PIECECOST[nI].reportTime;
      RUNSVIEW[nI].turninTime = pCOST->PIECECOST[nI].turninTime;
//
//  Total travel and paid break(s)
//
      RUNSVIEW[nI].paidMeal = RUNSVIEW[nI].paidMeal += pCOST->PIECECOST[nI].paidBreak;
    }
//
//  Fix up spread time by examining extraboard prior to start and after end of the run
//
    if(RUNSVIEW[0].startOfPieceExtraboardStart != NO_TIME)
    {
      time = RUNSVIEW[0].runOnTime - RUNSVIEW[0].reportTime - RUNSVIEW[0].startOfPieceTravel;
      time -= RUNSVIEW[0].startOfPieceExtraboardStart;
      RUNSVIEW[pPROPOSEDRUN->numPieces - 1].spreadTime += time;
    }
    lastPiece = pPROPOSEDRUN->numPieces - 1;
    if(RUNSVIEW[lastPiece].endOfPieceExtraboardEnd != NO_TIME)
    {
      time = RUNSVIEW[lastPiece].runOffTime + RUNSVIEW[lastPiece].turninTime + RUNSVIEW[lastPiece].endOfPieceTravel;
      time = RUNSVIEW[lastPiece].endOfPieceExtraboardEnd - time;
      RUNSVIEW[pPROPOSEDRUN->numPieces - 1].spreadTime += time;
    }
  }
  return(pieceNumber);
}
