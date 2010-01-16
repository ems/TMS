//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define NUMCOMBINATIONS 8
#define OFFSATSUN 0
#define OFFSUNMON 1
#define OFFSUNTUE 2
#define OFFSUNWED 3
#define OFFMONTUE 4
#define OFFMONWED 5
#define OFFTUEWED 6
#define OFFTHUFRI 7
#define MON 0
#define TUE 1
#define WED 2
#define THU 3
#define FRI 4
#define SAT 5
#define SUN 6
#define MONBIT 0x01
#define TUEBIT 0x02
#define WEDBIT 0x04
#define THUBIT 0x08
#define FRIBIT 0x10
#define SATBIT 0x20
#define SUNBIT 0x40

typedef struct SELECTIONSStruct
{
  long divisionRecordID;
  long runtype;
} SELECTIONSDef;

typedef struct BIDDINGStruct
{
  DRIVERSDef     DRIVERS;
  PDISPLAYINFO   pDI;
  int            numSelections;
  SELECTIONSDef  SELECTIONS[NUMRUNTYPES * NUMRUNTYPESLOTS];
} BIDDINGDef;

void DisplayRunAnalysis(HWND, HDC, long, BOOL, int);
BOOL PrintRoster(HWND, BIDDINGDef *, long *);
BOOL CALLBACK PaintHook(HWND, UINT, WPARAM, LPARAM);
static int  nPageOutputLine, xChar, yChar;
static char *szDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static BIDDINGDef *pBIDDING;
static HANDLE  hCtlRUNCHARACTERISTICS;
static HANDLE  hCtlTITLE;
static HANDLE  hCtlLIST;
static HANDLE  hCtlSPREADOT;
static HANDLE  hCtlMAKEUP;
static HANDLE  hCtlPAIDBREAKS;
static HANDLE  hCtlOVERTIME;
static HANDLE  hCtlPAYTIME;
static HANDLE  hCtlIDSHOWDIRECTIONS;
static COSTDef COST;

BOOL CALLBACK BIDDINGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlSATSUN;
  static HANDLE hCtlSUNMON;
  static HANDLE hCtlSUNTUE;
  static HANDLE hCtlSUNWED;
  static HANDLE hCtlMONTUE;
  static HANDLE hCtlMONWED;
  static HANDLE hCtlTUEWED;
  static HANDLE hCtlTHUFRI;
  static HANDLE hCtlEXTRAOFF;
  static HANDLE hCtlDAY1;
  static HANDLE hCtlDAY2;
  static HANDLE hCtlDAY3;
  static HANDLE hCtlDAY4;
  static HANDLE hCtlDAY5;
  static HANDLE hCtlAVAILABLE;
  static HANDLE hCtlCURRENT;
  static HANDLE hCtlIDVERIFY;
  static HANDLE hCtlIDSKIP;
  static HANDLE hCtlIDALTER;
  static HANDLE hCtlIDSUSPEND;
  static int    dayButtons[5] = {BIDDING_DAY1, BIDDING_DAY2, BIDDING_DAY3,
                                 BIDDING_DAY4, BIDDING_DAY5};
  static int    offDays[8] = {BIDDING_SATSUN, BIDDING_SUNMON, BIDDING_SUNTUE, BIDDING_SUNWED,
                              BIDDING_MONTUE, BIDDING_MONWED, BIDDING_TUEWED, BIDDING_THUFRI};
  static long   SERVICErecordIDs[BIDDING_MAXSERVICES];
  static long   selectedRuns[BIDDING_NUMSERVICEDAYS];
  static int    numServices;
  static int    numTotalRuns[BIDDING_NUMSERVICEDAYS];
  static char   *szPairs[] = {"Sat/Sun", "Sun/Mon", "Sun/Tue", "Sun/Wed",
                              "Mon/Tue", "Mon/Wed", "Tue/Wed", "Thu/Fri"};
  static long   pairBits[NUMCOMBINATIONS] = {
                               MONBIT | TUEBIT | WEDBIT | THUBIT | FRIBIT,   // Sat/Sun
                               TUEBIT | WEDBIT | THUBIT | FRIBIT | SATBIT,   // Sun/Mon
                               MONBIT | WEDBIT | THUBIT | FRIBIT | SATBIT,   // Sun/Tue
                               MONBIT | TUEBIT | THUBIT | FRIBIT | SATBIT,   // Sun/Wed
                               WEDBIT | THUBIT | FRIBIT | SATBIT | SUNBIT,   // Mon/Tue
                               TUEBIT | THUBIT | FRIBIT | SATBIT | SUNBIT,   // Mon/Wed
                               MONBIT | THUBIT | FRIBIT | SATBIT | SUNBIT,   // Tue/Wed
                               MONBIT | TUEBIT | WEDBIT | SATBIT | SUNBIT};  // Thu/Fri
  static long dayBits[7] = {MONBIT, TUEBIT, WEDBIT, THUBIT, FRIBIT, SATBIT, SUNBIT}; 
  static long rosterWeek;
  SUPERVISORPassedDataDef SUPERVISORPassedData;
  RUNSVIEWDef RUNDATA[BIDDING_NUMSERVICEDAYS][MAXPIECES];
  int         numPieces[BIDDING_NUMSERVICEDAYS];
  HANDLE hCtl;
  short int wmId;
  short int wmEvent;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  long  testBit;
  long  tempLong;
  long  RUNSrecordIDs[BIDDING_NUMSERVICEDAYS];
  BOOL  bGoodCombo;
  BOOL  bFound;
  int   reason;
  int   check[5];
  int   firstRun;
  int   secondRun;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   pairs[NUMCOMBINATIONS];
  int   totalOffDays[BIDDING_NUMSERVICEDAYS];
  int   totalShifts;
  int   total40HourShifts;
  int   totalExtraOffDays;
  int   remainingOffDays[BIDDING_NUMSERVICEDAYS];
  int   split;
  int   numRuns;
  int   serviceIndex;
  int   runNumber;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get the passed BIDDING record
//
      pBIDDING = (BIDDINGDef *)lParam;
      if(pBIDDING == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      rosterWeek = pBIDDING->pDI->fileInfo.rosterWeek;
//
//  Set the dialog box title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
      strcat(tempString, " ");
      strncpy(szarString, pBIDDING->DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(szarString, DRIVERS_BADGENUMBER_LENGTH);
      strcat(tempString, szarString);
      strcat(tempString, " - ");
      strncpy(szarString, pBIDDING->DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(szarString, DRIVERS_LASTNAME_LENGTH);
      strcat(tempString, szarString);
      strcat(tempString, ", ");
      strncpy(szarString, pBIDDING->DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(tempString, szarString);
      strncpy(szarString, pBIDDING->DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      trim(szarString, DRIVERS_INITIALS_LENGTH);
      if(strcmp(szarString, "") != 0)
      {
        strcat(tempString, " ");
        strcat(tempString, szarString);
      }
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Set up the handles to the controls
//
      hCtlSATSUN= GetDlgItem(hWndDlg, BIDDING_SATSUN);
      hCtlSUNMON= GetDlgItem(hWndDlg, BIDDING_SUNMON);
      hCtlSUNTUE= GetDlgItem(hWndDlg, BIDDING_SUNTUE);
      hCtlSUNWED= GetDlgItem(hWndDlg, BIDDING_SUNWED);
      hCtlMONTUE= GetDlgItem(hWndDlg, BIDDING_MONTUE);
      hCtlMONWED= GetDlgItem(hWndDlg, BIDDING_MONWED);
      hCtlTUEWED= GetDlgItem(hWndDlg, BIDDING_TUEWED);
      hCtlTHUFRI= GetDlgItem(hWndDlg, BIDDING_THUFRI);
      hCtlEXTRAOFF = GetDlgItem(hWndDlg, BIDDING_EXTRAOFF);
      hCtlDAY1 = GetDlgItem(hWndDlg, BIDDING_DAY1);
      hCtlDAY2 = GetDlgItem(hWndDlg, BIDDING_DAY2);
      hCtlDAY3 = GetDlgItem(hWndDlg, BIDDING_DAY3);
      hCtlDAY4 = GetDlgItem(hWndDlg, BIDDING_DAY4);
      hCtlDAY5 = GetDlgItem(hWndDlg, BIDDING_DAY5);
      hCtlAVAILABLE = GetDlgItem(hWndDlg, BIDDING_AVAILABLE);
      hCtlCURRENT = GetDlgItem(hWndDlg, BIDDING_CURRENT);
      hCtlIDVERIFY = GetDlgItem(hWndDlg, IDVERIFY);
      hCtlIDSKIP = GetDlgItem(hWndDlg, IDSKIP);
      hCtlIDALTER = GetDlgItem(hWndDlg, IDALTER);
      hCtlIDSUSPEND = GetDlgItem(hWndDlg, IDSUSPEND);
      hCtlRUNCHARACTERISTICS = GetDlgItem(hWndDlg, BIDDING_RUNCHARACTERISTICS);
      hCtlTITLE = GetDlgItem(hWndDlg, BIDDING_TITLE);
      hCtlLIST = GetDlgItem(hWndDlg, BIDDING_LIST);
      hCtlSPREADOT = GetDlgItem(hWndDlg, BIDDING_SPREADOT);
      hCtlMAKEUP = GetDlgItem(hWndDlg, BIDDING_MAKEUP);
      hCtlPAIDBREAKS = GetDlgItem(hWndDlg, BIDDING_PAIDBREAKS);
      hCtlOVERTIME = GetDlgItem(hWndDlg, BIDDING_OVERTIME);
      hCtlPAYTIME = GetDlgItem(hWndDlg, BIDDING_PAYTIME);
      hCtlIDSHOWDIRECTIONS = GetDlgItem(hWndDlg, IDSHOWDIRECTIONS);
//
//  Establish current list of off days
//
//  Get a list of service recordids
//
      numServices = 0;
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0 && numServices < BIDDING_MAXSERVICES)
      {
        SERVICErecordIDs[numServices] = SERVICES.recordID;
        numServices++;
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
//
//  Get the number of runs for each service
//
      for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
      {
        numTotalRuns[nI] = 0;
      }
      StatusBarText("Setting up Runs...");
      for(nI = 0; nI < numServices; nI++)
      {
        StatusBar((long)nI, (long)numServices);
        RUNSKey1.DIVISIONSrecordID = pBIDDING->pDI->fileInfo.divisionRecordID;
        RUNSKey1.SERVICESrecordID = SERVICErecordIDs[nI];
        RUNSKey1.runNumber = NO_RECORD;
        RUNSKey1.pieceNumber = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        while(rcode2 == 0 && RUNS.SERVICESrecordID == SERVICErecordIDs[nI])
        {
          for(bFound = FALSE, nJ = 0; nJ < pBIDDING->numSelections; nJ++)
          {
            if(RUNS.pieceNumber == 1)
            {
              for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
              {
                if(ROSTERPARMS.serviceDays[nK] == SERVICErecordIDs[nI] &&
                      pBIDDING->SELECTIONS[nJ].divisionRecordID == RUNS.DIVISIONSrecordID &&
                      pBIDDING->SELECTIONS[nJ].runtype == RUNS.cutAsRuntype)
                {
                  BIDDINGRUNLIST[numTotalRuns[nK]][nK].RUNSrecordID = RUNS.recordID;
                  BIDDINGRUNLIST[numTotalRuns[nK]][nK].runNumber = RUNS.runNumber;
                  BIDDINGRUNLIST[numTotalRuns[nK]][nK].runtype = RUNS.cutAsRuntype;
                  BIDDINGRUNLIST[numTotalRuns[nK]][nK].bTaken = FALSE;
                  numTotalRuns[nK]++;
                  bFound = TRUE;
                }
              }
              if(bFound)
                break;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        }
      }
      StatusBar(-1L, -1L);
//
//  Figure out the off days
//
      StatusBarText("Calculating Days Off...");
      totalShifts = 0;
      for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
      {
        totalShifts += numTotalRuns[nI];
      }
      total40HourShifts = totalShifts / 5;
      totalExtraOffDays = totalShifts - (total40HourShifts * 5);
      for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
      {
        totalOffDays[nI] = total40HourShifts - numTotalRuns[nI];
        remainingOffDays[nI] = totalOffDays[nI];
      }
      memset(pairs, 0x00, sizeof(int) * NUMCOMBINATIONS);
//
//  Sat/Sun
//
      pairs[OFFSATSUN] = min(totalOffDays[SAT], totalOffDays[SUN]);
      remainingOffDays[SAT] -= pairs[OFFSATSUN];
      remainingOffDays[SUN] -= pairs[OFFSATSUN];
//
//  Sun/Mon
//
      pairs[OFFSUNMON] = min(remainingOffDays[SUN], remainingOffDays[MON]);
      remainingOffDays[SUN] -= pairs[OFFSUNMON];
      remainingOffDays[MON] -= pairs[OFFSUNMON];
//
//  Whose got more than zero - sun or mon?
//
      if(remainingOffDays[SUN] > 0)
      {
//
//  Sun/Tue and Sun/Wed
//
        split = remainingOffDays[SUN] / 2;
        pairs[OFFSUNTUE] = split + remainingOffDays[SUN] % 2;
        pairs[OFFSUNWED] = split;
        remainingOffDays[SUN] = 0;
        remainingOffDays[TUE] -= pairs[OFFSUNTUE];
        remainingOffDays[WED] -= pairs[OFFSUNWED];
      }
//
//  Mon/Tue and Mon/Wed
//
      else
      {
        split = remainingOffDays[MON] / 2;
        pairs[OFFMONTUE] = split + remainingOffDays[MON] % 2;
        pairs[OFFMONWED] = split;
        remainingOffDays[MON] = 0;
        remainingOffDays[TUE] -= pairs[OFFMONTUE];
        remainingOffDays[WED] -= pairs[OFFMONWED];
      }
//
//  Tue/Wed
//
      pairs[OFFTUEWED] = min(remainingOffDays[TUE], remainingOffDays[WED]);
      remainingOffDays[TUE] -= pairs[OFFTUEWED];
      remainingOffDays[WED] -= pairs[OFFTUEWED];
      if(totalExtraOffDays == 1)
        pairs[OFFTUEWED]++;
//
//  Thu/Fri
//
      pairs[OFFTHUFRI] = min(remainingOffDays[THU], remainingOffDays[FRI]);
      remainingOffDays[THU] -= pairs[OFFTHUFRI];
      remainingOffDays[FRI] -= pairs[OFFTHUFRI];
//
//  Display them
//
      strcpy(tempString, "");
      for(nI = 0; nI < 7; nI++)
      {
        if(remainingOffDays[nI] != 0)
        {
          if(strcmp(tempString, "") != 0)
            strcat(tempString, ", ");
          strcat(tempString, szDays[nI]);
        }
      }
      if(strcmp(tempString, "") == 0)
        strcpy(tempString, "None");
      SendMessage(hCtlEXTRAOFF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Now reconcile it with the already rostered runs
//
      StatusBarText("Examining Existing Roster...");
      ROSTERKey1.DIVISIONSrecordID = pBIDDING->pDI->fileInfo.divisionRecordID;
      ROSTERKey1.rosterNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      while(rcode2 == 0 &&
            ROSTER.DIVISIONSrecordID == pBIDDING->pDI->fileInfo.divisionRecordID)
      {
        testBit = 0;
        for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
        {
          if(ROSTER.WEEK[rosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
            testBit |= dayBits[nI];
          else
          {
            for(nJ = 0; nJ < numTotalRuns[nI]; nJ++)
            {
              if(ROSTER.WEEK[rosterWeek].RUNSrecordIDs[nI] == BIDDINGRUNLIST[nJ][nI].RUNSrecordID)
              {
                BIDDINGRUNLIST[nJ][nI].bTaken = TRUE;
                break;
              }
            }
          }
        }
        if(testBit & SUNBIT)
        {
          if(testBit & SATBIT)
            pairs[OFFSATSUN]--;
          else if(testBit & MONBIT)
            pairs[OFFSUNMON]--;
          else if(testBit & TUEBIT)
            pairs[OFFSUNTUE]--;
          else if(testBit & WEDBIT)
            pairs[OFFSUNWED]--;
        }
        else if(testBit & MONBIT)
        {
          if(testBit & TUEBIT)
            pairs[OFFMONTUE]--;
          else if(testBit & WEDBIT)
            pairs[OFFMONWED]--;
        }
        else if(testBit & TUEBIT && testBit & WEDBIT)
          pairs[OFFTUEWED]--;
        else if(testBit & THUBIT && testBit & FRIBIT)
          pairs[OFFTHUFRI]--;
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      }
      StatusBarEnd();
//
//  Display the days off combinations
//
      for(nI = 0; nI < NUMCOMBINATIONS; nI++)
      {
        hCtl = GetDlgItem(hWndDlg, offDays[nI]);
        SendMessage(hCtl, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
        if(pairs[nI] <= 0)
        {
          pairs[nI] = 0;
          EnableWindow(hCtl, FALSE);
        }
        sprintf(tempString, szFormatString, pairs[nI]);
        SendMessage(hCtl, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      }
//
//  Disable the radio buttons until he has a pair
//
      EnableWindow(hCtlDAY1, FALSE);
      EnableWindow(hCtlDAY2, FALSE);
      EnableWindow(hCtlDAY3, FALSE);
      EnableWindow(hCtlDAY4, FALSE);
      EnableWindow(hCtlDAY5, FALSE);
//
//  Disable "Verify" button until the roster is full
//
      EnableWindow(hCtlIDVERIFY, FALSE);
//
//  Set all the selected runs to NO_RECORD
//
      for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
      {
        selectedRuns[nI] = NO_RECORD;
      }
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDSUSPEND, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  Days Off Radio Buttons
//
        case BIDDING_SATSUN:
        case BIDDING_SUNMON:
        case BIDDING_SUNTUE:
        case BIDDING_SUNWED:
        case BIDDING_MONTUE:
        case BIDDING_MONWED:
        case BIDDING_TUEWED:
        case BIDDING_THUFRI:
          if(!SendDlgItemMessage(hWndDlg, wmId, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            break;  // This takes care of the control getting focus on initialization
          SendMessage(hCtlAVAILABLE, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlCURRENT, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlIDVERIFY, FALSE);
          SendMessage(hCtlRUNCHARACTERISTICS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Run characteristics");
          SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          SendMessage(hCtlSPREADOT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlMAKEUP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlOVERTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlPAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          EnableWindow(hCtlIDSHOWDIRECTIONS, FALSE);              
          for(nI = 0; nI < 8; nI++)
          {
            if(wmId == offDays[nI])
              break;
          }
          testBit = pairBits[nI];
          nJ = 0;
          for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
          {
            if(testBit & (1 << nI))
            {
              sprintf(tempString, "%s\tNo run selected", szDays[nI]);
              hCtl = GetDlgItem(hWndDlg, dayButtons[nJ]);
              SendMessage(hCtl, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szDays[nI]);
              EnableWindow(hCtl, TRUE);
              SendMessage(hCtl, BM_SETCHECK, (WPARAM)(nJ == 0), (LPARAM)0);
              nJ++;
            }
            else
            {
              sprintf(tempString, "%s\t* Off *", szDays[nI]);
            }
            SendMessage(hCtlCURRENT, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          }
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(BIDDING_DAY1, 0), (LPARAM)0);
          break;
//
//  BIDDING_AVAILABLE List Box
//
        case BIDDING_AVAILABLE:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              for(nI = 0; nI < 5; nI++)
              {
                hCtl = GetDlgItem(hWndDlg, dayButtons[nI]);
                if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                {
                  SendMessage(hCtl, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
                  break;
                }
              }
              for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
              {
                if(strcmp(tempString, szDays[nI]) == 0)
                {
                  serviceIndex = nI;
                  break;
                }
              }
              nI = (int)SendMessage(hCtlAVAILABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              selectedRuns[serviceIndex] = SendMessage(hCtlAVAILABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              SendMessage(hCtlAVAILABLE, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
              sprintf(szarString, "%s\t%s", szDays[serviceIndex], tempString);
              SendMessage(hCtlCURRENT, LB_DELETESTRING, (WPARAM)serviceIndex, (LPARAM)0);
              SendMessage(hCtlCURRENT, LB_INSERTSTRING, (WPARAM)serviceIndex, (LONG)(LPSTR)szarString);
              nJ = 0;
              for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
              {
                if(selectedRuns[nI] != NO_RECORD)
                  nJ++;
              }
              EnableWindow(hCtlIDVERIFY, nJ == 5);
              tempLong = selectedRuns[serviceIndex];
              nI = LOWORD(tempLong);
              nJ = HIWORD(tempLong);
              DisplayRunAnalysis(hWndDlg, (HDC)NULL,
                    BIDDINGRUNLIST[nI][nJ].RUNSrecordID, FALSE, NO_RECORD);
              break;
          }
          break;
//
//  BIDDING_DAY1
//  BIDDING_DAY2
//  BIDDING_DAY3
//  BIDDING_DAY4
//  BIDDING_DAY5
//
        case BIDDING_DAY1:
        case BIDDING_DAY2:
        case BIDDING_DAY3:
        case BIDDING_DAY4:
        case BIDDING_DAY5:
          SendDlgItemMessage(hWndDlg, wmId, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
          {
            if(strcmp(tempString, szDays[nI]) == 0)
            {
              numRuns = numTotalRuns[nI];
              serviceIndex = nI;
              break;
            }
          }
          SendMessage(hCtlAVAILABLE, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
          for(nI = 0; nI < numRuns; nI++)
          {
            if(!BIDDINGRUNLIST[nI][serviceIndex].bTaken)
            {
              nK = (short int)LOWORD(BIDDINGRUNLIST[nI][serviceIndex].runtype);
              nL = (short int)HIWORD(BIDDINGRUNLIST[nI][serviceIndex].runtype);
              strcpy(szarString, nK < 0 || nL < 0 ? "Not typed" : RUNTYPE[nK][nL].localName);
              sprintf(tempString, "%ld\t(%s)", BIDDINGRUNLIST[nI][serviceIndex].runNumber, szarString);
              nJ = SendMessage(hCtlAVAILABLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
              tempLong = MAKELPARAM(nI, serviceIndex);
              SendMessage(hCtlAVAILABLE, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)tempLong);
              if(tempLong == selectedRuns[serviceIndex])
              {
                SendMessage(hCtlAVAILABLE, LB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
                nK = LOWORD(tempLong);
                nL = HIWORD(tempLong);
                DisplayRunAnalysis(hWndDlg, (HDC)NULL,
                      BIDDINGRUNLIST[nK][nL].RUNSrecordID, FALSE, NO_RECORD);
              }
            }
            if(SendMessage(hCtlAVAILABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0) == LB_ERR)
            {
              SendMessage(hCtlRUNCHARACTERISTICS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Run characteristics");
              SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              SendMessage(hCtlSPREADOT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlMAKEUP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlOVERTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              SendMessage(hCtlPAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
              EnableWindow(hCtlIDSHOWDIRECTIONS, FALSE);              
            }
          }
          break;
//
//  IDVERIFY
//
        case IDVERIFY:
//
//  Determine which records to compare
//
          nJ = 0;
          for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
          {
            if(selectedRuns[nI] == NO_RECORD)
              continue;
            check[nJ++] = nI;
          }
//
//  Get data for each run
//
          for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
          {
            if(selectedRuns[nI] == NO_RECORD)
              continue;
            nJ = LOWORD(selectedRuns[nI]);
            nK = HIWORD(selectedRuns[nI]);
            RUNSKey0.recordID = BIDDINGRUNLIST[nJ][nK].RUNSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            numPieces[nI] = GetRunElements(hWndDlg, &RUNS, &PROPOSEDRUN, &COST, TRUE);
            memcpy(&RUNDATA[nI], &RUNSVIEW, sizeof(RUNSVIEWDef) * numPieces[nI]);
          }
//
//  Go through and see if there's a valid match
//
          bGoodCombo = TRUE;
          for(nI = 0; nI < 5; nI++)
          {
            firstRun = check[nI];
            secondRun = nI == 4 ? check[0] : check[nI + 1];
//
//  Route
//
            if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEROUTE)
            {
              if(RUNDATA[firstRun][0].ROUTESrecordID != RUNDATA[secondRun][0].ROUTESrecordID)
              {
                bGoodCombo = FALSE;
                reason = ERROR_207;
                break;
              }
            }
//
//  Run Number
//
            if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMERUNNUMBER)
            {
              if(RUNDATA[firstRun][0].runNumber != RUNDATA[secondRun][0].runNumber)
              {
                bGoodCombo = FALSE;
                reason = ERROR_208;
                break;
              }
            }
//
//  Same start location
//
            if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMESTART)
            {
              if(RUNDATA[firstRun][0].runOnNODESrecordID != RUNDATA[secondRun][0].runOnNODESrecordID)
              {
                bGoodCombo = FALSE;
                reason = ERROR_209;
                break;
              }
            }
//
//  Same end location
//
            if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEEND)
            {
              if(RUNDATA[firstRun][numPieces[firstRun] - 1].runOnNODESrecordID !=
                    RUNDATA[secondRun][numPieces[secondRun] - 1].runOnNODESrecordID)
              {
                bGoodCombo = FALSE;
                reason = ERROR_210;
                break;
              }
            }
//
//  Runtype choices
//
//  0 - Must be the same
//
            if(ROSTERPARMS.runtypeChoice == 0)
            {
              if(RUNDATA[firstRun][0].runType != RUNDATA[secondRun][0].runType)
              {
                bGoodCombo = FALSE;
                reason = ERROR_211;
                break;
              }
            }
//
//  1 - Can differ(but don't have to)
//
            else if(ROSTERPARMS.runtypeChoice == 1)
            {
            }
//
//  2 - Can combine to other specific runtypes(skip this test if either of them is < 0)
//
            else
            {
              if(RUNDATA[firstRun][0].runType >= 0 && RUNDATA[secondRun][0].runType >= 0)
              {
                nJ = (short int)LOWORD(RUNDATA[firstRun][0].runType);
                nK = (short int)HIWORD(RUNDATA[firstRun][0].runType);
                nL = (short int)LOWORD(RUNDATA[secondRun][0].runType);
                nM = (short int)HIWORD(RUNDATA[secondRun][0].runType);
                if(!(ROSTERPARMS.match[nJ][nK][nM] &(2 << nL)))
                {
                  bGoodCombo = FALSE;
                  reason = ERROR_212;
                  break;
                }
              }
            }
//
//  Minimum Off Time
//
            if(secondRun == firstRun + 1 || (firstRun == 6 && secondRun == 0))
            {
              if(86400L + RUNDATA[secondRun][0].runOnTime -
                    RUNDATA[firstRun][numPieces[firstRun] - 1].runOffTime < ROSTERPARMS.minOffTime)
              {
                bGoodCombo = FALSE;
                reason = ERROR_213;
                break;
              }
            }
//
//  Start Variance
//
            if(labs(RUNDATA[firstRun][0].runOnTime - RUNDATA[secondRun][0].runOnTime) > ROSTERPARMS.startVariance / 2)
            {
              bGoodCombo = FALSE;
              reason = ERROR_214;
              break;
            }
          } // nI
//
//  What did we get?
//
          if(bGoodCombo)
          {
            MessageBeep(MB_ICONQUESTION);
            LoadString(hInst, ERROR_215, tempString, TEMPSTRING_LENGTH);
            if(MessageBox(hWndDlg, tempString, TMS, MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
              for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
              {
                if(selectedRuns[nI] == NO_RECORD)
                  RUNSrecordIDs[nI] = NO_RECORD;
                else
                {
                  nJ = LOWORD(selectedRuns[nI]);
                  nK = HIWORD(selectedRuns[nI]);
                  RUNSrecordIDs[nI] = BIDDINGRUNLIST[nJ][nK].RUNSrecordID;
                }
              }
              if(!PrintRoster(hWndDlg, pBIDDING, RUNSrecordIDs))
                MessageBox(hWndDlg, "Print Error", TMS, MB_OK);
            }
            MessageBeep(MB_ICONINFORMATION);
            LoadString(hInst, ERROR_216, tempString, TEMPSTRING_LENGTH);
            if(MessageBox(hWndDlg, tempString, TMS, MB_OKCANCEL | MB_ICONINFORMATION) == IDOK)
            {
//
//  Set up the ROSTER record
//
              rcode2 = btrieve(B_GETLAST, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
              tempLong = (rcode2 == 0 ? ROSTER.rosterNumber : 0L) + 1;
              rcode2 = btrieve(B_GETLAST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
              ROSTER.recordID = AssignRecID(rcode2, ROSTER.recordID);
              ROSTER.rosterNumber = tempLong;
              ROSTER.DIVISIONSrecordID = pBIDDING->pDI->fileInfo.divisionRecordID;
              ROSTER.DRIVERSrecordID = pBIDDING->DRIVERS.recordID;
              ROSTER.COMMENTSrecordID = NO_RECORD;
              ROSTER.WEEK[rosterWeek].flags = 0L;
              for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
              {
                if(selectedRuns[nI] == NO_RECORD)
                  ROSTER.WEEK[rosterWeek].RUNSrecordIDs[nI] = NO_RECORD;
                else
                {
                  nJ = LOWORD(selectedRuns[nI]);
                  nK = HIWORD(selectedRuns[nI]);
                  ROSTER.WEEK[rosterWeek].RUNSrecordIDs[nI] = BIDDINGRUNLIST[nJ][nK].RUNSrecordID;
                }
              }
              rcode2 = btrieve(B_INSERT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
              if(rcode2 != 0)
              {
                sprintf(tempString, "Insert failed.\nRcode2=%d\nRecID=%ld\nRostNum=%ld",
                      rcode2, ROSTER.recordID, ROSTER.rosterNumber);
                MessageBox(hWndDlg, tempString, TMS, MB_OK);
              }
              EndDialog(hWndDlg, TRUE);
            }
          }
          else
          {
            MessageBeep(MB_ICONINFORMATION);
            LoadString(hInst, reason, szarString, sizeof(szarString));
            sprintf(tempString, "%s %s and %s", szarString, szDays[firstRun], szDays[secondRun]);
            MessageBox(hWndDlg, tempString, TMS, MB_OK | MB_ICONINFORMATION);
          }
          break;
//
//  IDSHOWDIRECTIONS
//
        case IDSHOWDIRECTIONS:
          nI = SendMessage(hCtlAVAILABLE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
          tempLong = SendMessage(hCtlAVAILABLE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          nJ = LOWORD(tempLong);
          nK = HIWORD(tempLong);
          if(bUseCISPlan)
          {
            RUNSKey0.recordID = BIDDINGRUNLIST[nJ][nK].RUNSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            GetRunElements(hWndDlg, &RUNS, &PROPOSEDRUN, &COST, TRUE);
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
                hWndMain, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&COST);
          }
          else
          {
            runNumber = BIDDINGRUNLIST[nJ][nK].runNumber;
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
                hWndMain, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&runNumber);
          }
          break;
//
//  IDALTER
//  IDSKIP
//  IDSUSPEND
//
        case IDALTER:
        case IDSKIP:
        case IDSUSPEND:
          strcpy(SUPERVISORPassedData.szMessage, "Enter Bidding Supervisor Password");
          SUPERVISORPassedData.bAuthenticated = FALSE;
          SUPERVISORPassedData.flags = SUPERVISOR_FLAG_BIDDING;
          if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUPERVISOR),
                hWndMain, (DLGPROC)SUPERVISORMsgProc, (LPARAM)&SUPERVISORPassedData))
          {
            if(SUPERVISORPassedData.bAuthenticated)
            {
              switch(wmId)
              {
                case IDALTER:
                  updateRecordID = pBIDDING->DRIVERS.recordID;
                  DRIVERSKey0.recordID = pBIDDING->DRIVERS.recordID;
                  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
                  btrieve(B_GETPOSITION, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
                  btrieve(B_GETDIRECT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
                  break;
          
                case IDSKIP:
                  EndDialog(hWndDlg, TRUE);
                  break;
          
                case IDSUSPEND:
                  SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                  break;
              }
            }
          } 
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Roster_Table);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//  DisplayRunAnalysis()
//

#define TABSET 8

//  Block Pce  OnLoc  OnTime  OffTime  OffLoc  Plat  Rpt   Trv   Total
//  XXXXX XX   XXXX   XXXXX   XXXXX    XXXX    XXXXX XXXXX XXXXX XXXXXXXX
//  3         4         5         6         7         8         9
//  01234567890123456789012345678901234567890123456789012345678901234567890

void DisplayRunAnalysis(HWND hWnd, HDC hDC, long RUNSrecordID, BOOL bTitleOnly, int dayIndex)
{
  static  int tabPos[TABSET] = {7, 12, 19, 27, 35, 42, 48, 54};
  static  int tprtTabPos[TABSET + 1] = {36, 41, 48, 56, 65, 73, 79, 85, 91};
  int     printerTabPos[TABSET + 1];
  PROPOSEDRUNDef PROPOSEDRUN;
  EXTRABOARDDef  XBPrior[MAXPIECES];
  EXTRABOARDDef  XBAfter[MAXPIECES];

  HCURSOR saveCursor;
  time_t  ltime;
  WORD    DlgWidthUnits;
  BOOL    bDialog;
  char    nodeName[NODES_ABBRNAME_LENGTH + 1];
  long    onTime;
  long    offTime;
  long    cutAsRuntype;
  long    runNumber;
  long    serviceRecordID;
  long    divisionRecordID;
  long    pieceNumber;
  long    totalPlat;
  long    totalRpt;
  long    totalTrv;
  long    travelTime;
  long    tempLong;
  int     adjustedTabPos[TABSET];
  int     nI;
  int     nJ;
  int     rcode2;
//
//  Where are we going?
//
  bDialog = hWnd != NULL;
//
//  Set up the handles to the controls
//
  if(bDialog)
  {
//
//  Hourglass
//
    saveCursor = SetCursor(hCursorWait);
    SendMessage(hCtlTITLE, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    SendMessage(hCtlLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Set up the tab stops for BIDDING_TITLE and BIDDING_LIST
//
    DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
    for(nJ = 0; nJ < TABSET; nJ++)
    {
      adjustedTabPos[nJ] = (DlgWidthUnits * tabPos[nJ] * 2);
    }
    SendMessage(hCtlTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
    SendMessage(hCtlLIST, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
  }
//
//  Display the title
//
  if(bDialog)
  {
     LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
     SendMessage(hCtlTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
  }
  else
  {
    for(nI = 0; nI < TABSET + 1; nI++)
    {
      printerTabPos[nI] = tprtTabPos[nI] * xChar;
    }
    if(bTitleOnly)
    {
      nPageOutputLine += 2;
      TextOut(hDC, 0, yChar * nPageOutputLine, szPropertyName, strlen(szPropertyName));
      nPageOutputLine += 2;
      time(&ltime);
      sprintf(tempString, "Bid selection made: %s", ctime(&ltime)); //  The last char of ctime is junk
      TextOut(hDC, 0, yChar * nPageOutputLine, tempString, strlen(tempString) - 1);
      nPageOutputLine++;
      sprintf(tempString, "Bid effective date: %s", szEffectiveDate);
      TextOut(hDC, 0, yChar * nPageOutputLine, tempString, strlen(tempString));
      nPageOutputLine += 2;
      LoadString(hInst, TEXT_080, tempString, TEMPSTRING_LENGTH);
      strcat(tempString, "\tTotal");
      TabbedTextOut(hDC, xChar * 30, yChar * nPageOutputLine,
            tempString, strlen(tempString), TABSET + 1, printerTabPos, 0);
      nPageOutputLine++;
      for(nI = 0; nI < (int)strlen(tempString); nI++)
      {
        if(tempString[nI] != '\t')
          tempString[nI] = '~';
      }
      TabbedTextOut(hDC, xChar * 30, yChar * nPageOutputLine,
            tempString, strlen(tempString), TABSET + 1, printerTabPos, 0);
      nPageOutputLine++;
      return;
    }
  }
//
//  Get the run and set up PROPOSEDRUN
//
  RUNSKey0.recordID = RUNSrecordID;
  btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  runNumber = RUNS.runNumber;
  divisionRecordID = RUNS.DIVISIONSrecordID;
  serviceRecordID = RUNS.SERVICESrecordID;
  pieceNumber = 0;
  cutAsRuntype = RUNS.cutAsRuntype;
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == divisionRecordID &&
        RUNS.SERVICESrecordID == serviceRecordID &&
        RUNS.runNumber == runNumber)
  {
    RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
          RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
    PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
    PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
    PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
    PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
    PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
    PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
    PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
    PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
    PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
    PROPOSEDRUN.piece[pieceNumber].after.endTime = RUNS.after.endTime;
    XBPrior[pieceNumber].startTime = RUNS.prior.startTime;
    XBPrior[pieceNumber].endTime = RUNS.prior.endTime;
    XBAfter[pieceNumber].startTime = RUNS.after.startTime;
    XBAfter[pieceNumber].endTime = RUNS.after.endTime;
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    pieceNumber++;
  }
  PROPOSEDRUN.numPieces = (int)pieceNumber;
//
//  Cost the run
//
  bGenerateTravelInstructions = TRUE;
  RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
  bGenerateTravelInstructions = FALSE;
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
      EnableWindow(hCtlIDSHOWDIRECTIONS, TRUE);
    else
      EnableWindow(hCtlIDSHOWDIRECTIONS, numTravelInstructions > 0);
  }
//
//  Fill in the controls
//
//  Run characteristics group box title
//
  nI = (short int)LOWORD(COST.runtype);
  nJ = (short int)HIWORD(COST.runtype);
  if(nI < 0 || nJ < 0)
    LoadString(hInst, TEXT_052, tempString, TEMPSTRING_LENGTH);
  else
    strcpy(tempString, RUNTYPE[nI][nJ].localName);
  if(bDialog)
  {
    sprintf(szarString, "Characteristics of run number %d (%s)", runNumber, tempString);
    SendMessage(hCtlRUNCHARACTERISTICS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szarString);
  }
  else
  {
    sprintf(szarString, "%s: Run %4d (%s)", szDays[dayIndex], runNumber, tempString);
    TextOut(hDC, 0, yChar * nPageOutputLine, szarString, strlen(szarString));
  }
//
//  The list box of the pieces of the run
//
  totalPlat = 0;
  totalRpt = 0;
  totalTrv = 0;
  for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
  {
//
//  Block Number
//
    TRIPSKey0.recordID = PROPOSEDRUN.piece[nI].fromTRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    ltoa(TRIPS.standard.blockNumber, tempString, 10);
    strcat(tempString, "\t");
//
//  Piece Number
//
    sprintf(szarString, "%3d", nI + 1);
    strcat(tempString, szarString);
    strcat(tempString, "\t");
//
//  On Location
//
    NODESKey0.recordID = PROPOSEDRUN.piece[nI].fromNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(nodeName, NODES_ABBRNAME_LENGTH);
    strcat(tempString, nodeName);
    strcat(tempString, "\t");
//
//  On Time
//
    strcat(tempString, Tchar(PROPOSEDRUN.piece[nI].fromTime));
    strcat(tempString, "\t");
//
//  Off Time
//
    strcat(tempString, Tchar(PROPOSEDRUN.piece[nI].toTime));
    strcat(tempString, "\t");
//
//  Off Location
//
    NODESKey0.recordID = PROPOSEDRUN.piece[nI].toNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(nodeName, NODES_ABBRNAME_LENGTH);
    strcat(tempString, nodeName);
    strcat(tempString, "\t");
//
//  Platform time
//
    strcat(tempString, chhmm(COST.PIECECOST[nI].platformTime));
    strcat(tempString, "\t");
    totalPlat += COST.PIECECOST[nI].platformTime;
//
//  Report and Travel times
//
    tempLong = PROPOSEDRUN.piece[nI].fromTime - COST.PIECECOST[nI].reportTime;
    strcat(tempString, Tchar(tempLong));
    strcat(tempString, "\t");
    totalRpt += COST.PIECECOST[nI].reportTime;
    travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
    strcat(tempString, chhmm(travelTime));
    strcat(tempString, "\t");
    totalTrv += travelTime;
    if(bDialog)
      SendMessage(hCtlLIST, LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
    else
    {
      if(PROPOSEDRUN.numPieces == 1)
      {
        sprintf(szarString, "%s", chhmm(COST.TOTAL.payTime));
        strcat(tempString, szarString);
      }
      TabbedTextOut(hDC, xChar * 30, yChar * nPageOutputLine,
            tempString, strlen(tempString), TABSET + 1, printerTabPos, 0);
      nPageOutputLine++;
    }
  }
//
//  Add a total line if there's more than one piece
//
  if(PROPOSEDRUN.numPieces > 1)
  {
    strcpy(tempString, "\t\t\t\t\t\t");
    strcat(tempString, chhmm(totalPlat));
    strcat(tempString, "\t");
    strcat(tempString, chhmm(totalRpt));
    strcat(tempString, "\t");
    strcat(tempString, chhmm(totalTrv));
    if(bDialog)
      SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    else
    {
      sprintf(szarString, "\t%s", chhmm(COST.TOTAL.payTime));
      strcat(tempString, szarString);
      TabbedTextOut(hDC, xChar * 30, yChar * nPageOutputLine,
            tempString, strlen(tempString), TABSET + 1, printerTabPos, 0);
      nPageOutputLine++;
    }
  }
  nPageOutputLine++;
//
//  Spread O/T
//
  if(bDialog)
  {
    SendMessage(hCtlSPREADOT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.spreadOvertime));
//
//  Makeup time
//
    SendMessage(hCtlMAKEUP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.makeUpTime));
//
//  Paid breaks
//
    SendMessage(hCtlPAIDBREAKS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.paidBreak));
//
//  Overtime
//
    SendMessage(hCtlOVERTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.overTime));
//
//  Pay time
//
    SendMessage(hCtlPAYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)chhmm(COST.TOTAL.payTime));
//
//  Restore the cursor
//
    SetCursor(saveCursor);
//
//  Enable "Travel Instructions" button if using dynamic travels
//
    if(bUseDynamicTravels)
    {
      if(bUseCISPlan)
        EnableWindow(hCtlIDSHOWDIRECTIONS, TRUE);
      else
        EnableWindow(hCtlIDSHOWDIRECTIONS, numTravelInstructions > 0);
    }
  }
}

//
//  PrintRoster()
//
BOOL PrintRoster(HWND hWnd, BIDDINGDef *pBIDDING, long RUNSrecordIDs[])
{
  static DOCINFO  di = { sizeof(DOCINFO), "", NULL };
  static PRINTDLG pd;
  static PAGESETUPDLG psd;
  char   docName[256];
  TEXTMETRIC tm;
  BOOL bSuccess;
  WORD iColCopy, iNoiColCopy;
  int  iCharsPerLine, iLinesPerPage, iTotalLines, nI;
  DEVMODE *pDevMode;
  HGLOBAL hDM;
  DEVNAMES *pDevNames;
  OSVERSIONINFO osv;
  HDC hDC;
  int numCopies;
  char szDriver[64];
  char szDevice[64];
  char szOutput[64];

  osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osv);

//
//  Printer setup
//
//  Windows 95/98 - Use Print Setup
//
  if(osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
  {
    pd.lStructSize         = sizeof(PRINTDLG);
    pd.hwndOwner           = hWnd;
    pd.hDevMode            = NULL;
    pd.hDevNames           = NULL;
    pd.hDC                 = NULL;
    pd.Flags               = PD_ALLPAGES | PD_COLLATE | PD_RETURNDC | PD_HIDEPRINTTOFILE |
                             PD_NOPAGENUMS | PD_NOSELECTION ;
    pd.nFromPage           = 1;
    pd.nToPage             = 0xFFFF;
    pd.nMinPage            = 1;
    pd.nMaxPage            = 0xFFFF;
    pd.nCopies             = 2;
    pd.hInstance           = NULL;
    pd.lCustData           = 0L;
    pd.lpfnPrintHook       = NULL;
    pd.lpfnSetupHook       = NULL;
    pd.lpPrintTemplateName = NULL;
    pd.lpSetupTemplateName = NULL;
    pd.hPrintTemplate      = NULL;
    pd.hSetupTemplate      = NULL;

    if(!PrintDlg(&pd))
      return TRUE;
  

    numCopies = pd.nCopies;

    pDevMode = (DEVMODE *)GlobalLock(pd.hDevMode);
    hDC = pd.hDC;
    pDevMode->dmOrientation = DMORIENT_LANDSCAPE; //  Set landscape
    ResetDC(hDC, pDevMode);
    TMSGlobalFree(pd.hDevMode);
  }
//
//  Windows NT/2000 - Use Page Setup
//
  else
  {
    hDM = GlobalAlloc(GHND, sizeof(DEVMODE));
    pDevMode = (DEVMODE *)GlobalLock(hDM);
    pDevMode->dmSize = sizeof(DEVMODE);
    pDevMode->dmFields = DM_COPIES | DM_ORIENTATION;
    pDevMode->dmCopies = 2;
    pDevMode->dmOrientation = DMORIENT_LANDSCAPE;

    memset(&psd, 0x00, sizeof(PAGESETUPDLG));
    psd.lStructSize = sizeof(PAGESETUPDLG);
    psd.hwndOwner = hWnd;
    psd.hDevMode = hDM;
    psd.hDevNames = (HANDLE)NULL;
    psd.Flags = PSD_DEFAULTMINMARGINS | PSD_ENABLEPAGEPAINTHOOK |
                PSD_DISABLEMARGINS | PSD_DISABLEORIENTATION; 
    psd.hInstance = hInst;
    psd.lCustData = (LPARAM)NULL;
    psd.lpfnPagePaintHook = PaintHook;
    if(!PageSetupDlg(&psd))
    {
      return TRUE;
    }
  
    pDevMode = (DEVMODE *)GlobalLock(psd.hDevMode);
    pDevNames = (DEVNAMES *)GlobalLock(psd.hDevNames);
    memcpy(szarString, pDevNames, sizeof(szarString));
    strcpy(szDriver, &szarString[pDevNames->wDriverOffset]);
    strcpy(szDevice, &szarString[pDevNames->wDeviceOffset]);
    strcpy(szOutput, &szarString[pDevNames->wOutputOffset]);
    hDC = CreateDC(szDriver, szDevice, NULL, pDevMode);
    numCopies = 1;  // The printer prints two
    TMSGlobalFree(hDM);
    TMSGlobalFree(psd.hDevMode);
    TMSGlobalFree(psd.hDevNames);
  }
//
//  Set the orientation
//

  iTotalLines = 60;

  bSuccess = GetTextMetrics(hDC, &tm);
  if(bSuccess)
  {
    yChar = tm.tmHeight + tm.tmExternalLeading;
    xChar = tm.tmMaxCharWidth - 1;
    iCharsPerLine = GetDeviceCaps(hDC, HORZRES) / tm.tmAveCharWidth;
    iLinesPerPage = GetDeviceCaps(hDC, VERTRES) / yChar;
  }
  else
  {
    yChar = 1;
    xChar = 25;
    iCharsPerLine = 256;
    iLinesPerPage = 256;
  }


  EnableWindow(hWnd, FALSE);

  bSuccess   = TRUE;
  GetWindowText(hWnd, docName, sizeof(docName));
  di.lpszDocName = docName;

  if(StartDoc(hDC, &di) > 0)
  {
    for(iColCopy = 0; iColCopy < 1; iColCopy++)
    {
      for(iNoiColCopy = 0; iNoiColCopy < numCopies; iNoiColCopy++)
      {
        if(StartPage(hDC) < 0)
        {
          bSuccess = FALSE;
          break;
        }
        nPageOutputLine = 0;
        DisplayRunAnalysis((HWND)NULL, hDC, (long)NULL, TRUE, NO_RECORD);
        for(nI = 0; nI < BIDDING_NUMSERVICEDAYS; nI++)
        {
          if(RUNSrecordIDs[nI] == NO_RECORD)
          {
            sprintf(tempString, "%s: * Off *", szDays[nI]);
            TextOut(hDC, 0, yChar * nPageOutputLine, tempString, strlen(tempString));
            nPageOutputLine += 2;
          }
          else
          {
            DisplayRunAnalysis((HWND)NULL, hDC, RUNSrecordIDs[nI], FALSE, nI);
          }
        }
        nPageOutputLine += 2;
        strcpy(tempString, "____________________________________________");
        TextOut(hDC, 0, yChar * nPageOutputLine, tempString, strlen(tempString));
        nPageOutputLine++;
        strncpy(szarString, pBIDDING->DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(szarString, DRIVERS_LASTNAME_LENGTH);
        sprintf(tempString, "Signed by: %s, ", szarString);
        strncpy(szarString, pBIDDING->DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, " (Badge ");
        strncpy(szarString, pBIDDING->DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(szarString, DRIVERS_BADGENUMBER_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, ")");
        TextOut(hDC, 0, yChar * nPageOutputLine, tempString, strlen(tempString));

        if(EndPage(hDC) < 0)
        {
          bSuccess = FALSE;
          break;
        }
      }
      if(!bSuccess)
        break;
    }
  }
  else
  {
    bSuccess = FALSE;
  }
  if(bSuccess)
  {
    EndDoc(hDC);
  }

  EnableWindow(hWnd, TRUE);
  DeleteDC(hDC);

  return(bSuccess);
}

BOOL CALLBACK PaintHook(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
  LPRECT lprc; 
  COLORREF crMargRect; 
  HDC hdc, hdcOld; 
 
  switch(uMsg)
  { 
// 
//  Drawing the margin rectangle
//
    case WM_PSD_MARGINRECT: 
      hdc = (HDC) wParam; 
      lprc = (LPRECT) lParam; 
//
//  Get the system highlight color
//
      crMargRect = GetSysColor(COLOR_HIGHLIGHT); 
// 
//  Create a dash-dot pen of the system highlight color and 
//  select it into the DC of the sample page. 
//
      hdcOld = SelectObject(hdc, CreatePen(PS_DASHDOT, 0, crMargRect)); 
//
//  Draw the margin rectangle
//
      Rectangle(hdc, lprc->left, lprc->top, lprc->right,  lprc->bottom); 
// 
//  Restore the previous pen to the DC
//
      SelectObject(hdc, hdcOld); 
      return TRUE; 
 
    default: 
      return FALSE; 
  } 
  
  return TRUE; 
} 
 
 
