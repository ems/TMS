//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT33() - System Time Period Detail Report
//
//  From QETXT.INI:
//
//    [TMSRPT33]
//    FILE=TMSRPT33.TXT
//    FLN=0
//    TT=Tab
//    FIELD1=Sequence,NUMERIC,7,0,7,0,
//    FIELD2=Division,VARCHAR,32,0,32,0,
//    FIELD3=Route,VARCHAR,75,0,75,0,
//    FIELD4=Service,VARCHAR,32,0,32,0,
//    FIELD5=Period01,NUMERIC,8,0,8,0,
//    FIELD6=Period02,NUMERIC,8,0,8,0,
//    FIELD7=Period03,NUMERIC,8,0,8,0,
//    FIELD8=Period04,NUMERIC,8,0,8,0,
//

#include "TMSHeader.h"
#include <math.h>

static char szSection[] = "Report32";

#define TMSRPT33_NUMPERIODS 4

#define TMSRPT33_BYROUTEONLY_FLAG    0x0001
#define TMSRPT33_HHMM_FLAG           0x0002
#define TMSRPT33_INCLUDESCHOOL_FLAG  0x0004
#define TMSRPT33_INCLUDESHUTTLE_FLAG 0x0010
#define TMSRPT33_BEFORE_FLAG         0x0020
#define TMSRPT33_AFTER_FLAG          0x0040
#define TMSRPT33_ALLDAY_FLAG         0x0100
typedef struct PERIODStruct
{
  char name[16];
  long from;
  long to;
} PERIODDef;
typedef struct TMSRPT33Struct
{
  long flags;
  PERIODDef periods[TMSRPT33_NUMPERIODS];
  long time;
} TMSRPT33Def;


BOOL  CALLBACK TPDFILTERMsgProc(HWND, UINT, WPARAM, LPARAM);

BOOL FAR TMSRPT33(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  TMSRPT33Def    RPT33PARMS;
  HANDLE hOutputFile;
  DWORD dwBytesWritten;
  TRIPSKey2Def tripSave;
  float distance;
  long  categoryTotals[TMSRPT33_NUMPERIODS];
  long  statbarTotal;
  long  numTrips;
  long  equivalentTravelTime;
  long  fromNode;
  long  fromTime;
  long  fromRoute;
  long  fromService;
  long  layover;
  long  deadheadTime;
  long  absoluteRecord;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   sequence = 0;
  int   pos;

  pPassedData->nReportNumber = 32;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.PATTERNNAMESrecordID = NO_RECORD;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS;

  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT33.txt");
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
//
//  So far so good - see what's to be filtered
//
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TPDFILTER),
        hWndMain, (DLGPROC)TPDFILTERMsgProc, (LPARAM)&RPT33PARMS);
  if(!bRC)
    goto deallocate;
//
//  Set up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
//
//  Get the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[0];
  rcode2 = btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  if(rcode2 != 0)
    strcpy(divisionName, "");
  else
  {
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
  }
//
//  Loop through the services
//
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    SERVICESKey0.recordID = REPORTPARMS.pServiceList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
      continue;
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through the routes
//
    for(nJ = 0; nJ < REPORTPARMS.numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      ROUTESKey0.recordID = REPORTPARMS.pRouteList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 != 0)
        continue;
//
//  Check on school/shuttle routes
//
      if((ROUTES.flags & ROUTES_FLAG_SCHOOL) &&
            !(RPT33PARMS.flags & TMSRPT33_INCLUDESCHOOL_FLAG))
        continue;
      if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) &&
            !(RPT33PARMS.flags & TMSRPT33_INCLUDESHUTTLE_FLAG))
        continue;
//
//  Build up the route number and name
//
      strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
      strcat(routeNumberAndName, " - ");
      strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(szarString, ROUTES_NAME_LENGTH);
      strcat(routeNumberAndName, szarString);
      StatusBarText(routeNumberAndName);
      StatusBar((long)(nI * REPORTPARMS.numRoutes + nJ), (long)statbarTotal);
//
//  Zero out the category totals
//
      memset(categoryTotals, 0x00, sizeof(long) * TMSRPT33_NUMPERIODS);
      numTrips = 0;   
//
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          continue;
//
//  Get the direction name if we're reporting by route/direction,
//  and reset the totals to zero while we're at it
//
        if(!(RPT33PARMS.flags & TMSRPT33_BYROUTEONLY_FLAG))
        {
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
          memset(categoryTotals, 0x00, sizeof(long) * TMSRPT33_NUMPERIODS);
          numTrips = 0;
        }
//
//  And cycle through the trips
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nK)
        {
          numTrips++;
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Check on any before and after constraints
//
          if((RPT33PARMS.flags & TMSRPT33_ALLDAY_FLAG) ||
                ((RPT33PARMS.flags & TMSRPT33_BEFORE_FLAG) && RPT33PARMS.time > GTResults.firstNodeTime) ||
                ((RPT33PARMS.flags & TMSRPT33_AFTER_FLAG ) && RPT33PARMS.time < GTResults.firstNodeTime))
          {
//
//  Figure out where it goes
//
            for(pos = NO_RECORD, nL = 0; nL < TMSRPT33_NUMPERIODS; nL++)
            {
              if(GTResults.firstNodeTime >= RPT33PARMS.periods[nL].from &&
                    GTResults.firstNodeTime <= RPT33PARMS.periods[nL].to)
              {
                categoryTotals[nL] += (GTResults.lastNodeTime - GTResults.firstNodeTime);
                pos = nL;
                fromNode = GTResults.lastNODESrecordID;
                fromTime = GTResults.lastNodeTime;
                fromRoute = TRIPS.ROUTESrecordID;
                fromService = TRIPS.SERVICESrecordID;
                break;
              }
            }
//
//  Repostion the trips record to get the next trip on the block
//  We'll assume "standard" blocks here - not "dropback"
//
            if(TRIPS.standard.blockNumber > 0)
            {
              tripSave.assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
              tripSave.RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
              tripSave.SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
              tripSave.blockNumber = TRIPS.standard.blockNumber;
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              memcpy(&absoluteRecord, &TRIPS, 4);
              rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              if(rcode2 == 0 &&
                    TRIPS.standard.assignedToNODESrecordID == tripSave.assignedToNODESrecordID &&
                    TRIPS.standard.RGRPROUTESrecordID == tripSave.RGRPROUTESrecordID &&
                    TRIPS.standard.SGRPSERVICESrecordID == tripSave.SGRPSERVICESrecordID &&
                    TRIPS.standard.blockNumber == tripSave.blockNumber)
              {
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                if(NodesEquivalent(fromNode, GTResults.firstNODESrecordID, &equivalentTravelTime))
                  layover = GTResults.firstNodeTime - fromTime;
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = fromRoute;
                  GCTData.fromSERVICESrecordID = fromService;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = fromNode;
                  GCTData.toNODESrecordID = GTResults.lastNODESrecordID;
                  GCTData.timeOfDay = fromTime;
                  deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  if(deadheadTime == NO_TIME)
                    layover = GTResults.firstNodeTime - fromTime;
                  else
                    layover = (GTResults.firstNodeTime - fromTime - deadheadTime);
                }
                categoryTotals[pos] += layover;
              }
//
//  Reposition the Trips record and get the next one
//
              memcpy(&TRIPS, &absoluteRecord, 4);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
        if(numTrips > 0 && !(RPT33PARMS.flags & TMSRPT33_BYROUTEONLY_FLAG))
        {
          sprintf(tempString, "%d\t\"%s\"\t\"%s %s\"\t\"%s\"\t%ld\t%ld\t%ld\t%ld\r\n",
                sequence, divisionName, routeNumberAndName, directionName, serviceName,
                categoryTotals[0],categoryTotals[1],categoryTotals[2],categoryTotals[3]);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          sequence++;
        }
      }  // nK
      if(numTrips > 0 && (RPT33PARMS.flags & TMSRPT33_BYROUTEONLY_FLAG))
      {
        sprintf(tempString, "%d\t\"%s\"\t\"%s\"\t\"%s\"\t%ld\t%ld\t%ld\t%ld\r\n",
              sequence, divisionName, routeNumberAndName, serviceName,
              categoryTotals[0],categoryTotals[1],categoryTotals[2],categoryTotals[3]);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sequence++;
      }
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    CloseHandle(hOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
//  Write out the report name to TMS.INI so Crystal can pick it up
//
  LoadString(hInst, (RPT33PARMS.flags & TMSRPT33_BYROUTEONLY_FLAG) ? TEXT_259 : TEXT_260,
        tempString, TEMPSTRING_LENGTH);
  strcat(tempString, " - ");
//
//  Before/After/All Day
//
  if(RPT33PARMS.flags & TMSRPT33_ALLDAY_FLAG)
  {
    LoadString(hInst, TEXT_254, szarString, sizeof(szarString));
  }
  else
  {
    LoadString(hInst, (RPT33PARMS.flags & TMSRPT33_BEFORE_FLAG) ? TEXT_248 : TEXT_249,
          szarString, sizeof(szarString));
    strcat(szarString, " ");
    strcat(szarString, Tchar(RPT33PARMS.time));
  }
  strcat(tempString, szarString);
  strcat(tempString, ", ");
//
//  School trips
//
  LoadString(hInst, (RPT33PARMS.flags & TMSRPT33_INCLUDESCHOOL_FLAG) ? TEXT_250 : TEXT_251,
        szarString, sizeof(szarString));
  strcat(tempString, szarString);
  strcat(tempString, ", ");
//
//  Employee shuttles
//
  LoadString(hInst, (RPT33PARMS.flags & TMSRPT33_INCLUDESHUTTLE_FLAG) ? TEXT_252 : TEXT_253,
        szarString, sizeof(szarString));
  strcat(tempString, szarString);
//
//  Save it to the INI file
//
  WritePrivateProfileString("Reports", "CCReportName", tempString, TMSINIFile);

  return(TRUE);
}

BOOL CALLBACK TPDFILTERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static TMSRPT33Def *pRpt33Parms;
  static HANDLE hCtlBYROUTEONLY;
  static HANDLE hCtlBYROUTEANDDIRECTION;
  static HANDLE hCtlHHMM;
  static HANDLE hCtlDECIMAL;
  static HANDLE hCtlPERIODNAME[TMSRPT33_NUMPERIODS];
  static HANDLE hCtlPERIODFROM[TMSRPT33_NUMPERIODS];
  static HANDLE hCtlPERIODTO[TMSRPT33_NUMPERIODS];
  static HANDLE hCtlSCHOOL;
  static HANDLE hCtlSHUTTLE;
  static HANDLE hCtlNA;
  static HANDLE hCtlBEFORE;
  static HANDLE hCtlAFTER;
  static HANDLE hCtlTIME;
  long   time;
  long   tempLong;
  int    nI;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Validate the pointer
//
      pRpt33Parms = (TMSRPT33Def *)lParam;
      if(pRpt33Parms == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlBYROUTEONLY = GetDlgItem(hWndDlg, TPDFILTER_BYROUTEONLY);
      hCtlBYROUTEANDDIRECTION = GetDlgItem(hWndDlg, TPDFILTER_BYROUTEANDDIRECTION);
      hCtlHHMM = GetDlgItem(hWndDlg, TPDFILTER_HHMM);
      hCtlDECIMAL = GetDlgItem(hWndDlg, TPDFILTER_DECIMAL);
      hCtlPERIODNAME[0] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD1NAME);
      hCtlPERIODFROM[0] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD1FROM);
      hCtlPERIODTO[0] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD1TO);
      hCtlPERIODNAME[1] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD2NAME);
      hCtlPERIODFROM[1] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD2FROM);
      hCtlPERIODTO[1] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD2TO);
      hCtlPERIODNAME[2] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD3NAME);
      hCtlPERIODFROM[2] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD3FROM);
      hCtlPERIODTO[2] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD3TO);
      hCtlPERIODNAME[3] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD4NAME);
      hCtlPERIODFROM[3] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD4FROM);
      hCtlPERIODTO[3] = GetDlgItem(hWndDlg, TPDFILTER_PERIOD4TO);
      hCtlSCHOOL = GetDlgItem(hWndDlg, TPDFILTER_SCHOOL);
      hCtlSHUTTLE = GetDlgItem(hWndDlg, TPDFILTER_SHUTTLE);
      hCtlNA = GetDlgItem(hWndDlg, TPDFILTER_NA);
      hCtlBEFORE = GetDlgItem(hWndDlg, TPDFILTER_BEFORE);
      hCtlAFTER = GetDlgItem(hWndDlg, TPDFILTER_AFTER);
      hCtlTIME = GetDlgItem(hWndDlg, TPDFILTER_TIME);
//
//  Get whatever's in TMS.INI and populate the dialog
//
//  Flags
//
      tempLong = GetPrivateProfileInt(szSection, "flags", 0, TMSINIFile);
//
//  By route only or by route and direction
//
      if(tempLong & TMSRPT33_BYROUTEONLY_FLAG)
        SendMessage(hCtlBYROUTEONLY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlBYROUTEANDDIRECTION, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Output time format
//
      if(tempLong & TMSRPT33_HHMM_FLAG)
        SendMessage(hCtlHHMM, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlDECIMAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Include school routes
//
      if(tempLong & TMSRPT33_INCLUDESCHOOL_FLAG)
        SendMessage(hCtlSCHOOL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Include shuttle routes
//
      if(tempLong & TMSRPT33_INCLUDESHUTTLE_FLAG)
        SendMessage(hCtlSHUTTLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Before/after times
//
      if(!(tempLong & TMSRPT33_BEFORE_FLAG) && !(tempLong & TMSRPT33_AFTER_FLAG))
      {
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableWindow(hCtlTIME, FALSE);
      }
      else
      {
        time = GetPrivateProfileInt(szSection, "time", NO_TIME, TMSINIFile);
        if(tempLong & TMSRPT33_BEFORE_FLAG)
          SendMessage(hCtlBEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
          SendMessage(hCtlAFTER, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)Tchar(time));
      }
//
//  Period breakdowns
//
      for(nI = 0; nI < TMSRPT33_NUMPERIODS; nI++)
      {
//
//  Name
//
        sprintf(tempString, "Period%dName", nI + 1);
        GetPrivateProfileString(szSection, tempString, "",
              szarString, sizeof(szarString), TMSINIFile);
        SendMessage(hCtlPERIODNAME[nI], WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
//
//  From time
//
        sprintf(tempString, "Period%dFrom", nI + 1);
        GetPrivateProfileString(szSection, tempString, "",
              szarString, sizeof(szarString), TMSINIFile);
        SendMessage(hCtlPERIODFROM[nI], WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
//
//  To time
//
        sprintf(tempString, "Period%dTo", nI + 1);
        GetPrivateProfileString(szSection, tempString, "",
              szarString, sizeof(szarString), TMSINIFile);
        SendMessage(hCtlPERIODTO[nI], WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
      }
//
//  Done initializing
//
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  N/A - Before - After
//
        case TPDFILTER_NA:
        case TPDFILTER_BEFORE:
        case TPDFILTER_AFTER:
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableWindow(hCtlTIME, FALSE);
            SendMessage(hCtlTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          }
          else
          {
            EnableWindow(hCtlTIME, TRUE);
          }
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Get the values from the dialog
//
//  If before or after is selected, then there has to be a time
//
          if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                SendMessage(hCtlAFTER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlTIME, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_281, hCtlTIME);
              break;
            }
          }
//
//  Build up the flags
//
          pRpt33Parms->flags = 0;
//
//  By route only or by route and direction
//
          if(SendMessage(hCtlBYROUTEONLY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt33Parms->flags |= TMSRPT33_BYROUTEONLY_FLAG;
//
//  Output time format
//
          if(SendMessage(hCtlHHMM, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt33Parms->flags |= TMSRPT33_HHMM_FLAG;
//
//  Include school routes
//
          if(SendMessage(hCtlSCHOOL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt33Parms->flags |= TMSRPT33_INCLUDESCHOOL_FLAG;
//
//  Include shuttle routes
//
          if(SendMessage(hCtlSHUTTLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt33Parms->flags |= TMSRPT33_INCLUDESHUTTLE_FLAG;
//
//  Before/after times
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pRpt33Parms->flags |= TMSRPT33_ALLDAY_FLAG;
            pRpt33Parms->time = NO_TIME;
          }
          else
          {
            SendMessage(hCtlTIME, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRpt33Parms->time = cTime(tempString);
            if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              pRpt33Parms->flags |= TMSRPT33_BEFORE_FLAG;
            else
              pRpt33Parms->flags |= TMSRPT33_AFTER_FLAG;
          }
//
//  Period breakdowns
//
          for(nI = 0; nI < TMSRPT33_NUMPERIODS; nI++)
          {
//
//  Name
//
            SendMessage(hCtlPERIODNAME[nI], WM_GETTEXT,
                  (WPARAM)sizeof(pRpt33Parms->periods[nI].name),
                  (LONG)(LPSTR)pRpt33Parms->periods[nI].name);
//
//  From
//
            SendMessage(hCtlPERIODFROM[nI], WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRpt33Parms->periods[nI].from = cTime(tempString);
//
//  To
//
            SendMessage(hCtlPERIODTO[nI], WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRpt33Parms->periods[nI].to = cTime(tempString);
          }
//
//  Write the values out to TMS.INI
//
//  Flags
//
          ltoa(pRpt33Parms->flags, tempString, 10);
          WritePrivateProfileString(szSection, "flags", tempString, TMSINIFile);
          if(pRpt33Parms->flags & TMSRPT33_HHMM_FLAG)
            WritePrivateProfileString("Reports", "OutputTime", "HoursMinutes", TMSINIFile);
          else
            WritePrivateProfileString("Reports", "OutputTime", "Decimal", TMSINIFile);
//
//  Before / after times
//
          ltoa(pRpt33Parms->time, tempString, 10);
          WritePrivateProfileString(szSection, "time", tempString, TMSINIFile);
//
//  Period breakdowns
//
          for(nI = 0; nI < TMSRPT33_NUMPERIODS; nI++)
          {
//
//  Name
//
            sprintf(tempString, "Period%dName", nI + 1);
            WritePrivateProfileString(szSection, tempString,
              pRpt33Parms->periods[nI].name, TMSINIFile);
//
//  From time
//
            sprintf(tempString, "Period%dFrom", nI + 1);
            WritePrivateProfileString(szSection, tempString,
                  Tchar(pRpt33Parms->periods[nI].from), TMSINIFile);
//
//  To time
//
            sprintf(tempString, "Period%dTo", nI + 1);
            WritePrivateProfileString(szSection, tempString,
                  Tchar(pRpt33Parms->periods[nI].to), TMSINIFile);
          }
//
//  All done
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
