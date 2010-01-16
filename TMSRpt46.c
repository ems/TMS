//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
//
//  Relief Car Assignment Sheet
//
//  From QETXT.INI:
//
//
//  [TMSRPT46]
//  FILE=TMSRPT46.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Date,VARCHAR,64,0,64,0,
//  FIELD3=RunNumber,NUMERIC,5,0,5,0,
//  FIELD4=PieceNumber,NUMERIC,5,0,5,0,
//  FIELD5=BlockNumber,NUMERIC,5,0,5,0,
//  FIELD6=LeaveLocation,VARCHAR,8,0,8,0,
//  FIELD7=LeaveTime,VARCHAR,8,0,8,0,
//  FIELD8=ArriveLocation,VARCHAR,8,0,8,0,
//  FIELD9=ArriveTime,VARCHAR,8,0,8,0,
//  FIELD10=DepartTime,VARCHAR,8,0,8,0,
//  FIELD11=ReturnLocation,VARCHAR,8,0,8,0,
//  FIELD12=ReturnTime,VARCHAR,8,0,8,0,
//
typedef struct RPT46DataStruct
{
  long  runNumber;
  long  pieceNumber;
  long  TRIPSrecordID;
  long  blockNumber;
  long  leaveNODESrecordID;
  long  leaveTime;
  long  arriveNODESrecordID;
  long  arriveTime;
  long  departTime;
  long  returnNODESrecordID;
  long  returnTime;
} RPT46DataDef;

int sort_TMSRPT46byReportTime(const void *a, const void *b)
{
  RPT46DataDef *pa, *pb;

  pa = (RPT46DataDef *)a;
  pb = (RPT46DataDef *)b;
  
  return(pa->leaveTime < pb->leaveTime ? -1 : pa->leaveTime > pb->leaveTime ? 1 : 0);
}

int sort_TMSRPT46byEndTime(const void *a, const void *b)
{
  RPT46DataDef *pa, *pb;

  pa = (RPT46DataDef *)a;
  pb = (RPT46DataDef *)b;
  
  return(pa->returnTime < pb->returnTime ? -1 : pa->returnTime > pb->returnTime ? 1 : 0);
}

//
//  Relief Car Assignment Sheet
//

BOOL FAR TMSRPT46(TMSRPTPassedDataDef *pPassedData)
{
  DISPATCHSHEETPassedDataDef RParamData;
  RPT46DataDef  RPT46Data[MAXRUNSINRUNLIST];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef  COST;
  HFILE hfOutputFile;
  BOOL  bFound;
  char  outputString[TEMPSTRING_LENGTH];
  char  *pszReportName;
  long  seq;
  long  arriveNODESrecordID;
  long  firstPceRUNSrecordIDs[MAXRUNSINRUNLIST];
  long  closestReturn;
  long  thisReturn;
  long  pieceNumber;
  int   resIDs[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011,
                                   TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int   numRuns;
  int   numPieces;
  int   numFirstPieces;
  int   nI, nJ, nK;
  int   rcode2;
//
//  See what he wants to unload
//
  RParamData.flags = DISPATCHSHEET_FLAG_DISABLEOPTIONS | DISPATCHSHEET_FLAG_SORTBY |
                     DISPATCHSHEET_FLAG_INCLUDEGARAGEANDRELIEFS;
  RParamData.nReportNumber = 45;
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
  pPassedData->nReportNumber = 45;
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
//  Build the travel matrix
//
  StatusBarEnd();
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuildService(FALSE, TRUE, ROSTERPARMS.serviceDays[RParamData.dayOfWeek]))
      {
        goto deallocate;
      }
    }
    else
    {
      InitTravelMatrix(ROSTERPARMS.serviceDays[RParamData.dayOfWeek], TRUE);
    }
  }
//
//  Open the output file
//
  StatusBarStart(hWndMain, pszReportName);
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT46.txt");
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
//  Cycle through the runs
//
//  Record the recordID if this is the first piece of a run
//
  StatusBarText("Cycling through the runs...");
  RUNSKey1.DIVISIONSrecordID = RParamData.DIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[RParamData.dayOfWeek];
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  numFirstPieces = 0;
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == RParamData.DIVISIONSrecordID &&
        RUNS.SERVICESrecordID == ROSTERPARMS.serviceDays[RParamData.dayOfWeek])
  {
    if(StatusBarAbort())
      goto deallocate;
    if(RUNS.pieceNumber == 1)
    {
      firstPceRUNSrecordIDs[numFirstPieces] = RUNS.recordID;
      numFirstPieces++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Go through all the first pieces
//
  numRuns = 0;
  for(nI = 0; nI < numFirstPieces; nI++)
  {
    if(StatusBarAbort())
      goto deallocate;
    StatusBar((long)(nI + 1), (long)numFirstPieces);
//
//  Set up for and then call GetRunElements
//
    RUNSKey0.recordID = firstPceRUNSrecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
//
//  Cycle through the pieces.  If there's no travel on the front, skip through
//
    for(pieceNumber = 0; pieceNumber < numPieces; pieceNumber++)
    {
      if(COST.TRAVEL[pieceNumber].startTravelTime <= 0)
      {
        continue;
      }
//
//  Is he starting his piece at one of our selected relief points?
//
      for(bFound = FALSE, nJ = 0; nJ < RParamData.numReliefPoints; nJ++)
      {
//        if(RUNS.start.NODESrecordID == RParamData.reliefPointNODESrecordIDs[nJ])
        if(RUNSVIEW[pieceNumber].runOnNODESrecordID == RParamData.reliefPointNODESrecordIDs[nJ])
        {
          arriveNODESrecordID = RUNS.start.NODESrecordID;
          bFound = TRUE;
          break;
        }
      }
//
//  Yes, he is - now how do we get him there?
//  (He has to be starting from one of our selected garages)
//
      if(bFound)
      {
//
//  Loop through the garages to see if this is one that we want
//
        for(bFound = FALSE, nJ = 0; nJ < RParamData.numGarages; nJ++)
        {
          if(COST.TRAVEL[pieceNumber].startNODESrecordID == RParamData.garageNODESrecordIDs[nJ])
          {
            bFound = TRUE;
            break;
          }
        }
//
//  Yes it is.  Set up what we can from this run.
//
        if(bFound)
        {
          RPT46Data[numRuns].runNumber = RUNSVIEW[pieceNumber].runNumber;
          RPT46Data[numRuns].pieceNumber = pieceNumber ;
          RPT46Data[numRuns].TRIPSrecordID = RUNSVIEW[pieceNumber].runOnTRIPSrecordID;
          RPT46Data[numRuns].blockNumber = RUNSVIEW[pieceNumber].blockNumber; 
          RPT46Data[numRuns].leaveNODESrecordID = COST.TRAVEL[pieceNumber].startNODESrecordID;
          RPT46Data[numRuns].leaveTime =  RUNSVIEW[pieceNumber].runOnTime -
                COST.TRAVEL[pieceNumber].startTravelTime - COST.PIECECOST[pieceNumber].reportTime;
          RPT46Data[numRuns].arriveNODESrecordID = arriveNODESrecordID;
          RPT46Data[numRuns].arriveTime = RUNSVIEW[pieceNumber].runOnTime;
          RPT46Data[numRuns].departTime = NO_TIME;
          RPT46Data[numRuns].returnNODESrecordID = NO_RECORD;
          RPT46Data[numRuns].returnTime = NO_TIME;
//
//  Now see who's returning to the yard in the car we brought out
//
          closestReturn = 3600;  // Assume an hour into the future
          for(nJ = 0; nJ < numFirstPieces; nJ++)
          {
            RUNSKey0.recordID = firstPceRUNSrecordIDs[nJ];
            btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
            for(nK = 0; nK < numPieces; nK++)
            {
              thisReturn = RUNSVIEW[nK].runOffTime - RPT46Data[numRuns].arriveTime;
              if(COST.TRAVEL[nK].endBPointNODESrecordID == arriveNODESrecordID &&
                    COST.TRAVEL[nK].endNODESrecordID == RPT46Data[numRuns].leaveNODESrecordID &&
                    COST.TRAVEL[nK].endTravelTime > 0 &&
                    thisReturn >= 0 && thisReturn < closestReturn)
              {
                closestReturn = RUNSVIEW[nK].runOffTime - RPT46Data[numRuns].leaveTime;
                RPT46Data[numRuns].departTime =
                      COST.TRAVEL[nK].endBPointTime + COST.TRAVEL[nK].endDwellTime;
                RPT46Data[numRuns].returnNODESrecordID = COST.TRAVEL[nK].endNODESrecordID;
                RPT46Data[numRuns].returnTime =
                      RPT46Data[numRuns].departTime + COST.TRAVEL[nK].endTravelTime;
              }
            }
          }
          numRuns++;
        }  // found a garage
      }  // found a relief point
    }  // piece number
  }  // nI
//
//  Sort by report time or off time
//
  if(RParamData.flags & DISPATCHSHEET_FLAG_REPORTTIME)
  {
    qsort((void *)RPT46Data, numRuns, sizeof(RPT46DataDef), sort_TMSRPT46byReportTime);
  }
  else
  {
    qsort((void *)RPT46Data, numRuns, sizeof(RPT46DataDef), sort_TMSRPT46byEndTime);
  }
//
//  Set up the output string
//
  seq = 0;
  for(nI = 0; nI < numRuns; nI++)
  {
//
//  Don't bother with walking
//
    if(RPT46Data[nI].leaveTime == RPT46Data[nI].arriveTime)
    {
      continue;
    }
//
//  Sequence, Service Day, Run number, Piece number, Block number
//
    sprintf(outputString, "%ld\t%s\t%ld\t%ld\t%ld\t",
          seq++, RParamData.szDate, RPT46Data[nI].runNumber,
          RPT46Data[nI].pieceNumber + 1, RPT46Data[nI].blockNumber);
//
//  Leave node
//
    NODESKey0.recordID = RPT46Data[nI].leaveNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Leave time
//
    strcat(outputString, Tchar(RPT46Data[nI].leaveTime));
    strcat(outputString, "\t");
//
//  Arrive node
//
    NODESKey0.recordID = RPT46Data[nI].arriveNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Arrive time
//
    strcat(outputString, Tchar(RPT46Data[nI].arriveTime));
    strcat(outputString, "\t");
//
//  Depart time
//
    strcat(outputString, Tchar(RPT46Data[nI].departTime));
    strcat(outputString, "\t");
//
//  Return node
//
    NODESKey0.recordID = RPT46Data[nI].returnNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Return time
//
    strcat(outputString, Tchar(RPT46Data[nI].returnTime));
    strcat(outputString, "\t");
//
    strcat(outputString, "\r\n");
//
//  Write it out
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
  }

//
//  All done
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);

  return(TRUE);
}
