//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define MAXBLOCKS 500

BOOL CALLBACK UNHOOKTRIPSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlALL;
  static  HANDLE hCtlSPECIFIC;
  static  HANDLE hCtlBLOCKLIST;
  int     nI;
  int     nJ;
  int     selectedBlocks[MAXBLOCKS];
  int     rcode2;
  long    assignedToNODESrecordID;
  long    blockNumber;
  short   int wmId;
  short   int wmEvent;
  static  BLOCKSDef *pTRIPSChunk;
  static  int keyNumber;
  int     numBlocks;
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
//
//  Set up handles to the controls
//
      hCtlALL = GetDlgItem(hWndDlg, UNHOOKTRIPS_ALL);
      hCtlSPECIFIC = GetDlgItem(hWndDlg, UNHOOKTRIPS_SPECIFIC);
      hCtlBLOCKLIST = GetDlgItem(hWndDlg, UNHOOKTRIPS_BLOCKLIST);
//
//  Set up the "destination"
//
      flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE;
      DisplayPlacement(hWndDlg, UNHOOKTRIPS_DESTINATION, pDI, flags);
//
//  Get the blocks
//
      numBlocks = SetUpBlocksListbox(pDI, hCtlBLOCKLIST);
      if(numBlocks == NO_RECORD)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Default to all blocks
//
      SendMessage(hCtlALL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
        case UNHOOKTRIPS_BLOCKLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFIC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case UNHOOKTRIPS_ALL:
          if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlBLOCKLIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;
//
//  IDOK
//
        case IDOK:
          if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlBLOCKLIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
          nI = SendMessage(hCtlBLOCKLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
          if(nI == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_006, hCtlBLOCKLIST);
            break;
          }
          LoadString(hInst, TEXT_121, tempString, TEMPSTRING_LENGTH);
          StatusBarStart(hWndDlg, tempString);
          StatusBar(-1L, -1L);
//
//  Undo
//
          SendMessage(hCtlBLOCKLIST, LB_GETSELITEMS,
                (WPARAM)MAXBLOCKS, (LONG)(int FAR *)selectedBlocks);
          for(nJ = 0; nJ < nI; nJ++)
          {
            TRIPSKey0.recordID = SendMessage(hCtlBLOCKLIST, LB_GETITEMDATA,
                  (WPARAM)selectedBlocks[nJ], (LPARAM)0);
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
            blockNumber = pTRIPSChunk->blockNumber;
            while(rcode2 == 0 &&
                  pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                  pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                  pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                  pTRIPSChunk->blockNumber == blockNumber)
            {
              LoadString(hInst, TEXT_122, szarString, sizeof(szarString));
              sprintf(tempString, szarString, pTRIPSChunk->blockNumber);
              StatusBarText(tempString);
              if(StatusBarAbort())
                break;
              pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
              pTRIPSChunk->POGNODESrecordID = NO_RECORD;
              pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
              pTRIPSChunk->RGRPROUTESrecordID = NO_RECORD;
              pTRIPSChunk->SGRPSERVICESrecordID = NO_RECORD;
              pTRIPSChunk->blockNumber = 0;
              pTRIPSChunk->blockSequence = NO_TIME;
//
//  If standard blocks get undone, dropback get undone automatically
//
              if(keyNumber == 2)
              {
                TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
                TRIPS.dropback.POGNODESrecordID = NO_RECORD;
                TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
                TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
                TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
                TRIPS.dropback.blockNumber = 0;
                TRIPS.dropback.blockSequence = NO_TIME;
              }
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
              TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
              TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
              TRIPSKey2.blockNumber = blockNumber;
              TRIPSKey2.blockSequence = NO_TIME;
              rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            }
          }
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Unblocking_Trips_in_Standard_Mode);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
}
