//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#ifdef  MAXROSTERS
#undef  MAXROSTERS
#endif
#define MAXROSTERS  1000

BOOL CALLBACK RENUMROSTERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlALLROSTERS;
  static  HANDLE hCtlSPECIFICROSTER;
  static  HANDLE hCtlROSTERLIST;
  static  HANDLE hCtlSTARTAT;
  static  HANDLE hCtlINCREMENT;
  long    newRosterNumber;
  long    increment;
  int     selectedRosters[MAXROSTERS];
  int     numSelectedRosters;
  int     numRosters;
  int     nI;
  int     rcode2;
  HCURSOR saveCursor;
  short   int wmId;
  short   int wmEvent;

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
//  Set up handles to the controls
//
      hCtlALLROSTERS = GetDlgItem(hWndDlg, RENUMROSTER_ALLROSTERS);
      hCtlSPECIFICROSTER = GetDlgItem(hWndDlg, RENUMROSTER_SPECIFICROSTER);
      hCtlROSTERLIST = GetDlgItem(hWndDlg, RENUMROSTER_ROSTERLIST);
      hCtlSTARTAT = GetDlgItem(hWndDlg, RENUMROSTER_STARTAT);
      hCtlINCREMENT = GetDlgItem(hWndDlg, RENUMROSTER_INCREMENT);
//
//  Set up the Roster list
//
      numRosters = 0;
      ROSTERKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
      ROSTERKey1.rosterNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      while(rcode2 == 0 &&
            ROSTER.DIVISIONSrecordID == pDI->fileInfo.divisionRecordID)
      {
        sprintf(tempString, "%6ld", ROSTER.rosterNumber);
        nI = SendMessage(hCtlROSTERLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlROSTERLIST, LB_SETITEMDATA, (WPARAM)nI, ROSTER.recordID);
        numRosters++;
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      }
      if(numRosters == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_234, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      else
      {
        SendMessage(hCtlALLROSTERS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Set the starting number and increment to 1
//
      SendMessage(hCtlSTARTAT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"1");
      SendMessage(hCtlINCREMENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"1");
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
//  Comments
//
        case RENUMROSTER_ROSTERLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLROSTERS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLROSTERS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICROSTER, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case RENUMROSTER_ALLROSTERS:
          SendMessage(hCtlROSTERLIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;

        case RENUMROSTER_SPECIFICROSTER:
          SendMessage(hCtlROSTERLIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)0);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Renumbering_the_Roster);
          break;
//
//  IDOK
//
        case IDOK:
//
// The new Roster number can't be blank
//
          SendMessage(hCtlSTARTAT, WM_GETTEXT, TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0 || (newRosterNumber = atol(tempString)) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_061, hCtlSTARTAT);
            break;
          }
//
//  If the increment is blank, zero, or less than zero, set it to 1
//
          SendMessage(hCtlINCREMENT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0 || (increment = atol(tempString)) <= 0)
            increment = 1;
//
//  If specific Roster is selected, at least one of the entries
//  in the Rosters list box has to be selected
//
          if(SendMessage(hCtlSPECIFICROSTER, BM_GETCHECK, (WPARAM)0, (LPARAM)0) &&
                SendMessage(hCtlROSTERLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_120, hCtlROSTERLIST);
            break;
          }
//
//  If none are highlighted, highlight 'em all.
//
          if(SendMessage(hCtlROSTERLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
            SendMessage(hCtlROSTERLIST, LB_SETSEL, TRUE, MAKELPARAM(-1, 0));
          numSelectedRosters = SendMessage(hCtlROSTERLIST, LB_GETSELITEMS,
                (WPARAM)MAXROSTERS, (LPARAM)(int FAR *)selectedRosters);
//
//  Loop through the Rosters and make the new assignment
//
		  saveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < numSelectedRosters; nI++)
          {
            newRosterNumber += (nI == 0 ? 0 : increment);
//
//  If a proposed Roster number exists, try the next one
//
            ROSTERKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
            ROSTERKey1.rosterNumber = newRosterNumber;
            rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
            while(rcode2 == 0)
            {
              newRosterNumber += increment;
              ROSTERKey1.DIVISIONSrecordID = pDI->fileInfo.divisionRecordID;
              ROSTERKey1.rosterNumber = newRosterNumber;
              rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
            }
//
//  Ok, we know here that we have a unique Roster number
//
            ROSTERKey0.recordID = SendMessage(hCtlROSTERLIST, LB_GETITEMDATA,
                  (WPARAM)selectedRosters[nI], (LPARAM)0);
            rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
            if(rcode2 == 0)
            {
              ROSTER.rosterNumber = newRosterNumber;
              btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
            }
          }
          SetCursor(saveCursor);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}