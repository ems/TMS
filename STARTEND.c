//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static HANDLE hCtlNA;
static HANDLE hCtlTRAVELPAID;
static HANDLE hCtlSTARTOFRUN;
static HANDLE hCtlENDOFRUN;
static HANDLE hCtlLOCATIONS;
static HANDLE hCtlSAME;
static HANDLE hCtlDIFFER;
static HANDLE hCtlSTARTLOCATION;
static HANDLE hCtlSTARTANYWHERE;
static HANDLE hCtlSTARTPOG;
static HANDLE hCtlSTARTPIG;
static HANDLE hCtlSTARTCLOSEST;
static HANDLE hCtlSTARTNODE;
static HANDLE hCtlSTARTNODELIST;
static HANDLE hCtlENDLOCATION;
static HANDLE hCtlENDANYWHERE;
static HANDLE hCtlENDPOG;
static HANDLE hCtlENDPIG;
static HANDLE hCtlENDCLOSEST;
static HANDLE hCtlENDNODE;
static HANDLE hCtlENDNODELIST;
static HANDLE hCtlSPECIALCONSIDERATIONS;
static HANDLE hCtlSPECIALABCD;
static HANDLE hCtlSPECIALMANIPULATE;

void EnableStartEnd(BOOL);
void EnableEnd(BOOL);
void EnableSpecialConsiderations(BOOL);

BOOL CALLBACK STARTENDMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PREMIUMDef *pLP;
  static int sameDiffRG[2] = {STARTEND_SAME,
                              STARTEND_DIFFER};
  static int startRG[5]    = {STARTEND_STARTANYWHERE,
                              STARTEND_STARTPOG,
                              STARTEND_STARTPIG,
                              STARTEND_STARTCLOSEST,
                              STARTEND_STARTNODE};
  static int endRG[5]      = {STARTEND_ENDANYWHERE,
                              STARTEND_ENDPOG,
                              STARTEND_ENDPIG,
                              STARTEND_ENDCLOSEST,
                              STARTEND_ENDNODE};
  HANDLE hCtl;
  BOOL   bEnable;
  int    nI;
  int    nJ;
  short int wmId;

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
      hCtlNA = GetDlgItem(hWndDlg, STARTEND_NA);
      hCtlTRAVELPAID = GetDlgItem(hWndDlg, STARTEND_TRAVELPAID);
      hCtlSTARTOFRUN = GetDlgItem(hWndDlg, STARTEND_STARTOFRUN);
      hCtlENDOFRUN = GetDlgItem(hWndDlg, STARTEND_ENDOFRUN);
      hCtlLOCATIONS = GetDlgItem(hWndDlg, STARTEND_LOCATIONS);
      hCtlSAME = GetDlgItem(hWndDlg, STARTEND_SAME);
      hCtlDIFFER = GetDlgItem(hWndDlg, STARTEND_DIFFER);
      hCtlSTARTLOCATION = GetDlgItem(hWndDlg, STARTEND_STARTLOCATION);
      hCtlSTARTANYWHERE = GetDlgItem(hWndDlg, STARTEND_STARTANYWHERE);
      hCtlSTARTPOG = GetDlgItem(hWndDlg, STARTEND_STARTPOG);
      hCtlSTARTPIG = GetDlgItem(hWndDlg, STARTEND_STARTPIG);
      hCtlSTARTCLOSEST = GetDlgItem(hWndDlg, STARTEND_STARTCLOSEST);
      hCtlSTARTNODE = GetDlgItem(hWndDlg, STARTEND_STARTNODE);
      hCtlSTARTNODELIST = GetDlgItem(hWndDlg, STARTEND_STARTNODELIST);
      hCtlENDLOCATION = GetDlgItem(hWndDlg, STARTEND_ENDLOCATION);
      hCtlENDANYWHERE = GetDlgItem(hWndDlg, STARTEND_ENDANYWHERE);
      hCtlENDPOG = GetDlgItem(hWndDlg, STARTEND_ENDPOG);
      hCtlENDPIG = GetDlgItem(hWndDlg, STARTEND_ENDPIG);
      hCtlENDCLOSEST = GetDlgItem(hWndDlg, STARTEND_ENDCLOSEST);
      hCtlENDNODE = GetDlgItem(hWndDlg, STARTEND_ENDNODE);
      hCtlENDNODELIST = GetDlgItem(hWndDlg, STARTEND_ENDNODELIST);
      hCtlSPECIALCONSIDERATIONS = GetDlgItem(hWndDlg, STARTEND_SPECIALCONSIDERATIONS);
      hCtlSPECIALABCD = GetDlgItem(hWndDlg, STARTEND_SPECIALABCD);
      hCtlSPECIALMANIPULATE = GetDlgItem(hWndDlg, STARTEND_SPECIALMANIPULATE);
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
        LoadString(hInst, TEXT_056, szarString, sizeof(szarString));
        strcat(tempString, szarString);
      }
      else
        strcat(tempString, RUNTYPE[m_PremiumRuntype][m_PremiumTimePeriod].localName);
      SetWindowText(hWndDlg, tempString);
//
//  Set up the dialog
//
      if(pLP->startEnd == NO_RECORD)
      {
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableStartEnd(FALSE);
      }
      else
      {
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
        SendMessage(hCtlSTARTOFRUN, BM_SETCHECK, (WPARAM)(pLP->paidTravel & TRAVELPAID_START), (LPARAM)0);
        SendMessage(hCtlENDOFRUN, BM_SETCHECK, (WPARAM)(pLP->paidTravel & TRAVELPAID_END), (LPARAM)0);
        bEnable = (pLP->paidTravel & TRAVELPAID_START) || (pLP->paidTravel & TRAVELPAID_END);
        EnableSpecialConsiderations(bEnable);
        SendMessage(hCtlSPECIALABCD, BM_SETCHECK, (WPARAM)(pLP->paidTravel & TRAVELPAID_ABCDFLAG), (LPARAM)0);
        if(IsWindowEnabled(hCtlSPECIALABCD))
        {
          SendMessage(hCtlSPECIALMANIPULATE, BM_SETCHECK, (WPARAM)(pLP->paidTravel & TRAVELPAID_MANIPULATE), (LPARAM)0);
        }
        hCtl = GetDlgItem(hWndDlg, pLP->startEnd);
        SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        if(pLP->startEnd == STARTEND_SAME)
        {
          EnableEnd(FALSE);
        }
        hCtl = GetDlgItem(hWndDlg, pLP->startLocation);
        SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        if(pLP->startLocation == STARTEND_STARTNODE)
        {
          nI = SetUpNodeList(hWndDlg, STARTEND_STARTNODELIST, FALSE);
          for(nJ = 0; nJ < nI; nJ++)
          {
            if(SendMessage(hCtlSTARTNODELIST, CB_GETITEMDATA,
                  (WPARAM)nJ, (LPARAM)0) == pLP->startNODESrecordID)
            {
              SendMessage(hCtlSTARTNODELIST, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
              break;
            }
          }
        }
        hCtl = GetDlgItem(hWndDlg, pLP->endLocation);
        SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        if(pLP->endLocation == STARTEND_ENDNODE)
        {
          nI = SetUpNodeList(hWndDlg, STARTEND_ENDNODELIST, FALSE);
          for(nJ = 0; nJ < nI; nJ++)
          {
            if(SendMessage(hCtlENDNODELIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == pLP->endNODESrecordID)
            {
              SendMessage(hCtlENDNODELIST, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
              break;
            }
          }
        }
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
      switch(wmId)
      {
        case STARTEND_SPECIALABCD:
          if(SendMessage(hCtlSPECIALABCD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableWindow(hCtlSPECIALMANIPULATE, TRUE);
            SendMessage(hCtlSPECIALMANIPULATE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          }
          else
          {
            EnableWindow(hCtlSPECIALMANIPULATE, FALSE);
            SendMessage(hCtlSPECIALMANIPULATE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          }
          break;
//
//  STARTEND_NA check box
//
        case STARTEND_NA:
          SendMessage(hCtlSTARTNODELIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlENDNODELIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          EnableSpecialConsiderations(FALSE);
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            EnableStartEnd(FALSE);
          else
          {
            EnableStartEnd(TRUE);
            nCwCheckRadioButtons(hWndDlg, sameDiffRG, STARTEND_SAME, 2);
            EnableEnd(FALSE);
            nCwCheckRadioButtons(hWndDlg, startRG, STARTEND_STARTANYWHERE, 5);
            nCwCheckRadioButtons(hWndDlg, endRG, STARTEND_ENDANYWHERE, 5);
            SendMessage(hCtlSTARTOFRUN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
            SendMessage(hCtlENDOFRUN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
            EnableWindow(hCtlSTARTNODELIST, FALSE);
          }
          break;
//
//  STARTEND_SAME radio button
//  STARTEND_DIFFER radio button
//
        case STARTEND_SAME:
        case STARTEND_DIFFER:
          EnableEnd(SendMessage(hCtlDIFFER, BM_GETCHECK, (WPARAM)0, (LPARAM)0));
          break;
//
//  STARTEND_STARTPOG radio button
//  STARTEND_STARTPIG radio button
//  STARTEND_STARTCLOSEST radio button
//  STARTEND_STARTNODE radio button
//
        case STARTEND_STARTPOG:
        case STARTEND_STARTPIG:
        case STARTEND_STARTCLOSEST:
          EnableWindow(hCtlSTARTNODELIST, FALSE);
          SendMessage(hCtlSTARTNODELIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          break;

        case STARTEND_STARTNODE:
          EnableWindow(hCtlSTARTNODELIST, TRUE);
          SetUpNodeList(hWndDlg, STARTEND_STARTNODELIST, FALSE);
          break;
//
//  STARTEND_ENDPOG radio button
//  STARTEND_ENDPIG radio button
//  STARTEND_ENDCLOSEST radio button
//  STARTEND_ENDNODE radio button
//
        case STARTEND_ENDPOG:
        case STARTEND_ENDPIG:
        case STARTEND_ENDCLOSEST:
          EnableWindow(hCtlENDNODELIST, FALSE);
          SendMessage(hCtlENDNODELIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          break;

        case STARTEND_ENDNODE:
          EnableWindow(hCtlENDNODELIST, TRUE);
          SetUpNodeList(hWndDlg, STARTEND_ENDNODELIST, FALSE);
          break;
//
//  STARTEND_STARTOFRUN
//  STARTEND_ENDOFRUN
//
        case STARTEND_STARTOFRUN:
        case STARTEND_ENDOFRUN:
          bEnable = SendMessage(hCtlSTARTOFRUN, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                SendMessage(hCtlENDOFRUN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableSpecialConsiderations(bEnable);
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
//  Was NA Checked?
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pLP->paidTravel = NO_RECORD;
            pLP->startEnd = NO_RECORD;
            pLP->startLocation = NO_RECORD;
            pLP->startNODESrecordID = NO_RECORD;
            pLP->endLocation = NO_RECORD;
            pLP->endNODESrecordID = NO_RECORD;
          }
//
//  Nope - get the values
//
          else
          {
//
//  SAME / DIFFER
//
            for(nI = 0; nI < 2; nI++)
            {
              hCtl = GetDlgItem(hWndDlg, sameDiffRG[nI]);
              if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                pLP->startEnd = sameDiffRG[nI];
                break;
              }
            }
//
//  Start node, if checked, has to have a selection
//
            if(SendMessage(hCtlSTARTNODE, BM_GETCHECK, (WPARAM)0, (LPARAM)0) &&
                  SendMessage(hCtlSTARTNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlSTARTNODELIST);
              break;
            }
//
//  Likewise with the end node, but don't bother
//  checking unless startEnd is DIFFER
//
            if(pLP->startEnd == STARTEND_DIFFER)
            {
              if(SendMessage(hCtlENDNODE, BM_GETCHECK, (WPARAM)0, (LPARAM)0) &&
                    SendMessage(hCtlENDNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlENDNODELIST);
                break;
              }
            }
//
//  Travel Paid?
//
            pLP->paidTravel = TRAVELPAID_NONE;
            if(SendMessage(hCtlSTARTOFRUN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->paidTravel |= TRAVELPAID_START;
            }
            if(SendMessage(hCtlENDOFRUN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->paidTravel |= TRAVELPAID_END;
            }
            if(SendMessage(hCtlSPECIALABCD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pLP->paidTravel |= TRAVELPAID_ABCDFLAG;
              if(SendMessage(hCtlSPECIALMANIPULATE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                pLP->paidTravel |= TRAVELPAID_MANIPULATE;
              }
            }
//
//  Start buttons
//
            pLP->startNODESrecordID = NO_RECORD;
            for(nI = 0; nI < 5; nI++)
            {
              hCtl = GetDlgItem(hWndDlg, startRG[nI]);
              if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                pLP->startLocation = startRG[nI];
                if(startRG[nI] == STARTEND_STARTNODE)
                {
                  nJ = (int)SendMessage(hCtlSTARTNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                  if(nJ != CB_ERR)
                    pLP->startNODESrecordID =
                        SendMessage(hCtlSTARTNODELIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                }
                if(pLP->startEnd == STARTEND_SAME)  // Set up "end" if STARTEND_SAME is in effect
                {
                  pLP->endLocation = endRG[nI];
                  if(startRG[nI] == STARTEND_STARTNODE)
                    pLP->endNODESrecordID = pLP->startNODESrecordID;
                }
                break;
              }
            }
//
//  End buttons - don't bother
//  checking unless startEnd is DIFFER
//
            if(pLP->startEnd == STARTEND_DIFFER)
            {
              pLP->endNODESrecordID = NO_RECORD;
              for(nI = 0; nI < 5; nI++)
              {
                hCtl = GetDlgItem(hWndDlg, endRG[nI]);
                if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                {
                  pLP->endLocation = endRG[nI];
                  if(endRG[nI] == STARTEND_ENDNODE)
                  {
                    nJ = (int)SendMessage(hCtlENDNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    if(nJ != CB_ERR)
                      pLP->endNODESrecordID =
                            SendMessage(hCtlENDNODELIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                  }
                  break;
                }
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

void EnableStartEnd(BOOL bEnable)
{
  EnableWindow(hCtlTRAVELPAID, bEnable);
  EnableWindow(hCtlSTARTOFRUN, bEnable);
  EnableWindow(hCtlENDOFRUN, bEnable);
  EnableWindow(hCtlLOCATIONS, bEnable);
  EnableWindow(hCtlSAME, bEnable);
  EnableWindow(hCtlDIFFER, bEnable);
  EnableWindow(hCtlSTARTLOCATION, bEnable);
  EnableWindow(hCtlSTARTANYWHERE, bEnable);
  EnableWindow(hCtlSTARTPOG, bEnable);
  EnableWindow(hCtlSTARTPIG, bEnable);
  EnableWindow(hCtlSTARTCLOSEST, bEnable);
  EnableWindow(hCtlSTARTNODE, bEnable);
  EnableWindow(hCtlSTARTNODELIST, bEnable);
  EnableEnd(bEnable);
}

void EnableEnd(BOOL bEnable)
{
  EnableWindow(hCtlENDLOCATION, bEnable);
  EnableWindow(hCtlENDANYWHERE, bEnable);
  EnableWindow(hCtlENDPOG, bEnable);
  EnableWindow(hCtlENDPIG, bEnable);
  EnableWindow(hCtlENDCLOSEST, bEnable);
  EnableWindow(hCtlENDNODE, bEnable);
  EnableWindow(hCtlENDNODELIST, bEnable);
}

void EnableSpecialConsiderations(BOOL bEnable)
{
  EnableWindow(hCtlSPECIALCONSIDERATIONS, bEnable);
  EnableWindow(hCtlSPECIALABCD, bEnable);
  EnableWindow(hCtlSPECIALMANIPULATE, bEnable);
  if(!bEnable)
  {
    SendMessage(hCtlSPECIALMANIPULATE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  }
}
