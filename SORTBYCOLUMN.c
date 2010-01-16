//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK SORTBYCOLUMNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static HANDLE hCtlTMSDEFAULT;
  static HANDLE hCtlCOLUMN;
  static HANDLE hCtlLIST;
  static HANDLE hCtlASCENDING;
  static HANDLE hCtlDESCENDING;
  int  rcode2;
  int  nI;
  int  nJ;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlTMSDEFAULT = GetDlgItem(hWndDlg, SORTBYCOLUMN_TMSDEFAULT);
      hCtlCOLUMN = GetDlgItem(hWndDlg, SORTBYCOLUMN_COLUMN);
      hCtlLIST = GetDlgItem(hWndDlg, SORTBYCOLUMN_LIST);
      hCtlASCENDING = GetDlgItem(hWndDlg, SORTBYCOLUMN_ASCENDING);
      hCtlDESCENDING = GetDlgItem(hWndDlg, SORTBYCOLUMN_DESCENDING);
//
//  Set up the radio buttons
//
      if(pDI->fieldInfo.sortFieldId == 0)
      {
        SendMessage(hCtlTMSDEFAULT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlASCENDING, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
      else
      {
        SendMessage(hCtlCOLUMN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(pDI->fieldInfo.sortFieldId > 0 ? hCtlASCENDING : hCtlDESCENDING,
              BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Get the fields that are defined for this file
//
      FIELDDDFKey1.File = DDFFileNumbers[pDI->fileInfo.fileNumber];
      rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      while(rcode2 == 0 && FIELDDDF.File == DDFFileNumbers[pDI->fileInfo.fileNumber])
      {
//
//  Find out if there's an entry for it in ATTRIBUTES
//
        ATTRIBUTESKey0.Id = FIELDDDF.Id;
        rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
//
//  Yes - use the field name there
//
        if(rcode2 == 0)
        {
          strncpy(tempString, ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
          trim(tempString, ATTRIBUTES_FIELDTITLE_LENGTH);
        }
//
//  No - use the field name from FIELD.DDF
//
        else
        {
          strncpy(tempString, FIELDDDF.Name, sizeof(FIELDDDF.Name));
          trim(tempString, sizeof(FIELDDDF.Name));
        }
//
//  Add it to the combo-box and get the next record
//
        nI = SendMessage(hCtlLIST, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)FIELDDDF.Id);
        if(abs(pDI->fieldInfo.sortFieldId) == FIELDDDF.Id)
          nJ = SendMessage(hCtlLIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
        rcode2 = btrieve(B_GETNEXT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      }
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
        case SORTBYCOLUMN_LIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlTMSDEFAULT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlTMSDEFAULT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlCOLUMN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case SORTBYCOLUMN_TMSDEFAULT:
          if(SendMessage(hCtlTMSDEFAULT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlLIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case SORTBYCOLUMN_COLUMN:
          if(SendMessage(hCtlCOLUMN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlLIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Sorting_by_Column);
          break;

        case IDOK:
          if(SendMessage(hCtlTMSDEFAULT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pDI->fieldInfo.sortFieldId = 0;
          else
          {
            nI = (int)SendMessage(hCtlLIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI != CB_ERR)
            {
              pDI->fieldInfo.sortFieldId = 
                    SendMessage(hCtlLIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              if(SendMessage(hCtlDESCENDING, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                pDI->fieldInfo.sortFieldId = -pDI->fieldInfo.sortFieldId;
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
