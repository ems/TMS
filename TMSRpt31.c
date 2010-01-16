//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT31() - Bus Stops Download to Excel
//
#include "TMSHeader.h"

int SetRecordBodyForString(short int *, short int *, char *, BYTE *);
int SetRecordBodyForDouble(short int *, short int *, double *, BYTE *);
int SetRecordBodyForShortInteger(short int *, short int *, short int *, BYTE *);

#define NUMCOLS 6

BOOL FAR TMSRPT31(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef REPORTPARMS;
  HANDLE hOutputFile;
  double distance;
  double longitude;
  double prevLongitude;
  double latitude;
  double prevLatitude;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char  nodeName[NODES_ABBRNAME_LENGTH + 1];
  char  intersection[NODES_INTERSECTION_LENGTH + 1];
  char  *ptr;
  BYTE  recordBody[512];
  long  statbarTotal;
  long  prevPattern;
  long  prevNode;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   recordBodyLength;
  short int sequenceNumber;
  short int rowNumber;
  short int colNumber;
  short int previousRow;

  pPassedData->nReportNumber = 30;
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
    return(FALSE);
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
    goto deallocate;
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\TMSRPT31.XLS");
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
//  Write out the header for the Excel file
//
  recordBody[0] = 0x02;
  recordBody[1] = 0x00;
  recordBody[2] = 0x10;
  recordBody[3] = 0x00;
  WriteExcel(EXCEL_BOF, hOutputFile, recordBody, 4);
//
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  rowNumber = 0;
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
      continue;
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
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
      LoadString(hInst, TEXT_118, szFormatString, sizeof(szFormatString));
      strcpy(szarString, routeNumber);
      strcat(szarString, " - ");
      strcat(szarString, routeName);
      sprintf(tempString, szFormatString, szarString, serviceName);
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
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Get the base pattern nodes
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        prevPattern = NO_RECORD;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK)
        {
//
//  Determine a change in pattern
//
          if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
          {
            prevNode = NO_RECORD;
            prevPattern = PATTERNS.PATTERNNAMESrecordID;
            sequenceNumber = 1;
          }
//
//  Start outputting the data
//
//  Route number
//
          colNumber = 0;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, routeNumber, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Route name
//
          colNumber = 1;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, routeName, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Service name
//
          colNumber = 2;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, serviceName, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);

//  Direction name
//
          colNumber = 3;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, directionName, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Pattern name
//
          PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
          btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          trim(patternName, PATTERNNAMES_NAME_LENGTH);
          colNumber = 4;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, patternName, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Sequence number
//
          colNumber = 5;
          recordBodyLength =
                SetRecordBodyForShortInteger(&rowNumber, &colNumber, &sequenceNumber, recordBody);
          WriteExcel(EXCEL_INTEGER, hOutputFile, recordBody, recordBodyLength);
          sequenceNumber++;
//
//  Node abbreviation
//
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(nodeName, NODES_ABBRNAME_LENGTH);
          colNumber = 6;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, nodeName, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Node intersection
//
          strncpy(intersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
          trim(intersection, NODES_INTERSECTION_LENGTH);
          colNumber = 7;
          recordBodyLength =
                SetRecordBodyForString(&rowNumber, &colNumber, intersection, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Latitude
//
          latitude = NODES.latitude;
          colNumber = 8;
          recordBodyLength =
                SetRecordBodyForDouble(&rowNumber, &colNumber, &latitude, recordBody);
          WriteExcel(EXCEL_NUMBER, hOutputFile, recordBody, recordBodyLength);
//
//  Longitude
//
          longitude = NODES.longitude;
          colNumber = 9;
          recordBodyLength =
                SetRecordBodyForDouble(&rowNumber, &colNumber, &longitude, recordBody);
          WriteExcel(EXCEL_NUMBER, hOutputFile, recordBody, recordBodyLength);
//
//  Distance to next node
//

          if(prevNode != NO_RECORD)
          {
            distance = GreatCircleDistance(prevLongitude, prevLatitude, longitude, latitude);
            colNumber = 10;
            previousRow = rowNumber - 1;
            recordBodyLength =
                  SetRecordBodyForDouble(&previousRow, &colNumber, &distance, recordBody);
            WriteExcel(EXCEL_NUMBER, hOutputFile, recordBody, recordBodyLength);
          }
          prevNode = NODES.recordID;
          prevLatitude = latitude;
          prevLongitude = longitude;
//
//  Bump up the row and get the next node in the BASE pattern
//
          rowNumber++;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while
      }  // nK
    }  // nJ
  }  // nI
//
//  Write out the EOF marker for the Excel file
//
  WriteExcel(EXCEL_EOF, hOutputFile, NULL, NO_RECORD);
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    CloseHandle(hOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
//  Let him know
//
  strcpy(tempString, "TMSRPT31.xls was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  return(TRUE);
}

int SetRecordBodyForString(short int *pRowNumber,
      short int *pColNumber, char *string, BYTE *recordBody)
{
  BYTE stringLength = (BYTE)strlen(string);

  memcpy(&recordBody[0], pRowNumber, 2);
  memcpy(&recordBody[2], pColNumber, 2);
  memset(&recordBody[4], 0x00, 3);
  memcpy(&recordBody[7], &stringLength, 1);
  memcpy(&recordBody[8], string, stringLength);

  return(stringLength + 9);
}

int SetRecordBodyForDouble(short int *pRowNumber,
      short int *pColNumber, double *pDoubleNumber, BYTE *recordBody)
{
  memcpy(&recordBody[0], pRowNumber, 2);
  memcpy(&recordBody[2], pColNumber, 2);
  memset(&recordBody[4], 0x00, 3);
  memcpy(&recordBody[7], pDoubleNumber, 8);

  return(15);
}

int SetRecordBodyForShortInteger(short int *pRowNumber,
      short int *pColNumber, short int *pNumber, BYTE *recordBody)
{
  memcpy(&recordBody[0], pRowNumber, 2);
  memcpy(&recordBody[2], pColNumber, 2);
  memset(&recordBody[4], 0x00, 3);
  memcpy(&recordBody[7], pNumber, 2);

  return(9);
}