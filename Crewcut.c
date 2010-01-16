//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"

//
//  IdealRunCoster() - In an ideal world, check on the validity of a run
//
tod_t IdealRunCoster(long RuntypeToCut, IdealRunPiece *pieces, long numPieces)
{
  long nI;
  long totalPlatformTime;
  long totalSpreadTime;
  long breakTime;
  BOOL bCheckRunMinBreak, bGotRunMinBreak;
  BOOL bCheckRunMaxBreak, bGotRunMaxBreak;
  long contigTime;
  BOOL bCheckContigTime;
  RUNTYPEDef *pR = &RUNTYPE[LOWORD(RuntypeToCut)][HIWORD(RuntypeToCut)];

//
//  Make sure that the runtype is actually being used
//
  if(!(pR->flags & RTFLAGS_INUSE))
    return(NO_TIME);
//
//  No point going on if the number of pieces don't match
//
  if(pR->numPieces != numPieces)
    return(NO_TIME);
//
//  Loop through the pieces
//
  totalPlatformTime = 0;
  bCheckRunMinBreak = pR->minBreakOf != NO_TIME && numPieces > 2;
  bGotRunMinBreak = FALSE;
  bCheckRunMaxBreak = pR->maxBreakOf != NO_TIME && numPieces > 2;
  bGotRunMaxBreak = FALSE;
  bCheckContigTime = pR->maxContig != NO_TIME && numPieces > 2;
  contigTime = 0;
  for(nI = 0; nI < numPieces; nI++)
  {
    int pieceEnd = pieces[nI].start + pieces[nI].length;
    PIECEDef *pRP = &(pR->PIECE[nI]);
//
//  Check the min and max on times
//
    if((pRP->minOnTime != NO_TIME && pieces[nI].start < pRP->minOnTime) ||
           (pRP->maxOnTime != NO_TIME && pieces[nI].start > pRP->maxOnTime))
      return(NO_TIME);
//
//  Check the min and max off times
//
    if((pRP->minOffTime != NO_TIME && pieceEnd < pRP->minOffTime) ||
           (pRP->maxOffTime != NO_TIME && pieceEnd > pRP->maxOffTime))
      return(NO_TIME);
//
//  Check the min and max piece sizes.  The check for NO_TIME may
//  seem silly, but since this is a user input field, you never know.
//
    if((pRP->minPieceSize != NO_TIME && pieces[nI].length < pRP->minPieceSize) ||
           (pRP->maxPieceSize != NO_TIME && pieces[nI].length > pRP->maxPieceSize))
      return(NO_TIME);
//
//  Check the min and max break times
//
    if(nI != numPieces - 1)
    {
      breakTime = pieces[nI + 1].start - pieceEnd;
      if(pRP->minBreakTime != NO_TIME && breakTime < pRP->minBreakTime)
        return(NO_TIME);
      if(pRP->maxBreakTime != NO_TIME && breakTime > pRP->maxBreakTime)
        return(NO_TIME);
      if(bCheckRunMinBreak && !bGotRunMinBreak && breakTime > pR->minBreakOf)
        bGotRunMinBreak = TRUE;
      if(bCheckRunMaxBreak && !bGotRunMaxBreak && breakTime < pR->maxBreakOf)
        bGotRunMaxBreak = TRUE;
    }
//
//   Check the contiguous times of the pieces
//
    if(bCheckContigTime)
    {
      if(nI == numPieces)
      {
        if((contigTime += pieces[nI].length) > pR->maxContig)
          return(NO_TIME);
      }
      else
      {
        if(pieceEnd == pieces[nI + 1].start)
        {
          if((contigTime += pieces[nI].length) > pR->maxContig)
            return(NO_TIME);
        }
        else
        {
          contigTime = 0;  // Start again
        }
      }
    }

//
//  Add this piece to the total platform time
//
    totalPlatformTime += pieces[nI].length;
  }
//
//  Done checking the pieces - check the total platform
//
  if((pR->minPlatformTime != NO_TIME && totalPlatformTime < pR->minPlatformTime) ||
        (pR->maxPlatformTime != NO_TIME && totalPlatformTime > pR->maxPlatformTime))
    return(NO_TIME);
//
//  If necessary, check the total run breaks
//
  if(bCheckRunMinBreak && !bGotRunMinBreak)
    return(NO_TIME);
  if(bCheckRunMaxBreak && !bGotRunMaxBreak)
    return(NO_TIME);
//
//  Last but not least, check the spread time
//
  totalSpreadTime = pieces[numPieces - 1].start + pieces[numPieces - 1].length - pieces[0].start;
  if(pR->maxSpreadTime != NO_RECORD && totalSpreadTime > pR->maxSpreadTime)
    return(NO_TIME);
//
//  Return a value.  For the lack of anything better, return the total spread time
//
  return((tod_t)totalSpreadTime);
}

//
//  ReliefIsPossible() - See if we can travel from Point "B"
//                       to Point "C" in the allotted time.
//
BOOL ReliefIsPossible(long fromTRIPSrecordID, long fromNODESrecordID, long fromSGRPSERVICESrecordID,
      tod_t fromReliefTime, long toTRIPSrecordID, long toNODESrecordID, tod_t toReliefTime)
{
  GetConnectionTimeDef GCTData;
  float distance;
  long travelTime, dwellTime;
  long startTime, endTime;
  long waitTime;
  long equivalentTravelTime;
//
//  First things first - are the nodes equivalenced?
//
  if(NodesEquivalent(fromNODESrecordID, toNODESrecordID, &equivalentTravelTime))
    return(TRUE);
//
//  Dynamic travels?
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISplanReliefConnect(&startTime, &endTime, &dwellTime, 0, fromNODESrecordID,
            toNODESrecordID, fromReliefTime, FALSE, fromSGRPSERVICESrecordID);
      travelTime = (endTime - startTime) + dwellTime;
    }
    else
      travelTime = GetDynamicTravelTime(TRUE, fromNODESrecordID, toNODESrecordID,
            fromSGRPSERVICESrecordID, toReliefTime, &waitTime);
  }
//
//  Set up the GCTData structure for input into GetConnectionTime()
//
  else
  {
    TRIPSKey0.recordID = fromTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    GCTData.fromNODESrecordID = fromNODESrecordID;
    GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
    GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    TRIPSKey0.recordID = toTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    GCTData.toNODESrecordID = toNODESrecordID;
    GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
    GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    GCTData.timeOfDay = fromReliefTime;
    travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
  }
//
//  Check 1: Is it a valid travel?
//
  if(travelTime == NO_TIME)
    return(FALSE);
//
//  Check 2: Return TRUE if he can make it, FALSE if he can't
//
  return(fromReliefTime + travelTime <= toReliefTime);
}
