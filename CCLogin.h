//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCLOGIN_H__CB1B3AFA_BD47_431C_9C0B_597A58D08E13__INCLUDED_)
#define AFX_CCLOGIN_H__CB1B3AFA_BD47_431C_9C0B_597A58D08E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCLogin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCLogin dialog

class CCCLogin : public CDialog
{
// Construction
public:
	CCCLogin(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCLogin)
	enum { IDD = IDD_CCLOGIN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCLogin)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CEdit* pEditNAME;
  CEdit* pEditPASSWORD;

	// Generated message map functions
	//{{AFX_MSG(CCCLogin)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCLOGIN_H__CB1B3AFA_BD47_431C_9C0B_597A58D08E13__INCLUDED_)
