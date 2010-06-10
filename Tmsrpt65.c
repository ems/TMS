// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"

void EN2LL(int, double, double, char*, double *, double *);

#define TMSRPT65_MAXSERVICES      25
#define TMSRPT65_MAXROUTES       200
#define TMSRPT65_MAXPATTERNNODES 100
#define TMSRPT65_MAXTREQ        1000
 
int  distanceMeasureSave;

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


typedef struct TMSRPT65PATStruct
{
  long PATTERNNAMESrecordID;
  int  numNodes;
  long NODESrecordIDs[200];
  long flags[200];
} TMSRPT65PATDef;
  
typedef struct NODESUBSETStruct
{
  long recordID;
  long number;
  long OBStopNumber;
  long IBStopNumber;
  long flags;
} NODESUBSETDef;
//
//  Web-Based Trip Planner Download
//
#define OUTPUT_FOLDER_NAME "Web-Based Trip Planner Data Files"

BOOL FAR TMSRPT65(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  TMSRPT65PATDef  PAT[20];
  NODESUBSETDef  *pNODESUBSET;
  double  Long, Lat;
  double  prevLon, prevLat;
  double  distanceToHere;
  double  tripDistances[500];
  time_t  fromTime;
  ident_t fromLocID;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bFound;
  BOOL  bFirst;
  BOOL  bIncludeSchool;
  long  prevPattern;
  long  totalCycles;
  long  year, month, day;
  long  timeAtStop;
  long  lastTripNumber;
  long  maxTrips;
  long  recordID;
  long  stopNumber;
  char  szServiceName[SERVICES_NAME_LENGTH + 1];
  char  szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
  char  szRouteName[ROUTES_NAME_LENGTH + 1];
  char  szDirectionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  szOutputFileName[64];
  char  szOutputString[1024];
  char  dummy[256];
  char *pszReportName;
  char *pszDays[] = {"Monday", "Tuesday", "Wednesday",
                     "Thursday", "Friday", "Saturday", "Sunday"};
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   maxRoutes;
  int   maxServices;
  int   numRoutes;
  int   numServices;
  int   numPatterns;
  int   tripIndex;
  int   numNODESUBSET;

  bFinishedOK = FALSE;

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
  pPassedData->nReportNumber = 64;
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
//  maxTrips
//
  rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
  maxTrips = rcode2 == 0 ? BSTAT.numRecords : 0;
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
//  See if he wants for include school routes
//
  LoadString(hInst, TEXT_275, tempString, sizeof(tempString));
  MessageBeep(MB_ICONQUESTION);
  bIncludeSchool = (MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES);
//
//  Get the maximum number of routes
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  maxRoutes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Get the maximum number of services
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  maxServices = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Node allocation
//
//  Get the number of nodes in the table
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  numNODESUBSET = BSTAT.numRecords;
  pNODESUBSET = (NODESUBSETDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(NODESUBSETDef) * numNODESUBSET); 
  if(pNODESUBSET == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
    goto done;
  }

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  nI = 0;
  while(rcode2 == 0)
  {
    pNODESUBSET[nI].recordID = NODES.recordID;
    pNODESUBSET[nI].number = NODES.number;
    pNODESUBSET[nI].OBStopNumber = NODES.OBStopNumber;
    pNODESUBSET[nI].IBStopNumber = NODES.IBStopNumber;
    pNODESUBSET[nI].flags = NODES.flags;
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  ======================
//  Nodes File - Nodes.txt
//  ======================
//
//  Tab separated values
//
//  long     Nodes record ID
//  double   Latitude 
//  double   Longitude
//  char[64] Intersection/Address
//  char     Preferred transfer point (T/F)
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Nodes.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the nodes
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    if(m_bUseLatLong)
    {
      Lat  = (double)NODES.latitude;
      Long = (double)NODES.longitude;
    }
    else
    {
      EN2LL(5, (double)NODES.latitude, (double)NODES.longitude, szUTMZone, &Lat, &Long);
    }
    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(tempString, NODES_DESCRIPTION_LENGTH);
    sprintf(szOutputString, "%8ld\t%14.6f\t%14.6f\t\"%s\"", NODES.recordID, Lat, Long, tempString);
    strcat(szOutputString, "\t");
    strcat(szOutputString, (NODES.flags & NODES_FLAG_AVLTRANSFERPOINT ? "T" : "F"));
    strcat(szOutputString, "\r\n");
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
  _lclose(hfOutputFile);
//
//  ====================================
//  Equivalences File - Equivalences.txt
//  ====================================
//
//  Tab separated values
//
//  long     From nodes record ID
//  long     To nodes record ID
//  long     Connection time
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Equivalences.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the Connections Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      sprintf(szOutputString, "%8ld\t%8ld\t%ld\r\n", CONNECTIONS.fromNODESrecordID, CONNECTIONS.toNODESrecordID, CONNECTIONS.connectionTime);
      _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
  _lclose(hfOutputFile);
//
//  ================================
//  Directions File - Directions.txt
//  ================================
//
//  Tab separated values
//
//  long     Directions record ID
//  char[16] Direction long name
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Directions.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the directions
//
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    sprintf(szOutputString, "%8ld\t%s\r\n", DIRECTIONS.recordID, tempString);
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  }
  _lclose(hfOutputFile);
//
//  ============================
//  Services File - Services.txt
//  ============================
//
//  Tab separated values
//
//  long     Services record ID
//  char[32] Service name
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Services.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the services
//
  numServices = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    sprintf(szOutputString, "%8ld\t%s\r\n", SERVICES.recordID, tempString);
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    numServices++;
  }
  _lclose(hfOutputFile);
//
//  ============================================================
//  ServiceCalendarDefaults.txt - What service runs on which day
//  ============================================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "ServiceCalendarDefaults.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    sprintf(szarString, "ServiceDay%d", nI);
    GetPrivateProfileString("Rostering", szarString, "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    SERVICESKey0.recordID = atol(tempString);
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    sprintf(szOutputString, "%s\t%ld\r\n", pszDays[nI], (rcode2 != 0 ? NO_RECORD : SERVICES.recordID));
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
  }
  _lclose(hfOutputFile);
//
//  ==================================================================
//  ServiceCalendarExceptions.txt - What service runs on which holiday
//  ==================================================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "ServiceCalendarExceptions.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the DailyOps Audit Trail
//
  DAILYOPSKey1.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
  DAILYOPSKey1.pertainsToDate = 0;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_DATE))
  {
    bFound = FALSE;
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_DATESET)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
        sprintf(szOutputString, "%04ld-%02ld-%02ld\t%ld\r\n", year, month, day, DAILYOPS.DOPS.Date.SERVICESrecordID);
        _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  _lclose(hfOutputFile);
//
//  ================================
//  Boundaries.mif - Boundaries file
//  ================================
//
//  Copy it from installDirectory
//
  strcpy(tempString, szInstallDirectory);
  strcat(tempString, "\\");
  strcat(tempString, "boundaries.mif");

  CopyFile(tempString, "Boundaries.mif", FALSE);

//
//  ===================================================
//  English-text.txt - English language output template
//  ===================================================
//
//  Copy it from installDirectory
//
  strcpy(tempString, szInstallDirectory);
  strcat(tempString, "\\");
  strcat(tempString, "English-text.txt");

  CopyFile(tempString, "English-text.txt", FALSE);

//  ============================
//  Bustypes File - Bustypes.txt
//  ============================
//
//  Tab separated values
//
//  long     Bustypes record ID
//  char[32] Bustype name
//  char[1]  Surface vehicle (Y/N)
//  char[1]  Bicycle carrier (Y/N)
//  char[1]  Accessible (Y/N)
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Bustypes.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the bustypes
//
  rcode2 = btrieve(B_GETFIRST, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
    trim(tempString, BUSTYPES_NAME_LENGTH);
    strcat(tempString, "\t");
    strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_SURFACE ? "Y" : "N"));
    strcat(tempString, "\t");
    strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_CARRIER ? "Y" : "N"));
    strcat(tempString, "\t");
    strcat(tempString, (BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE ? "Y" : "N"));
    sprintf(szOutputString, "%8ld\t%s\r\n", BUSTYPES.recordID, tempString);
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    rcode2 = btrieve(B_GETNEXT, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
  }
  _lclose(hfOutputFile);
//
//  ========================
//  Routes File - Routes.txt
//  ========================
//
//  Tab separated values
//
//  long     Routes record ID
//  long     Service record ID
//  long     Direction record ID
//  char[76] Route number/name
//
//  If this seems a bit odd, it's because routes are unloaded 
//  based on the presence of a BASE pattern for the route/ser/dir
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Routes.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through the routes
//
  numRoutes = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    if(!(ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE))
    {
      if(((ROUTES.flags & ROUTES_FLAG_SCHOOL) && bIncludeSchool) ||
            (!(ROUTES.flags & ROUTES_FLAG_SCHOOL)))
      {
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strcat(tempString, " - ");
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(szarString, ROUTES_NAME_LENGTH);
        strcat(tempString, szarString);
//
//  Cycle through the services
//
        rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        while(rcode2 == 0)
        {
//
//  Cycle through the directions
//
          for(nI = 0; nI < 2; nI++)
          {
            if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
            {
              continue;
            }
//
//  A pattern on this route/ser/dir has to exist
//
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nI;
            PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            if(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nI)
            {
              sprintf(szOutputString, "%8ld\t%8ld\t%8ld\t%s\r\n",
                    ROUTES.recordID, SERVICES.recordID, ROUTES.DIRECTIONSrecordID[nI], tempString);
              _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
            }
          }  // nI
          rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        }  // while on services
      }  // school/no school
    }  //  shuttle/not a shuttle
    numRoutes++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  }
  _lclose(hfOutputFile);
//
//  ======================
//  Trips File - Trips.txt
//  ======================
//
//  Tab separated values
//
//  long Routes record ID
//  long Services record ID
//  long Directions record ID
//  long Bustypes record ID
//  long trip number
//  long block number
//  long from time
//  long from Nodes record ID
//  long to time
//  long to Nodes record ID
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Trips.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through all the services
//
  totalCycles = numServices * numRoutes * 2;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    btrieve((nI == 0 ? B_GETFIRST : B_GETNEXT), TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(szServiceName, SERVICES_NAME_LENGTH);
//
//  Cycle through all the routes
//
    for(nJ = 0; nJ < numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      btrieve((nJ == 0 ? B_GETFIRST : B_GETNEXT), TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE))
      {
        continue;
      }
      if((ROUTES.flags & ROUTES_FLAG_SCHOOL) && !bIncludeSchool)
      {
        continue;
      }
      strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
      strncpy(szRouteName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(szRouteName, ROUTES_NAME_LENGTH);
//
//  Loop through both directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(szDirectionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(szDirectionName, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(tempString, "Setting up %s trip data\n%s - %s\n(%s)",
              szServiceName, szRouteNumber, szRouteName, szDirectionName);
        StatusBarText(tempString);
        StatusBar((long)((nI * numRoutes * 2) + (nJ * 2) + nK), totalCycles);
//
//  Position into the patterns file
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        prevPattern = NO_RECORD;
        numPatterns = -1;
//
//  Read all the individual patterns (and nodes) for this service / route / direction
//
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
          {
            prevPattern = PATTERNS.PATTERNNAMESrecordID;
            numPatterns++;
            PAT[numPatterns].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
            PAT[numPatterns].numNodes = 0;
          }
          PAT[numPatterns].NODESrecordIDs[PAT[numPatterns].numNodes] = PATTERNS.NODESrecordID;
          PAT[numPatterns].flags[PAT[numPatterns].numNodes] = PATTERNS.flags;
          PAT[numPatterns].numNodes++;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
        numPatterns++;
//
//  Position into the trips file
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
//
//  Cycle through all the trips
//
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nK)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
//
//  Kludge - Don't unload trips marked as school-day only
//           when not unloading school trips
//
//          if(!(TRIPS.COMMENTSrecordID == 4 && !bIncludeSchool))
//          {
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Go through the pattern twice
//
//  Pass 1 - Determine distances at each timepoint
//
            if(TRIPS.tripNumber == 294)
            {
              tripIndex = 0;
            }
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nK;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            distanceToHere = 0.0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nK &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(tripIndex == 0)
              {
                distanceToHere = 0;
              }
              else
              {
                distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
              }
              if(NODES.flags & NODES_FLAG_STOP)
//              if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
              {
              }
              else
              {
                tripDistances[tripIndex] = distanceToHere;
                tripIndex++;
              }
              prevLat = NODES.latitude;
              prevLon = NODES.longitude;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
//
//  Pass 2 - Determine time interpolations
//
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nK;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            bFirst = TRUE;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nK &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(NODES.flags & NODES_FLAG_STOP)
              {
                timeAtStop = 0;
                if(GTResults.tripDistance == 0)
                {
                  timeAtStop = GTResults.tripTimes[tripIndex - 1];
                }
                else
                {
                  distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
                  timeAtStop = (long)((GTResults.tripTimes[tripIndex] - GTResults.tripTimes[tripIndex - 1]) *
                        (distanceToHere / (tripDistances[tripIndex] - tripDistances[tripIndex - 1])));
                  timeAtStop += (GTResults.tripTimes[tripIndex - 1]);
                }
              }
              else
              {
                timeAtStop = GTResults.tripTimes[tripIndex];
                tripIndex++;  
                distanceToHere = 0.0;
              }
              prevLat = NODES.latitude;
              prevLon = NODES.longitude;
// 
//  Output the associated stop number, if present
//
              recordID = PATTERNS.NODESrecordID;
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                NODESKey0.recordID = PATTERNS.NODESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                if(NODES.number > 0)
                {
                  stopNumber = NODES.number;
                }
                else
                {
                  if(nK == 0)
                  {
                    stopNumber = NODES.OBStopNumber;
                  }
                  else
                  {
                    stopNumber = NODES.IBStopNumber;
                  }
                }
                if(stopNumber > 0)
                {
                  for(nL = 0; nL < numNODESUBSET; nL++)
                  {
                    if(pNODESUBSET[nI].flags & NODES_FLAG_STOP)
                    {
                      if(pNODESUBSET[nI].number == stopNumber)
                      {
                        recordID = pNODESUBSET[nI].recordID;
                        break;
                      }
                    }
                  }
                }
              }
              if(bFirst)
              {
                bFirst = FALSE;
              }
              else
              {
                sprintf(szOutputString, "%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\r\n",
                      ROUTES.recordID, SERVICES.recordID, ROUTES.DIRECTIONSrecordID[nK], TRIPS.BUSTYPESrecordID,
                      TRIPS.tripNumber, TRIPS.standard.blockNumber,
                      fromTime, fromLocID, timeAtStop, recordID);
                _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
              }
              fromTime = timeAtStop;
              fromLocID = recordID;
//
//  Get the next node on the pattern
//
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }  // while patterns
//          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
      }  // for nK
    }  // for nJ
  }  // for nI
  _lclose(hfOutputFile);
//  
//  =================================
//  Pattern Names File - Patterns.txt
//  =================================
//
//  Tab separated values
//
//  long PatternID
//  char Name
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "Patterns.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Dump out the pattern names file
//
  rcode2 = btrieve(B_GETFIRST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
    trim(tempString, PATTERNNAMES_NAME_LENGTH);
    sprintf(szOutputString, "%ld\t%s\r\n", PATTERNNAMES.recordID, tempString);
    _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  }
  _lclose(hfOutputFile);
//
//  =====================================
//  Patterns List File - PatternsList.txt 
//  =====================================
//
//  Tab separated values
//
//  long Routes record ID
//  long Services record ID
//  char Direction name
//  long Pattern Names record ID
//  long Nodes record ID (repeated)
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(szOutputFileName, "PatternsList.txt");
  hfOutputFile = _lcreat(szOutputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(szOutputFileName);
//
//  Cycle through all the services
//
  totalCycles = numServices * numRoutes * 2;
  for(nI = 0; nI < numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    btrieve((nI == 0 ? B_GETFIRST : B_GETNEXT), TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(szServiceName, SERVICES_NAME_LENGTH);
//
//  Cycle through all the routes
//
    for(nJ = 0; nJ < numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      btrieve((nJ == 0 ? B_GETFIRST : B_GETNEXT), TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE))
      {
        continue;
      }
      strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
      strncpy(szRouteName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(szRouteName, ROUTES_NAME_LENGTH);
//
//  Loop through both directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(szDirectionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(szDirectionName, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(tempString, "Setting up %s pattern data\n%s - %s\n(%s)",
              szServiceName, szRouteNumber, szRouteName, szDirectionName);
        StatusBarText(tempString);
        StatusBar((long)((nI * numRoutes * 2) + (nJ * 2) + nK), totalCycles);
//
//  Position into the patterns file
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  Read all the individual patterns (and nodes) for this service / route / direction
//
        prevPattern = NO_RECORD;
        strcpy(szOutputString, "");
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
          {
            if(prevPattern != NO_RECORD)
            {
              strcat(szOutputString, "\r\n");
              _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
            }
            sprintf(szOutputString, "%ld\t%ld\t%ld\t%ld",
                  ROUTES.recordID, SERVICES.recordID, ROUTES.DIRECTIONSrecordID[nK], PATTERNS.PATTERNNAMESrecordID);
            prevPattern = PATTERNS.PATTERNNAMESrecordID;
          }
//
//  Output the associated stop number, if present
//
          recordID = PATTERNS.NODESrecordID;
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(NODES.number > 0)
            {
              stopNumber = NODES.number;
            }
            else
            {
              if(nK == 0)
              {
                stopNumber = NODES.OBStopNumber;
              }
              else
              {
                stopNumber = NODES.IBStopNumber;
              }
            }
            if(stopNumber > 0)
            {
              for(nL = 0; nL < numNODESUBSET; nL++)
              {
                if(pNODESUBSET[nI].flags & NODES_FLAG_STOP)
                {
                  if(pNODESUBSET[nI].number == stopNumber)
                  {
                    recordID = pNODESUBSET[nI].recordID;
                    break;
                  }
                }
              }
            }
          }
          sprintf(tempString, "\t%ld", recordID);
          strcat(szOutputString, tempString);
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
        strcat(szOutputString, "\r\n");
        _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
      }  // nK
    }  // nJ
  }  // nI
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
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_328, (HANDLE)NULL);
  }
  _lclose(hfErrorLog);
  
  TMSHeapFree(pNODESUBSET);


  return(bFinishedOK);
}

