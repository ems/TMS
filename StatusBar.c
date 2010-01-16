//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

HANDLE    hCtlTEXT;
HANDLE    hCtlPROGRESS;
HANDLE    hCtlCOMPLETE;
HWND      hWndStatusBar;
HWND      hWndParent;
WORD      Abort;

//
//  StatusBarStart()
//

void CALLBACK StatusBarStart(HWND hParentWnd, LPSTR pszTitle)
{
  Abort = FALSE;
  hWndParent = hParentWnd;
  hWndStatusBar = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_STATUSBAR),
        hParentWnd, (DLGPROC)StatusBarDlgProc, (LPARAM)pszTitle);
  if(hWndStatusBar != NULL)
  {
    ShowWindow(hWndStatusBar, SW_SHOWNORMAL);
    StatusBarYield(hWndStatusBar);
  }
}

//
//  StatusBarDlgProc()
//
BOOL CALLBACK StatusBarDlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char *pszTitle;
  short int wmId;

  switch(message)
  {
//
//  WM_INITDIALOG
//
    case WM_INITDIALOG:
      hCtlTEXT = GetDlgItem(hWndDlg, STATUSBAR_TEXT);
      hCtlPROGRESS = GetDlgItem(hWndDlg, STATUSBAR_PROGRESS);
      hCtlCOMPLETE = GetDlgItem(hWndDlg, STATUSBAR_COMPLETE);
      pszTitle = (char *)lParam;
      if(pszTitle != NULL)
        SetWindowText(hWndDlg, pszTitle);
      SendMessage(hCtlPROGRESS, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
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
          Abort = TRUE;
          StatusBarEnd();
          break;
//
//  default processing
//
        default:
          return(FALSE);
      }
      break;
//
//  default processing
//
    default:
      return(FALSE);
  }
  return(TRUE);
}

//
//  StatusBarYield()
//
void StatusBarYield(HWND hWnd)
{
  MSG msg;

  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if(!hWnd || !IsDialogMessage(hWnd, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return;
}

//
//  StatusBarEnd()
//

void CALLBACK StatusBarEnd(void)
{
  if(hWndStatusBar != NULL)
    DestroyWindow(hWndStatusBar);
}

//
//  StatusBarAbort()
//

BOOL CALLBACK StatusBarAbort(void)
{
  return(Abort);
}

//
//  StatusBarText()
//

void CALLBACK StatusBarText(LPSTR pszText)
{
  SendMessage(hCtlTEXT, WM_SETTEXT, (WPARAM)0, (LONG)pszText);
  StatusBarYield(hWndStatusBar);
}

//
//  StatusBar()
//

void CALLBACK StatusBar(long nThisRecord, long nLastRecord)
{
  char szarString[16];
  char tempString[16];
  long tempLong;
  int  nFillValue;

  if(nThisRecord == -1L && nLastRecord == -1L)
  {
    nFillValue = 0;
    strcpy(tempString, "");
  }
  else
  {
    if(nThisRecord == nLastRecord && nThisRecord != 0)
      tempLong = 100L;
    else
      tempLong = nLastRecord == 0L ? 0L : (nThisRecord * 100) / nLastRecord;
    nFillValue = (int)(tempLong > 100 ? 100 : tempLong);
    sprintf(tempString, "%d%%", nFillValue);
  }
  SendMessage(hCtlCOMPLETE, WM_GETTEXT, sizeof(szarString), (LONG)(LPSTR)szarString);
  if(strcmp(szarString, tempString) != 0)
  {
    SendMessage(hCtlCOMPLETE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlPROGRESS, PBM_SETPOS, (WPARAM)nFillValue, (LPARAM)0); 
  }
  StatusBarYield(hWndStatusBar);
}

//
//  StatusBarEnableCancel()
//
void CALLBACK StatusBarEnableCancel(BOOL bEnable)
{
  if(hWndStatusBar != NULL)
    EnableWindow(GetDlgItem(hWndStatusBar, IDCANCEL), bEnable);
}
