//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDIVISION_H__55B55985_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
#define AFX_ADDDIVISION_H__55B55985_0B69_11D6_A235_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDivision.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDivision dialog

class CAddDivision : public CDialog
{
// Construction
public:
	CAddDivision(CWnd* pParent = NULL, long *pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDivision)
	enum { IDD = IDD_ADDDIVISION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDivision)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long *m_pUpdateRecordID;
  BOOL m_bInit;

  CEdit*     pEditNUMBER;
  CEdit*     pEditNAME;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;



	// Generated message map functions
	//{{AFX_MSG(CAddDivision)
	afx_msg void OnCc();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnNa();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDIVISION_H__55B55985_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
