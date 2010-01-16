//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

typedef struct RENUMRUNSRDataStruct
{
  long runNumber;
  long time;
} RENUMRUNSRDataDef;

int sort_renumruns(const void *a, const void *b)
{
  RENUMRUNSRDataDef *pa, *pb;

  pa = (RENUMRUNSRDataDef *)a;
  pb = (RENUMRUNSRDataDef *)b;
  
  return(pa->time < pb->time ? -1 : pa->time > pb->time ? 1 : 0);
}


BOOL CALLBACK RENUMRUNSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlALLRUNS;
  static  HANDLE hCtlSPECIFICRUN;
  static  HANDLE hCtlDONTSORT;
  static  HANDLE hCtlSORTBYONTIME;
  static  HANDLE hCtlSORTBYOFFTIME;
  static  HANDLE hCtlRUNLIST;
  static  HANDLE hCtlSTARTAT;
  static  HANDLE hCtlINCREMENT;
  RENUMRUNSRDataDef RENUMRUNSRData[MAXRUNSINRUNLIST];
  HCURSOR saveCursor;
  long    newRunNumber;
  long    oldRunNumber;
  long    increment;
  BOOL    found;
  BOOL    bStartTime;
  int     selectedRuns[MAXRUNSINRUNLIST];
  int     numSelectedRuns;
  int     numRuns;
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
      pDI = NULL;
      if(specificRun == NO_RECORD)
      {
        pDI = (PDISPLAYINFO)lParam;
        if(pDI == NULL)
        {
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
      }
//
//  Set up handles to the controls
//
      hCtlALLRUNS = GetDlgItem(hWndDlg, RENUMRUNS_ALLRUNS);
      hCtlSPECIFICRUN = GetDlgItem(hWndDlg, RENUMRUNS_SPECIFICRUN);
      hCtlRUNLIST = GetDlgItem(hWndDlg, RENUMRUNS_RUNLIST);
      hCtlDONTSORT = GetDlgItem(hWndDlg, RENUMRUNS_DONTSORT);
      hCtlSORTBYONTIME = GetDlgItem(hWndDlg, RENUMRUNS_SORTBYONTIME);
      hCtlSORTBYOFFTIME = GetDlgItem(hWndDlg, RENUMRUNS_SORTBYOFFTIME);
      hCtlSTARTAT = GetDlgItem(hWndDlg, RENUMRUNS_STARTAT);
      hCtlINCREMENT = GetDlgItem(hWndDlg, RENUMRUNS_INCREMENT);
//
//  Initialize the radio buttons
//
      SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDONTSORT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Did we come in from: ChildWnd - specificRun = NO_RECORD
//                       MANCUT   - specificRun = 0
//
      SendMessage(hCtlRUNLIST, LB_RESETCONTENT, (WPARAM)NULL, (LPARAM)0);
      numRuns = 0;
//
//  MANCUT / CutRuns
//
      if(pDI == NULL)
      {
        for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
        {
          if(RUNLISTDATA[nI].runNumber != NO_RECORD && !RUNLISTDATA[nI].frozenFlag)
          {
            sprintf(tempString, "%7ld", RUNLISTDATA[nI].runNumber);
            nJ = SendMessage(hCtlRUNLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlRUNLIST, LB_SETITEMDATA, (WPARAM)nJ, RUNLISTDATA[nI].runNumber);
            if(RUNLISTDATA[nI].runNumber == specificRun)
            {
              SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICRUN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlRUNLIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nJ);
              SendMessage(hCtlRUNLIST, LB_SETCARETINDEX, (WPARAM)nJ, (LPARAM)FALSE);
              SetFocus(hCtlSTARTAT);
            }
            numRuns++;
          }
        }
      }
//
//  ChildWnd
//
      else
      {
        EnableWindow(hCtlDONTSORT, FALSE);
        EnableWindow(hCtlSORTBYONTIME, FALSE);
        EnableWindow(hCtlSORTBYOFFTIME, FALSE);
        RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
        RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
        RUNSKey1.runNumber = NO_RECORD;
        RUNSKey1.pieceNumber = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        while(rcode2 == 0 &&
              RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
              RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
        {
          if(RUNS.pieceNumber == 1)
          {
            sprintf(tempString, "%7ld", RUNS.runNumber);
            nI = SendMessage(hCtlRUNLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlRUNLIST, LB_SETITEMDATA, (WPARAM)nI, RUNS.runNumber);
            numRuns++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        }
      }
      if(numRuns == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_121, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the start at and increment
//
      SendMessage(hCtlSTARTAT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"1");
      SendMessage(hCtlINCREMENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"1");
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
//  ALLRUNS and SPECIFICRUNS
//
        case RENUMRUNS_RUNLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLRUNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICRUN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case RENUMRUNS_ALLRUNS:
          SendMessage(hCtlRUNLIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;

        case RENUMRUNS_SPECIFICRUN:
          SendMessage(hCtlRUNLIST, LB_SETCURSEL, (WPARAM)TRUE, (LPARAM)0);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Renumbering_Runs_from_the_Roster_Table);
          break;
//
//  IDOK
//
        case IDOK:
//
// The new run number can't be blank, or less than or equal to zero
//
          GetDlgItemText(hWndDlg, RENUMRUNS_STARTAT, tempString, TEMPSTRING_LENGTH);
          if(strcmp(tempString, "") == 0 || (newRunNumber = atol(tempString)) <= 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_155, hCtlSTARTAT);
            break;
          }
//
//  If the increment is blank, zero, or less than zero, set it to 1
//
          GetDlgItemText(hWndDlg, RENUMRUNS_INCREMENT, tempString, TEMPSTRING_LENGTH);
          if(strcmp(tempString, "") == 0 || (increment = atol(tempString)) <= 0)
            increment = 1;
//
//  If specific run is selected, at least one of the entries
//  in the runs list box has to be selected
//
          if(specificRun == NO_RECORD &&
                SendMessage(hCtlSPECIFICRUN, BM_GETCHECK, (WPARAM)0, (LPARAM)0) &&
                SendMessage(hCtlRUNLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_123, hCtlRUNLIST);
            break;
          }
//
//  If none are highlighted, highlight 'em all.
//
          if(SendMessage(hCtlRUNLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
            SendMessage(hCtlRUNLIST, LB_SETSEL, (WPARAM)TRUE, MAKELPARAM(-1, 0));
          numSelectedRuns = SendMessage(hCtlRUNLIST, LB_GETSELITEMS,
                (WPARAM)MAXRUNSINRUNLIST, (LPARAM)(int FAR *)selectedRuns);
//
//  Deal with entry from ChildWnd/Don't Sort first
//
          if(SendMessage(hCtlDONTSORT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            saveCursor = SetCursor(hCursorWait);
//
//  Loop through the runs and make the new assignment
//
            for(nI = 0; nI < numSelectedRuns; nI++)
            {
              newRunNumber += (nI == 0 ? 0 : increment);
//  
//  ChildWnd: Make sure we're not on top of an existing run number
//
              if(specificRun == NO_RECORD)
              {
                RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                RUNSKey1.runNumber = newRunNumber;
                RUNSKey1.pieceNumber = 1;
                rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                while(rcode2 == 0)
                {
                  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                  RUNSKey1.runNumber = newRunNumber += increment;
                  RUNSKey1.pieceNumber = 1;
                  rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                }
              }
//
//  MANCUT: Make sure we're not on top of an existing run number
//
              else
              {
                do
                {
                  for(found = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
                  {
                    if(RUNLISTDATA[nJ].runNumber != newRunNumber)
                      continue;
                    found = TRUE;
                    break;
                  }
                  if(found)
                    newRunNumber += increment;
                } while(found);
              }
//
//  Ok, we know here that we have a unique run number
//
              oldRunNumber = SendMessage(hCtlRUNLIST, LB_GETITEMDATA, (WPARAM)selectedRuns[nI], (LPARAM)0);
//
//  Renumber: ChildWnd
//
              if(specificRun == NO_RECORD)
              {
                RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                RUNSKey1.runNumber = oldRunNumber;
                RUNSKey1.pieceNumber = NO_RECORD;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                while(rcode2 == 0 &&
                      RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
                      RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                      RUNS.runNumber == oldRunNumber)
                {
                  RUNS.runNumber = newRunNumber;
                  btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                  RUNSKey1.runNumber = oldRunNumber;
                  RUNSKey1.pieceNumber = NO_RECORD;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                }
              }
//
//  Renumber: MANCUT
//
              else
              {
                for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
                {
                  if(RUNLISTDATA[nJ].runNumber == oldRunNumber)
                  {
                    RUNLISTDATA[nJ].runNumber = newRunNumber;
                    break;
                  }
                }
                for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
                {
                  if(m_pRELIEFPOINTS[nJ].start.runNumber == oldRunNumber)
                    m_pRELIEFPOINTS[nJ].start.runNumber = newRunNumber;
                  if(m_pRELIEFPOINTS[nJ].end.runNumber == oldRunNumber)
                    m_pRELIEFPOINTS[nJ].end.runNumber = newRunNumber;
                }
              }
            }
            SetCursor(saveCursor);
          }
//
//  Sort by...
//
          else
          {
            bStartTime = SendMessage(hCtlSORTBYONTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
            for(nI = 0; nI < numSelectedRuns; nI++)
            {
              RENUMRUNSRData[nI].runNumber =
                    SendMessage(hCtlRUNLIST, LB_GETITEMDATA, (WPARAM)selectedRuns[nI], (LPARAM)0);
              for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
              {
                if(RUNLIST[nJ].runNumber == RENUMRUNSRData[nI].runNumber)
                {
                  if(bStartTime)
                    RENUMRUNSRData[nI].time = RUNLIST[nJ].run.piece[0].fromTime;
                  else
                    RENUMRUNSRData[nI].time = RUNLIST[nJ].run.piece[RUNLIST[nJ].run.numPieces - 1].toTime;
                  break;
                }
              }
            }
            qsort((void *)RENUMRUNSRData, numSelectedRuns, sizeof(RENUMRUNSRDataDef), sort_renumruns);
//
//  Loop through the runs and make the new assignment
//
            for(nI = 0; nI < numSelectedRuns; nI++)
            {
              newRunNumber += (nI == 0 ? 0 : increment);
              do
              {
                for(found = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
                {
                  if(RUNLISTDATA[nJ].runNumber != newRunNumber)
                    continue;
                  found = TRUE;
                  break;
                }
                if(found)
                  newRunNumber += increment;
              } while(found);
//
//  Ok, we know here that we have a unique run number
//
              oldRunNumber = RENUMRUNSRData[nI].runNumber;
              for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
              {
                if(RUNLISTDATA[nJ].runNumber == oldRunNumber)
                {
                  RUNLISTDATA[nJ].runNumber = newRunNumber;
                  break;
                }
              }
              for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
              {
                if(m_pRELIEFPOINTS[nJ].start.runNumber == oldRunNumber)
                  m_pRELIEFPOINTS[nJ].start.runNumber = newRunNumber;
                if(m_pRELIEFPOINTS[nJ].end.runNumber == oldRunNumber)
                  m_pRELIEFPOINTS[nJ].end.runNumber = newRunNumber;
              }
            }
          }
//
//  Reset m_GlobalRunNumber if we came in via MANCUT
//
//          if(specificRun == 0)
//          {
//            m_GlobalRunNumber = max(m_GlobalRunNumber, newRunNumber) + 1;
//          }
          m_GlobalRunNumber = max(m_GlobalRunNumber, newRunNumber) + 1;
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
