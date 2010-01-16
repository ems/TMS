//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDBUS_H__EAB4207A_7FB6_41CF_80A2_0FCB1C977455__INCLUDED_)
#define AFX_ADDBUS_H__EAB4207A_7FB6_41CF_80A2_0FCB1C977455__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddBus.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddBus dialog

class CAddBus : public CDialog
{
// Construction
public:
	CAddBus(CWnd* pParent = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddBus)
	enum { IDD = IDD_ADDBUS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddBus)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long* m_pUpdateRecordID;
  int   m_numNodes;

  CComboBox* pComboBoxBUSTYPE;
  CEdit*     pEditNUMBER;
  CButton*   pButtonRETIRED;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;

	// Generated message map functions
	//{{AFX_MSG(CAddBus)
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnCc();
	afx_msg void OnNa();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDBUS_H__EAB4207A_7FB6_41CF_80A2_0FCB1C977455__INCLUDED_)
