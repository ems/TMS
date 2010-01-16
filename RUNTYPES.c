//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK RUNTYPESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlRUNTYPELIST;
  static HANDLE hCtlIDNEW;
  static HANDLE hCtlIDSETUP;
  static HANDLE hCtlIDDELETE;
  int    nI;
  int    nJ;
  int    nK;
  int    nRc;
  long   tempLong;
  BOOL   bEnable;
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
      hCtlRUNTYPELIST = GetDlgItem(hWndDlg, RUNTYPES_RUNTYPELIST);
      hCtlIDNEW = GetDlgItem(hWndDlg, IDNEW);
      hCtlIDSETUP = GetDlgItem(hWndDlg, IDSETUP);
      hCtlIDDELETE = GetDlgItem(hWndDlg, IDDELETE);
//
//  Display the list
//
      SendMessage(hCtlRUNTYPELIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
        {
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
          {
            nK = SendMessage(hCtlRUNTYPELIST, LB_ADDSTRING, (WPARAM)0,
                  (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
            SendMessage(hCtlRUNTYPELIST, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
          }
        }
      }
//
//  Disable the setup and delete buttons
//
      EnableWindow(hCtlIDSETUP, FALSE);
      EnableWindow(hCtlIDDELETE, FALSE);
      break;  //  End of WM_INITDIALOG
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  RUNTYPELIST - Respond to a double-click
//
        case RUNTYPES_RUNTYPELIST:
          switch(wmEvent)
          {
            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDSETUP, 0), (LPARAM)0);
              break;

            case LBN_SELCHANGE:
              nI = SendMessage(hCtlRUNTYPELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              bEnable = nI != LB_ERR;
              EnableWindow(hCtlIDSETUP, bEnable);
              EnableWindow(hCtlIDDELETE, bEnable);
          }
          break;
//
//  IDSETUP - Display the ADDRUNTYPE dialog box, and...
//  IDNEW - Add a new runtype
//
        case IDSETUP:
        case IDNEW:
          nI = SendMessage(hCtlRUNTYPELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR || wmId == IDNEW)
            tempLong = MAKELONG(NO_RECORD, NO_RECORD);
          else
            tempLong = SendMessage(hCtlRUNTYPELIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDRUNTYPE), hWndDlg,
                (DLGPROC)ADDRUNTYPEMsgProc, (LPARAM)tempLong);
//
//  Flag the change to the INI file
//
          if(nRc)
          {
            m_bWorkrulesChanged = TRUE;
            SendMessage(hCtlRUNTYPELIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            for(nI = 0; nI < NUMRUNTYPES; nI++)
            {
              for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
              {
                if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
                {
                  nK = SendMessage(hCtlRUNTYPELIST, LB_ADDSTRING, (WPARAM)0,
                        (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
                  SendMessage(hCtlRUNTYPELIST, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
                }
              }
            }
          }
          break;
//
//  IDDELETE - Nuke a user-defined runtype
//
        case IDDELETE:
          nI = SendMessage(hCtlRUNTYPELIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
//
//  First, ask him if he's sure...
//
          MessageBeep(MB_ICONQUESTION);
          LoadString(hInst, ERROR_086, szarString, sizeof(szarString));
          if(MessageBox(hWndDlg, szarString, TMS,
                MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) != IDYES)
            break;
//
//  Ok, he said yes - set inUse to FALSE, and it'll get blown away on an update
//
          tempLong = SendMessage(hCtlRUNTYPELIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          nI = LOWORD(tempLong);
          nJ = HIWORD(tempLong);
          RUNTYPE[nI][nJ].flags &= ~RTFLAGS_INUSE;
//
// Flag the changes to the INI file
//
          m_bWorkrulesChanged = TRUE;
          SendMessage(hCtlRUNTYPELIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < NUMRUNTYPES; nI++)
          {
            for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
            {
              if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
              {
                nK = SendMessage(hCtlRUNTYPELIST, LB_ADDSTRING, (WPARAM)0,
                      (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
                SendMessage(hCtlRUNTYPELIST, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
              }
            }
          }
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDFINISHED
//
        case IDFINISHED:
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Runtypes);
          break;
      }
      break;

    default:
      return FALSE;
   }
   return TRUE;
}
