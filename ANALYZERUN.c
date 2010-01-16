//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#ifdef TABSET
#undef TABSET
#endif
#define TABSET 8

//  Blk   Pce  OnLoc  OnTime  OffTime  OffLoc  Plt   Rpt   Trv
//  XXXXX XX   XXXX   XXXXX   XXXXX    XXXX    XXXXX XXXXX XXXXX
//           1         2         3         4         5         6
//  123456789012345678901234567890123456789012345678901234567890

BOOL CALLBACK ANALYZERUNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  COSTDef COST;
  static  long runNumber;
  static  int  tabPos[TABSET] = {7, 12, 19, 27, 36, 44, 50, 56};
  ANALYZERUNPassedDataDef *pPassedData;
  PROPOSEDRUNDef PROPOSEDRUN;
  HCURSOR saveCursor;
  HANDLE  hCtlRUNNUMBER;
  HANDLE  hCtlCUTASRUNTYPE;
  HANDLE  hCtlTITLE;
  HANDLE  hCtlLIST;
  HANDLE  hCtlSPREADOT;
  HANDLE  hCtlMAKEUP;
  HANDLE  hCtlPAIDBREAKS;
  HANDLE  hCtlOVERTIME;
  HANDLE  hCtlPAYTIME;
  HANDLE  hCtlRUNTYPE;
  HANDLE  hCtlREASON;
  HANDLE  hCtlIDSHOWDIRECTIONS;
  WORD    DlgWidthUnits;
  long    onTime;
  long    offTime;
  long    cutAsRuntype;
  long    serviceRecordID;
  long    divisionRecordID;
  long    pieceNumber;
  long    totalPlat;
  long    totalRpt;
  long    totalTrv;
  long    reportTime;
  long    travelTime;
  int     adjustedTabPos[TABSET];
  int     nI;
  int     nJ;
  int     rcode2;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Grab the parameter
//
      pPassedData = (ANALYZERUNPassedDataDef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlRUNNUMBER = GetDlgItem(hWndDlg, ANALYZERUN_RUNNUMBER);
      hCtlCUTASRUNTYPE = GetDlgItem(hWndDlg, ANALYZERUN_CUTASRUNTYPE);
      hCtlTITLE = GetDlgItem(hWndDlg, ANALYZERUN_TITLE);
      hCtlLIST = GetDlgItem(hWndDlg, ANALYZERUN_LIST);
      hCtlSPREADOT = GetDlgItem(hWndDlg, ANALYZERUN_SPREADOT);
      hCtlMAKEUP = GetDlgItem(hWndDlg, ANALYZERUN_MAKEUP);
      hCtlPAIDBREAKS = GetDlgItem(hWndDlg, ANALYZERUN_PAIDBREAKS);
      hCtlOVERTIME = GetDlgItem(hWndDlg, ANALYZERUN_OVERTIME);
      hCtlPAYTIME = GetDlgItem(hWndDlg, ANALYZERUN_PAYTIME);
      hCtlRUNTYPE = GetDlgItem(hWndDlg, ANALYZERUN_RUNTYPE);
      hCtlREASON = GetDlgItem(hWndDlg, ANALYZERUN_REASON);
      hCtlIDSHOWDIRECTIONS = GetDlgItem(hWndDlg, IDSHOWDIRECTIONS);
//
//  Hourglass
//
      saveCursor = SetCursor(hCursorWait);
//
//  Set up the tab stops for ANALYZERUN_TITLE and ANALYZERUN_LIST
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nJ = 0; nJ < TABSET; nJ++)
      {
        adjustedTabPos[nJ] = (DlgWidthUnits * tabPos[nJ] * 2);
      }
      SendMessage(hCtlTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlLIST, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  How did we get here?
//
//  From the menu on the Runs Table
//
      if(pPassedData->flags & ANALYZERUN_FLAGS_CHILDWND)
      {
        RUNSKey0.recordID = pPassedData->longValue;
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        runNumber = RUNS.runNumber;
        divisionRecordID = RUNS.DIVISIONSrecordID;
        serviceRecordID = RUNS.SERVICESrecordID;
        pieceNumber = 0;
        cutAsRuntype = RUNS.cutAsRuntype;
        while(rcode2 == 0 &&
              RUNS.runNumber == runNumber && 
              RUNS.SERVICESrecordID == serviceRecordID)
        {
          if(RUNS.DIVISIONSrecordID == divisionRecordID)
          {
            RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                  RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
            PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
            PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
            PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
            PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
            PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
            PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
            PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
            PROPOSEDRUN.piece[pieceNumber].after.endTime = RUNS.after.endTime;
            pieceNumber++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        }
        PROPOSEDRUN.numPieces = (int)pieceNumber;
      }
//
//  From MANCUT / Visual Runcutter
//
      else
      {
        nI = (int)pPassedData->longValue;
        runNumber = RUNLIST[nI].runNumber;
        memcpy(&PROPOSEDRUN, &RUNLIST[nI].run, sizeof(PROPOSEDRUN));
        for(cutAsRuntype = NO_RECORD, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
        {
          if(runNumber == RUNLISTDATA[nJ].runNumber)
          {
            cutAsRuntype = RUNLISTDATA[nJ].cutAsRuntype;
            break;
          }
        }
        if(pPassedData->flags & ANALYZERUN_FLAGS_VISUALRUNCUTTER)
        {
          sprintf(tempString, "Run %ld Properties", runNumber);
          SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
      }
//
//  Cost the run
//
      bGenerateTravelInstructions = TRUE;
      RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
      bGenerateTravelInstructions = FALSE;
//
//  Dynamic travels?
//
      if(bUseDynamicTravels)
      {
        if(bUseCISPlan)
          EnableWindow(hCtlIDSHOWDIRECTIONS, TRUE);
        else
          EnableWindow(hCtlIDSHOWDIRECTIONS, numTravelInstructions > 0);
      }
//
//  Fill in the controls
//
//  Run number
//
      SendMessage(hCtlRUNNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)ltoa(runNumber, tempString, 10));
//
//  Cut as runtype
//
      nI = (short int)LOWORD(cutAsRuntype);
      nJ = (short int)HIWORD(cutAsRuntype);
      if(nI >= 0 && nJ >= 0)
        SendMessage(hCtlCUTASRUNTYPE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
//
//  The list box of the pieces of the run
//
      totalPlat = 0;
      totalRpt = 0;
      totalTrv = 0;
      for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
      {
//
//  Block Number
//
        TRIPSKey0.recordID = PROPOSEDRUN.piece[nI].fromTRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        ltoa(TRIPS.standard.blockNumber, tempString, 10);
        strcat(tempString, "\t");
//
//  Piece Number
//
        sprintf(szarString, "%3d", nI + 1);
        strcat(tempString, szarString);
        strcat(tempString, "\t");
//
//  On Location
//
        if(m_pNodeAbbrs != NULL)
        {
          strcat(tempString, NodeAbbrFromRecID(PROPOSEDRUN.piece[nI].fromNODESrecordID));
        }
        else
        {
          NODESKey0.recordID = PROPOSEDRUN.piece[nI].fromNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString); 
        }
        strcat(tempString, "\t");
//
//  On Time
//
        strcat(tempString, Tchar(PROPOSEDRUN.piece[nI].fromTime));
        strcat(tempString, "\t");
//
//  Off Time
//
        strcat(tempString, Tchar(PROPOSEDRUN.piece[nI].toTime));
        strcat(tempString, "\t");
//
//  Off Location
//
        if(m_pNodeAbbrs != NULL)
        {
          strcat(tempString, NodeAbbrFromRecID(PROPOSEDRUN.piece[nI].toNODESrecordID));
        }
        else
        {
          NODESKey0.recordID = PROPOSEDRUN.piece[nI].toNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString); 
        }
        strcat(tempString, "\t");
//
//  Platform time
//
        strcat(tempString, chhmm(COST.PIECECOST[nI].platformTime));
        strcat(tempString, "\t");
        totalPlat += COST.PIECECOST[nI].platformTime;
//
//  Report and Travel times
//
        reportTime = COST.PIECECOST[nI].reportTime;
        totalRpt += reportTime;
        travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
        totalTrv += travelTime;
        strcat(tempString, chhmm(reportTime));
        strcat(tempString, "\t");
        strcat(tempString, chhmm(travelTime));
        SendMessage(hCtlLIST, LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
      }
//
//  Add a total line if there's more than one piece
//
      if(PROPOSEDRUN.numPieces > 1)
      {
        strcpy(tempString, "\t\t\t\t\t\t");
        strcat(tempString, chhmm(totalPlat));
        strcat(tempString, "\t");
        strcat(tempString, chhmm(totalRpt));
        strcat(tempString, "\t");
        strcat(tempString, chhmm(totalTrv));
        SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
//
//  Spread O/T
//
      SendMessage(hCtlSPREADOT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.spreadOvertime));
//
//  Makeup time
//
      SendMessage(hCtlMAKEUP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.makeUpTime));
//
//  Paid Breaks
//
      SendMessage(hCtlPAIDBREAKS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.paidBreak));
//
//  Overtime
//
      SendMessage(hCtlOVERTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.overTime));
//
//  Pay time
//
      SendMessage(hCtlPAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.payTime));
//
//  RunCoster runtype
//
      nI = (short int)LOWORD(COST.runtype);
      nJ = (short int)HIWORD(COST.runtype);
      if(nI < 0 || nJ < 0)
        LoadString(hInst, TEXT_052, tempString, TEMPSTRING_LENGTH);
      else
        strcpy(tempString, RUNTYPE[nI][nJ].localName);
      SendMessage(hCtlRUNTYPE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  RunCoster reasoning
//
      SendMessage(hCtlREASON, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)runcosterReason);
//
//  Restore the cursor
//
      SetCursor(saveCursor);
//
//  Disable "Travel Instructions" button if not using dynamic travels
//
      if(!bUseDynamicTravels)
        EnableWindow(hCtlIDSHOWDIRECTIONS, FALSE);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Analyze);
          break;
//
//  IDSHOWDIRECTIONS 
//
        case IDSHOWDIRECTIONS:
          if(bUseCISPlan)
          {
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
                hWndDlg, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&COST);
          }
          else
          {
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
                hWndDlg, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&runNumber);
          }
          break;
//
//  IDOK
//
        case IDOK:
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

