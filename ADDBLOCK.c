//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

#define TABSET          1

BOOL CALLBACK ADDBLOCKMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  typedef struct STATSStruct
  {
    float dhdDist;
    long  dhdTime;
    long  layTime;
    float revDist;
    long  revTime;
  } STATSDef;
  typedef struct PARMSStruct
  {
    BOOL bStandard;
    BOOL bDropback;
    BOOL bInterlining;
    BOOL bServiceWrap;
    BOOL bDeadheading;
    BOOL bDeadheadAll;
    long maxDeadheadTime;
  } PARMSDef;
  static PDISPLAYINFO   pDI;
  static HANDLE hCtlALLROUTES;
  static HANDLE hCtlCC;
  static HANDLE hCtlCOMMENTCODE;
  static HANDLE hCtlORROUTE;
  static HANDLE hCtlFROMROUTE;
  static HANDLE hCtlFROMSERVICE;
  static HANDLE hCtlLISTBOXTITLE;
  static HANDLE hCtlTRIPS;
  static HANDLE hCtlMIN;
  static HANDLE hCtlMINVEHICLES;
  static HANDLE hCtlMAX;
  static HANDLE hCtlMAXVEHICLES;
  static HANDLE hCtlILYES;
  static HANDLE hCtlILNO;
  static HANDLE hCtlSWYES;
  static HANDLE hCtlSWNO;
  static HANDLE hCtlDHYES;
  static HANDLE hCtlDHNO;
  static HANDLE hCtlDEADHEADALL;
  static HANDLE hCtlDEADHEADTHOSE;
  static HANDLE hCtlDEADHEADMINS;
  static HANDLE hCtlTEXT_MINUTES;
  static HANDLE hCtlIDREMOVE;
  static HANDLE hCtlIDPREVIEW;
  static HANDLE hCtlIDOPTIMAL;
  static HANDLE hCtlIDMANUAL;
  static HANDLE hCtlSTANDARD;
  static HANDLE hCtlDROPBACK;
  static HANDLE hCtlDEADHEADSTOUSE;
  static int    tabPos[TABSET] = {32};
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  PARMSDef PARMS;
  BLOCKSDef  *pTRIPSChunk;
  LAYOVERDef *pLayover;
  float  distance;
  short  int wmId;
  short  int wmEvent;
  BOOL   bFound;
  BOOL   bEnable;
  BOOL   bAbort;
  char   routeName[ROUTES_NAME_LENGTH + 1];
  char   routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char   serviceName[SERVICES_NAME_LENGTH + 1];
  long   fromRouteRecordID;
  long   fromServiceRecordID;
  long   lastBlock;
  long   minutes;
  long   percent;
  long   tempLong;
  long   minVehicles;
  long   maxVehicles;
  long   commentRecordID;
  long   position;
  long   flags;
  long   dhd;
  WORD   DlgWidthUnits;
  int adjustedTabPos[TABSET];
  int nI;
  int nJ;
  int nK;
  int nRc;
  int rcode2;
  int numRoutes;
  int numTrips;
  int maxTrips;
  int numComments;
  int numSelected;
  int fromRoutePosition;
  int fromServicePosition;
  int blockingType;
  int layoverType;

  long *pTRIPSrecordIDs = NULL;
  int  *pSelected = NULL;

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
      m_numConnectionAlerts = 0;
//
//  Set up some control handles for later use
//
      hCtlALLROUTES = GetDlgItem(hWndDlg, ADDBLOCK_ALLROUTES);
      hCtlCC = GetDlgItem(hWndDlg, ADDBLOCK_CC);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, ADDBLOCK_COMMENTCODE);
      hCtlORROUTE = GetDlgItem(hWndDlg, ADDBLOCK_ORROUTE);
      hCtlFROMROUTE = GetDlgItem(hWndDlg, ADDBLOCK_FROMROUTE);
      hCtlFROMSERVICE = GetDlgItem(hWndDlg, ADDBLOCK_FROMSERVICE);
      hCtlLISTBOXTITLE = GetDlgItem(hWndDlg, ADDBLOCK_LISTBOXTITLE);
      hCtlTRIPS = GetDlgItem(hWndDlg, ADDBLOCK_TRIPS);
      hCtlMIN = GetDlgItem(hWndDlg, ADDBLOCK_MIN);
      hCtlMINVEHICLES = GetDlgItem(hWndDlg, ADDBLOCK_MINVEHICLES);
      hCtlMAX = GetDlgItem(hWndDlg, ADDBLOCK_MAX);
      hCtlMAXVEHICLES = GetDlgItem(hWndDlg, ADDBLOCK_MAXVEHICLES);
      hCtlILYES = GetDlgItem(hWndDlg, ADDBLOCK_ILYES);
      hCtlILNO = GetDlgItem(hWndDlg, ADDBLOCK_ILNO);
      hCtlSWYES = GetDlgItem(hWndDlg, ADDBLOCK_SWYES);
      hCtlSWNO = GetDlgItem(hWndDlg, ADDBLOCK_SWNO);
      hCtlDHYES = GetDlgItem(hWndDlg, ADDBLOCK_DHYES);
      hCtlDHNO = GetDlgItem(hWndDlg, ADDBLOCK_DHNO);
      hCtlDEADHEADALL = GetDlgItem(hWndDlg, ADDBLOCK_DEADHEADALL);
      hCtlDEADHEADTHOSE = GetDlgItem(hWndDlg, ADDBLOCK_DEADHEADTHOSE);
      hCtlDEADHEADMINS = GetDlgItem(hWndDlg, ADDBLOCK_DEADHEADMINS);
      hCtlTEXT_MINUTES = GetDlgItem(hWndDlg, ADDBLOCK_TEXT_MINUTES);
      hCtlIDREMOVE = GetDlgItem(hWndDlg, IDREMOVE);
      hCtlIDPREVIEW = GetDlgItem(hWndDlg, IDPREVIEW);
      hCtlIDOPTIMAL = GetDlgItem(hWndDlg, IDOPTIMAL);
      hCtlIDMANUAL = GetDlgItem(hWndDlg, IDMANUAL);
      hCtlSTANDARD = GetDlgItem(hWndDlg, ADDBLOCK_STANDARD);
      hCtlDROPBACK = GetDlgItem(hWndDlg, ADDBLOCK_DROPBACK);
      hCtlDEADHEADSTOUSE = GetDlgItem(hWndDlg, ADDBLOCK_DEADHEADSTOUSE);
//
//  Set up the tab stops for ADDBLOCK_LISTBOXTITLE and ADDBLOCK_TRIPS
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nJ = 0; nJ < TABSET; nJ++)
      {
        adjustedTabPos[nJ] = (DlgWidthUnits * tabPos[nJ] * 2);
      }
      SendMessage(hCtlLISTBOXTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      LoadString(hInst, TEXT_019, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlLISTBOXTITLE, LB_ADDSTRING, 0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlTRIPS, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  Default to all routes and this service
//
      SendMessage(hCtlALLROUTES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      if(SetUpServiceList(hWndDlg, ADDBLOCK_FROMSERVICE, pDI->fileInfo.serviceRecordID) == 0)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the route listbox
//
      numRoutes = 0;
      SendMessage(hCtlFROMROUTE, LB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(szarString, ROUTES_NAME_LENGTH);
        strcat(tempString, " - ");
        strcat(tempString, szarString);
        nI = (int)SendMessage(hCtlFROMROUTE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlFROMROUTE, LB_SETITEMDATA, (WPARAM)nI, ROUTES.recordID);
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
//  Comment codes combo box
//
     numComments = SetUpCommentList(hWndDlg, ADDBLOCK_COMMENTCODE);
      if(numComments == 0)
      {
        EnableWindow(hCtlCC, FALSE);
        EnableWindow(hCtlCOMMENTCODE, FALSE);
      }
//
//  Default to "Min" vechicle requirements, but with a blank field
//
      SendMessage(hCtlMIN, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlMINVEHICLES, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
//
//  Default to allowing deadheading, service wraps, and interlining
//
      SendMessage(hCtlILYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlSWYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDHYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDEADHEADALL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Clear out edit controls associated with a spin control
//
      SendMessage(hCtlDEADHEADMINS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
//
//  Default to this table's blocking technique
//
      if(pDI->fileInfo.fileNumber == TMS_STANDARDBLOCKS)
      {
        SendMessage(hCtlSTANDARD, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableWindow(hCtlDROPBACK, FALSE);
      }
      else
      {
        SendMessage(hCtlDROPBACK, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableWindow(hCtlSTANDARD, FALSE);
      }
//
//  Set up the m_pPICKEDTRIPS structure
//
      if(m_pPICKEDTRIPS == NULL)
      {
        m_pPICKEDTRIPS = (PICKEDTRIPSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PICKEDTRIPSDef) * m_maxPICKEDTRIPS); 
        if(m_pPICKEDTRIPS == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
          goto OptimalBlockCleanup;
        }
      }
//
//  Set up the "destination"
//
      flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE;
      DisplayPlacement(hWndDlg, ADDBLOCK_DESTINATION, pDI, flags);
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
//  Note: The outbound and inbound direction buttons get
//         disabled when the route is set to "All routes"
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  ADDBLOCK_ALLROUTES radio button
//
        case ADDBLOCK_ALLROUTES:
          SendMessage(hCtlFROMROUTE, LB_SETSEL, (WPARAM)(FALSE), (LPARAM)(-1));
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;
//
//  ADDBLOCK_CC radio button
//
        case ADDBLOCK_CC:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
          break;
//
//  ADDBLOCK_ORROUTE radio button
//
        case ADDBLOCK_ORROUTE:
          SendMessage(hCtlCOMMENTCODE, CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          nI = (int)SendMessage(hCtlFROMROUTE, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)
          {
            nJ = (int)SendMessage(hCtlFROMROUTE, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            if(nJ == 0)
              break;
            for(nI = 0; nI < nJ; nI++)
            {
              tempLong = SendMessage(hCtlFROMROUTE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              if(tempLong == pDI->fileInfo.routeRecordID)
              {
                SendMessage(hCtlFROMROUTE, LB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
                break;
              }
            }
          }
//
//  ADDBLOCK_FROMROUTE listbox
//
        case ADDBLOCK_FROMROUTE:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLROUTES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlORROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          break;
//
//  ADDBLOCK_LISTBOXTITLE
//
        case ADDBLOCK_LISTBOXTITLE:  // Title string
          SendMessage(hCtlLISTBOXTITLE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;
//
//  ADDBLOCK_TRIPS
//
        case ADDBLOCK_TRIPS: // List box
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
            case LBN_SELCANCEL:
              bEnable = SendMessage(hCtlTRIPS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) > 0;
              EnableWindow(hCtlIDREMOVE, bEnable);
              EnableWindow(hCtlIDPREVIEW, bEnable);
              break;
 
            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDPREVIEW, 0), (LPARAM)0);
              EnableWindow(hCtlIDPREVIEW, TRUE);
              break;
          }
          break;
//
//  ADDBLOCK_MIN
//
        case ADDBLOCK_MIN:
          EnableWindow(hCtlMINVEHICLES, TRUE);
          EnableWindow(hCtlMAXVEHICLES, FALSE);
          break;
//
//  ADDBLOCK_MAX
//
        case ADDBLOCK_MAX:
          EnableWindow(hCtlMINVEHICLES, FALSE);
          EnableWindow(hCtlMAXVEHICLES, TRUE);
          break;
//
//  ADDBLOCK_DHYES / ADDBLOCK_DHNO
//
        case ADDBLOCK_DHYES:
        case ADDBLOCK_DHNO:
          bEnable = SendMessage(hCtlDHYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlDEADHEADALL, bEnable);
          EnableWindow(hCtlDEADHEADSTOUSE, bEnable);
          EnableWindow(hCtlDEADHEADTHOSE, bEnable);
          EnableWindow(hCtlDEADHEADMINS, bEnable);
          EnableWindow(hCtlTEXT_MINUTES, bEnable);
          break;
//
//  IDPREVIEW
//
        case IDPREVIEW: // Button text: "Preview"
          nI = SendMessage(hCtlTRIPS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
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
          numSelected = (int)SendMessage(hCtlTRIPS, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
          if(numSelected >= m_maxPICKEDTRIPS)
          {
            m_maxPICKEDTRIPS += (numSelected + 1);
            m_pPICKEDTRIPS = (PICKEDTRIPSDef *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, m_pPICKEDTRIPS, sizeof(PICKEDTRIPSDef) * m_maxPICKEDTRIPS); 
            if(m_pPICKEDTRIPS == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
              break;
            }
          }
          for(nI = 0; nI < numSelected; nI++)
          {
            position = SendMessage(hCtlTRIPS, LB_GETITEMDATA, (WPARAM)pSelected[nI], (LPARAM)0);
            fromRoutePosition = LOWORD(position);
            fromServicePosition = HIWORD(position);
            m_pPICKEDTRIPS[nI].ROUTESrecordID = SendMessage(hCtlFROMROUTE,
                  LB_GETITEMDATA, (WPARAM)fromRoutePosition, (LPARAM)0);
            m_pPICKEDTRIPS[nI].SERVICESrecordID = SendMessage(hCtlFROMSERVICE,
                  CB_GETITEMDATA, (WPARAM)fromServicePosition, (LPARAM)0);
          }
          DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREVIEWT),
                hWndDlg, (DLGPROC)PREVIEWTMsgProc, (LPARAM)numSelected);
          TMSHeapFree(pSelected);
          break;
//
//  IDADD
//
        case IDADD: // Button text: "&Add Trip(s)"
//
//  Get the service
//
          fromServicePosition =
                (int)SendMessage(hCtlFROMSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(fromServicePosition == CB_ERR)
          {
            break;
          }
          fromServiceRecordID =
                SendMessage(hCtlFROMSERVICE, CB_GETITEMDATA, (WPARAM)fromServicePosition, (LPARAM)0);
          SERVICESKey0.recordID = fromServiceRecordID;
          btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
          strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
          trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Get the route(s)
//
//  All
//
          if(SendMessage(hCtlALLROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlFROMROUTE, LB_SETSEL, (WPARAM)(TRUE), (LPARAM)(-1));
          }
//
//  Those labelled...
//
          else if(SendMessage(hCtlCC, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
             nK = (int)SendMessage(hCtlCOMMENTCODE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
             commentRecordID = (nK == CB_ERR ? NO_RECORD : 
                   SendMessage(hCtlCOMMENTCODE, CB_GETITEMDATA, (WPARAM)nK, (LPARAM)0));
            if(commentRecordID == NO_RECORD)
            {
              break;
            }
            nJ = (int)SendMessage(hCtlFROMROUTE, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            for(nI = 0; nI < nJ; nI++)
            {
              ROUTESKey0.recordID = SendMessage(hCtlFROMROUTE, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
              if(ROUTES.COMMENTSrecordID == commentRecordID)
              {
                SendMessage(hCtlFROMROUTE, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nI);
              }
            }
          }  
//
//  Got the selections - continue
//
          nI = (int)SendMessage(hCtlFROMROUTE, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
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
          numSelected = (int)SendMessage(hCtlFROMROUTE, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
//
//  Add the results to the "to be blocked" list box
//  
          for(nI = 0; nI < numSelected; nI++)
          {
            fromRoutePosition = pSelected[nI];
            fromRouteRecordID = SendMessage(hCtlFROMROUTE, LB_GETITEMDATA, (WPARAM)fromRoutePosition, (LPARAM)0);
//
//  Before we go any further, let's see if we've got this one already
//
            position = MAKELONG(fromRoutePosition, fromServicePosition);
            nJ = SendMessage(hCtlTRIPS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            for(bFound = FALSE, nK = 0; nK < nJ; nK++)
            {
              tempLong = SendMessage(hCtlTRIPS, LB_GETITEMDATA, (WPARAM)nK, (LPARAM)0);
              if(tempLong == position)
              {
                bFound = TRUE;
                break;
              }
            }
            if(bFound)
            {
              continue;
            }
//
//  Not found
//
            EnableWindow(hCtlIDOPTIMAL, TRUE);
            EnableWindow(hCtlIDMANUAL, TRUE);
//
//  Display it
//
            ROUTESKey0.recordID = fromRouteRecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(routeNumber, ROUTES_NUMBER_LENGTH);
            strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
            trim(routeName, ROUTES_NAME_LENGTH);
            strcpy(tempString, routeNumber);
            strcat(tempString, " - ");
            strcat(tempString, routeName);
            tempString[tabPos[0]] = '\0';
            strcat(tempString, "\t");
            strcat(tempString, serviceName);
            nK = (int)SendMessage(hCtlTRIPS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            SendMessage(hCtlTRIPS, LB_SETITEMDATA, (WPARAM)nK, (LPARAM)position);
          }
          SendMessage(hCtlFROMROUTE, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          TMSHeapFree(pSelected);
          break;
//
//  IDREMOVE
//
        case IDREMOVE: // Button text: "Remove"
          nI = (int)SendMessage(hCtlTRIPS, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
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
          numSelected = (int)SendMessage(hCtlTRIPS, LB_GETSELITEMS, (WPARAM)nI, (LPARAM)pSelected);
//
//  Process the list from the back to the front
//
          for(nI = numSelected - 1; nI >= 0; nI--)
          {
            SendMessage(hCtlTRIPS, LB_DELETESTRING, (WPARAM)pSelected[nI], (LPARAM)0);
          }
          if(SendMessage(hCtlTRIPS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0) == 0)
          {
            EnableWindow(hCtlIDOPTIMAL, FALSE);
            EnableWindow(hCtlIDMANUAL, FALSE);
          }
          EnableWindow(hCtlIDREMOVE, FALSE);
          EnableWindow(hCtlIDPREVIEW, FALSE);
          TMSHeapFree(pSelected);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Standard_Blocks_Table);
          break;
//
//  IDMANUAL and IDOPTIMAL
//
        case IDMANUAL:
        case IDOPTIMAL:
          m_bNetworkMode = FALSE;
//
//  Get the parameters
//
          PARMS.bStandard = SendMessage(hCtlSTANDARD, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          PARMS.bDropback = SendMessage(hCtlDROPBACK, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  Standard and Dropback are mutually exclusive on a manual block
//
          if(wmId == IDMANUAL)
          {
            if(PARMS.bStandard && PARMS.bDropback)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_134, hCtlSTANDARD);
              break;
            }
          }
//
//  At least one of Standard or Dropback must be chosen prior to pressing Optimal
//
          else
          {
            if(!PARMS.bStandard && !PARMS.bDropback)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_135, hCtlSTANDARD);
              break;
            }
          }
//
//  Get the rest of the parameters
//
          PARMS.bInterlining = SendMessage(hCtlILYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          PARMS.bServiceWrap = SendMessage(hCtlSWYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          PARMS.bDeadheading = SendMessage(hCtlDHYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(!PARMS.bDeadheading)
          {
            PARMS.bDeadheadAll = FALSE;
            PARMS.maxDeadheadTime = NO_TIME;
          }
          else
          {
            SendMessage(hCtlDEADHEADMINS, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              PARMS.bDeadheadAll = TRUE;
              PARMS.maxDeadheadTime = NO_TIME;
            }
            else
            {
              PARMS.bDeadheadAll = FALSE;
              PARMS.maxDeadheadTime = atol(tempString) * 60;
            }
          }
//
//  Build the list of selected trips
//
          m_numPICKEDTRIPS = SendMessage(hCtlTRIPS, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
          if(m_numPICKEDTRIPS >= m_maxPICKEDTRIPS)
          {
            m_maxPICKEDTRIPS += (m_numPICKEDTRIPS + 1);
            m_pPICKEDTRIPS = (PICKEDTRIPSDef *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, m_pPICKEDTRIPS, sizeof(PICKEDTRIPSDef) * m_maxPICKEDTRIPS); 
            if(m_pPICKEDTRIPS == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
              break;
            }
          }
          for(nI = 0; nI < m_numPICKEDTRIPS; nI++)
          {
            position = SendMessage(hCtlTRIPS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            fromRoutePosition = LOWORD(position);
            fromServicePosition = HIWORD(position);
            m_pPICKEDTRIPS[nI].ROUTESrecordID = SendMessage(hCtlFROMROUTE,
                  LB_GETITEMDATA, (WPARAM)fromRoutePosition, (LPARAM)0);
            m_pPICKEDTRIPS[nI].SERVICESrecordID = SendMessage(hCtlFROMSERVICE,
                  CB_GETITEMDATA, (WPARAM)fromServicePosition, (LPARAM)0);
          }
//
//  Dropback blocking isn't permitted unless it's been "standard" blocked already
//
          if(PARMS.bDropback)
          {
            bAbort = FALSE;
            for(nI = 0; nI < m_numPICKEDTRIPS; nI++)
            {
              if(bAbort)
                break;
              for(nJ = 0; nJ < 2; nJ++)
              {
                if(bAbort)
                  break;
                ROUTESKey0.recordID = m_pPICKEDTRIPS[nI].ROUTESrecordID;
                btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
                  continue;
                TRIPSKey1.ROUTESrecordID = m_pPICKEDTRIPS[nI].ROUTESrecordID;
                TRIPSKey1.SERVICESrecordID = m_pPICKEDTRIPS[nI].SERVICESrecordID;
                TRIPSKey1.directionIndex = nJ;
                TRIPSKey1.tripSequence = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
                while(rcode2 == 0 &&
                      TRIPS.ROUTESrecordID == m_pPICKEDTRIPS[nI].ROUTESrecordID &&
                      TRIPS.SERVICESrecordID == m_pPICKEDTRIPS[nI].SERVICESrecordID &&
                      TRIPS.directionIndex == nJ)
                {
                  if(TRIPS.standard.blockNumber == 0)
                  {
                    LoadString(hInst, TEXT_245, szFormatString, sizeof(szFormatString));
                    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
                    trim(routeNumber, ROUTES_NUMBER_LENGTH);
                    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
                    trim(routeName, ROUTES_NAME_LENGTH);
                    SERVICESKey0.recordID = m_pPICKEDTRIPS[nI].SERVICESrecordID;
                    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
                    trim(serviceName, SERVICES_NAME_LENGTH);
                    sprintf(tempString, szFormatString, serviceName, routeNumber, routeName);
                    MessageBeep(MB_ICONSTOP);
                    MessageBox(hWndDlg, tempString, TMS, MB_OK);
                    bAbort = TRUE;
                    break;
                  }
                  rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
                }
              }
            }
            if(bAbort)
              break;
          }
//  
//  IDMANUAL - Go into MANBLOCK
//
          if(wmId == IDMANUAL)
          {
            pDI->otherData = (long)(&PARMS);
            nRc = BlockDialog(pDI);
            if(nRc)
            {
              EndDialog(hWndDlg, TRUE);
            }
            break;
          }
//
//  IDOPTIMAL
//
          for(blockingType = 0; blockingType < 2; blockingType++)
          {
            if(blockingType == 0)
            {
              if(!PARMS.bStandard)
              {
                continue;
              }
              StatusBarStart(hWndDlg, "Optimal Standard Blocking Status");
              pTRIPSChunk = &TRIPS.standard;
              pLayover = &StandardLayover;
              layoverType = TMS_STANDARDBLOCKS;
            }
            else
            {
              if(!PARMS.bDropback)
              {
                continue;
              }
              StatusBarStart(hWndDlg, "Optimal Dropback Blocking Status");
              pTRIPSChunk = &TRIPS.dropback;
              pLayover = &DropbackLayover;
              layoverType = TMS_DROPBACKBLOCKS;
            }
            StatusBar(-1L, -1L);
            StatusBarText("Initializing...");
//
//  Initialize the interlining option
//
            BLSetOptions(SendMessage(hCtlILYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0));
//
//  Loop through numPICKEDTRIPS to determine what we're dealing with
//
            StatusBarText("Examining selected trips...");
            m_numConnectionAlerts = 0;
            maxTrips = 512;
            pTRIPSrecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxTrips); 
            if(pTRIPSrecordIDs == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              goto OptimalBlockCleanup;
            }
            for(numTrips = 0, nI = 0; nI < m_numPICKEDTRIPS; nI++)
            {
              StatusBar((long)nI, (long)m_numPICKEDTRIPS);
              if(StatusBarAbort())
              {
                goto OptimalBlockCleanup;
              }
              m_bEstablishRUNTIMES = TRUE;
              TRIPSKey1.ROUTESrecordID = m_pPICKEDTRIPS[nI].ROUTESrecordID;
              TRIPSKey1.SERVICESrecordID = m_pPICKEDTRIPS[nI].SERVICESrecordID;
              TRIPSKey1.directionIndex = NO_RECORD;
              TRIPSKey1.tripSequence = NO_RECORD;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              while(rcode2 == 0 &&
                    (m_pPICKEDTRIPS[nI].ROUTESrecordID == NO_RECORD ||
                          TRIPS.ROUTESrecordID == m_pPICKEDTRIPS[nI].ROUTESrecordID) &&
                    (m_pPICKEDTRIPS[nI].SERVICESrecordID == NO_RECORD ||
                          TRIPS.SERVICESrecordID == m_pPICKEDTRIPS[nI].SERVICESrecordID))
              {
                if(pTRIPSChunk->blockNumber == 0)
                {
                  if(numTrips >= maxTrips)
                  {
                    maxTrips += 512;
                    pTRIPSrecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY, pTRIPSrecordIDs, sizeof(long) * maxTrips); 
                    if(pTRIPSrecordIDs == NULL)
                    {
                      AllocationError(__FILE__, __LINE__, TRUE);
                      goto OptimalBlockCleanup;
                    }
                  }
                  pTRIPSrecordIDs[numTrips++] = TRIPS.recordID;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              }
            }
            if((m_numTripsForBlocker = numTrips) == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_102, (HANDLE)NULL);
              goto OptimalBlockCleanup;
            }
//
//  If m_numConnectionAlerts is non-zero, blow him off
//
            if(m_numConnectionAlerts != 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_173, (HANDLE)NULL);
              goto OptimalBlockCleanup;
            }
//
//  Make sure that we're not going in with bad values for min & max layover
//
            if(pLayover->minimum.type == NO_RECORD ||
                  pLayover->maximum.type == NO_RECORD ||
                  (pLayover->maximum.type == 0 && pLayover->maximum.minutes == 0))
            {
              MessageBeep(MB_ICONSTOP);
              LoadString(hInst, ERROR_030, szarString, sizeof(szarString));
              MessageBox(hWndDlg, szarString, TMS, MB_ICONSTOP);
              nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LAYOVER),
                    hWndDlg, (DLGPROC)LAYOVERMsgProc, (LPARAM)layoverType);
              if(!nRc)
                goto OptimalBlockCleanup;
            }
//
//  Generate trip data into TRIPINFO
//
            StatusBarText("Building trip data...");
            StatusBar(0L, 0L);
            BLInitNetwork();
            for(nI = 0; nI < numTrips; nI++)
            {
              StatusBar((long)nI, (long)numTrips);
              if(StatusBarAbort())
                goto OptimalBlockCleanup;
              TRIPSKey0.recordID = pTRIPSrecordIDs[nI];
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              nJ = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  If we've got a one-node trip, fool the blocker
//
              if(GTResults.firstNODESrecordID == GTResults.lastNODESrecordID &&
                    GTResults.firstNodeTime == GTResults.lastNodeTime)
              {
                GTResults.lastNodeTime++;
              }
              if(nJ != 0)
              {
                TRIPINFODef TRIPINFOrec;
                TRIPINFOrec.TRIPSrecordID = TRIPS.recordID;
                TRIPINFOrec.BUSTYPESrecordID = TRIPS.BUSTYPESrecordID;
                TRIPINFOrec.ROUTESrecordID = TRIPS.ROUTESrecordID;
//
//  If we're doing a dropback, and the standard block pulls out or in,
//  then replace the start/end node/time with the pullout/in gar/time.
//
//  Check pullouts
//
                if(PARMS.bStandard || TRIPS.standard.POGNODESrecordID == NO_RECORD)
                {
                  TRIPINFOrec.startNODESrecordID = GTResults.firstNODESrecordID;
                  TRIPINFOrec.startTime = GTResults.firstNodeTime;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
                  GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                  GCTData.timeOfDay = GTResults.firstNodeTime;
                  dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  if(dhd == NO_TIME)
                    dhd = 0;
                  TRIPINFOrec.startNODESrecordID = TRIPS.standard.POGNODESrecordID;
                  TRIPINFOrec.startTime = GTResults.firstNodeTime - dhd;
                }
//
//  Check pullins
//
                if(PARMS.bStandard || TRIPS.standard.PIGNODESrecordID == NO_RECORD)
                {
                  TRIPINFOrec.endNODESrecordID = GTResults.lastNODESrecordID;
                  TRIPINFOrec.endTime = GTResults.lastNodeTime;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                  GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                  GCTData.timeOfDay = GTResults.lastNodeTime;
                  dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  if(dhd == NO_TIME)
                    dhd = 0;
                  TRIPINFOrec.endNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                  TRIPINFOrec.endTime = GTResults.lastNodeTime + dhd;
                }
//
//  Figure out the minimum layover
//
                if(pTRIPSChunk->layoverMin != NO_TIME)
                  TRIPINFOrec.layoverMin = pTRIPSChunk->layoverMin;
                else
                {
                  minutes = pLayover->minimum.minutes  * 60;
                  percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * pLayover->minimum.percent / 100;
                  if(pLayover->minimum.type == 0)
                    TRIPINFOrec.layoverMin = minutes;
                  else if(pLayover->minimum.type == 1)
                    TRIPINFOrec.layoverMin = percent;
                  else if(pLayover->minimum.type == 2)
                    TRIPINFOrec.layoverMin =
                          pLayover->minimum.lower ? min(minutes, percent) : max(minutes, percent);
                }
//
//  Figure out the maximum layover
//
                if(pTRIPSChunk->layoverMax != NO_TIME)
                  TRIPINFOrec.layoverMax = pTRIPSChunk->layoverMax;
                else
                {
                  minutes = pLayover->maximum.minutes * 60;
                  percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * pLayover->maximum.percent / 100;
                  if(pLayover->maximum.type == 0)
                    TRIPINFOrec.layoverMax = minutes;
                  else if(pLayover->maximum.type == 1)
                    TRIPINFOrec.layoverMax = percent;
                  else if(pLayover->maximum.type == 2)
                    TRIPINFOrec.layoverMax =
                          pLayover->maximum.lower ? min(minutes, percent) : max(minutes, percent);
                }
//
//  And generate the trip arc
//
                BLGenerateTripArc( &TRIPINFOrec );
              }
            }
//
//  If m_numConnectionAlerts is non-zero, blow him off
//
            if(m_numConnectionAlerts != 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_173, (HANDLE)NULL);
              goto OptimalBlockCleanup;
            }
//
//  Generate the wait arcs between the trips
//
            StatusBarText("Generating wait arcs...");
            StatusBar(-1L, -1L);
            BLGenerateWaitArcs(FALSE);
//
//  Generate deadhead and equivalence data into DEADHEADINFO
//
            rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            numDeadheadsForBlocker = 0;
            while(rcode2 == 0)
            {
              if(StatusBarAbort())
                goto OptimalBlockCleanup;
              if(PARMS.bDeadheading)
              {
                if((CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME) &&
                      (PARMS.bDeadheadAll || CONNECTIONS.connectionTime <= PARMS.maxDeadheadTime))
                {
                  DEADHEADINFODef DEADHEADINFOrec;
                  DEADHEADINFOrec.fromTimeOfDay = CONNECTIONS.fromTimeOfDay;
                  DEADHEADINFOrec.toTimeOfDay = CONNECTIONS.toTimeOfDay;
                  DEADHEADINFOrec.fromNODESrecordID = CONNECTIONS.fromNODESrecordID;
                  DEADHEADINFOrec.toNODESrecordID = CONNECTIONS.toNODESrecordID;
                  DEADHEADINFOrec.flags = CONNECTIONS.flags;
                  DEADHEADINFOrec.deadheadTime = CONNECTIONS.connectionTime;
                  BLGenerateDeadheadArcs( &DEADHEADINFOrec );
                  numDeadheadsForBlocker++;
                }
              }
              if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
              {
                DEADHEADINFODef DEADHEADINFOrec;
                DEADHEADINFOrec.fromTimeOfDay = CONNECTIONS.fromTimeOfDay;
                DEADHEADINFOrec.toTimeOfDay = CONNECTIONS.toTimeOfDay;
                DEADHEADINFOrec.fromNODESrecordID = CONNECTIONS.fromNODESrecordID;
                DEADHEADINFOrec.toNODESrecordID = CONNECTIONS.toNODESrecordID;
                DEADHEADINFOrec.flags = CONNECTIONS.flags;
                DEADHEADINFOrec.deadheadTime = 0;
                BLGenerateDeadheadArcs( &DEADHEADINFOrec );
                numDeadheadsForBlocker++;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            }
            if(StatusBarAbort())
              goto OptimalBlockCleanup;
//
//  Get the minimum and maximum number of vehicles parameters.
//  This only applies to Optimal, so it's done here rather than above.
//
            if(SendMessage(hCtlMIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              maxVehicles = NO_TIME;
              SendMessage(hCtlMINVEHICLES, WM_GETTEXT,
                    (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              if(strcmp(tempString, "") == 0)
                minVehicles = NO_TIME;
              else
                minVehicles = atol(tempString);
            }
            else
            {
              minVehicles = NO_TIME;
              SendMessage(hCtlMAXVEHICLES, WM_GETTEXT,
                    (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
              if(strcmp(tempString, "") == 0)
                maxVehicles = NO_TIME;
              else
                maxVehicles = atol(tempString);
            }
//
//  Call the blocker
//
            m_pBLOCKSDATA = (BLOCKSDATADef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BLOCKSDATADef) * m_numTripsForBlocker); 
            if(m_pBLOCKSDATA == NULL)
            {
              AllocationError(__FILE__, __LINE__, FALSE);
              goto OptimalBlockCleanup;
            }
            StatusBarText("Optimal blocking in progress...");
            sblock(minVehicles, maxVehicles);
            if(StatusBarAbort())
            {
              goto OptimalBlockCleanup;
            }
//
//  Update the file with the results
//
            lastBlock = GetNewBlockNumber(pDI);
            StatusBarText("Blocking complete - writing results...");
            StatusBarEnableCancel(FALSE);
            for(nJ = 0; nJ < numTrips; nJ++)
            {
              StatusBar((long)nJ, (long)numTrips);
              TRIPSKey0.recordID = m_pBLOCKSDATA[nJ].TRIPSrecordID;
              if((rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0)) != 0)
              {
                MessageBeep(MB_ICONSTOP);
                LoadString(hInst, ERROR_015, szarString, sizeof(szarString));
                SetFocus(hCtlFROMROUTE);
                MessageBox(hWndDlg, szarString, TMS, MB_ICONSTOP);
                goto OptimalBlockCleanup;
              }
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
              pTRIPSChunk->blockNumber = m_pBLOCKSDATA[nJ].blockNumber + lastBlock;
              pTRIPSChunk->RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
              pTRIPSChunk->SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
              pTRIPSChunk->blockSequence = GTResults.firstNodeTime;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }

            OptimalBlockCleanup:
              BLCleanup();
              TMSHeapFree(m_pBLOCKSDATA);
              TMSHeapFree(pTRIPSrecordIDs);
              m_numTripsForBlocker = 0;
              StatusBarEnd();
          }  // blockingType

          EndDialog(hWndDlg, TRUE);
          break;
       }
     break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }

  return TRUE;
} //  End of ADDBLOCKMsgProc
