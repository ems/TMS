//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVERTYPE_H__55B55986_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
#define AFX_ADDDRIVERTYPE_H__55B55986_0B69_11D6_A235_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriverType.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriverType dialog

class CAddDriverType : public CDialog
{
// Construction
public:
	CAddDriverType(CWnd* pParent = NULL, long *pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDriverType)
	enum { IDD = IDD_ADDDRIVERTYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDriverType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long *m_pUpdateRecordID;
  BOOL m_bInit;

  CEdit*     pEditNUMBER;
  CEdit*     pEditNAME;
  CButton*   pButtonREGULARBOX;
  CEdit*     pEditREGULAR;
  CButton*   pButtonOVERTIMEBOX;
  CEdit*     pEditOVERTIME;
  CButton*   pButtonSPREADPREMIUMBOX;
  CEdit*     pEditSPREADPREMIUM;
  CButton*   pButtonMAKEUPTIMEBOX;
  CEdit*     pEditMAKEUPTIME;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;


	// Generated message map functions
	//{{AFX_MSG(CAddDriverType)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnSelendokCommentcode();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnRegularbox();
	afx_msg void OnOvertimebox();
	afx_msg void OnSpreadpremiumbox();
	afx_msg void OnMakeuptimebox();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVERTYPE_H__55B55986_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
