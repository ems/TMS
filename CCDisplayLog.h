//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCDISPLAYLOG_H__69AF6C9D_1143_4359_AE33_33100CA64CF6__INCLUDED_)
#define AFX_CCDISPLAYLOG_H__69AF6C9D_1143_4359_AE33_33100CA64CF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCDisplayLog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCDisplayLog dialog

class CCCDisplayLog : public CDialog
{
// Construction
public:
//	CCCDisplayLog(CWnd* pParent = NULL);   // standard constructor
  CCCDisplayLog(CWnd* pParent = NULL, CCFEDef* pCCFE = NULL);

// Dialog Data
	//{{AFX_DATA(CCCDisplayLog)
	enum { IDD = IDD_CCDISPLAYLOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCDisplayLog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CCFEDef*   m_pCCFE;
  char       m_previousLog[COMPLAINTS_VARIABLE_LENGTH];
  CEdit*     pEditCCDPREVIOUSLOG;
  CButton*   pButtonPRINTLOG;
  CButton*   pButtonEMAIL;
  CListCtrl* pListCtrlLIST;
  
  void PrintLog();

	// Generated message map functions
	//{{AFX_MSG(CCCDisplayLog)
	afx_msg void OnEmail();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCDISPLAYLOG_H__69AF6C9D_1143_4359_AE33_33100CA64CF6__INCLUDED_)
