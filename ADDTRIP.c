//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static  HANDLE hCtlRBNUMTRIPS;
static  HANDLE hCtlRBUNTILTIME;

BOOL CALLBACK ADDTRIPMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlPATTERNNAME;
  static  HANDLE hCtlPATTERNNODES;
  static  HANDLE hCtlSTARTTIME;
  static  HANDLE hCtlHEADWAY;
  static  HANDLE hCtlNUMTRIPS;
  static  HANDLE hCtlUNTILTIME;
  static  HANDLE hCtlLAYMIN;
  static  HANDLE hCtlLAYMAX;
  static  HANDLE hCtlBTNA;
  static  HANDLE hCtlBT;
  static  HANDLE hCtlBUSTYPE;
  static  HANDLE hCtlCCNA;
  static  HANDLE hCtlCC;
  static  HANDLE hCtlCOMMENTCODE;
  static  HANDLE hCtlCCCNA;
  static  HANDLE hCtlCCC;
  static  HANDLE hCtlCCOMMENTCODE;
  static  HANDLE hCtlSCNA;
  static  HANDLE hCtlSC;
  static  HANDLE hCtlSIGNCODE;
  static  HANDLE hCtlSCNA2;
  static  HANDLE hCtlSC2;
  static  HANDLE hCtlSIGNCODE2;
  static  HANDLE hCtlHEADWAY_TEXT;
  static  HANDLE hCtlSELECT;
  static  HANDLE hCtlIDADD;
  static  HANDLE hCtlLASTGROUPBOX;
  static  HANDLE hCtlLAST;
  static  char   szLastTrip[256];
  static  long   routeRecordID;
  static  long   serviceRecordID;
  static  long   directionIndex;
  static  long   lastRecordIDAdded;
  static  int    MLPNodeIndex;
  static  int    numPatternNames;
  TRIPSDef tempTRIPS;
  HCURSOR saveCursor;
  BOOL    bFound;
  long    bustypeRecordID;
  long    customerCommentRecordID;
  long    operatorCommentRecordID;
  long    signcodeRecordID;
  long    secondarySigncodeRecordID;
  long    headway;
  long    layoverMin;
  long    layoverMax;
  long    patternNameRecordID;
  long    timeAtMLP;
  long    untilTime;
  long    newRecID;
  int     nI;
  int     nJ;
  int     numTrips;
  int     numComments;
  int     numSigncodes;
  int     numBustypes;
  int     rcode2;
  int     counter;
  short int wmId;
  short int wmEvent;
  long flags;

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
     hCtlPATTERNNAME = GetDlgItem(hWndDlg, ADDTRIP_PATTERNNAME);
     hCtlPATTERNNODES = GetDlgItem(hWndDlg, ADDTRIP_PATTERNNODES);
     hCtlSTARTTIME = GetDlgItem(hWndDlg, ADDTRIP_STARTTIME);
     hCtlHEADWAY = GetDlgItem(hWndDlg, ADDTRIP_HEADWAY);
     hCtlNUMTRIPS = GetDlgItem(hWndDlg, ADDTRIP_NUMTRIPS);
     hCtlUNTILTIME = GetDlgItem(hWndDlg, ADDTRIP_UNTILTIME);
     hCtlRBNUMTRIPS = GetDlgItem(hWndDlg, ADDTRIP_RBNUMTRIPS);
     hCtlRBUNTILTIME = GetDlgItem(hWndDlg, ADDTRIP_RBUNTILTIME);
     hCtlLAYMIN = GetDlgItem(hWndDlg, ADDTRIP_LAYMIN);
     hCtlLAYMAX = GetDlgItem(hWndDlg, ADDTRIP_LAYMAX);
     hCtlBTNA = GetDlgItem(hWndDlg, ADDTRIP_BTNA);
     hCtlBT = GetDlgItem(hWndDlg, ADDTRIP_BT);
     hCtlBUSTYPE = GetDlgItem(hWndDlg, ADDTRIP_BUSTYPE);
     hCtlCCNA = GetDlgItem(hWndDlg, ADDTRIP_CCNA);
     hCtlCC = GetDlgItem(hWndDlg, ADDTRIP_CC);
     hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDTRIP_COMMENTCODE);
     hCtlCCCNA = GetDlgItem(hWndDlg, ADDTRIP_CCCNA);
     hCtlCCC = GetDlgItem(hWndDlg, ADDTRIP_CCC);
     hCtlCCOMMENTCODE = GetDlgItem(hWndDlg, ADDTRIP_CCOMMENTCODE);
     hCtlSCNA = GetDlgItem(hWndDlg, ADDTRIP_SCNA);
     hCtlSC = GetDlgItem(hWndDlg, ADDTRIP_SC);
     hCtlSIGNCODE = GetDlgItem(hWndDlg, ADDTRIP_SIGNCODE);
     hCtlSCNA2 = GetDlgItem(hWndDlg, ADDTRIP_SCNA2);
     hCtlSC2 = GetDlgItem(hWndDlg, ADDTRIP_SC2);
     hCtlSIGNCODE2 = GetDlgItem(hWndDlg, ADDTRIP_SIGNCODE2);
     hCtlHEADWAY_TEXT = GetDlgItem(hWndDlg, ADDTRIP_HEADWAY_TEXT);
     hCtlSELECT = GetDlgItem(hWndDlg, ADDTRIP_SELECT);
     hCtlIDADD = GetDlgItem(hWndDlg, IDADD);
     hCtlLASTGROUPBOX = GetDlgItem(hWndDlg, ADDTRIP_LASTGROUPBOX);
     hCtlLAST = GetDlgItem(hWndDlg, ADDTRIP_LAST);
//
//  Display the direction
//
     flags = PLACEMENT_DIRECTION;
     DisplayPlacement(hWndDlg, ADDTRIP_DESTINATION, pDI, flags);
//
//  Operator comment codes combo box
//
      numComments = SetUpCommentList(hWndDlg, ADDTRIP_COMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCC, FALSE);
        EnableWindow(hCtlCOMMENTCODE, FALSE);
      }
//
//  Customer comment codes combo box
//
      numComments = SetUpCommentList(hWndDlg, ADDTRIP_CCOMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCCC, FALSE);
        EnableWindow(hCtlCCOMMENTCODE, FALSE);
      }
//
//  Sign codes combo box
//
      numSigncodes = SetUpSigncodeList(hWndDlg, ADDTRIP_SIGNCODE);
      if(numSigncodes == 0)
      {
        EnableWindow(hCtlSC, FALSE);
        EnableWindow(hCtlSIGNCODE, FALSE);
      }
//
//  Bus types combo box
//
      numBustypes = SetUpBustypeList(hWndDlg, ADDTRIP_BUSTYPE);
      if(numBustypes == 0)
      {
        EnableWindow(hCtlBT, FALSE);
        EnableWindow(hCtlBUSTYPE, FALSE);
      }
//
//  Make sure NUMTRIPS and HEADWAY are blank
//
      SendMessage(hCtlNUMTRIPS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
      SendMessage(hCtlHEADWAY, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
//
//  Insert or update?
//
//  Insert
//
     if(updateRecordID == NO_RECORD)
     {
       lastRecordIDAdded = NO_RECORD;
       routeRecordID = pDI->fileInfo.routeRecordID;
       serviceRecordID = pDI->fileInfo.serviceRecordID;
       directionIndex = pDI->fileInfo.directionIndex;
       numPatternNames = SetUpPatternList(hWndDlg, ADDTRIP_PATTERNNAME, pDI->fileInfo.routeRecordID,
             pDI->fileInfo.serviceRecordID, pDI->fileInfo.directionIndex, TRUE, FALSE);
       if(numPatternNames == 0)
       {
         SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
         break;
       }
       SendMessage(hCtlPATTERNNAME, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(ADDTRIP_PATTERNNAME, CBN_SELCHANGE), (LPARAM)0);
       SendMessage(hCtlCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       SendMessage(hCtlCCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       SendMessage(hCtlSCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       SendMessage(hCtlSCNA2, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       bFound = FALSE;
       if(defaultBustype == NO_RECORD)
         SendMessage(hCtlBTNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       else
       {
         for(nJ = 0; nJ < numBustypes; nJ++)
         {
           if(SendMessage(hCtlBUSTYPE, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == defaultBustype)
           {
             bFound = TRUE;
             SendMessage(hCtlBUSTYPE, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
             break;
           }
         }
         SendMessage(bFound ? hCtlBT : hCtlBTNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
       }
       SendMessage(hCtlLAYMIN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
       SendMessage(hCtlLAYMAX, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
     }
//
//  Update
//
     else
     {
//
//  Change the Add button to Update and disable the headway/select controls
//
       LoadString(hInst, TEXT_029, tempString, TEMPSTRING_LENGTH);
       SendMessage(hCtlIDADD, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
       EnableWindow(hCtlHEADWAY_TEXT, FALSE);
       EnableWindow(hCtlHEADWAY, FALSE);
       EnableWindow(hCtlSELECT, FALSE);
       EnableWindow(hCtlRBNUMTRIPS, FALSE);
       EnableWindow(hCtlNUMTRIPS, FALSE);
       EnableWindow(hCtlRBUNTILTIME, FALSE);
       EnableWindow(hCtlUNTILTIME, FALSE);
//
//  Get the TRIPS record
//
       TRIPSKey0.recordID = updateRecordID;
       rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
       if(rcode2 != 0)
       {
         LoadString(hInst, ERROR_200, szFormatString, sizeof(szFormatString));
         sprintf(tempString, szFormatString, rcode2);
         MessageBeep(MB_ICONSTOP);
         MessageBox(NULL, tempString, TMS, MB_OK);
         SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
         break;
       }
       routeRecordID = TRIPS.ROUTESrecordID;
       serviceRecordID = TRIPS.SERVICESrecordID;
       directionIndex = TRIPS.directionIndex;
//
//  Get all the patterns
//
       numPatternNames = SetUpPatternList(hWndDlg, ADDTRIP_PATTERNNAME, TRIPS.ROUTESrecordID,
             TRIPS.SERVICESrecordID, TRIPS.directionIndex, TRUE, FALSE);
       if(numPatternNames == 0)
       {
         SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
         break;
       }
//
//  Select the pattern name
//
       for(nI = 0; nI < numPatternNames; nI++)
       {
         if(SendMessage(hCtlPATTERNNAME, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == TRIPS.PATTERNNAMESrecordID)
         {
           SendMessage(hCtlPATTERNNAME, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
           break;
         }
       }
//
//  And update the pattern nodes list box
//
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(ADDTRIP_PATTERNNAME, CBN_SELCHANGE), (LPARAM)0);
//
//  Set up the bustype
//
        if(TRIPS.BUSTYPESrecordID == NO_RECORD)
        {
          SendMessage(hCtlBTNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
        else
        {
          for(bFound = FALSE, nI = 0; nI < numBustypes; nI++)
          {
            if(SendMessage(hCtlBUSTYPE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == TRIPS.BUSTYPESrecordID)
            {
              SendMessage(hCtlBUSTYPE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlBT : hCtlBTNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
//
//  Operator comment
//
        if(TRIPS.COMMENTSrecordID == NO_RECORD)
          SendMessage(hCtlCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          for(bFound = FALSE, nI = 0; nI < numComments; nI++)
          {
            if(SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA,
                  (WPARAM)nI, (LPARAM)0) == TRIPS.COMMENTSrecordID)
            {
              SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlCC : hCtlCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
//
//  Customer comment
//
        if(TRIPS.customerCOMMENTSrecordID == NO_RECORD ||
              TRIPS.customerCOMMENTSrecordID == 0)  // Backward compatibility
          SendMessage(hCtlCCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          for(bFound = FALSE, nI = 0; nI < numComments; nI++)
          {
            if(SendMessage(hCtlCCOMMENTCODE, CB_GETITEMDATA,
                  (WPARAM)nI, (LPARAM)0) == TRIPS.customerCOMMENTSrecordID)
            {
              SendMessage(hCtlCCOMMENTCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlCCC : hCtlCCCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
//
//  Sign code
//
        if(TRIPS.SIGNCODESrecordID == NO_RECORD || TRIPS.SIGNCODESrecordID == 0)
          SendMessage(hCtlSCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
        {
          for(bFound = FALSE, nI = 0; nI < numSigncodes; nI++)
          {
            if(SendMessage(hCtlSIGNCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == TRIPS.SIGNCODESrecordID)
            {
              SendMessage(hCtlSIGNCODE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlSC : hCtlSCNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
//
//  Secondary sign code
//
        if(TRIPS.secondarySIGNCODESrecordID == NO_RECORD || TRIPS.secondarySIGNCODESrecordID == 0)
        {
          SendMessage(hCtlSCNA2, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
        else
        {
          for(bFound = FALSE, nI = 0; nI < numSigncodes; nI++)
          {
            if(SendMessage(hCtlSIGNCODE2, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0) == TRIPS.secondarySIGNCODESrecordID)
            {
              SendMessage(hCtlSIGNCODE2, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
              bFound = TRUE;
              break;
            }
          }
          SendMessage(bFound ? hCtlSC2 : hCtlSCNA2, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        }
//
//  Set up the time
//
       SendMessage(hCtlSTARTTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)Tchar(TRIPS.timeAtMLP));
//
//  Set up the min and max layovers
//
       if(TRIPS.standard.layoverMin == NO_TIME)
         SendMessage(hCtlLAYMIN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
       else
       {
         ltoa(TRIPS.standard.layoverMin / 60, tempString, 10);
         SendMessage(hCtlLAYMIN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
       }
       if(TRIPS.standard.layoverMax == NO_TIME)
         SendMessage(hCtlLAYMAX, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
       else
       {
         ltoa(TRIPS.standard.layoverMax / 60, tempString, 10);
         SendMessage(hCtlLAYMAX, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
       }
     }
//
//  Default to number of trips
//
     SendMessage(hCtlRBNUMTRIPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Set the "last added" message
//
     if(updateRecordID == NO_RECORD)
     {
       strcpy(szLastTrip, "None");
       SendMessage(hCtlLAST, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szLastTrip);
     }
     else
     {
       EnableWindow(hCtlLASTGROUPBOX, FALSE);
       EnableWindow(hCtlLAST, FALSE);
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
//  Operator comment code
//
        case ADDTRIP_COMMENTCODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlCCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlCCNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlCC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDTRIP_CCNA:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDTRIP_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Customer comment code
//
        case ADDTRIP_CCOMMENTCODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlCCCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlCCCNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlCCC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDTRIP_CCCNA:
          SendMessage(hCtlCCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDTRIP_CCC:
          SendMessage(hCtlCCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Sign code
//
        case ADDTRIP_SIGNCODE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlSCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlSCNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSC, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDTRIP_SCNA:
          SendMessage(hCtlSIGNCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDTRIP_SC:
          SendMessage(hCtlSIGNCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Secondary sign code
//
        case ADDTRIP_SIGNCODE2:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlSCNA2, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlSCNA2, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSC2, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDTRIP_SCNA2:
          SendMessage(hCtlSIGNCODE2, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDTRIP_SC2:
          SendMessage(hCtlSIGNCODE2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Bustype
//
        case ADDTRIP_BUSTYPE:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlBTNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlBTNA, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlBT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case ADDTRIP_BTNA:
          SendMessage(hCtlBUSTYPE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;

        case ADDTRIP_BT:
          SendMessage(hCtlBUSTYPE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  Pattern name
//
       case ADDTRIP_PATTERNNAME: // Combo Box
         switch(wmEvent)
         {
           case CBN_SELCHANGE:
             nI = (int)SendMessage(hCtlPATTERNNAME, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
             if(nI != CB_ERR)
               patternNameRecordID = SendMessage(hCtlPATTERNNAME, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
             else
             {
               patternNameRecordID = basePatternRecordID;
               nI = (int)SendMessage(hCtlPATTERNNAME, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
               for(nJ = 0; nJ < nI; nJ++)
               {
                 if(SendMessage(hCtlPATTERNNAME, CB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == patternNameRecordID)
                 {
                   SendMessage(hCtlPATTERNNAME, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
                   break;
                 }
               }
             }
//
//  Pattern name CBN_SELCHANGE - (Re)set the pattern nodes list box
//
             SendMessage(hCtlPATTERNNODES, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
             PATTERNSKey2.ROUTESrecordID = routeRecordID;
             PATTERNSKey2.SERVICESrecordID = serviceRecordID;
             PATTERNSKey2.directionIndex = directionIndex;
             PATTERNSKey2.PATTERNNAMESrecordID = patternNameRecordID;
             PATTERNSKey2.nodeSequence = 0;
             rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             while(rcode2 == 0 &&
                   PATTERNS.ROUTESrecordID == routeRecordID &&
                   PATTERNS.SERVICESrecordID == serviceRecordID &&
                   PATTERNS.directionIndex == directionIndex &&
                   PATTERNSKey2.PATTERNNAMESrecordID == patternNameRecordID)
             {
               if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
               {
                 NODESKey0.recordID = PATTERNS.NODESrecordID;
                 btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                 strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                 trim(tempString, NODES_ABBRNAME_LENGTH);
                 nI = (int)SendMessage(hCtlPATTERNNODES, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                 if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                 {
                   MLPNodeIndex = nI;
                   SendMessage(hCtlPATTERNNODES, LB_SETCURSEL, (WPARAM)MLPNodeIndex, (LPARAM)0);
                 }
               }
               rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
             }
             break;
         }
         break;
//
//  Pattern nodes
//
       case ADDTRIP_PATTERNNODES: // "Static" List Box
         switch(wmEvent)
         {
           case LBN_SELCHANGE:
             SendMessage(hCtlPATTERNNODES, LB_SETCURSEL, (WPARAM)MLPNodeIndex, (LPARAM)0);
             break;
         }
         break;
//
//  UNTILTIME edit control
//
       case ADDTRIP_UNTILTIME:
         switch(wmEvent)
         {
           case EN_CHANGE:
             SendMessage(hCtlRBUNTILTIME, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
             SendMessage(hCtlRBNUMTRIPS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
             break;
         }
         break;
//
//  IDADD
//
       case IDADD:
//
//  Pattern name must be selected
//
         if((nI = (int)SendMessage(hCtlPATTERNNAME, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == CB_ERR)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_028, hCtlPATTERNNAME);
           break;
         }
         patternNameRecordID = SendMessage(hCtlPATTERNNAME, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Start time must be entered
//
         SendMessage(hCtlSTARTTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
         if(strcmp(tempString, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_032, hCtlSTARTTIME);
           break;
         }
         timeAtMLP = cTime(tempString);
//
//  Headway, num trips/until time
//
         headway = GetDlgItemInt(hWndDlg, ADDTRIP_HEADWAY, NULL, TRUE);
         numTrips = GetDlgItemInt(hWndDlg, ADDTRIP_NUMTRIPS, NULL, TRUE);
         SendMessage(hCtlUNTILTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
         untilTime = (strcmp(tempString, "") == 0 ? NO_TIME : cTime(tempString));
//
//  On an insert, if from and to times are entered, and no headway, it's an error
//  On an insert, Set numTrips to 1 if numTrips, untilTime, and headway are zero
//
         if(updateRecordID == NO_RECORD)
         {
           if(headway == 0)
           {
             if(untilTime != NO_TIME)
             {
               TMSError(hWndDlg, MB_ICONSTOP, ERROR_038, hCtlHEADWAY);
               break;
             }
             else
             {
               if(numTrips == 0)
               {
                 SendMessage(hCtlRBNUMTRIPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
                 SendMessage(hCtlRBUNTILTIME, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                 numTrips = 1;
               }
             } 
           }
         }
//
//  If there's more than one trip to generate, headway can't be zero
//
         if(headway == 0)
         {
           if(numTrips > 1 || SendMessage(hCtlUNTILTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             TMSError(hWndDlg, MB_ICONSTOP, ERROR_038, hCtlHEADWAY);
             break;
           }
         }
//
//  If there's a headway, and both numTrips and untilTime are zero, it's an error
//
         if(headway != 0 && numTrips == 0 && untilTime == NO_TIME)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_039, hCtlNUMTRIPS);
           break;
         }
//
//  If he specified untilTime, it can't be less than the time at the MLP
//
         if(SendMessage(hCtlRBUNTILTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0) && timeAtMLP >= untilTime)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_181, hCtlUNTILTIME);
           break;
         }
//
//  Operator comment
//
         if(SendMessage(hCtlCCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           operatorCommentRecordID = NO_RECORD;
         else
         {
           nI = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           operatorCommentRecordID = nI == CB_ERR ? NO_RECORD :
                 SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         }
//
//  Customer comment
//
         if(SendMessage(hCtlCCCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           customerCommentRecordID = NO_RECORD;
         else
         {
           nI = (int)SendMessage(hCtlCCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           customerCommentRecordID = nI == CB_ERR ? NO_RECORD :
                 SendMessage(hCtlCCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         }
//
//  Sign code
//
         if(SendMessage(hCtlSCNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           signcodeRecordID = NO_RECORD;
         else
         {
           nI = (int)SendMessage(hCtlSIGNCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           signcodeRecordID = nI == CB_ERR ? NO_RECORD :
                 SendMessage(hCtlSIGNCODE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         }
//
//  Secondary sign code
//
         if(SendMessage(hCtlSCNA2, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
         {
           secondarySigncodeRecordID = NO_RECORD;
         }
         else
         {
           nI = (int)SendMessage(hCtlSIGNCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           secondarySigncodeRecordID = nI == CB_ERR ? NO_RECORD :
                 SendMessage(hCtlSIGNCODE2, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         }
//
//  Bustype
//
         if(SendMessage(hCtlBTNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           bustypeRecordID = NO_RECORD;
         else
         {
           nI = SendMessage(hCtlBUSTYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
           bustypeRecordID = nI == CB_ERR ? NO_RECORD :
                 SendMessage(hCtlBUSTYPE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
         }
//
//  Get the layovers
//
         SendMessage(hCtlLAYMIN, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
         layoverMin = (strcmp(tempString, "") == 0 ? NO_TIME : atol(tempString) * 60);
         SendMessage(hCtlLAYMAX, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
         layoverMax = (strcmp(tempString, "") == 0 ? NO_TIME : atol(tempString) * 60);
         if(layoverMin != NO_TIME && layoverMax != NO_TIME && layoverMin > layoverMax)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_106, hCtlLAYMIN);
           break;
         }
//
//  Insert - Add the trip(s)
//
         saveCursor = SetCursor(hCursorWait);
         if(updateRecordID == NO_RECORD)
         {
           rcode2 = btrieve(B_GETLAST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           TRIPS.recordID = AssignRecID(rcode2, TRIPS.recordID);
           newRecID = TRIPS.recordID;
           TRIPS.COMMENTSrecordID = operatorCommentRecordID;
           TRIPS.ROUTESrecordID = routeRecordID;
           TRIPS.SERVICESrecordID = serviceRecordID;
           TRIPS.BUSTYPESrecordID = bustypeRecordID;
           TRIPS.PATTERNNAMESrecordID = patternNameRecordID;
           TRIPS.SIGNCODESrecordID = signcodeRecordID;
           TRIPS.directionIndex = directionIndex;
           TRIPS.tripNumber = NO_RECORD;
           TRIPS.shift.negativeMax = NO_TIME;
           TRIPS.shift.positiveMax = NO_TIME;
           TRIPS.shift.actual = NO_TIME;
           TRIPS.standard.blockNumber = 0;
           TRIPS.standard.BLOCKNAMESrecordID = NO_RECORD;
           TRIPS.standard.POGNODESrecordID = NO_RECORD;
           TRIPS.standard.PIGNODESrecordID = NO_RECORD;
           TRIPS.standard.RGRPROUTESrecordID = NO_RECORD;
           TRIPS.standard.SGRPSERVICESrecordID = NO_RECORD;
           TRIPS.standard.assignedToNODESrecordID = NO_RECORD;
           TRIPS.standard.blockSequence = NO_TIME;
           TRIPS.standard.layoverMin = layoverMin;
           TRIPS.standard.layoverMax = layoverMax;
           TRIPS.dropback.blockNumber = 0;
           TRIPS.dropback.BLOCKNAMESrecordID = NO_RECORD;
           TRIPS.dropback.POGNODESrecordID = NO_RECORD;
           TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
           TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
           TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
           TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
           TRIPS.dropback.blockSequence = NO_TIME;
           TRIPS.dropback.layoverMin = layoverMin;
           TRIPS.dropback.layoverMax = layoverMax;
           TRIPS.customerCOMMENTSrecordID = customerCommentRecordID;
           TRIPS.secondarySIGNCODESrecordID = secondarySigncodeRecordID;
           memset(&TRIPS.reserved, 0x00, TRIPS_RESERVED_LENGTH);
           TRIPS.flags = 0;
           sprintf(szLastTrip, "Time at MLP: %s, ", Tchar(timeAtMLP));
           if(SendMessage(hCtlRBNUMTRIPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
           {
             for(nI = 0; nI < numTrips; nI++)
             {
               TRIPS.timeAtMLP = timeAtMLP + (headway * 60 * nI);
               TRIPS.tripSequence = TRIPS.timeAtMLP;
               counter = 0;
               while(counter < 500)  // bypass record number conflicts
               {
                 rcode2 = btrieve(B_GETLAST, TMS_TRIPS,&tempTRIPS, &TRIPSKey0, 0);
                 TRIPS.recordID = AssignRecID(rcode2, tempTRIPS.recordID);
                 if(TRIPS.recordID == newRecID)
                 {
                   btrieve(B_INSERT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                   break;
                 }
                 counter++;
                 newRecID = TRIPS.recordID;
               }
             }
             sprintf(tempString, "headway: %d, number of trips: %d", headway, numTrips); 
           }
           else
           {
             while(timeAtMLP <= untilTime)
             {
               TRIPS.timeAtMLP = timeAtMLP;
               TRIPS.tripSequence = TRIPS.timeAtMLP;
               counter = 0;
               while(counter < 500)  // bypass record number conflicts
               {
                 rcode2 = btrieve(B_GETLAST, TMS_TRIPS,&tempTRIPS, &TRIPSKey0, 0);
                 TRIPS.recordID = AssignRecID(rcode2, tempTRIPS.recordID);
                 if(TRIPS.recordID == newRecID)
                 {
                   btrieve(B_INSERT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                   break;
                 }
                 counter++;
                 newRecID = TRIPS.recordID;
               }
               timeAtMLP += (headway * 60);
             }
             sprintf(tempString, "headway: %d, until: %s", headway, Tchar(untilTime));
           }
           strcat(szLastTrip, tempString);
           MessageBeep(MB_ICONINFORMATION);
           SendMessage(hCtlLAST, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szLastTrip);
         }
//
//  Update
//
         else
         {
           TRIPSKey0.recordID = updateRecordID;
           btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
           TRIPS.COMMENTSrecordID = operatorCommentRecordID;
           TRIPS.BUSTYPESrecordID = bustypeRecordID;
           TRIPS.PATTERNNAMESrecordID = patternNameRecordID;
           TRIPS.SIGNCODESrecordID = signcodeRecordID;
           TRIPS.standard.layoverMin = layoverMin;
           TRIPS.standard.layoverMax = layoverMax;
           TRIPS.dropback.layoverMin = layoverMin;
           TRIPS.dropback.layoverMax = layoverMax;
           TRIPS.timeAtMLP = timeAtMLP;
           TRIPS.tripSequence = timeAtMLP;
           TRIPS.customerCOMMENTSrecordID = customerCommentRecordID;
           btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
         }
         SetCursor(saveCursor);
//
//  If he was updating, leave
//
         if(updateRecordID != NO_RECORD)
           EndDialog(hWndDlg, TRUE);
         else
         {
           SendMessage(hCtlHEADWAY, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
           SendMessage(hCtlNUMTRIPS, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
           SendMessage(hCtlUNTILTIME, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
           SendMessage(hCtlRBNUMTRIPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)"");
           SendMessage(hCtlRBUNTILTIME, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)"");
           SetFocus(hCtlSTARTTIME);
           SendMessage(hCtlSTARTTIME, EM_SETSEL, (WPARAM)0, (LPARAM)(-1));
           lastRecordIDAdded = TRIPS.recordID;
         }
         break;
//
//  IDCANCEL
//
       case IDCANCEL:
         EndDialog(hWndDlg, TRUE);  // Normally FALSE, overridden here because not
         break;                     // returning TRUE doesn't update the trips table
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Trips_Table);
          break;
//
//  IDFINISHED
//
       case IDFINISHED:
         updateRecordID = lastRecordIDAdded;
         EndDialog(hWndDlg, TRUE);
         break;
     }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
 }
 return TRUE;
} //  End of ADDTRIPMsgProc
