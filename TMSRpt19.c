//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
//
//  Dispatch Report (2)
//
//  From QETXT.INI:
//
//
//  [TMSRPT19]
//  FILE=Tmsrpt19.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Date,VARCHAR,64,0,64,0,
//  FIELD3=Operator,VARCHAR,51,0,51,0,
//  FIELD4=RunNumber,NUMERIC,5,0,5,0,
//  FIELD5=BlockNumber,NUMERIC,5,0,5,0,
//  FIELD6=Origin,VARCHAR,8,0,8,0,
//  FIELD7=TimeOfReport,VARCHAR,8,0,8,0,
//  FIELD8=OutTime,VARCHAR,8,0,8,0,
//  FIELD9=OffTime,VARCHAR,8,0,8,0,
//  FIELD10=Destination,VARCHAR,8,0,8,0,
//  FIELD11=ReportTime,VARCHAR,8,0,8,0,
//  FIELD12=PieceNumber,NUMERIC,5,0,5,0,
//  FIELD13=RelievedBy,NUMERIC,5,0,5,0,
//  FIELD14=StartNextPiece,VARCHAR,8,0,8,0,
//  FIELD15=RosterNumber,NUMERIC,5,0,5,0,
//  FIELD16=BusNumber,VARCHAR,16,0,16,0,
//

//
//  Dispatch sheet report
//

BOOL FAR TMSRPT19(TMSRPTPassedDataDef *pPassedData)
{
  DISPATCHSHEETPassedDataDef RParamData;
  DISPATCHSHEETRosterDataDef RData[MAXRUNSINRUNLIST];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef  COST;
  HFILE hfOutputFile;
  BOOL  bFound;
  char  lastName[DRIVERS_LASTNAME_LENGTH + 1];
  char  firstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  char  badgeNumber[DRIVERS_BADGENUMBER_LENGTH + 1];
  char  abbrName[NODES_ABBRNAME_LENGTH + 1];
  char  outputString[TEMPSTRING_LENGTH];
  char  *pszReportName;
  long  onTime;
  long  offTime;
  long  seq;
  long  DIVISIONSrecordID;
  long  xTime[2];
  long  pieceNumber;
  long  runNumber;
  long  cutAsRuntype;
  long  startTime;
  long  onNode;
  long  offNode;
  int   numRuns;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   numPieces;
  int   rcode2;
//
//  See what he wants to unload
//
  RParamData.flags = DISPATCHSHEET_FLAG_DISABLEOPTIONS;
  RParamData.nReportNumber = 18;
  if(DispatchSheet(&RParamData) != IDOK)
    return(FALSE);
  if(ROSTERPARMS.serviceDays[RParamData.dayOfWeek] == NO_RECORD)
  {
    TMSError(hWndMain, MB_ICONSTOP, ERROR_318, (HANDLE)NULL);
    return(FALSE);
  }
//
//  Back ok - set up the status bar
//
  pPassedData->nReportNumber = 18;
  pPassedData->numDataFiles = 1;
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
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT19.txt");
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
  StatusBar(-1L, -1L);
//
//  Loop through the roster to set up drivers, their associated
//  runs, and the start time of each run.  There's a data integrity
//  check here to ensure that the run pointed to by the roster actually exists.
//
  StatusBarText("Examining the roster...");
  numRuns = 0;
  ROSTERKey1.DIVISIONSrecordID = RParamData.DIVISIONSrecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == RParamData.DIVISIONSrecordID)
  {
    if(StatusBarAbort())
      goto deallocate;
    if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[RParamData.dayOfWeek] != NO_RECORD)
    {
      if(ROSTER.WEEK[m_RosterWeek].flags & (1 << RParamData.dayOfWeek))  // Crew-only run
      {
        CREWONLYKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[RParamData.dayOfWeek];
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        if(rcode2 == 0)
        {
          RData[numRuns].flags = RDATA_FLAG_CREWONLY;
          RData[numRuns].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
          RData[numRuns].RUNSrecordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[RParamData.dayOfWeek];
          RData[numRuns].startTime = CREWONLY.startTime;
          RData[numRuns].runNumber = CREWONLY.runNumber;
          RData[numRuns].rosterNumber = ROSTER.rosterNumber;
          RData[numRuns].blockNumber = NO_RECORD;
          RData[numRuns].TRIPSrecordID = NO_RECORD;
          RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
          RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
          RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
          RData[numRuns].BUSESrecordID = NO_RECORD;
          numRuns++;
        }
      }
      else
      {
        RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[RParamData.dayOfWeek];
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        if(rcode2 == 0)
        {
          RData[numRuns].flags = 0;
          RData[numRuns].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
          RData[numRuns].RUNSrecordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[RParamData.dayOfWeek];
          RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                NO_RECORD, NO_RECORD, &onTime, &offTime);
          RData[numRuns].startTime = onTime;
          RData[numRuns].runNumber = RUNS.runNumber;
          RData[numRuns].rosterNumber = ROSTER.rosterNumber;
          TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          RData[numRuns].TRIPSrecordID = TRIPS.recordID;
          RData[numRuns].blockNumber = TRIPS.standard.blockNumber;
          RData[numRuns].RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
          RData[numRuns].SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
          RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
          RData[numRuns].BUSESrecordID = NO_RECORD;
          numRuns++;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Now go through the runs to add any unassigned runs to RData
//
  StatusBarText("Setting up unassigned runs...");
  if(StatusBarAbort())
    goto deallocate;
  RUNSKey1.DIVISIONSrecordID = RParamData.DIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[RParamData.dayOfWeek];
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == RParamData.DIVISIONSrecordID &&
        RUNS.SERVICESrecordID == ROSTERPARMS.serviceDays[RParamData.dayOfWeek])
  {
    if(StatusBarAbort())
      goto deallocate;
    if(RUNS.pieceNumber == 1)
    {
      for(bFound = FALSE, nI = 0; nI < numRuns; nI++)
      {
        if(RUNS.recordID == RData[nI].RUNSrecordID &&
              !(RData[nI].flags & RDATA_FLAG_CREWONLY))
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        RData[numRuns].flags = 0;
        RData[numRuns].DRIVERSrecordID = NO_RECORD;
        RData[numRuns].RUNSrecordID = RUNS.recordID;
        RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
              NO_RECORD, NO_RECORD, &onTime, &offTime);
        RData[numRuns].startTime = onTime;
        RData[numRuns].runNumber = RUNS.runNumber;
        RData[numRuns].rosterNumber = 0;
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        RData[numRuns].blockNumber = TRIPS.standard.blockNumber;
        RData[numRuns].TRIPSrecordID = TRIPS.recordID;
        RData[numRuns].RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        RData[numRuns].SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
        RData[numRuns].BUSESrecordID = NO_RECORD;
        numRuns++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Now go through the runs to add any unassigned crew-only runs to RData
//
  CREWONLYKey1.DIVISIONSrecordID = RParamData.DIVISIONSrecordID;
  CREWONLYKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[RParamData.dayOfWeek];
  CREWONLYKey1.runNumber = NO_RECORD;
  CREWONLYKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
  while(rcode2 == 0 && 
        CREWONLY.DIVISIONSrecordID == RParamData.DIVISIONSrecordID &&
        CREWONLY.SERVICESrecordID == ROSTERPARMS.serviceDays[RParamData.dayOfWeek])
  {
    if(StatusBarAbort())
      goto deallocate;
    if(CREWONLY.pieceNumber == 1)
    {
      for(bFound = FALSE, nI = 0; nI < numRuns; nI++)
      {
        if(CREWONLY.recordID == RData[nI].RUNSrecordID &&
              (RData[nI].flags & RDATA_FLAG_CREWONLY))
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        RData[numRuns].flags = RDATA_FLAG_CREWONLY;
        RData[numRuns].DRIVERSrecordID = NO_RECORD;
        RData[numRuns].RUNSrecordID = CREWONLY.recordID;
        RData[numRuns].startTime = CREWONLY.startTime;
        RData[numRuns].runNumber = CREWONLY.runNumber;
        RData[numRuns].rosterNumber = 0;
        RData[numRuns].blockNumber = NO_RECORD;
        RData[numRuns].TRIPSrecordID = NO_RECORD;
        RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
        RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
        RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
        RData[numRuns].BUSESrecordID = NO_RECORD;
        numRuns++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
  }
//
//  Check for changes made via Daily Ops
//
  AdjustDispatchSheetRosterData(&RData[0], RParamData.todaysDate, RParamData.date, numRuns, TRUE, TRUE);
//
//  Generate the run details, and print them
//  out along with any associated driver data.
//
  StatusBarText("Generating run details...");
  seq = 0;
//
//  Initialize the travel matrix
//
  StatusBarEnd();
  SERVICESKey0.recordID = ROSTERPARMS.serviceDays[RParamData.dayOfWeek];
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  if(bUseDynamicTravels)
  {
    LoadString(hInst, bUseCISPlan ? TEXT_154 : TEXT_135, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
        goto deallocate;
    }
    else
    {
      InitTravelMatrix(SERVICES.recordID, FALSE);
    }
  }
  StatusBarStart(hWndMain, pszReportName);
//
//  Cycle through the runs
//
  for(nI = 0; nI < numRuns; nI++)
  {
    if(nI == 49)
    {
      nI = 49;
    }
    StatusBar((long)(nI + 1), (long)numRuns);
    if(StatusBarAbort())
    {
      goto deallocate;
    }
//
//  Run data
//
    if(RData[nI].flags & RDATA_FLAG_CREWONLY)
    {
      CREWONLYKey0.recordID = RData[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
      sprintf(tempString, "Processing run %ld (%ld/%ld)", CREWONLY.runNumber, nI + 1, numRuns);
    }
    else
    {
      RUNSKey0.recordID = RData[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      sprintf(tempString, "Processing run %ld (%ld/%ld)", RUNS.runNumber, nI + 1, numRuns);
    }
    StatusBarText(tempString);
//
//  Driver
//
    if(RData[nI].replacementDRIVERSrecordID != NO_RECORD)
    {
      RData[nI].DRIVERSrecordID = RData[nI].replacementDRIVERSrecordID;
    }
    if(RData[nI].DRIVERSrecordID != NO_RECORD)
    {
      DRIVERSKey0.recordID = RData[nI].DRIVERSrecordID;
    }
    else
    {
      DRIVERSKey0.recordID = NO_RECORD;
    }
    if(DRIVERSKey0.recordID != NO_RECORD)
    {
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(lastName, DRIVERS_LASTNAME_LENGTH);
      strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
    }
//
//  Run details
//
//  Crew only
//
    if(RData[nI].flags & RDATA_FLAG_CREWONLY)
    {
      btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
      pieceNumber = 0;
      runNumber = CREWONLY.runNumber;
      while(rcode2 == 0 && CREWONLY.runNumber == runNumber)
      {
        onTime = CREWONLY.startTime;
        offTime = CREWONLY.endTime;
        if(CREWONLY.pieceNumber == 1)
        {
          cutAsRuntype = CREWONLY.cutAsRuntype;
          startTime = onTime;
          onNode = CREWONLY.startNODESrecordID;
        }
        PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
        PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = CREWONLY.startNODESrecordID;
        PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = NO_RECORD;
        PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
        PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = CREWONLY.endNODESrecordID;
        PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = NO_RECORD;
        PROPOSEDRUN.piece[pieceNumber].prior.startTime = NO_TIME;
        PROPOSEDRUN.piece[pieceNumber].prior.endTime = NO_TIME;
        PROPOSEDRUN.piece[pieceNumber].after.startTime = NO_TIME;
        PROPOSEDRUN.piece[pieceNumber++].after.endTime = NO_TIME;
        offNode = CREWONLY.endNODESrecordID;
        rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
      }
      PROPOSEDRUN.numPieces = pieceNumber;
      RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
      numPieces = PROPOSEDRUN.numPieces;
      for(nJ = 0; nJ < numPieces; nJ++)
      {
        RUNSVIEW[nJ].platformTime = COST.PIECECOST[nJ].platformTime;
        RUNSVIEW[nJ].makeUpTime = COST.PIECECOST[nJ].makeUpTime;
        RUNSVIEW[nJ].reportTime = COST.PIECECOST[nJ].reportTime;
        RUNSVIEW[nJ].turninTime = COST.PIECECOST[nJ].turninTime;
        RUNSVIEW[nJ].travelTime = COST.TRAVEL[nJ].startTravelTime + COST.TRAVEL[nJ].endTravelTime;
        RUNSVIEW[nJ].paidBreaks = COST.PIECECOST[nJ].paidBreak;
        RUNSVIEW[nJ].blockNumber = NO_RECORD;
        RUNSVIEW[nJ].runOnNODESrecordID = PROPOSEDRUN.piece[nJ].fromNODESrecordID;
        RUNSVIEW[nJ].runOnTime = PROPOSEDRUN.piece[nJ].fromTime;
        RUNSVIEW[nJ].runOffTime = PROPOSEDRUN.piece[nJ].toTime;
        RUNSVIEW[nJ].runOffNODESrecordID = PROPOSEDRUN.piece[nJ].toNODESrecordID;
        RUNSVIEW[nJ].startOfPieceExtraboardStart = NO_TIME;
        RUNSVIEW[nJ].endOfPieceExtraboardStart = NO_TIME;
      }
    }
//
//  Regular run
//
    else
    {
      numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
    }
//
//  Loop through the pieces
//
    for(nJ = 0; nJ < numPieces; nJ++)
    {
//
//  Set up the output string
//
//  Sequence # (Piece start time), Service, Driver, Run Number
//
      if(RData[nI].DRIVERSrecordID != NO_RECORD)
      {
        sprintf(outputString, "%ld\t%s\t\"%s - %s, %s\"\t%ld\t",
              RUNSVIEW[nJ].runOnTime, RParamData.szDate, badgeNumber, lastName,
              firstName, RData[nI].runNumber);
      }
      else
      {
        sprintf(outputString, "%ld\t%s\t\t%ld\t",
              RUNSVIEW[nJ].runOnTime, RParamData.szDate, RData[nI].runNumber);
      }
//
//  Block number
//
      if(RUNSVIEW[nJ].blockNumber != NO_RECORD)
      {
        sprintf(tempString, "%ld\t", RUNSVIEW[nJ].blockNumber);
      }
      else
      {
        strcpy(tempString, "STBY\t");
      }
      strcat(outputString, tempString);
//      
//  Location of origin
//
      NODESKey0.recordID = RUNSVIEW[nJ].runOnNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcat(outputString, abbrName);
      strcat(outputString, "\t");
//
//  Time of report (show only if travel/report involved)
//
      if(COST.TRAVEL[nJ].startTravelTime > 0 || COST.PIECECOST[nJ].reportTime > 0)
      {
        strcat(outputString,
              Tchar(RUNSVIEW[nJ].runOnTime -
              COST.TRAVEL[nJ].startTravelTime - COST.PIECECOST[nJ].reportTime));
      }
      strcat(outputString, "\t");
// 
//  On time
//
      strcat(outputString, Tchar(RUNSVIEW[nJ].runOnTime));
      strcat(outputString, "\t");
//
//  Off time
//
      strcat(outputString, Tchar(RUNSVIEW[nJ].runOffTime));
      strcat(outputString, "\t");
//
//  Location of destination
//
      NODESKey0.recordID = RUNSVIEW[nJ].runOffNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcat(outputString, abbrName);
      strcat(outputString, "\t");
//
//  Report time
//
      if(RUNSVIEW[nJ].reportTime > 0)
        strcat(outputString, chhmm(RUNSVIEW[nJ].reportTime));
      strcat(outputString, "\t");
//
//  Piece number
//
      sprintf(tempString, "%d\t", nJ + 1);
      strcat(outputString, tempString);
//
//  Relieved by (not applicable to crew-only runs)
//
      if(!(RData[nJ].flags & RDATA_FLAG_CREWONLY))
      {
        for(nK = 0; nK < numRuns; nK++)
        {
          RUNSKey0.recordID = RData[nK].RUNSrecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          DIVISIONSrecordID = RUNS.DIVISIONSrecordID;
          bFound = FALSE;
          while(rcode2 == 0 &&
                RUNS.DIVISIONSrecordID == DIVISIONSrecordID &&
                RUNS.SERVICESrecordID == SERVICES.recordID && 
                RUNS.runNumber == RData[nK].runNumber)
          {
            if(RUNS.start.TRIPSrecordID == RUNSVIEW[nJ].runOffTRIPSrecordID && 
                  RUNS.start.NODESrecordID == RUNSVIEW[nJ].runOffNODESrecordID)
            {
              bFound = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          if(bFound)
          {
            sprintf(tempString, "%ld", RUNS.runNumber);
            strcat(outputString, tempString);
            break;
          }
        }
      }
      strcat(outputString, "\t");
//
//  Start of next piece
//
      if(nJ != numPieces - 1)
        strcat(outputString, Tchar(RUNSVIEW[nJ + 1].runOnTime));
      strcat(outputString, "\t");
//
//  Roster number
//
      sprintf(tempString, "%ld\t", RData[nI].rosterNumber);
      strcat(outputString, tempString);
//
//  Bus number
//
      if(RData[nI].BUSESrecordID != NO_RECORD)
      {
        BUSESKey0.recordID = RData[nI].BUSESrecordID;
        btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
        strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
        trim(tempString, BUSES_NUMBER_LENGTH);
        strcat(outputString, tempString);
      }
//
//  And one more tab to fill out the record
//
      strcat(outputString, "\t\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Deal with any extraboard on this piece
//
      for(nK = 0; nK < 2; nK++)
      {
        if(nK == 0)
        {
          xTime[0] = RUNSVIEW[nJ].startOfPieceExtraboardStart;
          xTime[1] = RUNSVIEW[nJ].startOfPieceExtraboardEnd;
        }
        else
        {
          xTime[0] = RUNSVIEW[nJ].endOfPieceExtraboardStart;
          xTime[1] = RUNSVIEW[nJ].endOfPieceExtraboardEnd;
        }
        if(xTime[0] == NO_TIME)
          continue;
//
//  Set up the output string
//
//  Sequence # (Piece start time), Service, Driver, Run Number, (No Block Number for XBD)
//
        if(RData[nI].DRIVERSrecordID != NO_RECORD)
        {
          sprintf(outputString, "%ld\t%s\t\"%s - %s, %s\"\t%ld\t\t",
                xTime[0], RParamData.szDate, badgeNumber, lastName,
                firstName, RData[nI].runNumber);
        }
        else
        {
          sprintf(outputString, "%ld\t%s\t\t%ld\t\t",
                xTime[0], RParamData.szDate, RData[nI].runNumber);
        }
//      
//  Location of origin 
//
        strcat(outputString, "THRU\t");
//
//  Time of report (show as "SPBD)
//
        strcat(outputString, "SPBD\t");
// 
//  On time
//
        strcat(outputString, Tchar(xTime[0]));
        strcat(outputString, "\t");
//
//  Off time
//
        strcat(outputString, Tchar(xTime[1]));
        strcat(outputString, "\t");
//
//  Location of destination
//
        strcat(outputString, "\t");
//
//  Report time left blank fot extraboard
//
        strcat(outputString, "\t");
//
//  Piece number
//
        sprintf(tempString, "%d\t", nJ + 1);
        strcat(outputString, tempString);
//
//  Relieved by
//
        if(nK == 1)
        {
          for(nL = 0; nL < numRuns; nL++)
          {
            RUNSKey0.recordID = RData[nL].RUNSrecordID;
            btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            DIVISIONSrecordID = RUNS.DIVISIONSrecordID;
            bFound = FALSE;
            while(rcode2 == 0 &&
                  RUNS.DIVISIONSrecordID == DIVISIONSrecordID &&
                  RUNS.SERVICESrecordID == SERVICES.recordID && 
                  RUNS.runNumber == RData[nL].runNumber)
            {
              if(RUNS.start.TRIPSrecordID == RUNSVIEW[nJ].runOffTRIPSrecordID && 
                    RUNS.start.NODESrecordID == RUNSVIEW[nJ].runOffNODESrecordID)
              {
                bFound = TRUE;
                break;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            }
            if(bFound)
            {
              sprintf(tempString, "%ld", RUNS.runNumber);
              strcat(outputString, tempString);
              break;
            }
          }
        }
        strcat(outputString, "\t");
//
//  Start of next piece
//
        if(nK == 0)
        {
          strcat(outputString, Tchar(RUNSVIEW[nJ].runOnTime));
        }
        else
        {
          if(nJ != numPieces - 1)
            strcat(outputString, Tchar(RUNSVIEW[nJ + 1].runOnTime));
        }
        strcat(outputString, "\t");
//
//  Roster number
//
        sprintf(tempString, "%ld\t", RData[nI].rosterNumber);
        strcat(outputString, tempString);
//
//  And one more tab to fill out the record
//
        strcat(outputString, "\t\n");
//
//  Write it out
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      }
    }
  }

//
//  All done
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);

  return(TRUE);
}
