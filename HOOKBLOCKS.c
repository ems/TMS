//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define MAXSELECTEDBLOCKS 100

BOOL CALLBACK HOOKBLOCKSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlDESTINATION;
 static  HANDLE hCtlBLOCKS;
 static  char   destination[sizeof(szFormatString)];
 HCURSOR hSaveCursor;
 long    blockNumber;
 long    destinationBlock;
 long    oldAssignedToNODESrecordID;
 long    newAssignedToNODESrecordID;
 int     nI;
 int     nJ;
 int     selectedBlocks[MAXSELECTEDBLOCKS];
 int     rcode2;
 short   int wmId;
 short   int wmEvent;
 static  BLOCKSDef *pTRIPSChunk;
 static  int keyNumber;

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
//
//  Set up handle to the controls
//
     hCtlBLOCKS = GetDlgItem(hWndDlg, HOOKBLOCKS_BLOCKS);
     hCtlDESTINATION = GetDlgItem(hWndDlg, HOOKBLOCKS_DESTINATION);
//
//  Save the format string and blank out the destination string for now
//
     SendMessage(hCtlDESTINATION, WM_GETTEXT,
           (WPARAM)sizeof(destination), (LONG)(LPSTR)destination);
     SendMessage(hCtlDESTINATION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
//
//  Get the blocks
//
     if(SetUpBlocksListbox(pDI, hCtlBLOCKS) == NO_RECORD)
     {
       TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
     }
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
      wmEvent =HIWORD(wParam);
      switch(wmId)
      {
//
//  HOOKBLOCKS_BLOCKS
//
       case HOOKBLOCKS_BLOCKS:
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             nI = SendMessage(hCtlBLOCKS, LB_GETSELITEMS,
                   (WPARAM)MAXSELECTEDBLOCKS, (LONG)(int FAR *)selectedBlocks);
             if(nI <= 1)
               SendMessage(hCtlDESTINATION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
             else
             {
               TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                     selectedBlocks[0], (LPARAM)0);
               btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
               destinationBlock = TRIPS.standard.blockNumber;
               sprintf(tempString, destination, destinationBlock);
               SendMessage(hCtlDESTINATION, WM_SETTEXT,
                     (WPARAM)0, (LONG)(LPSTR)tempString);
             }
             break;
         }
         break;
//
//  IDOK
//
       case IDOK:
         nI = SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
         if(nI < 2)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_029, hCtlBLOCKS);
           break;
         }
         SendMessage(hCtlBLOCKS, LB_GETSELITEMS,
               (WPARAM)MAXSELECTEDBLOCKS, (LONG)(int FAR *)selectedBlocks);
         TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
               selectedBlocks[0], (LPARAM)0);
         btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
         destinationBlock = pTRIPSChunk->blockNumber;
         newAssignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
         hSaveCursor = SetCursor(hCursorWait);
         for(nJ = 1; nJ < nI; nJ++)
         {
           TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                 selectedBlocks[nJ], (LPARAM)0);
           btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           blockNumber = pTRIPSChunk->blockNumber;
           oldAssignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
           while(rcode2 == 0 &&
                 pTRIPSChunk->assignedToNODESrecordID == oldAssignedToNODESrecordID &&
                 pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 pTRIPSChunk->blockNumber == blockNumber)
           {
             pTRIPSChunk->assignedToNODESrecordID = newAssignedToNODESrecordID;
             pTRIPSChunk->blockNumber = destinationBlock;
             btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
             TRIPSKey2.assignedToNODESrecordID = oldAssignedToNODESrecordID;
             TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
             TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
             TRIPSKey2.blockNumber = blockNumber;
             TRIPSKey2.blockSequence = NO_TIME;
             rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           }
         }
         SetCursor(hSaveCursor);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Hooking_Blocks_in_Standard_Mode);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
}
