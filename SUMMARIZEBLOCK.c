//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK SUMMARIZEBLOCKMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  SUMMARIZEBLOCKParameterDef *pSBParms;
  HANDLE hCtl;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get the passed data
//
      pSBParms = (SUMMARIZEBLOCKParameterDef *)lParam;
      if(pSBParms == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
        break;
      }
//
//  Block number and "assigned to" garage
//
      hCtl = GetDlgItem(hWndDlg, SUMMARIZEBLOCK_BLOCKNUMBER);
      SendMessage(hCtl, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      if(pSBParms->assignedToNODESrecordID == NO_RECORD)
        strcpy(szarString, "");
      else
      {
        NODESKey0.recordID = pSBParms->assignedToNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(szarString, " (%s)", tempString);
      }
      sprintf(tempString, szFormatString, pSBParms->blockNumber, szarString);
      SendMessage(hCtl, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Number of trips
//
      sprintf(tempString, "%d", pSBParms->numTrips);
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_NUMTRIPS,
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Platform time
//
      sprintf(tempString, "%s", chhmm(pSBParms->totalPlat));
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_PLATFORMTIME,
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Layover time
//
      sprintf(tempString, "%s", chhmm(pSBParms->totalLay));
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_LAYOVERTIME,
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Interline Deadhead time
//
      sprintf(tempString, "%s", chhmm(pSBParms->totalILDhd));
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_DEADHEADTIME, 
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Garage Deadhead time
//
      sprintf(tempString, "%s", chhmm(pSBParms->totalGADhd));
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_GARDEADHEADTIME,
            WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Block length
//
      sprintf(tempString, "%s",
            chhmm(pSBParms->totalPlat + pSBParms->totalLay +
                  pSBParms->totalILDhd + pSBParms->totalGADhd));
      SendDlgItemMessage(hWndDlg, SUMMARIZEBLOCK_BLOCKLENGTH, 
           WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Done
//
      break; //  End of WM_INITDLG
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break; //  End of WM_CLOSE
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  IDPRINT
//
        case IDPRINT:
          PrintWindowToDC(hWndDlg, "Block Summary");
          break;
//
//  IDOK
//
        case IDOK:
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Standard_Block_Summary);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of SUMMARIZEBLOCKMsgProc
