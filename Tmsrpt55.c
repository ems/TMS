//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT55() - Operator's Bid List
//
//  From QETXT.INI:
//
//  [TMSRPT55]
//  FILE=TMSRPT55.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=RosterNumber,NUMERIC,8,0,8,0,
//  FIELD3=Text,VARCHAR,255,0,255,0,
//

#include "TMSHeader.h"

#define TMSRPT55_MAXROSTERS 500

typedef struct TMSRPT55SelectionsStruct
{
  long startDay;
  long DIVISIONSrecordID;
} TMSRPT55SelectionsDef;

#define TMSRPT55_FLAG_CREWONLY       0x0001

typedef struct TMSRPT55RosterStruct
{
  long ROSTERrecordID;
  long rosterNumber;
} TMSRPT55RosterDef;

BOOL  CALLBACK SELECTDIVISIONSMsgProc(HWND, UINT, WPARAM, LPARAM);

BOOL FAR TMSRPT55(TMSRPTPassedDataDef *pPassedData)
{
  TMSRPT55SelectionsDef TMSRPT55Selections;
  TMSRPT55RosterDef     TMSRPT55Rosters[TMSRPT55_MAXROSTERS];
  PROPOSEDRUNDef        PROPOSEDRUN;
  COSTDef               COST;
  HANDLE       hOutputFile;
  DWORD        dwBytesWritten;
  long  runNumbers[7];
  long  weeklyPay;
  long  weeklyPayTotal;
  long  pieceNumber;
  long  runNumber;
  long  onTime;
  long  offTime;
  long  cutAsRuntype;
  long  startTime;
  long  onNode;
  long  offNode;
  long  time;
  long  RUNSrecordIDs[7];
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bLine1;
  BOOL  bStop;
  BOOL  bPrintWeeklyPay;
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  szDays[7][4];
  char  szODRN[64];
  char  outputString[256];
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   sequence = 0;
  int   numRosters;
  int   day[7];
  int   lastWeek;

  pPassedData->nReportNumber = 55;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SELECTDIVISIONS),
        hWndMain, (DLGPROC)SELECTDIVISIONSMsgProc, (LPARAM)&TMSRPT55Selections);
  if(!bRC)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT55.txt");
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
//  Set up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Get the division name
//
  DIVISIONSKey0.recordID = TMSRPT55Selections.DIVISIONSrecordID;
  strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(divisionName, DIVISIONS_NAME_LENGTH);
  StatusBarText(divisionName);
//
//  Save the roster recordIDs and numbers
//
  numRosters = 0;
  lastWeek = 0;
  ROSTERKey1.DIVISIONSrecordID = TMSRPT55Selections.DIVISIONSrecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == TMSRPT55Selections.DIVISIONSrecordID)
  {
    TMSRPT55Rosters[numRosters].ROSTERrecordID = ROSTER.recordID;
    TMSRPT55Rosters[numRosters].rosterNumber = ROSTER.rosterNumber;
    for(nI = lastWeek; nI < ROSTER_MAX_WEEKS; nI++)
    {
      for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
      {
        if(ROSTER.WEEK[nI].RUNSrecordIDs[nJ] > 0)
        {
          if(nI > lastWeek)
          {
            lastWeek = nI;
          }
        }
      }
    }
    numRosters++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
  lastWeek++;
//
//  Establish the traversal order based on start day
//
  nJ = TMSRPT55Selections.startDay;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    day[nI] = nJ;
    LoadString(hInst, TEXT_009 + nJ, tempString, TEMPSTRING_LENGTH);
    strncpy(szDays[nI], tempString, 3);
    szDays[nI][3] = '\0';
    nJ++;
    if(nJ > 6)
    {
      nJ = 0;
    }
  }
//
//  Dump out the structure
//
  sequence = 0;
  StatusBarText("Formatting output...");
  for(nI = 0; nI < numRosters; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    StatusBar((long)(nI + 1), (long)numRosters);
//
//  Title
//
//  Line 1
//
    strcpy(szarString, "+--------------------------------------------------------------------------------------------------------------------------------------------+");
    sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, szarString);
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Line 2
//
    strcpy(szarString, "|Wk| Ser | Run |Pce| Blk | Rte |Rpt  |Rpt |Dep  |Start|Strt|End  |End |Off  |Off |Make |Daily|Wkly |         Off Days / Run Numbers          |");
    sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, szarString);
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Line 3
//
    strcpy(szarString, "|  |     |  #  | # |  #  |  #  |Time |Loc |Time |Time |Loc |Time |Loc |Time |Loc | Up  | Pay |Pay  | ");
    for(nJ = 0; nJ < 7; nJ++)
    {
      strcat(szarString, szDays[nJ]);
      strcat(szarString, (nJ < 6 ? "   " : " |"));
    }
    sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, szarString);
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Line 4
//
    strcpy(szarString, "+--+-----+-----+---+-----+-----+-----+----+-----+-----+----+-----+----+-----+----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
    sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, szarString);
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Dump the roster for each week
//
    weeklyPayTotal = 0;
    for(nJ = 0; nJ < lastWeek; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Get the roster record
//
      ROSTERKey0.recordID = TMSRPT55Rosters[nI].ROSTERrecordID;
      btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
//
//  Get the run numbers for each day
//
      for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
      {
        if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]] == NO_RECORD)
        {
          runNumbers[nK] = NO_RECORD;
          continue;
        }
        if(ROSTER.WEEK[nJ].flags & (1 << day[nK]))  // Crew-only run
        {
          CREWONLYKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          runNumbers[nK] = CREWONLY.runNumber;
        }
        else
        {
          RUNSKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          runNumbers[nK] = RUNS.runNumber;
        }
      }
//
//  Build the "Off days / Run numbers" string
//
      strcpy(szODRN, "");
      for(nK = 0; nK < 7; nK++)
      {
        if(runNumbers[nK] == NO_RECORD)
        {
          strcat(szODRN, "  Off");
        }
        else
        {
          sprintf(tempString, "%5ld", runNumbers[nK]);
          strcat(szODRN, tempString);
        }
        if(nK != 6)
        {
          strcat(szODRN, " ");
        }
      }
      strcat(szODRN, "|");
//
//  Save the RUNSrecordIDs
//
      for(nK = 0; nK < 7; nK++)
      {
        RUNSrecordIDs[nK] = ROSTER.WEEK[nJ].RUNSrecordIDs[nK];
      }
//
//  Whittle down the list of runs to print
//
      for(nK = 0; nK < 6; nK++)
      {
        if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]] == NO_RECORD)
        {
          continue;
        }
        if(ROSTER.WEEK[nJ].flags & (1 << day[nK]))  // Crew-only run
        {
          for(nL = nK + 1; nL < ROSTER_MAX_DAYS; nL++)
          {
            if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] == NO_RECORD)
            {
              continue;
            }
            if((ROSTER.WEEK[nJ].flags & (1 << day[nL])) && ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] == ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]])
            {
              ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] = NO_RECORD;
            }
          }
        }
        else
        {
          for(nL = nK + 1; nL < ROSTER_MAX_DAYS; nL++)
          {
            if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] == NO_RECORD)
            {
              continue;
            }
            if(!(ROSTER.WEEK[nJ].flags & (1 << day[nL])) && ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] == ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]])
            {
              ROSTER.WEEK[nJ].RUNSrecordIDs[day[nL]] = NO_RECORD;
            }
          }
        }
      }
//
//  Figure out the weekly pay
//
      weeklyPay = 0;
      for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
      {
        if(RUNSrecordIDs[day[nK]] == NO_RECORD)
        {
          continue;
        }
        if(ROSTER.WEEK[nJ].flags & (1 << day[nK]))  // Crew-only run
        {
          CREWONLYKey0.recordID = RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          if(rcode2 == 0)
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
            for(bStop = FALSE, nL = 0; nL < pieceNumber; nL++)
            {
              if(PROPOSEDRUN.piece[nL].fromTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].fromNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].toNODESrecordID == NO_RECORD)
              {
                bStop = TRUE;
                break;
              }
            }
            if(!bStop)
            {
              RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
              weeklyPay += COST.TOTAL.payTime;
            }  // Valid crew only run (!bStop)
          }  // Found the crew only run
        }  // Crew only run
//
//  From the Runs Table
//
        else
        {
          RUNSKey0.recordID = RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 == 0)
          {
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            pieceNumber = 0;
            runNumber = RUNS.runNumber;
            while(rcode2 == 0 && RUNS.runNumber == runNumber)
            {
              RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                    RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
              if(RUNS.pieceNumber == 1)
              {
                cutAsRuntype = RUNS.cutAsRuntype;
                startTime = onTime;
                onNode = RUNS.start.NODESrecordID;
              }
              PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
              PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
              PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
              PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
              PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
              PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
              PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
              PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
              PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
              PROPOSEDRUN.piece[pieceNumber++].after.endTime = RUNS.prior.endTime;
              offNode = RUNS.end.NODESrecordID;
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            }
            PROPOSEDRUN.numPieces = pieceNumber;
            for(bStop = FALSE, nL = 0; nL < pieceNumber; nL++)
            {
              if(PROPOSEDRUN.piece[nI].fromTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].fromNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].fromTRIPSrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].toNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTRIPSrecordID == NO_RECORD)
              {
                bStop = TRUE;
                break;
              }
            }
            if(!bStop)
            {
              RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
              weeklyPay += COST.TOTAL.payTime;
            }  // Valid run (!bStop)
          }  // Found the run
        }  // Regular run
      }  // nK < 7
//
//  Get - and cost out - the unique runs for each day
//
      bLine1 = TRUE;
      for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
      {
        if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]] == NO_RECORD)
        {
          continue;
        }
        if(ROSTER.WEEK[nJ].flags & (1 << day[nK]))  // Crew-only run
        {
          CREWONLYKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          if(rcode2 == 0)
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
            for(bStop = FALSE, nL = 0; nL < pieceNumber; nL++)
            {
              if(PROPOSEDRUN.piece[nL].fromTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].fromNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].toNODESrecordID == NO_RECORD)
              {
                bStop = TRUE;
                break;
              }
            }
//
//  Output the crew-only details
//
            if(!bStop)
            {
              RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  Go through the pieces
//
              CREWONLYKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
              rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
              for(nL = 0; nL < PROPOSEDRUN.numPieces; nL++)
              {
                strcpy(outputString, "|");
//
//  Week number
//
                if(bLine1)
                {
                  sprintf(tempString, "%2d", nJ + 1);
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "  ");
                }                
                strcat(outputString, "|");
//
//  Service
//
                if(nL == 0)
                {
                  SERVICESKey0.recordID = CREWONLY.SERVICESrecordID;
                  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                  strncpy(tempString, SERVICES.name, 5);
                  pad(tempString, 5);
                  tempString[5] = '\0';
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "     ");
                }
                strcat(outputString, "|");
//
//  Run number
//
                if(nL == 0)
                {
                  sprintf(tempString, "%5ld", CREWONLY.runNumber);
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "     ");
                }
                strcat(outputString, "|");
//
//  Piece number
//
                sprintf(tempString, "%3d", nL + 1);
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Block number
//
                strcat(outputString, "  -  |");
//
//  Route number
//
                strcat(outputString, "  -  |");
//
//  Report time
//
                time = PROPOSEDRUN.piece[nL].fromTime - COST.PIECECOST[nL].reportTime - COST.TRAVEL[nL].startTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Report location
//
                NODESKey0.recordID = COST.TRAVEL[nL].startNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Depart time
//
                time = PROPOSEDRUN.piece[nL].fromTime - COST.TRAVEL[nL].startTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Start time
//
                sprintf(tempString, "%5s", Tchar(PROPOSEDRUN.piece[nL].fromTime));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Start location
//
                NODESKey0.recordID = COST.TRAVEL[nL].startAPointNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  End time
//
                sprintf(tempString, "%5s", Tchar(PROPOSEDRUN.piece[nL].toTime));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  End location
//
                NODESKey0.recordID = COST.TRAVEL[nL].endBPointNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Off time
//
                time = PROPOSEDRUN.piece[nL].toTime + COST.PIECECOST[nL].turninTime + COST.TRAVEL[nL].endTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Off location
//
                NODESKey0.recordID = COST.TRAVEL[nL].endNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Balance of run data (output on last piece only)
//
                if(nL == PROPOSEDRUN.numPieces - 1)
                {
//
//  Makeup
//
                  sprintf(tempString, "%5s", chhmm(COST.PIECECOST[nL].makeUpTime));
                  strcat(outputString, tempString);
                  strcat(outputString, "|");
//
//  Pay time
//
                  sprintf(tempString, "%5s", chhmm(COST.TOTAL.payTime));
                  strcat(outputString, tempString);
                  strcat(outputString, "|");
                }
                else
                {
                  strcat(outputString, "     |     |     |");
                }
//
//  Weekly pay - if this is the last piece on the last run being diplayed - print it
//
                if(nL == PROPOSEDRUN.numPieces - 1)
                {
                  for(bPrintWeeklyPay = TRUE, nM = nK + 1; nM < ROSTER_MAX_DAYS; nM++)
                  {
                    if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nM]] != NO_RECORD)
                    {
                      bPrintWeeklyPay = FALSE;
                      break;
                    }
                  }
                  if(bPrintWeeklyPay)
                  {
                    sprintf(tempString, "%5s|", chhmm(weeklyPay));
                    strcat(outputString, tempString);
                  }
                  else
                  {
                    strcat(outputString, "     |");
                  }
                }
//
//  Are we on the first line?  Put out the roster days.
//
                if(bLine1)
                {
                  strcat(outputString, szODRN);
                  bLine1 = FALSE;
                }
                else
                {
                  strcat(outputString, "                                         |");
                }
//
//  And the line terminators
//
                sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, outputString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
              }  // nL < numPieces
            }  // Valid crew only run (!bStop)
          }  // Found the crew only run
        }  // Crew only run
//
//  From the Runs Table
//
        else
        {
          RUNSKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 == 0)
          {
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            pieceNumber = 0;
            runNumber = RUNS.runNumber;
            while(rcode2 == 0 && RUNS.runNumber == runNumber)
            {
              RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                    RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
              if(RUNS.pieceNumber == 1)
              {
                cutAsRuntype = RUNS.cutAsRuntype;
                startTime = onTime;
                onNode = RUNS.start.NODESrecordID;
              }
              PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
              PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
              PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
              PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
              PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
              PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
              PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
              PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
              PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
              PROPOSEDRUN.piece[pieceNumber++].after.endTime = RUNS.prior.endTime;
              offNode = RUNS.end.NODESrecordID;
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            }
            PROPOSEDRUN.numPieces = pieceNumber;
            for(bStop = FALSE, nL = 0; nL < pieceNumber; nL++)
            {
              if(PROPOSEDRUN.piece[nI].fromTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].fromNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].fromTRIPSrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTime == NO_TIME ||
                    PROPOSEDRUN.piece[nL].toNODESrecordID == NO_RECORD ||
                    PROPOSEDRUN.piece[nL].toTRIPSrecordID == NO_RECORD)
              {
                bStop = TRUE;
                break;
              }
            }
            if(!bStop)
            {
              RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  Go through the pieces
//
              RUNSKey0.recordID = ROSTER.WEEK[nJ].RUNSrecordIDs[day[nK]];
              rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
              btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
              rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              for(nL = 0; nL < PROPOSEDRUN.numPieces; nL++)
              {
                strcpy(outputString, "|");
//
//  Week number
//
                if(bLine1)
                {
                  sprintf(tempString, "%2d", nJ + 1);
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "  ");
                }                
                strcat(outputString, "|");
//
//  Service
//
                if(nL == 0)
                {
                  SERVICESKey0.recordID = RUNS.SERVICESrecordID;
                  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                  strncpy(tempString, SERVICES.name, 5);
                  pad(tempString, 5);
                  tempString[5] = '\0';
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "     ");
                }
                strcat(outputString, "|");
//
//  Run number
//
                if(nL == 0)
                {
                  sprintf(tempString, "%5ld", RUNS.runNumber);
                  strcat(outputString, tempString);
                }
                else
                {
                  strcat(outputString, "     ");
                }
                strcat(outputString, "|");
//
//  Piece number
//
                sprintf(tempString, "%3d", nL + 1);
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Block number
//
                TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
                btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                sprintf(tempString, "%5ld", TRIPS.standard.blockNumber);
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Route number
//
                ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
                btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                strncpy(tempString, ROUTES.number, 5);
                tempString[5] = '\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Report time
//
                time = PROPOSEDRUN.piece[nL].fromTime - COST.PIECECOST[nL].reportTime - COST.TRAVEL[nL].startTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Report location
//
                if(COST.TRAVEL[nL].startNODESrecordID == NO_RECORD)
                {
                  NODESKey0.recordID = PROPOSEDRUN.piece[nL].fromNODESrecordID;
                }
                else
                {
                  NODESKey0.recordID = COST.TRAVEL[nL].startNODESrecordID;
                }
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Depart time
//
                time = PROPOSEDRUN.piece[nL].fromTime - COST.TRAVEL[nL].startTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Start time
//
                sprintf(tempString, "%5s", Tchar(PROPOSEDRUN.piece[nL].fromTime));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Start location
//
                if(COST.TRAVEL[nL].startAPointNODESrecordID == NO_RECORD)
                {
                  NODESKey0.recordID = PROPOSEDRUN.piece[nL].fromNODESrecordID;
                }
                else
                {
                  NODESKey0.recordID = COST.TRAVEL[nL].startAPointNODESrecordID;
                }
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  End time
//
                sprintf(tempString, "%5s", Tchar(PROPOSEDRUN.piece[nL].toTime));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  End location
//
                if(COST.TRAVEL[nL].endBPointNODESrecordID == NO_RECORD)
                {
                  NODESKey0.recordID = PROPOSEDRUN.piece[nL].toNODESrecordID;
                }
                else
                {
                  NODESKey0.recordID = COST.TRAVEL[nL].endBPointNODESrecordID;
                }
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Off time
//
                time = PROPOSEDRUN.piece[nL].toTime + COST.PIECECOST[nL].turninTime + COST.TRAVEL[nL].endTravelTime;
                sprintf(tempString, "%5s", Tchar(time));
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Off location
//
                NODESKey0.recordID = COST.TRAVEL[nL].endNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                tempString[NODES_ABBRNAME_LENGTH] ='\0';
                strcat(outputString, tempString);
                strcat(outputString, "|");
//
//  Balance of run data (output on last piece only)
//
                if(nL == PROPOSEDRUN.numPieces - 1)
                {
//
//  Makeup
//
                  sprintf(tempString, "%5s", chhmm(COST.PIECECOST[nL].makeUpTime));
                  strcat(outputString, tempString);
                  strcat(outputString, "|");
//
//  Pay time
//
                  sprintf(tempString, "%5s", chhmm(COST.TOTAL.payTime));
                  strcat(outputString, tempString);
                  strcat(outputString, "|");
                }
                else
                {
                  strcat(outputString, "     |     |     |");
                }
//
//  Weekly pay - if this is the last piece on the last run being diplayed - print it
//
                if(nL == PROPOSEDRUN.numPieces - 1)
                {
                  for(bPrintWeeklyPay = TRUE, nM = nK + 1; nM < ROSTER_MAX_DAYS; nM++)
                  {
                    if(ROSTER.WEEK[nJ].RUNSrecordIDs[day[nM]] != NO_RECORD)
                    {
                      bPrintWeeklyPay = FALSE;
                      break;
                    }
                  }
                  if(bPrintWeeklyPay)
                  {
                    sprintf(tempString, "%5s|", chhmm(weeklyPay));
                    strcat(outputString, tempString);
                  }
                  else
                  {
                    strcat(outputString, "     |");
                  }
                }
//
//  Are we on the first line?  Put out the roster days.
//
                if(bLine1)
                {
                  strcat(outputString, szODRN);
                  bLine1 = FALSE;
                }
                else
                {
                  strcat(outputString, "                                         |");
                }
//
//  And the line terminators
//
                sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, outputString);
                WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Get the next record in the Runs table
//
                rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }  // nL < numPieces
            }  // Valid run (!bStop)
          }  // Found the run
        }  // Regular run
      }  // nK < 7
//
//  Last output line
//
      strcpy(szarString, "+--------------------------------------------------------------------------------------------------------------------------------------------+");
      sprintf(tempString, "%d\t%ld\t%s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber, szarString);
      WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Sum the weekly pay
//
      weeklyPayTotal += weeklyPay;
    }  // nJ < numDivisions
//
//  Blank line
//
    sprintf(tempString, "%d\t%ld\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber);
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
//
//  Average weekly pay
//
    sprintf(tempString, "%d\t%ld\tAverarge Weekly Pay = %s\r\n", sequence++, TMSRPT55Rosters[nI].rosterNumber,
          chhmm(weeklyPayTotal / lastWeek));
    WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
  }  // nI < numRosters
  bKeepGoing = TRUE;
//
//  All done
//
  deallocate:
    CloseHandle(hOutputFile);
    StatusBarEnd();
    return(bKeepGoing);
}

BOOL CALLBACK SELECTDIVISIONSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static TMSRPT55SelectionsDef *pTMSRPT55Selections;
  static HANDLE hCtlLIST;
  static HANDLE hCtlSTARTDAY;
  int    nI;
  int    rcode2;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Validate the pointer
//
      pTMSRPT55Selections = (TMSRPT55SelectionsDef *)lParam;
      if(pTMSRPT55Selections == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlLIST = GetDlgItem(hWndDlg, SELECTDIVISIONS_LIST);
      hCtlSTARTDAY = GetDlgItem(hWndDlg, SELECTDIVISIONS_STARTDAY);
//
//  Populate the dialog
//
//  LIST - All the divisions and default to the first
//
      rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
        trim(tempString, DIVISIONS_NAME_LENGTH);
        nI = SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)DIVISIONS.recordID);
        rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
      }
      SendMessage(hCtlLIST, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
//
//  STARTDAY - Default to Monday
//
      for(nI = 0; nI < 7; nI++)
      {
        LoadString(hInst, TEXT_009 + nI, tempString, TEMPSTRING_LENGTH);
        SendMessage(hCtlSTARTDAY, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
      SendMessage(hCtlSTARTDAY, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
//
//  Done initializing
//
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  IDHELP
//
        case IDHELP:
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Get the values from the dialog
//
//  LIST
//
          nI = SendMessage(hCtlLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_342, hCtlLIST);
            break;
          }
          pTMSRPT55Selections->DIVISIONSrecordID = SendMessage(hCtlLIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  STARTDAY
//
          pTMSRPT55Selections->startDay = SendMessage(hCtlSTARTDAY, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
//
//  All done
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
