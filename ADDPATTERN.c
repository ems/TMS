//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

typedef struct PATStruct
{
  long NODESrecordID;
  long nodeSequence;
  char nodeName[NODES_ABBRNAME_LENGTH + 1];
  BOOL bMLP;
  BOOL bBusStop;
} PATDef;

typedef struct NODStruct
{
  long recordID;
  char szAbbrName[NODES_ABBRNAME_LENGTH + 1];
  char szDescription[NODES_DESCRIPTION_LENGTH + 1];
} NODDef;

int sort_AbbrName(const void *a, const void *b)
{
  NODDef *pa, *pb;
  pa = (NODDef *)a;
  pb = (NODDef *)b;

  return(strcmp(pa->szAbbrName, pb->szAbbrName));
}

int sort_Description(const void *a, const void *b)
{
  NODDef *pa, *pb;
  pa = (NODDef *)a;
  pb = (NODDef *)b;

  return(strcmp(pa->szDescription, pb->szDescription));
}

BOOL CALLBACK ADDPATTERNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static  PDISPLAYINFO pDI;
 static  HANDLE hCtlNAME;
 static  HANDLE hCtlALLTITLE;
 static  HANDLE hCtlSHOWSTOPS;
 static  HANDLE hCtlSHOWADDRESS;
 static  HANDLE hCtlSHOWDESCRIPTION;
 static  HANDLE hCtlALLNODES;
 static  HANDLE hCtlPATTERNNODES;
 static  HANDLE hCtlADD;
 static  HANDLE hCtlREMOVE;
 static  HANDLE hCtlSETMLP;
 static  HANDLE hCtlLOADREVERSE;
 static  HANDLE hCtlNA;
 static  HANDLE hCtlCC;
 static  HANDLE hCtlCOMMENTCODE;
 static  HANDLE hCtlFROMTEXT;
 static  HANDLE hCtlTOTEXT;
 static  HANDLE hCtlSORT;
 static  BOOL   bDisplaySystemNodes;
 static  BOOL   bDisplayStops;
 static  BOOL   bShowAddress;
 static  char   szMaxString[16];
 static  char   szStopString[16];
 static  int numPAT;
 static  int maxPAT;
 static  int numNodes;
 HCURSOR saveCursor;
 WORD    ALLNODEStabSet;
 WORD    DlgWidthUnits;
 long    firstEntryOfNewBase = NO_RECORD;
 long    thisPatternName;
 long    tempLong;
 long    maxBPN;
 long    previousSeq;
 BOOL    bFirst;
 BOOL    bFound;
 BOOL    bBreak;
 char    abbrName[NODES_ABBRNAME_LENGTH + 1];
 int     rcode2;
 int     nI;
 int     nJ;
 int     nK;
 int     nL;
 int     nM;
 int     nN;
 int     ALLNODEStabPositions[2];
 int     *ALLNODEStabPointer = ALLNODEStabPositions;
 int     numSelected;
 int     numComments;
 int     pos;
 int     divisor;
 int     numPatternNodes;
 short int wmId;
 short int wmEvent;
 long    flags;
 
 static  PATDef *pPAT = NULL;
 int    *pSelected = NULL;
 long   *pBASEPatternNodeSeqs = NULL;
 BOOL   *pbBASEPatternNodeSeqsSelected = NULL;

 static  NODDef *pNOD = NULL;
 static  BOOL bSortingByAbbrName = TRUE;
 char    dummy[256];
 int     numRecords;


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
//  Pattern allocation
//
      numPAT = 0;
      maxPAT = 1024;
      pPAT = (PATDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PATDef) * maxPAT); 
      if(pPAT == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Node allocation
//
//  Get the number of nodes in the table
//
      rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
      numRecords = BSTAT.numRecords;
      pNOD = (NODDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(NODDef) * numRecords); 
      if(pNOD == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
     hCtlNAME = GetDlgItem(hWndDlg, ADDPATTERN_NAME);
     hCtlALLTITLE = GetDlgItem(hWndDlg, ADDPATTERN_ALLTITLE);
     hCtlSHOWSTOPS = GetDlgItem(hWndDlg, ADDPATTERN_SHOWSTOPS);
     hCtlSHOWADDRESS = GetDlgItem(hWndDlg, ADDPATTERN_SHOWADDRESS);
     hCtlSHOWDESCRIPTION = GetDlgItem(hWndDlg, ADDPATTERN_SHOWDESCRIPTION);
     hCtlALLNODES = GetDlgItem(hWndDlg, ADDPATTERN_ALLNODES);
     hCtlPATTERNNODES = GetDlgItem(hWndDlg, ADDPATTERN_PATTERNNODES);
     hCtlADD = GetDlgItem(hWndDlg, IDADD);
     hCtlREMOVE = GetDlgItem(hWndDlg, IDREMOVE);
     hCtlSETMLP = GetDlgItem(hWndDlg, IDASSIGN);
     hCtlLOADREVERSE = GetDlgItem(hWndDlg, IDLOADREVERSE);
     hCtlNA = GetDlgItem(hWndDlg, ADDPATTERN_NA);
     hCtlCC = GetDlgItem(hWndDlg, ADDPATTERN_CC);
     hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDPATTERN_COMMENTCODE);
     hCtlFROMTEXT = GetDlgItem(hWndDlg, ADDPATTERN_FROMTEXT);
     hCtlTOTEXT = GetDlgItem(hWndDlg, ADDPATTERN_TOTEXT);
     hCtlSORT = GetDlgItem(hWndDlg, IDSORT);
//
//  Comment codes combo box
//
     numComments = SetUpCommentList(hWndDlg, ADDPATTERN_COMMENTCODE);
     if(numComments == 0)
     {
       EnableWindow(hCtlCC, FALSE);
       EnableWindow(hCtlCOMMENTCODE, FALSE);
     }
//
//  Set up * Max * and > Stop < strings
//
     strcpy(szMaxString, "\t");
     LoadString(hInst, TEXT_022, szarString, sizeof(szarString));
     strcat(szMaxString, szarString);
     strcpy(szStopString, "\t");
     LoadString(hInst, TEXT_044, szarString, sizeof(szarString));
     strcat(szStopString, szarString);
//
//  Set up the tab stops in the ALLNODES and PATTERNNODES list boxes
//
     ALLNODEStabSet = 1;
     DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
     ALLNODEStabPositions[0] = (DlgWidthUnits * (NODES_ABBRNAME_LENGTH + 4) * 2);
     SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
     SendMessage(hCtlALLNODES, LB_SETTABSTOPS, (WPARAM)ALLNODEStabSet, (LPARAM)ALLNODEStabPointer);
     SendMessage(hCtlPATTERNNODES, LB_SETTABSTOPS, (WPARAM)ALLNODEStabSet, (LPARAM)ALLNODEStabPointer);
//
//  Display the direction
//
     flags = PLACEMENT_DIRECTION;
     DisplayPlacement(hWndDlg, ADDPATTERN_DESTINATION, pDI, flags);
//
//  Update...
//
     if(updateRecordID != NO_RECORD)
     {
       PATTERNSKey0.recordID = updateRecordID;
       btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
       btrieve(B_GETPOSITION, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
       btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  Pattern Name
//
       PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
       btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
       strncpy(tempString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
       trim(tempString, sizeof(PATTERNNAMES.name));
       SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Set up the pattern nodes list box and the comment
//
       SendMessage(hCtlPATTERNNODES, LB_RESETCONTENT, 0, 0L);
       PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
       PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
       PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
       PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMES.recordID;
       PATTERNSKey2.nodeSequence = NO_RECORD;
       rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       bFirst = TRUE;
       while(rcode2 == 0 &&
             PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
             PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
             PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
             PATTERNS.PATTERNNAMESrecordID == PATTERNNAMES.recordID)
       {
         if(bFirst)
         {
           bFirst = FALSE;
           if(PATTERNS.COMMENTSrecordID == NO_RECORD)
           {
             SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
           }
           else
           {
             for(bFound = FALSE, nI = 0; nI < numComments; nI++)
             {
               if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == PATTERNS.COMMENTSrecordID)
               {
                 SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
                 bFound = TRUE;
                 break;
               }
             }
           }
           SendMessage(bFound ? hCtlCC : hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
           strncpy(tempString, PATTERNS.fromText, PATTERNS_FROMTEXT_LENGTH);
           trim(tempString, PATTERNS_FROMTEXT_LENGTH);
           SendMessage(hCtlFROMTEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
           strncpy(tempString, PATTERNS.toText, PATTERNS_TOTEXT_LENGTH);
           trim(tempString, PATTERNS_TOTEXT_LENGTH);
           SendMessage(hCtlTOTEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
         }
         NODESKey0.recordID = PATTERNS.NODESrecordID;
         btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
         strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
         trim(tempString, NODES_ABBRNAME_LENGTH);
         if(numPAT >= maxPAT)
         {
           maxPAT += 1024;
           pPAT = (PATDef *)HeapReAlloc(GetProcessHeap(),
                 HEAP_ZERO_MEMORY, pPAT, sizeof(PATDef) * maxPAT); 
           if(pPAT == NULL)
           {
             AllocationError(__FILE__, __LINE__, TRUE);
             break;
           }
         }
         pPAT[numPAT].NODESrecordID = PATTERNS.NODESrecordID;
         pPAT[numPAT].nodeSequence = PATTERNS.nodeSequence;
         strcpy(pPAT[numPAT].nodeName, tempString);
         pPAT[numPAT].bMLP = PATTERNS.flags & PATTERNS_FLAG_MLP;
         pPAT[numPAT].bBusStop = PATTERNS.flags & PATTERNS_FLAG_BUSSTOP;
         if(pPAT[numPAT].bMLP)
         {
           strcat(tempString, szMaxString);
         }
         else if(pPAT[numPAT].bBusStop)
         {
           strcat(tempString, szStopString);
         }
         numPAT++;
         nI = (int)SendMessage(hCtlPATTERNNODES, LB_ADDSTRING, (WPARAM)NULL, (LONG)(LPSTR)tempString);
         SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)nI);
         rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       }
       bDisplaySystemNodes = PATTERNNAMES.recordID == basePatternRecordID;
     }
//
//  Insert...
//
//  If there's no BASE pattern for this route/service/direction, the pattern name
//  automatically becomes the base pattern name, and system nodes are displayed
//
     else
     {
       PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
       PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
       PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
       PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
       PATTERNSKey2.nodeSequence = NO_RECORD;
       rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       bDisplaySystemNodes = rcode2 != 0 ||
             PATTERNS.ROUTESrecordID != pDI->fileInfo.routeRecordID ||
             PATTERNS.SERVICESrecordID != pDI->fileInfo.serviceRecordID ||
             PATTERNS.directionIndex != pDI->fileInfo.directionIndex ||
             PATTERNS.PATTERNNAMESrecordID != basePatternRecordID;
       if(bDisplaySystemNodes)
       {
         SendMessage(hCtlNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)basePatternName);
       }
       SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     }
//
//  No sort when not on BASE pattern
//
     if(!bDisplaySystemNodes)
     {
       EnableWindow(hCtlSORT, FALSE);
     }
//
//  Default to showing addresses
//
     bShowAddress = TRUE;
     SendMessage(hCtlSHOWADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Display a timepoint list, based upon bDisplaySystemNodes.
//
     numNodes = 0;
     bDisplayStops = TRUE;
     SendMessage(hCtlSHOWSTOPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
     EnableWindow(hCtlSHOWSTOPS, bDisplaySystemNodes);
//
//  Can't have no nodes
//
     if(numNodes == 0)
     {
       TMSError(hWndDlg, MB_ICONSTOP, ERROR_024, (HANDLE)NULL);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
//
//  Disable Add, Remove, SetMLP, and LOADREVERSE buttons until later
//
     EnableWindow(hCtlADD, FALSE);
     EnableWindow(hCtlREMOVE, FALSE);
     EnableWindow(hCtlSETMLP, FALSE);
     EnableWindow(hCtlLOADREVERSE, FALSE);
//
//  If we're on the BASE pattern and the other direction's
//  BASE pattern exists, enable the "Load Reverse" button
//
     if(bDisplaySystemNodes)
     {
       nI = (pDI->fileInfo.directionIndex + 1) % 2;  // The other direction index
       PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
       PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
       PATTERNSKey2.directionIndex = nI;
       PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
       PATTERNSKey2.nodeSequence = NO_RECORD;
       rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       bFound = rcode2 == 0 &&
             PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
             PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
             PATTERNS.directionIndex == nI &&
             PATTERNS.PATTERNNAMESrecordID == basePatternRecordID;
       EnableWindow(hCtlLOADREVERSE, bFound);
     }
//
//  Set the focus on the pattern name
//
     SetFocus(hCtlNAME);
     break; 
//
//  WM_CLOSE
//
   case WM_CLOSE:
     SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
     break; 
//
//  WM_USERSETUP
//
   case WM_USERSETUP:
     SendMessage(hCtlNAME, EM_SETREADONLY, (WPARAM)bDisplaySystemNodes, (LPARAM)0);
     SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  System nodes
//
     if(bDisplaySystemNodes)
     {
       LoadString(hInst, TEXT_020, tempString, TEMPSTRING_LENGTH);
       SendMessage(hCtlALLTITLE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
       rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
       numNodes = 0;
       while(rcode2 == 0)
       {
         if((NODES.flags & NODES_FLAG_STOP) && !bDisplayStops)
         {
         }
         else
         {
           pNOD[numNodes].recordID = NODES.recordID;
           strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
           trim(tempString, NODES_ABBRNAME_LENGTH);
           strcpy(pNOD[numNodes].szAbbrName, tempString);
           if(bShowAddress)
           {
             strncpy(szarString, NODES.intersection, NODES_INTERSECTION_LENGTH);
             trim(szarString, NODES_INTERSECTION_LENGTH);
           }
           else
           {
             strncpy(szarString, NODES.description, NODES_DESCRIPTION_LENGTH);
             trim(szarString, NODES_DESCRIPTION_LENGTH);
           }
           strcpy(pNOD[numNodes].szDescription, szarString);
           numNodes++;
         }
         rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
       }
       SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
       for(nI = 0; nI < numNodes; nI++)
       {
         strcpy(tempString, pNOD[nI].szAbbrName);
         strcat(tempString, "\t");
         strcat(tempString, pNOD[nI].szDescription);
         nJ = (int)SendMessage(hCtlALLNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
         SendMessage(hCtlALLNODES, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)pNOD[nI].recordID);
       }
       if(bSortingByAbbrName)
       {
         if(bShowAddress)
         {
           SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Address");
         }
         else
         {
           SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Description");
         }
       }
       else
       {
         SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Abbreviation");
       }
     }
//
//  BASE pattern nodes
//
     else
     {
       numNodes = 0;
       LoadString(hInst, TEXT_021, tempString, TEMPSTRING_LENGTH);
       SendMessage(hCtlALLTITLE, WM_SETTEXT, 0, (LONG)(LPSTR)tempString);
       PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
       PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
       PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
       PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
       PATTERNSKey2.nodeSequence = NO_RECORD;
       rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       while(rcode2 == 0 &&
             PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
             PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
             PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
             PATTERNSKey2.PATTERNNAMESrecordID == basePatternRecordID)
       {
         NODESKey0.recordID = PATTERNS.NODESrecordID;
         btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
         strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
         trim(tempString, NODES_ABBRNAME_LENGTH);
         strcat(tempString, "\t");
         if(bShowAddress)
         {
           strncpy(szarString, NODES.intersection, NODES_INTERSECTION_LENGTH);
           trim(szarString, NODES_INTERSECTION_LENGTH);
         }
         else
         {
           strncpy(szarString, NODES.description, NODES_DESCRIPTION_LENGTH);
           trim(szarString, NODES_DESCRIPTION_LENGTH);
         }
         strcat(tempString, szarString);
         nI = (int)SendMessage(hCtlALLNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
         SendMessage(hCtlALLNODES, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)PATTERNS.nodeSequence);
         numNodes++;
         rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
       }
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
//  Sort
//
        case IDSORT:
          bSortingByAbbrName = !bSortingByAbbrName;
          if(bSortingByAbbrName)
          {
            qsort((void *)pNOD, numNodes, sizeof(NODDef), sort_AbbrName);
          }
          else
          {
            qsort((void *)pNOD, numNodes, sizeof(NODDef), sort_Description);
          }
          SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < numNodes; nI++)
          {
            strcpy(tempString, pNOD[nI].szAbbrName);
            strcat(tempString, "\t");
            strcat(tempString, pNOD[nI].szDescription);
            nJ = (int)SendMessage(hCtlALLNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlALLNODES, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)pNOD[nI].recordID);
          }
          if(bSortingByAbbrName)
          {
            if(bShowAddress)
            {
              SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Address");
            }
            else
            {
              SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Description");
            }
          }
          else
          {
            SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Abbreviation");
          }
          break;
//
//  Show stops
//
        case ADDPATTERN_SHOWSTOPS:
          bDisplayStops = !bDisplayStops;
          SendMessage(hWndDlg, WM_USERSETUP, (WPARAM)0, (LPARAM)0);
          break;
//
//  Show address/description
//
        case ADDPATTERN_SHOWADDRESS:
          bShowAddress = TRUE;
          if(bDisplaySystemNodes)
          { 
            if(bSortingByAbbrName)
            {
              if(bShowAddress)
              {
                SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Address");
              }
              else
              {
                SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Description");
              }
            }
            else
            {
              SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Abbreviation");
            }
          }
          SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < numNodes; nI++)
          {
            strcpy(tempString, pNOD[nI].szAbbrName);
            strcat(tempString, "\t");
            NODESKey0.recordID = pNOD[nI].recordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szarString, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(szarString, NODES_INTERSECTION_LENGTH);
            strcpy(pNOD[nI].szDescription, szarString);
            strcat(tempString, pNOD[nI].szDescription);
            nJ = (int)SendMessage(hCtlALLNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlALLNODES, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)pNOD[nI].recordID);
          }
          break;

        case ADDPATTERN_SHOWDESCRIPTION:
          bShowAddress = FALSE;
          if(bDisplaySystemNodes)
          { 
            if(bSortingByAbbrName)
            {
              if(bShowAddress)
              {
                SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Address");
              }
              else
              {
                SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Description");
              }
            }
            else
            {
              SendMessage(hCtlSORT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Sort by Node Abbreviation");
            }
          }
          SendMessage(hCtlALLNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < numNodes; nI++)
          {
            strcpy(tempString, pNOD[nI].szAbbrName);
            strcat(tempString, "\t");
            NODESKey0.recordID = pNOD[nI].recordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szarString, NODES.description, NODES_DESCRIPTION_LENGTH);
            trim(szarString, NODES_DESCRIPTION_LENGTH);
            strcpy(pNOD[nI].szDescription, szarString);
            strcat(tempString, pNOD[nI].szDescription);
            nJ = (int)SendMessage(hCtlALLNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlALLNODES, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)pNOD[nI].recordID);
          }
          break;
//
//  Comments
//
        case ADDPATTERN_COMMENTCODE:
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

        case ADDPATTERN_NA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDPATTERN_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;

//
//  ADDPATTERN_ALLTITLE
//
//  If the title string gets selected, deselect it immediately
//
      case ADDPATTERN_ALLTITLE:  // Title string
        SendMessage(hCtlALLTITLE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
        break;

//
//  ADDPATTERN_ALLNODES
//
//  When something gets selected, enable the ADD button.
//  A double click on this list box is the same as an add.
//
       case ADDPATTERN_ALLNODES:
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             EnableWindow(hCtlADD, TRUE);
             break;
           case LBN_SELCANCEL:
             nI = (int)SendMessage(hCtlALLNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
             EnableWindow(hCtlADD, (nI != LB_ERR && nI != 0));
             break;
           case LBN_DBLCLK:
             SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDADD, 0), (LPARAM)0);
             break;
         }
         break;
//
//  ADDPATTERN_PATTERNNODES
//
//  When something gets selected, enable the REMOVE and SET MLP buttons.
//  A double click on this list box is the same as SET MLP.
//
       case ADDPATTERN_PATTERNNODES:
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             EnableWindow(hCtlREMOVE, TRUE);
             EnableWindow(hCtlSETMLP, TRUE);
             break;
           case LBN_SELCANCEL:
             EnableWindow(hCtlREMOVE, FALSE);
             EnableWindow(hCtlSETMLP, FALSE);
             break;
           case LBN_DBLCLK:
             SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDASSIGN, 0), (LPARAM)0);
             break;
         }
         break;
//
//  IDADD
//
//  Take the highlighted item(s) in the system/base pattern nodes list box
//  and add it (them) to the pattern nodes list box.
//
       case IDADD: // Button text: "&Add >>"
         nI = (int)SendMessage(hCtlALLNODES, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
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
         numSelected = (int)SendMessage(hCtlALLNODES, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
//
//  For the BASE pattern:  If a node is highlighted in the pattern nodes list box,
//  then the new node(s) is/are added after the highlighted entry.  If nothing in the
//  pattern nodes list box is highlighted, then the addition takes place at the front.
//  
//  Since this is a multiple selection listbox, consider the "highlighted entry"
//  to be the last highlighted entry in the list.
//
         if(bDisplaySystemNodes)
         {
           numPatternNodes = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
           pos = 0;
           if(numPatternNodes != 0)
           {
             for(nI = numPatternNodes - 1; nI >= 0; nI--)
             {
               if(SendMessage(hCtlPATTERNNODES, LB_GETSEL, (WPARAM)nI, (LPARAM)0) > 0)
               {
                 pos = nI + 1;
                 break;
               }
             }
           }  
           for(nI = 0; nI < numSelected; nI++)
           {
             NODESKey0.recordID = SendMessage(hCtlALLNODES, LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
             rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
             strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
             trim(tempString, NODES_ABBRNAME_LENGTH);
             if(NODES.flags & NODES_FLAG_STOP)
             {
               strcat(tempString, szStopString);
             }
             SendMessage(hCtlPATTERNNODES, LB_INSERTSTRING, (WPARAM)(pos + nI), (LONG)(LPSTR)tempString);
             for(bFound = FALSE, nJ = 0; nJ < numPAT; nJ++)
             {
               if(pPAT[nJ].NODESrecordID == NO_RECORD)
               {
                 bFound = TRUE;
                 break;
               }
             }
             if(!bFound)
             {
               nJ = numPAT;
             }
             if(numPAT >= maxPAT)
             {
               maxPAT += 1024;
               pPAT = (PATDef *)HeapReAlloc(GetProcessHeap(),
                     HEAP_ZERO_MEMORY, pPAT, sizeof(PATDef) * maxPAT); 
               if(pPAT == NULL)
               {
                 AllocationError(__FILE__, __LINE__, TRUE);
                 break;
               }
             }
             SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, pos + nI, (long)nJ);
             pPAT[nJ].NODESrecordID = NODES.recordID;
             pPAT[nJ].nodeSequence = NO_RECORD;
             strcpy(pPAT[nJ].nodeName, tempString);
             pPAT[nJ].bMLP = FALSE;
             pPAT[nJ].bBusStop = NODES.flags & NODES_FLAG_STOP;
             numPAT++;
           }
//
//  Highlight the last item in the list
//
           nI = SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
           if(nI > 0)
           {
             SendMessage(hCtlPATTERNNODES, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
             SendMessage(hCtlPATTERNNODES, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(nI - 1));
           }
         }
//
//  For other patterns: Add the nodes in the same relative positions as the BASE pattern
//
         else
         {
//
//  Get the base pattern
//
           maxBPN = 100;
           pBASEPatternNodeSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxBPN); 
           if(pBASEPatternNodeSeqs == NULL)
           {
             AllocationError(__FILE__, __LINE__, FALSE);
             TMSHeapFree(pSelected);
             break;
           }
           PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
           PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
           PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
           PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
           PATTERNSKey2.nodeSequence = NO_RECORD;
           rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           numNodes = 0;
           bBreak = FALSE;
           while(rcode2 == 0 &&
                 PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
                 PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
           {
             pBASEPatternNodeSeqs[numNodes] = PATTERNS.nodeSequence;
             numNodes++;
             if(numNodes >= maxBPN)
             {
               maxBPN += 100;
               pBASEPatternNodeSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                     HEAP_ZERO_MEMORY, pBASEPatternNodeSeqs, sizeof(long) * maxBPN); 
               if(pBASEPatternNodeSeqs == NULL)
               {
                 AllocationError(__FILE__, __LINE__, TRUE);
                 TMSHeapFree(pBASEPatternNodeSeqs);
                 TMSHeapFree(pSelected);
                 bBreak = TRUE;
                 break;
               }
             }
             rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           }
           if(bBreak)
           {
             break;
           }
//
//  See what's been added already and set up pbBASEPatternNodeSeqsSelected
//
           pbBASEPatternNodeSeqsSelected = (BOOL *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BOOL) * numNodes); 
           if(pbBASEPatternNodeSeqsSelected == NULL)
           {
             AllocationError(__FILE__, __LINE__, FALSE);
             TMSHeapFree(pBASEPatternNodeSeqs);
             TMSHeapFree(pSelected);
             break;
           }
           nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, 0, 0L);
           for(nI = 0; nI < nJ; nI++)
           {
             nL = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, nI, 0L);
             for(nK = 0; nK < numNodes; nK++)
             {
               if(pPAT[nL].nodeSequence == pBASEPatternNodeSeqs[nK])
               {
                 pbBASEPatternNodeSeqsSelected[nK] = TRUE;
                 break;
               }
             }
           }
//
//  And add the selections to pbBASEPatternNodeSeqsSelected
//
           for(nI = 0; nI < numSelected; nI++)
           {
             tempLong = SendMessage(hCtlALLNODES, LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
             for(nJ = 0; nJ < numNodes; nJ++)
             {
               if(tempLong == pBASEPatternNodeSeqs[nJ])
               {
                 pbBASEPatternNodeSeqsSelected[nJ] = TRUE;
                 break;
               }
             }
           }
//
//  Refresh the PATTERNNODES list box
//
           SendMessage(hCtlPATTERNNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Loop through pBASEPatternNodeSeqs Selected
//
           for(nI = 0; nI < numNodes; nI++)
           {
             if(pbBASEPatternNodeSeqsSelected[nI])
             {
//
//  See if we've got this node's data in PAT
//
               for(pos = NO_RECORD, nJ = 0; nJ < numPAT; nJ++)
               {
                 if(pBASEPatternNodeSeqs[nI] == pPAT[nJ].nodeSequence)
                 {
                   pos = nJ;
                   break;
                 }
               }
//
//  Nope - add it to PAT
//
               if(pos == NO_RECORD)
               {
                 if(numPAT >= maxPAT)
                 {
                   maxPAT += 1024;
                   pPAT = (PATDef *)HeapReAlloc(GetProcessHeap(),
                         HEAP_ZERO_MEMORY, pPAT, sizeof(PATDef) * maxPAT); 
                   if(pPAT == NULL)
                   {
                     AllocationError(__FILE__, __LINE__, TRUE);
                     break;
                   }
                 }
                 PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
                 PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
                 PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
                 PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                 PATTERNSKey2.nodeSequence = pBASEPatternNodeSeqs[nI];
                 btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                 NODESKey0.recordID = PATTERNS.NODESrecordID;
                 btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                 strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                 trim(abbrName, NODES_ABBRNAME_LENGTH);
                 strcpy(pPAT[numPAT].nodeName, abbrName);
                 pPAT[numPAT].nodeSequence = pBASEPatternNodeSeqs[nI];
                 pPAT[numPAT].NODESrecordID = PATTERNS.NODESrecordID;
                 pPAT[numPAT].bMLP = FALSE;
                 pPAT[numPAT].bBusStop = NODES.flags & NODES_FLAG_STOP;
                 pos = numPAT;
                 numPAT++;
               }
//
//  Set up the node name.
//
               strcpy(tempString, pPAT[pos].nodeName);
               if(pPAT[pos].bMLP)
               {
                 strcat(tempString, szMaxString);
               }
               else if(pPAT[pos].bBusStop)
               {
                 strcat(tempString, szStopString);
               }
//
//  Get a count of the entries in PATTERNNODES and loop through to insert
//  the nodes in the proper sequence
//
               nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
               for(nN = -1, nK = 0; nK < nJ - 1; nK++)
               {
                 nL = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
                 nM = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)(nK + 1), (LPARAM)0);
                 if(pPAT[pos].nodeSequence > pPAT[nL].nodeSequence &&
                       pPAT[pos].nodeSequence < pPAT[nM].nodeSequence)
                 {
                   nN = nK + 1;
                   break;
                 }
               }
//
//  Insert the node and its pointer into PAT
//
               nL = (int)SendMessage(hCtlPATTERNNODES, LB_INSERTSTRING, (WPARAM)nN, (LONG)(LPSTR)tempString);
               SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, (WPARAM)nL, (LPARAM)pos);
             }
           }
           TMSHeapFree(pBASEPatternNodeSeqs);
           TMSHeapFree(pbBASEPatternNodeSeqsSelected);
         }
         TMSHeapFree(pSelected);
         SendMessage(hCtlALLNODES, LB_SETSEL, (WPARAM)0, (LPARAM)(-1));
         EnableWindow(hCtlADD, FALSE);
         break;
//
//  IDREMOVE - Delete the highlighted item(s) from the pattern nodes list box
//
       case IDREMOVE: 
         nI = (int)SendMessage(hCtlPATTERNNODES, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
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
         numSelected = (int)SendMessage(hCtlPATTERNNODES, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
         for(nI = numSelected - 1; nI >= 0; nI--)
         {
           nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
           strcpy(pPAT[nJ].nodeName, "");
           pPAT[nJ].NODESrecordID = NO_RECORD;
           pPAT[nJ].nodeSequence = NO_RECORD;
           pPAT[nJ].bMLP = FALSE;
           pPAT[nJ].bBusStop = FALSE;
           SendMessage(hCtlPATTERNNODES, LB_DELETESTRING, (WPARAM)pSelected[nI], (LPARAM)0);
         }
         EnableWindow(hCtlREMOVE, FALSE);
         EnableWindow(hCtlSETMLP, FALSE);
         SendMessage(hCtlPATTERNNODES, LB_SETSEL, (WPARAM)0, (LPARAM)(-1));
         TMSHeapFree(pSelected);
         break;

//
//  IDASSIGN - Set the highlighted node as the MLP
//
       case IDASSIGN:  // Button text: "Set &MLP"
//
//  Get the highlighted item
//
         if((nI = (int)SendMessage(hCtlPATTERNNODES, LB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == LB_ERR)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_035, (HANDLE)NULL);
           break;
         }
//
//  The new MLP can't be just a stop
//
         nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         if(pPAT[nJ].bBusStop)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_221, (HANDLE)NULL);
           break;
         }
//
//  Clear out the previous MLP
//
         nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
         for(nK = 0; nK < nJ; nK++)
         {
           nL = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
           if(nL != LB_ERR && pPAT[nL].bMLP)
           {
             SendMessage(hCtlPATTERNNODES, LB_DELETESTRING, (WPARAM)nK, (LPARAM)0);
             SendMessage(hCtlPATTERNNODES, LB_INSERTSTRING, (WPARAM)nK, (LONG)(LPSTR)pPAT[nL].nodeName);
             SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, (WPARAM)nK, (LPARAM)nL);
             pPAT[nL].bMLP = FALSE;
             break;
           }
         }
//
//  Set the new MLP
//
         nJ = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         pPAT[nJ].bMLP = TRUE;
         strcpy(tempString, pPAT[nJ].nodeName);
         strcat(tempString, szMaxString);
         SendMessage(hCtlPATTERNNODES, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
         SendMessage(hCtlPATTERNNODES, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
         SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)nJ);
         SendMessage(hCtlPATTERNNODES, LB_SETSEL, (WPARAM)0, (LPARAM)(-1));
         EnableWindow(hCtlREMOVE, FALSE);
         EnableWindow(hCtlSETMLP, FALSE);
         break;
//
//  IDLOADREVERSE - Load the reverse of the other direction's BASE pattern
//
       case IDLOADREVERSE:
//
//  Clear out aything that's been put into the pattern nodes listbox
//
         SendMessage(hCtlPATTERNNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
         numPAT = 0;
//
//  Go through the other BASE pattern in reverse
//
         nI = (pDI->fileInfo.directionIndex + 1) % 2;  // The other direction index
         PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
         PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
         PATTERNSKey2.directionIndex = nI;
         PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID + 1;
         PATTERNSKey2.nodeSequence = NO_RECORD;
         pos = 0;
         rcode2 = btrieve(B_GETLESSTHAN, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
         while(rcode2 == 0 &&
               PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
               PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
               PATTERNS.directionIndex == nI &&
               PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
         {
           NODESKey0.recordID = PATTERNS.NODESrecordID;
           rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
           strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
           trim(tempString, NODES_ABBRNAME_LENGTH);
           if(NODES.flags & NODES_FLAG_STOP)
           {
             strcat(tempString, szStopString);
           }
           SendMessage(hCtlPATTERNNODES, LB_INSERTSTRING, (WPARAM)pos, (LONG)(LPSTR)tempString);
           nJ = numPAT;
           if(numPAT >= maxPAT)
           {
             maxPAT += 1024;
             pPAT = (PATDef *)HeapReAlloc(GetProcessHeap(),
                   HEAP_ZERO_MEMORY, pPAT, sizeof(PATDef) * maxPAT); 
             if(pPAT == NULL)
             {
               AllocationError(__FILE__, __LINE__, TRUE);
               break;
             }
           }
           SendMessage(hCtlPATTERNNODES, LB_SETITEMDATA, (WPARAM)pos, (long)nJ);
           pPAT[nJ].NODESrecordID = NODES.recordID;
           pPAT[nJ].nodeSequence = NO_RECORD;
           strcpy(pPAT[nJ].nodeName, tempString);
           pPAT[nJ].bMLP = FALSE;
           pPAT[nJ].bBusStop = NODES.flags & NODES_FLAG_STOP;
           numPAT++;
           pos++;
           rcode2 = btrieve(B_GETPREVIOUS, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
         }
         break;
//
//  IDOK - Save and get out
//
       case IDOK:
//
//  Must have at least one node in the pattern
//
         if(SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_072, hCtlALLNODES);
           break;
         }
//
//  Must have set the MLP
//
         for(bFound = FALSE, nI = 0; nI < numPAT; nI++)
         {
           if(pPAT[nI].NODESrecordID != NO_RECORD && pPAT[nI].bMLP)
           {
             bFound = TRUE;
             break;
           }
         }
         if(!bFound)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_073, hCtlPATTERNNODES);
           break;
         }
//
//  Must have a pattern name
//
         SendMessage(hCtlNAME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
         if(strcmp(tempString, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_028, hCtlNAME);
           break;
         }
//
//  See if the pattern name exists in PATTERNNAMES, and add it if it doesn't
//
         strncpy(PATTERNNAMESKey1.name, tempString, PATTERNNAMES_NAME_LENGTH);
         pad(PATTERNNAMESKey1.name, PATTERNNAMES_NAME_LENGTH);
         rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
         if(rcode2 != 0)
         {
           rcode2 = btrieve(B_GETLAST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
           PATTERNNAMES.recordID = AssignRecID(rcode2, PATTERNNAMES.recordID);
           strcpy(PATTERNNAMES.name, tempString);
           pad(PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
           btrieve(B_INSERT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
         }
//
//  But if it did exist, make sure that he's not trying to add a pattern that already exists
//
         else
         {
           if(updateRecordID == NO_RECORD)
           {
             PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
             PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
             PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
             PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMES.recordID;
             PATTERNSKey2.nodeSequence = NO_RECORD;
             rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             if(rcode2 == 0 &&
                   PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                   PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                   PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
                   PATTERNS.PATTERNNAMESrecordID == PATTERNNAMES.recordID)
             {
               TMSError(hWndDlg, MB_ICONSTOP, ERROR_074, hCtlNAME);
               break;
             }
           }
         }
//
//  On an update, nuke the previous pattern
//
         saveCursor = SetCursor(hCursorWait);
         if(updateRecordID != NO_RECORD)
         {
           PATTERNSKey0.recordID = updateRecordID;
           btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
           btrieve(B_GETPOSITION, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
           rcode2 = btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           thisPatternName = PATTERNS.PATTERNNAMESrecordID;
           while(rcode2 == 0 &&
                 PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 PATTERNS.directionIndex == pDI->fileInfo.directionIndex &&
                 PATTERNSKey2.PATTERNNAMESrecordID == thisPatternName)
           {
             btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           }
         }
//
//  Now, do the insert
//
         rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
         PATTERNS.recordID = AssignRecID(rcode2, PATTERNS.recordID);
         PATTERNS.ROUTESrecordID = pDI->fileInfo.routeRecordID;
         PATTERNS.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
         PATTERNS.directionIndex = pDI->fileInfo.directionIndex;
         PATTERNS.PATTERNNAMESrecordID = PATTERNNAMES.recordID;
         nI = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
         previousSeq = 0;
         for(nJ = 0; nJ < nI; nJ++)
         {
           nK = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
           PATTERNS.NODESrecordID = pPAT[nK].NODESrecordID;
           if(pPAT[nK].nodeSequence == NO_RECORD)
           {
             for(divisor = 2, bFound = FALSE, nL = nJ + 1; nL < nI; nL++, divisor++)
             {
               nM = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nL, (LPARAM)0);
               if(pPAT[nM].nodeSequence != NO_RECORD)
               {
                 bFound = TRUE;
                 break;
               }
             }
             if(!bFound)
               pPAT[nK].nodeSequence = previousSeq + 1000;
             else
               pPAT[nK].nodeSequence = previousSeq + ((pPAT[nM].nodeSequence - previousSeq) / divisor);
           }
           PATTERNS.nodeSequence = pPAT[nK].nodeSequence;
           previousSeq = PATTERNS.nodeSequence;
           if(nJ == 0)
           {
             if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
             {
              PATTERNS.COMMENTSrecordID = NO_RECORD;
             }
             else
             {
               nM = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
               PATTERNS.COMMENTSrecordID = (nM == CB_ERR ? NO_RECORD : SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nM, (LPARAM)0));
             }
             SendMessage(hCtlFROMTEXT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
             pad(tempString, PATTERNS_FROMTEXT_LENGTH);
             strncpy(PATTERNS.fromText, tempString, PATTERNS_FROMTEXT_LENGTH);
             SendMessage(hCtlTOTEXT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
             pad(tempString, PATTERNS_TOTEXT_LENGTH);
             strncpy(PATTERNS.toText, tempString, PATTERNS_TOTEXT_LENGTH);
           }
           PATTERNS.flags = 0;
           if(pPAT[nK].bMLP)
           {
             PATTERNS.flags |= PATTERNS_FLAG_MLP;
           }
           if(pPAT[nK].bBusStop)
           {
             PATTERNS.flags |= PATTERNS_FLAG_BUSSTOP;
           }
           memset(PATTERNS.reserved, 0x00, PATTERNS_RESERVED_LENGTH);
           btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
           if(updateRecordID == NO_RECORD && PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
             firstEntryOfNewBase = PATTERNS.recordID;
           PATTERNS.recordID++;
         }
//
//  If he updated the BASE pattern, make sure that none of the sub-patterns
//  contain a node that no longer exists.
//
         if(PATTERNNAMES.recordID == basePatternRecordID && updateRecordID != NO_RECORD)
         {
           PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
           PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
           PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
           PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID + 1;
           PATTERNSKey2.nodeSequence = NO_RECORD;
           rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           nI = (int)SendMessage(hCtlPATTERNNODES, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
           while(rcode2 == 0 &&
                 PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                 PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                 PATTERNS.directionIndex == pDI->fileInfo.directionIndex)
           {
             for(bFound = FALSE, nJ = 0; nJ < nI; nJ++)
             {
               nK = (int)SendMessage(hCtlPATTERNNODES, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
               if(PATTERNS.NODESrecordID == pPAT[nK].NODESrecordID)
               {
                 bFound = TRUE;
                 break;
               }
             }
             if(!bFound)
             {
               if(PATTERNS.flags & PATTERNS_FLAG_MLP)
               {
                 PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
                 btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                 strncpy(tempString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
                 trim(tempString, sizeof(PATTERNNAMES.name));
                 MessageBeep(MB_ICONINFORMATION);
                 LoadString(hInst, ERROR_139, szFormatString, sizeof(szFormatString));
                 sprintf(szarString, szFormatString, tempString);
                 MessageBox(hWndDlg, szarString, TMS, MB_ICONINFORMATION);
               }
               btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             }
             rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
           }
         }
//
//  In case we're adding a new base pattern...
//
         if(updateRecordID == NO_RECORD)
         {
           updateRecordID = firstEntryOfNewBase;
         }
//
//  Salright
//
         TMSHeapFree(pPAT);
         TMSHeapFree(pNOD);
         SetCursor(saveCursor);
         EndDialog(hWndDlg, TRUE);
         break;
//
//  IDCANCEL
//
       case IDCANCEL:
         TMSHeapFree(pPAT);
         TMSHeapFree(pNOD);
         EndDialog(hWndDlg, FALSE);
         break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Patterns_Table);
          break;
     }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
 }
 return TRUE;
} //  End of ADDPATTERNMsgProc

