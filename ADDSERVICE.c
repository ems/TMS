//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ADDSERVICEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlNUMBER;
  static HANDLE hCtlNAME;
  static HANDLE hCtlNA;
  static HANDLE hCtlCC;
  static HANDLE hCtlCOMMENTCODE;
  BOOL   bFound;
  char   name[SERVICES_NAME_LENGTH + 1];
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
      hCtlNUMBER = GetDlgItem(hWndDlg, ADDSERVICE_NUMBER);
      hCtlNAME = GetDlgItem(hWndDlg, ADDSERVICE_NAME);
      hCtlNA = GetDlgItem(hWndDlg, ADDSERVICE_NA);
      hCtlCC = GetDlgItem(hWndDlg, ADDSERVICE_CC);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDSERVICE_COMMENTCODE);
//
//  Comment codes combo box
//
      numComments = SetUpCommentList(hWndDlg, ADDSERVICE_COMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCC, FALSE);
        EnableWindow(hCtlCOMMENTCODE, FALSE);
      }
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
        SERVICESKey0.recordID = updateRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
//
//  Number
//
        ltoa(SERVICES.number, tempString, 10);
        SendMessage(hCtlNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Name
//
        strncpy(name, SERVICES.name, sizeof(SERVICES.name));
        trim(name, sizeof(SERVICES.name));
        SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)name);
//
//  Comment
//
        if(SERVICES.COMMENTSrecordID == NO_RECORD)
          SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          for(bFound = FALSE, nI = 0; nI < numComments; nI++)
          {
            if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == SERVICES.COMMENTSrecordID)
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
        case ADDSERVICE_COMMENTCODE:
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

        case ADDSERVICE_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDSERVICE_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

        case IDOK:
//
//  Number
//
          SendMessage(hCtlNUMBER, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_019, hCtlNUMBER);
            break;
          }
//
//  Name
//
          SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)sizeof(name), (LONG)(LPSTR)name);
          if(strcmp(name, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_020, hCtlNAME);
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
            rcode2 = btrieve(B_GETLAST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            SERVICES.recordID = AssignRecID(rcode2, SERVICES.recordID);
            opCode = B_INSERT;
           }
          else
          {
            SERVICESKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            opCode = B_UPDATE;
          }
//
//  Set up SERVICES
//
          SERVICES.number = atol(tempString);
          strncpy(SERVICES.name, name, sizeof(SERVICES.name));
          pad(SERVICES.name, sizeof(SERVICES.name));
          SERVICES.COMMENTSrecordID = commentRecordID;
          SERVICES.flags = 0L;
          memset(SERVICES.reserved, 0x00, SERVICES_RESERVED_LENGTH);
          btrieve(opCode, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
          updateRecordID = SERVICES.recordID;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Services_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of ADDSERVICEMsgProc
