//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define MAXBLOCKS 500
#define MAXTRIPS  500

BOOL CALLBACK ASSREMGARAGEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlBLOCKS;
 static  HANDLE hCtlTOPULLOUT;
 static  HANDLE hCtlTOPULLIN;
 static  HANDLE hCtlTO;
 static  HANDLE hCtlNODELIST;
 HCURSOR saveCursor;
 BOOL    bToPullout;
 BOOL    bToPullin;
 BOOL    bToNode;
 char    abbrName[NODES_ABBRNAME_LENGTH + 1];
 long    tripRecordIDs[MAXTRIPS];
 long    assignedToNODESrecordID;
 long    blockNumber;
 int     nI;
 int     nJ;
 int     selectedBlocks[MAXBLOCKS];
 int     numSelectedBlocks;
 int     numTrips;
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
     hCtlBLOCKS = GetDlgItem(hWndDlg, ASSREMGARAGE_BLOCKS);
     hCtlTOPULLOUT = GetDlgItem(hWndDlg, ASSREMGARAGE_TOPULLOUT);
     hCtlTOPULLIN = GetDlgItem(hWndDlg, ASSREMGARAGE_TOPULLIN);
     hCtlTO = GetDlgItem(hWndDlg, ASSREMGARAGE_TO);
     hCtlNODELIST = GetDlgItem(hWndDlg, ASSREMGARAGE_NODELIST);
//
//  Set up the "destination"
//
     flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE;
     DisplayPlacement(hWndDlg, ASSREMGARAGE_DESTINATION, pDI, flags);
//
//  Set up the garages combo-box
//
     numGarages = 0;
     NODESKey1.flags = NODES_FLAG_GARAGE;
     memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
     rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
     while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
     {
       strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
       trim(abbrName, NODES_ABBRNAME_LENGTH);
       nI = SendMessage(hCtlNODELIST, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)abbrName);
       SendMessage(hCtlNODELIST, CB_SETITEMDATA, (WPARAM)nI, NODES.recordID);
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
//  Set up the blocks list box
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
//  Set up the assign to" radio button
//
     SendMessage(hCtlTOPULLOUT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
        case ASSREMGARAGE_NODELIST:
          switch(wmEvent)
          {
            case CBN_SELCHANGE:
              if(SendMessage(hCtlTOPULLOUT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlTOPULLOUT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              else if(SendMessage(hCtlTOPULLIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlTOPULLIN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ASSREMGARAGE_TOPULLOUT:
        case ASSREMGARAGE_TOPULLIN:
          if(SendDlgItemMessage(hWndDlg, wmId, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlNODELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ASSREMGARAGE_TO:
          if(SendMessage(hCtlTO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlNODELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  IDASSIGN and IDREMOVE
//
       case IDASSIGN:
       case IDREMOVE:
         saveCursor = SetCursor(hCursorWait);
//
//  How many blocks did he select?  If none, set 'em all.
//
         if(SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
           SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
         numSelectedBlocks = SendMessage(hCtlBLOCKS, LB_GETSELITEMS,
               (WPARAM)MAXBLOCKS, (LPARAM)(int far *)selectedBlocks);
//
//  To where do the blocks get assigned?
//
         if(wmId == IDASSIGN)
         {
           bToPullout = SendMessage(hCtlTOPULLOUT, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
           bToPullin = SendMessage(hCtlTOPULLIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
           bToNode = SendMessage(hCtlTO, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
         }
//
//  Loop through the selected blocks
//
         for(nI = 0; nI < numSelectedBlocks; nI++)
         {
           TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA,
                 (WPARAM)selectedBlocks[nI], (LPARAM)0);
           btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Get all the recordIDs for the trips in this block
//
           btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
           blockNumber = pTRIPSChunk->blockNumber;
           numTrips = 0;
           while(numTrips < MAXTRIPS &&
                 rcode2 == 0 &&
                 pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                 pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 pTRIPSChunk->blockNumber == blockNumber)
           {
             tripRecordIDs[numTrips++] = TRIPS.recordID;
             rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
           }
           if(numTrips == 0)
             continue;
//
//  See where they're going to get assigned
//
           if(wmId == IDREMOVE)
             assignedToNODESrecordID = NO_RECORD;
           else
           {
             if(bToPullout)
             {
               TRIPSKey0.recordID = tripRecordIDs[0];
               btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
               assignedToNODESrecordID = pTRIPSChunk->POGNODESrecordID;
             }
             else if(bToPullin)
             {
               TRIPSKey0.recordID = tripRecordIDs[numTrips - 1];
               btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
               assignedToNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
             }
             else
             {
               nJ = (int)SendMessage(hCtlNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
               assignedToNODESrecordID =
                     SendMessage(hCtlNODELIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
             }
           }
//
//  Make the assignment
//
           for(nJ = 0; nJ < numTrips; nJ++)
           {
             TRIPSKey0.recordID = tripRecordIDs[nJ];
             btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
             pTRIPSChunk->assignedToNODESrecordID = assignedToNODESrecordID;
             btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Assigning_Garages_to_Standard_Blocks);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
}
