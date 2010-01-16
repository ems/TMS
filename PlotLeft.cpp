//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// PlotLeft.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "FlatSB.h"
#include "PlotLeft.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define NODEOFFSET 22
#define TIMEOFFSET 10

/////////////////////////////////////////////////////////////////////////////
// CPlotLeft dialog


CPlotLeft::CPlotLeft(CWnd* pParent /*=NULL*/)
	: CDialog(CPlotLeft::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlotLeft)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPlotLeft::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlotLeft)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlotLeft, CDialog)
	//{{AFX_MSG_MAP(CPlotLeft)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(PLOTLEFT_ALLBLOCKS, OnAllblocks)
	ON_BN_CLICKED(PLOTLEFT_WORKREMAINING, OnWorkremaining)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotLeft message handlers

BOOL CPlotLeft::OnInitDialog() 
{
	CDialog::OnInitDialog();

  long blockNumber;
  int  nI;
  int  nJ;

  m_pBP = NULL;
//
//  Set up the handles to the controls
//
  pGRAPH = (CWnd *)GetDlgItem(PLOTLEFT_GRAPH);
  pFlatSBSCROLLBAR = (CFlatSB *)GetDlgItem(PLOTLEFT_SCROLLBAR);
  pButtonALLBLOCKS = (CButton *)GetDlgItem(PLOTLEFT_ALLBLOCKS);
  pButtonWORKREMAINING = (CButton *)GetDlgItem(PLOTLEFT_WORKREMAINING);
//
//  And default to all blocks
//
  pButtonALLBLOCKS->SetCheck(TRUE);
//
//  Count the unique blocks
//
  blockNumber = NO_RECORD;
  numBPBlocks = 0;
  for(nI = 0; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].blockNumber != blockNumber)
    {
      numBPBlocks++;
      blockNumber = m_pRELIEFPOINTS[nI].blockNumber;
    }
  }
//
//  Allocate space for the structure
//
  m_pBP = (BLOCKPLOTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BLOCKPLOTDef) * numBPBlocks); 
  if(m_pBP == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    OnClose();
    return(TRUE);
  }
//
//  Save the pertinent block information
//
  m_pBP[0].blockNumber = m_pRELIEFPOINTS[0].blockNumber;
  m_pBP[0].startIndex = 0;
  m_pBP[0].endIndex = 0;
  m_pBP[0].flags = BP_DISPLAYED;
  nJ = 0;
  for(nI = 1; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].blockNumber != m_pBP[nJ].blockNumber)
    {
      nJ++;
      m_pBP[nJ].blockNumber = m_pRELIEFPOINTS[nI].blockNumber;
      m_pBP[nJ].startIndex = nI;
      m_pBP[nJ].flags = BP_DISPLAYED;
    }
    m_pBP[nJ].endIndex = nI;
  }
//
//  Set up the pens
//
  m_Pens[CUT].CreatePen(PS_SOLID,     THICK, RGB(255,   0,   0));  // Red
  m_Pens[UNCUT].CreatePen(PS_SOLID,   THICK, RGB(  0,   0, 255));  // Blue
  m_Pens[HASH].CreatePen(PS_SOLID,    THICK, RGB(  0,   0,   0));  // Black
  m_Pens[TXT].CreatePen(PS_SOLID, THICK + 2, RGB(  0,   0,   0));  // Black
  m_Pens[AXIS].CreatePen(PS_SOLID,        1, RGB(192, 192, 192));  // Light Grey
  m_Pens[WTXT].CreatePen(PS_SOLID,        1, RGB(255, 255, 255));  // White
//
//  Set up the window extents
//
//  This is nothing short of complete and utter bullshit
//
  RECT  control_rc;
  int   width;
  int   height;

  pGRAPH->GetClientRect(&control_rc);
  width  = control_rc.right;
  height = control_rc.bottom;

  GetWindowRect(&m_rcDialog);
  pGRAPH->GetWindowRect(&control_rc);
  m_rc.top = control_rc.top - m_rcDialog.top - GetSystemMetrics(SM_CYCAPTION);
  m_rc.left = control_rc.left - m_rcDialog.left;
  m_rc.bottom = m_rc.top + height;
  m_rc.right = m_rc.left + width;

  m_rcRepaint = m_rc;
  m_rcRepaint.bottom -= 4;
  m_rcRepaint.right -=4;
//
//  Determine the earliest and latest block times
//
  m_earliestTime = 86400L;
  m_latestTime = NO_TIME;
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      if(m_pRELIEFPOINTS[m_pBP[nI].startIndex].time < m_earliestTime)
        m_earliestTime = m_pRELIEFPOINTS[m_pBP[nI].startIndex].time;
      if(m_pRELIEFPOINTS[m_pBP[nI].endIndex].time > m_latestTime)
        m_latestTime = m_pRELIEFPOINTS[m_pBP[nI].endIndex].time;
    }
  }
//
//  Round the earliest down to the previous hour
//  Round the latest up to the next hour
//
  width -= INDENT * 2;
  m_earliestTime -= 60;
  m_earliestTime -= m_earliestTime % 3600;
  m_latestTime += 60;
  m_latestTime += 3600 - (m_latestTime % 3600);
  m_timeInc = (long)(width / (float)((m_latestTime - m_earliestTime) / 3600));
//
//  Set the maximum value for the scroll bar and the jump increment
//
  short int nChange = BLOCKSDISPLAYED - 1;

  pFlatSBSCROLLBAR->SetMax(numBPBlocks);
  pFlatSBSCROLLBAR->SetLargeChange(nChange);
//
//  Set the starting position of the display
//
  m_firstDisplayed = 0;
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPlotLeft::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  RECT  rcDraw;
  long  tempTime;
  int   nI;
  int   nJ;

//
//  Set up the font
//
  CFont   Font;
  LOGFONT lf;

  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(7, dc.GetDeviceCaps(LOGPIXELSY), 72);
  lf.lfWeight = FW_THIN;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
  strcpy(lf.lfFaceName, "Tahoma");
  Font.CreateFontIndirect(&lf);
//
//  Get the average character widht
//
  TEXTMETRIC tm;

  dc.GetTextMetrics(&tm);
  if(tm.tmAveCharWidth <= 0)
    tm.tmAveCharWidth = 4;
//
//  Set up the rest of the device context's properties
//
  dc.SelectObject(Font);
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(RGB(0, 0, 0));
//
//  Write out the time of day on the top
//
  memcpy(&rcDraw, &m_rc, sizeof(RECT));
//
//  Adjust the left and right to ensure there's room for a block number
//
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  rcDraw.top -= GetSystemMetrics(SM_CYCAPTION);
  dc.SelectObject(m_Pens[TXT]);
  tempTime = m_earliestTime;
  while(tempTime <= m_latestTime)
  {
    strcpy(tempString, Tchar(tempTime));
    dc.TextOut(rcDraw.left - 10, rcDraw.top, tempString, strlen(tempString));
    tempTime += 3600;
    rcDraw.left += m_timeInc;
  }
//
//  Write out the axis lines dropping from the time of day
//
  memcpy(&rcDraw, &m_rc, sizeof(RECT));
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  dc.SelectObject(m_Pens[AXIS]);
  tempTime = m_earliestTime;
  while(tempTime <= m_latestTime)
  {
    dc.MoveTo(rcDraw.left, rcDraw.top - 4);
    dc.LineTo(rcDraw.left, rcDraw.bottom - 4);
    tempTime += 3600;
    rcDraw.left += m_timeInc;
  }
//
//  Write out the legend
//
  POINT point;
//
//  Cut work
//
  dc.SelectObject(m_Pens[CUT]);
  point.x = ((m_rcDialog.right - m_rcDialog.left) / 2) - 3;
  point.y = (m_rc.bottom + GetSystemMetrics(SM_CYCAPTION) / 2) + 10;
  dc.Rectangle(point.x, point.y, point.x + 6, point.y + 6);
  dc.SelectObject(m_Pens[TXT]);
  strcpy(tempString, "Cut work");
  dc.TextOut(point.x + 12, point.y - 3, tempString, strlen(tempString));
//
//  Uncut
//
  dc.SelectObject(m_Pens[UNCUT]);
  point.y = m_rc.bottom + GetSystemMetrics(SM_CYCAPTION) + 14;
  dc.Rectangle(point.x, point.y, point.x + 6, point.y + 6);
  dc.SelectObject(m_Pens[TXT]);
  strcpy(tempString, "Available");
  dc.TextOut(point.x + 12, point.y - 3, tempString, strlen(tempString));
//
//  Set up the RELIEFPOINTS x,y co-ords
//
  memcpy(&rcDraw, &m_rc, sizeof(RECT));
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    m_pRELIEFPOINTS[m_pBP[nI].startIndex].plot.x = 0;
    m_pRELIEFPOINTS[m_pBP[nI].startIndex].plot.y = 0;
  }
  point.y = rcDraw.top + INDENT - 2;
  for(nI = m_firstDisplayed; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      if(point.y > rcDraw.bottom)
      {
        m_pRELIEFPOINTS[m_pBP[nI].startIndex].plot.y = NO_RECORD;
        break;
      }
      for(nJ = m_pBP[nI].startIndex; nJ <= m_pBP[nI].endIndex; nJ++)
      {
        point.x = (long)(rcDraw.left +
              (((float)(m_pRELIEFPOINTS[nJ].time - m_earliestTime) / 3600) * m_timeInc));
        m_pRELIEFPOINTS[nJ].plot.x = point.x;
        m_pRELIEFPOINTS[nJ].plot.y = point.y;
      }
      m_pRELIEFPOINTS[m_pBP[nI].startIndex].plot.x += 4;  // It looks better
      point.y += (long)(INDENT * 1.5);
    }
  }
//
//  Display the selected blocks
//
  for(nI = m_firstDisplayed; nI < numBPBlocks; nI++)
  {
    if(m_pRELIEFPOINTS[m_pBP[nI].startIndex].plot.y == NO_RECORD)  // End of paintable area
      break;
    if(!(m_pBP[nI].flags & BP_DISPLAYED))
      continue;
//
//  Front of block
//
    nJ = m_pBP[nI].startIndex;
//
//  Block number
//
    sprintf(tempString, "%ld", m_pBP[nI].blockNumber);
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - strlen(tempString) * tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y - 6,
          tempString, strlen(tempString));
//
//  Pullout node and time
//
    nJ = m_pBP[nI].startIndex;
    NODESKey0.recordID = m_pRELIEFPOINTS[nJ].NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    tempString[NODES_ABBRNAME_LENGTH] = '\0';
//
//  The node name goes above the line
//
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth * 2,
          m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
    strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
    if(tempString[0] == ' ')
      memcpy(tempString, &tempString[1], strlen(tempString));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth * 2,
          m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));
//
//  End of block
//
    nJ = m_pBP[nI].endIndex;
//
//  Block length
//
    long blockLength = m_pRELIEFPOINTS[nJ].time - m_pRELIEFPOINTS[m_pBP[nI].startIndex].time;

    strcpy(tempString, chhmm(blockLength));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x + tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y - 6,
          tempString, strlen(tempString));
//
//  Pullin node and time
//
    NODESKey0.recordID = m_pRELIEFPOINTS[nJ].NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    tempString[NODES_ABBRNAME_LENGTH] = '\0';
//
//  The node name goes above the line
//
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
    strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
    if(tempString[0] == ' ')
      memcpy(tempString, &tempString[1], strlen(tempString));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));

//
//  Cycle through all the relief points
//
    for(nJ = m_pBP[nI].startIndex + 1; nJ <= m_pBP[nI].endIndex; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == NO_RECORD)
        dc.SelectObject(m_Pens[UNCUT]);
      else
        dc.SelectObject(m_Pens[CUT]);
      dc.Rectangle(m_pRELIEFPOINTS[nJ - 1].plot.x, m_pRELIEFPOINTS[nJ - 1].plot.y - 1,
            m_pRELIEFPOINTS[nJ].plot.x, m_pRELIEFPOINTS[nJ].plot.y + 1);
    }
//
//  Do a second pass through so the hash marks aren't overwritten
//
    POINT startPiece;
    POINT midPiece;
    long  fromTime, totalTime;
    
    nJ = m_pBP[nI].startIndex;

    startPiece.x = m_pRELIEFPOINTS[nJ].plot.x;
    startPiece.y = m_pRELIEFPOINTS[nJ].plot.y;
    fromTime = m_pRELIEFPOINTS[nJ].time;

    for(nJ = m_pBP[nI].startIndex + 1; nJ <= m_pBP[nI].endIndex; nJ++)
    {
//
//  If this is a cut point, then display the node and its time along with a hash mark
//
      if(m_pRELIEFPOINTS[nJ].end.runNumber != m_pRELIEFPOINTS[nJ].start.runNumber &&
            nJ != m_pBP[nI].endIndex)
      {
//
//  Hash mark
//
        dc.SelectObject(m_Pens[HASH]);
        dc.Rectangle(m_pRELIEFPOINTS[nJ].plot.x, m_pRELIEFPOINTS[nJ].plot.y - 4,  
              m_pRELIEFPOINTS[nJ].plot.x + 2, m_pRELIEFPOINTS[nJ].plot.y + 4);
//
//  Node and time
//
        NODESKey0.recordID = m_pRELIEFPOINTS[nJ].NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        tempString[NODES_ABBRNAME_LENGTH] = '\0';
//
//  The node name goes above the line
//
        int xAdj = (int)(tm.tmAveCharWidth * (NODES_ABBRNAME_LENGTH * 0.375));

        dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
              m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
        strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
        if(tempString[0] == ' ')
          memcpy(tempString, &tempString[1], strlen(tempString));
        dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
              m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));
      }
//
//  Put out either the time in hh:mm in the middle of uncut pieces,
//  or the time and run number in the middle of cut pieces
//
//  Same if as before, just include the pullin this time
//
      if(m_pRELIEFPOINTS[nJ].end.runNumber != m_pRELIEFPOINTS[nJ].start.runNumber ||
            nJ == m_pBP[nI].endIndex)
      {
        totalTime = m_pRELIEFPOINTS[nJ].time - fromTime;
        midPiece.x = (startPiece.x + m_pRELIEFPOINTS[nJ].plot.x) / 2;
        midPiece.y = startPiece.y;
        strcpy(tempString, chhmm(totalTime));
//
//  Length of piece
//
        CSize size;
        int   cx;

        dc.SetTextColor(RGB(0, 128, 0));
        size = dc.GetTextExtent(tempString, strlen(tempString));
        cx = size.cx;
        dc.TextOut(midPiece.x - (cx / 2), midPiece.y + 2, tempString, strlen(tempString));
//
//  Run number in cut piece
//
        if(m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD)
        {
          sprintf(tempString, "%ld", m_pRELIEFPOINTS[nJ].end.runNumber);
          size = dc.GetTextExtent(tempString, strlen(tempString));
          cx = size.cx;
          dc.TextOut(midPiece.x - (cx / 2), midPiece.y - 14, tempString, strlen(tempString));
        }
        dc.SetTextColor(RGB(0, 0, 0));
//
//  Now set the start of this (next) piece
//
        startPiece.x = m_pRELIEFPOINTS[nJ].plot.x;
        startPiece.y = m_pRELIEFPOINTS[nJ].plot.y;
        fromTime = m_pRELIEFPOINTS[nJ].time;

      }
    }
  }
}
void CPlotLeft::OnClose() 
{
  OnOK();
}

void CPlotLeft::OnHelp() 
{
}

void CPlotLeft::OnOK() 
{
  TMSHeapFree(m_pBP);

  for(int nI = 0; nI < LASTPEN; nI++)
  {
    DeleteObject(m_Pens[nI]);
  }

	CDialog::OnOK();
}

void CPlotLeft::OnAllblocks() 
{
  int nI;

  for(nI = 0; nI < numBPBlocks; nI++)
  {
    m_pBP[nI].flags = BP_DISPLAYED;
  }
  pFlatSBSCROLLBAR->SetMax(numBPBlocks);
  m_firstDisplayed = 0;
  pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
  InvalidateRect(&m_rcRepaint, TRUE);
}

void CPlotLeft::OnWorkremaining() 
{
  int nI, nJ;
  int numDisplayed = 0;

  for(nI = 0; nI < numBPBlocks; nI++)
  {
    m_pBP[nI].flags = 0;
    for(nJ = m_pBP[nI].startIndex; nJ < m_pBP[nI].endIndex; nJ++)
    {
      if((m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
            m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD) ||
           (m_pRELIEFPOINTS[nJ].start.runNumber != NO_RECORD &&
            m_pRELIEFPOINTS[nJ].flags & RPFLAGS_FIRSTINBLOCK) ||
           (m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD &&
            m_pRELIEFPOINTS[nJ].flags & RPFLAGS_LASTINBLOCK))
      {
      }  // yeah yeah
      else
      {
        m_pBP[nI].flags = BP_DISPLAYED;
        numDisplayed++;
        break;
      }
    }
  }
  m_firstDisplayed = 0;
  pFlatSBSCROLLBAR->SetMax(numDisplayed);
  pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
  InvalidateRect(&m_rcRepaint, TRUE);
}

void PlotLeftDialog()
{
  CPlotLeft dlg;

  dlg.DoModal();
}

extern "C"
{

void PlotLeft()
{
  PlotLeftDialog();
}

}  // extern "C"

BEGIN_EVENTSINK_MAP(CPlotLeft, CDialog)
    //{{AFX_EVENTSINK_MAP(CPlotLeft)
	ON_EVENT(CPlotLeft, PLOTLEFT_SCROLLBAR, 1 /* Change */, OnChangeScrollbar, VTS_NONE)
	ON_EVENT(CPlotLeft, PLOTLEFT_SCROLLBAR, 2 /* Scroll */, OnScrollScrollbar, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPlotLeft::OnChangeScrollbar() 
{
  m_firstDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_rcRepaint, TRUE);
}

void CPlotLeft::OnScrollScrollbar() 
{
  m_firstDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_rcRepaint, TRUE);
}

//
//  PreTranslateMessage - Capture <Home>, <End>, <PgUp>, and <PgDn>
//
BOOL CPlotLeft::PreTranslateMessage(MSG* pMsg) 
{
  if(pMsg->message == WM_KEYUP)
  {
    int nI;
    int numDisplayable = 0;

    for(nI = 0; nI < numBPBlocks; nI++)
    {
      if(m_pBP[nI].flags & BP_DISPLAYED)
        numDisplayable++;
    }
    switch LOWORD(pMsg->wParam)
    {
      case VK_HOME:
        m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_END:
        m_firstDisplayed = numDisplayable - (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed < 0)
          m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_PRIOR:
        m_firstDisplayed -= (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed < 0)
          m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_NEXT:
        m_firstDisplayed += (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed > numDisplayable - (BLOCKSDISPLAYED - 1))
          m_firstDisplayed = numDisplayable - (BLOCKSDISPLAYED - 1);
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
    }
  }
	return CDialog::PreTranslateMessage(pMsg);
}
