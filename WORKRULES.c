//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void EnableTravelSection(HWND, BOOL);

BOOL CALLBACK WORKRULESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static PDISPLAYINFO pDI;
  static HANDLE hCtlALLRUNTYPES;
  static HANDLE hCtlONERUNTYPE;
  static HANDLE hCtlRUNTYPES;
  static HANDLE hCtlPREMIUMLIST;
  static HANDLE hCtlPREMIUMTIME;
  static HANDLE hCtlTIMECOUNTSINOT;
  static HANDLE hCtlPREMIUMREPORT;
  static HANDLE hCtlTRAVELTIMEPREMIUM;
  static HANDLE hCtlDONTPAY;
  static HANDLE hCtlDONTINCORPORATE;
  static HANDLE hCtlUSECONNIFNOSCHED;
  static HANDLE hCtlIDNEWPREMIUM;
  static HANDLE hCtlIDSETPREMIUM;
  static int    runtypeIndex;
  static int    slotNumber;
  static int    feasibleRG[4] = {WORKRULES_TIMEENTERED, WORKRULES_ASINCONNECTIONS,
                                 WORKRULES_ASPERSCHEDULE, WORKRULES_ASPERSCHEDULEDWELL};
  static int    paidHowRG[4]  = {WORKRULES_PAYACTUAL, WORKRULES_PAYFLAT, 
                                 WORKRULES_PAYPERCENTAGE, WORKRULES_PAYSTAGGERED};
  int    nI;
  int    nJ;
  int    nK;
  int    nRc;
  long   tempLong;
  BOOL   enable;
  int    rcode2;
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
      hCtlALLRUNTYPES = GetDlgItem(hWndDlg, WORKRULES_ALLRUNTYPES);
      hCtlONERUNTYPE = GetDlgItem(hWndDlg, WORKRULES_ONERUNTYPE);
      hCtlRUNTYPES = GetDlgItem(hWndDlg, WORKRULES_RUNTYPES);
      hCtlPREMIUMLIST = GetDlgItem(hWndDlg, WORKRULES_PREMIUMLIST);
      hCtlPREMIUMTIME = GetDlgItem(hWndDlg, WORKRULES_PREMIUMTIME);
      hCtlTIMECOUNTSINOT = GetDlgItem(hWndDlg, WORKRULES_TIMECOUNTSINOT);
      hCtlPREMIUMREPORT = GetDlgItem(hWndDlg, WORKRULES_PREMIUMREPORT);
      hCtlTRAVELTIMEPREMIUM= GetDlgItem(hWndDlg, WORKRULES_TRAVELTIMEPREMIUM);
      hCtlUSECONNIFNOSCHED = GetDlgItem(hWndDlg, WORKRULES_USECONNIFNOSCHED);
      hCtlDONTPAY = GetDlgItem(hWndDlg, WORKRULES_DONTPAY);
      hCtlDONTINCORPORATE = GetDlgItem(hWndDlg, WORKRULES_DONTINCORPORATE);
      hCtlIDNEWPREMIUM = GetDlgItem(hWndDlg, IDNEWPREMIUM);
      hCtlIDSETPREMIUM = GetDlgItem(hWndDlg, IDSETPREMIUM);
//
//  Set up the premium report combo box
//
      SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)WORKRULES_PREMIUMREPORT, (LPARAM)0);
//
//  Set the "All" runtypes radio button
//
      SendMessage(hCtlALLRUNTYPES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(WORKRULES_ALLRUNTYPES, 0), (LPARAM)0);
//
//  So we know if he hasn't picked a premium
//
      premiumIndex = NO_RECORD;
//
//  Now display the associated premiums with the selected runtype.  If there
//  are any premiums, the premiumIndex will be reset to the first item in the list box
//
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(WORKRULES_RUNTYPES, CBN_SELCHANGE), (LPARAM)0);
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
//
//  WORKRULES_PREMIUMREPORT
//
        case WORKRULES_PREMIUMREPORT:
          strcpy(TABLEVIEWKey1.Name, TABLENAME_RUNS);
          pad(TABLEVIEWKey1.Name, TABLEVIEW_NAME_LENGTH);
          btrieve(B_GETEQUAL, TMS_TABLEVIEW, &TABLEVIEW, &TABLEVIEWKey1, 1);
          CREATEDKey1.File = TABLEVIEW.Id;
          CREATEDKey1.Sequence = 0;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
          while(rcode2 == 0 &&
                CREATED.File == TABLEVIEW.Id)
          {
            strncpy(tempString, CREATED.Name, CREATED_NAME_LENGTH);
            trim(tempString, CREATED_NAME_LENGTH);
            nI = SendMessage(hCtlPREMIUMREPORT, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            ATTRIBUTESKey0.Id = CREATED.Id;
            rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
            if(rcode2 != 0)
              ATTRIBUTES.formattedDataType = NO_RECORD;
            SendMessage(hCtlPREMIUMREPORT, CB_SETITEMDATA, (WPARAM)nI,
                  MAKELPARAM(ATTRIBUTES.formattedDataType, CREATED.Id));
            rcode2 = btrieve(B_GETNEXT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
          }
          break;
//
//  WORKRULES_RUNTYPES - Set up the combo box
//
        case WORKRULES_RUNTYPES:
          SendMessage(hCtlRUNTYPES, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < NUMRUNTYPES; nI++)
          {
            for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
            {
              if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
              {
                nK = SendMessage(hCtlRUNTYPES, CB_ADDSTRING, (WPARAM)0,
                      (LONG)(LPSTR)RUNTYPE[nI][nJ].localName);
                SendMessage(hCtlRUNTYPES, CB_SETITEMDATA, (WPARAM)nK, MAKELPARAM(nI, nJ));
              }
            }
          }
          break;
//
//  WORKRULES_PREMIUMLIST - Set up the list box based on runtype
//
        case WORKRULES_PREMIUMLIST:
          nI = SendMessage(hCtlRUNTYPES, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          tempLong = (nI == CB_ERR
                ? NO_RECORD : SendMessage(hCtlRUNTYPES, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0));
          SendMessage(hCtlPREMIUMLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < numPremiums; nI++)
          {
            if(PREMIUM[nI].runtype == tempLong)
            {
              nJ = SendMessage(hCtlPREMIUMLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)PREMIUM[nI].localName);
              SendMessage(hCtlPREMIUMLIST, LB_SETITEMDATA, (WPARAM)nJ, MAKELPARAM(0, nI));
            }
          }
          nI = SendMessage(hCtlPREMIUMLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlIDSETPREMIUM, nI > 0);
          if(nI > 0)
          {
            SendMessage(hCtlPREMIUMLIST, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(WORKRULES_PREMIUMLIST, LBN_SELCHANGE), (LPARAM)0);
            nK = SendMessage(hCtlPREMIUMLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            tempLong = SendMessage(hCtlPREMIUMLIST, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
            premiumIndex = HIWORD(tempLong);
          }
          break;
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
//  WORKRULES_TRAVELTIMEPREMIUM check box
//
        case WORKRULES_TRAVELTIMEPREMIUM:
          enable = SendMessage(hCtlTRAVELTIMEPREMIUM, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableTravelSection(hWndDlg, enable);
          if(enable)
          {
            SendDlgItemMessage(hWndDlg, WORKRULES_TIMEENTERED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
            SendDlgItemMessage(hWndDlg, WORKRULES_PAYACTUAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          }
          break;
//
//  WORKRULES_TIMEENTERED and WORKRULES_ASINCONNECTIONS radio buttons
//
        case WORKRULES_TIMEENTERED:
        case WORKRULES_ASINCONNECTIONS:
          EnableWindow(hCtlUSECONNIFNOSCHED, FALSE);
          break;
//
//  WORKRULES_ASPERSCHEDULE and WORKRULES_ASPERSCHEDULEDWELL radio buttons
//
        case WORKRULES_ASPERSCHEDULE:
        case WORKRULES_ASPERSCHEDULEDWELL:
          EnableWindow(hCtlUSECONNIFNOSCHED, TRUE);
          break;
//
//  WORKRULES_ALLRUNTYPES radio button
//  WORKRULES_ONERUNTYPE radio button
//
        case WORKRULES_ALLRUNTYPES:
        case WORKRULES_ONERUNTYPE:
          if(wmId == WORKRULES_ALLRUNTYPES)
          {
            EnableWindow(hCtlRUNTYPES, FALSE);
            SendMessage(hCtlRUNTYPES, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          }
          else
          {
            EnableWindow(hCtlRUNTYPES, TRUE);
            SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)WORKRULES_RUNTYPES, (LPARAM)0);
            if(SendMessage(hCtlRUNTYPES, CB_GETCOUNT, (WPARAM)0, (LPARAM)0) > 0)
              SendMessage(hCtlRUNTYPES, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          }
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(WORKRULES_RUNTYPES, CBN_SELCHANGE), (LPARAM)0);
          break;

//
//  WORKRULES_RUNTYPES list box
//
        case WORKRULES_RUNTYPES:
          switch(wmEvent)
          {
//
//  CBN_SELCHANGE
//
            case CBN_SELCHANGE:
              nI = SendMessage(hCtlRUNTYPES, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR)
              {
                runtypeIndex = NO_RECORD;
                slotNumber = NO_RECORD;
              }
              else
              {
                tempLong = SendMessage(hCtlRUNTYPES, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                runtypeIndex = LOWORD(tempLong);
                slotNumber = HIWORD(tempLong);
              }
              SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)WORKRULES_PREMIUMLIST, (LPARAM)0);
              break;
          }
          break;
//
//  WORKRULES_PREMIUMLIST list box
//
        case WORKRULES_PREMIUMLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              nJ = SendMessage(hCtlPREMIUMLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              enable = (nJ != LB_ERR);
              EnableWindow(hCtlPREMIUMTIME, enable);
              EnableWindow(hCtlTIMECOUNTSINOT, enable);
              EnableWindow(hCtlPREMIUMREPORT, enable);
              EnableWindow(hCtlIDSETPREMIUM, enable);
              EnableWindow(hCtlTRAVELTIMEPREMIUM, enable);
//
//  Display the premium characteristics
//
//  Pay type
//
              if(enable)
              {
                tempLong = SendMessage(hCtlPREMIUMLIST, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                nI = HIWORD(tempLong);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(WORKRULES_PREMIUMTIME, 0), (LPARAM)0);
                SendMessage(hCtlPREMIUMTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(PREMIUM[nI].time));
//
//  Time counts in OT
//
                SendMessage(hCtlTIMECOUNTSINOT, BM_SETCHECK,
                      (WPARAM)(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT), (LPARAM)0);
//
//  Don't pay when paying intervening time
//
                SendMessage(hCtlDONTPAY, BM_SETCHECK,
                      (WPARAM)(PREMIUM[nI].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING),
                      (LPARAM)0);
//
//  Premium reported as...
//
                nK = SendMessage(hCtlPREMIUMREPORT, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                for(nJ = 0; nJ < nK; nJ++)
                {
                  if(PREMIUM[nI].reportedAs ==
                        HIWORD(SendMessage(hCtlPREMIUMREPORT, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0)))
                  {
                    SendMessage(hCtlPREMIUMREPORT, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
                    break;
                  }
                }
//
//  Travel time premiums
//
                if(!(PREMIUM[nI].flags & PREMIUMFLAGS_TRAVELTIMEPREMIUM))
                {
                  SendMessage(hCtlTRAVELTIMEPREMIUM, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                  EnableTravelSection(hWndDlg, FALSE);
                }
                else
                {
                  SendMessage(hCtlTRAVELTIMEPREMIUM, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                  EnableTravelSection(hWndDlg, TRUE);
                  SendDlgItemMessage(hWndDlg, PREMIUM[nI].payTravelTime, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                  if(PREMIUM[nI].payTravelTime == feasibleRG[0] ||
                        PREMIUM[nI].payTravelTime == feasibleRG[1])
                    EnableWindow(hCtlUSECONNIFNOSCHED, FALSE);
                  else if(PREMIUM[nI].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
                    SendMessage(hCtlUSECONNIFNOSCHED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                  SendDlgItemMessage(hWndDlg, PREMIUM[nI].payHow, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                  if(PREMIUM[nI].payHow != WORKRULES_PAYACTUAL)
                  {
                    gcvt(PREMIUM[nI].payHowPercent, NUMDECS, tempString);
                    ltoa(PREMIUM[nI].payHowMinutes, szarString, 10);
                    if(PREMIUM[nI].payHow == WORKRULES_PAYFLAT)
                    {
                      SendDlgItemMessage(hWndDlg, WORKRULES_PAYFLATMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
                    }
                    else if(PREMIUM[nI].payHow == WORKRULES_PAYPERCENTAGE)
                    {
                      SendDlgItemMessage(hWndDlg, WORKRULES_PERCENTOFACTUAL, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
                    }
                    else if(PREMIUM[nI].payHow == WORKRULES_PAYSTAGGERED)
                    {
                      SendDlgItemMessage(hWndDlg, WORKRULES_STAGGEREDPERCENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
                      SendDlgItemMessage(hWndDlg, WORKRULES_STAGGEREDMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
                    }
                  }
                }
//
//  And set up the premium Index
//
                nK = SendMessage(hCtlPREMIUMLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                tempLong = SendMessage(hCtlPREMIUMLIST, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
                premiumIndex = HIWORD(tempLong);
              }
              break;

            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDSETPREMIUM, 0), (LPARAM)0);
              break;
          }
          break;
//
//  IDNEWPREMIUM - Display the PREMIUMS dialog box, and...
//  IDSETPREMIUM - Add a new premium
//
        case IDNEWPREMIUM:
        case IDSETPREMIUM:
          premiumRuntype = runtypeIndex;
          premiumTimePeriod = slotNumber;
          premiumNew = wmId == IDNEWPREMIUM;
          if(!premiumNew)
          {
            nI = SendMessage(hCtlPREMIUMLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            tempLong = SendMessage(hCtlPREMIUMLIST, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            premiumIndex = HIWORD(tempLong);
          }
          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREMIUMS), hWndDlg,
                (DLGPROC)PREMIUMSMsgProc, (LPARAM)pDI);
          if(nRc)
          {
            if(wmId == IDNEWPREMIUM)
            {
              nJ = SendMessage(hCtlRUNTYPES, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              PREMIUM[premiumIndex].runtype =
                    nJ == CB_ERR ? NO_RECORD : SendMessage(hCtlRUNTYPES, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
            }
            SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)WORKRULES_PREMIUMLIST, (LPARAM)0);
          }
          break;
//
//  IDCANCEL Button labelled "Finished"
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Premium_Definition_Step_1);
          break;
//
//  IDOK Button labelled "Update"
//
        case IDOK:
//
//  Premiumtype and amount
//
          if(premiumIndex != NO_RECORD)
          {
            SendMessage(hCtlPREMIUMTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            PREMIUM[premiumIndex].time = thhmm(tempString);
//
//  Minutes count...
//
            if(SendMessage(hCtlTIMECOUNTSINOT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              PREMIUM[premiumIndex].flags |= PREMIUMFLAGS_TIMECOUNTSINOT;
            }
            else
            {
              if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_TIMECOUNTSINOT)
              {
                PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_TIMECOUNTSINOT;
              }
            }
//
//  Don't pay
//
            if(SendMessage(hCtlDONTPAY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              PREMIUM[premiumIndex].flags |= PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING;
            }
            else
            {
              if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING)
              {
                PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING;
              }
            }
//
//  Premium reported as
//
            nI = SendMessage(hCtlPREMIUMREPORT, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_091, hCtlPREMIUMREPORT);
              break;
            }
            tempLong = SendMessage(hCtlPREMIUMREPORT, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            PREMIUM[premiumIndex].reportedAs = HIWORD(tempLong);
            PREMIUM[premiumIndex].formattedDataType = LOWORD(tempLong);
//
//  Travel Time Premium
//
            if(!SendMessage(hCtlTRAVELTIMEPREMIUM, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_TRAVELTIMEPREMIUM)
              {
                PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_TRAVELTIMEPREMIUM;
              }
              if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
              {
                PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
              }
              PREMIUM[premiumIndex].payTravelTime = NO_RECORD;
              PREMIUM[premiumIndex].payHow = NO_RECORD;
              PREMIUM[premiumIndex].payHowMinutes = 0;
              PREMIUM[premiumIndex].payHowPercent = (float)0.0;
            }
            else
            {
              PREMIUM[premiumIndex].flags |= PREMIUMFLAGS_TRAVELTIMEPREMIUM;
              for(nI = 0; nI < 4; nI++)
              {
                if(SendDlgItemMessage(hWndDlg, feasibleRG[nI], BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                {
                  PREMIUM[premiumIndex].payTravelTime = feasibleRG[nI];
                  break;
                }
              }
              if(PREMIUM[premiumIndex].payTravelTime == feasibleRG[0] ||
                    PREMIUM[premiumIndex].payTravelTime == feasibleRG[1])
              {
                if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
                {
                  PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
                }
              }
              else
              {
                if(SendMessage(hCtlUSECONNIFNOSCHED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                {
                  PREMIUM[premiumIndex].flags |= PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
                }
                else if(PREMIUM[premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
                {
                  PREMIUM[premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
                }
              }
              for(nI = 0; nI < 4; nI++)
              {
                if(SendDlgItemMessage(hWndDlg, paidHowRG[nI], BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                {
                  PREMIUM[premiumIndex].payHow = paidHowRG[nI];
                  strcpy(tempString, "0");
                  strcpy(szarString, "0");
                  if(paidHowRG[nI] == WORKRULES_PAYFLAT)
                  {
                    SendDlgItemMessage(hWndDlg, WORKRULES_PAYFLATMINUTES, WM_GETTEXT,
                          (WPARAM)sizeof(szarString), (LONG)(LPSTR)szarString);
                  }
                  else if(paidHowRG[nI] == WORKRULES_PAYPERCENTAGE)
                  {
                    SendDlgItemMessage(hWndDlg, WORKRULES_PERCENTOFACTUAL, WM_GETTEXT,
                          (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
                  }
                  else if(PREMIUM[premiumIndex].payHow == WORKRULES_PAYSTAGGERED)
                  {
                    SendDlgItemMessage(hWndDlg, WORKRULES_STAGGEREDPERCENT, WM_GETTEXT,
                          (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
                    SendDlgItemMessage(hWndDlg, WORKRULES_STAGGEREDMINUTES, WM_GETTEXT,
                          (WPARAM)sizeof(szarString), (LONG)(LPSTR)szarString);
                  }
                  PREMIUM[premiumIndex].payHowMinutes = atol(szarString);
                  PREMIUM[premiumIndex].payHowPercent = (float)atof(tempString);
                  break;
                }
              }
              SendDlgItemMessage(hWndDlg, WORKRULES_STAGGEREDPERCENT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              PREMIUM[premiumIndex].payHowPercent = (float)atof(tempString);
            }
          }
//
//  Point each premium to the appropriate "formattedDataType"
//
          SetupPremiumAttributes();
//
//  Flag the changes to the INI file
//
          m_bWorkrulesChanged = TRUE;
          break;
      }
      break;

    default:
      return FALSE;
   }
   return TRUE;
}

void EnableTravelSection(HWND hWnd, BOOL bEnable)
{
//
//  Enable / disable everything
//
  EnableWindow(GetDlgItem(hWnd, WORKRULES_ACTUAL), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_TRAVELTIMEONLY), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_TIMEENTERED), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_ASINCONNECTIONS), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_ASPERSCHEDULE), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_ASPERSCHEDULEDWELL), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_USECONNIFNOSCHED), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAID), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAYACTUAL), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAYFLAT), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAYFLATMINUTES), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAYPERCENTAGE), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PERCENTOFACTUAL), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_PAYSTAGGERED), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_STAGGEREDPERCENT), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_STATIC1), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_STAGGEREDMINUTES), bEnable);
  EnableWindow(GetDlgItem(hWnd, WORKRULES_STATIC2), bEnable);
//
//  Clear the buttons
//
  SendDlgItemMessage(hWnd, WORKRULES_TIMEENTERED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_ASINCONNECTIONS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_ASPERSCHEDULE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_ASPERSCHEDULEDWELL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_USECONNIFNOSCHED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_PAYACTUAL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_PAYFLAT, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_PAYPERCENTAGE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
  SendDlgItemMessage(hWnd, WORKRULES_PAYSTAGGERED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
//
//  Clear the fields
//
  SendDlgItemMessage(hWnd, WORKRULES_PAYFLATMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
  SendDlgItemMessage(hWnd, WORKRULES_PERCENTOFACTUAL, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
  SendDlgItemMessage(hWnd, WORKRULES_STAGGEREDPERCENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
  SendDlgItemMessage(hWnd, WORKRULES_STAGGEREDMINUTES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");

  return;
}