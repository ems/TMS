//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK COSTHOOKEDRUNASMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlRUNTYPE;
  long   numSelected;
  int    nI;
  int    nJ;
  int    nK;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hCtlRUNTYPE = GetDlgItem(hWndDlg, COSTHOOKEDRUNAS_RUNTYPE);
      numSelected = lParam;
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
        {
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE && RUNTYPE[nI][nJ].numPieces == numSelected)
          {
            nK = SendMessage(hCtlRUNTYPE, CB_ADDSTRING, 0,
                  (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
            SendMessage(hCtlRUNTYPE, CB_SETITEMDATA, nK, MAKELONG(nI, nJ));
          }
        }
      }
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Hook_runs);
          break;
//
//  IDOK
//
        case IDOK:
          if((nI = SendMessage(hCtlRUNTYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_157, hCtlRUNTYPE);
            break;
          }
          hookedRunRuntype = SendMessage(hCtlRUNTYPE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
