//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

//
//  TMS Unload to the GRATA AS/400
//
BOOL FAR TMSRPT17(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  float distance;
  HFILE hfOutputFile[5];
  HFILE hfErrorLog;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  long  badgeNumber;
  long  servicesRecordID;
  char  outputString[1024];
  char  *ptr;
  char  *pszReportName;
  int   nI;
  int   nJ;
  int   rcode2;
  int   counter;
  int   pieceNumber;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  long  onTimes[MAXPIECES];
  long  offTime;
  long  cutAsRuntype;
  long  runNumber;
  long  reportTime;
  long  travelTime;

  pPassedData->nReportNumber = 16;
//
//  Open the error log and fire up the status bar
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\error.log");
  hfErrorLog = _lcreat(tempString, 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    bFinishedOK = FALSE;
    goto done;
  }
  bGotError = FALSE;
  bFinishedOK = FALSE;
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
//  GRATA TARUNMST Unload
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\tarunmst.txt");
  hfOutputFile[0] = _lcreat(tempString, 0);
  if(hfOutputFile[0] == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Set up the dynamic travels, if necessary
//
//  This code assumes that the first three services are
//  Weekday, Saturday-A, and Saturday-B.
//
  StatusBarEnd();
  if(bUseCISPlan)
  {
    CISfree();
    if(!CISbuild(FALSE, TRUE))
    {
      bFinishedOK = FALSE;
      goto done;
    }
  }
  StatusBarStart(hWndMain, pszReportName);
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  for(nI = 0; nI < 3; nI++)
  {
    if(rcode2 != 0)
      break;
    if(bUseDynamicTravels && !bUseCISPlan)
    {
      LoadString(hInst, TEXT_135, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      InitTravelMatrix(SERVICES.recordID, FALSE);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  ASCII file layout
//
//  Note: All times are in seconds after midnight
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//    1     Weekday/Saturday (W or S)
//   2-6    Run number
//   7-14   Route number
//  15-19   Block number
//  20-21   Piece number
//    22    Wheelchair (W or blank)
//    23    School code (S or blank)
//  24-28   Platform time
//  29-33   Report time
//  34-38   Travel time
//  39-43   Daily guarantee
//  44-48   Spread time
//  49-53   Hours paid
//  54-58   Overtime paid
//  59-65   Days operated (7x 0 or 1)
//  66-72   Miles per day (F4.2)
//  73-77   Time off
//  78-82   Time due
//    83    Run/Tripper (R or T)
//  84-98   Employee number #1
//  89-93   Employee number #2
//  94-98   Employee number #3
//  99-103  School straight time
// 
//  Go through the roster
//
  ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DivisionRecordID)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Get the driver
//
    if(ROSTER.DRIVERSrecordID == NO_RECORD)
      badgeNumber = 0;
    else
    {
      DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      badgeNumber = atol(tempString);
    }
//
//  Loop through all the days
//
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
        continue;
//
//  Get the run
//
      RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI];
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
//
//  Reposition the run
//
      servicesRecordID = RUNS.SERVICESrecordID;
      runNumber = RUNS.runNumber;
      RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
      RUNSKey1.SERVICESrecordID = servicesRecordID;
      RUNSKey1.runNumber = runNumber;
      RUNSKey1.pieceNumber = 1;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      pieceNumber = 0;
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
            RUNS.SERVICESrecordID == servicesRecordID &&
            RUNS.runNumber == runNumber)
      {
//
//  Generate the start trip
//
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullout?
//
        if(TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID)
        {
          GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
          GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
          GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
          GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
          GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
          GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
          GCTData.timeOfDay = GTResults.firstNodeTime;
          onTimes[pieceNumber] = GTResults.firstNodeTime -
                GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
        }
//
//  Nope - Find the node on the pattern
//
        else
        {
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          counter = 0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if((bFound = PATTERNS.NODESrecordID == RUNS.start.NODESrecordID) == TRUE)
              break;
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              counter++;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          onTimes[pieceNumber] = bFound ? GTResults.tripTimes[counter] : NO_TIME;
        }
//
//  Generate the end trip
//
        TRIPSKey0.recordID = RUNS.end.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullin?
//
        if(TRIPS.standard.PIGNODESrecordID == RUNS.end.NODESrecordID)
        {
          GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
          GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
          GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
          GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
          GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
          GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
          GCTData.timeOfDay = GTResults.lastNodeTime;
          offTime = GTResults.lastNodeTime +
                GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
        }
//
//  Nope - Find the node on the pattern
//
        else
        {
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          counter = 0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if((bFound = PATTERNS.NODESrecordID == RUNS.end.NODESrecordID) == TRUE)
              break;
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              counter++;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          offTime = bFound ? GTResults.tripTimes[counter] : NO_TIME;
        }
//
//  Set up PROPOSEDRUN
//
        PROPOSEDRUN.piece[pieceNumber].fromTime = onTimes[pieceNumber];
        PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
        PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
        PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
        PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
        PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
        PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
        PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
        PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
        PROPOSEDRUN.piece[pieceNumber].after.endTime = RUNS.prior.endTime;
//
//  And the runtype
//
        cutAsRuntype = RUNS.cutAsRuntype;
//
//  Get the next run record
//
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        pieceNumber++;
      }
//
//  Cost the run
//
      PROPOSEDRUN.numPieces = pieceNumber;
      RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  Loop through all the pieces
//
      for(nJ = 0; nJ < pieceNumber; nJ++)
      {
        memset(&outputString, 0x00, sizeof(outputString));
//
//  Weekday/Saturday
//
        outputString[0] = (nI < 5 ? 'W' : 'S');
//
//  Run number
//
        sprintf(tempString, "%5ld", runNumber);
        strncpy(&outputString[1], tempString, 5);
//
//  Route number
//
        TRIPSKey0.recordID = PROPOSEDRUN.piece[nJ].fromTRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        strncpy(&outputString[6], ROUTES.number, 8);
//
//  Block number
//
        sprintf(tempString, "%5ld", TRIPS.standard.blockNumber);
        strncpy(&outputString[14], tempString, 5);
//
//  Piece number
//
        sprintf(tempString, "%2d", nJ + 1);
        strncpy(&outputString[19], tempString, 2);
//
//  Wheelchair code
//
        if(TRIPS.BUSTYPESrecordID == NO_RECORD)
          outputString[21] = ' ';
        else
        {
          BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
          if(rcode2 != 0)
          {
            continue;
          }
          if(BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE)
            outputString[21] = 'W';
          else
            outputString[21] = ' ';
        }
//
//  School code
//
        if(ROUTES.flags & ROUTES.flags & ROUTES_FLAG_SCHOOL)
          outputString[22] = 'S';
        else
          outputString[22] = ' ';   
//
//  Platform time
//
        sprintf(tempString, "%5ld", COST.PIECECOST[nJ].platformTime);
        strncpy(&outputString[23], tempString, 5);
//
//  Report time
//
        reportTime = COST.PIECECOST[nJ].reportTime;
        sprintf(tempString, "%5ld", reportTime);
        strncpy(&outputString[28], tempString, 5);
//
//  Travel time
//
        travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
        sprintf(tempString, "%5ld", travelTime);
        strncpy(&outputString[33], tempString, 5);
//
//  Daily guarantee
//
        sprintf(tempString, "%5ld", 0L);
        strncpy(&outputString[38], tempString, 5);
//
//  Spread time (on the run)
//
        sprintf(tempString, "%5ld",
              PROPOSEDRUN.piece[pieceNumber - 1].toTime - PROPOSEDRUN.piece[0].fromTime);
        strncpy(&outputString[43], tempString, 5);
//
//  Pay time (on the run)
//
        sprintf(tempString, "%5ld", COST.TOTAL.payTime);
        strncpy(&outputString[48], tempString, 5);
//
//  Overtime (on the run)
//
        sprintf(tempString, "%5ld", COST.TOTAL.overTime);
        strncpy(&outputString[53], tempString, 5);
//
//  Days operated (on the run)
//
        if(outputString[0] == 'W')
          strncpy(&outputString[58], "1111100", 7);
        else if(nI == 5)
          strncpy(&outputString[58], "0000010", 7);
        else
          strncpy(&outputString[58], "0000001", 7);
//
//  Miles per day
//
        strncpy(&outputString[65], "0000.00", 7);
//
//  Time off
//
        sprintf(tempString, "%5ld", PROPOSEDRUN.piece[nJ].toTime);
        strncpy(&outputString[72], tempString, 5);
//
//  Time due
//
        sprintf(tempString, "%5ld", PROPOSEDRUN.piece[nJ].fromTime);
        strncpy(&outputString[77], tempString, 5);
//
//  Run or Tripper
//
        if(COST.TOTAL.payTime < 60 * 60 * 7)  // 7:00
          outputString[82] = 'T';
        else
          outputString[82] = 'R';
//
//  Employee number
//
        memset(&outputString[83], ' ', 15);
        sprintf(tempString, "%5ld", badgeNumber);
        if(outputString[0] == 'W')
          strncpy(&outputString[83], tempString, 5);
        else if(nI == 5)
          strncpy(&outputString[88], tempString, 5);
        else
          strncpy(&outputString[93], tempString, 5);
//
//  School straight time
//
        sprintf(tempString, "%5ld", 0L);
        strncpy(&outputString[98], tempString, 5);
//
//  Write it out
//
        outputString[103] = '\r';
        outputString[104] = '\n';
        _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      }  // nJ to numPieces
    }  // nI to MAX_ROSTER_DAYS
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }  // while
  _lclose(hfOutputFile[0]);
//
//  All done
//
  bFinishedOK = TRUE;

  done:

  StatusBarEnd();
  strcpy(tempString, "Unloaded file was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);
  if(!bFinishedOK)
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  if(bGotError)
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_228, (HANDLE)NULL);
  _lclose(hfErrorLog);

  return(TRUE);
}
