// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

#define TMSRPT53_MAXSERVICES 25

#define OUTPUT_FOLDER_NAME "init Interface Files"

 
char *ConvertCTranRunNumber(long);          // Source is in tmsrpt12.c
int  sort_runs(const void *, const void *);  // Source is in tmsrpt10.c
long GetStopToStopDistance(long, long, long);
int  distanceMeasureSave;

typedef struct HPRDStruct
{
  long runNumber;
  long pieceNumber;
  long blockNumber;
  long startNODESrecordID;
  long startTRIPSrecordID;
  long startTime;
  long endNODESrecordID;
  long endTRIPSrecordID;
  long endTime;
} HPRDDef;

typedef struct STOPNODESStruct
{
  long recordID;
  long stopNumber[2];
} STOPNODESDef;

typedef struct LINEStruct
{
  long recordID;
  long number;
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
} LINEDef;

#define POINT_FLAG_STOP    0x0001
#define POINT_FLAG_WRITTEN 0x0002
typedef struct POINTStruct
{
  long  flags;
  long  recordID;
  float longitude;
  float latitude;
} POINTDef;

typedef struct INITBLOCKSStruct
{
  long  ROUTESrecordID;
  long  SERVICESrecordID;
  long  blockNumber;
} INITBLOCKSDef;

#define PATCODE_FLAG_STD  0x0000
#define PATCODE_FLAG_PO   0x0001
#define PATCODE_FLAG_PI   0x0002
#define PATCODE_FLAG_ILDH 0x0004

typedef struct PATCODESStruct
{
  long  flags;
  long  recordID;
  long  ROUTESrecordID;
  long  SERVICESrecordID;
  long  directionIndex;
  long  flaggedNODESrecordIDs[2];
} PATCODESDef;
//
//  TMS Unload to the Init AVL System
//
BOOL FAR TMSRPT53(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  HFILE hfOutputFile[3];
  HFILE hfErrorLog;
  float fromLongitude;
  float fromLatitude;
  float toLongitude;
  float toLatitude;
  float distance;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bGotNextTrip;
  BOOL  bServiceFromRoster;
  long  SERVICESinEffect[TMSRPT53_MAXSERVICES];
  char  outputString[1024];
  char  dummy[256];
  char *ptr;
  char *pszReportName;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   maxNodes;
  int   maxTrips;

  struct tm tmED, tmUD;
  time_t EDmktime;
  time_t UDmktime;
  int   numSTOPNODES;
  long  year, month, day;
  long  currentRecordID;
  long  stopNumber;
  long  PATTERNNAMESrecordID;
  long  PATTERNSrecordID;
  long  meters;
  long  previousTime;
  long  nodeTime;
  long  EQTT;
  long  blockNumber;
  long  fromPATTERNNAMESrecordID;
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  fromROUTESrecordID;
  long  fromSERVICESrecordID;
  long  fromDirectionIndex;
  long  deadheadTime;
  long  POTime;
  long  PITime;
  long  TRIPSrecordID;
  long  fromTime;
  long  lastTripNumber;
  long  previousNODESrecordID;
  long  tempLong;
  long  date;
  long  lastDAILYOPSrecordID;
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char  nodeAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char  previousNodeAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  szOrganizationalUnits[COMMENTS_CODE_LENGTH + 1];
  char  szFromToNames[PATTERNS_FROMTEXT_LENGTH + 4 + PATTERNS_TOTEXT_LENGTH + 1];
  int   stopPosition;
  int   maxRoutes;
  int   maxServices;
  int   maxPatternNodes;
  int   PKCode;
  int   PKOrder;
  int   lineCode;
  int   fromPointCode;
  int   toPointCode;
  int   lastPointCode;
  int   pointCode;
  int   dayOfWeek;
  int   numPATCODES;
  int   numINITBLOCKS;
  int   numNodesInFile;
  int   timeDemandType;
  int   numNodes;
  int   patternCode;
  int   blockCode;
  int   numServices;
  int   firstDeadPattern;
  int   lastDeadPattern;
  int   pointCodeType;
  int   extendedTypeSpecification;

  LINEDef *pLINE = NULL;
  POINTDef *pPOINT = NULL;
  STOPNODESDef *pSTOPNODES = NULL;
  INITBLOCKSDef *pINITBLOCKS = NULL;
  PATCODESDef *pPATCODES = NULL;

  bFinishedOK = FALSE;
  distanceMeasureSave = distanceMeasure;
  distanceMeasure = PREFERENCES_KILOMETERS;
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
  pPassedData->nReportNumber = 52;
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
  if(maxServices > TMSRPT53_MAXSERVICES)
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
  maxNodes += 50;
//
//  maxPatternNodes
//
  rcode2 = btrieve(B_STAT, TMS_PATTERNS, &BSTAT, dummy, 0);
  maxPatternNodes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
// maxTrips
//
  rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
  maxTrips = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Establish the last trip number in the system
//
  rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  lastTripNumber = NO_RECORD;
  while(rcode2 == 0)
  {
    if(TRIPS.tripNumber > lastTripNumber)
    {
      lastTripNumber = TRIPS.tripNumber;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  }
  lastTripNumber++;
//
//  Version.csv
//
//  Variable  Type
//  ~~~~~~~~  ~~~~
//  Name      char[6]
//  Begin     date (dd.mm.yyyy)
//  End       date (dd.mm.yyyy)
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(tempString, "Version.csv");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(tempString);
//
//  Write out the comment string
//
  strcpy(outputString, "#Name;Begin;End\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Name
//
  strcpy(outputString, szShortDatabaseDescription);
  strcat(outputString, ";");
//
//  Effective date
//
  strcpy(szFormatString, "%02ld.%02ld.%4ld");
  GetYMD(effectiveDate, &year, &month, &day);
  sprintf(tempString, szFormatString, day, month, year);
  strcat(outputString, tempString);
  strcat(outputString, ";");
//
//  Until date
//
  GetYMD(untilDate, &year, &month, &day);
  sprintf(tempString, szFormatString, day, month, year);
  strcat(outputString, tempString);
  strcat(outputString, "\r\n");
//
//  Write it out
//
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Close the file
//
  _lclose(hfOutputFile[0]);
//
//  Calendar.csv
//
//  Variable  Type
//  ~~~~~~~~  ~~~~
//  Day       PK.1 (1,...,31)
//  Month     PK.2 (1,...,12)
//  Year      PK.3 (2000,...,2010)
//  Daytype   code (Service - char[SERVICES_NAME_LENGTH])
//
  numServices = 0;
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(tempString, "Calendar.csv");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(tempString);
//
//  Write out the comment string
//
  strcpy(outputString, "#Day;Month;Year;ServiceName\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
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
//
//  Look to see if there's been a service/date override in DailyOps
//
      date = (tmED.tm_year + 1900) * 10000 + (tmED.tm_mon + 1) * 100 + tmED.tm_mday; 
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      lastDAILYOPSrecordID = (rcode2 == 0 ? DAILYOPS.recordID : 0);
      DAILYOPSKey1.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
      DAILYOPSKey1.pertainsToDate = date;
      DAILYOPSKey1.pertainsToTime = lastDAILYOPSrecordID + 1;
      DAILYOPSKey1.recordFlags = 0;
      bServiceFromRoster = TRUE;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_DATE) &&
             DAILYOPS.pertainsToDate == date)
      {
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_DATESET)
        {
          bServiceFromRoster = ANegatedRecord(DAILYOPS.recordID, 1);
          if(!bServiceFromRoster)
          {
            SERVICESKey0.recordID = DAILYOPS.DOPS.Date.SERVICESrecordID;
            break;
          }
        }
        rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
//
//  ...or get it from the roster setup
//
      if(bServiceFromRoster)
      {
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
        SERVICESKey0.recordID = ROSTERPARMS.serviceDays[dayOfWeek];
      }
//
//  Get the SERVICES recordID
//
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(tempString, SERVICES_NAME_LENGTH);
      sprintf(outputString, "%ld;%ld;%ld;%s\r\n", tmED.tm_mday, tmED.tm_mon + 1, tmED.tm_year + 1900, tempString);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
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
  _lclose(hfOutputFile[0]);
//
//  Line.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Line code  code (PK)
//  Line #     int(3) (route numbers can't contain alphas)
//  Class.Unit not used - blank
//  Name       char[40]
//
  strcpy(tempString, "Line.csv");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(tempString);
//
//  Allocate space for the structure
//
  pLINE = (LINEDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LINEDef) * maxRoutes); 
  if(pLINE == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Write out the comment string
//
  strcpy(outputString, "#LineCode;LineNumber;Unused;LineName\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Traverse the Routes Table
//
  PKCode = 1;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Save to the data structure
//
    if(PKCode - 1 >= maxRoutes)
    {
      maxRoutes += 100;
      pLINE = (LINEDef *)HeapReAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, pLINE, sizeof(LINEDef) * maxRoutes); 
      if(pLINE == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        goto done;
      }
    }
    pLINE[PKCode - 1].recordID = ROUTES.recordID;
    if(ROUTES.alternate > 0)
    {
      sprintf(tempString, "%ld", ROUTES.alternate);
      pLINE[PKCode - 1].number = ROUTES.alternate;
    }
    else
    {
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      pLINE[PKCode - 1].number = atol(tempString);
    }
    strcpy(pLINE[PKCode - 1].routeNumber, tempString);
//
//  Build the output string
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    strcat(tempString, " - ");
    strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(szarString, ROUTES_NAME_LENGTH);
    strcat(tempString, szarString);
    sprintf(outputString, "%d;%ld;;%s\r\n", PKCode, pLINE[PKCode - 1].number, tempString);
//
//  Write it out and get the next record
//
    _lwrite(hfOutputFile[0], outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    PKCode++;
  }
//
//  Close the file
//
  _lclose(hfOutputFile[0]);
//
//  Block.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Block code code (PK)
//  Class.Unit not used - blank
//  Number     int(5)
//  Service    char[SERVICES_NAME_LENGTH]
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(tempString, "Block.csv");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(tempString);
//
//  Allocate space for the structure
//
  pINITBLOCKS = (INITBLOCKSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(INITBLOCKSDef) * maxTrips); 
  if(pINITBLOCKS == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Write out the comment string
//
  strcpy(outputString, "#BlockCode;Unused;BlockNumber;ServiceName\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Cycle through the services
//
  PKCode = 1;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Go through the blocks on this route
//
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
      TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
      TRIPSKey2.blockNumber = NO_RECORD;
      TRIPSKey2.blockSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      while(rcode2 == 0 &&
            TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
            TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
      {
//
//  Save it
//
        if(PKCode - 1 >= maxTrips)
        {
          maxTrips += 100;
          pINITBLOCKS = (INITBLOCKSDef *)HeapReAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, pINITBLOCKS, sizeof(INITBLOCKSDef) * maxTrips); 
          if(pINITBLOCKS == NULL)
          {
            AllocationError(__FILE__, __LINE__, FALSE);
            goto done;
          }
        }
        pINITBLOCKS[PKCode - 1].blockNumber = TRIPS.standard.blockNumber;
        pINITBLOCKS[PKCode - 1].ROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        pINITBLOCKS[PKCode - 1].SERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
//
//  Create the string
//
        sprintf(outputString, "%d;;%ld;%s\r\n", PKCode, TRIPS.standard.blockNumber, serviceName);
//
//  Write it out
//
        _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Get the next block
//
        TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
        TRIPSKey2.blockNumber = TRIPS.standard.blockNumber + 1;
        TRIPSKey2.blockSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        PKCode++;
      }
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Loop back for the next service
//
  }
//
//  Close the file
//
  numINITBLOCKS = PKCode - 1;
  _lclose(hfOutputFile[0]);
//
//  Point.csv
//
//  Variable  Type
//  ~~~~~~~~  ~~~~
//  Point#    code (PK)
//  Stop#     int(3) - Use RecordID
//  Stop pos  1
//  District# Not used
//  Stop name char[6] - Use abbrName
//  Stop name char[41] - Use Description
//  Long      double f10.5
//  Lat       double f10.5
//  Pre-emp#  Not used
//  Announ#   Not used
//  Dest#     Not used
//
//  and
//
//  Pointtariff.csv
//
//  Variable  Type
//  ~~~~~~~~  ~~~~
//  Point#    code (PK)
//  Tariff 1  int(3) - Use Tariff sort number
//
//  and
//
//  PIDmobil.csv
//
//  Variable  Type
//  ~~~~~~~~  ~~~~
//  Point#    code
//  Stop name char[6]  - Use abbrname
//  AVL Stop  char[24]
//
  if(StatusBarAbort())
  {
    goto done;
  }
//
//  hfOutputFile[0] is Point.csv
//
  strcpy(tempString, "Point.csv");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(tempString);
//
//  Write out the comment string
//
  strcpy(outputString, "#PointNumber;StopNumber;StopPosition;Unused;ShortStopName;LongStopName;Longitude;Latitude;Unused;Unused;Unused\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  hfOutputFile[1] is Pointtariff.csv
//
  strcpy(tempString, "Pointtariff.csv");
  hfOutputFile[1] = _lcreat(tempString, 0);
  if(hfOutputFile[1] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    _lclose(hfOutputFile[0]);
    goto done;
  }
//
//  Write out the comment string
//
  strcpy(outputString, "#PointNumber;Tariff1;Unused;Unused;Unused\r\n");
  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
//
//  hfOutputFile[2] is PIDmobil.csv
//
  strcpy(tempString, "PIDmobil.csv");
  hfOutputFile[2] = _lcreat(tempString, 0);
  if(hfOutputFile[2] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    _lclose(hfOutputFile[0]);
    _lclose(hfOutputFile[1]);
    goto done;
  }
//
//  Write out the comment string
//
  strcpy(outputString, "#PointNumber;ShortStopName;AVLStopName;\r\n");
  _lwrite(hfOutputFile[2], outputString, strlen(outputString));
//
//  Cycle through the Nodes Table
//
//  Save the list of nodes that have stops associated with them
//
  pSTOPNODES = (STOPNODESDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(STOPNODESDef) * maxNodes); 
  if(pSTOPNODES == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  numSTOPNODES = 0;
  while(rcode2 == 0)
  {
//
//  Ctran
//
/*
    if(!(NODES.flags & NODES_FLAG_STOP) && NODES.number != 0)
*/
//
//  ECan
//
    if(!(NODES.flags & NODES_FLAG_STOP) && (NODES.OBStopNumber != 0 || NODES.IBStopNumber != 0))
    {
      if(numSTOPNODES >= maxNodes)
      {
        maxNodes += 100;
        pSTOPNODES = (STOPNODESDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, pSTOPNODES, sizeof(STOPNODESDef) * maxNodes); 
        if(pSTOPNODES == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
          goto done;
        }
      }
      pSTOPNODES[numSTOPNODES].recordID = NODES.recordID;
//
//  Ctran
//
/*
      pSTOPNODES[numSTOPNODES].stopNumber[0] = NODES.number / 10000;
      pSTOPNODES[numSTOPNODES].stopNumber[1] = NODES.number - (pSTOPNODES[numSTOPNODES].stopNumber[0] * 10000);
*/
//
//  ECan
//
      pSTOPNODES[numSTOPNODES].stopNumber[0] = NODES.OBStopNumber;
      pSTOPNODES[numSTOPNODES].stopNumber[1] = NODES.IBStopNumber;
      if(pSTOPNODES[numSTOPNODES].stopNumber[0] == pSTOPNODES[numSTOPNODES].stopNumber[1])
      {
        pSTOPNODES[numSTOPNODES].stopNumber[1] = NO_RECORD;
      }
      numSTOPNODES++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Allocate space for the point (nodes/stops) structure
//
  pPOINT = (POINTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(POINTDef) * maxNodes); 
  if(pPOINT == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Register the garages
//
  PKCode = 1;
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_GARAGE)
    {
      pPOINT[PKCode - 1].recordID = NODES.recordID;
//
//  Build the output string
//
//  Point number, Stop number
//
      sprintf(outputString, "%d;%ld;1;;", PKCode, NODES.recordID);
//
//  Stop name short
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Stop name long
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Longitude
//
      pPOINT[PKCode - 1].longitude = NODES.longitude;
      sprintf(tempString, "%12.6f", NODES.longitude);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Latitude
//
      pPOINT[PKCode - 1].latitude = NODES.latitude;
      sprintf(tempString, "%12.6f", NODES.latitude);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  The balance of the record is unused
//
      strcat(outputString, ";;\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      PKCode++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Go through the stops
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  numNodesInFile = maxNodes;
  for(nI = 0; nI < numNodesInFile; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    if(rcode2 != 0)
    {
      break;
    }
    currentRecordID = NODES.recordID;
    StatusBar((long)(nI + 1), (long)(maxNodes));
//
//  Just do stops
//
    if(NODES.flags & NODES_FLAG_STOP)
    {
//
//  Do the pointtariff entry
//
      JURISDICTIONSKey0.recordID = NODES.JURISDICTIONSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      tempLong = atol(tempString);
      if(rcode2 == 0)
      {
        sprintf(outputString, "%d;%ld;;;\r\n", PKCode, JURISDICTIONS.number);
      }
      else
      {
        sprintf(outputString, "%d;0;;;\r\n", PKCode);
      }
//
//  Write it out
//
      _lwrite(hfOutputFile[1], outputString, strlen(outputString));
//
//  Do the PIDmobil entry
//
      strncpy(szarString, NODES.AVLStopName, NODES_AVLSTOPNAME_LENGTH);
      trim(szarString, NODES_AVLSTOPNAME_LENGTH);
      sprintf(outputString, "%d;%s;%s\r\n", PKCode, tempString, szarString);
//
//  Write it out
//
      _lwrite(hfOutputFile[2], outputString, strlen(outputString));
//
//  Save to the data structure
//
      if(PKCode - 1 >= maxNodes)
      {
        maxNodes += 100;
        pPOINT = (POINTDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, pPOINT, sizeof(POINTDef) * maxNodes); 
        if(pPOINT == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
          goto done;
        }
      }
      pPOINT[PKCode - 1].recordID = NODES.recordID;
//
//  Build the output string
//
//  Point number, Stop number
//
      pPOINT[PKCode - 1].flags = POINT_FLAG_STOP;
      sprintf(outputString, "%d;%ld;1;;", PKCode, NODES.recordID);
//
//  Stop name short
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Stop name long
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Longitude
//
      pPOINT[PKCode - 1].longitude = NODES.longitude;
      sprintf(tempString, "%12.6f", NODES.longitude);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  Latitude
//
      pPOINT[PKCode - 1].latitude = NODES.latitude;
      sprintf(tempString, "%12.6f", NODES.latitude);
      strcat(outputString, tempString);
      strcat(outputString, ";");
//
//  The balance of the record is unused
//
      strcat(outputString, ";;\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Move along
//
      PKCode++;
//
//  Now check to see where this stop is recorded on a timepoint level
//
//  Ctran
//
/*
      stopNumber = NODES.number;
*/
//
//  ECan
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      stopNumber = atol(tempString);
      stopPosition = 2;
      for(nJ = 0; nJ < numSTOPNODES; nJ++)
      {
        for(nK = 0; nK < 2; nK++)
        {
          if(pSTOPNODES[nJ].stopNumber[nK] == stopNumber)
          {
            NODESKey0.recordID = pSTOPNODES[nJ].recordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Save to the data structure
//
            if(PKCode - 1 >= maxNodes)
            {
              maxNodes += 100;
              pPOINT = (POINTDef *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, pPOINT, sizeof(POINTDef) * maxNodes); 
              if(pPOINT == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                goto done;
              }
            }
            pPOINT[PKCode - 1].recordID = NODES.recordID;
//
//  Build the output string
//
            sprintf(outputString, "%d;%ld;%d;;", PKCode, currentRecordID, stopPosition++);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, ";");
            strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
            trim(tempString, NODES_DESCRIPTION_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, ";");
            pPOINT[PKCode - 1].longitude = NODES.longitude;
            sprintf(tempString, "%12.6f", NODES.longitude);
            strcat(outputString, tempString);
            strcat(outputString, ";");
            pPOINT[PKCode - 1].latitude = NODES.latitude;
            sprintf(tempString, "%12.6f", NODES.latitude);
            strcat(outputString, tempString);
            strcat(outputString, ";");
            strcat(outputString, ";;\r\n");
            _lwrite(hfOutputFile[0], outputString, strlen(outputString));
            PKCode++;
          }
        }
      }
    }
//
//  Re-establish position
//
    NODESKey0.recordID = currentRecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Get the next node
//
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the files
//
  _lclose(hfOutputFile[0]);
  _lclose(hfOutputFile[1]);
  _lclose(hfOutputFile[2]);
//
//  Pattern.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Line#      code (FK)
//  Pattern    code (PK.1)
//  Pat Type   int (1=Service, 2=Dead run)
//  Order#     code (PK.2)
//  Point#     int(8) (FK)
//  Distance   int (to predecessor, in meters - 0 for first point)
//  Point type int (21=Stop, 1=Timepoint, 2=Depot, 6=District Border Point)
//  Direction  code
//  Announ#    not used - blank
//  Dest#      Last point on the pattern
//
//  and...
//
//  Dest.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Dest#      Last point on the pattern
//  Short Text char[16]
//  Long Text  char[24]
//
//  and...
//
//  Pattern_Property.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Pattern    code (PK.1)
//  Unused
//  Unused
//  Name       char
//
  if(StatusBarAbort())
  {
    goto done;
  }
//
//  Pattern.csv
//
  hfOutputFile[0] = _lcreat("Pattern.csv", 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
//
//  Dest.csv
//
  hfOutputFile[1] = _lcreat("Dest.csv", 0);
  if(hfOutputFile[1] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    _lclose(hfOutputFile[0]);
    goto done;
  }
  StatusBarText("Pattern.csv and Dest.csv");
//
//  Pattern_Property.csv
//
  hfOutputFile[2] = _lcreat("Pattern_Property.csv", 0);
  if(hfOutputFile[2] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    _lclose(hfOutputFile[0]);
    goto done;
  }
  StatusBarText("Pattern.csv and Dest.csv\nand Pattern_Property.csv");
//
//  Allocate space for the pattern codes structure
//
  pPATCODES = (PATCODESDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PATCODESDef) * maxPatternNodes); 
  if(pPATCODES == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Write out the comment strings
//
//  Pattern.csv
//
  strcpy(outputString, "#LineNumber;Pattern;PatternType;Order;PointNumber;Distance;PointType;Direction;Unused;DestinationNumber\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Dest.csv
//
  strcpy(outputString, "#DestinationNumber;ShortText;LongText\r\n");
  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
//
//  Pattern_Property.csv
//

//  strcpy(outputString, "#PatternCode;;;Name\r\n");
  strcpy(outputString, "#PatternCode;ExtendedTypeSpecification;ServiceTypeText;Name;unused;unused;OrganisationalUnits\r\n");
  _lwrite(hfOutputFile[2], outputString, strlen(outputString));
//
//  Cycle through the services
//
  PKCode = 1;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
      if(ROUTES.COMMENTSrecordID == NO_RECORD)
      {
        strcpy(szOrganizationalUnits, "");
      }
      else
      {
        COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        if(rcode2 != 0)
        {
          strcpy(szOrganizationalUnits, "");
        }
        else
        {
          memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
          strncpy(szOrganizationalUnits, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(szOrganizationalUnits, COMMENTS_CODE_LENGTH);
        }
      }
//
//  Find the line code
//
      lineCode = NO_RECORD;
      for(nK = 0; nK < maxRoutes; nK++)
      {
        if(pLINE[nK].recordID == ROUTES.recordID)
        {
          lineCode = nK + 1;
          break;
        }
      }
//
// It had to be there
//
      if(lineCode == NO_RECORD)
      {
        LoadString(hInst, ERROR_322, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(outputString, szFormatString, routeNumber);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
        bGotError = TRUE;
      }
//
//  Line code found - Cycle through the directions
//
      else
      {
        for(nK = 0; nK < 2; nK++)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          {
            continue;
          }
//
//  Keep the direction's abbreviated name
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(directionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Start with the BASE pattern on this route
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nK)
          {
//
//  Save the pertinent information
//
            if(PKCode - 1 >= maxPatternNodes)
            {
              maxPatternNodes += 100;
              pPATCODES = (PATCODESDef *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, pPATCODES, sizeof(PATCODESDef) * maxPatternNodes); 
              if(pPATCODES == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                goto done;
              }
            }
            pPATCODES[PKCode - 1].flags = PATCODE_FLAG_STD;
            pPATCODES[PKCode - 1].recordID = PATTERNS.PATTERNNAMESrecordID;
            pPATCODES[PKCode - 1].ROUTESrecordID = ROUTES.recordID;
            pPATCODES[PKCode - 1].SERVICESrecordID = SERVICES.recordID;
            pPATCODES[PKCode - 1].directionIndex = nK;
            pPATCODES[PKCode - 1].flaggedNODESrecordIDs[0] = NO_RECORD;
            pPATCODES[PKCode - 1].flaggedNODESrecordIDs[1] = NO_RECORD;
// 
//  Find the last point on the pattern
//
            PATTERNSrecordID = PATTERNS.recordID;
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nK;
            PATTERNSKey2.PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = LONG_MAX;
            rcode2 = btrieve(B_GETLESSTHAN, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            lastPointCode = NO_RECORD;
            if(rcode2 == 0)
            {
              for(nL = 0; nL < maxNodes; nL++)
              {
                if(pPOINT[nL].recordID == PATTERNS.NODESrecordID)
                {
                  lastPointCode = nL + 1;
                  break;
                }
              }
            }
//
//  The last point code must have been found
//
            if(lastPointCode == NO_RECORD)
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(rcode2 == 0)
              {
                strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
              }
              else
              {
                strcpy(nodeAbbr, "????");
              }
              PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
              if(rcode2 == 0)
              {
                strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                trim(patternName, PATTERNNAMES_NAME_LENGTH);
              }
              else
              {
                strcpy(patternName, "????");
              }
              LoadString(hInst, ERROR_323, szFormatString, SZFORMATSTRING_LENGTH);
              sprintf(outputString, szFormatString,
                    nodeAbbr, patternName, routeNumber, serviceName, directionAbbr);              
              _lwrite(hfErrorLog, outputString, strlen(outputString));
              bGotError = TRUE;
            }
//
//  It was - Write it out to Dest.csv if we haven't done so already
//
            else
            {
              if(!(pPOINT[lastPointCode - 1].flags & POINT_FLAG_WRITTEN))
              {
                NODESKey0.recordID = pPOINT[lastPointCode - 1].recordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                if(rcode2 != 0)
                {
                  PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                  if(rcode2 == 0)
                  {
                    strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                    trim(patternName, PATTERNNAMES_NAME_LENGTH);
                  }
                  else
                  {
                    strcpy(patternName, "????");
                  }
                  LoadString(hInst, ERROR_324, szFormatString, SZFORMATSTRING_LENGTH);
                  sprintf(outputString, szFormatString, pPOINT[lastPointCode - 1].recordID,
                        patternName, routeNumber, serviceName, directionAbbr);              
                  _lwrite(hfErrorLog, outputString, strlen(outputString));
                  bGotError = TRUE;
                }  
                else
                {
                  sprintf(outputString, "%d;", lastPointCode);
                  strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
                  trim(tempString, NODES_LONGNAME_LENGTH);
                  strcat(outputString, tempString);
                  strcat(outputString, ";");
                  strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
                  trim(tempString, NODES_DESCRIPTION_LENGTH);
                  tempString[24] = '\0';
                  strcat(outputString, tempString);
                  strcat(outputString, "\r\n");
                  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  pPOINT[lastPointCode - 1].flags |= POINT_FLAG_WRITTEN;
                }
              }
            }
//
//  Re-establish position in the Patterns Table
//
            PATTERNSKey0.recordID = PATTERNSrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
            btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  Write out the Pattern_Property record
//
            PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            if(rcode2 == 0)
            {
              strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
              trim(patternName, PATTERNNAMES_NAME_LENGTH);
            }
            else
            {
              strcpy(patternName, "????");
            }
//
//  Get the extended type is it exists
//
            extendedTypeSpecification = 0;
            if(ROUTES.COMMENTSrecordID != NO_RECORD)
            {
              COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
              recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
              rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
              recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
              if(rcode2 == 0)
              {
                memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
                strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
                trim(tempString, COMMENTS_CODE_LENGTH);
                if(strcmp(tempString, "Express") == 0)
                {
                  extendedTypeSpecification = 1;
                }
                else if(strcmp(tempString, "LTD Stop") == 0)
                {
                  extendedTypeSpecification = 2;
                }
              }
            }
//
//  Get everything from this pattern
//
            strncpy(szFromToNames, PATTERNS.fromText, PATTERNS_FROMTEXT_LENGTH);
            trim(szFromToNames, PATTERNS_FROMTEXT_LENGTH);
            if(strcmp(szFromToNames, "") != 0)
            {
              strcat(szFromToNames, " to ");
              strncpy(szarString, PATTERNS.toText, PATTERNS_TOTEXT_LENGTH);
              trim(szarString, PATTERNS_TOTEXT_LENGTH);
              strcat(szFromToNames, szarString);
            }
//            sprintf(outputString, "%d;;;%s\r\n", PKCode, patternName);
//            sprintf(outputString, "%d;%d;;%s;;;%s\r\n", PKCode, extendedTypeSpecification, patternName, szOrganizationalUnits);
            sprintf(outputString, "%d;%d;;\"%s\";;;%s\r\n", PKCode, extendedTypeSpecification, szFromToNames, szOrganizationalUnits);
            _lwrite(hfOutputFile[2], outputString, strlen(outputString));
            PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
            fromLongitude = NO_RECORD;
            PKOrder = 1;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nK &&
                  PATTERNS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
            {
// 
//  Find the node in pPOINT
//
              pointCode = NO_RECORD;
              for(nL = 0; nL < maxNodes; nL++)
              {
                if(pPOINT[nL].recordID == PATTERNS.NODESrecordID)
                {
                  pointCode = nL + 1;
                  toLongitude = pPOINT[nL].longitude;
                  toLatitude = pPOINT[nL].latitude;
                  break;
                }
              }
//  
//  Make sure we found it
//
              if(pointCode == NO_RECORD)
              {
                NODESKey0.recordID = PATTERNS.NODESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                if(rcode2 == 0)
                {
                  strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
                }
                else
                {
                  strcpy(nodeAbbr, "????");
                }
                PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                if(rcode2 == 0)
                {
                  strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                  trim(patternName, PATTERNNAMES_NAME_LENGTH);
                }
                else
                {
                  strcpy(patternName, "????");
                }
                LoadString(hInst, ERROR_321, szFormatString, SZFORMATSTRING_LENGTH);
                sprintf(outputString, szFormatString,
                      nodeAbbr, patternName, routeNumber, serviceName, directionAbbr);              
                _lwrite(hfErrorLog, outputString, strlen(outputString));
                bGotError = TRUE;
              }
//
//  No error - Figure out the distance from the previous stop/node
//
              else
              {
                if(fromLongitude == NO_RECORD)
                {
                  meters = 0;
                  fromLongitude = toLongitude;
                  fromLatitude = toLatitude;
                  previousNODESrecordID = PATTERNS.NODESrecordID;
                }
                else
                {
//
//  See if there's a stop-to-stop distance record.
//  Returns NO_RECORD if not found, or 0 if there's an entry but no distance
//
                  meters = GetStopToStopDistance(previousNODESrecordID, PATTERNS.NODESrecordID, CONNECTIONS_FLAG_STOPSTOP);
                  if(meters == NO_RECORD || meters == 0)
                  { 
                    distance = (float)GreatCircleDistance(fromLongitude, fromLatitude, toLongitude, toLatitude);
                    meters = (long)(distance * 1000);
                  }
//
//  A distance of zero means that he's got a stop entered into the
//  database that's right beside an identical timepoint - flag it 
//
                  if(meters == 0)
                  {
                    PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
                    btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                    strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                    trim(patternName, PATTERNNAMES_NAME_LENGTH);
                    NODESKey0.recordID = previousNODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    strncpy(previousNodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    trim(previousNodeAbbr, NODES_ABBRNAME_LENGTH);
                    NODESKey0.recordID = PATTERNS.NODESrecordID;
                    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
                    LoadString(hInst, ERROR_336, szFormatString, SZFORMATSTRING_LENGTH);
                    sprintf(outputString, szFormatString,
                          patternName, routeNumber, serviceName, directionAbbr, previousNodeAbbr, nodeAbbr);
                    _lwrite(hfErrorLog, outputString, strlen(outputString));
                    bGotError = TRUE;
                  }
                  fromLongitude = toLongitude;
                  fromLatitude = toLatitude;
                  previousNODESrecordID = PATTERNS.NODESrecordID;
                }
//
//  Build the output string
// 
                pointCodeType = (pPOINT[nL].flags & POINT_FLAG_STOP) ? 21 : 1;
                sprintf(outputString, "%d;%d;1;%d;%d;%ld;%d;%s;;%d\r\n",
                      lineCode, PKCode, PKOrder++, pointCode, meters, pointCodeType,
                      directionAbbr, lastPointCode);
//
//  Write it out
//
                _lwrite(hfOutputFile[0], outputString, strlen(outputString));
              }
//
//  Get the next stop/node in the pattern
//
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
//
//  We're already into the next pattern
//
            PKCode++;
          }
        }  // nK on directions
      } // lineCode found
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }  // nJ on routes
//
//  Loop back for the next service
//
  }  // nI on services
  numPATCODES = PKCode - 1;
//
//  Now cycle through the blocks to find pull-outs,
//  pull-ins, and interline deadheads.  When they're
//  encountered, go through the PATCODES tables to
//  see if we've built a dummy pattern, and add one
//  if we haven't
//
//  Cycle through the services
//
  firstDeadPattern = PKCode;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Find the line code
//
      lineCode = NO_RECORD;
      for(nK = 0; nK < maxRoutes; nK++)
      {
        if(pLINE[nK].recordID == ROUTES.recordID)
        {
          lineCode = nK + 1;
          break;
        }
      }
//
// It had to be there
//
      if(lineCode == NO_RECORD)
      {
        LoadString(hInst, ERROR_322, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(outputString, szFormatString, routeNumber);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
        bGotError = TRUE;
      }
//
//  Line code found - Go through the blocks
//
      else
      {
        TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
        TRIPSKey2.blockNumber = NO_RECORD;
        TRIPSKey2.blockSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        bGotNextTrip = FALSE;
        while(rcode2 == 0 &&
              TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
              TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
        {
//
//  Gen the trip
//
          if(!bGotNextTrip)
          {
            numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          }

//
//  Check on P/O
//
          bGotNextTrip = FALSE;
          if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
          {
//
//  See if we've recorded this P/O before
//
            for(bFound = FALSE, nK = firstDeadPattern; nK < PKCode; nK++)
            {
              if((pPATCODES[nK].flags & PATCODE_FLAG_PO) &&
                    pPATCODES[nK].recordID == TRIPS.PATTERNNAMESrecordID &&
                    pPATCODES[nK].ROUTESrecordID == TRIPS.ROUTESrecordID &&
                    pPATCODES[nK].SERVICESrecordID == TRIPS.SERVICESrecordID &&
                    pPATCODES[nK].directionIndex == TRIPS.directionIndex &&
                    pPATCODES[nK].flaggedNODESrecordIDs[0] == TRIPS.standard.POGNODESrecordID &&
                    pPATCODES[nK].flaggedNODESrecordIDs[1] == GTResults.firstNODESrecordID)
              {
                break;
              }
            }
//
// If it wasn't found, add it
//
            if(!bFound)
            {
              if(PKCode - 1 >= maxPatternNodes)
              {
                maxPatternNodes += 100;
                pPATCODES = (PATCODESDef *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pPATCODES, sizeof(PATCODESDef) * maxPatternNodes); 
                if(pPATCODES == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto done;
                }
              }
              pPATCODES[PKCode].flags = PATCODE_FLAG_PO;
              pPATCODES[PKCode].recordID = TRIPS.PATTERNNAMESrecordID;
              pPATCODES[PKCode].ROUTESrecordID = TRIPS.ROUTESrecordID;
              pPATCODES[PKCode].SERVICESrecordID = TRIPS.SERVICESrecordID;
              pPATCODES[PKCode].directionIndex = TRIPS.directionIndex;
              pPATCODES[PKCode].flaggedNODESrecordIDs[0] = TRIPS.standard.POGNODESrecordID;
              pPATCODES[PKCode].flaggedNODESrecordIDs[1] = GTResults.firstNODESrecordID;
              PKCode++;
            }
          }
//
//  Check on P/I or ILDH (mutually exclusive)
//
//  P/I
//
          if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)          
          {
//
//  See if we've recorded this P/I before
//
            for(bFound = FALSE, nK = firstDeadPattern; nK < PKCode; nK++)
            {
              if((pPATCODES[nK].flags & PATCODE_FLAG_PI) &&
                    pPATCODES[nK].recordID == TRIPS.PATTERNNAMESrecordID &&
                    pPATCODES[nK].ROUTESrecordID == TRIPS.ROUTESrecordID &&
                    pPATCODES[nK].SERVICESrecordID == TRIPS.SERVICESrecordID &&
                    pPATCODES[nK].directionIndex == TRIPS.directionIndex &&
                    pPATCODES[nK].flaggedNODESrecordIDs[0] == GTResults.lastNODESrecordID &&
                    pPATCODES[nK].flaggedNODESrecordIDs[1] == TRIPS.standard.PIGNODESrecordID)
              {
                break;
              }
            }
//
// If it wasn't found, add it
//
            if(!bFound)
            {
              if(PKCode - 1 >= maxPatternNodes)
              {
                maxPatternNodes += 100;
                pPATCODES = (PATCODESDef *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pPATCODES, sizeof(PATCODESDef) * maxPatternNodes); 
                if(pPATCODES == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto done;
                }
              }
              pPATCODES[PKCode].flags = PATCODE_FLAG_PI;
              pPATCODES[PKCode].recordID = TRIPS.PATTERNNAMESrecordID;
              pPATCODES[PKCode].ROUTESrecordID = TRIPS.ROUTESrecordID;
              pPATCODES[PKCode].SERVICESrecordID = TRIPS.SERVICESrecordID;
              pPATCODES[PKCode].directionIndex = TRIPS.directionIndex;
              pPATCODES[PKCode].flaggedNODESrecordIDs[0] = GTResults.lastNODESrecordID;
              pPATCODES[PKCode].flaggedNODESrecordIDs[1] = TRIPS.standard.PIGNODESrecordID;
              PKCode++;
            }
          }
//
//  ILDH
//      
          else
          {
            blockNumber = TRIPS.standard.blockNumber;
            fromNODESrecordID = GTResults.lastNODESrecordID;
            fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            fromROUTESrecordID = TRIPS.ROUTESrecordID;
            fromSERVICESrecordID = TRIPS.SERVICESrecordID;
            fromDirectionIndex = TRIPS.directionIndex;
//
//  Get the next trip on the block and gen it
//
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            bGotNextTrip = TRUE;
            if(rcode2 == 0 &&
                  TRIPS.standard.blockNumber == blockNumber &&
                  TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
                  TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
            {
              numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  See if he's deadheading
//
              if(!NodesEquivalent(fromNODESrecordID, GTResults.firstNODESrecordID, &EQTT))
              {
//
//  See if we've recorded this ILDH before
//
                for(bFound = FALSE, nK = firstDeadPattern; nK < PKCode; nK++)
                {
                  if((pPATCODES[nK].flags & PATCODE_FLAG_ILDH) &&
                        pPATCODES[nK].recordID == fromPATTERNNAMESrecordID &&
                        pPATCODES[nK].ROUTESrecordID == fromROUTESrecordID &&
                        pPATCODES[nK].SERVICESrecordID == fromSERVICESrecordID &&
                        pPATCODES[nK].directionIndex == fromDirectionIndex &&
                        pPATCODES[nK].flaggedNODESrecordIDs[0] == fromNODESrecordID &&
                        pPATCODES[nK].flaggedNODESrecordIDs[1] == GTResults.firstNODESrecordID)
                  {
                    break;
                  }
                }
//
// If it wasn't found, add it
//
                if(!bFound)
                {
                  if(PKCode - 1 >= maxPatternNodes)
                  {
                    maxPatternNodes += 100;
                    pPATCODES = (PATCODESDef *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, pPATCODES, sizeof(PATCODESDef) * maxPatternNodes); 
                    if(pPATCODES == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, FALSE);
                      goto done;
                    }
                  }
                  pPATCODES[PKCode].flags = PATCODE_FLAG_ILDH;
                  pPATCODES[PKCode].recordID = fromPATTERNNAMESrecordID;
                  pPATCODES[PKCode].ROUTESrecordID = fromROUTESrecordID;
                  pPATCODES[PKCode].SERVICESrecordID = fromSERVICESrecordID;
                  pPATCODES[PKCode].directionIndex = fromDirectionIndex;
                  pPATCODES[PKCode].flaggedNODESrecordIDs[0] = fromNODESrecordID;
                  pPATCODES[PKCode].flaggedNODESrecordIDs[1] = GTResults.firstNODESrecordID;
                  PKCode++;
                }
              }
            }
          }  //ILDH  
//
//  Get the next trip on the block (if necessary)
//
          if(!bGotNextTrip)
          {
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          }
        }  // while traversing blocks
      } // lineCode found
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }  // nJ on routes
//
//  Loop back for the next service
//
  }  // nI on services
//
//  Go through all the dead patterns and dump them out
//
  lastDeadPattern = PKCode;
  for(nI = firstDeadPattern; nI < lastDeadPattern; nI++)
  {
//
//  Get the service name
//
    SERVICESKey0.recordID = pPATCODES[nI].SERVICESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Get the route number
//
    ROUTESKey0.recordID = pPATCODES[nI].ROUTESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Find the line code
//
    lineCode = NO_RECORD;
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(pLINE[nJ].recordID == ROUTES.recordID)
      {
        lineCode = nJ + 1;
        break;
      }
    }
//
//  Get the direction's abbreviated name
//
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[pPATCODES[nI].directionIndex]; // ROUTES.DIRECTIONSrecordID[nK];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(directionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(directionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Get the from node
//
    NODESKey0.recordID = pPATCODES[nI].flaggedNODESrecordIDs[0];
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
// 
//  Find the node in pPOINT
//
    fromPointCode = NO_RECORD;
    if(rcode2 == 0)
    {
      for(nJ = 0; nJ < maxNodes; nJ++)
      {
        if(pPOINT[nJ].recordID == NODES.recordID)
        {
          fromPointCode = nJ + 1;
          fromLongitude = pPOINT[nJ].longitude;
          fromLatitude = pPOINT[nJ].latitude;
          fromNODESrecordID = NODES.recordID;
          break;
        }
      }
    }
//  
//  Make sure we found it
//
    if(fromPointCode == NO_RECORD)
    {
      if(rcode2 == 0)
      {
        strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
      }
      else
      {
        strcpy(nodeAbbr, "????");
      }
      strcpy(patternName, "Dead Pattern");
      LoadString(hInst, ERROR_321, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(outputString, szFormatString,
            nodeAbbr, patternName, routeNumber, serviceName, directionAbbr);              
      _lwrite(hfErrorLog, outputString, strlen(outputString));
      bGotError = TRUE;
      break;
    }
//
//  Get the to node
//
    NODESKey0.recordID = pPATCODES[nI].flaggedNODESrecordIDs[1];
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
// 
//  Find the node in pPOINT
//
    toPointCode = NO_RECORD;
    if(rcode2 == 0)
    {
      for(nJ = 0; nJ < maxNodes; nJ++)
      {
        if(pPOINT[nJ].recordID == NODES.recordID)
        {
          toPointCode = nJ + 1;
          toLongitude = pPOINT[nJ].longitude;
          toLatitude = pPOINT[nJ].latitude;
          toNODESrecordID = NODES.recordID;
          break;
        }
      }
    }
//  
//  Make sure we found it
//
    if(toPointCode == NO_RECORD)
    {
      if(rcode2 == 0)
      {
        strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
      }
      else
      {
        strcpy(nodeAbbr, "????");
      }
      strcpy(patternName, "Dead Pattern");
      LoadString(hInst, ERROR_321, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(outputString, szFormatString,
            nodeAbbr, patternName, routeNumber, serviceName, directionAbbr);              
      _lwrite(hfErrorLog, outputString, strlen(outputString));
      bGotError = TRUE;
      break;
    }
//
//  Figure out the distance on the deadhead
//
    meters = GetStopToStopDistance(fromNODESrecordID, toNODESrecordID, CONNECTIONS_FLAG_DEADHEADTIME);
    if(meters == NO_RECORD || meters == 0)
    { 
      distance = (float)GreatCircleDistance(fromLongitude, fromLatitude, toLongitude, toLatitude);
      meters = (long)(distance * 1000);
    }
//
//  Build the output string
// 
//
//  Pattern.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Line#      code (FK)
//  Pattern    code (PK.1)
//  Pat Type   int (1=Service, 2=Dead run)
//  Order#     code (PK.2)
//  Point#     int(8) (FK)
//  Distance   int (to predecessor, in meters - 0 for first point)
//  Point type int (1=Stop/Timepoint, 2=Depot, 6=District Border Point)
//  Direction  code
//  Announ#    not used - blank
//  Dest#      Last point on the pattern
//
//  Pull-outs
//
//    if(pPATCODES[nI].flags & PATCODE_FLAG_PO)
//    {
      sprintf(outputString, "%d;%d;2;%d;%d;0;2;%s;;%d\r\n",
            lineCode, nI + 1, PKOrder++, fromPointCode, directionAbbr, toPointCode);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      sprintf(outputString, "%d;%d;2;%d;%d;%ld;1;%s;;%d\r\n",
            lineCode, nI + 1, PKOrder++, toPointCode, meters, directionAbbr, toPointCode);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//      sprintf(outputString, "%d;%d;2;%d;%d;%ld;2;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, pointCode, meters, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//    }
//
//  Pull-ins
//
//    else if(pPATCODES[nI].flags & PATCODE_FLAG_PI)
//    {
//      sprintf(outputString, "%d;%d;2;%d;%d;0;1;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, pointCode, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//      sprintf(outputString, "%d;%d;2;%d;%d;%ld;2;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, lastPointCode, meters, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//      sprintf(outputString, "%d;%d;2;%d;%d;%ld;2;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, pointCode, meters, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//    }
//
//  ILDH
//
//    else if(pPATCODES[nI].flags & PATCODE_FLAG_ILDH)
//    {
//      sprintf(outputString, "%d;%d;2;%d;%d;0;1;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, pointCode, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//      sprintf(outputString, "%d;%d;2;%d;%d;%ld;1;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, lastPointCode, meters, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//      sprintf(outputString, "%d;%d;2;%d;%d;%ld;1;%s;;%d\r\n",
//            lineCode, nI + 1, PKOrder++, pointCode, meters, directionAbbr, lastPointCode);
//      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//    }
  }
//
//  Close the files
//
  _lclose(hfOutputFile[0]);
  _lclose(hfOutputFile[1]);
  _lclose(hfOutputFile[2]);
//
//  Journey.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Line#      code (FK)
//  Sched Dep  time in seconds
//  Pattern    code (FK)
//  Time Demnd code (set to 1)
//  Block      code (FK)
//  Journey    code (PK) (Trip number)
//  Daytype    code (Service - char[SERVICES_NAME_LENGTH])
//
//  and...
//
//  Times.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Pattern    code (PK.1)
//  Time Demnd code (PK.3)
//  Idx in Pat int  (PK.2)
//  Time       int  (0=first, others in seconds)
//  Stop time  int  (0) 
//
//
//  Journey.csv
//
  hfOutputFile[0] = _lcreat("Journey.csv", 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
//
//  Times.csv
//
  hfOutputFile[1] = _lcreat("Times.csv", 0);
  if(hfOutputFile[1] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    _lclose(hfOutputFile[0]);
    goto done;
  }
  StatusBarText("Journey.csv and Times.csv");
//
//  Journey.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Line#      code (FK)
//  Sched Dep  time in seconds
//  Pattern    code (FK)
//  Time Demnd code (set to 1)
//  Block      code (FK)
//  Journey    code (PK) (Trip number)
//  Daytype    code (Service - char[SERVICES_NAME_LENGTH])
//
//  and...
//
//  Times.csv
//
//  Variable   Type
//  ~~~~~~~~   ~~~~
//  Pattern    code (PK.1)
//  Time Demnd code (PK.3)
//  Idx in Pat int  (PK.2)
//  Time       int  (0=first, others in seconds)
//  Stop time  int  (0) 
//
//  Write out the comment strings
//
//  Journey.csv
//
  strcpy(outputString, "#LineCode;ScheduledDepart;PatternCode;TimeDemandType;BlockCode;JourneyCode;ServiceName\r\n");
  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Times.csv
//
  strcpy(outputString, "#PatternCode;TimeDemandType;IndexIntoPattern;Time;StopTime\r\n");
  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
//
//  Cycle through the services
//
  PKCode = 1;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    SERVICESKey0.recordID = SERVICESinEffect[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    StatusBar((long)(nI + 1), (long)numServices);
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    for(nJ = 0; nJ < maxRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Find the line code
//
      lineCode = NO_RECORD;
      for(nK = 0; nK < maxRoutes; nK++)
      {
        if(pLINE[nK].recordID == ROUTES.recordID)
        {
          lineCode = nK + 1;
          break;
        }
      }
//
//  The line code must have been found
//
      if(lineCode == NO_RECORD)
      {
        LoadString(hInst, ERROR_322, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(outputString, szFormatString, routeNumber);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
        bGotError = TRUE;
      }
//
//  It was - Cycle through the directions
//
      else
      {
        for(nK = 0; nK < 2; nK++)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          {
            continue;
          }
//
//  Keep the direction's abbreviated name
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(directionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Get the first trip
//
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
          TRIPSKey1.directionIndex = nK;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          timeDemandType = 1;
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nK)
          {
            if(TRIPS.tripNumber == 7895)
            {
              TRIPS.tripNumber = 7895;
            }
//
//  Generate it
//
            numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this trip a pull-out?
//
//  If so, preface the regular output with its "dead run"
//
            if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
            {
//
//  Locate the pattern in the dead runs portion of the PATCODES structure
//
              patternCode = NO_RECORD;
              for(nL = firstDeadPattern; nL < lastDeadPattern; nL++)
              {
                if((pPATCODES[nL].flags & PATCODE_FLAG_PO) &&
                      pPATCODES[nL].recordID == TRIPS.PATTERNNAMESrecordID &&
                      pPATCODES[nL].ROUTESrecordID == ROUTES.recordID &&
                      pPATCODES[nL].SERVICESrecordID == SERVICES.recordID &&
                      pPATCODES[nL].directionIndex == nK &&
                      pPATCODES[nL].flaggedNODESrecordIDs[0] == TRIPS.standard.POGNODESrecordID &&
                      pPATCODES[nL].flaggedNODESrecordIDs[1] == GTResults.firstNODESrecordID)
                {
                  patternCode = nL + 1;
                  break;
                }
              }
//
//  The patternCode must have been found
//
              if(patternCode == NO_RECORD)
              {
                LoadString(hInst, ERROR_332, szFormatString, SZFORMATSTRING_LENGTH);
                sprintf(outputString, szFormatString, TRIPS.recordID, 
                      routeNumber, serviceName, directionAbbr);              
                _lwrite(hfErrorLog, outputString, strlen(outputString));
                bGotError = TRUE;
                break;
              }
//
//  It was - Locate the block in the INITBLOCKS structure
//
              else
              {
                blockCode = NO_RECORD;
                for(nL = 0; nL < numINITBLOCKS; nL++)
                {
                  if(pINITBLOCKS[nL].blockNumber == TRIPS.standard.blockNumber &&
                        pINITBLOCKS[nL].ROUTESrecordID == TRIPS.standard.RGRPROUTESrecordID &&
                        pINITBLOCKS[nL].SERVICESrecordID == TRIPS.standard.SGRPSERVICESrecordID)
                  {
                    blockCode = nL + 1;
                    break;
                  }
                }
//
//  The block code has to have been found
//
                if(blockCode == NO_RECORD)
                {
                  LoadString(hInst, ERROR_327, szFormatString, SZFORMATSTRING_LENGTH);
                  sprintf(outputString, szFormatString, TRIPS.recordID, 
                        routeNumber, serviceName, directionAbbr);              
                  _lwrite(hfErrorLog, outputString, strlen(outputString));
                  bGotError = TRUE;
                  break;
                }
//
//  It was - set out the records
//
//
//  Journey
//
                else
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
                  POTime = deadheadTime == NO_TIME ?
                        GTResults.firstNodeTime : GTResults.firstNodeTime - deadheadTime;
                  sprintf(outputString, "%d;%ld;%d;%ld;%ld;%ld;%s\r\n",
                        lineCode, POTime, patternCode,
                        lastTripNumber, blockCode, lastTripNumber, serviceName);
                  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Times
//
                  sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                        patternCode, lastTripNumber, 1, 0);
                  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                        patternCode, lastTripNumber, 2, deadheadTime);
                  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  lastTripNumber++;
                }
              }
            }
//
//  Now, the body of the trip
//
//  Locate the pattern in the PATCODES structure
//
            patternCode = NO_RECORD;
            for(nL = 0; nL < numPATCODES; nL++)
            {
              if(pPATCODES[nL].recordID == TRIPS.PATTERNNAMESrecordID &&
                    pPATCODES[nL].ROUTESrecordID == ROUTES.recordID &&
                    pPATCODES[nL].SERVICESrecordID == SERVICES.recordID &&
                    pPATCODES[nL].directionIndex == nK)
              {
                patternCode = nL + 1;
                break;
              }
            }
//
//  The patternCode must have been found
//
            if(patternCode == NO_RECORD)
            {
              LoadString(hInst, ERROR_326, szFormatString, SZFORMATSTRING_LENGTH);
              sprintf(outputString, szFormatString, TRIPS.recordID, 
                    routeNumber, serviceName, directionAbbr);              
              _lwrite(hfErrorLog, outputString, strlen(outputString));
              bGotError = TRUE;
              break;
            }
//
//  It was - Locate the block in the INITBLOCKS structure
//
            else
            {
              blockCode = NO_RECORD;
              for(nL = 0; nL < numINITBLOCKS; nL++)
              {
                if(pINITBLOCKS[nL].blockNumber == TRIPS.standard.blockNumber &&
                      pINITBLOCKS[nL].ROUTESrecordID == TRIPS.standard.RGRPROUTESrecordID &&
                      pINITBLOCKS[nL].SERVICESrecordID == TRIPS.standard.SGRPSERVICESrecordID)
                {
                  blockCode = nL + 1;
                  break;
                }
              }
//
//  The block code has to have been found
//
              if(blockCode == NO_RECORD)
              {
                LoadString(hInst, ERROR_327, szFormatString, SZFORMATSTRING_LENGTH);
                sprintf(outputString, szFormatString, TRIPS.recordID, 
                      routeNumber, serviceName, directionAbbr);              
                _lwrite(hfErrorLog, outputString, strlen(outputString));
                bGotError = TRUE;
                break;
              }
//
//  It was - set out the records
//
//
//  Journey
//
              else
              {
                sprintf(outputString, "%d;%ld;%d;%ld;%ld;%ld;%s\r\n",
                      lineCode, GTResults.firstNodeTime, patternCode,
                      TRIPS.tripNumber, blockCode, TRIPS.tripNumber, serviceName);
                _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Times
//
//  Go through the pattern
//
                PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
                PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
                PATTERNSKey2.directionIndex = nK;
                PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                PATTERNSKey2.nodeSequence = NO_RECORD;
                rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                nL = 0;
                nM = 1;
                previousTime = NO_TIME;
                while(rcode2 == 0 &&
                      PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                      PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                      PATTERNS.directionIndex == nK &&
                      PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                {
//
//  If it's a stop, locate it and determine its time
//  base on the relative distance to the next timepoint
//
                  if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
                  {
                  }
                   else
                  {  
                    nodeTime = (previousTime == NO_TIME ? 0 :
                          GTResults.tripTimes[nL] - previousTime);
                    previousTime = GTResults.tripTimes[nL];
                    nL++;
                    sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                          patternCode, TRIPS.tripNumber, nM, nodeTime);
                    _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  }
                  nM++;
                  rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                }
              }          
            }
//
//  Is this trip a pull-in?  (Again, a mutually exclusive check with ILDH)
//
//  If so, suffix the regular output with its "dead run"
//
            if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
            {
//
//  Locate the pattern in the dead runs portion of the PATCODES structure
//
              patternCode = NO_RECORD;
              for(nL = firstDeadPattern; nL < lastDeadPattern; nL++)
              {
                if((pPATCODES[nL].flags & PATCODE_FLAG_PI) &&
                      pPATCODES[nL].recordID == TRIPS.PATTERNNAMESrecordID &&
                      pPATCODES[nL].ROUTESrecordID == ROUTES.recordID &&
                      pPATCODES[nL].SERVICESrecordID == SERVICES.recordID &&
                      pPATCODES[nL].directionIndex == nK &&
                      pPATCODES[nL].flaggedNODESrecordIDs[0] == GTResults.lastNODESrecordID &&
                      pPATCODES[nL].flaggedNODESrecordIDs[1] == TRIPS.standard.PIGNODESrecordID)
                {
                  patternCode = nL + 1;
                  break;
                }
              }
//
//  The patternCode must have been found
//
              if(patternCode == NO_RECORD)
              {
                LoadString(hInst, ERROR_333, szFormatString, SZFORMATSTRING_LENGTH);
                sprintf(outputString, szFormatString, TRIPS.recordID, 
                      routeNumber, serviceName, directionAbbr);              
                _lwrite(hfErrorLog, outputString, strlen(outputString));
                bGotError = TRUE;
                break;
              }
//
//  It was - Locate the block in the INITBLOCKS structure
//
              else
              {
                blockCode = NO_RECORD;
                for(nL = 0; nL < numINITBLOCKS; nL++)
                {
                  if(pINITBLOCKS[nL].blockNumber == TRIPS.standard.blockNumber &&
                        pINITBLOCKS[nL].ROUTESrecordID == TRIPS.standard.RGRPROUTESrecordID &&
                        pINITBLOCKS[nL].SERVICESrecordID == TRIPS.standard.SGRPSERVICESrecordID)
                  {
                    blockCode = nL + 1;
                    break;
                  }
                }
//
//  The block code has to have been found
//
                if(blockCode == NO_RECORD)
                {
                  LoadString(hInst, ERROR_327, szFormatString, SZFORMATSTRING_LENGTH);
                  sprintf(outputString, szFormatString, TRIPS.recordID, 
                        routeNumber, serviceName, directionAbbr);              
                  _lwrite(hfErrorLog, outputString, strlen(outputString));
                  bGotError = TRUE;
                  break;
                }
//
//  It was - set out the records
//
//  Journey
//
                else
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
                  PITime = deadheadTime == NO_TIME ?
                        GTResults.lastNodeTime : GTResults.lastNodeTime + deadheadTime;
                  sprintf(outputString, "%d;%ld;%d;%ld;%ld;%ld;%s\r\n",
                        lineCode, PITime, patternCode,
                        lastTripNumber, blockCode, lastTripNumber, serviceName);
                  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Times
//
                  sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                        patternCode, lastTripNumber, 1, 0);
                  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                        patternCode, lastTripNumber, 2, deadheadTime);
                  _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                  lastTripNumber++;
                }
              }
            }
//
//  Check on an interline deadhead
//
            else
            {
              TRIPSrecordID = TRIPS.recordID;
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              blockNumber = TRIPS.standard.blockNumber;
              fromNODESrecordID = GTResults.lastNODESrecordID;
              fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              fromROUTESrecordID = TRIPS.ROUTESrecordID;
              fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              fromDirectionIndex = TRIPS.directionIndex;
              fromTime = GTResults.lastNodeTime;
//
//  Get the next trip on the block and gen it
//
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              bGotNextTrip = TRUE;
              if(rcode2 == 0 &&
                    TRIPS.standard.blockNumber == blockNumber &&
                    TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
                    TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
              {
                numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  See if he's deadheading
//
                if(!NodesEquivalent(fromNODESrecordID, GTResults.firstNODESrecordID, &EQTT))
                {
//
//  Locate the pattern in the dead runs portion of the PATCODES structure
//
                  patternCode = NO_RECORD;
                  for(nL = firstDeadPattern; nL < lastDeadPattern; nL++)
                  {
                    if((pPATCODES[nL].flags & PATCODE_FLAG_ILDH) &&
                          pPATCODES[nL].recordID == fromPATTERNNAMESrecordID &&
                          pPATCODES[nL].ROUTESrecordID == fromROUTESrecordID &&
                          pPATCODES[nL].SERVICESrecordID == fromSERVICESrecordID &&
                          pPATCODES[nL].directionIndex == fromDirectionIndex &&
                          pPATCODES[nL].flaggedNODESrecordIDs[0] == fromNODESrecordID &&
                          pPATCODES[nL].flaggedNODESrecordIDs[1] == GTResults.firstNODESrecordID)
                    {
                      patternCode = nL + 1;
                      break;
                    }
                  }
//
//  The patternCode must have been found
//
                  if(patternCode == NO_RECORD)
                  {
                    LoadString(hInst, ERROR_334, szFormatString, SZFORMATSTRING_LENGTH);
                    sprintf(outputString, szFormatString, TRIPS.recordID, 
                          routeNumber, serviceName, directionAbbr);              
                    _lwrite(hfErrorLog, outputString, strlen(outputString));
                    bGotError = TRUE;
                    break;
                  }
//
//  It was - Locate the block in the INITBLOCKS structure
//
                  else
                  {
                    blockCode = NO_RECORD;
                    for(nL = 0; nL < numINITBLOCKS; nL++)
                    {
                      if(pINITBLOCKS[nL].blockNumber == TRIPS.standard.blockNumber &&
                            pINITBLOCKS[nL].ROUTESrecordID == TRIPS.standard.RGRPROUTESrecordID &&
                            pINITBLOCKS[nL].SERVICESrecordID == TRIPS.standard.SGRPSERVICESrecordID)
                      {
                        blockCode = nL + 1;
                        break;
                      }
                    }
//
//  The block code has to have been found
//
                    if(blockCode == NO_RECORD)
                    {
                      LoadString(hInst, ERROR_327, szFormatString, SZFORMATSTRING_LENGTH);
                      sprintf(outputString, szFormatString, TRIPS.recordID, 
                            routeNumber, serviceName, directionAbbr);              
                      _lwrite(hfErrorLog, outputString, strlen(outputString));
                      bGotError = TRUE;
                      break;
                    }
//
//  It was - set out the records
//
//  Journey
//
                    else
                    {
                      sprintf(outputString, "%d;%ld;%d;%ld;%ld;%ld;%s\r\n",
                            lineCode, fromTime, patternCode,
                            lastTripNumber, blockCode, lastTripNumber, serviceName);
                      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
//
//  Times
//
                      sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                            patternCode, lastTripNumber, 1, 0);
                      _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                      sprintf(outputString, "%d;%ld;%d;%ld;0\r\n",
                            patternCode, lastTripNumber, 2, GTResults.firstNodeTime - fromTime);
                      _lwrite(hfOutputFile[1], outputString, strlen(outputString));
                      lastTripNumber++;
                    }
                  }
                }
              }
//
//  Re-establish position
//
              TRIPSKey0.recordID = TRIPSrecordID;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            }
//
//  Get the next trips record
//
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
        }
      }  // nK - Directions
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }  // nJ - Routes
  }  // nI - Services
//
//  Close the files
//
  _lclose(hfOutputFile[0]);
  _lclose(hfOutputFile[1]);
//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  done:
    chdir("..");  // Back to where we started from
    TMSHeapFree(pLINE);
    TMSHeapFree(pPOINT);
    TMSHeapFree(pSTOPNODES);
    TMSHeapFree(pINITBLOCKS);
    TMSHeapFree(pPATCODES);
    StatusBarEnd();
  
  strcpy(tempString, "Init Unload files were written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  if(bGotError)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_328, (HANDLE)NULL);
  }
  _lclose(hfErrorLog);
  distanceMeasure = distanceMeasureSave;
  return(bFinishedOK);
}

//
//  Get a stop to stop or deadhead distance
//
//  Flags can be either CONNECTIONS_FLAG_STOPSTOP or CONNECTIONS_FLAG_DEADHEADTIME
//
long GetStopToStopDistance(long fromNODESrecordID, long toNODESrecordID, long flags)
{
  int  rcode2;
  long returnValue;

  CONNECTIONSKey1.fromNODESrecordID = fromNODESrecordID;
  CONNECTIONSKey1.toNODESrecordID = toNODESrecordID;
  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0 &&
        CONNECTIONS.fromNODESrecordID == fromNODESrecordID &&
        CONNECTIONS.toNODESrecordID == toNODESrecordID)
  {
    if(CONNECTIONS.flags & flags)
    {
      if(CONNECTIONS.distance <= 0.0)  // Older versions had NO_RECORD instead of 0.0
      {
        return(0L);
      }
      if(distanceMeasureSave == PREFERENCES_MILES)
      {
        returnValue = (long)((CONNECTIONS.distance / 0.6213712) * 1000);
      }
      else
      {
        returnValue = (long)(CONNECTIONS.distance * 1000);
      }
      return(returnValue);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }
//
//  In the case of a deadhead, try flipping the nodes to see if there's a two-way connection
//
  if(flags & CONNECTIONS_FLAG_STOPSTOP)
  {
    return(NO_RECORD);
  }
  CONNECTIONSKey1.fromNODESrecordID = toNODESrecordID;
  CONNECTIONSKey1.toNODESrecordID = fromNODESrecordID;
  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0 &&
        CONNECTIONS.fromNODESrecordID == toNODESrecordID &&
        CONNECTIONS.toNODESrecordID == fromNODESrecordID)
  {
    if((CONNECTIONS.flags & flags) && (CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY))
    {
      if(CONNECTIONS.distance <= 0.0)  // Older versions had NO_RECORD instead of 0.0
      {
        return(0L);
      }
      return(distanceMeasure == PREFERENCES_MILES ?
            (long)((CONNECTIONS.distance / 0.6213712) * 1000) : (long)(CONNECTIONS.distance * 1000));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }

  return(NO_RECORD);
}
