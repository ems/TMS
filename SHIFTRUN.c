//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define LISTTABSET 8
#define RELTABSET  2

static  PROPOSEDRUNDef  PR;
static  RELIEFPOINTSDef *pRP;
static  int  startIndex;
static  int  endIndex;

BOOL ShiftCheckStartBackward(int index, int pieceNumber);
BOOL ShiftCheckStartForward(int index, int pieceNumber);
BOOL ShiftCheckEndBackward(int index, int pieceNumber);
BOOL ShiftCheckEndForward(int index, int pieceNumber);


//  Blk   Pce  OnLoc  OnTime  OffTime  OffLoc  Plt   Rpt   Trv
//  XXXXX XX   XXXX   XXXXX   XXXXX    XXXX    XXXXX XXXXX XXXXX
//           1         2         3         4         5         6
//  123456789012345678901234567890123456789012345678901234567890

static HWND hWnd;

BOOL CALLBACK SHIFTRUNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  long    runNumber;
  static  long    cutAsRuntype;
  static  HANDLE  hCtlSTART;
  static  HANDLE  hCtlEND;
  static  HANDLE  hCtlRUNNUMBER;
  static  HANDLE  hCtlCUTASRUNTYPE;
  static  HANDLE  hCtlTITLE;
  static  HANDLE  hCtlLIST;
  static  HANDLE  hCtlSPREADOT;
  static  HANDLE  hCtlMAKEUP;
  static  HANDLE  hCtlPAIDBREAKS;
  static  HANDLE  hCtlOVERTIME;
  static  HANDLE  hCtlPAYTIME;
  static  HANDLE  hCtlRUNTYPE;
  static  HANDLE  hCtlREASON;
  static  HANDLE  hCtlIDSHOWDIRECTIONS;
  static  int  LISTTabPos[LISTTABSET] = {7, 12, 19, 27, 36, 44, 50, 56};
  static  int  RELTabPos[RELTABSET] = {7, 14};
  static  int  *pRunIndex;
  static  COSTDef COST;
  HCURSOR saveCursor;
  UINT    selection;
  WORD    DlgWidthUnits;
  BOOL    bDoSlash;
  long    totalPlat;
  long    totalRpt;
  long    totalTrv;
  long    travelTime;
  int     adjustedLISTTabPos[LISTTABSET];
  int     adjustedRELTabPos[RELTABSET];
  int     nI;
  int     nJ;
  int     nK;
  int     nL;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pRunIndex = (int *)lParam;
      if(pRunIndex == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      hWnd = hWndDlg;
//
//  Set up handles to the controls
//
      hCtlSTART = GetDlgItem(hWndDlg, SHIFTRUN_START);
      hCtlEND = GetDlgItem(hWndDlg, SHIFTRUN_END);
      hCtlRUNNUMBER = GetDlgItem(hWndDlg, SHIFTRUN_RUNNUMBER);
      hCtlCUTASRUNTYPE = GetDlgItem(hWndDlg, SHIFTRUN_CUTASRUNTYPE);
      hCtlTITLE = GetDlgItem(hWndDlg, SHIFTRUN_TITLE);
      hCtlLIST = GetDlgItem(hWndDlg, SHIFTRUN_LIST);
      hCtlSPREADOT = GetDlgItem(hWndDlg, SHIFTRUN_SPREADOT);
      hCtlMAKEUP = GetDlgItem(hWndDlg, SHIFTRUN_MAKEUP);
      hCtlPAIDBREAKS = GetDlgItem(hWndDlg, SHIFTRUN_PAIDBREAKS);
      hCtlOVERTIME = GetDlgItem(hWndDlg, SHIFTRUN_OVERTIME);
      hCtlPAYTIME = GetDlgItem(hWndDlg, SHIFTRUN_PAYTIME);
      hCtlRUNTYPE = GetDlgItem(hWndDlg, SHIFTRUN_RUNTYPE);
      hCtlREASON = GetDlgItem(hWndDlg, SHIFTRUN_REASON);
      hCtlIDSHOWDIRECTIONS = GetDlgItem(hWndDlg, IDSHOWDIRECTIONS);
//
//  Set up the tab stops for SHIFTRUN_TITLE and SHIFTRUN_LIST
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nJ = 0; nJ < LISTTABSET; nJ++)
      {
        adjustedLISTTabPos[nJ] = (DlgWidthUnits * LISTTabPos[nJ] * 2);
      }
      SendMessage(hCtlTITLE, LB_SETTABSTOPS, (WPARAM)LISTTABSET, (LPARAM)adjustedLISTTabPos);
      LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlLIST, LB_SETTABSTOPS, (WPARAM)LISTTABSET, (LPARAM)adjustedLISTTabPos);
//
//  Set up the tab stops for SHIFTRUN_START and SHIFTRUN_END
//
      for(nJ = 0; nJ < RELTABSET; nJ++)
      {
        adjustedRELTabPos[nJ] = (DlgWidthUnits * RELTabPos[nJ] * 2);
      }
      SendMessage(hCtlSTART, LB_SETTABSTOPS, (WPARAM)RELTABSET, (LPARAM)adjustedRELTabPos);
      SendMessage(hCtlEND, LB_SETTABSTOPS, (WPARAM)RELTABSET, (LPARAM)adjustedRELTabPos);
//
//  Display the run
//
      nI = *pRunIndex;
      runNumber = RUNLIST[nI].runNumber;
      memcpy(&PR, &RUNLIST[nI].run, sizeof(PR));
      for(cutAsRuntype = NO_RECORD, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
      {
        if(runNumber == RUNLISTDATA[nJ].runNumber)
        {
          cutAsRuntype = RUNLISTDATA[nJ].cutAsRuntype;
          break;
        }
      }
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
//
//  Disable "Travel Instructions" button if not using dynamic travels
//
      if(!bUseDynamicTravels)
      {
        EnableWindow(hCtlIDSHOWDIRECTIONS, FALSE);
      }
//
//  Since he can cancel, bring RELIEFPOINTS in locally
//
      pRP = (RELIEFPOINTSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RELIEFPOINTSDef) * m_numRELIEFPOINTS); 
      if(pRP == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      }
      else
      {
        memcpy(pRP, m_pRELIEFPOINTS, sizeof(RELIEFPOINTSDef) * m_numRELIEFPOINTS);
      }
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_USERSETUP
//
    case WM_USERSETUP:
//
//  Hourglass
//
      saveCursor = SetCursor(hCursorWait);
      selection = SendMessage(hCtlLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
      SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Cost the run
//
      bGenerateTravelInstructions = TRUE;
      RunCoster(&PR, cutAsRuntype, &COST);
      bGenerateTravelInstructions = FALSE;
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
      for(nI = 0; nI < PR.numPieces; nI++)
      {
//
//  Block Number
//
        TRIPSKey0.recordID = PR.piece[nI].fromTRIPSrecordID;
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
        strcat(tempString, NodeAbbrFromRecID(PR.piece[nI].fromNODESrecordID));
        strcat(tempString, "\t");
//
//  On Time
//
        strcat(tempString, Tchar(PR.piece[nI].fromTime));
        strcat(tempString, "\t");
//
//  Off Time
//
        strcat(tempString, Tchar(PR.piece[nI].toTime));
        strcat(tempString, "\t");
//
//  Off Location
//
        strcat(tempString, NodeAbbrFromRecID(PR.piece[nI].toNODESrecordID));
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
        strcat(tempString, chhmm(COST.PIECECOST[nI].reportTime));
        strcat(tempString, "\t");
        totalRpt += COST.PIECECOST[nI].reportTime;
        travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
        strcat(tempString, chhmm(travelTime));
        totalTrv += travelTime;
        nJ = (int)SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)nI);
      }
//
//  Add a total line if there's more than one piece
//
      if(PR.numPieces > 1)
      {
        strcpy(tempString, "\t\t\t\t\t\t");
        strcat(tempString, chhmm(totalPlat));
        strcat(tempString, "\t");
        strcat(tempString, chhmm(totalRpt));
        strcat(tempString, "\t");
        strcat(tempString, chhmm(totalTrv));
        nJ = (int)SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)NO_RECORD);
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
//  Restore the cursor and reselect the row
//
      SendMessage(hCtlLIST, LB_SETCURSEL, (WPARAM)selection, (LPARAM)0);
      SetCursor(saveCursor);
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
//  SHIFTRUN_START - He's on the left (start) listbox
//
        case SHIFTRUN_START:
          switch(wmEvent)
          {
//
//  LBN_SELCHANGE - A change has been made
//
            case LBN_SELCHANGE:
//
//  Get the current selection
//
              nI = (int)SendMessage(hCtlSTART, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR)
                break;
//
//  Get the itemdata for the selection.  It's the index into RELIEFPOINTS
//
              nJ = (int)SendMessage(hCtlSTART, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Pick up the current selection (piece) in the runs display box
//
              nK = (int)SendMessage(hCtlLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nK == LB_ERR)
                break;
//
//  Has he made the start and end times the same?
//
              if(pRP[nJ].time == PR.piece[nK].toTime)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_295, hCtlSTART);
                SendMessage(hWndDlg, WM_COMMAND,
                      MAKEWPARAM(SHIFTRUN_LIST, LBN_SELCHANGE), (LPARAM)0);
                break;
              }
//
//  Check on the implications of the shift.  
//
//  If everything's ok, then the pointers are altered in the ShiftCheck routines
//
              if(startIndex != nJ)
              {
                if(startIndex > nJ)
                  ShiftCheckStartBackward(nJ, nK);
                else
                  ShiftCheckStartForward(nJ, nK);
                SendMessage(hWndDlg, WM_COMMAND,
                      MAKEWPARAM(SHIFTRUN_LIST, LBN_SELCHANGE), (LPARAM)0);
                SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
              }
          }
          break;
//
//  SHIFTRUN_END - He's on the right (end) listbox
//
        case SHIFTRUN_END:
          switch(wmEvent)
          {
//
//  LBN_SELCHANGE - A change has been made
//
            case LBN_SELCHANGE:
//
//  Get the current selection
//
              nI = (int)SendMessage(hCtlEND, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR)
                break;
//
//  Get the itemdata for the selection.  It's the index into RELIEFPOINTS
//
              nJ = (int)SendMessage(hCtlEND, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Pick up the current selection (piece) in the runs display box
//
              nK = (int)SendMessage(hCtlLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nK == LB_ERR)
                break;
//
//  Has he made the start and end times the same?
//
              if(pRP[nJ].time == PR.piece[nK].fromTime)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_295, hCtlSTART);
                SendMessage(hWndDlg, WM_COMMAND,
                      MAKEWPARAM(SHIFTRUN_LIST, LBN_SELCHANGE), (LPARAM)0);
                break;
              }
//
//  Check on the implications of the shift.  
//
//  If everything's ok, then the pointers are altered in the ShiftCheck routines
//
              if(endIndex != nJ)
              {
                if(endIndex > nJ)
                  ShiftCheckEndBackward(nJ, nK);
                else
                  ShiftCheckEndForward(nJ, nK);
                SendMessage(hWndDlg, WM_COMMAND,
                      MAKEWPARAM(SHIFTRUN_LIST, LBN_SELCHANGE), (LPARAM)0);
                SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
              }
          }
          break;

        case SHIFTRUN_LIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              SendMessage(hCtlSTART, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              SendMessage(hCtlEND, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              selection = SendMessage(hCtlLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(selection == LB_ERR)
                break;
              nI = (int)SendMessage(hCtlLIST, LB_GETITEMDATA, (WPARAM)selection, (LPARAM)0);
              if(nI == NO_RECORD)
                break;
              SendMessage(hCtlLIST, LB_SETCURSEL, (WPARAM)selection, (LPARAM)0);
              TRIPSKey0.recordID = PR.piece[nI].fromTRIPSrecordID;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
              {
                if(pRP[nJ].blockNumber == TRIPS.standard.blockNumber)
                {
                  nK = nJ;
                  while(nK < m_numRELIEFPOINTS && 
                        pRP[nK].blockNumber == TRIPS.standard.blockNumber)
                  {
                    strcpy(tempString, pRP[nK].nodeName);
                    strcat(tempString, "\t");
                    strcat(tempString, Tchar(pRP[nK].time));
                    strcat(tempString, "\t");
                    bDoSlash = TRUE;
                    if(pRP[nK].end.runNumber == NO_RECORD)
                    {
                      if(pRP[nK].flags & RPFLAGS_FIRSTINBLOCK)
                      {
                        bDoSlash = FALSE;
                        if(pRP[nK].start.runNumber == NO_RECORD)
                          strcat(tempString, "-");
                      }
                      else
                      {
                        strcat(tempString, "-");
                      }
                    }
                    else
                    {
                      if(pRP[nK].end.runNumber == SECURELOCATION_RUNNUMBER)
                        strcpy(szarString, "SL");
                      else
                        itoa(pRP[nK].end.runNumber, szarString, 10);
                      strcat(tempString, szarString);
                    }
                    if(pRP[nK].end.runNumber != pRP[nK].start.runNumber)
                    {
                      if(pRP[nK].start.runNumber == NO_RECORD &&
                            pRP[nK].flags & RPFLAGS_LASTINBLOCK)
                        bDoSlash = FALSE;
                      if(bDoSlash)
                        strcat(tempString, "/");
                      if(pRP[nK].start.runNumber == NO_RECORD)
                      {
                        if(!(pRP[nK].flags & RPFLAGS_LASTINBLOCK))
                          strcat(tempString, "-");
                      }
                      else
                      {
                        if(pRP[nK].start.runNumber == SECURELOCATION_RUNNUMBER)
                          strcpy(szarString, "SL");
                        else
                          itoa(pRP[nK].start.runNumber, szarString, 10);
                        strcat(tempString, szarString);
                      }
                    }
                    nL = SendMessage(hCtlSTART, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                    SendMessage(hCtlSTART, LB_SETITEMDATA, (WPARAM)nL, (LPARAM)nK);
                    if(pRP[nK].NODESrecordID == PR.piece[nI].fromNODESrecordID &&
                          pRP[nK].TRIPSrecordID == PR.piece[nI].fromTRIPSrecordID)
                    {
                      SendMessage(hCtlSTART, LB_SETCURSEL, (WPARAM)nL, (LPARAM)0);
                      startIndex = nK;
                    }
                    nL = SendMessage(hCtlEND, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                    SendMessage(hCtlEND, LB_SETITEMDATA, (WPARAM)nL, (LPARAM)nK);
                    if(pRP[nK].NODESrecordID == PR.piece[nI].toNODESrecordID &&
                          pRP[nK].TRIPSrecordID == PR.piece[nI].toTRIPSrecordID)
                    {
                      SendMessage(hCtlEND, LB_SETCURSEL, (WPARAM)nL, (LPARAM)0);
                      endIndex = nK;
                    }
                    nK++;
                  }
                  break;
                }
              }
              break;
          }
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          TMSHeapFree(pRP);
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Shift_run);
          break;
//
//  IDSHOWDIRECTIONS 
//
        case IDSHOWDIRECTIONS:
          DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
              hWndMain, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&COST);
          break;
//
//  IDOK
//
        case IDOK:
          memcpy(m_pRELIEFPOINTS, pRP, sizeof(RELIEFPOINTSDef) * m_numRELIEFPOINTS);
          TMSHeapFree(pRP);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

BOOL ShiftCheckStartBackward(int index, int pieceNumber)
{
  long affectedRun;
  int  nI;
//
//  Check to see that the time boundaries are still ok.  Essentially, it's just
//  a check to see that the start of this piece is not earlier than the end of 
//  the previous piece.  Piece-to-piece travel is not taken into account - this is,
//  after all, a "force-the-issue" kind of command.
//
  if(pieceNumber != 0)
  {
    if(pRP[index].time < PR.piece[pieceNumber - 1].toTime)
    {
      TMSError(NULL, MB_ICONSTOP, ERROR_292, (HANDLE)NULL);
      return FALSE;
    }
  }
//
//  Check to see that we're not impinging upon secure location(s)
//
  for(nI = index; nI <= startIndex; nI++)
  {
    if(nI != index)
    {
      if(pRP[nI].end.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
    if(nI != startIndex)
    {
      if(pRP[nI].start.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
  }
//
//  Check on the impact of existing runs
//
  affectedRun = NO_RECORD;
  for(nI = index; nI <= startIndex; nI++)
  {
    if(nI != index)
    {
      if(pRP[nI].end.runNumber != NO_RECORD &&
            pRP[nI].end.runNumber != pRP[startIndex].start.runNumber)
      {
        affectedRun = pRP[nI].end.runNumber;
        break;
      }
    }
    if(nI != startIndex)
    {
      if(pRP[nI].start.runNumber != NO_RECORD &&
            pRP[nI].start.runNumber != pRP[startIndex].start.runNumber)
      {
        affectedRun = pRP[nI].start.runNumber;
        break;
      }
    }
  }
//
//  If we're going to affect another run, let him know
//
  if(affectedRun != NO_RECORD)
  {
    LoadString(hInst, TEXT_209, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, affectedRun, affectedRun);
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(hWnd, tempString, TMS, MB_YESNO) == IDNO)
      return FALSE;
  }
//
//  All done the checks - make the change
//
  PR.piece[pieceNumber].fromTime = pRP[index].time;
  PR.piece[pieceNumber].fromNODESrecordID = pRP[index].NODESrecordID;
  PR.piece[pieceNumber].fromTRIPSrecordID = pRP[index].TRIPSrecordID;
  for(nI = index; nI <= startIndex; nI++)
  {
    if(nI != index)
    {
      pRP[nI].end.recordID = NO_RECORD;
      pRP[nI].end.runNumber = pRP[startIndex].start.runNumber;
      pRP[nI].end.pieceNumber = pRP[startIndex].start.pieceNumber;
    }
    if(nI != startIndex)
    {
      pRP[nI].start.recordID = NO_RECORD;
      pRP[nI].start.runNumber = pRP[startIndex].start.runNumber;
      pRP[nI].start.pieceNumber = pRP[startIndex].start.pieceNumber;
    }
  }
  
  return TRUE;
}


BOOL ShiftCheckStartForward(int index, int pieceNumber)
{
  long affectedRun;
  int  nI;
//
//  Check to see that the time boundaries are still ok.  Essentially, it's just
//  a check to see that the start of this piece is not later or equal to the end
//  of this piece.  Piece-to-piece travel is not taken into account - this is,
//  after all, a "force-the-issue" kind of command.
//
  if(pRP[index].time >= PR.piece[pieceNumber].toTime)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_291, (HANDLE)NULL);
    return FALSE;
  }
//
//  Check to see that we're not impinging upon secure location(s)
//
  for(nI = startIndex; nI <= index; nI++)
  {
    if(nI != startIndex)
    {
      if(pRP[nI].end.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
    if(nI != index)
    {
      if(pRP[nI].start.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
  }
//
//  Check on the impact of existing runs
//
  affectedRun = NO_RECORD;
  for(nI = startIndex; nI <= index; nI++)
  {
    if(nI != startIndex)
    {
      if(pRP[nI].end.runNumber != NO_RECORD &&
            pRP[nI].end.runNumber != pRP[startIndex].start.runNumber)
      {
        affectedRun = pRP[nI].end.runNumber;
        break;
      }
    }
    if(nI != index)
    {
      if(pRP[nI].start.runNumber != NO_RECORD &&
            pRP[nI].start.runNumber != pRP[startIndex].start.runNumber)
      {
        affectedRun = pRP[nI].start.runNumber;
        break;
      }
    }
  }
//
//  If we're going to affect another run, let him know
//
  if(affectedRun != NO_RECORD)
  {
    LoadString(hInst, TEXT_209, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, affectedRun, affectedRun);
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(hWnd, tempString, TMS, MB_YESNO) == IDNO)
      return FALSE;
  }
//
//  All done the checks - make the change
//
  PR.piece[pieceNumber].fromTime = pRP[index].time;
  PR.piece[pieceNumber].fromNODESrecordID = pRP[index].NODESrecordID;
  PR.piece[pieceNumber].fromTRIPSrecordID = pRP[index].TRIPSrecordID;
  for(nI = startIndex; nI <= index; nI++)
  {
    if(nI != index)
    {
      pRP[nI].start.recordID = NO_RECORD;
      pRP[nI].start.runNumber = NO_RECORD;
      pRP[nI].start.pieceNumber = NO_RECORD;
    }
    if(nI != startIndex)
    {
      pRP[nI].end.recordID = NO_RECORD;
      pRP[nI].end.runNumber = NO_RECORD;
      pRP[nI].end.pieceNumber = NO_RECORD;
    }
  }
  
  return TRUE;
}

BOOL ShiftCheckEndBackward(int index, int pieceNumber)
{
  long affectedRun;
  int  nI;
//
//  Check to see that the time boundaries are still ok.  Essentially, it's just
//  a check to see that the end of this piece is not earlier to or equal to the beginning
//  of this piece.  Piece-to-piece travel is not taken into account - this is,
//  after all, a "force-the-issue" kind of command.
//
  if(pRP[index].time <= PR.piece[pieceNumber].fromTime)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_293, (HANDLE)NULL);
    return FALSE;
  }
//
//  Check to see that we're not impinging upon secure location(s)
//
  for(nI = index; nI <= endIndex; nI++)
  {
    if(nI != index)
    {
      if(pRP[nI].end.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
    if(nI != endIndex)
    {
      if(pRP[nI].start.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
  }
//
//  Check on the impact of existing runs
//
  affectedRun = NO_RECORD;
  for(nI = index; nI <= endIndex; nI++)
  {
    if(nI != index)
    {
      if(pRP[nI].end.runNumber != NO_RECORD &&
            pRP[nI].end.runNumber != pRP[endIndex].end.runNumber)
      {
        affectedRun = pRP[nI].end.runNumber;
        break;
      }
    }
    if(nI != endIndex)
    {
      if(pRP[nI].start.runNumber != NO_RECORD &&
            pRP[nI].start.runNumber != pRP[endIndex].end.runNumber)
      {
        affectedRun = pRP[nI].start.runNumber;
        break;
      }
    }
  }
//
//  If we're going to affect another run, let him know
//
  if(affectedRun != NO_RECORD)
  {
    LoadString(hInst, TEXT_209, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, affectedRun, affectedRun);
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(hWnd, tempString, TMS, MB_YESNO) == IDNO)
      return FALSE;
  }
//
//  All done the checks - make the change
//
  PR.piece[pieceNumber].toTime = pRP[index].time;
  PR.piece[pieceNumber].toNODESrecordID = pRP[index].NODESrecordID;
  PR.piece[pieceNumber].toTRIPSrecordID = pRP[index].TRIPSrecordID;
  for(nI = index; nI <= endIndex; nI++)
  {
    if(nI != index)
    {
      pRP[nI].end.recordID = NO_RECORD;
      pRP[nI].end.runNumber = NO_RECORD;
      pRP[nI].end.pieceNumber = NO_RECORD;
    }
    if(nI != endIndex)
    {
      pRP[nI].start.recordID = NO_RECORD;
      pRP[nI].start.runNumber = NO_RECORD;
      pRP[nI].start.pieceNumber = NO_RECORD;
    }
  }
  
  return TRUE;
}


BOOL ShiftCheckEndForward(int index, int pieceNumber)
{
  long affectedRun;
  int  nI;
//
//  Check to see that the time boundaries are still ok.  Essentially, it's just
//  a check to see that the end of this piece does not everlap the start of the
//  next piece.  Piece-to-piece travel is not taken into account - this is,
//  after all, a "force-the-issue" kind of command.
//
  if(PR.numPieces != pieceNumber + 1)
  {
    if(pRP[index].time > PR.piece[pieceNumber + 1].fromTime)
    {
      TMSError(NULL, MB_ICONSTOP, ERROR_294, (HANDLE)NULL);
      return FALSE;
    }
  }
//
//  Check to see that we're not impinging upon secure location(s)
//
  for(nI = endIndex; nI <= index; nI++)
  {
    if(nI != endIndex)
    {
      if(pRP[nI].end.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
    if(nI != index)
    {
      if(pRP[nI].start.runNumber == SECURELOCATION_RUNNUMBER)
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_290, (HANDLE)NULL);
        return FALSE;
      }
    }
  }
//
//  Check on the impact of existing runs
//
  affectedRun = NO_RECORD;
  for(nI = endIndex; nI <= index; nI++)
  {
    if(nI != endIndex)
    {
      if(pRP[nI].end.runNumber != NO_RECORD &&
            pRP[nI].end.runNumber != pRP[endIndex].end.runNumber)
      {
        affectedRun = pRP[nI].end.runNumber;
        break;
      }
    }
    if(nI != index)
    {
      if(pRP[nI].start.runNumber != NO_RECORD &&
            pRP[nI].start.runNumber != pRP[endIndex].end.runNumber)
      {
        affectedRun = pRP[nI].start.runNumber;
        break;
      }
    }
  }
//
//  If we're going to affect another run, let him know
//
  if(affectedRun != NO_RECORD)
  {
    LoadString(hInst, TEXT_209, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, affectedRun, affectedRun);
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(hWnd, tempString, TMS, MB_YESNO) == IDNO)
      return FALSE;
  }
//
//  All done the checks - make the change
//
  PR.piece[pieceNumber].toTime = pRP[index].time;
  PR.piece[pieceNumber].toNODESrecordID = pRP[index].NODESrecordID;
  PR.piece[pieceNumber].toTRIPSrecordID = pRP[index].TRIPSrecordID;
  for(nI = endIndex; nI <= index; nI++)
  {
    if(nI != index)
    {
      pRP[nI].start.recordID = NO_RECORD;
      pRP[nI].start.runNumber = pRP[endIndex].end.runNumber;
      pRP[nI].start.pieceNumber = pRP[endIndex].end.pieceNumber;
    }
    if(nI != endIndex)
    {
      pRP[nI].end.recordID = NO_RECORD;
      pRP[nI].end.runNumber = pRP[endIndex].end.runNumber;
      pRP[nI].end.pieceNumber = pRP[endIndex].end.pieceNumber;
    }
  }
  
  return TRUE;
}
