//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK DATAFIELDMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PFIELDDEFDef pFIELDDEF;
  static HANDLE hCtlTHISFIELD;
  static HANDLE hCtlCREATED;
  static HANDLE hCtlEXISTS;
  static HANDLE hCtlTABLE;
  static HANDLE hCtlFIELD;
  BOOL    bRC;
  int     nI;
  int     nJ;
  int     rcode2;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  See if we've got anything
//
      if((pFIELDDEF = (PFIELDDEFDef)lParam) == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlTHISFIELD = GetDlgItem(hWndDlg, DATAFIELD_THISFIELD);
      hCtlCREATED = GetDlgItem(hWndDlg, DATAFIELD_CREATED);
      hCtlEXISTS = GetDlgItem(hWndDlg, DATAFIELD_EXISTS);
      hCtlTABLE = GetDlgItem(hWndDlg, DATAFIELD_TABLE);
      hCtlFIELD = GetDlgItem(hWndDlg, DATAFIELD_FIELD);
//
//  Set the group box title
//
      SendMessage(hCtlTHISFIELD, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
      strcat(tempString, ": ");
      strcat(tempString, strcmp(pFIELDDEF->szDisplayedName, "") == 0 ? "New" : pFIELDDEF->szDisplayedName);
      SendMessage(hCtlTHISFIELD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Set up the current selections
//
      if(pFIELDDEF->Id != NO_RECORD)
      {
        ATTRIBUTESKey0.Id = pFIELDDEF->Id;
        rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
      }
      if(pFIELDDEF->Id == NO_RECORD || rcode2 != 0 || pFIELDDEF->createdFromFileId == NO_RECORD)
        SendMessage(hCtlCREATED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlEXISTS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Get all of the tables and put 'em in the FROMTABLE combobox
//
      rcode2 = btrieve(B_GETFIRST, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, FILEDDF.Name, sizeof(FILEDDF.Name));
        trim(tempString, sizeof(FILEDDF.Name));
        nI = (int)SendMessage(hCtlTABLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        if(FILEDDF.Id == pFIELDDEF->createdFromFileId)
          SendMessage(hCtlTABLE, LB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
        SendMessage(hCtlTABLE, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)FILEDDF.Id);
        rcode2 = btrieve(B_GETNEXT, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
      }
//
//  Set the selection
//
      if(SendMessage(hCtlTABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0) != LB_ERR)
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(DATAFIELD_TABLE, CBN_SELCHANGE), (LPARAM)0);
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  DATAFIELD_TABLE and DATAFIELD_FIELD list boxes
//
        case DATAFIELD_TABLE:
        case DATAFIELD_FIELD:
          switch(wmEvent)
          {
//
//  Build the list of table fields on a change
//
            case LBN_SELCHANGE:
              nI = (int)SendMessage(hCtlTABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR)
                break;
              if(SendMessage(hCtlCREATED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlCREATED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlEXISTS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              if(wmId == DATAFIELD_FIELD)
                break;
              nJ = (int)SendMessage(hCtlTABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              SendMessage(hCtlFIELD, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              FIELDDDFKey1.File = nJ;
              rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
              while(rcode2 == 0 && FIELDDDF.File == nJ)
              {
                strncpy(tempString, FIELDDDF.Name, sizeof(FIELDDDF.Name));
                trim(tempString, sizeof(FIELDDDF.Name));
                nI = SendMessage(hCtlFIELD, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                if(FIELDDDF.Id == pFIELDDEF->createdFromFieldId)
                  SendMessage(hCtlFIELD, LB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
                SendMessage(hCtlFIELD, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)FIELDDDF.Id);
                rcode2 = btrieve(B_GETNEXT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
              }
          }
          break;
//
//  IDOK
//
        case IDOK:
//
//  If "Already Exists..." is selected, there must be selections in both TABLE and FIELD
//
          if(SendMessage(hCtlEXISTS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            nI = (int)SendMessage(hCtlTABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == LB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_185, hCtlTABLE);
              break;
            }
            nJ = (int)SendMessage(hCtlFIELD, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nJ == LB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_186, hCtlFIELD);
              break;
            }
            pFIELDDEF->createdFromFileId = SendMessage(hCtlTABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            pFIELDDEF->createdFromFieldId = SendMessage(hCtlFIELD, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
          }
          bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FIELDDEF), hWndDlg, (DLGPROC)FIELDDEFMsgProc, (LPARAM)pFIELDDEF);
          if(bRC)
          {
            if(SendMessage(hCtlCREATED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pFIELDDEF->createdFromFileId = NO_RECORD;
              pFIELDDEF->createdFromFieldId = NO_RECORD;
              strcpy(pFIELDDEF->szFieldName, "TMS-Generated");
            }
            else
            {
              pFIELDDEF->createdFromFileId = SendMessage(hCtlTABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              pFIELDDEF->createdFromFieldId = SendMessage(hCtlFIELD, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
              FIELDDDFKey0.Id = pFIELDDEF->createdFromFieldId;
              btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
              strncpy(pFIELDDEF->szFieldName, FIELDDDF.Name, DDF_NAME_LENGTH);
              trim(pFIELDDEF->szFieldName, DDF_NAME_LENGTH);
            }
          }
          EndDialog(hWndDlg, bRC);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Field_Definition_Dialog_Box);
          break;
     }
     break;

   default:
     return FALSE;
 }
 return TRUE;
}
