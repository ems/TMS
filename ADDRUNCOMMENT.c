//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ADDRUNCOMMENTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlDESTINATION;
  static HANDLE hCtlNA;
  static HANDLE hCtlCC;
  static HANDLE hCtlCOMMENTCODE;
  BOOL  bFound;
  int   numComments;
  int   nI;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlDESTINATION = GetDlgItem(hWndDlg, ADDRUNCOMMENT_DESTINATION);
      hCtlNA = GetDlgItem(hWndDlg, ADDRUNCOMMENT_NA);
      hCtlCC = GetDlgItem(hWndDlg, ADDRUNCOMMENT_CC);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDRUNCOMMENT_COMMENTCODE);
//
//  And the rest of the dialog
//
      RUNSKey0.recordID = updateRecordID;
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      SendMessage(hCtlDESTINATION, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, RUNS.runNumber);
      SendMessage(hCtlDESTINATION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Comment codes combo box
//
      numComments = SetUpCommentList(hWndDlg, ADDRUNCOMMENT_COMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCC, FALSE);
        EnableWindow(hCtlCOMMENTCODE, FALSE);
      }

//
//  Comment
//
      if(RUNS.COMMENTSrecordID == NO_RECORD)
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
      {
        for(bFound = FALSE, nI = 0; nI < numComments; nI++)
        {
          if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == DIRECTIONS.COMMENTSrecordID)
          {
            SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
            bFound = TRUE;
            break;
          }
        }
        SendMessage(bFound ? hCtlCC : hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
        case ADDRUNCOMMENT_COMMENTCODE:
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

        case ADDRUNCOMMENT_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDRUNCOMMENT_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Run_Comments);
          break;

        case IDOK:
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNS.COMMENTSrecordID = NO_RECORD;
            btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          }
          else
          {
            nI = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI != CB_ERR)
            {
              RUNS.COMMENTSrecordID = SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            }
          }
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
