//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT04() - Time and Miles by Route (Summary)
//
//  From QETXT.INI:
//  
//  [TMSRPT04]
//  FILE=Tmsrpt04.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=RouteName,VARCHAR,64,0,64,0,
//  FIELD3=ServiceName,VARCHAR,16,0,16,0,
//  FIELD4=Direction,VARCHAR,16,0,16,0,
//  FIELD5=TripTime,NUMERIC,7,0,7,0,
//  FIELD6=Distance,NUMERIC,7,2,7,0,
//  
#include "TMSHeader.h"

#define NUMCOLS 6

BOOL FAR TMSRPT04(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  outputString[512];
  long  statbarTotal;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;

  pPassedData->nReportNumber = 3;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_COMMENTS;
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
  strcat(tempString, "\\tmsrpt04.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
    {
      continue;
    }
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Loop through the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
      LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
      sprintf(tempString, szarString, routeNumber, serviceName);
      StatusBarText(tempString);
      StatusBar((long)(nI * REPORTPARMS.numServices + nJ), (long)statbarTotal);
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
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
        {
          continue;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        m_bEstablishRUNTIMES = TRUE;
//
//  Get all the trips
//
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              TRIPS.directionIndex == nK)
        {
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Set up the output string
//
          strcpy(outputString, "");
          for(nL = 0; nL < NUMCOLS; nL++)
          {
            switch(nL)
            {
//
//  Route number
//
              case 0:
                strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                tempString[ROUTES_NUMBER_LENGTH] = '\0';
                sprintf(szarString, "\"%s\"", tempString);
                break;
//
//  Route name
//
              case 1:
                strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
                tempString[ROUTES_NAME_LENGTH] = '\0';
                sprintf(szarString, "\"%s\"", tempString);
                break;
//
//  Service Name
//
              case 2:
                strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
                tempString[SERVICES_NAME_LENGTH] = '\0';
                sprintf(szarString, "\"%s\"", tempString);
                break;
//
//  Direction
//
              case 3:
                strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
                tempString[DIRECTIONS_LONGNAME_LENGTH] = '\0';
                sprintf(szarString, "\"%s\"", tempString);
                break;
//
//  Trip time (in minutes)
//
              case 4:
                sprintf(szarString, "%ld", GTResults.lastNodeTime - GTResults.firstNodeTime);
                break;
//
//  Distance
//
              case 5:
                sprintf(szarString, "%7.2f", GTResults.tripDistance);
                break;
            }  // End of switch
            strcat(outputString, szarString);
            strcat(outputString, nL == NUMCOLS - 1 ? "\r\n" : "\t");
          }  // nL
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    _lclose(hfOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}

