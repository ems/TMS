//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define DISTBUTTONMAX 2
#define TIMEBUTTONMAX 2

BOOL CALLBACK PREFERENCESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static int TimeOfDayRG[TIMEBUTTONMAX];
  static int TimeHHMMRG[TIMEBUTTONMAX];
  static int DistRG[DISTBUTTONMAX];
  static HANDLE hCtlWRAP;
  static HANDLE hCtlAVAILABLE;
  static HANDLE hCtlSELECTED;
  static HANDLE hCtlBT;
  static HANDLE hCtlBUSTYPELIST;
  static HANDLE hCtlNABUSTYPE;
  static HANDLE hCtlUSETRIPPLANNER;
  static HANDLE hCtlUSEDYNAMICTRAVELS;
  static HANDLE hCtlROSTERCOMPRESSED;
  static HANDLE hCtlROSTEREXPANDED;
  static HANDLE hCtlROSTERSTYLE3;
  static HANDLE hCtlROSTERSTYLE4;
  static HANDLE hCtlROSTERSTYLE5;
  static HANDLE hCtlHOWMANY;
  static HANDLE hCtlMETERS;
  static HANDLE hCtlSTATE;
  static HANDLE hCtlZIP;
  static HANDLE hCtlSSN;
  HANDLE    hCtl;
  BOOL bEnable;
  BOOL bFound;
  int  nI;
  int  nJ;
  int  numBustypes;
  int  numEntries;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlWRAP = GetDlgItem(hWndDlg, PREFERENCES_WRAP);
      hCtlAVAILABLE = GetDlgItem(hWndDlg, PREFERENCES_AVAILABLE);
      hCtlSELECTED = GetDlgItem(hWndDlg, PREFERENCES_SELECTED);
      hCtlNABUSTYPE = GetDlgItem(hWndDlg, PREFERENCES_NABUSTYPE);
      hCtlBT = GetDlgItem(hWndDlg, PREFERENCES_BT);
      hCtlBUSTYPELIST = GetDlgItem(hWndDlg, PREFERENCES_BUSTYPELIST);
      hCtlUSETRIPPLANNER = GetDlgItem(hWndDlg, PREFERENCES_USETRIPPLANNER);
      hCtlUSEDYNAMICTRAVELS = GetDlgItem(hWndDlg, PREFERENCES_USEDYNAMICTRAVELS);
      hCtlROSTERCOMPRESSED = GetDlgItem(hWndDlg, PREFERENCES_ROSTERCOMPRESSED);
      hCtlROSTEREXPANDED = GetDlgItem(hWndDlg, PREFERENCES_ROSTEREXPANDED);
      hCtlROSTERSTYLE3 = GetDlgItem(hWndDlg, PREFERENCES_ROSTERSTYLE3);
      hCtlROSTERSTYLE4 = GetDlgItem(hWndDlg, PREFERENCES_ROSTERSTYLE4);
      hCtlROSTERSTYLE5 = GetDlgItem(hWndDlg, PREFERENCES_ROSTERSTYLE5);
      hCtlHOWMANY = GetDlgItem(hWndDlg, PREFERENCES_HOWMANY);
      hCtlMETERS = GetDlgItem(hWndDlg, PREFERENCES_METERS);
      hCtlSTATE = GetDlgItem(hWndDlg, PREFERENCES_STATE);
      hCtlZIP = GetDlgItem(hWndDlg, PREFERENCES_ZIP);
      hCtlSSN = GetDlgItem(hWndDlg, PREFERENCES_SSN);
//
//  Reports
//
      for(nI = 0; nI < m_LastReport; nI++)
      {
        hCtl = (TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE) ? hCtlSELECTED : hCtlAVAILABLE;
        nJ = (int)SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)TMSRPT[nI].szReportName);
        SendMessage(hCtl, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)nI);
      }
//
//  Time of day format
//
      TimeOfDayRG[0] = PREFERENCES_APX;
      TimeOfDayRG[1] = PREFERENCES_MILITARY;
      nI = GetPrivateProfileInt((LPSTR)userName,
            (LPSTR)"timeFormat", (int)PREFERENCES_APX, (LPSTR)szDatabaseFileName);
      if(nI != PREFERENCES_APX && nI != PREFERENCES_MILITARY)
      {
        nI = PREFERENCES_APX;
      }
      SendMessage(GetDlgItem(hWndDlg, nI), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Time wrap
//
      m_bWrapTime = GetPrivateProfileInt((LPSTR)userName,
            (LPSTR)"timeWrap", (int)0, (LPSTR)szDatabaseFileName);
      if(m_bWrapTime)
      {
        SendMessage(hCtlWRAP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Time of day format
//
      TimeHHMMRG[0] = PREFERENCES_HHMM;
      TimeHHMMRG[1] = PREFERENCES_HHDM;
      nI = GetPrivateProfileInt((LPSTR)userName,
            (LPSTR)"timeHHMMFormat", (int)PREFERENCES_HHMM, (LPSTR)szDatabaseFileName);
      if(nI != PREFERENCES_HHMM && nI != PREFERENCES_HHDM)
      {
        nI = PREFERENCES_HHMM;
      }
      SendMessage(GetDlgItem(hWndDlg, nI), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Default bustype
//
      bFound = FALSE;
      numBustypes = SetUpBustypeList(hWndDlg, PREFERENCES_BUSTYPELIST);
      if(numBustypes == 0)
      {
        EnableWindow(hCtlBT, FALSE);
        EnableWindow(hCtlBUSTYPELIST, FALSE);
      }
      else
      {
        if(defaultBustype != NO_RECORD)
        {
          for(nI = 0; nI < numBustypes; nI++)
          {
            if(SendMessage(hCtlBUSTYPELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == defaultBustype)
            {
              bFound = TRUE;
              SendMessage(hCtlBUSTYPELIST, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              break;
            }
          }
        }
      }
      SendMessage(hCtlNABUSTYPE, BM_SETCHECK, (WPARAM)!bFound, (LPARAM)0);
//
//  TMS-Generated Travel Times
//
      nI = GetPrivateProfileInt((LPSTR)userName, (LPSTR)"TMSGeneratedTravelTimes",
            (int)PREFERENCES_USETRIPPLANNER, (LPSTR)szDatabaseFileName);
      SendMessage(GetDlgItem(hWndDlg, nI), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
	  if(nI != PREFERENCES_USETRIPPLANNER)
	  {
      EnableWindow(hCtlHOWMANY, FALSE);
      EnableWindow(hCtlMETERS, FALSE);
    }
    else
    {
      nI = GetPrivateProfileInt((LPSTR)userName, (LPSTR)"MaxWalkingDistance",
            (int)300, (LPSTR)szDatabaseFileName);
      itoa(nI, tempString, 10);
      SendMessage(hCtlMETERS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
    }
//
//  Roster format
//
      nI = GetPrivateProfileInt((LPSTR)userName, (LPSTR)"RosterFormat",
            (int)PREFERENCES_ROSTERCOMPRESSED, (LPSTR)szDatabaseFileName);
      SendMessage(GetDlgItem(hWndDlg, nI), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Locale stuff
//
//  Distance format
//
      DistRG[0] = PREFERENCES_MILES;
      DistRG[1] = PREFERENCES_KILOMETERS;
      nI = GetPrivateProfileInt((LPSTR)userName,
            (LPSTR)"distanceMeasure", (int)PREFERENCES_MILES, (LPSTR)szDatabaseFileName);
      if(nI != PREFERENCES_MILES && nI != PREFERENCES_KILOMETERS)
        nI = PREFERENCES_MILES;
      SendMessage(GetDlgItem(hWndDlg, nI), BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  State display
//
      GetPrivateProfileString((LPSTR)userName, (LPSTR)"StateDisplay", "State",
            tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
      SendMessage(hCtlSTATE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  ZIP display
//
      GetPrivateProfileString((LPSTR)userName, (LPSTR)"ZIPDisplay", "ZIP",
            tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
      SendMessage(hCtlZIP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  SSN display
//
      GetPrivateProfileString((LPSTR)userName, (LPSTR)"SSNDisplay", "SSN",
            tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
      SendMessage(hCtlSSN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      break;
//
//  WM_CLOSE - Send an IDCANCEL
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
        case PREFERENCES_AVAILABLE:
          switch(wmEvent)
          {
            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDADD, 0), (LPARAM)0);
              break;
            default:
              nI = (int)SendMessage(hCtlAVAILABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              EnableWindow(GetDlgItem(hWndDlg, IDADD), nI != LB_ERR);
              break;
          }
          break;

        case PREFERENCES_SELECTED:
          switch(wmEvent)
          {
            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDREMOVE, 0), (LPARAM)0);
              break;
            default:
              nI = (int)SendMessage(hCtlSELECTED, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              bEnable = nI != LB_ERR;
              EnableWindow(GetDlgItem(hWndDlg, IDREMOVE), bEnable);
              EnableWindow(GetDlgItem(hWndDlg, IDDEFINE), FALSE);  // Assume disabled
              if(bEnable && (nJ = (int)SendMessage(hCtlSELECTED, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0)) != LB_ERR)
              {
                if(TMSRPT[nJ].lpConfigFunc != NULL)
                  EnableWindow(GetDlgItem(hWndDlg, IDDEFINE), TRUE);
              }
          }
          break;

         case IDADD:
          nI = (int)SendMessage(hCtlAVAILABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlAVAILABLE, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          nJ = (int)SendMessage(hCtlAVAILABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          SendMessage(hCtlAVAILABLE, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          nI = (int)SendMessage(hCtlSELECTED, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtlSELECTED, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)nJ);
          EnableWindow(GetDlgItem(hWndDlg, IDADD), FALSE);
          break;

        case IDREMOVE:
          nI = (int)SendMessage(hCtlSELECTED, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlSELECTED, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          nJ = (int)SendMessage(hCtlSELECTED, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          SendMessage(hCtlSELECTED, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          nI = (int)SendMessage(hCtlAVAILABLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtlAVAILABLE, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)nJ);
          EnableWindow(GetDlgItem(hWndDlg, IDREMOVE), FALSE);
          EnableWindow(GetDlgItem(hWndDlg, IDDEFINE), FALSE);
          break;

        case IDDEFINE:
          if((nI = (int)SendMessage(hCtlSELECTED, LB_GETCURSEL, (WPARAM)0, (LPARAM)0)) != LB_ERR &&
                (nJ = (int)SendMessage(hCtlSELECTED, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0)) != LB_ERR)
          {
            if(TMSRPT[nJ].lpConfigFunc != NULL)
            {
              DialogBox(hInst, MAKEINTRESOURCE(TMSRPT[nJ].ConfigFuncDlg), hWndDlg, (DLGPROC)TMSRPT[nJ].lpConfigFunc);
            }
          }
          break;

        case PREFERENCES_USETRIPPLANNER:
          EnableWindow(hCtlHOWMANY, TRUE);
          EnableWindow(hCtlMETERS, TRUE);
          break;

        case PREFERENCES_USEDYNAMICTRAVELS:
          EnableWindow(hCtlHOWMANY, FALSE);
          EnableWindow(hCtlMETERS, FALSE);
          SendMessage(hCtlMETERS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Setting_Preferences);
          break;

        case IDOK:
//
//  Time of day format
//
           for(nI = 0; nI < TIMEBUTTONMAX; nI++)
           {
             if(SendMessage(GetDlgItem(hWndDlg, TimeOfDayRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
             {
               timeFormat = TimeOfDayRG[nI];
               itoa(timeFormat, tempString, 10);
               WritePrivateProfileString((LPSTR)userName, (LPSTR)"timeFormat",
                     tempString, (LPSTR)szDatabaseFileName);
             }
           }
//
//  Time wrap
//
           itoa(m_bWrapTime, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"timeWrap",
                 tempString, (LPSTR)szDatabaseFileName);
//
//  Time hours/minutes format
//
           for(nI = 0; nI < TIMEBUTTONMAX; nI++)
           {
             if(SendMessage(GetDlgItem(hWndDlg, TimeHHMMRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
             {
               timeHHMMFormat = TimeHHMMRG[nI];
               itoa(timeHHMMFormat, tempString, 10);
               WritePrivateProfileString((LPSTR)userName, (LPSTR)"timeHHMMFormat",
                  tempString, (LPSTR)szDatabaseFileName);
             }
           }
//
//  Default bustype
//
           defaultBustype = NO_RECORD;
           if(!SendMessage(hCtlNABUSTYPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             nI = SendMessage(hCtlBUSTYPELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
             if(nI != CB_ERR)
               defaultBustype = SendMessage(hCtlBUSTYPELIST, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
           }
           ltoa(defaultBustype, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"defaultBustype",
                 tempString, (LPSTR)szDatabaseFileName);
//
//  TMS-Generated Travel Times
//
           if(SendMessage(hCtlUSETRIPPLANNER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             nI = PREFERENCES_USETRIPPLANNER;
             bUseCISPlan = TRUE;
             SendMessage(hCtlMETERS, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
             WritePrivateProfileString((LPSTR)userName, (LPSTR)"MaxWalkingDistance",
                   tempString, (LPSTR)szDatabaseFileName);
           }
           else
           {
             nI = PREFERENCES_USEDYNAMICTRAVELS;
             bUseCISPlan = FALSE;
           }
           itoa(nI, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"TMSGeneratedTravelTimes",
                 tempString, (LPSTR)szDatabaseFileName);
//
//  Roster format
//
           if(SendMessage(hCtlROSTERCOMPRESSED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             rosterFormat = PREFERENCES_ROSTERCOMPRESSED;
           }
           else if(SendMessage(hCtlROSTEREXPANDED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             rosterFormat = PREFERENCES_ROSTEREXPANDED;
           }
           else if(SendMessage(hCtlROSTERSTYLE3, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             rosterFormat = PREFERENCES_ROSTERSTYLE3;
           }
           else if(SendMessage(hCtlROSTERSTYLE4, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             rosterFormat = PREFERENCES_ROSTERSTYLE4;
           }
           else
           {
             rosterFormat = PREFERENCES_ROSTERSTYLE5;
           }
           itoa(rosterFormat, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"RosterFormat",
                 tempString, (LPSTR)szDatabaseFileName);
//
//
//  Bitmask stuff for the reports
//
//  Determine which reports are (still/now) active
//
           for(nI = 0; nI < m_LastReport; nI++)
           {  
             if(TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE)
             {
               TMSRPT[nI].flags &= ~TMSRPTFLAG_ACTIVE;
             }
           }
           numEntries = (int)SendMessage(hCtlSELECTED, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
           for(nI = 0; nI < numEntries; nI++)
           {
             nJ = (int)SendMessage(hCtlSELECTED, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
             TMSRPT[nJ].flags |= TMSRPTFLAG_ACTIVE;
           }
//
//  Locale stuff
//
//  Distance format
//
           for(nI = 0; nI < DISTBUTTONMAX; nI++)
           {
             if(SendMessage(GetDlgItem(hWndDlg, DistRG[nI]), BM_GETCHECK, (WPARAM)0, (LPARAM)0))
             {
               distanceMeasure = DistRG[nI];
               itoa(distanceMeasure, tempString, 10);
               WritePrivateProfileString((LPSTR)userName, (LPSTR)"distanceMeasure",
                  tempString, (LPSTR)szDatabaseFileName);
             }
           }
//
//  State
//
           SendMessage(hCtlSTATE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"StateDisplay",
                 tempString, (LPSTR)szDatabaseFileName);
//
//  ZIP
//
           SendMessage(hCtlZIP, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"ZIPDisplay",
                 tempString, (LPSTR)szDatabaseFileName);
//
//  SSN
//
           SendMessage(hCtlSSN, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           WritePrivateProfileString((LPSTR)userName, (LPSTR)"SSNDisplay",
                 tempString, (LPSTR)szDatabaseFileName);
           EndDialog(hWndDlg, TRUE);
           break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
