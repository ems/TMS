//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
//
//  Dispatch Sheet (1)
//
//  From QETXT.INI:
//
//
//  [TMSRPT05]
//  FILE=Tmsrpt05.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Date,VARCHAR,64,0,64,0,
//  FIELD3=Operator,VARCHAR,51,0,51,0,
//  FIELD4=RunNumber,NUMERIC,5,0,5,0,
//  FIELD5=BlockNumber,VARCHAR,5,0,5,0,
//  FIELD6=Origin,VARCHAR,8,0,8,0,
//  FIELD7=TimeOfReport,VARCHAR,8,0,8,0,
//  FIELD8=OutTime,VARCHAR,8,0,8,0,
//  FIELD9=OffTime,VARCHAR,8,0,8,0,
//  FIELD10=Destination,VARCHAR,8,0,8,0,
//  FIELD11=PlatformTime,VARCHAR,8,0,8,0,
//  FIELD12=ReportTime,VARCHAR,8,0,8,0,
//  FIELD13=PadTime,VARCHAR,8,0,8,0,
//  FIELD14=TrvPlusDwellTime,VARCHAR,8,0,8,0,
//  FIELD15=InterveningTime,VARCHAR,8,0,8,0,
//  FIELD16=TotalTime,VARCHAR,8,0,8,0,
//  FIELD17=SpreadPremiumTime,VARCHAR,8,0,8,0,
//

int sort_TMSRPT05(const void *a, const void *b)
{
  DISPATCHSHEETRosterDataDef *pa, *pb;
  
  pa = (DISPATCHSHEETRosterDataDef *)a;
  pb = (DISPATCHSHEETRosterDataDef *)b;
  
  return(pa->runNumber < pb->runNumber ? -1 : pa->runNumber > pb->runNumber ? 1 : 0);
}

//
//  Dispatch sheet report
//

BOOL FAR TMSRPT05(TMSRPTPassedDataDef *pPassedData)
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
  long  time;
  long  pieceNumber;
  long  runNumber;
  long  cutAsRuntype;
  long  startTime;
  long  onNode;
  long  offNode;
  int   numRuns;
  int   nI;
  int   nJ;
  int   numPieces;
  int   rcode2;
//  int   startLimit;
//
//  See what he wants to unload
//
  RParamData.flags = 0;
  RParamData.nReportNumber = 4;
  if(DispatchSheet(&RParamData) != IDOK)
  {
    return(FALSE);
  }
  if(ROSTERPARMS.serviceDays[RParamData.dayOfWeek] == NO_RECORD)
  {
    TMSError(hWndMain, MB_ICONSTOP, ERROR_318, (HANDLE)NULL);
    return(FALSE);
  }
//
//  Back ok - set up the status bar
//
  pPassedData->numDataFiles = 1;
  pPassedData->nReportNumber = 4;
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
  StatusBarText("Opening output file...");
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt05.txt");
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
          RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
          RData[numRuns].rosterNumber = NO_RECORD;
          RData[numRuns].BUSESrecordID = NO_RECORD;
          RData[numRuns].blockNumber = NO_RECORD;
          RData[numRuns].TRIPSrecordID = NO_RECORD;
          RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
          RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
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
          RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
          RData[numRuns].rosterNumber = NO_RECORD;
          RData[numRuns].BUSESrecordID = NO_RECORD;
          RData[numRuns].blockNumber = NO_RECORD;
          RData[numRuns].TRIPSrecordID = NO_RECORD;
          RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
          RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
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
        RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
        RData[numRuns].rosterNumber = NO_RECORD;
        RData[numRuns].BUSESrecordID = NO_RECORD;
        RData[numRuns].blockNumber = NO_RECORD;
        RData[numRuns].TRIPSrecordID = NO_RECORD;
        RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
        RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
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
        RData[numRuns].replacementDRIVERSrecordID = NO_RECORD;
        RData[numRuns].rosterNumber = NO_RECORD;
        RData[numRuns].BUSESrecordID = NO_RECORD;
        RData[numRuns].blockNumber = NO_RECORD;
        RData[numRuns].TRIPSrecordID = NO_RECORD;
        RData[numRuns].RGRPROUTESrecordID = NO_RECORD;
        RData[numRuns].SGRPSERVICESrecordID = NO_RECORD;
        numRuns++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
  }
//
//  Check for changes made via Daily Ops
//
  AdjustDispatchSheetRosterData(&RData[0], RParamData.todaysDate, RParamData.date, numRuns, TRUE, FALSE);
//
//  Sort by run number
//
  qsort((void *)RData, numRuns, sizeof(DISPATCHSHEETRosterDataDef), sort_TMSRPT05);
//
//  Go through each day to generate the run details, and print them
//  out along with any associated driver data.
//
  StatusBarText("Generating run details...");
  seq = 0;
//
//  Initialize the travel matrix
//
  StatusBarEnd();
  if(bUseDynamicTravels)
  {
    SERVICESKey0.recordID = ROSTERPARMS.serviceDays[RParamData.dayOfWeek];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
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
//
//  Cycle through the runs
//
  StatusBarStart(hWndMain, pszReportName);
  StatusBarText("Cycling through the runs...");
//  startLimit = 0;
//  while(startLimit < 1000)
//  {
    for(nI = 0; nI < numRuns; nI++)
    {
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
//        if(CREWONLY.runNumber < startLimit || CREWONLY.runNumber > startLimit + 100)
//          continue;
      }
      else
      {
        RUNSKey0.recordID = RData[nI].RUNSrecordID;
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
//        if(RUNS.runNumber < startLimit || RUNS.runNumber > startLimit + 100)
//          continue;
      }
//
//  Set up the output string
//
//  Sequence # and Date
//
      sprintf(outputString, "%d\t%s\t", seq++, RParamData.szDate);
//
//  Driver
//
      if(RData[nI].replacementDRIVERSrecordID != NO_RECORD)
      {
        DRIVERSKey0.recordID = RData[nI].replacementDRIVERSrecordID;
      }
      else if(RData[nI].DRIVERSrecordID != NO_RECORD)
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
        strcat(outputString, badgeNumber);
        strcat(outputString, " - ");
        strcat(outputString, lastName);
        strcat(outputString, ", ");
        strcat(outputString, firstName);
      }
      strcat(outputString, "\t");
//
//  Run number and details
//
      sprintf(tempString, "%ld\t", RData[nI].runNumber);
      strcat(outputString, tempString);
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
//  Change all NO_TIMEs to 0
//
      for(nJ = 0; nJ < numPieces; nJ++)
      {
        if(RUNSVIEW[nJ].platformTime == NO_TIME)
          RUNSVIEW[nJ].platformTime = 0;
        if(RUNSVIEW[nJ].makeUpTime == NO_TIME)
          RUNSVIEW[nJ].makeUpTime = 0;
        if(RUNSVIEW[nJ].reportTime == NO_TIME)
          RUNSVIEW[nJ].reportTime = 0;
        if(RUNSVIEW[nJ].turninTime == NO_TIME)
          RUNSVIEW[nJ].turninTime = 0;
        if(RUNSVIEW[nJ].travelTime == NO_TIME)
          RUNSVIEW[nJ].travelTime = 0;
        if(RUNSVIEW[nJ].paidBreaks == NO_TIME)
          RUNSVIEW[nJ].paidBreaks = 0;
      }
//
//  Run details
//
      for(nJ = 0; nJ < numPieces; nJ++)
      {
        if(nJ != 0)
        {
          sprintf(outputString,
                "%d\t%s\t\t%ld\t", seq++, RParamData.szDate, RData[nI].runNumber);
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
//  Time of report
//
        time = RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime;
        if(RParamData.flags & DISPATCHSHEET_FLAG_INCLUDESTARTOFPIECETRAVEL)
          time -= RUNSVIEW[nJ].startOfPieceTravel;
        strcat(outputString, Tchar(time));
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
//  Platform time
//
        strcat(outputString, chhmm(RUNSVIEW[nJ].platformTime));
        strcat(outputString, "\t");
//
//  Report time
//
        if(RUNSVIEW[nJ].reportTime > 0 || RUNSVIEW[nJ].turninTime > 0)
          strcat(outputString, chhmm(RUNSVIEW[nJ].reportTime + RUNSVIEW[nJ].turninTime));
        strcat(outputString, "\t");
//
//  Pad time
//
        strcat(outputString, "\t");
//
//  Travel and Dwell Time
//
        if(RUNSVIEW[nJ].travelTime > 0)
          strcat(outputString, chhmm(RUNSVIEW[nJ].travelTime));
        strcat(outputString, "\t");
//
//  Intervening (Paid Breaks) Time
//
        if(RUNSVIEW[nJ].paidBreaks > 0)
          strcat(outputString, chhmm(RUNSVIEW[nJ].paidBreaks));
        strcat(outputString, "\t");
//
//  And one more tab to fill out the record
//
        strcat(outputString, "\t\r\n");
//
//  Write it out
//
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      }
//
//  Write out the summary record
//
      sprintf(outputString,
            "%d\t%s\t\t%ld\t\t\t\t\t\tTotals\t", seq++, RParamData.szDate, RData[nI].runNumber);
//
//  Sum the platform, report, makeup, travel, and intervening times
//
//  Generate the sums
//
      for(nJ = 1; nJ < numPieces; nJ++)
      {
        RUNSVIEW[0].platformTime += RUNSVIEW[nJ].platformTime;
        RUNSVIEW[0].makeUpTime += RUNSVIEW[nJ].makeUpTime;
        RUNSVIEW[0].reportTime += RUNSVIEW[nJ].reportTime;
        RUNSVIEW[0].turninTime += RUNSVIEW[nJ].turninTime;
        RUNSVIEW[0].travelTime += RUNSVIEW[nJ].travelTime;
        RUNSVIEW[0].paidBreaks += RUNSVIEW[nJ].paidBreaks;
      }
//
//  And write them out
//
//  Platform time
//
      strcat(outputString, chhmm(RUNSVIEW[0].platformTime));
      strcat(outputString, "\t");
//
//  Report time
//
      if(RUNSVIEW[0].reportTime > 0 || RUNSVIEW[0].turninTime > 0)
        strcat(outputString, chhmm(RUNSVIEW[0].reportTime + RUNSVIEW[0].turninTime));
      strcat(outputString, "\t");
//
//  Pad time
//
      if(RUNSVIEW[0].makeUpTime > 0)
        strcat(outputString, chhmm(RUNSVIEW[0].makeUpTime));
      strcat(outputString, "\t");
//
//  Travel and Dwell Time
//
      if(RUNSVIEW[0].travelTime > 0)
        strcat(outputString, chhmm(RUNSVIEW[0].travelTime));
      strcat(outputString, "\t");
//
//  Intervening (Paid Breaks) Time
//
      if(RUNSVIEW[0].paidBreaks > 0)
        strcat(outputString, chhmm(RUNSVIEW[0].paidBreaks));
      strcat(outputString, "\t");
//
//  Pay time
//
      strcat(outputString, chhmm(RUNSVIEW[numPieces - 1].payTime));
      strcat(outputString, "\t");
//
//  Spread overtime
//
      if(RUNSVIEW[numPieces - 1].spreadOT > 0)
        strcat(outputString, chhmm(RUNSVIEW[numPieces - 1].spreadOT));
      strcat(outputString, "\t\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
//    startLimit += 100;
//  }

//
//  All done
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);

  return(TRUE);
}
