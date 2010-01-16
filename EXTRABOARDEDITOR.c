//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK EXTRABOARDEDITORMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static EXTRABOARDEDITORPARAMDef *pParam;
  static HANDLE hCtlRUNPIECE;
  static HANDLE hCtlAVAILABLETIME;
  static HANDLE hCtlPIECESTART;
  static HANDLE hCtlPIECEEND;
  static HANDLE hCtlPIECESIZE;
  static HANDLE hCtlSTARTSTART;
  static HANDLE hCtlSTARTEND;
  static HANDLE hCtlENDSTART;
  static HANDLE hCtlENDEND;
  static long   pieceSize;

  long timeToUse;
  long startStart;
  long startEnd;
  long endStart;
  long endEnd;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pParam = (EXTRABOARDEDITORPARAMDef *)lParam;
      if(pParam == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handle to the control
//
      hCtlRUNPIECE = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_RUNPIECE);
      hCtlAVAILABLETIME = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_AVAILABLETIME);
      hCtlPIECESTART = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_PIECESTART);
      hCtlPIECEEND = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_PIECEEND);
      hCtlPIECESIZE = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_PIECESIZE);
      hCtlSTARTSTART = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_STARTSTART);
      hCtlSTARTEND = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_STARTEND);
      hCtlENDSTART = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_ENDSTART);
      hCtlENDEND = GetDlgItem(hWndDlg, EXTRABOARDEDITOR_ENDEND);
//
//  Set the title
//
      SendMessage(hCtlRUNPIECE, WM_GETTEXT,
            (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, pParam->runNumber, pParam->pieceNumber);
      SendMessage(hCtlRUNPIECE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Fill in the controls.  If there's nothing in the extraboard fields
//  to start with, assume we're going to grab all we can for this piece
//  and put it on the front.
//
      pieceSize = pParam->pieceEnd - pParam->pieceStart;
      SendMessage(hCtlAVAILABLETIME, WM_SETTEXT,
            (WPARAM)0, (LONG)(LPSTR)chhmm(pParam->availableTime));
      SendMessage(hCtlPIECESTART, WM_SETTEXT,
            (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceStart));
      SendMessage(hCtlPIECEEND, WM_SETTEXT,
            (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceEnd));
      SendMessage(hCtlPIECESIZE, WM_SETTEXT,
            (WPARAM)0, (LONG)(LPSTR)chhmm(pieceSize));
      if(!(pParam->flags & RUNS_FLAG_EXTRABOARDPRIOR) &&
            !(pParam->flags & RUNS_FLAG_EXTRABOARDAFTER))
      {
        if(pParam->availableTime > 0)
        {
          if(pParam->availableTime + pieceSize > pParam->maxPieceSize)
            timeToUse = pParam->maxPieceSize - pieceSize;
          else
            timeToUse = pParam->availableTime;
//          if(pParam->pieceNumber == 1)
//          {
            SendMessage(hCtlSTARTSTART, WM_SETTEXT,
                  (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceStart - timeToUse));
            SendMessage(hCtlSTARTEND, WM_SETTEXT,
                  (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceStart));
//          }
//          else
//          {
            SendMessage(hCtlENDSTART, WM_SETTEXT,
                  (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceEnd));
            SendMessage(hCtlENDEND, WM_SETTEXT, 
                  (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->pieceEnd + timeToUse));
//          }
        }
      }
      else
      {
        if(pParam->flags & RUNS_FLAG_EXTRABOARDPRIOR)
        {
          SendMessage(hCtlSTARTSTART, WM_SETTEXT,
                (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->startStart));
          SendMessage(hCtlSTARTEND, WM_SETTEXT,
                (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->startEnd));
        }
        if(pParam->flags & RUNS_FLAG_EXTRABOARDAFTER)
        {
          SendMessage(hCtlENDSTART, WM_SETTEXT,
                (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->endStart));
          SendMessage(hCtlENDEND, WM_SETTEXT,
                (WPARAM)0, (LONG)(LPSTR)Tchar(pParam->endEnd));
        }
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Get the input times
//
          SendMessage(hCtlSTARTSTART, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          startStart = cTime(tempString);
          SendMessage(hCtlSTARTEND, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          startEnd = cTime(tempString);
          SendMessage(hCtlENDSTART, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          endStart = cTime(tempString);
          SendMessage(hCtlENDEND, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          endEnd = cTime(tempString);
//
//  Error checks
//
//  Rule 1: Start and end times must be both input or both blank
//
          if((startStart == NO_TIME && startEnd != NO_TIME) ||
                (startStart != NO_TIME && startEnd == NO_TIME))
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_244, hCtlSTARTSTART);
            break;
          }
          if((endStart == NO_TIME && endEnd != NO_TIME) ||
                (endStart != NO_TIME && endEnd == NO_TIME))
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_244, hCtlENDSTART);
            break;
          }
//
//  Rule 2: The start time must be earlier than the end time
//
          if(startStart != NO_TIME)
          {
            if(startStart > startEnd)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_245, hCtlSTARTSTART);
              break;
            }
          }
          if(endStart != NO_TIME)
          {
            if(endStart > endEnd)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_245, hCtlENDSTART);
              break;
            }
          }
//
//  Plug the values in
//
          pParam->flags = 0L;
          if(startStart != NO_TIME)
          {
            pParam->flags |= RUNS_FLAG_EXTRABOARDPRIOR;
            pParam->startStart = startStart;
            pParam->startEnd = startEnd;
          }
          if(endStart != NO_TIME)
          {
            pParam->flags |= RUNS_FLAG_EXTRABOARDAFTER;
            pParam->endStart = endStart;
            pParam->endEnd = endEnd;
          }
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
