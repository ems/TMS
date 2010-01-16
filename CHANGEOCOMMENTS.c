//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGEOCOMMENTSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
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
  static HANDLE hCtlCOMMENTCODE;
  static HANDLE hCtlNA;
  static HANDLE hCtlCC;
  HCURSOR hSaveCursor;
  long newCOMMENTSrecordID;
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
      hCtlNA = GetDlgItem(hWndDlg, CHANGECOMMENTS_NA);
      hCtlCC = GetDlgItem(hWndDlg, CHANGECOMMENTS_CC);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, CHANGECOMMENTS_COMMENTCODE);
      SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SetUpCommentList(hWndDlg, CHANGECOMMENTS_COMMENTCODE);
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
        case CHANGECOMMENTS_COMMENTCODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlCC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case CHANGECOMMENTS_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case CHANGECOMMENTS_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  IDOK
//
        case IDOK:
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            newCOMMENTSrecordID = NO_RECORD;
          else
          {
            nI = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_199, hCtlCOMMENTCODE);
              break;
            }
            newCOMMENTSrecordID = SendMessage(hCtlCOMMENTCODE, (WPARAM)CB_GETITEMDATA, nI, (LPARAM)0);
          }
          hSaveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < pPassedData->numRecords; nI++)
          {
            TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(rcode2 == 0)
            {
              TRIPS.COMMENTSrecordID = newCOMMENTSrecordID;
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
} //  End of CHANGECOMMENTSMsgProc
