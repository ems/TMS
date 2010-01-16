//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static HANDLE hCtlBREAKOF;
static HANDLE hCtlMINBREAKOF;
static HANDLE hCtlDESBREAKOF;
static HANDLE hCtlMAXBREAKOF;
static HANDLE hCtlCONTIG;
static HANDLE hCtlMINCONTIG;
static HANDLE hCtlDESCONTIG;
static HANDLE hCtlMAXCONTIG;
static HANDLE hCtlSPD;
static HANDLE hCtlMAXSPD;
static HANDLE hCtlTRAVELCOUNTS;
static HANDLE hCtlLARGEST;
static HANDLE hCtlLARGESTNOTPAID;
static HANDLE hCtlLARGESTPAID;
static HANDLE hCtlLARGESTPAIDFROM;
static HANDLE hCtlLARGESTTO;
static HANDLE hCtlLARGESTPAIDTO;
static HANDLE hCtlLARGESTPAIDTHROUGHWHEN;
static HANDLE hCtlLARGESTPAIDTHROUGHAMOUNT;
static HANDLE hCtlSMALLER;
static HANDLE hCtlSMALLERNOTPAID;
static HANDLE hCtlSMALLERPAID;
static HANDLE hCtlSMALLERPAIDFROM;
static HANDLE hCtlSMALLERTO;
static HANDLE hCtlSMALLERPAIDTO;
static HANDLE hCtlSMALLERPAIDTHROUGHWHEN;
static HANDLE hCtlSMALLERPAIDTHROUGHAMOUNT;
static HANDLE hCtlPAYSPDOT;

void EnableSections(int);

BOOL CALLBACK ADDRUNTYPEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlNAME;
  static HANDLE hCtlRUNTYPE;
  static HANDLE hCtl4DAY;
  static HANDLE hCtl5DAY;
  static HANDLE hCtlNUMPCES;
  static HANDLE hCtlMINPLATFORM;
  static HANDLE hCtlDESPLATFORM;
  static HANDLE hCtlMAXPLATFORM;
  static HANDLE hCtlMINPAY;
  static HANDLE hCtlDESPAY;
  static HANDLE hCtlMAXPAY;
  static HANDLE hCtlIDATTRIBUTES;
  static HANDLE hCtlIDDELETE;
  static HANDLE hCtlCREWONLY;
  static HANDLE hCtlBRINGUPTO;
  static HANDLE hCtlMINLAY;
  static HANDLE hCtlMINLAYTIME;
  static HANDLE hCtlMINLAYSPAN;
  static HANDLE hCtlMINLAYNOTBEFORE;
  static HANDLE hCtlMINLAYLABEL;
  static int    runtypeIndex;
  static int    slotNumber;
  long   numPieces;
  long   maxSpreadTime;
  int    nI;
  int    nJ;
  int    nK;
  int    rcode2;
  int    numLabels;
  BOOL   bFound;
  BOOL   bEnable;
  char   szLocalName[sizeof(RUNTYPE[0][0].localName)];
  char   *runtypes[] = {STRAIGHT_TEXT, TWOPIECE_TEXT, MULTIPIECE_TEXT,
                        SWING_TEXT,    TRIPPER_TEXT,  ILLEGAL_TEXT};
  short  int wmId;
  short  int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Retrieve the input runtype (if there was one)
//
      runtypeIndex = (short int)LOWORD(lParam);
      slotNumber = (short int)HIWORD(lParam);
//
//  Set up the handles to the controls
//
      hCtlNAME = GetDlgItem(hWndDlg, ADDRUNTYPE_NAME);
      hCtlRUNTYPE = GetDlgItem(hWndDlg, ADDRUNTYPE_RUNTYPE);
      hCtl4DAY = GetDlgItem(hWndDlg, ADDRUNTYPE_4DAY);
      hCtl5DAY = GetDlgItem(hWndDlg, ADDRUNTYPE_5DAY);
      hCtlNUMPCES = GetDlgItem(hWndDlg, ADDRUNTYPE_NUMPCES);
      hCtlMINPLATFORM = GetDlgItem(hWndDlg, ADDRUNTYPE_MINPLATFORM);
      hCtlDESPLATFORM = GetDlgItem(hWndDlg, ADDRUNTYPE_DESPLATFORM);
      hCtlMAXPLATFORM = GetDlgItem(hWndDlg, ADDRUNTYPE_MAXPLATFORM);
      hCtlMINPAY = GetDlgItem(hWndDlg, ADDRUNTYPE_MINPAY);
      hCtlDESPAY = GetDlgItem(hWndDlg, ADDRUNTYPE_DESPAY);
      hCtlMAXPAY = GetDlgItem(hWndDlg, ADDRUNTYPE_MAXPAY);
      hCtlBREAKOF = GetDlgItem(hWndDlg, ADDRUNTYPE_BREAKOF);
      hCtlMINBREAKOF = GetDlgItem(hWndDlg, ADDRUNTYPE_MINBREAKOF);
      hCtlDESBREAKOF = GetDlgItem(hWndDlg, ADDRUNTYPE_DESBREAKOF);
      hCtlMAXBREAKOF = GetDlgItem(hWndDlg, ADDRUNTYPE_MAXBREAKOF);
      hCtlCONTIG = GetDlgItem(hWndDlg, ADDRUNTYPE_CONTIG);
      hCtlMINCONTIG = GetDlgItem(hWndDlg, ADDRUNTYPE_MINCONTIG);
      hCtlDESCONTIG = GetDlgItem(hWndDlg, ADDRUNTYPE_DESCONTIG);
      hCtlMAXCONTIG = GetDlgItem(hWndDlg, ADDRUNTYPE_MAXCONTIG);
      hCtlSPD = GetDlgItem(hWndDlg, ADDRUNTYPE_SPD);
      hCtlMAXSPD = GetDlgItem(hWndDlg, ADDRUNTYPE_MAXSPD);
      hCtlIDATTRIBUTES = GetDlgItem(hWndDlg, IDATTRIBUTES);
      hCtlIDDELETE = GetDlgItem(hWndDlg, IDDELETE);
      hCtlCREWONLY = GetDlgItem(hWndDlg, ADDRUNTYPE_CREWONLY);
      hCtlTRAVELCOUNTS = GetDlgItem(hWndDlg, ADDRUNTYPE_TRAVELCOUNTS);
      hCtlLARGEST = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGEST);
      hCtlLARGESTNOTPAID = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTNOTPAID);
      hCtlLARGESTPAID = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTPAID);
      hCtlLARGESTPAIDFROM = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTPAIDFROM);
      hCtlLARGESTTO = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTTO);
      hCtlLARGESTPAIDTO = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTPAIDTO);
      hCtlLARGESTPAIDTHROUGHWHEN = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTPAIDTHROUGHWHEN);
      hCtlLARGESTPAIDTHROUGHAMOUNT = GetDlgItem(hWndDlg, ADDRUNTYPE_LARGESTPAIDTHROUGHAMOUNT);
      hCtlSMALLER = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLER);
      hCtlSMALLERNOTPAID = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERNOTPAID);
      hCtlSMALLERPAID = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERPAID);
      hCtlSMALLERPAIDFROM = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERPAIDFROM);
      hCtlSMALLERTO = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERTO);
      hCtlSMALLERPAIDTO = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERPAIDTO);
      hCtlSMALLERPAIDTHROUGHWHEN = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERPAIDTHROUGHWHEN);
      hCtlSMALLERPAIDTHROUGHAMOUNT = GetDlgItem(hWndDlg, ADDRUNTYPE_SMALLERPAIDTHROUGHAMOUNT);
      hCtlBRINGUPTO = GetDlgItem(hWndDlg, ADDRUNTYPE_BRINGUPTO);
      hCtlPAYSPDOT = GetDlgItem(hWndDlg, ADDRUNTYPE_PAYSPDOT);
      hCtlMINLAY = GetDlgItem(hWndDlg, ADDRUNTYPE_MINLAY);
      hCtlMINLAYTIME = GetDlgItem(hWndDlg, ADDRUNTYPE_MINLAYTIME);
      hCtlMINLAYSPAN = GetDlgItem(hWndDlg, ADDRUNTYPE_MINLAYSPAN);
      hCtlMINLAYNOTBEFORE = GetDlgItem(hWndDlg, ADDRUNTYPE_MINLAYNOTBEFORE);
      hCtlMINLAYLABEL = GetDlgItem(hWndDlg, ADDRUNTYPE_MINLAYLABEL);
//
//  Set up the predefined runtypes
//
      for(nI = 0; nI < NUMRUNTYPES; nI++)
      {
        SendMessage(hCtlRUNTYPE, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)runtypes[nI]);
      }
//
//  Did we come in with one?
//
      if(runtypeIndex != NO_RECORD)
      {
        EnableWindow(hCtlRUNTYPE, FALSE);
      }
//
//  Set up the node labels
//
      strcpy(szarString, "");
      rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
      numLabels = 0;
      while(rcode2 == 0)
      {
        strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
        for(nI = 0; nI < NODES_RELIEFLABELS_LENGTH; nI++)
        {
          if(tempString[nI] == ' ' || tempString[nI] == '-')
          {
            continue;
          }
          for(bFound = FALSE, nJ = 0; nJ < numLabels; nJ++)
          {
            if(tempString[nI] == szarString[nJ])
            {
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
          {
            szarString[numLabels] = tempString[nI];
            numLabels++;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
      SendMessage(hCtlMINLAYLABEL, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)"N/A");
      for(nI = 0; nI < numLabels; nI++)
      {
        tempString[0] = szarString[nI];
        tempString[1] = '\0';
        SendMessage(hCtlMINLAYLABEL, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      }

//
//  Set up the rest of the box
//
//  First time through: Do we have runtype already selected?  If so, show its properties.
//  Afterward: Is this runtype in use?
//
      nI = runtypeIndex;
      nJ = slotNumber;
      if(nI != NO_RECORD && nJ != NO_RECORD)
      {
        if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
        {
          SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
          SendMessage(hCtlRUNTYPE, CB_SELECTSTRING, (WPARAM)(-1), (LONG)(LPSTR)runtypes[nI]);
          itoa(RUNTYPE[nI][nJ].numPieces, tempString, 10);
          SendMessage(hCtlNUMPCES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(RUNTYPE[nI][nJ].flags & RTFLAGS_4DAY ? hCtl4DAY : hCtl5DAY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlMINPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minPlatformTime));
          SendMessage(hCtlDESPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].desPlatformTime));
          SendMessage(hCtlMAXPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].maxPlatformTime));
          SendMessage(hCtlMINPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minPayTime));
          SendMessage(hCtlDESPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].desPayTime));
          SendMessage(hCtlMAXPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].maxPayTime));
          SendMessage(hCtlMINBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minBreakOf));
          SendMessage(hCtlDESBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].desBreakOf));
          SendMessage(hCtlMAXBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].maxBreakOf));
          SendMessage(hCtlMINCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minContig));
          SendMessage(hCtlDESCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].desContig));
          SendMessage(hCtlMAXCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].maxContig));
          SendMessage(hCtlCREWONLY, BM_SETCHECK, (WPARAM)(RUNTYPE[nI][nJ].flags & RTFLAGS_CREWONLY), (LPARAM)0);
          SendMessage(hCtlTRAVELCOUNTS, BM_SETCHECK, (WPARAM)(RUNTYPE[nI][nJ].flags & RTFLAGS_TRAVELCOUNTS), (LPARAM)0);
          SendMessage(hCtlBRINGUPTO, BM_SETCHECK, (WPARAM)RUNTYPE[nI][nJ].flags & RTFLAGS_BRINGUPTO, (LPARAM)0);
          SendMessage(hCtlPAYSPDOT, BM_SETCHECK, (WPARAM)RUNTYPE[nI][nJ].flags & RTFLAGS_PAYSPDOT, (LPARAM)0);
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_MINLAY)
          {
            SendMessage(hCtlMINLAY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendMessage(hCtlMINLAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minLayTime));
            SendMessage(hCtlMINLAYSPAN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minLaySpan));
            SendMessage(hCtlMINLAYNOTBEFORE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].minLayNotBefore));
            if(RUNTYPE[nI][nJ].minLayLabel == '\0')
            {
              SendMessage(hCtlMINLAYLABEL, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            }
            else
            {
              for(nK = 1; nK < numLabels + 1; nK++)
              {
                SendMessage(hCtlMINLAYLABEL, CB_GETLBTEXT, (WPARAM)nK, (LONG)(LPSTR)tempString);
                if(tempString[0] == RUNTYPE[nI][nJ].minLayLabel[0])
                {
                  SendMessage(hCtlMINLAYLABEL, CB_SETCURSEL, (WPARAM)nK, (LPARAM)0);
                  break;
                }
              }
            }
          }
//
//  Paid Break (Largest)
//
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_PAIDBREAK)
          {
            SendMessage(hCtlLARGESTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendMessage(hCtlLARGESTPAIDFROM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidFrom));
            SendMessage(hCtlLARGESTPAIDTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidTo));
          }
          else if(RUNTYPE[nI][nJ].flags & RTFLAGS_LARGESTPAIDTHROUGHWHEN)
          {
            SendMessage(hCtlLARGESTPAIDTHROUGHWHEN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendMessage(hCtlLARGESTPAIDTHROUGHAMOUNT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidToWhen));
          }
          else
          {
            SendMessage(hCtlLARGESTNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            EnableWindow(hCtlLARGESTPAIDFROM, FALSE);
            EnableWindow(hCtlLARGESTPAIDTO, FALSE);
            EnableWindow(hCtlLARGESTPAIDTHROUGHAMOUNT, FALSE);
          }
//
//  Paid Break (Smaller)
//
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAID)
          {
            SendMessage(hCtlSMALLERPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendMessage(hCtlSMALLERPAIDFROM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidFromSmaller));
            SendMessage(hCtlSMALLERPAIDTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidToSmaller));
          }
          else if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAIDTHROUGHWHEN)
          {
            SendMessage(hCtlSMALLERPAIDTHROUGHWHEN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendMessage(hCtlSMALLERPAIDTHROUGHAMOUNT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].paidToWhenSmaller));
          }
          else
          {
            SendMessage(hCtlSMALLERNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            EnableWindow(hCtlSMALLERPAIDFROM, FALSE);
            EnableWindow(hCtlSMALLERPAIDTO, FALSE);
            EnableWindow(hCtlSMALLERPAIDTHROUGHAMOUNT, FALSE);
          }
//
//  Number of pieces
//
          EnableSections(RUNTYPE[nI][nJ].numPieces);
          if(RUNTYPE[nI][nJ].numPieces > 1)
          {
            SendMessage(hCtlMAXSPD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(RUNTYPE[nI][nJ].maxSpreadTime));
          }
        }
//
//  This runtype is not in use
//
        else
        {
          SendMessage(hCtlNUMPCES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAXPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAXPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAXBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAXCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAXSPD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlCREWONLY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlTRAVELCOUNTS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlBRINGUPTO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlPAYSPDOT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlMINLAY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlMINLAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINLAYSPAN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMINLAYNOTBEFORE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlLARGESTNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
        EnableWindow(hCtlIDDELETE, TRUE);
        EnableWindow(hCtlIDATTRIBUTES, TRUE);
      }
      else
      {
        EnableSections(0L);
        EnableWindow(hCtlIDDELETE, FALSE);
        EnableWindow(hCtlIDATTRIBUTES, FALSE);
      }
      break;
//
//  WM_CLOSE - Same as IDCANCEL
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
//  ADDRUNTYPE_RUNTYPE
//
        case ADDRUNTYPE_RUNTYPE:
          switch(wmEvent)
          {
            case CBN_SELCHANGE:
              nI = (int)SendMessage(hCtlRUNTYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                break;
              }
              for(slotNumber = NO_RECORD, nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
              {
                if(!RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
                {
                  runtypeIndex = nI;
                  slotNumber = nJ;;
                  break;
                }
              }
              if(slotNumber == NO_RECORD)
              {
                break;
              }
              SendMessage(hCtlNUMPCES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlDESPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAXPLATFORM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlDESPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAXPAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAXSPD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlDESBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAXBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlDESCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAXCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlCREWONLY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlTRAVELCOUNTS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlBRINGUPTO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlPAYSPDOT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlMINLAY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlMINLAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINLAYSPAN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINLAYNOTBEFORE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMINLAYLABEL, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
              SendMessage(hCtlLARGESTNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlSMALLERNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              break;
          }
          break;
//
//  ADDRUNTYPE_NAME - If a name is in the edit control, enable the delete & attributes buttons.
//
        case ADDRUNTYPE_NAME:
          switch(wmEvent)
          {
            case EN_CHANGE:
              SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              nI = (int)SendMessage(hCtlRUNTYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              bEnable = (nI != CB_ERR && strcmp(tempString, "") != 0);
              EnableWindow(hCtlIDDELETE, bEnable);
              SendMessage(hCtlNUMPCES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              nI = atoi(tempString);
              EnableWindow(hCtlIDATTRIBUTES, bEnable && nI > 0);
              break;
          }
          break;
//
//  ADDRUNTYPE_NUMPCES - Monitor the control to see if he's defining more than
//        one piece.  If so, enable the spread controls.
//
        case ADDRUNTYPE_NUMPCES:
          switch(wmEvent)
          {
            case EN_CHANGE:
              SendMessage(hCtlNUMPCES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              nI = atoi(tempString);
              EnableSections(nI);
              SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              EnableWindow(hCtlIDATTRIBUTES, nI > 0 && strcmp(tempString, "") != 0);
              break;
          }
          break;
//
//
//  IDATTRIBUTES
//
        case IDATTRIBUTES:
          SendMessage(hCtlNUMPCES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[runtypeIndex][slotNumber].numPieces = atoi(tempString);
          SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)sizeof(RUNTYPE[runtypeIndex][slotNumber].localName),
                (LONG)(LPSTR)RUNTYPE[runtypeIndex][slotNumber].localName);
          DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RTATTRIB), hWndDlg,
                (DLGPROC)RTATTRIBMsgProc, (LPARAM)&RUNTYPE[runtypeIndex][slotNumber]);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Runtypes);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Must have selected a local name
//
          SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)sizeof(szLocalName), (LONG)(LPSTR)szLocalName);
          if(strcmp(szLocalName, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_092, hCtlNAME);
            break;
          }
//
//  Must have selected a runtype
//
          if(runtypeIndex == NO_RECORD)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_093, hCtlRUNTYPE);
            break;
          }
//
// For runs of two or more pieces, the maximum spread time is mandatory
//
          SendMessage(hCtlNUMPCES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          numPieces = atoi(tempString);
          SendMessage(hCtlMAXSPD, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          maxSpreadTime = thhmm(tempString);
          if(numPieces > 1 && maxSpreadTime == NO_TIME)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_203, hCtlMAXSPD);
            break;
          }
//
//  Ok, passed the tests.  Record the info from the box into RUNTYPE, and exit.
//
          nI = runtypeIndex;
          nJ = slotNumber;
          RUNTYPE[nI][nJ].flags = 0;
//
//  inUse
//
          RUNTYPE[nI][nJ].flags |= RTFLAGS_INUSE;
//
//  localName - We still have tempString from above
//
          strcpy(RUNTYPE[nI][nJ].localName, szLocalName);
//
//  basedUpon
//
          RUNTYPE[nI][nJ].flags |= SendMessage(hCtl4DAY, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ? RTFLAGS_4DAY : RTFLAGS_5DAY;
//
//  numPieces
//
          RUNTYPE[nI][nJ].numPieces = numPieces;
//
//  minPlatformTime
//
          SendMessage(hCtlMINPLATFORM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].minPlatformTime = thhmm(tempString);
//
//  desPlatformTime
//
          SendMessage(hCtlDESPLATFORM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].desPlatformTime = thhmm(tempString);
//
//  maxPlatformTime
//
          SendMessage(hCtlMAXPLATFORM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].maxPlatformTime = thhmm(tempString);
//
//  minPayTime
//
          SendMessage(hCtlMINPAY, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].minPayTime = thhmm(tempString);
//
//  desPayTime
//
          SendMessage(hCtlDESPAY, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].desPayTime = thhmm(tempString);
//
//  maxPayTime
//
          SendMessage(hCtlMAXPAY, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].maxPayTime = thhmm(tempString);
//
//  minBreakOf
//
          SendMessage(hCtlMINBREAKOF, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].minBreakOf = thhmm(tempString);
//
//  desBreakOf
//
          SendMessage(hCtlDESBREAKOF, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].desBreakOf = thhmm(tempString);
//
//  maxBreakOf
//
          SendMessage(hCtlMAXBREAKOF, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].maxBreakOf = thhmm(tempString);
//
//  minContig
//
          SendMessage(hCtlMINCONTIG, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].minContig = thhmm(tempString);
//
//  desContig
//
          SendMessage(hCtlDESCONTIG, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].desContig = thhmm(tempString);
//
//  maxContig
//
          SendMessage(hCtlMAXCONTIG, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RUNTYPE[nI][nJ].maxContig = thhmm(tempString);
//
//  maxSpreadTime (Will be NO_TIME if the control is disabled and blank)
//
          RUNTYPE[nI][nJ].maxSpreadTime = maxSpreadTime;
//
//  crewOnly
//
          if(SendMessage(hCtlCREWONLY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_CREWONLY;
          }
//
//  travelCounts
//
          if(SendMessage(hCtlTRAVELCOUNTS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_TRAVELCOUNTS;
          }
//
//  bringUpTo
//
          if(SendMessage(hCtlBRINGUPTO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_BRINGUPTO;
          }
//
//  paySpdOT
//
          if(SendMessage(hCtlPAYSPDOT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_PAYSPDOT;
          }
//
//  minLay and minLayTime
//
          if(!SendMessage(hCtlMINLAY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].minLayTime = NO_TIME;
            RUNTYPE[nI][nJ].minLaySpan = NO_TIME;
            RUNTYPE[nI][nJ].minLayNotBefore = NO_TIME;
            RUNTYPE[nI][nJ].minLayLabel[0] = '\0';
          }
          else
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_MINLAY;
            SendMessage(hCtlMINLAYTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].minLayTime = thhmm(tempString);
            SendMessage(hCtlMINLAYSPAN, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].minLaySpan = thhmm(tempString);
            SendMessage(hCtlMINLAYNOTBEFORE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].minLayNotBefore = thhmm(tempString);
            nK = (int)SendMessage(hCtlMINLAYLABEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nK == CB_ERR || nK == 0)
            {
              RUNTYPE[nI][nJ].minLayLabel[0] = '\0';
            }
            else
            {
              SendMessage(hCtlMINLAYLABEL, CB_GETLBTEXT, (WPARAM)nK, (LONG)(LPSTR)tempString);
              RUNTYPE[nI][nJ].minLayLabel[0] = tempString[0];
            }
          }
//
//  Largest paid break
//
//  Clear out the previous value
//
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_PAIDBREAK)
          {
            RUNTYPE[nI][nJ].flags &= ~RTFLAGS_PAIDBREAK;
          }
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_LARGESTPAIDTHROUGHWHEN)
          {
            RUNTYPE[nI][nJ].flags &= ~RTFLAGS_LARGESTPAIDTHROUGHWHEN;
          }
//
//  Set the new value
//
          if(SendMessage(hCtlLARGESTNOTPAID, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].paidFrom = NO_TIME;
            RUNTYPE[nI][nJ].paidTo = NO_TIME;
            RUNTYPE[nI][nJ].paidToWhen = NO_TIME;
          }
          else if(SendMessage(hCtlLARGESTPAID, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_PAIDBREAK;
            SendMessage(hCtlLARGESTPAIDFROM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidFrom = thhmm(tempString);
            SendMessage(hCtlLARGESTPAIDTO, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidTo = thhmm(tempString);
          }
          else
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_LARGESTPAIDTHROUGHWHEN;
            SendMessage(hCtlLARGESTPAIDTHROUGHAMOUNT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidToWhen = thhmm(tempString);
          }
//
//  Smaller paid break(s)
//
//  Clear out the previous value
//
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAID)
          {
            RUNTYPE[nI][nJ].flags &= ~RTFLAGS_SMALLERPAID;
          }
          if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAIDTHROUGHWHEN)
          {
            RUNTYPE[nI][nJ].flags &= ~RTFLAGS_SMALLERPAIDTHROUGHWHEN;
          }
//
//  Set the new value
//
          if(SendMessage(hCtlSMALLERNOTPAID, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].paidFromSmaller = NO_TIME;
            RUNTYPE[nI][nJ].paidToSmaller = NO_TIME;
            RUNTYPE[nI][nJ].paidToWhenSmaller = NO_TIME;
          }
          else if(SendMessage(hCtlSMALLERPAID, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_SMALLERPAID;
            SendMessage(hCtlSMALLERPAIDFROM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidFromSmaller = thhmm(tempString);
            SendMessage(hCtlSMALLERPAIDTO, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidToSmaller = thhmm(tempString);
          }
          else
          {
            RUNTYPE[nI][nJ].flags |= RTFLAGS_SMALLERPAIDTHROUGHWHEN;
            SendMessage(hCtlSMALLERPAIDTHROUGHAMOUNT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            RUNTYPE[nI][nJ].paidToWhenSmaller = thhmm(tempString);
          }
          m_bWorkrulesChanged = TRUE;
//
//  Ciao
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

void EnableSections(int numPieces)
{
  BOOL bEnable;

//
//  Stuff that gets enabled for three or more pieces
//
  if(hCtlSMALLER == NULL)
  {
    return;
  }
  bEnable = numPieces >= 3;
  if(bEnable)
  {
    if(!IsWindowEnabled(hCtlSMALLER))
    {
      SendMessage(hCtlSMALLERNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
    }
  }
  EnableWindow(hCtlBREAKOF, bEnable);
  EnableWindow(hCtlMINBREAKOF, bEnable);
  EnableWindow(hCtlDESBREAKOF, bEnable);
  EnableWindow(hCtlMAXBREAKOF, bEnable);
  EnableWindow(hCtlCONTIG, bEnable);
  EnableWindow(hCtlMINCONTIG, bEnable);
  EnableWindow(hCtlDESCONTIG, bEnable);
  EnableWindow(hCtlMAXCONTIG, bEnable);
  EnableWindow(hCtlSMALLER, bEnable);
  EnableWindow(hCtlSMALLERNOTPAID, bEnable);
  EnableWindow(hCtlSMALLERPAID, bEnable);
  EnableWindow(hCtlSMALLERPAIDFROM, bEnable);
  EnableWindow(hCtlSMALLERTO, bEnable);
  EnableWindow(hCtlSMALLERPAIDTO, bEnable);
  EnableWindow(hCtlSMALLERPAIDTHROUGHWHEN, bEnable);
  EnableWindow(hCtlSMALLERPAIDTHROUGHAMOUNT, bEnable);
  if(!bEnable)
  {
    SendMessage(hCtlMINBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlDESBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlMAXBREAKOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlMINCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlDESCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlMAXCONTIG, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlSMALLERNOTPAID, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlSMALLERPAID, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlSMALLERPAIDTHROUGHWHEN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlSMALLERPAIDFROM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlSMALLERPAIDTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlSMALLERPAIDTHROUGHAMOUNT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
  }
//
//  Stuff that gets enabled for two or more pieces
//
  bEnable = numPieces >= 2;
  if(bEnable)
  {
    if(!IsWindowEnabled(hCtlLARGEST))
      SendMessage(hCtlLARGESTNOTPAID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
  }
  EnableWindow(hCtlSPD, bEnable);
  EnableWindow(hCtlMAXSPD, bEnable);
  EnableWindow(hCtlTRAVELCOUNTS, bEnable);
  EnableWindow(hCtlLARGEST, bEnable);
  EnableWindow(hCtlLARGESTNOTPAID, bEnable);
  EnableWindow(hCtlLARGESTPAID, bEnable);
  EnableWindow(hCtlLARGESTPAIDFROM, bEnable);
  EnableWindow(hCtlLARGESTTO, bEnable);
  EnableWindow(hCtlLARGESTPAIDTO, bEnable);
  EnableWindow(hCtlLARGESTPAIDTHROUGHWHEN, bEnable);
  EnableWindow(hCtlLARGESTPAIDTHROUGHAMOUNT, bEnable);
  EnableWindow(hCtlPAYSPDOT, bEnable);
  if(!bEnable)
  {
    SendMessage(hCtlMAXSPD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlTRAVELCOUNTS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlLARGESTNOTPAID, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlLARGESTPAID, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlLARGESTPAIDTHROUGHWHEN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
    SendMessage(hCtlLARGESTPAIDFROM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlLARGESTPAIDTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlLARGESTPAIDTHROUGHAMOUNT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
    SendMessage(hCtlPAYSPDOT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  }

  return;
}
