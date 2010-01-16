//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ADDROUTEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static HANDLE hCtlNUMBER;
 static HANDLE hCtlNAME;
 static HANDLE hCtlBIDIRECTIONAL;
 static HANDLE hCtlCIRCULAR;
 static HANDLE hCtlOUTBOUND;
 static HANDLE hCtlINBOUND;
 static HANDLE hCtlSHUTTLE;
 static HANDLE hCtlSCHOOL;
 static HANDLE hCtlNA;
 static HANDLE hCtlCC;
 static HANDLE hCtlCOMMENTCODE;
 BOOL   bFound;
 char   number[ROUTES_NUMBER_LENGTH + 1];
 char   name[ROUTES_NAME_LENGTH + 1];
 long   commentRecordID;
 int    rcode2;
 int    nI;
 int    nJ;
 int    nK;
 int    numDirections;
 int    numComments;
 int    opCode;
 short  int    wmId;
 short  int    wmEvent;

 switch(Message)
 {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
     hCtlNUMBER = GetDlgItem(hWndDlg, ADDROUTE_NUMBER);
     hCtlNAME = GetDlgItem(hWndDlg, ADDROUTE_NAME);
     hCtlBIDIRECTIONAL = GetDlgItem(hWndDlg, ADDROUTE_BIDIRECTIONAL);
     hCtlCIRCULAR = GetDlgItem(hWndDlg, ADDROUTE_CIRCULAR);
     hCtlOUTBOUND = GetDlgItem(hWndDlg, ADDROUTE_OUTBOUND);
     hCtlINBOUND = GetDlgItem(hWndDlg, ADDROUTE_INBOUND);
     hCtlSHUTTLE = GetDlgItem(hWndDlg, ADDROUTE_SHUTTLE);
     hCtlSCHOOL = GetDlgItem(hWndDlg, ADDROUTE_SCHOOL);
     hCtlNA = GetDlgItem(hWndDlg, ADDROUTE_NA);
     hCtlCC = GetDlgItem(hWndDlg, ADDROUTE_CC);
     hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDROUTE_COMMENTCODE);
//
// Set up diretions - Can't have none
//
     numDirections = SetUpDirectionList(hWndDlg, ADDROUTE_OUTBOUND, ADDROUTE_INBOUND);
     if(numDirections == 0)
     {
       SendMessage(hWndDlg, WM_COMMAND, IDCANCEL, (LPARAM)0);
       break;
     } 
//
//  Comment codes combo box
//
     numComments = SetUpCommentList(hWndDlg, ADDROUTE_COMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCC, FALSE);
        EnableWindow(hCtlCOMMENTCODE, FALSE);
      }
//
// Are we updating?
//
     if(updateRecordID == NO_RECORD)
     {
       SendMessage(hCtlBIDIRECTIONAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       SendMessage(hCtlSHUTTLE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
       SendMessage(hCtlSCHOOL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
       SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     }
     else
     {
//
//  Get the data for the route
//
       ROUTESKey0.recordID = updateRecordID;
       btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
//
//  Number
//
       strncpy(number, ROUTES.number, ROUTES_NUMBER_LENGTH);
       trim(number, ROUTES_NUMBER_LENGTH);
       SendMessage(hCtlNUMBER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)number);
//
//  Name
//
       strncpy(name, ROUTES.name, ROUTES_NAME_LENGTH);
       trim(name, ROUTES_NAME_LENGTH);
       SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)name);
//
//  Direction(s)
//
       for(nI = 0; nI < numDirections; nI++)
       {
         if(SendMessage(hCtlOUTBOUND, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) ==
               ROUTES.DIRECTIONSrecordID[0])
         {
           SendMessage(hCtlOUTBOUND, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
           break;
         }
       }
       if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
       {
         SendMessage(hCtlCIRCULAR, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
         EnableWindow(hCtlINBOUND, FALSE);
       }
       else
       {
         SendMessage(hCtlBIDIRECTIONAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
         EnableWindow(hCtlINBOUND, TRUE);
         for(nI = 0; nI < numDirections; nI++)
         {
           if(SendMessage(hCtlOUTBOUND, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == 
                 ROUTES.DIRECTIONSrecordID[1])
           {
             SendMessage(hCtlINBOUND, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
             break;
           }
         }
       }
//
//  Employee shuttle
//
       SendMessage(hCtlSHUTTLE, BM_SETCHECK,
             (WPARAM)(ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE), (LPARAM)0);
//
//  School route
//
       SendMessage(hCtlSCHOOL, BM_SETCHECK,
             (WPARAM)(ROUTES.flags & ROUTES_FLAG_SCHOOL), (LPARAM)0);
//
//  Comment
//
       if(ROUTES.COMMENTSrecordID == NO_RECORD)
         SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       else
       {
         for(bFound = FALSE, nI = 0; nI < numComments; nI++)
         {
           if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == ROUTES.COMMENTSrecordID)
           {
             SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
             bFound = TRUE;
             break;
           }
         }
         SendMessage(bFound ? hCtlCC : hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       }
     }
     break; //  End of WM_INITDLG
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
        case ADDROUTE_COMMENTCODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlCC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDROUTE_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDROUTE_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

//
//  ADDROUTE_BIDIRECTIONAL
//
       case ADDROUTE_BIDIRECTIONAL:
         EnableWindow(hCtlINBOUND, TRUE);
         break;
//
//  ADDROUTE_CIRCULAR
//
       case ADDROUTE_CIRCULAR:
         SendMessage(hCtlINBOUND, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
         EnableWindow(hCtlINBOUND, FALSE);
         break;
//
//  IDOK
//
       case IDOK:
/*
         nJ = 0;
         if(nJ == 0)
         {
           for(nI = 101; nI < 225; nI++)
           {
             sprintf(number, "%d", nI);
             strcpy(name, "School Route ");
             strcat(name, number);
             rcode2 = btrieve(B_GETLAST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
             ROUTES.recordID = AssignRecID(rcode2, ROUTES.recordID);
             strncpy(ROUTES.number, number, ROUTES_NUMBER_LENGTH);
             pad(ROUTES.number, ROUTES_NUMBER_LENGTH);
             strncpy(ROUTES.name, name, ROUTES_NAME_LENGTH);
             pad(ROUTES.name, ROUTES_NAME_LENGTH);
             ROUTES.DIRECTIONSrecordID[0] = 2;
             ROUTES.DIRECTIONSrecordID[1] = NO_RECORD;
             ROUTES.flags = ROUTES_FLAG_SCHOOL;
             ROUTES.COMMENTSrecordID = NO_RECORD;;
             memset(ROUTES.reserved, 0x00, ROUTES_RESERVED_LENGTH);
             btrieve(B_INSERT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
           }
           EndDialog(hWndDlg, TRUE);
           break;
         }
*/
//
//  Route Number
//
         SendMessage(hCtlNUMBER, WM_GETTEXT, (WPARAM)sizeof(number), (LONG)(LPSTR)number);
         if(strcmp(number, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_009, hCtlNUMBER);
           break;
         }
//
//  Route Name
//
         SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)sizeof(name), (LONG)(LPSTR)name);
         if(strcmp(name, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_012, hCtlNAME);
           break;
         }
//
//  Outbound direction - A direction must have been selected
//
         nI = (int)SendMessage(hCtlOUTBOUND, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
         if(nI == CB_ERR)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_013, hCtlOUTBOUND);
           break;
         }
//
//  Inbound direction - If selected, it can't be the same as outbound
//
         nJ = (int)SendMessage(hCtlINBOUND, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
         if(nJ == nI)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_014, hCtlINBOUND);
           break;
         }
//
//  Comment code
//
         if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           commentRecordID = NO_RECORD;
         else
         {
           nK = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           commentRecordID = (nK == CB_ERR ? NO_RECORD : 
                 SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nK, (LPARAM)0));
         }
//
//  Are we updating or inserting?
//
         if(updateRecordID == NO_RECORD)
         {
           rcode2 = btrieve(B_GETLAST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
           ROUTES.recordID = AssignRecID(rcode2, ROUTES.recordID);
           opCode = B_INSERT;
         }
         else
         {
           ROUTESKey0.recordID = updateRecordID;
           btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
           opCode = B_UPDATE;
         }
//
//  Set up ROUTES
//
         strncpy(ROUTES.number, number, ROUTES_NUMBER_LENGTH);
         pad(ROUTES.number, ROUTES_NUMBER_LENGTH);
         strncpy(ROUTES.name, name, ROUTES_NAME_LENGTH);
         pad(ROUTES.name, ROUTES_NAME_LENGTH);
         ROUTES.DIRECTIONSrecordID[0] = SendMessage(hCtlOUTBOUND, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         ROUTES.DIRECTIONSrecordID[1] = nJ == CB_ERR ? NO_RECORD : SendMessage(hCtlINBOUND, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
//
//  Flags
//
         ROUTES.flags = 0;
         if(SendMessage(hCtlSHUTTLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           ROUTES.flags |= ROUTES_FLAG_EMPSHUTTLE;
         }
         if(SendMessage(hCtlSCHOOL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           ROUTES.flags |= ROUTES_FLAG_SCHOOL;
         }
//
//  Comments
//
         ROUTES.COMMENTSrecordID = commentRecordID;
//
//  Bays
//
         ROUTES.OBBayNODESrecordID = NO_RECORD;
         ROUTES.IBBayNODESrecordID = NO_RECORD;
//
//  Insert / Update
//
         btrieve(opCode, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
//
//  Make adjustments to the global route data variables
//
         if(updateRecordID == m_RouteRecordID)
         {
           strcpy(m_RouteNumber, number);
           strcpy(m_RouteName, name);
         }
         updateRecordID = ROUTES.recordID;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Routes_Table);
          break;
     }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
 }
 return TRUE;
} //  End of ADDROUTEMsgProc
