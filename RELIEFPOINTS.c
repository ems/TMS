//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK RELIEFPOINTSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  HANDLE hCtlLABELS;
  static  HANDLE hCtlDONTCUTATNA;
  static  HANDLE hCtlIGNORE;
  static  HANDLE hCtlDONTCUTAT2;
  static  HANDLE hCtlIGNOREMINUTES;
  static  HANDLE hCtlDONTCUTMIDTRIP;

  HCURSOR saveCursor;
  int     reliefLabelsSelected[sizeof(CUTPARMS.labels)];
  int     nI;
  int     nJ;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlLABELS = GetDlgItem(hWndDlg, RELIEFPOINTS_LABELS);
      hCtlDONTCUTATNA = GetDlgItem(hWndDlg, RELIEFPOINTS_DONTCUTATNA);
      hCtlIGNORE = GetDlgItem(hWndDlg, RELIEFPOINTS_IGNORE);
      hCtlDONTCUTAT2 = GetDlgItem(hWndDlg, RELIEFPOINTS_DONTCUTAT2);
      hCtlIGNOREMINUTES = GetDlgItem(hWndDlg, RELIEFPOINTS_IGNOREMINUTES);
      hCtlDONTCUTMIDTRIP = GetDlgItem(hWndDlg, RELIEFPOINTS_DONTCUTMIDTRIP);
//
//  Set up the node labels listbox
//
      SERVICESKey1.number = m_ServiceNumber;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      SetUpReliefLabels(hWndDlg, SERVICES.recordID,
            RELIEFPOINTS_LABELS, CUTPARMS.labels, (int)NULL);
//
//  "Ignore" relief points
//
      if((CUTPARMS.flags & CUTPARMSFLAGS_IGNORE) ||
            (CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTAT2))
      {
        if(CUTPARMS.flags & CUTPARMSFLAGS_IGNORE)
          SendMessage(hCtlIGNORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
          SendMessage(hCtlDONTCUTAT2, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        itoa(CUTPARMS.ignoreMinutes, tempString, 10);
        SendMessage(hCtlIGNOREMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
      else
      {
        SendMessage(hCtlDONTCUTATNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlIGNOREMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
        EnableWindow(hCtlIGNOREMINUTES, FALSE);
      }
//
//  Don't cut mid trip
//
      if(CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTMIDTRIP)
      {
        SendMessage(hCtlDONTCUTMIDTRIP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_CANCEL
//
    case WM_COMMAND:
      switch(wParam)
      {
//
//  RELIEFPOINTS_DONTCUTATNA
//
        case RELIEFPOINTS_DONTCUTATNA:
          EnableWindow(hCtlIGNOREMINUTES, FALSE);
          SendMessage(hCtlIGNOREMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          break;
//
//  RELIEFPOINTS_IGNORE
//  RELIEFPOINTS_DONTCUTAT2
//
        case RELIEFPOINTS_IGNORE:
        case RELIEFPOINTS_DONTCUTAT2:
          EnableWindow(hCtlIGNOREMINUTES, TRUE);
          itoa(CUTPARMS.ignoreMinutes, tempString, 10);
          SendMessage(hCtlIGNOREMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;
//
//  IDOK
//
        case IDOK:
//
//  reliefLabels
//
          nI = SendMessage(hCtlLABELS, LB_GETSELITEMS, (WPARAM)sizeof(CUTPARMS.labels),
                (LPARAM)(int FAR *)reliefLabelsSelected);
          if(nI == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_125, hCtlLABELS);
            break;
          }
          strcpy(CUTPARMS.labels, "");
          for(nJ = 0; nJ < nI; nJ++)
          {
            SendMessage(hCtlLABELS, LB_GETTEXT, (WPARAM)reliefLabelsSelected[nJ], (LONG)(LPSTR)tempString);
            strcat(CUTPARMS.labels, tempString);
          }
//
//  "Ignore" / "DontCutAt2"
//
//  Clear previous values
//
          if(CUTPARMS.flags & CUTPARMSFLAGS_IGNORE)
          {
            CUTPARMS.flags &= ~CUTPARMSFLAGS_IGNORE;
          }
          if(CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTAT2)
          {
            CUTPARMS.flags &= ~CUTPARMSFLAGS_DONTCUTAT2;
          }
          if(SendMessage(hCtlDONTCUTATNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.ignoreMinutes = 0;
          }
          else
          {
            if(SendMessage(hCtlIGNORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              CUTPARMS.flags |= CUTPARMSFLAGS_IGNORE;
            }
            else
            {
              CUTPARMS.flags |= CUTPARMSFLAGS_DONTCUTAT2;
            }
            SendMessage(hCtlIGNOREMINUTES, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            CUTPARMS.ignoreMinutes = atoi(tempString);
          }
//
//  Don't cut mid-trip
//
          if(SendMessage(hCtlDONTCUTMIDTRIP, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_DONTCUTMIDTRIP;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_DONTCUTMIDTRIP)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_DONTCUTMIDTRIP;
            }
          }
          saveCursor = SetCursor(hCursorWait);
          m_bWorkrulesChanged = TRUE;
          SetCursor(saveCursor);
//
//  Done
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
