//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT09() - Driver Paddle
//
//  [RPT09PAD]
//  FILE=Tmsrpt09PAD.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Service,VARCHAR,16,0,16,0,
//  FIELD2=RunNumber,NUMERIC,7,0,7,0,
//  FIELD3=Sequence,NUMERIC,7,0,7,0,
//  FIELD4=PaddleText,VARCHAR,255,0,255,0,
//  
//  [RPT09RUN]
//  FILE=Tmsrpt09RUN.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Service,VARCHAR,16,0,16,0,
//  FIELD2=RunNumber,NUMERIC,7,0,7,0,
//  FIELD3=NumPieces,NUMERIC,7,0,7,0,
//  FIELD4=RunType,VARCHAR,20,0,20,0,
//  FIELD5=OnTime,VARCHAR,5,0,5,0,
//  FIELD6=OffTime,VARCHAR,5,0,5,0,
//  FIELD7=PlatformTime,VARCHAR,7,0,7,0,
//  FIELD8=ReportTime,VARCHAR,7,0,7,0,
//  FIELD9=TurninTime,VARCHAR,7,0,7,0,
//  FIELD10=TravelTime,VARCHAR,7,0,7,0,
//  FIELD11=SpreadOT,VARCHAR,7,0,7,0,
//  FIELD12=MakeUp,VARCHAR,7,0,7,0,
//  FIELD13=OT,VARCHAR,7,0,7,0,
//  FIELD14=PayTime,VARCHAR,7,0,7,0,
//  FIELD15=ReportAtTime,VARCHAR,7,0,7,0,
//  FIELD16=TurninAtTime,VARCHAR,7,0,7,0,
//
#include "TMSHeader.h"
#include "cistms.h"

BOOL FAR TMSRPT09(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  BLOCKSDef *pTRIPSChunk;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HFILE hfOutputFile[TMSRPT09_NUMFILES];
  BOOL  bFound;
  BOOL  bGotComment;
  BOOL  bGotSignCode;
  BOOL  bGotRunComment;
  BOOL  bGotFirstNode;
  BOOL  bKeepGoing = FALSE;
  BOOL  bLastTrip;
  BOOL  bRC;
  BOOL  bGotServiceIndex[4];
  BOOL  bFinishedTrip;
  NDef  *pN[MAXN];
  TDef  *pT[MAXT];
  char  travelDirections[1024];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
  char  nextRoute[ROUTES_NUMBER_LENGTH + 1];
  char  codeString[32];
  char  *ptr;
  char  *tempString2;
  char  szBlanks[NODES_DESCRIPTION_LENGTH + 1];
  char  lineSep[3] = {13, 10, 0};
  long  NODESrecordIDs[MAXN];
  long  patNodeList[RECORDIDS_KEPT];
  long  runNumber;
  long  tempLong[MAXN];
  long  blockNumber;
  long  runCommentRecordID;
  long  SERVICErecordID;
  long  prevBlock;
  long  seqNum;
  long  travelTime;
  long  dwellTime;
  long  fromNode;
  long  toNode;
  long  startTime;
  long  endTime;
  long  tLong;
  long  totalReportTime;
  long  totalTravelTime;
  long  totalTurninTime;
  long  equivalentTravelTime;
  UINT  nodeOutputSelection;
  int   dir;
  int   foundAt;
  int   lastFound;
  int   nI;
  int   nJ;
  int   nK;
  int   nKStart;
  int   nL;
  int   nM;
  int   numN;
  int   numNodes;
  int   numPatNodes;
  int   numPieces;
  int   numSkipped;
  int   numT;
  int   rcode2;
  int   seq;
  int   serviceIndex;
  int   showCode;
  int   displayTravelInstructions;
  int   pos;
  int   keyNumber;

  pPassedData->nReportNumber = 8;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pRunList = NULL;
  REPORTPARMS.flags = RPFLAG_SERVICES | RPFLAG_DIVISIONS | 
                      RPFLAG_NOALLDIVISIONS | RPFLAG_RUNS | RPFLAG_OUTPUT;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
  if(REPORTPARMS.numServices == 0 || REPORTPARMS.numRuns == 0)
  {
    goto deallocate;
  }
//
//  Open the output files
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT09PAD.TXT");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);

  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT09RUN.TXT");
  hfOutputFile[1] = _lcreat(tempString, 0);
  if(hfOutputFile[1] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[1], tempString);
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
  for(nI = 0; nI < 4; nI++)
  {
    bGotServiceIndex[nI] = FALSE;
  }
//
//  Decide how the node names will appear
//
  memset(szBlanks, ' ', sizeof(szBlanks));
  nodeOutputSelection = GetPrivateProfileInt("Report09", "nodeOutput", RPTCFG09_LONG, TMSINIFile);
  if(nodeOutputSelection == RPTCFG09_ABBR)
  {
    szBlanks[NODES_ABBRNAME_LENGTH] = '\0';
  }
  else if(nodeOutputSelection == RPTCFG09_LONG)
  {
    szBlanks[NODES_LONGNAME_LENGTH] = '\0';
  }
  else 
  {
    szBlanks[NODES_DESCRIPTION_LENGTH] = '\0';
  }
  showCode = GetPrivateProfileInt("Report09", "showCode", 0, TMSINIFile);
  displayTravelInstructions = GetPrivateProfileInt("Report09", "displayTravelInstructions", 1, TMSINIFile);
//
//  Start the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  If we're showing a farebox code, it means that we need all the runs up front
//
  if(showCode)
  {
    GetRunRecords(REPORTPARMS.pDivisionList[0], REPORTPARMS.pServiceList[0]);
    if(m_numRunRecords == 0)
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    prevBlock = NO_RECORD;
    for(nI = 0; nI < m_numRunRecords; nI++)
    {
      if(m_pRunRecordData[nI].blockNumber / 100 != prevBlock)
      {
        prevBlock = m_pRunRecordData[nI].blockNumber / 100;
        seqNum = 0;
      }
      m_pRunRecordData[nI].sequenceNumber = seqNum++;
    }
  }
//
//  Loop through the runs
//
  bKeepGoing = TRUE;
  for(nI = 0; nI < REPORTPARMS.numRuns; nI++)
  {
//
//  Get the run
//
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
//
//  Get the service name
//
    SERVICESKey0.recordID = RUNS.SERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  See if we need to load the Travel Matrix
//
    serviceIndex = SERVICES.number - 1;
    if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
    {
      serviceIndex = 0;
    }
    if(!bGotServiceIndex[serviceIndex] && bUseDynamicTravels)
    {
      StatusBarEnd();
      bGotServiceIndex[serviceIndex] = TRUE;
      LoadString(hInst, bUseCISPlan ? TEXT_154 : TEXT_135, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      if(bUseCISPlan)
      {
        CISfree();
        if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
        {
          goto deallocate;
        }
      }
      else
      {
        InitTravelMatrix(SERVICES.recordID, FALSE);
      }
      LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
      StatusBarStart(hWndMain, tempString);
    }
//
//  Cost the run
//
    runNumber = RUNS.runNumber;
    LoadString(hInst, TEXT_120, szarString, sizeof(szarString));
    sprintf(tempString, szarString, serviceName, runNumber);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)REPORTPARMS.numRuns);
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      break;
    }
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
//
//  Check to see if we couldn't set up the run
//
    if(numPieces <= 0)
    {
      MessageBeep(MB_ICONSTOP);
      LoadString(hInst, ERROR_183, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, blockNumber, RUNS.runNumber);
      MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
      bKeepGoing = FALSE;
      goto deallocate;
    }
//
//  Set up the output string
//
//  Service
//
    strcpy(tempString, "\"");
    strcat(tempString, serviceName);
    strcat(tempString, "\"\t");
//
//  Run number
//
    sprintf(szarString, "%ld", runNumber);
    strcat(tempString, szarString);
    strcat(tempString, "\t");
//
//  Number of pieces
//
    sprintf(szarString, "%d", numPieces);
    strcat(tempString, szarString);
    strcat(tempString, "\t");
//
//  Runtype
//
    nJ = (short int)LOWORD(RUNSVIEW[0].runType);
    nK = (short int)HIWORD(RUNSVIEW[0].runType);
    strcat(tempString, "\"");
    if(nJ != NO_RECORD && nK != NO_RECORD)
    {
      strcat(tempString, RUNTYPE[nJ][nK].localName);
    }
    strcat(tempString, "\"\t");
//
//  On time
//
    strcat(tempString, Tchar(RUNSVIEW[0].runOnTime));
    strcat(tempString, "\t");
//
//  Off time
//
    strcat(tempString, Tchar(RUNSVIEW[numPieces - 1].runOffTime));
    strcat(tempString, "\t");
//
//  Platform time
//
    for(nJ = 1; nJ < numPieces; nJ++)
    {
      RUNSVIEW[0].platformTime += RUNSVIEW[nJ].platformTime;
    }
    strcat(tempString, "\"");
    strcat(tempString, chhmm(RUNSVIEW[0].platformTime));
    strcat(tempString, "\"\t");
//
//  Report time
//
    totalReportTime = 0;
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      totalReportTime += RUNSVIEW[nJ].reportTime;
    }
    strcat(tempString, "\"");
    strcat(tempString, chhmm(totalReportTime));
    strcat(tempString, "\"\t");
//
//  Turnin time
//
    totalTurninTime = 0;
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      totalTurninTime += RUNSVIEW[nJ].turninTime;
    }
    strcat(tempString, "\"");
    strcat(tempString, chhmm(totalTurninTime));
    strcat(tempString, "\"\t");
//
//  Travel Time
//
    totalTravelTime = 0;
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      totalTravelTime += RUNSVIEW[nJ].travelTime;
    }
    strcat(tempString, "\"");
    strcat(tempString, chhmm(totalTravelTime));
    strcat(tempString, "\"\t");
//
//  Spread overtime
//
    strcat(tempString, "\"");
    strcat(tempString, chhmm(RUNSVIEW[numPieces - 1].spreadOT));
    strcat(tempString, "\"\t");
//
//  Bonus to make 8 hours
//
    strcat(tempString, "\"");
    strcat(tempString, chhmm(RUNSVIEW[numPieces - 1].makeUpTime));
    strcat(tempString, "\"\t");
//
//  Overtime
//
    strcat(tempString, "\"");
    strcat(tempString, chhmm(RUNSVIEW[numPieces - 1].overTime));
    strcat(tempString, "\"\t");
//
//  Pay time
//
    strcat(tempString, "\"");
    strcat(tempString, chhmm(RUNSVIEW[numPieces - 1].payTime));
    strcat(tempString, "\"\t");
//
//  Report at time
//
    tLong = RUNSVIEW[0].runOnTime - RUNSVIEW[0].reportTime - RUNSVIEW[0].startOfPieceTravel;
    if(RUNSVIEW[numPieces - 1].startOfPieceExtraboardStart != NO_TIME)
    {
      tLong = RUNSVIEW[numPieces - 1].startOfPieceExtraboardStart;
    }
    strcat(tempString, "\"");
    strcat(tempString, Tchar(tLong));
    strcat(tempString, "\"\t");
//
//  Turnin at time
//
    tLong = RUNSVIEW[numPieces - 1].runOffTime + RUNSVIEW[numPieces - 1].turninTime + RUNSVIEW[numPieces - 1].endOfPieceTravel;
    if(RUNSVIEW[numPieces - 1].endOfPieceExtraboardEnd != NO_TIME)
    {
      tLong = RUNSVIEW[numPieces - 1].endOfPieceExtraboardEnd;
    }
    strcat(tempString, "\"");
    strcat(tempString, Tchar(tLong));
    strcat(tempString, "\"");
//
//  Done
//
    strcat(tempString, "\r\n");
    _lwrite(hfOutputFile[TMSRPT09_COSTING], tempString, strlen(tempString));
//
//  Re-establish position
//
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    runNumber = RUNS.runNumber;
//
//  Generate a list of all routes/services/directions encompassing this run/piece
//
    seq = 0;
    SERVICErecordID = RUNS.SERVICESrecordID;
    while(rcode2 == 0 &&
          RUNS.SERVICESrecordID == SERVICErecordID &&
          RUNS.runNumber == runNumber)
    {
      if(RUNS.pieceNumber == 1)
      {
        runCommentRecordID = RUNS.COMMENTSrecordID;
        bGotRunComment = runCommentRecordID != NO_RECORD;
      }
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      keyNumber = TRIPS.dropback.blockNumber == 0 ? 2 : 3;
      rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
//
//  Generate the list of nodes based on all the routes/services/directions
//
      nJ = 0;
      numN = 0;
      blockNumber = TRIPS.standard.blockNumber;
      while(rcode2 == 0)
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
          {
            patNodeList[numPatNodes++] = PATTERNS.NODESrecordID;
          }
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
            {
              break;
            }
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
              {
                continue;
              }
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
                {
                  break;
                }
                numSkipped++;
              }
//
//  Found it.  Insert any we skipped over just before the located node.
//
              if(bFound)
              {
                memcpy(&tempLong, &NODESrecordIDs[foundAt], sizeof(long) * (numN - foundAt));
                memcpy(&NODESrecordIDs[foundAt + numSkipped], &tempLong, sizeof(long) * (numN - foundAt));
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
                  {
                    break;
                  }
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
                  memcpy(&tempLong, &NODESrecordIDs[foundAt], sizeof(long) * (numN - foundAt));
                  memcpy(&NODESrecordIDs[foundAt + numSkipped], &tempLong, sizeof(long) * (numN - foundAt));
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
        if(TRIPS.recordID == RUNS.end.TRIPSrecordID)
        {
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
//
//  This next check, believe it or not, takes care of a situation that can
//  arise when the "scheduler" unblocks one or more trips in the middle of
//  an existing run and then attempts to run this report.
//
        if(rcode2 == 0 && TRIPS.standard.blockNumber != blockNumber)
        {
          MessageBeep(MB_ICONSTOP);
          LoadString(hInst, ERROR_183, szFormatString, sizeof(szFormatString));
          sprintf(tempString, szFormatString, blockNumber, RUNS.runNumber);
          MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
          bKeepGoing = FALSE;
          goto deallocate;
        }
        nJ++;
      }  // while rcode2 == 0
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
//  Get the times of the trips on this run
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(TRIPS.dropback.blockNumber == 0)
      {
        keyNumber = 2;
        pTRIPSChunk = &TRIPS.standard;
      }
      else
      {
        keyNumber = 3;
        pTRIPSChunk = &TRIPS.dropback;
      }
      rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      bGotFirstNode = pTRIPSChunk->POGNODESrecordID != NO_RECORD &&
            pTRIPSChunk->POGNODESrecordID == RUNS.start.NODESrecordID;
      numT = 0;
      while(rcode2 == 0)
      {
//
//  Generate the trip
//
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
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
          {
            NODESrecordIDs[numNodes++] = PATTERNS.NODESrecordID;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
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
        if(rcode2 == 0)
        {
          memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
          strncpy(pT[numT]->commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(pT[numT]->commentCode, COMMENTS_CODE_LENGTH);
        }
        else
        {
          strcpy(pT[numT]->commentCode, "");
        }
//
//  Get the sign code of the trip
//
        SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(pT[numT]->signCode, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
          trim(pT[numT]->signCode, SIGNCODES_CODE_LENGTH);
        }
        else
        {
          strcpy(pT[numT]->signCode, "");
        }
//
//  Get the trip number of the trip
//
        pT[numT]->tripNumber = TRIPS.tripNumber < 0 ? 0 : TRIPS.tripNumber;
//
//  Move along the pattern and fill the structure
//
        nKStart = numT % 2 == 0 ? 0 : numN - 1;
        for(bFinishedTrip = TRUE, nJ = 0; nJ < numNodes; nJ++)
        {
          if(bGotFirstNode || NODESrecordIDs[nJ] == RUNS.start.NODESrecordID)
          {
            bGotFirstNode = TRUE;
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
            if(TRIPS.recordID == RUNS.end.TRIPSrecordID &&
                  NODESrecordIDs[nJ] == RUNS.end.NODESrecordID)
            {
              if(nJ < numNodes - 1)
              {
                bFinishedTrip = FALSE;
              }
              break;
            }
          }
        }
        numT++;
        if(numT >= MAXT)
        {
          break;
        }
        bLastTrip = TRIPS.recordID == RUNS.end.TRIPSrecordID;
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        if(bLastTrip)  // Get next trip information
        {
          if(rcode2 == 0 && pTRIPSChunk->blockNumber == RUNSVIEW[RUNS.pieceNumber - 1].blockNumber)
          {
            if(bFinishedTrip)
            {
              ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
            }
            else
            {
              ROUTESKey0.recordID = RUNS.end.TRIPSrecordID;
            }
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(nextRoute, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(nextRoute, ROUTES_NUMBER_LENGTH);
          }
          else
          {
            strcpy(nextRoute, "");
          }
          break;
        }
      }  // while
//
//  Write out the structures...
//
//  Start of piece information
//
//  Any extraboard?
//
      if(RUNS.prior.startTime != NO_TIME && 
            (RUNS.prior.startTime != 0L && RUNS.prior.endTime != 0L))
      {
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"*** Extraboard duty: Start at: %s\"\r\n",
              serviceName, runNumber, seq++, Tchar(RUNS.prior.startTime));
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"*** Extraboard duty:   End at: %s\"\r\n",
              serviceName, runNumber, seq++, Tchar(RUNS.prior.endTime));
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
//
//  Dump out any travel instructions
//
      if(displayTravelInstructions && bUseCISPlan)
      {
        fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].startNODESrecordID;
        toNode = COST.TRAVEL[RUNS.pieceNumber - 1].startAPointNODESrecordID;
        if(fromNode != NO_RECORD && toNode != NO_RECORD && 
              !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
        {
          strcpy(travelDirections, CISplanReliefConnect(&startTime, &endTime, &dwellTime,
                TRUE, fromNode, toNode, COST.TRAVEL[RUNS.pieceNumber - 1].startAPointTime, FALSE, SERVICErecordID));
          if(startTime == -25 * 60 * 60)
          {
            travelTime = NO_TIME;
          }
          else
          {
            travelTime = (endTime - startTime) + dwellTime;
          }
          sprintf(tempString, "\"%s\"\t%ld\t%d\t\"Piece %d: Start\"\r\n", serviceName, runNumber, seq++, RUNS.pieceNumber);
          _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
          ptr = strtok(travelDirections, lineSep);
          while(ptr != 0)
          {
            sprintf(tempString, "\"%s\"\t%ld\t%d\t\"%s\"\r\n", serviceName, runNumber, seq++, ptr);
            _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
            ptr = strtok(NULL, lineSep);
          }
        }
      }
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Block number, start location and time
//
      NODESKey0.recordID = RUNSVIEW[RUNS.pieceNumber - 1].runOnNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szarString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szarString, NODES_LONGNAME_LENGTH);
      if(!showCode)
      {
        strcpy(codeString, "");
      }
      else
      {
        for(nJ = 0; nJ < m_numRunRecords; nJ++)
        {
          if(m_pRunRecordData[nJ].blockNumber == RUNSVIEW[RUNS.pieceNumber - 1].blockNumber)
          {
            pos = 0;
            for(nK = nJ; nK < m_numRunRecords; nK++)
            {
              if(m_pRunRecordData[nK].blockNumber != RUNSVIEW[RUNS.pieceNumber - 1].blockNumber)
              {
                break;
              }
              if(RUNSVIEW[RUNS.pieceNumber - 1].runOnTime <= m_pRunRecordData[nK].startTime)
              {
                break;
              }
              pos++;
            }
            break;
          }
        }
        sprintf(codeString, " - Farebox code is %ld:%02d", RUNSVIEW[RUNS.pieceNumber - 1].blockNumber / 100, pos);
      }
      sprintf(szFormatString, "\"Block: %ld  Start at %s at %s%s\"",
            RUNSVIEW[RUNS.pieceNumber - 1].blockNumber, szarString,
            Tchar(RUNSVIEW[RUNS.pieceNumber - 1].runOnTime), codeString);
      strcat(tempString, szFormatString);
      strcat(tempString, "\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  Route Title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Route
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(nJ = 0; nJ < numT; nJ++)
      {
        sprintf(szarString, "Rte:%4s%c", pT[nJ]->routeNumber, '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  Direction Title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Direction
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(nJ = 0; nJ < numT; nJ++)
      {
        sprintf(szarString, "%2s%s%c", pT[nJ]->dirAbbr, nJ % 2 == 0 ? "(Down)" : "  (Up)", '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  Comments title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Comments
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(bGotComment = FALSE, nJ = 0; nJ < numT; nJ++)
      {
        if(strcmp(pT[nJ]->commentCode, "") == 0)
        {
          sprintf(szarString, "        %c", '|');
        }
        else
        {
          bGotComment = TRUE;
          sprintf(szarString, "Rem:%4s%c", pT[nJ]->commentCode, '|');
        }
        strcat(tempString, szarString);
      }
      if(bGotComment)
      {
        strcat(tempString, "\"\r\n");
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
//
//  Sign code title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Sign codes
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(bGotSignCode = FALSE, nJ = 0; nJ < numT; nJ++)
      {
        if(strcmp(pT[nJ]->signCode, "") == 0)
        {
          sprintf(szarString, "        %c", '|');
        }
        else
        {
          bGotSignCode = TRUE;
          sprintf(szarString, "Sgn:%4s%c", pT[nJ]->signCode, '|');
        }
        strcat(tempString, szarString);
      }
      if(bGotSignCode)
      {
        strcat(tempString, "\"\r\n");
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
//
//  Trip number Title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Trip number
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(nJ = 0; nJ < numT; nJ++)
      {
        sprintf(szarString, "Num:%4ld%c", pT[nJ]->tripNumber, '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  Nodes and times
//
      for(nJ = 0; nJ < numN; nJ++)
      {
        if(pN[nJ]->numTimes == 0)
        {
          continue;
        }
        NODESKey0.recordID = pN[nJ]->NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(nodeOutputSelection == RPTCFG09_ABBR)
        {
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          szarString[NODES_ABBRNAME_LENGTH] = '\0';
        }
        else if(nodeOutputSelection == RPTCFG09_LONG)
        {
          strncpy(szarString, NODES.longName, NODES_LONGNAME_LENGTH);
          szarString[NODES_LONGNAME_LENGTH] = '\0';
        }
        else
        {
          strncpy(szarString, NODES.intersection, 32);
          szarString[32] = '\0';
        }
//
//  Service, run number and sequence
//
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"%s|", serviceName, RUNS.runNumber, seq + nJ, szarString);
        for(nK = 0; nK < numT; nK++)
        {
          sprintf(szarString, "%8s%c", Tchar(pN[nJ]->times[nK]), '|');
          strcat(tempString, szarString);
        }
        strcat(tempString, "\"\r\n");
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
      seq += numN;
//
//  Direction Title
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Direction
//
      sprintf(szarString, "\"%s|", szBlanks);
      strcat(tempString, szarString);
      for(nJ = 0; nJ < numT; nJ++)
      {
        sprintf(szarString, "%2s%s%c", pT[nJ]->dirAbbr, nJ % 2 == 0 ? "(Down)" : "  (Up)", '|');
        strcat(tempString, szarString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  End of piece information
//
//  Service
//
      strcpy(tempString, "\"");
      strcat(tempString, serviceName);
      strcat(tempString, "\"\t");
//
//  Run number and sequence
//
      sprintf(szarString, "%ld\t%d\t", RUNS.runNumber, seq++);
      strcat(tempString, szarString);
//
//  Block number, start location and time
//
      NODESKey0.recordID = RUNSVIEW[RUNS.pieceNumber - 1].runOffNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szarString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szarString, NODES_LONGNAME_LENGTH);
      sprintf(szFormatString, "\"End at %s at %s", szarString, Tchar(RUNSVIEW[RUNS.pieceNumber - 1].runOffTime));
      strcat(tempString, szFormatString);
      if(strcmp(nextRoute, "") != 0)
      {
        sprintf(szFormatString, " *** Park bus for route %s ***", nextRoute);
        strcat(tempString, szFormatString);
      }
      strcat(tempString, "\"\r\n");
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
//
//  Dump out any travel instructions
//
      if(displayTravelInstructions && bUseCISPlan)
      {
        fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].endBPointNODESrecordID;
        toNode = COST.TRAVEL[RUNS.pieceNumber - 1].endNODESrecordID;
        if(fromNode != NO_RECORD &&
              toNode != NO_RECORD &&
              !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
        {
          strcpy(travelDirections, CISplanReliefConnect(&startTime, &endTime, &dwellTime,
                TRUE, fromNode, toNode, COST.TRAVEL[RUNS.pieceNumber - 1].endBPointTime,
                TRUE, SERVICErecordID));
          if(startTime == -25 * 60 * 60)
          {
            travelTime = NO_TIME;
          }
          else
          {
            travelTime = (endTime - startTime) + dwellTime;
          }
          sprintf(tempString, "\"%s\"\t%ld\t%d\t\"Piece %d: End\"\r\n", serviceName, runNumber, seq++, RUNS.pieceNumber);
          _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
          ptr = strtok(travelDirections, lineSep);
          while(ptr != 0)
          {
            sprintf(tempString, "\"%s\"\t%ld\t%d\t\"%s\"\r\n", serviceName, runNumber, seq++, ptr);
            _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
            ptr = strtok(NULL, lineSep);
          }
        }
      }
//
//  Spit out the comments
//
      for(nJ = 0; nJ < numT; nJ++)
      {
        if(strcmp(pT[nJ]->commentCode, "") == 0)
        {
          continue;
        }
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        strcpy(COMMENTSKey1.code, pT[nJ]->commentCode);
        pad(COMMENTSKey1.code, sizeof(COMMENTSKey1.code));
        btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey1, 1);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        sprintf(tempString, "\"%s\"\t%ld\t%d\t \r\n", serviceName, RUNS.runNumber, seq++);
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        szarString[0] = 0;

        for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
             tempString2 = strtok(NULL, "\r\n") )
        {
          strcpy(szarString, tempString2 );
          sprintf(tempString, "\"%s\"\t%ld\t%d\t\"%s: %s\"\r\n", serviceName, RUNS.runNumber, seq++, pT[nJ]->commentCode, szarString);
          _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        }
        for(nK = nJ + 1; nK < numT; nK++)
        {
          if(strcmp(pT[nJ]->commentCode, pT[nK]->commentCode) == 0)
          {
            strcpy(pT[nK]->commentCode, "");
          }
        }
      }
//
//  Spit out the sign codes
//
      for(nJ = 0; nJ < numT; nJ++)
      {
        if(strcmp(pT[nJ]->signCode, "") == 0)
        {
          continue;
        }
        strcpy(SIGNCODESKey1.code, pT[nJ]->signCode);
        pad(SIGNCODESKey1.code, sizeof(SIGNCODESKey1.code));
        btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey1, 1);
        sprintf(tempString, "%s\t%ld\t%d\t \r\n", serviceName, RUNS.runNumber, seq++);
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        strncpy(szarString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
        trim(szarString, SIGNCODES_TEXT_LENGTH);
        sprintf(tempString, "\"%s\"\t%ld\t%d\t%\"s: %s\"\r\n", serviceName, RUNS.runNumber, seq++, pT[nJ]->signCode, szarString);
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        for(nK = nJ + 1; nK < numT; nK++)
        {
          if(strcmp(pT[nJ]->signCode, pT[nK]->signCode) == 0)
          {
            strcpy(pT[nK]->signCode, "");
          }
        }
      }
//
//  Any extraboard?
//
      if(RUNS.after.startTime != NO_TIME &&
            (RUNS.after.startTime != 0L && RUNS.after.endTime != 0L))
      {
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"*** Extraboard duty: Start at: %s\"\r\n",
              serviceName, runNumber, seq++, Tchar(RUNS.after.startTime));
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"*** Extraboard duty:   End at: %s\"\r\n",
              serviceName, runNumber, seq++, Tchar(RUNS.after.endTime));
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
//
//  Get the next run / piece
//
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      SERVICErecordID = RUNS.SERVICESrecordID;
      if(rcode2 == 0 &&
            RUNS.SERVICESrecordID == SERVICErecordID &&
            RUNS.runNumber == runNumber)
      {
        for(nJ = 0; nJ < 2; nJ++)
        {
          strcpy(tempString, "\"");
          strcat(tempString, serviceName);
          strcat(tempString, "\"\t");
          sprintf(szarString, "%ld\t%d\t \r\n", RUNS.runNumber, seq++);
          strcat(tempString, szarString);
          _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
        }
      }
    }  // while runNumber == RUNS.runNumber
//
//  Do the run comment
//
    if(bGotRunComment)
    {
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      COMMENTSKey0.recordID = runCommentRecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
      strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
      trim(commentCode, COMMENTS_CODE_LENGTH);
      sprintf(tempString, "\"%s\"\t%ld\t%d\t \r\n", serviceName, RUNS.runNumber, seq++);
      _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      szarString[0] = 0;

      for(tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2; tempString2 = strtok(NULL, "\r\n") )
      {
        strcpy(szarString, tempString2 );
        sprintf(tempString, "\"%s\"\t%ld\t%d\t\"%s: %s\"\r\n", serviceName, runNumber, seq++, commentCode, szarString);
        _lwrite(hfOutputFile[TMSRPT09_PADDLE], tempString, strlen(tempString));
      }
    }
  }  // nI
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pRunList);
    StatusBarEnd();
    for(nI = 0; nI < MAXT; nI++)
    {
      TMSHeapFree(pT[nI]);
    }
    for(nI = 0; nI < MAXN; nI++)
    {
      TMSHeapFree(pN[nI]);
    }
    for(nI = 0; nI < TMSRPT09_NUMFILES; nI++)
    {
      _lclose(hfOutputFile[nI]);
    }
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

BOOL CALLBACK TMSRPT09CMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlSHOWCODE;
  static HANDLE hCtlDISPLAYTRAVELINSTRUCTIONS;
  UINT   nodeOutputSelection;
  int    showCode;
  int    displayTravelInstructions;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hCtlSHOWCODE = GetDlgItem(hWndDlg, RPTCFG09_SHOWCODE);
      hCtlDISPLAYTRAVELINSTRUCTIONS = GetDlgItem(hWndDlg, RPTCFG09_DISPLAYTRAVELINSTRUCTIONS);
      nodeOutputSelection = GetPrivateProfileInt("Report09", "nodeOutput", 0, TMSINIFile);
      if(nodeOutputSelection == 0)
        nodeOutputSelection = RPTCFG09_LONG;
      SendDlgItemMessage(hWndDlg, nodeOutputSelection, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      showCode = GetPrivateProfileInt("Report09", "showCode", 0, TMSINIFile);
      if(showCode)
        SendMessage(hCtlSHOWCODE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      displayTravelInstructions = 
            GetPrivateProfileInt("Report09", "displayTravelInstructions", 1, TMSINIFile);
      if(displayTravelInstructions)
        SendMessage(hCtlDISPLAYTRAVELINSTRUCTIONS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDOK:
          if(IsDlgButtonChecked(hWndDlg, RPTCFG09_ABBR))
            nodeOutputSelection = RPTCFG09_ABBR;
          else if(IsDlgButtonChecked(hWndDlg, RPTCFG09_LONG))
            nodeOutputSelection = RPTCFG09_LONG;
          else
            nodeOutputSelection = RPTCFG09_ADDRESS;
          itoa(nodeOutputSelection, tempString, 10);
          WritePrivateProfileString("Report09", "nodeOutput", tempString, TMSINIFile);
          showCode = SendMessage(hCtlSHOWCODE, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          itoa(showCode, tempString, 10);
          WritePrivateProfileString("Report09", "showCode", tempString, TMSINIFile);
          displayTravelInstructions =
                SendMessage(hCtlDISPLAYTRAVELINSTRUCTIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          itoa(displayTravelInstructions, tempString, 10);
          WritePrivateProfileString("Report09",
                "displayTravelInstructions", tempString, TMSINIFile);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
