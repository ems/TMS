//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_REASSIGNRUNTYPE_H__32BA8D5C_FBA8_45B6_99FA_759DB1AC2616__INCLUDED_)
#define AFX_REASSIGNRUNTYPE_H__32BA8D5C_FBA8_45B6_99FA_759DB1AC2616__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReassignRuntype.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReassignRuntype dialog

class CReassignRuntype : public CDialog
{
// Construction
public:
	CReassignRuntype(CWnd* pParent = NULL, long runNumber = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReassignRuntype)
	enum { IDD = IDD_REASSIGNRUNTYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReassignRuntype)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CStatic   *pStaticCURRENT;
  CComboBox *pComboBoxRUNTYPES;
  long m_RunNumber;
  long m_CurrentRuntype;
  int  m_RUNLISTDATAIndex;


	// Generated message map functions
	//{{AFX_MSG(CReassignRuntype)
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

#endif // !defined(AFX_REASSIGNRUNTYPE_H__32BA8D5C_FBA8_45B6_99FA_759DB1AC2616__INCLUDED_)
