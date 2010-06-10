//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGETRIPNUMBERSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  typedef struct PASSEDDATAStruct
  {
    long ROUTESrecordID;
    long SERVICESrecordID;
    long directionIndex;
    long *pRecordIDs;
    int  numRecords;
  } PASSEDDATADef;
  static  PASSEDDATADef *pPassedData;
  static  HANDLE hCtlSTARTAT;
  static  HANDLE hCtlINCREMENT;
  HCURSOR hSaveCursor;
  long  newTripNumber;
  long  increment;
  long  testTripNumber;
  char  szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
  char  szServiceName[SERVICES_NAME_LENGTH + 1];
  char  szDirectionAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  BOOL  bContinue;
  int   nI;
  int   rcode2;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pPassedData = (PASSEDDATADef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      hCtlSTARTAT = GetDlgItem(hWndDlg, TRIPNUMBERS_STARTAT);
      hCtlINCREMENT = GetDlgItem(hWndDlg, TRIPNUMBERS_INCREMENT);
      SendMessage(hCtlSTARTAT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
      SendMessage(hCtlINCREMENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
      break; //  End of WM_INITDLG
//
//  WM_CLOSE
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
        case IDOK:
//
// The new trip number can't be blank, or less than or equal to zero
//
          GetDlgItemText(hWndDlg, TRIPNUMBERS_STARTAT, tempString, TEMPSTRING_LENGTH);
          if(strcmp(tempString, "") == 0 || (newTripNumber = atol(tempString)) <= 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_374, hCtlSTARTAT);
            break;
          }
//
//  If the increment is blank, zero, or less than zero, set it to 1
//
          GetDlgItemText(hWndDlg, TRIPNUMBERS_INCREMENT, szarString, SZARSTRING_LENGTH);
          if(strcmp(szarString, "") == 0 || (increment = atol(szarString)) <= 0)
          {
            increment = 1;
          }
//
//  Fake a renumber to see if there is going to be a duplicate
//
          bContinue = TRUE;
          rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          while(rcode2 == 0)
          {
            if(TRIPS.tripNumber != NO_RECORD)
            {
              testTripNumber = newTripNumber;
              for(nI = 0; nI < pPassedData->numRecords; nI++)
              {
                if(TRIPS.tripNumber == testTripNumber)
                {
                  ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
                  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                  strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
                  trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
                  DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
                  btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
                  strncpy(szDirectionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
                  trim(szDirectionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
                  SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
                  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                  strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
                  trim(szServiceName, SERVICES_NAME_LENGTH);
                  sprintf(tempString, "Trip number %ld already exists on\nRoute %s, Service %s, Direction %s",
                        TRIPS.tripNumber, szRouteNumber, szServiceName, szDirectionAbbr);
                  MessageBeep(MB_ICONSTOP);
                  MessageBox(hWndDlg, tempString, TMS, MB_ICONSTOP | MB_OK);
                  MessageBeep(MB_ICONINFORMATION);
                  MessageBox(hWndDlg, "No renumbering took place", TMS, MB_OK);
                  bContinue = FALSE;
                  break;
                }
                testTripNumber += increment;
              }
              if(!bContinue)
              {
                break;
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          }
          if(!bContinue)
          {
            break;
          }
//
//  Do the renumber
//
          hSaveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < pPassedData->numRecords; nI++)
          {
            TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(rcode2 == 0)
            {
              TRIPS.tripNumber = newTripNumber;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              newTripNumber += increment;
            }
          }
          SetCursor(hSaveCursor);
          EndDialog(hWndDlg, TRUE);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Multiple_Modifications_on_the_Trips_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of CHANGETRIPNUMBERSMsgProc
