//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDIRECTION_H__55B55981_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
#define AFX_ADDDIRECTION_H__55B55981_0B69_11D6_A235_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDirection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDirection dialog

class CAddDirection : public CDialog
{
// Construction
public:
	CAddDirection(CWnd* pParent = NULL, long *pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDirection)
	enum { IDD = IDD_ADDDIRECTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDirection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long *m_pUpdateRecordID;
  BOOL m_bInit;

  CEdit*     pEditABBRNAME;
  CEdit*     pEditLONGNAME;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;


	// Generated message map functions
	//{{AFX_MSG(CAddDirection)
	afx_msg void OnCc();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnNa();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDIRECTION_H__55B55981_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
