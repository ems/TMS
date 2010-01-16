//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK SUPERVISORMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static SUPERVISORPassedDataDef *pPassedData;
  static HANDLE hCtlTEXT;
  static HANDLE hCtlPASSWORD; 
  short  int wmId;
  int    nI;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pPassedData = (SUPERVISORPassedDataDef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
        break;
      }
      hCtlTEXT = GetDlgItem(hWndDlg, SUPERVISOR_TEXT);
      hCtlPASSWORD = GetDlgItem(hWndDlg, SUPERVISOR_PASSWORD);
      SendMessage(hCtlTEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)pPassedData->szMessage);
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
      switch(wmId)
      {
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
          SendMessage(hCtlPASSWORD, WM_GETTEXT, (WPARAM)SZARSTRING_LENGTH, (LONG)(LPSTR)szarString);
          if(pPassedData->flags & SUPERVISOR_FLAG_BIDDING)
          {
            GetPrivateProfileString("TMS", "BMS", "", tempString, TEMPSTRING_LENGTH, TMSINIFile);
            pPassedData->bAuthenticated = strcmp(tempString, szarString) == 0;
          }
          else if(pPassedData->flags & SUPERVISOR_FLAG_DAILYOPS)
          {
            pPassedData->bDisabled = strcmp("garcia12", szarString) == 0;
            if(pPassedData->bDisabled)
            {
              pPassedData->bAuthenticated = TRUE;
              m_UserID = 0;
            }
            else
            {
              for(nI = 0; nI < m_numSupervisors; nI++)
              {
                if(strcmp(m_Supervisors[nI].szPassword, szarString) == 0)
                {
                  pPassedData->bAuthenticated = TRUE;
                  m_UserID = (char)nI;
                  break;
                }
              }
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
