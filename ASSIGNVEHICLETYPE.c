//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ASSIGNVEHICLETYPEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static HANDLE hCtlALL;
  static HANDLE hCtlSPECIFICBLOCK;
  static HANDLE hCtlBLOCKS;
  static HANDLE hCtlBUSTYPES;
  static HANDLE hCtlNA;
  static HANDLE hCtlBT;
  HCURSOR hSaveCursor;
  long newBUSTYPESrecordID;
  long assignedToNODESrecordID;
  long numBlocks;
  long blockNumber;
  int  nI;
  int  nJ;
  int  rcode2;
  short int wmId;
  short int wmEvent;
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
      if(keyNumber != 2)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_136, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlALL = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_ALL);
      hCtlSPECIFICBLOCK = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_SPECIFICBLOCK);
      hCtlBLOCKS = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_BLOCKS);
      hCtlNA = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_NA);
      hCtlBT = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_BT);
      hCtlBUSTYPES = GetDlgItem(hWndDlg, ASSIGNVEHICLETYPE_BUSTYPES);
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
//
//  Set up a list of bustypes and default to N/A
//
      SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      if(SetUpBustypeList(hWndDlg, ASSIGNVEHICLETYPE_BUSTYPES) == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_018, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
        case ASSIGNVEHICLETYPE_BUSTYPES:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlBT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ASSIGNVEHICLETYPE_NA:
          SendMessage(hCtlBUSTYPES, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ASSIGNVEHICLETYPE_BT:
          SendMessage(hCtlBUSTYPES, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  ASSIGNVEHICLETYPE_ALL
//
        case ASSIGNVEHICLETYPE_ALL:
          SendMessage(hCtlBLOCKS, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  ASSIGNVEHICLETYPE_SPECIFICBLOCK
//
        case ASSIGNVEHICLETYPE_SPECIFICBLOCK:
          SendMessage(hCtlBLOCKS, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  ASSIGNVEHICLETYPE_BLOCKS
//
        case ASSIGNVEHICLETYPE_BLOCKS:
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
//  IDOK
//
        case IDOK:
//
//  Get the new bustype
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            newBUSTYPESrecordID = NO_RECORD;
          else
          {
            nI = (int)SendMessage(hCtlBUSTYPES, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_175, hCtlBUSTYPES);
              break;
            }
            newBUSTYPESrecordID = SendMessage(hCtlBUSTYPES, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Get the blocks.  If "All" is selected, select all the blocks and proceed
//
          if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
          nJ = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
//
//  Loop through and change the selected blocks' vehicle type
//
          hSaveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < nJ; nI++)
          {
            if(!SendMessage(hCtlBLOCKS, LB_GETSEL, (WPARAM)nI, (LPARAM)0))
              continue;
            TRIPSKey0.recordID = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
            blockNumber = TRIPS.standard.blockNumber;
            TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
            TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            TRIPSKey2.blockNumber = blockNumber;
            TRIPSKey2.blockSequence = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            while(rcode2 == 0 &&
                  TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
                  TRIPS.standard.RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                  TRIPS.standard.SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                  TRIPS.standard.blockNumber == blockNumber)
            {
              TRIPS.BUSTYPESrecordID = newBUSTYPESrecordID;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);

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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Trips_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of ASSIGNVEHICLETYPEMsgProc
