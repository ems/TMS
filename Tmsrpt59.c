//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT59() - Download Schedule Data to BC Transit in Excel Format
//

#include "TMSHeader.h"

extern int SetRecordBodyForString(short int *, short int *, char *, BYTE *);
extern int SetRecordBodyForDouble(short int *, short int *, double *, BYTE *);
extern int SetRecordBodyForShortInteger(short int *, short int *, short int *, BYTE *);

char *TMSRPT59chhmm(long);

BOOL FAR TMSRPT59(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HANDLE  hOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bFound;
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  *ptr;
  char  *tempString2;
  long  BASEPatternNodeList[RECORDIDS_KEPT];
  long  subPatternNodeList[RECORDIDS_KEPT];
  long  commentRecordIDs[100];
  int   numComments;
  int   maxSelectedPatternNodes = 50;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numBASENodes;
  int   numNodes;
  int   rcode2;
  BYTE  recordBody[512];
  int   recordBodyLength;
  short int rowNumber;
  short int colNumber;

  long  *pSelectedPatternNodes = NULL;

  pPassedData->nReportNumber = 58;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.PATTERNNAMESrecordID = NO_RECORD;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_PATTERNNAMES | RPFLAG_COMMENTS;
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
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\TMSRPT59.XLS");
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
//  Allocate the nodes arrays
//
  pSelectedPatternNodes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pSelectedPatternNodes == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
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
  rowNumber = 0;
  SetCursor(hCursorWait);
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
    {
      continue;
    }
//
//  Loop through the directions
//
    for(nK = 0; nK < 2; nK++)
    {
      if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
      {
        continue;
      }
//
//  Loop through the services
//
      for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
      {
        SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
        rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(serviceName, SERVICES_NAME_LENGTH);
//
//  If there are no trips on this route/ser/dir, skip it
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        if(rcode2 != 0)
        {
          continue;
        }
        if(TRIPS.ROUTESrecordID != ROUTES.recordID ||
              TRIPS.SERVICESrecordID != SERVICES.recordID ||
              TRIPS.directionIndex != nK)
        {
          continue;
        }
//
//  Write out the route record
//
        colNumber = 1;
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strcat(tempString, " ");
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        szarString[ROUTES_NAME_LENGTH] = '\0';
        strcat(tempString, szarString);
        recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
        WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        rowNumber++;
//
//  Write out the direction record
//
        colNumber = 1;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
        WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        rowNumber++;
//
//  Write out the service record
//
        colNumber = 1;
        strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(tempString, SERVICES_NAME_LENGTH);
        recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
        WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        rowNumber++;
//
//  Get the BASE pattern nodes for this route, service, and direction
//
        PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        numBASENodes = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              PATTERNS.directionIndex == nK &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            BASEPatternNodeList[numBASENodes++] = PATTERNS.NODESrecordID;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while rcode2 == 0 on the pattern
//
//  Output the map codes starting in column 3
//
        for(nL = 0; nL < numBASENodes; nL++)
        {
          colNumber = nL + 2;
          NODESKey0.recordID = BASEPatternNodeList[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.mapCodes, NODES_MAPCODES_LENGTH);
          trim(tempString, NODES_MAPCODES_LENGTH);
          recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        }
        rowNumber++;
//
//  Output the long names
//
        for(nL = 0; nL < numBASENodes; nL++)
        {
          colNumber = nL + 2;
          NODESKey0.recordID = BASEPatternNodeList[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
          trim(tempString, NODES_DESCRIPTION_LENGTH);
          recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        }
        rowNumber++;
//
//  Go through all the trips for this route, service, and direction
//
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        numComments = 0;
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
//  Was there a comment?
//
          if(TRIPS.COMMENTSrecordID != NO_RECORD)
          {
            COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(tempString, COMMENTS_CODE_LENGTH);
            colNumber = 1;
            recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
            WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
            for(bFound = FALSE, nL = 0; nL < numComments; nL++)
            {
              if(commentRecordIDs[nL] == COMMENTS.recordID)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              commentRecordIDs[numComments] = COMMENTS.recordID;
              numComments++;
            }
          }
//
//  Get the trip times
//
//  If necessary, get the pattern nodes on the trip
//
          if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            for(nL = 0; nL < numBASENodes; nL++)
            {
              colNumber = nL + 2;
              strcpy(tempString, TMSRPT59chhmm(GTResults.tripTimes[nL]));
              recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
              WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
            }
          }
//
//  Not the BASE pattern - get the pattern and fill in the times
//
          else
          {
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = 0;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            numNodes = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                subPatternNodeList[numNodes++] = PATTERNS.NODESrecordID;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
//
//  Move along the pattern
//
            nM = 0;
            for(nL = 0; nL < numBASENodes; nL++)
            {
              colNumber = nL + 2;
              if(BASEPatternNodeList[nL] == subPatternNodeList[nM] && nM < numNodes)
              {
                strcpy(tempString, TMSRPT59chhmm(GTResults.tripTimes[nM]));
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
                nM++;
              }
              else
              {
                strcpy(tempString, "-");
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              }
            }
          }
          rowNumber++;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
//
//  Dump out the comments
//
        if(numComments > 0)
        {
          rowNumber++;
          for(nL = 0; nL < numComments; nL++)
          {
//
//  Code
//
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            COMMENTSKey0.recordID = commentRecordIDs[nL];
            btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(tempString, COMMENTS_CODE_LENGTH);
            colNumber = 1;
            recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
            WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Text
//
            szarString[0] = 0;
            for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n"); tempString2;
                 tempString2 = strtok(NULL, "\r\n") )
            {
              if(strlen(tempString2) >= SZARSTRING_LENGTH)
              {
                szarString[SZARSTRING_LENGTH - 1] = '\0';
              }
              strcpy(szarString, tempString2 );
              colNumber = 2;
              recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, szarString, recordBody);
              WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              rowNumber++;
            }
          }
        }
//
//  Add a blank line
//
        rowNumber++;
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
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
    TMSHeapFree(pSelectedPatternNodes);
    CloseHandle(hOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
//  Let him know
//
  strcpy(tempString, "TMSRPT59.xls was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  return(TRUE);
}

char *TMSRPT59chhmm( long t )
{
  static char szTime[9];
  int    hour, min;

  if(t == NO_TIME)
  {
    strcpy(szTime,"");
  }
  else
  {
    if(t < 0)
    {
      t += 86400L;
    }
    hour = t / 3600;
    hour %= 12;
    if(hour == 0)
    {
      hour = 12;
    }
    min = (t % 3600) / 60;
    sprintf(szTime, "%ld:%02ld", hour, min);
  }

  return(&szTime[0]);
}

