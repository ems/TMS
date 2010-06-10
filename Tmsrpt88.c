// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the ITS4mobility AVL System
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "ITS4mobility Interface"

BOOL FAR TMSRPT88(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef  GTResults;
  BLOCKSDef       *pTRIPSChunk;
  double  prevLat, prevLon;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  float   distanceToHere;
  float   tripDistances[100];
  float   distance;
  struct  tm today;
  time_t  now;
  struct  tm tmED, tmUD;
  time_t  EDmktime;
  time_t  UDmktime;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bFirst;
  BOOL  bFound;
  BOOL  bAllGeocoded;
  long  year, month, day;
  long  serviceRecordID;
  long  timeAtStop;
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  lastTripNumber;
  long  maxTrips;
  long  startTime;
  long  deadheadTime;
  long  stopNumber;
  long  lastTimepoint;
  char  outputString[1024];
  char  outputStringSave[1024];
  char *ptr;
  char *pszReportName;
  char  outputFileName[64];
  char  dummy[256];
  int   nI;
  int   rcode2;
  int   tripIndex;
  int   seq;

  bFinishedOK = FALSE;
//
//  Maintain the distance measure
//
  distanceMeasureSave = distanceMeasure;
  distanceMeasure = PREFERENCES_MILES;
//
//  Test for geocode
//
  bAllGeocoded = FALSE;
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.longitude == 0.0 || NODES.latitude == 0.0)
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      sprintf(outputString, "Node \"%s\" has not been geocoded", tempString);
      MessageBeep(MB_ICONINFORMATION);
      strcat(outputString, "\n\nThe following fields will be unloaded as 0:00:00:\n");
      strcat(outputString, "Point in Journey: \"Departure time\" (interpolated stop times)\n");
      MessageBox(NULL, outputString, TMS, MB_ICONINFORMATION | MB_OK);
      bAllGeocoded = FALSE;
      break;      
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
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
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    goto done;
  }
  bGotError = FALSE;
  pPassedData->nReportNumber = 88;
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
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  LoadString(hInst, nI, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  ====================================
//  Network Version - NetworkVersion.csv
//  ====================================
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "NetworkVersion.csv");
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
  GetYMD(effectiveDate, &year, &month, &day);
//
//  1 - 'NETWORKVERSION'
//  2 - Database description
//  3 - Effective date
//  4 - '1'
//
  sprintf(outputString, "'NETWORKVERSION';'%s';%ld-%02ld-%02ld;1;\r\n", 
        szDatabaseDescription, year, month, day);
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==================================================
//  Network Version Date File - NetworkVersionDate.csv
//  ==================================================
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "NetworkVersionDate.csv");
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
//  1 - 'NETWORKVERSIONDATE'
//  2 - Today's date
//  3 - '1'
//
  time(&now);
  today = *localtime(&now);
  sprintf(outputString, "'NETWORKVERSIONDATE';%4d-%02d-%02d;1;\r\n",
        today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================
//  Municipality File - Municipality.csv
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Municipality.csv");
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
//  Cycle through the jurisdictions
//
  rcode2 = btrieve(B_GETFIRST, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
    trim(tempString, JURISDICTIONS_NAME_LENGTH);
//
//  1 - 'MUNICIPALITY'
//  2 - Jurisdiction recordID
//  3 - Jurisdiction name
//
    sprintf(outputString, "'MUNICIPALITY';%ld;'%s';\r\n", JURISDICTIONS.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =============================
//  Stop Area File - StopArea.csv
//  =============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "StopArea.csv");
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
    strcpy(outputString, "'STOPAREA';");
//
//  StopAreaID (NODES.recordID)
//
    sprintf(tempString, "%ld;", NODES.recordID);
    strcat(outputString, tempString);
//
//  Name (NODES.description)
//
    strncpy(tempString, NODES.intersection, NODES_DESCRIPTION_LENGTH);
    trim(tempString, NODES_DESCRIPTION_LENGTH);
    ptr = strstr(tempString, "&&");  // Get rid of the MapInfo convention
    if(ptr)
    {
      *ptr = 'a';
      *(ptr + 1) = 't';
    }
    sprintf(szarString, "'%s';", tempString);
    strcat(outputString, szarString);
//
//  MunicipalityID (NODES.JURISDICTIONSrecordID)
//
    sprintf(tempString, "%ld;", NODES.JURISDICTIONSrecordID);
    strcat(outputString, tempString);
//
//  ShortName Was NODES.abbrName, now NODES.longName
//
    strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
    trim(tempString, NODES_LONGNAME_LENGTH);
    sprintf(szarString, "'%s';\r\n", tempString);
    strcat(outputString, szarString);
//
//  Write it out and get the next node
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===============================
//  Stop Point File - StopPoint.csv
//  ===============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "StopPoint.csv");
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
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strcpy(outputString, "'STOPPOINT';");
//
//  StopAreaID (NODES.recordID)
//
      sprintf(tempString, "%ld;", NODES.recordID);
      strcat(outputString, tempString);
//
//  StopPointID (NODES.recordID)
//
      sprintf(tempString, "%ld;", NODES.recordID);
      strcat(outputString, tempString);
//
//  PointID (NODES.recordID)
//
      sprintf(tempString, "%ld;", NODES.recordID);
      strcat(outputString, tempString);
//
//  Name (NODES.description)
//
      strncpy(tempString, NODES.intersection, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      ptr = strstr(tempString, "&&");  // Get rid of the MapInfo convention
      if(ptr)
      {
        *ptr = 'a';
        *(ptr + 1) = 't';
      }
      sprintf(szarString, "'%s';", tempString);
      strcat(outputString, szarString);
//
//  StopPointNumber (not used)
//
      strcat(outputString, "'';");
//
//  ExternalID (NODES.abbrName)
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      sprintf(szarString, "'%s';", tempString);
      strcat(outputString, szarString);
//
//  ShortName Was NODES.abbrName, now NODES.longName.
//
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      sprintf(szarString, "'%s';\r\n", tempString);
      strcat(outputString, szarString);
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
//  ======================
//  Point File - Point.csv
//  ======================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Point.csv");
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
    strcpy(outputString, "'POINT';");
//
//  PointID (NODES.recordID)
//
    sprintf(tempString, "%ld;", NODES.recordID);
    strcat(outputString, tempString);
//
//  Description (NODES.description)
//
    strncpy(tempString, NODES.intersection, NODES_DESCRIPTION_LENGTH);
    trim(tempString, NODES_DESCRIPTION_LENGTH);
    ptr = strstr(tempString, "&&");  // Get rid of the MapInfo convention
    if(ptr)
    {
      *ptr = 'a';
      *(ptr + 1) = 't';
    }
    sprintf(szarString, "'%s';", tempString);
    strcat(outputString, szarString);
//
//  Longitude (NODES.longitude)
//
    sprintf(tempString, "%12.8f", NODES.longitude);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "'%s';", ptr);
    strcat(outputString, szarString);
//
//  Latitude (NODES.latitude)
//
    sprintf(tempString, "%12.8f", NODES.latitude);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "'%s';", ptr);
    strcat(outputString, szarString);
//
//  Altitude (not used)
//
    strcat(outputString, ";\r\n");
//
//  Write it out and get the next node
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================
//  Destination File - Destination.csv
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Destination.csv");
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
//  Cycle through the directions
//
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(szarString, DIRECTIONS_ABBRNAME_LENGTH);
//
//  1 - 'DESTINATION'
//  2 - Directions recordID
//  3 - Directions name
//  4 - Directions short name
//
    sprintf(outputString, "'DESTINATION';%ld;'%s';'%s';\r\n", DIRECTIONS.recordID, tempString, szarString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==============================
//  Direction File - Direction.csv
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Direction.csv");
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
//  Cycle through the directions
//
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
//
//  1 - 'DIRECTION'
//  2 - Directions recordID
//  3 - Directions name
//
    sprintf(outputString, "'DIRECTION';%ld;'%s';\r\n", DIRECTIONS.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==============================
//  Timetable File - Timetable.csv
//  ==============================
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Timetable.csv");
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
  GetYMD(effectiveDate, &year, &month, &day);
//
//  1 - 'TIMETABLE'
//  2 - TimetableID Was: 1, now: SERVICES.recordID
//  3 - Was: Database file name, SERVICES.name
//  4 - CompanyID
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    sprintf(outputString, "'TIMETABLE';%ld;'%s';1;\r\n", 
          SERVICES.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ============================
//  Calendar File - Calendar.csv
//  ============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Calendar.csv");
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
//  Dump out the calendar days
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
//  Figure out the service day
//
      nI = (tmED.tm_wday == 0 ? 6 : tmED.tm_wday - 1);
//
//  Create the record
//
      sprintf(outputString, "'CALENDAR';%d-%02d-%02d;%ld;\r\n",
            tmED.tm_year + 1900, tmED.tm_mon + 1, tmED.tm_mday, ROSTERPARMS.serviceDays[nI]);
//
//  Write out the record
//
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
//  ==========================
//  Journey File - Journey.csv
//  ==========================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Journey.csv");
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
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Loop through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the trips for this route/ser/dir
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
//
//  CompanyID (1)
//  LineID (ROUTES.recordID)
//  JourneyNumber (TRIPS.tripNumber)
//  DirectionID (ROUTES.DIRECTIONSrecordID[nI])
//  DestinationID (ROUTES.DIRECTIONSrecordID[nI])
//
          sprintf(outputStringSave, "'JOURNEY';1;%ld;%ld;%ld;%ld;",
                ROUTES.recordID, TRIPS.tripNumber,
                ROUTES.DIRECTIONSrecordID[nI], 
                ROUTES.DIRECTIONSrecordID[nI]);
//
//  Generate the trip
//           
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Does the trip pull out?  If so, dump out an extra record.
//
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
//
//  Only do this if we had the deadhead
//
            if(deadheadTime != NO_TIME)
            {
              strcpy(outputString, outputStringSave);
//
//  'DEADRUN'
//  JourneyID (not used)
//  JourneyPatternID (not used)
//  TimetableID Was 1, now SERVICES.recordID;
//
              sprintf(tempString, "'DEADRUN';;;%ld;", TRIPS.SERVICESrecordID);
              strcat(outputString, tempString);
//
//  Start time
//  Description (not used)
//
              startTime = GTResults.firstNodeTime - deadheadTime;
              sprintf(tempString, "%02ld:%02ld:%02ld;;\r\n", startTime / 3600, (startTime % 3600) / 60, startTime % 60);
              strcat(outputString, tempString);
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
          }
//
//  Regular trip data
//
//
//  'NORMAL'
//  JourneyID (not used)
//  JourneyPatternID (not used)
//  TimetableID Was 1, now SERVICES.recordID
//
          strcpy(outputString, outputStringSave);
          sprintf(tempString, "'NORMAL';;;%ld;", SERVICES.recordID);
          strcat(outputString, tempString);
//
//  Start time
//  Description (not used)
//
          startTime = GTResults.firstNodeTime;
          sprintf(tempString, "%02ld:%02ld:%02ld;;\r\n", startTime / 3600, (startTime % 3600) / 60, startTime % 60);
          strcat(outputString, tempString);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Does the trip pull in?  If so, dump out an extra record.
//
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
//
//  Only do this if we had the deadhead
//
            if(deadheadTime != NO_TIME)
            {
              strcpy(outputString, outputStringSave);
//
//  'DEADRUN'
//  JourneyID (not used)
//  JourneyPatternID (not used)
//  TimetableID Was 1, now SERVICES.recordID
//
              sprintf(tempString, "'DEADRUN';;;%ld;", SERVICES.recordID);
              strcat(outputString, tempString);
//
//  Start time (which is really the last time on the trip)
//  Description (not used)
//
              startTime = GTResults.lastNodeTime;
              sprintf(tempString, "%02ld:%02ld:%02ld;;\r\n", startTime / 3600, (startTime % 3600) / 60, startTime % 60);
              strcat(outputString, tempString);
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
          }
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        } // while on trips
      } // nI (directions on the route)
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ========================================
//  PointInJourney File - PointInJourney.csv
//  ========================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "PointInJourney.csv");
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
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Loop through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the trips for this route/ser/dir
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
//
//  CompanyID (1)
//  LineID (ROUTES.recordID)
//  JourneyNumber (TRIPS.tripNumber)
//
          sprintf(outputStringSave, "'POINTINJOURNEY';1;%ld;%ld;",
                ROUTES.recordID, TRIPS.tripNumber);
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
          if(GTResults.tripDistance != 0.0)
          {
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nI;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            distanceToHere = 0.0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nI &&
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
                if(bAllGeocoded)
                {
                  distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
                }
              }
              if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
              {
              }
              else
              {
                tripDistances[tripIndex] = distanceToHere;
                tripIndex++;
              }
              prevLat = NODES.latitude;
              prevLon = NODES.longitude;
              lastTimepoint = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
//
//  Pass 2 - Determine time interpolations and dump out the records
//
          seq = 1;
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripIndex = 0;
          stopNumber = 1;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nI &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(NODES.flags & NODES_FLAG_STOP)
            {
              timeAtStop = 0;
              if(GTResults.tripDistance == 0)
              {
                if(bAllGeocoded)
                {
                  timeAtStop = GTResults.tripTimes[tripIndex - 1];
                }
              }
              else
              {
                if(bAllGeocoded)
                {
                  distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
                  timeAtStop = (long)((GTResults.tripTimes[tripIndex] - GTResults.tripTimes[tripIndex - 1]) *
                        (distanceToHere / (tripDistances[tripIndex] - tripDistances[tripIndex - 1])));
                  timeAtStop += (GTResults.tripTimes[tripIndex - 1] - GTResults.firstNodeTime);
                }
              }
            }
            else
            {
              timeAtStop = GTResults.tripTimes[tripIndex];
              tripIndex++;  
              if(bAllGeocoded)
              {
                distanceToHere = 0.0;
              }
            }
//
//  Sequence in Journey (seq)
//  StopPointID (PATTERNS.NODESrecordID)
//  Departure Time (timeAtStop)
//  Arrival Time (not used)
//  Minimum wait time (not used)
//  Timing point (yes,no - (NODES.flags & NODES_FLAG_STOP))
//  LinkID (not used)
//  JourneyID (not used)
//  Destination1 (not used)
//  Destination2 (not used)
//  Via1 (not used)
//  Via2 (not used)
//  TimetableID Was 1, now SERVICES.recordID;
//
            strcpy(outputString, outputStringSave);
            if(timeAtStop <= 0)
            {
              strcpy(szarString, "");
            }
            else
            {
              sprintf(szarString, "%02ld:%02ld:%02ld", timeAtStop / 3600, (timeAtStop % 3600) / 60, timeAtStop % 60);
            }
            sprintf(tempString, "%d;%ld;%s;%s;;%s;;;;;;;%ld;\r\n",
                  seq++, PATTERNS.NODESrecordID,
                  (timeAtStop == GTResults.lastNodeTime ? "" : szarString),
                  (timeAtStop == GTResults.lastNodeTime ? szarString : ""),
                  ((NODES.flags & NODES_FLAG_STOP) ? "no" : "yes"), SERVICES.recordID);
            strcat(outputString, tempString);
            _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next pattern record
//
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        } // while on trips
      } // nI (directions on the route)
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ======================
//  Block File - Block.csv
//  ======================
//
  pTRIPSChunk = &TRIPS.standard;
  strcpy(outputFileName, "Block.csv");
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
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Get all the blocks on this route group
//
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      bFirst = TRUE;
      while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
      {
//
//  Cycle through the blocks
//
        assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
        TRIPSKey2.blockNumber = NO_RECORD;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SERVICES.recordID)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
//
//  TimetableID Was 1, now SERVICES.recordID
//  CompanyID (1)
//  BlockName (TRIPS.standard.blockNumber)
//  BlockID (not used)
//
          sprintf(tempString, "%ld", TRIPS.standard.blockNumber);
          sprintf(outputString, "'BLOCK';%ld;1;'%s';;\r\n", SERVICES.recordID, tempString);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next block
//
          assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
          TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
          TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
          TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
          TRIPSKey2.blockNumber = TRIPS.standard.blockNumber + 1;
          TRIPSKey2.blockSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        }  // while blocks on this garage node

//
//  Get the next garage node
//
        if(bFirst)
        {
          bFirst = FALSE;
        }
        else
        {
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
        }
      }  // while cycling through garage nodes
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==========================================
//  Journey in Block File - JourneyInBlock.csv
//  ==========================================
//
  pTRIPSChunk = &TRIPS.standard;
  strcpy(outputFileName, "JourneyInBlock.csv");
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
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Get all the blocks on this route group
//
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      bFirst = TRUE;
      while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
      {
//
//  Cycle through the blocks
//
        assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
        TRIPSKey2.blockNumber = NO_RECORD;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        blockNumber = NO_RECORD;
//
//  Get the first trip on the block
//
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SERVICES.recordID)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(pTRIPSChunk->blockNumber != blockNumber)
          {
            seq = 1;
            blockNumber = pTRIPSChunk->blockNumber;
          }
//
//  TimetableID Was 1, now SERVICES.recordID
//  CompanyID (1)
//  Block name (TRIPS.standard.blockNumber)
//  Seq in Block (seq)
//  LineID (TRIPS.ROUTESrecordID)
//  JourneyNumber (TRIPS.tripNumber)
//  JourneyID (not used)
//  BlockID (not used)
//
          sprintf(tempString, "%ld", TRIPS.standard.blockNumber);
          sprintf(outputString, "'JOURNEYINBLOCK';%ld;1;'%s';%d;%ld;%ld;;;\r\n",
                SERVICES.recordID, tempString, seq, TRIPS.ROUTESrecordID, TRIPS.tripNumber);
//
//  Write it
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          seq++;
        }  // while blocks on this garage node

//
//  Get the next garage node
//
        if(bFirst)
        {
          bFirst = FALSE;
        }
        else
        {
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
        }
      }  // while cycling through garage nodes
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ======================
//  Routes File - Line.csv
//  ======================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Line.csv");
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
//  Don't do RGRP routes
//
    if(!(ROUTES.flags & ROUTES_FLAG_RGRP))
    {
//
//  CompanyID (1)
//  Line Name *Changed from ROUTES.name to ROUTES.number 14-Apr-10
//  Line ID (ROUTES.recordID)
//  External ID (ROUTES.number)
//  Description (ROUTES.name)
//
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szarString, ROUTES_NUMBER_LENGTH);
      ptr = szarString;
      while(ptr && *ptr == ' ')
      {
        ptr++;
      }
      sprintf(outputString, "'LINE';1;'%s';%ld;'%s';'%s - %s';\r\n",
            ptr, ROUTES.recordID, ptr, ptr, tempString);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==========================
//  Company File - Company.csv
//  ==========================
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Company.csv");
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
  GetYMD(effectiveDate, &year, &month, &day);
//
//  1 - 'COMPANY'
//  2 - TimetableID (1)
//  3 - Database file name
//  4 - CompanyID
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    sprintf(outputString, "'COMPANY';'%s';%ld;'%s';\r\n", 
        szPropertyName, SERVICES.recordID, szPropertyName);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
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
