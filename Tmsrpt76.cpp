// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  Ocean County Operator's Daily Report
//
//  From QETXT.INI
//
//  [TMSRPT76]
//  FILE=TMSRPT76.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=RouteNumberAndName,VARCHAR,72,0,72,0,
//  FIELD3=Date,VARCHAR,32,0,32,0,
//  FIELD4=BusNumber,VARCHAR,8,0,8,0,
//  FIELD5=Driver,VARCHAR,32,0,32,0,
//  FIELD6=RunNumber,NUMERIC,8,0,8,0,
//  FIELD7=TripNumber,NUMERIC,8,0,8,0,
//  FIELD8=Node,VARCHAR,64,0,64,0,
//  FIELD9=Time,VARCHAR,16,0,16,0,
//

#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
}  // extern "C"

int  TMSRPT76Ex(TMSRPTPassedDataDef *);

extern "C" {
BOOL PickDate(long *);

BOOL FAR TMSRPT76(TMSRPTPassedDataDef *pPassedData)
{
  return TMSRPT76Ex(pPassedData);
}
} // extern "C"

#include "TMS.h"
#include <math.h>

#define TMSRPT76_MAXTRIPS 50
#define TMSRPT76_MAXNODES 50
#define TMSRPT76_FLAGS_PO 0x0001
#define TMSRPT76_FLAGS_PI 0x0002

typedef struct NodesAndTimesStruct
{
  long NODESrecordID;
  long timeAtNode;
  long flags;
} NodesAndTimesDef;

typedef struct RunDataStruct
{
  long tripNumber;
  int  numNodes;
  NodesAndTimesDef data[TMSRPT76_MAXNODES];
} RunDataDef;

BOOL FAR TMSRPT76Ex(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef      GTResults;
  REPORTPARMSDef REPORTPARMS;
  RunDataDef RunData[TMSRPT76_MAXTRIPS];
  BLOCKSDef     *pTRIPSChunk;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  CString fromFile;
  CString s;
  CString date;
  float   distance;
  HANDLE  hOutputFile;
  DWORD   dwBytesWritten;
  BOOL  bFound;
  BOOL  bFinishedOK;
  BOOL  bRC;
  long  year, month, day;
  long  fromDate;
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  m_DateToUse;
  long  m_SERVICESrecordIDInEffect;
  long  m_ROSTERDivisionInEffect;
  long  m_RUNSDivisionInEffect;
  char  outputString[1024];
  char *pszReportName;
  char  szRouteNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  szOperator[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  char  szBusNumber[BUSES_NUMBER_LENGTH + 1];
  int   nI, nJ, nK, nL;
  int   rcode2;
  int   seq;
  int   numPieces;
  int   m_Today;
  BOOL bFirstTrip;
  long SGRPSERVICESrecordID;
  long serviceRecordID;
  long RGRPROUTESrecordID;
  long onTime;
  long offTime;
  int  keyNumber = 2;
  int  numBlocks = 0;
  int  numSERVICESrecordIDsFromTRIPS = 0;
  int  numTrips;
  int  numNodes;
  int  counter;
  int  indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};


  bFinishedOK = FALSE;
  pPassedData->nReportNumber = 75;
  pPassedData->numDataFiles = 1;
//
//  Get the date
//
  fromDate = NO_RECORD;
  if(!PickDate(&fromDate) || fromDate == NO_RECORD)
  {
    return(FALSE);
  }
  GetYMD(fromDate, &year, &month, &day);
  
  CTime workDate(year, month, day, 0, 0, 0);
  date = workDate.Format("%A, %B %d, %Y");
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT76.TXT");
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
//
//  See if he just wants a blank manifest
//
  s.LoadString(TEXT_399);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(NULL, s, TMS, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
  {
    sprintf(outputString, "%1\tSpecial\t%s\r\n", date);
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    goto deallocate;
  }
//
//  Figure out what day of the week we are
//
  m_Today = indexToRoster[workDate.GetDayOfWeek() - 1];
  m_DateToUse = (year * 10000) + (month * 100) + day;
  m_SERVICESrecordIDInEffect = ROSTERPARMS.serviceDays[m_Today];
//
//  Make sure a roster template's been established
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = year * 10000 + month * 100 + day;
  DAILYOPSKey1.pertainsToTime = 86400;  // Midnight
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  if(rcode2 == 0 && (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    m_ROSTERDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.ROSTERDIVISIONSrecordID;
    m_RUNSDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID;
  }
  else
  {
    m_ROSTERDivisionInEffect = m_DivisionRecordID;
    m_RUNSDivisionInEffect = m_DivisionRecordID;
  }
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_RUNSDivisionInEffect;
  REPORTPARMS.SERVICESrecordID = m_SERVICESrecordIDInEffect;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pRunList = NULL;
  REPORTPARMS.flags = RPFLAG_SERVICES | RPFLAG_NOALLSERVICES | RPFLAG_DIVISIONS |
        RPFLAG_NOALLDIVISIONS | RPFLAG_RUNS;
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

  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      break;
    }
  }
  StatusBarStart(hWndMain, pszReportName);
//
//  Set up the trip planner
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuild(FALSE, TRUE))
      {
        chdir("..");  // Back to where we started from
        return(FALSE);
      }
    }
    else
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        serviceRecordID = SERVICES.recordID;
        InitTravelMatrix(serviceRecordID, TRUE);
        SERVICESKey0.recordID = serviceRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
    }
  }
//
//  Cycle through the runs
//
  seq = 1;
  pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
  for(nI = 0; nI < REPORTPARMS.numRuns; nI++)
  {
//
//  Get the rostered operator for this run
//
    strcpy(szOperator, "");
    rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0)
    {
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] == RUNS.recordID)
      {
        DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
          trim(tempString, DRIVERS_LASTNAME_LENGTH);
          strcpy(szOperator, tempString);
          strcat(szOperator, ", ");
          strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
          trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
          strcat(szOperator, tempString);
        }
        break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
    StatusBar((long)nI, REPORTPARMS.numRuns);
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    numPieces = GetRunElements(NULL, &RUNS, &PROPOSEDRUN, &COST, FALSE);
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      if(nJ == 0)
      {
        RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
//
//  Clear the structure
//
      for(nK = 0; nK < TMSRPT76_MAXTRIPS; nK++)
      {
        RunData[nK].tripNumber = NO_RECORD;
        for(nL = 0; nL < TMSRPT76_MAXNODES; nL++)
        {
          RunData[nK].data[nL].NODESrecordID = NO_RECORD;
          RunData[nK].data[nL].timeAtNode = NO_TIME;
          RunData[nK].data[nL].flags = 0;
        }
      }
//
//  Get the first trip on this piece of the run
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
      RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
      SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
      blockNumber = pTRIPSChunk->blockNumber;
//
//  Reposition the key into the trips
//
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
//
//  Cycle through the trips on the run to save
//  the trip number, and the nodes and times.
//
      bFirstTrip = TRUE;
      numTrips = 0;
      while(rcode2 == 0 &&
            pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
            pTRIPSChunk->RGRPROUTESrecordID == RGRPROUTESrecordID &&
            pTRIPSChunk->SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
            pTRIPSChunk->blockNumber == blockNumber)
      {
        numNodes = 0;
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Save the route number and name
//
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strcpy(szRouteNumberAndName, tempString);
        strcat(szRouteNumberAndName, " - ");
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(szRouteNumberAndName, tempString);
//
//  Position into the pattern
//
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        counter = 0;
//
//  Trip number
//
        RunData[numTrips].tripNumber = TRIPS.tripNumber;
//
//  For the first trip only...
//
        if(bFirstTrip)
        {
          bFirstTrip = FALSE;
//
//  Is the node a pullout?
//
          if(pTRIPSChunk->POGNODESrecordID == RUNS.start.NODESrecordID)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            onTime = GTResults.firstNodeTime - GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            RunData[numTrips].data[numNodes].NODESrecordID = pTRIPSChunk->POGNODESrecordID;
            RunData[numTrips].data[numNodes].timeAtNode = onTime;
            RunData[numTrips].data[numNodes].flags = TMSRPT76_FLAGS_PO;
          }
//
//  Nope - Find the node on the pattern
//
          else
          {
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              if((bFound = PATTERNS.NODESrecordID == RUNS.start.NODESrecordID) == TRUE)
              {
                break;
              }
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                counter++;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
            onTime = (bFound ? GTResults.tripTimes[counter] : NO_TIME);
            RunData[numTrips].data[numNodes].NODESrecordID = RUNS.start.NODESrecordID;
            RunData[numTrips].data[numNodes].timeAtNode = onTime;
          }
          numNodes++;
        }
//
//  Get the intermediate nodes and times
//
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex &&
              PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            RunData[numTrips].data[numNodes].NODESrecordID = PATTERNS.NODESrecordID;
            RunData[numTrips].data[numNodes].timeAtNode = GTResults.tripTimes[counter];
            numNodes++;
            counter++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Check on a pull-in
//
        if(TRIPS.recordID == RUNS.end.TRIPSrecordID &&
              pTRIPSChunk->PIGNODESrecordID == RUNS.end.NODESrecordID)
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
          offTime = GTResults.lastNodeTime + GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          RunData[numTrips].data[numNodes].NODESrecordID = pTRIPSChunk->PIGNODESrecordID;
          RunData[numTrips].data[numNodes].timeAtNode = offTime;
          RunData[numTrips].data[numNodes].flags = TMSRPT76_FLAGS_PI;
          numNodes++;
        }
//
//  Get the next trip
//
        RunData[numTrips].numNodes = numNodes;
        numTrips++;
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      }
    }  // nJ < numPieces
//
//  Get the bus number
//
    strcpy(szBusNumber, "");
//
//  Dump out the data
//
    for(nJ = 0; nJ < numTrips; nJ++)
    {
      for(nK = 0; nK < RunData[nJ].numNodes; nK++)
      {
//
//  Seq, RouteNumberAndName, Date, BusNumber, Driver, RunNumber
//
        sprintf(outputString, "%d\t%s\t%s\t%s\t%s\t%ld\t", seq, szRouteNumberAndName, date, szBusNumber, szOperator, RUNS.runNumber);
//
//  Trip number
//
        sprintf(tempString, "%ld\t", RunData[nJ].tripNumber);
        strcat(outputString, tempString);
//
//  Node
//
        if(RunData[nJ].data[nK].flags & TMSRPT76_FLAGS_PO)
        {
          strcat(outputString, "Pull out from ");
        }
        else if(RunData[nJ].data[nK].flags & TMSRPT76_FLAGS_PI)
        {
          strcat(outputString, "Pull in to ");
        }
        NODESKey0.recordID = RunData[nJ].data[nK].NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
        trim(tempString, NODES_DESCRIPTION_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
// 
//  Time
// 
        strcat(outputString, Tchar(RunData[nJ].data[nK].timeAtNode));
        strcat(outputString, "\r\n");
//
//  Write it out
//
        WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
        seq++;
      }  // nK < numNodes
    }  // nJ < numTrips
  }  // nI < numRuns
//
//  All done
//
  StatusBar(-1L, -1L);
  
  deallocate:
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pRunList);
    StatusBarEnd();
  
    CloseHandle(hOutputFile);

  return(TRUE);
}
