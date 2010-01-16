//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDBUSTYPE_H__4191E1DF_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
#define AFX_ADDBUSTYPE_H__4191E1DF_0926_11D6_A22D_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddBustype.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddBustype dialog

class CAddBustype : public CDialog
{
// Construction
public:
	CAddBustype(CWnd* pParent = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddBustype)
	enum { IDD = IDD_ADDBUSTYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddBustype)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long*      m_pUpdateRecordID;
  BOOL       m_bInit;

  CEdit*     pEditNUMBER;
  CEdit*     pEditFLEETNAME;
  CEdit*     pEditNAME;
  CButton*   pButtonACCESSIBLE;
  CButton*   pButtonCARRIER;
  CButton*   pButtonSURFACE;
  CButton*   pButtonRAIL;
  CButton*   pButtonCAPACITY;
  CEdit*     pEditSEAT;
  CEdit*     pEditSTAND;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;

	// Generated message map functions
	//{{AFX_MSG(CAddBustype)
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnChangeSeat();
	afx_msg void OnChangeStand();
	virtual void OnOK();
	afx_msg void OnHelp();
	virtual void OnCancel();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDBUSTYPE_H__4191E1DF_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
