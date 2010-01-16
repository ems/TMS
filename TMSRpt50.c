// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
//
//  Event report
//
//  From QETXT.INI:
//
//  [TMSRPT50]
//  FILE=TMSRPT50.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Date,VARCHAR,64,0,64,0,
//  FIELD2=TimeInSeconds,NUMERIC,7,0,7,0,
//  FIELD3=Time,VARCHAR,16,0,16,0,
//  FIELD4=Text,VARCHAR,255,0,255,0,
//

BOOL FAR TMSRPT50(TMSRPTPassedDataDef *pPassedData)
{
  EVENTREPORTPassedDataDef   RParamData;
  DISPATCHSHEETRosterDataDef RData[MAXRUNSINRUNLIST];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef  COST;
  HFILE hfOutputFile;
  BOOL  bFound;
  char  abbrName[NODES_ABBRNAME_LENGTH + 1];
  char  outputString[TEMPSTRING_LENGTH];
  char  szAction[32];
  char  szOnAction[32];
  char  szOffAction[32];
  char  szActualDriver[DRIVERS_FIRSTNAME_LENGTH + 1 + DRIVERS_LASTNAME_LENGTH + 1];
  char  szScheduledDriver[DRIVERS_FIRSTNAME_LENGTH + 1 + DRIVERS_LASTNAME_LENGTH + 1];
  char  szBlockNumber[16];
  char  szOnLocation[NODES_ABBRNAME_LENGTH + 1];
  char  szOffLocation[NODES_ABBRNAME_LENGTH + 1];
  char  *pszReportName;
  long  onTime;
  long  offTime;
  long  seq;
  long  xTime[2];
  long  pieceNumber;
  long  runNumber;
  long  cutAsRuntype;
  long  startTime;
  long  onNode;
  long  offNode;
  long  outputFlags;
  int   numRuns;
  int   nI;
  int   nJ;
  int   nK;
  int   numPieces;
  int   rcode2;
//
//  This report is also called by DailyOps in order to set up its chronology
//  chronology for the day.  On a DailyOps call, TMSRPT_PASSEDDATAFLAG_FROMDAILYOPS
//  is passed in under pPassedData->flags.  When this flag is encountered, all
//  the Crystal stuff is bypassed (along with the dialog that asks him what
//  he wants to track.  Later on, the entire output schema is changed to write
//  out the intermediate text file as "TMSRPT50D.TXT", for reading in at the
//  DailyOps initialization level.
//
  BOOL bFromDailyOps = pPassedData->flags & TMSRPT_PASSEDDATAFLAG_FROMDAILYOPS;
  long DIVISIONSrecordID;
  long date;
  long todaysDate;
  int  dayOfWeek;
  int  numOutputRecords;
//
//
//  See what he wants to unload
//
  if(bFromDailyOps)
  {
    dayOfWeek = pPassedData->dayOfWeek;
    DIVISIONSrecordID = pPassedData->DIVISIONSrecordID;
    outputFlags = EVENTREPORT_FLAG_REPORTTIME | EVENTREPORT_FLAG_STARTOFPIECETRAVEL | EVENTREPORT_FLAG_STARTOFPIECE |
          EVENTREPORT_FLAG_ENDOFPIECE | EVENTREPORT_FLAG_ENDOFPIECETRAVEL | EVENTREPORT_FLAG_EXTRABOARDDUTY;
    date = pPassedData->date;
    todaysDate = date;
    numOutputRecords = 0;
  }
  else
  {
    RParamData.nReportNumber = 49;
    if(EventReport(&RParamData) != IDOK)
    {
      return(FALSE);
    }
//
//  If there's nothing happening on the selected day, leave
//
    dayOfWeek = RParamData.dayOfWeek;
    if(ROSTERPARMS.serviceDays[dayOfWeek] == NO_RECORD)
    {
      TMSError(hWndMain, MB_ICONSTOP, ERROR_318, (HANDLE)NULL);
      return(FALSE);
    }
    DIVISIONSrecordID = RParamData.DIVISIONSrecordID;
    outputFlags = RParamData.flags;
    date = RParamData.date;
    todaysDate = RParamData.todaysDate;
//
//  Back ok - set up the status bar
//
    pPassedData->nReportNumber = 49;
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
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  if(bFromDailyOps)
  {
    strcat(tempString, "\\TMSRPT50D.txt");
  }
  else
  {
    strcat(tempString, "\\TMSRPT50.txt");
  }
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  if(!bFromDailyOps)
  {
    strcpy(pPassedData->szReportDataFile[0], tempString);
    StatusBar(-1L, -1L);
    StatusBarText("Examining the roster...");
  }
//
//  Loop through the roster to set up drivers, their associated
//  runs, and the start time of each run.  There's a data integrity
//  check here to ensure that the run pointed to by the roster actually exists.
//
  numRuns = 0;
  ROSTERKey1.DIVISIONSrecordID = DIVISIONSrecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == DIVISIONSrecordID)
  {
    if(!bFromDailyOps)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
    }
    if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[dayOfWeek] != NO_RECORD)
    {
      if(ROSTER.WEEK[m_RosterWeek].flags & (1 << dayOfWeek))  // Crew-only run
      {
        CREWONLYKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[dayOfWeek];
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        if(rcode2 == 0)
        {
          RData[numRuns].flags = RDATA_FLAG_CREWONLY;
          RData[numRuns].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
          RData[numRuns].RUNSrecordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[dayOfWeek];
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
        RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[dayOfWeek];
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        if(rcode2 == 0)
        {
          RData[numRuns].flags = 0;
          RData[numRuns].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
          RData[numRuns].RUNSrecordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[dayOfWeek];
          RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID, NO_RECORD, NO_RECORD, &onTime, &offTime);
          RData[numRuns].startTime = onTime;
          RData[numRuns].runNumber = RUNS.runNumber;
          RData[numRuns].rosterNumber = ROSTER.rosterNumber;
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
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Now go through the runs to add any unassigned runs to RData
//
  if(!bFromDailyOps)
  {
    StatusBarText("Setting up unassigned runs...");
  }
  RUNSKey1.DIVISIONSrecordID = DIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[dayOfWeek];
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == DIVISIONSrecordID &&
        RUNS.SERVICESrecordID == ROSTERPARMS.serviceDays[dayOfWeek])
  {
    if(!bFromDailyOps)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
    }
    if(RUNS.pieceNumber == 1)
    {
      for(bFound = FALSE, nI = 0; nI < numRuns; nI++)
      {
        if(RUNS.recordID == RData[nI].RUNSrecordID && !(RData[nI].flags & RDATA_FLAG_CREWONLY))
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
        RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID, NO_RECORD, NO_RECORD, &onTime, &offTime);
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
  CREWONLYKey1.DIVISIONSrecordID = DIVISIONSrecordID;
  CREWONLYKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[dayOfWeek];
  CREWONLYKey1.runNumber = NO_RECORD;
  CREWONLYKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
  while(rcode2 == 0 && 
        CREWONLY.DIVISIONSrecordID == DIVISIONSrecordID &&
        CREWONLY.SERVICESrecordID == ROSTERPARMS.serviceDays[dayOfWeek])
  {
    if(!bFromDailyOps)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
    }
    if(CREWONLY.pieceNumber == 1)
    {
      for(bFound = FALSE, nI = 0; nI < numRuns; nI++)
      {
        if(CREWONLY.recordID == RData[nI].RUNSrecordID && (RData[nI].flags & RDATA_FLAG_CREWONLY))
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
  AdjustDispatchSheetRosterData(&RData[0], todaysDate, date, numRuns, TRUE, TRUE);
//
//  Generate the run details, and print them
//  out along with any associated driver data.
//
  if(!bFromDailyOps)
  {
    StatusBarText("Generating run details...");
    StatusBarEnd();
//
//  Initialize the travel matrix
//
    if(bUseDynamicTravels)
    {
      SERVICESKey0.recordID = ROSTERPARMS.serviceDays[dayOfWeek];
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      LoadString(hInst, bUseCISPlan ? TEXT_154 : TEXT_135, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      if(bUseCISPlan)
      {
        CISfree();
        if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
        {
          goto deallocate;
        }
      }
      else
      {
        InitTravelMatrix(SERVICES.recordID, FALSE);
      }
    }
  }
  seq = 0;
//
//  Cycle through the runs
//
  if(!bFromDailyOps)
  {
    StatusBarStart(hWndMain, pszReportName);
    StatusBarText("Cycling through the runs...");
  }
  for(nI = 0; nI < numRuns; nI++)
  {
    if(!bFromDailyOps)
    {
      StatusBar((long)(nI + 1), (long)numRuns);
      if(StatusBarAbort())
      {
        goto deallocate;
      }
    }
//
//  Run data
//
    if(RData[nI].flags & RDATA_FLAG_CREWONLY)
    {
      CREWONLYKey0.recordID = RData[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    }
    else
    {
      RUNSKey0.recordID = RData[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    }
//
//  Driver
//
    if(RData[nI].DRIVERSrecordID == NO_RECORD)
    {
      strcpy(szScheduledDriver, "Open");
    }
    else
    {
      DRIVERSKey0.recordID = RData[nI].DRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strcpy(szScheduledDriver, BuildOperatorString(NO_RECORD));
    }
//
//  Actual Driver
//
    if(RData[nI].replacementDRIVERSrecordID == NO_RECORD)
    {
      strcpy(szActualDriver, "");
    }
    else
    {
      DRIVERSKey0.recordID = RData[nI].replacementDRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strcpy(szActualDriver, BuildOperatorString(NO_RECORD));
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
      cutAsRuntype = NO_RECORD;
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
        RUNSVIEW[nJ].startOfPieceTravel = COST.TRAVEL[nJ].startTravelTime;
        RUNSVIEW[nJ].startOfPieceNODESrecordID = COST.TRAVEL[nJ].startAPointNODESrecordID;
        RUNSVIEW[nJ].endOfPieceTravel = COST.TRAVEL[nJ].endTravelTime;
        RUNSVIEW[nJ].endOfPieceNODESrecordID = COST.TRAVEL[nJ].endBPointNODESrecordID;
        RUNSVIEW[nJ].payTime = nJ == numPieces - 1 ? COST.TOTAL.payTime : NO_TIME;
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
//  Call from DailyOps
//
    if(bFromDailyOps)
    {
//
//  Cycle through the pieces
//
      for(nJ = 0; nJ < numPieces; nJ++)
      {
//
//  Get the stuff we'll need for most of the output records
//
//  Block number
//
        if(RUNSVIEW[nJ].blockNumber != NO_RECORD)
        {
          sprintf(szBlockNumber, "%ld", RUNSVIEW[nJ].blockNumber);
        }
        else
        {
          strcpy(szBlockNumber, "SBY\t");
        }
//
//  On location
//
        NODESKey0.recordID = RUNSVIEW[nJ].runOnNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szOnLocation, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        szOnLocation[NODES_ABBRNAME_LENGTH] = '\0';
        strcpy(szOnAction, ((NODES.flags & NODES_FLAG_GARAGE) ? "P/O" : "RLF"));
//
//  Force a relief start to be after a relief end
//
        if(!(NODES.flags & NODES_FLAG_GARAGE))
        {
          RUNSVIEW[nJ].runOnTime += 1;
        }
//
//  Off location
//
        NODESKey0.recordID = RUNSVIEW[nJ].runOffNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szOffLocation, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        szOffLocation[NODES_ABBRNAME_LENGTH] = '\0';
        strcpy(szOffAction, ((NODES.flags & NODES_FLAG_GARAGE) ? "P/I" : "RLF"));
//
//  EVENTREPORT_FLAG_REPORTTIME: Report at time
//
        if(outputFlags & EVENTREPORT_FLAG_REPORTTIME)
        {
          if(RUNSVIEW[nJ].reportTime > 0)
          {
//
//  Set up the output string: Date, on time in seconds, on time as character
//
           sprintf(outputString, "%ld\t%s\t",
                  RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime,
                  Tchar(RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime));
//
//  Run number, Piece number, Block number, Driver info.
//
            if(strcmp(szActualDriver, "") != 0)
            {
              sprintf(tempString, "%s:RPT: %s (was %s) run %ld, pce %d, blk %s",
                    szOnLocation, szActualDriver, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
            }
            else
            {
              sprintf(tempString, "%s:RPT: %s run %ld, pce %d, blk %s",
                    szOnLocation, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
            }
//
//  Write it out
//
            strcat(outputString, tempString);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            numOutputRecords++;
          }
        }
//
//  EVENTREPORT_FLAG_STARTOFPIECETRAVEL: Start of Piece Travel
//  
        if(outputFlags & EVENTREPORT_FLAG_STARTOFPIECETRAVEL)
        {
          if(RUNSVIEW[nJ].startOfPieceTravel > 0)
          {
          }
        }
//
//  EVENTREPORT_FLAG_STARTOFPIECE: Start of piece
//
        if(outputFlags & EVENTREPORT_FLAG_STARTOFPIECE)
        {
//
//  Set up the output string: Date, on time in seconds, on time as character
//
          sprintf(outputString, "%ld\t%s\t",
                RUNSVIEW[nJ].runOnTime, Tchar(RUNSVIEW[nJ].runOnTime));
//
//  Run number, Piece number, Block number, Driver info.
//
          if(strcmp(szActualDriver, "") != 0)
          {
            sprintf(tempString, "%s:%s: %s (was %s) start run %ld, pce %d, blk %s.",
                  szOnLocation, szOnAction, szActualDriver, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
          }
          else
          {
            sprintf(tempString, "%s:%s: %s start run %ld, pce %d, blk %s.",
                  szOnLocation, szOnAction, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
          }
//
//  Write it out
//
          strcat(outputString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          numOutputRecords++;
        }
//
//  EVENTREPORT_FLAG_ENDOFPIECETRAVEL End of piece travel
//
        if(outputFlags & EVENTREPORT_FLAG_ENDOFPIECETRAVEL)
        {
          if(RUNSVIEW[nJ].endOfPieceTravel > 0)
          {
          }
        }
//
//  EVENTREPORT_FLAG_ENDOFPIECE: End of piece
//
        if(outputFlags & EVENTREPORT_FLAG_ENDOFPIECE)
        {
//
//  Set up the output string: Date, off time in seconds, off time as character
//
          sprintf(outputString, "%ld\t%s\t",
                RUNSVIEW[nJ].runOffTime, Tchar(RUNSVIEW[nJ].runOffTime));
//
//  Run number, Piece number, Block number, Driver info.
//
          if(strcmp(szActualDriver, "") != 0)
          {
            sprintf(tempString, "%s:%s: %s (was %s) end run %ld, pce %d, blk %s.",
                  szOffLocation, szOffAction, szActualDriver, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
          }
          else
          {
            sprintf(tempString, "%s:%s: %s end run %ld, pce %d, blk %s.",
                  szOffLocation, szOffAction, szScheduledDriver, RData[nI].runNumber, nJ + 1, szBlockNumber);
          }
//
//  Write it out
//
          strcat(outputString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          numOutputRecords++;
        }
//
//  EVENTREPORT_FLAG_EXTRABOARDDUTY: Extraboard duty attached to the piece
//
        if(outputFlags & EVENTREPORT_FLAG_EXTRABOARDDUTY)
        {
          for(nK = 0; nK < 2; nK++)
          {
            if(nK == 0)
            {
              xTime[0] = RUNSVIEW[nJ].startOfPieceExtraboardStart;
              xTime[1] = RUNSVIEW[nJ].startOfPieceExtraboardEnd;
              NODESKey0.recordID = RUNSVIEW[nJ].startOfPieceNODESrecordID;
            }
            else
            {
              xTime[0] = RUNSVIEW[nJ].endOfPieceExtraboardStart;
              xTime[1] = RUNSVIEW[nJ].endOfPieceExtraboardEnd;
              NODESKey0.recordID = RUNSVIEW[nJ].endOfPieceNODESrecordID;
            }
            if(xTime[0] == NO_TIME)
            {
              continue;
            }
//
//  Set up the output string: Date, off time in seconds, off time as character
//
            sprintf(outputString, "%ld\t%s\t",
                  xTime[0], Tchar(xTime[0]));
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            abbrName[NODES_ABBRNAME_LENGTH] = '\0';
            strcpy(szAction, "CVR");
            if(strcmp(szActualDriver, "") != 0)
            {
              sprintf(tempString, "%s:%s: %s (was %s) cover duty until %s",
                    abbrName, szAction, szActualDriver, szScheduledDriver, Tchar(xTime[1]));
            }
            else
            {
              sprintf(tempString, "%s:%s: %s cover duty until %s",
                    abbrName, szAction, szScheduledDriver, Tchar(xTime[1]));
            }
//
//  Write it out
//
            strcat(outputString, tempString);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            numOutputRecords++;
          }
        }
      }
    }
//
//  Call from TMSRPT
//
    else
    {
//
//  Cycle through the pieces
//
      for(nJ = 0; nJ < numPieces; nJ++)
      {
//
//  Get the stuff we'll need for most of the output records
//
//  Block number
//
        if(RUNSVIEW[nJ].blockNumber != NO_RECORD)
        {
          sprintf(szBlockNumber, "%4ld", RUNSVIEW[nJ].blockNumber);
        }
        else
        {
          strcpy(szBlockNumber, "STBY\t");
        }
//
//  On location
//
        NODESKey0.recordID = RUNSVIEW[nJ].runOnNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szOnLocation, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        szOnLocation[NODES_ABBRNAME_LENGTH] = '\0';
        strcpy(szOnAction, ((NODES.flags & NODES_FLAG_GARAGE) ? "P/O" : "Rlf"));
//
//  Force a relief start to be after a relief end
//
        if(!(NODES.flags & NODES_FLAG_GARAGE))
        {
          RUNSVIEW[nJ].runOnTime += 1;
        }
//
//  Off location
//
        NODESKey0.recordID = RUNSVIEW[nJ].runOffNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szOffLocation, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        szOffLocation[NODES_ABBRNAME_LENGTH] = '\0';
        strcpy(szOffAction, ((NODES.flags & NODES_FLAG_GARAGE) ? "P/I" : "Rlf"));
//
//  EVENTREPORT_FLAG_REPORTTIME: Report at time
//
        if(outputFlags & EVENTREPORT_FLAG_REPORTTIME)
        {
          if(RUNSVIEW[nJ].reportTime > 0)
          {
//
//  Set up the output string: Date, on time in seconds, on time as character
//
           sprintf(outputString, "%s\t%ld\t%s\t",
                  RParamData.szDate, RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime,
                  Tchar(RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime));
//
//  Run number, Piece number, Block number, Driver info.
//
            if(strcmp(szActualDriver, "") != 0)
            {
              sprintf(tempString, "%s - Rpt - Report for Run %4ld Piece %d on block %s.  Operator: %s (replacing %s).",
                    szOnLocation, RData[nI].runNumber, nJ + 1, szBlockNumber, szActualDriver, szScheduledDriver);
            }
            else
            {
              sprintf(tempString, "%s - Rpt - Report for Run %4ld Piece %d on block %s.  Operator: %s",
                    szOnLocation, RData[nI].runNumber, nJ + 1, szBlockNumber, szScheduledDriver);
            }
//
//  Write it out
//
            strcat(outputString, tempString);
            strcat(outputString, "\r\n");
           _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
        }
//
//  EVENTREPORT_FLAG_STARTOFPIECETRAVEL: Start of Piece Travel
//  
        if(outputFlags & EVENTREPORT_FLAG_STARTOFPIECETRAVEL)
        {
          if(RUNSVIEW[nJ].startOfPieceTravel > 0)
          {
          }
        }
//
//  EVENTREPORT_FLAG_STARTOFPIECE: Start of piece
//
        if(outputFlags & EVENTREPORT_FLAG_STARTOFPIECE)
        {
//
//  Set up the output string: Date, on time in seconds, on time as character
//
          sprintf(outputString, "%s\t%ld\t%s\t",
                RParamData.szDate, RUNSVIEW[nJ].runOnTime, Tchar(RUNSVIEW[nJ].runOnTime));
//
//  Run number, Piece number, Block number, Driver info.
//
          if(strcmp(szActualDriver, "") != 0)
          {
            sprintf(tempString, "%s - %s -  Start  of Run %4ld Piece %d on block %s.  Operator: %s (replacing %s).",
                  szOnLocation, szOnAction, RData[nI].runNumber, nJ + 1, szBlockNumber, szActualDriver, szScheduledDriver);
          }
          else
          {
            sprintf(tempString, "%s - %s -  Start  of Run %4ld Piece %d on block %s.  Operator: %s",
                  szOnLocation, szOnAction, RData[nI].runNumber, nJ + 1, szBlockNumber, szScheduledDriver);
          }
//
//  Write it out
//
          strcat(outputString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  EVENTREPORT_FLAG_ENDOFPIECETRAVEL End of piece travel
//
        if(outputFlags & EVENTREPORT_FLAG_ENDOFPIECETRAVEL)
        {
          if(RUNSVIEW[nJ].endOfPieceTravel > 0)
          {
          }
        }
//
//  EVENTREPORT_FLAG_ENDOFPIECE: End of piece
//
        if(outputFlags & EVENTREPORT_FLAG_ENDOFPIECE)
        {
//
//  Set up the output string: Date, off time in seconds, off time as character
//
          sprintf(outputString, "%s\t%ld\t%s\t",
                RParamData.szDate, RUNSVIEW[nJ].runOffTime, Tchar(RUNSVIEW[nJ].runOffTime));
//
//  Run number, Piece number, Block number, Driver info.
//
          if(strcmp(szActualDriver, "") != 0)
          {
            sprintf(tempString, "%s - %s -    End  of Run %4ld Piece %d on block %s.  Operator: %s (replacing %s).",
                  szOffLocation, szOffAction, RData[nI].runNumber, nJ + 1, szBlockNumber, szActualDriver, szScheduledDriver);
          }
          else
          {
            sprintf(tempString, "%s - %s -    End  of Run %4ld Piece %d on block %s.  Operator: %s.",
                  szOffLocation, szOffAction, RData[nI].runNumber, nJ + 1, szBlockNumber, szScheduledDriver);
          }
//
//  Write it out
//
          strcat(outputString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  EVENTREPORT_FLAG_EXTRABOARDDUTY: Extraboard duty attached to the piece
//
        if(outputFlags & EVENTREPORT_FLAG_EXTRABOARDDUTY)
        {
          for(nK = 0; nK < 2; nK++)
          {
            if(nK == 0)
            {
              xTime[0] = RUNSVIEW[nJ].startOfPieceExtraboardStart;
              xTime[1] = RUNSVIEW[nJ].startOfPieceExtraboardEnd;
              NODESKey0.recordID = RUNSVIEW[nJ].startOfPieceNODESrecordID;
            }
            else
            {
              xTime[0] = RUNSVIEW[nJ].endOfPieceExtraboardStart;
              xTime[1] = RUNSVIEW[nJ].endOfPieceExtraboardEnd;
              NODESKey0.recordID = RUNSVIEW[nJ].endOfPieceNODESrecordID;
            }
            if(xTime[0] == NO_TIME)
            {
              continue;
            }
//
//  Set up the output string: Date, off time in seconds, off time as character
//
            sprintf(outputString, "%s\t%ld\t%s\t",
                  RParamData.szDate, xTime[0], Tchar(xTime[0]));
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            abbrName[NODES_ABBRNAME_LENGTH] = '\0';
            strcpy(szAction, "Cvr");
            if(strcmp(szActualDriver, "") != 0)
            {
              sprintf(tempString, "%s - %s - Cover duty until %s.                Operator: %s (replacing %s).",
                    abbrName, szAction, Tchar(xTime[1]), szActualDriver, szScheduledDriver);
            }
            else
            {
              sprintf(tempString, "%s - %s - Cover duty until %s.                Operator: %s.",
                    abbrName, szAction, Tchar(xTime[1]), szScheduledDriver);
            }
//
//  Write it out
//
            strcat(outputString, tempString);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
        }
      }
    }
  }
//
//  All done
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);
    if(bFromDailyOps)
    {
      pPassedData->numDataFiles = numOutputRecords++;
    }

  return(TRUE);
}
