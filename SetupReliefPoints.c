//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>
//
//  Set up the m_pBLOCKDATA and m_pRELIEFPOINTS lists
//

void AddReliefPointToList(NODESDef *pNODES, GenerateTripDef *pGTResults,
      int index, long TRIPSrecordID, long blockNumber, BOOL bSecure);

int SetUpReliefPoints(HWND hWnd, PDISPLAYINFO pDI,
      int idBLOCKS, int numTabs, int tabPosZero, BOOL bStatusBar)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  BLOCKDATADef BLOCKDATA;
  HANDLE  hCtlBLOCKS;
  float   distance;
  long    PODeadheadTime;
  long    PIDeadheadTime;
  long    thisBlock;
  long    thisSGRP;
  long    thisRGRP;
  long    thisTime;
  long    previousRGRP;
  long    previousSGRP;
  long    previousBlockNumber;
  long    previousLastTime;
  long    lastTRIPSrecordID;
  long    serviceIndex;
  long    equivalentTravelTime;
  long    previousNODESrecordID;
  char    previousNodeName[NODES_ABBRNAME_LENGTH + 1];
  long    previousTRIPSrecordID;
  long    previousTime;
  char    RGRPNumber[ROUTES_NUMBER_LENGTH + 1];
  char    RGRPName[ROUTES_NAME_LENGTH + 1];
  char    SGRPName[SERVICES_NAME_LENGTH + 1];
  char    errorString[128];
  char    nodeName[NODES_ABBRNAME_LENGTH + 1];
  BOOL    bDisplay;
  BOOL    bSame;
  BOOL    bGotNextTrip;
  BOOL    bDidNext;
  int     numNodes;
  int     rcode2;
  int     entryPoint;
  int     exitPoint;
  int     reliefPointIndex;
  int     nI;
  int     nJ;
  int     nK;
  int     nL;
  BLOCKSDef *pTRIPSChunk;
  int keyNumber;

  if((bDisplay = idBLOCKS != (BOOL)NULL) == TRUE)
  {
    hCtlBLOCKS = GetDlgItem(hWnd, idBLOCKS);
  }
//
//  Take the selected blocks
//
  m_numBLOCKDATA = 0;
  if(bStatusBar)
  {
    StatusBarStart(NULL, "Setup in Progress");
    StatusBarText("Examining selected blocks");
    StatusBar(-1L, -1L);
  }
  for(nI = 0; nI < m_numPICKEDBLOCKS; nI++)
  {
    keyNumber = (m_pPICKEDBLOCKS[nI].flags & PICKEDBLOCKS_FLAG_STANDARD) ? 2 : 3;
    pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
    TRIPSKey2.assignedToNODESrecordID = m_pPICKEDBLOCKS[nI].NODESrecordID;
    TRIPSKey2.RGRPROUTESrecordID = m_pPICKEDBLOCKS[nI].RGRPROUTESrecordID;
    TRIPSKey2.SGRPSERVICESrecordID = m_pPICKEDBLOCKS[nI].SGRPSERVICESrecordID;
    TRIPSKey2.blockNumber = NO_RECORD;
    TRIPSKey2.blockSequence = NO_TIME;
    previousRGRP = NO_RECORD;
    previousSGRP = NO_RECORD;
    previousBlockNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
    while(rcode2 == 0 &&
          pTRIPSChunk->assignedToNODESrecordID == m_pPICKEDBLOCKS[nI].NODESrecordID &&
          pTRIPSChunk->RGRPROUTESrecordID == m_pPICKEDBLOCKS[nI].RGRPROUTESrecordID &&
          pTRIPSChunk->SGRPSERVICESrecordID == m_pPICKEDBLOCKS[nI].SGRPSERVICESrecordID)
    {
      if(pTRIPSChunk->RGRPROUTESrecordID != previousRGRP ||
            pTRIPSChunk->SGRPSERVICESrecordID != previousSGRP ||
            pTRIPSChunk->blockNumber != previousBlockNumber)
      {
        BLOCKDATA.RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
        BLOCKDATA.SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
        BLOCKDATA.firstTRIPSrecordID = TRIPS.recordID;
        BLOCKDATA.blockNumber = pTRIPSChunk->blockNumber;
        BLOCKDATA.flags = m_pPICKEDBLOCKS[nI].flags;
        previousRGRP = pTRIPSChunk->RGRPROUTESrecordID;
        previousSGRP = pTRIPSChunk->SGRPSERVICESrecordID;
        previousBlockNumber = pTRIPSChunk->blockNumber;
      }
      lastTRIPSrecordID = TRIPS.recordID;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      if(rcode2 != 0 ||
            pTRIPSChunk->RGRPROUTESrecordID != previousRGRP ||
            pTRIPSChunk->SGRPSERVICESrecordID != previousSGRP ||
            pTRIPSChunk->blockNumber != previousBlockNumber)
      {
        if(m_numBLOCKDATA >= m_maxBLOCKDATA)
        {
          m_maxBLOCKDATA += 128;
          m_pBLOCKDATA = (BLOCKDATADef *)HeapReAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, m_pBLOCKDATA, sizeof(BLOCKDATADef) * m_maxBLOCKDATA); 
          if(m_pBLOCKDATA == NULL)
          {
            AllocationError(__FILE__, __LINE__, TRUE);
            break;
          }
        }
        m_pBLOCKDATA[m_numBLOCKDATA].RGRPROUTESrecordID = BLOCKDATA.RGRPROUTESrecordID;
        m_pBLOCKDATA[m_numBLOCKDATA].SGRPSERVICESrecordID = BLOCKDATA.SGRPSERVICESrecordID;
        m_pBLOCKDATA[m_numBLOCKDATA].firstTRIPSrecordID = BLOCKDATA.firstTRIPSrecordID;
        m_pBLOCKDATA[m_numBLOCKDATA].blockNumber = BLOCKDATA.blockNumber;
        m_pBLOCKDATA[m_numBLOCKDATA].lastTRIPSrecordID = lastTRIPSrecordID;
        m_pBLOCKDATA[m_numBLOCKDATA].flags = BLOCKDATA.flags;
        m_numBLOCKDATA++;
      }
    }
  }
//
//  Were there any
//
  if(m_numBLOCKDATA == 0)
  {
    MessageBeep(MB_ICONSTOP);
    LoadString(hInst, ERROR_082, szarString, sizeof(szarString));
    MessageBox(hWnd, szarString, TMS, MB_ICONSTOP);
    m_numRELIEFPOINTS = NO_RECORD;
    goto done;
  }
//
//  Well, we're here, so there must have been
//
//  Initialize the number of potential relief points
//
  m_numRELIEFPOINTS = 0;
//
//  Set up the index into the node labels string
//
  serviceIndex = m_ServiceNumber - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Loop through the structure and set up the block data list box and the relief points
//
  m_bEstablishRUNTIMES = TRUE;
  if(bStatusBar)
  {
    StatusBarText("Building relief point list");
  }
  for(nI = 0; nI < m_numBLOCKDATA; nI++)
  {
    if(bStatusBar)
    {
      StatusBar((long)nI, (long)m_numBLOCKDATA);
    }
    keyNumber = (m_pBLOCKDATA[nI].flags & PICKEDBLOCKS_FLAG_STANDARD) ? 2 : 3;
    pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
    TRIPSKey0.recordID = m_pBLOCKDATA[nI].firstTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
    thisBlock = pTRIPSChunk->blockNumber;
    thisRGRP = pTRIPSChunk->RGRPROUTESrecordID;
    thisSGRP = pTRIPSChunk->SGRPSERVICESrecordID;
    m_pBLOCKDATA[nI].onLocNODESrecordID = NO_RECORD;
    reliefPointIndex = m_numRELIEFPOINTS;
//
//  Loop through all of the trips on the selected block
//
    bGotNextTrip = FALSE;
    while(rcode2 == 0 &&
          pTRIPSChunk->blockNumber == thisBlock &&
          pTRIPSChunk->RGRPROUTESrecordID == thisRGRP &&
          pTRIPSChunk->SGRPSERVICESrecordID == thisSGRP)
    {
      if(!bGotNextTrip)
      {
        numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID, TRIPS.timeAtMLP,
              GENERATETRIP_FLAG_GETLABELS | GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
        previousLastTime = GTResults.lastNodeTime;
      }
//
//  Check on a pull-out
//
      if(pTRIPSChunk->POGNODESrecordID != NO_RECORD)
      {
        if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
        {
          m_maxRELIEFPOINTS += 1024;
          m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
          if(m_pRELIEFPOINTS == NULL)
          {
            AllocationError(__FILE__, __LINE__, TRUE);
            break;
          }
        }
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
        GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
        GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
        GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
        GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
        GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
        GCTData.timeOfDay = GTResults.firstNodeTime;
        PODeadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        if(PODeadheadTime == NO_TIME)
        {
          MessageBeep(MB_ICONINFORMATION);
          LoadString(hInst, ERROR_179, szarString, sizeof(szarString));
          sprintf(tempString, szarString, pTRIPSChunk->blockNumber);
          MessageBox(hWnd, tempString, TMS, MB_OK | MB_ICONINFORMATION);
        }
        else
        {
          thisTime = GTResults.firstNodeTime - PODeadheadTime;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE | RPFLAGS_GARAGE;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = pTRIPSChunk->POGNODESrecordID;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = thisTime;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPS.recordID;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = pTRIPSChunk->blockNumber;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
          m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = NO_RECORD;
          strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName,
                NodeAbbrFromRecID(pTRIPSChunk->POGNODESrecordID));
          m_numRELIEFPOINTS++;
          if(m_pBLOCKDATA[nI].onLocNODESrecordID == NO_RECORD)
          {
            m_pBLOCKDATA[nI].onLocNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            m_pBLOCKDATA[nI].onTime = thisTime;
          }
        }
      }
//
//  Before we look at the trip for a relief point, there's the possibility that this block
//  didn't pull out.  If this is the case, then we'll create a relief point at the first node
//  of the first trip - even if the node isn't labelled as relief point.  This lets us out
//  of the situation where a portion of a trip wouldn't be cut if the first relief point
//  occured after the first node.
//
      if(m_pBLOCKDATA[nI].onLocNODESrecordID == NO_RECORD)
      {
        m_pBLOCKDATA[nI].onLocNODESrecordID = GTResults.firstNODESrecordID;
        m_pBLOCKDATA[nI].onTime = GTResults.firstNodeTime;
      }
//
//  Set up the offLocNODESrecordID here.  This will change through each trip that's generated,
//  so when it's finally gone through the block, we'll be left with the last one.
//  Following the same line of thought above, we'll set it to the last node on the trip,
//  just in case the block doesn't pull in, and so there's no possibility of chopping out
//  a portion of the last trip.
//
      m_pBLOCKDATA[nI].offLocNODESrecordID = GTResults.lastNODESrecordID;
      m_pBLOCKDATA[nI].offTime = GTResults.lastNodeTime;
//
//  Check on the nodes on the trip
//
      bDidNext = FALSE;
      for(nK = 0; nK < numNodes; nK++)
      {
/*
JERUSALEM
//
//  Don't set out cut points on this trip if it pulls out or pulls in
//
        if(pTRIPSChunk->POGNODESrecordID != NO_RECORD ||
              pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
        {
          break;
        }
*/
//
//  Skip through if we've already done the next node
//
        if(bDidNext)
        {
          bDidNext = FALSE;
          continue;
        }
//
//  Skip through if we read ahead to the next trip
//
        if(bGotNextTrip)
        {
          bGotNextTrip = FALSE;
          continue;
        }

//
//  Is this node labelled and set for consideration?
//
        if(!GTResults.bNodeIsAGarage[nK])
        {
          if(!CheckNodeLabel(GTResults.labelledNODESrecordIDs[nK], &NODES, (int)serviceIndex))
          {
            continue;
          }
        }
        else
        {
          NODESKey0.recordID = GTResults.labelledNODESrecordIDs[nK];
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        }
        thisTime = GTResults.tripTimes[nK];
//
//  Don't cut mid trip (if the flag is set)
//
        if((CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTMIDTRIP) &&
              thisTime != GTResults.firstNodeTime && thisTime != GTResults.lastNodeTime)
        {
          continue;
        }
/*
JERUSALEM
//
//  Don't cut at the first timepoint of a trip
//
        if(thisTime == GTResults.firstNodeTime)
        {
          continue;
        }
*/
//
//  Get rid of identical relief times
//
        if(m_numRELIEFPOINTS != 0 && 
              (NodesEquivalent(m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].NODESrecordID,
                     NODES.recordID, &equivalentTravelTime) &&
               m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].time == thisTime &&
               m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].blockNumber == pTRIPSChunk->blockNumber))
        {
          continue;
        }
//
//  Get rid of "arrival" reliefs (if so requested)
//
        if((CUTPARMS.flags & CUTPARMSFLAGS_IGNORE) && m_numRELIEFPOINTS != 0 && nK == 0 &&
              !(m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].flags & RPFLAGS_GARAGE) &&
                    (NodesEquivalent(m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].NODESrecordID,
                          NODES.recordID, &equivalentTravelTime) &&
                     m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].time +
                          (CUTPARMS.ignoreMinutes * 60) > thisTime &&
                     m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].blockNumber == pTRIPSChunk->blockNumber))
        {
          m_numRELIEFPOINTS--;
        }
//
//  Get rid of "departure" reliefs (if so requested)
//
        else if((CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTAT2) && m_numRELIEFPOINTS != 0 && nK == 0 &&
              !(m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].flags & RPFLAGS_GARAGE) &&
                    (NodesEquivalent(m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].NODESrecordID,
                           NODES.recordID, &equivalentTravelTime) &&
                     m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].time +
                           (CUTPARMS.ignoreMinutes * 60) > thisTime &&
                     m_pRELIEFPOINTS[m_numRELIEFPOINTS - 1].blockNumber == pTRIPSChunk->blockNumber))
        {
          continue;
        }
//
//  Add the relief point in
//
        if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
        { 
          m_maxRELIEFPOINTS += 1024;
          m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
          if(m_pRELIEFPOINTS == NULL)
          {
            AllocationError(__FILE__, __LINE__, TRUE);
            break;
          }
        }
//
//  If this isn't a secure location, just add it in.
//
        if(!(NODES.flags & NODES_FLAG_SECURE))
        {
          AddReliefPointToList(&NODES, &GTResults, nK, TRIPS.recordID, pTRIPSChunk->blockNumber, FALSE);
        }
//
//  It is a secure location
//
        else
        {
//
//  Is this the last node on the trip?
//
          if(nK == numNodes - 1)
          {
//
//  Does the trip pull in?  If so, add this secure location as a regular relief.
//
            if(pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
            {
              AddReliefPointToList(&NODES, &GTResults, nK, TRIPS.recordID, pTRIPSChunk->blockNumber, FALSE);
            }
//
//  No pull-in on this trip.  Get the next trip to see if this is a regular/secure relief.
//  Of course, the trip have to be one the same block and route/service group.
//
            else
            {
              strncpy(previousNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(previousNodeName, NODES_ABBRNAME_LENGTH);
              previousNODESrecordID = NODES.recordID;
              previousTRIPSrecordID = TRIPS.recordID;
              previousTime = thisTime;
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              bGotNextTrip = TRUE;
              if(rcode2 == 0 &&
                    pTRIPSChunk->blockNumber == thisBlock &&
                    pTRIPSChunk->RGRPROUTESrecordID == thisRGRP &&
                    pTRIPSChunk->SGRPSERVICESrecordID == thisSGRP)
              {
                numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID, TRIPS.timeAtMLP,
                      GENERATETRIP_FLAG_GETLABELS | GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  The sequence is just too weird to use AddReliefPointToList,
//  so we'll just do all the dirty work here.  If the last node
//  of the previous trip is equivalent to the first node of the 
//  next trip and the last node of the previous trip was deemed
//  secure, then there's a pair.
//
                if(NodesEquivalent(previousNODESrecordID,
                      GTResults.firstNODESrecordID, &equivalentTravelTime) &&
                            GTResults.firstNodeTime - previousLastTime > 1800)  // It has to be 30 minutes or more apart
                {
                  if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
                  {
                    m_maxRELIEFPOINTS += 1024;
                    m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
                    if(m_pRELIEFPOINTS == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, TRUE);
                      break;
                    }
                  }
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = previousNODESrecordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = previousTime;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = previousTRIPSrecordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = thisBlock;
                  strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, previousNodeName);
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = SECURELOCATION_RUNNUMBER;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = 1;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags |= RPFLAGS_SECUREPAIR;
                  m_numRELIEFPOINTS++;
//
//  The next
//
                  if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
                  {
                    m_maxRELIEFPOINTS += 1024;
                    m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
                    if(m_pRELIEFPOINTS == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, TRUE);
                      break;
                    }
                  }
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = GTResults.firstNODESrecordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = GTResults.firstNodeTime;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPS.recordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = thisBlock;
                  if(previousNODESrecordID == GTResults.firstNODESrecordID)
                  {
                    strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, previousNodeName);
                  }
                  else
                  {
                    NODESKey0.recordID = GTResults.firstNODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    trim(nodeName, NODES_ABBRNAME_LENGTH);
                    strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, nodeName);
                  }
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = SECURELOCATION_RUNNUMBER;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = 1;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags |= RPFLAGS_SECUREPAIR;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
                  m_numRELIEFPOINTS++;
                }
//
//  Ok, the node equivalence failed.  This means that we add the
//  last node of the previous trip, as well as the first node of
//  the next trip (only if that first node is a relief point).  Yikes.
//
                else
                {
                  if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
                  {
                    m_maxRELIEFPOINTS += 1024;
                    m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
                    if(m_pRELIEFPOINTS == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, TRUE);
                      break;
                    }
                  }
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = previousNODESrecordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = previousTime;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = previousTRIPSrecordID;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = thisBlock;
                  strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, previousNodeName);
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
                  m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
                  m_numRELIEFPOINTS++;
                  if(CheckNodeLabel(GTResults.labelledNODESrecordIDs[0], &NODES, (int)serviceIndex))
                  {
                    if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
                    {
                      m_maxRELIEFPOINTS += 1024;
                      m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                            HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
                      if(m_pRELIEFPOINTS == NULL)
                      {
                        AllocationError(__FILE__, __LINE__, TRUE);
                        break;
                      }
                    }
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = GTResults.firstNODESrecordID;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = GTResults.firstNodeTime;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPS.recordID;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = thisBlock;
                    if(previousNODESrecordID == GTResults.firstNODESrecordID)
                    {
                      strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, previousNodeName);
                    }
                    else
                    {
                      NODESKey0.recordID = GTResults.firstNODESrecordID;
                      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                      strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                      trim(nodeName, NODES_ABBRNAME_LENGTH);
                      strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, nodeName);
                    }
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = NO_RECORD;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
                    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
                    m_numRELIEFPOINTS++;
                  }
                }
                previousLastTime = GTResults.lastNodeTime;
              }
            }
          }
//
//  Not the last node on the trip
//
          else
          {
//
//  Is the next node the same secure location?  If so, add them both in.
//
            if(NodesEquivalent(GTResults.labelledNODESrecordIDs[nK],
                  GTResults.labelledNODESrecordIDs[nK + 1], &equivalentTravelTime))
            {
              AddReliefPointToList(&NODES, &GTResults, nK, TRIPS.recordID, pTRIPSChunk->blockNumber, TRUE);
              bDidNext = TRUE;
            }
//
//  No - the next node isn't the same secure location.  Just add this one.
//
            else
            {
              AddReliefPointToList(&NODES, &GTResults, nK, TRIPS.recordID, pTRIPSChunk->blockNumber, FALSE);
            }
          }
        }
      }
//
//  Check on a pull-in
//
      if(!bGotNextTrip)
      {
        if(pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
        {
          if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
          {
            m_maxRELIEFPOINTS += 1024;
            m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
            if(m_pRELIEFPOINTS == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
              break;
            }
          }
          GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
          GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
          GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
          GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
          GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
          GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
          GCTData.timeOfDay = GTResults.lastNodeTime;
          PIDeadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
          if(PIDeadheadTime == NO_TIME)
          {
            MessageBeep(MB_ICONINFORMATION);
            LoadString(hInst, ERROR_180, szarString, sizeof(szarString));
            sprintf(tempString, szarString, pTRIPSChunk->blockNumber);
            MessageBox(hWnd, tempString, TMS, MB_OK | MB_ICONINFORMATION);
          }
          else
          {
            thisTime = GTResults.lastNodeTime + PIDeadheadTime;
            if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
            {
              m_maxRELIEFPOINTS += 1024;
              m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
              if(m_pRELIEFPOINTS == NULL)
              {
                AllocationError(__FILE__, __LINE__, TRUE);
                break;
              }
            }
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE | RPFLAGS_GARAGE;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = thisTime;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPS.recordID;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = pTRIPSChunk->blockNumber;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
            m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
            strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName,
                  NodeAbbrFromRecID(pTRIPSChunk->PIGNODESrecordID));
            m_numRELIEFPOINTS++;
            m_pBLOCKDATA[nI].offLocNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            m_pBLOCKDATA[nI].offTime = thisTime;
          }
        }
      }
      if(!bGotNextTrip)
      {
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      }
    }
//
//  Ok, now that all of that crap has been built, let's display it
//
    if(bDisplay)
    {
      for(nJ = 0; nJ < numTabs + 1; nJ++)
      {
        if(nJ != 0)
        {
          strcat(tempString, "\t");
        }
        switch(nJ)
        {
          case 0:
            ROUTESKey0.recordID = m_pBLOCKDATA[nI].RGRPROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(RGRPNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(RGRPNumber, ROUTES_NUMBER_LENGTH);
            strncpy(RGRPName, ROUTES.name, ROUTES_NAME_LENGTH);
            trim(RGRPName, ROUTES_NAME_LENGTH);
            strcpy(tempString, RGRPNumber);
            strcat(tempString, " - ");
            strcat(tempString, RGRPName);
            trim(tempString, tabPosZero - 1);
            break;
          case 1:
            SERVICESKey0.recordID = m_pBLOCKDATA[nI].SGRPSERVICESrecordID;
            btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            strncpy(SGRPName, SERVICES.name, sizeof(SERVICES.name));
            trim(SGRPName, sizeof(SERVICES.name));
            strcat(tempString, SGRPName);
            break;
          case 2:
            sprintf(szarString, "%5d", m_pBLOCKDATA[nI].blockNumber);
            strcat(tempString, szarString);
            break;
          case 3:
            strcat(tempString, NodeAbbrFromRecID(m_pBLOCKDATA[nI].onLocNODESrecordID));
            break;
          case 4:
            strcat(tempString, Tchar(m_pBLOCKDATA[nI].onTime));
            break;
          case 5:
            strcat(tempString, Tchar(m_pBLOCKDATA[nI].offTime));
            break;
          case 6:
            strcat(tempString, NodeAbbrFromRecID(m_pBLOCKDATA[nI].offLocNODESrecordID));
            break;
        }
      }
      nL = (int)SendMessage(hCtlBLOCKS, LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlBLOCKS, LB_SETITEMDATA, nL, MAKELONG(reliefPointIndex, m_numRELIEFPOINTS));
    }
  }
//
//  Make sure all the firstInBlock and lastInBlock flags are set correctly.
//  This code assumes that all relief points for a block are contiguous
//  in the RELIEFPOINTS array.
//
  for( nK = 0; nK < m_numRELIEFPOINTS; nK++ )
  {
    m_pRELIEFPOINTS[nK].flags = RPFLAGS_INUSE;
//
//  Handle the starting case
//
    if( nK == 0 )
    {
      m_pRELIEFPOINTS[nK].flags |= RPFLAGS_FIRSTINBLOCK;
    }
//
//  Handle the ending case
//
    else if( nK == m_numRELIEFPOINTS - 1 )
    {
      m_pRELIEFPOINTS[nK].flags |= RPFLAGS_LASTINBLOCK;
    }
//
//  Handle the beginning of block case
//
    else if( m_pRELIEFPOINTS[nK-1].blockNumber != m_pRELIEFPOINTS[nK].blockNumber )
    {
      m_pRELIEFPOINTS[nK].flags |= RPFLAGS_FIRSTINBLOCK;
    }
//
//  Handle the end of block case
//
    else if( m_pRELIEFPOINTS[nK].blockNumber != m_pRELIEFPOINTS[nK+1].blockNumber )
    {
      m_pRELIEFPOINTS[nK].flags |= RPFLAGS_LASTINBLOCK;
    }
  }
//
//  Ok, all the potential relief points have been read in.  Now go through
//  any existing runs and adjust the RELIEFPOINTS structure accordingly.
//
  if(bStatusBar)
  {
    StatusBar(-1L, -1L);
  }
  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
        RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
  {
    if(bStatusBar)
    {
      sprintf(tempString, "Building run %ld", RUNS.runNumber);
      StatusBarText(tempString);
    }
//
//  Establish entry point into RELIEFPOINTS
//
    for(entryPoint = NO_RECORD, nI = 0; nI < m_numRELIEFPOINTS; nI++)
    {
      if(RUNS.start.TRIPSrecordID == m_pRELIEFPOINTS[nI].TRIPSrecordID &&
            RUNS.start.NODESrecordID == m_pRELIEFPOINTS[nI].NODESrecordID)
      {
        entryPoint = nI;
        break;
      }
    }
//
//  Not found?  Let him know.
//
    if(entryPoint == NO_RECORD)
    {
      MessageBeep(MB_ICONSTOP);
      LoadString(hInst, TEXT_067, szarString, SZARSTRING_LENGTH);
      LoadString(hInst, ERROR_149, tempString, TEMPSTRING_LENGTH);
      sprintf(errorString, tempString, RUNS.runNumber, szarString);
      MessageBox(hWnd, errorString, TMS, MB_ICONSTOP | MB_OK);
    }
//
//  Establish exit point from RELIEFPOINTS
//
    else
    {
      for(exitPoint = NO_RECORD, nI = entryPoint + 1; nI < m_numRELIEFPOINTS; nI++)
      { 
        if(RUNS.end.TRIPSrecordID == m_pRELIEFPOINTS[nI].TRIPSrecordID &&
              RUNS.end.NODESrecordID == m_pRELIEFPOINTS[nI].NODESrecordID)
        {
          exitPoint = nI;
          break;
        }
      }
//
//  Not found?  Let him know.
//
      if(exitPoint == NO_RECORD)
      {
        MessageBeep(MB_ICONSTOP);
        LoadString(hInst, TEXT_068, szarString, sizeof(szarString));
        LoadString(hInst, ERROR_149, tempString, TEMPSTRING_LENGTH);
        sprintf(errorString, tempString, RUNS.runNumber, szarString);
        MessageBox(hWnd, errorString, TMS, MB_ICONSTOP | MB_OK);
      }
//
//  Verify that the block number is the same from entryPoint to exitPoint
//
      else
      {
        thisBlock = m_pRELIEFPOINTS[entryPoint].blockNumber;
        bSame = TRUE;
        for(nI = entryPoint + 1; nI <= exitPoint; nI++)
        {
          if(m_pRELIEFPOINTS[nI].blockNumber != thisBlock)
          {
            bSame = FALSE;
            break;
          }
        }
        if(!bSame)
        {
          NODESKey0.recordID = m_pRELIEFPOINTS[nI].NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          MessageBeep(MB_ICONSTOP);
          LoadString(hInst, ERROR_279, szFormatString, sizeof(szFormatString));
          sprintf(tempString, szFormatString, thisBlock, m_pRELIEFPOINTS[nI].blockNumber,
                szarString, Tchar(m_pRELIEFPOINTS[nI].time), RUNS.runNumber);
          MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
        }
//
//  Ok, got both the entry and exit points.  Loop through RELIEFPOINTS and
//  set up the start/end run/piece numbers thusly:
//
//  If nI == entryPoint: set start run/piece numbers
//     nI == exitPoint: set end run/piece numbers
//     nI == between: set start/end run/piece numbers
//
        else
        {
          for(nI = entryPoint; nI <= exitPoint; nI++)
          {
            if(nI != exitPoint)
            {
              m_pRELIEFPOINTS[nI].start.recordID = RUNS.recordID;
              m_pRELIEFPOINTS[nI].start.runNumber = RUNS.runNumber;
              m_pRELIEFPOINTS[nI].start.pieceNumber = RUNS.pieceNumber;
              m_pRELIEFPOINTS[nI].start.prior.startTime = RUNS.prior.startTime;
              m_pRELIEFPOINTS[nI].start.prior.endTime = RUNS.prior.endTime;
              m_pRELIEFPOINTS[nI].start.after.startTime = RUNS.after.startTime;
              m_pRELIEFPOINTS[nI].start.after.endTime = RUNS.after.endTime;
              m_pRELIEFPOINTS[nI].start.flags = RUNS.flags;
            }
            if(nI != entryPoint)
            {
              m_pRELIEFPOINTS[nI].end.recordID = RUNS.recordID;
              m_pRELIEFPOINTS[nI].end.runNumber = RUNS.runNumber;
              m_pRELIEFPOINTS[nI].end.pieceNumber = RUNS.pieceNumber;
              m_pRELIEFPOINTS[nI].end.prior.startTime = RUNS.prior.startTime;
              m_pRELIEFPOINTS[nI].end.prior.endTime = RUNS.prior.endTime;
              m_pRELIEFPOINTS[nI].end.after.startTime = RUNS.after.startTime;
              m_pRELIEFPOINTS[nI].end.after.endTime = RUNS.after.endTime;
              m_pRELIEFPOINTS[nI].end.flags = RUNS.flags;
            }
          }
        }
      }
    }
//
//  Get the next run record
//
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Initialize RUNLISTDATA
//
  for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
  {
    RUNLISTDATA[nI].runNumber = NO_RECORD;
    RUNLISTDATA[nI].cutAsRuntype = NO_RUNTYPE;
    RUNLISTDATA[nI].COMMENTSrecordID = NO_RECORD;
    RUNLISTDATA[nI].frozenFlag = FALSE;
  }
  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  nI = 0;
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
        RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
        nI < MAXRUNSINRUNLIST)
  {
    if(RUNS.pieceNumber == 1)
    {
      RUNLISTDATA[nI].runNumber = RUNS.runNumber;
      RUNLISTDATA[nI].cutAsRuntype = RUNS.cutAsRuntype;
      RUNLISTDATA[nI].COMMENTSrecordID = RUNS.COMMENTSrecordID;
      nI++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Kill the status bar and leave
//
  done:
    StatusBarEnd();

  return(m_numRELIEFPOINTS);
}

BOOL CheckNodeLabel(long NODESrecordID, NODESDef *pNODES, int serviceIndex)
{
  char reliefLabels[NODES_RELIEFLABELS_LENGTH + 1];
  int  nI;
  int  rcode2;

  if(NODESrecordID != NO_RECORD)
  {
    NODESKey0.recordID = NODESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, pNODES, &NODESKey0, 0);
    if(rcode2 != 0)
    {
      return FALSE;
    }
    strncpy(reliefLabels, pNODES->reliefLabels, NODES_RELIEFLABELS_LENGTH);
    trim(reliefLabels, NODES_RELIEFLABELS_LENGTH);
    for(nI = 0; nI < (int)strlen(CUTPARMS.labels); nI++)
    {
      if(reliefLabels[serviceIndex] == CUTPARMS.labels[nI])
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

void AddReliefPointToList(NODESDef *pNODES, GenerateTripDef *pGTResults, int index, long TRIPSrecordID, long blockNumber, BOOL bSecure)
{
  char nodeName[NODES_ABBRNAME_LENGTH + 1];

  if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
  {
    m_maxRELIEFPOINTS += 1024;
    m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
          HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
    if(m_pRELIEFPOINTS == NULL)
    {
      AllocationError(__FILE__, __LINE__, TRUE);
      return;
    }
  }
//
//  Data common to both secure and non-secure locations
//
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = pNODES->recordID;
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = pGTResults->tripTimes[index];
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPSrecordID;
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = blockNumber;
  strncpy(nodeName, pNODES->abbrName, NODES_ABBRNAME_LENGTH);
  trim(nodeName, NODES_ABBRNAME_LENGTH);
  strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, nodeName);
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = NO_RECORD;
  m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = NO_RECORD;
//
//  Non-secure location
//
  if(!bSecure)
  {
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
  }
//
//  Secure location.  Do this one and the next.
//
  else
  {
//
//  This one
//
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = SECURELOCATION_RUNNUMBER;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = 1;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags |= RPFLAGS_SECUREPAIR;
//
//  The next
//
    m_numRELIEFPOINTS++;
    if(m_numRELIEFPOINTS >= m_maxRELIEFPOINTS)
    {
      m_maxRELIEFPOINTS += 1024;
      m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapReAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
      if(m_pRELIEFPOINTS == NULL)
      {
        AllocationError(__FILE__, __LINE__, TRUE);
        return;
      }
    }
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags = RPFLAGS_INUSE;
    if(pGTResults->labelledNODESrecordIDs[index] !=
          pGTResults->labelledNODESrecordIDs[index + 1])
    {
      NODESKey0.recordID = pGTResults->labelledNODESrecordIDs[index + 1];
      btrieve(B_GETEQUAL, TMS_NODES, pNODES, &NODESKey0, 0);
      strncpy(nodeName, pNODES->abbrName, NODES_ABBRNAME_LENGTH);
      trim(nodeName, NODES_ABBRNAME_LENGTH);
    }
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].NODESrecordID = pNODES->recordID;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].time = pGTResults->tripTimes[index + 1];
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].TRIPSrecordID = TRIPSrecordID;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].blockNumber = blockNumber;
    strcpy(m_pRELIEFPOINTS[m_numRELIEFPOINTS].nodeName, nodeName);
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.recordID = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.runNumber = SECURELOCATION_RUNNUMBER;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].end.pieceNumber = 1;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].flags |= RPFLAGS_SECUREPAIR;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.recordID = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.runNumber = NO_RECORD;
    m_pRELIEFPOINTS[m_numRELIEFPOINTS].start.pieceNumber = NO_RECORD;
  }

  m_numRELIEFPOINTS++;
}
