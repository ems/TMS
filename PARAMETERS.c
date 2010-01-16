//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define CUTBUTTONS 2

BOOL CALLBACK PARAMETERSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  HANDLE hCtlSMART;
  static  HANDLE hCtlSTARTAT;
  static  HANDLE hCtlSTARTTIME;
  static  HANDLE hCtlENDAT;
  static  HANDLE hCtlENDTIME;
  static  HANDLE hCtlRUNTYPES;
  static  HANDLE hCtlMINIMUMLEFTOVER;
  static  HANDLE hCtlIMPROVE;
  static  HANDLE hCtlPENALIZE_TRAVEL;
  static  HANDLE hCtlPENALIZE_MAKEUPTIME;
  static  HANDLE hCtlPENALIZE_OVERTIME;
  static  HANDLE hCtlPENALIZE_PAIDBREAKS;
  static  HANDLE hCtlPENALIZE_SPDOT;
  static  HANDLE hCtlPENALIZE_RUNSCUTAT;
  static  HANDLE hCtlPENALIZE_NODELIST;
  static  HANDLE hCtlCHECKRUNTYPE;
  static  int    cutRG[CUTBUTTONS];
  static  int    cutDirection;
  static  int    numReliefNodes;
  HCURSOR saveCursor;
  int     runtypesSelected[NUMRUNTYPES * NUMRUNTYPESLOTS];
  long    tempLong;
  char    tString[64];
  int     nI;
  int     nJ;
  int     nK;
  short   int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlSMART = GetDlgItem(hWndDlg, PARAMETERS_SMART);
      hCtlSTARTAT = GetDlgItem(hWndDlg, PARAMETERS_STARTAT);
      hCtlSTARTTIME = GetDlgItem(hWndDlg, PARAMETERS_STARTTIME);
      hCtlENDAT = GetDlgItem(hWndDlg, PARAMETERS_ENDAT);
      hCtlENDTIME = GetDlgItem(hWndDlg, PARAMETERS_ENDTIME);
      hCtlRUNTYPES = GetDlgItem(hWndDlg, PARAMETERS_RUNTYPES);
      hCtlMINIMUMLEFTOVER = GetDlgItem(hWndDlg, PARAMETERS_MINIMUMLEFTOVER);
      hCtlPENALIZE_TRAVEL = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_TRAVEL);
      hCtlPENALIZE_MAKEUPTIME = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_MAKEUPTIME);
      hCtlPENALIZE_OVERTIME = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_OVERTIME);
      hCtlPENALIZE_PAIDBREAKS = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_PAIDBREAKS);
      hCtlPENALIZE_SPDOT = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_SPDOT);
      hCtlPENALIZE_RUNSCUTAT = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_RUNSCUTAT);
      hCtlPENALIZE_NODELIST = GetDlgItem(hWndDlg, PARAMETERS_PENALIZE_NODELIST);
      hCtlCHECKRUNTYPE = GetDlgItem(hWndDlg, PARAMETERS_CHECKRUNTYPE);
      hCtlIMPROVE = GetDlgItem(hWndDlg, PARAMETERS_IMPROVE);
//
//  Set up the radio group
//
      cutRG[0] = PARAMETERS_STARTAT;
      cutRG[1] = PARAMETERS_ENDAT;
      nCwCheckRadioButtons(hWndDlg, cutRG, cutRG[CUTPARMS.cutRuns], CUTBUTTONS);
//
//  And the associated times
//
      SendMessage(hCtlSTARTTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)Tchar(CUTPARMS.startTime));
      SendMessage(hCtlENDTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)Tchar(CUTPARMS.endTime));
//
//  Disable the unselected button's time window
//
      EnableWindow(hCtlSTARTTIME, (CUTPARMS.cutRuns == 0));
      EnableWindow(hCtlENDTIME, (CUTPARMS.cutRuns == 1));
//
//  Set up the runtypes listbox
//
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
        {
          if((RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE) &&
              !(RUNTYPE[nI][nJ].flags & RTFLAGS_CREWONLY))
          {
            nK = SendMessage(hCtlRUNTYPES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
            SendMessage(hCtlRUNTYPES, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
            SendMessage(hCtlRUNTYPES, LB_SETSEL, (WPARAM)CUTPARMS.runtypes[nI][nJ], MAKELPARAM(nK, 0));
          }
        }
      }
//
//  Set the minimum leftover piece size
//
      SendMessage(hCtlMINIMUMLEFTOVER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(CUTPARMS.minLeftover));
//
//  Set the front/back button
//
      SendMessage(GetDlgItem(hWndDlg, CUTPARMS.cutDirection), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      cutDirection = CUTPARMS.cutDirection;
//
//  "Smart" lookahead
//
      SendMessage(hCtlSMART, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_SMART), (LPARAM)0);
//
//  Improvements
//
      SendMessage(hCtlIMPROVE, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE), (LPARAM)0);
//
//  Check runtype
//
      SendMessage(hCtlCHECKRUNTYPE, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_CHECKRUNTYPE), (LPARAM)0);
//
//  Penalty flags
//
      SendMessage(hCtlPENALIZE_TRAVEL, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_TRAVEL), (LPARAM)0);
      SendMessage(hCtlPENALIZE_MAKEUPTIME, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_MAKEUP), (LPARAM)0);
      SendMessage(hCtlPENALIZE_OVERTIME, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_OVERTIME), (LPARAM)0);
      SendMessage(hCtlPENALIZE_PAIDBREAKS, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_PAIDBREAKS), (LPARAM)0);
      SendMessage(hCtlPENALIZE_SPDOT, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_SPDOT), (LPARAM)0);
      SendMessage(hCtlPENALIZE_RUNSCUTAT, BM_SETCHECK,
            (WPARAM)(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT), (LPARAM)0);
//
//  Highlight the node list where applicable
//
      SERVICESKey1.number = m_ServiceNumber;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      SetUpLabelArray(tString, SERVICES.recordID, hCtlPENALIZE_NODELIST);
      numReliefNodes = SendMessage(hCtlPENALIZE_NODELIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
      if(SendMessage(hCtlPENALIZE_RUNSCUTAT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
      {
        EnableWindow(hCtlPENALIZE_NODELIST, TRUE);
        for(nI = 0; nI < numReliefNodes; nI++)
        {
          tempLong = SendMessage(hCtlPENALIZE_NODELIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          for(nJ = 0; nJ < CUTPARMS.numPenalizedNodes; nJ++)
          {
            if(tempLong == CUTPARMS.penalizedNODESrecordIDs[nJ])
            {
              SendMessage(hCtlPENALIZE_NODELIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nI);
              break;
            }
          }
        }
      }
//
//  Done initializing
//
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
//  PARAMETERS_PENALIZE_RUNSCUTAT
//
        case PARAMETERS_PENALIZE_RUNSCUTAT:
          if(SendMessage(hCtlPENALIZE_RUNSCUTAT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableWindow(hCtlPENALIZE_NODELIST, TRUE);
          }
          else
          {
            EnableWindow(hCtlPENALIZE_NODELIST, FALSE);
            SendMessage(hCtlPENALIZE_NODELIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          }
          break;
//
//  PARAMETERS_STARTAT
//  PARAMETERS_ENDAT
//
        case PARAMETERS_STARTAT:
        case PARAMETERS_ENDAT:
          nCwCheckRadioButtons(hWndDlg, cutRG, wmId, CUTBUTTONS);
          EnableWindow(hCtlSTARTTIME, (wmId == PARAMETERS_STARTAT));
          EnableWindow(hCtlENDTIME, (wmId == PARAMETERS_ENDAT));
          break;
//
//  PARAMETERS_FRONT
//  PARAMETERS_BACK
//  PARAMETERS_TMSDECIDES
//
        case PARAMETERS_FRONT:
        case PARAMETERS_BACK:
        case PARAMETERS_TMSDECIDES:
          cutDirection = wmId;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Setting_up_the_Runcut_Parameters);
          break;
//
//  IDOK
//
        case IDOK:
//
//  cutRuns and the time
//
          for(nI = 0; nI < CUTBUTTONS; nI++)
          {
            if(IsDlgButtonChecked(hWndDlg, cutRG[nI]))
            {
              CUTPARMS.cutRuns = nI;
              break;
            }
          }
//
//  startTime
//
          if(cutRG[CUTPARMS.cutRuns] == PARAMETERS_STARTAT)
          {
            SendMessage(hCtlSTARTTIME, WM_GETTEXT, TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_097, hCtlSTARTTIME);
              break;
            }
            CUTPARMS.startTime = cTime(tempString);
          }
//
//  endTime
//
          else
          {
            SendMessage(hCtlENDTIME, WM_GETTEXT, TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_098, hCtlENDTIME);
              break;
            }
            CUTPARMS.endTime = cTime(tempString);
          }
//
//  cutDirection
//
          CUTPARMS.cutDirection = cutDirection;
//
//  runtypes
//
          nI = SendMessage(hCtlRUNTYPES, LB_GETSELITEMS, (WPARAM)(NUMRUNTYPES * NUMRUNTYPESLOTS),
                (LPARAM)(int FAR *)runtypesSelected);
          if(nI == 0)
          {
            SendMessage(hCtlRUNTYPES, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
            nI = SendMessage(hCtlRUNTYPES, LB_GETSELITEMS,
                  (WPARAM)(NUMRUNTYPES * NUMRUNTYPESLOTS),
                  (LPARAM)(int FAR *)runtypesSelected);
          }
          memset(CUTPARMS.runtypes, FALSE, sizeof(CUTPARMS.runtypes));
          for(nJ = 0; nJ < nI; nJ++)
          {
            tempLong = SendMessage(hCtlRUNTYPES, LB_GETITEMDATA, (WPARAM)runtypesSelected[nJ], (LPARAM)0);
            CUTPARMS.runtypes[LOWORD(tempLong)][HIWORD(tempLong)] = TRUE;
          }
//
//  minLeftover
//
          SendMessage(hCtlMINIMUMLEFTOVER, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_099, hCtlMINIMUMLEFTOVER);
            break;
          }
          CUTPARMS.minLeftover = thhmm(tempString);
//
//  "Smart" lookahead
//
          if(SendMessage(hCtlSMART, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_SMART;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_SMART)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_SMART;
            }
          }
//
//  Improvements
//
          if(SendMessage(hCtlIMPROVE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_IMPROVE;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_IMPROVE;
            }
          }
//
//  Check Runtypes
//
          if(SendMessage(hCtlCHECKRUNTYPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_CHECKRUNTYPE;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_CHECKRUNTYPE)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_CHECKRUNTYPE;
            }
          }
//
//  Penalty flags
//
//  Travel
//
          if(SendMessage(hCtlPENALIZE_TRAVEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_TRAVEL;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_TRAVEL)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_TRAVEL;
            }
          }          
//
//  Makeup time
//
          if(SendMessage(hCtlPENALIZE_MAKEUPTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_MAKEUP;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_MAKEUP)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_MAKEUP;
            }
          }
//
//  Overtime
//
          if(SendMessage(hCtlPENALIZE_OVERTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_OVERTIME;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_OVERTIME)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_OVERTIME;
            }
          }
//
//  Paid breaks
//
          if(SendMessage(hCtlPENALIZE_PAIDBREAKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_PAIDBREAKS;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_PAIDBREAKS)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_PAIDBREAKS;
            }
          }
//
//  Spread overtime
//
          if(SendMessage(hCtlPENALIZE_SPDOT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_SPDOT;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_SPDOT)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_SPDOT;
            }
          }
//
//  Runs cut at
//
          if(SendMessage(hCtlPENALIZE_RUNSCUTAT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            CUTPARMS.flags |= CUTPARMSFLAGS_PEN_RUNSCUTAT;
          }
          else
          {
            if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT)
            {
              CUTPARMS.flags &= ~CUTPARMSFLAGS_PEN_RUNSCUTAT;
            }
          }
//
//  Save or clear the penalty nodes
//
          CUTPARMS.numPenalizedNodes = 0;
          if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT)
          {
            for(nI = 0; nI < numReliefNodes; nI++)
            {
              if(SendMessage(hCtlPENALIZE_NODELIST, LB_GETSEL, (WPARAM)nI, (LPARAM)0))
              {
                CUTPARMS.penalizedNODESrecordIDs[CUTPARMS.numPenalizedNodes] =
                      SendMessage(hCtlPENALIZE_NODELIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                CUTPARMS.numPenalizedNodes++;
              }
            }
          }
          saveCursor = SetCursor(hCursorWait);
          m_bWorkrulesChanged = TRUE;
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
