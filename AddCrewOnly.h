//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDCREWONLY_H__D13028B6_7A92_48D5_A25F_C4911D99AF2E__INCLUDED_)
#define AFX_ADDCREWONLY_H__D13028B6_7A92_48D5_A25F_C4911D99AF2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddCrewOnly.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddCrewOnly dialog

class CAddCrewOnly : public CDialog
{
// Construction
public:
	CAddCrewOnly(CWnd* pParent = NULL, long* pUpdateRecordID = NULL, PDISPLAYINFO pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddCrewOnly)
	enum { IDD = IDD_ADDCREWONLY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddCrewOnly)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long*     m_pUpdateRecordID;
  PDISPLAYINFO m_pDI;

  CEdit*     pEditPIECENUMBER;
  CEdit*     pEditRUNNUMBER;
  CComboBox* pComboBoxRUNTYPE;
  CComboBox* pComboBoxSTARTNODE;
  CEdit*     pEditSTARTTIME;
  CComboBox* pComboBoxENDNODE;
  CEdit*     pEditENDTIME;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;

	// Generated message map functions
	//{{AFX_MSG(CAddCrewOnly)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePiecenumber();
	afx_msg void OnNa();
	afx_msg void OnCc();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnSelendokCommentcode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDCREWONLY_H__D13028B6_7A92_48D5_A25F_C4911D99AF2E__INCLUDED_)
