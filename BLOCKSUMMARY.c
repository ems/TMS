//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

BOOL CALLBACK BLOCKSUMMARYMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static HANDLE hCtlTHISRGRP;
  static HANDLE hCtlALLRGRPS;
  static HANDLE hCtlALL;
  static HANDLE hCtlSPECIFICBLOCK;
  static HANDLE hCtlBLOCKS;
  static char   szWindowTitle[64];
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  GetConnectionTimeDef GCTLocal;
  HCURSOR hSaveCursor;
  float distance;
  long  totalPlat  = 0;
  long  totalLay   = 0;
  long  totalILDhd = 0;
  long  totalGADhd = 0;
  long  blockNumber;
  long  assignedToNODESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  dhd;
  long  equivalentTravelTime;
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  BOOL  bFirst;
  int   numTrips = 0;
  int   numBlocks;
  int   rcode2;
  int   nI;
  int   nIStart;
  int   nIEnd;
  short int wmId;
  short int wmEvent;
  static BLOCKSDef *pTRIPSChunk;
  static int keyNumber;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get the selected trip data from lParam
//
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      keyNumber = pDI->fileInfo.keyNumber;
      pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
//
//  Set up handles to some of the controls
//
      hCtlALL = GetDlgItem(hWndDlg, BLOCKSUMMARY_ALL);
      hCtlSPECIFICBLOCK = GetDlgItem(hWndDlg, BLOCKSUMMARY_SPECIFICBLOCK);
      hCtlBLOCKS = GetDlgItem(hWndDlg, BLOCKSUMMARY_BLOCKS);
      hCtlTHISRGRP = GetDlgItem(hWndDlg, BLOCKSUMMARY_THISRGRP);
      hCtlALLRGRPS = GetDlgItem(hWndDlg, BLOCKSUMMARY_ALLRGRPS);
//
//  Default to this RGRP
//
      SendMessage(hCtlTHISRGRP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Get the blocks
//
      numBlocks = SetUpBlocksListbox(pDI, hCtlBLOCKS);
      if(numBlocks == NO_RECORD)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      else if(numBlocks == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_107, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Establish "All" as the selection
//
      SendMessage(hCtlALL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
//
//  Set the title
//
      ROUTESKey0.recordID = pDI->fileInfo.routeRecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 == 0)
      {
        strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
        strcat(routeNumberAndName, " - ");
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(routeNumberAndName, tempString);
        sprintf(szWindowTitle, "Block Summary: Route Group %s, %s", routeNumberAndName, m_ServiceName);
        SetWindowText(hWndDlg, szWindowTitle);
      }
      break; //  End of WM_INITDLG
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break; //  End of WM_CLOSE
//
//  WM_USERSETUP
//
    case WM_USERSETUP:
      if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
      {
        nIStart = 0;
        nIEnd = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        sprintf(tempString, "Total blocks: %d", nIEnd);
      }
      else
      {
        nIStart = SendMessage(hCtlBLOCKS, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        nIEnd = nIStart + 1;
        if(nIStart == LB_ERR)
          strcpy(tempString, "No selection");
        else
        {
          SendMessage(hCtlBLOCKS, LB_GETTEXT, (WPARAM)nIStart, (LONG)(LPSTR)szarString);
          sprintf(tempString, "Block %s", szarString);
        }
      }
      SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_BLOCKNUMBER,
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      totalPlat = 0;
      totalLay = 0;
      totalILDhd = 0;
      totalGADhd = 0;
      numTrips = 0;
      hSaveCursor = SetCursor(hCursorWait);
      m_numConnectionAlerts = 0;
//
//  Loop through
//
      for(nI = nIStart; nI < nIEnd; nI++)
      {
//
//  Get the trip data
//
        TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
        RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
        SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
        blockNumber = pTRIPSChunk->blockNumber;
//
//  Position to the first trip in the block
//
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = RGRPROUTESrecordID;
        TRIPSKey2.SGRPSERVICESrecordID = SGRPSERVICESrecordID;
        TRIPSKey2.blockNumber = blockNumber;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
//
//  Loop through the block
//
        GCTLocal.fromNODESrecordID = NO_RECORD;
        GCTLocal.timeOfDay = NO_TIME;
        m_bEstablishRUNTIMES = TRUE;
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == RGRPROUTESrecordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
              pTRIPSChunk->blockNumber == blockNumber)
        {
//
//  Number of trips
//
          numTrips++;
//
//  Gen the whole trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Total platform
//
          totalPlat += (GTResults.lastNodeTime - GTResults.firstNodeTime);
//
//  Garage deadheads
//
//  Pullout
//
          if(pTRIPSChunk->POGNODESrecordID != NO_RECORD)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            totalGADhd += (dhd > 0 ? dhd : 0);
          }
//
//  Pullin
//
          if(pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd == NO_TIME)
              dhd = 0;
            totalGADhd += dhd;
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
                dhd = 0;
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
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        }
//
//  Number of trips
//
        sprintf(tempString, "%d", numTrips);
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_NUMTRIPS,
              WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Platform time
//
        sprintf(tempString, "%s", chhmm(totalPlat));
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_PLATFORMTIME,
              WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Layover time
//
        sprintf(tempString, "%s", chhmm(totalLay));
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_LAYOVERTIME,
              WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Interline Deadhead time
//
        sprintf(tempString, "%s", chhmm(totalILDhd));
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_DEADHEADTIME, 
              WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Garage Deadhead time
//
        sprintf(tempString, "%s", chhmm(totalGADhd));
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_GARDEADHEADTIME,
              WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Block length
//
        sprintf(tempString, "%s", chhmm(totalPlat + totalLay + totalILDhd + totalGADhd));
        SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_BLOCKLENGTH, 
             WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }  // nI
//
//  Done
//
      SetCursor(hSaveCursor);
      break;  // End of WM_USERSETUP
//
//  WM_USERSETUP2
//
    case WM_USERSETUP2:
      totalPlat = 0;
      totalLay = 0;
      totalILDhd = 0;
      totalGADhd = 0;
      numTrips = 0;
      numBlocks = 0;
      hSaveCursor = SetCursor(hCursorWait);
      m_numConnectionAlerts = 0;
//
//  Cycle through all the routes
//
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      while(rcode2 == 0)
      {
//
//  Get all the trips on this route group
//
        m_bEstablishRUNTIMES = TRUE;
        NODESKey1.flags = NODES_FLAG_GARAGE;
        memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
        if(rcode2 != 0)  // No Garages
        {
          TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
          goto done;
        }
        bFirst = TRUE;
        while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
        {
//
//  Cycle through the blocks
//
          assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
          TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
          TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
          TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
          TRIPSKey2.blockNumber = 1;
          TRIPSKey2.blockSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          blockNumber = pTRIPSChunk->blockNumber;
//
//  Loop through the block
//
          GCTLocal.fromNODESrecordID = NO_RECORD;
          GCTLocal.timeOfDay = NO_TIME;
          m_bEstablishRUNTIMES = TRUE;
          while(rcode2 == 0 &&
                pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
                pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                pTRIPSChunk->blockNumber == blockNumber)
          {
            numBlocks++;
            GCTLocal.fromNODESrecordID = NO_RECORD;
            GCTLocal.timeOfDay = NO_TIME;
            while(rcode2 == 0 &&
                  pTRIPSChunk->blockNumber == blockNumber)
            {
//
//  Number of trips
//
              numTrips++;
//
//  Gen the whole trip
//
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Total platform
//
              totalPlat += (GTResults.lastNodeTime - GTResults.firstNodeTime);
//
//  Garage deadheads
//
//  Pullout
//
              if(pTRIPSChunk->POGNODESrecordID != NO_RECORD)
              {
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
                GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                GCTData.timeOfDay = GTResults.firstNodeTime;
                dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
               totalGADhd += (dhd > 0 ? dhd : 0);
              }
//
//  Pullin
//
              if(pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
              {
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
                GCTData.timeOfDay = GTResults.lastNodeTime;
                dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
                if(dhd == NO_TIME)
                  dhd = 0;
                totalGADhd += dhd;
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
                    dhd = 0;
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
//  And get the next trip on the block
//
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            }
//
//  And adjust to get the next block
//
            if(rcode2 == 0)
            {
              blockNumber = pTRIPSChunk->blockNumber;
            }
          }

//
//  Number of trips
//
          sprintf(tempString, "%d", numTrips);
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_NUMTRIPS,
                WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Platform time
//
          sprintf(tempString, "%s", chhmm(totalPlat));
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_PLATFORMTIME,
                WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Layover time
//
          sprintf(tempString, "%s", chhmm(totalLay));
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_LAYOVERTIME,
                WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Interline Deadhead time
//
          sprintf(tempString, "%s", chhmm(totalILDhd));
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_DEADHEADTIME, 
                WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Garage Deadhead time
//
          sprintf(tempString, "%s", chhmm(totalGADhd));
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_GARDEADHEADTIME,
                WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Block length
//
          sprintf(tempString, "%s", chhmm(totalPlat + totalLay + totalILDhd + totalGADhd));
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_BLOCKLENGTH, 
               WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Total blocks
//
          sprintf(tempString, "Total blocks: %d", numBlocks);
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_BLOCKNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Get the next garage node
//
          if(bFirst)
            bFirst = FALSE;
          else
            rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
        }
//
//  Get the next route
//
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      }
//
//  Done
//
      done:
        SetCursor(hSaveCursor);
      break;  // End of WM_USERSETUP2
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  BLOCKSUMMARY_THISRGRP
//
        case BLOCKSUMMARY_THISRGRP:
          EnableWindow(hCtlALL, TRUE);
          EnableWindow(hCtlSPECIFICBLOCK, TRUE);
          EnableWindow(hCtlBLOCKS, TRUE);
          SetWindowText(hWndDlg, szWindowTitle);
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  BLOCKSUMMARY_ALLRGRPS
//
        case BLOCKSUMMARY_ALLRGRPS:
          EnableWindow(hCtlALL, FALSE);
          EnableWindow(hCtlSPECIFICBLOCK, FALSE);
          EnableWindow(hCtlBLOCKS, FALSE);
          sprintf(tempString, "Block Summary: All %s Route Groups", m_ServiceName);
          SetWindowText(hWndDlg, tempString);
          SendDlgItemMessage(hWndDlg, BLOCKSUMMARY_BLOCKNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hWndDlg, WM_USERSETUP2, (WPARAM)0, (LPARAM)0);
         break;
//
//  BLOCKSUMMARY_ALL
//
        case BLOCKSUMMARY_ALL:
          SendMessage(hCtlBLOCKS, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  BLOCKSUMMARY_SPECIFICBLOCK
//
        case BLOCKSUMMARY_SPECIFICBLOCK:
          SendMessage(hCtlBLOCKS, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  BLOCKSUMMARY_BLOCKS
//
        case BLOCKSUMMARY_BLOCKS:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICBLOCK, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  IDPRINT
//
        case IDPRINT:
          PrintWindowToDC(hWndDlg, "Block Summary");
          break;
//
//  IDOK
//
        case IDOK:
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Standard_Block_Summary);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of BLOCKSUMMARYMsgProc
