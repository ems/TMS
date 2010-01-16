//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// TripGraph.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}

#include "TMS.h"
#include "ResizingDialog.h"
#include "TripGraph.h"
#include "SelectPatterns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTripGraph dialog


CTripGraph::CTripGraph(CWnd* pParent, PDISPLAYINFO pDI)
	: CResizingDialog(CTripGraph::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTripGraph)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
  m_pDI = pDI;

  SetControlInfo(IDPRINT,  ANCHORE_RIGHT);
  SetControlInfo(IDOK,     ANCHORE_RIGHT);
  SetControlInfo(IDCANCEL, ANCHORE_RIGHT);
  SetControlInfo(IDHELP,   ANCHORE_RIGHT);
  SetControlInfo(TRIPGRAPH_STATUS, ANCHORE_BOTTOM);
	m_bDrawGripper = TRUE;
}


void CTripGraph::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTripGraph)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTripGraph, CDialog)
	//{{AFX_MSG_MAP(CTripGraph)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONUP()
	ON_COMMAND(TRIPGRAPH_SELECTPATTERNS, OnSelectpatterns)
	ON_COMMAND(TRIPGRAPH_EDITTRIP, OnEdittrip)
	ON_COMMAND(TRIPGRAPH_RELEASETRIP, OnReleasetrip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTripGraph message handlers

BOOL CTripGraph::OnInitDialog() 
{
	CDialog::OnInitDialog();

  long  patNodeList[2][RECORDIDS_KEPT];
  long  patNodeSeqs[2][RECORDIDS_KEPT];
  long  tempLong[RECORDIDS_KEPT];
  long  previousPATTERNNAMESrecordID;
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  BOOL  bFound;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   pos;
  int   lastFound;
  int   numNodes;

  GetClientRect(&m_rcOriginal);
  m_rcOriginal.left += 16;
  m_rcOriginal.right -= 80;
//
//  Set up handles to the controls
//
  pStaticOBLEGEND = (CStatic *)GetDlgItem(TRIPGRAPH_OBLEGEND);
  pStaticIBLEGEND = (CStatic *)GetDlgItem(TRIPGRAPH_IBLEGEND);
  pButtonPRINT = (CButton *)GetDlgItem(IDPRINT);
  pButtonOK = (CButton *)GetDlgItem(IDOK);
  pButtonCANCEL = (CButton *)GetDlgItem(IDCANCEL);
  pButtonHELP = (CButton *)GetDlgItem(IDHELP);
  pStaticSTATUS = (CStatic *)GetDlgItem(TRIPGRAPH_STATUS);
//
//  Set the dialog title
//
  SERVICESKey0.recordID = m_pDI->fileInfo.serviceRecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(serviceName, SERVICES_NAME_LENGTH);

  ROUTES.recordID = m_pDI->fileInfo.routeRecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  strcpy(routeNumberAndName, tempString);
  strcat(routeNumberAndName, " - ");
  strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
  trim(tempString, ROUTES_NAME_LENGTH);
  strcat(routeNumberAndName, tempString);

  CString s;
  GetWindowText(s);

  sprintf(tempString, s, serviceName, routeNumberAndName);
  SetWindowText(tempString);
//
//  Display the direction abbreviations
//
  for(nI = 0; nI < 2; nI++)
  {
    if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      break;
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
    strcat(tempString, " Trips");
    if(nI == 0)
    {
      pStaticOBLEGEND->SetWindowText(tempString);
    }
    else
    {
      pStaticIBLEGEND->SetWindowText(tempString);
    }
  }
//
//  Get all the sequences for all the patterns on this route
//
  ROUTESKey0.recordID = m_pDI->fileInfo.routeRecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  for(nI = 0; nI < 2; nI++)
  {
    m_numPatterns[nI] = 0;
    if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      continue;
    PATTERNSKey2.ROUTESrecordID = m_pDI->fileInfo.routeRecordID;
    PATTERNSKey2.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
    PATTERNSKey2.directionIndex = nI;
    PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
    PATTERNSKey2.nodeSequence = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    previousPATTERNNAMESrecordID = NO_RECORD;
    nJ = 0;
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == m_pDI->fileInfo.routeRecordID &&
          PATTERNS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID &&
          PATTERNS.directionIndex == nI)
    {
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        if(PATTERNS.PATTERNNAMESrecordID != previousPATTERNNAMESrecordID)
        {
          m_PATTERNDATA[nI][nJ].flags = TRIPGRAPH_FLAG_PATTERNACTIVE;
          if(previousPATTERNNAMESrecordID != NO_RECORD)
            nJ++;
          previousPATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
          m_PATTERNDATA[nI][nJ].numSeqences = 0;
          m_PATTERNDATA[nI][nJ].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
          nK = 0;
        }
        if(nJ == 0)
        {
          patNodeList[nI][nK] = PATTERNS.NODESrecordID;
          patNodeSeqs[nI][nK] = PATTERNS.nodeSequence;
        }
        m_PATTERNDATA[nI][nJ].patternSequence[nK] = PATTERNS.nodeSequence;
        m_PATTERNDATA[nI][nJ].numSeqences++;
        nK++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
    m_numPatterns[nI] = nJ + 1;
  }
//
//  Merge the BASE pattern lists
//
  m_posMax = 0;
  for(nI = 0; nI < 2; nI++)
  {
    for(nJ = 0; nJ < RECORDIDS_KEPT; nJ++)
    {
      m_NODEDATA.sequence[nI][nJ] = NO_RECORD;
    }
  }
  for(nI = 0; nI < 2; nI++)
  {
    if(m_numPatterns[nI] == 0)
      break;
    pos = nI == 0 ? 0 : m_posMax - 1;
    lastFound = pos;
    for(nJ = 0; nJ < m_PATTERNDATA[nI][0].numSeqences; nJ++)
    {
      if(m_posMax >= RECORDIDS_KEPT)
        break;
      if(nI == 0)
      {
        m_NODEDATA.sequence[0][pos] = patNodeSeqs[0][nJ];
        m_NODEDATA.recordIDs[pos++] = patNodeList[0][nJ];
        m_posMax = pos;
      }
      else
      {
        for(bFound = FALSE, nK = pos; nK >= 0; nK--)
        {
          if(patNodeList[1][nJ] == m_NODEDATA.recordIDs[nK])
          {
            bFound = TRUE;
            m_NODEDATA.sequence[1][nK] = patNodeSeqs[1][nJ];
            lastFound = nK;
            pos--;
            break;
          }
        }
        if(!bFound)
        {
          memcpy(&tempLong, &m_NODEDATA.recordIDs[lastFound], sizeof(long) * (m_posMax - lastFound + 1));
          memcpy(&m_NODEDATA.recordIDs[lastFound + 1], &tempLong, sizeof(long) * (m_posMax - lastFound + 1));
          for(nL = 0; nL < 2; nL++)
          {
            memcpy(&tempLong, &m_NODEDATA.sequence[nL][lastFound], sizeof(long) * (m_posMax - lastFound + 1));
            memcpy(&m_NODEDATA.sequence[nL][lastFound + 1], &tempLong, sizeof(long) * (m_posMax - lastFound + 1));
          }
          m_NODEDATA.recordIDs[lastFound] = patNodeList[1][nJ];
          m_NODEDATA.sequence[0][lastFound] = NO_RECORD;
          m_NODEDATA.sequence[1][lastFound] = patNodeSeqs[1][nJ];
          m_posMax++;
        }  // if !bFound
      }  // else
    }  // for nJ
  }  // for nI
//
//  If there weren't any, leave
//
  if(m_posMax == 0)
  {
    OnClose();
    return TRUE;
  }
//
//  Get the trips
//
//
//  Reposition to this route
//
  ROUTESKey0.recordID = m_pDI->fileInfo.routeRecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
//
//  Get the earliest and latest trip times
//
//
  m_earliestTime = 86400L;
  m_latestTime = NO_TIME;
  for(nI = 0; nI < 2; nI++)
  {
    if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      break;
//
//  Get the earliest trip in this direction
//
    TRIPSKey1.ROUTESrecordID = m_pDI->fileInfo.routeRecordID;
    TRIPSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
    TRIPSKey1.directionIndex = nI;
    TRIPSKey1.tripSequence = NO_TIME;
    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    if(rcode2 != 0 ||
          TRIPS.ROUTESrecordID != m_pDI->fileInfo.routeRecordID ||
          TRIPS.SERVICESrecordID != m_pDI->fileInfo.serviceRecordID ||
          TRIPS.directionIndex != nI)
      break;
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &m_GTResults);
    if(m_GTResults.firstNodeTime < m_earliestTime)
      m_earliestTime = m_GTResults.firstNodeTime;
    if(m_GTResults.lastNodeTime > m_latestTime)
      m_latestTime = m_GTResults.lastNodeTime;
//
//  Get the latest trip in this direction
//
    TRIPSKey1.ROUTESrecordID = m_pDI->fileInfo.routeRecordID;
    TRIPSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
    TRIPSKey1.directionIndex = nI;
    TRIPSKey1.tripSequence =  172800L;
    rcode2 = btrieve(B_GETLESSTHAN, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    if(rcode2 != 0 ||
          TRIPS.ROUTESrecordID != m_pDI->fileInfo.routeRecordID ||
          TRIPS.SERVICESrecordID != m_pDI->fileInfo.serviceRecordID ||
          TRIPS.directionIndex != nI)
      break;
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &m_GTResults);
    if(m_GTResults.firstNodeTime < m_earliestTime)
      m_earliestTime = m_GTResults.firstNodeTime;
    if(m_GTResults.lastNodeTime > m_latestTime)
      m_latestTime = m_GTResults.lastNodeTime;
  }
//
//  Round the earliest down to the previous hour
//  Round the latest up to the next hour
//
  m_earliestTime -= 60;
  m_earliestTime -= m_earliestTime % 3600;
  m_latestTime += 60;
  m_latestTime += 3600 - (m_latestTime % 3600);
//
//  Go through all the trips
//
  for(nI = 0; nI < 2; nI++)
  {
    m_numTrips[nI] = 0;
    if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
      break;
//
//  Loop through the trips
//
    TRIPSKey1.ROUTESrecordID = m_pDI->fileInfo.routeRecordID;
    TRIPSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
    TRIPSKey1.directionIndex = nI;
    TRIPSKey1.tripSequence = NO_TIME;
    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    while(rcode2 == 0 &&
          TRIPS.ROUTESrecordID == m_pDI->fileInfo.routeRecordID &&
          TRIPS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID &&
          TRIPS.directionIndex == nI)
    {
//
//  Get the index into m_PATTERNDATA
//
      for(nJ = 0; nJ < m_numPatterns[nI]; nJ++)
      {
        if(TRIPS.PATTERNNAMESrecordID == m_PATTERNDATA[nI][nJ].PATTERNNAMESrecordID)
        {
          m_TRIPGRAPH[nI][m_numTrips[nI]].patternIndex = nJ;
          break;
        }
      }
//
//  Generate the trip
//

      numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &m_GTResults);
//
//  Put the times in the m_TRIPGRAPH structure
//
      for(nJ = 0; nJ < numNodes; nJ++)
      {
        m_TRIPGRAPH[nI][m_numTrips[nI]].tripTimes[nJ] = m_GTResults.tripTimes[nJ];
      }
//
//  Get the rest of the TRIPGRAPH stuff and get the next trip
//
      m_TRIPGRAPH[nI][m_numTrips[nI]].TRIPSrecordID = TRIPS.recordID;
      m_numTrips[nI]++;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    }
  }
//
//  Set up some spacing information for OnPaint()
//
//  Set up the font
//
  CFont    Font;
  CPaintDC dc(this);
  LOGFONT  lf;

  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(7, dc.GetDeviceCaps(LOGPIXELSY), 72);
  lf.lfWeight = FW_THIN;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
  strcpy(lf.lfFaceName, "Tahoma");
  Font.CreateFontIndirect(&lf);
//
//  Get the average character width
//
  TEXTMETRIC tm;
  SIZE  size;

  dc.GetTextMetrics(&tm);
  if(tm.tmAveCharWidth <= 0)
    tm.tmAveCharWidth = 4;
//
//  Set up the rest of the device context's properties
//
  dc.SelectObject(Font);
  GetTextExtentPoint32(dc.m_hDC, "W", 1, &size);
  m_CharWidth  = size.cx;
  m_CharHeight = size.cy;

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTripGraph::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CTripGraph::OnPaint() 
{
  CResizingDialog::OnPaint();

  
  CPaintDC dc(this);

  LOGFONT  lf;
  long  tempTime;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   xPos1, xPos2;
  RECT  rc;
  HPEN  hPen, hPenOld, hPenDot;
  long topMost;
  long bottomMost;
  long spacing;
  long indent;
  long  timeInc;
  long  timeAtNode;
  RECT  rcTemp;
  BOOL  Printing = FALSE;

  HDC hDC = dc.m_hDC;
    
//
//  Set up the font
//
  CFont    Font;

  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(7, dc.GetDeviceCaps(LOGPIXELSY), 72);
  lf.lfWeight = FW_THIN;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
  strcpy(lf.lfFaceName, "Tahoma");
  Font.CreateFontIndirect(&lf);
  m_hFontOld = (HFONT)dc.SelectObject(Font);
  memcpy(&rc, &m_rcOriginal, sizeof(RECT));
  rc.top += m_CharHeight;
  rc.left += m_CharWidth;
  rc.bottom -= m_CharHeight;
  rc.right -=m_CharWidth;
  SetBkMode(hDC, TRANSPARENT);
//
//  Establish the spacing between the nodes
//
  indent = m_CharHeight * 4;
  topMost = rc.top + indent;
  bottomMost = rc.bottom; // - indent;
  spacing = (bottomMost - topMost + 1) / m_posMax;
  
//
//  Display the nodes
//
  memcpy(&rcTemp, &rc, sizeof(RECT));
  rcTemp.top += (indent - (m_CharHeight / 2)); //(long)(m_CharHeight * 2.5);
  for(nJ = 0; nJ < m_posMax; nJ++)
  {
    NODESKey0.recordID = m_NODEDATA.recordIDs[nJ];
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    if(m_NODEDATA.sequence[0][nJ] != NO_RECORD && m_NODEDATA.sequence[1][nJ] != NO_RECORD)
      SetTextColor(hDC, RGB(0, 0, 0));
    else
      SetTextColor(hDC, m_NODEDATA.sequence[0][nJ] != NO_RECORD ? RGB(255, 0, 0) : RGB(0, 0, 255));
    DrawText(hDC, tempString, -1, &rcTemp, DT_LEFT | DT_TOP | DT_NOPREFIX);
    m_NODEDATA.yPos[nJ] = rcTemp.top + (m_CharHeight / 2); //(m_CharHeight / 2);
    rcTemp.top += spacing; //m_CharHeight;
  }
//
//  Draw the graph axes
//
  memcpy(&rcTemp, &rc, sizeof(RECT));
  rcTemp.top += indent;//m_CharHeight * 3;
  rcTemp.left += (long)(m_CharWidth * 5 * 0.95);
  hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
  hPenOld = (HPEN)dc.SelectObject(hPen);
  for(nI = 0; nI < m_posMax; nI++)
  {
    MoveToEx(hDC, rcTemp.left, rcTemp.top, (LPPOINT)NULL);
    LineTo(hDC, rcTemp.right, rcTemp.top);
    rcTemp.top += spacing; //m_CharHeight;
  }
  SelectObject(hDC, hPenOld);
  DeleteObject(hPen);
  memcpy(&rcTemp, &rc, sizeof(RECT));
  rcTemp.top += indent; //m_CharHeight * 3;
  rcTemp.left += m_CharWidth * 5;
  rcTemp.bottom = rcTemp.top + ((m_posMax - 1) * spacing);
  hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  hPenOld = (HPEN)dc.SelectObject(hPen);
  MoveToEx(hDC, rcTemp.left, rcTemp.top, (LPPOINT)NULL);
  LineTo(hDC, rcTemp.left, rcTemp.bottom);
  LineTo(hDC, rcTemp.right, rcTemp.bottom);
  SelectObject(hDC, hPenOld);
  DeleteObject(hPen);
  SelectObject(hDC, m_hFontOld);
  DeleteObject(m_hFont);
  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(Printing? 4 : 8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
  strcpy(lf.lfFaceName, "Verdana");
  lf.lfEscapement = -450;
  m_hFont = CreateFontIndirect(&lf);
  m_hFontOld = (HFONT)dc.SelectObject(m_hFont);
  timeInc = (rcTemp.right - rcTemp.left) / ((m_latestTime - m_earliestTime) / 3600);
  hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
  hPenOld = (HPEN)dc.SelectObject(hPen);
  SetTextColor(hDC, RGB(0, 0, 0));
//
//  Write out the x-axis labels on the bottom
//
  tempTime = m_earliestTime;
  m_TIMEPERIOD.numTimePeriods = 0;
  m_TIMEPERIOD.interval = timeInc;
  while(tempTime <= m_latestTime)
  {
    strcpy(tempString, Tchar(tempTime));
    TextOut(hDC, rcTemp.left, rcTemp.bottom, tempString, strlen(tempString));
    MoveToEx(hDC, rcTemp.left, rcTemp.bottom, (LPPOINT)NULL);
    LineTo(hDC, rcTemp.left, rcTemp.bottom + 4);
    if(tempTime != m_earliestTime)
    {
      MoveToEx(hDC, rcTemp.left, rcTemp.bottom, (LPPOINT)NULL);
      LineTo(hDC, rcTemp.left, rcTemp.top);
    }
    m_TIMEPERIOD.time[m_TIMEPERIOD.numTimePeriods] = tempTime;
    m_TIMEPERIOD.xPos[m_TIMEPERIOD.numTimePeriods] = rcTemp.left;
    m_TIMEPERIOD.numTimePeriods++;
    tempTime += 3600;
    rcTemp.left += timeInc;
  }
  SelectObject(hDC, m_hFontOld);
  DeleteObject(m_hFont);
  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(Printing? 4 : 8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
  strcpy(lf.lfFaceName, "Verdana");
  lf.lfEscapement = 450;
  m_hFont = CreateFontIndirect(&lf);
  m_hFontOld = (HFONT)dc.SelectObject(m_hFont);
//
//  Write out the x-axis labels on the top
//
  tempTime = m_earliestTime;
  memcpy(&rcTemp, &rc, sizeof(RECT));
  rcTemp.top += indent;
  rcTemp.left += m_CharWidth * 5;
  while(tempTime <= m_latestTime)
  {
    strcpy(tempString, Tchar(tempTime));
    TextOut(hDC, rcTemp.left - 10, rcTemp.top - 10, tempString, strlen(tempString));
    MoveToEx(hDC, rcTemp.left, rcTemp.top, (LPPOINT)NULL);
    LineTo(hDC, rcTemp.left, rcTemp.top - 4);
    tempTime += 3600;
    rcTemp.left += timeInc;
  }
  SelectObject(hDC, m_hFontOld);
  DeleteObject(m_hFont);
  SelectObject(hDC, hPenOld);
  DeleteObject(hPen);
//
//  Go through all the trips
//
  int start, end;

  memcpy(&rcTemp, &rc, sizeof(RECT));
  rcTemp.left += m_CharWidth * 5;
  for(nI = 0; nI < 2; nI++)
  {
    hPen = CreatePen(PS_SOLID, 1, nI == 0 ? RGB(255, 0, 0) : RGB(0, 0, 255));
    hPenDot = CreatePen(PS_DOT, 1, nI == 0 ? RGB(255, 0, 0) : RGB(0, 0, 255));
    hPenOld = (HPEN)dc.SelectObject(hPen);
    for(nJ = 0; nJ < m_numTrips[nI]; nJ++)
    {
      nK = m_TRIPGRAPH[nI][nJ].patternIndex;
      if(!(m_PATTERNDATA[nI][nK].flags & TRIPGRAPH_FLAG_PATTERNACTIVE))
        continue;
      if(nI == 0)
      {
        start = 0;
        for(nL = 0; nL < m_PATTERNDATA[nI][nK].numSeqences - 1; nL++)
        {
          while(start < m_posMax)
          {
            if(m_PATTERNDATA[nI][nK].patternSequence[nL] == m_NODEDATA.sequence[nI][start])
              break;
            start++;
          }
          end = start + 1;
          while(end <= m_posMax)
          {
            if(m_PATTERNDATA[nI][nK].patternSequence[nL + 1] == m_NODEDATA.sequence[nI][end])
              break;
            end++;
          }
//          if(end - start == 1)
            dc.SelectObject(hPen);
//          else
//            dc.SelectObject(hPenDot);

          timeAtNode = m_TRIPGRAPH[nI][nJ].tripTimes[nL];
          xPos1 = (int)(rcTemp.left + (((float)(timeAtNode - m_earliestTime) / 3600) * timeInc));

          timeAtNode = m_TRIPGRAPH[nI][nJ].tripTimes[nL + 1];
          xPos2 = (int)(rcTemp.left + (((float)(timeAtNode - m_earliestTime) / 3600) * timeInc));

          MoveToEx(hDC, xPos1, m_NODEDATA.yPos[start], (LPPOINT)NULL);
          LineTo(hDC, xPos2, m_NODEDATA.yPos[end]);
          
          start = end;
          end = start + 1;
        }
      }
      else
      {
        start = m_posMax;
        for(nL = 0; nL < m_PATTERNDATA[nI][nK].numSeqences - 1; nL++)
        {
          while(start > 0)
          {
            if(m_PATTERNDATA[nI][nK].patternSequence[nL] == m_NODEDATA.sequence[nI][start])
              break;
            start--;
          }
          end = start - 1;
          while(end >= 0)
          {
            if(m_PATTERNDATA[nI][nK].patternSequence[nL + 1] == m_NODEDATA.sequence[nI][end])
              break;
            end--;
          }
//          if(start - end == 1)
            dc.SelectObject(hPen);
//          else
//            dc.SelectObject(hPenDot);

          timeAtNode = m_TRIPGRAPH[nI][nJ].tripTimes[nL];
          xPos1 = (int)(rcTemp.left + (((float)(timeAtNode - m_earliestTime) / 3600) * timeInc));

          timeAtNode = m_TRIPGRAPH[nI][nJ].tripTimes[nL + 1];
          xPos2 = (int)(rcTemp.left + (((float)(timeAtNode - m_earliestTime) / 3600) * timeInc));

          MoveToEx(hDC, xPos1, m_NODEDATA.yPos[start], (LPPOINT)NULL);
          LineTo(hDC, xPos2, m_NODEDATA.yPos[end]);
          
          start = end;
          end = start - 1;
        }
      }
    }
    SelectObject(hDC, hPenOld);
    DeleteObject(hPen);
    DeleteObject(hPenDot);
  }
}

void CTripGraph::OnSize(UINT nType, int cx, int cy) 
{
  RECT r;
  GetClientRect(&r);
  if(r.bottom < 200 || r.right < 400)
    return;

  CResizingDialog::OnSize(nType, cx, cy);
	
  GetClientRect(&m_rcOriginal);
  m_rcOriginal.left += 16;
  m_rcOriginal.right -= 80;

  Invalidate(TRUE);	
}

void CTripGraph::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Trip Graph");
}

void CTripGraph::OnOK() 
{
	CDialog::OnOK();
}

void CTripGraph::OnCancel() 
{
	CDialog::OnCancel();
}

void CTripGraph::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Technical_Support);
}

void CTripGraph::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  if(m_ContextMenu != NULL)
    return;
//
//  point comes in in screen coords.  Translate it
//  to dialog coords, which is what it should come
//  in in the first place.  Jeez.
//
  RECT r;

  pWnd->GetWindowRect(&r);

  long dlgX = point.x - r.left;
  long dlgY = point.y - r.top;
//
//  If it's outside the drawing region, don't bother
//
  if(dlgX >= m_rcOriginal.left && dlgX <= m_rcOriginal.right &&
        dlgY >= m_rcOriginal.top && dlgY <= m_rcOriginal.bottom)
  {
    m_ContextMenu.LoadMenu(IDR_TRIPGRAPH_CONTEXT);
    CMenu* pMenu = m_ContextMenu.GetSubMenu(0);
    if(pMenu != NULL)
      pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, pWnd, NULL);
    m_ContextMenu.DestroyMenu();
  }
}

HBRUSH CTripGraph::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  int id = pWnd->GetDlgCtrlID();
  
  if(id == TRIPGRAPH_OBLEGEND)
  {
    SetTextColor(pDC->m_hDC, RGB(255, 0, 0));
  }	
  else if(id == TRIPGRAPH_IBLEGEND)
  {
    SetTextColor(pDC->m_hDC, RGB(0, 0, 255));
  }

	return hbr;
}

void CTripGraph::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CTripGraph::OnLButtonDown(UINT nFlags, CPoint point) 
{
//  point comes in in dialog coords.  This should come as 
//  no surprize, as the right-click for the context menu
//  comes in as screen coords.
//
//  If it's outside the drawing region, don't bother
//
  if(point.x >= m_rcOriginal.left && point.x <= m_rcOriginal.right &&
        point.y >= m_rcOriginal.top && point.y <= m_rcOriginal.bottom)
  {
    m_LButtonPoint = point;
  }
  else
  {
    m_LButtonPoint.x = NO_RECORD;
    m_LButtonPoint.y = NO_RECORD;
  }

	CDialog::OnLButtonDown(nFlags, point);
}

void CTripGraph::OnLButtonUp(UINT nFlags, CPoint point) 
{
  int nI;
  int index;
  int dl;

//
//  Were we in the region?
//
  if(m_LButtonPoint.x != NO_RECORD)
	{
//
//  Check to see that the mouse hasn't strayed too far
//
    if(abs(m_LButtonPoint.x - point.x) <= 4 &&
          abs(m_LButtonPoint.y - point.y) <= 4)
    {
      BOOL bFound;
//
//  Look for the position
//

      m_bMoving = SnapToTrip();

    }
  }
	
	CDialog::OnLButtonUp(nFlags, point);
}


BOOL CTripGraph::SnapToTrip()
{
  long distance;
  long closest = 32767;
  int  nI;
  int  position = NO_RECORD;
//
//  Find the closest node
//
  for(nI = 0; nI < m_posMax; nI++)
  {
    distance = abs(m_LButtonPoint.y - m_NODEDATA.yPos[nI]);
    if(distance < closest)
    {
      position = nI;
      closest = distance;
    }
  }
  if(position == NO_RECORD)
    return(FALSE);

  NODESKey0.recordID = m_NODEDATA.recordIDs[position];
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(szarString, NODES_ABBRNAME_LENGTH);
  sprintf(tempString, "Closest is %s at ", szarString);
//
//  Find the time of day
//
  closest = 32767;
  position = NO_RECORD;
  for(nI = 0; nI < m_TIMEPERIOD.numTimePeriods; nI++)
  {
    distance = abs(m_LButtonPoint.x - m_TIMEPERIOD.xPos[nI]);
    if(distance < closest)
    {
      position = nI;
      closest = distance;
    }
  }
  if(position == NO_RECORD)
    return(FALSE);

  long timeOfDay = m_TIMEPERIOD.time[0] +
        (long)(((float)(m_LButtonPoint.x - m_TIMEPERIOD.xPos[0]) / m_TIMEPERIOD.interval) * 3600);

  strcat(tempString, Tchar(timeOfDay));
  pStaticSTATUS->SetWindowText(tempString);

  return(TRUE);
}

void CTripGraph::OnSelectpatterns() 
{
  int nI, nJ;

  m_PATSEL.ROUTESrecordID = m_pDI->fileInfo.routeRecordID;
  m_PATSEL.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
  for(nI = 0; nI < 2; nI++)
  {
    m_PATSEL.numPatterns[nI] = m_numPatterns[nI];
    for(nJ = 0; nJ < m_numPatterns[nI]; nJ++)
    {
      m_PATSEL.flags[nI][nJ] = m_PATTERNDATA[nI][nJ].flags;
      m_PATSEL.PATTERNNAMESrecordID[nI][nJ] = m_PATTERNDATA[nI][nJ].PATTERNNAMESrecordID;     
    }
  }
  
  CSelectPatterns dlg(this, &m_PATSEL);
  if(dlg.DoModal() == IDOK)
  {
    for(nI = 0; nI < 2; nI++)
    {
      m_PATSEL.numPatterns[nI] = m_numPatterns[nI];
      for(nJ = 0; nJ < m_numPatterns[nI]; nJ++)
      {
        m_PATTERNDATA[nI][nJ].flags = m_PATSEL.flags[nI][nJ];
      }
    }
    Invalidate(TRUE);
  }
}

void CTripGraph::OnEdittrip() 
{
	// TODO: Add your command handler code here
	
}

void CTripGraph::OnReleasetrip() 
{
  int nI, nJ;

  m_bMoving = FALSE;

  for(nI = 0; nI < 2; nI++)
  {
    for(nJ = 0; nJ < m_numTrips[nI]; nJ++)
    {
      m_TRIPGRAPH[nI][nJ].flags = 0;
    }
  }

  Invalidate(TRUE);
}
