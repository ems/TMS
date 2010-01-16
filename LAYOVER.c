//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#ifdef  MINIMUMBUTTONS
#undef  MINIMUMBUTTONS
#endif
#define MINIMUMBUTTONS 3
#ifdef  MAXIMUMBUTTONS
#undef  MAXIMUMBUTTONS
#endif
#define MAXIMUMBUTTONS 3
#define BOTHBUTTONS    2

BOOL CALLBACK LAYOVERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static HANDLE hCtlWHICH;
 static HANDLE hCtlMINMINUTESTITLE;
 static HANDLE hCtlMINMINUTES;
 static HANDLE hCtlMINPERCENTTITLE;
 static HANDLE hCtlMINPERCENT;
 static HANDLE hCtlMINBOTHTITLE;
 static HANDLE hCtlMINBOTHMINUTES;
 static HANDLE hCtlMINBOTHPERCENTTITLE;
 static HANDLE hCtlMINBOTHPERCENT;
 static HANDLE hCtlMINBOTHWHICHEVERTITLE;
 static HANDLE hCtlMINBOTHLOWER;
 static HANDLE hCtlMINBOTHHIGHER;
 static int    minimumRG[MINIMUMBUTTONS];
 static int    minimumBothRG[BOTHBUTTONS];
 static int    minimumRGSelection;
 static int    minimumBothRGSelection;
 static HANDLE hCtlMAXMINUTESTITLE;
 static HANDLE hCtlMAXMINUTES;
 static HANDLE hCtlMAXPERCENTTITLE;
 static HANDLE hCtlMAXPERCENT;
 static HANDLE hCtlMAXBOTHTITLE;
 static HANDLE hCtlMAXBOTHMINUTES;
 static HANDLE hCtlMAXBOTHPERCENTTITLE;
 static HANDLE hCtlMAXBOTHPERCENT;
 static HANDLE hCtlMAXBOTHWHICHEVERTITLE;
 static HANDLE hCtlMAXBOTHLOWER;
 static HANDLE hCtlMAXBOTHHIGHER;
 static int    maximumRG[MAXIMUMBUTTONS];
 static int    maximumBothRG[BOTHBUTTONS];
 static int    maximumRGSelection;
 static int    maximumBothRGSelection;
 static long   which;
 static LAYOVERDef LocalLayover;
 HANDLE hCtl;
 long   layoverTime;
 int    nI;
 int    nJ;
 int    nK;
 int    nL;
 short int wmId;

 switch(Message)
 {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Determine which we're doing - standard or dropback
//
     which = (long)lParam;
//
//  Assign handles to the controls
//
     hCtlWHICH = GetDlgItem(hWndDlg, LAYOVER_WHICH);
     hCtlMINMINUTESTITLE = GetDlgItem(hWndDlg, LAYOVER_MINMINUTESTITLE);
     hCtlMINMINUTES = GetDlgItem(hWndDlg, LAYOVER_MINMINUTES);
     hCtlMINPERCENTTITLE = GetDlgItem(hWndDlg, LAYOVER_MINPERCENTTITLE);
     hCtlMINPERCENT = GetDlgItem(hWndDlg, LAYOVER_MINPERCENT);
     hCtlMINBOTHTITLE = GetDlgItem(hWndDlg, LAYOVER_MINBOTHTITLE);
     hCtlMINBOTHMINUTES = GetDlgItem(hWndDlg, LAYOVER_MINBOTHMINUTES);
     hCtlMINBOTHPERCENTTITLE = GetDlgItem(hWndDlg, LAYOVER_MINBOTHPERCENTTITLE);
     hCtlMINBOTHPERCENT = GetDlgItem(hWndDlg, LAYOVER_MINBOTHPERCENT);
     hCtlMINBOTHWHICHEVERTITLE = GetDlgItem(hWndDlg, LAYOVER_MINBOTHWHICHEVERTITLE);
     hCtlMINBOTHLOWER = GetDlgItem(hWndDlg, LAYOVER_MINBOTHLOWER);
     hCtlMINBOTHHIGHER = GetDlgItem(hWndDlg, LAYOVER_MINBOTHHIGHER);
     hCtlMAXMINUTESTITLE = GetDlgItem(hWndDlg, LAYOVER_MAXMINUTESTITLE);
     hCtlMAXMINUTES = GetDlgItem(hWndDlg, LAYOVER_MAXMINUTES);
     hCtlMAXPERCENTTITLE = GetDlgItem(hWndDlg, LAYOVER_MAXPERCENTTITLE);
     hCtlMAXPERCENT = GetDlgItem(hWndDlg, LAYOVER_MAXPERCENT);
     hCtlMAXBOTHTITLE = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHTITLE);
     hCtlMAXBOTHMINUTES = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHMINUTES);
     hCtlMAXBOTHPERCENTTITLE = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHPERCENTTITLE);
     hCtlMAXBOTHPERCENT = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHPERCENT);
     hCtlMAXBOTHWHICHEVERTITLE = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHWHICHEVERTITLE);
     hCtlMAXBOTHLOWER = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHLOWER);
     hCtlMAXBOTHHIGHER = GetDlgItem(hWndDlg, LAYOVER_MAXBOTHHIGHER);
//
//  Display the type of layover we're here for
//
     if(which == TMS_STANDARDBLOCKS)
     {
       LoadString(hInst, TEXT_241, tempString, TEMPSTRING_LENGTH);
       memcpy(&LocalLayover, &StandardLayover, sizeof(LAYOVERDef));
     }
     else
     {
       LoadString(hInst, TEXT_242, tempString, TEMPSTRING_LENGTH);
       memcpy(&LocalLayover, &DropbackLayover, sizeof(LAYOVERDef));
     }
     SendMessage(hCtlWHICH, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Set up the radio groups
//
//  LAYOVER_MINIMUM
//
     minimumRG[0] = LAYOVER_MINMINUTESTITLE;
     minimumRG[1] = LAYOVER_MINPERCENTTITLE;
     minimumRG[2] = LAYOVER_MINBOTHTITLE;
     minimumBothRG[0] = LAYOVER_MINBOTHLOWER;
     minimumBothRG[1] = LAYOVER_MINBOTHHIGHER;
//
//  LAYOVER_MAXIMUM
//
     maximumRG[0] = LAYOVER_MAXMINUTESTITLE;
     maximumRG[1] = LAYOVER_MAXPERCENTTITLE;
     maximumRG[2] = LAYOVER_MAXBOTHTITLE;
     maximumBothRG[0] = LAYOVER_MAXBOTHLOWER;
     maximumBothRG[1] = LAYOVER_MAXBOTHHIGHER;
//
//  Set up the "Minimum" side
//
     SendMessage(hCtlMINMINUTES, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.minimum.minutes, tempString, 10));
     SendMessage(hCtlMINPERCENT, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.minimum.percent, tempString, 10));
     SendMessage(hCtlMINBOTHMINUTES, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.minimum.minutes, tempString, 10));
     SendMessage(hCtlMINBOTHPERCENT, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.minimum.percent, tempString, 10));
//
//  Set up the "Maximum" side
//
     SendMessage(hCtlMAXMINUTES, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.maximum.minutes, tempString, 10));
     SendMessage(hCtlMAXPERCENT, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.maximum.percent, tempString, 10));
     SendMessage(hCtlMAXBOTHMINUTES, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.maximum.minutes, tempString, 10));
     SendMessage(hCtlMAXBOTHPERCENT, WM_SETTEXT, (WPARAM)0,
           (LONG)(LPSTR)ltoa(LocalLayover.maximum.percent, tempString, 10));
//
//  And make the radio button selection based on LocalLayover.*.type
//
     minimumRGSelection = LocalLayover.minimum.type == NO_RECORD ? 0 : LocalLayover.minimum.type;
     minimumBothRGSelection = LocalLayover.minimum.lower ? 0 : 1;
     maximumRGSelection = LocalLayover.maximum.type == NO_RECORD ? 0 : LocalLayover.maximum.type;
     maximumBothRGSelection = LocalLayover.maximum.lower ? 0 : 1;
     nI = (minimumRGSelection == 0 ? LAYOVER_MINMINUTESTITLE :
           minimumRGSelection == 1 ? LAYOVER_MINPERCENTTITLE : LAYOVER_MINBOTHTITLE);
     nJ = LocalLayover.minimum.lower ? LAYOVER_MINBOTHLOWER : LAYOVER_MINBOTHHIGHER;
     nK = (maximumRGSelection == 0 ? LAYOVER_MAXMINUTESTITLE :
           maximumRGSelection == 1 ? LAYOVER_MAXPERCENTTITLE : LAYOVER_MAXBOTHTITLE);
     nL = LocalLayover.maximum.lower ? LAYOVER_MAXBOTHLOWER : LAYOVER_MAXBOTHHIGHER;
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(nI, 0), (LPARAM)0);
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(nJ, 0), (LPARAM)0);
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(nK, 0), (LPARAM)0);
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(nL, 0), (LPARAM)0);
     break; //  End of WM_INITDLG
//
//  WM_CLOSE - Same as cancel
//
   case WM_CLOSE:
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
     break; //  End of WM_CLOSE
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  LAYOVER_MINMINUTESTITLE
//  LAYOVER_MINPERCENTTITLE
//  LAYOVER_MINBOTHTITLE Radio Buttons
//
       case LAYOVER_MINMINUTESTITLE:
       case LAYOVER_MINPERCENTTITLE:
       case LAYOVER_MINBOTHTITLE:
         minimumRGSelection = nCwCheckRadioButtons(hWndDlg, minimumRG, wmId, MINIMUMBUTTONS);
         EnableWindow(hCtlMINMINUTES, minimumRGSelection == 0);
         EnableWindow(hCtlMINPERCENT, minimumRGSelection == 1);
         EnableWindow(hCtlMINBOTHMINUTES, minimumRGSelection == 2);
         EnableWindow(hCtlMINBOTHPERCENTTITLE, minimumRGSelection == 2);
         EnableWindow(hCtlMINBOTHPERCENT, minimumRGSelection == 2);
         EnableWindow(hCtlMINBOTHWHICHEVERTITLE, minimumRGSelection == 2);
         EnableWindow(hCtlMINBOTHLOWER, minimumRGSelection == 2);
         EnableWindow(hCtlMINBOTHHIGHER, minimumRGSelection == 2);
         if(minimumRGSelection == 0)
           SetFocus(hCtlMINMINUTES);
         else if(minimumRGSelection == 1)
           SetFocus(hCtlMINPERCENT);
         else
         {
           minimumBothRGSelection = LAYOVER_MINBOTHLOWER;
           SetFocus(hCtlMINBOTHMINUTES);
           nCwCheckRadioButtons(hWndDlg, minimumBothRG, minimumBothRGSelection, BOTHBUTTONS);
         }
         break;
//
//  LAYOVER_MINBOTHLOWER
//  LAYOVER_MINBOTHHIGHER Radio Buttons
//
       case LAYOVER_MINBOTHLOWER:
       case LAYOVER_MINBOTHHIGHER:
         minimumBothRGSelection = nCwCheckRadioButtons(hWndDlg, minimumBothRG, wmId, BOTHBUTTONS);
         break;
//
//  LAYOVER_MAXMINUTESTITLE
//  LAYOVER_MAXPERCENTTITLE
//  LAYOVER_MAXBOTHTITLE Radio Buttons
//
       case LAYOVER_MAXMINUTESTITLE:
       case LAYOVER_MAXPERCENTTITLE:
       case LAYOVER_MAXBOTHTITLE:
         maximumRGSelection = nCwCheckRadioButtons(hWndDlg, maximumRG, wmId, MAXIMUMBUTTONS);
         EnableWindow(hCtlMAXMINUTES, maximumRGSelection == 0);
         EnableWindow(hCtlMAXPERCENT, maximumRGSelection == 1);
         EnableWindow(hCtlMAXBOTHMINUTES, maximumRGSelection == 2);
         EnableWindow(hCtlMAXBOTHPERCENTTITLE, maximumRGSelection == 2);
         EnableWindow(hCtlMAXBOTHPERCENT, maximumRGSelection == 2);
         EnableWindow(hCtlMAXBOTHWHICHEVERTITLE, maximumRGSelection == 2);
         EnableWindow(hCtlMAXBOTHLOWER, maximumRGSelection == 2);
         EnableWindow(hCtlMAXBOTHHIGHER, maximumRGSelection == 2);
         if(maximumRGSelection == 0)
           SetFocus(hCtlMAXMINUTES);
         else if(maximumRGSelection == 1)
           SetFocus(hCtlMAXPERCENT);
         else
         {
           maximumBothRGSelection = LAYOVER_MAXBOTHLOWER;
           SetFocus(hCtlMAXBOTHMINUTES);
           nCwCheckRadioButtons(hWndDlg, maximumBothRG, maximumBothRGSelection, BOTHBUTTONS);
         }
         break;
//
//  LAYOVER_MAXBOTHLOWER
//  LAYOVER_MAXBOTHHIGHER
//
       case LAYOVER_MAXBOTHLOWER:
       case LAYOVER_MAXBOTHHIGHER:
         maximumBothRGSelection = nCwCheckRadioButtons(hWndDlg, maximumBothRG, wmId, BOTHBUTTONS);
         break;
//
//  IDOK
//
       case IDOK:
//
//  Get the "Minimum" side
//
//  "Time in Minutes" and "Percentage of Trip Time"
//
         if(minimumRGSelection == 0 || minimumRGSelection == 1)
         {
           hCtl = minimumRGSelection == 0 ? hCtlMINMINUTES : hCtlMINPERCENT;
           SendMessage(hCtl, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_016, hCtl);
             break;
           }
           layoverTime = atol(tempString);
           LocalLayover.minimum.type = minimumRGSelection;
           if(minimumRGSelection == 0)
             LocalLayover.minimum.minutes = layoverTime;
           else
             LocalLayover.minimum.percent = layoverTime;
         }
//
//  "Minutes or Percentage"
//
         else
         {
           SendMessage(hCtlMINBOTHMINUTES, WM_GETTEXT, (WPARAM) TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_016, hCtlMINBOTHMINUTES);
             break;
           }
           layoverTime = atol(tempString);
           if(layoverTime < 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_033, hCtlMINBOTHMINUTES);
             break;
           }
           SendMessage(hCtlMINBOTHPERCENT, WM_GETTEXT, (WPARAM) TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_016, hCtlMINBOTHPERCENT);
             break;
           }
           if(atol(tempString) < 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_033, hCtlMINBOTHPERCENT);
             break;
           }
           LocalLayover.minimum.type = 2;
           LocalLayover.minimum.minutes = layoverTime;
           LocalLayover.minimum.percent = atol(tempString);
           LocalLayover.minimum.lower = minimumBothRGSelection == 0;
         }
//
//  Get the "Maximum" side
//
//  "Time in Minutes" and "Percentage of Trip Time"
//
         if(maximumRGSelection == 0 || maximumRGSelection == 1)
         {
           hCtl = maximumRGSelection == 0 ? hCtlMAXMINUTES : hCtlMAXPERCENT;
           SendMessage(hCtl, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_017, hCtl);
             break;
           }
           layoverTime = atol(tempString);
           if(layoverTime < 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_034, hCtl);
             break;
           }
           LocalLayover.maximum.type = maximumRGSelection;
           if(maximumRGSelection == 0)
             LocalLayover.maximum.minutes = layoverTime;
           else
             LocalLayover.maximum.percent = layoverTime;
         }
//
//  "Minutes or Percentage"
//
         else
         {
           SendMessage(hCtlMAXBOTHMINUTES, WM_GETTEXT, (WPARAM) TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_017, hCtlMAXBOTHMINUTES);
             break;
           }
           layoverTime = atol(tempString);
           if(layoverTime < 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_034, hCtlMAXBOTHMINUTES);
             break;
           }
           SendMessage(hCtlMAXBOTHPERCENT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
           if(strcmp(tempString, "") == 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_016, hCtlMAXBOTHPERCENT);
             break;
           }
           if(atol(tempString) < 0)
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_033, hCtlMAXBOTHPERCENT);
             break;
           }
           LocalLayover.maximum.type = 2;
           LocalLayover.maximum.minutes = layoverTime;
           LocalLayover.maximum.percent = atol(tempString);
           LocalLayover.maximum.lower = maximumBothRGSelection == 0;
         }
//
//  Ok, passed all the tests - write out the results
//
//  Standard layover
//
         if(which == TMS_STANDARDBLOCKS)
         {
           memcpy(&StandardLayover, &LocalLayover, sizeof(LAYOVERDef));
           ltoa(StandardLayover.minimum.type, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMinType",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(StandardLayover.maximum.type, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMaxType",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(StandardLayover.minimum.minutes, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMinMinutes",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(StandardLayover.maximum.minutes, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMaxMinutes",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(StandardLayover.minimum.percent, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMinPercent",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(StandardLayover.maximum.percent, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "layoverMaxPercent",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           strcpy(tempString, StandardLayover.minimum.lower ? szTRUE : szFALSE);
           WritePrivateProfileString((LPSTR)userName, "layoverMinLower",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           strcpy(tempString, StandardLayover.maximum.lower ? szTRUE : szFALSE);
           WritePrivateProfileString((LPSTR)userName, "layoverMaxLower",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
         }
//
//  Dropback
//
         else
         {
           memcpy(&DropbackLayover, &LocalLayover, sizeof(LAYOVERDef));
           ltoa(DropbackLayover.minimum.type, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMinType",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(DropbackLayover.maximum.type, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMaxType",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(DropbackLayover.minimum.minutes, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMinMinutes",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(DropbackLayover.maximum.minutes, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMaxMinutes",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(DropbackLayover.minimum.percent, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMinPercent",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           ltoa(DropbackLayover.maximum.percent, tempString, 10);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMaxPercent",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           strcpy(tempString, DropbackLayover.minimum.lower ? szTRUE : szFALSE);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMinLower",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
           strcpy(tempString, DropbackLayover.maximum.lower ? szTRUE : szFALSE);
           WritePrivateProfileString((LPSTR)userName, "DropbackLayoverMaxLower",
                 (LPSTR)tempString, (LPSTR)szDatabaseFileName);
         }
//
//  All done
//
         EndDialog(hWndDlg, TRUE);
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
          WinHelp(hWndDlg, szarHelpFile,
                HELP_CONTEXT, Setting_the_Default_Layover_Times_on_the_Standard_Blocks_Table);
          break;
     }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
 }
 return TRUE;
} //  End of LAYOVERMsgProc
