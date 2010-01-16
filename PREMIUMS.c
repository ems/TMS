//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void   EnableVerbNoun(BOOL);

#define NUMVERBS 5
#define NUMNOUNS 4

static HANDLE hCtlPULLOUT;
static HANDLE hCtlPULLIN;
static HANDLE hCtlSTARTRELIEF;
static HANDLE hCtlENDRELIEF;
static HANDLE hCtlFINISHAT;
static HANDLE hCtlANYGARAGE;
static HANDLE hCtlANYNONGARAGE;
static HANDLE hCtlANYTIMEPOINT;
static HANDLE hCtlTIMEPOINT;
static HANDLE hCtlTIMEPOINTLIST;
static HANDLE hCtlACTION;
static HANDLE hCtlATFROMTO;

BOOL CALLBACK PREMIUMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static PREMIUMDef   localPremium;
  static HANDLE hCtlNAME;
  static HANDLE hCtlNAROUTE;
  static HANDLE hCtlROUTE;
  static HANDLE hCtlROUTELIST;
  static HANDLE hCtlNASERVICE;
  static HANDLE hCtlSERVICE;
  static HANDLE hCtlSERVICELIST;
  static HANDLE hCtlNABEFORE;
  static HANDLE hCtlBEFORE;
  static HANDLE hCtlBEFORETIME;
  static HANDLE hCtlNAPIECE;
  static HANDLE hCtlPIECENUMBER;
  static HANDLE hCtlPIECENUMBERTITLE;
  static HANDLE hCtlNAPIECESIZE;
  static HANDLE hCtlPIECESIZE;
  static HANDLE hCtlPIECESIZETITLE;
  static HANDLE hCtlNAASSIGNEDTO;
  static HANDLE hCtlASSIGNEDTO;
  static HANDLE hCtlGARAGELIST;
  static HANDLE hCtlNAON;
  static HANDLE hCtlNAWITH;
  static HANDLE hCtlANYACCESSIBLE;
  static HANDLE hCtlBUSTYPE;
  static HANDLE hCtlBUSTYPELIST;
  static HANDLE hCtlNASELECT;
  static HANDLE hCtlPAIDSTARTEND;
  static HANDLE hCtlPAIDTRAVEL;
  static HANDLE hCtlIDEXECUTE;
  static HANDLE hCtlIDDELETE;
  static BOOL bGaragesOnly;
  static int  numRoutes;
  static int  numServices;
  static int  numGarages;
  static int  numBustypes;
  static int  numNodes;
  static int selectRG[2]         = {PREMIUMS_PAIDTRAVEL,    PREMIUMS_PAIDSTARTEND};
  static int onVerbRG[NUMVERBS]  = {PREMIUMS_PULLOUT,       PREMIUMS_PULLIN,
                                    PREMIUMS_STARTRELIEF,   PREMIUMS_ENDRELIEF,    PREMIUMS_FINISHAT};
  static int onNounRG[NUMNOUNS]  = {PREMIUMS_ANYGARAGE,     PREMIUMS_ANYNONGARAGE,
                                    PREMIUMS_ANYTIMEPOINT,  PREMIUMS_TIMEPOINT};
  int    nI;
  int    nJ;
  int    rcode2;
  BOOL   enable;
  HANDLE hCtl;
  short int wmId;
  short int wmEvent;

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
      hCtlNAME = GetDlgItem(hWndDlg, PREMIUMS_NAME);
      hCtlNAROUTE = GetDlgItem(hWndDlg, PREMIUMS_NAROUTE);
      hCtlROUTE = GetDlgItem(hWndDlg, PREMIUMS_ROUTE);
      hCtlROUTELIST = GetDlgItem(hWndDlg, PREMIUMS_ROUTELIST);
      hCtlNASERVICE = GetDlgItem(hWndDlg, PREMIUMS_NASERVICE);
      hCtlSERVICE = GetDlgItem(hWndDlg, PREMIUMS_SERVICE);
      hCtlSERVICELIST = GetDlgItem(hWndDlg, PREMIUMS_SERVICELIST);
      hCtlNABEFORE = GetDlgItem(hWndDlg, PREMIUMS_NABEFORE);
      hCtlBEFORE = GetDlgItem(hWndDlg, PREMIUMS_BEFORE);
      hCtlBEFORETIME = GetDlgItem(hWndDlg, PREMIUMS_BEFORETIME);
      hCtlNAPIECE = GetDlgItem(hWndDlg, PREMIUMS_NAPIECE);
      hCtlPIECENUMBER = GetDlgItem(hWndDlg, PREMIUMS_PIECENUMBER);
      hCtlPIECENUMBERTITLE = GetDlgItem(hWndDlg, PREMIUMS_PIECENUMBERTITLE);
      hCtlNAPIECESIZE = GetDlgItem(hWndDlg, PREMIUMS_NAPIECESIZE);
      hCtlPIECESIZE = GetDlgItem(hWndDlg, PREMIUMS_PIECESIZE);
      hCtlPIECESIZETITLE = GetDlgItem(hWndDlg, PREMIUMS_PIECESIZETITLE);
      hCtlNAASSIGNEDTO = GetDlgItem(hWndDlg, PREMIUMS_NAASSIGNEDTO);
      hCtlASSIGNEDTO = GetDlgItem(hWndDlg, PREMIUMS_ASSIGNEDTO);
      hCtlGARAGELIST = GetDlgItem(hWndDlg, PREMIUMS_GARAGELIST);
      hCtlNAON = GetDlgItem(hWndDlg, PREMIUMS_NAON);
      hCtlNAWITH = GetDlgItem(hWndDlg, PREMIUMS_NAWITH);
      hCtlANYACCESSIBLE = GetDlgItem(hWndDlg, PREMIUMS_ANYACCESSIBLE);
      hCtlBUSTYPE = GetDlgItem(hWndDlg, PREMIUMS_BUSTYPE);
      hCtlBUSTYPELIST = GetDlgItem(hWndDlg, PREMIUMS_BUSTYPELIST);
      hCtlNASELECT = GetDlgItem(hWndDlg, PREMIUMS_NASELECT);
      hCtlPAIDSTARTEND = GetDlgItem(hWndDlg, PREMIUMS_PAIDSTARTEND);
      hCtlPAIDTRAVEL = GetDlgItem(hWndDlg, PREMIUMS_PAIDTRAVEL);
      hCtlIDEXECUTE = GetDlgItem(hWndDlg, IDEXECUTE);
      hCtlIDDELETE = GetDlgItem(hWndDlg, IDDELETE);
      hCtlPULLOUT = GetDlgItem(hWndDlg, PREMIUMS_PULLOUT);
      hCtlPULLIN = GetDlgItem(hWndDlg, PREMIUMS_PULLIN);
      hCtlSTARTRELIEF = GetDlgItem(hWndDlg, PREMIUMS_STARTRELIEF);
      hCtlENDRELIEF = GetDlgItem(hWndDlg, PREMIUMS_ENDRELIEF);
      hCtlFINISHAT = GetDlgItem(hWndDlg, PREMIUMS_FINISHAT);
      hCtlANYGARAGE = GetDlgItem(hWndDlg, PREMIUMS_ANYGARAGE);
      hCtlANYNONGARAGE = GetDlgItem(hWndDlg, PREMIUMS_ANYNONGARAGE);
      hCtlANYTIMEPOINT = GetDlgItem(hWndDlg, PREMIUMS_ANYTIMEPOINT);
      hCtlTIMEPOINT = GetDlgItem(hWndDlg, PREMIUMS_TIMEPOINT);
      hCtlTIMEPOINTLIST = GetDlgItem(hWndDlg, PREMIUMS_TIMEPOINTLIST);
      hCtlACTION = GetDlgItem(hWndDlg, PREMIUMS_ACTION);
      hCtlATFROMTO = GetDlgItem(hWndDlg, PREMIUMS_ATFROMTO);
//
//  Set the title text
//
      GetWindowText(hWndDlg, tempString, TEMPSTRING_LENGTH);
      strcat(tempString, " - ");
      if(m_PremiumRuntype == NO_RECORD)
      {
        LoadString(hInst, TEXT_056, szarString, sizeof(szarString));
        strcat(tempString, szarString);
      }
      else
        strcat(tempString, RUNTYPE[m_PremiumRuntype][m_PremiumTimePeriod].localName);
      SetWindowText(hWndDlg, tempString);
//
//  Set up the dropdowns
//
//  Routes, services
//
      numRoutes = SetUpRouteList(hWndDlg, PREMIUMS_ROUTELIST, NO_RECORD);
      numServices = SetUpServiceList(hWndDlg, PREMIUMS_SERVICELIST, NO_RECORD);
//
//  Bustypes
//
      numBustypes = SetUpBustypeList(hWndDlg, PREMIUMS_BUSTYPELIST);
      if(numBustypes == 0)
      {
        EnableWindow(hCtlANYACCESSIBLE, FALSE);
        EnableWindow(hCtlBUSTYPE, FALSE);
        EnableWindow(hCtlBUSTYPELIST, FALSE);
      }
//
//  Garages (assigned to)
//
      numGarages = 0;
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        nI = SendMessage(hCtlGARAGELIST, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlGARAGELIST, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)NODES.recordID);
        numGarages++;
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
      if(numGarages == 0)
      {
        EnableWindow(hCtlASSIGNEDTO, FALSE);
        EnableWindow(hCtlGARAGELIST, FALSE);
      }
//
//  Is this a new premium?
//
      if(m_bPremiumNew)
//
//  Yes
//
      {
        if(m_numPremiums + 1 >= MAXPREMIUMS)
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_087, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
//
//  Default all choices to N/A
//
        m_premiumIndex = m_numPremiums++;
        SendMessage(hCtlNAROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNASERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNABEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNAPIECE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNAPIECESIZE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNAON, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNAWITH, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNASELECT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlNAASSIGNEDTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableVerbNoun(FALSE);
        EnableWindow(hCtlIDEXECUTE, FALSE);
        EnableWindow(hCtlIDDELETE, FALSE);
        PREMIUM[m_premiumIndex].pieceTravel = NO_RECORD;
        PREMIUM[m_premiumIndex].startEnd = NO_RECORD;
      }
//
//  No - it's an existing one
//
      else
      {
//
//  Name
//
        SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0,
              (LONG)(LPSTR)PREMIUM[m_premiumIndex].localName);
//
//  Route
//
        if(PREMIUM[m_premiumIndex].ROUTESrecordID == NO_RECORD)
          SendMessage(hCtlNAROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          for(nI = 0; nI < numRoutes; nI++)
          {
            if(SendMessage(hCtlROUTELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) ==
                  PREMIUM[m_premiumIndex].ROUTESrecordID)
            {
              SendMessage(hCtlROUTELIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              break;
            }
          }
        }
//
//  Service
//
        if(PREMIUM[m_premiumIndex].SERVICESrecordID == NO_RECORD)
          SendMessage(hCtlNASERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlSERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          for(nI = 0; nI < numServices; nI++)
          {
            if(SendMessage(hCtlSERVICELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) ==
                  PREMIUM[m_premiumIndex].SERVICESrecordID)
            {
              SendMessage(hCtlSERVICELIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              break;
            }
          }
        }
//
//  Before time
//
        if(PREMIUM[m_premiumIndex].beforeTime == NO_TIME)
          SendMessage(hCtlNABEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlBEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          strcpy(tempString, Tchar(PREMIUM[m_premiumIndex].beforeTime));
          SendMessage(hCtlBEFORETIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  With (bustype)
//
        if(PREMIUM[m_premiumIndex].bustype == NO_RECORD || numBustypes == 0)
          SendMessage(hCtlNAWITH, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlNAWITH, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          hCtl = GetDlgItem(hWndDlg, PREMIUM[m_premiumIndex].bustype);
          SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          if(PREMIUM[m_premiumIndex].bustype == PREMIUMS_BUSTYPE)
          {
            for(nI = 0; nI < numBustypes; nI++)
            {
              if(SendMessage(hCtlBUSTYPELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) ==
                    PREMIUM[m_premiumIndex].BUSTYPESrecordID)
              {
                SendMessage(hCtlBUSTYPELIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
                break;
              }
            }
          }
        }
//
//  Assigned to
//
        if(PREMIUM[m_premiumIndex].assignedToNODESrecordID == NO_RECORD ||
              !IsWindowEnabled(hCtlGARAGELIST))  // Just in case 
          SendMessage(hCtlNAASSIGNEDTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlASSIGNEDTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          for(nI = 0; nI < numGarages; nI++)
          {
            if(SendMessage(hCtlGARAGELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) ==
                  PREMIUM[m_premiumIndex].assignedToNODESrecordID)
            {
              SendMessage(hCtlGARAGELIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              break;
            }
          }
        }
//
//  forPiece and forPieceNumber
//
        if(PREMIUM[m_premiumIndex].forPieceNumber == NO_RECORD)
        {
          SendMessage(hCtlNAPIECE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlPIECENUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
        }
        else
        {
          SendMessage(hCtlPIECENUMBERTITLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          itoa(PREMIUM[m_premiumIndex].forPieceNumber, tempString, 10);
          SendMessage(hCtlPIECENUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  withPieceSize
//
        if(PREMIUM[m_premiumIndex].withPieceSize == NO_RECORD ||
              PREMIUM[m_premiumIndex].withPieceSize == 0)
          SendMessage(hCtlNAPIECESIZE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          SendMessage(hCtlPIECESIZETITLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          strcpy(tempString, chhmm(PREMIUM[m_premiumIndex].withPieceSize));
          SendMessage(hCtlPIECESIZE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        }
//
//  onVerb and onNoun
//
        if(PREMIUM[m_premiumIndex].onVerb == NO_RECORD)
        {
          SendMessage(hCtlNAON, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          EnableVerbNoun(FALSE);
        }
        else
        {
          SendMessage(hCtlNAON, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          hCtl = GetDlgItem(hWndDlg, PREMIUM[m_premiumIndex].onVerb);
          SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          hCtl = GetDlgItem(hWndDlg, PREMIUM[m_premiumIndex].onNoun);
          SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          if(PREMIUM[m_premiumIndex].onVerb == PREMIUMS_PULLOUT)
          {
            LoadString(hInst, TEXT_102, tempString, TEMPSTRING_LENGTH);  // From
            SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            EnableWindow(hCtlANYNONGARAGE, FALSE);
            EnableWindow(hCtlANYTIMEPOINT, FALSE);
            bGaragesOnly = TRUE;
          }
          else if(PREMIUM[m_premiumIndex].onVerb == PREMIUMS_PULLIN)
          {
            LoadString(hInst, TEXT_103, tempString, TEMPSTRING_LENGTH);  // To
            SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            EnableWindow(hCtlANYNONGARAGE, FALSE);
            EnableWindow(hCtlANYTIMEPOINT, FALSE);
            bGaragesOnly = TRUE;
          }
          else
          {
            LoadString(hInst, TEXT_101, tempString, TEMPSTRING_LENGTH);  // At
            SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            bGaragesOnly = FALSE;
          }
          if(PREMIUM[m_premiumIndex].onNoun != PREMIUMS_TIMEPOINT)
          {
            EnableWindow(hCtlTIMEPOINTLIST, FALSE);
          }
          else
          {
            nI = SetUpNodeList(hWndDlg, PREMIUMS_TIMEPOINTLIST, bGaragesOnly);
            for(nJ = 0; nJ < nI; nJ++)
            {
              if(SendMessage(hCtlTIMEPOINTLIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) ==
                    PREMIUM[m_premiumIndex].nounNODESrecordID)
              {
                SendMessage(hCtlTIMEPOINTLIST, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
                break;
              }
            }
          }
        }
//
//  Select
//
        if(PREMIUM[m_premiumIndex].select == NO_RECORD)
        {
          SendMessage(hCtlNASELECT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          EnableWindow(hCtlIDEXECUTE, FALSE);
        }
        else
        {
          SendMessage(hCtlNASELECT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          hCtl = GetDlgItem(hWndDlg, PREMIUM[m_premiumIndex].select);
          SendMessage(hCtl, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
      }
//
//  Make a local copy of the premium in case he hits Cancel
//
      localPremium = PREMIUM[m_premiumIndex];
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
//  Route stuff
//
        case PREMIUMS_ROUTELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNAROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNAROUTE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case PREMIUMS_NAROUTE:
          SendMessage(hCtlROUTELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case PREMIUMS_ROUTE:
          SendMessage(hCtlROUTELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Service stuff
//
        case PREMIUMS_SERVICELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNASERVICE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNASERVICE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case PREMIUMS_NASERVICE:
          SendMessage(hCtlSERVICELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case PREMIUMS_SERVICE:
          SendMessage(hCtlSERVICELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Bustype stuff
//
        case PREMIUMS_BUSTYPELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNAWITH, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNAWITH, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlBUSTYPE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              else if(SendMessage(hCtlANYACCESSIBLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlANYACCESSIBLE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlBUSTYPE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case PREMIUMS_NAWITH:
        case PREMIUMS_ANYACCESSIBLE:
          SendMessage(hCtlBUSTYPELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case PREMIUMS_BUSTYPE:
          SendMessage(hCtlBUSTYPELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  "Assigned to Garage" stuff
//
        case PREMIUMS_GARAGELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNAASSIGNEDTO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNAASSIGNEDTO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlASSIGNEDTO, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case PREMIUMS_NAASSIGNEDTO:
          SendMessage(hCtlGARAGELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case PREMIUMS_ASSIGNEDTO:
          SendMessage(hCtlGARAGELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  The "Verb" radio buttons change the title of the "Noun" groupbox
//
        case PREMIUMS_PULLOUT:
          LoadString(hInst, TEXT_102, tempString, TEMPSTRING_LENGTH);  // From
          SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          EnableWindow(hCtlANYNONGARAGE, FALSE);
          EnableWindow(hCtlANYTIMEPOINT, FALSE);
          bGaragesOnly = TRUE;
          if(SendMessage(hCtlTIMEPOINT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SetUpNodeList(hWndDlg, PREMIUMS_TIMEPOINTLIST, bGaragesOnly);
          break;

        case PREMIUMS_PULLIN:
          LoadString(hInst, TEXT_103, tempString, TEMPSTRING_LENGTH);  // To
          SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          EnableWindow(hCtlANYNONGARAGE, FALSE);
          EnableWindow(hCtlANYTIMEPOINT, FALSE);
          bGaragesOnly = TRUE;
          if(SendMessage(hCtlTIMEPOINT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SetUpNodeList(hWndDlg, PREMIUMS_TIMEPOINTLIST, bGaragesOnly);
          break;

        case PREMIUMS_STARTRELIEF:
        case PREMIUMS_ENDRELIEF:
        case PREMIUMS_FINISHAT:
          LoadString(hInst, TEXT_101, tempString, TEMPSTRING_LENGTH);  // At
          SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          EnableWindow(hCtlANYNONGARAGE, TRUE);
          EnableWindow(hCtlANYTIMEPOINT, TRUE);
          bGaragesOnly = FALSE;
          if(SendMessage(hCtlTIMEPOINT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SetUpNodeList(hWndDlg, PREMIUMS_TIMEPOINTLIST, bGaragesOnly);
          }
          break;
//
//  PREMIUMS_ANYGARAGE radio button
//  PREMIUMS_ANYNONGARAGE radio button
//  PREMIUMS_ANYTIMEPOINT radio button
//  PREMIUMS_TIMEPOINT radio button
//
        case PREMIUMS_ANYGARAGE:
        case PREMIUMS_ANYNONGARAGE:
        case PREMIUMS_ANYTIMEPOINT:
        case PREMIUMS_TIMEPOINT:
          enable = (wmId == PREMIUMS_TIMEPOINT);
          EnableWindow(hCtlTIMEPOINTLIST, enable);
          if(enable)
            SetUpNodeList(hWndDlg, PREMIUMS_TIMEPOINTLIST, bGaragesOnly);
          else
            SendMessage(hCtlTIMEPOINTLIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          break;
//
//  PREMIUMS_NAON check box
//
        case PREMIUMS_NAON:
          SendMessage(hCtlTIMEPOINTLIST, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          if(SendMessage(hCtlNAON, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableVerbNoun(FALSE);
          }
          else
          {
            EnableVerbNoun(TRUE);
            nCwCheckRadioButtons(hWndDlg, onVerbRG, PREMIUMS_PULLOUT, NUMVERBS);
            LoadString(hInst, TEXT_102, tempString, TEMPSTRING_LENGTH);  // From
            SendMessage(hCtlATFROMTO, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            nCwCheckRadioButtons(hWndDlg, onNounRG, PREMIUMS_ANYGARAGE, NUMNOUNS);
            EnableWindow(hCtlANYNONGARAGE, FALSE);
            EnableWindow(hCtlANYTIMEPOINT, FALSE);
            bGaragesOnly = TRUE;
            EnableWindow(hCtlTIMEPOINTLIST, FALSE);
          }
          break;
//
//  PREMIUMS_NASELECT check box
//
        case PREMIUMS_NASELECT:
          if(SendMessage(hCtlNASELECT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            EnableWindow(hCtlIDEXECUTE, FALSE);
          else
          {
            EnableWindow(hCtlIDEXECUTE, TRUE);
            nCwCheckRadioButtons(hWndDlg, selectRG, PREMIUMS_PAIDTRAVEL, 2);
          }
          break;
//
//  IDEXECUTE
//
        case IDEXECUTE:
//
//  Determine the selection
//
          if(SendMessage(hCtlPAIDTRAVEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BCTRAVEL), hWndDlg,
                  (DLGPROC)BCTRAVELMsgProc, (LPARAM)&localPremium);
          }
          else if(SendMessage(hCtlPAIDSTARTEND, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STARTEND), hWndDlg,
                  (DLGPROC)STARTENDMsgProc, (LPARAM)&localPremium);
          }
          break;
//
//  IDDELETE
//
        case IDDELETE:
          LoadString(hInst, ERROR_178, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONQUESTION);
          if(MessageBox(hWndDlg, tempString, TMS,
                MB_OK | MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2) != IDYES)
          {
            break;
          }
//
//  Shift the remaining premiums up
//
          for(nI = m_premiumIndex; nI < m_numPremiums - 1; nI++)
          {
            memcpy(&PREMIUM[nI], &PREMIUM[nI + 1], sizeof(PREMIUMDef));
            WritePremiums(nI);
          }
          memset(&PREMIUM[m_numPremiums - 1], 0x00, sizeof(PREMIUMDef));
          sprintf(szarString, "premium%d", m_numPremiums - 1);
          WritePrivateProfileString("Premiums", szarString, "", szWorkruleFile);
          WritePrivateProfileString(szarString, NULL, NULL, szWorkruleFile);
          m_numPremiums--;
          m_bWorkrulesChanged = TRUE;
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          if(m_bPremiumNew)
          {
            --m_numPremiums;
          }
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
//  Name
//
          SendMessage(hCtlNAME, WM_GETTEXT,
                (WPARAM)sizeof(localPremium.localName), (LONG)(LPSTR)localPremium.localName);
          if(strcmp(localPremium.localName, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_145, hCtlNAME);
            break;
          }
//
//  Route
//
          if(SendMessage(hCtlNAROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.ROUTESrecordID = NO_RECORD;
          else
          {
            nI = SendMessage(hCtlROUTELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_010, hCtlROUTELIST);
              break;
            }
            localPremium.ROUTESrecordID = SendMessage(hCtlROUTELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Service
//
          if(SendMessage(hCtlNASERVICE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.SERVICESrecordID = NO_RECORD;
          else
          {
            nI = SendMessage(hCtlSERVICELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_011, hCtlSERVICELIST);
              break;
            }
            localPremium.SERVICESrecordID =
                  SendMessage(hCtlSERVICELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Before
//
          if(SendMessage(hCtlNABEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.beforeTime = NO_TIME;
          else
          {
            nI = SendMessage(hCtlBEFORETIME, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localPremium.beforeTime = cTime(tempString);
          }
//
//  Bustype
//
          if(SendMessage(hCtlNAWITH, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.bustype = NO_RECORD;
          else
          {
            if(SendMessage(hCtlANYACCESSIBLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              localPremium.bustype = PREMIUMS_ANYACCESSIBLE;
            else
            {
              nI = SendMessage(hCtlBUSTYPELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_090, hCtlBUSTYPELIST);
                break;
              }
              localPremium.bustype = PREMIUMS_BUSTYPE;
              localPremium.BUSTYPESrecordID =
                    SendMessage(hCtlBUSTYPELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            }
          }
//
//  AssignedTo
//
          if(SendMessage(hCtlNAASSIGNEDTO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.assignedToNODESrecordID = NO_RECORD;
          else
          {
            nI = SendMessage(hCtlGARAGELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_127, hCtlGARAGELIST);
              break;
            }
            localPremium.assignedToNODESrecordID =
                  SendMessage(hCtlGARAGELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Piece
//
          if(SendMessage(hCtlNAPIECE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            localPremium.forPieceNumber = NO_RECORD;
          }
          else
          {
            nI = GetDlgItemInt(hWndDlg, PREMIUMS_PIECENUMBER, (BOOL *)NULL, TRUE);
            if(nI < 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_089, hCtlPIECENUMBER);
              break;
            }
            localPremium.forPieceNumber = nI;
          }
//
//  Piece size
//
          if(SendMessage(hCtlNAPIECESIZE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.withPieceSize = NO_RECORD;
          else
          {
            SendMessage(hCtlPIECESIZE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localPremium.withPieceSize = thhmm(tempString);
          }
//
//  On verb and noun
//
          if(SendMessage(hCtlNAON, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            localPremium.onVerb = NO_RECORD;
            localPremium.onNoun = NO_RECORD;
          }
          else
          {
            for(nI = 0; nI < NUMVERBS; nI++)
            {
              if(SendMessage(GetDlgItem(hWndDlg, onVerbRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                localPremium.onVerb = onVerbRG[nI];
                break;
              }
            }
            for(nI = 0; nI < NUMNOUNS; nI++)
            {
              if(SendMessage(GetDlgItem(hWndDlg, onNounRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                localPremium.onNoun = onNounRG[nI];
                break;
              }
            }
            if(localPremium.onNoun != PREMIUMS_TIMEPOINT)
              localPremium.nounNODESrecordID = NO_RECORD;
            else
            {
              nJ = SendMessage(hCtlTIMEPOINTLIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nJ == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlTIMEPOINTLIST);
                break;
              }
              else
                localPremium.nounNODESrecordID =
                      SendMessage(hCtlTIMEPOINTLIST, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
            }
          }

//
//  Select
//
          if(SendMessage(hCtlNASELECT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localPremium.select = NO_RECORD;
          else
          {
            for(nI = 0; nI < 2; nI++)
            {
              if(SendMessage(GetDlgItem(hWndDlg, selectRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                localPremium.select = selectRG[nI];
                break;
              }
            }
          }
//
//  Passed all the tests - All done
//
          PREMIUM[m_premiumIndex] = localPremium;
          m_bWorkrulesChanged = TRUE;
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
   }
   return TRUE;
}

void EnableVerbNoun(BOOL bEnable)
{
  EnableWindow(hCtlACTION, bEnable);
  EnableWindow(hCtlPULLOUT, bEnable);
  EnableWindow(hCtlPULLIN, bEnable);
  EnableWindow(hCtlSTARTRELIEF, bEnable);
  EnableWindow(hCtlENDRELIEF, bEnable);
  EnableWindow(hCtlFINISHAT, bEnable);
  EnableWindow(hCtlATFROMTO, bEnable);
  EnableWindow(hCtlANYGARAGE, bEnable);
  EnableWindow(hCtlANYNONGARAGE, bEnable);
  EnableWindow(hCtlANYTIMEPOINT, bEnable);
  EnableWindow(hCtlTIMEPOINT, bEnable);
  EnableWindow(hCtlTIMEPOINTLIST, bEnable);
}
