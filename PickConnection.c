//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#undef  BUTTONMAX
#define BUTTONMAX 2

BOOL CALLBACK PICKCONNECTIONMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlALL;
 static  HANDLE hCtlSPECIFIC;
 static  HANDLE hCtlROUTETITLE;
 static  HANDLE hCtlROUTE;
 static  HANDLE hCtlSERVICETITLE;
 static  HANDLE hCtlSERVICE;
 static  HANDLE hCtlRUNNINGTIMES;
 static  HANDLE hCtlTRAVELTIMES;
 static  HANDLE hCtlDEADHEADTIMES;
 static  HANDLE hCtlEQUIV;
 static  HANDLE hCtlSTOPSTOP;
 static  HANDLE hCtlLIST;
 static  HANDLE hCtlADDTODISPLAY;
 static  HANDLE hCtlREPLACEDISPLAY;
 static  HANDLE hCtlEVERYTHING;
 static  HANDLE hCtlJUST;
 static  HANDLE hCtlIDTAKEALL;
 static  long   selectedRouteRecordID;
 static  long   selectedServiceRecordID;
 static  int    criteriaRG[BUTTONMAX];
 static  int    criteriaSelection;
 static  int    optionsRG[BUTTONMAX];
 static  int    optionsSelection;
 static  int    displayRG[BUTTONMAX];
 static  int    displaySelection;
 static  int    maxConnections;
 static  int    *pSelections = NULL;
 static  long   *pFromNODESrecordIDs = NULL;
 static  long   *pToNODESrecordIDs = NULL;
 int     numSelected;
 int     numConnections;
 int     nJ;
 int     nI;
 int     rcode2;
 long    fromNodeRecordID;
 long    toNodeRecordID;
 char    fromAbbrName[NODES_ABBRNAME_LENGTH + 1];
 char    toAbbrName[NODES_ABBRNAME_LENGTH + 1];
 long    previousFromNodeRecordID;
 long    previousToNodeRecordID;
 long    previousPATTERNNAMESrecordID;
 HCURSOR saveCursor;
 BOOL    theSamePair;
 BOOL    found;
 BOOL    enable;
 BOOL    bBreakLoop;
 BOOL    bGetStops;
 char    dummy[256];
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
      selectedRouteRecordID = pDI->fileInfo.routeRecordID;
      selectedServiceRecordID = pDI->fileInfo.serviceRecordID;
//
//  Allocate space for the arrays
//
     rcode2 = btrieve(B_STAT, TMS_CONNECTIONS, &BSTAT, dummy, 0);
     if(rcode2 != 0 || BSTAT.numRecords == 0)
     {
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }

     maxConnections = BSTAT.numRecords + 100;  // Add some space in case someone else is adding too
     pFromNODESrecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxConnections); 
     if(pFromNODESrecordIDs == NULL)
     {
       AllocationError(__FILE__, __LINE__, FALSE);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }

     pToNODESrecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxConnections); 
     if(pToNODESrecordIDs == NULL)
     {
       AllocationError(__FILE__, __LINE__, FALSE);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
//
//  Set up the handles to the controls
//
     hCtlALL = GetDlgItem(hWndDlg, PICKCONNECTION_ALL);
     hCtlSPECIFIC = GetDlgItem(hWndDlg, PICKCONNECTION_SPECIFIC);
     hCtlROUTETITLE = GetDlgItem(hWndDlg, PICKCONNECTION_ROUTETITLE);
     hCtlROUTE = GetDlgItem(hWndDlg, PICKCONNECTION_ROUTE);
     hCtlSERVICETITLE = GetDlgItem(hWndDlg, PICKCONNECTION_SERVICETITLE);
     hCtlSERVICE = GetDlgItem(hWndDlg, PICKCONNECTION_SERVICE);
     hCtlRUNNINGTIMES = GetDlgItem(hWndDlg, PICKCONNECTION_RUNNINGTIMES);
     hCtlTRAVELTIMES = GetDlgItem(hWndDlg, PICKCONNECTION_TRAVELTIMES);
     hCtlDEADHEADTIMES = GetDlgItem(hWndDlg, PICKCONNECTION_DEADHEADTIMES);
     hCtlEQUIV = GetDlgItem(hWndDlg, PICKCONNECTION_EQUIV);
     hCtlSTOPSTOP = GetDlgItem(hWndDlg, PICKCONNECTION_STOPSTOP);
     hCtlLIST = GetDlgItem(hWndDlg, PICKCONNECTION_LIST);
     hCtlADDTODISPLAY = GetDlgItem(hWndDlg, PICKCONNECTION_ADDTODISPLAY);
     hCtlREPLACEDISPLAY = GetDlgItem(hWndDlg, PICKCONNECTION_REPLACEDISPLAY);
     hCtlEVERYTHING = GetDlgItem(hWndDlg, PICKCONNECTION_EVERYTHING);
     hCtlJUST = GetDlgItem(hWndDlg, PICKCONNECTION_JUST);
     hCtlIDTAKEALL = GetDlgItem(hWndDlg, IDTAKEALL);
//
//  Set up the display radio group
//
     displayRG[0] = PICKCONNECTION_EVERYTHING;
     displayRG[1] = PICKCONNECTION_JUST;
     displaySelection = PICKCONNECTION_EVERYTHING;
     SendMessage(hCtlEVERYTHING, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(PICKCONNECTION_EVERYTHING, 0), (LPARAM)0);
//
//  Set up the Criteria radio group
//
     criteriaRG[0] = PICKCONNECTION_ALL;
     criteriaRG[1] = PICKCONNECTION_SPECIFIC;
     criteriaSelection = PICKCONNECTION_SPECIFIC;
//
//  Set up the Options radio group
//
     optionsRG[0] = PICKCONNECTION_REPLACEDISPLAY;
     optionsRG[1] = PICKCONNECTION_ADDTODISPLAY;
     optionsSelection = PICKCONNECTION_REPLACEDISPLAY;
     nCwCheckRadioButtons(hWndDlg, optionsRG, optionsSelection, BUTTONMAX);
//
//  Set up the display check boxes
//
     SendMessage(hCtlRUNNINGTIMES, BM_SETCHECK,
           (WPARAM)(CONN.flags & CONNECTIONS_FLAG_RUNNINGTIME), (LPARAM)0);
     SendMessage(hCtlTRAVELTIMES, BM_SETCHECK,
           (WPARAM)(CONN.flags & CONNECTIONS_FLAG_TRAVELTIME), (LPARAM)0);
     SendMessage(hCtlDEADHEADTIMES, BM_SETCHECK,
           (WPARAM)(CONN.flags & CONNECTIONS_FLAG_DEADHEADTIME), (LPARAM)0);
     SendMessage(hCtlEQUIV, BM_SETCHECK,
           (WPARAM)(CONN.flags & CONNECTIONS_FLAG_EQUIVALENT), (LPARAM)0);
     SendMessage(hCtlSTOPSTOP, BM_SETCHECK,
           (WPARAM)(CONN.flags & CONNECTIONS_FLAG_STOPSTOP), (LPARAM)0);
//
//  Let's see what we've got
//
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(PICKCONNECTION_SPECIFIC, 0), (LPARAM)0);
     break; //  End of WM_INITDLG
//
//  WM_CLOSE
//
   case WM_CLOSE:
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
     break; //  End of WM_CLOSE
//
//  WM_USERSETUP
//
   case WM_USERSETUP:
     switch(wParam)
     {
//
//  Nodes on the route/service
//
       case PICKCONNECTION_SPECIFIC:
//
//  Route
//
         nI = SendMessage(hCtlROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
         if(nI == CB_ERR)
           break;
         selectedRouteRecordID = SendMessage(hCtlROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Service
//
         nI = SendMessage(hCtlSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
         if(nI == CB_ERR)
           break;
         selectedServiceRecordID = SendMessage(hCtlSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Stops?
//
         bGetStops = SendMessage(hCtlSTOPSTOP, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  Get the base pattern nodes by direction
//
         ROUTESKey0.recordID = selectedRouteRecordID;
         btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
         numConnections = 0;
         for(bBreakLoop = FALSE, nJ = 0; nJ < 2; nJ++)
         {
           if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
             continue;
           PATTERNSKey2.ROUTESrecordID = selectedRouteRecordID;
           PATTERNSKey2.SERVICESrecordID = selectedServiceRecordID;
           PATTERNSKey2.directionIndex = nJ;
           PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
           PATTERNSKey2.nodeSequence = 0;
           rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  Set up unique from/to node combinations.  This section of code
//  assumes that ADDPATTERN has done its job and not permitted a
//  bus stop to be the first or last node in a pattern.
//
           if(rcode2 == 0 &&
                 PATTERNS.ROUTESrecordID == selectedRouteRecordID &&
                 PATTERNS.SERVICESrecordID == selectedServiceRecordID &&
                 PATTERNS.directionIndex == nJ)
           {
             fromNodeRecordID = PATTERNS.NODESrecordID;
             rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             while(rcode2 == 0 &&
                   PATTERNS.ROUTESrecordID == selectedRouteRecordID &&
                   PATTERNS.SERVICESrecordID == selectedServiceRecordID &&
                   PATTERNS.directionIndex == nJ)
             {
               if(bGetStops || !(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
               {
                 toNodeRecordID = PATTERNS.NODESrecordID;
                 for(found = FALSE, nI = 0; nI < numConnections; nI++)
                 {
                   if((found = (fromNodeRecordID == pFromNODESrecordIDs[nI] &&
                         toNodeRecordID == pToNODESrecordIDs[nI])) == TRUE)
                     break;
                 }
                 if(!found)
                 {
                   pFromNODESrecordIDs[numConnections] = fromNodeRecordID;
                   pToNODESrecordIDs[numConnections++] = toNodeRecordID;
                 }
                 fromNodeRecordID = toNodeRecordID;
               }
               previousPATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
               rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
               if(PATTERNS.PATTERNNAMESrecordID != previousPATTERNNAMESrecordID)
               {
                 fromNodeRecordID = PATTERNS.NODESrecordID;
                 rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
               }
             }
           }
         }
         if(bBreakLoop)
           break;
//
//  Got 'em - now add them to the LIST list box
//
         SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         for(nJ = 0; nJ < numConnections; nJ++)
         {
           CONNECTIONSKey1.fromNODESrecordID = pFromNODESrecordIDs[nJ];
           CONNECTIONSKey1.toNODESrecordID = pToNODESrecordIDs[nJ];
           CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
           rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
           if(rcode2 == 0 &&
                 CONNECTIONS.fromNODESrecordID == pFromNODESrecordIDs[nJ] &&
                 CONNECTIONS.toNODESrecordID == pToNODESrecordIDs[nJ])
           {
             if(displaySelection == PICKCONNECTION_EVERYTHING ||
                   (CONN.flags == CONNECTIONS.flags || (CONN.flags | CONNECTIONS_FLAG_TWOWAY) == CONNECTIONS.flags))
             {
               NODESKey0.recordID = CONNECTIONS.fromNODESrecordID;
               btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
               strncpy(fromAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
               trim(fromAbbrName, NODES_ABBRNAME_LENGTH);
               NODESKey0.recordID = CONNECTIONS.toNODESrecordID;
               btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
               strncpy(toAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
               trim(toAbbrName, NODES_ABBRNAME_LENGTH);
               strcpy(tempString, fromAbbrName);
               strcat(tempString, "\t");
               strcat(tempString, toAbbrName);
               nI = SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
               SendMessage(hCtlLIST, LB_SETITEMDATA, (WPARAM)nI, CONNECTIONS.recordID);
             }
           }
         }
         enable = SendMessage(hCtlLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) != 0;
         EnableWindow(hCtlIDTAKEALL, enable);
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
//  PICKCONNECTION_EVERYTHING
//  PICKCONNECTION_JUST
//
       case PICKCONNECTION_EVERYTHING:
       case PICKCONNECTION_JUST:
         enable = wmId == PICKCONNECTION_JUST;
         EnableWindow(hCtlRUNNINGTIMES, enable);
         EnableWindow(hCtlTRAVELTIMES, enable);
         EnableWindow(hCtlDEADHEADTIMES, enable);
         EnableWindow(hCtlEQUIV, enable);
         EnableWindow(hCtlSTOPSTOP, enable);
         displaySelection = wmId;
         break;
//
//  PICKCONNECTION_RUNNINGTIMES
//  PICKCONNECTION_TRAVELTIMES
//  PICKCONNECTION_DEADHEADTIMES
//  PICKCONNECTION_EQUIV
//  PICKCONNECTION_STOPSTOP check boxes
//
       case PICKCONNECTION_RUNNINGTIMES:
         if(CONN.flags & CONNECTIONS_FLAG_RUNNINGTIME)
           CONN.flags &= ~CONNECTIONS_FLAG_RUNNINGTIME;
         else
           CONN.flags |= CONNECTIONS_FLAG_RUNNINGTIME;
         SendMessage(hCtlRUNNINGTIMES, BM_SETCHECK,
               (WPARAM)(CONN.flags & CONNECTIONS_FLAG_RUNNINGTIME), (LPARAM)0);
         if(criteriaSelection == PICKCONNECTION_SPECIFIC)
           SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
         else
           SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(PICKCONNECTION_ALL, 0), (LPARAM)0);
         break;

       case PICKCONNECTION_TRAVELTIMES:
         if(CONN.flags & CONNECTIONS_FLAG_TRAVELTIME)
           CONN.flags &= ~CONNECTIONS_FLAG_TRAVELTIME;
         else
           CONN.flags |= CONNECTIONS_FLAG_TRAVELTIME;
         SendMessage(hCtlTRAVELTIMES, BM_SETCHECK,
             (WPARAM)(CONN.flags & CONNECTIONS_FLAG_TRAVELTIME), (LPARAM)0);
         if(criteriaSelection == PICKCONNECTION_SPECIFIC)
           SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
         else
           SendMessage(hWndDlg, WM_COMMAND, (WPARAM)PICKCONNECTION_ALL, (LPARAM)0);
         break;

       case PICKCONNECTION_DEADHEADTIMES:
         if(CONN.flags & CONNECTIONS_FLAG_DEADHEADTIME)
           CONN.flags &= ~CONNECTIONS_FLAG_DEADHEADTIME;
         else
           CONN.flags |= CONNECTIONS_FLAG_DEADHEADTIME;
         SendMessage(hCtlDEADHEADTIMES, BM_SETCHECK,
               (WPARAM)(CONN.flags & CONNECTIONS_FLAG_DEADHEADTIME), (LPARAM)0);
         if(criteriaSelection == PICKCONNECTION_SPECIFIC)
           SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
         else
           SendMessage(hWndDlg, WM_COMMAND, (WPARAM)PICKCONNECTION_ALL, (LPARAM)0);
         break;

       case PICKCONNECTION_EQUIV:
         if(CONN.flags & CONNECTIONS_FLAG_EQUIVALENT)
           CONN.flags &= ~CONNECTIONS_FLAG_EQUIVALENT;
         else
           CONN.flags |= CONNECTIONS_FLAG_EQUIVALENT;
         SendMessage(hCtlEQUIV, BM_SETCHECK,
               (WPARAM)(CONN.flags & CONNECTIONS_FLAG_EQUIVALENT), (LPARAM)0);
         if(criteriaSelection == PICKCONNECTION_SPECIFIC)
           SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
         else
           SendMessage(hWndDlg, WM_COMMAND, (WPARAM)PICKCONNECTION_ALL, (LPARAM)0);
         break;
      
       case PICKCONNECTION_STOPSTOP:
         if(CONN.flags & CONNECTIONS_FLAG_STOPSTOP)
           CONN.flags &= ~CONNECTIONS_FLAG_STOPSTOP;
         else
           CONN.flags |= CONNECTIONS_FLAG_STOPSTOP;
         SendMessage(hCtlSTOPSTOP, BM_SETCHECK,
               (WPARAM)(CONN.flags & CONNECTIONS_FLAG_STOPSTOP), (LPARAM)0);
         if(criteriaSelection == PICKCONNECTION_SPECIFIC)
           SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
         else
           SendMessage(hWndDlg, WM_COMMAND, (WPARAM)PICKCONNECTION_ALL, (LPARAM)0);
         break;
//
//  OPTIONS Radio buttons
//
       case PICKCONNECTION_ADDTODISPLAY:
       case PICKCONNECTION_REPLACEDISPLAY:
         optionsSelection = wmId;
         nCwCheckRadioButtons(hWndDlg, optionsRG, (int)wmId, BUTTONMAX);
         break;
//
//  SPECIFIC Radio button - Set up the route and service list, enable the
//        SPECIFIC list box, and disable the ALL list box.  Lastly, fill
//        the SPECIFIC list box through a call to WM_USERSETUP.
//
       case PICKCONNECTION_SPECIFIC:
         nCwCheckRadioButtons(hWndDlg, criteriaRG, PICKCONNECTION_SPECIFIC, BUTTONMAX);
         if((SetUpRouteList(hWndDlg, PICKCONNECTION_ROUTE, selectedRouteRecordID)) == 0)
           SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
         if((SetUpServiceList(hWndDlg, PICKCONNECTION_SERVICE, selectedServiceRecordID)) == 0)
           SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
         EnableWindow(hCtlROUTETITLE, TRUE);
         EnableWindow(hCtlROUTE, TRUE);
         EnableWindow(hCtlSERVICETITLE, TRUE);
         EnableWindow(hCtlSERVICE, TRUE);
         SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         SendMessage(hWndDlg, WM_USERSETUP, PICKCONNECTION_SPECIFIC, (LPARAM)0);
         break;
//
//  ALL Radio button - Nuke out the route and service combo boxes, and disable them.
//
       case PICKCONNECTION_ALL:
         nCwCheckRadioButtons(hWndDlg, criteriaRG, PICKCONNECTION_ALL, BUTTONMAX);
         saveCursor = SetCursor(hCursorWait);
         SendMessage(hCtlROUTE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         SendMessage(hCtlSERVICE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         EnableWindow(hCtlROUTETITLE, FALSE);
         EnableWindow(hCtlROUTE, FALSE);
         EnableWindow(hCtlSERVICETITLE, FALSE);
         EnableWindow(hCtlSERVICE, FALSE);
         rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
         theSamePair = FALSE;
         while(rcode2 == 0)
         {
           if(displaySelection == PICKCONNECTION_EVERYTHING ||
                 (CONN.flags == CONNECTIONS.flags || (CONN.flags | CONNECTIONS_FLAG_TWOWAY) == CONNECTIONS.flags))
           {
             if(!theSamePair)
             {
               NODESKey0.recordID = CONNECTIONS.fromNODESrecordID;
               btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
               strncpy(fromAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
               trim(fromAbbrName, NODES_ABBRNAME_LENGTH);
               NODESKey0.recordID = CONNECTIONS.toNODESrecordID;
               btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
               strncpy(toAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
               trim(toAbbrName, NODES_ABBRNAME_LENGTH);
               strcpy(tempString, fromAbbrName);
               strcat(tempString, "\t");
               strcat(tempString, toAbbrName);
               nI = SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
               SendMessage(hCtlLIST, LB_SETITEMDATA, (WPARAM)nI, CONNECTIONS.recordID);
               previousFromNodeRecordID = CONNECTIONS.fromNODESrecordID;
               previousToNodeRecordID = CONNECTIONS.toNODESrecordID;
             }
           }
           rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
           theSamePair = (CONNECTIONS.fromNODESrecordID == previousFromNodeRecordID) &&
                 (CONNECTIONS.toNODESrecordID == previousToNodeRecordID);
         }
         enable = SendMessage(hCtlLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) != 0;
         EnableWindow(hCtlIDTAKEALL, enable);
         SetCursor(saveCursor);
//
//  ROUTE
//
       case PICKCONNECTION_ROUTE:
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)NULL, (LPARAM)0);
             SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)PICKCONNECTION_SPECIFIC, (LPARAM)0);
             break;
         }
         break;
//
//  IDTAKEALL
//
       case IDTAKEALL:
         nI = SendMessage(hCtlLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
         if(nI == 0)
           break;
         SendMessage(hCtlLIST, LB_SELITEMRANGE, (WPARAM)TRUE, MAKELPARAM(0, nI - 1));
         break;
//
//  IDOK
//
       case IDOK:
//
//  He must've selected at least one pair
//
         numSelected = SendMessage(hCtlLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
         if(numSelected == LB_ERR || numSelected == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_036, hCtlLIST);
           break;
         }
//
//  Build up a list of connections
//
         if(optionsSelection == PICKCONNECTION_REPLACEDISPLAY)
         {
           CONN.numConnections = 0;
         }
//
//  Allocate space
//
         pSelections = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * numSelected); 
         if(pSelections == NULL)
         {
           AllocationError(__FILE__, __LINE__, FALSE);
           SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
           break;
         }
         SendMessage(hCtlLIST, LB_GETSELITEMS, (WPARAM)numSelected, (LPARAM)pSelections);
         for(nI = 0; nI < numSelected; nI++)
         {
           CONNECTIONSKey0.recordID = SendMessage(hCtlLIST, LB_GETITEMDATA, (WPARAM)pSelections[nI], (LPARAM)0);
           btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
           btrieve(B_GETPOSITION, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
           rcode2 = btrieve(B_GETDIRECT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
           if(rcode2 == 0)
           {
             for(found = FALSE, nJ = 0; nJ < CONN.numConnections; nJ++)
             {
               if((found = (CONN.startRecordID[nJ] == CONNECTIONS.recordID)) == TRUE)
               {
                 break;
               }
             }
             if(!found)
             {
               CONN.startRecordID[CONN.numConnections++] = CONNECTIONS.recordID;
             }
             if(CONN.numConnections >= MAXCONNECTIONSKEPT - 1)
             {
               break;
             }
           }
           fromNodeRecordID = CONNECTIONS.fromNODESrecordID;
           toNodeRecordID = CONNECTIONS.toNODESrecordID;
           CONNECTIONSKey1.fromNODESrecordID = toNodeRecordID;
           CONNECTIONSKey1.toNODESrecordID = fromNodeRecordID;
           CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
           rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
           if(rcode2 == 0 &&
                 CONNECTIONS.fromNODESrecordID == toNodeRecordID &&
                 CONNECTIONS.toNODESrecordID == fromNodeRecordID)
           {
             for(found = FALSE, nJ = 0; nJ < CONN.numConnections; nJ++)
             {
               if((found = (CONN.startRecordID[nJ] == CONNECTIONS.recordID)) == TRUE)
               {
                 break;
               }
             }
             if(!found)
             {
               CONN.startRecordID[CONN.numConnections++] = CONNECTIONS.recordID;
             }
             if(CONN.numConnections >= MAXCONNECTIONSKEPT - 1)
             {
               break;
             }
           }
         }
//
//  Couldn't find the connections
//
         if(CONN.numConnections == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_043, hCtlLIST);
           break;
         }
//
//  Record his display preferences
//
         WritePrivateProfileString((LPSTR)userName, "DisplayRT",
               (LPSTR)(CONN.flags & CONNECTIONS_FLAG_RUNNINGTIME ? "1" : "0"), (LPSTR)szDatabaseFileName);
         WritePrivateProfileString((LPSTR)userName, "DisplayTT",
               (LPSTR)(CONN.flags & CONNECTIONS_FLAG_TRAVELTIME ? "1" : "0"), (LPSTR)szDatabaseFileName);
         WritePrivateProfileString((LPSTR)userName, "DisplayDT",
               (LPSTR)(CONN.flags & CONNECTIONS_FLAG_DEADHEADTIME ? "1" : "0"), (LPSTR)szDatabaseFileName);
         WritePrivateProfileString((LPSTR)userName, "DisplayEQ",
               (LPSTR)(CONN.flags & CONNECTIONS_FLAG_EQUIVALENT ? "1" : "0"), (LPSTR)szDatabaseFileName);
         WritePrivateProfileString((LPSTR)userName, "DisplayNN",
               (LPSTR)(CONN.flags & CONNECTIONS_FLAG_STOPSTOP ? "1" : "0"), (LPSTR)szDatabaseFileName);
//
//  All done
//
         TMSHeapFree(pFromNODESrecordIDs);
         TMSHeapFree(pToNODESrecordIDs);
         TMSHeapFree(pSelections);
         EndDialog(hWndDlg, TRUE);
         break;
//
//  IDCANCEL
//
       case IDCANCEL:
         TMSHeapFree(pFromNODESrecordIDs);
         TMSHeapFree(pToNODESrecordIDs);
         EndDialog(hWndDlg, FALSE);
         break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Selecting_Connection_Times_to_Display);
          break;
    }
    break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of PICKCONNECTIONMsgProc
