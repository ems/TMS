//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_PLOTLEFT_H__AD0FD75A_44C9_4B1B_AB52_E16CB69E3C81__INCLUDED_)
#define AFX_PLOTLEFT_H__AD0FD75A_44C9_4B1B_AB52_E16CB69E3C81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlotLeft.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CPlotLeft dialog

class CPlotLeft : public CDialog
{
// Construction
public:
	CPlotLeft(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlotLeft)
	enum { IDD = IDD_PLOTLEFT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlotLeft)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CFlatSB* pFlatSBSCROLLBAR;
  CButton* pButtonALLBLOCKS;
  CButton* pButtonWORKREMAINING;
  CWnd*    pGRAPH;
  CPen  m_Pens[LASTPEN];
  RECT  m_rc;
  RECT  m_rcRepaint;
  RECT  m_rcDialog;
  long  m_earliestTime;
  long  m_latestTime;
  long  m_timeInc;
  long  m_firstDisplayed;

	// Generated message map functions
	//{{AFX_MSG(CPlotLeft)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnAllblocks();
	afx_msg void OnWorkremaining();
	afx_msg void OnPaint();
	afx_msg void OnChangeScrollbar();
	afx_msg void OnScrollScrollbar();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLOTLEFT_H__AD0FD75A_44C9_4B1B_AB52_E16CB69E3C81__INCLUDED_)
