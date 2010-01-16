//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <io.h>
#include <fcntl.h>

BOOL PrintTripPlan(HWND, char *);
BOOL CALLBACK TRIPPLANNINGRESULTSMsgProc(HWND, UINT, WPARAM, LPARAM);

typedef struct COORDStruct
{
  float x;	// correspondes to longitude
  float y;	// correspondes to latitude
} COORDDef;

typedef struct RESULTStruct
{
  char     *szPlan;
  CISpoint *cisPointArray;
  int      numCISpoints;
} RESULTSDef;

typedef struct LANDMARKSStruct
{
  char szDescription[64];
  float x;
  float y;
} LANDMARKSDef;

#define NOW_TEXT "NOW"

BOOL CALLBACK TRIPPLANMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlORIGIN;
  static HANDLE hCtlORIGINADDRESS;
  static HANDLE hCtlORIGINLANDMARK;
  static HANDLE hCtlLANDMARK1;
  static HANDLE hCtlORIGINFOUND;
  static HANDLE hCtlDESTINATION;
  static HANDLE hCtlDESTINATIONADDRESS;
  static HANDLE hCtlDESTINATIONLANDMARK;
  static HANDLE hCtlLANDMARK2;
  static HANDLE hCtlDESTINATIONFOUND;
  static HANDLE hCtlTODAY;
  static HANDLE hCtlTOMORROW;
  static HANDLE hCtlDAY3;
  static HANDLE hCtlDAY4;
  static HANDLE hCtlDAY5;
  static HANDLE hCtlDAY6;
  static HANDLE hCtlDAY7;
  static HANDLE hCtlLEAVEAT;
  static HANDLE hCtlLEAVEATTIME;
  static HANDLE hCtlLEAVEEARLY;
  static HANDLE hCtlLEAVELATE;
  static HANDLE hCtlARRIVEBY;
  static HANDLE hCtlARRIVEBYTIME;
  static HANDLE hCtlFASTEST;
  static HANDLE hCtlFEWESTTRANSFERS;
  static HANDLE hCtlACCESSIBLE;
  static HANDLE hCtlBIKERACK;
  static HANDLE hCtlFIRSTDAY;
  static HANDLE hCtlMAP;
  static HANDLE hCtlLANDMARKS;
  static COORDDef origin;
  static COORDDef destination;
  static long serviceDays[ROSTER_MAX_DAYS];
  static BOOL bGotLandmarks;
  static char szNow[8];
  static LANDMARKSDef *pLANDMARKS = NULL;
  static int numLandmarks;
  static int maxLandmarks;
  RESULTSDef results;
  HCURSOR hSaveCursor;
  HANDLE hCtl;
  BOOL   bUseStreet;
  BOOL   bCalcTime;
  BOOL   bFound;
  time_t ltime;

  time_t tripTime;
  long	 serviceID;
  struct tm tripTM;
  int	 isLeave;

  char   *szDays[] = {"Monday", "Tuesday", "Wednesday",
                      "Thursday", "Friday", "Saturday", "Sunday"};
  int    dayIDs[] = {TRIPPLAN_TODAY, TRIPPLAN_TOMORROW, TRIPPLAN_DAY3,
                     TRIPPLAN_DAY4, TRIPPLAN_DAY5, TRIPPLAN_DAY6, TRIPPLAN_DAY7};
  int    nI;
  int    nJ;
  int    nK;
  short int wmEvent;
  short int wmId;
  FILE *fp;

  // return values of the trip planner.
  char	*szPlan = "This is the plan.";
  // The cisPointArray is freed the next time the trip planner is called.
  static	CISpoint	*cisPointArray = NULL;
  static	int		numCISpoints = 0;

  static	char	szOrigin[512], szDestination[512];


  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlORIGIN = GetDlgItem(hWndDlg, TRIPPLAN_ORIGIN);
      hCtlORIGINADDRESS = GetDlgItem(hWndDlg, TRIPPLAN_ORIGINADDRESS);
      hCtlORIGINLANDMARK = GetDlgItem(hWndDlg, TRIPPLAN_ORIGINLANDMARK);
      hCtlLANDMARK1 = GetDlgItem(hWndDlg, TRIPPLAN_LANDMARK1);
      hCtlORIGINFOUND = GetDlgItem(hWndDlg, TRIPPLAN_ORIGINFOUND);
      hCtlDESTINATION = GetDlgItem(hWndDlg, TRIPPLAN_DESTINATION);
      hCtlDESTINATIONADDRESS = GetDlgItem(hWndDlg, TRIPPLAN_DESTINATIONADDRESS);
      hCtlDESTINATIONLANDMARK = GetDlgItem(hWndDlg, TRIPPLAN_DESTINATIONLANDMARK);
      hCtlLANDMARK2 = GetDlgItem(hWndDlg, TRIPPLAN_LANDMARK2);
      hCtlDESTINATIONFOUND = GetDlgItem(hWndDlg, TRIPPLAN_DESTINATIONFOUND);
      hCtlTODAY = GetDlgItem(hWndDlg, TRIPPLAN_TODAY);
      hCtlTOMORROW = GetDlgItem(hWndDlg, TRIPPLAN_TOMORROW);
      hCtlDAY3 = GetDlgItem(hWndDlg, TRIPPLAN_DAY3);
      hCtlDAY4 = GetDlgItem(hWndDlg, TRIPPLAN_DAY4);
      hCtlDAY5 = GetDlgItem(hWndDlg, TRIPPLAN_DAY5);
      hCtlDAY6 = GetDlgItem(hWndDlg, TRIPPLAN_DAY6);
      hCtlDAY7 = GetDlgItem(hWndDlg, TRIPPLAN_DAY7);
      hCtlLEAVEAT = GetDlgItem(hWndDlg, TRIPPLAN_LEAVEAT);
      hCtlLEAVEATTIME = GetDlgItem(hWndDlg, TRIPPLAN_LEAVEATTIME);
      hCtlLEAVEEARLY = GetDlgItem(hWndDlg, TRIPPLAN_LEAVEEARLY);
      hCtlLEAVELATE = GetDlgItem(hWndDlg, TRIPPLAN_LEAVELATE);
      hCtlARRIVEBY = GetDlgItem(hWndDlg, TRIPPLAN_ARRIVEBY);
      hCtlARRIVEBYTIME = GetDlgItem(hWndDlg, TRIPPLAN_ARRIVEBYTIME);
      hCtlFASTEST = GetDlgItem(hWndDlg, TRIPPLAN_FASTEST);
      hCtlFEWESTTRANSFERS = GetDlgItem(hWndDlg, TRIPPLAN_FEWESTTRANSFERS);
      hCtlACCESSIBLE = GetDlgItem(hWndDlg, TRIPPLAN_ACCESSIBLE);
      hCtlBIKERACK = GetDlgItem(hWndDlg, TRIPPLAN_BIKERACK);
      hCtlMAP = GetDlgItem(hWndDlg, TRIPPLAN_MAPINEFFECT);
      hCtlLANDMARKS = GetDlgItem(hWndDlg, TRIPPLAN_LANDMARKSINEFFECT);
//
//  And now...
//
      strcpy(szNow, NOW_TEXT);
//
//
//  Read the file and popluate the dropdowns
//
      maxLandmarks = 250;
      pLANDMARKS = (LANDMARKSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LANDMARKSDef) * maxLandmarks); 
      if(pLANDMARKS == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        bGotLandmarks = FALSE;
      }
      else
      {
        bGotLandmarks = TRUE;
        fp = fopen("Landmarks.txt", "r");
        if(fp == NULL)
        {
          MessageBox(NULL, "Failed to open Landmarks.txt", TMS, MB_OK);
          bGotLandmarks = FALSE;
        }
        else
        { 
          numLandmarks = 0;
          while(fgets(tempString, TEMPSTRING_LENGTH, fp) != NULL)
          {
            if(numLandmarks >= maxLandmarks)
            {
              maxLandmarks += 100;
              pLANDMARKS = (LANDMARKSDef *)HeapReAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY, pLANDMARKS, sizeof(LANDMARKSDef) * maxLandmarks); 
              if(pLANDMARKS == NULL)
              {
                AllocationError(__FILE__, __LINE__, TRUE);
                bGotLandmarks = FALSE;
                break;
              }
            }
            strcpy(szarString, strtok(tempString, "\t\n"));
            trim(szarString, 64);
            strcpy(pLANDMARKS[numLandmarks].szDescription, szarString);
            strcat(pLANDMARKS[numLandmarks].szDescription, " - ");
            strcpy(szarString, strtok(NULL, "\t\n"));
            trim(szarString, 64);
            strcat(pLANDMARKS[numLandmarks].szDescription, szarString);
            strcpy(szarString, strtok(NULL, "\t\n"));
            pLANDMARKS[numLandmarks].y = (float)atof(szarString);
            strcpy(szarString, strtok(NULL, "\t\n"));
            pLANDMARKS[numLandmarks].x = (float)atof(szarString);
            nJ = (int)SendMessage(hCtlLANDMARK1, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)pLANDMARKS[numLandmarks].szDescription);
            SendMessage(hCtlLANDMARK1, CB_SETITEMDATA, (WPARAM)nJ, (LPARAM)numLandmarks);
            nJ = (int)SendMessage(hCtlLANDMARK2, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)pLANDMARKS[numLandmarks].szDescription);
            SendMessage(hCtlLANDMARK2, CB_SETITEMDATA, (WPARAM)nJ, (LPARAM)numLandmarks);
            numLandmarks++;
          }
        }
      }
//
//  Still got landmarks?
//
//  No.
//
      if(!bGotLandmarks)
      {
        EnableWindow(hCtlORIGINLANDMARK, FALSE);
        EnableWindow(hCtlDESTINATIONLANDMARK, FALSE);
        EnableWindow(hCtlLANDMARK1, FALSE);
        EnableWindow(hCtlLANDMARK2, FALSE);
      }
//
//  Default to origin and destination address
//
      SendMessage(hCtlORIGINADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDESTINATIONADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Figure out the days for DAY3 through DAY7
//
      time(&ltime);
      strcpy(tempString, ctime(&ltime));
      for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
      {
        if(strncmp(tempString, szDays[nI], 3) == 0)
        {
          nK = nI;
          for(nJ = 0; nJ < 2; nJ++) // Skip ahead two days
          {
            serviceDays[nJ] = ROSTERPARMS.serviceDays[nK];
            if(serviceDays[nJ] == NO_RECORD)
            {
              hCtl = GetDlgItem(hWndDlg, dayIDs[nJ]);
              EnableWindow(hCtl, FALSE);
            }  
            nK++;
            if(nK >= ROSTER_MAX_DAYS)
            {
              nK = 0;
            }
          }
          for(nJ = 0; nJ < 5; nJ++)
          {
            hCtl = GetDlgItem(hWndDlg, dayIDs[nJ + 2]);
            SendMessage(hCtl, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szDays[nK]);
            serviceDays[nJ + 2] = ROSTERPARMS.serviceDays[nK];
            if(serviceDays[nJ + 2] == NO_RECORD)
            {
              EnableWindow(hCtl, FALSE);
            }
            nK++;
            if(nK >= ROSTER_MAX_DAYS)
            {
              nK = 0;
            }
          }
          break;
        }
      }
//
//  Default to the first departure day with an enabled radio button
//
      for(bFound = FALSE, nI = 0; nI < ROSTER_MAX_DAYS; nI++)
      {
        hCtlFIRSTDAY = GetDlgItem(hWndDlg, dayIDs[nI]);
        if(IsWindowEnabled(hCtlFIRSTDAY))
        {
          SendMessage(hCtlFIRSTDAY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        TMSError(hWndMain, MB_ICONSTOP, ERROR_225, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Default to "Leave at NOW"
//
      SendMessage(hCtlLEAVEAT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlLEAVEATTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szNow);
//
//  Default to "Fastest" itinerary
//
      SendMessage(hCtlFASTEST, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Map/Landmarks in effect
//
      if(m_bAllowTripPlanning)
      {
        strcpy(tempString, "Map in effect: ");
        strcat(tempString, szStreetFile);
      }
      else
      {
        strcpy(tempString, "MapInfo or MapInfo map not found - Address input disabled");
        EnableWindow(hCtlORIGIN, FALSE);
        EnableWindow(hCtlORIGINADDRESS, FALSE);
        EnableWindow(hCtlDESTINATION, FALSE);
        EnableWindow(hCtlDESTINATIONADDRESS, FALSE);
      }
      SendMessage(hCtlMAP, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      strcpy(tempString, "Landmark file in effect: ");
      strcat(tempString, (bGotLandmarks ? szLandmarkFile : "None"));
      SendMessage(hCtlLANDMARKS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Edward's stuff
//
      if(!CISbuild(TRUE, TRUE))
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      }
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      CISfree();
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
        case TRIPPLAN_LEAVEEARLY:
        case TRIPPLAN_LEAVELATE:
          if(SendMessage(hCtlLEAVEEARLY, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                SendMessage(hCtlLEAVELATE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLEAVEATTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
            SendMessage(hCtlARRIVEBYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          }
          break;
//
//  TRIPPLAN_ORIGINLANDMARK:
//
        case TRIPPLAN_ORIGINLANDMARK:
          if(SendMessage(hCtlLANDMARK1, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLANDMARK1, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          }
          break;
//
//  TRIPPLAN_DESTINATIONLANDMARK
//
        case TRIPPLAN_DESTINATIONLANDMARK:
          if(SendMessage(hCtlLANDMARK2, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLANDMARK2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          }
          break;
//
//  Address(es)
//
        case TRIPPLAN_ORIGIN:
          switch(wmEvent)
          {
            case EN_CHANGE:
              if(SendMessage(hCtlORIGINLANDMARK, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlORIGINLANDMARK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORIGINADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }

          }
          break;

        case TRIPPLAN_DESTINATION:
          switch(wmEvent)
          {
            case EN_CHANGE:
              if(SendMessage(hCtlDESTINATIONLANDMARK, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlDESTINATIONLANDMARK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlDESTINATIONADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }

          }
          break;
//
//  Landmark(s)
//
        case TRIPPLAN_LANDMARK1:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlORIGINADDRESS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlORIGINADDRESS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORIGINLANDMARK, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case TRIPPLAN_LANDMARK2:
          switch(wmEvent)
          {
            case CBN_SELENDOK:
              if(SendMessage(hCtlDESTINATIONADDRESS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlDESTINATIONADDRESS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlDESTINATIONLANDMARK, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;
//
//  TRIPPLAN_LEAVEAT
//
        case TRIPPLAN_LEAVEAT:
          if(SendMessage(hCtlLEAVEAT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlARRIVEBYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
            SendMessage(hCtlLEAVEATTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szNow);
            SendMessage(hCtlLEAVEATTIME, EM_SETSEL, (WPARAM)0, (LPARAM)(-1));
            SetFocus(hCtlLEAVEATTIME);
          }
          break;
//
//  TRIPPLAN_ARRIVEBY
//
        case TRIPPLAN_ARRIVEBY:
          if(SendMessage(hCtlARRIVEBY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlLEAVEATTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
            SendMessage(hCtlARRIVEBYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szNow);
            SendMessage(hCtlARRIVEBYTIME, EM_SETSEL, (WPARAM)0, (LPARAM)(-1));
            SetFocus(hCtlARRIVEBYTIME);
          }
          break;
//
//  IDRESET (labelled "Reset all fields")
//
        case IDRESET:
          SendMessage(hCtlORIGIN, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlORIGINFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlORIGINADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlORIGINLANDMARK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlLANDMARK1, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          SendMessage(hCtlDESTINATION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESTINATIONFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlDESTINATIONADDRESS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlDESTINATIONLANDMARK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlLANDMARK2, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          SendMessage(hCtlTODAY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlTOMORROW, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlDAY3, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlDAY4, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlDAY5, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlDAY6, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlDAY7, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlFIRSTDAY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlLEAVEAT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlLEAVEATTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szNow);
          SendMessage(hCtlLEAVEEARLY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlLEAVELATE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlARRIVEBY, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlARRIVEBYTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          SendMessage(hCtlFASTEST, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
          SendMessage(hCtlFEWESTTRANSFERS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlACCESSIBLE, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlBIKERACK, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SetFocus(hCtlORIGIN);
          break;

//
//  IDCANCEL (labelled "Finished")
//
        case IDCANCEL:
          hSaveCursor = SetCursor(hCursorWait);
          CISfree();
          if( cisPointArray != NULL )
          {
            free( cisPointArray );
            cisPointArray = NULL;
          }
          TMSHeapFree(pLANDMARKS);
          SetCursor(hSaveCursor);
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Starting_the_TMS_Trip_Planner);
          break;
//
//  IDOK (labelled "Plan trip...")
//
        case IDOK:
//
//  Verfiy the origin
//
          if(SendMessage(hCtlORIGINLANDMARK, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            nI = SendMessage(hCtlLANDMARK1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            nJ = SendMessage(hCtlLANDMARK1, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            if(nJ < 0)
            {
              break;
            }
            strcpy(szOrigin, pLANDMARKS[nJ].szDescription);
            origin.x = pLANDMARKS[nJ].x;
            origin.y = pLANDMARKS[nJ].y;
          }
          else
          {
            SendMessage(hCtlORIGIN, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            strcpy(szOrigin, tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_219, hCtlORIGIN);
              break;
            }
            else
            {
              bUseStreet = SendMessage(hCtlORIGINADDRESS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
              SendMessage(hCtlORIGINFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Searching...");
              if(!GetLongLat(tempString, &origin.x, &origin.y, bUseStreet, hWndDlg))
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_219, hCtlORIGIN);
                break;
              }
            }
          }
          sprintf(tempString, "%12.6f,%12.6f", origin.x, origin.y);
          SendMessage(hCtlORIGINFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Verify the destination
//
          if(SendMessage(hCtlDESTINATIONLANDMARK, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            nI = SendMessage(hCtlLANDMARK2, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            nJ = SendMessage(hCtlLANDMARK2, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            if(nJ < 0)
            {
              break;
            }
            strcpy(szDestination, pLANDMARKS[nJ].szDescription);
            destination.x = pLANDMARKS[nJ].x;
            destination.y = pLANDMARKS[nJ].y;
          }
          else
          {
            SendMessage(hCtlDESTINATION, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            strcpy(szDestination, tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_220, hCtlDESTINATION);
              break;
            }
            else
            {
              bUseStreet = SendMessage(hCtlDESTINATIONADDRESS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
              SendMessage(hCtlDESTINATIONFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Searching...");
              if(!GetLongLat(tempString, &destination.x, &destination.y, bUseStreet, hWndDlg))
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_220, hCtlDESTINATION);
                break;
              }
            }
          }
          sprintf(tempString, "%12.6f,%12.6f", destination.x, destination.y);
          SendMessage(hCtlDESTINATIONFOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Is this an arrive or leave request?
//
          isLeave = !SendMessage(hCtlARRIVEBY, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  Get the time
//
          bCalcTime = TRUE;
//
//  Leave at {time}
//
          if(SendMessage(hCtlLEAVEAT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlLEAVEATTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
//
//  Leave as early as possible
//
          else if(SendMessage(hCtlLEAVEEARLY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            bCalcTime = FALSE;
            tripTime = -1;
          }
//
//  Leave as late as possible
//
          else if(SendMessage(hCtlLEAVELATE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            bCalcTime = FALSE;
            tripTime = 48 * 60 * 60 + 1;
          }
//
//  Arrive by {time}
//
          else if(SendMessage(hCtlARRIVEBY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlARRIVEBYTIME, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          }
//
//  If need be, determine what {time} was
//
          if(bCalcTime)
          {
            if(strcmp(tempString, szNow) == 0)
            {
              time(&tripTime);
              memcpy( &tripTM, localtime(&tripTime), sizeof(tripTM) );
              tripTime = tripTM.tm_hour * 60 * 60 + tripTM.tm_min * 60 + tripTM.tm_sec;
            }
            else
            {
              tripTime = cTime(tempString);
              if(tripTime == NO_TIME)
              {
                TMSError(hWndDlg, MB_ICONSTOP, ERROR_223, isLeave ? hCtlLEAVEATTIME : hCtlARRIVEBYTIME);
                break;
              }
            }
          }
//
//  Determine the service record ID
//
          for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
          {
            hCtl = GetDlgItem(hWndDlg, dayIDs[nI]);
            if(SendMessage(hCtl, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              serviceID = serviceDays[nI];
              break;
            }
          }
//
//  Plan the trip
//
//  Free the cisPointArray from the previous plan.
//
          if( cisPointArray != NULL )
          {
            free( cisPointArray );
          }
          numCISpoints = 0;
          szPlan = CISplan(	szOrigin, origin.x, origin.y,
                szDestination, destination.x, destination.y,
                tripTime, isLeave, serviceID, &cisPointArray, &numCISpoints );
//
//  Display the results and stay in the dialog upon returning from the display
//
          results.szPlan = szPlan;
          results.cisPointArray = cisPointArray;
          results.numCISpoints = numCISpoints;
          DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRIPPLANNINGRESULTS),
                hWndDlg, (DLGPROC)TRIPPLANNINGRESULTSMsgProc, (LPARAM)&results);
          SetFocus(hCtlORIGIN);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}


BOOL CALLBACK TRIPPLANNINGRESULTSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlTEXT;
  static HANDLE hCtlDISCLAIMER;
  static HANDLE hCtlIDMAP;
  static RESULTSDef *pResults;
  UINT   nRC;
  FILE   *pf;
  long   prevRoute;
  char   routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char   routeName[ROUTES_NAME_LENGTH + 1];
  int    nI;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hCtlTEXT = GetDlgItem(hWndDlg, TRIPPLANNINGRESULTS_TEXT);
      hCtlDISCLAIMER = GetDlgItem(hWndDlg, TRIPPLANNINGRESULTS_DISCLAIMER);
      hCtlIDMAP = GetDlgItem(hWndDlg, IDMAP);
      pResults = (RESULTSDef *)lParam;
      SendMessage(hCtlTEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)(pResults->szPlan));
      LoadString(hInst, TEXT_151, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlDISCLAIMER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      if(pResults->numCISpoints == 0 || !m_bAllowTripPlanning)
      {
        EnableWindow(hCtlIDMAP, FALSE);
      }
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      switch(wParam)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Starting_the_TMS_Trip_Planner);
          break;

        case IDPRINT:
          if(!PrintTripPlan(hWndDlg, (pResults->szPlan)))
            MessageBox(hWndDlg, "Print Error", TMS, MB_OK);
          break;

        case IDMAP:
          pf = fopen("tripplan.raw", "w");
          if(pf == NULL)
          {
            LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
            sprintf(tempString, szFormatString, "tripplan.raw");
            MessageBeep(MB_ICONSTOP);
            MessageBox((HWND)NULL, tempString, TMS, MB_ICONSTOP);
            EnableWindow(hCtlIDMAP, FALSE);
          }
          else
          {
            prevRoute = NO_RECORD;
            for(nI = 0; nI < pResults->numCISpoints; nI++)
            {
              if(pResults->cisPointArray[nI].routeID == NO_RECORD)
              {
                strcpy(tempString, (nI == 0 ? "Start" : "End"));
              }
              else
              {
                if(pResults->cisPointArray[nI].routeID != prevRoute)
                {
                  ROUTESKey0.recordID = pResults->cisPointArray[nI].routeID;
                  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                  strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
                  trim(routeNumber, ROUTES_NUMBER_LENGTH);
                  strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
                  trim(routeName, ROUTES_NAME_LENGTH);
                  sprintf(tempString, "%s - %s", routeNumber, routeName);
                  prevRoute = pResults->cisPointArray[nI].routeID;
                }
              }
              if(m_bUseLatLong)
              {
                fprintf(pf, "\"%s\",\"%s\",%f,%f\n",
                      Tchar(pResults->cisPointArray[nI].t),
                      tempString,
                      pResults->cisPointArray[nI].latitude,
                      pResults->cisPointArray[nI].longitude);
              }
              else
              {
                fprintf(pf, "\"%s\",\"%s\",%f,%f\n",
                      Tchar(pResults->cisPointArray[nI].t),
                      tempString,
                      pResults->cisPointArray[nI].longitude,
                      pResults->cisPointArray[nI].latitude);
              }
            }
            fclose(pf);
            strcpy(tempString, szMappingProgram);
            strcat(tempString, " \"");
            strcat(tempString, szInstallDirectory);
            strcat(tempString, "\\TRIPPLAN.MBX\"");
            nRC = WinExec((LPSTR)tempString, SW_SHOW);
            if(nRC == 2 || nRC == 3)
            {
              TMSError(NULL, MB_ICONSTOP, ERROR_096, (HANDLE)NULL);
            }
          }
          break;

        case IDOK:
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//  PrintTripPlan()
//
BOOL PrintTripPlan(HWND hWnd, char *szResult)
{
  static DOCINFO  di = { sizeof(DOCINFO), "", NULL };
  static PRINTDLG pd;
  char   docName[256];
  char   *ptr = NULL;
  char   *ptr2;
  TEXTMETRIC tm;
  BOOL bSuccess;
  WORD iColCopy, iNoiColCopy;
  int  iCharsPerLine, iLinesPerPage, iTotalLines, yChar, numLines;
  DEVMODE *pDevMode;

//
//  Printer setup
//
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
  pd.nCopies             = 1;
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
//
//  Set landscape
//
  pDevMode = (DEVMODE *)pd.hDevMode;
  pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
  ResetDC(pd.hDC, pDevMode);

  iTotalLines = 60;

  bSuccess = GetTextMetrics(pd.hDC, &tm);
  if(bSuccess)
  {
    yChar = tm.tmHeight + tm.tmExternalLeading;
    iCharsPerLine = GetDeviceCaps(pd.hDC, HORZRES) / tm.tmAveCharWidth;
    iLinesPerPage = GetDeviceCaps(pd.hDC, VERTRES) / yChar;
  }
  else
  {
    yChar = 1;
    iCharsPerLine = 256;
    iLinesPerPage = 256;
  }

  EnableWindow(hWnd, FALSE);

  bSuccess   = TRUE;
  GetWindowText(hWnd, docName, sizeof(docName));
  di.lpszDocName = docName;

  if(StartDoc(pd.hDC, &di) > 0)
  {
    for(iColCopy = 0; iColCopy < ((WORD)pd.Flags & PD_COLLATE ? pd.nCopies : 1); iColCopy++)
    {
      for(iNoiColCopy = 0; iNoiColCopy < (pd.Flags & PD_COLLATE ? 1 : pd.nCopies); iNoiColCopy++)
      {
        if(StartPage(pd.hDC) < 0)
        {
          bSuccess = FALSE;
          break;
        }
        ptr = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(szResult) + 1); 
        if(ptr == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
        }
        else
        {
          strcpy(ptr, szResult);
          numLines = 0;
          for(ptr2 = strtok(ptr, "\r\n"); ptr2; ptr2 = strtok(NULL, "\r\n"))
          {
            TextOut(pd.hDC, 0, yChar * numLines, ptr2, strlen(ptr2));
            numLines++;
          }
          TMSHeapFree(ptr);
        }
        numLines += 2;
        LoadString(hInst, TEXT_151, szarString, sizeof(szarString));
        for(ptr2 = strtok(szarString, "\n"); ptr2; ptr2 = strtok(NULL, "\n"))
        {
          TextOut(pd.hDC, 0, yChar * numLines, ptr2, strlen(ptr2));
          numLines++;
        }
        if(EndPage(pd.hDC) < 0)
        {
          bSuccess = FALSE;
          break;
        }
      }
      if(!bSuccess)
      {
        break;
      }
    }
  }
  else
  {
    bSuccess = FALSE;
  }
  if(bSuccess)
  {
    EndDoc(pd.hDC);
  }

  EnableWindow(hWnd, TRUE);
  DeleteDC(pd.hDC);

  return(bSuccess);
}
