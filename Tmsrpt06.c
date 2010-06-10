//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT06() - Block Listing
//
#include "TMSHeader.h"
#include <math.h>
//              1     2        3     4      5        6    7    8     9      10     11  12   13   14    15   16   17    18        19    20      21       22       23
#define HEADER "Blk#\tBustype\tRGRP\tSGRP\tRoute\tService\tPOG\tPOT\tFrom\tFTime\tTTime\tTo\tPIG\tPIT\tTime\tLay\tDhd\tDist\tCum.Dist\tGarDhd\tGarDist\tDhdDist\tSeq\n"
#define NUMCOLS 23

BOOL FAR TMSRPT06(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults[2];  
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  BLOCKSDef *pTRIPSChunk, *pLocalTRIPSChunk;
  float distance;
  float cumDist;
  float PODistance, PIDistance;
  float deadheadDist;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bFirst;
  BOOL  bFoundStandard;
  BOOL  bFoundDropback;
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  long  assignedToNODESrecordID;
  long  equivalentTravelTime;
  long  statbarTotal;
  long  layoverTime;
  long  deadheadTime;
  long  GarageDhdTime;
  long  POTime;
  long  PITime;
  long  previousBlock;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   seqNum;
  int   keyNumber;
//
//  Local Database declarations
//
  ROUTESDef      LocalROUTES;
  TMSKey0Def     LocalROUTESKey0;
  ORDEREDLISTDef LocalSERVICES;
  TMSKey0Def     LocalSERVICESKey0;
  TRIPSDef       LocalTRIPS;

  pPassedData->nReportNumber = 5;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_COMMENTS;
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
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt06.txt");
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
//
//  Loop through the routes and services to see if there
//  are both standard and dropback blocks.  If there are
//  both, then prompt him as to which he wants.
//
  bFoundStandard = FALSE;
  bFoundDropback = FALSE;
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    if(bFoundStandard && bFoundDropback)
      break;
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(bFoundStandard && bFoundDropback)
        break;
      for(keyNumber = 2; keyNumber <= 3; keyNumber++)
      {
        if(keyNumber == 2)
        {
          if(bFoundStandard)
            continue;
          pTRIPSChunk = &TRIPS.standard;
        }
        else
        {
          if(bFoundDropback)
            continue;
          pTRIPSChunk = &TRIPS.dropback;
        }
        NODESKey1.flags = NODES_FLAG_GARAGE;
        memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
        if(rcode2 != 0)  // No Garages
        {
          TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
          bKeepGoing = FALSE;
          goto deallocate;
        }
        bFirst = TRUE;
        while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
        {
          assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
          TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
          TRIPSKey2.RGRPROUTESrecordID = REPORTPARMS.pRouteList[nI];
          TRIPSKey2.SGRPSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          TRIPSKey2.blockNumber = 1;
          TRIPSKey2.blockSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
          if(rcode2 == 0 &&
                pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                pTRIPSChunk->RGRPROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                pTRIPSChunk->SGRPSERVICESrecordID == REPORTPARMS.pServiceList[nJ])
          {
            if(keyNumber == 2)
              bFoundStandard = TRUE;
            else
              bFoundDropback = TRUE;
            break;
          }
          if(bFirst)
             bFirst = FALSE;
          else
             rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
        }
      }
    }
  }
//
//  Tell him what he's got
//
  if(bFoundStandard && bFoundDropback)
  {
    LoadString(hInst, TEXT_243, tempString, TEMPSTRING_LENGTH);
    if(MessageBox(NULL, tempString, TMS, MB_YESNO) == IDYES)
      bFoundDropback = FALSE;
    else
    {
      bFoundStandard = FALSE;
      LoadString(hInst, TEXT_244, tempString, TEMPSTRING_LENGTH);
      if(MessageBox(NULL, tempString, TMS, MB_YESNO) == IDYES)
        bFoundStandard = FALSE;
    }
  }
//
//  Ok, he's made the choice
//
  if(bFoundStandard)
  {
    keyNumber = 2;
    pTRIPSChunk = &TRIPS.standard;
    pLocalTRIPSChunk = &LocalTRIPS.standard;
  }
  else if(bFoundDropback)
  {
    keyNumber = 3;
    pTRIPSChunk = &TRIPS.dropback;
    pLocalTRIPSChunk = &LocalTRIPS.dropback;
  }
  else
  {
    bKeepGoing = FALSE;
    goto deallocate;
  }
//
//  Loop through the routes
//
  seqNum = 0;
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
      continue;
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Loop through the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      if(rcode2 != 0)
        continue;
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
      LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
      sprintf(tempString, szarString, routeNumber, serviceName);
      StatusBarText(tempString);
      StatusBar((long)(nI * REPORTPARMS.numServices + nJ), (long)statbarTotal);
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      m_bEstablishRUNTIMES = TRUE;
//
//  Get all the trips
//
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      if(rcode2 != 0)  // No Garages
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
        bKeepGoing = FALSE;
        goto deallocate;
      }
      bFirst = TRUE;
      while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
      {
        assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = REPORTPARMS.pRouteList[nI];
        TRIPSKey2.SGRPSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        TRIPSKey2.blockNumber = 1;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        previousBlock = NO_RECORD;
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              pTRIPSChunk->SGRPSERVICESrecordID == REPORTPARMS.pServiceList[nJ])
        {
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
          if(previousBlock != pTRIPSChunk->blockNumber)
            cumDist = (float)0.0;
          previousBlock = pTRIPSChunk->blockNumber;
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults[0]);
//
//  Set up the output string
//
          strcpy(tempString, "");
          for(nK = 0; nK < NUMCOLS; nK++)
          {
            switch(nK)
            {
//
//  Block number
//
              case 0:
                sprintf(szarString, "%ld", pTRIPSChunk->blockNumber);
                break;
//
//  Bustype
//
              case 1:
                if(TRIPS.BUSTYPESrecordID == NO_RECORD)
                  strcpy(szarString, "");
                else
                {
                  BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
                  strncpy(szarString, BUSTYPES.name, sizeof(BUSTYPES.name));
                  szarString[sizeof(BUSTYPES.name)] = '\0';
                }
                break;
//
//  RGRP
//
              case 2:
                LocalROUTESKey0.recordID = pTRIPSChunk->RGRPROUTESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &LocalROUTES, &LocalROUTESKey0, 0);
                strncpy(szarString, LocalROUTES.number, sizeof(LocalROUTES.number));
                szarString[sizeof(LocalROUTES.number)] = '\0';
                break;
//
//  SGRP
//
              case 3:
                LocalSERVICESKey0.recordID = pTRIPSChunk->SGRPSERVICESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &LocalSERVICES, &LocalSERVICESKey0, 0);
                strncpy(szarString, LocalSERVICES.name, sizeof(LocalSERVICES.name));
                szarString[sizeof(LocalSERVICES.name)] = '\0';
                break;
//
//  Route number
//
              case 4:
                LocalROUTESKey0.recordID = TRIPS.ROUTESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &LocalROUTES, &LocalROUTESKey0, 0);
                strncpy(szarString, LocalROUTES.number, sizeof(LocalROUTES.number));
                szarString[sizeof(LocalROUTES.number)] = '\0';
                break;
//
//  Service
//
              case 5:
                LocalSERVICESKey0.recordID = TRIPS.SERVICESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &LocalSERVICES, &LocalSERVICESKey0, 0);
                strncpy(szarString, LocalSERVICES.name, sizeof(LocalSERVICES.name));
                szarString[sizeof(LocalSERVICES.name)] = '\0';
                break;
//
//  POG
//
              case 6:
                if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
                  strcpy(szarString, "");
                else
                {
                  NODESKey0.recordID = pTRIPSChunk->POGNODESrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                  strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  szarString[NODES_ABBRNAME_LENGTH] = '\0';
                }
                break;
//
//  POT
//
              case 7:
                if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
                {
                  strcpy(szarString, "");
                  PODistance = (float)0.0;
                  POTime = 0L;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = REPORTPARMS.pRouteList[nI];
                  GCTData.fromSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                  GCTData.toROUTESrecordID = REPORTPARMS.pRouteList[nI];
                  GCTData.toSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                  GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
                  GCTData.toNODESrecordID = GTResults[0].firstNODESrecordID;
                  GCTData.timeOfDay = GTResults[0].firstNodeTime;
                  POTime = GTResults[0].firstNodeTime -
                        GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  PODistance = distance;
                  strcpy(szarString, Tchar(POTime));
                }
                break;
//
//  From node
//
              case 8:
                NODESKey0.recordID = GTResults[0].firstNODESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                szarString[NODES_ABBRNAME_LENGTH] = '\0';
                break;
//
//  From time
//
              case 9:
                strcpy(szarString, Tchar(GTResults[0].firstNodeTime));
                GarageDhdTime = pTRIPSChunk->POGNODESrecordID != NO_RECORD
                      ? GTResults[0].firstNodeTime - POTime : 0;
                break;
//
//  To time
//
              case 10:
                strcpy(szarString, Tchar(GTResults[0].lastNodeTime));
                break;
//
//  To node
//
              case 11:
                NODESKey0.recordID = GTResults[0].lastNODESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                szarString[NODES_ABBRNAME_LENGTH] = '\0';
                break;
//
//  PIG
//
              case 12:
                if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
                  strcpy(szarString, "");
                else
                {
                  NODESKey0.recordID = pTRIPSChunk->PIGNODESrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                  strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  szarString[NODES_ABBRNAME_LENGTH] = '\0';
                }
                break;
//
//  PIT
//
              case 13:
                if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
                {
                  strcpy(szarString, "");
                  PIDistance = (float)0.0;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = REPORTPARMS.pRouteList[nI];
                  GCTData.fromSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                  GCTData.toROUTESrecordID = REPORTPARMS.pRouteList[nI];
                  GCTData.toSERVICESrecordID = REPORTPARMS.pServiceList[nJ];
                  GCTData.fromNODESrecordID = GTResults[0].lastNODESrecordID;
                  GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
                  GCTData.timeOfDay = GTResults[0].lastNodeTime;
                  PITime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  PIDistance = distance;
                  strcpy(szarString, Tchar(GTResults[0].lastNodeTime + PITime));
                  GarageDhdTime += PITime;
                }
                break;
//
//  Trip time (in seconds)
//
              case 14:
                sprintf(szarString, "%ld", GTResults[0].lastNodeTime - GTResults[0].firstNodeTime);
                break;
//
//  Layover and deadhead
//
              case 15:
                rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &LocalTRIPS, &TRIPSKey2, keyNumber);
                layoverTime = 0L;
                deadheadTime = 0L;
                deadheadDist = (float)0.0;
                if(rcode2 == 0 &&
                      pLocalTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                      pLocalTRIPSChunk->RGRPROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                      pLocalTRIPSChunk->SGRPSERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                      pLocalTRIPSChunk->blockNumber == pTRIPSChunk->blockNumber)
                {
                  GenerateTrip(LocalTRIPS.ROUTESrecordID, LocalTRIPS.SERVICESrecordID,
                        LocalTRIPS.directionIndex, LocalTRIPS.PATTERNNAMESrecordID,
                        LocalTRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults[1]);
                  if(!NodesEquivalent(GTResults[1].firstNODESrecordID,
                        GTResults[0].lastNODESrecordID, &equivalentTravelTime))
                  {
                    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                    GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                    GCTData.toROUTESrecordID = LocalTRIPS.ROUTESrecordID;
                    GCTData.toSERVICESrecordID = LocalTRIPS.SERVICESrecordID;
                    GCTData.fromNODESrecordID = GTResults[0].lastNODESrecordID;
                    GCTData.toNODESrecordID = GTResults[1].firstNODESrecordID;
                    GCTData.timeOfDay = GTResults[0].lastNodeTime;
                    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                    deadheadDist = deadheadTime == NO_TIME ? (float)0.0 : (float)fabs((double)distance);
                    deadheadTime = deadheadTime == NO_TIME ? 0L : deadheadTime;
                  }
                  layoverTime = GTResults[1].firstNodeTime -
                        GTResults[0].lastNodeTime - deadheadTime;
                }
                sprintf(szarString, "%ld\t%ld", layoverTime, deadheadTime);
                break;
//
//  Deadhead
//
              case 16:
                break;
//
//  Distance
//
              case 17:
                sprintf(szarString, "%7.2f", GTResults[0].tripDistance);
                cumDist += GTResults[0].tripDistance;
                break;
//
//  Cumulative distance
//
              case 18:
                sprintf(szarString, "%7.2f", cumDist);
                break;
//
//  Garage deadhead time
//
              case 19:
                sprintf(szarString, "%ld", GarageDhdTime);
                break;
//
//  Garage Distance
//
              case 20:
                sprintf(szarString, "%7.2f", PODistance + PIDistance); 
                break;
//
//  Deadhead distance
//
              case 21:
                sprintf(szarString, "%7.2f", deadheadDist); 
                break;
//
//  Block Sequence
//
              case 22:
                sprintf(szarString, "%d", seqNum++);
                break;
            }  // End of switch
            if(nK != 16)  // Layover and deadhead are combined
            {
              strcat(tempString, szarString);
              strcat(tempString, nK == NUMCOLS - 1 ? "\r\n" : "\t");
            }
          }  // nK
          _lwrite(hfOutputFile, tempString, strlen(tempString));
          memcpy(&TRIPS, &LocalTRIPS, sizeof(TRIPSDef));
        }  // while
        if(bFirst)
           bFirst = FALSE;
        else
           rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    _lclose(hfOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}


