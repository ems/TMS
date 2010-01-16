//John Melas
//jmelas@cc.ece.ntua.gr

#include "stdafx.h"
#include "BitmapCtrl.h"

CBitmapCtrl::~CBitmapCtrl()
{
	if(memDC.GetSafeHdc() != NULL)
	{
		memDC.SelectObject(pOldBitmap);
		if(pBitmap != NULL)
			delete pBitmap;
	}
}

BEGIN_MESSAGE_MAP(CBitmapCtrl, CWnd)
	//{{AFX_MSG_MAP(CBitmapCtrl)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBitmapCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd) 
{
	scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
	
	// create window
	BOOL ret;
	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	ret = CWnd::CreateEx(WS_EX_CLIENTEDGE,className,NULL,dwStyle, 
		rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(),0);
	// init virtual screen
	GetClientRect(rcClient);
	CClientDC dc(this);
	if(memDC.GetSafeHdc() == NULL)
	{
		memDC.CreateCompatibleDC(&dc);
		if((pBitmap = new CBitmap()) == NULL) return FALSE;
		pBitmap->CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height());
		pOldBitmap = memDC.SelectObject(pBitmap);
		memDC.SelectStockObject(ANSI_VAR_FONT);
		memDC.SetBkMode(TRANSPARENT);
	}
	x_offset = 0;
	y_offset = 0;
	memDC.FillSolidRect(&rcClient,RGB(128,128,128));

	return ret;
}

void CBitmapCtrl::OnPaint() 
{
	CPaintDC dc(this);
	if(memDC.GetSafeHdc() != NULL)
		dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&memDC,0,0,SRCCOPY);
}


void CBitmapCtrl::SetBitmap(HBITMAP hBmp)
{
	//reset offsets
	x_offset = 0;
	y_offset = 0;

	//reset scrollbar
	SCROLLINFO si;
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;
	si.nMax = 0;
	si.nPage = 0;
	SetScrollInfo(SB_HORZ, &si, TRUE);
	SetScrollInfo(SB_VERT, &si, TRUE);
	
	//redraw background
	GetClientRect(&rcClient);
	memDC.FillSolidRect(&rcClient,RGB(128,128,128));
		
	//get bitmap handle
	hBitmap = hBmp;

	if (hBitmap == 0)
	{
		memDC.TextOut(0,0,"File not found.");
		Invalidate();
		return;
	}
	
	//get image size
	CPoint point(x_offset,y_offset);
	BITMAP bmpInfo;
	CBitmap::FromHandle(hBitmap)->GetBitmap(&bmpInfo);
	image_size.cx = bmpInfo.bmWidth;
	image_size.cy = bmpInfo.bmHeight;
	
	//check image size
	BOOL x_fit;
	BOOL y_fit;
	x_fit = (bmpInfo.bmWidth <= rcClient.Width());
	if (!x_fit)
		rcClient.bottom -= scrollbar_width;
	y_fit = (bmpInfo.bmHeight <= rcClient.Height());
	if (!y_fit)
	{
		rcClient.right -= scrollbar_width;
		x_fit = (bmpInfo.bmWidth <= rcClient.Width());
	}
	if (!x_fit)
	{
		// show scrollbar
		ShowScrollBar(SB_HORZ);
	/*CRect scroll_rect;
	GetScrollBarCtrl(SB_HORZ)->GetWindowRect(&scroll_rect);
	char sz[256];
	sprintf(sz,"%d",scroll_rect.left - scroll_rect.right);
	MessageBox(sz);*/
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = bmpInfo.bmWidth-1;
		si.nPage = rcClient.Width();
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	if (!y_fit)
	{
		// show scrollbar
		ShowScrollBar(SB_VERT);
		// update scrollbar
		SCROLLINFO si;
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = bmpInfo.bmHeight-1;
		si.nPage = rcClient.Height();
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	
	DrawBitmap();
}

void CBitmapCtrl::DrawBitmap()
{
	CPoint point(-x_offset,-y_offset);
	memDC.DrawState(point,image_size,hBitmap,DST_BITMAP|DSS_NORMAL);
	Invalidate();
}

void CBitmapCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nSmall = 1;
	int nLarge = 20;

	if(pScrollBar == GetScrollBarCtrl(SB_HORZ))
	{
		if(nSBCode == SB_LEFT || nSBCode == SB_LINELEFT)
			x_offset -= nSmall;
		if(nSBCode == SB_PAGELEFT)
			x_offset -= nLarge;
		if(nSBCode == SB_RIGHT || nSBCode == SB_LINERIGHT)
			x_offset += nSmall;
		if(nSBCode == SB_PAGERIGHT)
			x_offset += nLarge;
		if(nSBCode == SB_THUMBPOSITION)
			x_offset = (int)nPos;
		if(nSBCode == SB_THUMBTRACK)
			x_offset = (int)nPos;
		if (x_offset<0)
			x_offset = 0;
		if (x_offset>image_size.cx - rcClient.Width())
			x_offset = image_size.cx - rcClient.Width();
		SetScrollPos(SB_HORZ, x_offset, TRUE);
		DrawBitmap();
	}
}

void CBitmapCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int nSmall = 1;
	int nLarge = 20;

	if(pScrollBar == GetScrollBarCtrl(SB_VERT))
	{
		if(nSBCode == SB_TOP || nSBCode == SB_LINEUP)
			y_offset -= nSmall;
		if(nSBCode == SB_PAGEUP)
			y_offset -= nLarge;
		if(nSBCode == SB_BOTTOM || nSBCode == SB_LINEDOWN)
			y_offset += nSmall;
		if(nSBCode == SB_PAGEDOWN)
			y_offset += nLarge;
		if(nSBCode == SB_THUMBPOSITION)
			y_offset = (int)nPos;
		if(nSBCode == SB_THUMBTRACK)
			y_offset = (int)nPos;
		if (y_offset<0)
			y_offset = 0;
		if (y_offset>image_size.cy - rcClient.Height())
			y_offset = image_size.cy - rcClient.Height();
		SetScrollPos(SB_VERT, y_offset, TRUE);
		DrawBitmap();
	}
}

void CBitmapCtrl::LoadFile(CString filename)
{
	HBITMAP hBitmap = (HBITMAP) LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	SetBitmap(hBitmap);
}

