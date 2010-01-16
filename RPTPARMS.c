//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define DEFAULT_PATTERN "BUSBOOK"

int RptSetupBlockList(long, long, HANDLE, REPORTPARMSDef *);

BOOL CALLBACK RPTPARMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static REPORTPARMSDef *pRP;
  static HANDLE hCtlROUTEGROUPBOX;
  static HANDLE hCtlALLROUTES;
  static HANDLE hCtlALLLABELLED;
  static HANDLE hCtlROUTESLABELLED;
  static HANDLE hCtlROUTE;
  static HANDLE hCtlSPECIFICROUTE;
  static HANDLE hCtlSERVICEGROUPBOX;
  static HANDLE hCtlALLSERVICES;
  static HANDLE hCtlSERVICE;
  static HANDLE hCtlSPECIFICSERVICE;
  static HANDLE hCtlDIVISIONGROUPBOX;
  static HANDLE hCtlALLDIVISIONS;
  static HANDLE hCtlDIVISION;
  static HANDLE hCtlSPECIFICDIVISION;
  static HANDLE hCtlPATTERNGROUPBOX;
  static HANDLE hCtlPATTERNNAME_TEXT;
  static HANDLE hCtlSPECIFICPATTERN;
  static HANDLE hCtlNODEGROUPBOX;
  static HANDLE hCtlNODEABBR;
  static HANDLE hCtlSPECIFICNODE;
  static HANDLE hCtlNODELABEL;
  static HANDLE hCtlSPECIFICLABEL;
  static HANDLE hCtlRUNGROUPBOX;
  static HANDLE hCtlALLRUNS;
  static HANDLE hCtlRUN;
  static HANDLE hCtlSPECIFICRUN;
  static HANDLE hCtlBLOCKGROUPBOX;
  static HANDLE hCtlALLBLOCKS;
  static HANDLE hCtlBLOCK;
  static HANDLE hCtlSPECIFICBLOCK;
  static HANDLE hCtlOUTPUT;
  static HANDLE hCtlMAXLINES_TEXT;
  static HANDLE hCtlMAXLINES;
  static HANDLE hCtlMAXCOLS_TEXT;
  static HANDLE hCtlMAXCOLS;
  static HANDLE hCtlOTHERFLAGSGROUPBOX;
  static HANDLE hCtlSHOWSUMMARYOFSIGNCODES;
  static HANDLE hCtlINCLUDETRIPNUMBERS;
  static HANDLE hCtlSPECIFICTRIPGROUPBOX;
  static HANDLE hCtlTRIPNUMBER_TEXT;
  static HANDLE hCtlTRIPNUMBER;
  static BLOCKSDef *pTRIPSChunk;
  static int    keyNumber;
  BOOL   bFound;
  BOOL   bFirst;
  BOOL   bGotBusbook;
  char   dummy[256];
  long   assignedToNODESrecordID;
  long   recordID;
  long   tempLong;
  long   divisionRecordID;
  long   serviceRecordID;
  long   busbookPatternRecordID;
  int    maxRoutes;
  int    maxServices;
  int    maxDivisions;
  int    maxBlocks;
  int    maxRuns;
  int    nI;
  int    nJ;
  int    nK;
  int    numLabels;
  int    rcode2;
  int    basePatternPos;
  int    numPatterns;
  short  int wmId;
  short  int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pRP = (REPORTPARMSDef *)lParam;
      if(pRP == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      if(pRP->flags & RPFLAG_STANDARDBLOCKS)
      {
        pTRIPSChunk = &TRIPS.standard;
        keyNumber = 2;
      }
      else
      {
        pTRIPSChunk = &TRIPS.dropback;
        keyNumber = 3;
      }
//
//  Set up the handles to the controls
//
      hCtlROUTEGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_ROUTEGROUPBOX);
      hCtlALLROUTES = GetDlgItem(hWndDlg, RPTPARMS_ALLROUTES);
      hCtlALLLABELLED = GetDlgItem(hWndDlg, RPTPARMS_ALLLABELLED);
      hCtlROUTESLABELLED = GetDlgItem(hWndDlg, RPTPARMS_ROUTESLABELLED);
      hCtlROUTE = GetDlgItem(hWndDlg, RPTPARMS_ROUTE);
      hCtlSPECIFICROUTE = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICROUTE);
      hCtlSERVICEGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_SERVICEGROUPBOX);
      hCtlALLSERVICES = GetDlgItem(hWndDlg, RPTPARMS_ALLSERVICES);
      hCtlSERVICE = GetDlgItem(hWndDlg, RPTPARMS_SERVICE);
      hCtlSPECIFICSERVICE = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICSERVICE);
      hCtlDIVISIONGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_DIVISIONGROUPBOX);
      hCtlALLDIVISIONS = GetDlgItem(hWndDlg, RPTPARMS_ALLDIVISIONS);
      hCtlDIVISION = GetDlgItem(hWndDlg, RPTPARMS_DIVISION);
      hCtlSPECIFICDIVISION = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICDIVISION);
      hCtlPATTERNGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_PATTERNGROUPBOX);
      hCtlPATTERNNAME_TEXT = GetDlgItem(hWndDlg, RPTPARMS_PATTERNNAME_TEXT);
      hCtlSPECIFICPATTERN = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICPATTERN);
      hCtlNODEGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_NODEGROUPBOX);
      hCtlNODEABBR = GetDlgItem(hWndDlg, RPTPARMS_NODEABBR);
      hCtlSPECIFICNODE = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICNODE);
      hCtlNODELABEL = GetDlgItem(hWndDlg, RPTPARMS_NODELABEL);
      hCtlSPECIFICLABEL = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICLABEL);
      hCtlRUNGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_RUNGROUPBOX);
      hCtlALLRUNS = GetDlgItem(hWndDlg, RPTPARMS_ALLRUNS);
      hCtlRUN = GetDlgItem(hWndDlg, RPTPARMS_RUN);
      hCtlSPECIFICRUN = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICRUN);
      hCtlBLOCKGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_BLOCKGROUPBOX);
      hCtlALLBLOCKS = GetDlgItem(hWndDlg, RPTPARMS_ALLBLOCKS);
      hCtlBLOCK = GetDlgItem(hWndDlg, RPTPARMS_BLOCK);
      hCtlSPECIFICBLOCK = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICBLOCK);
      hCtlOUTPUT = GetDlgItem(hWndDlg, RPTPARMS_OUTPUT);
      hCtlMAXLINES_TEXT = GetDlgItem(hWndDlg, RPTPARMS_MAXLINES_TEXT);
      hCtlMAXLINES = GetDlgItem(hWndDlg, RPTPARMS_MAXLINES);
      hCtlMAXCOLS_TEXT = GetDlgItem(hWndDlg, RPTPARMS_MAXCOLS_TEXT);
      hCtlMAXCOLS = GetDlgItem(hWndDlg, RPTPARMS_MAXCOLS);
      hCtlOTHERFLAGSGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_OTHERFLAGSGROUPBOX);
      hCtlSHOWSUMMARYOFSIGNCODES = GetDlgItem(hWndDlg, RPTPARMS_SHOWSUMMARYOFSIGNCODES);
      hCtlINCLUDETRIPNUMBERS = GetDlgItem(hWndDlg, RPTPARMS_INCLUDETRIPNUMBERS);
      hCtlSPECIFICTRIPGROUPBOX = GetDlgItem(hWndDlg, RPTPARMS_SPECIFICTRIPGROUPBOX);
      hCtlTRIPNUMBER_TEXT = GetDlgItem(hWndDlg, RPTPARMS_TRIPNUMBER_TEXT);
      hCtlTRIPNUMBER = GetDlgItem(hWndDlg, RPTPARMS_TRIPNUMBER);

      pRP->returnedFlags = 0;
//
//  Set the dialog title
//
//  TMSRPT has been sorted - locate the original report number
//
      for(nI = 0; nI < m_LastReport; nI++)
      {
        if(TMSRPT[nI].originalReportNumber == pRP->nReportNumber)
        {
          SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
          sprintf(tempString, szFormatString, TMSRPT[nI].szReportName);
          SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          break;
        }
      }
//
//  Set up only what's requested, and disable the rest
//
//  Route requested
//
      if(pRP->flags & RPFLAG_ROUTES)
      {
        if(pRP->flags & RPFLAG_NOALLROUTES)
        {
          EnableWindow(hCtlALLROUTES, FALSE);
        }
        if(SetUpRouteList(hWndDlg, RPTPARMS_SPECIFICROUTE, pRP->ROUTESrecordID) == 0)
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
//
//  If he hasn't picked a route, choose the first one in the combo box
//
        if(m_RouteRecordID == NO_RECORD)
        {
          SendMessage(hCtlSPECIFICROUTE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        }
        SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Route not requested
//
      else
      {
        EnableWindow(hCtlROUTEGROUPBOX, FALSE);
        EnableWindow(hCtlALLROUTES, FALSE);
        EnableWindow(hCtlALLLABELLED, FALSE);
        EnableWindow(hCtlROUTESLABELLED, FALSE);
        EnableWindow(hCtlROUTE, FALSE);
        EnableWindow(hCtlSPECIFICROUTE, FALSE);
      }
//
//  Comments (only used when RPFLAG_ROUTES is in effect)
//
      if(pRP->flags & RPFLAG_COMMENTS)
      {
//
//  Build a unique list of route comment codes
//
        rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        while(rcode2 == 0)
        {
          if(ROUTES.COMMENTSrecordID != NO_RECORD)
          {
            nJ = (int)SendMessage(hCtlROUTESLABELLED, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            for(bFound = FALSE, nI = 0; nI < nJ; nI++)
            {
              if(SendMessage(hCtlROUTESLABELLED, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == ROUTES.COMMENTSrecordID)
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
              nI = (int)SendMessage(hCtlROUTESLABELLED, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
              SendMessage(hCtlROUTESLABELLED, CB_SETITEMDATA, (WPARAM)nI, ROUTES.COMMENTSrecordID);
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        }
//
//  If we found at least one, make it the current selection
//
        if(SendMessage(hCtlROUTESLABELLED, CB_GETCOUNT, (WPARAM)0, (LPARAM)0) > 0)
        {
          SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        }
//
//  No comments
//
        else
        {
          EnableWindow(hCtlALLLABELLED, FALSE);
          EnableWindow(hCtlROUTESLABELLED, FALSE);
        }
      }
//
//  Services requested
//
      if(pRP->flags & RPFLAG_SERVICES)
      {
        if(pRP->flags & RPFLAG_NOALLSERVICES)
        {
          EnableWindow(hCtlALLSERVICES, FALSE);
        }
        if(SetUpServiceList(hWndDlg, RPTPARMS_SPECIFICSERVICE, pRP->SERVICESrecordID) == 0)
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
        SendMessage(hCtlSERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Services not requested
//
      else
      {
        EnableWindow(hCtlSERVICEGROUPBOX, FALSE);
        EnableWindow(hCtlALLSERVICES, FALSE);
        EnableWindow(hCtlSERVICE, FALSE);
        EnableWindow(hCtlSPECIFICSERVICE, FALSE);
      }
//
//  Divisions requested
//
      if(pRP->flags & RPFLAG_DIVISIONS)
      {
        if(pRP->flags & RPFLAG_NOALLDIVISIONS)
        {
          EnableWindow(hCtlALLDIVISIONS, FALSE);
        }
        if(SetUpDivisionList(hWndDlg, RPTPARMS_SPECIFICDIVISION, pRP->DIVISIONSrecordID) == 0)
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_080, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
        SendMessage(hCtlDIVISION, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      }
//
//  Divisions not requested
//
      else
      {
        EnableWindow(hCtlDIVISIONGROUPBOX, FALSE);
        EnableWindow(hCtlALLDIVISIONS, FALSE);
        EnableWindow(hCtlDIVISION, FALSE);
        EnableWindow(hCtlSPECIFICDIVISION, FALSE);
      }
//
//  Pattern names
//
      numPatterns = 0;
      if(pRP->flags & RPFLAG_PATTERNNAMES)
      {
        rcode2 = btrieve(B_GETFIRST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
        busbookPatternRecordID = NO_RECORD;
        while(rcode2 == 0)
        {
          strncpy(tempString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
          trim(tempString, sizeof(PATTERNNAMES.name));
          if(strcmp(tempString, DEFAULT_PATTERN) == 0)
          {
            busbookPatternRecordID = PATTERNNAMES.recordID;
          }
          nI = (int)SendMessage(hCtlSPECIFICPATTERN, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtlSPECIFICPATTERN, CB_SETITEMDATA, (WPARAM)nI, PATTERNNAMES.recordID);
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          numPatterns++;
        }
//
//  Since it was sorted, look for BUSBOOK / BASE
//
        bGotBusbook = FALSE;
        for(nI = 0; nI < numPatterns; nI++)
        {
          tempLong = SendMessage(hCtlSPECIFICPATTERN, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          if(tempLong == busbookPatternRecordID)
          {
            SendMessage(hCtlSPECIFICPATTERN, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
            bGotBusbook = TRUE;
            break;
          }
          else if(tempLong == basePatternRecordID)
          {
            basePatternPos = nI;
          }
        }
        if(!bGotBusbook)
        {
          SendMessage(hCtlSPECIFICPATTERN, CB_SETCURSEL, (WPARAM)basePatternPos, (LPARAM)0);
        }
      }
      else
      {
        EnableWindow(hCtlPATTERNGROUPBOX, FALSE);
        EnableWindow(hCtlPATTERNNAME_TEXT, FALSE);
        EnableWindow(hCtlSPECIFICPATTERN, FALSE);
      }
//
//  Nodes
//
      if(pRP->flags & RPFLAG_NODES)
      {
        if(SetUpNodeList(hWndDlg, RPTPARMS_SPECIFICNODE, FALSE) == 0)
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
        SendMessage(hCtlNODEABBR, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlSPECIFICNODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        strcpy(szarString, "");
        rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
        numLabels = 0;
        while(rcode2 == 0)
        {
          strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
          for(nI = 0; nI < NODES_RELIEFLABELS_LENGTH; nI++)
          {
            if(tempString[nI] == ' ' || tempString[nI] == '-')
            {
              continue;
            }
            for(bFound = FALSE, nJ = 0; nJ < numLabels; nJ++)
            {
              if(tempString[nI] == szarString[nJ])
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              szarString[numLabels] = tempString[nI];
              numLabels++;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
        }
        if(numLabels == 0)
        {
          EnableWindow(hCtlNODELABEL, FALSE);
          EnableWindow(hCtlSPECIFICLABEL, FALSE);
        }
        else
        {
          for(nI = 0; nI < numLabels; nI++)
          {
            tempString[0] = szarString[nI];
            tempString[1] = '\0';
            SendMessage(hCtlSPECIFICLABEL, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          }
        }
      }
      else
      {
        EnableWindow(hCtlNODEGROUPBOX, FALSE);
        EnableWindow(hCtlNODEABBR, FALSE);
        EnableWindow(hCtlSPECIFICNODE, FALSE);
        EnableWindow(hCtlNODELABEL, FALSE);
        EnableWindow(hCtlSPECIFICLABEL, FALSE);
      }
//
//  Runs
//
      if(pRP->flags & RPFLAG_RUNS)
      {
        if(pRP->flags & RPFLAG_NOALLRUNS)
        {
          EnableWindow(hCtlALLRUNS, FALSE);
        }
        else
        {
          SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
        if(SendMessage(hCtlSERVICE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
        {
          nI = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)  // This should never occur
          {
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
            break;
          }
          serviceRecordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)  // This should never occur
          {
            SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
            break;
          }
          divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          if((SetUpRunList(hWndDlg, divisionRecordID, serviceRecordID, RPTPARMS_SPECIFICRUN)) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_193, (HANDLE)hCtlSPECIFICSERVICE);
            break;
          }
        }
      }
      else
      {
        EnableWindow(hCtlRUNGROUPBOX, FALSE);
        EnableWindow(hCtlALLRUNS, FALSE);
        EnableWindow(hCtlRUN, FALSE);
        EnableWindow(hCtlSPECIFICRUN, FALSE);
      }
//
//  Block numbers
//
      if(pRP->flags & RPFLAG_BLOCKS)
      {
        nI = RptSetupBlockList(m_RouteRecordID, m_ServiceRecordID, hCtlSPECIFICBLOCK, pRP);
        if(nI == IDCANCEL)
        {
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
        if(pRP->flags & RPFLAG_NOALLBLOCKS)
        {
          EnableWindow(hCtlALLBLOCKS, FALSE);
        }
        else
        {
          SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
      }
      else
      {
        EnableWindow(hCtlBLOCKGROUPBOX, FALSE);
        EnableWindow(hCtlALLBLOCKS, FALSE);
        EnableWindow(hCtlBLOCK, FALSE);
        EnableWindow(hCtlSPECIFICBLOCK, FALSE);
      }
//
//  Output characteristics
//
//  Used only for the block and run paddles (landscape, legal)
//
      if(pRP->flags & RPFLAG_OUTPUT)
      {
        itoa(TMSRPTMaxLinesPerPage, tempString, 10);
        SendMessage(hCtlMAXLINES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
        itoa(TMSRPTMaxColsPerPage, tempString, 10);
        SendMessage(hCtlMAXCOLS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
      else
      {
        EnableWindow(hCtlOUTPUT, FALSE);
        EnableWindow(hCtlMAXLINES_TEXT, FALSE);
        EnableWindow(hCtlMAXLINES, FALSE);
        SendMessage(hCtlMAXLINES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
        EnableWindow(hCtlMAXCOLS_TEXT, FALSE);
        EnableWindow(hCtlMAXCOLS, FALSE);
        SendMessage(hCtlMAXCOLS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
      }
//
//  Specific trip number
//
      if(pRP->flags & RPFLAG_SPECIFICTRIPNUMBER)
      {
        
      }
      else
      {
        EnableWindow(hCtlSPECIFICTRIPGROUPBOX, FALSE);
        EnableWindow(hCtlTRIPNUMBER_TEXT, FALSE);
        EnableWindow(hCtlTRIPNUMBER, FALSE);
      }
//
//  Other flags
//
      if((pRP->flags & RPFLAG_SIGNCODES) || (pRP->flags & RPFLAG_TRIPNUMBERS))
      {
        EnableWindow(hCtlOTHERFLAGSGROUPBOX, TRUE);
        EnableWindow(hCtlSHOWSUMMARYOFSIGNCODES, (pRP->flags & RPFLAG_SIGNCODES));
        EnableWindow(hCtlINCLUDETRIPNUMBERS, (pRP->flags & RPFLAG_TRIPNUMBERS));
      }
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
//  Routes
//
//  RPTPARMS_ALLROUTES
//
        case RPTPARMS_ALLROUTES:
          if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            SendMessage(hCtlSPECIFICROUTE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            if(pRP->flags & RPFLAG_BLOCKS)
            {
              SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlBLOCK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              EnableWindow(hCtlBLOCK, FALSE);
              EnableWindow(hCtlSPECIFICBLOCK, FALSE);
            }
          }
          break;
//
//  RPTPARMS_ALLLABELLED
//
        case RPTPARMS_ALLLABELLED:
          if(SendMessage(hCtlALLLABELLED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)(0), (LPARAM)0);
            SendMessage(hCtlSPECIFICROUTE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            if(pRP->flags & RPFLAG_BLOCKS)
            {
              SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlBLOCK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              EnableWindow(hCtlBLOCK, FALSE);
              EnableWindow(hCtlSPECIFICBLOCK, FALSE);
            }
          }
          break;
//
//  RPTPARMS_ROUTESLABELLED
//
        case RPTPARMS_ROUTESLABELLED:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                    SendMessage(hCtlROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLROUTES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlALLLABELLED, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                SendMessage(hCtlSPECIFICROUTE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
              }
              if(pRP->flags & RPFLAG_BLOCKS)
              {
                SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                SendMessage(hCtlBLOCK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                EnableWindow(hCtlBLOCK, FALSE);
                EnableWindow(hCtlSPECIFICBLOCK, FALSE);
              }
              break;  
          }
          break;
//
//  RPTPARMS_ROUTE
//
        case RPTPARMS_ROUTE:
          if(SendMessage(hCtlROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
            SendMessage(hCtlSPECIFICROUTE, CB_SETCURSEL, (WPARAM)(0), (LPARAM)0);
            if(pRP->flags & RPFLAG_BLOCKS)
            {
              SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              tempLong = SendMessage(hCtlSPECIFICROUTE, CB_GETITEMDATA, (WPARAM)0, (LPARAM)0);
              nJ = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nJ == CB_ERR)
              {
                break;
              }
              recordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
              if(RptSetupBlockList(tempLong, recordID, hCtlSPECIFICBLOCK, pRP) == IDCANCEL)
              {
                break;
              }
              EnableWindow(hCtlBLOCK, TRUE);
              EnableWindow(hCtlSPECIFICBLOCK, TRUE);
            }
          }
          break;
//
//  RPTPARMS_SPECIFICROUTE
//
        case RPTPARMS_SPECIFICROUTE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                    SendMessage(hCtlALLLABELLED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLROUTES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlALLLABELLED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
              }
              if(pRP->flags & RPFLAG_BLOCKS)
              {
                SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                nI = (int)SendMessage(hCtlSPECIFICROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                tempLong = SendMessage(hCtlSPECIFICROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                nJ = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                if(nJ == CB_ERR)
                {
                  break;
                }
                recordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                if(RptSetupBlockList(tempLong, recordID, hCtlSPECIFICBLOCK, pRP) == IDCANCEL)
                {
                  break;
                }
                EnableWindow(hCtlBLOCK, TRUE);
                EnableWindow(hCtlSPECIFICBLOCK, TRUE);
              }
              break;  
          }
          break;
//
//  Services
//
//  RPTPARMS_ALLSERVICES
//
        case RPTPARMS_ALLSERVICES:
          if(SendMessage(hCtlALLSERVICES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            if(pRP->flags & RPFLAG_BLOCKS)
            {
              SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlBLOCK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              EnableWindow(hCtlBLOCK, FALSE);
              EnableWindow(hCtlSPECIFICBLOCK, FALSE);
            }
            if(pRP->flags & RPFLAG_RUNS)
            {
              SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlRUN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICRUN, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              EnableWindow(hCtlRUN, FALSE);
              EnableWindow(hCtlSPECIFICRUN, FALSE);
            }
            SendMessage(hCtlSPECIFICSERVICE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          }
          break;
//
//  RPTPARMS_SERVICE
//
        case RPTPARMS_SERVICE:
          if(SendMessage(hCtlSERVICE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlSPECIFICSERVICE, CB_SETCURSEL, (WPARAM)(0), (LPARAM)0);
            if(pRP->flags & RPFLAG_BLOCKS)
            {
              SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              nI = (int)SendMessage(hCtlSPECIFICROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                break;
              }
              tempLong = SendMessage(hCtlSPECIFICROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              nJ = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              recordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
              if(RptSetupBlockList(tempLong, recordID, hCtlSPECIFICBLOCK, pRP) == IDCANCEL)
              {
                break;
              }
              EnableWindow(hCtlBLOCK, TRUE);
              EnableWindow(hCtlSPECIFICBLOCK, TRUE);
            }
            if(pRP->flags & RPFLAG_RUNS)
            {
              serviceRecordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)0, (LPARAM)0);
              nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)  // This should never occur
              {
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              if((SetUpRunList(hWndDlg, divisionRecordID, serviceRecordID, RPTPARMS_SPECIFICRUN)) == 0)
              {
               TMSError(hWndDlg, MB_ICONSTOP, ERROR_193, (HANDLE)hCtlSPECIFICSERVICE);
               break;
              }
              EnableWindow(hCtlRUN, TRUE);
              EnableWindow(hCtlSPECIFICRUN, TRUE);
            }
          }
          break;
//
//  RPTPARMS_SPECIFICSERVICE
//
        case RPTPARMS_SPECIFICSERVICE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlALLSERVICES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLSERVICES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSERVICE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              if(pRP->flags & RPFLAG_BLOCKS)
              {
                SendMessage(hCtlSPECIFICBLOCK, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                nI = (int)SendMessage(hCtlSPECIFICROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                if(nI == CB_ERR)
                {
                  break;
                }
                tempLong = SendMessage(hCtlSPECIFICROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                nJ = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                recordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                if(RptSetupBlockList(tempLong, recordID, hCtlSPECIFICBLOCK, pRP) == IDCANCEL)
                {
                  break;
                }
                EnableWindow(hCtlBLOCK, TRUE);
                EnableWindow(hCtlSPECIFICBLOCK, TRUE);
              }
              if(pRP->flags & RPFLAG_RUNS)
              {
                nI = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)(0), (LPARAM)0);
                if(nI == CB_ERR)  // This should never occur
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                serviceRecordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                if(nI == CB_ERR)  // This should never occur
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                if((SetUpRunList(hWndDlg, divisionRecordID, serviceRecordID, RPTPARMS_SPECIFICRUN)) == 0)
                {
                  TMSError(hWndDlg, MB_ICONSTOP, ERROR_193, (HANDLE)hCtlSPECIFICSERVICE);
                  break;
                }
                EnableWindow(hCtlRUN, TRUE);
                EnableWindow(hCtlSPECIFICRUN, TRUE);
              }
              break;  
          }
          break;
//
//  Divisions
//
//  RPTPARMS_ALLDIVISIONS
//
        case RPTPARMS_ALLDIVISIONS:
          if(SendMessage(hCtlALLDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            if(pRP->flags & RPFLAG_RUNS)
            {
              SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              SendMessage(hCtlRUN, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
              SendMessage(hCtlSPECIFICRUN, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              EnableWindow(hCtlRUN, FALSE);
              EnableWindow(hCtlSPECIFICRUN, FALSE);
            }
            SendMessage(hCtlSPECIFICDIVISION, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          }
          break;
//
//  RPTPARMS_DIVISION
//
        case RPTPARMS_DIVISION:
          if(SendMessage(hCtlDIVISION, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlSPECIFICDIVISION, CB_SETCURSEL, (WPARAM)(0), (LPARAM)0);
            if(pRP->flags & RPFLAG_RUNS)
            {
              serviceRecordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)0, (LPARAM)0);
              nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)  // This should never occur
              {
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              if((SetUpRunList(hWndDlg, divisionRecordID, serviceRecordID, RPTPARMS_SPECIFICRUN)) == 0)
              {
               TMSError(hWndDlg, MB_ICONSTOP, ERROR_193, (HANDLE)hCtlSPECIFICDIVISION);
               break;
              }
              EnableWindow(hCtlRUN, TRUE);
              EnableWindow(hCtlSPECIFICRUN, TRUE);
            }
          }
          break;
//
//  RPTPARMS_SPECIFICDIVISION
//
        case RPTPARMS_SPECIFICDIVISION:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlALLDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLDIVISIONS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlDIVISION, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              if(pRP->flags & RPFLAG_RUNS)
              {
                nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)(0), (LPARAM)0);
                if(nI == CB_ERR)  // This should never occur
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                serviceRecordID = SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                if(nI == CB_ERR)  // This should never occur
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
                if((SetUpRunList(hWndDlg, divisionRecordID, serviceRecordID, RPTPARMS_SPECIFICRUN)) == 0)
                {
                  TMSError(hWndDlg, MB_ICONSTOP, ERROR_193, (HANDLE)hCtlSPECIFICDIVISION);
                  break;
                }
                EnableWindow(hCtlRUN, TRUE);
                EnableWindow(hCtlSPECIFICRUN, TRUE);
              }
              break;  
          }
          break;
//
//  Nodes
//
//  RPTPARMS_NODEABBR
//
        case RPTPARMS_NODEABBR:
          if(SendMessage(hCtlNODEABBR, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlSPECIFICNODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(hCtlSPECIFICLABEL, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          }
          break;
//
//  RPTPARMS_NODELABEL
//
        case RPTPARMS_NODELABEL:
          if(SendMessage(hCtlNODELABEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlSPECIFICLABEL, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(hCtlSPECIFICNODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          }
          break;
//
//  RPTPARMS_SPECIFICNODE
//
        case RPTPARMS_SPECIFICNODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNODELABEL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNODELABEL, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICLABEL, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
                SendMessage(hCtlNODEABBR, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          break;
//
//  RPTPARMS_SPECIFICLABEL
//
        case RPTPARMS_SPECIFICLABEL:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlNODEABBR, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlNODEABBR, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICNODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
                SendMessage(hCtlNODELABEL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          break;
//
//  Blocks
//
//  RPTPARMS_ALLBLOCKS
//
        case RPTPARMS_ALLBLOCKS:
          if(SendMessage(hCtlALLBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlSPECIFICBLOCK, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
          break;
//
//  RPTPARMS_SPECIFICBLOCK
//
        case RPTPARMS_SPECIFICBLOCK:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlBLOCK, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;
//
//  Runs
//
//  RPTPARMS_ALLRUNS
//
        case RPTPARMS_ALLRUNS:
          if(SendMessage(hCtlALLRUNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlSPECIFICRUN, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
          }
          break;
//
//  RPTPARMS_SPECIFICRUN
//
        case RPTPARMS_SPECIFICRUN:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLRUNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLRUNS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlRUN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;
//
//  IDOK
//
        case IDOK:
//
//  Just get what's required
//
//  Output characteristics
//
          if(pRP->flags & RPFLAG_OUTPUT)
          {
            SendMessage(hCtlMAXLINES, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRP->linesPerPage = atoi(tempString);
            if(pRP->linesPerPage < 1 || pRP->linesPerPage > TMSRPT_MAX_LINESPERPAGE)
            {
              LoadString(hInst, ERROR_104, szFormatString, sizeof(szFormatString));
              sprintf(tempString, szFormatString, TMSRPT_MAX_LINESPERPAGE);
              MessageBeep(MB_ICONSTOP);
              MessageBox(hWndDlg, tempString, TMS, MB_OK);
              SetFocus(hCtlMAXLINES);
              break;
            }
            SendMessage(hCtlMAXCOLS, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRP->colsPerPage = atoi(tempString);
            if(pRP->colsPerPage < 1 || pRP->colsPerPage > TMSRPT_MAX_COLSPERPAGE)
            {
              LoadString(hInst, ERROR_105, szFormatString, sizeof(szFormatString));
              sprintf(tempString, szFormatString, TMSRPT_MAX_LINESPERPAGE);
              MessageBeep(MB_ICONSTOP);
              MessageBox(hWndDlg, tempString, TMS, MB_OK);
              SetFocus(hCtlMAXCOLS);
              break;
            }
          }
//
//  Trip number
//
          if(pRP->flags & RPFLAG_SPECIFICTRIPNUMBER)
          {
            SendMessage(hCtlTRIPNUMBER, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              pRP->tripNumber = NO_RECORD;
            }
            else
            {
              pRP->tripNumber = atol(tempString);
              if(pRP->tripNumber == 0)
              {
                pRP->tripNumber = NO_RECORD;
              }
            }
          }
//
//  Routes
//
          if(pRP->flags & RPFLAG_ROUTES)
          {
//
//  Allocate space for the list of selections
//
            TMSHeapFree(pRP->pRouteList);
            rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
            if(rcode2 != 0 || BSTAT.numRecords == 0)
            {
              TMSError(NULL, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            maxRoutes = BSTAT.numRecords;
            pRP->pRouteList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxRoutes); 
            if(pRP->pRouteList == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            pRP->numRoutes = 0;
//
//  All routes
//
            if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
              while(rcode2 == 0)
              {
                pRP->pRouteList[pRP->numRoutes++] = ROUTES.recordID;
                rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
              }
            }
//
//  All labelled
//
            else if(SendMessage(hCtlALLLABELLED, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              nI = (int)SendMessage(hCtlROUTESLABELLED, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                TMSError(NULL, MB_ICONSTOP, ERROR_066, (HANDLE)NULL);
                SendMessage(hCtlROUTESLABELLED, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
                SendMessage(hCtlALLLABELLED, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlALLROUTES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                EnableWindow(hCtlALLLABELLED, FALSE);
                EnableWindow(hCtlROUTESLABELLED, FALSE);
                break;
              }
              recordID = SendMessage(hCtlROUTESLABELLED, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
              while(rcode2 == 0)
              {
                if(ROUTES.COMMENTSrecordID == recordID)
                {
                  pRP->pRouteList[pRP->numRoutes++] = ROUTES.recordID;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
              }
            }
//
//  Specific Route
//
            else if(SendMessage(hCtlROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              nI = (int)SendMessage(hCtlSPECIFICROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_188, hCtlSPECIFICROUTE);
                break;
              }
              pRP->pRouteList[pRP->numRoutes++] = 
                    SendMessage(hCtlSPECIFICROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            }
          }
//
//  Services
//
          if(pRP->flags & RPFLAG_SERVICES)
          {
//
//  Allocate space for the list of selections
//
            TMSHeapFree(pRP->pServiceList);
            rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
            if(rcode2 != 0 || BSTAT.numRecords == 0)
            {
              TMSError(NULL, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            maxServices = BSTAT.numRecords;
            pRP->pServiceList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxServices); 
            if(pRP->pServiceList == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            pRP->numServices = 0;
//
//  All services
//
            if(SendMessage(hCtlALLSERVICES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
              while(rcode2 == 0)
              {
                pRP->pServiceList[pRP->numServices++] = SERVICES.recordID;
                rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
              }
            }
//
//  Specific service
//
            else if(SendMessage(hCtlSERVICE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              nI = (int)SendMessage(hCtlSPECIFICSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_190, hCtlSPECIFICSERVICE);
                break;
              }
              pRP->pServiceList[pRP->numServices++] = 
                    SendMessage(hCtlSPECIFICSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            }
          }
//
//  Divisions
//
          if(pRP->flags & RPFLAG_DIVISIONS)
          {
//
//  Allocate space for the list of selections
//
            TMSHeapFree(pRP->pDivisionList);
            rcode2 = btrieve(B_STAT, TMS_DIVISIONS, &BSTAT, dummy, 0);
            if(rcode2 != 0 || BSTAT.numRecords == 0)
            {
              TMSError(NULL, MB_ICONSTOP, ERROR_080, (HANDLE)NULL);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            maxDivisions = BSTAT.numRecords;
            pRP->pDivisionList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxDivisions); 
            if(pRP->pDivisionList == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
              break;
            }
            pRP->numDivisions = 0;
//
//  All divisions
//
            if(SendMessage(hCtlALLDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
              while(rcode2 == 0)
              {
                pRP->pDivisionList[pRP->numDivisions++] = DIVISIONS.recordID;
                rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
              }
            }
//
//  Specific division
//
            else if(SendMessage(hCtlDIVISION, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              nI = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_190, hCtlSPECIFICDIVISION);
                break;
              }
              pRP->pDivisionList[pRP->numDivisions++] = 
                    SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            }
          }
//
//  Blocks
//
          if(pRP->flags & RPFLAG_BLOCKS)
          {
//
//  All blocks - We'll use the specific block list box as in intermediate
//               storage location, as we loop through the routes and services.
//
            if(SendMessage(hCtlALLBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
//
//  Make sure there's a garage
//
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
              if(rcode2 != 0)  // No Garages
              {
                TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
              }
//
//  Allocate space for the list
//
              TMSHeapFree(pRP->pBlockList);
              rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
              if(rcode2 != 0 || BSTAT.numRecords == 0)
              {
                TMSError(NULL, MB_ICONSTOP, ERROR_265, (HANDLE)NULL);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              maxBlocks = BSTAT.numRecords;
              pRP->pBlockList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxBlocks); 
              if(pRP->pBlockList == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              pRP->numBlocks = 0;
//
//  Loop through the routes and services
//
              LoadString(hInst, TEXT_149, tempString, TEMPSTRING_LENGTH);
              StatusBarStart(hWndDlg, tempString);
              LoadString(hInst, TEXT_147, tempString, TEMPSTRING_LENGTH);
              StatusBarText(tempString);
              for(nI = 0; nI < pRP->numRoutes; nI++)
              {
                if(StatusBarAbort())
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                for(nJ = 0; nJ < pRP->numServices; nJ++)
                {
                  StatusBar((long)(nI * pRP->numServices + nJ + 1),
                        (long)(pRP->numRoutes * pRP->numServices));
                  if(StatusBarAbort())
                  {
                    SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                    break;
                  }
                  NODESKey1.flags = NODES_FLAG_GARAGE;
                  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
                  bFirst = TRUE;
                  while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
                  {
                    assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
                    if(assignedToNODESrecordID == NO_RECORD)
                    {
                      strcpy(szarString, "");
                    }
                    else
                    {
                      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                      trim(tempString, NODES_ABBRNAME_LENGTH);
                      sprintf(szarString, "\t(%s)", tempString);
                    }
                    TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
                    TRIPSKey2.RGRPROUTESrecordID = pRP->pRouteList[nI];
                    TRIPSKey2.SGRPSERVICESrecordID = pRP->pServiceList[nJ];
                    TRIPSKey2.blockNumber = 0;
                    TRIPSKey2.blockSequence = NO_TIME;
                    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
                    while(rcode2 == 0 &&
                          pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                          pTRIPSChunk->RGRPROUTESrecordID == pRP->pRouteList[nI] &&
                          pTRIPSChunk->SGRPSERVICESrecordID == pRP->pServiceList[nJ])
                    {
                      pRP->pBlockList[pRP->numBlocks++] = TRIPS.recordID;
                      TRIPSKey2.blockNumber++;
                      TRIPSKey2.blockSequence = NO_TIME;
                      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
                    }
                    if(bFirst)
                    {
                      bFirst = FALSE;
                    }
                    else
                    {
                      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
                    }
                  }
                }
              }
            }
//
//  Selected block(s) - The case statements handling the 
//  radio buttons (above) ensures that if we're here, a
//  specific route and service must have been selected.
//
            else if(SendMessage(hCtlBLOCK, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
//
//  He must have chosen at least one
//
              nI = (int)SendMessage(hCtlSPECIFICBLOCK, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR || nI < 0)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_068, hCtlSPECIFICBLOCK);
                break;
              }
//
//  Allocate space for the list of selections
//
              TMSHeapFree(pRP->pBlockList);
              pRP->pBlockList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * nI); 
              if(pRP->pBlockList == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
//
//  Get the selections, then loop through to get the record IDs
//
              SendMessage(hCtlSPECIFICBLOCK, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)(LPINT)(pRP->pBlockList));
              pRP->numBlocks = nI;
              for(nI = 0; nI < pRP->numBlocks; nI++)
              {
                recordID = SendMessage(hCtlSPECIFICBLOCK, LB_GETITEMDATA, (WPARAM)(pRP->pBlockList[nI]), (LPARAM)0);
                pRP->pBlockList[nI] = recordID;
              }
            }
          }
//
//  Runs
//
          if(pRP->flags & RPFLAG_RUNS)
          {
//
//  If "All runs" is selected, then build the list back in the calling routine
//
            if(SendMessage(hCtlALLRUNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
//
//  Allocate space for the list
//
              TMSHeapFree(pRP->pRunList);
              rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
              if(rcode2 != 0 || BSTAT.numRecords == 0)
              {
                TMSError(NULL, MB_ICONSTOP, ERROR_266, (HANDLE)NULL);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              maxRuns = BSTAT.numRecords;
              pRP->pRunList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxRuns); 
              if(pRP->pRunList == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
              pRP->numRuns = 0;
//
//  Loop through the services
//
              LoadString(hInst, TEXT_150, tempString, TEMPSTRING_LENGTH);
              StatusBarStart(hWndDlg, tempString);
              LoadString(hInst, TEXT_148, tempString, TEMPSTRING_LENGTH);
              StatusBarText(tempString);
              for(nI = 0; nI < pRP->numServices; nI++)
              {
                StatusBar((long)nI, (long)pRP->numServices);
                if(StatusBarAbort())
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                nJ = (int)SendMessage(hCtlSPECIFICDIVISION, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                if(nJ == CB_ERR)  // This should never occur
                {
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
                }
                divisionRecordID = SendMessage(hCtlSPECIFICDIVISION, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                if((SetUpRunList(hWndDlg, divisionRecordID, pRP->pServiceList[nI], RPTPARMS_SPECIFICRUN)) == 0)
                {
                  continue;
                }
                nJ = (int)SendMessage(hCtlSPECIFICRUN, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                for(nK = 0; nK < nJ; nK++)
                {
                  pRP->pRunList[pRP->numRuns++] = SendMessage(hCtlSPECIFICRUN, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
                }
              }
            }
//
//  He's chosen at least one specific run.  Since he'd only get the list if he'd
//  picked a specific service, build the list here so that we know what he's chosen.
//
            else
            {
              nI = (int)SendMessage(hCtlSPECIFICRUN, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR || nI <= 0)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_194, hCtlSPECIFICRUN);
                break;
              }
//
//  Allocate space for the list of selections
//
              TMSHeapFree(pRP->pRunList);
              pRP->pRunList = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * nI); 
              if(pRP->pRunList == NULL)
              {
                AllocationError(__FILE__, __LINE__, FALSE);
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
//
//  Get the selections, then loop through to get the record IDs
//
              SendMessage(hCtlSPECIFICRUN, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)(LPINT)(pRP->pRunList));
              pRP->numRuns = nI;
              for(nI = 0; nI < pRP->numRuns; nI++)
              {
                recordID = SendMessage(hCtlSPECIFICRUN, LB_GETITEMDATA, (WPARAM)(pRP->pRunList[nI]), (LPARAM)0);
                pRP->pRunList[nI] = recordID;
              }
            }
          }
//
//  Pattern names
//
          if(pRP->flags & RPFLAG_PATTERNNAMES)
          {
            nI = (int)SendMessage(hCtlSPECIFICPATTERN, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == CB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_191, hCtlSPECIFICPATTERN);
              break;
            }
            pRP->PATTERNNAMESrecordID = SendMessage(hCtlSPECIFICPATTERN, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Nodes
//
          if(pRP->flags & RPFLAG_NODES)
          {
            if(SendMessage(hCtlNODEABBR, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              nI = (int)SendMessage(hCtlSPECIFICNODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR) // This should never occur
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_088, hCtlSPECIFICNODE);
                break;
              }
              pRP->NODESrecordID = SendMessage(hCtlSPECIFICNODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              pRP->nodeLabel = '\0';
            }
            else
            {
              nI = (int)SendMessage(hCtlSPECIFICLABEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == CB_ERR) // This should never occur
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_229, hCtlSPECIFICLABEL);
                break;
              }
              SendMessage(hCtlSPECIFICLABEL, CB_GETLBTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
              pRP->nodeLabel = tempString[0];
              pRP->NODESrecordID = NO_RECORD;
            }
          }
//
//  Returned flags
//
          if(SendMessage(hCtlSHOWSUMMARYOFSIGNCODES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pRP->returnedFlags |= RPFLAG_SIGNCODES;
          }
          if(SendMessage(hCtlINCLUDETRIPNUMBERS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pRP->returnedFlags |= RPFLAG_TRIPNUMBERS;
          }
//
//  Passed all the tests - update TMS.INI if necessary
//
          if(pRP->flags & RPFLAG_OUTPUT)
          { 
            itoa(pRP->linesPerPage, tempString, 10);
            WritePrivateProfileString("TMS", "TMSRPTMaxLinesPerPage", tempString, TMSINIFile);
            itoa(pRP->colsPerPage, tempString, 10);
            WritePrivateProfileString("TMS", "TMSRPTMaxColsPerPage", tempString, TMSINIFile);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Reports);
          break;
      }

    default:
      return FALSE;
  }
  return TRUE;
} //  End of RPTPARMSMsgProc


int RptSetupBlockList(long routeRecordID, long serviceRecordID, HANDLE hCtl, REPORTPARMSDef *pRP)
{
  BOOL   bFirst;
  long   assignedToNODESrecordID;
  int    nI;
  int    numBlocks;
  int    rcode2;
  BLOCKSDef *pTRIPSChunk;
  int    keyNumber;

  if(pRP->flags & RPFLAG_STANDARDBLOCKS)
  {
    pTRIPSChunk = &TRIPS.standard;
    keyNumber = 2;
  }
  else
  {
    pTRIPSChunk = &TRIPS.dropback;
    keyNumber = 3;
  }

  numBlocks = 0;
  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  if(rcode2 != 0)  // No Garages
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
    return(IDCANCEL);
  }
  else
  {
    bFirst = TRUE;
    while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
    {
      assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
      if(assignedToNODESrecordID == NO_RECORD)
      {
        strcpy(szarString, "");
      }
      else
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(szarString, "\t(%s)", tempString);
      }
      TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = routeRecordID;
      TRIPSKey2.SGRPSERVICESrecordID = serviceRecordID;
      TRIPSKey2.blockNumber = 0;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      while(rcode2 == 0 &&
            pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
            pTRIPSChunk->RGRPROUTESrecordID == routeRecordID &&
            pTRIPSChunk->SGRPSERVICESrecordID == serviceRecordID)
      {
        sprintf(tempString, "%6ld%s", pTRIPSChunk->blockNumber, szarString);
        nI = SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        numBlocks++;
        SendMessage(hCtl, LB_SETITEMDATA, (WPARAM)nI, TRIPS.recordID);
        TRIPSKey2.blockNumber++;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      }
      if(bFirst)
      {
        bFirst = FALSE;
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
    }
    if(numBlocks == 0)
    {
      TMSError(NULL, MB_ICONSTOP, ERROR_107, (HANDLE)NULL);
      return(IDCANCEL);
    }
  }

  return(IDOK);
}
