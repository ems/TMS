//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>
//
//  Block/Bus Assignment Sheet
//
//  From QETXT.INI:
//
//
//  [TMSRPT45]
//  FILE=TMSRPT45.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Date,VARCHAR,64,0,64,0,
//  FIELD3=RunNumber,NUMERIC,5,0,5,0,
//  FIELD4=BlockNumber,NUMERIC,5,0,5,0,
//  FIELD5=Origin,VARCHAR,8,0,8,0,
//  FIELD6=TimeOfReport,VARCHAR,8,0,8,0,
//  FIELD7=OutTime,VARCHAR,8,0,8,0,
//  FIELD8=OffTime,VARCHAR,8,0,8,0,
//  FIELD9=Destination,VARCHAR,8,0,8,0,
//  FIELD10=TotalHours,VARCHAR,8,0,8,0,
//  FIELD11=TotalMileage,NUMERIC,7,2,7,0,
//

#define TMSRPT45_FLAGS_PULLOUT 1

typedef struct RPT45DataStruct
{
  long  RUNSrecordID;
  long  runNumber;
  long  pieceNumber;
  long  TRIPSrecordID;
  long  blockNumber;
  long  reportTime;
  long  startNode;
  long  startTime;
  long  endNode;
  long  endTime;
  long  blockLength;
  float blockDistance;
  long  flags;
} RPT45DataDef;


int sort_TMSRPT45byReportTime(const void *a, const void *b)
{
  RPT45DataDef *pa, *pb;

  pa = (RPT45DataDef *)a;
  pb = (RPT45DataDef *)b;
  
  return(pa->reportTime  < pb->reportTime  ? -1 : pa->reportTime  > pb->reportTime  ? 1 :
         pa->blockNumber < pb->blockNumber ? -1 : pa->blockNumber > pb->blockNumber ? 1 : 0);
}

int sort_TMSRPT45byEndTime(const void *a, const void *b)
{
  RPT45DataDef *pa, *pb;

  pa = (RPT45DataDef *)a;
  pb = (RPT45DataDef *)b;
  
  return(pa->endTime     < pb->endTime     ? -1 : pa->endTime     > pb->endTime     ? 1 :
         pa->blockNumber < pb->blockNumber ? -1 : pa->blockNumber > pb->blockNumber ? 1 : 0);
}

BOOL FAR TMSRPT45(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  GetConnectionTimeDef GCTLocal;
  DISPATCHSHEETPassedDataDef RParamData;
  RPT45DataDef  RPT45Data[MAXRUNSINRUNLIST];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef  COST;
  HFILE hfOutputFile;
  float distance;
  char  outputString[TEMPSTRING_LENGTH];
  char *pszReportName;
  long  totalPlat;
  long  totalLay;
  long  totalILDhd;
  long  totalGADhd;
  long  assignedToNODESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  blockNumber;
  long  seq;
  long  dhd;
  long  equivalentTravelTime;
  int   resIDs[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011,
                                   TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int   numRunPieces;
  int   nI, nJ, nK;
  int   index;
  int   numPieces;
  int   rcode2;
//
//  See what he wants to unload
//
  RParamData.flags = DISPATCHSHEET_FLAG_DISABLEOPTIONS | DISPATCHSHEET_FLAG_SORTBY;
  RParamData.nReportNumber = 44;
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
  pPassedData->nReportNumber = 44;
  pPassedData->numDataFiles = 1;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
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
      if(!CISbuild(FALSE, TRUE))
      {
        goto deallocate;
      }
    }
    else
    {
      long serviceRecordID;

      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        serviceRecordID = SERVICES.recordID;
        InitTravelMatrix(serviceRecordID, TRUE);
        SERVICESKey0.recordID = serviceRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
    }
  }
//
//  Open the output file
//
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT45.txt");
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
//  Now go through the runs to add any unassigned runs to RPT45Data
//
  StatusBarText("Setting up unassigned runs...");
  numRunPieces = 0;
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
    TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//    if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
    if(TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID)
    {
      RPT45Data[numRunPieces].RUNSrecordID = RUNS.recordID;
      RPT45Data[numRunPieces].runNumber = RUNS.runNumber;
      RPT45Data[numRunPieces].pieceNumber = RUNS.pieceNumber;
      RPT45Data[numRunPieces].TRIPSrecordID = RUNS.start.TRIPSrecordID;
      RPT45Data[numRunPieces].blockNumber = TRIPS.standard.blockNumber; 
      RPT45Data[numRunPieces].flags = 0;
      numRunPieces++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Generate the block details
//
  StatusBarText("Generating block details...");
  seq = 0;
  m_bEstablishRUNTIMES = TRUE;
  for(nI = 0; nI < numRunPieces; nI++)
  {
    StatusBar((long)(nI + 1), (long)numRunPieces);
    if(StatusBarAbort())
    {
      goto deallocate;
    }
//
//  Fill in the rest of RPT45Data
//
    if(RPT45Data[nI].pieceNumber == 1)
    {
      RUNSKey0.recordID = RPT45Data[nI].RUNSrecordID;
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
      for(nJ = 0; nJ < numPieces; nJ++)
      {
        if(nJ == 0)
        {
          index = nI;
        }
        else
        {
          for(index = NO_RECORD, nK = nI + 1; nK < numRunPieces; nK++)
          {
            if(RPT45Data[nK].runNumber != RPT45Data[nI].runNumber)
            {
              break;
            }
            if(RPT45Data[nK].pieceNumber == nJ + 1)
            {
              index = nK;
              break;
            }
          }
        }
        if(index == NO_RECORD)
        {
          continue;
        }
        RPT45Data[index].startNode = RUNSVIEW[nJ].runOnNODESrecordID;
        RPT45Data[index].startTime = RUNSVIEW[nJ].runOnTime;
        RPT45Data[index].endNode = RUNSVIEW[nJ].runOffNODESrecordID;
        RPT45Data[index].endTime = RUNSVIEW[nJ].runOffTime;
        RPT45Data[index].reportTime = RUNSVIEW[nJ].runOnTime -
              COST.TRAVEL[nJ].startTravelTime - COST.PIECECOST[nJ].reportTime;
//
//  Get the first trip on the block
//
        TRIPSKey0.recordID = RPT45Data[index].TRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
        RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        SGRPSERVICESrecordID  = TRIPS.standard.SGRPSERVICESrecordID;
        blockNumber = TRIPS.standard.blockNumber;
//
//  Loop through the block
//
        totalPlat = 0;
        totalLay = 0;
        totalILDhd = 0;
        totalGADhd = 0;
        GCTLocal.fromNODESrecordID = NO_RECORD;
        GCTLocal.timeOfDay = NO_TIME;
        RPT45Data[index].blockDistance = 0.0;
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = RGRPROUTESrecordID;
        TRIPSKey2.SGRPSERVICESrecordID = SGRPSERVICESrecordID;
        TRIPSKey2.blockNumber = blockNumber;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        while(rcode2 == 0 &&
              TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
              TRIPS.standard.RGRPROUTESrecordID == RGRPROUTESrecordID &&
              TRIPS.standard.SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
              TRIPS.standard.blockNumber == blockNumber)
        {
          if(StatusBarAbort())
          {
            goto deallocate;
          }
//
//  Gen the whole trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Distance
//
          RPT45Data[index].blockDistance += GTResults.tripDistance;
//
//  Total platform
//
          totalPlat += (GTResults.lastNodeTime - GTResults.firstNodeTime);
//
//  Garage deadheads
//
//  Pullout
//
          if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
          {
            RPT45Data[index].flags = TMSRPT45_FLAGS_PULLOUT;
            RPT45Data[index].startNode = TRIPS.standard.POGNODESrecordID;
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd != NO_TIME)
            {
              RPT45Data[index].startTime = GTResults.firstNodeTime - dhd;
              totalGADhd += dhd;
            }
          }
//
//  Pullin
//
          if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
          {
            RPT45Data[index].endNode = TRIPS.standard.PIGNODESrecordID;
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd != NO_TIME)
            {
              RPT45Data[index].endTime = GTResults.lastNodeTime + dhd;
              totalGADhd += dhd;
            }
          }
//
//  Interline deadheads
//
          dhd = 0;
          if(GCTLocal.fromNODESrecordID != NO_RECORD)
          {
            GCTLocal.toROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTLocal.toSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTLocal.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTLocal.toNODESrecordID = GTResults.firstNODESrecordID;
            if(!NodesEquivalent(GCTLocal.fromNODESrecordID,
                  GCTLocal.toNODESrecordID, &equivalentTravelTime))
            {
              memcpy(&GCTData, &GCTLocal, sizeof(GetConnectionTimeDef));
              dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(dhd == NO_TIME)
              {
                dhd = 0;
              }
              totalILDhd += dhd;
            }
          }
//
//  Layover
//
          totalLay += (GCTLocal.timeOfDay == NO_TIME ?
                0 : GTResults.firstNodeTime - GCTLocal.timeOfDay - dhd);
//
//  Set up for next trip through
//
          GCTLocal.fromROUTESrecordID = TRIPS.ROUTESrecordID;
          GCTLocal.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
          GCTLocal.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTLocal.fromNODESrecordID = GTResults.lastNODESrecordID;
          GCTLocal.timeOfDay = GTResults.lastNodeTime;
//
//  And get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        }
//
//  Block length
//
        RPT45Data[index].blockLength = totalPlat + totalLay + totalILDhd + totalGADhd;
      }  // nJ
    }  // if piece == 1
  }  // nI
//
//  Sort by report time or off time
//
  if(RParamData.flags & DISPATCHSHEET_FLAG_REPORTTIME)
    qsort((void *)RPT45Data, numRunPieces, sizeof(RPT45DataDef), sort_TMSRPT45byReportTime);
  else
    qsort((void *)RPT45Data, numRunPieces, sizeof(RPT45DataDef), sort_TMSRPT45byEndTime);
//
//  Set up the output string
//
  for(nI = 0; nI < numRunPieces; nI++)
  {
//
//  Report mod: 15-Dec-04 - Don't list entries that don't pull out
//
    if(!(RPT45Data[nI].flags & TMSRPT45_FLAGS_PULLOUT))
    {
      continue;
    }
//
//  Sequence, Service Day, Run number, Block number
//
    sprintf(outputString, "%ld\t%s\t%ld\t%ld\t", seq++, RParamData.szDate, RPT45Data[nI].runNumber, RPT45Data[nI].blockNumber);
//
//  Origin
//
    NODESKey0.recordID = RPT45Data[nI].startNode;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Time of report
//
    strcat(outputString, Tchar(RPT45Data[nI].reportTime));
    strcat(outputString, "\t");
//
//  Out time
//
    strcat(outputString, Tchar(RPT45Data[nI].startTime));
    strcat(outputString, "\t");
//
//  Off time
//
    strcat(outputString, Tchar(RPT45Data[nI].endTime));
    strcat(outputString, "\t");
//
//  Destination
//
    NODESKey0.recordID = RPT45Data[nI].endNode;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Total hours
//
    strcat(outputString, chhmm(RPT45Data[nI].blockLength));
    strcat(outputString, "\t");
//
//  Total mileage
//
    sprintf(tempString, "%8.2f\r\n", RPT45Data[nI].blockDistance);
    strcat(outputString, tempString);
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
