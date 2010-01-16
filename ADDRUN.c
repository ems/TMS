//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define TABSET    2

typedef struct BLOCKSINDEXStruct
{
  short int route;
  char      service;
  char      garage;
} BLOCKSINDEXDef;

BOOL CALLBACK ADDRUNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlALLRGRPS;
 static  HANDLE hCtlALLWITHLABEL;
 static  HANDLE hCtlLABELS;
 static  HANDLE hCtlORRGRP;
 static  HANDLE hCtlRGRP;
 static  HANDLE hCtlSGRP;
 static  HANDLE hCtlNA;
 static  HANDLE hCtlONLYASSIGNED;
 static  HANDLE hCtlNODELIST;
 static  HANDLE hCtlLISTBOXTITLE;
 static  HANDLE hCtlBLOCKS;
 static  HANDLE hCtlIDREMOVE;
 static  HANDLE hCtlIDINTERACTIVE;
 static  HANDLE hCtlIDVISUAL;
 static  long   RGRPRouteRecordID;
 static  long   SGRPServiceRecordID;
 static  int    tabPos[TABSET] = {22, 32};
 static  int    selectedFrom;
 static  int    numRoutes;
 HCURSOR saveCursor;
 WORD    DlgWidthUnits;
 BOOL    bFound;
 char    RGRPNumber[ROUTES_NUMBER_LENGTH + 1];
 char    RGRPName[ROUTES_NAME_LENGTH + 1];
 char    SGRPName[SERVICES_NAME_LENGTH + 1];
 char    NODEName[NODES_ABBRNAME_LENGTH + 1];
 char    dummy[256];
 int     adjustedTabPos[TABSET];
 int     nI;
 int     nJ;
 int     rcode2;
 int     numGarages;
 int     nRc;

 short int wmId;
 short int wmEvent;
 long flags;
 long tempLong;
  int numSelected;
  long position;
  BLOCKSINDEXDef BLOCKSINDEX;
  int selectedLabel;
  long commentsRecordID;
  BLOCKSDef *pTRIPSChunk;
  int serviceIndex;
  long SERVICESrecordID;
  long NODESrecordID;
  long ROUTESrecordID;
  int  garageIndex;
  int  numBlocks;
  int  bEnable;

  int *pSelected = NULL;

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
//  Set up the handle to the controls
//
     hCtlALLRGRPS = GetDlgItem(hWndDlg, ADDRUN_ALLRGRPS);
     hCtlALLWITHLABEL = GetDlgItem(hWndDlg, ADDRUN_ALLWITHLABEL);
     hCtlLABELS = GetDlgItem(hWndDlg, ADDRUN_LABELS);
     hCtlORRGRP = GetDlgItem(hWndDlg, ADDRUN_ORRGRP);
     hCtlRGRP = GetDlgItem(hWndDlg, ADDRUN_RGRP);
     hCtlSGRP = GetDlgItem(hWndDlg, ADDRUN_SGRP);
     hCtlNA = GetDlgItem(hWndDlg, ADDRUN_NA);
     hCtlONLYASSIGNED = GetDlgItem(hWndDlg, ADDRUN_ONLYASSIGNED);
     hCtlNODELIST = GetDlgItem(hWndDlg, ADDRUN_NODELIST);
     hCtlLISTBOXTITLE = GetDlgItem(hWndDlg, ADDRUN_LISTBOXTITLE);
     hCtlBLOCKS = GetDlgItem(hWndDlg, ADDRUN_BLOCKS);
     hCtlIDREMOVE = GetDlgItem(hWndDlg, IDREMOVE);
     hCtlIDINTERACTIVE = GetDlgItem(hWndDlg, IDINTERACTIVE);
     hCtlIDVISUAL = GetDlgItem(hWndDlg, IDVISUAL);
//
//  Set up the tab stops for ADDRUN_LISTBOXTITLE and ADDRUN_BLOCKS, and draw the title
//
     DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
     for(nJ = 0; nJ < TABSET; nJ++)
     {
       adjustedTabPos[nJ] = (DlgWidthUnits * tabPos[nJ] * 2);
     }
     SendMessage(hCtlLISTBOXTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
     LoadString(hInst, TEXT_024, tempString, TEMPSTRING_LENGTH);
     SendMessage(hCtlLISTBOXTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
     SendMessage(hCtlBLOCKS, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  Display the "destination"
//
     flags = PLACEMENT_SERVICE | PLACEMENT_DIVISION;
     DisplayPlacement(hWndDlg, ADDRUN_DESTINATION, pDI, flags);
//
//  SGRP
//
     if(SetUpServiceList(hWndDlg, ADDRUN_SGRP, pDI->fileInfo.serviceRecordID) == 0)
     {
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
//
//  Default to All RGRPS and N/A on the subset
//
     SendMessage(hCtlALLRGRPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  However, set up the combo boxes in case he picks one of them
//
//  Labels
//
     saveCursor = SetCursor(hCursorWait);
     rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
     while(rcode2 == 0)
     {
       if(ROUTES.COMMENTSrecordID != NO_RECORD)
       {
         nJ = (int)SendMessage(hCtlLABELS, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
         for(bFound = FALSE, nI = 0; nI < nJ; nI++)
         {
           if(SendMessage(hCtlLABELS, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == ROUTES.COMMENTSrecordID)
           {
             bFound = TRUE;
             break;
           }
         }
         if(!bFound)
         {
           COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
           btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
           strncpy(tempString, COMMENTS.code, sizeof(COMMENTS.code));
           trim(tempString, sizeof(COMMENTS.code));
           nI = (int)SendMessage(hCtlLABELS, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
           SendMessage(hCtlLABELS, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)ROUTES.COMMENTSrecordID);
         }
       }
       rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
     }
     SendMessage(hCtlLABELS, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
     SetCursor(saveCursor);
//
//  RGRP
//
//
//  Set up the route listbox
//
      SendMessage(hCtlRGRP, LB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(szarString, ROUTES_NAME_LENGTH);
        strcat(tempString, " - ");
        strcat(tempString, szarString);
        nI = (int)SendMessage(hCtlRGRP, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlRGRP, LB_SETITEMDATA, (WPARAM)nI, ROUTES.recordID);
        numRoutes++;
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      }
      if(numRoutes == 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL,0), (LPARAM)0);
        break;
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
        nI = SendMessage(hCtlNODELIST, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlNODELIST, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)NODES.recordID);
        numGarages++;
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
      if(numGarages == 0)
      {
        EnableWindow(hCtlONLYASSIGNED, FALSE);
        EnableWindow(hCtlNODELIST, FALSE);
      }
//
//  Allocate space for m_pPICKEDBLOCKS
//
      m_maxPICKEDBLOCKS = 128;
      m_pPICKEDBLOCKS = (PICKEDBLOCKSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PICKEDBLOCKSDef) * m_maxPICKEDBLOCKS); 
      if(m_pPICKEDBLOCKS == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Zero out the selections
//
      m_numPICKEDBLOCKS = 0;
      break;
//
//  WM_CLOSE
//
   case WM_CLOSE:
     TMSHeapFree(m_pPICKEDBLOCKS);
     EndDialog(hWndDlg, FALSE);
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
//  ADDRUN_ALLRGRPS
//
        case ADDRUN_ALLRGRPS:
          if(SendMessage(hCtlALLRGRPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLABELS, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
          }
          break;
//
//  ADDRUN_ALLWITHLABEL
//
        case ADDRUN_ALLWITHLABEL:
          if(SendMessage(hCtlALLWITHLABEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLABELS, CB_SETCURSEL, (WPARAM)(0), (LPARAM)0);
            SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
          }
          break;
//
//  ADDRUN_LABELS
//
        case ADDRUN_LABELS:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlALLRGRPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                    SendMessage(hCtlORRGRP, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLRGRPS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORRGRP, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlALLWITHLABEL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
              }
              break;  
          }
          break;
//
//  ADDRUN_ORRGRP
//
        case ADDRUN_ORRGRP:
          if(SendMessage(hCtlORRGRP, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLABELS, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          }
          break;
//
//  ADDRUN_RGRP
//
        case ADDRUN_RGRP:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLRGRPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                    SendMessage(hCtlALLWITHLABEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLRGRPS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlALLWITHLABEL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORRGRP, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          break;
//
//  ADDRUN_LISTBOXTITLE
//
       case ADDRUN_LISTBOXTITLE:    // Title string
         SendMessage(hCtlLISTBOXTITLE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
         break;
//
//  ADDRUN_BLOCKS
//
       case ADDRUN_BLOCKS: // List box
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             if(SendMessage(hWndDlg, LB_GETCURSEL, (WPARAM)0, (LPARAM)0) == LB_ERR)
               break;
             EnableWindow(hCtlIDREMOVE, TRUE);
             break;
         }
         break;
//
//  "Assigned to Garage" stuff
//
        case ADDRUN_NODELIST:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlONLYASSIGNED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDRUN_NA:
          SendMessage(hCtlNODELIST, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDRUN_ONLYASSIGNED:
          SendMessage(hCtlNODELIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  IDADD
//
       case IDADD:
         saveCursor = SetCursor(hCursorWait);
//
//  Highlight the applicable entries in ADDRUN_RGRP
//
//  All routes
//
         if(SendMessage(hCtlALLRGRPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(-1));
         }
//
//  All labelled
//
         else if(SendMessage(hCtlALLWITHLABEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           selectedLabel = SendMessage(hCtlLABELS, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           commentsRecordID = SendMessage(hCtlLABELS,
                 CB_GETITEMDATA, (WPARAM)selectedLabel, (LPARAM)0);
           SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
           numRoutes = SendMessage(hCtlRGRP, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
           for(nI = 0; nI < numRoutes; nI++)
           {
             ROUTESKey0.recordID = SendMessage(hCtlRGRP, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
             btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
             if(ROUTES.COMMENTSrecordID == commentsRecordID)
               SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nI);
           }
         }
//
//  Get the service
//
         serviceIndex = SendMessage(hCtlSGRP, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
         SendMessage(hCtlSGRP, CB_GETLBTEXT, (WPARAM)serviceIndex, (LONG)(LPSTR)SGRPName);
         SERVICESrecordID = SendMessage(hCtlSGRP,
               CB_GETITEMDATA, (WPARAM)serviceIndex, (LPARAM)0);
//
//  Is there a subset garage?
//
         if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           garageIndex = NO_RECORD;
           LoadString(hInst, TEXT_026, NODEName, sizeof(NODEName));
           NODESrecordID = NO_RECORD;
         }
         else
         {
           garageIndex = SendMessage(hCtlNODELIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           SendMessage(hCtlNODELIST, CB_GETLBTEXT, (WPARAM)garageIndex, (LONG)(LPSTR)NODEName);
           NODESrecordID = SendMessage(hCtlNODELIST,
                 CB_GETITEMDATA, (WPARAM)garageIndex, (LPARAM)0);
         }
//
//  Ok, got all the pertinent data
//
//  Loop through all the selections and all the current
//  entries in ADDRUN_BLOCKS to ensure that the same
//  information isn't added twice.  We'll do this in three
//  passes, the first of which will deselect any duplicate 
//  route from the RGRP box.  The second pass determines if
//  there are actually any blocks in that RGRP/SGRP to add.
//  The last pass is the physical add.
//
          nI = (int)SendMessage(hCtlRGRP, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
          if(nI == 0 || nI == LB_ERR)
          {
            break;
          }
          pSelected = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * nI); 
          if(pSelected == NULL)
          {
            AllocationError(__FILE__, __LINE__, FALSE);
            break;
          }
          numSelected = (int)SendMessage(hCtlRGRP, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
          numBlocks = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
//
//  Pass 1: Loop through and knock out any duplicates
//
          BLOCKSINDEX.service = serviceIndex;
          BLOCKSINDEX.garage = garageIndex;
          for(nI = 0; nI < numSelected; nI++)
          {
            BLOCKSINDEX.route = pSelected[nI];
            memcpy(&position, &BLOCKSINDEX, sizeof(long));
            for(nJ = 0; nJ < numBlocks; nJ++)
            {
              tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
              if(tempLong == position)
                SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)FALSE, (LPARAM)pSelected[nI]);
            }
          }
//
//  Pass 2: Loop through an deselect the RGRP if there are no blocks
//
//  Figure out nI, numSelected, and selected again, 
//  in case we deselected anything in step 1.
//
          nI = (int)SendMessage(hCtlRGRP, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
          numSelected = (int)SendMessage(hCtlRGRP, LB_GETSELITEMS,
                (WPARAM)nI, (LPARAM)(int far *)pSelected);
          for(nI = 0; nI < numSelected; nI++)
          {
            for(bFound = FALSE, nJ = 2; nJ <=3; nJ++)
            {
              pTRIPSChunk = nJ == 2 ? &TRIPS.standard : &TRIPS.dropback;
              ROUTESrecordID = SendMessage(hCtlRGRP, LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
              TRIPSKey2.assignedToNODESrecordID = NODESrecordID;
              TRIPSKey2.RGRPROUTESrecordID = ROUTESrecordID;
              TRIPSKey2.SGRPSERVICESrecordID = SERVICESrecordID;
              TRIPSKey2.blockNumber = NO_RECORD;
              TRIPSKey2.blockSequence = NO_TIME;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, nJ);
              if(rcode2 == 0 &&
                    pTRIPSChunk->assignedToNODESrecordID == NODESrecordID &&
                    pTRIPSChunk->RGRPROUTESrecordID == ROUTESrecordID &&
                    pTRIPSChunk->SGRPSERVICESrecordID == SERVICESrecordID &&
                    pTRIPSChunk->blockNumber > 0)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
              SendMessage(hCtlRGRP, LB_SETSEL, (WPARAM)FALSE, (LPARAM)pSelected[nI]);
          }
//
//  Pass 3: Add the route/ser/gar to the ADDRUN_BLOCKS box
//
//  Figure out nI, numSelected, and selected again, 
//  in case we deselected anything in steps 1 and 2.
//
          nI = (int)SendMessage(hCtlRGRP, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
          numSelected = (int)SendMessage(hCtlRGRP, LB_GETSELITEMS,
                (WPARAM)nI, (LPARAM)(int far *)pSelected);
          for(nI = 0; nI < numSelected; nI++)
          {
//
//  Get the route name and number
//
            ROUTESKey0.recordID = SendMessage(hCtlRGRP,
                  LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(RGRPNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(RGRPNumber, ROUTES_NUMBER_LENGTH);
            strncpy(RGRPName, ROUTES.name, ROUTES_NAME_LENGTH);
            trim(RGRPName, ROUTES_NAME_LENGTH);
//
//  Set up itemdata
//
            BLOCKSINDEX.route = pSelected[nI];
            BLOCKSINDEX.service = serviceIndex;
            BLOCKSINDEX.garage = garageIndex;
            memcpy(&position, &BLOCKSINDEX, sizeof(long));
// 
//  Get the display string ready
//
            strcpy(tempString, RGRPNumber);
            strcat(tempString, " - ");
            strcat(tempString, RGRPName);
            trim(tempString, tabPos[0]);
            strcat(tempString, "\t");
            strcat(tempString, SGRPName);
            strcat(tempString, "\t");
            strcat(tempString, NODEName);
//
//  Add it and set itemdata
//
            nJ = (int)SendMessage(hCtlBLOCKS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlBLOCKS, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)position);
          }
//
//  Enable the VISUAL and INTERACTIVE buttons
//
         bEnable = numSelected > 0;
         EnableWindow(hCtlIDINTERACTIVE, bEnable);
         EnableWindow(hCtlIDVISUAL, bEnable);
//
//  And release the memory and reset the cursor
//
         TMSHeapFree(pSelected);
         SetCursor(saveCursor);
         break;
//
//  IDREMOVE
//
        case IDREMOVE: // Button text: "Remove"
          nI = (int)SendMessage(hCtlBLOCKS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
          if(nI == 0 || nI == LB_ERR)
          {
            break;
          }
          pSelected = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * nI); 
          if(pSelected == NULL)
          {
            AllocationError(__FILE__, __LINE__, FALSE);
            break;
          }
          numSelected = (int)SendMessage(hCtlBLOCKS, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
//
//  Process the list from the back to the front
//
          for(nI = numSelected - 1; nI >= 0; nI--)
          {
            SendMessage(hCtlBLOCKS, LB_DELETESTRING, (WPARAM)pSelected[nI], (LPARAM)0);
          }
          if(SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) == 0)
          {
            EnableWindow(hCtlIDINTERACTIVE, FALSE);
          }
          EnableWindow(hCtlIDREMOVE, FALSE);
          TMSHeapFree(pSelected);
          break;
//
//  IDCANCEL
//
       case IDCANCEL:
         TMSHeapFree(m_pPICKEDBLOCKS);
         EndDialog(hWndDlg, FALSE);
         break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Runs_Table);
          break;
//
//  IDINTERACTIVE and IDVISUAL
//
       case IDINTERACTIVE:
       case IDVISUAL:
//
//  Grab the selections
//
         numSelected = SendMessage(hCtlBLOCKS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
         m_numPICKEDBLOCKS = 0;
         for(nI = 0; nI < numSelected; nI++)
         {
           if(m_numPICKEDBLOCKS >= m_maxPICKEDBLOCKS)
           {
             m_maxPICKEDBLOCKS += 128;
             m_pPICKEDBLOCKS = (PICKEDBLOCKSDef *)HeapReAlloc(GetProcessHeap(),
                   HEAP_ZERO_MEMORY, m_pPICKEDBLOCKS, sizeof(PICKEDBLOCKSDef) * m_maxPICKEDBLOCKS); 
             if(m_pPICKEDBLOCKS == NULL)
             {
               AllocationError(__FILE__, __LINE__, TRUE);
               break;
             }
           }
           tempLong = SendMessage(hCtlBLOCKS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
           memcpy(&BLOCKSINDEX, &tempLong, sizeof(long));
           NODESrecordID =
                 SendMessage(hCtlNODELIST, CB_GETITEMDATA, (WPARAM)BLOCKSINDEX.garage, (LPARAM)0);
           ROUTESrecordID =
                 SendMessage(hCtlRGRP, LB_GETITEMDATA, (WPARAM)BLOCKSINDEX.route, (LPARAM)0);
           SERVICESrecordID =
                 SendMessage(hCtlSGRP, CB_GETITEMDATA, (WPARAM)BLOCKSINDEX.service, (LPARAM)0);
//
//  Are these standard or dropback blocks?
//
//  Dropback (key 3) are always selected over standard
//
           for(bFound = FALSE, nJ = 3; nJ >=2; nJ--)
           {
             pTRIPSChunk = nJ == 2 ? &TRIPS.standard : &TRIPS.dropback;
             TRIPSKey2.assignedToNODESrecordID = NODESrecordID;
             TRIPSKey2.RGRPROUTESrecordID = ROUTESrecordID;
             TRIPSKey2.SGRPSERVICESrecordID = SERVICESrecordID;
             TRIPSKey2.blockNumber = NO_RECORD;
             TRIPSKey2.blockSequence = NO_TIME;
             rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, nJ);
             if(rcode2 == 0 &&
                   pTRIPSChunk->assignedToNODESrecordID == NODESrecordID &&
                   pTRIPSChunk->RGRPROUTESrecordID == ROUTESrecordID &&
                   pTRIPSChunk->SGRPSERVICESrecordID == SERVICESrecordID &&
                   pTRIPSChunk->blockNumber > 0)
             {
               m_pPICKEDBLOCKS[m_numPICKEDBLOCKS].flags = (nJ == 2 ? PICKEDBLOCKS_FLAG_STANDARD : PICKEDBLOCKS_FLAG_DROPBACK);
               bFound = TRUE;
               break;
             }
           }
           m_pPICKEDBLOCKS[m_numPICKEDBLOCKS].RGRPROUTESrecordID = ROUTESrecordID;
           m_pPICKEDBLOCKS[m_numPICKEDBLOCKS].SGRPSERVICESrecordID = SERVICESrecordID;
           m_pPICKEDBLOCKS[m_numPICKEDBLOCKS].NODESrecordID = NODESrecordID;
           m_numPICKEDBLOCKS++;
         }
//
//  Establish the internal list of node names and record IDs
//
         TMSHeapFree(m_pNodeAbbrs);
         rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
         m_numNodeAbbrs = BSTAT.numRecords;
         m_pNodeAbbrs = (NODEABBRSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(NODEABBRSDef) * m_numNodeAbbrs); 
         if(m_pNodeAbbrs == NULL)
         {
           AllocationError(__FILE__, __LINE__, FALSE);
           break;
         }
         else
         {
           nI = 0;
           rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
           while(rcode2 == 0)
           {
             m_pNodeAbbrs[nI].recordID = NODES.recordID;
             strncpy(m_pNodeAbbrs[nI].abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
             trim(m_pNodeAbbrs[nI].abbrName, NODES_ABBRNAME_LENGTH);
             rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
             nI++;
           }
//
//  Fire up the manual/automatic runcutter
//
           RUNSAssignToDIVISION = pDI->fileInfo.divisionRecordID;
           if(wmId == IDINTERACTIVE)
           {
             nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MANCUT),
                   hWndDlg, (DLGPROC)MANCUTMsgProc, (LPARAM)pDI);
           }
//
//  Fire up the visual runcutter
//
           else
           {
             nRc = VisualRuncutter(pDI);
           }
           TMSHeapFree(m_pPICKEDBLOCKS);
           EndDialog(hWndDlg, nRc);
         }
         break;
       }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
  }
  return TRUE;
} //  End of ADDRUNMsgProc
