//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT81() - Download Schedule Data for C-Tran in Excel Format
//

#include "TMSHeader.h"

extern int SetRecordBodyForString(short int *, short int *, char *, BYTE *);
extern int SetRecordBodyForDouble(short int *, short int *, double *, BYTE *);
extern int SetRecordBodyForShortInteger(short int *, short int *, short int *, BYTE *);

BOOL FAR TMSRPT81(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HANDLE  hOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  *ptr;
  long  BASEPatternNodeList[2][RECORDIDS_KEPT];
  long  subPatternNodeList[RECORDIDS_KEPT];
  int   numComments = 0;
  int   maxSelectedPatternNodes = 50;
  int   maxTrips = 200;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numBASENodes[2];
  int   numNodes;
  int   rcode2;
  int   timeFormatSave = timeFormat;
  int   numTrips[2];
  int   numRows;
  BYTE  recordBody[512];
  int   recordBodyLength;
  short int rowNumber;
  short int colNumber;

  long  *pSelectedPatternNodes = NULL;
  long  *pOutboundTripRecordIDs = NULL;
  long  *pInboundTripRecordIDs = NULL;
  long  *lPtr = NULL;

  pPassedData->nReportNumber = 80;
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
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES;
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
  strcat(tempString, "\\C-Tran Excel Export.XLS");
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
//  Allocate the trips arrays
//
  pOutboundTripRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxTrips); 
  if(pOutboundTripRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
  pInboundTripRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxTrips); 
  if(pInboundTripRecordIDs == NULL)
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
        numTrips[nK] = 0;
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        lPtr = (nK == 0 ? pOutboundTripRecordIDs : pInboundTripRecordIDs);
//
//  Pass 1 through the trips - figure out
//  how many and save the recordIDs
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
          if(numTrips[nK] >= maxTrips)
          {
            maxTrips += 100;
            pOutboundTripRecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, pOutboundTripRecordIDs, sizeof(long) * maxTrips); 
            if(pOutboundTripRecordIDs == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
            goto deallocate;
            }
            pInboundTripRecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, pInboundTripRecordIDs, sizeof(long) * maxTrips); 
            if(pInboundTripRecordIDs == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
              goto deallocate;
            }
          }
          lPtr[numTrips[nK]] = TRIPS.recordID;
          numTrips[nK]++;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
      }
//
//  Gotta have trips
//
      if(numTrips[0] == 0)
      {
        continue;
      }
//
//  #{routeNumber} {service}
//
      strcpy(tempString, "#");
      strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szarString, ROUTES_NUMBER_LENGTH);
      strcat(tempString, szarString);
      strcat(tempString, " ");
      strcat(tempString, serviceName);
      colNumber = 5;
      recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
      WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
      rowNumber++;
//
//  Get the BASE pattern nodes for this route, service, and direction
//
      for(nK = 0; nK < 2; nK++)
      {
        numBASENodes[nK] = 0;
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              PATTERNS.directionIndex == nK &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            BASEPatternNodeList[nK][numBASENodes[nK]] = PATTERNS.NODESrecordID;
            numBASENodes[nK]++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while rcode2 == 0 on the pattern
      }
//
//  {direction} {direction}
//
      colNumber = 1;
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
      trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
      recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
      WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
      if(numTrips[1] > 0)
      {
        colNumber = numBASENodes[0] + 1;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
        WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
      }
      rowNumber++;
//
//  Output the abbreviated node names
//
      colNumber = 0;
      for(nK = 0; nK < 2; nK++)
      {
        for(nL = 0; nL < numBASENodes[nK]; nL++)
        {
          NODESKey0.recordID = BASEPatternNodeList[nK][nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
          WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
          colNumber++;
        }
        colNumber++;
      }
      rowNumber++;
//
//  Blank line
//
      rowNumber++;
//
//  Go through all the trips for this route and service
//
      numRows = max(numTrips[0], numTrips[1]);
      for(nK = 0; nK < numRows; nK++)
      {
//
//  Outbound
//
        if(nK < numTrips[0])
        {
          colNumber = 0;
          TRIPSKey0.recordID = pOutboundTripRecordIDs[nK];
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
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
            for(nL = 0; nL < numBASENodes[0]; nL++)
            {
              strcpy(tempString, TcharNAP(GTResults.tripTimes[nL], TRUE));
              recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
              WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              colNumber++;
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
            for(nL = 0; nL < numBASENodes[0]; nL++)
            {
              if(nM >= numNodes)
              {
                break;
              }
              if(BASEPatternNodeList[0][nL] == subPatternNodeList[nM])
              {
                strcpy(tempString, TcharNAP(GTResults.tripTimes[nM], TRUE));
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
                nM++;
              }
              else
              {
                strcpy(tempString, "X");
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              }
              colNumber++;
            }
          }
        }  // if nK < numTrips[0]
//
// Inbound
//
        if(nK < numTrips[1])
        {
          colNumber = numBASENodes[0] + 1;
          TRIPSKey0.recordID = pInboundTripRecordIDs[nK];
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
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
            for(nL = 0; nL < numBASENodes[1]; nL++)
            {
              strcpy(tempString, TcharNAP(GTResults.tripTimes[nL], TRUE));
              recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
              WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              colNumber++;
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
            for(nL = 0; nL < numBASENodes[1]; nL++)
            {
              if(nM >= numNodes)
              {
                break;
              }
              if(BASEPatternNodeList[1][nL] == subPatternNodeList[nM])
              {
                strcpy(tempString, TcharNAP(GTResults.tripTimes[nM], TRUE));
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
                nM++;
              }
              else
              {
                strcpy(tempString, "X");
                recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
                WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
              }
              colNumber++;
            }
          }
        }  // if nK < numTrips[1]
        rowNumber++;
      }  // nK
    }  // nJ
    rowNumber += 2;
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
    TMSHeapFree(pOutboundTripRecordIDs);
    TMSHeapFree(pInboundTripRecordIDs);
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
  strcpy(tempString, "C-Tran Excel Export.xls was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  return(TRUE);
}
