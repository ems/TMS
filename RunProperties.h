//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_RUNPROPERTIES_H__F75E925B_70BD_433A_9052_38ABB36D9364__INCLUDED_)
#define AFX_RUNPROPERTIES_H__F75E925B_70BD_433A_9052_38ABB36D9364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RunProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRunProperties dialog

class CRunProperties : public CDialog
{
// Construction
public:
	CRunProperties(CWnd* pParent = NULL, long runNumber = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRunProperties)
	enum { IDD = IDD_RUNPROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  long m_RunNumber;
  COSTDef m_COST;
  CStatic *pStaticREASON;
  CButton *pButtonSHOWDIRECTIONS;

	// Generated message map functions
	//{{AFX_MSG(CRunProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnShowdirections();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RUNPROPERTIES_H__F75E925B_70BD_433A_9052_38ABB36D9364__INCLUDED_)
