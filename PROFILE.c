//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define BUCKETSIZE      900       // 900 Seconds = 15 minutes
#define BUCKETSPERHOUR (3600 / BUCKETSIZE)
#define MAXBUCKETS     (BUCKETSPERHOUR * 36)
#define ORIGINAL          0
#define LEFTOVER          1

BOOL CALLBACK PROFILEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlIDGRAPH;
  static WORD   DlgWidthUnits, DlgHeightUnits;
  static int    xMin, xMax, yMax;
  static HPEN   hPenText, hPen[2];
  static HBRUSH hBrushText, hBrush[2];
  static int    blockCount[2][MAXBUCKETS];
  static HDC    hDC;
  PAINTSTRUCT ps;
  TEXTMETRIC  textMetrics;
  HCURSOR     saveCursor;
  RECT   rcArea, rcLabel;
  char   szarLabel[10];
  SIZE   size;
  int    xBorder, xOffset;
  int    yBorder, yOffset;
  int    xSize, xScale, xPos;
  int    ySize, yScale, yPos;
  int    nI, nJ, nK;
  int    height;
  int    kLo, kHi;
  int    ascent;
  int    xStart, xEnd;
  int    timeLabel;
  int    numInternalRuns;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
    saveCursor = SetCursor(hCursorWait);
    DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
    DlgHeightUnits = HIWORD(GetDialogBaseUnits()) / 4;
//
//  Set up blockCount[ORIGINAL][] - The original blocks
//     and blockCount[LEFTOVER][] - What's left over to be cut
//
    memset(&blockCount, 0x00, sizeof(blockCount));
    for(nI = 0; nI < m_numBLOCKDATA; nI++)
    {
      kLo = max((int)(m_pBLOCKDATA[nI].onTime / BUCKETSIZE), 0);
      kHi = min((int)(m_pBLOCKDATA[nI].offTime / BUCKETSIZE), MAXBUCKETS - 1);
      for(nK = kLo; nK <= kHi; nK++)
      {
        blockCount[ORIGINAL][nK] = ++blockCount[LEFTOVER][nK];
      }
    }
//
//  Remove any cut blocks from blockCount[LEFTOVER][]
//
    numInternalRuns = AssembleRuns();
    for(nI = 0; nI < numInternalRuns; nI++)
    {
      for(nJ = 0; nJ < RUNLIST[nI].run.numPieces; nJ++)
      {
        kLo = max((int)(RUNLIST[nI].run.piece[nJ].fromTime / BUCKETSIZE), 0);
        kHi = min((int)(RUNLIST[nI].run.piece[nJ].toTime / BUCKETSIZE), MAXBUCKETS - 1);
        for(nK = kLo; nK <= kHi; nK++)
        {
          blockCount[LEFTOVER][nK]--;
        }
      }
    }
//
//  Set up the handles to the controls
//
    hCtlIDGRAPH = GetDlgItem(hWndDlg, PROFILE_GRAPH);
//
//  Create the pens for the graph
//
    hPenText = GetStockObject(BLACK_PEN);
    hBrushText = GetStockObject(BLACK_BRUSH);
    hPen[ORIGINAL] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    hBrush[ORIGINAL] = CreateSolidBrush(RGB(255, 128, 128));
    hPen[LEFTOVER] = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
    hBrush[LEFTOVER] = CreateSolidBrush(RGB(128, 128, 255));
//
//  Determine the plot extents
//
    xMin = 32767;
    xMax = 0;
    yMax = 0;
    for(nI = 0; nI < MAXBUCKETS; nI++)
    {
      if(blockCount[ORIGINAL][nI] > 0)
      {
        if(xMin > nI)
        {
          xMin = nI;
        }
        if(xMax < nI)
        {
          xMax = nI;
        }
        if(yMax < blockCount[ORIGINAL][nI])
        {
          yMax = blockCount[ORIGINAL][nI];
        }
      }
    }
    xMin = xMin / BUCKETSPERHOUR;
    xMax = (xMax + 7) / BUCKETSPERHOUR;
    yMax = max(yMax, 5);
//
//  InvalidateRect causes a redraw
//
    InvalidateRect(hWndDlg, NULL, TRUE);
    SetCursor(saveCursor);
    break;
//
//  WM_PAINT
//
  case WM_PAINT:
    if(GetUpdateRect(hWndDlg, NULL, FALSE))
    {
      hDC = BeginPaint(hWndDlg, &ps);

      SetBkMode(hDC, TRANSPARENT);
      GetWindowRect(hCtlIDGRAPH, &rcArea);
      MapWindowPoints(HWND_DESKTOP, hWndDlg, (POINT *)&rcArea, 2);
      GetTextExtentPoint32(hDC, "999", 3, &size);
      xOffset = size.cx + BUCKETSPERHOUR;
      yOffset = size.cy + BUCKETSPERHOUR;
      xBorder = 5;
      yBorder = 5;
      xSize = rcArea.right - rcArea.left - xOffset - xBorder * 2;
      ySize = rcArea.bottom - rcArea.top - yOffset - yBorder * 2;
      xScale = xSize / (xMax - xMin + 1);
      xScale += xScale % BUCKETSPERHOUR;
      yScale = ySize / ((yMax + 4) / 5);
      SelectObject(hDC, hBFont);
      GetTextMetrics(hDC, &textMetrics);
      xPos = rcArea.left + xBorder + xOffset;
      yPos = rcArea.bottom - yBorder - yOffset;
      MoveToEx(hDC, xPos, rcArea.top + yBorder, NULL);
      LineTo(hDC, xPos, yPos);
      LineTo(hDC, rcArea.right - xBorder, yPos);
      yPos -= yScale;
//
//  Draw the y-axis labels.
//
//  Keeping track of ascent lets us ensure that the labels won't overlap
//
      ascent = textMetrics.tmAscent;
      for(nJ = 1; nJ <= yMax / 5; nJ++)
      {
        SelectObject(hDC, hPenText);
        SelectObject(hDC, hBrushText);
        sprintf(szarLabel, "%d", nJ * 5);
        SetRect(&rcLabel, rcArea.left + xBorder, yPos - yOffset / 2, xPos - BUCKETSPERHOUR, yPos + yOffset / 2);
        if(ascent - yScale < 0)
        {
          DrawText(hDC, szarLabel, -1, &rcLabel, DT_BOTTOM | DT_SINGLELINE | DT_RIGHT );
          ascent = textMetrics.tmAscent;
        }
        else
        {
          ascent -= yScale;
        }
        MoveToEx(hDC, xPos - (BUCKETSPERHOUR / 2), yPos, NULL);
        LineTo(hDC, xPos + xMax, yPos);
        LineTo(hDC, rcArea.right - xBorder, yPos);
        yPos -= yScale;
      }
//
//  Draw the x-axis labels and the rectangles.  xMin and xMax were divided by BUCKETSPERHOUR
//  in WM_INITDIALOG so we could display the time of day at one hour intervals even though
//  the data reflects every BUCKETSIZE minutes.
//
      xPos = rcArea.left + xBorder + xOffset;
      yPos = rcArea.bottom - yBorder - yOffset;
      for(nI = xMin; nI < MAXBUCKETS; nI++)
      {
        if(xPos > rcArea.right - xBorder)
        {
          break;
        }
        SelectObject(hDC, hPenText);
        SelectObject(hDC, hBrushText);
        if(timeFormat == PREFERENCES_APX)
        {
          timeLabel = (nI % 12 == 0 ? 12 : nI % 12);
          sprintf(szarLabel, "%d%s", timeLabel, (nI < 12 ? "A" : nI < 24 ? "P" : "X"));
        }
        else
        {
          sprintf(szarLabel, "%d", nI);
        }
        SetRect(&rcLabel, xPos, yPos + 1, min(xPos + xScale, rcArea.right - xBorder), yPos + yOffset + 1);
        DrawText(hDC, szarLabel, -1, &rcLabel, DT_LEFT | DT_VCENTER);
        MoveToEx(hDC, xPos, yPos + 2, NULL);
        LineTo(hDC, xPos, yPos);
        LineTo(hDC, xPos, rcArea.top + yBorder);
        for(nJ = 0; nJ < 2; nJ++)
        {
          xStart = xPos;
          SelectObject(hDC, hPen[nJ]);
          SelectObject(hDC, hBrush[nJ]);
          for(nK = 0; nK < BUCKETSPERHOUR; nK++)
          {
            xEnd = xStart + xScale / BUCKETSPERHOUR;
            if((height = blockCount[nJ][BUCKETSPERHOUR * nI + nK]) > 0)
            {
              Rectangle(hDC, xStart, yPos - height * yScale / 5, xEnd, yPos);
            }
            xStart = xEnd;
          }
        }
        xPos += xScale;
      }
      SelectObject(hDC, hPenText);
      SelectObject(hDC, hBrushText);
      EndPaint(hWndDlg, &ps);
    }
    break;       //  End of WM_PAINT
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
      switch(wmId)
      {
//
//  IDOK / IDCANCEL
//
        case IDOK:
        case IDCANCEL:
          ReleaseDC(hCtlIDGRAPH, hDC);
          EndDialog(hWndDlg, wmId == IDOK);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Profile);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
