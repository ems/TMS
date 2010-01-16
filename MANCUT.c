//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
  
#define BLKTABSET    6
#define RUNTABSET    10
#define MAXCUTPOINTS 2
#define MAXBLOCKS    1000

#define MENU1POS_IDTAKEBLOCK    0
#define MENU1POS_SEPARATOR1     1 
#define MENU1POS_IDEXPAND       2 
#define MENU1POS_SEPARATOR2     3 
#define MENU1POS_CUTWHOLEBLOCK  4
#define MENU1POS_SEPARATOR3     5
#define MENU1POS_IDMANUAL       6
#define MENU1POS_SEPARATOR4     7 
#define MENU1POS_IDRECURSIVE    8
#define MENU1POS_IDAUTOMATIC    9
#define MENU1POS_IDCREWCUT     10

#define MENU2POS_IDTAKERUN      0
#define MENU2POS_IDFREEZE       1
#define MENU2POS_SEPARATOR1     2
#define MENU2POS_IDHOOKRUNS     3 
#define MENU2POS_IDSHIFT        4
#define MENU2POS_IDSWAP         5
#define MENU2POS_IDUNHOOK       6
#define MENU2POS_IDUNDORUN      7
#define MENU2POS_SEPARATOR2     8
#define MENU2POS_IDRENUMBER     9
#define MENU2POS_SEPARATOR3    10 
#define MENU2POS_IDIMPROVE     11
#define MENU2POS_SEPARATOR4    12
#define MENU2POS_IDANALYZERUN  13
#define MENU2POS_IDSUMMARY     14
#define MENU2POS_SEPARATOR5    15
#define MENU2POS_IDPROFILE     16
#define MENU2POS_IDPLOTLEFT    17

#define MANCUT_UNDERDEVEL

static int runStart[MAXPIECES];

void DisplayRun(HANDLE, BOOL, int, PROPOSEDRUNDef *, COSTDef *, BOOL);
int  GetSelectedRunNumbers(HWND, HANDLE, HGLOBAL *, int * *);

extern int sort_pieces(const void *a, const void *b);

BOOL CALLBACK MANCUTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HCURSOR hSaveCursor;
 static  HGLOBAL hMemRuns;
 static  HANDLE  hCtlBLOCKSTITLE;
 static  HANDLE  hCtlBLOCKS;
 static  HANDLE  hCtlRUNSTITLE;
 static  HANDLE  hCtlRUNS;
 static  HMENU   hMenu1;
 static  HMENU   hMenu2;
 static  int     BLKTabPos[BLKTABSET] = {20, 30, 37, 43, 50, 57};
 static  int     RUNTabPos[RUNTABSET] = { 7, 12, 19, 26, 33, 40, 47, 54, 61, 69};
 static  int     numInternalRuns;
 ANALYZERUNPassedDataDef ANALYZERUNPassedData;
 GetConnectionTimeDef    GCTData;
 PROPOSEDRUNDef  PR;
 COSTDef COST;
 float   distance;
 HMENU   hMenu;
 POINT   point;
 BOOL    setSel;
 BOOL    bFound;
 BOOL    bDoneOnce;
 HWND    hWnd;
 UINT    enable;
 WORD    index;
 WORD    DlgWidthUnits;
 char    labelsWere[sizeof(CUTPARMS.labels)];
 char    holdString[MAXPIECES + 1][128];
 long    tempLong;
 long    recordID;
 long    runNumber;
 long    fromNode;
 long    fromTrip;
 long    toNode;
 long    toTrip;
 long	   savings;
 long    travelTime;
 long    waitTime;
 long    startTime;
 long    endTime;
 long    equivalentTravelTime;
 long    prevTime;
 long    blockNumber;
 long    testStart;
 long    testEnd;
 int     adjustedBLKTabPos[BLKTABSET];
 int     adjustedRUNTabPos[RUNTABSET];
 int    *runs;
 int     reliefPointsStart;
 int     reliefPointsEnd;
 int     RLDIndex;
 int     nI;
 int     nJ;
 int     nK;
 int     nL;
 int     nM;
 int     rcode2;
 int     numItems;
 int     numRuns;
 int     numSelected;
 int     blocksSelected[MAXBLOCKS];
 int     improvements;
 int     previousLast;
 int     runEnd[MAXPIECES];
 int     runlistIndex[MAXPIECES];
 int     nRc;
 int     oldRunNumbers[MAXRUNSINRUNLIST];
 int     maxPieceNumber;
 int     runIndexes[2];
 int     numIndexes;
 short int wmId;
 short int wmEvent;
 short int xx;

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
    bGenerateTravelInstructions = FALSE;
    hSaveCursor = SetCursor(hCursorWait);
//
//  Set up the handles to the controls
//
    hCtlBLOCKSTITLE = GetDlgItem(hWndDlg, MANCUT_BLOCKSTITLE);
    hCtlBLOCKS = GetDlgItem(hWndDlg, MANCUT_BLOCKS);
    hCtlRUNSTITLE = GetDlgItem(hWndDlg, MANCUT_RUNSTITLE);
    hCtlRUNS = GetDlgItem(hWndDlg, MANCUT_RUNS);
//
//  Set up the handles to the popup menus
//
    hMenu1 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANCUT1));
    hMenu1 = GetSubMenu(hMenu1, 0);
    hMenu2 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANCUT2));
    hMenu2 = GetSubMenu(hMenu2, 0);
//
//  MANCUT_BLOCKSTITLE
//  MANCUT_BLOCKS
//
//  Set up the tab stops and the title
//
    DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
    for(nI = 0; nI < BLKTABSET; nI++)
    {
      adjustedBLKTabPos[nI] = (DlgWidthUnits * BLKTabPos[nI] * 2);
    }
    SendMessage(hCtlBLOCKSTITLE, LB_SETTABSTOPS, (WPARAM)BLKTABSET, (LPARAM)adjustedBLKTabPos);
    LoadString(hInst, TEXT_050, tempString, TEMPSTRING_LENGTH);
    SendMessage(hCtlBLOCKSTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlBLOCKS, LB_SETTABSTOPS, (WPARAM)BLKTABSET, (LPARAM)adjustedBLKTabPos);
    SendMessage(hCtlBLOCKS, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  MANCUT_RUNSTITLE
//  MANCUT_RUN
//
//  Set up the tab stops and the title
//
    for(nI = 0; nI < RUNTABSET; nI++)
    {
      adjustedRUNTabPos[nI] = (DlgWidthUnits * RUNTabPos[nI] * 2);
    }
    SendMessage(hCtlRUNSTITLE, LB_SETTABSTOPS, (WPARAM)RUNTABSET, (LPARAM)adjustedRUNTabPos);
    LoadString(hInst, TEXT_051, tempString, TEMPSTRING_LENGTH);
    SendMessage(hCtlRUNSTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlRUNS, LB_SETTABSTOPS, (WPARAM)RUNTABSET, (LPARAM)adjustedRUNTabPos);
    SendMessage(hCtlRUNS, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Verify we have at least one garage
//
    if(numGaragesInGarageList == 0)
    {
      TMSError(hWndDlg, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
    }
//
//  Establish the GlobalRunNumber
//
    RUNSKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
    RUNSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID + 1;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETLESSTHAN, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    m_GlobalRunNumber = rcode2 == 0 ? RUNS.runNumber + 1 : 1;
//
//  Build the relief point list from the selected blocks
//
    SetUpReliefPoints(hWndDlg, pDI, MANCUT_BLOCKS, BLKTABSET, BLKTabPos[0], TRUE);
    if(m_numRELIEFPOINTS == NO_RECORD)
    {
      EndDialog(hWndDlg, FALSE);
      break;
    }
//
//  Display the runs.  The cursor is reset in WM_USERSETUP.
//
    SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
    break;
//
//  WM_CLOSE
//
  case WM_CLOSE:
    EndDialog(hWndDlg, FALSE);
    break;
//
//  WM_USERSETUP
//
  case WM_USERSETUP:
    switch(wParam)
    {
      case MANCUT_RUNS:
        numInternalRuns = AssembleRuns();
//
//  Empty the displayed run list.
//
        if((previousLast = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0)) > 0)
          previousLast--;
        SendMessage(hCtlRUNS, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Display all the cut runs.
//
        for(nI = 0; nI < numInternalRuns; nI++)
        {
//
//  Cost the run
//
          for(bFound = FALSE, RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
          {
            if(RUNLIST[nI].runNumber == RUNLISTDATA[RLDIndex].runNumber)
            {
              bFound = TRUE;
              break;
            }
          }
          tempLong = RunCoster(&RUNLIST[nI].run,
                (bFound ? RUNLISTDATA[RLDIndex].cutAsRuntype : NO_RUNTYPE), &COST);
          if(bFound)
          {
            if(RUNLISTDATA[RLDIndex].cutAsRuntype == NO_RUNTYPE)
              RUNLISTDATA[RLDIndex].cutAsRuntype = tempLong;
          }
          else
          {
            for(RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
            {
              if(RUNLISTDATA[RLDIndex].runNumber == NO_RECORD)
              {
                RUNLISTDATA[RLDIndex].runNumber = RUNLIST[nI].runNumber;
                RUNLISTDATA[RLDIndex].cutAsRuntype = tempLong;
                break;
              }
            }
          }
          DisplayRun(hCtlRUNS, RUNLISTDATA[RLDIndex].frozenFlag,
                RUNLIST[nI].runNumber, &RUNLIST[nI].run, &COST, FALSE);
        }
//
//  Position hCtlRUNS to previousLast
//
        SendMessage(hCtlRUNS, LB_SETTOPINDEX, (WPARAM)previousLast, (LPARAM)0);
//
//  Ok, we got 'em - cycle through RELIEFPOINTS and nuke out any that were taken.
//  Since the inUse flag means that this RELIEFPOINT is currently under consideration,
//  there is no need to change its status.
//  All the other rouines can tell if a RELIEFPOINT is available by calling the
//  IsReliefPointMatched macro (defined in runcut.h).
//
        nI = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nJ = nI - 1; nJ >= 0; nJ--)
        {
          tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
          if((short int)HIWORD(tempLong) == NO_RECORD &&
                ((m_pRELIEFPOINTS[LOWORD(tempLong)].start.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[LOWORD(tempLong)].end.runNumber != NO_RECORD) ||
                 (m_pRELIEFPOINTS[LOWORD(tempLong)].start.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[LOWORD(tempLong)].flags & RPFLAGS_FIRSTINBLOCK) ||
                 (m_pRELIEFPOINTS[LOWORD(tempLong)].end.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[LOWORD(tempLong)].flags & RPFLAGS_LASTINBLOCK)))
            SendMessage(hCtlBLOCKS, LB_DELETESTRING, (WPARAM)nJ, (LPARAM)0);
        }
        SetCursor(hSaveCursor);
        break;
    }
    break;
//
//  WM_CONTEXTMENU
//
   case WM_CONTEXTMENU:
     hWnd = (HWND)wParam;
     point.x = LOWORD(lParam);  // horizontal position of cursor 
     point.y = HIWORD(lParam);  // vertical position of cursor 
     hMenu = NULL;
     if(hWnd == hCtlBLOCKSTITLE || hWnd == hCtlBLOCKS)
     {
       hMenu = hMenu1;
       nI = SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
       if(nI == 0 || nI == LB_ERR)
       {
         EnableMenuItem(hMenu, MENU1POS_IDEXPAND, MF_BYPOSITION	| MF_GRAYED);
         EnableMenuItem(hMenu, MENU1POS_IDMANUAL, MF_BYPOSITION	| MF_GRAYED);
         EnableMenuItem(hMenu, MENU1POS_CUTWHOLEBLOCK, MF_BYPOSITION	| MF_GRAYED);
       }
       else
       {
         EnableMenuItem(hMenu, MENU1POS_IDEXPAND, MF_BYPOSITION	| MF_ENABLED);  
         EnableMenuItem(hMenu, MENU1POS_CUTWHOLEBLOCK, MF_BYPOSITION	| MF_ENABLED);
         if(nI != 2)
           EnableMenuItem(hMenu, MENU1POS_IDMANUAL, MF_BYPOSITION	| MF_GRAYED); 
         else
           EnableMenuItem(hMenu, MENU1POS_IDMANUAL, MF_BYPOSITION	| MF_ENABLED);
       }
     }
     else if(hWnd == hCtlRUNSTITLE || hWnd == hCtlRUNS)
     {
       hMenu = hMenu2;
       nI = SendMessage(hCtlRUNS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
       if(nI == 0 || nI == LB_ERR)
         enable = MF_BYPOSITION | MF_GRAYED;
       else
         enable = MF_BYPOSITION | MF_ENABLED;
       EnableMenuItem(hMenu, MENU2POS_IDTAKERUN, enable);    // Invert runs
       EnableMenuItem(hMenu, MENU2POS_IDFREEZE, enable);     // Freeze
       EnableMenuItem(hMenu, MENU2POS_IDSHIFT, enable);      // Shift run
       EnableMenuItem(hMenu, MENU2POS_IDUNHOOK, enable);     // Unhook pieces
       EnableMenuItem(hMenu, MENU2POS_IDUNDORUN, enable);    // Undo
       EnableMenuItem(hMenu, MENU2POS_IDRENUMBER, enable);   // Renumber
       EnableMenuItem(hMenu, MENU2POS_IDANALYZERUN, enable); // Analyze
       if(nI <= 1)
       {
         EnableMenuItem(hMenu, MENU2POS_IDHOOKRUNS, MF_BYPOSITION | MF_GRAYED); // Hook
         EnableMenuItem(hMenu, MENU2POS_IDSWAP, MF_BYPOSITION | MF_GRAYED);     // Swap pieces
       }
       else
       {
         EnableMenuItem(hMenu, MENU2POS_IDHOOKRUNS, MF_BYPOSITION | MF_ENABLED);
         EnableMenuItem(hMenu, MENU2POS_IDSWAP, MF_BYPOSITION | MF_ENABLED);
       }
       nI = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
       if(nI == 0 || nI == LB_ERR)
         enable = MF_BYPOSITION | MF_GRAYED;
       else
         enable = MF_BYPOSITION | MF_ENABLED;
       EnableMenuItem(hMenu, MENU2POS_IDIMPROVE, enable); // Improve
       EnableMenuItem(hMenu, MENU2POS_IDSUMMARY, enable); // Summary
     }
     if(hMenu != NULL)
     {
       TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWndDlg, NULL);
     }
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
//  MANCUT_BLOCKSTITLE
//  MANCUT_RUNSTITLE
//
//  If any of the title list boxes are selected, just deselect them and send him on his way
//
      case MANCUT_BLOCKSTITLE:  // Title string
        SendMessage(hCtlBLOCKSTITLE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
        SetFocus(hCtlBLOCKS);
        break;

      case MANCUT_RUNSTITLE:  // Title string
        SendMessage(hCtlRUNSTITLE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
        SetFocus(hCtlRUNS);
        break;
//
//  MANCUT_BLOCKS - List box containing blocks to select
//
      case MANCUT_BLOCKS:
        switch(wmEvent)
        {
          case LBN_SELCHANGE:
            nI = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            numSelected = 0;
            for(nJ = 0; nJ < nI; nJ++)
            {
              if(SendMessage(hCtlBLOCKS, LB_GETSEL, (WPARAM)nJ, (LPARAM)0))
              {
                tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                if((short int)HIWORD(tempLong) != NO_RECORD)
                {
                  if(numSelected != 0)
                    break;
                }
                else
                  if(++numSelected == 2)
                    break;
              }
            }
            break;
        }
        break;
//
//  IDEXPAND - Expand/Condense the display of a block
//
      case IDEXPAND:
        numSelected = SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
        if(numSelected != 0)
          SendMessage(hCtlBLOCKS, LB_GETSELITEMS, (WPARAM)MAXBLOCKS, (LPARAM)(int FAR *)blocksSelected);
        for(nI = 0; nI < numSelected; nI++)
        {
          nK = blocksSelected[nI];
          tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)(blocksSelected[nI] + 1), (LPARAM)0);
//
//  Expand
//
          if(tempLong == LB_ERR || (short int)HIWORD(tempLong) != NO_RECORD)
          {
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
            reliefPointsStart = LOWORD(tempLong);
            reliefPointsEnd = (short int)HIWORD(tempLong);
            for(nJ = reliefPointsStart; nJ < reliefPointsEnd; nJ++)
            {
              if(m_pRELIEFPOINTS[nJ].start.runNumber == SECURELOCATION_RUNNUMBER)
                testStart = NO_RECORD;
              else
                testStart = m_pRELIEFPOINTS[nJ].start.runNumber;
              if(m_pRELIEFPOINTS[nJ].end.runNumber == SECURELOCATION_RUNNUMBER)
                testEnd = NO_RECORD;
              else
                testEnd = m_pRELIEFPOINTS[nJ].end.runNumber;
              if((testStart != NO_RECORD && testEnd != NO_RECORD) ||
                 (testStart != NO_RECORD && m_pRELIEFPOINTS[nJ].flags & RPFLAGS_FIRSTINBLOCK) ||
                 (testEnd   != NO_RECORD && m_pRELIEFPOINTS[nJ].flags & RPFLAGS_LASTINBLOCK))
                continue;
              strcpy(tempString, "\t");
              LoadString(hInst, TEXT_055, szarString, sizeof(szarString));
              strcat(tempString, szarString);
              strcat(tempString, "\t");
              sprintf(szarString, "%5d", m_pRELIEFPOINTS[nJ].blockNumber);
              strcat(tempString, szarString);
              strcat(tempString, "\t");
              strcat(tempString, m_pRELIEFPOINTS[nJ].nodeName);
              strcat(tempString, "\t");
              strcat(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
              strcat(tempString, "\t");
              if(testStart != NO_RECORD || m_pRELIEFPOINTS[nJ].flags & RPFLAGS_LASTINBLOCK)
              {
                strcat(tempString, chhmm(m_pRELIEFPOINTS[nJ].time - prevTime));
                strcat(tempString, "\t");
                LoadString(hInst, TEXT_071, szarString, sizeof(szarString));
                strcat(tempString, szarString);
              }
              else if(testEnd != NO_RECORD || m_pRELIEFPOINTS[nJ].flags & RPFLAGS_FIRSTINBLOCK)
              {
                strcat(tempString, "0:00\t");
                LoadString(hInst, TEXT_070, szarString, sizeof(szarString));
                strcat(tempString, szarString);
                prevTime = m_pRELIEFPOINTS[nJ].time;
              }
              else
              {
                strcat(tempString, chhmm(m_pRELIEFPOINTS[nJ].time - prevTime));
              }
              nL = SendMessage(hCtlBLOCKS, LB_INSERTSTRING, (WPARAM)(++nK), (LONG)(LPSTR)tempString);
              index = (WORD)nJ;
              tempLong = MAKELONG(index, NO_RECORD);
              SendMessage(hCtlBLOCKS, LB_SETITEMDATA, (WPARAM)nL, (LPARAM)tempLong);
            }
          }
//
//  Condense
//
          else
          {
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
            xx = (short int)HIWORD(tempLong);
            while(tempLong != LB_ERR && xx == NO_RECORD)  // Position to top of block entry
            {
              nK--;
              tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
              xx = (short int)HIWORD(tempLong);
            }
            nK++;
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
            xx = (short int)HIWORD(tempLong);
            while(tempLong != LB_ERR && xx == NO_RECORD)
            {
              SendMessage(hCtlBLOCKS, LB_DELETESTRING, (WPARAM)nK, (LPARAM)0);
              tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
              xx = (short int)HIWORD(tempLong);
            }
          }
//
//  Refresh the list
//
          SendMessage(hCtlBLOCKS, LB_GETSELITEMS, (WPARAM)MAXBLOCKS, (LPARAM)(int FAR *)blocksSelected);
        }
        SendMessage(hCtlBLOCKS, LB_SETCURSEL, (WPARAM)blocksSelected[0], (LPARAM)0);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(MANCUT_BLOCKS, LBN_SELCHANGE), (LPARAM)0);
        break;
//
//  MANCUT_RUNS
//
      case MANCUT_RUNS:
        switch(wmEvent)
        {
          case LBN_DBLCLK:
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDFREEZE, 0), (LPARAM)0);
            break;
        }
        break;
//
//  IDTAKEBLOCK - Blocks context menu item "Invert Blocks"
//
//  Select/deselect all the blocks in the blocks display box
//
      case IDTAKEBLOCK:
        hSaveCursor = SetCursor(hCursorWait);
        nI = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nJ = 0; nJ < nI; nJ++)
        {
          tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
          if((short int)HIWORD(tempLong) != NO_RECORD)
          {
            setSel = !SendMessage(hCtlBLOCKS, LB_GETSEL, (WPARAM)nJ, (LPARAM)0);
            SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)setSel, (LPARAM)(nJ));
          }
        }
        SetCursor(hSaveCursor);
        break;
//
//  IDRECURSIVE
//  IDCREWCUT
//  IDAUTOMATIC
//
//  Cursor is reset in WM_USERSETUP
//
      case IDCREWCUT:
      case IDAUTOMATIC:
      case IDRECURSIVE:
        hSaveCursor = SetCursor(hCursorWait);
        if(wmId == IDAUTOMATIC)
        {
          AutomaticRunCut(IDAUTOMATIC);
        }
        else if(wmId == IDCREWCUT)
        {
          StatusBarStart(NULL, "Crew Cut in Progress");
          StatusBarText("Performing first pass");
          StatusBar(-1L, -1L);
          CutCrew();
          StatusBarEnd();
        }
        else if(wmId == IDRECURSIVE)
        {
          StatusBarStart(NULL, "Recursive Runcut in Progress");
          StatusBarText("Tries: 0  Runs cut: 0");
          StatusBarEnableCancel(TRUE);
          StatusBar(-1L, -1L);
          AutomaticRunCut(IDRECURSIVE);
          StatusBarEnableCancel(TRUE);
          StatusBarEnd();
        }
        SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
        break;
//
//  IDCUTWHOLEBLOCK
//
      case IDCUTWHOLEBLOCK:
        numSelected = SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
        if(numSelected != 0)
          SendMessage(hCtlBLOCKS, LB_GETSELITEMS, (WPARAM)MAXBLOCKS, (LPARAM)(int FAR *)blocksSelected);
        for(nI = 0; nI < numSelected; nI++)
        {
          nK = blocksSelected[nI];
          tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)(blocksSelected[nI] + 1), (LPARAM)0);
          if(tempLong == LB_ERR || (short int)HIWORD(tempLong) != NO_RECORD)
          {
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
            reliefPointsStart = LOWORD(tempLong);
            reliefPointsEnd = (short int)HIWORD(tempLong);
            blockNumber = m_pRELIEFPOINTS[reliefPointsStart].blockNumber;
            bDoneOnce = FALSE;
            for(nJ = reliefPointsStart; nJ <= reliefPointsEnd; nJ++)
            {
              if((m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD) ||
                 (m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[nJ].flags & RPFLAGS_FIRSTINBLOCK) ||
                 (m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD &&
                  m_pRELIEFPOINTS[nJ].flags & RPFLAGS_LASTINBLOCK) ||
                  m_pRELIEFPOINTS[nJ].blockNumber != blockNumber)
              {
                if(bDoneOnce)
                {
                  m_GlobalRunNumber++;
                  bDoneOnce = FALSE;
                }
                continue;
              }
              if(nJ != reliefPointsEnd)
              {
                m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
                m_pRELIEFPOINTS[nJ].start.runNumber = m_GlobalRunNumber;
                m_pRELIEFPOINTS[nJ].start.pieceNumber = 1;
                bDoneOnce = TRUE;
              }
              if(nJ != reliefPointsStart)
              {
                m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
                m_pRELIEFPOINTS[nJ].end.runNumber = m_GlobalRunNumber;
                m_pRELIEFPOINTS[nJ].end.pieceNumber = 1;
                bDoneOnce = TRUE;
              }
            }
            m_GlobalRunNumber++;
          }
        }
        SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
        break;
//
//  IDMANUAL
//
      case IDMANUAL:
        hSaveCursor = SetCursor(hCursorWait);
//
//  Find the start and end positions in RELIEFPOINTS
//
        nI = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(bFound = FALSE, nJ = 0; nJ < nI; nJ++)
        {
          if(SendMessage(hCtlBLOCKS, LB_GETSEL, (WPARAM)nJ, (LPARAM)0))
          {
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
            nK = LOWORD(tempLong);
            nL = (short int)HIWORD(tempLong);
            if(nL == NO_RECORD)
            {
              if(!bFound)
                reliefPointsStart = nK;
              else
              {
                reliefPointsEnd = nK;
                break;
              }
              bFound = TRUE;
            }
          }
        }
        if(!bFound)
          break;
//
//  Ensure that he hasn't overlapped a piece cut out of the block
//
        for(bFound = FALSE, nJ = reliefPointsStart; nJ <= reliefPointsEnd; nJ++)
        {
          if((m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
                m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD) ||
             (m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
                m_pRELIEFPOINTS[nJ].flags & RPFLAGS_FIRSTINBLOCK) ||
             (m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD &&
                m_pRELIEFPOINTS[nJ].flags & RPFLAGS_LASTINBLOCK))
          {
            bFound = TRUE;
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_161, (HANDLE)NULL);
            break;
          }
        }
        if(bFound)
          break;
//
//  Assign GlobalRunNumber to the range in question
//
        for(nJ = reliefPointsStart; nJ <= reliefPointsEnd; nJ++)
        {
          if(nJ != reliefPointsEnd)
          {
            m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
            m_pRELIEFPOINTS[nJ].start.runNumber = m_GlobalRunNumber;
            m_pRELIEFPOINTS[nJ].start.pieceNumber = 1;
          }
          if(nJ != reliefPointsStart)
          {
            m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
            m_pRELIEFPOINTS[nJ].end.runNumber = m_GlobalRunNumber;
            m_pRELIEFPOINTS[nJ].end.pieceNumber = 1;
          }
        }
//
//  Nuke out all the timepoint selections
//
        for(nJ = 0; nJ < nI; nJ++)
        {
          if(SendMessage(hCtlBLOCKS, LB_GETSEL, nJ, (LPARAM)0))
          {
            tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
            nL = (short int)HIWORD(tempLong);
            if(nL == NO_RECORD)
            {
              index = (WORD)nJ;
              SendMessage(hCtlBLOCKS, LB_SETSEL, (WPARAM)FALSE, MAKELONG(index, 0));
            }
          }
        }
//
//  Increment m_GlobalRunNumber and send a WM_USERSETUP to MANCUT_RUNS
//
        m_GlobalRunNumber++;
        SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
        break;
//
//  IDPARAMETERS
//
      case IDPARAMETERS:
        strcpy(labelsWere, CUTPARMS.labels);
        nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_PARAMETERS),
              hWndDlg, (DLGPROC)PARAMETERSMsgProc);
        if(nRc && strcmp(labelsWere, CUTPARMS.labels) != 0)
          TMSError(hWndDlg, MB_ICONINFORMATION, ERROR_158, (HANDLE)NULL);
        break;
//
//  IDRUNTYPES
//
      case IDRUNTYPES:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_RUNTYPES),
              hWndDlg, (DLGPROC)RUNTYPESMsgProc);
        break;
//
//  IDIMPROVE
//
     case IDIMPROVE:
       hSaveCursor = SetCursor(hCursorWait);
       improvements = RuncutImprove(&savings);
       SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
       sprintf( tempString, "Found %ld improvements.\n\nSaved %ld:%02ld:%02ld pay time.",
		         improvements, savings / (60L * 60L), (savings / 60L) % 60L, savings % 60L);
       MessageBox( NULL, tempString, "Runcut Improve", MB_OK | MB_ICONINFORMATION );
       break;
//
//  IDSUMMARY
//
      case IDSUMMARY:
        RunSummary();
        break;
//
//  IDSHIFT - Shift relief points on a run
//
      case IDSHIFT:
        bGenerateTravelInstructions = TRUE;
        numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nI = 0; nI < numItems; nI++)
        {
          if(SendMessage(hCtlRUNS, LB_GETSEL, (WPARAM)nI, (LPARAM)0) > 0)
          {
            runNumber = SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
            {
              if(RUNLIST[nJ].runNumber == runNumber)
              {
                if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SHIFTRUN),
                      hWndDlg, (DLGPROC)SHIFTRUNMsgProc, (LPARAM)&nJ))
                {
                  SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
                }
                break;
              }
            }
          }
        }
        bGenerateTravelInstructions = FALSE;
        break;
//
//  IDSWAP - Swap the pieces between two runs
//
      case IDSWAP:
        numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        numIndexes = 0;
        for(nI = 0; nI < numItems; nI++)
        {
          if(SendMessage(hCtlRUNS, LB_GETSEL, (WPARAM)nI, (LPARAM)0) > 0)
          {
            runNumber = SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
            {
              if(RUNLIST[nJ].runNumber == runNumber)
              {
                for(bFound = FALSE, nK = 0; nK < numIndexes; nK++)
                {
                  if(runIndexes[nK] == nJ)
                  {
                    bFound = TRUE;
                    break;
                  }
                }
                if(!bFound)
                {
                  runIndexes[numIndexes] = nJ;
                  numIndexes++;
                }
                break;
              }
            }
          }
          if(numIndexes == 2)
            break;
        }
        if(numIndexes != 2)
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_239, (HANDLE)NULL);
        else
        {
          if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SWAPPIECES),
                hWndDlg, (DLGPROC)SWAPPIECESMsgProc, (LPARAM)&runIndexes))
          {
            SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
          }
        }
        break;
//
//  IDUNHOOK - Unhook the pieces of a run, but leave them as cut
//
      case IDUNHOOK:
        hSaveCursor = SetCursor(hCursorWait);
        hMemRuns = NULL;
        numRuns = GetSelectedRunNumbers(hWndDlg, hCtlRUNS, &hMemRuns, &runs);
        if(numRuns == NO_RECORD || numRuns == 0)
          break;
//
//  Loop through the runs
//
        for(nI = 0; nI < numRuns; nI++)
        {
          runNumber = runs[nI];
//
//  If it's frozen, skip it
//
          for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(RUNLISTDATA[nJ].runNumber == runNumber)
            {
              bFound = RUNLISTDATA[nJ].frozenFlag;
              break;
            }
          }
          if(bFound)
            continue;
//
//  Not frozen - continue.
//
//  Modify the run's data in the RELIEFPOINTS structure
//
          maxPieceNumber = 0;
          for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
          {
            if(m_pRELIEFPOINTS[nJ].start.runNumber == runNumber)
            {
              m_pRELIEFPOINTS[nJ].start.runNumber = m_GlobalRunNumber + m_pRELIEFPOINTS[nJ].start.pieceNumber - 1;
              if(m_pRELIEFPOINTS[nJ].start.pieceNumber > maxPieceNumber)
              {
                maxPieceNumber = m_pRELIEFPOINTS[nJ].start.pieceNumber;
              }
              m_pRELIEFPOINTS[nJ].start.pieceNumber = 1;
            }
            if(m_pRELIEFPOINTS[nJ].end.runNumber == runNumber)
            {
              m_pRELIEFPOINTS[nJ].end.runNumber = m_GlobalRunNumber + m_pRELIEFPOINTS[nJ].end.pieceNumber - 1;
              if(m_pRELIEFPOINTS[nJ].end.pieceNumber > maxPieceNumber)
              {
                maxPieceNumber = m_pRELIEFPOINTS[nJ].end.pieceNumber;
              }
              m_pRELIEFPOINTS[nJ].end.pieceNumber = 1;
            }
          }
          m_GlobalRunNumber += maxPieceNumber;
//
//  Delete the run from the listbox
//
          numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          for(nJ = numItems - 1; nJ >= 0; nJ--)
          {
            if(SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == runNumber)
              SendMessage(hCtlRUNS, LB_DELETESTRING, (WPARAM)nJ, (LPARAM)0);
          }
        }
        TMSGlobalFree(hMemRuns);
        SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)MANCUT_RUNS, (LPARAM)0);
        SetCursor(hSaveCursor);
        break;
//
//  IDTAKERUN -  Runs context menu item "Invert Runs"
//
//  Select/deselect all the runs in the runs list box
//
      case IDTAKERUN:
        hSaveCursor = SetCursor(hCursorWait);
        nI = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nJ = 0; nJ < nI; nJ++)
        {
          setSel = !SendMessage(hCtlRUNS, LB_GETSEL, (WPARAM)nJ, (LPARAM)0);
          SendMessage(hCtlRUNS, LB_SETSEL, (WPARAM)setSel, (LPARAM)(nJ));
        }
        SetCursor(hSaveCursor);
        break;
//
//  IDUNDORUN - Delete a run or set of runs from the "Cut Runs" list box
//
      case IDUNDORUN:
        hSaveCursor = SetCursor(hCursorWait);
        hMemRuns = NULL;
        numRuns = GetSelectedRunNumbers(hWndDlg, hCtlRUNS, &hMemRuns, &runs);
        if(numRuns == NO_RECORD)
          break;
//
//  Loop through the runs
//
        for(nI = 0; nI < numRuns; nI++)
        {
          runNumber = runs[nI];
//
//  If it's frozen, skip it
//
          for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(RUNLISTDATA[nJ].runNumber == runNumber)
            {
              bFound = RUNLISTDATA[nJ].frozenFlag;
              break;
            }
          }
          if(bFound)
            continue;
//
//  Not frozen - continue.
//
//  Zero out the run's data in the RELIEFPOINTS structure
//
          for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
          {
            if(m_pRELIEFPOINTS[nJ].start.runNumber == runNumber)
            {
              m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nJ].start.runNumber = NO_RECORD;
              m_pRELIEFPOINTS[nJ].start.pieceNumber = NO_RECORD;
            }
            if(m_pRELIEFPOINTS[nJ].end.runNumber == runNumber)
            {
              m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nJ].end.runNumber = NO_RECORD;
              m_pRELIEFPOINTS[nJ].end.pieceNumber = NO_RECORD;
            }
          }
//
//  And delete it fron RUNLISTDATA
//
          for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(RUNLISTDATA[nJ].runNumber == runNumber)
            {
              RUNLISTDATA[nJ].runNumber = NO_RECORD;
              RUNLISTDATA[nJ].cutAsRuntype = NO_RUNTYPE;
              RUNLISTDATA[nJ].COMMENTSrecordID = NO_RECORD;
              RUNLISTDATA[nJ].frozenFlag = FALSE;
              break;
            }
          }
//
//  And delete the run from the listbox
//
          numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          for(nJ = numItems - 1; nJ >= 0; nJ--)
          {
            if(SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == runNumber)
              SendMessage(hCtlRUNS, LB_DELETESTRING, (WPARAM)nJ, (LPARAM)0);
          }
        }
        TMSGlobalFree(hMemRuns);
        AssembleRuns();  // Bring the internal lists up to date
        SetCursor(hSaveCursor);
        break;
//
//  IDFREEZE
//
      case IDFREEZE:
        hSaveCursor = SetCursor(hCursorWait);
        hMemRuns = NULL;
        numRuns = GetSelectedRunNumbers(hWndDlg, hCtlRUNS, &hMemRuns, &runs);
        if(numRuns == NO_RECORD)
          break;
//
//  Loop through the runs and set / unset the frozen flag
//
        numItems = (int)SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nI = 0; nI < numRuns; nI++)
        {
          runNumber = runs[nI];
          for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(RUNLISTDATA[nJ].runNumber == runNumber)
            {
              RUNLISTDATA[nJ].frozenFlag = !RUNLISTDATA[nJ].frozenFlag;
              break;
            }
          }
//
//  Find the run(s) in the listbox and store them in holdString
//
          nJ = 0;
          nK = 0;
          nL = 0;
          bFound = FALSE;
          while(nJ < numItems)
          {
            if(SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) != runNumber)
            {
              if(bFound)
                break;
            }
            else
            {
              if(!bFound)
                nL = nJ;
              bFound = TRUE;
              SendMessage(hCtlRUNS, LB_GETTEXT, (WPARAM)nJ, (LONG)(LPSTR)holdString[nK++]);
            }
            nJ++;
          }
//
//  Got the strings in holdString - delete the old strings. Note that nL is a 
//  place holder, as the strings after nL move into its position whenever it's deleted.
//
          for(nJ = 0; nJ < nK; nJ++)
          {
            SendMessage(hCtlRUNS, LB_DELETESTRING, (WPARAM)nL, (LPARAM)0);
          }
//
//  Add the strings back in with the first character changed to either 'F' or ' '.
//
          for(nJ = 0; nJ < nK; nJ++)
          {
            holdString[nJ][0] = holdString[nJ][0] == 'F' ? ' ' : 'F';
            nL = SendMessage(hCtlRUNS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)holdString[nJ]);
            SendMessage(hCtlRUNS, LB_SETITEMDATA, (WPARAM)nL, (LPARAM)runNumber);
          }
        }
        TMSGlobalFree(hMemRuns);
        SetCursor(hSaveCursor);
        break;
//
//  IDRENUMBER
//
      case IDRENUMBER:
//
//  Save the previous run numbers
//
        for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
        {
          oldRunNumbers[nI] = RUNLISTDATA[nI].runNumber;
        }
//
//  Fire up the renumber dialog
//
        specificRun = 0;
        nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_RENUMRUNS),
              hWndDlg, (DLGPROC)RENUMRUNSMsgProc);
//
//  If he said ok, RUNLISTDATA and RELIEFPOINTS were adjusted
//  in the dialog routine.  Match the new to the old to see what
//  changes have to be made in the listbox.
//
        if(nRc)
        {
          hSaveCursor = SetCursor(hCursorWait);
          numItems = (int)SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
          {
            if(RUNLISTDATA[nI].runNumber != oldRunNumbers[nI])
            {
              for(nJ = 0; nJ < numItems; nJ++)
              {
                runNumber = SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                if(runNumber == oldRunNumbers[nI])
                {
                  bFound = FALSE;
                  nK = nJ;
                  nL = 0;
                  while(nK < numItems)
                  {
                    if(SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0) != runNumber)
                    {
                      if(bFound)
                        break;
                    }
                    else
                    {
                      if(!bFound)
                        nM = nK;
                      bFound = TRUE;
                      SendMessage(hCtlRUNS, LB_GETTEXT, (WPARAM)nK, (LONG)(LPSTR)holdString[nL++]);
                    }
                    nK++;
                  }
//
//  Got the strings in holdString - delete the old strings. Note that nM is a 
//  place holder, as the strings after nM move into its position whenever it's deleted.
//
                  for(nK = 0; nK < nL; nK++)
                  {
                    SendMessage(hCtlRUNS, LB_DELETESTRING, (WPARAM)nM, (LPARAM)0);
                  }
//
//  Add the strings back in with the changed run number
//
                  for(nK = 0; nK < nL; nK++)
                  {
                    sprintf(tempString, "%5d", RUNLISTDATA[nI].runNumber);
                    memcpy(holdString[nK], tempString, 5);
                    nM = SendMessage(hCtlRUNS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)holdString[nK]);
                    SendMessage(hCtlRUNS, LB_SETITEMDATA, (WPARAM)nM, (LPARAM)RUNLISTDATA[nI].runNumber);
                  }
                }
              }
            }
          }
          AssembleRuns();  // Bring the internal lists up to date
          SetCursor(hSaveCursor);
        }
        break;
//
//  IDHOOKRUNS
//
      case IDHOOKRUNS:
        hSaveCursor = SetCursor(hCursorWait);
        hMemRuns = NULL;
        numRuns = GetSelectedRunNumbers(hWndDlg, hCtlRUNS, &hMemRuns, &runs);
        if(numRuns == NO_RECORD || numRuns > MAXPIECES)
          break;
//
//  Get the runs to be hooked and place them into a local copy of PROPOSEDRUN
//
        PR.numPieces = 0;
        for(nI = 0; nI < numRuns; nI++)
        {
          for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(runs[nI] == RUNLIST[nJ].runNumber)
            {
              for(nK = 0; nK < RUNLIST[nJ].run.numPieces; nK++)
              {
                memcpy(&PR.piece[PR.numPieces++], &RUNLIST[nJ].run.piece[nK], sizeof(PROPOSEDRUNPIECESDef)); 
              }
              break;
            }
          }
        }
//
//  Got all the pieces, now sort them in on time order
//
        qsort((void *)PR.piece, PR.numPieces, sizeof(PROPOSEDRUNPIECESDef), sort_pieces);
//
//  Ensure that none of the pieces overlap.  If any of them do,
//  display the pertinent data of the first overlap and break out.
//
        for(bFound = FALSE, nI = 0; nI < PR.numPieces - 1; nI++)
        {
          if(PR.piece[nI].toTime > PR.piece[nI + 1].fromTime)
          {
            strcpy(tempString, "\t");
            for(nJ = nI; nJ <= nI + 1; nJ++)
            {
              strcat(tempString, NodeAbbrFromRecID(PR.piece[nJ].fromNODESrecordID));
              strcat(tempString, "\t");
              strcat(tempString, Tchar(PR.piece[nJ].fromTime));
              strcat(tempString, "\t");
              strcat(tempString, Tchar(PR.piece[nJ].toTime));
              strcat(tempString, "\t");
              NODESKey0.recordID = PR.piece[nJ].toNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(szarString, NODES_ABBRNAME_LENGTH);
              strcat(tempString, szarString);
              strcat(tempString, "\n\t");
            }
            LoadString(hInst, ERROR_152, szFormatString, sizeof(szFormatString));
            sprintf(szarString, szFormatString, tempString);
            MessageBeep(MB_ICONSTOP);
            MessageBox(hWndDlg, szarString, TMS, MB_ICONSTOP | MB_OK);
            bFound = TRUE;
            break;
          }
        }
        if(bFound)
          break;  
//
//  See if there's a travel entry for the end of a piece to the start of the next
//
        for(nI = 0; nI < PR.numPieces - 1; nI++)
        {
          fromNode = PR.piece[nI].toNODESrecordID;
          fromTrip = PR.piece[nI].toTRIPSrecordID;
          toNode = PR.piece[nI + 1].fromNODESrecordID;
          toTrip = PR.piece[nI + 1].fromTRIPSrecordID;
          if(!NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
          {
            if(bUseDynamicTravels)
            {
              if(bUseCISPlan)
              {
                CISplanReliefConnect(&startTime, &endTime, &waitTime, 0, fromNode, toNode,
                      PR.piece[nI + 1].fromTime, FALSE, TRIPS.SERVICESrecordID);
                travelTime = (endTime - startTime) + waitTime;
              }
              else
                travelTime = GetDynamicTravelTime(TRUE, fromNode, toNode,
                      TRIPS.SERVICESrecordID, PR.piece[nI + 1].fromTime, &waitTime);
            }
            else
            {
              GCTData.fromNODESrecordID = fromNode;
              TRIPSKey0.recordID = fromTrip;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toNODESrecordID = toNode;
              TRIPSKey0.recordID = toTrip;
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.timeOfDay = PR.piece[nI].toTime;
              travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
            }
            if(travelTime == NO_TIME ||
                  PR.piece[nI].toTime + travelTime > PR.piece[nI + 1].fromTime)
            {
              if(travelTime == NO_TIME)
                LoadString(hInst, ERROR_153, szFormatString, sizeof(szFormatString));
              else
                LoadString(hInst, ERROR_217, szFormatString, sizeof(szFormatString));
              sprintf(szarString, szFormatString, NodeAbbrFromRecID(fromNode));
              strcat(szarString, NodeAbbrFromRecID(toNode));
              MessageBeep(MB_ICONINFORMATION);
              MessageBox(hWndDlg, szarString, TMS, MB_ICONINFORMATION | MB_OK);
            }
          }
        }
//
//  Ok, we made it this far, let's try the hook.
//
        nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COSTHOOKEDRUNAS),
              hWndDlg, (DLGPROC)COSTHOOKEDRUNASMsgProc, (LPARAM)PR.numPieces);
        if(!nRc)
          break;
//
//  Determine the indexes into the RELIEFPOINTS structure
//
        bFound = FALSE;
        for(nI = 0; nI < PR.numPieces; nI++)
        {
          runStart[nI] = NO_RECORD;
          runEnd[nI] = NO_RECORD;
          for(bFound = FALSE, nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
          {
            if(m_pRELIEFPOINTS[nJ].NODESrecordID == PR.piece[nI].fromNODESrecordID &&
                  m_pRELIEFPOINTS[nJ].TRIPSrecordID == PR.piece[nI].fromTRIPSrecordID &&
                  runStart[nI] == NO_RECORD)
            { 
              runStart[nI] = nJ;
            }
            else if(m_pRELIEFPOINTS[nJ].NODESrecordID == PR.piece[nI].toNODESrecordID &&
                  m_pRELIEFPOINTS[nJ].TRIPSrecordID == PR.piece[nI].toTRIPSrecordID &&
                  runStart[nI] != NO_RECORD)
            {
              runEnd[nI] = nJ;
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
            break;
        }
        if(!bFound)
          break;
//
//  Determine the indexes into the RUNLISTDATA structure
//
        for(nI = 0; nI < numRuns; nI++)
        {
          runNumber = runs[nI];
          for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
          {
            if(RUNLISTDATA[nJ].runNumber == runNumber)
            {
              runlistIndex[nI] = nJ;
              break;
            }
          }
        }
//
//  Modify the run and piece numbers in the RELIEFPOINTS structure.
//  The resulting run gets the lowest of the run numbers.
//
        for(nI = 0; nI < PR.numPieces; nI++)
        {
          for(nJ = runStart[nI]; nJ <= runEnd[nI]; nJ++)
          {
            if(nJ != runEnd[nI])
            {
              m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nJ].start.runNumber = runs[0];
              m_pRELIEFPOINTS[nJ].start.pieceNumber = nI + 1;
            }
            if(nJ != runStart[nI])
            {
              m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nJ].end.runNumber = runs[0];
              m_pRELIEFPOINTS[nJ].end.pieceNumber = nI + 1;
            }
          }
        }
//
//  Modify the first run in, and delete the other runs from, RUNLISTDATA
//
        RUNLISTDATA[runlistIndex[0]].runNumber = runs[0];
        RUNLISTDATA[runlistIndex[0]].cutAsRuntype = hookedRunRuntype;
        for(nI = 1; nI < numRuns; nI++)
        {
          RUNLISTDATA[runlistIndex[nI]].runNumber = NO_RECORD;
          RUNLISTDATA[runlistIndex[nI]].cutAsRuntype = NO_RUNTYPE;
          RUNLISTDATA[runlistIndex[nI]].COMMENTSrecordID = NO_RECORD;
          RUNLISTDATA[runlistIndex[nI]].frozenFlag = FALSE;
        }
//
//  Delete all the runs in question from the listbox
//
        for(nI = 0; nI < numRuns; nI++)
        {
          runNumber = runs[nI];
          numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          for(nJ = numItems - 1; nJ >= 0; nJ--)
          {
            if(SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == runNumber)
              SendMessage(hCtlRUNS, LB_DELETESTRING, (WPARAM)nJ, (LPARAM)0);
          }
        }
//
//  Cost and display the hooked run
//
        tempLong = RunCoster(&PR, hookedRunRuntype, &COST);
        DisplayRun(hCtlRUNS, FALSE, runs[0], &PR, &COST, TRUE);
//
//  Restore the cursor and free the runs array
//
        AssembleRuns();  // Bring the internal lists up to date
        TMSGlobalFree(hMemRuns);
        hSaveCursor = SetCursor(hCursorWait);
        break;
//
//  IDANALYZERUN
//
      case IDANALYZERUN:
        bGenerateTravelInstructions = TRUE;
        numItems = SendMessage(hCtlRUNS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        for(nI = 0; nI < numItems; nI++)
        {
          if(SendMessage(hCtlRUNS, LB_GETSEL, (WPARAM)nI, (LPARAM)0) > 0)
          {
            runNumber = SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
            {
              if(RUNLIST[nJ].runNumber == runNumber)
              {
                ANALYZERUNPassedData.flags = ANALYZERUN_FLAGS_MANCUT;
                ANALYZERUNPassedData.longValue = (long)nJ;
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ANALYZERUN),
                      hWndDlg, (DLGPROC)ANALYZERUNMsgProc, (LPARAM)&ANALYZERUNPassedData);
                break;
              }
            }
          }
        }
        bGenerateTravelInstructions = FALSE;
        break;
//
//  IDPROFILE
//
      case IDPROFILE:
        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROFILE),
              hWndDlg, (DLGPROC)PROFILEMsgProc, (LPARAM)pDI);
        break;
//
//  IDPLOTLEFT
//
      case IDPLOTLEFT:
        PlotLeft();
        break;
//
//  IDWORKRULES
//
      case IDWORKRULES:
        PremiumDefinition(pDI);
//        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WORKRULES),
//              hWndDlg, (DLGPROC)WORKRULESMsgProc, (LPARAM)pDI);
        break;
//
//  IDCANCEL
//
      case IDCANCEL:
        MessageBeep(MB_ICONINFORMATION);
        LoadString(hInst, ERROR_159, tempString, TEMPSTRING_LENGTH);
        if(MessageBox(hWndDlg, tempString, TMS, MB_ICONINFORMATION | MB_YESNO) == IDYES)
          EndDialog(hWndDlg, FALSE);
        break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Classic_Runcutter);
          break;
//
//  IDSAVE
//  IDOK
//
      case IDSAVE:
      case IDOK:
        MessageBeep(MB_ICONINFORMATION);
        LoadString(hInst, ERROR_160, tempString, TEMPSTRING_LENGTH);
        if(MessageBox(hWndDlg, tempString, TMS, MB_ICONINFORMATION | MB_YESNO) == IDNO)
          break;
        hSaveCursor = SetCursor(hCursorWait);
//
//  Nuke the previous runcut, but get the last recordID first
//
        rcode2 = btrieve(B_GETLAST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        recordID = AssignRecID(rcode2, RUNS.recordID);
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
//
//  Loop through the cut runs
//
        numInternalRuns = AssembleRuns();
        for(nI = 0; nI < numInternalRuns; nI++)
        {
//
//  Establish the runtype and comment code
//
          RUNS.cutAsRuntype = NO_RECORD;
          RUNS.COMMENTSrecordID = NO_RECORD;
          for(nK = 0; nK < MAXRUNSINRUNLIST; nK++)
          {
            if(RUNLIST[nI].runNumber == RUNLISTDATA[nK].runNumber)
            {
              RUNS.cutAsRuntype = RUNLISTDATA[nK].cutAsRuntype;
              RUNS.COMMENTSrecordID = RUNLISTDATA[nK].COMMENTSrecordID;
              break;
            }
          }
//
//  Loop through the pieces and write out the runs
//
          for(nJ = 0; nJ < RUNLIST[nI].run.numPieces; nJ++)
          {
            if(RUNLIST[nI].recordID[nJ] == NO_RECORD)
            {
              RUNS.recordID = recordID++;
           			RUNS.flags = 0L;
           			RUNS.prior.startTime = NO_TIME;
           			RUNS.prior.endTime = NO_TIME;
        	   		RUNS.after.startTime = NO_TIME;
        		   	RUNS.after.endTime = NO_TIME;
            }
            else
            {
              RUNS.recordID = RUNLIST[nI].recordID[nJ];
           			RUNS.flags = RUNLIST[nI].run.piece[nJ].flags;
		           	RUNS.prior.startTime = RUNLIST[nI].run.piece[nJ].prior.startTime;
			           RUNS.prior.endTime = RUNLIST[nI].run.piece[nJ].prior.endTime;
        	   		RUNS.after.startTime = RUNLIST[nI].run.piece[nJ].after.startTime;
        			   RUNS.after.endTime = RUNLIST[nI].run.piece[nJ].after.endTime;
            }
            RUNS.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
            RUNS.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            RUNS.start.TRIPSrecordID = RUNLIST[nI].run.piece[nJ].fromTRIPSrecordID;
            RUNS.start.NODESrecordID = RUNLIST[nI].run.piece[nJ].fromNODESrecordID;
            RUNS.end.TRIPSrecordID = RUNLIST[nI].run.piece[nJ].toTRIPSrecordID;
            RUNS.end.NODESrecordID = RUNLIST[nI].run.piece[nJ].toNODESrecordID;
            RUNS.runNumber = RUNLIST[nI].runNumber;
            RUNS.pieceNumber = nJ + 1;
            btrieve(B_INSERT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          }
        }
        SetCursor(hSaveCursor);
        if(wParam == IDOK)
          EndDialog(hWndDlg, TRUE);
        break;
    }
    break;

  default:
    return FALSE;
 }
 return TRUE;
}


int GetSelectedRunNumbers(HWND hWndDlg, HANDLE hCtlRUNS, HGLOBAL *hMemRuns, int **runs)
{
  HGLOBAL hMemSelections = NULL;
  BOOL    bFound;
  int     nI;
  int     nJ;
  int     numSelected;
  int     numRuns;
  int     runNumber;
  int    *selections;
//
//  Get the selections
//
  numSelected = SendMessage(hCtlRUNS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
  if(numSelected == 0 || numSelected == LB_ERR)
  {
    return(NO_RECORD);
  }
  hMemSelections = GlobalAlloc(GPTR, numSelected * sizeof(int));
  selections = (int *)hMemSelections;
  if(hMemSelections == NULL || selections == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return(NO_RECORD);
  }
//
//  Get the run numbers associated with the selections.  There won't be more
//  than "numSelected" unique run numbers, so use that number in the allocation.
//
  SendMessage(hCtlRUNS, LB_GETSELITEMS, (WPARAM)numSelected, (LPARAM)(LPINT)selections);
  *hMemRuns = GlobalAlloc(GPTR, numSelected * sizeof(int));
  *runs = (int *)(*hMemRuns);
  if(*hMemRuns == NULL || *runs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    TMSGlobalFree(hMemSelections);
    return(NO_RECORD);
  }
  numRuns = 0;
//
//  Loop through the selections to determine the run numbers
//
  for(nI = 0; nI < numSelected; nI++)
  {
    runNumber = SendMessage(hCtlRUNS, LB_GETITEMDATA, (WPARAM)selections[nI], (LPARAM)0);
    for(bFound = FALSE, nJ = 0; nJ < numRuns; nJ++)
    {
      if(runNumber == *(*runs + nJ))
      {
        bFound = TRUE;
        break;
      }
    }
    if(bFound)
      continue;
    *(*runs + numRuns) = runNumber;
    numRuns++;
  }
  TMSGlobalFree(hMemSelections);

  return(numRuns);
}


void DisplayRun(HANDLE hCtlRUNS, BOOL bFrozen,
      int runNumber, PROPOSEDRUNDef *pPR, COSTDef *pC, BOOL bSetTopIndex)
{
  int nI;
  int nJ;
//
//  Loop through the pieces
//
  for(nI = 0; nI < pPR->numPieces; nI++)
  {
//
//  Frozen Flag / Run number
//
    if(bFrozen)
      sprintf(tempString, "F%4d", runNumber);
    else
      sprintf(tempString, "%5d", runNumber);
    strcat(tempString, "\t");
//
//  Piece number
//
    sprintf(szarString, "%3d", nI + 1);
    strcat(tempString, szarString);
    strcat(tempString, "\t");
//
//  Block number
//
    TRIPSKey0.recordID = pPR->piece[nI].fromTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    sprintf(szarString, "%5ld", TRIPS.standard.blockNumber);
    strcat(tempString, szarString);
    strcat(tempString, "\t");
//
//  On location
//
    strcat(tempString, NodeAbbrFromRecID(pPR->piece[nI].fromNODESrecordID));
    strcat(tempString, "\t");
//
//  On time
//
    strcat(tempString, Tchar(pPR->piece[nI].fromTime));
    strcat(tempString, "\t");
//
//  Off time
//
    strcat(tempString, Tchar(pPR->piece[nI].toTime));
    strcat(tempString, "\t");
//
//  Off location
//
    strcat(tempString, NodeAbbrFromRecID(pPR->piece[nI].toNODESrecordID));
    strcat(tempString, "\t");
//
//  Platform time
//
    strcat(tempString, chhmm(pC->PIECECOST[nI].platformTime));
    strcat(tempString, "\t");
//
//  Premium time
//
    strcat(tempString, chhmm((pPR->numPieces > 1
          ? pC->PIECECOST[nI].premiumTime : pC->TOTAL.premiumTime)));
    strcat(tempString, "\t");
//
//  Over time
//
    strcpy(szarString, chhmm((pPR->numPieces > 1 ? NO_TIME : pC->TOTAL.overTime)));
    strcat(tempString, chhmm((pPR->numPieces > 1 ? NO_TIME : pC->TOTAL.overTime)));
    strcat(tempString, "\t");
//
//  Pay time
//
    strcpy(szarString, chhmm((pPR->numPieces > 1 ? NO_TIME : pC->TOTAL.payTime)));
    strcat(tempString, chhmm((pPR->numPieces > 1 ? NO_TIME : pC->TOTAL.payTime)));
//
//  Display it
//
    nJ = SendMessage(hCtlRUNS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlRUNS, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)runNumber);
    if(bSetTopIndex && nI == 0)
      SendMessage(hCtlRUNS, LB_SETTOPINDEX, (WPARAM)nJ, (LPARAM)0);
  }
//
//  Provide a sum if more than one piece
//
  if(pPR->numPieces > 1)
  {
//
//  Run number
//
    if(bFrozen)
      sprintf(tempString, "F%4d", runNumber);
    else
      sprintf(tempString, "%5d", runNumber);
    strcat(tempString, "\t");
//
//  Piece number
//
    strcat(tempString, "  T");
//
//  Skip over block and on and off locations and times
//
    for(nJ = 0; nJ < 6; nJ++)
    {
      strcat(tempString, "\t");
    }
//
//  Platform time
//
    strcat(tempString, chhmm(pC->TOTAL.platformTime));
    strcat(tempString, "\t");
//
//  Premium time
//
    strcat(tempString, chhmm(pC->TOTAL.premiumTime));
    strcat(tempString, "\t");
//
//  Over time
//
    strcat(tempString, chhmm(pC->TOTAL.overTime));
    strcat(tempString, "\t");
//
//  Pay time
//
    strcat(tempString, chhmm(pC->TOTAL.payTime));
//
//  Display it
//
    nJ = SendMessage(hCtlRUNS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlRUNS, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)runNumber);
  }

    sprintf(tempString, "%5d\t-------------------------------", runNumber);
    nJ = SendMessage(hCtlRUNS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtlRUNS, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)runNumber);

  return;
}
