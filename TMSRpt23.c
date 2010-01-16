//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT23() - Stop Listing
//
//
//  From QETXT.INI:
//
//  [TMSRPT23]
//  FILE=Tmsrpt23.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Service,VARCHAR,32,0,32,0,
//  FIELD3=Route,VARCHAR,75,0,75,0,
//  FIELD4=Direction,VARCHAR,16,0,16,0,
//  FIELD5=Jurisdiction,VARCHAR,32,0,32,0,
//  FIELD6=CommentCode,VARCHAR,8,0,8,0,
//  FIELD7=AbbrName,VARCHAR,4,0,4,0,
//  FIELD8=LongName,VARCHAR,8,0,8,0,
//  FIELD9=Intersection,VARCHAR,64,0,64,0,
//  FIELD10=Latitude,VARCHAR,10,0,10,0,
//  FIELD11=Longitude,VARCHAR,10,0,10,0,
//  FIELD12=Description,VARCHAR,64,0,64,0, 
//  FIELD13=DistanceToNext,NUMERIC,7,2,7,0
//
#include "TMSHeader.h"

#define NUMCOLS 6

BOOL FAR TMSRPT23(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  float longitude[2], latitude[2];
  float previousLongitude, previousLatitude;
  float distanceToNext;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  long  statbarTotal;
  long  previousNode;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   seq;

  pPassedData->nReportNumber = 22;
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
  strcat(tempString, "\\TMSRPT23.txt");
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
  seq = 0;
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
    strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
    strcat(routeNumberAndName, " - ");
    strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(tempString, ROUTES_NAME_LENGTH);
    strcat(routeNumberAndName, tempString);
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
      sprintf(tempString, szarString, routeNumberAndName, serviceName);
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
        {
          continue;
        }
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Get the base pattern nodes
//
        previousNode = NO_RECORD;
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK &&
              PATTERNSKey2.PATTERNNAMESrecordID == basePatternRecordID)
        {
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Build the output string
//
//  Sequence, service, route, direction
//
          sprintf(outputString, "%d\t%s\t%s\t%s\t",
                seq++, serviceName, routeNumberAndName, directionName);
//
//  Jurisdiction
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
          }
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Comment code
//
          if(NODES.COMMENTSrecordID == NO_RECORD)
          {
            strcpy(tempString, "");
          }
          else
          {
            COMMENTSKey0.recordID = NODES.COMMENTSrecordID;
            btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
            strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(tempString, COMMENTS_CODE_LENGTH);
          }
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Node abbreviation
//
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Node long name
//
          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          trim(tempString, NODES_LONGNAME_LENGTH);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Node intersection
//
          strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
          trim(tempString, NODES_INTERSECTION_LENGTH);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Latitude
//
          sprintf(tempString, "%12.6f", NODES.latitude);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Longitude
//
          sprintf(tempString, "%12.6f", NODES.longitude);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Node description
//
          strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
          trim(tempString, NODES_DESCRIPTION_LENGTH);
          strcat(outputString, tempString);
          strcat(outputString, "\t");
//
//  Distance to next node
//
          distanceToNext = NO_RECORD;
          if(previousNode != NO_RECORD)
          {
//
//  Look first on the Connections Table for a distance
//
            CONNECTIONSKey1.fromNODESrecordID = previousNode;
            CONNECTIONSKey1.toNODESrecordID = NODES.recordID;
            CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
            if(rcode2 == 0 &&
                  CONNECTIONS.fromNODESrecordID == previousNode &&
                  CONNECTIONS.toNODESrecordID == NODES.recordID)
            {
              if(CONNECTIONS.distance > 0.0)
              {
                distanceToNext = CONNECTIONS.distance;
              }
            }
//
//  If no distance on the connections record, figure it out
//
            if(distanceToNext == NO_RECORD)
            {
              longitude[0] = previousLongitude;
              latitude[0] = previousLatitude;
              if(longitude[0] != 0.0 && latitude[0] != 0.0)
              {
                longitude[1] = NODES.longitude;
                latitude[1] = NODES.latitude;
                if(longitude[1] != 0.0 && latitude[1] != 0.0)
                {
                  distanceToNext = (float)GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
                }
              }
            }
          }
          previousLongitude = NODES.longitude;
          previousLatitude = NODES.latitude;
          previousNode = NODES.recordID;
          if(distanceToNext == NO_RECORD)
          {
            sprintf(tempString, "0.00");
          }
          else
          {
            sprintf(tempString, "%7.2f", distanceToNext);
          }
          strcat(outputString, tempString);
          strcat(outputString, "\r\n");
//
//  Write it out
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next node in the BASE pattern
//
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
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

