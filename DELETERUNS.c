//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static  PDISPLAYINFO pDI;
static  HANDLE hCtlALL;
static  HANDLE hCtlALLILLEGAL;
static  HANDLE hCtlFROMTITLE;
static  HANDLE hCtlFROM;
static  HANDLE hCtlTO;
static  HANDLE hCtlSPECIFICRUNS;
static  HANDLE hCtlRUNSOFRUNTYPE;
static  HANDLE hCtlLISTBOX;
static  HWND   hWnd;
static  int    selection;

/* Replace this with EM_
void DELETERUNSEvent(VBXEVENT FAR *lpEvent)
{
  switch(lpEvent->ID)
  {
    case DELETERUNS_FROM:
    case DELETERUNS_TO:
      switch(lpEvent->EventIndex)
      {
        case Event_MhIntInput_Change:
          SendDlgItemMessage(hWnd, selection, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlFROMTITLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          selection = DELETERUNS_FROMTITLE;
          break;
      }
      break;
  }
}
*/
BOOL CALLBACK DELETERUNSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  int     nI;
  int     nJ;
  int     nK;
  int     rcode2;
  long    fromRunNumber;
  long    toRunNumber;
  BOOL    deleteFromFirst;
  BOOL    deleteToLast;
  BOOL    bFinished;
  HCURSOR saveCursor;
  int     numSelected;
  long    thisRun;
  long    recordID;
  int     numPieces;
  long    runType;
  long    runNumber;
  short int wmId;

  int    *pSelections = NULL;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hWnd = hWndDlg;
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlALL = GetDlgItem(hWndDlg, DELETERUNS_ALL);
      hCtlALLILLEGAL = GetDlgItem(hWndDlg, DELETERUNS_ALLILLEGAL);
      hCtlFROMTITLE = GetDlgItem(hWndDlg, DELETERUNS_FROMTITLE);
      hCtlFROM = GetDlgItem(hWndDlg, DELETERUNS_FROM);
      hCtlTO = GetDlgItem(hWndDlg, DELETERUNS_TO);
      hCtlSPECIFICRUNS = GetDlgItem(hWndDlg, DELETERUNS_SPECIFICRUNS);
      hCtlRUNSOFRUNTYPE = GetDlgItem(hWndDlg, DELETERUNS_RUNSOFRUNTYPE);
      hCtlLISTBOX = GetDlgItem(hWndDlg, DELETERUNS_LISTBOX);
//
//  Default to DELETERUNS_ALL
//
      selection = DELETERUNS_ALL;
      SendMessage(hCtlALL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
//  Radio buttons
//
//  DELETERUNS_ALL
//  DELETERUNS_ALLILLEGAL
//  DELETERUNS_FROMTITLE
//  DELETERUNS_SPECIFICRUNS
//  DELETERUNS_RUNSOFRUNTYPE
//
        case DELETERUNS_ALL:
        case DELETERUNS_ALLILLEGAL:
        case DELETERUNS_FROMTITLE:
        case DELETERUNS_SPECIFICRUNS:
        case DELETERUNS_RUNSOFRUNTYPE:
          SendDlgItemMessage(hWndDlg, selection, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          selection = wmId;
          SendMessage(hCtlLISTBOX, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlLISTBOX, (wmId == DELETERUNS_SPECIFICRUNS || wmId == DELETERUNS_RUNSOFRUNTYPE));
//
//  If he clicks FROMTITLE, set the focus to FROM
//
          if(wmId == DELETERUNS_FROMTITLE)
            SetFocus(hCtlFROM);
//
//  Specific runs - get all the runs on this service
//
          else if(wParam == DELETERUNS_SPECIFICRUNS)
          {
            RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
            RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            RUNSKey1.runNumber = NO_RECORD;
            RUNSKey1.pieceNumber = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            while(rcode2 == 0 &&
                  RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
                  RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
            {
              ltoa(RUNS.runNumber, tempString, 10);
              if(RUNS.pieceNumber == 1)
              {
                nK = SendMessage(hCtlLISTBOX, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nK, RUNS.runNumber);
              }
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            }
            SetFocus(hCtlLISTBOX);
          }
//
//  Runs of run type - display the active run type names
//
          else if(wParam == DELETERUNS_RUNSOFRUNTYPE)
          {
            for(nI = 0; nI < NUMRUNTYPES; nI++)
              for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
                if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
                {
                  nK = SendMessage(hCtlLISTBOX, LB_ADDSTRING,
                        (WPARAM)0, (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
                  SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
                }
            SetFocus(hCtlLISTBOX);
          }
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
        case IDOK:
          bFinished = TRUE;
          saveCursor = SetCursor(hCursorWait);
//
//  Process the selection
//
          switch(selection)
          {
//
//  All
//
            case DELETERUNS_ALL:
              RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
              RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
              RUNSKey1.runNumber = NO_RECORD;
              RUNSKey1.pieceNumber = NO_RECORD;
              rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              while(rcode2 == 0 &&
                    RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
                    RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
              {
                btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }
              break;
//
//  All illegal runs
//
            case DELETERUNS_ALLILLEGAL:
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
                  recordID = RUNS.recordID;
                  runNumber = RUNS.runNumber;
                  numPieces = GetRunElements(hWnd, &RUNS, &PROPOSEDRUN, &COST, TRUE);
                  RUNSKey0.recordID = recordID;
                  btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                  btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                  btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                  if(RUNSVIEW[numPieces - 1].payTime == 0L)
                  {
                    for(nI = 0; nI < numPieces; nI ++)
                    {
                      btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                      btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                    }
                  }
                }
                RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                RUNSKey1.runNumber = runNumber + 1;
                RUNSKey1.pieceNumber = NO_RECORD;
                rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }
              break;
//
//  From run to run
//
            case DELETERUNS_FROMTITLE:
              SendMessage(hCtlFROM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              if((deleteFromFirst = strcmp(tempString, "") == 0) == FALSE)
                fromRunNumber = atol(tempString);
              SendMessage(hCtlTO, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              if((deleteToLast = strcmp(tempString, "") == 0) == FALSE)
                toRunNumber = atol(tempString);
              if(deleteFromFirst && deleteToLast)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_141, (HANDLE)NULL);
                bFinished = FALSE;
                break;
              }
              if(!deleteFromFirst && !deleteToLast && fromRunNumber > toRunNumber)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_142, (HANDLE)NULL);
                bFinished = FALSE;
                break;
              }
              RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
              RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
              RUNSKey1.runNumber = (deleteFromFirst ? NO_RECORD : fromRunNumber);
              RUNSKey1.pieceNumber = NO_RECORD;
              rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              while(rcode2 == 0 &&
                    RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
                    RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
              {
                if(!deleteToLast && RUNS.runNumber > toRunNumber)
                  break;
                btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }
              break;
//
//  Specific runs and specific runtypes
//
            case DELETERUNS_SPECIFICRUNS:
            case DELETERUNS_RUNSOFRUNTYPE:
              numSelected = SendMessage(hCtlLISTBOX, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
              if(numSelected == 0 || numSelected == LB_ERR)
              {
                break;
              }
              pSelections = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * numSelected); 
              if(pSelections == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                break;
              }
              SendMessage(hCtlLISTBOX, LB_GETSELITEMS, (WPARAM)numSelected, (LPARAM)pSelections);
              if(selection == DELETERUNS_SPECIFICRUNS)
              {
                for(nI = 0; nI < numSelected; nI++)
                {
                  thisRun = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)pSelections[nI], (LPARAM)0);
                  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                  RUNSKey1.runNumber = thisRun;
                  RUNSKey1.pieceNumber = NO_RECORD;
                  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                  while(rcode2 == 0 &&
                        RUNS.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID &&
                        RUNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                        RUNS.runNumber == thisRun)
                  {
                    btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                  }
                }
              }
              else  // DELETERUNS_RUNSOFRUNTYPE
              {
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
                    recordID = RUNS.recordID;
                    runNumber = RUNS.runNumber;
                    numPieces = GetRunElements(hWnd, &RUNS, &PROPOSEDRUN, &COST, TRUE);
                    RUNSKey0.recordID = recordID;
                    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                    btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                    for(nI = 0; nI < numSelected; nI++)
                    {
                      runType = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)pSelections[nI], (LPARAM)0);
                      if(RUNSVIEW[0].runType == runType)
                      {
                        for(nJ = 0; nJ < numPieces; nJ ++)
                        {
                          btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                          btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                        }
                        break;
                      }
                    }
                  }
                  RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
                  RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                  RUNSKey1.runNumber = runNumber + 1;
                  RUNSKey1.pieceNumber = NO_RECORD;
                  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                }
              }
              TMSHeapFree(pSelections);
              break;
          }
          SetCursor(saveCursor);
          if(bFinished)
            EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
