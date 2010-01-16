// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the Avail AVL System
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "Avail Interface"

#define SERVICELEVELINFO_REPORTLABEL_LENGTH 15
#define SERVICELEVELINFO_DESCRIPTION_LENGTH 50

#define STOPINFO_REPORTLABEL_LENGTH         NODES_DESCRIPTION_LENGTH
#define STOPINFO_DESCRIPTION_LENGTH         NODES_INTERSECTION_LENGTH

#define ROUTEINFO_REPORTLABEL_LENGTH        15
#define ROUTEINFO_DESCRIPTION_LENGTH        50
#define ROUTEINFO_INTERNETNAME_LENGTH       40

#define PATTERNINFO_PATTERNNAME_LENGTH      20

#define RUNINFO_DESCRIPTION_LENGTH        50


BOOL FAR TMSRPT66(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  ORDEREDLISTDef  SERVICEData[TMSRPT66_MAXSERVICES];
  double  prevLat, prevLon;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  float   distanceToHere;
  float   tripDistances[100];
  struct  tm tmED, tmUD, today;
  time_t  EDmktime;
  time_t  UDmktime;
  time_t  now;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bActive;
  BOOL  bLastTrip;
  BOOL  bLastTripBreak;
  BOOL  bFirstTrip;
  long  SERVICESinEffect[TMSRPT66_MAXSERVICES];
  long  year, month, day;
  long  serviceRecordID;
  long  previousPattern;
  long  Pattern_Record_Id;
  long  timeAtStop;
  long  endTime;
  long  outputRecordID;
  long  tempLong;
  long  serviceNumber;
  long  previousTime_Offset;
  long  previousDistance_Offset;
  long  distanceInFeet;
  char  outputString[1024];
  char *ptr;
  char *pszReportName;
  char *tempString2;
  char  outputFileName[64];
  char  szRoutesReportLabel[DIRECTIONS_ABBRNAME_LENGTH + 1 + ROUTES_NAME_LENGTH + 1];
  int   nI;
  int   rcode2;
  int   dayOfWeek;
  int   numServicesInEffect;
  int   numServices;
  int   Sort_Order;
  int   tripIndex;
  int   reliefFlag;

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
  pPassedData->nReportNumber = 65;
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
//  Get today's date
//
   time(&now);
   today = *localtime(&now);
//
//  ==============================================================
//  Service Level Definitions File - Service_Level_Definitions.dat
//  ==============================================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Service_Level_Definitions.dat");
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
  numServicesInEffect = 0;
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
      for(bFound = FALSE, nI = 0; nI < numServicesInEffect; nI++)
      {
        if(ROSTERPARMS.serviceDays[dayOfWeek] == SERVICESinEffect[nI])
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        SERVICESinEffect[numServicesInEffect] = ROSTERPARMS.serviceDays[dayOfWeek];
        numServicesInEffect++;
      }
//
//  SL_Record_Id (SERVICES.recordID)
//
      sprintf(tempString, "%ld,", ROSTERPARMS.serviceDays[dayOfWeek]);
      strcpy(outputString, tempString);
//
//  Service_Date
//
      sprintf(tempString, "%4ld%02ld%02ld\r\n", tmED.tm_year + 1900, tmED.tm_mon + 1, tmED.tm_mday);
      strcat(outputString, tempString);
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
//  ==============================================================
//  Service Level Info File - Service_Level_Info.dat
//  ==============================================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Service_Level_Info.dat");
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
//  Go through the services
//
  numServices = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  while(rcode2 == 0)
  {
//
//  SL_Record_Id (SERVICES.recordID)
//
    sprintf(tempString, "%ld,", SERVICES.recordID);
    strcpy(outputString, tempString);
//
//  Date_Active (Today)
//
    sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
    strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
    strcpy(tempString, ",");
    strcat(outputString, tempString);
//
//  Active (Always 1)
//
    strcpy(tempString, "1,");
    strcat(outputString, tempString);
//
//  SL_Id (Service number)
//
    sprintf(tempString, "%ld,", SERVICES.number);
    strcat(outputString, tempString);
//
//  Report_Label (Service name)
//
    strncpy(tempString, SERVICES.name, SERVICELEVELINFO_REPORTLABEL_LENGTH);
    trim(tempString, SERVICELEVELINFO_REPORTLABEL_LENGTH);
    strcat(outputString, "\"");
    strcat(outputString, tempString);
    strcat(outputString, "\",");
//
//  Description (Comment text, if available, null otherwise)
//
    if(SERVICES.COMMENTSrecordID != NO_RECORD)
    {
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      COMMENTSKey0.recordID = SERVICES.COMMENTSrecordID;
      btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      strcpy(tempString, "");
      for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
           tempString2 = strtok(NULL, "\r\n") )
      {
        strcat(tempString, tempString2);
        if(strlen(tempString) >= SERVICELEVELINFO_DESCRIPTION_LENGTH)
        {
          break;
        }
      }
      trim(tempString, SERVICELEVELINFO_DESCRIPTION_LENGTH);
      strcat(outputString, "\"");
      strcat(outputString, tempString);
      strcat(outputString, "\"");
    }
    strcat(outputString, "\r\n");
//
//  Write out the record
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Save the service record
//
    memcpy(&SERVICEData[numServices], &SERVICES, sizeof(ORDEREDLISTDef));
    numServices++;
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==============================
//  Stop_Info File - Stop_Info.dat
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Stop_Info.dat");
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
//  Stop_Record_Id (NODES.recordID)
//
    sprintf(tempString, "%ld,", NODES.recordID);
    strcpy(outputString, tempString);
//
//  Date_Active (Today)
//
    sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
    strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
    strcpy(tempString, ",");
    strcat(outputString, tempString);
//
//  Active (Always 1)
//
    strcpy(tempString, "1,");
    strcat(outputString, tempString);
//
//  Stop_Id (NODES.recordID)
//
    sprintf(tempString, "%ld,", NODES.recordID);
    strcat(outputString, tempString);
//
//  Report_Label (NODES.description)
//
    strncpy(tempString, NODES.description, STOPINFO_REPORTLABEL_LENGTH);
    trim(tempString, STOPINFO_REPORTLABEL_LENGTH);
    while(ptr = strchr(tempString, '"'))
    {
      strcpy(ptr, ptr + 1);
    }
    tempString[STOPINFO_REPORTLABEL_LENGTH] = '\0';
    strcat(outputString, "\"");
    strcat(outputString, tempString);
    strcat(outputString, "\",");
//
//  Timepoint (0 = Stop Only, 1 = Timepoint)
//
    strcat(outputString, ((NODES.flags & NODES_FLAG_STOP) ? "0" : "1"));
    strcat(outputString, ",");
//
//  Description (NODES.intersection)
//
    strncpy(tempString, NODES.intersection, STOPINFO_DESCRIPTION_LENGTH);
    trim(tempString, STOPINFO_DESCRIPTION_LENGTH);
    while(ptr = strchr(tempString, '"'))
    {
      strcpy(ptr, ptr + 1);
    }
    tempString[STOPINFO_DESCRIPTION_LENGTH] = '\0';
    strcat(outputString, "\"");
    strcat(outputString, tempString);
    strcat(outputString, "\",");
//
//  Latitude (NODES.latitude)
//
    sprintf(szarString, "%12.6f,", NODES.latitude);
    ptr = szarString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    strcat(outputString, ptr);
//
//  Longitude (NODES.longitude)
//
    sprintf(szarString, "%12.6f,", NODES.longitude);
    ptr = szarString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    strcat(outputString, ptr);
//
//  Internet_Name (NODES.description)
//
    strncpy(tempString, NODES.description, STOPINFO_REPORTLABEL_LENGTH);
    trim(tempString, STOPINFO_REPORTLABEL_LENGTH);
    while(ptr = strchr(tempString, '"'))
    {
      strcpy(ptr, ptr + 1);
    }
    tempString[STOPINFO_REPORTLABEL_LENGTH] = '\0';
    strcat(outputString, "\"");
    strcat(outputString, tempString);
    strcat(outputString, "\",");
//
//  Stoppoint (Always 1)
//
    strcat(outputString, "1,");
//
//  External_Stop_Id
//  External_Node_Id
//  External_Garage_Id (All always NULL)
//
    strcat(outputString, ",,");
//
//  AVL Stop name
//
    strncpy(tempString, NODES.AVLStopName, NODES_AVLSTOPNAME_LENGTH);
    trim(tempString, NODES_AVLSTOPNAME_LENGTH);
    while(ptr = strchr(tempString, '"'))
    {
      strcpy(ptr, ptr + 1);
    }
    strcat(outputString, "\"");
    strcat(outputString, tempString);
    strcat(outputString, "\"");
    strcat(outputString, "\r\n");
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
//  ================================
//  Route_Info File - Route_Info.dat
//  ================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Route_Info.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
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
//  Go through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  There must be trips on this route to show as "active"
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        bActive = (rcode2 == 0 && TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID && TRIPS.directionIndex == nI);
//
//  Route_Record_Id
//
        sprintf(tempString, "%ld,", ROUTES.recordID);
        strcpy(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
        sprintf(tempString, "%ld,", SERVICES.recordID);
        strcat(outputString, tempString);
//
//  Date_Active (Today)
//
        sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
        strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
        strcpy(tempString, ",");
        strcat(outputString, tempString);
//
//  Active
//
        sprintf(tempString, "%d,", (bActive ? 1 : 0));
        strcat(outputString, tempString);
//
//  Farebox_Id was (ROUTES.recordID) now (ROUTES.number with any alphas truncated)
//
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        tempLong = atol(tempString);  // atol kicks out after encountering an alpha
        sprintf(tempString, "%ld,", tempLong);
        strcat(outputString, tempString);
//
//  Report_Label (ROUTES.name)
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        if(rcode2 != 0)
        {
          strcpy(tempString, "");
        }
        else
        {
          strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
          strcat(tempString, ":");
        }
        strcpy(szRoutesReportLabel, tempString);
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(szRoutesReportLabel, tempString);
        trim(szRoutesReportLabel, ROUTEINFO_REPORTLABEL_LENGTH);
        strcat(outputString, "\"");
        strcat(outputString, szRoutesReportLabel);
        strcat(outputString, "\",");
//
//  Description (Comment text, if available, Report_Label otherwise)
//
        if(ROUTES.COMMENTSrecordID == NO_RECORD)
        {
          strcat(outputString, "\"");
          strcat(outputString, szRoutesReportLabel);
          strcat(outputString, "\",");
        }
        else
        {
          recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
          COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
          btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
          recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
          strcpy(tempString, "");
          for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
               tempString2 = strtok(NULL, "\r\n") )
          {
            strcat(tempString, tempString2);
            if(strlen(tempString) >= ROUTEINFO_DESCRIPTION_LENGTH)
            {
              break;
            }
          }
          trim(tempString, ROUTEINFO_DESCRIPTION_LENGTH);
          strcat(outputString, "\"");
          strcat(outputString, tempString);
          strcat(outputString, "\",");
        }
//
//  Fareset_Record_Id (Always 0)
//
        strcat(outputString, "0,");
//
//  Start_Time (Time at first node of first trip)
//
        if(bActive)
        {
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          sprintf(tempString, "%ld,", GTResults.firstNodeTime);
        }
        else
        {
          strcpy(tempString, "0,");
        }
        strcat(outputString, tempString);
// 
//  End_Time (Time at last node of last trip)
//
        if(bActive)
        {
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = 1;
          TRIPSKey1.directionIndex = nI + 1;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          rcode2 = btrieve(B_GETPREVIOUS, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          sprintf(tempString, "%ld,", GTResults.lastNodeTime);
        }
        else
        {
          strcpy(tempString, "0,");
        }
        strcat(outputString, tempString);
//
//  Deadhead_Miles
//  Deadhead_Minutes
//  Revenue_Miles
//  Revenue_Minutes (All always NULL)
//
        strcat(outputString, ",,,,");
//
//  Internet_Name (Same as Report_Label)
//
        strcat(outputString, "\"");
        strcat(outputString, szRoutesReportLabel);
        strcat(outputString, "\",");
//
//  Headway_Route (Always 0)
//
        strcat(outputString, "0,");
//
//  APC_Ridership (Always 0)
//
        strcat(outputString, "0\r\n");
//
//  Write it out and get the next direction
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      } // nI
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==================================
//  Route_Stops File - Route_Stops.dat
//  ==================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Route_Stops.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
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
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the base pattern nodes
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.nodeSequence = -1;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        Sort_Order = 1;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID &&
              PATTERNS.directionIndex == nI)
        {
//
//  Route_Record_Id
//
          sprintf(tempString, "%ld,", ROUTES.recordID);
          strcpy(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
          sprintf(tempString, "%ld,", SERVICES.recordID);
          strcat(outputString, tempString);
//
//  Stop_Record_Id (PATTERNS.NODESrecordID)
//
          sprintf(tempString, "%ld,", PATTERNS.NODESrecordID);
          strcat(outputString, tempString);
//
//  Sort_Order (Incremental)
//
          sprintf(tempString, "%d\r\n", Sort_Order++);
          strcat(outputString, tempString);
//
//  Write it out and get the next node on the pattern
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        } // while on patterns
      } // nI
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================
//  Pattern_Info File - Pattern_Info.dat
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Pattern_Info.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
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
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the unique patterns for this route/dir on this service
//
        previousPattern = NO_RECORD;
        for(;;)
        {
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = previousPattern;
          PATTERNSKey2.nodeSequence = 9999999;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 != 0 || PATTERNS.ROUTESrecordID != ROUTES.recordID ||
                PATTERNS.SERVICESrecordID != SERVICES.recordID || PATTERNS.directionIndex != nI)
          {
            break;
          }
          previousPattern = PATTERNS.PATTERNNAMESrecordID;
//
//  Pattern_Record_Id
//
          outputRecordID = ((((ROUTES.recordID * 1000 + nI) * 1000) + PATTERNS.PATTERNNAMESrecordID) * 10) + SERVICES.number;
          sprintf(tempString, "%ld,", outputRecordID);
          strcpy(outputString, tempString);
//
//  Date_Active (Today)
//
          sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
          strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
          strcpy(tempString, ",");
          strcat(outputString, tempString);
//
//  Active (Find a trip with this pattern on this route/ser/dir)
//
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
          TRIPSKey1.directionIndex = nI;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          bActive = FALSE;
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nI)
          {
            if(TRIPS.PATTERNNAMESrecordID == PATTERNS.PATTERNNAMESrecordID)
            {
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
              bActive = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  Must be active (at least one trip built on it)
//
//
//  Changed 27-Oct-09 (R Webber) - Now unload all patterns
//
//          if(bActive)
          {
            sprintf(tempString, "%d,", (bActive ? 1 : 0));
            strcat(outputString, tempString);
//
//  Route_Record_Id
//
            sprintf(tempString, "%ld,", ROUTES.recordID);
            strcat(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
            sprintf(tempString, "%ld,", SERVICES.recordID);
            strcat(outputString, tempString);
//
//  Pattern_Name (PATTERNNAMES.name)
//
            PATTERNNAMESKey0.recordID = previousPattern;
            rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            if(rcode2 != 0)
            {
              strcpy(tempString, "");
            }
            else
            {
              strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
              trim(tempString, PATTERNNAMES_NAME_LENGTH);
            }
            trim(tempString, PATTERNINFO_PATTERNNAME_LENGTH);
            strcat(outputString, "\"");
            strcat(outputString, tempString);
            strcat(outputString, "\",");
//
//  Makeup_Time_Factor (Always 0)
//
            strcat(outputString, "0,");
//
//  Direction ("I"nbound, "O"utbound, "L"oop)
//
            if(nI == 0)
            {
              strcpy(tempString, (ROUTES.DIRECTIONSrecordID[1] == NO_RECORD ? "L" : "O"));
            }
            else
            {
              strcpy(tempString, "I");
            }
            strcat(outputString, "\"");
            strcat(outputString, tempString);
            strcat(outputString, "\",");
//
//  Description
//  Service_Description (Always NULL)
//
            strcat(outputString, ",,");
//
//  Total distance (in feet)
//
            if(bActive)
            {
              sprintf(tempString, "%ld,", (int)(GTResults.tripDistance * 5280));
            }
            else
            {
              strcpy(tempString, "0,");
            }
            strcat(outputString, tempString);
//
//  Total Minutes (not seconds)
//
            if(bActive)
            {
              sprintf(tempString, "%ld,", (int)((GTResults.lastNodeTime - GTResults.firstNodeTime) / 60));
            }
            else
            {
              strcpy(tempString, "0,");
            }
            strcat(outputString, tempString);
//
//  Deadhead (0 = Revenue Service, 1 = Deadhead)
//
            strcat(outputString, "0,");
//
//  External_Pattern_Id (Always NULL)
//
            strcat(outputString, ",");
//
//  Headsign_Id
//
            if(TRIPS.SIGNCODESrecordID <= 0)
            {
              TRIPS.SIGNCODESrecordID = NO_RECORD;
            }
            sprintf(tempString, "%ld\r\n", TRIPS.SIGNCODESrecordID);
            strcat(outputString, tempString);
//
//  Write it out and go back
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          } // bActive
        } // for(;;)
      } // nI
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ======================================
//  Pattern_Stops File - Pattern_Stops.dat
//  ======================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Pattern_Stops.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
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
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the unique patterns for this route/ser/dir
//
        previousPattern = NO_RECORD;
        for(;;)
        {
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = previousPattern;
          PATTERNSKey2.nodeSequence = 9999999;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 != 0 || PATTERNS.ROUTESrecordID != ROUTES.recordID ||
                PATTERNS.SERVICESrecordID != SERVICES.recordID || PATTERNS.directionIndex != nI)
          {
            break;
          }
          previousPattern = PATTERNS.PATTERNNAMESrecordID;
//
//  Find a trip with this pattern on this route/ser/dir
//
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
          TRIPSKey1.directionIndex = nI;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          bActive = FALSE;
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nI)
          {
            if(TRIPS.PATTERNNAMESrecordID == previousPattern)
            {
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
              bActive = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  There has to be one
//
//          if(bActive)
          {
//
//  Pattern_Record_Id
//
            Pattern_Record_Id = previousPattern;
//
//  Kludge to ensure we don't unload identical time/distance data
//
            previousTime_Offset = NO_RECORD;
            previousDistance_Offset = NO_RECORD;

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
              PATTERNSKey2.PATTERNNAMESrecordID = previousPattern;
              PATTERNSKey2.nodeSequence = NO_RECORD;
              rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              tripIndex = 0;
              distanceToHere = 0.0;
              while(rcode2 == 0 &&
                    PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                    PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                    PATTERNS.directionIndex == nI &&
                    PATTERNS.PATTERNNAMESrecordID == previousPattern)
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
//  Pass 2 - Determine time interpolations and output the records
//
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nI;
            PATTERNSKey2.PATTERNNAMESrecordID = previousPattern;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nI &&
                  PATTERNS.PATTERNNAMESrecordID == previousPattern)
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(NODES.flags & NODES_FLAG_STOP)
//              if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
              {
                if(GTResults.tripDistance == 0)
                {
                  timeAtStop = GTResults.tripTimes[tripIndex - 1] - GTResults.firstNodeTime;
                }
                else
                {
                  distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
                  timeAtStop = (long)((GTResults.tripTimes[tripIndex] - GTResults.tripTimes[tripIndex - 1]) *
                        (distanceToHere / (tripDistances[tripIndex] - tripDistances[tripIndex - 1])));
                  timeAtStop += (GTResults.tripTimes[tripIndex - 1] - GTResults.firstNodeTime);
                }
              }
              else
              {
                timeAtStop = GTResults.tripTimes[tripIndex] - GTResults.firstNodeTime;
                distanceToHere = 0.0;
                tripIndex++;
              }
//
//  Route_Record_Id
//
              sprintf(tempString, "%ld,", ROUTES.recordID);
              strcpy(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
              sprintf(tempString, "%ld,", SERVICES.recordID);
              strcat(outputString, tempString);
//
//  Pattern_Record_Id (ibid)
//
              outputRecordID = ((((ROUTES.recordID * 1000 + nI) * 1000) + PATTERNS.PATTERNNAMESrecordID) * 10) + SERVICES.number;
              sprintf(tempString, "%ld,", outputRecordID);
              strcat(outputString, tempString);
//
//  Stop_Record_Id (PATTERNS.NODESrecordID)
//
              sprintf(tempString, "%ld,", PATTERNS.NODESrecordID);
              strcat(outputString, tempString);
//
//  Time_Offset (Time at the stop, in seconds)
//
              if(timeAtStop == previousTime_Offset)
              {
                timeAtStop++;
                sprintf(tempString, "%ld has identical time offset to previous node on %ld.  Time adjusted by 1 second to %ld.\r\n",
                      PATTERNS.NODESrecordID, outputRecordID, timeAtStop);
                _lwrite(hfErrorLog, tempString, strlen(tempString));
              }
              sprintf(tempString, "%ld,", timeAtStop);
              strcat(outputString, tempString);
//
//  Distance_Offset (Distance from the beginning of the trip to this stop, in feet)
//
              distanceInFeet = (long)((tripDistances[tripIndex - 1] + distanceToHere) * 5280);
              if(distanceInFeet == previousDistance_Offset)
              {
                distanceInFeet++;
                sprintf(tempString, "%ld has identical distance offset to previous node on %ld.  Distance adjusted by 1 foot to %ld.\r\n",
                      PATTERNS.NODESrecordID, outputRecordID, distanceInFeet);
                _lwrite(hfErrorLog, tempString, strlen(tempString));
              }
              sprintf(tempString, "%ld\r\n", distanceInFeet);
              strcat(outputString, tempString);
//
//  Keep the previous time and distance offsets
//
              previousTime_Offset = timeAtStop;
              previousDistance_Offset = distanceInFeet;
//
//  Write it out and go back
//
              _lwrite(hfOutputFile, outputString, strlen(outputString));
              prevLat = NODES.latitude;
              prevLon = NODES.longitude;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            } // while on pattern
          }  // bActive
        } // for(;;)
      } // nI
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==============================
//  Trip Info File - Trip_Info.dat
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Trip_Info.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
//
//  Go through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
//
//  Go through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Go through the trips
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
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Trip_Record_Id (TRIPS.recordID)
//
          sprintf(tempString, "%ld,", TRIPS.recordID);
          strcpy(outputString, tempString);
//
//  Route_Record_Id
//
          sprintf(tempString, "%ld,", ROUTES.recordID);
          strcat(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
          sprintf(tempString, "%ld,", SERVICES.recordID);
          strcat(outputString, tempString);
//
//  Date_Active (Today)
//
          sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
          strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
          strcpy(tempString, ",");
          strcat(outputString, tempString);
//
//  Active (Always 1)
//
          strcpy(tempString, "1,");
          strcat(outputString, tempString);
//
//  Farebox_Id (Military start time of the trip)
//
          strcpy(tempString, chhmmnc(GTResults.firstNodeTime));
          strcat(outputString, tempString);
          strcat(outputString, ",");
//
//  Report_Label (Trip plus direction: i.e. 800 Outbound trips would be "Trip 800-O"
//
          if(nI == 0)
          {
            strcpy(szarString, (ROUTES.DIRECTIONSrecordID[1] == NO_RECORD ? "L" : "O"));
          }
          else
          {
            strcpy(szarString, "I");
          }
          strcpy(tempString, "Trip ");
          strcat(tempString, chhmmnc(GTResults.firstNodeTime));
          strcat(tempString, "-");
          strcat(tempString, szarString);

          strcat(outputString, "\"");
          strcat(outputString, tempString);
          strcat(outputString, "\",");
//
//  Direction
//
          strcat(outputString, "\"");
          strcat(outputString, szarString);
          strcat(outputString, "\",");
//
//  Start_Time (in seconds)
//
          sprintf(tempString, "%ld,", GTResults.firstNodeTime);
          strcat(outputString, tempString);
//
//  End_Time (in seconds)
//
          sprintf(tempString, "%ld,", GTResults.lastNodeTime);
          strcat(outputString, tempString);
//
//  Threshold (Always NULL)
//
          strcat(outputString, ",");
//
//  Deadhead (0=Revenue, 1=Deadhead)
//
          strcat(outputString, "0\r\n");
//
//  Write out the record
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
      } // nI
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ============================
//  Run Info File - Run_Info.dat
//  ============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Run_Info.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
//
//  Go through the runs
//
    RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
    RUNSKey1.SERVICESrecordID = SERVICES.recordID;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 &&
          RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
          RUNS.SERVICESrecordID == SERVICES.recordID)
    {
//
//  Run_Record_Id (RUNS.recordID)
//
      sprintf(tempString, "%ld,", RUNS.recordID);
      strcpy(outputString, tempString);
//
//  SL_Record_Id (SERVICES.recordID)
//
      sprintf(tempString, "%ld,", SERVICES.recordID);
      strcat(outputString, tempString);
//
//  Date_Active (Today)
//
      sprintf(tempString, "%4ld%02ld%02ld,", today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
      strcat(outputString, tempString);
//
//  Date_Deleted (Always null)
//
      strcpy(tempString, ",");
      strcat(outputString, tempString);
//
//  Active (Always 1)
//
      strcpy(tempString, "1,");
      strcat(outputString, tempString);
//
//  Farebox_Id was (RUNS.recordID) now (RUNS.runNumber)
//
      sprintf(tempString, "%ld,", RUNS.runNumber);
      strcat(outputString, tempString);
//
//  Report_Label (Run number)
//
      sprintf(tempString, "%ld/%ld", RUNS.runNumber, RUNS.pieceNumber);
      strcat(outputString, "\"");
      strcat(outputString, tempString);
      strcat(outputString, "\",");
//
//  Description (Comment text, if available, null otherwise)
//
      if(RUNS.COMMENTSrecordID != NO_RECORD)
      {
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        COMMENTSKey0.recordID = RUNS.COMMENTSrecordID;
        btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        strcpy(tempString, "");
        for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
             tempString2 = strtok(NULL, "\r\n") )
        {
          strcat(tempString, tempString2);
          if(strlen(tempString) >= RUNINFO_DESCRIPTION_LENGTH)
          {
            break;
          }
        }
        trim(tempString, RUNINFO_DESCRIPTION_LENGTH);
        strcat(outputString, "\"");
        strcat(outputString, tempString);
        strcat(outputString, "\"");
      }
      strcat(outputString, "\r\n");
//
//  Write out the record
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next run record
//
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================================
//  Run Schedule Working File - Run_Schedule_Working.dat
//  ====================================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Run_Schedule_Working.dat");
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
//  Go through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  while(rcode2 == 0)
  {
//
//  Go through the runs
//
    RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
    RUNSKey1.SERVICESrecordID = SERVICES.recordID;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 &&
          RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
          RUNS.SERVICESrecordID == SERVICES.recordID)
    {
//
//  SL_Record_Id (SERVICES.recordID)
//
      sprintf(tempString, "%ld,", SERVICES.recordID);
      strcpy(szarString, tempString);
//
//  Run_Record_Id (RUNS.recordID)
//
      sprintf(tempString, "%ld,", RUNS.recordID);
      strcat(szarString, tempString);
//
//  Go through all the trips on the run
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);      
      rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
      bLastTrip = FALSE;
      bLastTripBreak = FALSE;
      bFirstTrip = TRUE;
      while(rcode2 == 0)
      {
        endTime = GTResults.lastNodeTime;
        strcpy(outputString, szarString);
//
//  Trip_Record_Id
//
        sprintf(tempString, "%ld,", TRIPS.recordID);
        strcat(outputString, tempString);
//
//  Pattern_Record_Id
//
//  If necessary, locate the service number
//
        if(SERVICES.recordID == TRIPS.SERVICESrecordID)
        {
          serviceNumber = SERVICES.number;
        }
        else
        {
          for(serviceNumber = NO_RECORD, nI = 0; nI < numServices; nI++)
          {
            if(SERVICEData[nI].recordID == TRIPS.SERVICESrecordID)
            {
              serviceNumber = SERVICEData[nI].number;
              break;
            }
          }
        }
        outputRecordID = ((((TRIPS.ROUTESrecordID * 1000 + TRIPS.directionIndex) * 1000) + TRIPS.PATTERNNAMESrecordID) * 10) + serviceNumber;
        sprintf(tempString, "%ld,", outputRecordID);
        strcat(outputString, tempString);
//
//  Relief flag
//
        if(bFirstTrip)
        {
          if(RUNS.start.NODESrecordID == GTResults.firstNODESrecordID)
          {
            reliefFlag = 3;
          }
          else if(RUNS.start.NODESrecordID == GTResults.lastNODESrecordID)
          {
            reliefFlag = 2;
          }
          else
          {
            reliefFlag = 1;
          }
        }
        else
        {
          reliefFlag = 0;
        }
        bFirstTrip = FALSE;
//
//  Makeup_Seconds (Layover)
//
        if(bLastTrip || TRIPS.recordID == RUNS.end.TRIPSrecordID)
        {
          strcat(outputString, "0,");
          bLastTripBreak = TRUE;
        }
        else
        {
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          sprintf(tempString, "%ld,", GTResults.firstNodeTime - endTime);
          strcat(outputString, tempString);
          if(TRIPS.recordID == RUNS.end.TRIPSrecordID)
          {
            bLastTrip = TRUE;
          }
        }
//
//  Relief flag (used to be Ignored (Always 0))
//
//        strcat(outputString, "0");
        sprintf(tempString, "%d", reliefFlag);
        strcat(outputString, tempString);
        strcat(outputString, "\r\n");
//
//  Write out the record
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        if(bLastTripBreak)
        {
          break;
        }
      }  // rcode2 on TRIPS
//
//  Get the next run
//
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }  // rcode2 on RUNS
//
//  Get the next service
//
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
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
