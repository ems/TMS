//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static RUNTYPEDef localRT;
static HANDLE hCtlPIECE;
static HANDLE hCtlMINONTIME;
static HANDLE hCtlMAXONTIME;
static HANDLE hCtlMINOFFTIME;
static HANDLE hCtlMAXOFFTIME;
static HANDLE hCtlMINPAYTIME;
static HANDLE hCtlMAXPAYTIME;
static HANDLE hCtlMINPIECESIZE;
static HANDLE hCtlDESPIECESIZE;
static HANDLE hCtlMAXPIECESIZE;
static HANDLE hCtlMINBREAKTIME;
static HANDLE hCtlDESBREAKTIME;
static HANDLE hCtlMAXBREAKTIME;
static HANDLE hCtlTRAVELINCLUDEDINMIN;
static HANDLE hCtlTRAVELINCLUDEDINMAX;
static HANDLE hCtlPAYONLYAPPLIES;

void RTDisplayFields(int);
void RTSaveFields(int);

BOOL CALLBACK RTATTRIBMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static RUNTYPEDef *pRT;
  static HANDLE hCtlOF;
  static int    pieceNumber;
  static BOOL   bReInitInProgress;
  int    nI;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Get the data from lParam 
//
      pRT = (RUNTYPEDef *)lParam;
      if(pRT == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
        break;
      }
      memcpy(&localRT, (RUNTYPEDef *)lParam, sizeof(RUNTYPEDef));
//
//  Set up the handles to the controls
//
      hCtlPIECE = GetDlgItem(hWndDlg, RTATTRIB_PIECE);
      hCtlOF = GetDlgItem(hWndDlg, RTATTRIB_OF);
      hCtlMINONTIME = GetDlgItem(hWndDlg, RTATTRIB_MINONTIME);
      hCtlMAXONTIME = GetDlgItem(hWndDlg, RTATTRIB_MAXONTIME);
      hCtlMINOFFTIME = GetDlgItem(hWndDlg, RTATTRIB_MINOFFTIME);
      hCtlMAXOFFTIME = GetDlgItem(hWndDlg, RTATTRIB_MAXOFFTIME);
      hCtlMINPAYTIME = GetDlgItem(hWndDlg, RTATTRIB_MINPAYTIME);
      hCtlMAXPAYTIME = GetDlgItem(hWndDlg, RTATTRIB_MAXPAYTIME);
      hCtlMINPIECESIZE = GetDlgItem(hWndDlg, RTATTRIB_MINPIECESIZE);
      hCtlDESPIECESIZE = GetDlgItem(hWndDlg, RTATTRIB_DESPIECESIZE);
      hCtlMAXPIECESIZE = GetDlgItem(hWndDlg, RTATTRIB_MAXPIECESIZE);
      hCtlMINBREAKTIME = GetDlgItem(hWndDlg, RTATTRIB_MINBREAKTIME);
      hCtlDESBREAKTIME = GetDlgItem(hWndDlg, RTATTRIB_DESBREAKTIME);
      hCtlMAXBREAKTIME = GetDlgItem(hWndDlg, RTATTRIB_MAXBREAKTIME);
      hCtlTRAVELINCLUDEDINMIN = GetDlgItem(hWndDlg, RTATTRIB_TRAVELINCLUDEDINMIN);
      hCtlTRAVELINCLUDEDINMAX = GetDlgItem(hWndDlg, RTATTRIB_TRAVELINCLUDEDINMAX);
      hCtlPAYONLYAPPLIES = GetDlgItem(hWndDlg, RTATTRIB_PAYONLYAPPLIES);
//
//  Runtype (On the title bar)
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, localRT.localName);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Number of pieces.  Set the maximum for the control.
//
      itoa(localRT.numPieces, tempString, 10);
      SendMessage(hCtlOF, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      bReInitInProgress = FALSE;
      pieceNumber = NO_RECORD;
      SendMessage(hCtlPIECE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"1");
      break;
//
//  WM_CLOSE - Send an IDCANCEL
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
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
//  RTATTRIB_PIECE
//
        case RTATTRIB_PIECE:
          switch(wmEvent)
          {
            case EN_CHANGE:
              if(bReInitInProgress)
                bReInitInProgress = FALSE;
              else
              {
                if(pieceNumber == NO_RECORD)
                  nI = 1;
                else
                {
                  RTSaveFields(pieceNumber);
                  SendMessage(hCtlPIECE, WM_GETTEXT,
                        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
                  nI = atoi(tempString);
                  if(nI < 1 || nI > localRT.numPieces)
                  {
                    if(nI < 1)
                      nI = 1;
                    else
                      nI = localRT.numPieces;
                    bReInitInProgress = TRUE;
                  }
                }
                RTDisplayFields(nI);
                if(bReInitInProgress)
                {
                  itoa(nI, tempString, 10);
                  SendMessage(hCtlPIECE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
                }
                pieceNumber = nI;
              }
              break;
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Runtypes);
          break;
//
//  IDOK
//
        case IDOK:
          SendMessage(hCtlPIECE, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          RTSaveFields(pieceNumber);
          memcpy(pRT, &localRT, sizeof(RUNTYPEDef));
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
//  Field update routine
//
void RTDisplayFields(int pieceNumber)
{
  int nI = pieceNumber - 1;

  SendMessage(hCtlMINONTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)Tchar(localRT.PIECE[nI].minOnTime));
  SendMessage(hCtlMAXONTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)Tchar(localRT.PIECE[nI].maxOnTime));
  SendMessage(hCtlMINOFFTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)Tchar(localRT.PIECE[nI].minOffTime));
  SendMessage(hCtlMAXOFFTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)Tchar(localRT.PIECE[nI].maxOffTime));
  SendMessage(hCtlMINPAYTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].minPayTime));
  SendMessage(hCtlMAXPAYTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].maxPayTime));
  SendMessage(hCtlMINPIECESIZE, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].minPieceSize));
  SendMessage(hCtlDESPIECESIZE, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].desPieceSize));
  SendMessage(hCtlMAXPIECESIZE, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].maxPieceSize));
  SendMessage(hCtlMINBREAKTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].minBreakTime));
  SendMessage(hCtlDESBREAKTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].desBreakTime));
  SendMessage(hCtlMAXBREAKTIME, WM_SETTEXT, (WPARAM)0,
        (LONG)(LPSTR)chhmm(localRT.PIECE[nI].maxBreakTime));
  SendMessage(hCtlTRAVELINCLUDEDINMIN, BM_SETCHECK, 
        (WPARAM)(localRT.PIECE[nI].flags & PIECEFLAGS_TRAVELINCLUDEDINMIN), (LPARAM)0);
  SendMessage(hCtlTRAVELINCLUDEDINMAX, BM_SETCHECK, 
        (WPARAM)(localRT.PIECE[nI].flags & PIECEFLAGS_TRAVELINCLUDEDINMAX), (LPARAM)0);
  SendMessage(hCtlPAYONLYAPPLIES, BM_SETCHECK,
        (WPARAM)(localRT.PIECE[nI].flags & PIECEFLAGS_PAYONLYAPPLIES), (LPARAM)0);
}

void RTSaveFields(int pieceNumber)
{
  int nI = pieceNumber - 1;

  SendMessage(hCtlMINONTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].minOnTime = cTime(tempString);
  SendMessage(hCtlMAXONTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].maxOnTime = cTime(tempString);
  SendMessage(hCtlMINOFFTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].minOffTime = cTime(tempString);
  SendMessage(hCtlMAXOFFTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].maxOffTime = cTime(tempString);
  SendMessage(hCtlMINPAYTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].minPayTime = thhmm(tempString);
  SendMessage(hCtlMAXPAYTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].maxPayTime = thhmm(tempString);
  SendMessage(hCtlMINPIECESIZE, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].minPieceSize = thhmm(tempString);
  SendMessage(hCtlDESPIECESIZE, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].desPieceSize = thhmm(tempString);
  SendMessage(hCtlMAXPIECESIZE, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].maxPieceSize = thhmm(tempString);
  SendMessage(hCtlMINBREAKTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].minBreakTime = thhmm(tempString);
  SendMessage(hCtlDESBREAKTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].desBreakTime = thhmm(tempString);
  SendMessage(hCtlMAXBREAKTIME, WM_GETTEXT,
        (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
  localRT.PIECE[nI].maxBreakTime = thhmm(tempString);
  localRT.PIECE[nI].flags = 0;
  if(SendMessage(hCtlTRAVELINCLUDEDINMIN, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
    localRT.PIECE[nI].flags |= PIECEFLAGS_TRAVELINCLUDEDINMIN;
  if(SendMessage(hCtlTRAVELINCLUDEDINMAX, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
    localRT.PIECE[nI].flags |= PIECEFLAGS_TRAVELINCLUDEDINMAX;
  if(SendMessage(hCtlPAYONLYAPPLIES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
    localRT.PIECE[nI].flags |= PIECEFLAGS_PAYONLYAPPLIES;
}