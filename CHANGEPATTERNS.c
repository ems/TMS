//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGEPATTERNSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  typedef struct PASSEDDATAStruct
  {
    long ROUTESrecordID;
    long SERVICESrecordID;
    long directionIndex;
    long *pRecordIDs;
    int  numRecords;
  } PASSEDDATADef;
  static PASSEDDATADef *pPassedData;
  static HANDLE hCtlPATTERN;
  HCURSOR hSaveCursor;
  long newPATTERNNAMESrecordID;
  int  nI;
  int  rcode2;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pPassedData = (PASSEDDATADef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      hCtlPATTERN = GetDlgItem(hWndDlg, CHANGEPATTERNS_PATTERN);
      SetUpPatternList(hWndDlg, CHANGEPATTERNS_PATTERN, pPassedData->ROUTESrecordID,
            pPassedData->SERVICESrecordID, pPassedData->directionIndex, TRUE, FALSE);
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
      switch(wmId)
      {
        case IDOK:
          nI = (int)SendMessage(hCtlPATTERN, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_175, hCtlPATTERN);
            break;
          }
          hSaveCursor = SetCursor(hCursorWait);
          newPATTERNNAMESrecordID = SendMessage(hCtlPATTERN, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          for(nI = 0; nI < pPassedData->numRecords; nI++)
          {
            TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(rcode2 == 0)
            {
              TRIPS.PATTERNNAMESrecordID = newPATTERNNAMESrecordID;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
          }
          SetCursor(hSaveCursor);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Multiple_Modifications_on_the_Trips_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of CHANGEPATTERNSMsgProc
