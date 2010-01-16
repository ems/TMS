//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT32() - Download Schedule Data to WMATA in Excel Format
//

#include "TMSHeader.h"

extern int SetRecordBodyForString(short int *, short int *, char *, BYTE *);
extern int SetRecordBodyForDouble(short int *, short int *, double *, BYTE *);
extern int SetRecordBodyForShortInteger(short int *, short int *, short int *, BYTE *);

extern char *TcharAMPMAM(long);

BOOL FAR TMSRPT32(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HANDLE  hOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  *ptr;
  long  BASEPatternNodeList[RECORDIDS_KEPT];
  long  subPatternNodeList[RECORDIDS_KEPT];
  int   numComments = 0;
  int   maxSelectedPatternNodes = 50;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numBASENodes;
  int   numNodes;
  int   rcode2;
  int   timeFormatSave = timeFormat;
  BYTE  recordBody[512];
  int   recordBodyLength;
  short int rowNumber;
  short int colNumber;

  long  *pSelectedPatternNodes = NULL;

  pPassedData->nReportNumber = 31;
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
  strcat(tempString, "\\TMSRPT32.XLS");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
//
//  Write out the id record
//
        colNumber = 0;
        recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, serviceName, recordBody);
        WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        tempString[ROUTES_NUMBER_LENGTH] = '\0';
        strcat(tempString, " ");
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        szarString[ROUTES_NAME_LENGTH] = '\0';
        strcat(tempString, szarString);
        strcat(tempString, " ");
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        szarString[DIRECTIONS_ABBRNAME_LENGTH] = '\0';
        strcat(tempString, szarString);
        strcat(tempString, " ");
        strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
        szarString[SERVICES_NAME_LENGTH] = '\0';
//
//  Send the id record to the excel file
//
        colNumber = 1;
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
//  Output the long names
//
        for(nL = 0; nL < numBASENodes; nL++)
        {
          colNumber = nL;
          NODESKey0.recordID = BASEPatternNodeList[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          trim(tempString, NODES_LONGNAME_LENGTH);
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
//  Get the trip times
//
//  If necessary, get the pattern nodes on the trip
//
          if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            for(nL = 0; nL < numBASENodes; nL++)
            {
              strcpy(tempString, TcharAMPMAM(GTResults.tripTimes[nL]));
              colNumber = nL;
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
              if(nM >= numNodes)
              {
                break;
              }
              if(BASEPatternNodeList[nL] == subPatternNodeList[nM])
              {
                strcpy(tempString, TcharAMPMAM(GTResults.tripTimes[nM]));
                colNumber = nL;
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
                nM++;
              }
            }
          }
          rowNumber++;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
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
  strcpy(tempString, "TMSRpt32.xls was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  return(TRUE);
}
