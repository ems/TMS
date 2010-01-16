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

typedef struct SWAPStruct
{
  long cutAsRuntype[2];
  int  runNumber[2];
  PROPOSEDRUNDef PROPOSEDRUN[2];
} SWAPDef;

BOOL CALLBACK VERIFYSWAPMsgProc(HWND, UINT, WPARAM, LPARAM);

static long *pRunIndexes;
static int  tabPos[TABSET] = {7, 12, 19, 27, 36, 44, 50, 56};
static int  numPieces[2];

BOOL CALLBACK SWAPPIECESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlHEADER[2];
  static HANDLE hCtlTITLE[2];
  static HANDLE hCtlLIST[2];
  static HANDLE hCtlSPREADOT[2];
  static HANDLE hCtlMAKEUP[2];
  static HANDLE hCtlPAIDBREAKS[2];
  static HANDLE hCtlOVERTIME[2];
  static HANDLE hCtlPAYTIME[2];
  static HANDLE hCtlIDSWAP;
  PROPOSEDRUNDef PROPOSEDRUN;
  PROPOSEDRUNPIECESDef piece;
  COSTDef COST;
  SWAPDef SWAP;
  WORD    DlgWidthUnits;
  long    cutAsRuntype;
  long    totalPlat;
  long    totalRpt;
  long    totalTrv;
  long    runNumber;
  long    travelTime;
  BOOL    bRC;
  int     adjustedTabPos[TABSET];
  int     nI;
  int     nJ;
  int     nK;
  int     pieceToSwap[2];
  int     startBegIndex[2];
  int     startFinIndex[2];
  int     endBegIndex[2];
  int     endFinIndex[2];
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get the two indexes into RUNLIST
//
      pRunIndexes = (long *)lParam;
      if(pRunIndexes == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlHEADER[0] = GetDlgItem(hWndDlg, SWAPPIECES_HEADER1);
      hCtlTITLE[0] = GetDlgItem(hWndDlg, SWAPPIECES_TITLE1);
      hCtlLIST[0] = GetDlgItem(hWndDlg, SWAPPIECES_LIST1);
      hCtlSPREADOT[0] = GetDlgItem(hWndDlg, SWAPPIECES_SPREADOT1);
      hCtlMAKEUP[0] = GetDlgItem(hWndDlg, SWAPPIECES_MAKEUP1);
      hCtlPAIDBREAKS[0] = GetDlgItem(hWndDlg, SWAPPIECES_PAIDBREAKS1);
      hCtlOVERTIME[0] = GetDlgItem(hWndDlg, SWAPPIECES_OVERTIME1);
      hCtlPAYTIME[0] = GetDlgItem(hWndDlg, SWAPPIECES_PAYTIME1);
      hCtlHEADER[1] = GetDlgItem(hWndDlg, SWAPPIECES_HEADER2);
      hCtlTITLE[1] = GetDlgItem(hWndDlg, SWAPPIECES_TITLE2);
      hCtlLIST[1] = GetDlgItem(hWndDlg, SWAPPIECES_LIST2);
      hCtlSPREADOT[1] = GetDlgItem(hWndDlg, SWAPPIECES_SPREADOT2);
      hCtlMAKEUP[1] = GetDlgItem(hWndDlg, SWAPPIECES_MAKEUP2);
      hCtlPAIDBREAKS[1] = GetDlgItem(hWndDlg, SWAPPIECES_PAIDBREAKS2);
      hCtlOVERTIME[1] = GetDlgItem(hWndDlg, SWAPPIECES_OVERTIME2);
      hCtlPAYTIME[1] = GetDlgItem(hWndDlg, SWAPPIECES_PAYTIME2);
      hCtlIDSWAP = GetDlgItem(hWndDlg, IDOK);
//
//  Set up the tab stops for SWAPPIECES_TITLE[] and SWAPPIECES_LIST[]
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nI = 0; nI < TABSET; nI++)
      {
        adjustedTabPos[nI] = (DlgWidthUnits * tabPos[nI] * 2);
      }
      LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
      for(nI = 0; nI < 2; nI++)
      {
        SendMessage(hCtlTITLE[nI], LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
        SendMessage(hCtlTITLE[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST[nI], LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      }
//
//  Display the runs
//
      for(nI = 0; nI < 2; nI++)
      {
        nJ = pRunIndexes[nI];
        runNumber = RUNLIST[nJ].runNumber;
        memcpy(&PROPOSEDRUN, &RUNLIST[nJ].run, sizeof(PROPOSEDRUN));
        numPieces[nI] = PROPOSEDRUN.numPieces;
        for(cutAsRuntype = NO_RECORD, nK = 0; nK < MAXRUNSINRUNLIST; nK++)
        {
          if(runNumber == RUNLISTDATA[nK].runNumber)
          {
            cutAsRuntype = RUNLISTDATA[nK].cutAsRuntype;
            break;
          }
        }
//
//  Cost the run and fill in the controls
//
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  Cut as runtype
//
        nJ = (short int)LOWORD(cutAsRuntype);
        nK = (short int)HIWORD(cutAsRuntype);
        if(nJ >= 0 && nK >= 0)
        {
          sprintf(tempString, "Run %ld (%s)", runNumber, RUNTYPE[nJ][nK].localName);
          SendMessage(hCtlHEADER[nI], WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  The list box of the pieces of the run
//
        totalPlat = 0;
        totalRpt = 0;
        totalTrv = 0;
        for(nJ = 0; nJ < PROPOSEDRUN.numPieces; nJ++)
        {
//
//  Block Number
//
          TRIPSKey0.recordID = PROPOSEDRUN.piece[nJ].fromTRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          ltoa(TRIPS.standard.blockNumber, tempString, 10);
          strcat(tempString, "\t");
//
//  Piece Number
//
          sprintf(szarString, "%3d", nJ + 1);
          strcat(tempString, szarString);
          strcat(tempString, "\t");
//
//  On Location
//
          strcat(tempString, NodeAbbrFromRecID(PROPOSEDRUN.piece[nJ].fromNODESrecordID));
          strcat(tempString, "\t");
//
//  On Time
//
          strcat(tempString, Tchar(PROPOSEDRUN.piece[nJ].fromTime));
          strcat(tempString, "\t");
//
//  Off Time
//
          strcat(tempString, Tchar(PROPOSEDRUN.piece[nJ].toTime));
          strcat(tempString, "\t");
//
//  Off Location
//
          strcat(tempString, NodeAbbrFromRecID(PROPOSEDRUN.piece[nJ].toNODESrecordID));
          strcat(tempString, "\t");
//
//  Platform time
//
          strcat(tempString, chhmm(COST.PIECECOST[nJ].platformTime));
          strcat(tempString, "\t");
          totalPlat += COST.PIECECOST[nJ].platformTime;
//
//  Report and Travel times
//
          strcat(tempString, chhmm(COST.PIECECOST[nJ].reportTime));
          strcat(tempString, "\t");
          totalRpt += COST.PIECECOST[nJ].reportTime;
          travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
          strcat(tempString, chhmm(travelTime));
          totalTrv += travelTime;
          SendMessage(hCtlLIST[nI], LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
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
          SendMessage(hCtlLIST[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  Spread O/T
//
        SendMessage(hCtlSPREADOT[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.spreadOvertime));
//
//  Makeup time
//
        SendMessage(hCtlMAKEUP[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.makeUpTime));
//
//  Paid Breaks
//
        SendMessage(hCtlPAIDBREAKS[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.paidBreak));
//
//  Overtime
//
        SendMessage(hCtlOVERTIME[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.overTime));
//
//  Pay time
//
        SendMessage(hCtlPAYTIME[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.payTime));
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  SWAPPIECES_LIST1 listbox
//  SWAPPIECES_LIST2 listbox
//
        case SWAPPIECES_LIST1:
        case SWAPPIECES_LIST2:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              nI = (int)SendMessage(hCtlLIST[0], LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              nJ = (int)SendMessage(hCtlLIST[1], LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR || nJ == LB_ERR)
                EnableWindow(hCtlIDSWAP, FALSE);
              else
                EnableWindow(hCtlIDSWAP, TRUE);
              break;
          }
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Swap_pieces);
          break;
//
//  IDOK (Labelled "Swap...")
//
        case IDOK:
//
//  Get the two original runs
//
          for(nI = 0; nI < 2; nI++)
          {
            nJ = pRunIndexes[nI];
            SWAP.runNumber[nI] = RUNLIST[nJ].runNumber;
            memcpy(&SWAP.PROPOSEDRUN[nI], &RUNLIST[nJ].run, sizeof(PROPOSEDRUN));
          }
          pieceToSwap[0] = (int)SendMessage(hCtlLIST[0], LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          pieceToSwap[1] = (int)SendMessage(hCtlLIST[1], LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          memcpy(&piece,
                 &SWAP.PROPOSEDRUN[0].piece[pieceToSwap[0]], sizeof(PROPOSEDRUNPIECESDef));
          memcpy(&SWAP.PROPOSEDRUN[0].piece[pieceToSwap[0]],
                 &SWAP.PROPOSEDRUN[1].piece[pieceToSwap[1]], sizeof(PROPOSEDRUNPIECESDef));
          memcpy(&SWAP.PROPOSEDRUN[1].piece[pieceToSwap[1]],
                 &piece, sizeof(PROPOSEDRUNPIECESDef));
//
//  Show him the results so he can decide
//
          bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SWAPPIECES),
              hWndDlg, (DLGPROC)VERIFYSWAPMsgProc, (LPARAM)&SWAP);
//
//  If he liked the swap, make the changes and return
//
          if(bRC)
          {
            for(nI = 0; nI < 2; nI++)
            {
              for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
              {
                if(SWAP.runNumber[nI] == RUNLISTDATA[nJ].runNumber)
                {
                  RUNLISTDATA[nJ].cutAsRuntype = SWAP.cutAsRuntype[nI];
                  break;
                }
              }
              startBegIndex[nI] = NO_RECORD;
              endBegIndex[nI] = NO_RECORD;
              for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
              {
                if(m_pRELIEFPOINTS[nJ].start.runNumber == SWAP.runNumber[nI] &&
                      m_pRELIEFPOINTS[nJ].start.pieceNumber == pieceToSwap[nI] + 1)
                {
                  if(startBegIndex[nI] == NO_RECORD)
                    startBegIndex[nI] = nJ;
                  startFinIndex[nI] = nJ;
                }
                if(m_pRELIEFPOINTS[nJ].end.runNumber == SWAP.runNumber[nI] &&
                      m_pRELIEFPOINTS[nJ].end.pieceNumber == pieceToSwap[nI] + 1)
                {
                  if(endBegIndex[nI] == NO_RECORD)
                    endBegIndex[nI] = nJ;
                  endFinIndex[nI] = nJ;
                }
              }
            }
            for(nI = 0; nI < 2; nI++)
            {
              nK = (nI == 0 ? 1 : 0);
              for(nJ = startBegIndex[nI]; nJ <= startFinIndex[nI]; nJ++)
              {
                m_pRELIEFPOINTS[nJ].start.runNumber = SWAP.runNumber[nK];
                m_pRELIEFPOINTS[nJ].start.pieceNumber = pieceToSwap[nK] + 1;
              }
              for(nJ = endBegIndex[nI]; nJ <= endFinIndex[nI]; nJ++)
              {
                m_pRELIEFPOINTS[nJ].end.runNumber = SWAP.runNumber[nK];
                m_pRELIEFPOINTS[nJ].end.pieceNumber = pieceToSwap[nK] + 1;
              }
            }
          }
          EndDialog(hWndDlg, bRC);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//xxxxxxxxxxxxxxxxxxxxxxxxx


BOOL CALLBACK VERIFYSWAPMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlHEADER[2];
  static HANDLE hCtlTITLE[2];
  static HANDLE hCtlLIST[2];
  static HANDLE hCtlSPREADOT[2];
  static HANDLE hCtlMAKEUP[2];
  static HANDLE hCtlPAIDBREAKS[2];
  static HANDLE hCtlOVERTIME[2];
  static HANDLE hCtlPAYTIME[2];
  static HANDLE hCtlGROUPBOX;
  static HANDLE hCtlIDOK;
  static SWAPDef *pSWAP;
  COSTDef COST;
  WORD    DlgWidthUnits;
  long    totalPlat;
  long    totalRpt;
  long    totalTrv;
  long    travelTime;
  int     adjustedTabPos[TABSET];
  int     nI;
  int     nJ;
  int     nK;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get a pointer to the SWAP structure
//
      pSWAP = (SWAPDef *)lParam;
      if(pSWAP == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlHEADER[0] = GetDlgItem(hWndDlg, SWAPPIECES_HEADER1);
      hCtlTITLE[0] = GetDlgItem(hWndDlg, SWAPPIECES_TITLE1);
      hCtlLIST[0] = GetDlgItem(hWndDlg, SWAPPIECES_LIST1);
      hCtlSPREADOT[0] = GetDlgItem(hWndDlg, SWAPPIECES_SPREADOT1);
      hCtlMAKEUP[0] = GetDlgItem(hWndDlg, SWAPPIECES_MAKEUP1);
      hCtlPAIDBREAKS[0] = GetDlgItem(hWndDlg, SWAPPIECES_PAIDBREAKS1);
      hCtlOVERTIME[0] = GetDlgItem(hWndDlg, SWAPPIECES_OVERTIME1);
      hCtlPAYTIME[0] = GetDlgItem(hWndDlg, SWAPPIECES_PAYTIME1);
      hCtlHEADER[1] = GetDlgItem(hWndDlg, SWAPPIECES_HEADER2);
      hCtlTITLE[1] = GetDlgItem(hWndDlg, SWAPPIECES_TITLE2);
      hCtlLIST[1] = GetDlgItem(hWndDlg, SWAPPIECES_LIST2);
      hCtlSPREADOT[1] = GetDlgItem(hWndDlg, SWAPPIECES_SPREADOT2);
      hCtlMAKEUP[1] = GetDlgItem(hWndDlg, SWAPPIECES_MAKEUP2);
      hCtlPAIDBREAKS[1] = GetDlgItem(hWndDlg, SWAPPIECES_PAIDBREAKS2);
      hCtlOVERTIME[1] = GetDlgItem(hWndDlg, SWAPPIECES_OVERTIME2);
      hCtlPAYTIME[1] = GetDlgItem(hWndDlg, SWAPPIECES_PAYTIME2);
      hCtlGROUPBOX = GetDlgItem(hWndDlg, SWAPPIECES_GROUPBOX);
      hCtlIDOK = GetDlgItem(hWndDlg, IDOK);
//
//  ...and change some text
//
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Please Verify");
      SendMessage(hCtlGROUPBOX, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Modified runs");
      SendMessage(hCtlIDOK, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"&OK");
      EnableWindow(hCtlIDOK, TRUE);
//
//  Set up the tab stops for SWAPPIECES_TITLE[] and SWAPPIECES_LIST[]
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nI = 0; nI < TABSET; nI++)
      {
        adjustedTabPos[nI] = (DlgWidthUnits * tabPos[nI] * 2);
      }
      LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
      for(nI = 0; nI < 2; nI++)
      {
        SendMessage(hCtlTITLE[nI], LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
        SendMessage(hCtlTITLE[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlLIST[nI], LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      }
//
//  Display the runs
//
      for(nI = 0; nI < 2; nI++)
      {

//
//  Cost the run and fill in the controls
//
        pSWAP->cutAsRuntype[nI] = RunCoster(&(pSWAP->PROPOSEDRUN[nI]), NO_RECORD, &COST);
//
//  Cut as runtype
//
        nJ = (short int)LOWORD(pSWAP->cutAsRuntype[nI]);
        nK = (short int)HIWORD(pSWAP->cutAsRuntype[nI]);
        if(nJ >= 0 && nK >= 0)
          strcpy(szarString, RUNTYPE[nJ][nK].localName);
        else
          strcpy(szarString, "Unclassified");
        sprintf(tempString, "Run %ld (%s)", pSWAP->runNumber[nI], szarString);
        SendMessage(hCtlHEADER[nI], WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  The list box of the pieces of the run
//
        totalPlat = 0;
        totalRpt = 0;
        totalTrv = 0;
        for(nJ = 0; nJ < pSWAP->PROPOSEDRUN[nI].numPieces; nJ++)
        {
//
//  Block Number
//
          TRIPSKey0.recordID = pSWAP->PROPOSEDRUN[nI].piece[nJ].fromTRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          ltoa(TRIPS.standard.blockNumber, tempString, 10);
          strcat(tempString, "\t");
//
//  Piece Number
//
          sprintf(szarString, "%3d", nJ + 1);
          strcat(tempString, szarString);
          strcat(tempString, "\t");
//
//  On Location
//
          strcat(tempString, NodeAbbrFromRecID(pSWAP->PROPOSEDRUN[nI].piece[nJ].fromNODESrecordID));
          strcat(tempString, "\t");
//
//  On Time
//
          strcat(tempString, Tchar(pSWAP->PROPOSEDRUN[nI].piece[nJ].fromTime));
          strcat(tempString, "\t");
//
//  Off Time
//
          strcat(tempString, Tchar(pSWAP->PROPOSEDRUN[nI].piece[nJ].toTime));
          strcat(tempString, "\t");
//
//  Off Location
//
          strcat(tempString, NodeAbbrFromRecID(pSWAP->PROPOSEDRUN[nI].piece[nJ].toNODESrecordID));
          strcat(tempString, "\t");
//
//  Platform time
//
          strcat(tempString, chhmm(COST.PIECECOST[nJ].platformTime));
          strcat(tempString, "\t");
          totalPlat += COST.PIECECOST[nJ].platformTime;
//
//  Report and Travel times
//
          strcat(tempString, chhmm(COST.PIECECOST[nJ].reportTime));
          strcat(tempString, "\t");
          totalRpt += COST.PIECECOST[nJ].reportTime;
          travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
          strcat(tempString, chhmm(travelTime));
          totalTrv += travelTime;
          SendMessage(hCtlLIST[nI], LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
        }
//
//  Add a total line if there's more than one piece
//
        if(pSWAP->PROPOSEDRUN[nI].numPieces > 1)
        {
          strcpy(tempString, "\t\t\t\t\t\t");
          strcat(tempString, chhmm(totalPlat));
          strcat(tempString, "\t");
          strcat(tempString, chhmm(totalRpt));
          strcat(tempString, "\t");
          strcat(tempString, chhmm(totalTrv));
          SendMessage(hCtlLIST[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  Spread O/T
//
        SendMessage(hCtlSPREADOT[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.spreadOvertime));
//
//  Makeup time
//
        SendMessage(hCtlMAKEUP[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.makeUpTime));
//
//  Paid Breaks
//
        SendMessage(hCtlPAIDBREAKS[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.paidBreak));
//
//  Overtime
//
        SendMessage(hCtlOVERTIME[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.overTime));
//
//  Pay time
//
        SendMessage(hCtlPAYTIME[nI], WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)chhmm(COST.TOTAL.payTime));
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Swap_pieces);
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
