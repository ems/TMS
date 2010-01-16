//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

#define MAXBLOCKS  500
#define MAXGARAGES 100

BOOL CALLBACK PULLOUTPULLINMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlBLOCKS;
 static  HANDLE hCtlGARAGES;
 static  HANDLE hCtlPULLOUT;
 static  HANDLE hCtlPULLIN;
 static  HANDLE hCtlMUSTBESAME;
 GenerateTripDef      GTResults;
 GetConnectionTimeDef GCTData;
 HCURSOR saveCursor;
 float   distance;
 BOOL    bPullout;
 BOOL    bPullin;
 BOOL    bMustBeSame;
 char    abbrName[NODES_ABBRNAME_LENGTH + 1];
 char    garName[NODES_ABBRNAME_LENGTH + 1];
 long    assignedToNODESrecordID;
 long    bestTime;
 long    bestGarage;
 long    deadheadTime;
 long    blockNumber;
 long    thisGarage;
 int     nI;
 int     nJ;
 int     selectedBlocks[MAXBLOCKS];
 int     selectedGarages[MAXGARAGES];
 int     numSelectedBlocks;
 int     numSelectedGarages;
 int     rcode2;
 int     numBlocks;
 int     numGarages;
 short int wmId;
 short int wmEvent;
 static BLOCKSDef *pTRIPSChunk;
 static int keyNumber;
 long flags;

 switch(Message)
 {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      keyNumber = pDI->fileInfo.keyNumber;
      pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
      m_numConnectionAlerts = 0;
//
//  Set up handles to the controls
//
     hCtlBLOCKS = GetDlgItem(hWndDlg, PULLOUTPULLIN_BLOCKS);
     hCtlGARAGES = GetDlgItem(hWndDlg, PULLOUTPULLIN_GARAGES);
     hCtlPULLOUT = GetDlgItem(hWndDlg, PULLOUTPULLIN_PULLOUT);
     hCtlPULLIN = GetDlgItem(hWndDlg, PULLOUTPULLIN_PULLIN);
     hCtlMUSTBESAME = GetDlgItem(hWndDlg, PULLOUTPULLIN_MUSTBESAME);
//
//  Set up the "destination"
//
     flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE;
     DisplayPlacement(hWndDlg, PULLOUTPULLIN_DESTINATION, pDI, flags);
//
//  Set up the garages list box
//
     numGarages = 0;
     SendMessage(hCtlGARAGES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
     NODESKey1.flags = NODES_FLAG_GARAGE;
     memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
     rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
     while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
     {
       strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
       trim(abbrName, NODES_ABBRNAME_LENGTH);
       nI = SendMessage(hCtlGARAGES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)abbrName);
       SendMessage(hCtlGARAGES, LB_SETITEMDATA, (WPARAM)nI, NODES.recordID);
       numGarages++;
       rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
     }
     if(numGarages == 0)
     {
       TMSError(hWndDlg, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
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
 //  Set up the pull out, pull in, and must be same check boxes
 //
     SendMessage(hCtlPULLOUT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     SendMessage(hCtlPULLIN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     SendMessage(hCtlMUSTBESAME, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     break; //  End of WM_INITDLG
//
//  WM_CLOSE
//
   case WM_CLOSE:
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
     break; //  End of WM_CLOSE
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  IDASSIGN
//
       case IDASSIGN:
         m_bEstablishRUNTIMES = TRUE;
//
//  Get the garage(s) in question.  If none are selected, select 'em all
//
         if(SendMessage(hCtlGARAGES, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
           SendMessage(hCtlGARAGES, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
         numSelectedGarages = SendMessage(hCtlGARAGES, LB_GETSELITEMS,
               (WPARAM)MAXGARAGES, (LPARAM)(int FAR *)selectedGarages);
//
//  How many blocks did he select?  If none, set 'em all.
//
         if(SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
           SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
         numSelectedBlocks = SendMessage(hCtlBLOCKS, LB_GETSELITEMS,
               (WPARAM)MAXBLOCKS, (LPARAM)(int far *)selectedBlocks);
//
//  Set the hourglass and start the assign process
//
         saveCursor = SetCursor(hCursorWait);
         bPullout = SendMessage(hCtlPULLOUT, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
         bPullin = SendMessage(hCtlPULLIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
         bMustBeSame = SendMessage(hCtlMUSTBESAME, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
         assignedToNODESrecordID = NO_RECORD;
//
//  Do Pullouts first
//
//  Loop through the selected blocks
//
         if(bPullout)
         {
           for(nI = 0; nI < numSelectedBlocks; nI++)
           {
             TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                   (WPARAM)selectedBlocks[nI], (LPARAM)0);
             btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
             GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                   TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                   TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Now, look for the garage connection.  If GetConnectionTime comes
//  back as NO_TIME, it means that the connection doesn't exist.
//
             bestTime = NO_TIME;
             for(nJ = 0; nJ < numSelectedGarages; nJ++)
             {
               thisGarage = SendMessage(hCtlGARAGES, LB_GETITEMDATA,
                     (WPARAM)selectedGarages[nJ], (LPARAM)0);
               GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
               GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
               GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
               GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
               GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
               GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
               GCTData.fromNODESrecordID = thisGarage;
               GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
               GCTData.timeOfDay = GTResults.firstNodeTime;
               deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
               distance = (float)fabs((double)distance);
               if(deadheadTime == NO_TIME)
                 continue;
               if(bestTime == NO_TIME || deadheadTime < bestTime)
               {
                 bestGarage = thisGarage;
                 bestTime = deadheadTime;
               }
             }  
//
//  If we've got a good one, update the record
//
             if(bestTime != NO_TIME)
             {
               pTRIPSChunk->POGNODESrecordID = bestGarage;
               btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
             }
//
//  No connection?  Let him know.
//
             else
             {
               NODESKey0.recordID = GTResults.firstNODESrecordID;
               btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
               strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
               trim(abbrName, NODES_ABBRNAME_LENGTH);
               MessageBeep(MB_ICONINFORMATION);
               LoadString(hInst, ERROR_108, tempString, TEMPSTRING_LENGTH);
               sprintf(szarString, tempString, abbrName, TRIPS.standard.blockNumber);
               LoadString(hInst, TEXT_030, tempString, TEMPSTRING_LENGTH);
               strcat(szarString, tempString);
               MessageBox(hWndDlg, szarString, TMS, MB_ICONINFORMATION);
             }
           }
         }
//
//  Now, do the pullins
//
//  Loop through the selected blocks
//
         if(bPullin)
         {
           for(nI = 0; nI < numSelectedBlocks; nI++)
           {
             TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                   (WPARAM)selectedBlocks[nI], (LPARAM)0);
             btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
             blockNumber = pTRIPSChunk->blockNumber;
             TRIPSKey2.assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
             TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
             TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
             TRIPSKey2.blockNumber = blockNumber + 1;
             TRIPSKey2.blockSequence = NO_TIME;
             rcode2 = btrieve(B_GETLESSTHAN, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
             if(rcode2 == 0 &&
                   pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                   pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                   pTRIPSChunk->blockNumber == blockNumber)
             {
//
// Gen the trip to establish the last node
//
               GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                     TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                     TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Now, look for the garage connection
//
//  Do the pullout and pullin garages have to be the same?
//
               bestTime = NO_TIME;
               bestGarage = NO_RECORD;
               if(bMustBeSame)
               {
                 long currentRecordID = TRIPS.recordID;

                 TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                       (WPARAM)selectedBlocks[nI], (LPARAM)0);
                 btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                 bestGarage = pTRIPSChunk->POGNODESrecordID;
                 GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                 GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                 GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                 GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                 GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                 GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                 GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                 GCTData.toNODESrecordID = bestGarage;
                 GCTData.timeOfDay = GTResults.lastNodeTime;
                 bestTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                 distance = (float)fabs((double)distance);
                 TRIPSKey0.recordID = currentRecordID;
                 btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
               }
//
//  Garages don't have to be the same
//
//  If GetConnectionTime comes back as NO_TIME, it means that the connection doesn't exist.
//
               else
               {
                 for(nJ = 0; nJ < numSelectedGarages; nJ++)
                 {
                   thisGarage = SendMessage(hCtlGARAGES, LB_GETITEMDATA,
                         (WPARAM)selectedGarages[nJ], (LPARAM)0);
                   GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                   GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                   GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                   GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                   GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
                   GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
                   GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                   GCTData.toNODESrecordID = thisGarage;
                   GCTData.timeOfDay = GTResults.lastNodeTime;
                   deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                   distance = (float)fabs((double)distance);
                   if(deadheadTime == NO_TIME)
                     continue;
                   if(bestTime == NO_TIME || deadheadTime < bestTime)
                   {
                     bestGarage = thisGarage;
                     bestTime = deadheadTime;
                   }
                 }
               }
//
//  No connection?  Let him know.
//
               if(bestTime == NO_TIME)
               {
                 NODESKey0.recordID = GTResults.lastNODESrecordID;
                 btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                 strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                 trim(abbrName, NODES_ABBRNAME_LENGTH);
                 if(bMustBeSame)
                 {
                   if(bestGarage == NO_RECORD)
                   {
                     LoadString(hInst, ERROR_177, tempString, TEMPSTRING_LENGTH);
                     sprintf(szarString, tempString, pTRIPSChunk->blockNumber);
                   }
                   else
                   {
                     NODESKey0.recordID = bestGarage;
                     btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                     strncpy(garName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                     trim(garName, NODES_ABBRNAME_LENGTH);
                     LoadString(hInst, ERROR_144, tempString, TEMPSTRING_LENGTH);
                     sprintf(szarString, tempString, garName, abbrName, pTRIPSChunk->blockNumber);
                     LoadString(hInst, TEXT_031, tempString, TEMPSTRING_LENGTH);
                     strcat(szarString, tempString);
                   }
                 }
                 else
                 {
                   LoadString(hInst, ERROR_108, tempString, TEMPSTRING_LENGTH);
                   sprintf(szarString, tempString, abbrName, pTRIPSChunk->blockNumber);
                   LoadString(hInst, TEXT_031, tempString, TEMPSTRING_LENGTH);
                   strcat(szarString, tempString);
                 }
                 MessageBeep(MB_ICONINFORMATION);
                 MessageBox(hWndDlg, szarString, TMS, MB_ICONINFORMATION);
               }
//
//  If we've got a good one, update the record
//
               else
               {
                 pTRIPSChunk->PIGNODESrecordID = bestGarage;
                 btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
               }
             }
           }
         }
         SetCursor(saveCursor);
         EndDialog(hWndDlg, TRUE);
         break;
//
//  IDREMOVE
//
       case IDREMOVE:
//
//  How many blocks did he select?  If none, set 'em all.
//
         if(SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
           SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
         numSelectedBlocks = SendMessage(hCtlBLOCKS, LB_GETSELITEMS,
               (WPARAM)MAXBLOCKS, (LPARAM)(int FAR *)selectedBlocks);
//
//  Set the hourglass and start the remove process
//
         saveCursor = SetCursor(hCursorWait);
         bPullout = SendMessage(hCtlPULLOUT, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
         bPullin = SendMessage(hCtlPULLIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  Loop through the selected blocks
//
         for(nI = 0; nI < numSelectedBlocks; nI++)
         {
           TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                 (WPARAM)selectedBlocks[nI], (LPARAM)0);
           btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           blockNumber = pTRIPSChunk->blockNumber;
           while(rcode2 == 0 &&
                 pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 pTRIPSChunk->blockNumber == blockNumber)
           {
             if(bPullout)
               pTRIPSChunk->POGNODESrecordID = NO_RECORD;
             if(bPullin)
               pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
             pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
             btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
             rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           }
         }
         SetCursor(saveCursor);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Pull_outs_and_Pull_ins_in_Standard_Mode);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
}
