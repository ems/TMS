//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "tms.h"

#include <math.h>
#include "MainFrm.h"

int CTMSApp::RenderField(CF1Book *F1, TableDisplayDef *pTD, int index,
      DATABASEDef *pDATABASE, int lineID, int pieceNumber, char *outString)
{
  GetConnectionTimeDef GCTData;
  PROPOSEDRUNDef PROPOSEDRUN;
  DATABASEDef     derivedKey0;
  DATABASEDef     derivedFile;
  COSTDef        COST;
  float      distance;
  BOOL       bRosterOK;
  BOOL       bStop;
  BOOL       bPlaceHolder;
  long       equivalentTravelTime;
  int        nI;
  int        nJ;
  int        nK;
  int        position;
  int        pos;
  int        rcode2;
  int        extraboardTotal;
  union      TMSRecord
  {
    char     String[256];
    float    Decimal;
    float    Float;
    long     Integer;
    short int Smallint;
    BOOL     Logical;
    long     Date;
    struct   _btrDate
    {
      char   day;
      char   month;
      int    year;
    } btrDate;
    struct   _btrTime
    {
      char   hundredth;
      char   second;
      char   minute;
      char   hour;
    } btrTime;
  };
  union TMSRecord TMSInput;
  long  tempLong;
  char  abbrName[NODES_ABBRNAME_LENGTH + 1];
  long  deadheadTime;
  long  numRecords;
  long  cutAsRuntype;
  char  onNodeName[NODES_ABBRNAME_LENGTH + 1];
  char  offNodeName[NODES_ABBRNAME_LENGTH + 1];
  long  runNumber;
  long  onNode;
  long  offNode;
  long  onTime;
  long  offTime;
  long  startTime;
  long  timeAtNode;
  char  onTchar[16];
  char  offTchar[16];
  char  szSpreadTime[16];
  char  tempString[256];
  long  year, month, day;
  char  *daysOfTheWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  int   selectedFile;

  bRosterOK = TRUE;
  numRecords = F1->GetLastRow();
  position = pTD->fieldInfo.offset[index] - 4;
  if(pTD->fieldInfo.flags[index] & ATTRIBUTES_FLAG_DERIVED)
  {
    memcpy(&derivedKey0.recordID, &pDATABASE->recordData[position], sizeof(long));
    if(derivedKey0.recordID == NO_RECORD || pTD->fieldInfo.linkFile[index] == NO_RECORD)
    {
      memset(TMSInput.String, 0, sizeof(TMSInput.String));
    }
    else
    {
      rcode2 = btrieve(B_GETEQUAL, pTD->fieldInfo.linkFile[index], &derivedFile, &derivedKey0, 0);
      if(rcode2 == 0 || (rcode2 == 22 && pTD->fieldInfo.linkFile[index] == TMS_COMMENTS))
      {
        position = pTD->fieldInfo.linkOffset[index] - 4;
        for(nI = 0; nI < pTD->fieldInfo.numBytes[index]; nI++)
        {
          TMSInput.String[nI] = derivedFile.recordData[position + nI];
        }
        TMSInput.String[pTD->fieldInfo.numBytes[index]] = '\0';
      }
      else
      {
        memset(TMSInput.String, 0x00, sizeof(TMSInput.String));
      }
    }
  }
  else
  {
    for(nI = 0; nI < pTD->fieldInfo.numBytes[index]; nI++)
    {
      TMSInput.String[nI] = pDATABASE->recordData[position + nI];
    }
    TMSInput.String[pTD->fieldInfo.numBytes[index]] = '\0';
  }
  switch(pTD->fieldInfo.dataType[index])
  {
  case DATATYPE_CHARACTER:
    strcpy(outString, TMSInput.String);
    break;

  case DATATYPE_DECIMAL:
    sprintf(outString, "%c", TMSInput.Decimal);
    break;

  case DATATYPE_FLOAT:
//
//  Kludge for conneciton time distances.  Earlier version set NO_DISTANCE to -1
//  So, if we're here displaying a connection distance and it's -1, return blank
//
    if(index == 13 && pTD->fileInfo.fileNumber == 14 && TMSInput.Float == -1.0)
    {
      strcpy(outString, "");
    }
    else
    {
      sprintf(outString, "%9.2f", TMSInput.Float);
    }
    break;

  case DATATYPE_INTEGER:
    if(pTD->fieldInfo.numBytes[index] == 4)
    {
      sprintf(outString, "%10ld", TMSInput.Integer);
    }
    else
    {
      sprintf(outString, "%5d", TMSInput.Smallint);
    }
    break;

  case DATATYPE_LOGICAL:
    sprintf(outString, TMSInput.Logical ? CONST_YES : CONST_NO);
    break;

  case DATATYPE_DATE:
    sprintf(outString, "%4d/%02d/%02d",
          TMSInput.btrDate.year,
          TMSInput.btrDate.month,
          TMSInput.btrDate.day);
    break;

  case DATATYPE_TIME:
    sprintf(outString, "%d:%02d",
          TMSInput.btrTime.hour,
          TMSInput.btrTime.minute);
    break;

  case DATATYPE_LVAR:
    if(strcmp(TMSInput.String, "") == 0)
    {
      strcpy(outString, "");
    }
    else
    {
      strcpy(outString, strtok(TMSInput.String, "\r"));
    }
    break;

  case TMSDATA_TIMEOFDAY + DATATYPE_BIT:
    strcpy(outString, Tchar(TMSInput.Integer));
    break;

  case TMSDATA_TIMEINMINUTES + DATATYPE_BIT:
    if(pTD->fieldInfo.numBytes[index] == 4)
    {
      if(TMSInput.Integer == NO_TIME)
      {
        strcpy(outString, "");
      }
      else
      {
        sprintf(outString, "%d", TMSInput.Integer / 60);
        if(TMSInput.Integer % 60 != 0)
        {
          strcat(outString, "+");
        }
      }
    }
    else
    {
      if(TMSInput.Smallint == NO_TIME)
      {
        strcpy(outString, "");
      }
      else
      {
        sprintf(outString, "%d", TMSInput.Smallint / 60);
        if(TMSInput.Smallint % 60 != 0)
        {
          strcat(outString, "+");
        }
      }
    }
    break;


  case TMSDATA_TIMEINHM + DATATYPE_BIT:
    strcpy(outString, chhmm(TMSInput.Integer));
    break;

  case TMSDATA_INTEGER + DATATYPE_BIT:
    if(pTD->fieldInfo.numBytes[index] == 4)
    {
      sprintf(outString, "%ld", TMSInput.Integer);
    }
    else
    {
      sprintf(outString, "%d", TMSInput.Smallint);
    }
    break;

  case TMSDATA_FIXEDFOUR + DATATYPE_BIT:
    sprintf(outString, "%12.6f", TMSInput.Float);
    break;

  case TMSDATA_PULLOUTTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(BLOCKSVIEW.pullOutTime));
    break;

  case TMSDATA_FIRSTNODE + DATATYPE_BIT:
    NODESKey0.recordID = BLOCKSVIEW.firstNodeRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(outString, "");
    }
    else
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcpy(outString, abbrName);
    }
    break;

  case TMSDATA_FIRSTTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(BLOCKSVIEW.firstNodeTime));
    break;

  case TMSDATA_LASTTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(BLOCKSVIEW.lastNodeTime));
    break;

  case TMSDATA_LASTNODE + DATATYPE_BIT:
    NODESKey0.recordID = BLOCKSVIEW.lastNodeRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(outString, "");
    }
    else
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcpy(outString, abbrName);
    }
    break;

  case TMSDATA_PULLINTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(BLOCKSVIEW.pullInTime));
    break;

  case TMSDATA_TRIPTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(BLOCKSVIEW.lastNodeTime - BLOCKSVIEW.firstNodeTime));
    break;

  case TMSDATA_LAYOVERTIME + DATATYPE_BIT:
    if(m_bCompressedBlocks)
    {
      strcpy(outString, chhmm(BLOCKSVIEW.layoverTime));
    }
    else
    {
      if(BLOCKSVIEW.blockNumber != PREVBLOCKSVIEW.blockNumber)
      {
        strcpy(outString, "");
      }
      else
      {
        if(BLOCKSVIEW.deadheadTime == NO_TIME)
        {
          deadheadTime = 0;
          if(BLOCKSVIEW.firstNodeRecordID != PREVBLOCKSVIEW.lastNodeRecordID)
          {
            GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
            GCTData.toPATTERNNAMESrecordID = NO_RECORD;
            GCTData.fromROUTESrecordID = PREVBLOCKSVIEW.lastRouteRecordID;
            GCTData.fromSERVICESrecordID = PREVBLOCKSVIEW.lastServiceRecordID;
            GCTData.toROUTESrecordID = BLOCKSVIEW.firstRouteRecordID;
            GCTData.toSERVICESrecordID = BLOCKSVIEW.firstServiceRecordID;
            GCTData.fromNODESrecordID = PREVBLOCKSVIEW.lastNodeRecordID;
            GCTData.toNODESrecordID = BLOCKSVIEW.firstNodeRecordID;
            GCTData.timeOfDay = PREVBLOCKSVIEW.lastNodeTime;
            distance = 0;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            deadheadTime = (deadheadTime == NO_TIME ? 0 : deadheadTime);
          }
          BLOCKSVIEW.deadheadTime = deadheadTime;
        }
//
//  Check for secure
//
        BLOCKSVIEW.layoverTime = BLOCKSVIEW.firstNodeTime - PREVBLOCKSVIEW.lastNodeTime - BLOCKSVIEW.deadheadTime;
        strcpy(outString, chhmm(BLOCKSVIEW.layoverTime));
        if(NodesEquivalent(PREVBLOCKSVIEW.lastNodeRecordID, BLOCKSVIEW.firstNodeRecordID, &equivalentTravelTime))
        {
          NODESKey0.recordID = BLOCKSVIEW.firstNodeRecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          if(NODES.flags & NODES_FLAG_SECURE && BLOCKSVIEW.layoverTime > 1800)
          {
            strcat(outString, " (SL)");
          }
        }
      }
    }
    break;

  case TMSDATA_DEADHEADTIME + DATATYPE_BIT:
    if(m_bCompressedBlocks)
    {
      strcpy(outString, chhmm(BLOCKSVIEW.deadheadTime));
    }
    else
    {
      if(BLOCKSVIEW.blockNumber != PREVBLOCKSVIEW.blockNumber)
      {
        strcpy(outString, "");
      }
      else
      {
        if(BLOCKSVIEW.deadheadTime == NO_TIME)
        {
          deadheadTime = 0;
          if(BLOCKSVIEW.firstNodeRecordID != PREVBLOCKSVIEW.lastNodeRecordID)
          {
            GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
            GCTData.toPATTERNNAMESrecordID = NO_RECORD;
            GCTData.fromROUTESrecordID = PREVBLOCKSVIEW.lastRouteRecordID;
            GCTData.fromSERVICESrecordID = PREVBLOCKSVIEW.lastServiceRecordID;
            GCTData.toROUTESrecordID = BLOCKSVIEW.firstRouteRecordID;
            GCTData.toSERVICESrecordID = BLOCKSVIEW.firstServiceRecordID;
            GCTData.fromNODESrecordID = PREVBLOCKSVIEW.lastNodeRecordID;
            GCTData.toNODESrecordID = BLOCKSVIEW.firstNodeRecordID;
            GCTData.timeOfDay = PREVBLOCKSVIEW.lastNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            deadheadTime = (deadheadTime == NO_TIME ? 0 : deadheadTime);
          }
          BLOCKSVIEW.deadheadTime = deadheadTime;
        }
        strcpy(outString, chhmm(BLOCKSVIEW.deadheadTime));
      }
    }
    break;

  case TMSDATA_TRIPDISTANCE + DATATYPE_BIT:
  case TMSDATA_CUMULATIVEDISTANCE + DATATYPE_BIT:
    if(pTD->fieldInfo.dataType[index] == TMSDATA_TRIPDISTANCE + DATATYPE_BIT)
    {
      TMSInput.Float = m_bCompressedBlocks ? (float)0.00 : BLOCKSVIEW.tripDistance;
    }
    else
    {
      TMSInput.Float = BLOCKSVIEW.totalDistance;
    }
    if(m_bCompressedBlocks)
    {
      sprintf(outString, "");
    }
    else
    {
      sprintf(outString, "%9.2f", TMSInput.Float);
    }
    break;

  case TMSDATA_RUNTYPE + DATATYPE_BIT:
    if(pTD->fileInfo.fileNumber == TMS_CREWONLY)
    {
      nI = LOWORD(TMSInput.Integer);
      nJ = HIWORD(TMSInput.Integer);
    }
    else
    {
      nI = LOWORD(RUNSVIEW[pieceNumber].runType);
      nJ = HIWORD(RUNSVIEW[pieceNumber].runType);
    }
    strcpy(outString, "");
    if(nI < NUMRUNTYPES && nJ < NUMRUNTYPESLOTS && nI != NO_RECORD && nJ != NO_RECORD)
    {
      if(pTD->fileInfo.fileNumber == TMS_CREWONLY ||
            RUNSVIEW[pieceNumber].runType != NO_RECORD)
      {      
        strcpy(outString, RUNTYPE[nI][nJ].localName);
      }
    }
    break;

  case TMSDATA_BLOCKNUMBER + DATATYPE_BIT:
    if(RUNSVIEW[pieceNumber].blockNumber == 9101)
    {
      int a = 1;
    }
    if(RUNSVIEW[pieceNumber].blockNumber == 0)
    {
      strcpy(outString, "");
    }
    else
    {
      sprintf(outString, "%10ld", RUNSVIEW[pieceNumber].blockNumber);
    }
    break;

  case TMSDATA_REPORTATTIME + DATATYPE_BIT:
    tempLong = RUNSVIEW[pieceNumber].runOnTime -
          RUNSVIEW[pieceNumber].reportTime -
          RUNSVIEW[pieceNumber].startOfPieceTravel;
    if(RUNSVIEW[pieceNumber].startOfPieceExtraboardStart != NO_TIME)
    {
      tempLong = RUNSVIEW[pieceNumber].startOfPieceExtraboardStart;
    }
    strcpy(outString, Tchar(tempLong));
    break;

  case TMSDATA_RUNONTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].runOnTime));
    break;

  case TMSDATA_RUNOFFTIME + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].runOffTime));
    break;

  case TMSDATA_PLATFORMTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].platformTime));
    break;

  case TMSDATA_REPORTATLOCATION + DATATYPE_BIT:
    NODESKey0.recordID = RUNSVIEW[pieceNumber].startOfPieceNODESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(outString, "");
    }
    else
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcpy(outString, abbrName);
    }
    break;

  case TMSDATA_REPORTTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].reportTime));
    break;

  case TMSDATA_TURNINTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].turninTime));
    break;

  case TMSDATA_REPORTTURNIN + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].reportTime + RUNSVIEW[pieceNumber].turninTime));
    break;

  case TMSDATA_STARTOFPIECETRAVEL + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].startOfPieceTravel));
    break;

  case TMSDATA_ENDOFPIECETRAVEL + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].endOfPieceTravel));
    break;

  case TMSDATA_TRAVELTIME + DATATYPE_BIT:
    strcpy(outString, (RUNSVIEW[pieceNumber].travelTime < 0 ? "" : chhmm(RUNSVIEW[pieceNumber].travelTime)));
    break;

  case TMSDATA_MEAL + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].paidMeal));
    break;

  case TMSDATA_SPREADOT + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].spreadOT));
    break;

  case TMSDATA_MAKEUPTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].makeUpTime));
    break;

  case TMSDATA_PAIDBREAKS + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].paidBreaks));
    break;

  case TMSDATA_OVERTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].overTime));
    break;

  case TMSDATA_PREMIUMTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].premiumTime));
    break;

  case TMSDATA_PAYTIME + DATATYPE_BIT:
    strcpy(outString, chhmm(RUNSVIEW[pieceNumber].payTime));
    break;

  case TMSDATA_BOPEXTRABOARDSTART + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].startOfPieceExtraboardStart));
    break;

  case TMSDATA_BOPEXTRABOARDEND + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].startOfPieceExtraboardEnd));
    break;

  case TMSDATA_EOPEXTRABOARDSTART + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].endOfPieceExtraboardStart));
    break;

  case TMSDATA_EOPEXTRABOARDEND + DATATYPE_BIT:
    strcpy(outString, Tchar(RUNSVIEW[pieceNumber].endOfPieceExtraboardEnd));
    break;

  case TMSDATA_ROSTERDAY1 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY2 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY3 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY4 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY5 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY6 + DATATYPE_BIT:
  case TMSDATA_ROSTERDAY7 + DATATYPE_BIT:
    pos = position + (m_RosterWeek) * sizeof(ROSTERWEEKDef);
    memcpy(&tempLong, &pDATABASE->recordData[pos], sizeof(long));
    if(tempLong == NO_RECORD)
    {
      strcpy(outString, "* Off *");
    }
    else
    {
      if((pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY1 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY1_CREWONLY) ||
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY2 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY2_CREWONLY) ||   
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY3 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY3_CREWONLY) ||   
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY4 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY4_CREWONLY) ||   
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY5 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY5_CREWONLY) ||   
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY6 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY6_CREWONLY) ||   
         (pTD->fieldInfo.dataType[index] == TMSDATA_ROSTERDAY7 + DATATYPE_BIT &&
            RosterData[3] & ROSTER_FLAG_DAY7_CREWONLY))
//
//  This run is from the CrewOnly Table
      {
        selectedFile = TMS_CREWONLY;
        CREWONLYKey0.recordID = tempLong;
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        if(rcode2 != 0)
        {
          sprintf(outString, "Crew-only Run\nnot found\n\nRecID=%ld", tempLong);
          bRosterOK = FALSE;
        }
        else
        {
          bPlaceHolder = CREWONLY.flags & CREWONLY_FLAG_PLACEHOLDER;
          btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          pieceNumber = 0;
          extraboardTotal = 0;
          runNumber = CREWONLY.runNumber;
          while(rcode2 == 0 && CREWONLY.runNumber == runNumber)
          {
            onTime = CREWONLY.startTime;
            offTime = CREWONLY.endTime;
            if(CREWONLY.pieceNumber == 1 || bPlaceHolder)
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
          for(bStop = FALSE, nI = 0; nI < pieceNumber; nI++)
          {
            if(bPlaceHolder)
            {
              break;
            }
            if(PROPOSEDRUN.piece[nI].fromTime == NO_TIME ||
                  PROPOSEDRUN.piece[nI].fromNODESrecordID == NO_RECORD ||
                  PROPOSEDRUN.piece[nI].toTime == NO_TIME ||
                  PROPOSEDRUN.piece[nI].toNODESrecordID == NO_RECORD)
            {
              bStop = TRUE;
              break;
            }
          }
          if(!bStop)
          {
            RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
          }
          else
          {
            bRosterOK = FALSE;
          }
        }
      }
//
//  This run is from the Runs Table
//
      else
      {
        selectedFile = TMS_RUNS;
        RUNSKey0.recordID = tempLong;
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        if(rcode2 != 0)
        {
          sprintf(outString, "Regular Run\nnot found\n\nRecID=%ld", tempLong);
          bRosterOK = FALSE;
        }
        else
        {
          btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          pieceNumber = 0;
          extraboardTotal = 0;
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
            PROPOSEDRUN.piece[pieceNumber++].after.endTime = RUNS.after.endTime;
            offNode = RUNS.end.NODESrecordID;
            if(RUNS.flags & RUNS_FLAG_EXTRABOARDPRIOR)
            {
              extraboardTotal += (RUNS.prior.endTime - RUNS.prior.startTime);
            }
            if(RUNS.flags & RUNS_FLAG_EXTRABOARDAFTER)
            {
              extraboardTotal += (RUNS.after.endTime - RUNS.after.startTime);
            }
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          PROPOSEDRUN.numPieces = pieceNumber;
          for(bStop = FALSE, nI = 0; nI < pieceNumber; nI++)
          {
            if(PROPOSEDRUN.piece[nI].fromTime == NO_TIME ||
                  PROPOSEDRUN.piece[nI].fromNODESrecordID == NO_RECORD ||
                  PROPOSEDRUN.piece[nI].fromTRIPSrecordID == NO_RECORD ||
                  PROPOSEDRUN.piece[nI].toTime == NO_TIME ||
                  PROPOSEDRUN.piece[nI].toNODESrecordID == NO_RECORD ||
                  PROPOSEDRUN.piece[nI].toTRIPSrecordID == NO_RECORD)
            {
              bStop = TRUE;
              break;
            }
          }
          if(!bStop)
          {
            RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
          }
          else
          {
            bRosterOK = FALSE;
          }
        } 
      }
//
//  Ok - four output formats - compressed and expanded
//
//  Compressed
//
      if(bRosterOK)
      {
        if(rosterFormat == PREFERENCES_ROSTERCOMPRESSED)
        {
          NODESKey0.recordID = onNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(onNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(onNodeName, NODES_ABBRNAME_LENGTH);
          NODESKey0.recordID = offNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(offNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(offNodeName, NODES_ABBRNAME_LENGTH);
          strcpy(onTchar, Tchar(startTime));
          strcpy(offTchar, Tchar(offTime));
          nI = (short int)LOWORD(cutAsRuntype);
          nJ = (short int)HIWORD(cutAsRuntype);
          if(nI >= 0 && nJ >= 0 && nI < NUMRUNTYPES && nJ < NUMRUNTYPESLOTS)
          {
            if(extraboardTotal == 0)
            {
              sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s",
                   runNumber, chhmm(COST.TOTAL.payTime), RUNTYPE[nI][nJ].localName,
                   onTchar, onNodeName, offTchar, offNodeName);
            }
            else
            {
              strcpy(tempString, chhmm(extraboardTotal));
              sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s\nTotal XBD: %s",
                   runNumber, chhmm(COST.TOTAL.payTime), RUNTYPE[nI][nJ].localName,
                   onTchar, onNodeName, offTchar, offNodeName, tempString);
            }
          }
          else
          {
            if(extraboardTotal == 0)
            {
              sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s",
                   runNumber, chhmm(COST.TOTAL.payTime), "* Invalid Runtype *",
                   onTchar, onNodeName, offTchar, offNodeName);
            }
            else
            {
              strcpy(tempString, chhmm(extraboardTotal));
              sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s\nTotal XBD: %s",
                   runNumber, chhmm(COST.TOTAL.payTime), "* Invalid Runtype *",
                   onTchar, onNodeName, offTchar, offNodeName, tempString);
            }
          }
        }
//
//  Expanded
//
        else if(rosterFormat == PREFERENCES_ROSTEREXPANDED)
        {
          long startTime, endTime, xbdSum;
          char routeNumber[ROUTES_NUMBER_LENGTH + 1];
//
//  Re-establish position
//
          if(selectedFile == TMS_RUNS)
          {
            RUNSKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          else
          {
            CREWONLYKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          }
          sprintf(outString, "Run:%ld\n", runNumber);
//
//  Loop through all the pieces
//
          xbdSum = 0;
          for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
          {
//
//  Get start and end data and the runtype
//
            if(selectedFile == TMS_RUNS)
            {
              RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                    RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
              if(RUNS.pieceNumber == 1)
              {
                cutAsRuntype = RUNS.cutAsRuntype;
                startTime = onTime;
                onNode = RUNS.start.NODESrecordID;
              }
            }
            else
            {
              onTime = CREWONLY.startTime;
              offTime = CREWONLY.endTime;
              if(CREWONLY.pieceNumber == 1)
              {
                cutAsRuntype = CREWONLY.cutAsRuntype;
                startTime = onTime;
                onNode = CREWONLY.startNODESrecordID;
              }
            }
//
//  See when the real start time is
//
            if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDPRIOR))
            {
              startTime = RUNS.prior.startTime;
              xbdSum += (RUNS.prior.endTime - RUNS.prior.startTime);
            }
            else if(COST.TRAVEL[nI].startTravelTime != 0)
              startTime = onTime - COST.TRAVEL[nI].startTravelTime; // - COST.TRAVEL[nI].startDwellTime;
            else
              startTime = onTime;
//
//  Get the on location name
//
            if(selectedFile == TMS_RUNS)
              NODESKey0.recordID = RUNS.start.NODESrecordID;
            else
              NODESKey0.recordID = CREWONLY.startNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(onNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(onNodeName, NODES_ABBRNAME_LENGTH);
//
//  Get the route number
//
            if(selectedFile == TMS_RUNS)
            {
              TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
              btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
              strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
              trim(routeNumber, ROUTES_NUMBER_LENGTH);
            }
//
//  Get the off location
//
            if(selectedFile == TMS_RUNS)
              NODESKey0.recordID = RUNS.end.NODESrecordID;
            else
              NODESKey0.recordID = CREWONLY.endNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(offNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(offNodeName, NODES_ABBRNAME_LENGTH);
//
//  Figure out the real finish time
//
            if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDAFTER))
            {
              endTime = RUNS.after.endTime;
              xbdSum += (RUNS.after.endTime - RUNS.after.startTime);
            }
            else if(COST.TRAVEL[nI].endTravelTime != 0)
              endTime = offTime + COST.TRAVEL[nI].endTravelTime; // + COST.TRAVEL[nI].endDwellTime;
            else
              endTime = offTime;
//
//  Got it all - write it out
//
//  Piece number
            sprintf(tempString, "Pce:%d\n", nI + 1);
            strcat(outString, tempString);
//
//  Start time
//
            if(COST.PIECECOST[nI].reportTime != NO_TIME)
              startTime -= COST.PIECECOST[nI].reportTime;
            strcpy(szarString, Tchar(startTime));
            if(szarString[0] == ' ')
              memcpy(szarString, &szarString[1], strlen(szarString));
            sprintf(tempString, "Rpt:%s\n", szarString);
            strcat(outString, tempString);
//
//  On time
//
            strcpy(szarString, Tchar(onTime));
            if(szarString[0] == ' ')
              memcpy(szarString, &szarString[1], strlen(szarString));
            sprintf(tempString, "On: %s\n", szarString);
            strcat(outString, tempString);
//
//  At location
//
            sprintf(tempString, "At: %s\n", onNodeName);
            strcat(outString, tempString);
//
//  Route number
//
            if(selectedFile == TMS_RUNS)
            {
              if(routeNumber[0] == ' ')
                memcpy(routeNumber, &routeNumber[1], ROUTES_NUMBER_LENGTH);
              sprintf(tempString, "Rte:%s\n", routeNumber);
              strcat(outString, tempString);
            }
//
//  Off time
//
            strcpy(szarString, Tchar(offTime));
            if(szarString[0] == ' ')
              memcpy(szarString, &szarString[1], strlen(szarString));
            sprintf(tempString, "Off:%s\n", szarString);
            strcat(outString, tempString);
//
//  At location
//
            sprintf(tempString, "At: %s\n", offNodeName);
            strcat(outString, tempString);
//
//  End time
//
            strcpy(szarString, Tchar(endTime));
            if(szarString[0] == ' ')
              memcpy(szarString, &szarString[1], strlen(szarString));
            sprintf(tempString, "Fin:%s\n\n", szarString);
            strcat(outString, tempString);
//
//  Get the next piece and go back
//
            if(selectedFile == TMS_RUNS)
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            else
              rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          }
//
//  And the extraboard sum
//
          if(xbdSum > 0)
          {
            sprintf(tempString, "XBD:%s\n\n", chhmm(xbdSum));
            strcat(outString, tempString);
          }
//
//  And the pay time and the runtype
//
          sprintf(tempString, "Tot:%s\n", chhmm(COST.TOTAL.payTime));
          strcat(outString, tempString);
          nI = (short int)LOWORD(cutAsRuntype);
          nJ = (short int)HIWORD(cutAsRuntype);
          if(nI >= 0 && nJ >= 0 && nI < NUMRUNTYPES && nJ < NUMRUNTYPESLOTS)
          {
            sprintf(tempString, "%s", RUNTYPE[nI][nJ].localName);
            strcat(outString, tempString);
          }
        }
//
//  Roster output style 3
//
        else if(rosterFormat == PREFERENCES_ROSTERSTYLE3)
        {
          long startTime, endTime, xbdSum;
//
//  Re-establish position
//
          if(selectedFile == TMS_RUNS)
          {
            RUNSKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          else
          {
            CREWONLYKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          }
          sprintf(outString, "Run:%ld\n", runNumber);
//
//  Loop through all the pieces
//
          xbdSum = 0;
          for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
          {
//
//  Get start and end data and the runtype
//
            if(selectedFile == TMS_RUNS)
            {
              RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                    RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
              if(RUNS.pieceNumber == 1)
              {
                cutAsRuntype = RUNS.cutAsRuntype;
                startTime = onTime;
                onNode = RUNS.start.NODESrecordID;
              }
            }
            else
            {
              onTime = CREWONLY.startTime;
              offTime = CREWONLY.endTime;
              if(CREWONLY.pieceNumber == 1)
              {
                cutAsRuntype = CREWONLY.cutAsRuntype;
                startTime = onTime;
                onNode = CREWONLY.startNODESrecordID;
              }
            }
//
//  Show the runtype and pay time:
//
            if(nI == 0)
            {
              nJ = (short int)LOWORD(cutAsRuntype);
              nK = (short int)HIWORD(cutAsRuntype);
              if(nI >= 0 && nJ >= 0 && nJ < NUMRUNTYPES && nK < NUMRUNTYPESLOTS)
              {
                sprintf(tempString, "%s\n", RUNTYPE[nJ][nK].localName);
                strcat(outString, tempString);
              }
              sprintf(tempString, "Pay:%s\n\n", chhmm(COST.TOTAL.payTime));
              strcat(outString, tempString);
            }
//
//  See when the real start time and location is
//
            if(COST.TRAVEL[nI].startTravelTime != 0)
            {
              startTime = onTime - COST.TRAVEL[nI].startTravelTime; // - COST.TRAVEL[nI].startDwellTime;
              NODESKey0.recordID = COST.TRAVEL[nI].startNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(onNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(onNodeName, NODES_ABBRNAME_LENGTH);
            }
            else
            {
              if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDPRIOR))
              {
                startTime = RUNS.prior.startTime;
                xbdSum += (RUNS.prior.endTime - RUNS.prior.startTime);
              }
              else
              {
                startTime = onTime;
              }
              NODESKey0.recordID = onNode;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(onNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(onNodeName, NODES_ABBRNAME_LENGTH);
            }
//
//  Figure out the real finish time and location
//
            if(COST.TRAVEL[nI].endTravelTime != 0)
            {
              endTime = offTime + COST.TRAVEL[nI].endTravelTime; // + COST.TRAVEL[nI].endDwellTime;
              NODESKey0.recordID = COST.TRAVEL[nI].endNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(offNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(offNodeName, NODES_ABBRNAME_LENGTH);
            }
            else
            {
              if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDAFTER))
              {
                endTime = RUNS.after.endTime;
                xbdSum += (RUNS.after.endTime - RUNS.after.startTime);
              }
              else
              {
                endTime = offTime;
              }
//
//  Get the off location
//
              if(selectedFile == TMS_RUNS)
                NODESKey0.recordID = RUNS.end.NODESrecordID;
              else
                NODESKey0.recordID = CREWONLY.endNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(offNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(offNodeName, NODES_ABBRNAME_LENGTH);
            }
//
//  Got it all - write it out
//
//  Piece number
            sprintf(tempString, "Pce:%d\n", nI + 1);
            strcat(outString, tempString);
//
//  Start time
//
            if(COST.PIECECOST[nI].reportTime != NO_TIME)
              startTime -= COST.PIECECOST[nI].reportTime;
            strcpy(szarString, Tchar(startTime));
            if(szarString[0] == ' ')
              memcpy(szarString, &szarString[1], strlen(szarString));
            sprintf(tempString, "Rpt:%s\n", szarString);
            strcat(outString, tempString);
//
//  At location
//
            sprintf(tempString, "At: %s\n", onNodeName);
            strcat(outString, tempString);
//
//  End time
//
            if(COST.PIECECOST[nI].turninTime != NO_TIME)
            {
              endTime += COST.PIECECOST[nI].turninTime;
            }
            strcpy(szarString, Tchar(endTime));
            if(szarString[0] == ' ')
            {
              memcpy(szarString, &szarString[1], strlen(szarString));
            }
            sprintf(tempString, "End:%s\n", szarString);
            strcat(outString, tempString);
//
//  At location
//
            sprintf(tempString, "At: %s\n\n", offNodeName);
            strcat(outString, tempString);
//
//  Get the next piece and go back
//
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
//
//  And the extraboard sum
//
          if(xbdSum > 0)
          {
            sprintf(tempString, "XBD:%s\n\n", chhmm(xbdSum));
            strcat(outString, tempString);
          }
        }
//
//  Roster output style 4
//
        else if(rosterFormat == PREFERENCES_ROSTERSTYLE4)
        {
          long startTime, endTime, xbdSum;
//
//  Re-establish position
//
          if(selectedFile == TMS_RUNS)
          {
            RUNSKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          else
          {
            CREWONLYKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          }
//
//  Loop through all the pieces
//
          xbdSum = 0;
          for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
          {
//
//  Get start and end data and the runtype
//
            if(selectedFile == TMS_RUNS)
            {
              RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                    RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
              if(RUNS.pieceNumber == 1)
              {
                cutAsRuntype = RUNS.cutAsRuntype;
                startTime = onTime;
                onNode = RUNS.start.NODESrecordID;
              }
            }
            else
            {
              onTime = CREWONLY.startTime;
              offTime = CREWONLY.endTime;
              if(CREWONLY.pieceNumber == 1)
              {
                cutAsRuntype = CREWONLY.cutAsRuntype;
                startTime = onTime;
                onNode = CREWONLY.startNODESrecordID;
              }
            }
//
//  Do the extraboard sum
//
            if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDPRIOR))
            {
              xbdSum += (RUNS.prior.endTime - RUNS.prior.startTime);
            }
            if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDAFTER))
            {
              xbdSum += (RUNS.after.endTime - RUNS.after.startTime);
            }
//
//  See when the real start time and location is
//
            if(nI == 0)
            {
              startTime = onTime - COST.TRAVEL[nI].startTravelTime; // - COST.TRAVEL[nI].startDwellTime;
              NODESKey0.recordID = COST.TRAVEL[nI].startNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(onNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(onNodeName, NODES_ABBRNAME_LENGTH);
              if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDPRIOR))
              {
                startTime = RUNS.prior.startTime;
              }
//              else
//              {
//                startTime = onTime;
//              }
              else if(COST.PIECECOST[0].reportTime != NO_TIME)
              {
                startTime -= COST.PIECECOST[0].reportTime;
              }
            }
//
//  Figure out the real finish time and location
//
            if(nI == PROPOSEDRUN.numPieces - 1)
            {
              endTime = offTime + COST.TRAVEL[nI].endTravelTime;
              NODESKey0.recordID = COST.TRAVEL[nI].endNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(offNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(offNodeName, NODES_ABBRNAME_LENGTH);
              if(selectedFile == TMS_RUNS && (RUNS.flags & RUNS_FLAG_EXTRABOARDAFTER))
              {
                endTime = RUNS.after.endTime;
              }
//              else
//              {
//                endTime = offTime;
//              }
              if(COST.PIECECOST[nI].turninTime != NO_TIME)
              {
                endTime += COST.PIECECOST[nI].turninTime;
              }
            }
//
//  Get the next piece and go back
//
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
//
//  Output the mess
//
          strcpy(onTchar, Tchar(startTime));
          strcpy(offTchar, Tchar(endTime));
          strcpy(szSpreadTime, chhmm(endTime - startTime));
          nI = (short int)LOWORD(cutAsRuntype);
          nJ = (short int)HIWORD(cutAsRuntype);
          if(nI >= 0 && nJ >= 0 && nI < NUMRUNTYPES && nJ < NUMRUNTYPESLOTS)
          {
            strcpy(tempString, chhmm(extraboardTotal));
            sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s\nIncl XBD: %s\nSpread: %s",
                 runNumber, chhmm(COST.TOTAL.payTime), RUNTYPE[nI][nJ].localName,
                 onTchar, onNodeName, offTchar, offNodeName, tempString, szSpreadTime);
          }
          else
          {
            strcpy(tempString, chhmm(extraboardTotal));
            sprintf(outString, "Run: %ld\nPay: %s\n%s\nOn: %s\nAt: %s\nOff: %s\nAt: %s\nIncl XBD: %s",
                 runNumber, chhmm(COST.TOTAL.payTime), "* Invalid Runtype *",
                 onTchar, onNodeName, offTchar, offNodeName, tempString);
          }
        }
//
//  Roster output style 5 (Run number only)
//
        else if(rosterFormat == PREFERENCES_ROSTERSTYLE5)
        {
//
//  Re-establish position
//
          if(selectedFile == TMS_RUNS)
          {
            RUNSKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          }
          else
          {
            CREWONLYKey0.recordID = tempLong;
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          }
          sprintf(outString, "Run:%ld\n", runNumber);
        }
//
//  Put the costs in
//
        nI = strlen(outString);
        if(nI > 255)
          outString[255] = '\0';
        RosterData[0] += COST.TOTAL.payTime;
        RosterData[1] += COST.TOTAL.overTime;
      }
    }
    break;

  case TMSDATA_ROSTERHOURS + DATATYPE_BIT:
    tempLong = RosterData[0] - RosterData[1];
    if(tempLong < 0)
      strcpy(tempString, "");
    else
      strcpy(tempString, chhmm(tempLong));
    sprintf(outString, "%s", tempString);
    break;

  case TMSDATA_ROSTEROT + DATATYPE_BIT:
    if(RosterData[0] > OVERTIME.weeklyAfter[WEEK5DAYS])
      tempLong = (long)((RosterData[0] - OVERTIME.weeklyAfter[WEEK5DAYS]) *
            (OVERTIME.weeklyRate[WEEK5DAYS] - 1));
    else
      tempLong = 0;
    RosterData[1] += tempLong;
    strcpy(tempString, (RosterData[1] == NO_TIME ? "" : chhmm(RosterData[1])));
    sprintf(outString, "%s", tempString);
    break;

  case TMSDATA_ROSTERPAY + DATATYPE_BIT:
    tempLong = RosterData[0] + RosterData[1];
//   tempLong = RosterData[0];
    if(tempLong < OVERTIME.guarantee5DayWorkWeek && OVERTIME.guarantee5DayWorkWeek != NO_TIME)
      tempLong = OVERTIME.guarantee5DayWorkWeek;
    strcpy(tempString, (tempLong == NO_TIME ? "" : chhmm(tempLong)));
    sprintf(outString, "%s", tempString);
    break;

  case TMSDATA_DATE + DATATYPE_BIT:
    GetYMD(TMSInput.Date, &year, &month, &day);
    sprintf(outString, "%04d/%02d/%02d", year, month, day);
    break;

  case TMSDATA_ROSTERDRIVER + DATATYPE_BIT:
    if(TMSInput.Integer == NO_RECORD)
      strcpy(outString, "Unassigned\n\n\n");
    else
    {
      DRIVERSKey0.recordID = TMSInput.Integer;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        sprintf(outString, "Driver not found\n\nRecID=%ld", TMSInput.Integer);
        bRosterOK = FALSE;
      }
      else
      {
        strncpy(outString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(outString, DRIVERS_BADGENUMBER_LENGTH);
        strcat(outString, "\n\n");
        strncpy(szarString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(szarString, DRIVERS_LASTNAME_LENGTH);
        strcat(outString, szarString);
        strcat(outString, "\n");
        strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(outString, szarString);
        strcat(outString, " ");
        strncpy(szarString, DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
        trim(szarString, DRIVERS_INITIALS_LENGTH);
        strcat(outString, szarString);
      }
    }
    break;
//
//  Pay time in decimal minutes
//
  case TMSDATA_TIMEINDECIMALMINUTES + DATATYPE_BIT:
    sprintf(outString, Tdec(RUNSVIEW[pieceNumber].payTime));
    break;
//
//  Physical start location of a piece
//
  case TMSDATA_PIECESTARTLOCATION + DATATYPE_BIT:
    NODESKey0.recordID = RUNSVIEW[pieceNumber].startOfPieceNODESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
      strcpy(outString, "");
    else
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcpy(outString, abbrName);
    }
    break;
//
//  Physical end location of a piece
//
  case TMSDATA_PIECEENDLOCATION + DATATYPE_BIT:
    NODESKey0.recordID = RUNSVIEW[pieceNumber].endOfPieceNODESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
      strcpy(outString, "");
    else
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      strcpy(outString, abbrName);
    }
    break;
//
//  Sign-off time of a piece
//
  case TMSDATA_SIGNOFFTIME + DATATYPE_BIT:
    tempLong = RUNSVIEW[pieceNumber].runOffTime +
          RUNSVIEW[pieceNumber].turninTime +
          RUNSVIEW[pieceNumber].endOfPieceTravel;
    strcpy(outString, Tchar(tempLong));
    break;
//
//  Day of week of a time check
//
  case TMSDATA_DAYOFTIMECHECK + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    GetYMD(TIMECHECKS.checkDate, &year, &month, &day);
    {
      COleDateTime checkDate(year, month, day, 0, 0, 0);
      int          dayOfWeek = checkDate.GetDayOfWeek();
      if(dayOfWeek >= 1 && dayOfWeek <= 7)
        strcpy(outString, daysOfTheWeek[dayOfWeek - 1]);
      else
        strcpy(outString, "---");
    }
    break;
//
//  Direction abbreviation of a route
//
  case TMSDATA_ROUTEDIRECTION + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    ROUTESKey0.recordID = TIMECHECKS.ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TIMECHECKS.directionIndex];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(outString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(outString, DIRECTIONS_ABBRNAME_LENGTH);
    break;
//
//  Scheduled time at a timepoint
//
  case TMSDATA_SCHEDULEDTIME + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    timeAtNode = GetTimeAtNode(TIMECHECKS.TRIPSrecordID, TIMECHECKS.NODESrecordID,
          TIMECHECKS.nodePositionInTrip, (TRIPSDef *)NULL);
    strcpy(outString, Tchar(timeAtNode));
    timecheckScheduled = timeAtNode;
    timecheckActual = TIMECHECKS.actualTime;
    break;
//
//  Supervisor index displayed as supervisor name
//
  case TMSDATA_SUPERVISOR + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    if(TIMECHECKS.supervisorIndex >= 0 && TIMECHECKS.supervisorIndex < MAXROADSUPERVISORS)
    {
      strcpy(outString, m_RoadSupervisors[TIMECHECKS.supervisorIndex].szText);
    }
    else
    {
      sprintf(outString, "*Unknown* (%ld)", TIMECHECKS.supervisorIndex);
    }
    break;
//
//  Passenger load index displayed as passenger load text
//
  case TMSDATA_PASSENGERLOAD + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    if(TIMECHECKS.passengerLoadIndex >= 0 && TIMECHECKS.passengerLoadIndex < MAXPASSENGERLOADS)
    {
      strcpy(outString, m_PassengerLoads[TIMECHECKS.passengerLoadIndex].szText);
    }
    else
    {
      sprintf(outString, "*Unknown* (%ld)", TIMECHECKS.passengerLoadIndex);
    }
    break;
//
//  Weather Conditions index displayed as weather condition text
//
  case TMSDATA_WEATHER + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    nI = LOWORD(TIMECHECKS.weatherIndex);
    if(nI >= 0 && nI < MAXWEATHERCONDITIONS)
    {
      strcpy(outString, m_WeatherConditions[nI].szText);
    }
    else
    {
      sprintf(outString, "*Unknown* (%d)", nI);
    }
    break;
//
//  Traffic Conditions index displayed as traffic condition text
//
  case TMSDATA_TRAFFIC + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    nI = HIWORD(TIMECHECKS.weatherIndex);
    if(nI >= 0 && nI < MAXTRAFFICCONDITIONS)
    {
      strcpy(outString, m_TrafficConditions[nI].szText);
    }
    else
    {
      sprintf(outString, "*Unknown* (%d)", nI);
    }
    break;
//
//  Other Comments index displayed as other comments text
//
  case TMSDATA_OTHERCOMMENTS + DATATYPE_BIT:
    memcpy(&TIMECHECKS, pDATABASE, sizeof(TIMECHECKSDef));
    if(TIMECHECKS.otherCommentsIndex >= 0 && TIMECHECKS.otherCommentsIndex < MAXOTHERCOMMENTS)
    {
      strcpy(outString, m_OtherComments[TIMECHECKS.otherCommentsIndex].szText);
    }
    else
    {
      sprintf(outString, "*Unknown* (%ld)", TIMECHECKS.otherCommentsIndex);
    }
    break;
//
//  Difference between actual and scheduled time.
//  This relies upon scheduled time being displayed
//  prior to figuring out this field.
//
  case TMSDATA_ACTSCHEDDIFF + DATATYPE_BIT:
    sprintf(outString, "%ld", (timecheckActual - timecheckScheduled) / 60);
    break;
//
//  Spread time on a run
//
  case TMSDATA_SPREADTIME + DATATYPE_BIT:
    sprintf(outString, "%s", (RUNSVIEW[pieceNumber].spreadTime < 0 ? "" : chhmm(RUNSVIEW[pieceNumber].spreadTime)));
    break;
//
//  Roster pay time without daily overtime added
//
  case TMSDATA_ROSTERPAYNOOT + DATATYPE_BIT:
    tempLong = RosterData[0];
    if(tempLong < OVERTIME.guarantee5DayWorkWeek && OVERTIME.guarantee5DayWorkWeek != NO_TIME)
      tempLong = OVERTIME.guarantee5DayWorkWeek;
    strcpy(tempString, (tempLong == NO_TIME ? "" : chhmm(tempLong)));
    sprintf(outString, "%s", tempString);
    break;
//
//  Service of a trip on a run
//
  case TMSDATA_SERVICEOFTRIPONRUN + DATATYPE_BIT:
    SERVICESKey0.recordID = RUNSVIEW[pieceNumber].onTripSERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(outString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(outString, SERVICES_NAME_LENGTH);
    break;

  }
  return bRosterOK;
}
