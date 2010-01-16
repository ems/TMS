// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the Orbital AVL System
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define TREQPATDATA_FLAG_FIRSTNODEONPATTERN 0x0001
#define TREQPATDATA_FLAG_LASTNODEONPATTERN  0x0002
#define TREQPATDATA_MAXNODES                50

typedef struct TREQPATDATAStruct
{
  long ROUTESrecordID;
  int  numNodes;
  long NODESrecordIDs[TREQPATDATA_MAXNODES];
  long flags[TREQPATDATA_MAXNODES];
} TREQPATDATADef;
  
typedef struct ODStruct
{
  long ROUTESrecordID;
  int  directionNumber;
  long NODESrecordID;
} ODDef;

typedef struct TREQStruct
{
  ODDef origin;
  ODDef destination;
} TREQDef;

long NodeEquivalenced(long NODESrecordID)
{
  int nI;

  for(nI = 0; nI < numNEQ; nI++)
  {
    if(NODESrecordID == NEQ[nI].secondaryNODESrecordID)
    {
      return(NEQ[nI].primaryNODESrecordID);
    }
  }

  return(NO_RECORD);
}

void PipeCommas(char *string)
{
  char *ptr;

  while(ptr = strchr(string, ','))
  {
    *ptr = '|';
  }
}

#define OUTPUT_FOLDER_NAME "OrbCAD Interface"

#define SZBOOKINGNAME_LENGTH            7
#define SZBOOKINGID_LENGTH             10

#define BOOKINGFILE_DESCRIPTION_LENGTH  9

#define PLACEFILE_STREETABBR_LENGTH     8
#define PLACEFILE_DISTRICTNAME_LENGTH   8
#define PLACEFILE_DESCRIPTION_LENGTH   40

#define RTEFILE_DIRECTIONNAME_LENGTH    8
#define RTEFILE_ROUTENAME_LENGTH       40
#define RTEFILE_ROUTENUMBER_LENGTH      8
#define RTEFILE_STOPNAME_LENGTH         8

#define TRIPFILE_DIRECTIONNAME_LENGTH   8
#define TRIPFILE_PATTERNNAME_LENGTH     6
#define TRIPFILE_NODENAME_LENGTH        8

#define COMFILE_SYMBOL_LENGTH           8
#define COMFILE_ABBR_LENGTH             8

#define BLKFILE_BUSTYPES_LENGTH         8

#define RUNFILE_DIVISIONNAME_LENGTH     8
#define RUNFILE_NODENAME_LENGTH         8

#define BUSFILE_DESCRIPTION_LENGTH     40

#define TREQFILE_DIRECTIONNAME_LENGTH   8

#define VTPFILE_DIRECTIONNAME_LENGTH    8
#define VTPFILE_PATTERNNAME_LENGTH      6
#define VTPFILE_TPBS_NODESTOPTOSTOP     1
#define VTPFILE_TPBS_NODETOSTOP         2
#define VTPFILE_TPBS_NODETONODE         3
#define VTPFILE_TPBS_NODETONODESTOP     4
#define VTPFILE_TPBS_STOPTOSTOP         5
#define VTPFILE_TPBS_STOPTONODE         6
#define VTPFILE_TPBS_STOPTONODESTOP     7
#define VTPFILE_TPBS_NODESTOPTONODESTOP 8
#define VTPFILE_TPBS_NODESTOPTONODE     9

#define SERFILE_DESCRIPTION_LENGTH     10

#define SIGNFILE_SYMBOL_LENGTH          8
#define SIGNFILE_ABBR_LENGTH            8

BOOL FAR TMSRPT61(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef      GTResults;
  MAPPOINTDef         *pFrom, *pTo;
  MIFLINESDef          MIFLINES[TMSRPT61_MAXMIFLINES];
  MIFPASSEDDATADef     MIFPASSEDDATA;
  TREQPATDATADef TREQPATDATA[TMSRPT61_MAXROUTES][2];
  TREQDef TREQ[TMSRPT61_MAXTREQ];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  double  Long, Lat;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  float   distance;
  struct  tm tmED, tmUD;
  time_t  EDmktime;
  time_t  UDmktime;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bFirstNode;
  BOOL  bFirst;
  BOOL  bReposition;
  BOOL  bWriteRecord;
  BOOL  bFirstTimepoint;
  BOOL  bFromIsAStop;
  BOOL  bToIsAStop;
  long  startTime;
  long  endTime;
  long  startPlace;
  long  endPlace;
  long  lastTRIPSrecordID;
  long  lastNODESrecordID;
  long  lastNODEtime;
  long  firstNODESrecordID;
  long  firstNODEtime;
  long  SERVICESinEffect[TMSRPT61_MAXSERVICES];
  long  previousNode;
  long  RUNSrecordID;
  long  numPieces;
  long  year, month, day;
  long  serviceRecordID;
  long  blockNumber;
  long  deadheadTime;
  long  POTime;
  long  PITime;
  long  TRIPSrecordID;
  long  lastTripNumber;
  long  BASEPatternNODESrecordIDs[TMSRPT61_MAXPATTERNNODES];
  long  startNODESrecordID;
  long  BUSTYPESrecordID;
  long  PIGNODESrecordID;
  long  tempLong;
  long  pieceNumber;
  long  ROUTESrecordID;
  long  PATTERNNAMESrecordID;
  long  reportTime;
  long  routeList[TMSRPT61_MAXROUTES];
  long  PATTERNSrecordID;
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  primaryNODESrecordID;
  long  NODESrecordID;
  long  stopNumber;
  long  stopNODESrecordID;
  char  outputString[1024];
  char  outputStringSave[1024];
  char  dummy[256];
  char *ptr;
  char *pszReportName;
  char *tempString2;
  char  szBookingName[SZBOOKINGNAME_LENGTH + 1];
  char  szBookingID[SZBOOKINGID_LENGTH + 1];
  char  szDivisionName[DIVISIONS_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  szOpDays[TMSRPT61_MAXSERVICES][8];
  char  outputFileName[64];
  char  szPatternName[PATTERNNAMES_NAME_LENGTH + 1];
  char  szFromNode[NODES_ABBRNAME_LENGTH + 1];
  char  szToNode[NODES_ABBRNAME_LENGTH + 1];
  char  szRouteID[ROUTES_NUMBER_LENGTH + 1];
  char  szFromStop[NODES_LONGNAME_LENGTH + 1];
  char  szToStop[NODES_LONGNAME_LENGTH + 1];
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   nN;
  int   rcode2;
  int   maxNodes;
  int   maxTrips;
  int   maxDivisions;
  int   numBASEPatternNodes;
  int   maxRoutes;
  int   maxServices;
  int   maxPatternNodes;
  int   dayOfWeek;
  int   numNodes;
  int   numServices;
  int   seq;
  int   TripTraversal[5] = {2, 3, 1, 4, 0};
  int   tripType;
  int   TPBS;
  int   fareCode;
  int   previousFareCode;
  int   numTREQ;
  int   numRoutes;
  int   numMIFLINES;
  int   totalOutputLines;
  int   outputSeq;

  bFinishedOK = FALSE;
//
//  Maintain the distance measure
//
  distanceMeasureSave = distanceMeasure;
  distanceMeasure = PREFERENCES_MILES;
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
        goto done;
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
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  LoadString(hInst, nI, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    goto done;
  }
  bGotError = FALSE;
  pPassedData->nReportNumber = 60;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Set some initial limits
//
//  maxServices
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  maxServices = rcode2 == 0 ? BSTAT.numRecords : 0;
  if(maxServices > TMSRPT61_MAXSERVICES)
  {
    TMSError(hWndMain, MB_ICONSTOP, ERROR_331, (HANDLE)NULL);
    goto done;
  }
//
//  maxRoutes
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  maxRoutes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  maxNodes
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  maxNodes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  maxPatternNodes
//
  rcode2 = btrieve(B_STAT, TMS_PATTERNS, &BSTAT, dummy, 0);
  maxPatternNodes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  maxTrips
//
  rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
  maxTrips = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  maxDivisions
//
  rcode2 = btrieve(B_STAT, TMS_DIVISIONS, &BSTAT, dummy, 0);
  maxDivisions = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Verify that all trips in the system are numbered
//
  StatusBarText("Checking if all trips are numbered");
  rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  nI = 0;
  bFound = FALSE;
  lastTripNumber = NO_RECORD;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    StatusBar((long)nI, (long)maxTrips);
    if(TRIPS.tripNumber <= 0)
    {
      MessageBeep(MB_ICONQUESTION);
      LoadString(hInst, ERROR_344, tempString, TEMPSTRING_LENGTH);
      if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OK) != IDOK)
      {
        goto done;
      }
      bFound = TRUE;
      break;
    }
    if(TRIPS.tripNumber > lastTripNumber)
    {
      lastTripNumber = TRIPS.tripNumber;
    }
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  }
  if(bFound)
  {
    StatusBarText("Renumbering Trips");
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    nI = 1;
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      StatusBar((long)nI, (long)maxTrips);
      TRIPS.tripNumber = (long)nI;
      nI++;
      rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
    lastTripNumber = nI;
  }
//
//  Set up the equivalend nodes (NEQ) structure
//
  numNEQ = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(numNEQ >= TMSRPT61_MAXNEQ)
    {
      break;
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      NEQ[numNEQ].primaryNODESrecordID = CONNECTIONS.fromNODESrecordID;
      NEQ[numNEQ].secondaryNODESrecordID = CONNECTIONS.toNODESrecordID;
      numNEQ++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }
//
//  Check on overflow
//
  if(numNEQ >= TMSRPT61_MAXNEQ)
  {
    sprintf(tempString, "*ERR - Number of Equivalences exceeds %d\r\n", TMSRPT61_MAXNEQ);
    _lwrite(hfErrorLog, tempString, strlen(tempString));
    bGotError = TRUE;
  }
//
//  Figure out the "Booking name" - it's used as part of the output file names
//  We'll just use the first six characters of the short database description
//
  strncpy(szarString, szShortDatabaseDescription, SZBOOKINGNAME_LENGTH);
  trim(szarString, SZBOOKINGNAME_LENGTH);
  strcpy(szBookingName, szarString);
//
//  Figure out the "Booking ID" - it's used in various files
//
  GetYMD(effectiveDate, &year, &month, &day);
  sprintf(tempString, "%4ld%02ld%02ld", year, month, day);
  strcpy(szBookingID, tempString);
//
//  ================================
//  Booking File - book<bbbbbbb>.dat
//  ================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "book");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  This file has one record.
//
//  Record type - 5
//
  strcpy(outputString, "5,");
//
//  Description (same as szBookingName)
//
  strcat(outputString, szBookingName);
  strcat(outputString, ",");
//
//  Booking ID (same as effectiveDate)
//
  GetYMD(effectiveDate, &year, &month, &day);
  sprintf(tempString, "%4ld%02ld%02ld,", year, month, day);
  strcat(outputString, tempString);
//
//  Start date (Effective date)
//
  strcat(outputString, tempString);
//
//  End date (Until date
//
  GetYMD(untilDate, &year, &month, &day);
  sprintf(tempString, "%4ld%02ld%02ld\r\n", year, month, day);
  strcat(outputString, tempString);
//
//  Write it out and close the file
//
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  _lclose(hfOutputFile);
//
//  ================================
//  Calendar File - cal<bbbbbbb>.dat
//  ================================
//
  numServices = 0;
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "cal");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Dump out the service day based on calendar days
//
//  Effective date
//
  GetYMD(effectiveDate, &year, &month, &day);
  memset(&tmED, 0x00, sizeof(tmED));
  tmED.tm_mday = day;
  tmED.tm_mon = month - 1;
  tmED.tm_year = year - 1900;
//
//  Until date
//
  GetYMD(untilDate, &year, &month, &day);
  memset(&tmUD, 0x00, sizeof(tmUD));
  tmUD.tm_mday = day;
  tmUD.tm_mon = month - 1;
  tmUD.tm_year = year - 1900;
//
//  Start at the effective date and go to the until date
//
  EDmktime = mktime(&tmED);
  if(EDmktime == (time_t)(-1))
  {
    LoadString(hInst, ERROR_328, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(outputString, szFormatString, 1);
    _lwrite(hfErrorLog, outputString, strlen(outputString));
    bGotError = TRUE;
  }
  else
  {
    UDmktime = mktime(&tmUD);
    while(EDmktime <= UDmktime)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      dayOfWeek = (tmED.tm_wday == 0 ? 6 : tmED.tm_wday - 1);
      for(bFound = FALSE, nI = 0; nI < numServices; nI++)
      {
        if(ROSTERPARMS.serviceDays[dayOfWeek] == SERVICESinEffect[nI])
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        SERVICESinEffect[numServices] = ROSTERPARMS.serviceDays[dayOfWeek];
        numServices++;
      }
//
//  Record type
//
      strcpy(outputString, "25,");
//
//  Booking ID
//
      strcat(outputString, szBookingID);
      strcat(outputString, ",");
//
//  Date
//
      sprintf(tempString, "%4ld%02ld%02ld,", tmED.tm_year + 1900, tmED.tm_mon + 1, tmED.tm_mday);
      strcat(outputString, tempString);
//
//  Service type (SERVICES.number)
//
      SERVICESKey0.recordID = ROSTERPARMS.serviceDays[dayOfWeek];
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      sprintf(tempString, "%ld,", SERVICES.number);
      strcat(outputString, tempString);
//
//  Special Trip Option ID (always 0)
//
      strcat(outputString, "0\r\n");
//
//  Write out the record
      _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Figure out the next day
//
      tmED.tm_mday++;
      EDmktime = mktime(&tmED);
      if(EDmktime == (time_t)(-1))
      {
        LoadString(hInst, ERROR_328, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(outputString, szFormatString, 1);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
        bGotError = TRUE;
      }
    }
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===============================
//  Place File - place<bbbbbbb>.dat
//  ===============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "place");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Cycle through the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
//
//  Don't bother with equivalenced nodes
//
    if(NodeEquivalenced(NODES.recordID) == NO_RECORD)
    {
//
//  Omit bus stops from the place file: Bob Antonisse, 02-Aug-07
//
      if(!(NODES.flags & NODES_FLAG_STOP))
      {
//
//  Record type
//
        strcpy(outputString, "4,");
//
//  Place ID (NODES.recordID)
//
        sprintf(tempString, "%ld,", NODES.recordID);
        strcat(outputString, tempString);
//
//  Place name (NODES.abbrName)
//
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, ",");
//
//  Place description
//
//  Now "Intersection" (as per:Cheryl Arnett [mailto:CArnett@intercitytransit.com] Tuesday, July 28, 2009 4:48 PM)
//
        strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
        trim(tempString, NODES_INTERSECTION_LENGTH);
        PipeCommas(tempString);
        while(ptr = strchr(tempString, '"'))
        {
          strcpy(ptr, ptr + 1);
        }
        tempString[NODES_INTERSECTION_LENGTH] = '\0';
        strcat(outputString, tempString);
        strcat(outputString, ",");
//
//  "At" street abbreviation, "On" street abbreviation
//
//  Parse the intersection field to get the on and at streets
//
        strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
        trim(tempString, NODES_INTERSECTION_LENGTH);
        PipeCommas(tempString);
        ptr = strstr(tempString, "&&");
        if(ptr)
        {
          *ptr = '\0';
          tempString[PLACEFILE_STREETABBR_LENGTH] = '\0';
          strcpy(szarString, (ptr + 2));
          if(szarString[0] == ' ')
          {
            strcpy(szarString, &szarString[1]);
          }
          szarString[PLACEFILE_STREETABBR_LENGTH] = '\0';
        }
        else
        {
          strcpy(szarString, "");
        }
//
//  On
//
        trim(tempString, PLACEFILE_STREETABBR_LENGTH);
        while(ptr = strchr(tempString, '"'))
        {
          strcpy(ptr, ptr + 1);
        }
        strcat(outputString, tempString);
        strcat(outputString, ",");
//
//  At
//
        trim(szarString, PLACEFILE_STREETABBR_LENGTH);
        while(ptr = strchr(szarString, '"'))
        {
          strcpy(ptr, ptr + 1);
        }
        strcat(outputString, szarString);
        strcat(outputString, ",");
//
//  District name (Jurisdiction);
//
        if(NODES.JURISDICTIONSrecordID == NO_RECORD)
        {
          strcpy(tempString, "");
        }
        else
        {
          JURISDICTIONSKey0.recordID = NODES.JURISDICTIONSrecordID;
          btrieve(B_GETEQUAL, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
          strncpy(tempString, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
          trim(tempString, JURISDICTIONS_NAME_LENGTH);
          tempString[PLACEFILE_DISTRICTNAME_LENGTH] = '\0';
        }
        strcat(outputString, tempString);
        strcat(outputString, ",");
//
//  Place type (0 = Timepoint/stop, 1 = Garage)
//
        sprintf(tempString, "%d\r\n", (NODES.flags & NODES_FLAG_GARAGE) ? 1 : 0);
        strcat(outputString, tempString);
//
//  Write it out and get the next node
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =============================
//  Route File - rte<bbbbbbb>.dat
//  =============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "rte");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Traverse the Routes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  And the directions on the route
//
    for(nI = 0; nI < 2; nI++)
    {
      if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      {
        continue;
      }
//
//  There must be trips on this route to record route values
//
      TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
      TRIPSKey1.SERVICESrecordID = 0;
      TRIPSKey1.directionIndex = 0;
      TRIPSKey1.tripSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
      if(rcode2 != 0 ||
            TRIPS.ROUTESrecordID != ROUTES.recordID)
      {
        continue;
      }
//
//  Record type
//
      strcpy(outputString, "40,");
//
//  Booking ID
//
      strcat(outputString, szBookingID);
      strcat(outputString, ",");
//
//  Internal route ID (ROUTES.recordID)
//
      sprintf(tempString, "%ld,", ROUTES.recordID);
      strcat(outputString, tempString);
//
//  Original: Route ID (make it the same as Internal Route ID)
//
//      strcat(outputString, tempString);
//  New: Route ID - Make it as close to "Route Short Name" as possible
//       This takes alphas into account and outputs them 62A to 621, 62B to 622, and so on.
//
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
      ptr = tempString;
      while(*ptr)
      {
        if(isalpha(*ptr))
        {
          *ptr -= 16;
        }
        ptr++;
      }
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Direction num
//
      sprintf(tempString, "%d,", nI);
      strcat(outputString, tempString);
//
//  Direction name
//
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
      trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
      tempString[RTEFILE_DIRECTIONNAME_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, ",");
      strcpy(szarString, outputString);  // Used later
      szarString[1] = '1';               // Change record type to "41" - used later
//
//  Route long name
//
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      tempString[RTEFILE_ROUTENAME_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Route short name
//
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
//
//  Write it out and start the cycle through the BASE pattern
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
      PATTERNSKey2.SERVICESrecordID = 1;
      PATTERNSKey2.directionIndex = nI;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      seq = 1;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == ROUTES.recordID &&
            PATTERNS.SERVICESrecordID == 1 &&
            PATTERNS.directionIndex == nI &&
            PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
      {
//
//  In outputting the BASE pattern, the record type is 41, and
//  the balance of the record - up to including direction name
//  is the same as a type 40 record. We copied outputString to
//  szarString (above), and fixed the record type, so we just
//  need to extablish the first portion of outputString below
//  and then continue from there.
//
        strcpy(outputString, szarString);
//
//  Sequence number
//
        sprintf(tempString, "%d,", seq++);
        strcat(outputString, tempString);
//
//  Stop number (0 when a stop, NODES.recordID when a timepoint) and
//  Stop ID (NODES.recordID when a stop, associatedNODESrecordID when a timepoint)
//  Stop name (NODES.abbrName when a stop, abbrName of the associatedNODESrecordID when a timepoint)
//  Timepoint ID (NODES.recordID when a timepoint, 0 when a stop)
//  Timepoint Name (NODES.abbrName when a timepoint, "" when a stop)
//
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
        {
//
//  Stop number (0 when a stop, NODES.recordID when a timepoint) and
//  Stop ID (NODES.recordID when a stop, associatedNODESrecordID when a timepoint)
//
//          sprintf(tempString, "0,%ld,", NODES.recordID);
//          strcat(outputString, tempString);
//
//  Change to stop number (as derived from long name) : Bob Antonisse, 2-Aug-07
//
          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          trim(tempString, NODES_LONGNAME_LENGTH);
          tempLong = atol(&tempString[4]);
          sprintf(tempString, "0,%ld,", tempLong);
          strcat(outputString, tempString);
//
//  Change to "Stopxxxx" : Bob Antonisse, 4-Feb-08
//
//          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
//          trim(tempString, NODES_LONGNAME_LENGTH);
//          sprintf(szarString, "0,%s,", tempString);
//          strcat(outputString, szarString);
//
//  Stop name (NODES.abbrName when a stop, abbrName of the associatedNODESrecordID when a timepoint)
//
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[RTEFILE_STOPNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Timepoint ID (NODES.recordID when a timepoint, 0 when a stop)
//  Timepoint Name (NODES.abbrName when a timepoint, "" when a stop)
//
          strcat(outputString, ",0,");
        }
        else
        {
//
//  If there's a node equivalence along the pattern, change the value to the first node encountered
//
          primaryNODESrecordID = NodeEquivalenced(NODES.recordID);
          if(primaryNODESrecordID != NO_RECORD)
          {
            NODESKey0.recordID = primaryNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          }
          NODESrecordID = NODES.recordID;
//
//  Stop number (0 when a stop, NODES.recordID when a timepoint) and
//
          sprintf(tempString, "%ld,", NODES.recordID);
          strcat(outputString, tempString);
//
//  Stop ID (NODES.recordID when a stop, associatedNODESrecordID when a timepoint)
//  Stop name (NODES.abbrName when a stop, abbrName of the associatedNODESrecordID when a timepoint)
//
          stopNumber = (nI == 0 ? NODES.OBStopNumber : NODES.IBStopNumber);
          stopNODESrecordID = NO_RECORD;
          bFound = FALSE;
          if(stopNumber <= 0)
          {
            stopNumber = NODES.number;
          }
          if(stopNumber > 0)
          {
            rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
            while(rcode2 == 0)
            {
              if((NODES.flags & NODES_FLAG_STOP) && NODES.number == stopNumber)
              {
                stopNODESrecordID = NODES.recordID;
                bFound = TRUE;
                break;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
            }
          }
          if(!bFound)
          {
            strcat(outputString, "0,,");
          }
          else
          {
//            sprintf(tempString, "%ld,", stopNODESrecordID);
//            strcat(outputString, tempString);
//            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
//            trim(tempString, NODES_ABBRNAME_LENGTH);
//            strcat(outputString, tempString);
//            strcat(outputString, ",");
//
//  Change to stop number (as derived from long name) : Bob Antonisse, 2-Aug-07
//
            strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(tempString, NODES_LONGNAME_LENGTH);
            tempLong = atol(&tempString[4]);
            sprintf(tempString, "%ld,", tempLong);
            strcat(outputString, tempString);
            strcat(outputString, ",");
          }
//
//  Timepoint ID (NODES.recordID when a timepoint, 0 when a stop)
//  Timepoint Name (NODES.abbrName when a timepoint, "" when a stop)
//
          NODESKey0.recordID = NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          sprintf(tempString, "%ld,", NODES.recordID);
          strcat(outputString, tempString);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[RTEFILE_STOPNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
        }
        strcat(outputString, "\r\n");
//
//  Write it out and get the next pattern record
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =================================
//  Trip File - trip<ss><bbbbbbb>.dat
//  =================================
//
//
//  Clear out and set up szOpDays
//
  for(nI = 0; nI < TMSRPT61_MAXSERVICES; nI++)
  {
    strcpy(szOpDays[nI], "");
    for(nJ = 0; nJ < 7; nJ++)
    {
      day = (nJ == 6 ? 0 : nJ + 1);
      szOpDays[nI][day] = (ROSTERPARMS.serviceDays[nJ] == SERVICESinEffect[nI] ? '1' : '0');
    }
    szOpDays[nI][7] = '\0';
  }
//
//  Cycle through the services
//
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    sprintf(outputFileName, "trip%02ld%s.dat", SERVICES.number, szBookingName);
    hfOutputFile = _lcreat(outputFileName, 0);
    if(hfOutputFile == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(szarString, szFormatString, outputFileName);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
      goto done;
    }
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    strcpy(tempString, outputFileName);
    strcat(tempString, " - ");
    strcat(tempString, serviceName);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    bFirst = TRUE;
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      if(bFirst)
      {
        bFirst = FALSE;
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
        if(rcode2 != 0)
        {
          break;
        }
      }
//
//  There have to be blocks on this route and service
//
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
      TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
      TRIPSKey2.blockNumber = NO_RECORD;
      TRIPSKey2.blockSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      while(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
            TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
            TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
      {
//
//  Get the base pattern nodes
//
        numBASEPatternNodes = 0;
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.nodeSequence = -1;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        bFirstNode = TRUE;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            BASEPatternNODESrecordIDs[numBASEPatternNodes] = PATTERNS.NODESrecordID;
            numBASEPatternNodes++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Build up the trip record(s) cycling through "type"
//
        for(nK = 0; nK < 5; nK++)
        {
          if(TRIPS.standard.blockNumber == 6402)
          {
            rcode2 = 0;
          }
          tripType = TripTraversal[nK];
          numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Only do those that we need to
//    0 = Revenue
//    1 = Deadhead
//    2 = Time in garage from sign on to pullout (AKA Prep Time)
//    3 = Pull Out
//    4 = Pull In
//
          lastTRIPSrecordID = TRIPS.recordID;
          lastNODESrecordID = GTResults.lastNODESrecordID;
          lastNODEtime = GTResults.lastNodeTime;
          blockNumber = TRIPS.standard.blockNumber;
          if(tripType == 0)  // Revenue
          {
            startTime = GTResults.firstNodeTime;
            endTime = GTResults.lastNodeTime;
            startPlace = GTResults.firstNODESrecordID;
            endPlace = GTResults.lastNODESrecordID;
          }
          else if(tripType == 1)  // Deadhead
          {
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            if(rcode2 != 0 ||
                  TRIPS.standard.assignedToNODESrecordID != NO_RECORD ||
                  TRIPS.standard.RGRPROUTESrecordID != ROUTES.recordID ||
                  TRIPS.standard.SGRPSERVICESrecordID != SERVICES.recordID ||
                  TRIPS.standard.blockNumber != blockNumber)
            {
              TRIPSKey0.recordID = lastTRIPSrecordID;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              continue;
            }
            numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
            if(NodesEquivalent(lastNODESrecordID, GTResults.firstNODESrecordID, &tempLong))
            {
              TRIPSKey0.recordID = lastTRIPSrecordID;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              continue;
            }
            firstNODESrecordID = GTResults.firstNODESrecordID;
            firstNODEtime = GTResults.firstNodeTime;
            TRIPSKey0.recordID = lastTRIPSrecordID;
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
            startTime = GTResults.lastNodeTime;
            startPlace = lastNODESrecordID;
            endPlace = firstNODESrecordID;
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.fromNODESrecordID = previousNode;
            GCTData.toNODESrecordID = PATTERNS.NODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            endTime = GTResults.lastNodeTime + (deadheadTime == NO_TIME ? 0 : deadheadTime);
          }
          else if(tripType == 2 || tripType == 3)  // Time in garage from sign on to pullout (AKA Prep Time) and Pullout
          {
            if(TRIPS.standard.POGNODESrecordID == NO_RECORD)
            {
              continue;
            }
            if(tripType == 2)
            {
              reportTime = 0;
              rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              while(rcode2 == 0)
              {
                if(RUNS.start.TRIPSrecordID == TRIPS.recordID)
                {
                  pieceNumber = RUNS.pieceNumber - 1;
                  if(RUNS.pieceNumber != 1)
                  {
                    for(nL = RUNS.pieceNumber - 1; nL > 0; nL--)
                    {
                      btrieve(B_GETPREVIOUS, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                    }
                  }
                  TRIPSrecordID = TRIPS.recordID;
                  numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, FALSE);
                  reportTime = RUNSVIEW[pieceNumber].reportTime;
                  TRIPSKey0.recordID = TRIPSrecordID;
                  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                  btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
                  btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }
            }
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
            if(tripType == 2)
            {
              endTime = deadheadTime == NO_TIME ? GTResults.firstNodeTime : GTResults.firstNodeTime - deadheadTime;
              endPlace = TRIPS.standard.POGNODESrecordID;
              startTime = endTime - reportTime;
            }
            else
            {
              startTime = deadheadTime == NO_TIME ?
                    GTResults.firstNodeTime : GTResults.firstNodeTime - deadheadTime;
              endTime = GTResults.firstNodeTime;
              endPlace = GTResults.firstNODESrecordID;
            }
            startPlace = TRIPS.standard.POGNODESrecordID;
          }    
          else if(tripType == 4)  // Pullin
          {
            if(TRIPS.standard.PIGNODESrecordID == NO_RECORD)
            {
              continue;
            }
            startTime = GTResults.lastNodeTime;
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
            endTime = deadheadTime == NO_TIME ?
                  GTResults.lastNodeTime : GTResults.lastNodeTime + deadheadTime;
            startPlace = GTResults.lastNODESrecordID;
            endPlace = TRIPS.standard.PIGNODESrecordID;
          }
//
//  Record type
//
          strcpy(outputString, "11,");
//
//  Booking ID
//
          strcat(outputString, szBookingID);
          strcat(outputString, ",");
//
//  Service type
//
          sprintf(tempString, "%ld,", SERVICES.number);
          strcat(outputString, tempString);
//
//  Trip type
//
          sprintf(tempString, "%d,", tripType);
          strcat(outputString, tempString);
//
//  Trip ID
//
          sprintf(tempString, "%ld,", TRIPS.tripNumber);
          strcat(outputString, tempString);
          if(TRIPS.tripNumber == 716)
          {
            bReposition = FALSE;
          }
//
//  Block number
//
          sprintf(tempString, "%ld,", TRIPS.standard.blockNumber);
          strcat(outputString, tempString);
//
//  Direction number
//
          sprintf(tempString, "%ld,", TRIPS.directionIndex);
          strcat(outputString, tempString);
//
//  Direction name
//
          bReposition = (TRIPS.ROUTESrecordID != TRIPS.standard.RGRPROUTESrecordID);
          if(bReposition)  // could be on a different RGRP
          {
            ROUTESrecordID = ROUTES.recordID;
            ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          }
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
          tempString[TRIPFILE_DIRECTIONNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  Route ID
//
//          sprintf(tempString, "%ld,", TRIPS.ROUTESrecordID);
//          strcat(outputString, tempString);
//  New: Route ID - Make it as close to "Route Short Name" as possible
//       This takes alphas into account and outputs them 62A to 621, 62B to 622, and so on.
//
          strncpy(szRouteID, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(szRouteID, ROUTES_NUMBER_LENGTH);
          szRouteID[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
          ptr = szRouteID;
          while(*ptr)
          {
            if(isalpha(*ptr))
            {
              *ptr -= 16;
            }
            ptr++;
          }
          strcat(outputString, szRouteID);
          strcat(outputString, ",");
          if(bReposition)
          {
            ROUTESKey0.recordID = ROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            btrieve(B_GETPOSITION, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            btrieve(B_GETDIRECT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
          }
//
//  Route Variant (Pattern)
//
          if(tripType == 0)
          {
            PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            trim(tempString, PATTERNNAMES_NAME_LENGTH);
            tempString[TRIPFILE_PATTERNNAME_LENGTH] = '\0';
            strcat(outputString, tempString);
          }
          strcat(outputString, ",");
//
//  Save the position up to this point for use in the "Passing Time" portion later on
//
          strcpy(outputStringSave, outputString);
//
//  Distance (in miles)
//
          if(tripType == 0)
          {
            sprintf(tempString, "%4.1f,", GTResults.tripDistance);
          }
          else
          {
            strcpy(tempString, "0,");
          }
          strcat(outputString, tempString);
//
//  Comment ID
//
          if(tripType == 0)
          {
            sprintf(tempString, "%ld,", (TRIPS.COMMENTSrecordID == NO_RECORD ? 0L : TRIPS.COMMENTSrecordID));
          }
          else
          {
            strcpy(tempString, "0,");
          }
          strcat(outputString, tempString);
//
//  Primary Sign ID
//
          if(tripType == 0)
          {
            sprintf(tempString, "%ld,", (TRIPS.SIGNCODESrecordID == NO_RECORD ? 0L : TRIPS.SIGNCODESrecordID));
          }
          else
          {
            strcpy(tempString, "0,");
          }
          strcat(outputString, tempString);
//
//  Start time
//
          sprintf(tempString, "%ld,", startTime);
          strcat(outputString, tempString);
//
//  End time
//
          sprintf(tempString, "%ld,", endTime);
          strcat(outputString, tempString);
//
//  Start place
//
          primaryNODESrecordID = NodeEquivalenced(startPlace);
          NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? startPlace : primaryNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  End place
// 
          primaryNODESrecordID = NodeEquivalenced(endPlace);
          NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? endPlace : primaryNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  Special trip option ID
//
          strcat(outputString, "0,");
//
//  Opdays
//
          strcat(outputString, szOpDays[nI]);
          strcat(outputString, ",");
//
//  Primary route ID
//
          strcat(outputString, szRouteID);
          strcat(outputString, ",");
//
//  Secondary route ID
//  Seconday sign ID
//  External trip ID
//
//  All 0 if the Secondary sign ID is NO_RECORD
//
          if(TRIPS.secondarySIGNCODESrecordID == NO_RECORD || TRIPS.secondarySIGNCODESrecordID == 0)  // 0 for backwards compatibility
          {
            strcat(outputString, "0,0");
          }
          else
          {
//
//  Secondary route ID (Same as primary)
//
            strcat(outputString, szRouteID);
            strcat(outputString, ",");
//
//  Secondary sign code 
//
            sprintf(tempString, "%ld", TRIPS.secondarySIGNCODESrecordID);
            strcat(outputString, tempString);
          }
//
//  Write out the primary trip record
//
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Passing times (tripType 0)
//
          if(TRIPS.standard.blockNumber == 6801)
          {
            nL = 0;
          }
          if(tripType == 0)
          {
//
//  Build the record
//
            outputStringSave[0] = '2';
            outputStringSave[1] = '0';
            nL = 0;
            nM = 0;
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.nodeSequence = -1;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            bFirstNode = TRUE;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                while(nL < numBASEPatternNodes)
                {
                  bFound = FALSE;
                  if(PATTERNS.NODESrecordID == BASEPatternNODESrecordIDs[nL])
                  {
                    bFound = TRUE;
                    NODESKey0.recordID = PATTERNS.NODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    if(bFirstNode)
                    {
                      bFirstNode = FALSE;
                      distance = 0.0;
                    }
                    else
                    {
                      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                      GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                      GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                      GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                      GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                      GCTData.fromNODESrecordID = previousNode;
                      GCTData.toNODESrecordID = PATTERNS.NODESrecordID;
                      GCTData.timeOfDay = GTResults.firstNodeTime;
                      GetConnectionTime(GCT_FLAG_RUNNINGTIME, &GCTData, &distance);
                      if(distance < 0.0)
                      {
                        distance = -distance;
                      }
                    }
//
//  Copy the first portion of the output string
//
                    strcpy(outputString, outputStringSave);
//
//  Distance (in miles) from previous timepoint
//
                    sprintf(tempString, "%4.1f,", distance);
                    strcat(outputString, tempString);
//
//  Comment ID
//
                    sprintf(tempString, "%ld,", NODES.COMMENTSrecordID == NO_RECORD ? 0 : NODES.COMMENTSrecordID);
                    strcat(outputString, tempString);
//
//  Stop number (record ID)
// 
//  If there's a node equivalence along the pattern, change the value to the primaryNODESrecordID
//
                    primaryNODESrecordID = NodeEquivalenced(NODES.recordID);
                    if(primaryNODESrecordID != NO_RECORD)
                    {
                      previousNode = NODES.recordID;
                      NODESKey0.recordID = primaryNODESrecordID;
                      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    }
/*
//
//  If there's a node equivalence along the pattern, change the value to the first node encountered
//
                    for(nJ = 0; nJ < nL; nJ++)
                    {
                      if(NodesEquivalent(BASEPatternNODESrecordIDs[nJ], NODES.recordID, &tempLong) &&
                            BASEPatternNODESrecordIDs[nJ] != NODES.recordID)
                      {
                        sprintf(tempString, "INFO - Node equivalence replacement (TR3/%s): Trip type = %d, Route recordID = %ld (Node %ld overrides %ld)\r\n",
                              outputFileName, tripType, TRIPS.ROUTESrecordID, BASEPatternNODESrecordIDs[nJ], NODES.recordID);
                        _lwrite(hfErrorLog, tempString, strlen(tempString));
                        NODES.recordID = BASEPatternNODESrecordIDs[nJ];
                        break;
                      }
                    }
*/
//                    sprintf(tempString, "%ld,", NODES.recordID);
//                    strcat(outputString, tempString);
//
//  Change to stop number (as derived from long name) : Bob Antonisse, 2-Aug-07
//
//                    strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
//                    trim(tempString, NODES_LONGNAME_LENGTH);
//                    tempLong = atol(&tempString[4]);
//                    sprintf(tempString, "%ld,", tempLong);
//                    strcat(outputString, tempString);
//
//  Timepoints don't have the Stopxxxx naming convention, so get the value elsewise
//
/*
                    stopNumber = (TRIPS.directionIndex == 0 ? NODES.OBStopNumber : NODES.IBStopNumber);
                    stopNODESrecordID = NO_RECORD;
                    bFound = FALSE;
                    if(stopNumber <= 0)
                    {
                      stopNumber = NODES.number;
                    }
                    if(stopNumber > 0)
                    {
                      rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
                      while(rcode2 == 0)
                      {
                        if((NODES.flags & NODES_FLAG_STOP) && NODES.number == stopNumber)
                        {
                          stopNODESrecordID = NODES.recordID;
                          bFound = TRUE;
                          break;
                        }
                        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
                      }
                    }
                    if(!bFound)
                    {
                      strcat(outputString, "0,");
                    }
                    else
                    {
                      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
                      trim(tempString, NODES_LONGNAME_LENGTH);
                      tempLong = atol(&tempString[4]);
                      sprintf(tempString, "%ld,", tempLong);
                      strcat(outputString, tempString);
                    }
*/
                    sprintf(tempString, "%ld,", NODES.recordID);
                    strcat(outputString, tempString);
//
//  Passing time (time at the node)
//
                    sprintf(tempString, "%ld,", GTResults.tripTimes[nM++]);
                    strcat(outputString, tempString);
//
//  Transfer point only (always 0)
//
                    strcat(outputString, "0\r\n");
//
//  Write out the secondary trip record
//
                    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle back
//
                    if(primaryNODESrecordID == NO_RECORD)
                    {
                      previousNode = NODES.recordID;
                    }
                  }
                  nL++;
                  if(bFound)
                  {
                    break;
                  }
                }
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            } // while on the patterns
          } // tripType == 0
        } // nK on tripTypes
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      }  // while going through the trips
    }  // nJ - routes
  }  // nI - services
//
//  Close the files
//
  _lclose(hfOutputFile);
//
//  ================================
//  Comment File - com<bbbbbbb>.dat
//  ================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "com");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
  recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
  rcode2 = btrieve(B_GETFIRST, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
  while(rcode2 == 0)
  {
    memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
    szarString[0] = 0;
    nI = 1;
    for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
         tempString2 = strtok(NULL, "\r\n") )
    {
//
//  Record type - 30
//
      strcpy(outputString, "30,");
//
//  Comment ID
//
      sprintf(tempString, "%ld,", COMMENTS.recordID);
      strcat(outputString, tempString);
//
//  Comment sequence
//
      sprintf(tempString, "%d,", nI);
      strcat(outputString, tempString);
//
//  Comment symbol
//
      strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
      trim(tempString, COMMENTS_CODE_LENGTH);
      tempString[COMFILE_SYMBOL_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Comment abbr (same as symbol)
//
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Comment text
//
      while(ptr = strchr(tempString2, '"'))
      {
        strcpy(ptr, ptr + 1);
      }
      sprintf(tempString, "\"%s\"", tempString2);
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      nI++;
    }
//
//  Get the next comment record
//
    rcode2 = btrieve(B_GETNEXT, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
//
//  =================================
//  Block File - blk<ss><bbbbbbb>.dat
//  =================================
//
//
//  Cycle through the services
//
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    sprintf(outputFileName, "blk%02ld%s.dat", SERVICES.number, szBookingName);
    hfOutputFile = _lcreat(outputFileName, 0);
    if(hfOutputFile == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(szarString, szFormatString, outputFileName);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
      goto done;
    }
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    strcpy(tempString, outputFileName);
    strcat(tempString, " - ");
    strcat(tempString, serviceName);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    bFirst = TRUE;
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      if(bFirst)
      {
        bFirst = FALSE;
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
        if(rcode2 != 0)
        {
          break;
        }
      }
//
//  There have to be blocks on this route and service
//
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
      TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
      TRIPSKey2.blockNumber = NO_RECORD;
      TRIPSKey2.blockSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      while(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
            TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
            TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
      {
//
//  If it's not a record with a P/O, cycle through until we hit one
//
        if(TRIPS.standard.POGNODESrecordID == NO_RECORD)
        {
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2,2 );
        }
//
//  Build up the record
//
        else
        {
//  Record type
//
          strcpy(outputString, "10,");
//
//  Booking ID
//
          strcat(outputString, szBookingID);
          strcat(outputString, ",");
//
//  Service type
//
          sprintf(tempString, "%ld,", SERVICES.number);
          strcat(outputString, tempString);
//
//  Block number
//
          sprintf(tempString, "%ld,", TRIPS.standard.blockNumber);
          strcat(outputString, tempString);
//
//  Route ID
//
//          sprintf(tempString, "%ld,", TRIPS.standard.RGRPROUTESrecordID);
//          strcat(outputString, tempString);
//  New: Route ID - Make it as close to "Route Short Name" as possible
//       This takes aplhas into account and outputs them 62A to 621, 62B to 622, and so on.
//  New: Make it the route ID of the first trip on the block
//
          ROUTESrecordID = ROUTES.recordID;
          ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
          ptr = tempString;
          while(*ptr)
          {
            if(isalpha(*ptr))
            {
              *ptr -= 16;
            }
            ptr++;
          }
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  Reposition the Routes file
//
          ROUTESKey0.recordID = ROUTESrecordID;
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          btrieve(B_GETPOSITION, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          btrieve(B_GETDIRECT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
//
//  Logon block ID
//
          sprintf(tempString, "%ld,", TRIPS.standard.blockNumber);
          strcat(outputString, tempString);
//
//  Start time
//
//  Generate the trip
//
          numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Determine the P/O Time
//
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
          POTime = deadheadTime == NO_TIME ?
                GTResults.firstNodeTime : GTResults.firstNodeTime - deadheadTime;
          sprintf(tempString, "%ld,", POTime);
          strcat(outputString, tempString);
          startNODESrecordID = TRIPS.standard.POGNODESrecordID;
          BUSTYPESrecordID = TRIPS.BUSTYPESrecordID;
//
//  End time
//
//  Cycle through the block until we hit the P/I
//
          blockNumber = TRIPS.standard.blockNumber;
          PIGNODESrecordID = NO_RECORD;
          while(rcode2 == 0 &&
                TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
                TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
                TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID &&
                TRIPS.standard.blockNumber == blockNumber)
          {
            if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
            {
              PIGNODESrecordID = TRIPS.standard.PIGNODESrecordID;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          }
//
//  Got it - maybe
//
          if(PIGNODESrecordID != NO_RECORD)
          {
            numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
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
            PITime = deadheadTime == NO_TIME ?
                  GTResults.lastNodeTime : GTResults.lastNodeTime + deadheadTime;
            sprintf(tempString, "%ld,", PITime);
            strcat(outputString, tempString);
//
//  Start place
//
            primaryNODESrecordID = NodeEquivalenced(startNODESrecordID);
            NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? startNODESrecordID : primaryNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  End place
// 
            primaryNODESrecordID = NodeEquivalenced(TRIPS.standard.PIGNODESrecordID);
            NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? TRIPS.standard.PIGNODESrecordID : primaryNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  Garage
//
            primaryNODESrecordID = NodeEquivalenced(startNODESrecordID);
            NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? startNODESrecordID : primaryNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  Block name
//
            sprintf(tempString, "%ld,", TRIPS.standard.blockNumber);
            strcat(outputString, tempString);
//
//  Preferred bustype
//
            if(BUSTYPESrecordID == NO_RECORD)
            {
              strcpy(tempString, "0");
            }
            else
            {
              BUSTYPESKey0.recordID = BUSTYPESrecordID;
              btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
              strncpy(tempString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
              trim(tempString, BUSTYPES_NAME_LENGTH);
              tempString[BLKFILE_BUSTYPES_LENGTH] = '\0';
            }
            strcat(outputString, tempString);
            strcat(outputString, ",0,0\r\n");
//
//  Write out the record
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }  // PIG != NO_RECORD
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2,2 );
        }  // POG != NO_RECORD
      }  // while
    }  // nJ
  }  // nI
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===============================
//  Run File - run<ss><bbbbbbb>.dat
//  ===============================
//
//  Cycle through the services
//
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    sprintf(outputFileName, "run%02ld%s.dat", SERVICES.number, szBookingName);
    hfOutputFile = _lcreat(outputFileName, 0);
    if(hfOutputFile == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(szarString, szFormatString, outputFileName);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
      goto done;
    }
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    strcpy(tempString, outputFileName);
    strcat(tempString, " - ");
    strcat(tempString, serviceName);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Get the garage name.  Orbital uses the terms division and garage interchangably - we don't
//
    strcpy(szDivisionName, "GAR");
//
//  Cycle through the divisions
//
    rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
    bFirst = TRUE;
    for(nJ = 0; nJ < maxDivisions; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      if(bFirst)
      {
        bFirst = FALSE;
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
        if(rcode2 != 0)
        {
          break;
        }
      }
//
//  Get the runs
//
      RUNSKey1.DIVISIONSrecordID = DIVISIONS.recordID;
      RUNSKey1.SERVICESrecordID = SERVICES.recordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == DIVISIONS.recordID &&
            RUNS.SERVICESrecordID == SERVICES.recordID)
      {
//
//  Build up the run "50" record
//
//  Record type
//
        strcpy(outputString, "50,");
//
//  Booking ID
//
        strcat(outputString, szBookingID);
        strcat(outputString, ",");
//
//  Service type
//
        sprintf(tempString, "%ld,", SERVICES.number);
        strcat(outputString, tempString);
//
//  Division name
//
        strcat(outputString, szDivisionName);
        strcat(outputString, ",");
//
//  Run ID
//
        sprintf(tempString, "%ld,", RUNS.runNumber);
        strcat(outputString, tempString);
//
//  Save the position up to this point for use in the "Passing Time" portion later on
//
        strcpy(outputStringSave, outputString);
//
//  Number of pieces
//
        RUNSrecordID = RUNS.recordID;
        numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, FALSE);
        sprintf(tempString, "%ld\r\n", numPieces);
        strcat(outputString, tempString);
//
//  Reposition
//
        RUNSKey0.recordID = RUNSrecordID;
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
//
//  Run name
//
//      Not used.

//
//  Write out the record
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle through the number of pieces
//
        for(nK = 0; nK < numPieces; nK++)
        {
//
//  Build the secondary (piece) record
//
          outputStringSave[0] = '5';
          outputStringSave[1] = '1';
//
//  Copy the first portion of the output string
//
          strcpy(outputString, outputStringSave);
//
//  Block number
//
          sprintf(tempString, "%ld,", RUNSVIEW[nK].blockNumber);
          strcat(outputString, tempString);
//
//  Piece number
//
          sprintf(tempString, "%d,", nK);
          strcat(outputString, tempString);
//
//  Start place
//
          primaryNODESrecordID = NodeEquivalenced(RUNSVIEW[nK].runOnNODESrecordID);
          NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? RUNSVIEW[nK].runOnNODESrecordID : primaryNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[RUNFILE_NODENAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  End place
//
          primaryNODESrecordID = NodeEquivalenced(RUNSVIEW[nK].runOffNODESrecordID);
          NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? RUNSVIEW[nK].runOffNODESrecordID : primaryNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          tempString[TRIPFILE_NODENAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  Signon time of piece
//
          tempLong = RUNSVIEW[nK].runOnTime - RUNSVIEW[nK].reportTime - RUNSVIEW[nK].startOfPieceTravel;
          if(RUNSVIEW[nK].startOfPieceExtraboardStart != NO_TIME)
          {
            tempLong = RUNSVIEW[nK].startOfPieceExtraboardStart;
          }
          sprintf(tempString, "%ld,", tempLong);
          strcat(outputString, tempString);
//
//  Start time of piece
//
          sprintf(tempString, "%ld,", RUNSVIEW[nK].runOnTime);
          strcat(outputString, tempString);
//
//  End time of piece
//
          sprintf(tempString, "%ld,", RUNSVIEW[nK].runOffTime);
          strcat(outputString, tempString);
//
//  Clear time of piece
//
          tempLong = RUNSVIEW[nK].runOffTime + RUNSVIEW[nK].endOfPieceTravel + RUNSVIEW[nK].turninTime;
          if(RUNSVIEW[nK].endOfPieceExtraboardEnd != NO_TIME)
          {
            tempLong = RUNSVIEW[nK].endOfPieceExtraboardEnd;
          }
          sprintf(tempString, "%ld,", tempLong);
          strcat(outputString, tempString);
//
//  Special Trip Option ID
//
          strcat(outputString, "0,");
//
//  Opdays
//
          strcat(outputString, szOpDays[nI]);
          strcat(outputString, "\r\n");
//
//  Write out the record and position to the next piece
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        }  // nK
      }  // while
    }  // nJ
  }  // nI
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =========================================
//  Transfer Request File - treq<bbbbbbb>.dat
//  =========================================
//
//  The Transfer Request file lists all potential transfers flagged as such within the database.
//  Specifically, if a node has been flagged with NODES_FLAG_AVLTRANSFERPOINT, then it is eligible
//  for consideration as a transfer point.  Stops (NODES_FLAG_STOP) are not considered.
//
//  The idea here is that "origin" timepoints are considered as those occurring 2 through n in the
//  pattern, and "destination" timepoints are those that fit 1 though n-1 on their pattern.
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "treq");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Go through all the BASE patterns to get those timepoints marked as NODES_FLAG_AVLTRANSFERPOINT
//
//  Build a list of routes - there has to be at least one trip on the route
//
  numRoutes = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(numRoutes >= TMSRPT61_MAXROUTES)
    {
      break;
    }
//
//  There have to be trips on this route and service
//
    TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
    TRIPSKey1.SERVICESrecordID = 0;
    TRIPSKey1.directionIndex = 0;
    TRIPSKey1.tripSequence = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    if(rcode2 == 0 &&
          TRIPS.ROUTESrecordID == ROUTES.recordID)
    {
      routeList[numRoutes] = ROUTES.recordID;
      numRoutes++;
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Check on overflow
//
  if(numRoutes >= TMSRPT61_MAXROUTES)
  {
    sprintf(tempString, "*ERR - Number of Routes exceeds %d\r\n", TMSRPT61_MAXROUTES);
    _lwrite(hfErrorLog, tempString, strlen(tempString));
    bGotError = TRUE;
  }
//
//  Zero out the TREQPATDATA structure
//
  memset(&TREQPATDATA, 0x00, sizeof(TREQPATDATADef) * TMSRPT61_MAXROUTES * 2);
//
//  Cycle through the routes to build up the TREQPATDATA structure
//
//  Just consider the weekday service
//
  numTREQ = 0;
  for(nI = 0; nI < numRoutes; nI++)
  {
    if(numTREQ >= TMSRPT61_MAXTREQ)
    {
      break;
    }
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Go through the directions
//
    ROUTESKey0.recordID = routeList[nI];
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(numTREQ >= TMSRPT61_MAXTREQ)
      {
        break;
      }
      TREQPATDATA[nI][nJ].ROUTESrecordID = routeList[nI];
      if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
      {
        continue;
      }
//
//  Get the base pattern for this route and direction
//
      bFirstTimepoint = TRUE;
      PATTERNSKey2.ROUTESrecordID = routeList[nI];
      PATTERNSKey2.SERVICESrecordID = 1;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.directionIndex = nJ;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == routeList[nI] &&
            PATTERNS.SERVICESrecordID == 1 &&
            PATTERNS.PATTERNNAMESrecordID == basePatternRecordID &&
            PATTERNS.directionIndex == nJ)
      {
//
//  Only look at timepoints
//
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
        {
//
//  Only look at timepoints flagged as a transfer point
//
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          if(NODES.flags & NODES_FLAG_AVLTRANSFERPOINT)
          {
            TREQPATDATA[nI][nJ].NODESrecordIDs[TREQPATDATA[nI][nJ].numNodes] = NODES.recordID;
            if(bFirstTimepoint)
            {
              TREQPATDATA[nI][nJ].flags[TREQPATDATA[nI][nJ].numNodes] = TREQPATDATA_FLAG_FIRSTNODEONPATTERN;
            }
            PATTERNSrecordID = PATTERNS.recordID;
//
//  Is this the last timepoint on the pattern?
//
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            if(rcode2 != 0 ||
                  PATTERNS.ROUTESrecordID != routeList[nI] ||
                  PATTERNS.SERVICESrecordID != 1 ||
                  PATTERNS.PATTERNNAMESrecordID != basePatternRecordID ||
                  PATTERNS.directionIndex != nJ)
            {
              TREQPATDATA[nI][nJ].flags[TREQPATDATA[nI][nJ].numNodes] |= TREQPATDATA_FLAG_LASTNODEONPATTERN;
            }
            if(rcode2 == 0)
            {
              rcode2 = btrieve(B_GETPREVIOUS, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
            TREQPATDATA[nI][nJ].numNodes++;
          }
        }
//
//  Get the next pattern record
//
        bFirstTimepoint = FALSE;
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
    }  // nJ
  }  // nI
//
//  Go through all the TREQPATDATA records to build up the output record
//
  for(nI = 0; nI < numRoutes; nI++)
  {
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(TREQPATDATA[nI][nJ].numNodes == 0)
      {
        continue;
      }
      for(nK = 0; nK < numRoutes; nK++)
      {
        if(nI == nK)
        {
          continue;
        }
        for(nL = 0; nL < 2; nL++)
        {
          for(nM = 0; nM < TREQPATDATA[nI][nJ].numNodes; nM++)
          {
            if(TREQPATDATA[nI][nJ].flags[nM] & TREQPATDATA_FLAG_FIRSTNODEONPATTERN)
            {
              continue;
            }
            for(nN = 0; nN < TREQPATDATA[nK][nL].numNodes; nN++)
            {
              if(TREQPATDATA[nK][nL].flags[nN] & TREQPATDATA_FLAG_LASTNODEONPATTERN)
              {
                continue;
              }
              if(TREQPATDATA[nI][nJ].NODESrecordIDs[nM] == TREQPATDATA[nK][nL].NODESrecordIDs[nN] ||
                    NodesEquivalent(TREQPATDATA[nI][nJ].NODESrecordIDs[nM], TREQPATDATA[nK][nL].NODESrecordIDs[nN], &tempLong))
              {
                TREQ[numTREQ].origin.ROUTESrecordID = routeList[nI];
                TREQ[numTREQ].origin.directionNumber = nJ;
                primaryNODESrecordID = NodeEquivalenced(TREQPATDATA[nI][nJ].NODESrecordIDs[nM]);
                if(primaryNODESrecordID == NO_RECORD)
                {
                  primaryNODESrecordID = TREQPATDATA[nI][nJ].NODESrecordIDs[nM];
                }
                TREQ[numTREQ].origin.NODESrecordID = primaryNODESrecordID;
                TREQ[numTREQ].destination.ROUTESrecordID = routeList[nK];
                TREQ[numTREQ].destination.directionNumber = nL;
                primaryNODESrecordID = NodeEquivalenced(TREQPATDATA[nK][nL].NODESrecordIDs[nN]);
                if(primaryNODESrecordID == NO_RECORD)
                {
                  primaryNODESrecordID = TREQPATDATA[nK][nL].NODESrecordIDs[nN];
                }
                TREQ[numTREQ].destination.NODESrecordID = primaryNODESrecordID;
                numTREQ++;
              }
            }
          }
        }
      }
    }
  }
//
//  Check on overflow
//
  if(numTREQ >= TMSRPT61_MAXTREQ)
  {
    sprintf(tempString, "*ERR - Number of Transfer Request entries exceeds %d\r\n", TMSRPT61_MAXTREQ);
    _lwrite(hfErrorLog, tempString, strlen(tempString));
    bGotError = TRUE;
  }
//
//  Dump out the structure
//
  for(nI = 0; nI < numTREQ; nI++)
  {
//
//  Record type
//
    strcpy(outputString, "70,");
//
//  Booking ID
//
    strcat(outputString, szBookingID);
    strcat(outputString, ",");
//
//  Origin direction number
//
    sprintf(tempString, "%d,", TREQ[nI].origin.directionNumber);
    strcat(outputString, tempString);
//
//  Origin direction name
//
    ROUTESKey0.recordID = TREQ[nI].origin.ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TREQ[nI].origin.directionNumber];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    tempString[RTEFILE_DIRECTIONNAME_LENGTH] = '\0';
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Origin Route ID - Make it as close to "Route Short Name" as possible
//  This takes aplhas into account and outputs them 62A to 621, 62B to 622, and so on.
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
    ptr = tempString;
    while(*ptr)
    {
      if(isalpha(*ptr))
      {
        *ptr -= 16;
      }
      ptr++;
    }
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Origin place name (NODES.abbrName)
//
    primaryNODESrecordID = NodeEquivalenced(TREQ[nI].origin.NODESrecordID);
    NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? TREQ[nI].origin.NODESrecordID : primaryNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Destination direction number
//
    sprintf(tempString, "%d,", TREQ[nI].destination.directionNumber);
    strcat(outputString, tempString);
//
//  Destination direction name
//
    ROUTESKey0.recordID = TREQ[nI].destination.ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TREQ[nI].destination.directionNumber];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    tempString[RTEFILE_DIRECTIONNAME_LENGTH] = '\0';
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Destination Route ID - Make it as close to "Route Short Name" as possible
//  This takes aplhas into account and outputs them 62A to 621, 62B to 622, and so on.
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
    ptr = tempString;
    while(*ptr)
    {
      if(isalpha(*ptr))
      {
        *ptr -= 16;
      }
      ptr++;
    }
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Destination place name (NODES.abbrName)
//
    primaryNODESrecordID = NodeEquivalenced(TREQ[nI].destination.NODESrecordID);
    NODESKey0.recordID = primaryNODESrecordID == NO_RECORD ? TREQ[nI].destination.NODESrecordID : primaryNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Distance
//
    strcat(outputString, "0.0,");
//
//  Hub ID
//
    strcat(outputString, "0,");
//
//  Transfer level
//
    strcat(outputString, "1,");
//
//  Stay in seat
//
    strcat(outputString, "0\r\n");
//
//  Write out the record
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
  } //nI
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===========================================
//  Real Timepoints File - RealTimepoints.dat
//  ===========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "RealTimepoints.dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Cycle through the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP) && NodeEquivalenced(NODES.recordID) == NO_RECORD)
    {
      if(!m_bUseLatLong)
      {
        EN2LL(23, (double)NODES.longitude, (double)NODES.latitude, szUTMZone, &Lat, &Long);
        NODES.latitude = (float)Lat;
        NODES.longitude = (float)Long;
      }
//
//  Place name (NODES.abbrName)
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strcpy(outputString, tempString);
      strcat(outputString, ",");
//
//  Place description
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      tempString[PLACEFILE_DESCRIPTION_LENGTH] = '\0';
      while(ptr = strchr(tempString, '"'))
      {
        strcpy(ptr, ptr + 1);
      }
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Longitude
//
      sprintf(szarString, "%12.6f,", NODES.longitude);
      ptr = szarString;
      while(ptr && *ptr == ' ')
      {
        ptr++;
      }
      strcat(outputString, ptr);
//
//  Latitude
//
      sprintf(szarString, "%12.6f\r\n", NODES.latitude);
      ptr = szarString;
      while(ptr && *ptr == ' ')
      {
        ptr++;
      }
      strcat(outputString, ptr);
//
//  Write out the record and get the next node
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==========================================
//  Virtual Timepoints File - vtp<bbbbbbb>.dat
//  ==========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "vtp");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Traverse the Routes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Fare code
//
    COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    if(rcode2 != 0)
    {
      fareCode = NO_RECORD;
      strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szarString, ROUTES_NUMBER_LENGTH);
      sprintf(tempString, "*ERR - Bad route Fare Code when processing Virutal timepoint file.  Route: %s, Code: %ld\r\n",
            szarString, ROUTES.COMMENTSrecordID);
      _lwrite(hfErrorLog, tempString, strlen(tempString));
      bGotError = TRUE;
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      continue;
    }
    else
    {
      memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
      strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
      trim(tempString, COMMENTS_CODE_LENGTH);
      fareCode = (int)strtoul(tempString, &ptr, 10);
    }
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
//
//  And the directions on the route
//
    for(nI = 0; nI < 2; nI++)
    {
      if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      {
        continue;
      }
//
//  There must be patterns on this route to record virtual timepoints
//
      PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
      PATTERNSKey2.SERVICESrecordID = 1;
      PATTERNSKey2.directionIndex = nI;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      if(rcode2 != 0 ||
            PATTERNS.ROUTESrecordID != ROUTES.recordID ||
            PATTERNS.SERVICESrecordID != 1 ||
            PATTERNS.directionIndex != nI)
      {
        continue;
      }
//
//  Record type
//
      strcpy(outputString, "80,");
//
//  Booking ID
//
      strcat(outputString, szBookingID);
      strcat(outputString, ",");
//
//  Direction num
//
      sprintf(tempString, "%d,", nI);
      strcat(outputString, tempString);
//
//  Direction name
//
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
      trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
      tempString[VTPFILE_DIRECTIONNAME_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Route ID - Make it as close to "Route Short Name" as possible
//  This takes aplhas into account and outputs them 62A to 621, 62B to 622, and so on.
//
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
      ptr = tempString;
      while(*ptr)
      {
        if(isalpha(*ptr))
        {
          *ptr -= 16;
        }
        ptr++;
      }
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Save the front of the record
//
      strcpy(outputStringSave, outputString);
//
//  Route variant (Pattern)
//  
//  Establish the route tracings file name
//
      strcpy(MIFPASSEDDATA.szMIFFileName, szRouteTracingsFolder);
      strcat(MIFPASSEDDATA.szMIFFileName, "\\rt");
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      tempString[RTEFILE_ROUTENUMBER_LENGTH] = '\0';
//      if(isalpha(tempString[strlen(tempString) - 1]))
//      {
//        tempString[strlen(tempString) - 1] -= 16;
//      }
//      else
//      {
//        strcat(tempString, "0");
//      }
      ptr = tempString;
      while(*ptr)
      {
        if(isalpha(*ptr))
        {
          *ptr -= 16;
        }
        ptr++;
      }
      sprintf(szarString, "%07ld%d", atol(tempString), nI);
      strcat(MIFPASSEDDATA.szMIFFileName, szarString);
      strcat(szarString, "\r\n");
      _lwrite(hfErrorLog, szarString, strlen(szarString));
//
//  Start here as the main while loop for all patterns on this route/ser/dir
//
      if(ROUTES.recordID == 14)
      {
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
      }
      PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
      PATTERNSKey2.SERVICESrecordID = 1;
      PATTERNSKey2.directionIndex = nI;
      PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == ROUTES.recordID &&
            PATTERNS.SERVICESrecordID == 1 &&
            PATTERNS.directionIndex == nI)
      {
        PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
//
//  Build the MIFLINES structure based on this pattern
//
        if(ROUTES.recordID == 14)
        {
          previousFareCode = NO_RECORD;
        }
        MIFPASSEDDATA.ROUTESrecordID = ROUTES.recordID;
        MIFPASSEDDATA.SERVICESrecordID = 1L;
        MIFPASSEDDATA.directionIndex = nI;
        MIFPASSEDDATA.PATTERNNAMESrecordID = PATTERNNAMESrecordID;
        MIFPASSEDDATA.hfErrorLog = hfErrorLog;
        numMIFLINES = MIFDigest(&MIFPASSEDDATA, &MIFLINES[0]);
        if(numMIFLINES >= TMSRPT61_MAXMIFLINES)
        {
          sprintf(tempString, "*ERR - Number of MIF Lines exceeds %d\r\n", TMSRPT61_MAXMIFLINES);
          _lwrite(hfErrorLog, tempString, strlen(tempString));
          bGotError = TRUE;
        }
#define DUMPLINES

#ifdef DUMPLINES
        sprintf(tempString, "*Inf - Dump of line arrangement in map\r\n");
        _lwrite(hfErrorLog, tempString, strlen(tempString));
        for(nJ = 0; nJ < numMIFLINES; nJ++)
        {
          pFrom = &MIFLINES[nJ].from;
          pTo = &MIFLINES[nJ].to;
//
//  From Node
//
          if(pFrom->NODESrecordID == NO_RECORD)
          {
            strcpy(szFromNode, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szFromNode, NODES_ABBRNAME_LENGTH);
          }
//
//  To Node
//
          if(pTo->NODESrecordID == NO_RECORD)
          {
            strcpy(szToNode, "");
          }
          else
          {
            NODESKey0.recordID = pTo->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szToNode, NODES_ABBRNAME_LENGTH);
          }
//
//  From stop
//
          if(pFrom->associatedStopNODESrecordID == NO_RECORD)
          {
            strcpy(szFromStop, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szFromStop, NODES_LONGNAME_LENGTH);
          }
//
//  To stop
//
          if(pTo->associatedStopNODESrecordID == NO_RECORD)
          {
            strcpy(szToStop, "");
          }
          else
          {
            NODESKey0.recordID = pTo->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szToStop, NODES_LONGNAME_LENGTH);
          }
          sprintf(tempString, "%4d,\"%4s\",\"%4s\",%10.7f,%10.7f,\"%4s\",\"%4s\",%10.7f,%10.7f\r\n", nJ,
                szFromNode, szFromStop, pFrom->longitude, pFrom->latitude,
                szToNode, szToStop, pTo->longitude, pTo->latitude);
          _lwrite(hfErrorLog, tempString, strlen(tempString));
        }
#endif
//
//  Pattern name
//
        PATTERNNAMESKey0.recordID = PATTERNNAMESrecordID;
        btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
        strncpy(szPatternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
        trim(szPatternName, PATTERNNAMES_NAME_LENGTH);
        szPatternName[VTPFILE_PATTERNNAME_LENGTH] = '\0';
//
//  Output the pattern data
//
        previousFareCode = NO_RECORD;
//
//  For mid-trip layovers, we need to add an extra output record
//
        totalOutputLines = numMIFLINES;
        for(nJ = 0; nJ < numMIFLINES; nJ++)
        {
          if(MIFLINES[nJ].flags & MIFLINES_FLAG_NEXTISSAMELOCATION)
          {
            totalOutputLines++;
          }
        }
//
//  There are nine cases of timepoints and/or bus stops netween two named points.
//  A node can be a) a timepoint b) a stop c) a timepoint with an associated stop
//
//  MIFLINES contains node and associated stop data.
//  MIFPASSEDDATA contains an in-order list of the nodes/stops
//
        seq = 0;
        outputSeq = 1;
        for(nJ = 0; nJ < numMIFLINES; nJ++)
        {
          fromNODESrecordID = MIFPASSEDDATA.NODESrecordIDs[seq];
          toNODESrecordID = MIFPASSEDDATA.NODESrecordIDs[seq + 1];
          pFrom = &MIFLINES[nJ].from;
          pTo = &MIFLINES[nJ].to;
//
//  Who's a stop?
//
          bFromIsAStop = pFrom->flags & PATTERNS_FLAG_BUSSTOP;
          bToIsAStop = pTo->flags & PATTERNS_FLAG_BUSSTOP;
//
//  Determine abbreviated names for nodes/stops
//
//  From Node
//
          if(pFrom->NODESrecordID == NO_RECORD)
          {
            pFrom->NODESrecordID = 0;
            strcpy(szFromNode, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szFromNode, NODES_ABBRNAME_LENGTH);
          }
//
//  To Node
//
          if(pTo->NODESrecordID == NO_RECORD)
          {
            pTo->NODESrecordID = 0;
            strcpy(szToNode, "");
          }
          else
          {
            NODESKey0.recordID = pTo->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szToNode, NODES_ABBRNAME_LENGTH);
          }
//
//  From stop
//
          if(pFrom->associatedStopNODESrecordID == NO_RECORD)
          {
            pFrom->associatedStopNODESrecordID = 0;
            strcpy(szFromStop, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szFromStop, NODES_LONGNAME_LENGTH);
          }
//
//  To stop
//
          if(pTo->associatedStopNODESrecordID == NO_RECORD)
          {
            pTo->associatedStopNODESrecordID = 0;
            strcpy(szToStop, "");
          }
          else
          {
            NODESKey0.recordID = pTo->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szToStop, NODES_LONGNAME_LENGTH);
          }
//
//  Ressurect the outputString
//
          strcpy(outputString, outputStringSave);
//
//  Pattern name
//
          strcat(outputString, szPatternName);
          strcat(outputString, ",");
//
//  Line number
//
          sprintf(tempString, "%d,", outputSeq++);
          strcat(outputString, tempString);
//
//  Total output lines
//
          sprintf(tempString, "%d,", totalOutputLines);
          strcat(outputString, tempString);
//
//  Determine output precidence
//
          TPBS = NO_RECORD;
          strcpy(tempString, ",,");
          if(!bFromIsAStop)  // first not a stop second a stop
          {
            if(bToIsAStop)
            {
              if(pFrom->associatedStopNODESrecordID != 0)
              {
                TPBS = VTPFILE_TPBS_NODESTOPTOSTOP;  // case 1
              }
              else
              {
                TPBS = VTPFILE_TPBS_NODETOSTOP;  // case 2
              }
              sprintf(tempString, "%s,,", szFromNode);
            }
            else  // first not a stop second not a stop
            {
              if(pFrom->associatedStopNODESrecordID != 0)
              {
                if(pTo->associatedStopNODESrecordID != 0)
                {
                  TPBS = VTPFILE_TPBS_NODESTOPTONODESTOP;  // case 8
                }
                else
                {
                  TPBS = VTPFILE_TPBS_NODESTOPTONODE;  // case 9
                }
              }
              else
              {
                if(pTo->associatedStopNODESrecordID != 0)
                {
                  TPBS = VTPFILE_TPBS_NODETONODESTOP;  // case 4
                }
                else
                {
                  TPBS = VTPFILE_TPBS_NODETONODE;  // case 3
                }
              }
            }
            sprintf(tempString, "%s,%s,", szFromNode, szToNode);
          }
          else  // first a stop
          {
            if(bToIsAStop)
            {
              TPBS = VTPFILE_TPBS_STOPTOSTOP;  // case 5
            }
            else
            {
              if(pTo->associatedStopNODESrecordID != 0)
              {
                TPBS = VTPFILE_TPBS_STOPTONODESTOP;  // case 7
              }
              else
              {
                TPBS = VTPFILE_TPBS_STOPTONODE;  // case 6
              }
              sprintf(tempString, ",%s,", szToNode); 
            }
          }
//
//  Add the string
//
          strcat(outputString, tempString);
//
//  From longitude and latitude
//
          sprintf(szarString, "%12.6f,", pFrom->longitude);
          ptr = szarString;
          while(ptr && *ptr == ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
          sprintf(szarString, "%12.6f,", pFrom->latitude);
          ptr = szarString;
          while(ptr && *ptr == ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
//
//  To longitude and latitude
//
          sprintf(szarString, "%12.6f,", pTo->longitude);
          ptr = szarString;
          while(ptr && *ptr == ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
          sprintf(szarString, "%12.6f,", pTo->latitude);
          ptr = szarString;
          while(ptr && *ptr == ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
//
//  Switch on the node/stop configuration
//
          bWriteRecord = TRUE;
//
//  In all cases, the following holds true:
//    fromStopName is the szFromStop if it's a stop or associated stop, and blank otherwise
//      toStopName is the szToStop   if it's a stop or associated stop, and blank otherwise
//
//  These two values precede the fromNode/toNode/fromStop/toStop below
//
          switch(TPBS)
          {
//
//  case 1 - Node (with associated stop) to stop
//  Gets: fromNode = Node, toNode = 0, fromStop = Associated Stop, toStop = stop
//
            case VTPFILE_TPBS_NODESTOPTOSTOP:
              sprintf(tempString, "%s,%s,%ld,0,%ld,%ld,", szFromStop, szToStop,
//                    pFrom->NODESrecordID, pFrom->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->NODESrecordID, pFrom->associatedStopNumber, pTo->associatedStopNumber);
              break;
//
//  case 2 - Node to a stop
//  Gets: fromNode = Node, toNode = 0, fromStop = 0, toStop = Stop
//  
            case VTPFILE_TPBS_NODETOSTOP:
              sprintf(tempString, ",%s,%ld,0,0,%ld,", szToStop,
//                    pFrom->NODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->NODESrecordID, pTo->associatedStopNumber);
              break;
//
//  case 3 - Node to a node
//  Gets: fromNode = Node, toNode = Node, fromStop = 0, toStop = 0
//
            case VTPFILE_TPBS_NODETONODE:
              sprintf(tempString, ",,%ld,%ld,0,0,",
                    pFrom->NODESrecordID, pTo->NODESrecordID);
              break;
//
//  case 4 - Node to a node (with associated stop)
//  Gets: fromNode = Node, toNode = Node, fromStop = 0, toStop = Associated Stop
//
            case VTPFILE_TPBS_NODETONODESTOP:
              sprintf(tempString, ",%s,%ld,%ld,0,%ld,", szToStop,
//                    pFrom->NODESrecordID, pTo->NODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->NODESrecordID, pTo->NODESrecordID, pTo->associatedStopNumber);
              break;
//
//  case 5 - Stop to a stop
//  Gets: fromNode = 0, toNode = 0, fromStop = Stop, toStop = Stop
//
            case VTPFILE_TPBS_STOPTOSTOP:
              sprintf(tempString, "%s,%s,0,0,%ld,%ld,", szFromStop, szToStop,
//                    pFrom->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->associatedStopNumber, pTo->associatedStopNumber);
              break;
//
//  case 6 - Stop to a node
//  Gets: fromNode = 0, toNode = Node, fromStop = Stop, toStop = 0
//
            case VTPFILE_TPBS_STOPTONODE:
              sprintf(tempString, "%s,,0,%ld,%ld,0,", szFromStop,
//                    pFrom->associatedStopNODESrecordID, pTo->NODESrecordID);
                    pFrom->associatedStopNumber, pTo->NODESrecordID);
              break;
//
//  case 7 - Stop to a node (with associated stop)
//  Gets: fromNode = 0, toNode = Node, fromStop = Stop, toStop = Associated stop
//
            case VTPFILE_TPBS_STOPTONODESTOP:
              sprintf(tempString, "%s,%s,0,%ld,%ld,%ld,", szFromStop, szToStop,
//                    pTo->NODESrecordID, pFrom->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
                    pTo->NODESrecordID, pFrom->associatedStopNumber, pTo->associatedStopNumber);
              break;
//
//  case 8 - Node (with associated stop) to node (with associated stop)
//  Gets: fromNode = Node, toNode = Node, fromStop = Associated stop, toStop = Associated stop
//
            case VTPFILE_TPBS_NODESTOPTONODESTOP:
              sprintf(tempString, "%s,%s,%ld,%ld,%ld,%ld,", szFromStop, szToStop,
//                    pFrom->NODESrecordID, pTo->NODESrecordID, pFrom->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->NODESrecordID, pTo->NODESrecordID, pFrom->associatedStopNumber, pTo->associatedStopNumber);
              break;
//
//  case 9 - Node (with associated stop) to node
//  Gets: fromNode = Node, toNode = 0, fromStop = Associated stop, toStop = Stop
//
            case VTPFILE_TPBS_NODESTOPTONODE:
              sprintf(tempString, "%s,,%ld,0,%ld,%ld,", szFromStop,
//                    pFrom->NODESrecordID, pFrom->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
                    pFrom->NODESrecordID, pFrom->associatedStopNumber, pTo->associatedStopNumber);
              break;
//
//  Bad one - write a record to the error file
//
            default:
              strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
              trim(szarString, ROUTES_NUMBER_LENGTH);
              sprintf(tempString, "*ERR - Bad case in Virutal timepoint file.  Route: %s, from: %ld to %ld\r\n",
                    szarString, pFrom->NODESrecordID, pTo->NODESrecordID);
              _lwrite(hfErrorLog, tempString, strlen(tempString));
              bWriteRecord = FALSE;
              bGotError = TRUE;
              break;
          }  // switch
//
//  Add to the output string
//
          strcat(outputString, tempString);
//
//  First stop, secondary route (always 0)
// 
          strcat(outputString, "0,");
//
//  Fare code
//
          sprintf(tempString, "%d,", (fareCode == previousFareCode ? 0 : fareCode));
          strcat(outputString, tempString);
          previousFareCode = fareCode;
//
//  Cut point (always 0)
//  
          strcat(outputString, "0\r\n");
//
//  Write out the record
//
          if(bWriteRecord)
          {
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  If we've encountered a mid-trip layover, it gets a special record.
//
//  From Orbital:
//    "To declare a mid-trip layover, the pattern in the VTP File must contain exactly one record where the
//    'from timepoint name' and 'to timepoint name' fields are set to exactly the same non-null value.
//    In the example case, they should both be set to 'LTCA'.  It wouldn't work to set one to 'LTCA' and
//    the other to 'LTCD' because our software would interpret this as two distinct timepoints.  The following
//    field pairs must also match:  'from timepoint id' = 'to timepoint id', 'from stop name' = 'to stop name',
//    'from stop id' = 'to stop id', 'from longitude' = 'to longitude', 'from latitude' = 'to latitude'.
//
          if(bWriteRecord && (MIFLINES[nJ].flags & MIFLINES_FLAG_NEXTISSAMELOCATION))
          {
//
//  The string to this point
//
            strcpy(outputString, outputStringSave);
//
//  Pattern name
//
            strcat(outputString, szPatternName);
            strcat(outputString, ",");
//
//  Sequence number
//
            sprintf(tempString, "%d,", outputSeq++);
            strcat(outputString, tempString);
//
//  Total output lines
//
            sprintf(tempString, "%d,", totalOutputLines);
            strcat(outputString, tempString);
//
//  From and to timepoint names
//
            sprintf(tempString, "%s,%s,", szToNode, szToNode);
            strcat(outputString, tempString);
//
//  Longitude and latitude
//
            sprintf(szarString, "%12.6f,", pTo->longitude);
            ptr = szarString;
            while(ptr && *ptr == ' ')
            {
              ptr++;
            }
            strcpy(tempString, ptr);
            sprintf(szarString, "%12.6f,", pTo->latitude);
            ptr = szarString;
            while(ptr && *ptr == ' ')
            {
              ptr++;
            }
            strcat(tempString, ptr);
//
//  (repeated)
//
            strcat(outputString, tempString);
            strcat(outputString, tempString);
//
//  Stop / NODESrecordID / associatedStopNODESrecordID
//
//            sprintf(tempString, "%s,%s,%ld,%ld,%ld,%ld,", szToStop, szToStop,
//                  pTo->NODESrecordID, pTo->NODESrecordID, pTo->associatedStopNODESrecordID, pTo->associatedStopNODESrecordID);
            nK = atoi(&szToStop[4]);
            sprintf(tempString, "%s,%s,%ld,%ld,%d,%d,", szToStop, szToStop,
                  pTo->NODESrecordID, pTo->NODESrecordID, nK, nK);
            strcat(outputString, tempString);
//
//  First stop, secondary route (always 0)
// 
            strcat(outputString, "0,");
//
//  Fare code
//
            sprintf(tempString, "%d,", (fareCode == previousFareCode ? 0 : fareCode));
            strcat(outputString, tempString);
            previousFareCode = fareCode;
//
//  Cut point (always 0)
//  
            strcat(outputString, "0\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }  // Mid-trip layover
        }  // nJ cycle through
//
//  Get the next pattern
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = 1;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMESrecordID + 1;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }  // while rcode2 on patterns
    }  // nI on directions
//
//  Get the next route and syscle back
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  // while rcode2 on routes
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ================================
//  Bus Stop File - bus<bbbbbbb>.dat
//  ================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "bus");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Cycle through the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
//
//  Omit the "real timpoints" (RTPs) from this file: Bob Antonisse, 2-Aug-07
//
    if(NODES.flags & NODES_FLAG_STOP)
    {
//
//  Record type
//
      strcpy(outputString, "90,");
//
//  Bus stop ID (NODES.recordID)
//
//      sprintf(tempString, "%ld,", NODES.recordID);
//
//  Change to stop number (as derived from long name) : Bob Antonisse, 2-Aug-07
//
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      tempLong = atol(&tempString[4]);
      sprintf(tempString, "%ld,", tempLong);
      strcat(outputString, tempString);
//
//  Bus stop short name (NODES.longName)
//
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Bus stop long name
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      PipeCommas(tempString);
      while(ptr = strchr(tempString, '"'))
      {
        strcpy(ptr, ptr + 1);
      }
      tempString[BUSFILE_DESCRIPTION_LENGTH] = '\0';
      strcat(outputString, tempString);
      strcat(outputString, ",");
//
//  Longitude and latitude
//
      if(!m_bUseLatLong)
      {
        EN2LL(23, (double)NODES.longitude, (double)NODES.latitude, szUTMZone, &Lat, &Long);
        NODES.latitude = (float)Lat;
        NODES.longitude = (float)Long;
      }
//
//  Longitude
//
      sprintf(szarString, "%12.6f,", NODES.longitude);
      ptr = szarString;
      while(ptr && *ptr == ' ')
      {
        ptr++;
      }
      strcat(outputString, ptr);
//
//  Latitude
//
      sprintf(szarString, "%12.6f\r\n", NODES.latitude);
      ptr = szarString;
      while(ptr && *ptr == ' ')
      {
        ptr++;
      }
      strcat(outputString, ptr);
//
//  Write it out and get the next node
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===========================================
//  Service Type File - Service.dat
//  ===========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Service.dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Cycle through the Services Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
//
//  Record type
//
    strcpy(outputString, "1,");
//
//  Service number
//
    sprintf(tempString, "%ld,", SERVICES.number);
    strcat(outputString, tempString);
//
//  Service description
//
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    tempString[SERFILE_DESCRIPTION_LENGTH] = '\0';
    strcat(outputString, tempString);
    strcat(outputString, "\r\n");
//
//  Write out the record and get the next service
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =========================================
//  Destination Sign File - sign<bbbbbbb>.dat
//  =========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "sign");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Cycle through the signcodes file
//
  rcode2 = btrieve(B_GETFIRST, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
  while(rcode2 == 0)
  {
//
//  Record type - 31
//
    strcpy(outputString, "31,");
//
//  Sign code ID
//
    sprintf(tempString, "%ld,", SIGNCODES.recordID);
    strcat(outputString, tempString);
//
//  Sign code symbol
//
    strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
    trim(tempString, SIGNCODES_CODE_LENGTH);
    tempString[SIGNFILE_SYMBOL_LENGTH] = '\0';
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Sign code abbr (same as symbol)
//
    strcat(outputString, tempString);
    strcat(outputString, ",");
//
//  Sign code text (Orbital's limit is 2000.  Ours is considerably less.)
//
    strncpy(tempString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
    trim(tempString, SIGNCODES_TEXT_LENGTH);
    while(ptr = strchr(tempString, '"'))
    {
      strcpy(ptr, ptr + 1);
    }
    strcat(outputString, tempString);
    strcat(outputString, "\r\n");
//
//  Write it out
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next comment record
//
    rcode2 = btrieve(B_GETNEXT, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===========================================
//  Special Trip Option File - opt<bbbbbbb>.dat
//  ===========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "opt");
  strcat(outputFileName, szBookingName);
  strcat(outputFileName, ".dat");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  There are no special trip options in TMS so drop out a dummy record
//
//  Record type
//
  strcpy(outputString, "26,");
//
//  Booking ID
//
  strcat(outputString, szBookingID);
  strcat(outputString, ",");
//
//  Balance of the dummy record
//
  strcat(outputString, "1,SD,School Day,0\r\n");
//
//  Write it out
//
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  done:
    chdir("..");  // Back to where we started from
    StatusBarEnd();
  
  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  if(bGotError)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_350, (HANDLE)NULL);
  }
  _lclose(hfErrorLog);
  distanceMeasure = distanceMeasureSave;
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  return(bFinishedOK);
}
