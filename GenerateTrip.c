//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  GenerateTrip.c: Populate a GenerateTripDef structure with trip times
//
//         Returns: Number of nodes in the pattern used to generate the trip
//
#include "TMSHeader.h"

int GenerateTrip(long ROUTESrecordID, long SERVICESrecordID, int directionIndex,
      long PATTERNNAMESrecordID, long timeAtMLP, long flags, GenerateTripDef *pGTResults)
{
  GetConnectionTimeDef GCTData;
  float  distance;
  BOOL bGotMLP;
  BOOL bFound;
  BOOL bZeroOutTrip;
  long patternNODESrecordIDs[MAXTRIPTIMES];
  long numNodes;
  long currentTime;
  long addTime;
  int  MLPIndex = 0;
  int  fromNode;
  int  toNode;
  int  firstNode;
  int  lastNode;
  int  rcode2;
  int  nI;

//
//  Zero out the return structure
//
  memset(pGTResults, 0x00, sizeof(GenerateTripDef));
//
//  Get the pattern nodes and establish the MLP
//
  PATTERNSKey2.ROUTESrecordID = ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
  PATTERNSKey2.directionIndex = directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMESrecordID;
  PATTERNSKey2.nodeSequence = 0;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  numNodes = 0;
  bGotMLP = FALSE;
  while(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == SERVICESrecordID &&
        PATTERNS.directionIndex == directionIndex &&
        PATTERNS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
  {
    if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
    {
      patternNODESrecordIDs[numNodes] = PATTERNS.NODESrecordID;
      if(flags & GENERATETRIP_FLAG_GETLABELS)
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(NODES.flags & NODES_FLAG_GARAGE)
        {
          pGTResults->labelledNODESrecordIDs[numNodes] = NODES.recordID;
          pGTResults->bNodeIsAGarage[numNodes] = TRUE;
        }
        else
        {
          strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
          trim(tempString, NODES_RELIEFLABELS_LENGTH);
          pGTResults->labelledNODESrecordIDs[numNodes] =
                (strcmp(tempString, "") == 0 ? NO_RECORD : NODES.recordID);
          pGTResults->bNodeIsAGarage[numNodes] = FALSE;
        }
      }
      if((PATTERNS.flags & PATTERNS_FLAG_MLP) && !bGotMLP)
      {
        MLPIndex = numNodes;
        bGotMLP = TRUE;
      }
      numNodes++;
      if(numNodes > MAXTRIPTIMES)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_282, (HANDLE)NULL);
        return(NO_RECORD);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//
//  Set up the guts of the GCTData structure
//
  GCTData.fromROUTESrecordID = ROUTESrecordID;
  GCTData.fromSERVICESrecordID = SERVICESrecordID;
  GCTData.fromPATTERNNAMESrecordID = PATTERNNAMESrecordID;
  GCTData.toROUTESrecordID = ROUTESrecordID;
  GCTData.toSERVICESrecordID = SERVICESrecordID;
  GCTData.toPATTERNNAMESrecordID = PATTERNNAMESrecordID;
//
//  Generate the trip
//
//  If a connection time distance comes back negative, it means it was calculated
//  If this is the case, change it back to positive, and set the GENERATETRIP_FLAG_DISTANCECALC
//  flag in the output structure.
//
  currentTime = timeAtMLP;
  pGTResults->tripTimes[MLPIndex] = currentTime;
  fromNode = MLPIndex;
  lastNode = fromNode;
  toNode = MLPIndex + 1;
  bZeroOutTrip = FALSE;
  while(toNode < numNodes)
  {
    GCTData.fromNODESrecordID = patternNODESrecordIDs[fromNode];
    GCTData.toNODESrecordID = patternNODESrecordIDs[toNode];
    GCTData.timeOfDay = currentTime;
    addTime = GetConnectionTime(GCT_FLAG_RUNNINGTIME, &GCTData, &distance);
    if(addTime == NO_TIME)
    {
      if(flags & GENERATETRIP_FLAG_DISPLAYERRORS)
      {
        bFound = FALSE;
        for(nI = 0; nI < m_numConnectionAlerts; nI++)
        {
          if((bFound = m_ALERT[nI].from == GCTData.fromNODESrecordID &&
                m_ALERT[nI].to == GCTData.toNODESrecordID) == TRUE)
            break;
        }
        if(!bFound && m_numConnectionAlerts < ALERTMAX)
        {
          NoConnectionErrorMessage(GCTData.fromNODESrecordID, GCTData.toNODESrecordID);
          m_ALERT[m_numConnectionAlerts].from = GCTData.fromNODESrecordID;
          m_ALERT[m_numConnectionAlerts].to = GCTData.toNODESrecordID;
          m_numConnectionAlerts++;
        }
        bZeroOutTrip = TRUE;
      }
    }
    if(bZeroOutTrip)
    {
      pGTResults->tripTimes[toNode] = NO_TIME;
    }
    else
    {
      pGTResults->tripTimes[toNode] = currentTime + addTime;
      if(distance < 0.0)
      {
        distance = -distance;
        pGTResults->returnFlags = GENERATETRIP_FLAG_DISTANCECALC;
      }
      pGTResults->tripDistance += distance;
    }
    currentTime = pGTResults->tripTimes[toNode];
    fromNode = toNode;
    lastNode = toNode;
    toNode++;
  }
//
//  Loop through trips from MLPIndex back to first node
//
  currentTime = timeAtMLP;
  fromNode = MLPIndex - 1;
  toNode = MLPIndex;
  firstNode = MLPIndex;
  bZeroOutTrip = FALSE;
  while(fromNode >= 0)
  {
    GCTData.fromNODESrecordID = patternNODESrecordIDs[fromNode];
    GCTData.toNODESrecordID = patternNODESrecordIDs[toNode];
    GCTData.timeOfDay = currentTime;
    addTime = GetConnectionTime(GCT_FLAG_RUNNINGTIME, &GCTData, &distance);
    if(addTime == NO_TIME)
    {
      if(flags & GENERATETRIP_FLAG_DISPLAYERRORS)
      {
        bFound = FALSE;
        for(nI = 0; nI < m_numConnectionAlerts; nI++)
        {
          if((bFound = m_ALERT[nI].from == GCTData.fromNODESrecordID &&
                m_ALERT[nI].to == GCTData.toNODESrecordID) == TRUE)
            break;
        }
        if(!bFound && m_numConnectionAlerts < ALERTMAX)
        {
          NoConnectionErrorMessage(GCTData.fromNODESrecordID, GCTData.toNODESrecordID);
          m_ALERT[m_numConnectionAlerts].from = GCTData.fromNODESrecordID;
          m_ALERT[m_numConnectionAlerts].to = GCTData.toNODESrecordID;
          m_numConnectionAlerts++;
        }
        bZeroOutTrip = TRUE;
      }
    }
    if(bZeroOutTrip)
    {
      pGTResults->tripTimes[toNode] = NO_TIME;
    }
    else
    {
      pGTResults->tripTimes[fromNode] = currentTime - addTime;
      if(distance < 0.0)
      {
        distance = -distance;
        pGTResults->returnFlags = GENERATETRIP_FLAG_DISTANCECALC;
      }
      pGTResults->tripDistance += distance;
    }
    currentTime = pGTResults->tripTimes[fromNode];
    toNode = fromNode;
    firstNode = fromNode;
    fromNode--;
  }
//
//  All done
//
  pGTResults->firstNODESrecordID = patternNODESrecordIDs[firstNode];
  pGTResults->firstNodeTime = pGTResults->tripTimes[firstNode];
  pGTResults->lastNODESrecordID = patternNODESrecordIDs[lastNode];
  pGTResults->lastNodeTime = pGTResults->tripTimes[lastNode];

  return(numNodes);
}

long GetConnectionTime(long flags, GetConnectionTimeDef *pGCTData, float *pDistance)
{
  BOOL  bRunningTime;
  BOOL  bDeadheadTime;
  BOOL  bTravelTime;
  BOOL  bAtLeastOneStop;
  int   nI;
  int   rcode2;
  long  fromNode;
  long  toNode;
  long  time;
  float longitude[2];
  float latitude[2];
  BOOL  bFound;
  int   numToCheck = 0;
  int   startPos = NO_RECORD;

  bRunningTime = flags & GCT_FLAG_RUNNINGTIME;
  bDeadheadTime = flags & GCT_FLAG_DEADHEADTIME;
  bTravelTime = flags & GCT_FLAG_TRAVELTIME;
  bAtLeastOneStop = flags & GCT_FLAG_ATLEASTONESTOP;
//
//  If we're dealing with a pair that is comprised of at least one stop
//  do the lookup directly, as opposed to going through the data structure.
//
//  Here, we're not actually looking for a time, we're after a distance
//
  *pDistance = 0.0;
  if(bAtLeastOneStop)
  {
    bFound = FALSE;
    for(nI = 0; nI < 2; nI++)
    {
      if(nI == 0)
      {
        fromNode = pGCTData->fromNODESrecordID;
        toNode = pGCTData->toNODESrecordID;
      }
      else
      {
        if(pGCTData->toNODESrecordID == pGCTData->fromNODESrecordID)
          break;
        fromNode = pGCTData->toNODESrecordID;
        toNode = pGCTData->fromNODESrecordID;
      }
//
//  Get the connections record
//
      CONNECTIONSKey1.fromNODESrecordID = fromNode;
      CONNECTIONSKey1.toNODESrecordID = toNode;
      CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      if(rcode2 == 0 && CONNECTIONS.fromNODESrecordID == fromNode && CONNECTIONS.toNODESrecordID == toNode)
      {
        if(CONNECTIONS.distance != NO_RECORD && CONNECTIONS.distance != 0.0)
        {
          *pDistance = CONNECTIONS.distance;
          bFound = TRUE;
          break;
        }
      }
    }
//
//  No record found or no distance on the record - can we calculate the distance?
//  
//  If we can, set it to negative to show is was calculated
//
    if(!bFound)
    {
      NODESKey0.recordID = pGCTData->fromNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      longitude[0] = NODES.longitude;
      latitude[0] = NODES.latitude;
      if(longitude[0] != 0.0 && latitude[0] != 0.0)
      {
        NODESKey0.recordID = pGCTData->toNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        longitude[1] = NODES.longitude;
        latitude[1] = NODES.latitude;
        if(longitude[1] != 0.0 && latitude[1] != 0.0)
        {
          *pDistance = -(float)GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
        }
      }
    }
    return(NO_TIME);
  }
//
//  Are we establishing runtimes?
//
  bFound = FALSE;
  if(m_bEstablishRUNTIMES)
  {
    m_numRUNTIMES = 0;
    m_bEstablishRUNTIMES = FALSE;
  }
//
//  No?  Look for the first entry in the table, and then the last
//
  else
  {
    for(nI = 0; nI < m_numRUNTIMES; nI++)
    {
      if((m_pRUNTIMES[nI].fromNODESrecordID == pGCTData->fromNODESrecordID &&
            m_pRUNTIMES[nI].toNODESrecordID == pGCTData->toNODESrecordID) ||
         (m_pRUNTIMES[nI].fromNODESrecordID == pGCTData->toNODESrecordID &&
            m_pRUNTIMES[nI].toNODESrecordID == pGCTData->fromNODESrecordID))
      {
        if(!bFound)
        {
          bFound = TRUE;
          startPos = nI;
        }
      }
      else
      {
        if(bFound)
        {
          numToCheck = nI - startPos;
          break;
        }
      }
    }
    if(bFound && numToCheck == 0)
      numToCheck = m_numRUNTIMES - startPos;
  }
//
//  We're establishing the list or the connection wasn't there...
//  Read in all of the potential connections - From -> To and To -> From
//
  if(!bFound)
  {
    startPos = m_numRUNTIMES;
    for(nI = 0; nI < 2; nI++)
    {
      if(nI == 0)
      {
        fromNode = pGCTData->fromNODESrecordID;
        toNode = pGCTData->toNODESrecordID;
      }
      else
      {
        if(pGCTData->toNODESrecordID == pGCTData->fromNODESrecordID)
          break;
        fromNode = pGCTData->toNODESrecordID;
        toNode = pGCTData->fromNODESrecordID;
      }
      CONNECTIONSKey1.fromNODESrecordID = fromNode;
      CONNECTIONSKey1.toNODESrecordID = toNode;
      CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      while(rcode2 == 0 &&
            CONNECTIONS.fromNODESrecordID == fromNode &&
            CONNECTIONS.toNODESrecordID == toNode)
      {
        if(CONNECTIONS.distance == NO_RECORD || CONNECTIONS.distance == 0.0)
        {
          CONNECTIONS.distance = 0.0;
          NODESKey0.recordID = pGCTData->fromNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          longitude[0] = NODES.longitude;
          latitude[0] = NODES.latitude;
          if(longitude[0] != 0.0 && latitude[0] != 0.0)
          {
            NODESKey0.recordID = pGCTData->toNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            longitude[1] = NODES.longitude;
            latitude[1] = NODES.latitude;
            if(longitude[1] != 0.0 && latitude[1] != 0.0)
            {
              CONNECTIONS.distance = -(float)GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
            }
          }
        }
        memcpy(&m_pRUNTIMES[m_numRUNTIMES++], &CONNECTIONS, sizeof(CONNECTIONS));
        numToCheck++;
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      }
    }
  }
//
//  Leave if there are no connections
//
  if(numToCheck == 0)
    return(pGCTData->fromNODESrecordID == pGCTData->toNODESrecordID ? 0L : NO_TIME);
  else
  {
    time = GetMostAppropriateConnectionTime(startPos, numToCheck, flags, pGCTData, pDistance);
    if(time == NO_TIME && pGCTData->fromNODESrecordID == pGCTData->toNODESrecordID)
    {
      return(0);
    }
    else
    {
      return(time);
    }
  }
}
//
// GetMostAppropriateConnectionTime - Get the appropriate running time
//
// Connection time examination matrix
//
// *  Have read all FROM-TO records into core.
//
// Establish FNode - Looping through incore records
// ~~~~~~~~~~~~~~~
// a) -> FNod didn't match.  If 2way, flip input and try b)
// b) -> FNod matched
//
// Once a test has succeeded, continue through the rest of the incore
// records, keeping track of those that also pass.
//
// Weight     4    8   16    2    Score
//      FNod FRte FSer FPat FTTi
//  1.   X    X    X    X    X     30
//  2.   X    X    X    X          28
//  3.   X    X    X         X     14
//  4.   X    X    X               12
//  5.   X         X         X     10
//  6.   X         X                8
//  7.   X    X              X      6
//  8.   X    X                     4
//  9.   X                   X      2
// 10.   X                          0
//
// On those records that passed the test, perform the 1-8 and 9-16
// traversal again, but compare TNod TRte TSer TPat FTTi instead.
//
long GetMostAppropriateConnectionTime(int startPos,
      int numToCheck, long flags, GetConnectionTimeDef *pGCTData, float *pDistance)
{
  GetConnectionTimeDef originalGCT;
  BOOL  bRunningTime;
  BOOL  bDeadheadTime;
  BOOL  bTravelTime;
  BOOL  bTestTOD;
  long  tempArray[4];
  int   nI;
  int   highest;
  int   score[MAXCONNECTIONS];

  bRunningTime = flags & GCT_FLAG_RUNNINGTIME;
  bDeadheadTime = flags & GCT_FLAG_DEADHEADTIME;
  bTravelTime = flags & GCT_FLAG_TRAVELTIME;

  memcpy(&originalGCT, pGCTData, sizeof(GetConnectionTimeDef));
//
//  Check out the from side first
//
  bTestTOD = pGCTData->timeOfDay >= 86400L;
  if(bTestTOD)
  {
    pGCTData->timeOfDay -= 86400L;
  }
  for(nI = startPos; nI < startPos + numToCheck; nI++)
  {
    score[nI] = NO_RECORD;
    if((bRunningTime && !(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_RUNNINGTIME)) ||
          (bTravelTime && !(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_TRAVELTIME)) ||
          (bDeadheadTime && !(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_DEADHEADTIME)))
    {
      continue;
    }
    memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
    if(bTestTOD)
    {
      pGCTData->timeOfDay -= 86400L;
    }
//
//  If the from nodes don't match, see if we can flip them
//
    if(pGCTData->fromNODESrecordID != m_pRUNTIMES[nI].fromNODESrecordID)
    {
      if(!(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_TWOWAY))
      {
        continue;
      }
      memcpy(tempArray, &pGCTData->fromNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->fromNODESrecordID, &pGCTData->toNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->toNODESrecordID, tempArray, sizeof(tempArray));
    }
//
//  If we're dealing with specifics and they don't match, just cycle through
//
    if(m_pRUNTIMES[nI].fromROUTESrecordID != pGCTData->fromROUTESrecordID &&
          m_pRUNTIMES[nI].fromROUTESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromSERVICESrecordID != pGCTData->fromSERVICESrecordID &&
          m_pRUNTIMES[nI].fromSERVICESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromPATTERNNAMESrecordID != pGCTData->fromPATTERNNAMESrecordID &&
          m_pRUNTIMES[nI].fromPATTERNNAMESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          (pGCTData->timeOfDay < m_pRUNTIMES[nI].fromTimeOfDay ||
           pGCTData->timeOfDay > m_pRUNTIMES[nI].toTimeOfDay))
    {
      continue;
    }
//
//  Build a score for this component
//
    score[nI] = 0;
    score[nI] += (pGCTData->fromPATTERNNAMESrecordID == m_pRUNTIMES[nI].fromPATTERNNAMESrecordID &&
          pGCTData->fromROUTESrecordID == m_pRUNTIMES[nI].fromROUTESrecordID &&
          pGCTData->fromSERVICESrecordID == m_pRUNTIMES[nI].fromSERVICESrecordID ? 16 : 0);
    score[nI] += (pGCTData->fromSERVICESrecordID == m_pRUNTIMES[nI].fromSERVICESrecordID ? 8 : 0);
    score[nI] += (pGCTData->fromROUTESrecordID == m_pRUNTIMES[nI].fromROUTESrecordID ? 4 : 0);
    score[nI] += (m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          pGCTData->timeOfDay >= m_pRUNTIMES[nI].fromTimeOfDay &&
          pGCTData->timeOfDay <= m_pRUNTIMES[nI].toTimeOfDay ? 2 : 0);
    score[nI] += (score[nI] != 0 ? 1 : 0);
  }
//
//  Now do the to side
//
  highest = startPos;
  for(nI = startPos; nI < startPos + numToCheck; nI++)
  {
//
//  If this connection failed a test earlier, don't bother with it now
//
    if(score[nI] == NO_RECORD)
    {
      continue;
    }
    memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
    if(bTestTOD)
    {
      pGCTData->timeOfDay -= 86400L;
    }
//
//  If the to nodes don't match, see if we can flip them
//
    if(pGCTData->toNODESrecordID != m_pRUNTIMES[nI].toNODESrecordID)
    {
      if(!(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_TWOWAY))
      {
        continue;
      }
      memcpy(tempArray, &pGCTData->fromNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->fromNODESrecordID, &pGCTData->toNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->toNODESrecordID, tempArray, sizeof(tempArray));
    }
//
//  If we're dealing with specifics and they don't match, just cycle through
//
    if(m_pRUNTIMES[nI].toROUTESrecordID != pGCTData->toROUTESrecordID &&
          m_pRUNTIMES[nI].toROUTESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].toSERVICESrecordID != pGCTData->toSERVICESrecordID &&
          m_pRUNTIMES[nI].toSERVICESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].toPATTERNNAMESrecordID != pGCTData->toPATTERNNAMESrecordID &&
          m_pRUNTIMES[nI].toPATTERNNAMESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          (pGCTData->timeOfDay < m_pRUNTIMES[nI].fromTimeOfDay ||
           pGCTData->timeOfDay > m_pRUNTIMES[nI].toTimeOfDay))
    {
      continue;
    }
//
//  Build a score for this component
//
    score[nI] += (pGCTData->toPATTERNNAMESrecordID == m_pRUNTIMES[nI].toPATTERNNAMESrecordID &&
          pGCTData->toSERVICESrecordID == m_pRUNTIMES[nI].toSERVICESrecordID &&
          pGCTData->toROUTESrecordID == m_pRUNTIMES[nI].toROUTESrecordID ? 16 : 0);
    score[nI] += (pGCTData->toSERVICESrecordID == m_pRUNTIMES[nI].toSERVICESrecordID ? 8 : 0);
    score[nI] += (pGCTData->toROUTESrecordID == m_pRUNTIMES[nI].toROUTESrecordID ? 4 : 0);
    if(score[nI] > score[highest])
    {
      highest = nI;
    }
  }
//
//  Check the score of the highest to see if it failed a test earlier.
//  If it did, start at the beginning to find one that didn't.
//  If none paseed, send back NO_TIME.
//
  if(score[highest] == NO_RECORD)
  {
    highest = NO_RECORD;
    for(nI = startPos; nI < startPos + numToCheck; nI++)
    {
      if(score[nI] != NO_RECORD)
      {
        if(highest == NO_RECORD || score[nI] > score[highest])
        {
          highest = nI;
        }
      }
    }
    if(highest == NO_RECORD)
    {
      memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
      *pDistance = (float)0.0;
      return(NO_TIME);
    }
  }
  memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
//  *pDistance = max(m_pRUNTIMES[highest].distance, 0);
  *pDistance = m_pRUNTIMES[highest].distance;
  
  return(m_pRUNTIMES[highest].connectionTime);
}
