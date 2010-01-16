//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ADDJURISDICTIONMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlNUMBER;
  static HANDLE hCtlNAME;
  static HANDLE hCtlNA;
  static HANDLE hCtlCC;
  static HANDLE hCtlCOMMENTCODE;
  BOOL   bFound;
  char   name[JURISDICTIONS_NAME_LENGTH + 1];
  long   commentRecordID;
  int    rcode2;
  int    opCode;
  int    numComments;
  int    nI;
  short  int wmId;
  short  int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hCtlNUMBER = GetDlgItem(hWndDlg, ADDJURISDICTION_NUMBER);
      hCtlNAME = GetDlgItem(hWndDlg, ADDJURISDICTION_NAME);
      hCtlNA = GetDlgItem(hWndDlg, ADDJURISDICTION_NA);
      hCtlCC = GetDlgItem(hWndDlg, ADDJURISDICTION_CC);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDJURISDICTION_COMMENTCODE);
//
//  Comment codes combo box
//
      numComments = SetUpCommentList(hWndDlg, ADDJURISDICTION_COMMENTCODE);
//
//  Are we updating?
//
      if(updateRecordID == NO_RECORD)
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
      {
//
//  Get the record
//
        JURISDICTIONSKey0.recordID = updateRecordID;
        btrieve(B_GETEQUAL, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
//
//  Number
//
        ltoa(JURISDICTIONS.number, tempString, 10);
        SendMessage(hCtlNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Name
//
        strncpy(name, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
        trim(name, JURISDICTIONS_NAME_LENGTH);
        SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)name);
//
//  Comment
//
        if(JURISDICTIONS.COMMENTSrecordID == NO_RECORD)
          SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          for(bFound = FALSE, nI = 0; nI < numComments; nI++)
          {
            if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == JURISDICTIONS.COMMENTSrecordID)
            {
              SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlCC : hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
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
//
//  Comments
//
        case ADDJURISDICTION_COMMENTCODE:
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

        case ADDJURISDICTION_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDJURISDICTION_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

        case IDOK:
//
//  Number
//
          SendMessage(hCtlNUMBER, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_075, hCtlNUMBER);
            break;
          }
//
//  Name
//
          SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)sizeof(name), (LONG)(LPSTR)name);
          if(strcmp(name, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_076, hCtlNAME);
            break;
          }
//
//  Comment code
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            commentRecordID = NO_RECORD;
          else
          {
            nI = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            commentRecordID = (nI == CB_ERR ? NO_RECORD : SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0));
          }
//
//  Insert / update?
//

          if(updateRecordID == NO_RECORD)
          {
            rcode2 = btrieve(B_GETLAST, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
            JURISDICTIONS.recordID = AssignRecID(rcode2, JURISDICTIONS.recordID);
            opCode = B_INSERT;
           }
          else
          {
            JURISDICTIONSKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
            opCode = B_UPDATE;
          }
//
//  Set up JURISDICTIONS
//
          JURISDICTIONS.number = atol(tempString);
          strncpy(JURISDICTIONS.name, name, JURISDICTIONS_NAME_LENGTH);
          pad(JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
          JURISDICTIONS.COMMENTSrecordID = commentRecordID;
          JURISDICTIONS.flags = 0L;
          memset(JURISDICTIONS.reserved, 0x00, JURISDICTIONS_RESERVED_LENGTH);
          btrieve(opCode, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
          updateRecordID = JURISDICTIONS.recordID;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Jurisdictions_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of ADDJURISDICTIONMsgProc
