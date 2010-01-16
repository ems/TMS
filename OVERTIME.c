//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK OVERTIMEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static OVERTIMEDef localOT;
  static HANDLE hCtlSERVICEDAY4;
  static HANDLE hCtlDAILYSTARTS4;
  static HANDLE hCtlDAILYRATE4;
  static HANDLE hCtlWEEKLYSTARTS4;
  static HANDLE hCtlWEEKLYRATE4;
  static HANDLE hCtlSERVICEDAY5;
  static HANDLE hCtlDAILYSTARTS5;
  static HANDLE hCtlDAILYRATE5;
  static HANDLE hCtlWEEKLYSTARTS5;
  static HANDLE hCtlWEEKLYRATE5;
  static HANDLE hCtlSERVICEDAYSPD;
  static HANDLE hCtlSPDSTARTS;
  static HANDLE hCtlSPDRATE;
  static HANDLE hCtlOTCALCMBYM;
  static HANDLE hCtlOTCALCBOUNDED;
  static HANDLE hCtlBOUNDEDMINUTES;
  static HANDLE hCtlROUNDUPTONEXT;
  static HANDLE hCtlBOUNDEDINTERVAL;
  static HANDLE hCtlROUNDUP;
  static HANDLE hCtlROUNDDOWN;
  static HANDLE hCtlKEEPHALF;
  static HANDLE hCtlPAYLOWER;
  static HANDLE hCtlPAYHIGHER;
  static HANDLE hCtlPAYBOTH;
  static HANDLE hCtlPAYMAKEUPBEFORESPREAD;
  static HANDLE hCtl4DAYWEEKGUARANTEE;
  static HANDLE hCtl5DAYWEEKGUARANTEE;
  static HANDLE serviceBoxes[3];
  static int    currentService[3];
  char   serviceName[SERVICES_NAME_LENGTH + 1];
  int    nI;
  int    nJ;
  int    rcode2;
  short  int wmId;
  short  int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlSERVICEDAY4 = GetDlgItem(hWndDlg, OVERTIME_SERVICEDAY4);
      hCtlDAILYSTARTS4 = GetDlgItem(hWndDlg, OVERTIME_DAILYSTARTS4);
      hCtlDAILYRATE4 = GetDlgItem(hWndDlg, OVERTIME_DAILYRATE4);
      hCtlWEEKLYSTARTS4 = GetDlgItem(hWndDlg, OVERTIME_WEEKLYSTARTS4);
      hCtlWEEKLYRATE4 = GetDlgItem(hWndDlg, OVERTIME_WEEKLYRATE4);
      hCtlSERVICEDAY5 = GetDlgItem(hWndDlg, OVERTIME_SERVICEDAY5);
      hCtlDAILYSTARTS5 = GetDlgItem(hWndDlg, OVERTIME_DAILYSTARTS5);
      hCtlDAILYRATE5 = GetDlgItem(hWndDlg, OVERTIME_DAILYRATE5);
      hCtlWEEKLYSTARTS5 = GetDlgItem(hWndDlg, OVERTIME_WEEKLYSTARTS5);
      hCtlWEEKLYRATE5 = GetDlgItem(hWndDlg, OVERTIME_WEEKLYRATE5);
      hCtlSERVICEDAYSPD = GetDlgItem(hWndDlg, OVERTIME_SERVICEDAYSPD);
      hCtlSPDSTARTS = GetDlgItem(hWndDlg, OVERTIME_SPDSTARTS);
      hCtlSPDRATE = GetDlgItem(hWndDlg, OVERTIME_SPDRATE);
      hCtlOTCALCMBYM = GetDlgItem(hWndDlg, OVERTIME_OTCALCMBYM);
      hCtlOTCALCBOUNDED = GetDlgItem(hWndDlg, OVERTIME_OTCALCBOUNDED);
      hCtlBOUNDEDMINUTES = GetDlgItem(hWndDlg, OVERTIME_BOUNDEDMINUTES);
      hCtlROUNDUPTONEXT = GetDlgItem(hWndDlg, OVERTIME_ROUNDUPTONEXT);
      hCtlBOUNDEDINTERVAL = GetDlgItem(hWndDlg, OVERTIME_BOUNDEDINTERVAL);
      hCtlROUNDUP = GetDlgItem(hWndDlg, OVERTIME_ROUNDUP);
      hCtlROUNDDOWN = GetDlgItem(hWndDlg, OVERTIME_ROUNDDOWN);
      hCtlKEEPHALF = GetDlgItem(hWndDlg, OVERTIME_KEEPHALF);
      hCtlPAYLOWER = GetDlgItem(hWndDlg, OVERTIME_PAYLOWER);
      hCtlPAYHIGHER = GetDlgItem(hWndDlg, OVERTIME_PAYHIGHER);
      hCtlPAYBOTH = GetDlgItem(hWndDlg, OVERTIME_PAYBOTH);
      hCtlPAYMAKEUPBEFORESPREAD = GetDlgItem(hWndDlg, OVERTIME_PAYMAKEUPBEFORESPREAD);
      hCtl4DAYWEEKGUARANTEE = GetDlgItem(hWndDlg, OVERTIME_4DAYWEEKGUARANTEE);
      hCtl5DAYWEEKGUARANTEE = GetDlgItem(hWndDlg, OVERTIME_5DAYWEEKGUARANTEE);
//
//  Set up the service list boxes
//
      serviceBoxes[0] = hCtlSERVICEDAY4;
      serviceBoxes[1] = hCtlSERVICEDAY5;
      serviceBoxes[2] = hCtlSERVICEDAYSPD;
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      nJ = 0;
      while(rcode2 == 0)
      {
        if(nJ >= MAXSERVICES)
          break;
        strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(serviceName, SERVICES_NAME_LENGTH);
        for(nI = 0; nI < 3; nI++)
        {
          SendMessage(serviceBoxes[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)serviceName);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        nJ++;
      }
//
//  Display the current values
//
      memcpy(&localOT, &OVERTIME, sizeof(OVERTIMEDef));
      currentService[WEEK4DAYS] = NO_RECORD;
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_FOURDAYWORKWEEK, (LPARAM)0);
      currentService[WEEK5DAYS] = NO_RECORD;
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_FIVEDAYWORKWEEK, (LPARAM)0);
      currentService[SPREADOT] = NO_RECORD;
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_SPREADOVERTIME, (LPARAM)0);
      sprintf(tempString, "%ld", localOT.roundedBounded);
      if(localOT.payMinuteByMinute)
        SendMessage(hCtlOTCALCMBYM, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else if(!(localOT.flags & OTF_ROUNDUPTONEXT))
      {
        SendMessage(hCtlOTCALCBOUNDED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlBOUNDEDMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
      else
      {
        SendMessage(hCtlROUNDUPTONEXT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlBOUNDEDINTERVAL, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
//
//  And the spread payment flag
//
      if(localOT.flags & OTF_PAYMAKEUPBEFORESPREAD)
        SendMessage(hCtlPAYMAKEUPBEFORESPREAD, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  And the rounding buttons
//
      if(localOT.flags & OTF_ROUNDUP)
        SendMessage(hCtlROUNDUP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else if(localOT.flags & OTF_ROUNDDOWN)
        SendMessage(hCtlROUNDDOWN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlKEEPHALF, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  And the overlap flags
//
      if(localOT.flags & OTF_PAYLOWER)
        SendMessage(hCtlPAYLOWER, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else if(localOT.flags & OTF_PAYHIGHER)
        SendMessage(hCtlPAYHIGHER, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlPAYBOTH, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);   
//
//  And the guarantees
//
      strcpy(tempString, chhmm(localOT.guarantee4DayWorkWeek));
      SendMessage(hCtl4DAYWEEKGUARANTEE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      strcpy(tempString, chhmm(localOT.guarantee5DayWorkWeek));
      SendMessage(hCtl5DAYWEEKGUARANTEE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      break;  //  End of WM_INITDIALOG
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_USERSETUP:
//
    case WM_USERSETUP:
      switch(wParam)
      {
        case OVERTIME_FOURDAYWORKWEEK:
          if(currentService[WEEK4DAYS] == NO_RECORD)
          {
            SendMessage(hCtlWEEKLYSTARTS4, WM_SETTEXT, (WPARAM)0,
                  (LONG)(LPSTR)chhmm(localOT.weeklyAfter[WEEK4DAYS]));
            gcvt(localOT.weeklyRate[WEEK4DAYS], NUMDECS, tempString);
            SendMessage(hCtlWEEKLYRATE4, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            currentService[WEEK4DAYS] = 0;
          }
          else  // Save current values into index LOWORD(lParam)
          {
            nJ = currentService[WEEK4DAYS];
            SendMessage(hCtlDAILYSTARTS4, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.dailyAfter[WEEK4DAYS][nJ] = thhmm(tempString);
            SendMessage(hCtlDAILYRATE4, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.dailyRate[WEEK4DAYS][nJ] = (float)atof(tempString);
            SendMessage(hCtlWEEKLYSTARTS4, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.weeklyAfter[WEEK4DAYS] = thhmm(tempString);
            SendMessage(hCtlWEEKLYRATE4, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.weeklyRate[WEEK4DAYS] = (float)atof(tempString);
            currentService[WEEK4DAYS] = LOWORD(lParam);
          }
          SendMessage(hCtlSERVICEDAY4, LB_SETCURSEL, (WPARAM)currentService[WEEK4DAYS], (LPARAM)0);
          SendMessage(hCtlDAILYSTARTS4, WM_SETTEXT, (WPARAM)0,
                (LONG)(LPSTR)chhmm(localOT.dailyAfter[WEEK4DAYS][currentService[WEEK4DAYS]]));
          gcvt(localOT.dailyRate[WEEK4DAYS][currentService[WEEK4DAYS]], NUMDECS, tempString);
          SendMessage(hCtlDAILYRATE4, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          break;

        case OVERTIME_FIVEDAYWORKWEEK:
          if(currentService[WEEK5DAYS] == NO_RECORD)
          {
            SendMessage(hCtlWEEKLYSTARTS5, WM_SETTEXT, (WPARAM)0,
                  (LONG)(LPSTR)chhmm(localOT.weeklyAfter[WEEK5DAYS]));
            gcvt(localOT.weeklyRate[WEEK5DAYS], NUMDECS, tempString);
            SendMessage(hCtlWEEKLYRATE5, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
            currentService[WEEK5DAYS] = 0;
          }
          else  // Save current values into index LOWORD(lParam)
          {
            nJ = currentService[WEEK5DAYS];
            SendMessage(hCtlDAILYSTARTS5, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.dailyAfter[WEEK5DAYS][nJ] = thhmm(tempString);
            SendMessage(hCtlDAILYRATE5, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.dailyRate[WEEK5DAYS][nJ] = (float)atof(tempString);
            SendMessage(hCtlWEEKLYSTARTS5, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.weeklyAfter[WEEK5DAYS] = thhmm(tempString);
            SendMessage(hCtlWEEKLYRATE5, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.weeklyRate[WEEK5DAYS] = (float)atof(tempString);
            currentService[WEEK5DAYS] = LOWORD(lParam);
          }
          SendMessage(hCtlSERVICEDAY5, LB_SETCURSEL, (WPARAM)currentService[WEEK5DAYS], (LPARAM)0);
          SendMessage(hCtlDAILYSTARTS5, WM_SETTEXT, (WPARAM)0,
                (LONG)(LPSTR)chhmm(localOT.dailyAfter[WEEK5DAYS][currentService[WEEK5DAYS]]));
          gcvt(localOT.dailyRate[WEEK5DAYS][currentService[WEEK5DAYS]], NUMDECS, tempString);
          SendMessage(hCtlDAILYRATE5, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          break;

        case OVERTIME_SPREADOVERTIME:
          if(currentService[SPREADOT] == NO_RECORD)
            currentService[SPREADOT] = 0;
          else  // Save current values into index LOWORD(lParam)
          {
            nJ = currentService[SPREADOT];
            SendMessage(hCtlSPDSTARTS, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.spreadAfter[nJ] = thhmm(tempString);
            SendMessage(hCtlSPDRATE, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            localOT.spreadRate[nJ] = (float)atof(tempString);
            currentService[SPREADOT] = LOWORD(lParam);
          }
          SendMessage(hCtlSERVICEDAYSPD, LB_SETCURSEL, (WPARAM)currentService[SPREADOT], (LPARAM)0);
          SendMessage(hCtlSPDSTARTS, WM_SETTEXT, (WPARAM)0,
                (LONG)(LPSTR)chhmm(localOT.spreadAfter[currentService[SPREADOT]]));
          gcvt(localOT.spreadRate[currentService[SPREADOT]], NUMDECS, tempString);
          SendMessage(hCtlSPDRATE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          break;
      }
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
        case OVERTIME_SERVICEDAY4:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              nI = SendMessage(hCtlSERVICEDAY4, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI != LB_ERR)
                SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_FOURDAYWORKWEEK, MAKELPARAM(nI, 0));
              break;
          }
          break;

        case OVERTIME_SERVICEDAY5:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              nI = SendMessage(hCtlSERVICEDAY5, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI != LB_ERR)
                SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_FIVEDAYWORKWEEK, MAKELPARAM(nI, 0));
              break;
          }

        case OVERTIME_SERVICEDAYSPD:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              nI = SendMessage(hCtlSERVICEDAYSPD, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI != LB_ERR)
                SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)OVERTIME_SPREADOVERTIME, MAKELPARAM(nI, 0));
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Overtime_Rules);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Force the current values into the structure
//
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(OVERTIME_SERVICEDAY4, LBN_SELCHANGE), (LPARAM)0);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(OVERTIME_SERVICEDAY5, LBN_SELCHANGE), (LPARAM)0);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(OVERTIME_SERVICEDAYSPD, LBN_SELCHANGE), (LPARAM)0);
          localOT.flags = 0;
//
//  Get the Overtime Calculation
//
          localOT.payMinuteByMinute = SendMessage(hCtlOTCALCMBYM, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(!localOT.payMinuteByMinute)
          {
            if(!SendMessage(hCtlROUNDUPTONEXT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              SendMessage(hCtlBOUNDEDMINUTES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            }
            else
            {
              localOT.flags |= OTF_ROUNDUPTONEXT;
              SendMessage(hCtlBOUNDEDINTERVAL, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            }
            localOT.roundedBounded = atol(tempString);
          }
//
//  And the spread overtime payment flag
//
          if(SendMessage(hCtlPAYMAKEUPBEFORESPREAD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localOT.flags |= OTF_PAYMAKEUPBEFORESPREAD;
//
//  And the rounding flags
//
          if(SendMessage(hCtlROUNDUP, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localOT.flags |= OTF_ROUNDUP;
          else if(SendMessage(hCtlROUNDDOWN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localOT.flags |= OTF_ROUNDDOWN;
          else
            localOT.flags |= OTF_KEEPHALF;
//
//  And the overlap flags
//
          if(SendMessage(hCtlPAYLOWER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localOT.flags |= OTF_PAYLOWER;
          else if(SendMessage(hCtlPAYHIGHER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            localOT.flags |= OTF_PAYHIGHER;
          else
            localOT.flags |= OTF_PAYBOTH;
//
//  And the guarantees
//
          SendMessage(hCtl4DAYWEEKGUARANTEE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          localOT.guarantee4DayWorkWeek = thhmm(tempString);
          SendMessage(hCtl5DAYWEEKGUARANTEE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          localOT.guarantee5DayWorkWeek = thhmm(tempString);
//
//  Flag the change to the INI file and leave
//
          memcpy(&OVERTIME, &localOT, sizeof(OVERTIMEDef));
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
