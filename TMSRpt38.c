//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>
//
//  Dispatch Report (4)
//
//  From QETXT.INI:
//
//
//  [TMSRPT38]
//  FILE=TMSRPT38.txt
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
//  FIELD11=BackAt,VARCHAR,8,0,8,0,
//  FIELD12=Pay,NUMERIC,7,1,7,0,
//  FIELD13=BusNumber,VARCHAR,16,0,16,0,
//  FIELD14=ActualOperator,51,0,51,0,
//

//
//  Dispatch sheet report
//

BOOL FAR TMSRPT38(TMSRPTPassedDataDef *pPassedData)
{
  DISPATCHSHEETPassedDataDef RParamData;
  DISPATCHSHEETRosterDataDef RData[MAXRUNSINRUNLIST];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef  COST;
  HFILE hfOutputFile;
  BOOL  bFound;
  double decimalTime;
  char  lastName[2][DRIVERS_LASTNAME_LENGTH + 1];
  char  firstName[2][DRIVERS_FIRSTNAME_LENGTH + 1];
  char  badgeNumber[2][DRIVERS_BADGENUMBER_LENGTH + 1];
  char  abbrName[NODES_ABBRNAME_LENGTH + 1];
  char  outputString[TEMPSTRING_LENGTH];
  char  *pszReportName;
  long  onTime;
  long  offTime;
  long  seq;
  long  pieceNumber;
  long  runNumber;
  long  cutAsRuntype;
  long  startTime;
  long  onNode;
  long  offNode;
  int   hour, min;
  int   numRuns;
  int   nI;
  int   nJ;
  int   numPieces;
  int   rcode2;
//
//  See what he wants to unload
//
  RParamData.flags = DISPATCHSHEET_FLAG_DISABLEOPTIONS;
  RParamData.nReportNumber = 37;
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
  pPassedData->nReportNumber = 37;
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
  strcat(tempString, "\\TMSRPT38.txt");
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
  StatusBarStart(hWndMain, pszReportName);
  StatusBarText("Cycling through the runs..");
//
//  Cycle through the runs
//
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
    }
    else
    {
      RUNSKey0.recordID = RData[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    }
//
//  Driver
//
    if(RData[nI].DRIVERSrecordID != NO_RECORD)
    {
      DRIVERSKey0.recordID = RData[nI].DRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(badgeNumber[0], DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(badgeNumber[0], DRIVERS_BADGENUMBER_LENGTH);
      strncpy(lastName[0], DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(lastName[0], DRIVERS_LASTNAME_LENGTH);
      strncpy(firstName[0], DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(firstName[0], DRIVERS_FIRSTNAME_LENGTH);
    }
//
//  Actual Driver
//
    if(RData[nI].replacementDRIVERSrecordID != NO_RECORD)
    {
      DRIVERSKey0.recordID = RData[nI].replacementDRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(badgeNumber[1], DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(badgeNumber[1], DRIVERS_BADGENUMBER_LENGTH);
      strncpy(lastName[1], DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(lastName[1], DRIVERS_LASTNAME_LENGTH);
      strncpy(firstName[1], DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(firstName[1], DRIVERS_FIRSTNAME_LENGTH);
    }
//
//  Run details
//
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
//  Cycle through the pieces
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
//  Time of report
//
      strcat(outputString, Tchar(RUNSVIEW[nJ].runOnTime - COST.PIECECOST[nJ].reportTime - COST.TRAVEL[nJ].startTravelTime));
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
//  Back at
//
      strcat(outputString, Tchar(RUNSVIEW[nJ].runOffTime + COST.TRAVEL[nJ].endTravelTime));
      strcat(outputString, "\t");
//
//  Pay Time in hours and tenths
//
      hour = RUNSVIEW[numPieces - 1].payTime / 3600;
      min = (RUNSVIEW[numPieces - 1].payTime % 3600) / 60;
      decimalTime = (double)(hour + ((double)min / 60));
      sprintf(tempString, "%4.1f\t", decimalTime);
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
      strcat(outputString, "\t");
//
//  Actual driver
//
      if(RData[nI].replacementDRIVERSrecordID != NO_RECORD)
      {
        if(strcmp(badgeNumber[1], "") == 0)
        {
          sprintf(tempString, "\"%s, %s\"", lastName[1], firstName[1]);
        }
        else
        {
          sprintf(tempString, "\"%s - %s, %s\"", badgeNumber[1], lastName[1], firstName[1]);
        }
        strcat(outputString, tempString);
      }
      strcat(outputString, "\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
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
