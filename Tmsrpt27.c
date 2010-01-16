//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

#define TMSRPT27_ROUTES            0
#define TMSRPT27_NODES             1
#define TMSRPT27_TIMES             2
#define TMSRPT27_SERVICES          3
#define TMSRPT27_COMMENTS          4
#define TMSRPT27_NUMOUTPUTFILES    5
#define TMSRPT27_MAXTRIPTIMES     20
#define TMSRPT27_COMMENTS_LENGTH 255

//
//  TMS Unload to the BC Transit Web Page
//
BOOL FAR TMSRPT27(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  float distance;
  HFILE hfOutputFile[TMSRPT27_NUMOUTPUTFILES];
  BOOL  bRC;
  BOOL  bFinishedOK;
  BOOL  bDoSubset;
  long  *pROUTErecordIDs = NULL;
  long  serviceIndex;
  long  MLPIndex;
  long  statbarTotal;
  long  POTime;
  long  PITime;
  long  deadheadTime;
  char  outputString[1024];
  char  dummy[256];
  char  *ptr;
  char  directionName[2][DIRECTIONS_LONGNAME_LENGTH + 1];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  commentsBuffer[COMMENTS_FIXED_LENGTH + TMSRPT27_COMMENTS_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   numRoutes;
  int   numServices;
  int   numSelectedPatternNodes;
  int   maxSelectedPatternNodes = 50;
  int   numTripPatternNodes;
  int   maxTripPatternNodes = 50;
  int   numBasePatternNodes;
  int   maxBasePatternNodes = 50;

  long  *pSERVICErecordIDs = NULL;
  long  *pSelectedPatternNodes = NULL;
  long  *pSelectedPatternSeqs = NULL;
  long  *pTripPatternSeqs = NULL;
  long  *pBasePatternNodes = NULL;
  long  *pBasePatternSeqs = NULL;

  pPassedData->nReportNumber = 26;
//
//  See what he wants to unload
//
  bFinishedOK = FALSE;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.flags = RPFLAG_PATTERNNAMES;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Fire up the status bar
//
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Allocate the trip/pattern nodes arrays
//
//  Selected pattern nodes
//
  pSelectedPatternNodes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pSelectedPatternNodes == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Selected pattern sequences
//
  pSelectedPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pSelectedPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Base pattern nodes
//
  pBasePatternNodes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxBasePatternNodes); 
  if(pBasePatternNodes == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
// Base pattern sequences
//
  pBasePatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxBasePatternNodes); 
  if(pBasePatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Trip pattern sequences
//
  pTripPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxTripPatternNodes); 
  if(pTripPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Get all the routes
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    goto done;
  }
  numRoutes = BSTAT.numRecords;
  pROUTErecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * numRoutes); 
  if(pROUTErecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  for(nI = 0; nI < numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    pROUTErecordIDs[nI] = ROUTES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Route Data
//
  LoadString(hInst, TEXT_139, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\WEBROUTES.TXT");
  hfOutputFile[TMSRPT27_ROUTES] = _lcreat(tempString, 0);
  if(hfOutputFile[TMSRPT27_ROUTES] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Route data file layout from QETXT.INI:
//
//  [TMSRPT27R]
//  FILE=WEBROUTES.TXT
//  FLN=0
//  TT=Fixed
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=RouteName,VARCHAR,64,0,64,0,
//  FIELD3=Direction0,VARCHAR,16,0,16,0,
//  FIELD4=Direction1,VARCHAR,16,0,16,0,
//  FIELD5=CommentCodeNumber,NUMERIC,8,0,8,0,
//
  for(nI = 0; nI < numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    StatusBar((long)nI, (long)numRoutes);
    ROUTESKey0.recordID = pROUTErecordIDs[nI];
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    routeNumber[ROUTES_NUMBER_LENGTH] = '\0';
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    routeName[ROUTES_NAME_LENGTH] = '\0';
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
      {
        memset(directionName[nJ], ' ', DIRECTIONS_LONGNAME_LENGTH);
      }
      else
      {
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName[nJ], DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
      }
      directionName[nJ][DIRECTIONS_LONGNAME_LENGTH] = '\0';
    }
    sprintf(outputString, "%s%s%s%s%8ld\r\n",
          routeNumber, routeName, directionName[0], directionName[1], ROUTES.COMMENTSrecordID);
    _lwrite(hfOutputFile[TMSRPT27_ROUTES], outputString, strlen(outputString));
  }
  StatusBar(-1L, -1L);
//
//  Get all the services
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    goto done;
  }
  numServices = BSTAT.numRecords;
  pSERVICErecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * numServices); 
  if(pSERVICErecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    pSERVICErecordIDs[nI] = SERVICES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Service Data
//
  LoadString(hInst, TEXT_263, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\WEBSERVICES.TXT");
  hfOutputFile[TMSRPT27_SERVICES] = _lcreat(tempString, 0);
  if(hfOutputFile[TMSRPT27_SERVICES] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Service data file layout from QETXT.INI:
//
//  [TMSRPT27S]
//  FILE=WEBSERVICES.TXT
//  FLN=0
//  TT=Fixed
//  FIELD1=ServiceSortNumber,NUMERIC,8,0,8,0,
//  FIELD2=ServiceName,VARCHAR,32,0,32,0,
//
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    StatusBar((long)nI, (long)numServices);
    SERVICESKey0.recordID = pSERVICErecordIDs[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    serviceName[SERVICES_NAME_LENGTH] = '\0';
    sprintf(outputString, "%8ld%s\r\n", SERVICES.number, serviceName);
    _lwrite(hfOutputFile[TMSRPT27_SERVICES], outputString, strlen(outputString));
  }
  StatusBar(-1L, -1L);
//
//  Comments file data
//
  LoadString(hInst, TEXT_264, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\WEBCOMMENTS.TXT");
  hfOutputFile[TMSRPT27_COMMENTS] = _lcreat(tempString, 0);
  if(hfOutputFile[TMSRPT27_COMMENTS] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Comments data file layout from QETXT.INI:
//
//  [TMSRPT27C]
//  FILE=WEBCOMMENTS.TXT
//  FLN=0
//  TT=Fixed
//  FIELD1=CommentCodeNumber,NUMERIC,8,0,8,0,
//  FIELD2=CommentCodeSymbol,VARCHAR,8,0,8,0,
//  FIELD3=CommentCodeText,VARCHAR,255,0,255,0,
//
  recordLength[TMS_COMMENTS] = sizeof(commentsBuffer);
  rcode2 = btrieve(B_GETFIRST, TMS_COMMENTS, &commentsBuffer, &COMMENTSKey0, 0);
  while(rcode2 == 0 || rcode2 == 22)
  {
    memcpy(&COMMENTS, &commentsBuffer, COMMENTS_FIXED_LENGTH);
    strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
    commentCode[COMMENTS_CODE_LENGTH] = '\0';
    ptr = &commentsBuffer[COMMENTS_FIXED_LENGTH];
    ptr[TMSRPT27_COMMENTS_LENGTH] = '\0';
    sprintf(outputString, "%8ld%s%s\r\n", COMMENTS.recordID, commentCode, ptr);
    _lwrite(hfOutputFile[TMSRPT27_COMMENTS], outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_COMMENTS, &commentsBuffer, &COMMENTSKey0, 0);
  }
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  StatusBar(-1L, -1L);
//
//  Node and time data
//
  LoadString(hInst, TEXT_257, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//
//  Nodes
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\WEBNODES.TXT");
  hfOutputFile[TMSRPT27_NODES] = _lcreat(tempString, 0);
  if(hfOutputFile[TMSRPT27_NODES] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Times
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\WEBTIMES.TXT");
  hfOutputFile[TMSRPT27_TIMES] = _lcreat(tempString, 0);
  if(hfOutputFile[TMSRPT27_TIMES] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Nodes data file layout from QETXT.INI:
//
//  [TMSRPT27N]
//  FILE=WEBNODES.TXT
//  FLN=0
//  TT=Fixed
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=ServiceSortNumber,NUMERIC,4,0,4,0,
//  FIELD3=NodeAbbr,VARCHAR,4,0,4,0,
//  FIELD4=NodeLabel,VARCHAR,4,0,4,0,
//  FIELD5=Direction,NUMERIC,1,0,1,0,
//  FIELD6=Sequence,NUMERIC,4,0,4,0,
//  FIELD7=Position,NUMERIC,4,0,4,0,
//  FIELD8=LongName,VARCHAR,8,0,8,0,
//  FIELD9=Intersection,VARCHAR,64,0,64,0,
//  FIELD10=MapCodes,VARCHAR,4,0,4,0,
//
//  Loop through the services
//
  statbarTotal = numRoutes * numServices * 2;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = pSERVICErecordIDs[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    serviceName[SERVICES_NAME_LENGTH] = '\0';
    serviceIndex = SERVICES.number - 1;
    if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
    {
      serviceIndex = 0;
    }
//
//  Loop through the routes
//
    for(nJ = 0; nJ < numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      ROUTESKey0.recordID = pROUTErecordIDs[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      routeNumber[ROUTES_NUMBER_LENGTH] = '\0';
      strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
      strcat(routeNumberAndName, " - ");
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strcat(routeNumberAndName, tempString);
//
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          goto done;
        }
//
//  Re-establish the ROUTES record, 'cause it gets changed later on
//
        if(nK == 1)
        {
          ROUTESKey0.recordID = pROUTErecordIDs[nJ];
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        }
//
//  And if there's no second direction, just leave
//
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
//
//  Get the BASE pattern for this route, service, and direction
//
//  If there isn't one, cycle through 
//
        PATTERNSKey2.ROUTESrecordID = pROUTErecordIDs[nJ];
        PATTERNSKey2.SERVICESrecordID = pSERVICErecordIDs[nI];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        if(rcode2 != 0 ||
              PATTERNS.ROUTESrecordID != pROUTErecordIDs[nJ] ||
              PATTERNS.SERVICESrecordID != pSERVICErecordIDs[nI] ||
              PATTERNS.directionIndex != nK ||
              PATTERNS.PATTERNNAMESrecordID != basePatternRecordID)
        {
          continue;
        }
//
//  Get the BASE pattern nodes and sequence numbers
//
        numBasePatternNodes = 0;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == pROUTErecordIDs[nJ] &&
              PATTERNS.SERVICESrecordID == pSERVICErecordIDs[nI] &&
              PATTERNS.directionIndex == nK &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            if(PATTERNS.flags & PATTERNS_FLAG_MLP)
            {
              MLPIndex = numBasePatternNodes;
            }
            pBasePatternNodes[numBasePatternNodes] = PATTERNS.NODESrecordID;
            pBasePatternSeqs[numBasePatternNodes++] = PATTERNS.nodeSequence;
            if(numBasePatternNodes >= maxBasePatternNodes)
            {
              maxBasePatternNodes += 50;
              pBasePatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, pBasePatternSeqs, sizeof(long) * maxBasePatternNodes); 
              if(pBasePatternSeqs == NULL)
              {
                AllocationError(__FILE__, __LINE__, TRUE);
                goto done;
              }
              pBasePatternNodes = (long *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, pBasePatternNodes, sizeof(long) * maxBasePatternNodes); 
              if(pBasePatternNodes == NULL)
              {
                AllocationError(__FILE__, __LINE__, TRUE);
                goto done;
              }
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }

//
//  Passed the tests
//
        strcpy(szarString, serviceName);
        trim(szarString, SERVICES_NAME_LENGTH);
        strcat(szarString, ": ");
        strcat(szarString, routeNumberAndName);
        StatusBarText(szarString);
        StatusBar((long)((nI + 1) * (numRoutes - 1) + nJ + nK), (long)statbarTotal);
//
//  See if the selected pattern exists on this route, service, and direction
//
        bDoSubset = FALSE;
        if(REPORTPARMS.PATTERNNAMESrecordID != basePatternRecordID)
        {
          PATTERNSKey2.ROUTESrecordID = pROUTErecordIDs[nJ];
          PATTERNSKey2.SERVICESrecordID = pSERVICErecordIDs[nI];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = REPORTPARMS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == pROUTErecordIDs[nJ] &&
                PATTERNS.SERVICESrecordID == pSERVICErecordIDs[nI] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == REPORTPARMS.PATTERNNAMESrecordID)
          {
            bDoSubset = TRUE;
            numSelectedPatternNodes = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == pROUTErecordIDs[nJ] &&
                  PATTERNS.SERVICESrecordID == pSERVICErecordIDs[nI] &&
                  PATTERNS.directionIndex == nK &&
                  PATTERNS.PATTERNNAMESrecordID == REPORTPARMS.PATTERNNAMESrecordID)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                {
                  MLPIndex = numSelectedPatternNodes;
                }
                pSelectedPatternNodes[numSelectedPatternNodes] = PATTERNS.NODESrecordID;
                pSelectedPatternSeqs[numSelectedPatternNodes++] = PATTERNS.nodeSequence;
                if(numSelectedPatternNodes >= maxSelectedPatternNodes)
                {
                  maxSelectedPatternNodes += 50;
                  pSelectedPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                         HEAP_ZERO_MEMORY, pSelectedPatternSeqs, sizeof(long) * maxSelectedPatternNodes); 
                  if(pSelectedPatternSeqs == NULL)
                  {
                    AllocationError(__FILE__, __LINE__, TRUE);
                    goto done;
                  }
                  pSelectedPatternNodes = (long *)HeapReAlloc(GetProcessHeap(),
                         HEAP_ZERO_MEMORY, pSelectedPatternNodes, sizeof(long) * maxSelectedPatternNodes); 
                  if(pSelectedPatternNodes == NULL)
                  {
                    AllocationError(__FILE__, __LINE__, TRUE);
                    goto done;
                  }
                }
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
        }
//
//  If we're not doing a subset, set up pSelectedPatternNodes
//  to fool the code into believing we are.
//
        if(!bDoSubset)
        {
          pSelectedPatternSeqs = pBasePatternSeqs;
          pSelectedPatternNodes = pBasePatternNodes;
          numSelectedPatternNodes = numBasePatternNodes;
        }
//
//  Build the output string
//
//  Loop through all the nodes on the selected pattern
//
        for(nL = 0; nL < numSelectedPatternNodes; nL++)
        {
          NODESKey0.recordID = pSelectedPatternNodes[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Route
//
          strcpy(outputString, routeNumber);
//
//  Service sort number
//
          sprintf(tempString, "%4ld", SERVICES.number);
          strcat(outputString, tempString);
//
//  Node abbreviation
//
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          tempString[NODES_ABBRNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Labels.  First and last get a "T".  Non-blank relief labels from the
//  NODES record get an "R".  If it's the *Max*, it also gets an "M".
//
          strcpy(tempString, (nL == 0 || nL == numSelectedPatternNodes - 1 ? "T" : ""));
          if(MLPIndex == nL)
          {
            strcat(tempString, "M");
          }
          if(NODES.reliefLabels[serviceIndex] != ' ' &&
                NODES.reliefLabels[serviceIndex] !='-')
          {
            strcat(tempString, "R");
          }
          pad(tempString, 4);
          tempString[4] = '\0';
          strcat(outputString, tempString);
//
//  Direction
//
          strcat(outputString, (nK == 0 ? "0" : "1"));
//
//  Sequence
//
          sprintf(tempString, "%4d", nK + 1);
          strcat(outputString, tempString);
//
//  Long name
//
          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          tempString[NODES_LONGNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Description (was Intersection)
//
          strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
          tempString[NODES_DESCRIPTION_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  MapCodes
//
          strncpy(tempString, NODES.mapCodes, NODES_MAPCODES_LENGTH);
          tempString[NODES_MAPCODES_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Write it out
//
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile[TMSRPT27_NODES], outputString, strlen(outputString));
        }
//
//  Done the nodes - do the trip times
//
//  Loop through all the trips
//
        m_bEstablishRUNTIMES = TRUE;
        TRIPSKey1.ROUTESrecordID = pROUTErecordIDs[nJ];
        TRIPSKey1.SERVICESrecordID = pSERVICErecordIDs[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == pROUTErecordIDs[nJ] &&
              TRIPS.SERVICESrecordID == pSERVICErecordIDs[nI] &&
              TRIPS.directionIndex == nK)
        {
//
//  Find the TRIP pattern sequences and nodes
//
          numTripPatternNodes = 0;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              pTripPatternSeqs[numTripPatternNodes++] = PATTERNS.nodeSequence;
              if(numTripPatternNodes >= maxTripPatternNodes)
              {
                maxTripPatternNodes += 50;
                pTripPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                       HEAP_ZERO_MEMORY, pTripPatternSeqs, sizeof(long) * maxTripPatternNodes); 
                if(pTripPatternSeqs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto done;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Times data file layout from QETXT.INI
//
//  [TMSRPT27T]
//  FILE=WEBTIMES.TXT
//  FLN=0
//  TT=Fixed
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=ServiceSortNumber,NUMERIC,4,0,4,0,
//  FIELD3=Direction,NUMERIC,1,0,1,0,
//  FIELD4=TripSort,NUMERIC,6,0,6,0,
//  FIELD5=BlockSort,NUMERIC,6,0,6,0,
//  FIELD6=BlockNumber,NUMERIC,10,0,10,0
//  FIELD7=LGRP,VARCHAR,8,0,8,0,
//  FIELD8=PatternName,VARCHAR,16,0,16,0,
//  FIELD9=FromNodeAbbr,VARCHAR,4,0,4,0,
//  FIELD10=FromTime,NUMERIC,6,0,6,0,
//  FIELD11=ToNodeAbbr,VARCHAR,4,0,4,0,
//  FIELD12=ToTime,NUMERIC,6,0,6,0,
//  FIELD13=POG,VARCHAR,4,0,4,0,
//  FIELD14=POT,NUMERIC,6,0,6,0,
//  FIELD15=PIG,VARCHAR,4,0,4,0,
//  FIELD16=PIT,NUMERIC,6,0,6,0,
//  FIELD17=CommentCodeNumber,NUMERIC,8,0,8,0,
//  FIELD18=SignCode,VARCHAR,8,0,8,0,
//  FIELD19=TripNumber,NUMERIC,10,0,10,0,
//  FIELD20=EffectiveDate,VARCHAR,32,0,32,0,
//  FIELD21=VehicleFlags,VARCHAR,4,0,4,0,
//  FIELD22=TripTime01,NUMERIC,6,0,6,0,
//  FIELD23=TripTime02,NUMERIC,6,0,6,0,
//  FIELD24=TripTime03,NUMERIC,6,0,6,0,
//  FIELD25=TripTime04,NUMERIC,6,0,6,0,
//  FIELD26=TripTime05,NUMERIC,6,0,6,0,
//  FIELD27=TripTime06,NUMERIC,6,0,6,0,
//  FIELD28=TripTime07,NUMERIC,6,0,6,0,
//  FIELD29=TripTime08,NUMERIC,6,0,6,0,
//  FIELD30=TripTime09,NUMERIC,6,0,6,0,
//  FIELD31=TripTime10,NUMERIC,6,0,6,0,
//  FIELD32=TripTime11,NUMERIC,6,0,6,0,
//  FIELD33=TripTime12,NUMERIC,6,0,6,0,
//  FIELD34=TripTime13,NUMERIC,6,0,6,0,
//  FIELD35=TripTime14,NUMERIC,6,0,6,0,
//  FIELD36=TripTime15,NUMERIC,6,0,6,0,
//  FIELD37=TripTime16,NUMERIC,6,0,6,0,
//  FIELD38=TripTime17,NUMERIC,6,0,6,0,
//  FIELD39=TripTime18,NUMERIC,6,0,6,0,
//  FIELD40=TripTime19,NUMERIC,6,0,6,0,
//  FIELD41=TripTime20,NUMERIC,6,0,6,0,
//
//  Build the output string
//
//
//  Route
//
          strcpy(outputString, routeNumber);
//
//  Service sort number
//
          sprintf(tempString, "%4ld", SERVICES.number);
          strcat(outputString, tempString);
//
//  Direction
//
          strcat(outputString, (TRIPS.directionIndex == 0 ? "0" : "1"));
//
//  Trip Sort
//
          sprintf(tempString, "%6ld", TRIPS.tripSequence);
          strcat(outputString, tempString);
//
//  Block Sort
//
          sprintf(tempString, "%6ld", TRIPS.standard.blockSequence);
          strcat(outputString, tempString);
//
//  Block number
//
          sprintf(tempString, "%10ld", TRIPS.standard.blockNumber);
          strcat(outputString, tempString);
//
//  LGRP number
//
          ROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          tempString[ROUTES_NUMBER_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Pattern name
//
          PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
          btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          tempString[PATTERNNAMES_NAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  From node abbreviation
//
          NODESKey0.recordID = GTResults.firstNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          tempString[NODES_ABBRNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  From time
//
          sprintf(tempString, "%6ld", GTResults.firstNodeTime);
          strcat(outputString, tempString);
//
//  To node abbreviation
//
          NODESKey0.recordID = GTResults.lastNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          tempString[NODES_ABBRNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  To time
//
          sprintf(tempString, "%6ld", GTResults.lastNodeTime);
          strcat(outputString, tempString);
//
//  POG and POT
//
          POTime = NO_TIME;
          if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
          {
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
            if(deadheadTime != NO_TIME)
            {
              POTime = GTResults.firstNodeTime - deadheadTime;
            }
          }
          if(POTime == NO_TIME)
          {
            memset(tempString, ' ', 10);
            tempString[10] = '\0';
          }
          else
          {
            NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            sprintf(&tempString[4], "%6ld", POTime);
          }
          strcat(outputString, tempString);
//
//  PIG and PIT
//
          PITime = NO_TIME;
          if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
          {
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
            if(deadheadTime != NO_TIME)
            {
              PITime = GTResults.lastNodeTime + deadheadTime;
            }
          }
          if(PITime == NO_TIME)
          {
            memset(tempString, ' ', 10);
            tempString[10] = '\0';
          }
          else
          {
            NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            sprintf(&tempString[4], "%6ld", PITime);
          }
          strcat(outputString, tempString);
//
//  Comment code number.  If there's a customer comment, use that.  If there isn't,
//  then use the operator comment.  If there isn't one of those, dump out NO_RECORD.
//
//
//  22-Jul-03: Tania changed her mind.  Just send out the customer comment.
//
          if(TRIPS.customerCOMMENTSrecordID != NO_RECORD)
          {
            sprintf(tempString, "%8ld", TRIPS.customerCOMMENTSrecordID);
          }
          else
          {
            sprintf(tempString, "%8ld", -1L);
          }
          strcat(outputString, tempString);
//
//  Sign code
//
          if(TRIPS.SIGNCODESrecordID == NO_RECORD)
          {
            memset(tempString, ' ', 8);
            tempString[8] = '\0';
          }
          else
          {
            SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
            btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
            strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
            tempString[SIGNCODES_CODE_LENGTH] = '\0';
          }
          strcat(outputString, tempString);
//
//  Trip number
//
          if(TRIPS.tripNumber == NO_RECORD)
          {
            TRIPS.tripNumber = 0;
          }
          sprintf(tempString, "%10ld", TRIPS.tripNumber);
          strcat(outputString, tempString);
//
//  Effective Date
//
          strcpy(tempString, szEffectiveDate);
          pad(tempString, 32);
          tempString[32] = '\0';
          strcat(outputString, tempString);
//
//  Vehicle flags
//
          if(TRIPS.BUSTYPESrecordID == NO_RECORD)
          {
            strcpy(tempString, "    ");
          }
          else
          {
            BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
            btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
            strcpy(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE) ? "1" : "0");
            strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_CARRIER) ? "1" : "0");
            strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_SURFACE) ? "1" : "0");
            strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_RAIL) ? "1" : "0");
          }
          strcat(outputString, tempString);
//
//  Trip times
//
//  We only want those times that exist on the selected pattern
//
          nL = 0;
          nM = 0;
          while(nL < numSelectedPatternNodes)
          {
            if(nM >= TMSRPT27_MAXTRIPTIMES)
            {
              break;
            }
            if(nM >= numTripPatternNodes)
            {
              break;
            }
            if(pTripPatternSeqs[nM] < pSelectedPatternSeqs[nL])
            {
              nM++;
              continue;
            }
            if(pSelectedPatternSeqs[nL] == pTripPatternSeqs[nM])
            {
              sprintf(tempString, "%6ld", GTResults.tripTimes[nM]);
              nM++;
            }
            else
            {
              memset(tempString, ' ', 6);
              tempString[6] = '\0';
            }
            strcat(outputString, tempString);
            nL++;
          }
 //
//  Write the record
//
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile[TMSRPT27_TIMES], outputString, strlen(outputString));
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
      }  // nK
    }  // nJ
  }  // nI
  StatusBar(-1L, -1L);
//
//  Schedule Data
//
//
//  All done
//
  bFinishedOK = TRUE;
  done:
    TMSHeapFree(pROUTErecordIDs);
    TMSHeapFree(pSERVICErecordIDs);
    TMSHeapFree(pSelectedPatternNodes);
    TMSHeapFree(pSelectedPatternSeqs);
    TMSHeapFree(pTripPatternSeqs);
    TMSHeapFree(pBasePatternNodes);
    TMSHeapFree(pBasePatternSeqs);
    for(nI = 0; nI < TMSRPT27_NUMOUTPUTFILES; nI++)
    {
      _lclose(hfOutputFile[nI]);
    }
    StatusBarEnd();

  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  else
  {
    LoadString(hInst, TEXT_256, szFormatString, sizeof(szFormatString));
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(szarString, '\\')) != NULL)
      *ptr = '\0';
    sprintf(tempString, szFormatString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
  }

  return(bFinishedOK);
}
