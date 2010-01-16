//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK RENUMBERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{

  switch(Message)
  {
    static RENUMBERPARAMDef *pParam;
    static HANDLE hCtlTO;
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pParam = (RENUMBERPARAMDef *)lParam;
      if(pParam == NULL)
      {
      }
//
//  Set up the handle to the control
//
      hCtlTO = GetDlgItem(hWndDlg, RENUMBER_TO);
//
//  Set the title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, pParam->type, pParam->number);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      switch(wParam)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Renumbering_Standard_Blocks);
          break;

        case IDOK:
          SendMessage(hCtlTO, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          pParam->number = atol(tempString);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
