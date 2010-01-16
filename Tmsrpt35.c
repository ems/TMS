//
//  TMSRPT35() - Driver Paddle (2)
//
//  From QETXT.INI
//
//    [TMSRPT35]
//    FILE=TMSRPT35.TXT
//    FLN=0
//    TT=Tab
//    FIELD1=Sequence,NUMERIC,7,0,7,0,
//    FIELD2=Division,VARCHAR,32,0,32,0,
//    FIELD3=Service,VARCHAR,32,0,32,0,
//    FIELD4=RunNumber,NUMERIC,8,0,8,0,
//    FIELD5=PieceNumber,NUMERIC,8,0,8,0,
//    FIELD6=TotalPieces,NUMERIC,8,0,8,0,
//    FIELD7=Runtype,VARCHAR,20,0,20,0,
//    FIELD8=ReportTime,VARCHAR,8,0,8,0,
//    FIELD9=PayTime,VARCHAR,8,0,8,0,
//    FIELD10=EndTime,VARCHAR,8,0,8,0,
//    FIELD11=Text,VARCHAR,256,0,256,0,
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

#define MAXTRIPSAVE 200

#define TRIPSAVE_FLAG_FIRSTTRIP 0x0001
#define TRIPSAVE_FLAG_LASTTRIP  0x0002
#define TRIPSAVE_FLAG_PULLOUT   0x0004

typedef struct TRIPSAVEStruct
{
  long SERVICESrecordID;
  long serviceNumber;
  long ROUTESrecordID;
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
  long directionIndex;
  long TRIPSrecordID;
  long startTime;
  long startNode;
  long endTime;
  long endNode;
  long nextROUTESrecordID;
  long nextStartTime;
  long flags;
} TRIPSAVEDef;

#define MAXRUNSAVE 1000

typedef struct RUNSAVEStruct
{
  long runNumber;
  STARTENDDef start;
  STARTENDDef end;
} RUNSAVEDef;

//
//  sort_tripsave: Sort a TRIPSAVE structure on service, route, direction, and start time
//
int sort_tripsave(const void *a, const void *b)
{
  TRIPSAVEDef *pa, *pb;
  int nI;

  pa = (TRIPSAVEDef *)a;
  pb = (TRIPSAVEDef *)b;

//
//  Service
//
  if(pa->serviceNumber < pb->serviceNumber)
    return(-1);
  else if(pa->serviceNumber > pb->serviceNumber)
    return(1);
//
//  Route
//
  else
  {
    nI = strcmp(pa->routeNumber, pb->routeNumber);
    if(nI != 0)
      return(nI);
//
//  Direction
//
    else
    {
      if(pa->directionIndex < pb->directionIndex)
        return(-1);
      else if(pa->directionIndex > pb->directionIndex)
        return(1);
//
//  Start time
//
      else
      {
        return(pa->startTime < pb->startTime ? -1 : pa->startTime > pb->startTime ? 1 : 0);
      }
    }
  }
}


BOOL FAR TMSRPT35(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef GTResults;
  REPORTPARMSDef  REPORTPARMS;
  TRIPSAVEDef TRIPSAVEFIRST[MAXTRIPSAVE];     
  TRIPSAVEDef TRIPSAVEREST[MAXTRIPSAVE];     
  TRIPSAVEDef *pTRIPSAVE, *pPriorTRIPSAVE;
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
  BOOL  bGotFirst;
  BOOL  bGotLast;
  BOOL  bIncludeSignCodes;
  BOOL  bIncludeTripNumbers;
  BOOL  bIncludeSpreadTime;
  char  travelDirections[1024];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
  char  nextRoute[32];
  char  signCode[SIGNCODES_CODE_LENGTH + 1];
  char  nodeNames[RECORDIDS_KEPT][NODES_LONGNAME_LENGTH + 1];
  char  nodeName[NODES_LONGNAME_LENGTH + 1];
  char  intersection[NODES_INTERSECTION_LENGTH + 1];
  char  garageAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  *ptr;
  char  *tempString2;
  char  szRuntype[40];
  char  szReportTime[8];
  char  szPayTime[8];
  char  szEndTime[8];
  char  szPOTime[8];
  char  szPITime[8];
  char  szStartTime[8];
  char  szParkForRoute[ROUTES_NUMBER_LENGTH + 1];
  char  szFromNode[NODES_INTERSECTION_LENGTH + 1];
  char  szToNode[NODES_INTERSECTION_LENGTH + 1];
  char  lineSep[3] = {13, 10, 0};
  char  szTripNumber[16];
  long  baseNodeRecordIDs[RECORDIDS_KEPT];
  long  tripNodeRecordIDs[RECORDIDS_KEPT];
  long  commentRecordIDs[100];
  long  signcodeRecordIDs[100];
  long  runNumber;
  long  blockNumber;
  long  runCommentRecordID;
  long  serviceNumber;
  long  previousRouteRecordID;
  long  previousDirectionIndex;
  long  deadheadTime;
  long  tempLong;
  long  travelTime;
  long  dwellTime;
  long  fromNode;
  long  toNode;
  long  startTime;
  long  endTime;
  long  equivalentTravelTime;
  long  reliefRunNumber;
  int   limit;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   nN;
  int   numBaseNodes;
  int   numTripNodes;
  int   numPieces;
  int   numTripsFirst;
  int   numTripsRest;
  int   numComments;
  int   numSigncodes;
  int   numInRunsave;
  int   rcode2;
  int   rcode3;
  int   seq;
  int   serviceIndex;

  pPassedData->nReportNumber = 34;
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
  strcat(tempString, "\\TMSRPT35.TXT");
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
  strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Get the service name
//
  SERVICESKey0.recordID = REPORTPARMS.pServiceList[0];
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
    bGotServiceIndex[serviceIndex] = TRUE;
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
        goto deallocate;
    }
    else
    {
      InitTravelMatrix(SERVICES.recordID, FALSE);
    }
  }
//
//  As if he want's to see the spread time as part of the runtype
//
  MessageBeep(MB_ICONQUESTION);
  LoadString(hInst, TEXT_379, tempString, TEMPSTRING_LENGTH);
  bIncludeSpreadTime = (MessageBox(NULL, tempString, TMS, MB_YESNO) == IDYES);
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
    sprintf(tempString, szarString, serviceName, runNumber);
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
    if(bIncludeSpreadTime)
    {
      sprintf(tempString, "/%s", chhmm(RUNSVIEW[numPieces - 1].spreadTime));
      strcat(szRuntype, tempString);
    }
//
//  Report at time
//
    tempLong = RUNSVIEW[0].runOnTime - RUNSVIEW[0].reportTime - RUNSVIEW[0].startOfPieceTravel;
    if(RUNSVIEW[0].startOfPieceExtraboardStart != NO_TIME)
      tempLong = RUNSVIEW[0].startOfPieceExtraboardStart;
    strcpy(szReportTime, Tchar(tempLong));
//
//  Pay time
//
    strcpy(szPayTime, chhmm(RUNSVIEW[numPieces - 1].payTime));
//
//  Off time
//
    tempLong = RUNSVIEW[numPieces - 1].runOffTime +
          RUNSVIEW[numPieces - 1].turninTime + RUNSVIEW[numPieces - 1].endOfPieceTravel;
    if(RUNSVIEW[numPieces - 1].endOfPieceExtraboardEnd != NO_TIME)
     tempLong = RUNSVIEW[numPieces - 1].endOfPieceExtraboardEnd;
    strcpy(szEndTime, Tchar(tempLong));
//
//  Get all the trips in this run/piece
//
    numComments = 0;
    numSigncodes = 0;
    for(nJ = 0; nJ < numPieces; nJ++)
    {
//
//  Get a run record.  If it's the first piece, re-establish position.
//
      if(nJ != 0)
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
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
//  Follow the block to get the trips
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      blockNumber = TRIPS.standard.blockNumber;
      rcode3 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      numTripsFirst = 0;
      numTripsRest = 0;
      pPriorTRIPSAVE = NULL;
      while(rcode3 == 0 &&
            TRIPS.standard.blockNumber == blockNumber)
      {
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
        SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        serviceNumber = SERVICES.number;
        if(numTripsFirst == 0)
          pTRIPSAVE = &TRIPSAVEFIRST[0];
        else
        {
          if(TRIPS.SERVICESrecordID == TRIPSAVEFIRST[0].SERVICESrecordID &&
                TRIPS.ROUTESrecordID == TRIPSAVEFIRST[0].ROUTESrecordID &&
                TRIPS.directionIndex == TRIPSAVEFIRST[0].directionIndex)
          {
            pTRIPSAVE = &TRIPSAVEFIRST[numTripsFirst];
          }
          else
          {
            pTRIPSAVE = &TRIPSAVEREST[numTripsRest];
          }
        }
        if(pTRIPSAVE == &TRIPSAVEFIRST[0])
          pTRIPSAVE->flags = TRIPSAVE_FLAG_FIRSTTRIP;
        else
          pTRIPSAVE->flags = 0;
        pTRIPSAVE->SERVICESrecordID = TRIPS.SERVICESrecordID;
        pTRIPSAVE->serviceNumber = serviceNumber;
        pTRIPSAVE->ROUTESrecordID = TRIPS.ROUTESrecordID;
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(pTRIPSAVE->routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(pTRIPSAVE->routeNumber, ROUTES_NUMBER_LENGTH);
        pTRIPSAVE->directionIndex = TRIPS.directionIndex;
        pTRIPSAVE->TRIPSrecordID = TRIPS.recordID;
        pTRIPSAVE->startTime = GTResults.firstNodeTime;
        pTRIPSAVE->startNode = GTResults.firstNODESrecordID;
        pTRIPSAVE->endTime = GTResults.lastNodeTime;
        pTRIPSAVE->endNode = GTResults.lastNODESrecordID;
        pTRIPSAVE->nextROUTESrecordID = NO_RECORD;
        pTRIPSAVE->nextStartTime = NO_TIME;
        if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
        {
          pTRIPSAVE->flags |= TRIPSAVE_FLAG_PULLOUT;
        }
        if(pPriorTRIPSAVE)
        {
          pPriorTRIPSAVE->nextROUTESrecordID = pTRIPSAVE->ROUTESrecordID;
          pPriorTRIPSAVE->nextStartTime = pTRIPSAVE->startTime;
        }
        if(pTRIPSAVE == &TRIPSAVEFIRST[numTripsFirst])
        {
          numTripsFirst++;
          if(numTripsFirst > MAXTRIPSAVE)
            break;
        }
        else
        {
          numTripsRest++;
          if(numTripsRest > MAXTRIPSAVE)
            break;
        }
        pPriorTRIPSAVE = pTRIPSAVE;
        if(TRIPS.recordID == RUNS.end.TRIPSrecordID)
        {
          pTRIPSAVE->flags |= TRIPSAVE_FLAG_LASTTRIP;  
          if(GTResults.lastNODESrecordID == RUNS.end.NODESrecordID)
          {
            rcode3 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            if(rcode3 == 0 && TRIPS.standard.blockNumber == blockNumber)
            {
              pTRIPSAVE->nextROUTESrecordID = TRIPS.ROUTESrecordID;
            }
          }        
          break;
        }
        rcode3 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      }  // while rcode3 == 0
//
//  Sort TRIPSAVEREST based on service, route number, direction, and start time.
//  Viewers with a keen eye will realize that TRIPSAVEFIRST is already sorted.
//
      qsort((void *)TRIPSAVEREST, numTripsRest, sizeof(TRIPSAVEDef), sort_tripsave);
//
//  Go through the sorted trips
//
      for(nK = 0; nK < 2; nK++)
      {
        if(nK == 0)
          limit = numTripsFirst;
        else
          limit = numTripsRest;
//
//  Do either TRIPSAVE or TRIPREST
//
        previousRouteRecordID = NO_RECORD;
        previousDirectionIndex = NO_RECORD;
        for(nL = 0; nL < limit; nL++)
        {
          if(nK == 0)
            pTRIPSAVE = &TRIPSAVEFIRST[nL];
          else
            pTRIPSAVE = &TRIPSAVEREST[nL];
//
//  Get the trip
//
          TRIPSKey0.recordID = pTRIPSAVE->TRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Is this a pull-out?  If so, get the time.
//
          if(TRIPS.standard.POGNODESrecordID == NO_RECORD || TRIPS.standard.POGNODESrecordID != RUNS.start.NODESrecordID)
            strcpy(szPOTime, "");
          else
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
            GCTData.toNODESrecordID = pTRIPSAVE->startNode;
            GCTData.timeOfDay = pTRIPSAVE->startTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            strcpy(szPOTime, Tchar(pTRIPSAVE->startTime - deadheadTime));
          }
          if(pTRIPSAVE->ROUTESrecordID != previousRouteRecordID ||
                pTRIPSAVE->directionIndex != previousDirectionIndex)
          {
//
//  Start of piece information
//
            if(nL == 0 && pTRIPSAVE == &TRIPSAVEFIRST[0])
            {
//
//  Report at time and location
//
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
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
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, szarString);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Any extraboard?
//
              if(RUNS.prior.startTime != NO_TIME && 
                    (RUNS.prior.startTime != 0L && RUNS.prior.endTime != 0L))
              {
                sprintf(szarString, "*** Extraboard duty: Start at: %s\n",
                      Tchar(RUNS.prior.startTime));
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                sprintf(szarString, "*** Extraboard duty:   End at: %s\n",
                      Tchar(RUNS.prior.endTime));
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              }
//
//  A pull-out?
//
              if(pTRIPSAVE->flags & TRIPSAVE_FLAG_PULLOUT && TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID)
              {
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                sprintf(szarString, "Block: %ld", TRIPS.standard.blockNumber);
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                NODESKey0.recordID = GTResults.firstNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(nodeName, NODES.longName, NODES_LONGNAME_LENGTH);
                trim(nodeName, NODES_LONGNAME_LENGTH);
                strncpy(intersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
                trim(intersection, NODES_INTERSECTION_LENGTH);
                NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(garageAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                trim(garageAbbr, NODES_ABBRNAME_LENGTH);
                sprintf(szarString, "Pull Out from %s at %s, Deadhead to %s (%s)",
                      garageAbbr, szPOTime, nodeName, intersection);
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              }
//
//  Dump out any travel instructions
//
              fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].startNODESrecordID;
              toNode = COST.TRAVEL[RUNS.pieceNumber - 1].startAPointNODESrecordID;
//              if(fromNode != NO_RECORD && toNode != NO_RECORD &&
//                    !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
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
                  if(startTime == -25 * 60 * 60)
                  {
                    travelTime = NO_TIME;
                  }
                  else
                  {
                    travelTime = (endTime - startTime) + dwellTime;
                  }
//                  if(travelTime != 0)
                  if(RUNSVIEW[nJ].startOfPieceTravel != 0)
                  {
//                    sprintf(szarString, "Piece %d: Start Travel Instructions:\n", RUNS.pieceNumber);
                    strcpy(szarString, "Start Travel Instructions:\n");
                    sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                           seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                           szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                    ptr = strtok(travelDirections, lineSep);
                    while(ptr != 0)
                    {
                      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                            szRuntype, szReportTime, szPayTime, szEndTime, ptr);
                      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                      ptr = strtok(NULL, lineSep);
                    }
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
                    sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                          seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                          szRuntype, szReportTime, szPayTime, szEndTime);
                    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
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
                    sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                          seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                          szRuntype, szReportTime, szPayTime, szEndTime, szarString);
                    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                  }
                }
              }
//
//  No pullout - see who we're relieving
//
              if(!((pTRIPSAVE->flags & TRIPSAVE_FLAG_PULLOUT) && TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID))
              {
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                      szRuntype, szReportTime, szPayTime, szEndTime);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                sprintf(szarString, "Block: %ld", TRIPS.standard.blockNumber);
                sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                      seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
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
                  sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                        seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                        szRuntype, szReportTime, szPayTime, szEndTime);
                  WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                  sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"This piece of work relieves run %ld\"\r\n",
                        seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                        szRuntype, szReportTime, szPayTime, szEndTime, reliefRunNumber);
                  WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
                }
              }
            }
//
//  Blank line
//
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Route number, name, direction
//
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"ROUTE %s ",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime, pTRIPSAVE->routeNumber);
            ROUTESKey0.recordID = TRIPSAVEFIRST[0].ROUTESrecordID;
            ROUTESKey0.recordID = pTRIPSAVE->ROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[pTRIPSAVE->directionIndex];
            btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
            strncpy(szarString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
            trim(szarString, DIRECTIONS_LONGNAME_LENGTH);
            strcat(tempString, szarString);
            strcat(tempString, "\"\r\n");
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Blank line
//
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Get the trip
//
            TRIPSKey0.recordID = pTRIPSAVE->TRIPSrecordID;
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Get the base pattern nodes for this route/ser/dir
//
             
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
            PATTERNSKey2.nodeSequence = 0;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            numBaseNodes = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                NODESKey0.recordID = PATTERNS.NODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(nodeNames[numBaseNodes], NODES.longName, NODES_LONGNAME_LENGTH);
                nodeNames[numBaseNodes][NODES_LONGNAME_LENGTH] = '\0';
                baseNodeRecordIDs[numBaseNodes] = PATTERNS.NODESrecordID;
                numBaseNodes++;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
//
//  Write out the two title lines for this route/ser/dir
//
//  Line 1
//
            if(bIncludeTripNumbers)
              strcpy(szarString, "Sign  Comment   Trip #   P/O    ");
            else
              strcpy(szarString, "Sign  Comment   P/O    ");
            for(nM = 0; nM < numBaseNodes; nM++)
            {
              strncpy(nodeName, nodeNames[nM], 4);
              nodeName[4] = '\0';
              strcat(szarString, nodeName);
              strcat(szarString, "   ");
            }
            strcat(szarString, "Next     Next");
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Write out the two title lines for this route/ser/dir
//
//  Line 2
//
            if(bIncludeTripNumbers)
              strcpy(szarString, "Code  Code               Time   ");
            else
              strcpy(szarString, "Code  Code      Time   ");
            for(nM = 0; nM < numBaseNodes; nM++)
            {
              strcpy(nodeName, &nodeNames[nM][4]);
              strcat(szarString, nodeName);
              strcat(szarString, "   ");
            }
            strcat(szarString, "Route    Trip");
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Done the header
//
            previousRouteRecordID = pTRIPSAVE->ROUTESrecordID;
            previousDirectionIndex = pTRIPSAVE->directionIndex;
          }  // if not the same route/dir as previous record
//
//  Spit out the trips
//
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the sign code of the trip
//
          SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
          if(rcode2 != 0)
            strcpy(signCode, "");
          else
          {
            strncpy(signCode, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
            trim(signCode, SIGNCODES_CODE_LENGTH);
            for(bFound = FALSE, nM = 0; nM < numSigncodes; nM++)
            {
              if(SIGNCODES.recordID == signcodeRecordIDs[nM])
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              signcodeRecordIDs[numSigncodes++] = SIGNCODES.recordID;
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
            strcpy(commentCode, "");
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
//  Get the trip number of the trip
//
          if(TRIPS.tripNumber == NO_RECORD)
            strcpy(szTripNumber, "");
          else
          {
            sprintf(szTripNumber, "%8ld", TRIPS.tripNumber);
          }
//
//  Is this a pull-in?  If so, get the time.
//
          if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
          {
            NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(garageAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(garageAbbr, NODES_ABBRNAME_LENGTH);
            sprintf(nextRoute, "P/IN to %s @  ", garageAbbr);
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = pTRIPSAVE->endNode;
            GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
            GCTData.timeOfDay = pTRIPSAVE->endTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            strcpy(szPITime, Tchar(pTRIPSAVE->endTime + deadheadTime));
          }
//
//  Not a pull-in - get next route information if available
//
          else
          {
            if(pTRIPSAVE->flags & TRIPSAVE_FLAG_LASTTRIP)
            {
              strcpy(nextRoute, "RLF     ");
              strcpy(szPITime, "");
              strcpy(szParkForRoute, "");
              if(pTRIPSAVE->nextROUTESrecordID == NO_RECORD)
                strcpy(szPITime, "");
              else
              {
                ROUTESKey0.recordID = pTRIPSAVE->nextROUTESrecordID;
                btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                strncpy(szParkForRoute, ROUTES.number, ROUTES_NUMBER_LENGTH);
                trim(szParkForRoute, ROUTES_NUMBER_LENGTH);
              }
            }
            else
            {
              ROUTESKey0.recordID = pTRIPSAVE->nextROUTESrecordID;
              btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
              strncpy(nextRoute, ROUTES.number, ROUTES_NUMBER_LENGTH);
              trim(nextRoute, ROUTES_NUMBER_LENGTH);
              ptr = nextRoute;
              while(*ptr == ' ')
              {
                ptr++;
              }
              strcpy(nextRoute, ptr);
              pad(nextRoute, 8);
              nextRoute[8] = '\0';
              strcpy(szPITime, Tchar(pTRIPSAVE->nextStartTime));
            }
          }
//
//  Start building the output string
//
//  Sign Code
//  Comment Code
//  Trip number
//  PO Time
//
          pad(signCode, 6);
          signCode[6] = '\0';
          pad(commentCode, 8);
          commentCode[8] = '\0';
          pad(szTripNumber, 8);
          szTripNumber[8] = '\0';
          pad(szPOTime, 8);
          szPOTime[7] = '\0';
          pad(szPITime, 8);
          szPITime[7] = '\0';
          if(timeFormat == PREFERENCES_MILITARY)
          {
            if(bIncludeTripNumbers)
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%6s%8s %8s  %6s",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, signCode, commentCode, szTripNumber, szPOTime);
            }
            else
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%6s%8s  %6s",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, signCode, commentCode, szPOTime);
            }
          }
          else
          {
            if(bIncludeTripNumbers)
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%6s%8s%8s  %6s",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, signCode, commentCode, szTripNumber, szPOTime);
            }
            else
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%6s%8s %6s",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, signCode, commentCode, szPOTime);
            }
          }
//
//  Dumping the times is easy if this is the base pattern
//
          bGotFirst = FALSE;
          bGotLast = FALSE;
          if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            nM = 0;
            while(nM < numBaseNodes)
            {
              if((pTRIPSAVE->flags & TRIPSAVE_FLAG_FIRSTTRIP) && !bGotFirst) 
              {
                if(baseNodeRecordIDs[nM] == RUNS.start.NODESrecordID ||
                      ((pTRIPSAVE->flags & TRIPSAVE_FLAG_PULLOUT) && TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID))
                {
                  strcat(tempString, Tchar(GTResults.tripTimes[nM]));
                  strcat(tempString, timeFormat == PREFERENCES_MILITARY ? "   " : "  ");
                  bGotFirst = TRUE;
                }
                else
                {
                  strcat(tempString, " ----  ");
                }
              }
              else if(pTRIPSAVE->flags & TRIPSAVE_FLAG_LASTTRIP) 
              {
                if(bGotLast)
                {
                  strcat(tempString, " ----  ");
                  if(nM == numBaseNodes - 1  && timeFormat == PREFERENCES_MILITARY)
                    strcat(tempString, " ");
                   
                }
                else
                {
                  strcat(tempString, Tchar(GTResults.tripTimes[nM]));
                  strcat(tempString, "  ");
                  if(baseNodeRecordIDs[nM] == RUNS.end.NODESrecordID)
                    bGotLast = TRUE;
                  else
                  {
                    if(timeFormat == PREFERENCES_MILITARY)
                      strcat(tempString, " ");
                  }
                }
              }
              else
              {
                strcat(tempString, Tchar(GTResults.tripTimes[nM]));
                strcat(tempString, timeFormat == PREFERENCES_MILITARY ? "   " : "  ");
              }
              nM++;
            }
          }
//
//  Not the base - we only want those times that exist on the selected pattern
//
          else
          {
//
//  Get the trip pattern nodes
//
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = 0;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            numTripNodes = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                tripNodeRecordIDs[numTripNodes++] = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
            nM = 0;
            for(nN = 0; nN < numBaseNodes; nN++)
            {
              if(timeFormat == PREFERENCES_MILITARY)
                strcpy(szarString, "----   ");
              else
                strcpy(szarString, " ----  ");
              if(nM < numTripNodes)
              {
                if(pTRIPSAVE->flags & TRIPSAVE_FLAG_FIRSTTRIP) 
                {
                  if(baseNodeRecordIDs[nN] == tripNodeRecordIDs[nM])
                  {
                    if(baseNodeRecordIDs[nN] == RUNS.start.NODESrecordID ||
                          ((pTRIPSAVE->flags & TRIPSAVE_FLAG_PULLOUT)  && TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID))
                      bGotFirst = TRUE;
                    if(bGotFirst)
                    {
                      strcpy(szarString, Tchar(GTResults.tripTimes[nM]));
                      strcat(szarString, timeFormat == PREFERENCES_MILITARY ? "   " : "  ");
                    }
                    nM++;
                  }
                }
                else if(pTRIPSAVE->flags & TRIPSAVE_FLAG_LASTTRIP)
                {
                  if(baseNodeRecordIDs[nN] == tripNodeRecordIDs[nM])
                  {
                    if(!bGotLast)
                    {
                      strcpy(szarString, Tchar(GTResults.tripTimes[nM]));
                      strcat(szarString, timeFormat == PREFERENCES_MILITARY ? "   " : "  ");
                    }
                    if(baseNodeRecordIDs[nN] == RUNS.end.NODESrecordID)
                      bGotLast = TRUE;
                    nM++;
                  }
                }
                else
                {
                  if(baseNodeRecordIDs[nN] == tripNodeRecordIDs[nM])
                  {
                    strcpy(szarString, Tchar(GTResults.tripTimes[nM]));
                    strcat(szarString, timeFormat == PREFERENCES_MILITARY ? "   " : "  ");
                    nM++;
                  }
                }
              }
              strcat(tempString, szarString);
            }  // nN
          }
//
//  Do the balance of the output string
//
//  Next Route
//  Next Trip
//
          sprintf(szarString, " %8s%6s\"\r\n", nextRoute, szPITime);
          strcat(tempString, szarString);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        }  // nL (through the trips)
      }  // nK (through TRIPSAVEFIRST or TRIPSAVEREST)
//
//  See who's relieving us
//
      reliefRunNumber = NO_RECORD;
      for(nK = 0; nK < numInRunsave; nK++)
      {
        if(NodesEquivalent(RUNS.end.NODESrecordID, RUNSAVE[nK].start.NODESrecordID, &equivalentTravelTime) &&
              RUNS.end.TRIPSrecordID == RUNSAVE[nK].start.TRIPSrecordID &&
              RUNS.runNumber != RUNSAVE[nK].runNumber)
        {
          reliefRunNumber = RUNSAVE[nK].runNumber;
          break;
        }
      }
      if(reliefRunNumber != NO_RECORD)
      {
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        if(strcmp(szParkForRoute, "") == 0)
        {
          sprintf(tempString,
                "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"This piece of work is relieved by run %ld\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                szRuntype, szReportTime, szPayTime, szEndTime, reliefRunNumber);
        }
        else
        {
          sprintf(tempString,
                "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"This piece of work is relieved by run %ld, which starts on route %s\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                szRuntype, szReportTime, szPayTime, szEndTime, reliefRunNumber, szParkForRoute);
        }
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  End of piece travel instructions
//
      fromNode = COST.TRAVEL[RUNS.pieceNumber - 1].endBPointNODESrecordID;
      toNode = COST.TRAVEL[RUNS.pieceNumber - 1].endNODESrecordID;
      if(bUseCISPlan)
      {
//        if(fromNode != NO_RECORD &&
//            toNode != NO_RECORD &&
//              !NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
        if(fromNode != NO_RECORD && toNode != NO_RECORD && fromNode != toNode)
        {
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                szRuntype, szReportTime, szPayTime, szEndTime);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          strcpy(travelDirections, CISplanReliefConnect(&startTime, &endTime, &dwellTime,
                TRUE, fromNode, toNode, COST.TRAVEL[RUNS.pieceNumber - 1].endBPointTime,
                TRUE, SERVICES.recordID));
          if(startTime == -25 * 60 * 60)
          {
            travelTime = NO_TIME;
          }
          else
          {
            travelTime = (endTime - startTime) + dwellTime;
          }
//          if(travelTime != 0)
          if(RUNSVIEW[nJ].endOfPieceTravel != 0)
          {
//            sprintf(szarString, "Piece %d: End Travel Instructions:\n", RUNS.pieceNumber);
            strcpy(szarString, "End Travel Instructions:\n");
            sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                  szRuntype, szReportTime, szPayTime, szEndTime, szarString);
            WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
            ptr = strtok(travelDirections, lineSep);
            while(ptr != 0)
            {
              sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                    seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                    szRuntype, szReportTime, szPayTime, szEndTime, ptr);
              WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              ptr = strtok(NULL, lineSep);
            }
          }
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
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
                szRuntype, szReportTime, szPayTime, szEndTime);
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
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
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
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "*** End of Piece Extraboard duty: Start at: %s\n",
              Tchar(RUNS.after.startTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sprintf(szarString, "*** End of Piece Extraboard duty:   End at: %s\n",
              Tchar(RUNS.after.endTime));
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
//
//  Off at
//
//
//  Off time and location
//
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
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
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
    }  // nJ (through number of pieces)
//
//  Spit out the comments
//
    if(numComments > 0)
    {
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"Comments:\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      for(nJ = 0; nJ < numComments; nJ++)
      {
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        COMMENTSKey0.recordID = commentRecordIDs[nJ];
        btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
        strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
        trim(szarString, COMMENTS_CODE_LENGTH);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s:\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        szarString[0] = 0;
        for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
             tempString2 = strtok(NULL, "\r\n") )
        {
          strcpy(szarString, tempString2 );
          sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"    %s\"\r\n",
                seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
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
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"Signcodes:\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      for(nJ = 0; nJ < numSigncodes; nJ++)
      {
        SIGNCODESKey0.recordID = signcodeRecordIDs[nJ];
        btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
        strncpy(szarString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
        trim(szarString, SIGNCODES_CODE_LENGTH);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s:\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        strncpy(szarString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
        trim(szarString, SIGNCODES_TEXT_LENGTH);
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"    %s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
              szRuntype, szReportTime, szPayTime, szEndTime, szarString);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      }
    }
//
//  Do the run comment
//
    if(bGotRunComment)
    {
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"Run comment:\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      COMMENTSKey0.recordID = runCommentRecordID;
      btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
      strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
      trim(szarString, COMMENTS_CODE_LENGTH);
      sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t%s:\r\n",
            seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
            szRuntype, szReportTime, szPayTime, szEndTime, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
      szarString[0] = 0;
      for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
           tempString2 = strtok(NULL, "\r\n") )
      {
        strcpy(szarString, tempString2 );
        sprintf(tempString, "%ld\t\"%s\"\t\"%s\"\t%ld\t%ld\t%d\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s\"\r\n",
              seq++, divisionName, serviceName, RUNS.runNumber, RUNS.pieceNumber, numPieces,
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
