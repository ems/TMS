//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK SORTTRIPMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static HANDLE hCtlBASEMLP;
  static HANDLE hCtlNODES;
  static int    selectedSortPoint;
  static int    baseMLP;
  HANDLE hCtl;
  int    atNodeColumn;
  int    counter;
  int    nI;
  int    rcode2;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      if((pDI = (PDISPLAYINFO)lParam) == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to some selected controls
//
      hCtlBASEMLP = GetDlgItem(hWndDlg, SORTTRIP_BASEMLP);
      hCtlNODES = GetDlgItem(hWndDlg, SORTTRIP_NODES);
//
//  Build a list of BASE pattern nodes
//
      PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
      PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
      PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
            PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
            PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
      {
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)) 
        {
          if(PATTERNS.flags & PATTERNS_FLAG_MLP)
            baseMLP = counter;  
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          nI = SendMessage(hCtlNODES, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtlNODES, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)NODES.recordID);
          counter++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      SendMessage(hCtlNODES, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
//
//  Default to "BASE MLP"
//
      SendMessage(hCtlBASEMLP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      selectedSortPoint = SORTTRIP_BASEMLP;
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
//  Radio buttons
//
        case SORTTRIP_BASEMLP:
        case SORTTRIP_TRIPMLP:
        case SORTTRIP_FIRSTTIME:
        case SORTTRIP_ATNODE:
          selectedSortPoint = wmId;
          break;
//
//  Node list
//
        case SORTTRIP_NODES:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              hCtl = GetDlgItem(hWndDlg, selectedSortPoint);
              if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtl, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                hCtl = GetDlgItem(hWndDlg, SORTTRIP_ATNODE);
                SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                selectedSortPoint = SORTTRIP_ATNODE;
              }
              break;  
          }
          break;

//
//  IDOK
//
        case IDOK:
//
//  If he's picked "At node", he has to have picked a node too
//
          if(selectedSortPoint == SORTTRIP_ATNODE)
          {
            nI = SendMessage(hCtlNODES, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlNODES);
              break;
            }
            atNodeColumn = pDI->fieldInfo.firstNodeColumn + nI;
          }
          else if(selectedSortPoint == SORTTRIP_BASEMLP)
          {
            atNodeColumn = pDI->fieldInfo.firstNodeColumn + baseMLP;
          }
//
//  Save his selections and return
//
          pDI->otherData = atNodeColumn;
          pDI->baggage = selectedSortPoint;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Sorting_Trips);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
