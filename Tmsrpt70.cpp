// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the Continental AVL System
//
#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
}  // extern "C"

int  TMSRPT70Ex(TMSRPTPassedDataDef *);

extern "C" {
BOOL FAR TMSRPT70(TMSRPTPassedDataDef *pPassedData)
{
  return TMSRPT70Ex(pPassedData);
}
} // extern "C"

#include "TMS.h"
#include <math.h>


int CompassDirection(double aLong1, double aLat1, double aLong2, double aLat2)
{
  if(aLong1 == aLong2 && aLat1 == aLat2)
  {
    return(0);
  }
//
//  Returns a compass direction from 0 to 360 where:
//    * North (N): 0° = 360°'
//    * East (E): 90°'
//    * South (S): 180°'
//    * West (W): 270°'
//    * Northeast (NE), 45°, halfway between north and east, is the opposite of southwest.
//    * Southeast (SE), 135°, halfway between south and east, is the oposite of northwest.
//    * Southwest (SW), 225°, halfway between south and west, is the opposite of northeast.
//    * Northwest (NW), 315°, halfway between north and west, is the opposite of southeast.
//
#define PI 3.14159265358979323846
#define toRadians (PI / 180.0)
#define toDegrees (180.0 / PI)

  double dLong, y, x, radians;
  int bearing;
//
// Convert lat/long to radians.
//
  aLong1 *= toRadians;
  aLat1  *= toRadians;
  aLong2 *= toRadians;
  aLat2  *= toRadians;
  dLong = (aLong2 - aLong1);
  y = sin(dLong) * cos(aLat2);
  x = cos(aLat1)*sin(aLat2) - sin(aLat1)*cos(aLat2)*cos(dLong);
  radians = atan2( y, x );
  if(radians < 0.0)
  {
    radians += 2.0 * PI;
  }
  bearing = (int)(radians * toDegrees + 0.5);

  return bearing;
}

 

//
//  ILG Recordset includes
//
#include "ILGBlock.h"
#include "ILGPattern.h"
#include "ILGRoster.h"
#include "ILGRouteDirection.h"
#include "ILGRoutes.h"
#include "ILGRouteStops.h"
#include "ILGRouteTimePoints.h"
#include "ILGRuns.h"
#include "ILGServiceType.h"
#include "ILGStops.h"
#include "ILGTimePoints.h"
#include "ILGTimeTable.h"
#include "ILGTrips.h"
#include "ILGTripStops.h"
#include "ILGVersionInfo.h"

#define TMSRPT70_MAXSERVICES      25
#define TMSRPT70_MAXROUTES       200
#define TMSRPT70_MAXPATTERNNODES 100
#define TMSRPT70_MAXTREQ        2000
#define TMSRPT70_MAXNEQ           50
#define TMSRPT70_DATABASE_FILENAME "ILGInterface.mdb"

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "Continental Interface"

BOOL FAR TMSRPT70Ex(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef      GTResults;
  CODBCFieldInfo FI;
  CDatabase      db;
  BLOCKSDef     *pTRIPSChunk;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  CString fromFile;
  CString s;
  float   prevLat, prevLon;
  float   distanceToHere;
  float   tripDistances[100];
  float   distanceFromLast;
  float   distance;
  HFILE   hfErrorLog;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bFirst;
  BOOL  bAllGeocoded;
  long  year, month, day;
  long  serviceRecordID;
  long  previousPattern;
  long  timeAtStop;
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  fromTime;
  long  toTime;
  long  dhd;
  long  lastTripNumber;
  long  recordID;
  char  outputString[1024];
  char *pszReportName;
  char  dummy[256];
  int   nI, nJ;
  int   rcode2;
  int   tripIndex;
  int   numRecords;
  int   maxLength[50];
  int   stopNumber;
  int   seq;
  int   numPieces;

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
      strcat(outputString, "\n\nThe following fields will be unloaded as 0:\n");
      strcat(outputString, "  Route stops table: \"Compass Direction\" and \"Distance from Last\"\n");
      strcat(outputString, "   Trip stops table: \"Crossing time in minutes (interpolated stop times)\"\n");
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
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  s.LoadString(nI);
  sprintf(tempString, s, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  Copy the empty data file into the output folder
//
  fromFile = szInstallDirectory;
  fromFile += "\\FRESH\\";
  fromFile += TMSRPT70_DATABASE_FILENAME;
  CopyFile(fromFile, TMSRPT70_DATABASE_FILENAME, FALSE);
//
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    chdir("..");  // Back to where we started from
    return(FALSE);
  }
  bGotError = FALSE;
  pPassedData->nReportNumber = 69;
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
//  Verify that all trips in the system are numbered
//
  rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
  StatusBarText("Checking if all trips are numbered");
  rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  nI = 0;
  bFound = FALSE;
  lastTripNumber = NO_RECORD;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      chdir("..");  // Back to where we started from
      return(FALSE);
    }
    StatusBar((long)nI, (long)numRecords);
    if(TRIPS.tripNumber <= 0)
    {
      MessageBeep(MB_ICONQUESTION);
      LoadString(hInst, ERROR_344, tempString, TEMPSTRING_LENGTH);
      if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OK) != IDOK)
      {
        chdir("..");  // Back to where we started from
        return(FALSE);
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
        chdir("..");  // Back to where we started from
        return(FALSE);
      }
      StatusBar((long)nI, (long)numRecords);
      TRIPS.tripNumber = (long)nI;
      nI++;
      rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
    lastTripNumber = nI;
  }
//
//  Tell the registry where the database is
//
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS to ILG";
  HKEY  hKey;
  char  szDir[_MAX_PATH];
  long  result;

  result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
  if(result != 0)
  {
    StatusBarEnd();
    s.LoadString(ERROR_240);
    sprintf(tempString, s, result);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, tempString, TMS, MB_OK);
    chdir("..");  // Back to where we started from
    return(FALSE);
  }

  DWORD type = REG_SZ;

  _getcwd(szDir, sizeof(szDir));
  strcat(szDir, "\\");
  strcat(szDir, TMSRPT70_DATABASE_FILENAME);
  RegSetValueEx(hKey, "DBQ", 0, type, (LPBYTE)szDir, strlen(szDir));
//
//  Open the empty database
//
  db.OpenEx("DSN=TMS to ILG", CDatabase::noOdbcDialog );
  int xx = db.CanUpdate();
//
//  Create the recordsets
//
  CILGBlock           ILGBlock(&db);
  CILGPattern         ILGPattern(&db);
  CILGRoster          ILGRoster(&db);
  CILGRouteDirection  ILGRouteDirection(&db);
  CILGRoutes          ILGRoutes(&db);
  CILGRouteStops      ILGRouteStops(&db);
  CILGRouteTimePoints ILGRouteTimePoints(&db);
  CILGRuns            ILGRuns(&db);
  CILGServiceType     ILGServiceType(&db);
  CILGStops           ILGStops(&db);
  CILGTimePoints      ILGTimePoints(&db);
  CILGTimeTable       ILGTimeTable(&db);
  CILGTrips           ILGTrips(&db);
  CILGTripStops       ILGTripStops(&db);
  CILGVersionInfo     ILGVersionInfo(&db);
//
//  ...and the effective and until dates
//
  GetYMD(effectiveDate, &year, &month, &day);
  CTime AD((int)year, (int)month, (int)day, 0, 0, 0, 0);
  GetYMD(untilDate, &year, &month, &day);
  CTime DD((int)year, (int)month, (int)day, 0, 0, 0, 0);
//
//  ================
//  TimePoints Table
//  ================
//
//  Get the number of nodes in the table
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Open the recordset and get the field widths
//
  ILGTimePoints.Open();
  for(nI = 0; nI < (int)ILGTimePoints.m_nFields; nI++)
  {
    ILGTimePoints.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], NODES_ABBRNAME_LENGTH);      // TimePointAbbr
  maxLength[1] = min(maxLength[1], NODES_DESCRIPTION_LENGTH);   // TimePointName
//
//  Cycle through the nodes
//
  s.LoadString(TEXT_385);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      ILGTimePoints.AddNew();
//
//  TimePointAbbr
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      trim(tempString, maxLength[0]);
      ILGTimePoints.m_TimePointAbbr = tempString;
//
//  TimePointName
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      trim(tempString, maxLength[1]);
      ILGTimePoints.m_TimePointName = tempString;
//
//  Insert
//
      ILGTimePoints.Update();
      ILGTimePoints.MoveNext();
    }
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
  ILGTimePoints.Close();
//
//  ===========
//  Stops Table
//  ===========
//
//  Open the recordset and get the field widths
//
  ILGStops.Open();
  for(nI = 0; nI < (int)ILGStops.m_nFields; nI++)
  {
    ILGStops.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], NODES_LONGNAME_LENGTH);       // StopAbbr
  maxLength[1] = min(maxLength[1], NODES_DESCRIPTION_LENGTH);    // StopName
  maxLength[2] = maxLength[2];                                   // Latitude
  maxLength[3] = maxLength[3];                                   // Longtidue
//
//  Cycle through the nodes
//
  s.LoadString(TEXT_386);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
    if(NODES.flags & NODES_FLAG_STOP)
    {
      ILGStops.AddNew();
//
//  StopAbbr (actually, the longName)
//
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      trim(tempString, maxLength[0]);
      ILGStops.m_StopAbbr = tempString;
//
//  StopName
//
      strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(tempString, NODES_DESCRIPTION_LENGTH);
      trim(tempString, maxLength[1]);
      ILGStops.m_StopName = tempString;
//
//  Latitude
//
//      if(NODES.latitude != 0.0)
//      {
        sprintf(tempString, "%f12.7", NODES.latitude);
        trim(tempString, maxLength[2]);
        ILGStops.m_Latitude = tempString;
//      }
//
//  Longitude
//
//      if(NODES.longitude != 0.0)
//      {
        sprintf(tempString, "%f12.7", NODES.longitude);
        trim(tempString, maxLength[2]);
        ILGStops.m_Longitude = tempString;
//      }
//
//  Insert
//
      ILGStops.Update();
      ILGStops.MoveNext();
    }
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
  ILGStops.Close();
//
//  ====================
//  RouteDirection Table
//  ====================
//
//  Get the number of directions in the table
//
  rcode2 = btrieve(B_STAT, TMS_DIRECTIONS, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Open the recordset and get the field widths
//
  ILGRouteDirection.Open();
  for(nI = 0; nI < (int)ILGRouteDirection.m_nFields; nI++)
  {
    ILGRouteDirection.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], DIRECTIONS_ABBRNAME_LENGTH);    // RouteDirectionAbbr
  maxLength[1] = min(maxLength[1], DIRECTIONS_LONGNAME_LENGTH);    // RouteDirectionName
//
//  Cycle through the directions
//
  s.LoadString(TEXT_387);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
    ILGRouteDirection.AddNew();
//
//  RouteDirectionAbbr
//
    strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
    trim(tempString, maxLength[0]);
    ILGRouteDirection.m_RouteDirectionAbbr = tempString;
//
//  RouteDirectionName
//
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, maxLength[1]);
    ILGRouteDirection.m_RouteDirectionName = tempString;
//
//  Insert
//
    ILGRouteDirection.Update();
    ILGRouteDirection.MoveNext();
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  }
  ILGRouteDirection.Close();
//
//  ============
//  Routes Table
//  ============
//
//  Open the recordset and get the field widths
//
  ILGRoutes.Open();
  for(nI = 0; nI < (int)ILGRoutes.m_nFields; nI++)
  {
    ILGRoutes.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], ROUTES_NUMBER_LENGTH);     // RouteAbbr
  maxLength[1] = min(maxLength[1], ROUTES_NAME_LENGTH);       // RouteName
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  s.LoadString(TEXT_388);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
    ILGRoutes.AddNew();
//
//  RouteAbbr
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    trim(tempString, maxLength[0]);
    ILGRoutes.m_RouteAbbr = tempString;
//
//  RouteName
//
    strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(tempString, ROUTES_NAME_LENGTH);
    trim(tempString, maxLength[1]);
    ILGRoutes.m_RouteName = tempString;
//
//  Insert
//
    ILGRoutes.Update();
    ILGRoutes.MoveNext();
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  ILGRoutes.Close();
//
//  =============
//  Pattern Table
//  =============
//
//  Open the recordset and get the field widths
//
  ILGPattern.Open();
  for(nI = 0; nI < (int)ILGPattern.m_nFields; nI++)
  {
    ILGPattern.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], PATTERNNAMES_NAME_LENGTH);        // PatternAbbr
  maxLength[1] = min(maxLength[1], ROUTES_NUMBER_LENGTH);            // RouteAbbr
  maxLength[2] = min(maxLength[2], DIRECTIONS_ABBRNAME_LENGTH);      // RouteDirectionAbbr
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  s.LoadString(TEXT_389);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the directions for this route
//
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
        previousPattern = NO_RECORD;
        for(;;)
        {
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = previousPattern;
          PATTERNSKey2.nodeSequence = 9999999;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 != 0 || PATTERNS.ROUTESrecordID != ROUTES.recordID ||
                PATTERNS.SERVICESrecordID != SERVICES.recordID || PATTERNS.directionIndex != nJ)
          {
            break;
          }
          previousPattern = PATTERNS.PATTERNNAMESrecordID;

          ILGPattern.AddNew();
//
//  PatternAbbr (which has the service number prepended to the name)
//
          PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          trim(szarString, PATTERNNAMES_NAME_LENGTH);
          sprintf(tempString, "%ld%s", SERVICES.number, szarString);
          trim(tempString, maxLength[0]);
          ILGPattern.m_PatternAbbr = tempString;
//
//  RouteAbbr
//
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          trim(tempString, maxLength[1]);
          ILGPattern.m_RouteAbbr = tempString;
//
//  RouteDirectionAbbr
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, maxLength[2]);
          ILGPattern.m_RouteDirectionAbbr = tempString;
//
//  Insert
//
          ILGPattern.Update();
          ILGPattern.MoveNext();
        }  // for(;;)
      }  // nJ
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }  // while services
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  ILGPattern.Close();
//
//  ================
//  RouteStops Table
//  ================
//
//  Open the recordset and get the field widths
//
  ILGRouteStops.Open();
  for(nI = 0; nI < (int)ILGRouteStops.m_nFields; nI++)
  {
    ILGRouteStops.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], ROUTES_NUMBER_LENGTH);        // RouteAbbr
  maxLength[1] = min(maxLength[1], DIRECTIONS_ABBRNAME_LENGTH);  // RouteDirectionAbbr
  maxLength[2] = min(maxLength[2], PATTERNNAMES_NAME_LENGTH);    // PatternAbbr
  maxLength[3] = maxLength[3];                                   // StopNumber
  maxLength[4] = min(maxLength[4], NODES_LONGNAME_LENGTH);       // StopAbbr
  maxLength[5] = min(maxLength[5], NODES_ABBRNAME_LENGTH);       // TimePointAbbr
  maxLength[6] = maxLength[6];                                   // DistanceFromLast
  maxLength[7] = maxLength[7];                                   // CompassDirection
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  long OBBayNODESrecordID;
  long IBBayNODESrecordID;
  long absRecID;

  s.LoadString(TEXT_390);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    OBBayNODESrecordID = (long)ROUTES.OBBayNODESrecordID;
    IBBayNODESrecordID = (long)ROUTES.IBBayNODESrecordID;
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the directions for this route
//
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nJ;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        previousPattern = NO_RECORD;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nJ)
        {
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          if(PATTERNS.PATTERNNAMESrecordID != previousPattern)
          {
            stopNumber = 1;
            distanceFromLast = 0;
            if(previousPattern != NO_RECORD)
            {
              ILGRouteStops.Update();
              ILGRouteStops.MoveNext();
            }
          }
          else
          {
//
//  CompassDirection
//
            if(bAllGeocoded)
            {
              ILGRouteStops.m_CompassDirection =
                    CompassDirection((double)prevLon, (double)prevLat, (double)NODES.longitude, (double)NODES.latitude);
            }
            else
            {
              ILGRouteStops.m_CompassDirection = 0;
            }
            ILGRouteStops.Update();
            ILGRouteStops.MoveNext();
            stopNumber++;
            if(bAllGeocoded)
            {
              distanceFromLast = (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
            }
          }
          previousPattern = PATTERNS.PATTERNNAMESrecordID;
          prevLat = NODES.latitude;
          prevLon = NODES.longitude;

          ILGRouteStops.AddNew();
//
//  RouteAbbr
//
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          trim(tempString, maxLength[0]);
          ILGRouteStops.m_RouteAbbr = tempString;
//
//  RouteDirectionAbbr
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, maxLength[1]);
          ILGRouteStops.m_RouteDirectionAbbr = tempString;
//
//  PatternAbbr (which has the service number prepended to the name)
//
          PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          trim(szarString, PATTERNNAMES_NAME_LENGTH);
          sprintf(tempString, "%ld%s", SERVICES.number, szarString);
          trim(tempString, maxLength[2]);
          ILGRouteStops.m_PatternAbbr = tempString;
//
//  StopNumber
//
          ILGRouteStops.m_StopNumber = stopNumber;
//
//  StopAbbr (actually, the longName)
//
//  If it's not a stop, then get the associated stop from the number field
//
          if(NODES.flags & NODES_FLAG_STOP)
          {
            strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          }
          else
          {
            strcpy(tempString, "");
            if(NODES.reliefLabels[0] == 'T')
            {
              btrieve(B_GETPOSITION, TMS_NODES, &absRecID, &NODESKey0, 0);
              NODESKey0.recordID = (nJ == 0 ? OBBayNODESrecordID : IBBayNODESrecordID);
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
              NODES.recordID = absRecID;
              btrieve(B_GETDIRECT, TMS_NODES, &NODES, &NODESKey0, 0);
            }
            else
            {
              if(NODES.number > 0)
              {
                sprintf(tempString, "Stop%04d", NODES.number);
              }
              else
              {
                if(nJ == 0)
                {
                  if(NODES.OBStopNumber > 0)
                  {
                    sprintf(tempString, "Stop%04d", NODES.OBStopNumber);
                  }
                }
                else
                {
                  if(NODES.IBStopNumber > 0)
                  { 
                    sprintf(tempString, "Stop%04d", NODES.IBStopNumber);
                  }
                }
              }
            }
          }
          trim(tempString, maxLength[4]);
          ILGRouteStops.m_StopAbbr = tempString;
//
//  TimePointAbbr
//
          if(!(NODES.flags & NODES_FLAG_STOP))
          {
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            trim(tempString, maxLength[5]);
            ILGRouteStops.m_TimePointAbbr = tempString;
          }
//
//  DistanceFromLast
//
          sprintf(tempString, "%ld", (long)(distanceFromLast * 5280));
          trim(tempString, maxLength[6]);
          ILGRouteStops.m_DistanceFromLast = tempString;
//
//  Get the next pattern record
//
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while patterns
//
//  Insert
//
        if(previousPattern != NO_RECORD)
        {
          ILGRouteStops.Update();
          ILGRouteStops.MoveNext();
        }
      }  // nJ
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }  // while services
//
//  Get the next record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  ILGRouteStops.Close();
//
//  =====================
//  RouteTimePoints Table
//  =====================
//
//  Open the recordset and get the field widths
//
  ILGRouteTimePoints.Open();
  for(nI = 0; nI < (int)ILGRouteTimePoints.m_nFields; nI++)
  {
    ILGRouteTimePoints.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], ROUTES_NUMBER_LENGTH);       // RouteAbbr
  maxLength[1] = min(maxLength[1], DIRECTIONS_ABBRNAME_LENGTH); // RouteDirectionAbbr
  maxLength[2] = min(maxLength[2], NODES_ABBRNAME_LENGTH);      // TimePointAbbr
  maxLength[3] = maxLength[3];                                  // RouteSequence
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  s.LoadString(TEXT_391);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Go through the directions
//
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
      {
        continue;
      }

//
//  Go through the weekday BASE pattern
//
      PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
      PATTERNSKey2.SERVICESrecordID = 1;
      PATTERNSKey2.directionIndex = nJ;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      seq = 1;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == ROUTES.recordID &&
            PATTERNS.SERVICESrecordID == 1 &&
            PATTERNS.directionIndex == nJ &&
            PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(!(NODES.flags & NODES_FLAG_STOP))
        {
          ILGRouteTimePoints.AddNew();
//
//  RouteAbbr
//
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          trim(tempString, maxLength[0]);
          ILGRouteTimePoints.m_RouteAbbr = tempString;
//
//  RouteDirectionAbbr
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, maxLength[1]);
          ILGRouteTimePoints.m_RouteDirectionAbbr = tempString;
//
//  TimePointAbbr
//
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          trim(tempString, maxLength[2]);
          ILGRouteTimePoints.m_TimePointAbbr = tempString;
//
//  RouteSequence
//
          ILGRouteTimePoints.m_RouteSequence = (long)seq++;
//
//  Insert
//
          ILGRouteTimePoints.Update();
          ILGRouteTimePoints.MoveNext();
        }
//
//  Get the next pattern record
//
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }  // while patterns
    }  // nJ
//
//  Get the next route record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  ILGRouteTimePoints.Close();
//
//  =================
//  ServiceType Table
//  =================
//
//  Open the recordset and get the field widths
//
  ILGServiceType.Open();
  for(nI = 0; nI < (int)ILGServiceType.m_nFields; nI++)
  {
    ILGServiceType.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], SERVICES_NAME_LENGTH);       // ServiceAbbr
  maxLength[1] = min(maxLength[1], SERVICES_NAME_LENGTH);       // ServiceName
//
//  Get the number of services in the table
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the services
//
  s.LoadString(TEXT_392);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);

    ILGServiceType.AddNew();
//
//  ServiceAbbr
//
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    trim(tempString, maxLength[0]);
    ILGServiceType.m_ServiceAbbr = tempString;
//
//  ServiceName
//
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    trim(tempString, maxLength[1]);
    ILGServiceType.m_ServiceName = tempString;
//
//  Insert
//
    ILGServiceType.Update();
    ILGServiceType.MoveNext();
//
//  Get the next route record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
  ILGServiceType.Close();
//
//  ===========
//  Block Table
//  ===========
//
//  Open the recordset and get the field widths
//
  ILGBlock.Open();
  for(nI = 0; nI < (int)ILGBlock.m_nFields; nI++)
  {
    ILGBlock.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                   // BlockNum
  maxLength[1] = min(maxLength[1], NODES_ABBRNAME_LENGTH);       // BeginNodeAbbr
  maxLength[2] = min(maxLength[2], NODES_ABBRNAME_LENGTH);       // EndNodeAbbr
  maxLength[3] = maxLength[3];                                   // BeginTimeInTime
  maxLength[4] = maxLength[4];                                   // EndTimeInTime
  maxLength[5] = maxLength[5];                                   // BeginTimeInMins
  maxLength[6] = maxLength[6];                                   // EndTimeInMins
  maxLength[7] = min(maxLength[7], SERVICES_NAME_LENGTH);        // ServiceAbbr
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  m_bEstablishRUNTIMES = TRUE;
  s.LoadString(TEXT_393);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  pTRIPSChunk = &TRIPS.standard;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
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
        TRIPSKey2.blockNumber = 1;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        blockNumber = pTRIPSChunk->blockNumber;
//
//  Get the first trip on the block
//
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SERVICES.recordID)
        {
//
//  Gen the whole trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Pullout?
//
          if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
          {
            fromNODESrecordID = GTResults.firstNODESrecordID;
            fromTime = GTResults.firstNodeTime;
          }
          else
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
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd == NO_TIME)
            {
              dhd = 0;
            }
            fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            fromTime = GTResults.firstNodeTime - dhd;
          }
//
//  Get the last trip on the block
//
          blockNumber = pTRIPSChunk->blockNumber;
          TRIPSKey2.assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
          TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
          TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
          TRIPSKey2.blockNumber = blockNumber + 1;
          TRIPSKey2.blockSequence = NO_TIME;
          rcode2 = btrieve(B_GETLESSTHAN, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          if(rcode2 == 0 &&
                pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
                pTRIPSChunk->SGRPSERVICESrecordID == SERVICES.recordID &&
                pTRIPSChunk->blockNumber == blockNumber)
          {
//
// Gen the trip to establish the last node
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Pullin
//
            if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
            {
              toNODESrecordID = GTResults.lastNODESrecordID;
              toTime = GTResults.lastNodeTime;
            }
            else
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
              GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
              GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
              GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
              GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
              GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
              GCTData.timeOfDay = GTResults.lastNodeTime;
              dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(dhd == NO_TIME)
              {
                dhd = 0;
              }
              toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
              toTime = GTResults.lastNodeTime + dhd;
            }
//
//  Write out the data
//
            ILGBlock.AddNew();
//
//  BlockNum
//
            ILGBlock.m_BlockNum = blockNumber;
//
//  BeginNodeAbbr
//
            NODESKey0.recordID = fromNODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            trim(tempString, maxLength[1]);
            ILGBlock.m_BeginNodeAbbr = tempString;
//
//  EndNodeAbbr
//
            NODESKey0.recordID = toNODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            trim(tempString, maxLength[2]);
            ILGBlock.m_EndNodeAbbr = tempString;
//
//  BeginTimeInTime
//
//     Not unloaded
//
//  EndTimeInTime
//
//     Not unloaded
//
//  BeginTimeInMins
//
            ILGBlock.m_BeginTimeInMins = fromTime / 60;
//
//  EndTimeInMins
//
            ILGBlock.m_EndTimeInMins = toTime / 60;
//
//  ServiceName
//
            strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(tempString, SERVICES_NAME_LENGTH);
            trim(tempString, maxLength[7]);
            ILGBlock.m_ServiceAbbr = tempString;
//
//  Insert
//
            ILGBlock.Update();
            ILGBlock.MoveNext();
          }  // last trip on the block ok
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);  // Position to first trip of next block
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
//  Get the next service record
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }  // while services
//
//  Get the next route record
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  // while routes
  ILGBlock.Close();
//
//  ===========
//  Trips Table
//  ===========
//
//  Open the recordset and get the field widths
//
  ILGTrips.Open();
  for(nI = 0; nI < (int)ILGTrips.m_nFields; nI++)
  {
    ILGTrips.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                       // TripID
  maxLength[1] = maxLength[1];                                       // TripNum
  maxLength[2] = maxLength[2];                                       // BlockNum
  maxLength[3] = min(maxLength[3], SERVICES_NAME_LENGTH);            // ServiceAbr
  maxLength[4] = min(maxLength[4], ROUTES_NUMBER_LENGTH);            // RouteAbbr
  maxLength[5] = min(maxLength[5], DIRECTIONS_ABBRNAME_LENGTH);      // RouteDirectionAbbr
  maxLength[6] = min(maxLength[6], PATTERNNAMES_NAME_LENGTH);        // PatternAbbr
  maxLength[7] = maxLength[7];                                       // BlockSequence
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  s.LoadString(TEXT_394);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the directions for this route
//
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
//
//  Get the trips for this route/ser/dir
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nJ;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nJ)
        {
          ILGTrips.AddNew();
//
//  TripID (use recordID)
//
          ILGTrips.m_TripID = TRIPS.recordID;
//
//  TripNum
//
          ILGTrips.m_TripNum = TRIPS.tripNumber;
//
//  BlockNum
//
          ILGTrips.m_BlockNum = pTRIPSChunk->blockNumber;
//
//  ServiceAbbr
//
          strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
          trim(tempString, SERVICES_NAME_LENGTH);
          trim(tempString, maxLength[0]);
          ILGTrips.m_ServiceAbbr = tempString;
          strcpy(szarString, tempString);
//
//  RouteAbbr
//
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          trim(tempString, maxLength[1]);
          ILGTrips.m_RouteAbbr = tempString;
          strcat(szarString, " Route ");
          strcat(szarString, tempString);
//
//  RouteDirectionAbbr
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, maxLength[2]);
          ILGTrips.m_RouteDirectionAbbr = tempString;
          strcat(szarString, " ");
          strcat(szarString, tempString);
          StatusBarText(szarString);
//
//  PatternAbbr (which has the service number prepended to the name)
//
          PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          trim(szarString, PATTERNNAMES_NAME_LENGTH);
          sprintf(tempString, "%ld%s", SERVICES.number, szarString);
          trim(tempString, maxLength[0]);
          ILGTrips.m_PatternAbbr = tempString;
//
//  BlockSequence
//
          ILGTrips.m_BlockSequence = pTRIPSChunk->blockSequence;
//
//  Insert
//
          ILGTrips.Update();
          ILGTrips.MoveNext();
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while trips
      }  // nJ
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }  // while services
//
//  Get the next route
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  // while routes
  ILGTrips.Close();
//
//  ===============
//  TripStops Table
//  ===============
//
//  Open the recordset and get the field widths
//
  ILGTripStops.Open();
  for(nI = 0; nI < (int)ILGTripStops.m_nFields; nI++)
  {
    ILGTripStops.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                       // TripID
  maxLength[1] = maxLength[1];                                       // StopNum
  maxLength[2] = maxLength[2];                                       // CrossingTimeInMinutes
//
//  Get the number of routes in the table
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the routes
//
  s.LoadString(TEXT_395);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the directions for this route
//
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
//
//  Get the trips for this route/ser/dir
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nJ;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nJ)
        {
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
            PATTERNSKey2.directionIndex = nJ;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            distanceToHere = 0.0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nJ &&
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
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
//
//  Pass 2 - Determine time interpolations
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripIndex = 0;
          stopNumber = 1;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nJ &&
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
//                  timeAtStop += GTResults.tripTimes[tripIndex - 1];
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
            ILGTripStops.AddNew();
//
//  TripID
//
            ILGTripStops.m_TripID = TRIPS.recordID;
//
//  StopNumber
//
            ILGTripStops.m_StopNum = stopNumber++;
//
//  CrossingTimeInMinutes
//
            ILGTripStops.m_CrossingTimeInMinutes = timeAtStop / 60;
//
//  Insert
//
            ILGTripStops.Update();
            ILGTripStops.MoveNext();
//
//  Get the next node on the pattern
//
            prevLat = NODES.latitude;
            prevLon = NODES.longitude;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }  // while patterns
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while trips
      }  // nJ
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }  // while services
//
//  Get the next route
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  // while routes
  ILGTripStops.Close();
//
//  ==========
//  Runs Table
//  ==========
//
//  Open the recordset and get the field widths
//
  ILGRuns.Open();
  for(nI = 0; nI < (int)ILGRuns.m_nFields; nI++)
  {
    ILGRuns.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                       // RunNum
  maxLength[1] = maxLength[1];                                       // BlockNum
  maxLength[2] = min(maxLength[2], SERVICES_NAME_LENGTH);            // ServiceAbr
  maxLength[3] = maxLength[3];                                       // PieceNum
  maxLength[4] = maxLength[4];                                       // StartTimeInMinutes
  maxLength[5] = maxLength[5];                                       // EndTimeInMinutes
//
//  Get the number of divisions in the table
//
  rcode2 = btrieve(B_STAT, TMS_DIVISIONS, &BSTAT, dummy, 0);
  numRecords = BSTAT.numRecords;
//
//  Cycle through the divisions
//
  s.LoadString(TEXT_396);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
    StatusBar((long)nI, (long)numRecords);
//
//  Cycle through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the runs
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
        if(RUNS.pieceNumber == 1)
        {
          recordID = RUNS.recordID;
          numPieces = GetRunElements(NULL, &RUNS, &PROPOSEDRUN, &COST, TRUE);
          RUNSKey0.recordID = recordID;              //  Re-establish position
          btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
//
//  Go through the pieces
//
          for(nJ = 0; nJ < numPieces; nJ++)
          {
//
//  Write out the data
//
            ILGRuns.AddNew();
//
//  RunNum
//
            ILGRuns.m_RunNum = RUNSVIEW[nJ].runNumber;
//
//  BlockNum
//
            ILGRuns.m_BlockNum = RUNSVIEW[nJ].blockNumber;
//
//  ServiceAbbr
//
            strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(tempString, SERVICES_NAME_LENGTH);
            trim(tempString, maxLength[0]);
            ILGRuns.m_ServiceAbbr = tempString;
//
//  PieceNum
//
            ILGRuns.m_PieceNum = nJ + 1;
//
//  StartTimeInMinutes
//
            ILGRuns.m_StartTimeInMinutes = RUNSVIEW[nJ].runOnTime / 60;
//
//  EndTimeInMinutes
//
            ILGRuns.m_EndTimeInMinutes = RUNSVIEW[nJ].runOffTime / 60;
//
//  Insert
//
            ILGRuns.Update();
            ILGRuns.MoveNext();
          }  // nJ
        }  // piece 1
//
//  Get the next run
//
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }  // while runs
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next division
//
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  }
  ILGRuns.Close();
//
//  ============
//  Roster Table
//  ============
//
//  Open the recordset and get the field widths
//
  ILGRoster.Open();
  for(nI = 0; nI < (int)ILGRoster.m_nFields; nI++)
  {
    ILGRoster.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                       // RosterNum
  maxLength[1] = maxLength[1];                                       // Day
  maxLength[2] = maxLength[2];                                       // RunNum
  maxLength[3] = min(maxLength[3], SERVICES_NAME_LENGTH);            // ServiceAbbr
//
//  Cycle through the divisions
//
  s.LoadString(TEXT_397);
  StatusBarText((char *)LPCTSTR(s));
  rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Get the number of rosters in the table
//
    nI = 0;
    rcode2 = btrieve(B_STAT, TMS_ROSTER, &BSTAT, dummy, 0);
    numRecords = BSTAT.numRecords;
//
//  Cycle through the roster
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0)
    {
      StatusBar((long)nI, (long)numRecords);
      if(StatusBarAbort())
      {
        bFinishedOK = FALSE;
        goto done;
      }
      for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
      {
        if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ] == NO_RECORD)
        {
          continue;
        }
//
//  Write out the data
//
        ILGRoster.AddNew();
//
//  RosterNum
//
        ILGRoster.m_RosterNum = ROSTER.rosterNumber;
//
//  Day
//
        ILGRoster.m_Day = nJ + 1;
//
//  RunNum
//
        if(ROSTER.WEEK[m_RosterWeek].flags & (1 << nJ))
        {
          CREWONLYKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          ILGRoster.m_RunNum = CREWONLY.runNumber;
        }
        else
        {
          RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          ILGRoster.m_RunNum = RUNS.runNumber;
        }
//
//  ServiceAbbr
//
        SERVICESKey0.recordID = ROSTERPARMS.serviceDays[nJ];
        rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(tempString, SERVICES_NAME_LENGTH);
        trim(tempString, maxLength[3]);
        ILGRoster.m_ServiceAbbr = tempString;
//
//  Insert
//
        ILGRoster.Update();
        ILGRoster.MoveNext();
      }  // nJ
//
//  Get the next roster record
//
      nI++;
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }  // while roster
//
//  Get the next division
//
    rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  }
  ILGRoster.Close();
//
//  ===============
//  TimeTable Table
//  ===============
//
//  Open the recordset and get the field widths
//
  ILGTimeTable.Open();
  for(nI = 0; nI < (int)ILGTimeTable.m_nFields; nI++)
  {
    ILGTimeTable.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = min(maxLength[0], sizeof(szDatabaseDescription));   // TimeTableVersionName
  maxLength[1] = maxLength[1];                                       // ActivationDate
  maxLength[2] = maxLength[2];                                       // DeactivationDate
//
//  Write out the data
//
  ILGTimeTable.AddNew();
//
//  TimeTableVersionName
//
  strcpy(tempString, szDatabaseDescription);
  trim(tempString, maxLength[0]);
  ILGTimeTable.m_TimeTableVersionName = tempString;
//
//  ActivationDate
//
  ILGTimeTable.m_ActivationDate = AD;
//
//  DeactivationDate
//
  ILGTimeTable.m_DeactivationDate = DD;
//
//  Insert
//
  ILGTimeTable.Update();
  ILGTimeTable.Close();
//
//  =================
//  VersionInfo Table
//  =================
//
//  Open the recordset and get the field widths
//
  ILGVersionInfo.Open();
  for(nI = 0; nI < (int)ILGVersionInfo.m_nFields; nI++)
  {
    ILGVersionInfo.GetODBCFieldInfo(nI, FI);
    maxLength[nI] = FI.m_nPrecision;
  }
//
//  Establish the appropriate field widths
//
  maxLength[0] = maxLength[0];                                       // VersionInfo (2)
  maxLength[1] = maxLength[1];                                       // TimeInMinutes (1)
//
//  Delete anything that's there
//
  for(;;)
  {
    if(ILGVersionInfo.IsBOF())
    {
      break;
    }
    ILGVersionInfo.Delete();
    ILGVersionInfo.MoveFirst();
  }
//
//  Write out the data
//
  ILGVersionInfo.AddNew();
//
//  VersionInfo
//
  ILGVersionInfo.m_Version = 2;
//
//  TimeInMinutes
//
  ILGVersionInfo.m_TimeInMinutes = 1;
//
//  Insert
//
  ILGVersionInfo.Update();
  ILGVersionInfo.Close();
//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  done:
    db.Close();
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

  return(bFinishedOK);
}
