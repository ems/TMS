//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#if !defined(AFX_CUSTOMCONTROLS_H__)
#define AFX_CUSTOMCONTROLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MainFrm.h"

class CControlButtonLine : public CXTPControlButton
{
	DECLARE_XTP_CONTROL(CControlButtonLine)
public:
	CControlButtonLine(double dWidth = 0);
protected:
	virtual CSize GetSize(CDC* pDC);
	virtual void Draw(CDC* pDC);
	virtual void Copy(CXTPControl* pControl, BOOL bRecursive = FALSE);
	virtual void DoPropExchange(CXTPPropExchange* pPX);

private:
	double m_dWidth;
};

class CControlButtonStyle : public CXTPControlButton
{
	DECLARE_XTP_CONTROL(CControlButtonStyle)
public:
	CControlButtonStyle(int nStyle = 0);
protected:
	virtual CSize GetSize(CDC* pDC);
	virtual void Draw(CDC* pDC);
	virtual void Copy(CXTPControl* pControl, BOOL bRecursive = FALSE);
	virtual void DoPropExchange(CXTPPropExchange* pPX);

private:
	int m_nStyle;
};


class CControlButtonArrow : public CXTPControlButton
{
	DECLARE_XTP_CONTROL(CControlButtonArrow)
public:
	CControlButtonArrow(int nStyle = 0);
protected:
	virtual CSize GetSize(CDC* pDC);
	virtual void Draw(CDC* pDC);
	virtual void Copy(CXTPControl* pControl, BOOL bRecursive = FALSE);
	virtual void DoPropExchange(CXTPPropExchange* pPX);

private:
	int m_nStyle;
};

class CControlComboBoxEditEx : public CXTMaskEditT<CXTPControlComboBoxEditCtrl>
{
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

class CControlComboBoxEx: public CXTPControlComboBox
{
	DECLARE_XTP_CONTROL(CControlComboBoxEx)
public:
	CXTPControlComboBoxEditCtrl* CreateEditControl()
	{
		return new CControlComboBoxEditEx();
	}
};


class CControlComboBoxPopup: public CXTPControlComboBox
{
	DECLARE_XTP_CONTROL(CControlComboBoxPopup)
public:
	CString GetListBoxText() const;
};


class CControlComboBoxCustomDrawList : public CXTPControlFontComboBoxList
{
	DECLARE_XTP_COMMANDBAR(CControlComboBoxCustomDrawList);
	DECLARE_MESSAGE_MAP()
public:

	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct );

	virtual void CreateListBox()
	{
		CWnd* pParentWnd = CMainFrame::m_pInstance;

		CreateEx(WS_EX_STATICEDGE|WS_EX_TOOLWINDOW, _T("LISTBOX"), _T(""),
			WS_CHILD|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN|LBS_SORT|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_NOTIFY,
			CRect(0,0,0,0), pParentWnd,  0);

		SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, 0 );
		ModifyStyle(WS_CHILD, WS_POPUP);
	#ifdef _WIN64
		SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)(pParentWnd->m_hWnd));
	#else
		SetWindowLong(m_hWnd, GWLP_HWNDPARENT, (LONG)(LONG_PTR)(pParentWnd->m_hWnd));
	#endif
	}

	friend class CControlComboBoxCustomDraw;
};

class CControlComboBoxCustomDraw: public CXTPControlFontComboBox
{
	DECLARE_XTP_CONTROL(CControlComboBoxCustomDraw)
public:

	CControlComboBoxCustomDraw()
	{
		if (m_pCommandBar)
		{
			m_pCommandBar->InternalRelease();
		}

		m_pCommandBar = new CControlComboBoxCustomDrawList();
		((CControlComboBoxCustomDrawList*)m_pCommandBar)->CreateListBox();

		((CControlComboBoxCustomDrawList*)m_pCommandBar)->EnumFontFamiliesEx(1);

	}
};


#endif //AFX_CUSTOMCONTROLS_H__
