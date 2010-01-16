//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK RENRUNSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlTARGET;
  static  HANDLE hCtlSOURCE;
  HCURSOR saveCursor;
  BOOL    reliefRun;
  long    targetDivision;
  long    targetService;
  long    sourceService;
  long    targetRunNumber;
  long    sourceRunNumber;
  long    dailyRunNumbers[ROSTER_MAX_DAYS];
  int     serviceIndex;
  int     rcode2;
  int     nI;
  int     nJ;
  int     nK;
  short int wmId;
//
//  m_bAdjustRRLIST == TRUE  means call from ROSTERMsgProc
//                  == FALSE means call from ChildWnd
//
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
//  Set up the handles to the controls
//
      hCtlTARGET = GetDlgItem(hWndDlg, RENRUNS_TARGET);
      hCtlSOURCE = GetDlgItem(hWndDlg, RENRUNS_SOURCE);
//
//  Set up the two combo-boxes
//
      if(SetUpServiceList(hWndDlg, RENRUNS_TARGET, pDI->fileInfo.serviceRecordID) == 0)
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      else
        SetUpServiceList(hWndDlg, RENRUNS_SOURCE, pDI->fileInfo.serviceRecordID);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Renumbering_Runs);
          break;

        case IDOK:
//
//  Make sure that he's both selected services and that they're not the same
//
//  Target
//
          if((nI = SendMessage(hCtlTARGET, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_011, hCtlTARGET);
            break;
          }
          targetService = SendMessage(hCtlTARGET, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Source
//
          if((nI = SendMessage(hCtlSOURCE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_011, hCtlSOURCE);
            break;
          }
          sourceService = SendMessage(hCtlSOURCE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Target == Source?
//
          if(targetService == sourceService)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_166, hCtlSOURCE);
            break;
          }
//
//  Determine the serviceIndex so we can get into m_pRRLIST
//
          if(m_bAdjustRRLIST)
          {
            serviceIndex = NO_RECORD;
            nK = 0;
            while(m_pRRLIST[nK].SERVICESrecordID != NO_RECORD)
            {
              if(targetService == m_pRRLIST[nK].SERVICESrecordID)
              {
                serviceIndex = nK;
                break;
              }
            }
            if(serviceIndex == NO_RECORD)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_167, hCtlTARGET);
              break;
            }
          }
//
//  Got target and source.  Loop through all the service days looking for the target
//
          saveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
            if(ROSTERPARMS.serviceDays[nI] == targetService)
            {
              for(nJ = 0; nJ < m_numCOMBINED; nJ++)
              {
//
//  Only do the "Rostered" runs
//
                if(!m_pCOMBINED[nJ].frozen)
                  continue;
//
//  Find the daily run numbers in this COMBINED record
//
                for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
                {
                  if(ROSTERPARMS.serviceDays[nK] != sourceService ||
                        m_pCOMBINED[nJ].RUNSrecordID[nK] == NO_RECORD)
                    dailyRunNumbers[nK] = NO_RECORD;
                  else
                  {
                    RUNSKey0.recordID = m_pCOMBINED[nJ].RUNSrecordID[nK];
                    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                    dailyRunNumbers[nK] = RUNS.runNumber;
                  }
                }
//
//  If the run number on the source service differs just once, then it's
//  a relief run and the target service run record doesn't get renumbered.
//
                reliefRun = FALSE;
                sourceRunNumber = NO_RECORD;
                for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
                  if(dailyRunNumbers[nK] != NO_RECORD &&
                        ROSTERPARMS.serviceDays[nK] == sourceService)
                  {
                    if(sourceRunNumber == NO_RECORD)
                      sourceRunNumber = dailyRunNumbers[nK];
                    else
                    {
                      if(sourceRunNumber != dailyRunNumbers[nK])
                      {
                        reliefRun = TRUE;
                        break;
                      }
                    }
                  }
                if(reliefRun || sourceRunNumber == NO_RECORD)
                  continue;
//
//  Ok, we know the source runNumber and we know the target record id - renumber now.
//
                RUNSKey0.recordID = m_pCOMBINED[nJ].RUNSrecordID[nI];
                btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                targetDivision = RUNS.DIVISIONSrecordID;
                targetRunNumber = RUNS.runNumber;
                RUNSKey1.DIVISIONSrecordID = targetDivision;
                RUNSKey1.SERVICESrecordID = targetService;
                RUNSKey1.runNumber = targetRunNumber;
                RUNSKey1.pieceNumber = NO_RECORD;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                while(rcode2 == 0 &&
                      RUNS.DIVISIONSrecordID == targetDivision &&
                      RUNS.SERVICESrecordID == targetService &&
                      RUNS.runNumber == targetRunNumber)
                {
                  RUNS.runNumber = sourceRunNumber;
                  btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                  RUNSKey1.SERVICESrecordID = targetService;
                  RUNSKey1.runNumber = targetRunNumber;
                  RUNSKey1.pieceNumber = NO_RECORD;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                }
//
//  And fix up m_pRRLIST too
//
                if(m_bAdjustRRLIST)
                {
                  for(nK = 0; nK < m_pRRLIST[serviceIndex].numRuns; nK++)
                    if(m_pRRLIST[serviceIndex].pData[nK].RUNSrecordID == m_pCOMBINED[nJ].RUNSrecordID[nI])
                    {
                      m_pRRLIST[serviceIndex].pData[nK].runNumber = sourceRunNumber;
                      break;
                    }
                }
              }
            }
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
