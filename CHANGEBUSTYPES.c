//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGEBUSTYPESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  typedef struct PASSEDDATAStruct
  {
    long ROUTESrecordID;
    long SERVICESrecordID;
    long directionIndex;
    long *pRecordIDs;
    int  numRecords;
  } PASSEDDATADef;
  static PASSEDDATADef *pPassedData;
  static HANDLE hCtlBUSTYPES;
  static HANDLE hCtlNA;
  static HANDLE hCtlBT;
  HCURSOR hSaveCursor;
  long newBUSTYPESrecordID;
  int  nI;
  int  rcode2;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pPassedData = (PASSEDDATADef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      hCtlNA = GetDlgItem(hWndDlg, CHANGEBUSTYPES_NA);
      hCtlBT = GetDlgItem(hWndDlg, CHANGEBUSTYPES_BT);
      hCtlBUSTYPES = GetDlgItem(hWndDlg, CHANGEBUSTYPES_BUSTYPES);
      SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      if(SetUpBustypeList(hWndDlg, CHANGEBUSTYPES_BUSTYPES) == 0)
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
        case CHANGEBUSTYPES_BUSTYPES:
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

        case CHANGEBUSTYPES_NA:
          SendMessage(hCtlBUSTYPES, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case CHANGEBUSTYPES_BT:
          SendMessage(hCtlBUSTYPES, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  IDOK
//
        case IDOK:
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
          hSaveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < pPassedData->numRecords; nI++)
          {
            TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(rcode2 == 0)
            {
              TRIPS.BUSTYPESrecordID = newBUSTYPESrecordID;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Multiple_Modifications_on_the_Trips_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of CHANGEBUSTYPESMsgProc
