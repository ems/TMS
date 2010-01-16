/*
 Written by Steve Bryndin (fishbed@tezcat.com, steveb@gvsi.com).

 This code may be used in compiled form in any way you wish. This
 file may be redistributed unmodified by any means PROVIDING it is 
 not sold for profit without the authors written consent, and 
 providing that this notice and the authors name is included. 
 An email letting me know that you are using it would be 
 nice as well. 

 This software is provided "as is" without express or implied warranty. 
 Use it at you own risk! The author accepts no liability for any damages 
 to your computer or data these products may cause.
*/


// InfoBar.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "TMS.h"
#include "resource.h"
#include "InfoBar.h"

#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CStaticCmdUI : public CCmdUI      
{
public: 
	virtual void SetText(LPCTSTR lpszText);
};

void CStaticCmdUI::SetText(LPCTSTR lpszText)
{
	CInfoBar* pInfoBar = (CInfoBar*)m_pOther;
	ASSERT(pInfoBar != NULL);

	pInfoBar->SetText(lpszText);
}


/////////////////////////////////////////////////////////////////////////////
// CInfoBar

CInfoBar::CInfoBar()
{
	m_cyBottomBorder = m_cyTopBorder = 4;	// l&r default to 6; t&b was 1
	m_caption = "Database closed";					// must have some text for CalcFixedLayout
	m_cxAvailable = 0;
	m_crBackgroundColor = ::GetSysColor(COLOR_3DSHADOW); //Gray
	m_crTextColor = RGB(255,255,255); // White
}

CInfoBar::~CInfoBar()
{
}


BEGIN_MESSAGE_MAP(CInfoBar, CControlBar)
	//{{AFX_MSG_MAP(CInfoBar)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CInfoBar message handlers

int CInfoBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	// Comment out this next line for a different 3D effect
	ModifyStyleEx(0, WS_EX_CLIENTEDGE /*WS_EX_STATICEDGE*/);
//	ModifyStyle(0, WS_DLGFRAME);
//	ModifyStyle(0, WS_THICKFRAME);
	// Create a font twice as tall as the system status bar font
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(metrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0);

	metrics.lfStatusFont.lfHeight *= 2;
	VERIFY(m_font.CreateFontIndirect(&metrics.lfStatusFont));
	return 0;
}

BOOL CInfoBar::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(rect);
	pDC->FillSolidRect(rect, m_crBackgroundColor);

	return CControlBar::OnEraseBkgnd(pDC);
}

void CInfoBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect r;
	GetClientRect(r);
	CalcInsideRect(r, TRUE);
	r.InflateRect(0, 2);

	CFont* pOldFont = dc.SelectObject(&m_font);

	dc.SetTextColor(m_crTextColor);
	dc.DrawText(m_caption, -1, r, DT_EDITCONTROL |
		DT_EXTERNALLEADING | DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	dc.DrawState(CPoint((r.right - m_sizeBitmap.cx), 
		         (r.bottom - m_sizeBitmap.cy)/2), 
				 m_sizeBitmap, &m_bm, DST_BITMAP);

	if (pOldFont != NULL)
		dc.SelectObject(pOldFont);
}

LRESULT CInfoBar::OnSizeParent(WPARAM, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;

	// keep track of the available width for use by CalcFixedLayout later	
	m_cxAvailable = lpLayout->rect.right - lpLayout->rect.left;

	return CControlBar::OnSizeParent(0, lParam);
}

// Overridden virtual helper methods

void CInfoBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CStaticCmdUI state;
	state.m_pOther = this;
	state.m_nIndexMax = 1;		// there's only one thing to update
	state.m_nID = AFX_IDW_STATUS_BAR;

	// allow the statusbar itself to have update handlers
	if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
		return;

	// allow target (owner) to handle the remaining updates
	state.DoUpdate(pTarget, FALSE);
}

CSize CInfoBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	// Get border size (values will be negative)
	CRect rBorder; rBorder.SetRectEmpty();
	CalcInsideRect(rBorder, bHorz);

	// Based on the available width minus the border area,
	// calculate the necessary Y value to hold the text
	CRect rCalc(0, 0, m_cxAvailable - (-rBorder.Width()), 0);
	{
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(&m_font);

		dc.DrawText(m_caption, -1, rCalc, DT_CALCRECT | DT_EDITCONTROL |
			DT_EXTERNALLEADING | DT_LEFT | DT_WORDBREAK);

		if (pOldFont != NULL)
			dc.SelectObject(pOldFont);
	}

	// The Y value is the sum of the calculated height from DrawText,
	// plus the top and bottom border.
	CSize size;
	size.cx = 32767;
	size.cy = rCalc.Height();
	size.cy += (-rBorder.Height());

	return size;
}

void CInfoBar::SetText(LPCTSTR lpszNew)
{
	ASSERT(lpszNew && AfxIsValidString(lpszNew));

	if (m_caption != lpszNew)
	{
		m_caption = lpszNew;
		Invalidate();
	}
}


BOOL CInfoBar::SetBitmap(UINT nResID)
{
	BOOL bRet = FALSE;

  if(nResID == NO_RECORD)
  {
    m_sizeBitmap.cx = 0;
    m_sizeBitmap.cy = 0;

    return TRUE;
  }
  else
  {
  	BITMAP	bitmap;	
	  CSize bitSize;

  	m_bm.DeleteObject();
	  bRet = m_bm.LoadBitmap(nResID);
  	if(bRet)
	  {
		  m_bm.GetBitmap(&bitmap);
  		m_sizeBitmap.cx = bitmap.bmWidth;
	  	m_sizeBitmap.cy = bitmap.bmHeight;
  	}
	  Invalidate();
  	return bRet;
  }
}

BOOL CInfoBar::SetTextFont(LPCTSTR lpFontName)
{
	BOOL bRet;
	m_font.DeleteObject();
	
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(metrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0);

	metrics.lfStatusFont.lfHeight *= 2;
	strcpy(metrics.lfStatusFont.lfFaceName, lpFontName);
	bRet = m_font.CreateFontIndirect(&metrics.lfStatusFont);
	Invalidate();
	return bRet;
}

void CInfoBar::SetBackgroundColor(COLORREF crNew)
{
	m_crBackgroundColor = crNew;
	Invalidate();
}

void CInfoBar::SetTextColor(COLORREF crNew)
{
	m_crTextColor = crNew;
	Invalidate();
}

