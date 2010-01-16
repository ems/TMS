//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_JUMPTORUN_H__3352D0D8_CBA6_493B_8809_1E13EE662C2A__INCLUDED_)
#define AFX_JUMPTORUN_H__3352D0D8_CBA6_493B_8809_1E13EE662C2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JumpToRun.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJumpToRun dialog

class CJumpToRun : public CDialog
{
// Construction
public:
	CJumpToRun(CWnd* pParent = NULL, CUIntArray *pRunNumbers = NULL, long *pRun = NULL); // standard constructor

// Dialog Data
	//{{AFX_DATA(CJumpToRun)
	enum { IDD = IDD_JUMPTORUN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJumpToRun)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  
  CUIntArray *m_pRunNumbers;
  long *m_pRun;

  CListCtrl *pListCtrlLIST;

	// Generated message map functions
	//{{AFX_MSG(CJumpToRun)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JUMPTORUN_H__3352D0D8_CBA6_493B_8809_1E13EE662C2A__INCLUDED_)
