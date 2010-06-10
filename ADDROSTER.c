//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define TABSET       1

int CompressCOMBINED(void)
{
  BOOL bFound;
  int  nI;
  int  nJ;
  int  iDest;

  for(iDest = 0, nI = 0; nI < m_numCOMBINED; nI++)
  {
    for(bFound = FALSE, nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      if(m_pCOMBINED[nI].RUNSrecordID[nJ] != NO_RECORD)
      {
        bFound = TRUE;
        break;
      }
    }
    if(bFound)
    {
      if(iDest != nI)
        memcpy(&m_pCOMBINED[iDest], &m_pCOMBINED[nI], sizeof(m_pCOMBINED[0]));
      iDest++;
    }
  }

	return iDest;
}

typedef struct SELECTIONSStruct
{
  long divisionRecordID;
  long runtype;
} SELECTIONSDef;

typedef struct BIDDINGStruct
{
  DRIVERSDef     DRIVERS;
  PDISPLAYINFO   pDI;
  int            numSelections;
  SELECTIONSDef  SELECTIONS[NUMRUNTYPES * NUMRUNTYPESLOTS];
} BIDDINGDef;

BOOL CALLBACK ADDROSTERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlALLDIVISIONS;
  static  HANDLE hCtlORDIVISION;
  static  HANDLE hCtlDIVISIONLIST;
  static  HANDLE hCtlALLRUNTYPES;
  static  HANDLE hCtlORRUNTYPE;
  static  HANDLE hCtlRUNTYPELIST;
  static  HANDLE hCtlSELECTIONS;
  static  HANDLE hCtlIDREMOVE;
  static  HANDLE hCtlIDAUTOMATIC;
  static  HANDLE hCtlIDBIDDING;
  static  BOOL   bAllDivisions;
  static  BOOL   bAllRuntypes;
  static  BOOL   bUseDriverTypes;
  static  int    tabPos[TABSET] = {sizeof(DIVISIONS.name) + 1};
  PROPOSEDRUNDef PROPOSEDRUN;
  BIDDINGDef     BIDDING;
  HCURSOR saveCursor;
  COSTDef COST;
  WORD    DlgWidthUnits;
  BOOL    bFound;
  BOOL    bGotNext;
  BOOL    bStopBidding;
  long    selectedRuntype;
  long    onTime;
  long    offTime;
  long    RUNSrecordID;
  long    runNumber;
  long    RGRPROUTESrecordID;
  long    currentCombined;
  long    serviceRecordID;
  char    listboxString[DIVISIONS_NAME_LENGTH + sizeof(RUNTYPE[0][0].localName) + 2];
  char    dummy[256];
  int     adjustedTabPos[TABSET];
  int     nRc;
  int     pieceNumber;
  int     serviceIndex;
  int     numServices;
  int     numDivisions;
  int     numDriverTypes;
  int     rcode2;
  int     nI;
  int     nJ;
  int     nK;
  int     nL;
  int     nM;
  int     numSelectedDivisions;
  int     nType;
  int     nSlot;
  short int wmId;
  short int wmEvent;
  long flags;

  long  *pDriverTypes = NULL;

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
      hCtlALLDIVISIONS = GetDlgItem(hWndDlg, ADDROSTER_ALLDIVISIONS);
      hCtlORDIVISION = GetDlgItem(hWndDlg, ADDROSTER_ORDIVISION);
      hCtlDIVISIONLIST = GetDlgItem(hWndDlg, ADDROSTER_DIVISIONLIST);
      hCtlALLRUNTYPES = GetDlgItem(hWndDlg, ADDROSTER_ALLRUNTYPES);
      hCtlORRUNTYPE = GetDlgItem(hWndDlg, ADDROSTER_ORRUNTYPE);
      hCtlRUNTYPELIST = GetDlgItem(hWndDlg, ADDROSTER_RUNTYPELIST);
      hCtlSELECTIONS = GetDlgItem(hWndDlg, ADDROSTER_SELECTIONS);
      hCtlIDREMOVE = GetDlgItem(hWndDlg, IDREMOVE);
      hCtlIDAUTOMATIC = GetDlgItem(hWndDlg, IDAUTOMATIC);
      hCtlIDBIDDING = GetDlgItem(hWndDlg, IDBIDDING);
//
//  Set up the tab stops for ADDROSTER_SELECTIONS
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nJ = 0; nJ < TABSET; nJ++)
      {
        adjustedTabPos[nJ] = (DlgWidthUnits * tabPos[nJ] * 2);
      }
      SendMessage(hCtlSELECTIONS, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  Show the "destination"
//
      flags = PLACEMENT_DIVISION;
      DisplayPlacement(hWndDlg, ADDROSTER_DESTINATION, pDI, flags);
//
//  Set up the All Divisions check box
//
      SendMessage(hCtlALLDIVISIONS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Set up the divisions listbox
//
      numDivisions = 0;
      SendMessage(hCtlDIVISIONLIST, LB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
      rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, DIVISIONS.name, sizeof(DIVISIONS.name));
        trim(tempString, sizeof(DIVISIONS.name));
        nI = (int)SendMessage(hCtlDIVISIONLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlDIVISIONLIST, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)DIVISIONS.recordID);
        numDivisions++;
        rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
      }
      if(numDivisions == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_080, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the All Runtypes check box
//
      SendMessage(hCtlALLRUNTYPES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Set up the runtypes combobox
//
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
        {
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
          {
            nK = (int)SendMessage(hCtlRUNTYPELIST, LB_ADDSTRING, (WPARAM)0,
                  (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
            SendMessage(hCtlRUNTYPELIST, LB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
          }
        }
      }
//
//  Assuming not bidding by driver type
//
      bUseDriverTypes = FALSE;
      pDriverTypes = NULL;
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
//  Division stuff
//
        case ADDROSTER_DIVISIONLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLDIVISIONS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORDIVISION, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDROSTER_ALLDIVISIONS:
          SendMessage(hCtlDIVISIONLIST, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDROSTER_ORDIVISION:
          SendMessage(hCtlDIVISIONLIST, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Runtype stuff
//
        case ADDROSTER_RUNTYPELIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLRUNTYPES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLRUNTYPES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORRUNTYPE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDROSTER_ALLRUNTYPES:
          SendMessage(hCtlRUNTYPELIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;

        case ADDROSTER_ORRUNTYPE:
          SendMessage(hCtlRUNTYPELIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)0);
          break;
//
//  ADDROSTER_SELECTIONS:
//
        case ADDROSTER_SELECTIONS:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              EnableWindow(hCtlIDREMOVE, TRUE);
              break;
          }
          break;
//
//  IDREMOVE
//
        case IDREMOVE:
          nI = (int)SendMessage(hCtlSELECTIONS, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlSELECTIONS, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          if(SendMessage(hCtlSELECTIONS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) <= 0)
          {
            EnableWindow(hCtlIDAUTOMATIC, FALSE);
            EnableWindow(hCtlIDBIDDING, FALSE);
          }
          EnableWindow(hCtlIDREMOVE, FALSE);
          break;
//
//  IDADD
//
        case IDADD:
          bAllDivisions = SendMessage(hCtlALLDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(bAllDivisions)
            numSelectedDivisions = SendMessage(hCtlDIVISIONLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          else
            numSelectedDivisions = 1;
          bAllRuntypes = SendMessage(hCtlALLRUNTYPES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(bAllRuntypes)
          {
            nJ = (int)SendMessage(hCtlRUNTYPELIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            for(nI = 0; nI < nJ; nI++)
            {
              SendMessage(hCtlRUNTYPELIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nI);
            }
          }
//
//  Loop through the selected divisions and runtypes
//  to build the contents of the SELECTIONS list box
//
          for(nI = 0; nI < numSelectedDivisions; nI++)
          {
            if(bAllDivisions)
              DIVISIONSKey0.recordID = SendMessage(hCtlDIVISIONLIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            else
            {
              nJ = (int)SendMessage(hCtlDIVISIONLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              DIVISIONSKey0.recordID = SendMessage(hCtlDIVISIONLIST, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
            }
            btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
            strncpy(tempString, DIVISIONS.name, sizeof(DIVISIONS.name));
            trim(tempString, sizeof(DIVISIONS.name));
            strcat(tempString, "\t");
            nJ = (int)SendMessage(hCtlRUNTYPELIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            for(nK = 0; nK < nJ; nK++)
            {
              if(SendMessage(hCtlRUNTYPELIST, LB_GETSEL, (WPARAM)nK, (LPARAM)0))
              {
                strcpy(listboxString, tempString);
                SendMessage(hCtlRUNTYPELIST, LB_GETTEXT, (WPARAM)nK, (LONG)(LPSTR)szarString);
                strcat(listboxString, szarString);
                nL = (int)SendMessage(hCtlSELECTIONS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)listboxString);
                SendMessage(hCtlSELECTIONS, LB_SETITEMDATA, (WPARAM)nL,
                      SendMessage(hCtlRUNTYPELIST, LB_GETITEMDATA, nK, (LPARAM)0));
              }
            }
          }
          EnableWindow(hCtlIDAUTOMATIC, TRUE);
          EnableWindow(hCtlIDBIDDING, TRUE);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
//
//  Free allocated memory
//
/*          if(bUseDriverTypes)
          {
            TMSHeapFree(pDriverTypes);
          }
          nI = 0;
          if(m_pRRLIST != NULL)
          {
            while(m_pRRLIST[nI].SERVICESrecordID != NO_RECORD)
            {
              TMSHeapFree(m_pRRLIST[nI].pData);
              nI++;
            }
            TMSHeapFree(m_pRRLIST);
          }
          TMSHeapFree(m_pCOMBINED);
*/
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Roster_Table);
          break;
//
//  IDAUTOMATIC button
//
        case IDAUTOMATIC:
//
//  Allocate space for m_pRRLIST
//
          rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
          if(rcode2 != 0 || BSTAT.numRecords == 0)
          {
            TMSError(NULL, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
            break;
          }
          m_pRRLIST = (RRLISTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RRLISTDef) * (BSTAT.numRecords + 1)); 
          if(m_pRRLIST == NULL)
          {
            AllocationError(__FILE__, __LINE__, FALSE);
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
            break;
          }
//
//  Build the list of runs for ROSTER
//
          saveCursor = SetCursor(hCursorWait);
          m_bEstablishRUNTIMES = TRUE;
//
//  Build a list of services and allocate space for the run data
//
          numServices = 0;
          rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
          while(rcode2 == 0)
          {
            m_pRRLIST[numServices].SERVICESrecordID = SERVICES.recordID;
            m_pRRLIST[numServices].maxRuns = 128;
            m_pRRLIST[numServices].pData =
                  (RRDATADef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RRDATADef) * m_pRRLIST[numServices].maxRuns); 
            if(m_pRRLIST[numServices].pData == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            m_pRRLIST[numServices++].numRuns = 0;
            rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
          }
          m_pRRLIST[numServices].SERVICESrecordID = NO_RECORD;
//
//  Loop through the SELECTIONS list box
//
          nI = (int)SendMessage(hCtlSELECTIONS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          for(nJ = 0; nJ < nI; nJ++)
          {
            SendMessage(hCtlSELECTIONS, LB_GETTEXT, (WPARAM)nJ, (LONG)(LPSTR)tempString);
            strcpy(szarString, strtok(tempString, "\t"));
            trim(szarString, sizeof(DIVISIONS.name));
            rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
            bFound = FALSE;
            while(rcode2 == 0)
            {
              strncpy(tempString, DIVISIONS.name, sizeof(DIVISIONS.name));
              trim(tempString, sizeof(DIVISIONS.name));
              if((bFound = strcmp(tempString, szarString) == 0) == TRUE)
              {
                break;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
            }
            if(!bFound)
            {
              continue;
            }
            selectedRuntype = SendMessage(hCtlSELECTIONS, LB_GETITEMDATA, nJ, (LPARAM)0);
            nType = LOWORD(selectedRuntype);
            nSlot = HIWORD(selectedRuntype);
            if(nType == NO_RECORD || nSlot == NO_RECORD)
            {
              continue;
            }
//
//  Crew only runs?
//
            if(RUNTYPE[nType][nSlot].flags & RTFLAGS_CREWONLY)
            {
              rcode2 = btrieve(B_GETFIRST, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
              while(rcode2 == 0)
              {
                bGotNext = FALSE;
                if(CREWONLY.DIVISIONSrecordID == DIVISIONS.recordID &&
                      CREWONLY.cutAsRuntype == selectedRuntype)
                {
//
//  Cost the run so we can fill in the rest of the RRLIST structure
//
                  for(serviceIndex = NO_RECORD, nK = 0; nK < numServices; nK++)
                  {
                    if(CREWONLY.SERVICESrecordID == m_pRRLIST[nK].SERVICESrecordID)
                    {
                      serviceIndex = nK;
                      break;
                    }
                  }
                  if(serviceIndex != NO_RECORD)
                  {
                    pieceNumber = 0;
                    RUNSrecordID = CREWONLY.recordID;
                    runNumber = CREWONLY.runNumber;
                    serviceRecordID = CREWONLY.SERVICESrecordID;
                    while(rcode2 == 0 &&
                          runNumber == CREWONLY.runNumber &&
                          serviceRecordID == CREWONLY.SERVICESrecordID)
                    {
//
//  Set up PROPOSEDRUN
//
                      PROPOSEDRUN.piece[pieceNumber].fromTime = CREWONLY.startTime;
                      PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = CREWONLY.startNODESrecordID;
                      PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = NO_RECORD;
                      PROPOSEDRUN.piece[pieceNumber].toTime = CREWONLY.endTime;
                      PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = CREWONLY.endNODESrecordID;
                      PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = NO_RECORD;
                      PROPOSEDRUN.piece[pieceNumber].prior.startTime = NO_TIME;
                      PROPOSEDRUN.piece[pieceNumber].prior.endTime = NO_TIME;
                      PROPOSEDRUN.piece[pieceNumber].after.startTime = NO_TIME;
                      PROPOSEDRUN.piece[pieceNumber].after.endTime = NO_TIME;
//
//  Get the next (crewonly) run record
//
                      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
                      bGotNext = TRUE;
                      pieceNumber++;
                    }
//
//  Cost the (crewonly) run
//
                    if(pieceNumber == 0)  // Run didn't set up
                    {
                      MessageBeep(MB_ICONSTOP);
                      LoadString(hInst, TEXT_067, szarString, sizeof(szarString));
                      LoadString(hInst, ERROR_149, szFormatString, sizeof(szFormatString));
                      sprintf(tempString, szFormatString, runNumber, szarString);
                      MessageBox(hWndDlg, tempString, TMS, MB_ICONSTOP | MB_OK);
                    }
                    else
                    {
                      PROPOSEDRUN.numPieces = pieceNumber;
                      RunCoster(&PROPOSEDRUN, selectedRuntype, &COST);
                      nL = m_pRRLIST[serviceIndex].numRuns;
                      if(m_pRRLIST[serviceIndex].numRuns >= m_pRRLIST[serviceIndex].maxRuns)
                      {
                        m_pRRLIST[serviceIndex].maxRuns += 128;
                        m_pRRLIST[serviceIndex].pData = (RRDATADef *)HeapReAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY, m_pRRLIST[serviceIndex].pData,
                              sizeof(RRDATADef) * m_pRRLIST[serviceIndex].maxRuns); 
                        if(m_pRRLIST[serviceIndex].pData == NULL)
                        {
                          AllocationError(__FILE__, __LINE__, TRUE);
                          break;
                        }
                      }
                      m_pRRLIST[serviceIndex].pData[nL].RUNSrecordID = RUNSrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].runNumber = runNumber;
                      m_pRRLIST[serviceIndex].pData[nL].runtype = selectedRuntype;
                      m_pRRLIST[serviceIndex].pData[nL].RGRPROUTESrecordID = NO_RECORD;
                      m_pRRLIST[serviceIndex].pData[nL].onTime = PROPOSEDRUN.piece[0].fromTime - 
                            COST.PIECECOST[0].reportTime - COST.TRAVEL[0].startTravelTime;
                      m_pRRLIST[serviceIndex].pData[nL].onNODESrecordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].offTime = PROPOSEDRUN.piece[pieceNumber - 1].toTime +
                            COST.PIECECOST[nK].turninTime + COST.TRAVEL[nK].endTravelTime;
                      m_pRRLIST[serviceIndex].pData[nL].offNODESrecordID = PROPOSEDRUN.piece[pieceNumber - 1].toNODESrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].payTime = COST.TOTAL.payTime;
                      m_pRRLIST[serviceIndex].pData[nL].flags = RTFLAGS_CREWONLY;
                      m_pRRLIST[serviceIndex].numRuns++;
                    }
                  }
                }
                if(!bGotNext)
                {
                  rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
                }
              }
            }
//
//  Regular runs
//
            else
            {
              rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              while(rcode2 == 0)
              {
                bGotNext = FALSE;
                if(RUNS.DIVISIONSrecordID == DIVISIONS.recordID &&
                      RUNS.cutAsRuntype == selectedRuntype)
                {
//
//  Cost the run so we can fill in the rest of the RRLIST structure
//
                  for(serviceIndex = NO_RECORD, nK = 0; nK < numServices; nK++)
                  {
                    if(RUNS.SERVICESrecordID == m_pRRLIST[nK].SERVICESrecordID)
                    {
                      serviceIndex = nK;
                      break;
                    }
                  }
                  if(serviceIndex != NO_RECORD)
                  {
                    pieceNumber = 0;
                    RUNSrecordID = RUNS.recordID;
                    runNumber = RUNS.runNumber;
                    RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                    while(rcode2 == 0 && runNumber == RUNS.runNumber)
                    {
                      RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                            RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
//
//  Set up PROPOSEDRUN
//
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
//
//  Get the next run record
//
                      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
                      bGotNext = TRUE;
                      pieceNumber++;
                    }
//
//  Cost the run
//
                    if(pieceNumber == 0)  // Run didn't set up
                    {
                      MessageBeep(MB_ICONSTOP);
                      LoadString(hInst, TEXT_067, szarString, sizeof(szarString));
                      LoadString(hInst, ERROR_149, szFormatString, sizeof(szFormatString));
                      sprintf(tempString, szFormatString, runNumber, szarString);
                      MessageBox(hWndDlg, tempString, TMS, MB_ICONSTOP | MB_OK);
                    }
                    else
                    {
                      PROPOSEDRUN.numPieces = pieceNumber;
                      RunCoster(&PROPOSEDRUN, selectedRuntype, &COST);
                      nL = m_pRRLIST[serviceIndex].numRuns;
                      if(m_pRRLIST[serviceIndex].numRuns >= m_pRRLIST[serviceIndex].maxRuns)
                      {
                        m_pRRLIST[serviceIndex].maxRuns += 128;
                        m_pRRLIST[serviceIndex].pData = (RRDATADef *) HeapReAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY, m_pRRLIST[serviceIndex].pData,
                              sizeof(RRDATADef) * m_pRRLIST[serviceIndex].maxRuns); 
                        if(m_pRRLIST[serviceIndex].pData == NULL)
                        {
                          AllocationError(__FILE__, __LINE__, TRUE);
                          break;
                        }
                      }
                      m_pRRLIST[serviceIndex].pData[nL].RUNSrecordID = RUNSrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].runNumber = runNumber;
                      m_pRRLIST[serviceIndex].pData[nL].runtype = selectedRuntype;
                      m_pRRLIST[serviceIndex].pData[nL].RGRPROUTESrecordID = RGRPROUTESrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].onTime = PROPOSEDRUN.piece[0].fromTime -
                            COST.PIECECOST[0].reportTime - COST.TRAVEL[0].startTravelTime;
                      m_pRRLIST[serviceIndex].pData[nL].onNODESrecordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].offTime = PROPOSEDRUN.piece[pieceNumber - 1].toTime +
                            COST.PIECECOST[nK].turninTime + COST.TRAVEL[nK].endTravelTime;
                      m_pRRLIST[serviceIndex].pData[nL].offNODESrecordID = PROPOSEDRUN.piece[pieceNumber - 1].toNODESrecordID;
                      m_pRRLIST[serviceIndex].pData[nL].payTime = COST.TOTAL.payTime;
                      m_pRRLIST[serviceIndex].pData[nL].flags = 0;
                      m_pRRLIST[serviceIndex].numRuns++;
                    }
                  }
                }
                if(!bGotNext)
                  rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
              }
            }
          }
//
//  Allocate space for the m_pCOMBINED structure
//
          m_maxCOMBINED = 512;
          m_pCOMBINED = (COMBINEDDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COMBINEDDef) * m_maxCOMBINED); 
          if(m_pCOMBINED == NULL)
          {
            AllocationError(__FILE__, __LINE__, FALSE);
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
            break;
          }

//
//  Set up COMBINED
//
          for(m_numCOMBINED = 0, nI = 0; nI < ROSTER_MAX_DAYS; nI++)
          {
//
// First, find a valid service for this day.
//
            for(serviceIndex = NO_RECORD, nK = 0; nK < numServices; nK++)
            {
              if(ROSTERPARMS.serviceDays[nI] == m_pRRLIST[nK].SERVICESrecordID)
              {
                serviceIndex = nK;
                break;
              }
            }
            if(serviceIndex == NO_RECORD)
              continue;
//
// For all runs for this service, try to find an existing
// roster to combine it with.
//
            for(nK = 0; nK < m_pRRLIST[serviceIndex].numRuns; nK++)
            {
              for(bFound = FALSE, nL = 0; nL < m_numCOMBINED; nL++)
              {
                for(nM = 0; nM < nI; nM++)
                {
                  if((m_pCOMBINED[nL].RUNSrecordID[nM] ==
                        m_pRRLIST[serviceIndex].pData[nK].RUNSrecordID) &&
                     (m_pCOMBINED[nL].flags[nM] ==
                        m_pRRLIST[serviceIndex].pData[nK].flags))
                  {
                    bFound = TRUE;
                    m_pCOMBINED[nL].RUNSrecordID[nI] =
                          m_pRRLIST[serviceIndex].pData[nK].RUNSrecordID;
                    m_pCOMBINED[nL].flags[nI] = 
                          m_pRRLIST[serviceIndex].pData[nK].flags;
                    break;
                  }
                }
              }
//
//  If it wasn't there, create a new roster, initialize it, and assign this run to it
//
              if(!bFound)
              {
                int d;
                long rID;

                if(m_numCOMBINED >= m_maxCOMBINED)
                {
                  m_maxCOMBINED += 512;
                  m_pCOMBINED = (COMBINEDDef *)HeapReAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY, m_pCOMBINED, sizeof(COMBINEDDef) * m_maxCOMBINED); 
                  if(m_pCOMBINED == NULL)
                  {
                    AllocationError(__FILE__, __LINE__, TRUE);
                    break;
                  }
                }
                m_pCOMBINED[m_numCOMBINED].frozen = FALSE;
                m_pCOMBINED[m_numCOMBINED].rosterNumber = NO_RECORD;
                for(d = 0; d < ROSTER_MAX_DAYS; d++)
                {
                  m_pCOMBINED[m_numCOMBINED].RUNSrecordID[d] = NO_RECORD;
                  m_pCOMBINED[m_numCOMBINED].flags[d] = 0;
                }
                rID = m_pRRLIST[serviceIndex].pData[nK].RUNSrecordID;
                m_pCOMBINED[m_numCOMBINED].RUNSrecordID[nI] = rID;
                m_pCOMBINED[m_numCOMBINED].COMMENTSrecordID = NO_RECORD;
                m_pCOMBINED[m_numCOMBINED].DIVISIONSrecordID = NO_RECORD;
                m_pCOMBINED[m_numCOMBINED].DRIVERSrecordID = NO_RECORD;
                m_pCOMBINED[m_numCOMBINED].flags[nI] = m_pRRLIST[serviceIndex].pData[nK].flags;
                m_numCOMBINED++;
              }
            }
          }
//
//  Now go through the existing roster to take out those runs that
//  have already been rostered.  These rosters start in position currentCombined.
//
          currentCombined = m_numCOMBINED;
          ROSTERKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
          ROSTERKey1.rosterNumber = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
          while(rcode2 == 0 &&
                ROSTER.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID)
          {
            if(currentCombined >= m_maxCOMBINED)
            {
              m_maxCOMBINED += 512;
              m_pCOMBINED = (COMBINEDDef *)HeapReAlloc(GetProcessHeap(),
                    HEAP_ZERO_MEMORY, m_pCOMBINED, sizeof(COMBINEDDef) * m_maxCOMBINED); 
              if(m_pCOMBINED == NULL)
              {
                AllocationError(__FILE__, __LINE__, TRUE);
                break;
              }
            }
//
//  Set up a new COMBINED record, starting at the end of the list
//
            m_pCOMBINED[currentCombined].frozen = TRUE;
            m_pCOMBINED[currentCombined].rosterNumber = ROSTER.rosterNumber;
            m_pCOMBINED[currentCombined].COMMENTSrecordID = ROSTER.COMMENTSrecordID;
            m_pCOMBINED[currentCombined].DIVISIONSrecordID = ROSTER.DIVISIONSrecordID;
            m_pCOMBINED[currentCombined].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
//
//  Pluck out the runs that are already in the first
//  portion of COMBINED, and put them into this record
//
            for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
            {
              m_pCOMBINED[currentCombined].RUNSrecordID[nI] = NO_RECORD;
              if(ROSTER.WEEK[pDI->fileInfo.rosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
                continue;
              for(nJ = 0; nJ < m_numCOMBINED; nJ++)
              {
                if(ROSTER.WEEK[pDI->fileInfo.rosterWeek].RUNSrecordIDs[nI] == m_pCOMBINED[nJ].RUNSrecordID[nI])
                {
                  if((m_pCOMBINED[nJ].flags[nI] == 0 && !(ROSTER.WEEK[pDI->fileInfo.rosterWeek].flags & (1 << nI))) ||
                        (m_pCOMBINED[nJ].flags[nI] & RTFLAGS_CREWONLY) &&
                              (ROSTER.WEEK[pDI->fileInfo.rosterWeek].flags & (1 << nI)))
                  {
                    m_pCOMBINED[currentCombined].RUNSrecordID[nI] = m_pCOMBINED[nJ].RUNSrecordID[nI];
                    m_pCOMBINED[currentCombined].flags[nI] = m_pCOMBINED[nJ].flags[nI];
                    m_pCOMBINED[nJ].RUNSrecordID[nI] = NO_RECORD;
                    m_pCOMBINED[nJ].flags[nI] = 0;
                    break;
                  }
                }
              }
            }
            currentCombined++;
            rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
          }
//
//  Squash the list
//
          m_numCOMBINED = currentCombined;
          m_numCOMBINED = CompressCOMBINED();
//
//  Got our runs - bring up the ROSTER dialog
//
          SetCursor(saveCursor);
          nRc = RosterDialog(pDI);
//          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ROSTER),
//                hWndDlg, (DLGPROC)ROSTERMsgProc, (LPARAM)pDI);
          if(nRc)
          {
//
//  Free allocated memory
//
            if(bUseDriverTypes)
            {
              TMSHeapFree(pDriverTypes);
            }
            nI = 0;
            while(m_pRRLIST[nI].SERVICESrecordID != NO_RECORD)
            {
              TMSHeapFree(m_pRRLIST[nI].pData);
              nI++;
            }
            TMSHeapFree(m_pRRLIST);
            TMSHeapFree(m_pCOMBINED);
            EndDialog(hWndDlg, TRUE);
          }
          break;
//
//  IDBIDDING button
//
        case IDBIDDING:
//
//  Build up the parameter structure
//
          BIDDING.pDI = pDI;
//
//  Loop through the SELECTIONS list box
//
          BIDDING.numSelections =
                SendMessage(hCtlSELECTIONS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          BIDDING.numSelections = min(BIDDING.numSelections, NUMRUNTYPES * NUMRUNTYPESLOTS);
          for(nI = 0, nJ = 0; nI < BIDDING.numSelections; nI++)
          {
            SendMessage(hCtlSELECTIONS, LB_GETTEXT, (WPARAM)nJ, (LONG)(LPSTR)tempString);
            strcpy(szarString, strtok(tempString, "\t"));
            trim(szarString, sizeof(DIVISIONS.name));
            rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
            bFound = FALSE;
            while(rcode2 == 0)
            {
              strncpy(tempString, DIVISIONS.name, sizeof(DIVISIONS.name));
              trim(tempString, sizeof(DIVISIONS.name));
              if((bFound = strcmp(tempString, szarString) == 0) == TRUE)
                break;
              rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
            }
            if(!bFound)
              continue;
            BIDDING.SELECTIONS[nJ].divisionRecordID = DIVISIONS.recordID;
            BIDDING.SELECTIONS[nJ].runtype = 
                  SendMessage(hCtlSELECTIONS, LB_GETITEMDATA, nJ, (LPARAM)0);
            nJ++;
          }
//
//  Loop through the drivers in seniority order.
//  Omit any drivers from the loop that have been rostered already.
//
          bUseDriverTypes = (ROSTERPARMS.flags & ROSTERPARMS_FLAG_BIDBYDRIVERTYPE);
          if(!bUseDriverTypes)
            numDriverTypes = 1;
          else
          {
            rcode2 = btrieve(B_STAT, TMS_DRIVERTYPES, &BSTAT, dummy, 0);
            if(rcode2 != 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_222, (HANDLE)NULL);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            numDriverTypes = (int)BSTAT.numRecords;
            pDriverTypes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * numDriverTypes); 
            if(pDriverTypes == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              numDriverTypes = 0;
            }
            else
            {
              rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
              nI = 0;
              while(rcode2 == 0 && nI < numDriverTypes)
              {
                if(driversToConsider & (1 << (DRIVERTYPES.number - 1)))
                {
                  pDriverTypes[nI++] = DRIVERTYPES.number;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
              }
              numDriverTypes = nI;
            }
          }
          for(bStopBidding = FALSE, nI = 0; nI < numDriverTypes; nI++)
          {
            if(bStopBidding)
            {
              break;
            }
            rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
            while(rcode2 == 0)
            {
              ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
              if(rcode2 != 0)
              {
                if(DRIVERS.seniorityDate != 0)
                {
                  if(!bUseDriverTypes ||
                        (bUseDriverTypes && DRIVERS.DRIVERTYPESrecordID == pDriverTypes[nI]))
                  {
                    BIDDING.DRIVERS = DRIVERS;
                    nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BIDDING),
                          hWndDlg, (DLGPROC)BIDDINGMsgProc, (LPARAM)&BIDDING);
                    if(!nRc)
                    {
                      bStopBidding = TRUE;
                      break;
                    }
                  }
                }
              }
              rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
            }
          }
//
//  Free allocated memory
//
          if(bUseDriverTypes)
          {
            TMSHeapFree(pDriverTypes);
          }
          if(m_pRRLIST != NULL)
          {
            nI = 0;
            while(m_pRRLIST[nI].SERVICESrecordID != NO_RECORD)
            {
              TMSHeapFree(m_pRRLIST[nI].pData);
              nI++;
            }
            TMSHeapFree(m_pRRLIST);
          }
          TMSHeapFree(m_pCOMBINED);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
