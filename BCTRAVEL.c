//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define RGTRAVEL_NUM 7

BOOL CALLBACK BCTRAVELMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static int rgTravel[RGTRAVEL_NUM] =
                   {BCTRAVEL_BTOCDIRECT,   BCTRAVEL_BTOBPOGTOC,  BCTRAVEL_BTOBPIGTOC,
                    BCTRAVEL_BTOCPOGTOC,   BCTRAVEL_BTOCPIGTOC,  BCTRAVEL_BTOCLOSESTTOC,
                    BCTRAVEL_BTONODE};
  static  PREMIUMDef *pLP;
  static  HANDLE hCtlNA;
  static  HANDLE hCtlBTOCDIRECT;
  static  HANDLE hCtlBTOBPOGTOC;
  static  HANDLE hCtlBTOBPIGTOC;
  static  HANDLE hCtlBTOCPOGTOC;
  static  HANDLE hCtlBTOCPIGTOC;
  static  HANDLE hCtlBTOCLOSESTTOC;
  static  HANDLE hCtlBTONODE;
  static  HANDLE hCtlNODELIST;
  static  HANDLE hCtlNODETOC;
  static  HANDLE hCtlINCLUDED;
  static  HANDLE hCtlPAYBTOCDIRECT;
  static  HANDLE hCtlFROMBIN;
  static  HANDLE hCtlOUTTOC;
  static  HANDLE hCtlABCD;
  static  HANDLE hCtlMANIPULATE;
  static  HANDLE hCtlRECORDONAB;
  static  HANDLE hCtlDONTTRAVELBEQC;
  static  HANDLE hCtlTRAVELBTOCWHENINTINFEASIBLE;
  static  int numNodes;
  HANDLE  hCtl;
  BOOL    bEnable;
  int     nI;
  int     nJ;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pLP = (PREMIUMDef *)lParam;
      if(pLP == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlNA = GetDlgItem(hWndDlg, BCTRAVEL_NA);
      hCtlBTOCDIRECT = GetDlgItem(hWndDlg, BCTRAVEL_BTOCDIRECT);
      hCtlBTOBPOGTOC = GetDlgItem(hWndDlg, BCTRAVEL_BTOBPOGTOC);
      hCtlBTOBPIGTOC = GetDlgItem(hWndDlg, BCTRAVEL_BTOBPIGTOC);
      hCtlBTOCPOGTOC = GetDlgItem(hWndDlg, BCTRAVEL_BTOCPOGTOC);
      hCtlBTOCPIGTOC = GetDlgItem(hWndDlg, BCTRAVEL_BTOCPIGTOC);
      hCtlBTOCLOSESTTOC = GetDlgItem(hWndDlg, BCTRAVEL_BTOCLOSESTTOC);
      hCtlBTONODE = GetDlgItem(hWndDlg, BCTRAVEL_BTONODE);
      hCtlNODELIST = GetDlgItem(hWndDlg, BCTRAVEL_NODELIST);
      hCtlNODETOC = GetDlgItem(hWndDlg, BCTRAVEL_NODETOC);
      hCtlINCLUDED = GetDlgItem(hWndDlg, BCTRAVEL_INCLUDED);
      hCtlPAYBTOCDIRECT = GetDlgItem(hWndDlg, BCTRAVEL_PAYBTOCDIRECT);
      hCtlFROMBIN = GetDlgItem(hWndDlg, BCTRAVEL_FROMBIN);
      hCtlOUTTOC = GetDlgItem(hWndDlg, BCTRAVEL_OUTTOC);
      hCtlABCD = GetDlgItem(hWndDlg, BCTRAVEL_ABCD);
      hCtlMANIPULATE = GetDlgItem(hWndDlg, BCTRAVEL_MANIPULATE);
      hCtlRECORDONAB = GetDlgItem(hWndDlg, BCTRAVEL_RECORDONAB);
      hCtlDONTTRAVELBEQC = GetDlgItem(hWndDlg, BCTRAVEL_DONTTRAVELBEQC);
      hCtlTRAVELBTOCWHENINTINFEASIBLE = GetDlgItem(hWndDlg, BCTRAVEL_TRAVELBTOCWHENINTINFEASIBLE);
//
//  Set the title text
//
      GetWindowText(hWndDlg, tempString, TEMPSTRING_LENGTH);
      strcat(tempString, " - ");
      if(strcmp(pLP->localName, "") != 0)
      {
        strcat(tempString, pLP->localName);
        strcat(tempString, " - ");
      }
      if(m_PremiumRuntype == NO_RECORD)
      {
        LoadString(hInst, TEXT_056, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
      }
      else
      {
        strcat(tempString, RUNTYPE[m_PremiumRuntype][m_PremiumTimePeriod].localName);
      }
      SetWindowText(hWndDlg, tempString);
//
//  Set up the dialog
//
      numNodes = SetUpNodeList(hWndDlg, BCTRAVEL_NODELIST, FALSE);
      if(pLP->pieceTravel == NO_RECORD)
      {
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
      else
      {
        hCtl = GetDlgItem(hWndDlg, pLP->pieceTravel);
        SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        if(pLP->pieceTravel == BCTRAVEL_BTOCDIRECT)
        {
          EnableWindow(hCtlFROMBIN, FALSE);
          EnableWindow(hCtlOUTTOC, FALSE);
        }
        if(pLP->pieceTravel == BCTRAVEL_BTONODE)
        {
          for(nJ = 0; nJ < numNodes; nJ++)
          {
            if(SendMessage(hCtlNODELIST, CB_GETITEMDATA,
                  (WPARAM)nJ, (LPARAM)0) == pLP->payBtoNODESrecordID)
            {
              SendMessage(hCtlNODELIST, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
              break;
            }
          }
        }
        SendMessage(hCtlINCLUDED, BM_SETCHECK, (WPARAM)pLP->travelInBreak, (LPARAM)0);
        SendMessage(hCtlPAYBTOCDIRECT, BM_SETCHECK, (WPARAM)(pLP->payWhen & PAYWHEN_DIRECT), (LPARAM)0);
        SendMessage(hCtlFROMBIN, BM_SETCHECK, (WPARAM)(pLP->payWhen & PAYWHEN_FROMBIN), (LPARAM)0);
        SendMessage(hCtlOUTTOC, BM_SETCHECK, (WPARAM)(pLP->payWhen & PAYWHEN_OUTTOC), (LPARAM)0);
        SendMessage(hCtlABCD, BM_SETCHECK, (WPARAM)(pLP->payWhen & PAYWHEN_ABCDFLAG), (LPARAM)0);
        if(IsWindowEnabled(hCtlABCD))
        {
          EnableWindow(hCtlMANIPULATE, TRUE);
          SendMessage(hCtlMANIPULATE, BM_SETCHECK, (WPARAM)(pLP->payWhen & PAYWHEN_MANIPULATE), (LPARAM)0);
        }
        SendMessage(hCtlRECORDONAB, BM_SETCHECK, (WPARAM)(pLP->flags & PREMIUMFLAGS_RECORDONAB), (LPARAM)0);
        SendMessage(hCtlDONTTRAVELBEQC, BM_SETCHECK, (WPARAM)(pLP->flags & PREMIUMFLAGS_DONTTRAVELBEQC), (LPARAM)0);
        SendMessage(hCtlTRAVELBTOCWHENINTINFEASIBLE, BM_SETCHECK, (WPARAM)(pLP->flags & PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE), (LPARAM)0);
      }
      break;
//
//  WM_CLOSE - Acts just like cancel
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
//  ABCD
//
        case BCTRAVEL_ABCD:
          if(SendMessage(hCtlABCD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableWindow(hCtlMANIPULATE, TRUE);
            SendMessage(hCtlMANIPULATE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          }
          else
          {
            EnableWindow(hCtlMANIPULATE, FALSE);
            SendMessage(hCtlMANIPULATE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          }
          break;
//
//  Comments
//
        case BCTRAVEL_NODELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              else
              {
                for(nI = 0; nI < RGTRAVEL_NUM; nI++)
                {
                  if(SendDlgItemMessage(hWndDlg, rgTravel[nI], BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                  {
                    SendDlgItemMessage(hWndDlg, rgTravel[nI], BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                    break;
                  }
                }
              }
              SendMessage(hCtlBTONODE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              break;  
          }
          break;
//
//  "Travel From" radio buttons
//
        case BCTRAVEL_NA:
        case BCTRAVEL_BTOCDIRECT:
        case BCTRAVEL_BTOBPOGTOC:
        case BCTRAVEL_BTOBPIGTOC:
        case BCTRAVEL_BTOCPOGTOC:
        case BCTRAVEL_BTOCPIGTOC:
        case BCTRAVEL_BTOCLOSESTTOC:
        case BCTRAVEL_BTONODE:
          if(wmId == BCTRAVEL_BTOCDIRECT)
          {
            SendMessage(hCtlFROMBIN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
            SendMessage(hCtlOUTTOC, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
            EnableWindow(hCtlFROMBIN, FALSE);
            EnableWindow(hCtlOUTTOC, FALSE);
          }
          else
          {
            if(SendMessage(hCtlPAYBTOCDIRECT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              EnableWindow(hCtlFROMBIN, FALSE);
              EnableWindow(hCtlOUTTOC, FALSE);
            }
            else
            {
              EnableWindow(hCtlFROMBIN, TRUE);
              EnableWindow(hCtlOUTTOC, TRUE);
            }
          }
          break;
//
//  Pay B->C Direct checkbox
//
        case BCTRAVEL_PAYBTOCDIRECT:
          SendMessage(hCtlFROMBIN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlOUTTOC, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          bEnable = !SendMessage(hCtlPAYBTOCDIRECT, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlFROMBIN, bEnable);
          EnableWindow(hCtlOUTTOC, bEnable);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Premiums);
          break;
//
//  IDOK
//
        case IDOK:
//
//  If N/A was checked, clear everything out
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pLP->pieceTravel = NO_RECORD;
            pLP->payBtoNODESrecordID = NO_RECORD;
            pLP->travelInBreak = NO_RECORD;
            pLP->payWhen = NO_RECORD;
          }
          else
          {
//
//  Not N/A determine what it was, and set up the rest
//
            for(nI = 0; nI < RGTRAVEL_NUM; nI++)
            {
              if(SendDlgItemMessage(hWndDlg, rgTravel[nI], BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                if(rgTravel[nI] != BCTRAVEL_BTONODE)
                  pLP->payBtoNODESrecordID = NO_RECORD;
                else
                {
                  nJ = SendMessage(hCtlNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                  if(nJ == CB_ERR)
                  {
                    TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlNODELIST);
                    break;
                  }
                  pLP->payBtoNODESrecordID = SendMessage(hCtlNODELIST,
                        CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                }
                pLP->pieceTravel = rgTravel[nI];
                break;
              }
            }
//
//  travelInBreak
//
            pLP->travelInBreak = SendMessage(hCtlINCLUDED, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  payWhen
//
            pLP->payWhen = PAYWHEN_NONE;
            if(SendMessage(hCtlPAYBTOCDIRECT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->payWhen |= PAYWHEN_DIRECT;
            }
            if(SendMessage(hCtlFROMBIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->payWhen |= PAYWHEN_FROMBIN;
            }
            if(SendMessage(hCtlOUTTOC, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->payWhen |= PAYWHEN_OUTTOC;
            }
            if(SendMessage(hCtlABCD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->payWhen |= PAYWHEN_ABCDFLAG;
            }
            if(SendMessage(hCtlMANIPULATE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->payWhen |= PAYWHEN_MANIPULATE;
            }
//
//  Premium flags
//
            if(SendMessage(hCtlRECORDONAB, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->flags |= PREMIUMFLAGS_RECORDONAB;
            }
            else
            {
              if(pLP->flags & PREMIUMFLAGS_RECORDONAB)
              {
                pLP->flags &= ~PREMIUMFLAGS_RECORDONAB;
              }
            }
            if(SendMessage(hCtlDONTTRAVELBEQC, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->flags |= PREMIUMFLAGS_DONTTRAVELBEQC;
            }
            else
            {
              if(pLP->flags & PREMIUMFLAGS_DONTTRAVELBEQC)
              {
                pLP->flags &= ~PREMIUMFLAGS_DONTTRAVELBEQC;
              }
            }
            if(SendMessage(hCtlTRAVELBTOCWHENINTINFEASIBLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->flags |= PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE;
            }
            else
            {
              if(pLP->flags & PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE)
              {
                pLP->flags &= ~PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE;
              }
            }
          }
//
//  All done
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
