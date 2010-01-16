//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT16() - Timepoint Activity Report
//
//  [TMSRPT16]
//  FILE=TMSRPT16.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=ServiceNumber,NUMERIC,6,0,6,0,
//  FIELD2=TimeAtNode,NUMERIC,6,0,6,0,
//  FIELD3=ZoneOrNode,VARCHAR,16,0,16,0,
//  FIELD4=ServiceName,VARCHAR,16,0,16,0,
//  FIELD5=BlockNumber,NUMERIC,6,0,6,0,
//  FIELD6=Bustype,VARCHAR,32,0,32,0,
//  FIELD7=Route,VARCHAR,75,0,75,0,
//  FIELD8=TextBefore,VARCHAR,32,0,32,0,
//  FIELD9=TimepointTime,VARCHAR,8,0,8,0,
//  FIELD10=TextAfter,VARCHAR,32,0,32,0,
//  FIELD11=PatternName,VARCHAR,16,0,16,0,
//  FIELD12=StartRunNumber,NUMERIC,6,0,6,0,
//  FIELD13=EndRunNumber,NUMERIC,6,0,6,0,
//  FIELD14=ReliefNode,VARCHAR,16,0,16,0,

#include "TMSHeader.h"
#include <math.h>

BOOL FAR TMSRPT16(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  float distance;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bGotOne;
  BOOL  bIncludeSchool;
  BOOL  bIncludeShuttle;
  BOOL  bRuncut;
  char  dummy[256];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  firstNode[NODES_ABBRNAME_LENGTH + 1];
  char  lastNode[NODES_ABBRNAME_LENGTH + 1];
  char  POGNode[NODES_ABBRNAME_LENGTH + 1];
  char  PIGNode[NODES_ABBRNAME_LENGTH + 1];
  char  nodeName[NODES_ABBRNAME_LENGTH + 1];
  char  patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char  szTextBefore[32];
  char  szTextAfter[32];
  long  statbarTotal;
  long  timeAtNode;
  long  tempLong;
  long  PATTERNNAMESrecordID;
  long  directionIndex;
  long  deadheadTime;
  long  pullOutTime;
  long  pullinTime;
  long  runNumber[2];
  long  reliefNode;
  long  reliefTime;
  int   serviceIndex;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   numNodes;
  int   maxNodes;

  long  *pNodeList = NULL;

  pPassedData->nReportNumber = 15;
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
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_COMMENTS | RPFLAG_SERVICES | RPFLAG_DIVISIONS | RPFLAG_NODES;;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS), hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0 || REPORTPARMS.numDivisions == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT16.txt");
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
//  Build the "list" of nodes if a specific node was selected
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  maxNodes = BSTAT.numRecords;
  pNodeList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxNodes); 
  if(pNodeList == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
  if(REPORTPARMS.NODESrecordID != NO_RECORD)
  {
    pNodeList[0] = REPORTPARMS.NODESrecordID;
    numNodes = 1;
    NODESKey0.recordID = REPORTPARMS.NODESrecordID;
    btrieve(B_GETEQUAL,TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(nodeName, NODES_ABBRNAME_LENGTH);
  }
//
//  See if he wants for include school routes
//
  LoadString(hInst, TEXT_275, tempString, sizeof(tempString));
  MessageBeep(MB_ICONQUESTION);
  bIncludeSchool = (MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES);
//
//  See if he wants to include shuttle routes
//
  MessageBeep(MB_ICONQUESTION);
  bIncludeShuttle = (MessageBox(NULL, "Do you want to include shuttle routes?", TMS, MB_ICONQUESTION | MB_YESNO) == IDYES);
//
//  Loop through all the divisions
//
  bKeepGoing = TRUE;
  for(nI = 0; nI < REPORTPARMS.numDivisions; nI++)
  {
    DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[nI];
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Loop through the services
//
    LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
    StatusBarStart(hWndMain, tempString);
    statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Get all the runs for this division and service
//
      GetRunRecords(REPORTPARMS.pDivisionList[nI], REPORTPARMS.pServiceList[nJ]);
//      if(m_numRunRecords == 0)
//      {
//        bKeepGoing = FALSE;
//        goto deallocate;
//      }
      bRuncut = (m_numRunRecords > 0);
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  If necessary, build a list of nodes for this service
//
      if(REPORTPARMS.NODESrecordID == NO_RECORD)
      {
        serviceIndex = SERVICES.number - 1;
        if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
        {
          serviceIndex = 0;
        }
        rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
        numNodes = 0;
        while(rcode2 == 0)
        {
          if(NODES.reliefLabels[serviceIndex] == REPORTPARMS.nodeLabel)
          {
            pNodeList[numNodes++] = NODES.recordID;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
        }
      }
//
//  Loop through the routes
//
      for(nK = 0; nK < REPORTPARMS.numRoutes; nK++)
      {
        ROUTESKey0.recordID = REPORTPARMS.pRouteList[nK];
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        if((ROUTES.flags & ROUTES_FLAG_SCHOOL) && !bIncludeSchool)
        {
          continue;
        }
        if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) && !bIncludeShuttle)
        {
          continue;
        }
        strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
        strcat(routeNumberAndName, " - ");
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(routeNumberAndName, tempString);
        LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
        sprintf(tempString, szarString, routeNumberAndName, serviceName);
        StatusBarText(tempString);
        StatusBar((long)(nJ * REPORTPARMS.numRoutes + nK), (long)statbarTotal);
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
//
//  Loop through the nodes in order to generate the trips that the node appears on.
//
        for(nL = 0; nL < numNodes; nL++)
        {
          PATTERNSKey1.NODESrecordID = pNodeList[nL];
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          while(rcode2 == 0 && PATTERNS.NODESrecordID == pNodeList[nL])
          {
            btrieve(B_GETPOSITION, TMS_PATTERNS, &tempLong, &PATTERNSKey1, 1);
            if(PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                  PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ])
            {
              PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
              directionIndex = PATTERNS.directionIndex;
              TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nK];
              TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
              TRIPSKey1.directionIndex = directionIndex;
              TRIPSKey1.tripSequence = NO_RECORD;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              while(rcode2 == 0 &&
                    TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                    TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                    TRIPS.directionIndex == directionIndex)
              {
                if(TRIPS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
                {
                  PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
                  btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                  strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                  trim(patternName, PATTERNNAMES_NAME_LENGTH);
//
//  Generate the trip
//
                  GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                        TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                        TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the run number(s) on the trip
//
                  if(bRuncut)
                  {
                    runNumber[0] = 0;
                    runNumber[1] = 0;
                    for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
                    {
                      if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
                        break;
                      if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                            (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                             GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                             GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
                      {
                        if(bGotOne)
                        {
                          runNumber[1] = m_pRunRecordData[nM].runNumber;
                          reliefNode = m_pRunRecordData[nM].startNODESrecordID;
                          reliefTime = m_pRunRecordData[nM].startTime;
                          break;
                        }
                        else
                        {
                          runNumber[0] = m_pRunRecordData[nM].runNumber;
                          runNumber[1] = m_pRunRecordData[nM].runNumber;
                          reliefNode = NO_RECORD;
                          bGotOne = TRUE;
                        }
                      }
                    }
                  }
//
//  Set up the output string
//
//  Service number
//
                  sprintf(tempString, "%ld\t", SERVICES.number);
//
//  Time at node.  The first two are easy.  If, however, the node isn't
//  the first or last timepoint in the trip, we've got to hunt for it.
//
                  NODESKey0.recordID = GTResults.firstNODESrecordID;
                  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                  strncpy(firstNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  firstNode[NODES_ABBRNAME_LENGTH] = '\0';
                  NODESKey0.recordID = GTResults.lastNODESrecordID;
                  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                  strncpy(lastNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  lastNode[NODES_ABBRNAME_LENGTH] = '\0';
                  if(GTResults.firstNODESrecordID == pNodeList[nL])
                  {
                    timeAtNode = GTResults.firstNodeTime;
                    strcpy(szTextBefore, "");
                    sprintf(szTextAfter, "%s %s", lastNode, Tchar(GTResults.lastNodeTime));
                  }
                  else if(GTResults.lastNODESrecordID == pNodeList[nL])
                  {
                    timeAtNode = GTResults.lastNodeTime;
                    sprintf(szTextBefore, "%s %s", firstNode, Tchar(GTResults.firstNodeTime));
                    strcpy(szTextAfter, "");
                  }
                  else
                  {
                    sprintf(szTextBefore, "%s %s", firstNode, Tchar(GTResults.firstNodeTime));
                    sprintf(szTextAfter, "%s %s", lastNode, Tchar(GTResults.lastNodeTime));
                    PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nK];
                    PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                    PATTERNSKey2.directionIndex = directionIndex;
                    PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    PATTERNSKey2.nodeSequence = NO_RECORD;
                    rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                    nM = 0;
                    while(rcode2 == 0 &&
                          PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                          PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                          PATTERNS.directionIndex == directionIndex &&
                          PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                    {
                      if(PATTERNS.NODESrecordID == pNodeList[nL])
                        break;
                      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                        nM++;
                      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                    }
                    timeAtNode = GTResults.tripTimes[nM];
                  }
                  sprintf(szarString, "%ld\t", timeAtNode);
                  strcat(tempString, szarString);
//
//  Adjust szTextBefore and szTextAfter if there's any garage action
//
                  if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
                  {
                    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.fromROUTESrecordID = REPORTPARMS.pRouteList[nK];
                    GCTData.fromSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                    GCTData.toROUTESrecordID = REPORTPARMS.pRouteList[nK];
                    GCTData.toSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                    GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
                    GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                    GCTData.timeOfDay = GTResults.firstNodeTime;
                    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                    distance = (float)fabs((double)distance);
                    pullOutTime = GTResults.firstNodeTime - deadheadTime;
                    NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    strncpy(POGNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    POGNode[NODES_ABBRNAME_LENGTH] = '\0';
                    strcpy(szarString, szTextBefore);
                    sprintf(szTextBefore, "PO %s %s ", POGNode, Tchar(pullOutTime));
                    strcat(szTextBefore, szarString);
                  }
                  if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
                  {
                    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.fromROUTESrecordID = REPORTPARMS.pRouteList[nK];
                    GCTData.fromSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                    GCTData.toROUTESrecordID = REPORTPARMS.pRouteList[nK];
                    GCTData.toSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                    GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                    GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                    GCTData.timeOfDay = GTResults.lastNodeTime;
                    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                    distance = (float)fabs((double)distance);
                    pullinTime = GTResults.lastNodeTime + deadheadTime;
                    NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    strncpy(PIGNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    PIGNode[NODES_ABBRNAME_LENGTH] = '\0';
                    sprintf(szarString, " PI %s %s", PIGNode, Tchar(pullinTime));
                    strcat(szTextAfter, szarString);
                  }
//
//  Zone or Node
//
                  if(REPORTPARMS.NODESrecordID == NO_RECORD)
                  {
                    sprintf(szarString, "Zone: %c\t", REPORTPARMS.nodeLabel);
                  }
                  else
                  {
                    sprintf(szarString, "%s\t", nodeName);
                  }
                  strcat(tempString, szarString);
//
//  Service name
//
                  strcat(tempString, serviceName);
                  strcat(tempString, "\t");
//
//  Block number
//          
                  sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
                  strcat(tempString, szarString);
//
//  Vehicle type
//
                  if(TRIPS.BUSTYPESrecordID != NO_RECORD)
                  {
                    BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
                    btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
                    strncpy(szarString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
                    szarString[BUSTYPES_NAME_LENGTH] = '\0';
                    strcat(tempString, szarString);
                  }
                  strcat(tempString, "\t");
//
//  Route name and number
//
                  strcat(tempString, routeNumberAndName);
                  strcat(tempString, "\t");
//
//  Text before timepoint
//
                  strcat(tempString, szTextBefore);
                  strcat(tempString, "\t");
//
//  Timepoint time
//
                  strcat(tempString, Tchar(timeAtNode));
                  strcat(tempString, "\t");
//
//  Text after timepoint
//
                  strcat(tempString, szTextAfter);
                  strcat(tempString, "\t");         
//
//  Pattern name
//
                  strcat(tempString, patternName);
//
//  Runcut data
//
                  if(bRuncut)
                  {
                    strcat(tempString, "\t");
//
//  Start run number
//
                    sprintf(szarString, "%ld\t", runNumber[0]);
                    strcat(tempString, szarString);
//
//  End run number
//
                    sprintf(szarString, "%ld\t", runNumber[1]);
                    strcat(tempString, szarString);
//
//  Relief node
//
                    if(reliefNode == NO_RECORD)
                    {
                      strcpy(szarString, "");
                    }
                    else
                    {
                      NODESKey0.recordID = reliefNode;
                      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                      strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                      trim(szarString, NODES_ABBRNAME_LENGTH);
                      strcat(tempString, szarString);
                      sprintf(szarString, "/%s", Tchar(reliefTime));
                    }
                    strcat(tempString, szarString);
                  }
                  strcat(tempString, "\r\n");
//
//  Write the record
//
                  _lwrite(hfOutputFile, tempString, strlen(tempString));
                }
                rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              }
            }
            PATTERNS.recordID = tempLong;
            btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
          }
        }  // nL
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(pNodeList);
    _lclose(hfOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}


