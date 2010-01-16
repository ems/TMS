//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

//
//  getdynam.c - Calculate travel times dynamically
//

#define UPPERLIMIT 200

void InitTravelMatrix(long SERVICESrecordID, BOOL bDisplayStatus)
{
  typedef struct PATHStruct
  {
    long NODESrecordID;
    long ROUTESrecordID;
    long directionIndex;
    long PATTERNNAMESrecordID;
    long nodeSequence;
  } PATHDef;
  typedef struct COMBINEDStruct
  {
    long fromNODESrecordID;
    long toNODESrecordID;
    long ROUTESrecordID;
    long directionIndex;
    long PATTERNNAMESrecordID;
    int  fromNodePos;
    int  toNodePos;
  } COMBINEDDef;
  typedef struct BESTStruct
  {
    long fromNODESrecordID;
    long toNODESrecordID;
    long TRIPSrecordID;
    long cushionTime;
    long dwellTime;
    long tripTime[2];
  } BESTDef;
  PATHDef from[UPPERLIMIT];
  PATHDef to[UPPERLIMIT];
  COMBINEDDef fromTo[UPPERLIMIT];
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  float distance;
  long fromEquiv[UPPERLIMIT];
  long toEquiv[UPPERLIMIT];
  long NODESrecordID;
  long nodeList[50];
  long fromNODESrecordID;
  long toNODESrecordID;
  long deadheadTime;
  BOOL bFound;
  int  rcode2;
  int  numNodes;
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
  int  nM;
  int  pos;
  int  numFromEquiv;
  int  numToEquiv;
  int  numFromTo;
  int  numFrom;
  int  numTo;
  int  serviceIndex;

//
//  Set up the service index
//
  SERVICESKey0.recordID = SERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  serviceIndex = SERVICES.number - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
  strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(szarString, SERVICES_NAME_LENGTH);
//
//  Show the status bar
//
  if(bDisplayStatus)
  {
    sprintf(tempString, "Status for service %s", szarString);
    StatusBarStart(NULL, tempString);
    LoadString(hInst, TEXT_135, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
  }
//
//  Set up the node equivalences
//
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  numNodeEquiv = 0;
  while(rcode2 == 0 && numNodeEquiv < MAXNODEEQUIV)
  {
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      NODEEQUIV[numNodeEquiv].fromNODESrecordID = CONNECTIONS.fromNODESrecordID;
      NODEEQUIV[numNodeEquiv].toNODESrecordID = CONNECTIONS.toNODESrecordID;
      numNodeEquiv++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
//
//  Set up, along with all garages, all relief points
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  numNodes = 0;
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_GARAGE ||
          (NODES.reliefLabels[serviceIndex] != ' ' && NODES.reliefLabels[serviceIndex] != '-'))
    {
      nodeList[numNodes] = NODES.recordID;
      numNodes++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Free any previous incarnation of the travel matrix
//
  for(nI = 0; nI < numInTM[serviceIndex]; nI++)
  {
    TMSHeapFree(TRAVELMATRIX[serviceIndex][nI].pTrips);
  }
//
//  Build the matrix
//
  numInTM[serviceIndex] = 0;
  for(nI = 0; nI < numNodes - 1; nI++)
  {
    if(bDisplayStatus)
    {
      StatusBar((long)nI, (long)numNodes);
    }
    for(nJ = 1; nJ < numNodes; nJ++)
    {
      for(nK = 0; nK < 2; nK++)
      {
        fromNODESrecordID = nK == 0 ? nodeList[nI] : nodeList[nJ];
        toNODESrecordID = nK == 0 ? nodeList[nJ] : nodeList[nI];
//
//  Get the node equivalences from the NODEEQUIV structure
//
        numFromEquiv = 0;
        numToEquiv = 0;
        for(nL = 0; nL < numNodeEquiv; nL++)
        {
          if(NODEEQUIV[nL].fromNODESrecordID == fromNODESrecordID)
          {
            fromEquiv[numFromEquiv++] = NODEEQUIV[nL].toNODESrecordID;
          }
          else if(NODEEQUIV[nL].toNODESrecordID == fromNODESrecordID)
          {
            fromEquiv[numFromEquiv++] = NODEEQUIV[nL].fromNODESrecordID;
          }
          else if(NODEEQUIV[nL].fromNODESrecordID == toNODESrecordID)
          {
            toEquiv[numToEquiv++] = NODEEQUIV[nL].toNODESrecordID;
          }
          else if(NODEEQUIV[nL].toNODESrecordID == toNODESrecordID)
          {
            toEquiv[numToEquiv++] = NODEEQUIV[nL].fromNODESrecordID;
          }
        }
//
//  Ok, we know where we are (fromNODESrecordID) and where we have to go to (toNODESrecordID).
//
//  The algorithm is find the route required to get us from a to b in the shortest time,
//  placing instructions in "travelInstructions".
//
//  First, get a list of patterns that fromNODESrecordID (or one of its equivalences) reside in.
//  Note that we keep PATTERNS.NODESrecordID, as it may actually be the substituted, equivalent, node.
//
        numFrom = 0;
        for(nL = 0; nL < numFromEquiv + 1; nL++)
        {
          NODESrecordID = nL == 0 ? fromNODESrecordID : fromEquiv[nL - 1];
          PATTERNSKey1.NODESrecordID = NODESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          while(rcode2 == 0 && PATTERNS.NODESrecordID == NODESrecordID)
          {
            if(PATTERNS.SERVICESrecordID == SERVICESrecordID)
            {
              from[numFrom].NODESrecordID = PATTERNS.NODESrecordID;
              from[numFrom].ROUTESrecordID = PATTERNS.ROUTESrecordID;
              from[numFrom].directionIndex = PATTERNS.directionIndex;
              from[numFrom].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
              from[numFrom].nodeSequence = PATTERNS.nodeSequence;
              numFrom++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          }
        }
//
//  If there weren't any, cycle through
//
        if(numFrom == 0)
        {
          continue;
        }
//
//  Now, get a list of patterns that toNODESrecordID are in.
//
        numTo = 0;
        for(nL = 0; nL < numToEquiv + 1; nL++)
        {
          NODESrecordID = nL == 0 ? toNODESrecordID : toEquiv[nL - 1];
          PATTERNSKey1.NODESrecordID = NODESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          while(rcode2 == 0 && PATTERNS.NODESrecordID == NODESrecordID)
          {
            if(PATTERNS.SERVICESrecordID == SERVICESrecordID)
            {
              to[numTo].NODESrecordID = PATTERNS.NODESrecordID;
              to[numTo].ROUTESrecordID = PATTERNS.ROUTESrecordID;
              to[numTo].directionIndex = PATTERNS.directionIndex;
              to[numTo].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
              to[numTo].nodeSequence = PATTERNS.nodeSequence;
              numTo++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          }
        }
//
//  If there weren't any, cycle through
//
        if(numTo == 0)
          continue;
//
//  Develop a list of patterns that have both the from and to nodes
//  on them, and that have the "to" node following the "from" node.
//
//  First, "merge" the lists
//
        numFromTo = 0;
        for(nL = 0; nL < numFrom; nL++)
        {
          for(nM = 0; nM < numTo; nM++)
          {
            if(from[nL].ROUTESrecordID == to[nM].ROUTESrecordID &&
                  from[nL].directionIndex == to[nM].directionIndex &&
                  from[nL].PATTERNNAMESrecordID == to[nM].PATTERNNAMESrecordID &&
                  from[nL].nodeSequence < to[nM].nodeSequence)
            {
              fromTo[numFromTo].fromNODESrecordID = from[nL].NODESrecordID;
              fromTo[numFromTo].toNODESrecordID = to[nM].NODESrecordID;
              fromTo[numFromTo].ROUTESrecordID = from[nL].ROUTESrecordID;
              fromTo[numFromTo].directionIndex = from[nL].directionIndex;
              fromTo[numFromTo].PATTERNNAMESrecordID = from[nL].PATTERNNAMESrecordID;
              fromTo[numFromTo].fromNodePos = NO_RECORD;
              fromTo[numFromTo].toNodePos = NO_RECORD;
              numFromTo++;
            }
          }
        }
//
//  Did we get any?
//
        if(numFromTo == 0)
          continue;
//
//  Second, establish fromNodePos and toNodePos so we
//  know where to look on the tripTimes array later on
//
        for(nL = 0; nL < numFromTo; nL++)
        {
          PATTERNSKey2.ROUTESrecordID = fromTo[nL].ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
          PATTERNSKey2.directionIndex = fromTo[nL].directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = fromTo[nL].PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          pos = 0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == fromTo[nL].ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == SERVICESrecordID &&
                PATTERNS.directionIndex == fromTo[nL].directionIndex &&
                PATTERNSKey2.PATTERNNAMESrecordID == fromTo[nL].PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(PATTERNS.NODESrecordID == fromTo[nL].fromNODESrecordID &&
                    fromTo[nL].fromNodePos == NO_RECORD)
              {
                fromTo[nL].fromNodePos = pos;
              }
              else if(PATTERNS.NODESrecordID == fromTo[nL].toNODESrecordID &&
                    fromTo[nL].toNodePos == NO_RECORD)
              {
                fromTo[nL].toNodePos = pos;
                break;
              }
              pos++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          } 
        }
//
//  Build the matrix
//
        TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].fromNODESrecordID = fromNODESrecordID; // nodeList[nI];
        TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].toNODESrecordID = toNODESrecordID; // nodeList[nJ];
        TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips = 
              (TMTRIPSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TMTRIPSDef) * 50); 
        if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
          return;
        }
        TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc = 50;
        TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips = 0;
        for(nL = 0; nL < numFromTo; nL++)
        {
          TRIPSKey1.ROUTESrecordID = fromTo[nL].ROUTESrecordID;
          TRIPSKey1.SERVICESrecordID = SERVICESrecordID;
          TRIPSKey1.directionIndex = fromTo[nL].directionIndex;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == fromTo[nL].ROUTESrecordID &&
                TRIPS.SERVICESrecordID == SERVICESrecordID &&
                TRIPS.directionIndex == fromTo[nL].directionIndex)
          {
            if(TRIPS.PATTERNNAMESrecordID == fromTo[nL].PATTERNNAMESrecordID)
            {
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
              nM = TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips;
              TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].TRIPSrecordID = TRIPS.recordID;
              TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[0] = GTResults.tripTimes[fromTo[nL].fromNodePos];
              TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[1] = GTResults.tripTimes[fromTo[nL].toNodePos];
              TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips++;
              if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips >= TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc)
              {
                TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc += 50;
                TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips = 
                      (TMTRIPSDef *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips,
                      sizeof(TMTRIPSDef) * TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc); 
                if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips == NULL)
                {
                  AllocationError(__FILE__, __LINE__, TRUE);
                  return;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  Pass 2 - check on pullouts and pullins
//
          rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          while(rcode2 == 0)
          {
//
//  Pullout
//
            if(TRIPS.SERVICESrecordID == SERVICESrecordID &&
                  TRIPS.standard.POGNODESrecordID == fromNODESrecordID)
            {
              PATTERNSKey1.NODESrecordID = toNODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              bFound = FALSE;
              while(rcode2 == 0 && PATTERNS.NODESrecordID == toNODESrecordID)
              {
                if(PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                      PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                      PATTERNS.directionIndex == TRIPS.directionIndex &&
                      PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                {
                  bFound = TRUE;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              }
              if(bFound)
              {
                PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
                PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
                PATTERNSKey2.directionIndex = TRIPS.directionIndex;
                PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                PATTERNSKey2.nodeSequence = NO_RECORD;
                rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                pos = 0;
                while(rcode2 == 0 &&
                      PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                      PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                      PATTERNS.directionIndex == TRIPS.directionIndex &&
                      PATTERNSKey2.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                {
                  if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                  {
                    if(PATTERNS.NODESrecordID == toNODESrecordID)
                      break;
                    pos++;
                  }
                  rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                } 
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
                GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                GCTData.timeOfDay = GTResults.firstNodeTime;
                deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
                if(deadheadTime != NO_RECORD)
                {
                  nM = TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].TRIPSrecordID = TRIPS.recordID;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[0] = GTResults.firstNodeTime - deadheadTime;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[1] = GTResults.tripTimes[pos];
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips++;
                  if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips >= TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc)
                  {
                    TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc += 50;
                    TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips = 
                          (TMTRIPSDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips,
                          sizeof(TMTRIPSDef) * TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc); 
                    if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, FALSE);
                      return;
                    }
                  }
                }
              }
            }
//
//  Pullin
//
            if(TRIPS.SERVICESrecordID == SERVICESrecordID &&
                  TRIPS.standard.PIGNODESrecordID == toNODESrecordID)
            {
              PATTERNSKey1.NODESrecordID = fromNODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              bFound = FALSE;
              while(rcode2 == 0 && PATTERNS.NODESrecordID == fromNODESrecordID)
              {
                if(PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                      PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                      PATTERNS.directionIndex == TRIPS.directionIndex &&
                      PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                {
                  bFound = TRUE;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              }
              if(bFound)
              {
                PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
                PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
                PATTERNSKey2.directionIndex = TRIPS.directionIndex;
                PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                PATTERNSKey2.nodeSequence = NO_RECORD;
                rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                pos = 0;
                while(rcode2 == 0 &&
                      PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                      PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                      PATTERNS.directionIndex == TRIPS.directionIndex &&
                      PATTERNSKey2.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                {
                  if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                  {
                    if(PATTERNS.NODESrecordID == fromNODESrecordID)
                      break;
                    pos++;
                  }
                  rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                } 
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                GCTData.timeOfDay = GTResults.lastNodeTime;
                deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
                if(deadheadTime != NO_RECORD)
                {
                  nM = TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].TRIPSrecordID = TRIPS.recordID;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[0] = GTResults.tripTimes[pos];
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips[nM].tripTime[1] = GTResults.lastNodeTime + deadheadTime;
                  TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips++;
                  if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].numTrips >= TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc)
                  {
                    TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc += 50;
                    TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips = 
                          (TMTRIPSDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips,
                          sizeof(TMTRIPSDef) * TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].currentAlloc); 
                    if(TRAVELMATRIX[serviceIndex][numInTM[serviceIndex]].pTrips == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, FALSE);
                      return;
                    }
                  }
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          }
        }// nL
        numInTM[serviceIndex]++;
      }  // nK
    }    // nJ
  }      // nI
  if(bDisplayStatus)
  {
    StatusBar((long)numNodes, (long)numNodes);
    StatusBarEnd();
  }
}


long GetDynamicTravelTime(BOOL bBeAtToBeforeCurrentTime, long fromNODESrecordID,
      long toNODESrecordID, long SERVICESrecordID, long currentTime, long *waitTime)
{
  typedef struct BESTStruct
  {
    long TRIPSrecordID;
    long cushionTime;
    long dwellTime;
    long tripTime[2];
    long fromNODESrecordID;
    long toNODESrecordID;
  } BESTDef;
  BESTDef best;
  BOOL bSuccess;
  long fromEquiv[UPPERLIMIT];
  long toEquiv[UPPERLIMIT];
  long NODESrecordID;
  long bestTime;
  long cushionTime;
  long dwellTime;
  long actualFromNODESrecordID;
  long actualToNODESrecordID;
  int  TMIndex;
  int  numFromEquiv;
  int  numToEquiv;
  int  nI;
  int  nJ;
  int  serviceIndex;

//
//  Establish serviceIndex
//
  SERVICESKey0.recordID = SERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  serviceIndex = SERVICES.number - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Get the node equivaelnces from the NODEEQUIV structure
//
  numFromEquiv = 0;
  numToEquiv = 0;
  for(nI = 0; nI < numNodeEquiv; nI++)
  {
    if(NODEEQUIV[nI].fromNODESrecordID == fromNODESrecordID)
      fromEquiv[numFromEquiv++] = NODEEQUIV[nI].toNODESrecordID;
    else if(NODEEQUIV[nI].toNODESrecordID == fromNODESrecordID)
      fromEquiv[numFromEquiv++] = NODEEQUIV[nI].fromNODESrecordID;
    else if(NODEEQUIV[nI].fromNODESrecordID == toNODESrecordID)
      toEquiv[numToEquiv++] = NODEEQUIV[nI].toNODESrecordID;
    else if(NODEEQUIV[nI].toNODESrecordID == toNODESrecordID)
      toEquiv[numToEquiv++] = NODEEQUIV[nI].fromNODESrecordID;
  }
//
//  Find fromNODESrecordID and toNODESrecordID in the travel matrix
//
  bSuccess = FALSE;
  bestTime = 86400L;
  for(nI = 0; nI < numInTM[serviceIndex]; nI++)
  {
    for(bSuccess = FALSE, nJ = 0; nJ < numFromEquiv + 1; nJ++)
    {
      NODESrecordID = nJ == 0 ? fromNODESrecordID : fromEquiv[nJ - 1];
      if(TRAVELMATRIX[serviceIndex][nI].fromNODESrecordID == NODESrecordID)
      {
        bSuccess = TRUE;
        actualFromNODESrecordID = NODESrecordID;
        break;
      }
    }
    if(!bSuccess)
      continue;
    for(bSuccess = FALSE, nJ = 0; nJ < numToEquiv + 1; nJ++)
    {
      NODESrecordID = nJ == 0 ? toNODESrecordID : toEquiv[nJ - 1];
      if(TRAVELMATRIX[serviceIndex][nI].toNODESrecordID == NODESrecordID)
      {
        bSuccess = TRUE;
        actualToNODESrecordID = NODESrecordID;
        TMIndex = nI;
        break;
      }
    }
    if(bSuccess)
    {
      for(nJ = 0; nJ < TRAVELMATRIX[serviceIndex][TMIndex].numTrips; nJ++)
      {
        bSuccess = FALSE;
        if(bBeAtToBeforeCurrentTime)
        {
          if(TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[1] <= currentTime)
          {
            bSuccess = TRUE;
            dwellTime = currentTime - TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[1];
          }
        }
        else
        {
          if(TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[0] >= currentTime)
          {
            bSuccess = TRUE;
            dwellTime = TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[0] - currentTime;
          }
        }
        if(bSuccess)
        {
          cushionTime = TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[1] - TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[0];
          if(bestTime > dwellTime + cushionTime)
          {
            bestTime = dwellTime + cushionTime;
            best.TRIPSrecordID = TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].TRIPSrecordID;
            best.dwellTime = dwellTime;
            best.cushionTime = cushionTime;
            best.tripTime[0] = TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[0];
            best.tripTime[1] = TRAVELMATRIX[serviceIndex][TMIndex].pTrips[nJ].tripTime[1];
            best.fromNODESrecordID = actualFromNODESrecordID;
            best.toNODESrecordID = actualToNODESrecordID;
          }
        }
      }
    }
  }
//
//  If the best time is all day, return no time
//
  if(bestTime == 86400L)
  {
    return(NO_TIME);
  }
//
//  Put the travel Instructions into TIData
//
  else
  {
    if(bGenerateTravelInstructions)
    {
      TRIPSKey0.recordID = best.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      TIData[numTravelInstructions].dwellTime = best.dwellTime;
      TIData[numTravelInstructions].fromNODESrecordID = best.fromNODESrecordID;
      TIData[numTravelInstructions].fromTime = best.tripTime[0];
      TIData[numTravelInstructions].ROUTESrecordID = TRIPS.ROUTESrecordID;
      TIData[numTravelInstructions].DIRECTIONSrecordID = 
            ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
      TIData[numTravelInstructions].toNODESrecordID = best.toNODESrecordID;
      TIData[numTravelInstructions].toTime = best.tripTime[1];
      numTravelInstructions++;
    }
  }
//
//  All done
//
  *waitTime = best.dwellTime;
  return(bestTime);
}
