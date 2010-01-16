//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//  From QETXT.INI
//
//  [TMSRPT03]
//  FILE=Tmsrpt03.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=RouteName,VARCHAR,40,0,40,0,
//  FIELD3=ServiceName,VARCHAR,16,0,16,0,
//  FIELD4=Direction,VARCHAR,16,0,16,0,
//  FIELD5=From,VARCHAR,4,0,4,0,
//  FIELD6=FTime,VARCHAR,5,0,5,0,
//  FIELD7=TTime,VARCHAR,5,0,5,0,
//  FIELD8=To,VARCHAR,4,0,4,0,
//  FIELD9=TripTime,NUMERIC,4,0,4,0,
//  FIELD10=Distance,NUMERIC,7,2,7,0,
//  FIELD11=BlockNumber,NUMERIC,7,0,7,0,
//  FIELD12=RGRPNumber,VARCHAR,8,0,8,0,
//  FIELD13=SGRPName,VARCHAR,16,0,16,0,
//  FIELD14=SeqNum,NUMERIC,5,0,5,0,
//  FIELD15=SignCode,VARCHAR,16,0,16,0,
//  FIELD16=PatternName,VARCHAR,16,0,16,0,
//
//  TMSRPT03() - Time and Miles by Route (Detailed)
//
#include "TMSHeader.h"

#define NUMCOLS 16

BOOL FAR TMSRPT03(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  long  statbarTotal;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   seqNum;
//
//  Local Database declarations
//
  ROUTESDef       LocalROUTES;
  TMSKey0Def      LocalROUTESKey0;
  ORDEREDLISTDef  LocalSERVICES;
  TMSKey0Def      LocalSERVICESKey0;

  pPassedData->nReportNumber = 2;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_COMMENTS;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Build the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt03.txt");
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
  seqNum = 0;
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
        continue;
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
          continue;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        if(rcode2 != 0)
          continue;
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
          strcpy(tempString, "");
          for(nL = 0; nL < NUMCOLS; nL++)
          {
            switch(nL)
            {
//
//  Route number
//
              case 0:
                strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                szarString[ROUTES_NUMBER_LENGTH] = '\0';
                break;
//
//  Route name
//
              case 1:
                strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
                szarString[ROUTES_NAME_LENGTH] = '\0';
                break;
//
//  Service Name
//
              case 2:
                strncpy(szarString, SERVICES.name, sizeof(SERVICES.name));
                szarString[sizeof(SERVICES.name)] = '\0';
                break;
//
//  Direction
//
              case 3:
                strncpy(szarString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
                szarString[DIRECTIONS_LONGNAME_LENGTH] = '\0';
                break;
//
//  From node
//
              case 4:
                NODESKey0.recordID = GTResults.firstNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                szarString[NODES_ABBRNAME_LENGTH] = '\0';
                break;
//
//  From time
//
              case 5:
                strcpy(szarString, Tchar(GTResults.firstNodeTime));
                break;
//
//  To time
//
              case 6:
                strcpy(szarString, Tchar(GTResults.lastNodeTime));
                break;
//
//  To node
//
              case 7:
                NODESKey0.recordID = GTResults.lastNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                szarString[NODES_ABBRNAME_LENGTH] = '\0';
                break;
//
//  Trip time (in minutes)
//
              case 8:
                sprintf(szarString, "%ld", GTResults.lastNodeTime - GTResults.firstNodeTime);
                break;
//
//  Distance
//
              case 9:
                sprintf(szarString, "%7.2f", GTResults.tripDistance);
                break;
//
//  Block number
//
              case 10:
                if(TRIPS.standard.blockNumber == NO_RECORD || TRIPS.standard.blockNumber == 0)
                  strcpy(szarString, "");
                else
                  sprintf(szarString, "%ld", TRIPS.standard.blockNumber);
                break;
//
//  RGRP
//
              case 11:
                if(TRIPS.standard.blockNumber == NO_RECORD || TRIPS.standard.blockNumber == 0)
                  strcpy(szarString, "");
                else
                {
                  LocalROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
                  btrieve(B_GETEQUAL, TMS_ROUTES, &LocalROUTES, &LocalROUTESKey0, 0);
                  strncpy(szarString, LocalROUTES.number, sizeof(LocalROUTES.number));
                  szarString[sizeof(LocalROUTES.number)] = '\0';
                }
                break;
//
//  SGRP
//
              case 12:
                if(TRIPS.standard.blockNumber == NO_RECORD || TRIPS.standard.blockNumber == 0)
                  strcpy(szarString, "");
                else
                {
                  LocalSERVICESKey0.recordID = TRIPS.standard.SGRPSERVICESrecordID;
                  btrieve(B_GETEQUAL, TMS_SERVICES, &LocalSERVICES, &LocalSERVICESKey0, 0);
                  strncpy(szarString, LocalSERVICES.name, sizeof(LocalSERVICES.name));
                  szarString[sizeof(LocalSERVICES.name)] = '\0';
                }
                break;
//
//  Trip Sequence
//
              case 13:
                sprintf(szarString, "%d", seqNum++);
                break;
//
//  Sign code
//
              case 14:
                if(TRIPS.SIGNCODESrecordID == NO_RECORD)
                {
                  strcpy(szarString, "");
                }
                else
                {
                  SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
                  btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
                  strncpy(szarString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
                  trim(szarString, SIGNCODES_CODE_LENGTH);
                }
                break;
//
//  Pattern name
//
              case 15:
                PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
                btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
                trim(szarString, PATTERNNAMES_NAME_LENGTH);
                break;
            }  // End of switch
            strcat(tempString, szarString);
            strcat(tempString, nL == NUMCOLS - 1 ? "\r\n" : "\t");
          }  // nL
          _lwrite(hfOutputFile, tempString, strlen(tempString));
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

