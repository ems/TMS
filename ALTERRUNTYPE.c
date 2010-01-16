//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ALTERRUNTYPEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlDESTINATION;
  static HANDLE hCtlRUNTYPE;
  static long   firstPieceRecordID;
  static int    numPieces;
  long   divisionRecordID;
  long   cutAsRuntype;
  long   runNumber;
  long   serviceRecordID;
  int    rcode2;
  int    nI;
  int    nJ;
  int    nK;
  int    nL;
  int    nM;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up handles to the controls
//
      hCtlDESTINATION = GetDlgItem(hWndDlg, ALTERRUNTYPE_DESTINATION);
      hCtlRUNTYPE = GetDlgItem(hWndDlg, ALTERRUNTYPE_RUNTYPE);
//
//  Get the record to be updated and establish position on key 1
//
      RUNSKey0.recordID = updateRecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      divisionRecordID = RUNS.DIVISIONSrecordID;
      serviceRecordID = RUNS.SERVICESrecordID;
      runNumber = RUNS.runNumber;
      RUNSKey1.DIVISIONSrecordID = divisionRecordID;
      RUNSKey1.SERVICESrecordID = serviceRecordID;
      RUNSKey1.runNumber = runNumber;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      numPieces = 0;
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == divisionRecordID &&
            RUNS.SERVICESrecordID == serviceRecordID &&
            RUNS.runNumber == runNumber)
      {
        if(RUNS.pieceNumber == 1)
        {
          firstPieceRecordID = RUNS.recordID;
          cutAsRuntype = RUNS.cutAsRuntype;
        }
        numPieces++;
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
//
//  Fill in the controls
//
//  Destination
//
      SendMessage(hCtlDESTINATION, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
      sprintf(szarString, tempString, runNumber);
      SendMessage(hCtlDESTINATION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
//
//  Run types - only display those that correspnd to the number of pieces
//
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
        {
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE && RUNTYPE[nI][nJ].numPieces == numPieces)
          {
            nK = (int)SendMessage(hCtlRUNTYPE, CB_ADDSTRING, (WPARAM)0,
                  (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
            SendMessage(hCtlRUNTYPE, CB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
            nL = (short int)LOWORD(cutAsRuntype);
            nM = (short int)HIWORD(cutAsRuntype);
            if(nL == nI && nM == nJ)
              SendMessage(hCtlRUNTYPE, CB_SETCURSEL, (WPARAM)nK, (LPARAM)0);
          }
        }
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
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Altering_Runtypes);
          break;

        case IDOK:
//
//  Get the current selection
//
          nI = (int)SendMessage(hCtlRUNTYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_157, hCtlRUNTYPE);
            break;
          }
          cutAsRuntype = SendMessage(hCtlRUNTYPE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Re-establish the first record of the run
//
          RUNSKey0.recordID = firstPieceRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          divisionRecordID = RUNS.DIVISIONSrecordID;
          serviceRecordID = RUNS.SERVICESrecordID;
          runNumber = RUNS.runNumber;
          while(rcode2 == 0 &&
                RUNS.DIVISIONSrecordID == divisionRecordID &&
                RUNS.SERVICESrecordID == serviceRecordID &&
                RUNS.runNumber == runNumber)
          {
            RUNS.cutAsRuntype = cutAsRuntype;
            btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
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
