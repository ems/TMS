//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ADDSIGNCODEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  HANDLE  hCtlCODE;
  static  HANDLE  hCtlTEXT;
  HCURSOR saveCursor;
  char    code[SIGNCODES_CODE_LENGTH + 1];
  int     opCode;
  int     rcode2;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up handles to the controls
//
      hCtlCODE = GetDlgItem(hWndDlg, ADDSIGNCODE_CODE);
      hCtlTEXT = GetDlgItem(hWndDlg, ADDSIGNCODE_TEXT);
//
//  Are we updating?
//
      if(updateRecordID != NO_RECORD)
      {
        SIGNCODESKey0.recordID = updateRecordID;
        btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
        strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
        trim(tempString, SIGNCODES_CODE_LENGTH);
        SendMessage(hCtlCODE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        strncpy(tempString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
        trim(tempString, SIGNCODES_TEXT_LENGTH);
        SendMessage(hCtlTEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
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
          SendMessage(hCtlCODE, WM_GETTEXT, (WPARAM)sizeof(code), (LONG)(LPSTR)code);
//
//  The code can't be blank
//
          if(strcmp(code, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_053, hCtlCODE);
            break;
          }
//
//  Update / Insert
//
          saveCursor = SetCursor(hCursorWait);
          if(updateRecordID == NO_RECORD)
          {
            rcode2 = btrieve(B_GETLAST, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
            SIGNCODES.recordID = AssignRecID(rcode2, SIGNCODES.recordID);
            opCode = B_INSERT;
          }
          else
          {
            SIGNCODESKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
            opCode = B_UPDATE;
          }
//
//  Fill the record
//
          strncpy(SIGNCODES.code, code, SIGNCODES_CODE_LENGTH);
          pad(SIGNCODES.code, SIGNCODES_CODE_LENGTH);
          SendMessage(hCtlTEXT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          strncpy(SIGNCODES.text, tempString, SIGNCODES_TEXT_LENGTH);
          pad(SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
          SIGNCODES.flags = 0;
          memset(&SIGNCODES.reserved, 0x00, SIGNCODES_RESERVED_LENGTH);
//
//  Update / Insert
//
          btrieve(opCode, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
          SetCursor(saveCursor);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
