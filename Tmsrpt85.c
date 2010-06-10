//
//  TMSRPT85() - Driver Paddle (4)
//
//  From Schema.ini
//  
//  [TMSRPT85.TXT]
//  ColNameHeader=False
//  Format=TabDelimited
//  CharacterSet=ANSI
//  Col1=Sequence Integer
//  Col2=Division Char
//  Col3=Service char
//  Col4=RunNumber Integer
//  Col5=Runtype Char
//  Col6=ReportTime Char
//  Col7=PayTime Char
//  Col8=EndTime Char
//  Col9=Text Char
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

#define MAXTRIPSAVE 200

#define TRIPSAVE_FLAG_FIRSTTRIP 0x0001
#define TRIPSAVE_FLAG_LASTTRIP  0x0002
#define TRIPSAVE_FLAG_PULLOUT   0x0004

#define MAXRUNSAVE 1000

typedef struct RUNSAVEStruct
{
  long runNumber;
  STARTENDDef start;
  STARTENDDef end;
} RUNSAVEDef;

//
//  sort_tripsave: Sort a TRIPSAVE structure on service, route, direction, and start time (located in tmsrpt35.c)
//
extern int sort_tripsave(const void *a, const void *b);

BOOL FAR TMSRPT85(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef GTResults;
  REPORTPARMSDef  REPORTPARMS;
  RUNSAVEDef  RUNSAVE[MAXRUNSAVE];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  float  distance;
  BOOL  bGotRunComment;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bFound;
  BOOL  bGotServiceIndex[4];
  BOOL  bIncludeSignCodes;
  BOOL  bIncludeTripNumbers;
  BOOL  bFirstTrip;
  BOOL  bLastTrip;
  BOOL  bFirstNode;
  char  travelDirections[1024];
  char  szDivisionName[DIVISIONS_NAME_LENGTH + 1];
  char  szServiceName[SERVICES_NAME_LENGTH + 1];
  char  szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
  char  intersection[NODES_INTERSECTION_LENGTH + 1];
  char  garageAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  *ptr;
  char  *tempString2;
  char  szRuntype[40];
  char  szReportTime[8];
  char  szPayTime[8];
  char  szEndTime[8];
  char  szStartTime[8];
  char  szFromNode[NODES_INTERSECTION_LENGTH + 1];
  char  szToNode[NODES_INTERSECTION_LENGTH + 1];
  char  lineSep[3] = {13, 10, 0};
  long  commentRecordIDs[100];
  long  signcodeRecordIDs[100];
  long  runNumber;
  long  blockNumber;
  long  runCommentRecordID;
  long  deadheadTime;
  long  tempLong;
  long  dwellTime;
  long  fromNode;
  long  toNode;
  long  startTime;
  long  endTime;
  long  equivalentTravelTime;
  long  reliefRunNumber;
  long  pieceReportTime[MAXPIECES];
  long  pieceEndTime[MAXPIECES];
  int   nI;
  int   nJ;
  int   nK;
  int   nM;
  int   numNodes;
  int   numPieces;
  int   numComments;
  int   numSigncodes;
  int   numInRunsave;
  int   rcode2;
  int   rcode3;
  int   seq;
  int   serviceIndex;

  pPassedData->nReportNumber = 84;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pRunList = NULL;
  REPORTPARMS.flags = RPFLAG_SERVICES | RPFLAG_NOALLSERVICES | RPFLAG_DIVISIONS |
        RPFLAG_NOALLDIVISIONS | RPFLAG_RUNS | RPFLAG_SIGNCODES | RPFLAG_TRIPNUMBERS;
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
//  Does he want a summary of signcodes or trip numbers?
//
  bIncludeSignCodes = REPORTPARMS.returnedFlags & RPFLAG_SIGNCODES;
  bIncludeTripNumbers = REPORTPARMS.returnedFlags & RPFLAG_TRIPNUMBERS;
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT85.TXT");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
  for(nI = 0; nI < 4; nI++)
  {
    bGotServiceIndex[nI] = FALSE;
  }
//
//  Get the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[0];
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(szDivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(szDivisionName, DIVISIONS_NAME_LENGTH);
//
//  Get the service name
//
  SERVICESKey0.recordID = REPORTPARMS.pServiceList[0];
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(szServiceName, SERVICES_NAME_LENGTH);
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
    bGotServiceIndex[serviceIndex] = TRUE;
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
  }
//
//  Get all the runs
//
  RUNSKey1.DIVISIONSrecordID = REPORTPARMS.pDivisionList[0];
  RUNSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[0];
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  numInRunsave = 0;
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == REPORTPARMS.pDivisionList[0] &&
        RUNS.SERVICESrecordID == REPORTPARMS.pServiceList[0] &&
        numInRunsave < MAXRUNSAVE)
  {
    RUNSAVE[numInRunsave].runNumber = RUNS.runNumber;
    RUNSAVE[numInRunsave].start = RUNS.start;
    RUNSAVE[numInRunsave].end = RUNS.end;
    numInRunsave++;
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Start the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Loop through the runs
//
  bKeepGoing = TRUE;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numRuns; nI++)
  {
    StatusBar((long)(nI + 1), (long)REPORTPARMS.numRuns);
    if(StatusBarAbort())
    {
      goto deallocate;
    }
//
//  Get the run
//
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
//
//  Cost the run
//
    runNumber = RUNS.runNumber;
    LoadString(hInst, TEXT_120, szarString, sizeof(szarString));
    sprintf(tempString, szarString, szServiceName, runNumber);
    StatusBarText(tempString);
    StatusBar((long)nI, (long)REPORTPARMS.numRuns);
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      break;
    }
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
//
//  Runtype
//
    nJ = (short int)LOWORD(RUNSVIEW[0].runType);
    nK = (short int)HIWORD(RUNSVIEW[0].runType);
    if(nJ == NO_RECORD || nK == NO_RECORD)
    {
      strcpy(szRuntype, "");
    }
    else
    {
      strcpy(szRuntype, RUNTYPE[nJ][nK].localName);
    }
//
//  Report at time
//
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      pieceReportTime[nJ] = RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime - RUNSVIEW[nJ].startOfPieceTravel;
      if(RUNSVIEW[nJ].startOfPieceExtraboardStart != NO_TIME)
      {
        pieceReportTime[nJ] = RUNSVIEW[nJ].startOfPieceExtraboardStart;
      }
    }
//
//  Pay time
//
    strcpy(szPayTime, chhmm(RUNSVIEW[numPieces - 1].payTime));
//
//  Off time
//
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      pieceEndTime[nJ] = RUNSVIEW[nJ].runOffTime +
            RUNSVIEW[nJ].turninTime + RUNSVIEW[nJ].endOfPieceTravel;
      if(RUNSVIEW[nJ].endOfPieceExtraboardEnd != NO_TIME)
      {
        pieceEndTime[nJ] = RUNSVIEW[nJ].endOfPieceExtraboardEnd;
      }
    }
    strcpy(szReportTime, Tchar(pieceReportTime[0]));
    strcpy(szEndTime, Tchar(pieceEndTime[numPieces - 1]));
//
//  Cycle through this run/piece
//
    numComments = 0;
    numSigncodes = 0;
    for(nJ = 0; nJ < numPieces; nJ++)
    {
//
//  Get a run record.  If it's the first piece, re-establish position.
//
      if(nJ != 0)
      {
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      else
      {
        RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        runNumber = RUNS.runNumber;
        runCommentRecordID = RUNS.COMMENTSrecordID;
        bGotRunComment = runCommentRecordID != NO_RECORD;
      }
//
//  Start of piece information
//
      sprintf(szarString, "> > > > Piece %d of %d < < < <", nJ + 1, numPieces);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Report at time and location
//
      tempLong = RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime - RUNSVIEW[nJ].startOfPieceTravel;
      if(RUNSVIEW[nJ].startOfPieceExtraboardStart != NO_TIME)
      {
        tempLong = RUNSVIEW[nJ].startOfPieceExtraboardStart;
      }
      NODESKey0.recordID = RUNSVIEW[nJ].startOfPieceNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      sprintf(szarString, "Report at %s at %s", tempString, Tchar(tempLong));
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Any "prior to piece" extraboard?
//
      if(RUNS.prior.startTime != NO_TIME && 
            (RUNS.prior.startTime != 0L && RUNS.prior.endTime != 0L))
      {
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        strcpy(szarString, "Extraboard duty:");
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "  Start at: %s\n", Tchar(RUNS.prior.startTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "  End at: %s\n", Tchar(RUNS.prior.endTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  Dump out any travel instructions
//
      fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].startNODESrecordID;
      toNode = COST.TRAVEL[RUNS.pieceNumber - 1].startAPointNODESrecordID;
      if(fromNode != NO_RECORD && toNode != NO_RECORD && fromNode != toNode)
      {
//
//  Using the trip planner
//
        if(bUseCISPlan)
        {
          strcpy(travelDirections, CISplanReliefConnect(&startTime, &endTime, &dwellTime,
                TRUE, fromNode, toNode, COST.TRAVEL[RUNS.pieceNumber - 1].startAPointTime,
                FALSE, RUNS.SERVICESrecordID));
          if(RUNSVIEW[nJ].startOfPieceTravel != 0)
          {
            strcpy(szarString, "Travel Instructions:\n");
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            ptr = strtok(travelDirections, lineSep);
            while(ptr != 0)
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                     seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                     szRuntype, szReportTime, szPayTime, szEndTime, ptr);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              ptr = strtok(NULL, lineSep);
            }
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          }
        }
//
//  Not using the trip planner
//
        else
        {
          if(fromNode != NO_RECORD && toNode != NO_RECORD &&
                !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
          {
            NODESKey0.recordID = fromNode;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromNode, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(szFromNode, NODES_INTERSECTION_LENGTH);
            NODESKey0.recordID = toNode;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToNode, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(szToNode, NODES_INTERSECTION_LENGTH);
            if(COST.TRAVEL[RUNS.pieceNumber - 1].startTravelTime > 0)
            {
              tempLong = COST.TRAVEL[RUNS.pieceNumber - 1].startAPointTime -
                    COST.TRAVEL[RUNS.pieceNumber - 1].startTravelTime;
              strcpy(szStartTime, Tchar(tempLong));
              sprintf(szarString, "Block: %ld - Travel at %s from %s to %s to arrive by %s",
                    TRIPS.standard.blockNumber, szStartTime,
                    szFromNode, szToNode, Tchar(COST.TRAVEL[RUNS.pieceNumber - 1].startAPointTime));
            }
            else
            {
              sprintf(szarString, "Block: %ld - Travel from %s to %s to arrive by %s",
                    TRIPS.standard.blockNumber, szFromNode, szToNode,
                    Tchar(COST.TRAVEL[RUNS.pieceNumber - 1].startAPointTime));
            }
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          }
        }
      }
//
//  Follow the block to get the trips
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      blockNumber = TRIPS.standard.blockNumber;
      bFirstTrip = TRUE;
      bFirstNode = FALSE;
      rcode3 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Go through the trips on the block
//
      while(rcode3 == 0 &&
            TRIPS.standard.blockNumber == blockNumber)
      {
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
        bLastTrip = (TRIPS.recordID == RUNS.end.TRIPSrecordID);
//
//  Sign code
//
        SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
          trim(tempString, SIGNCODES_CODE_LENGTH);
          sprintf(szarString, "Sign code: %s", tempString);
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }
//
//  Trip number
//
        if(bIncludeTripNumbers)
        {
          sprintf(szarString, "Trip #: %ld", TRIPS.tripNumber);
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }
//
//  First time through
//
        if(bFirstTrip)
        {
//
//  Is this a pull-out? - No
//
          if(TRIPS.standard.POGNODESrecordID == NO_RECORD || TRIPS.standard.POGNODESrecordID != RUNS.start.NODESrecordID)
          {
//            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
//                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
//                  szRuntype, szReportTime, szPayTime, szEndTime);
//            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            sprintf(szarString, "Block: %ld", TRIPS.standard.blockNumber);
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            reliefRunNumber = NO_RECORD;
            for(nM = 0; nM < numInRunsave; nM++)
            {
              if(NodesEquivalent(RUNS.start.NODESrecordID, RUNSAVE[nM].end.NODESrecordID, &equivalentTravelTime) &&
                    RUNS.start.TRIPSrecordID == RUNSAVE[nM].end.TRIPSrecordID)
              {
                reliefRunNumber = RUNSAVE[nM].runNumber;
                break;
              }
            }
            if(reliefRunNumber != NO_RECORD)
            {
//              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
//                    seq++, szDivisionName, szServiceName, RUNS.runNumber, 
//                    szRuntype, szReportTime, szPayTime, szEndTime);
//              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              sprintf(szarString, "This piece of work relieves run %ld", reliefRunNumber);
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                    szRuntype, szReportTime, szPayTime, szEndTime, szarString);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            }
          }
//
//  Yes, it's a pull-out
//
          else
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
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            sprintf(szarString, "Block: %ld", TRIPS.standard.blockNumber);
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            NODESKey0.recordID = GTResults.firstNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(intersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(intersection, NODES_INTERSECTION_LENGTH);
            NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(garageAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(garageAbbr, NODES_ABBRNAME_LENGTH);
            sprintf(szarString, "Pull Out from %s at %s", garageAbbr, Tchar(GTResults.firstNodeTime - deadheadTime));
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            if(deadheadTime > 0)
            {
              sprintf(szarString, "Deadhead to %s", intersection);
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                    szRuntype, szReportTime, szPayTime, szEndTime, szarString);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            }
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            bFirstTrip = FALSE;  // Cause the pattern will start with the first timepoint on a P/O
          }
        }
//
//  Get the comment code of the trip
//
        COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        if(rcode2 != 0)
        {
          strcpy(commentCode, "");
        }
        else
        {
          memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
          strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(commentCode, COMMENTS_CODE_LENGTH);
          for(bFound = FALSE, nM = 0; nM < numComments; nM++)
          {
            if(COMMENTS.recordID == commentRecordIDs[nM])
            {
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
          {
            commentRecordIDs[numComments++] = COMMENTS.recordID;
          }
        }
//
//  Route number, name, direction
//
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(szarString, "Route %s %s", szRouteNumber, tempString);
        if(strcmp(commentCode, "") == 0)
        {
          strcat(szarString, ":");
        }
        else
        {
          strcat(szarString, "  Note: ");
          strcat(szarString, commentCode);
        }
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Go through the pattern for this trip
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
            if(bFirstTrip && RUNS.start.NODESrecordID != PATTERNS.NODESrecordID)
            {
            }
            else
            {
              strcpy(szarString, Tchar(GTResults.tripTimes[numNodes]));
              strcat(szarString, " - ");
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
              trim(tempString, NODES_DESCRIPTION_LENGTH);
              strcat(szarString, tempString);
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                    szRuntype, szReportTime, szPayTime, szEndTime, szarString);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              if(bLastTrip && PATTERNS.NODESrecordID == RUNS.end.NODESrecordID)
              {
                break;
              }
              bFirstTrip = FALSE;
            }
            numNodes++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Get the next trip on the block
//
        if(bLastTrip)
        {
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  Done with the trips on this block
//
//  Blank line
//
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);

//
//  Is this a pull-in?  If so, get the time.
//
      if(TRIPS.standard.PIGNODESrecordID != NO_RECORD && TRIPS.standard.PIGNODESrecordID == RUNS.end.NODESrecordID)
      {
        strcpy(szarString, "Pull-in to:");
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szarString, NODES.description, NODES_DESCRIPTION_LENGTH);
        trim(szarString, NODES_DESCRIPTION_LENGTH);
        strcat(szarString, " for ");
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
        GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
        GCTData.timeOfDay = GTResults.lastNodeTime;
        deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        strcat(szarString, Tchar(GTResults.lastNodeTime + deadheadTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  Not a pull-in - get next route/run information if available
//
      else
      {
//
//  Only show the next route if we're at the end of the trip
//
        if(RUNS.end.NODESrecordID == GTResults.lastNODESrecordID)
        {
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          if(rcode2 == 0 && TRIPS.standard.blockNumber == blockNumber)
          {
            ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(tempString, ROUTES_NUMBER_LENGTH);
            sprintf(szarString, "Next route on this block: %s", tempString);
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          }
        }
//
//  Show who's relieving us
//
        for(nK = 0; nK < numInRunsave; nK++)
        {
          if(NodesEquivalent(RUNS.end.NODESrecordID, RUNSAVE[nK].start.NODESrecordID, &equivalentTravelTime) &&
                RUNS.end.TRIPSrecordID == RUNSAVE[nK].start.TRIPSrecordID &&
                RUNS.runNumber != RUNSAVE[nK].runNumber)
          {
            sprintf(szarString, "Relieved by run %ld", RUNSAVE[nK].runNumber);
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            break;
          }
        }
      }
//
//  End of piece travel instructions
//
      fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].endBPointNODESrecordID;
      toNode = COST.TRAVEL[RUNS.pieceNumber - 1].endNODESrecordID;
      if(bUseCISPlan)
      {
        if(fromNode != NO_RECORD && toNode != NO_RECORD && fromNode != toNode)
        {
          strcpy(szarString, "Travel Instructions:");
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          strcpy(travelDirections, CISplanReliefConnect(&startTime, &endTime, &dwellTime,
                TRUE, fromNode, toNode, COST.TRAVEL[RUNS.pieceNumber - 1].endBPointTime,
                TRUE, SERVICES.recordID));
          ptr = strtok(travelDirections, lineSep);
          while(ptr != 0)
          {
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, ptr);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            ptr = strtok(NULL, lineSep);
          }
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }
      }
//
//  Not using the trip planner
//
      else
      {
        if(fromNode != NO_RECORD && toNode != NO_RECORD &&
              !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
        {
          strcpy(szarString, "Travel Instructions:");
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          NODESKey0.recordID = fromNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szFromNode, NODES.intersection, NODES_INTERSECTION_LENGTH);
          trim(szFromNode, NODES_INTERSECTION_LENGTH);
          NODESKey0.recordID = toNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szToNode, NODES.intersection, NODES_INTERSECTION_LENGTH);
          trim(szToNode, NODES_INTERSECTION_LENGTH);
          tempLong = COST.TRAVEL[RUNS.pieceNumber - 1].endBPointTime +
                COST.TRAVEL[RUNS.pieceNumber - 1].endTravelTime;
          sprintf(szarString, "Travel from %s to %s to arrive at %s",
                szFromNode, szToNode, Tchar(tempLong));
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }
      }
//
//  Any extraboard?
//
      if(RUNS.after.startTime != NO_TIME &&
            (RUNS.after.startTime != 0L && RUNS.after.endTime != 0L))
      {
        strcpy(szarString, "Extraboard duty:");
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "  Start at: %s\n",
              Tchar(RUNS.after.startTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "  End at: %s\n",
              Tchar(RUNS.after.endTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  Off at
//
//
//  Off time and location
//
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      tempLong = RUNSVIEW[nJ].runOffTime + RUNSVIEW[nJ].turninTime + RUNSVIEW[nJ].endOfPieceTravel;
      if(RUNSVIEW[nJ].endOfPieceExtraboardEnd != NO_TIME)
      {
        tempLong = RUNSVIEW[nJ].endOfPieceExtraboardEnd;
      }
      NODESKey0.recordID = RUNSVIEW[nJ].endOfPieceNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      sprintf(szarString, "Off at %s at %s", tempString, Tchar(tempLong));
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      if(nJ == numPieces - 1)
      {
        break;
      }
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
    }  // nJ (through number of pieces)
//
//  Spit out the comments
//
      if(numComments > 0)
      {
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        strcpy(szarString, "Notes:");
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        for(nK = 0; nK < numComments; nK++)
        {
          recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
          COMMENTSKey0.recordID = commentRecordIDs[nK];
          btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
          recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
          memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
          strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(szarString, COMMENTS_CODE_LENGTH);
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          szarString[0] = 0;
          for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
               tempString2 = strtok(NULL, "\r\n") )
          {
            strcpy(szarString, tempString2 );
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"    %s\"\r\n",
                  seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          }
        }
      }
//
//  Spit out the sign codes
//
      if(numSigncodes > 0 && bIncludeSignCodes)
      {
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        strcpy(szarString, "Signcodes:");
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        for(nK = 0; nK < numSigncodes; nK++)
        {
          SIGNCODESKey0.recordID = signcodeRecordIDs[nK];
          btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
          strncpy(szarString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
          trim(szarString, SIGNCODES_CODE_LENGTH);
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          strncpy(szarString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
          trim(szarString, SIGNCODES_TEXT_LENGTH);
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"    %s\"\r\n",
                seq++, szDivisionName, szServiceName, RUNS.runNumber, 
                szRuntype, szReportTime, szPayTime, szEndTime, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }
      }
//
//  Do the run comment
//
    if(bGotRunComment)
    {
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      strcpy(szarString, "Run comment:");
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      COMMENTSKey0.recordID = runCommentRecordID;
      btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
      strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
      trim(szarString, COMMENTS_CODE_LENGTH);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s\"\r\n",
            seq++, szDivisionName, szServiceName, RUNS.runNumber, 
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      szarString[0] = 0;
      for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
           tempString2 = strtok(NULL, "\r\n") )
      {
        strcpy(szarString, tempString2 );
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"    %s\"\r\n",
              seq++, szDivisionName, szServiceName, RUNS.runNumber, 
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
    }
  }  // nI  (through run list)


//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pRunList);
    StatusBarEnd();
    CloseHandle(hOutputFile);
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
