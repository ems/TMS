//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#include "stdafx.h"
#include <afxole.h>
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include <dos.h>
#include <direct.h>
#include "BitmapDialog.h"
#include <version.h>
#include "cshuser.h"
#include "mapinfow.h"   // ADDED FOR INTEGRATED MAPPING SUPPORT

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here

}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);

  long displayIndex = GetWindowLong(this->m_hWnd, GWL_USERDATA);
  TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

  if(pTD && pTD->F1)
  {
    pTD->F1.MoveWindow(0, 0, cx, cy, TRUE);
  }
}


void CChildFrame::OnClose() 
{
  long displayIndex = GetWindowLong(this->m_hWnd, GWL_USERDATA);

  m_TableDisplay[displayIndex].flags = 0;
  m_TableDisplay[displayIndex].pane = NULL;
	
	CMDIChildWnd::OnClose();
}
