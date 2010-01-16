//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT10() - Block Paddle
//
#include "TMSHeader.h"
#include <math.h>
//              1     2        3       4      5    6
#define HEADER "Rte#\tRteName\tService\tBlk#\tSeq\tText"
#define NUMCOLS 6

typedef struct SEStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long NODESrecordID;
  long PATTERNNAMESrecordID;
  long timeAtNode;
} SEDef;

BOOL FAR TMSRPT10(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  float distance;
  HFILE hfOutputFile;
  SEDef START;
  SEDef END;
  BOOL  bFirst;
  BOOL  bFound;
  BOOL  bGotComment;
  BOOL  bGotOne;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  NDef  *pN[MAXN];
  TDef  *pT[MAXT];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  nodeName[NODES_LONGNAME_LENGTH + 1];
  char  RteSerBlk[sizeof(routeNumber) + sizeof(routeName) + SERVICES_NAME_LENGTH + 10 + 1];
  char  *tempString2;
  long  NODESrecordIDs[MAXN];
  long  patNodeList[RECORDIDS_KEPT];
  long  tempLong[MAXN];
  long  blockNumber;
  long  POGNODESrecordID;
  long  PIGNODESrecordID;
  long  lastTRIPSrecordID;
  long  assignedToNODESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  POTime;
  long  PITime;
  long  deadheadTime;
  int   dir;
  int   foundAt;
  int   lastFound;
  int   mostOnLine;
  int   nI;
  int   nJ;
  int   nK;
  int   nKStart;
  int   nL;
  int   nLStart;
  int   nLEnd;
  int   nM;
  int   numN;
  int   numNodes;
  int   numPatNodes;
  int   numSkipped;
  int   numSets;
  int   numT;
  int   rcode2;
  int   seq;
  int   serviceIndex;

  pPassedData->nReportNumber = 9;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pBlockList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_NOALLSERVICES |
        RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS |
        RPFLAG_COMMENTS | RPFLAG_BLOCKS | RPFLAG_STANDARDBLOCKS | RPFLAG_OUTPUT;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt10.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Allocate the internal structures
//
  for(nI = 0; nI < MAXT; nI++)
  {
    pT[nI] = (TDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TDef)); 
    if(pT[nI] == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      goto deallocate;
    }
  }
  for(nI = 0; nI < MAXN; nI++)
  {
    pN[nI] = (NDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(NDef)); 
    if(pN[nI] == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      goto deallocate;
    }
  }
//
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Get all the runs
//
  GetRunRecords(REPORTPARMS.pDivisionList[0], REPORTPARMS.pServiceList[0]);
  if(m_numRunRecords == 0)
  {
    bKeepGoing = FALSE;
    goto deallocate;
  }
//
//  Set up the service index
//
  SERVICESKey0.recordID = REPORTPARMS.pServiceList[0];
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  serviceIndex = SERVICES.number - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Loop through the Blocks
//
  bKeepGoing = TRUE;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numBlocks; nI++)
  {
//
//  Get the first trip on the block
//
    TRIPSKey0.recordID = REPORTPARMS.pBlockList[nI];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
    RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
    SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
    POGNODESrecordID = TRIPS.standard.POGNODESrecordID;
    blockNumber = TRIPS.standard.blockNumber;
//
//  Get the route and service name
//
    ROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
    SERVICESKey0.recordID = TRIPS.standard.SGRPSERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Display them in the status bar
//
    LoadString(hInst, TEXT_040, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, routeNumber, routeName, serviceName, blockNumber);
    StatusBarText(tempString);
    StatusBar((long)nI, (long)REPORTPARMS.numBlocks);
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Pass 1: Generate a list of all routes/services/directions used in this block
//
    nJ = 0;
    numN = 0;
    for(;;)
    {
      ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.nodeSequence = 0;
      numPatNodes = 0;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
      {
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          patNodeList[numPatNodes++] = PATTERNS.NODESrecordID;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }  // while rcode2 == 0 on the pattern
//
//  If this is the first trip, just add the nodes to the list
//
      if(nJ == 0)
      {
        memcpy(&NODESrecordIDs, &patNodeList, sizeof(long) * numPatNodes);
        numN = numPatNodes;
      }
//
//  If it's not the first trip, set the direction (down or up), then merge.
//
      else
      {
        dir = nJ % 2;
        lastFound = dir == 0 ? 0 : numN - 1;
//
//  Go through all the nodes on this base pattern
//
        for(nK = 0; nK < numPatNodes; nK++)
        {
          if(numN >= MAXN)
            break;
//
//  dir 0: read down
//
          if(dir == 0)
          {
//
//  If this pattern node is next in the list, just continue
//
            if(patNodeList[nK] == NODESrecordIDs[lastFound])
            {
              lastFound += 1;
              continue;
            }
//
//  It wasn't.  See if we can find it.
//
            for(bFound = FALSE, nL = lastFound; nL < numN; nL++)
            {
              if(patNodeList[nK] == NODESrecordIDs[nL])
              {
                bFound = TRUE;
                lastFound = nL + 1;
                break;
              }
            }
//
//  Got it.  Continue on to the next node.
//
            if(bFound)
              continue;
//
//  Not found.  See if we can find the next (or subsequent) node(s)
//  from patNodeList in NODESrecordIDs.
//
            for(numSkipped = 0, nL = nK; nL < numPatNodes; nL++)
            {
              for(nM = lastFound; nM < numN; nM++)
              {
                if(patNodeList[nL] == NODESrecordIDs[nM])
                {
                  bFound = TRUE;
                  foundAt = nM;
                  break;
                }
              }
              if(bFound)
                break;
              numSkipped++;
            }
//
//  Found it.  Insert any we skipped over just before the located node.
//
            if(bFound)
            {
              memcpy(&tempLong, &NODESrecordIDs[foundAt],
                    sizeof(long) * (numN - foundAt));
              memcpy(&NODESrecordIDs[foundAt + numSkipped], &tempLong,
                    sizeof(long) * (numN - foundAt));
              for(nM = foundAt; nM < foundAt + numSkipped; nM++)
              {
                NODESrecordIDs[nM] = patNodeList[nK++];
              }
              nK--;  // Messing with the loop variant
              numN += numSkipped;
              lastFound = foundAt + 1;
            }
//
//  Didn't find it.  Add the balance to the end of the list.
//
            else
            {
              for(nL = nK; nL < numPatNodes; nL++)
              {
                NODESrecordIDs[numN++] = patNodeList[nL];
              }
              break;
            }
          }
//
//  dir 1: read up
//
          else
          {
            if(lastFound >= 0 && patNodeList[nK] == NODESrecordIDs[lastFound])
            {
              lastFound -= 1;
              continue;
            }
//
//  It wasn't the next node in sequence, so move up the list looking for the node
//
            for(bFound = FALSE, nL = lastFound; nL >= 0; nL--)
            {
              if(patNodeList[nK] == NODESrecordIDs[nL])
              {
                bFound = TRUE;
                lastFound = nL;
                break;
              }
            }
//
//  Didn't find the next node in sequence, so move along the
//  pattern of this trip to see if we can find another node
//
            if(!bFound)
            {
              for(numSkipped = 0, nL = nK; nL < numPatNodes; nL++)
              {
                for(nM = lastFound; nM >= 0; nM--)
                {
                  if(patNodeList[nL] == NODESrecordIDs[nM])
                  {
                    bFound = TRUE;
                    foundAt = nM;
                    break;
                  }
                }
                if(bFound)
                  break;
                numSkipped++;
              }
//
//  If we found one, open a gap in NODESrecordIDs for all the intervening
//  nodes, and drop them into the array.  Note that nK has to be adjusted
//  to account for the fact that we've skipped a bunch, and that numN
//  increases accordingly.
//
              if(bFound)
              {
                memcpy(&tempLong, &NODESrecordIDs[foundAt],
                      sizeof(long) * (numN - foundAt));
                memcpy(&NODESrecordIDs[foundAt + numSkipped], &tempLong,
                      sizeof(long) * (numN - foundAt));
                for(nM = foundAt + numSkipped; nM >= foundAt; nM--)
                {
                  NODESrecordIDs[nM] = patNodeList[nK++];  // Yeah Yeah
                }
                nK--;  // Ditto
                numN += numSkipped;
                lastFound = foundAt - 1;
              }
//
//  We didn't find a node on the pattern in the list of NODESrecordIDs.
//  Add the balance of the pattern in reverse order to the top of the list.
//
              else
              {
                for(nL = nK; nL < numPatNodes; nL++)
                {
                  for(nM = numN; nM > 0; nM--)
                  {
                    NODESrecordIDs[nM] = NODESrecordIDs[nM - 1];
                  }
                  numN++;
                  NODESrecordIDs[0] = patNodeList[nL];
                }
                break;
              }
            }  // if !bFound
          }  // else dir == 0
        }  //  nK
      }  // else nJ == 0
      lastTRIPSrecordID = TRIPS.recordID;
      PIGNODESrecordID = TRIPS.standard.PIGNODESrecordID;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      if(rcode2 != 0 ||
            TRIPS.standard.assignedToNODESrecordID != assignedToNODESrecordID ||
            TRIPS.standard.blockNumber != blockNumber ||
            TRIPS.standard.RGRPROUTESrecordID != RGRPROUTESrecordID ||
            TRIPS.standard.SGRPSERVICESrecordID != SGRPSERVICESrecordID)
        break;
      nJ++;
    }  // for(;;)
//
//  Set up the N structure
//
    for(nJ = 0; nJ < numN; nJ++)
    {
      pN[nJ]->NODESrecordID = NODESrecordIDs[nJ];
      pN[nJ]->numTimes = 0;
      for(nK = 0; nK < MAXT; nK++)
      {
        pN[nJ]->times[nK] = NO_TIME;
      }
    }
//
//  Pass 2: Get the times of the trips on this block
//
    TRIPSKey0.recordID = REPORTPARMS.pBlockList[nI];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
    POGNODESrecordID = TRIPS.standard.POGNODESrecordID;
    numT = 0;
    bFirst = TRUE;
    for(;;)
    {
//
//  Generate the trip
//
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
      if(bFirst)
      {
        START.ROUTESrecordID = TRIPS.ROUTESrecordID;
        START.SERVICESrecordID = TRIPS.SERVICESrecordID;
        START.NODESrecordID = GTResults.firstNODESrecordID;
        START.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        START.timeAtNode = GTResults.firstNodeTime;
        bFirst = FALSE;
      }
      if(TRIPS.recordID == lastTRIPSrecordID)
      {
        END.ROUTESrecordID = TRIPS.ROUTESrecordID;
        END.SERVICESrecordID = TRIPS.SERVICESrecordID;
        END.NODESrecordID = GTResults.lastNODESrecordID;
        END.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        END.timeAtNode = GTResults.lastNodeTime;
      }
//
//  Get the trip pattern nodes
//
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.nodeSequence = 0;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      numNodes = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          NODESrecordIDs[numNodes++] = PATTERNS.NODESrecordID;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
//
//  Get the run number(s) on the trip
//
      strcpy(pT[numT]->runNumber, "");
      for(bGotOne = FALSE, nJ = 0; nJ < m_numRunRecords; nJ++)
      {
        if(bGotOne && m_pRunRecordData[nJ].blockNumber != blockNumber)
          break;
        if(m_pRunRecordData[nJ].startTRIPSrecordID == TRIPS.recordID ||
              (m_pRunRecordData[nJ].blockNumber == blockNumber &&
               GTResults.firstNodeTime >= m_pRunRecordData[nJ].startTime &&
               GTResults.firstNodeTime <= m_pRunRecordData[nJ].endTime))
        {
          sprintf(tempString, "%s%ld", (bGotOne ? "/" : ""), m_pRunRecordData[nJ].runNumber);
          strcat(pT[numT]->runNumber, tempString);
          bGotOne = TRUE;
        }
      }
//
//  Get the route number of the trip
//
      ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(pT[numT]->routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(pT[numT]->routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Get the direction abbreviation of the trip
//
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(pT[numT]->dirAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(pT[numT]->dirAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Get the comment code of the trip
//
      COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      if(rcode2 != 0)
        strcpy(pT[numT]->commentCode, "");
      else
      {
        memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
        strncpy(pT[numT]->commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
        trim(pT[numT]->commentCode, COMMENTS_CODE_LENGTH);
      }
//
//  Move along the pattern and fill the structure
//
      nKStart = numT % 2 == 0 ? 0 : numN - 1;
      for(nJ = 0; nJ < numNodes; nJ++)
      {
        if(numT % 2 == 0)
        {
          for(nK = nKStart; nK < numN; nK++)
          {
            if(NODESrecordIDs[nJ] == pN[nK]->NODESrecordID)
            {
              pN[nK]->numTimes++;
              pN[nK]->times[numT] = GTResults.tripTimes[nJ];
              nKStart = nK + 1;
              break;
            }
          }
        }
        else
        {
          for(nK = nKStart; nK >= 0; nK--)
          {
            if(NODESrecordIDs[nJ] == pN[nK]->NODESrecordID)
            {
              pN[nK]->numTimes++;
              pN[nK]->times[numT] = GTResults.tripTimes[nJ];
              nKStart = nK - 1;
              break;
            }
          }
        }
      }
      numT++;
      if(numT >= MAXT)
        break;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      if(rcode2 != 0 ||
            TRIPS.standard.assignedToNODESrecordID != assignedToNODESrecordID ||
            TRIPS.standard.blockNumber != blockNumber ||
            TRIPS.standard.RGRPROUTESrecordID != RGRPROUTESrecordID ||
            TRIPS.standard.SGRPSERVICESrecordID != SGRPSERVICESrecordID)
        break;
    }  // for(;;)
//
//  Write out the records...
//
//  Route number
//
    strcpy(RteSerBlk, routeNumber);
    strcat(RteSerBlk, "\t");
//
//  Route name
//
    strcat(RteSerBlk, routeName);
    strcat(RteSerBlk, "\t");
//
//  Service
//
    strcat(RteSerBlk, serviceName);
    strcat(RteSerBlk, "\t");
//
//  Block number
//
    sprintf(szarString, "%ld\t", blockNumber);
    strcat(RteSerBlk, szarString);
//
//  Sequence
//
    sprintf(szarString, "%d\t", seq++);
    strcpy(tempString, RteSerBlk);
    strcat(tempString, szarString);
//
//  Block number, start location and time
//
    if(POGNODESrecordID == NO_RECORD)
      strcat(tempString, "No pull-out on this block");
    else
    {
      GCTData.fromPATTERNNAMESrecordID = START.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = START.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = START.ROUTESrecordID;
      GCTData.fromSERVICESrecordID = START.SERVICESrecordID;
      GCTData.toROUTESrecordID = START.ROUTESrecordID;
      GCTData.toSERVICESrecordID = START.SERVICESrecordID;
      GCTData.fromNODESrecordID = POGNODESrecordID;
      GCTData.toNODESrecordID = START.NODESrecordID;
      GCTData.timeOfDay = START.timeAtNode;
      deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
      if(deadheadTime == NO_TIME)
        POTime = NO_TIME;
      else
        POTime = START.timeAtNode - deadheadTime;
      NODESKey0.recordID = POGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szarString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szarString, NODES_LONGNAME_LENGTH);
      sprintf(szFormatString, "Block: %ld  Start at %s at %s",
          blockNumber, szarString, Tchar(POTime));
      strcat(tempString, szFormatString);
    }
    strcat(tempString, "\r\n");
    _lwrite(hfOutputFile, tempString, strlen(tempString));
    sprintf(szarString, "%d\t \r\n", seq++);
    strcpy(tempString, RteSerBlk);
    strcat(tempString, szarString);
    _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Output text
//
//  9 is the width of each time column
//
    mostOnLine = (int)floor((float)(REPORTPARMS.colsPerPage - 9) / 9);
    numSets = (int)ceil((float)numT / (float)mostOnLine);
    for(nJ = 0; nJ < numSets; nJ++)
    {
      nLStart = nJ * mostOnLine;
      nLEnd = min(numT, nLStart + mostOnLine);
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
//
//  Run
//
      strcat(tempString, "\"    Run:|");
      for(nL = nLStart; nL < nLEnd; nL++)
      {
        sprintf(szarString, "%8s%c", pT[nL]->runNumber, '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
//
//  Route
//
      strcat(tempString, "\"  Route:|");
      for(nL = nLStart; nL < nLEnd; nL++)
      {
        sprintf(szarString, "    %4s%c", pT[nL]->routeNumber, '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Direction Title
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
//
//  Direction
//
      strcat(tempString, "\"    Dir:|");
      for(nL = nLStart; nL < nLEnd; nL++)
      {
        sprintf(szarString, "%2s%s%c", pT[nL]->dirAbbr,
              nL % 2 == 0 ? "(Down)" : "  (Up)", '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Comments title
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
//
//  Comments
//
      strcat(tempString, "\"Comment:|");
      for(bGotComment = FALSE, nL = nLStart; nL < nLEnd; nL++)
      {
        if(strcmp(pT[nL]->commentCode, "") == 0)
          sprintf(szarString, "        %c", '|');
        else
        {
          bGotComment = TRUE;
          sprintf(szarString, "%8s%c", pT[nL]->commentCode, '|');
        }
        strcat(tempString, szarString);
      }
      if(bGotComment)
      {
        strcat(tempString, "\"\r\n");
        _lwrite(hfOutputFile, tempString, strlen(tempString));
      }
//
//  Nodes and times
//
      for(nK = 0; nK < numN; nK++)
      {
        if(pN[nK]->numTimes == 0)
          continue;
        NODESKey0.recordID = pN[nK]->NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(nodeName, NODES.longName, NODES_LONGNAME_LENGTH);
        trim(nodeName, NODES_LONGNAME_LENGTH);
        sprintf(tempString, "%s%d\t\"%8s|", RteSerBlk, seq++, nodeName);
        for(nL = nLStart; nL < nLEnd; nL++)
        {
          sprintf(szarString, "%8s%c", Tchar(pN[nK]->times[nL]), '|');
          for(nM = 0; nM < m_numRunRecords; nM++)
          {
            if(m_pRunRecordData[nM].blockNumber == blockNumber &&
                  pN[nK]->times[nL] == m_pRunRecordData[nM].startTime)
            {
              szarString[0] = NODES.reliefLabels[serviceIndex];
              break;
            }
          }
          strcat(tempString, szarString);
        }
        strcat(tempString, "\"\r\n");
        _lwrite(hfOutputFile, tempString, strlen(tempString));
      }
//
//  Direction Title (again)
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
//
//  Direction
//
      strcat(tempString, "\"    Dir:|");
      for(nL = nLStart; nL < nLEnd; nL++)
      {
        sprintf(szarString, "%2s%s%c", pT[nL]->dirAbbr,
              nL % 2 == 0 ? "(Down)" : "  (Up)", '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Blank line
//
      if(nJ != numSets - 1)
      {
        for(nK = 0; nK < 3; nK++)
        {
          sprintf(tempString, "%s\t%s\t%s\t%ld\t%d\t \r\n", routeNumber, routeName,
                serviceName, blockNumber, seq++);
          _lwrite(hfOutputFile, tempString, strlen(tempString));
        }
      }
    }
//
//  End of block information
//
//  Sequence
//
    sprintf(szarString, "%d\t \r\n", seq++);
    strcpy(tempString, RteSerBlk);
    strcat(tempString, szarString);
    _lwrite(hfOutputFile, tempString, strlen(tempString));
    sprintf(szarString, "%d\t", seq++);
    strcpy(tempString, RteSerBlk);
    strcat(tempString, szarString);
//
//  Block number, end location and time
//
    if(PIGNODESrecordID == NO_RECORD)
      strcat(tempString, "No pull-in on this block");
    else
    {
      GCTData.fromPATTERNNAMESrecordID = END.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = END.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = END.ROUTESrecordID;
      GCTData.fromSERVICESrecordID = END.SERVICESrecordID;
      GCTData.toROUTESrecordID = END.ROUTESrecordID;
      GCTData.toSERVICESrecordID = END.SERVICESrecordID;
      GCTData.fromNODESrecordID = END.NODESrecordID;
      GCTData.toNODESrecordID = PIGNODESrecordID;
      GCTData.timeOfDay = END.timeAtNode;
      deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
      if(deadheadTime == NO_TIME)
        PITime = NO_TIME;
      else
        PITime = END.timeAtNode + deadheadTime;
      NODESKey0.recordID = PIGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szarString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szarString, NODES_LONGNAME_LENGTH);
      sprintf(szFormatString, "Block: %ld  End at %s at %s",
          blockNumber, szarString, Tchar(PITime));
      strcat(tempString, szFormatString);
    }
    strcat(tempString, "\r\n");
    _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Spit out the comments
//
    for(nJ = 0; nJ < numT; nJ++)
    {
      if(strcmp(pT[nJ]->commentCode, "") == 0)
        continue;
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      strcpy(COMMENTSKey1.code, pT[nJ]->commentCode);
      pad(COMMENTSKey1.code, sizeof(COMMENTSKey1.code));
      btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey1, 1);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
//
//  Sequence
//
      sprintf(szarString, "%d\t", seq++);
      strcpy(tempString, RteSerBlk);
      strcat(tempString, szarString);
      strcat(tempString, " \r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
      szarString[0] = 0;
      for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\r\n"); tempString2;
           tempString2 = strtok(NULL, "\r\r\n") )
      {
        strcpy(szarString, tempString2 );
        sprintf(tempString, "%s%d\t%s: %s\r\n", RteSerBlk,
              seq++, pT[nJ]->commentCode, szarString);
        _lwrite(hfOutputFile, tempString, strlen(tempString));
      }
      for(nK = nJ + 1; nK < numT; nK++)
      {
        if(strcmp(pT[nJ]->commentCode, pT[nK]->commentCode) == 0)
          strcpy(pT[nK]->commentCode, "");
      }
    }
  }  // nI
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pBlockList);
    StatusBarEnd();
    for(nI = 0; nI < MAXT; nI++)
    {
      TMSHeapFree(pT[nI]);
    }
    for(nI = 0; nI < MAXN; nI++)
    {
      TMSHeapFree(pN[nI]);
    }
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}
