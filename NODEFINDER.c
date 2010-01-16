//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define TABSET    3
#define NUMFLAGS 15

BOOL CALLBACK NODEFINDERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlLISTBOXTITLE;
  static HANDLE hCtlLISTBOX;
  static HANDLE hCtlABBR;
  static HANDLE hCtlLONGNAME;
  static HANDLE hCtlLABELS;
  static HANDLE hCtlJURISDICTION;
  static HANDLE hCtlCOMMENTCODE;
  static HANDLE hCtlINTERSECTION;
  static HANDLE hCtlLONGITUDE;
  static HANDLE hCtlLATITUDE;
  static HANDLE hCtlLATITUDE_TEXT;
  static HANDLE hCtlLONGITUDE_TEXT;
  static int    tabPos[TABSET] = {20, 33, 43};
  WORD DlgWidthUnits;
  int  adjustedTabPos[TABSET];
  int  nI;
  int  rcode2;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the nodes
//
      if(SetUpNodeList(hWndDlg, NODEFINDER_ABBR, FALSE) == 0)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlLISTBOXTITLE = GetDlgItem(hWndDlg, NODEFINDER_LISTBOXTITLE);
      hCtlLISTBOX = GetDlgItem(hWndDlg, NODEFINDER_LISTBOX);
      hCtlABBR = GetDlgItem(hWndDlg, NODEFINDER_ABBR);
      hCtlLONGNAME = GetDlgItem(hWndDlg, NODEFINDER_LONGNAME);
      hCtlJURISDICTION = GetDlgItem(hWndDlg, NODEFINDER_JURISDICTION);
      hCtlLABELS = GetDlgItem(hWndDlg, NODEFINDER_LABELS);
      hCtlCOMMENTCODE = GetDlgItem(hWndDlg, NODEFINDER_COMMENTCODE);
      hCtlINTERSECTION = GetDlgItem(hWndDlg, NODEFINDER_INTERSECTION);
      hCtlLONGITUDE = GetDlgItem(hWndDlg, NODEFINDER_LONGITUDE);
      hCtlLATITUDE = GetDlgItem(hWndDlg, NODEFINDER_LATITUDE);
      hCtlLONGITUDE_TEXT = GetDlgItem(hWndDlg, NODEFINDER_LONGITUDE_TEXT);
      hCtlLATITUDE_TEXT = GetDlgItem(hWndDlg, NODEFINDER_LATITUDE_TEXT);
//
//  Set up the listbox tab stops
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nI = 0; nI < TABSET; nI++)
      {
        adjustedTabPos[nI] = (DlgWidthUnits * tabPos[nI] * 2);
      }
      SendMessage(hCtlLISTBOXTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      LoadString(hInst, TEXT_095, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlLISTBOXTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlLISTBOX, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      SendMessage(hCtlABBR, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
//
//  Set up easting/northing if he's not in lat/long
//
      if(!m_bUseLatLong)
      {
        SendMessage(hCtlLONGITUDE_TEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Easting:");
        SendMessage(hCtlLATITUDE_TEXT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"Northing:");
      }
//
//  Show the first listed node's characteristics
//
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(NODEFINDER_ABBR, CBN_SELCHANGE), (LPARAM)0);
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
//  NODEFINDER_ABBR
//
        case NODEFINDER_ABBR:
          switch(wmEvent)
          {
            case CBN_SELCHANGE:
              if((nI = (int)SendMessage(hCtlABBR, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) == CB_ERR)
              {
                SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
                break;
              }
//
//  Get the node record
//
              NODESKey0.recordID = SendMessage(hCtlABBR, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Long Name
//
              strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
              trim(tempString, NODES_LONGNAME_LENGTH);
              SendMessage(hCtlLONGNAME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Jurisdiction
//
              if(NODES.JURISDICTIONSrecordID == NO_RECORD)
                strcpy(tempString, "");
              else
              {
                JURISDICTIONSKey0.recordID = NODES.JURISDICTIONSrecordID;
                btrieve(B_GETEQUAL, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
                strncpy(tempString, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
                trim(tempString, JURISDICTIONS_NAME_LENGTH);
              }
              SendMessage(hCtlJURISDICTION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Labels
//
              strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
              trim(tempString, NODES_RELIEFLABELS_LENGTH);
              SendMessage(hCtlLABELS, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Comment Code
//
              if(NODES.COMMENTSrecordID == NO_RECORD || NODES.COMMENTSrecordID == 0)
                strcpy(tempString, "");
              else
              {
                COMMENTSKey0.recordID = NODES.COMMENTSrecordID;
                btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
                strncpy(tempString, COMMENTS.code, sizeof(COMMENTS.code));
                trim(tempString, sizeof(COMMENTS.code));
              }
              SendMessage(hCtlCOMMENTCODE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Intersection
//
              strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
              trim(tempString, NODES_INTERSECTION_LENGTH);
              SendMessage(hCtlINTERSECTION, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Longitude
//
              sprintf(tempString, "%12.6f", NODES.longitude);
              SendMessage(hCtlLONGITUDE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Latitude
//
              sprintf(tempString, "%12.6f", NODES.latitude);
              SendMessage(hCtlLATITUDE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Got all the node details, now find the node in the system
//
              SendMessage(hCtlLISTBOX, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              PATTERNSKey1.NODESrecordID = NODES.recordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              while(rcode2 == 0 && PATTERNS.NODESrecordID == NODES.recordID)
              {
                ROUTESKey0.recordID = PATTERNS.ROUTESrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                if(rcode2 == 0) 
                {
                  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                  trim(tempString, ROUTES_NUMBER_LENGTH);
                  strcat(tempString, " - ");
                  strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
                  trim(szarString, ROUTES_NAME_LENGTH);
                  strcat(tempString, szarString);
                  tempString[tabPos[0]] = '\0';
                  strcat(tempString, "\t");
                  SERVICESKey0.recordID = PATTERNS.SERVICESrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                  if(rcode2 == 0)
                  {
                    strncpy(szarString, SERVICES.name, sizeof(SERVICES.name));
                    trim(szarString, sizeof(SERVICES.name));
                    strcat(tempString, szarString);
                    strcat(tempString, "\t");
                    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[PATTERNS.directionIndex];
                    rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
                    if(rcode2 == 0)
                    {
                      strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
                      trim(szarString, DIRECTIONS_ABBRNAME_LENGTH);
                      strcat(tempString, szarString);
                      strcat(tempString, "\t");
                      PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
                      rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                      if(rcode2 == 0)
                      {
                        strncpy(szarString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
                        trim(szarString, sizeof(PATTERNNAMES.name));
                        strcat(tempString, szarString);
                        SendMessage(hCtlLISTBOX, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
                      }
                    }
                  }
                }
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
              }
              break;
          }
          break;
//
//  IDOK
//
        case IDOK:
        case IDCANCEL:
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Node_Finder);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
