//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
#pragma once

using namespace XTPPaintThemes;

class CCustomMacTheme : public CXTPDefaultTheme
{

protected:
	void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	void FillDockBar(CDC* pDC, CXTPDockBar* pBar);
	void FillStatusBar(CDC* pDC, CXTPStatusBar* pBar);

private:
	void _FillRect(CDC* pDC, CRect rc, int nTop);

};


class CCustomGrayTheme : public CXTPDefaultTheme
{

protected:
	void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	void FillDockBar(CDC* pDC, CXTPDockBar* pBar);
	void FillStatusBar(CDC* pDC, CXTPStatusBar* pBar);
	CSize DrawCommandBarGripper(CDC* pDC, CXTPCommandBar* pBar, BOOL bDraw = TRUE);
	void DrawRectangle(CDC* pDC, CRect rc, BOOL bSelected, BOOL bPressed, BOOL bEnabled = TRUE, BOOL bChecked = FALSE, BOOL bPopuped = FALSE, BOOL bToolBar = TRUE, XTPBarPosition barPosition = xtpBarPopup);
};





class CCustomGradientTheme : public CXTPDefaultTheme
{
protected:
	virtual void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	virtual void DrawRectangle(CDC* pDC, CRect rc, BOOL bSelected, BOOL bPressed, BOOL bEnabled = TRUE, BOOL bChecked = FALSE, BOOL bPopuped = FALSE, BOOL bToolBar = TRUE, XTPBarPosition barPosition = xtpBarPopup);
	virtual void AdjustExcludeRect(CRect& rc, CXTPControl* pControl, BOOL bVertical);
};

class CCustomIcyTheme : public CXTPOfficeTheme
{
protected:
	virtual void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	void DrawPopupBarGripper(CDC* pDC, int x, int y, int cx, int cy, BOOL bExpanded = FALSE);
	CSize DrawCommandBarGripper(CDC* pDC, CXTPCommandBar* pBar, BOOL bDraw = TRUE);
	CSize DrawCommandBarSeparator (CDC* pDC, CXTPCommandBar* pBar, CXTPControl* pControl, BOOL bDraw = TRUE);
};




class CCustomFancyTheme : public CXTPOfficeTheme
{
public:
	CCustomFancyTheme();
protected:
	virtual void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	void FillDockBar(CDC* pDC, CXTPDockBar* pBar);
	void RefreshMetrics();

};


class CCustomBevelTheme : public CXTPDefaultTheme
{
public:
	CCustomBevelTheme();

protected:
	virtual void DrawRectangle(CDC* pDC, CRect rc, BOOL bSelected, BOOL bPressed, BOOL bEnabled = TRUE, BOOL bChecked = FALSE, BOOL bPopuped = FALSE, BOOL bToolBar = TRUE, XTPBarPosition barPosition = xtpBarPopup);
	virtual void RefreshMetrics();
	virtual void FillCommandBarEntry(CDC* pDC, CXTPCommandBar* pBar);
	void AdjustExcludeRect(CRect& rc, CXTPControl* pControl, BOOL bVertical);

private:
	void DrawUnpressedPushButton(CDC *pDC, const CRect &rect);
	void DrawCornerShadows(CDC *pDC, const CRect &rect);
	void GetColors();
	void DeletePens();
	void CreatePens();

private:
	COLORREF m_crHilight;
	COLORREF m_crLiteFace;
	COLORREF m_crFace;
	COLORREF m_crLiteShadow;
	COLORREF m_crShadow;
	COLORREF m_crDkShadow;
	COLORREF m_crDarkDkShadow;

	CPen m_penShadow;
	CPen m_penHilight;
	CPen m_penLiteShadow;
};
