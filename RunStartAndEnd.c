//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

void RunStartAndEnd(long startTRIPSrecordID, long startNODESrecordID,
      long endTRIPSrecordID, long endNODESrecordID, long *onTime, long *offTime)
{
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  float distance;
  BOOL  found;
  int   rcode2;
  int   counter;

//
//  Generate the start trip
//
  if(startTRIPSrecordID == NO_RECORD)
    *onTime = NO_TIME;
  else
  {
    TRIPSKey0.recordID = startTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullout?
//
    if(TRIPS.standard.POGNODESrecordID == startNODESrecordID)
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
      *onTime = GTResults.firstNodeTime -
            GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
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
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS,
            &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if((found = PATTERNS.NODESrecordID == startNODESrecordID) == TRUE)
          break;
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          counter++;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      *onTime = (found ? GTResults.tripTimes[counter] : NO_TIME);
    }
  }
//
//  Generate the end trip
//
  if(endTRIPSrecordID == NO_RECORD)
    *offTime = NO_TIME;
  else
  {
    TRIPSKey0.recordID = endTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullin?
//
    if(TRIPS.standard.PIGNODESrecordID == endNODESrecordID)
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
      *offTime = GTResults.lastNodeTime +
            GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
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
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS,
            &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if((found = PATTERNS.NODESrecordID == endNODESrecordID) == TRUE)
          break;
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          counter++;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      *offTime = (found ? GTResults.tripTimes[counter] : NO_TIME);
    }
  }
  return;
}
