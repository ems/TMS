//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//{{AFX_INCLUDES()
#include "msmask.h"
//}}AFX_INCLUDES
#if !defined(AFX_CUSTOMER_H__93483BA6_F0AB_4266_924C_F7B91FA7467F__INCLUDED_)
#define AFX_CUSTOMER_H__93483BA6_F0AB_4266_924C_F7B91FA7467F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Customer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomer dialog

class CCustomer : public CDialog
{
// Construction
public:
//	CCustomer(CWnd* pParent = NULL);   // standard constructor
  CCustomer(CWnd* pParent = NULL, CUSTOMERSDef* pCUSTOMERS = NULL);

// Dialog Data
	//{{AFX_DATA(CCustomer)
	enum { IDD = IDD_CUSTOMER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CUSTOMERSDef* m_pCUSTOMERS;

  CEdit* pEditSALUTATION;
  CEdit* pEditFIRSTNAME;
  CEdit* pEditINITIALS;
  CEdit* pEditLASTNAME;
  CEdit* pEditADDRESS;
  CEdit* pEditCITY;
  CEdit* pEditSTATE;
  CEdit* pEditZIP;
  CEdit* pEditEMAIL;
  CMSMask* pMSMaskDAYPHONE;
  CMSMask* pMSMaskNIGHTPHONE;
  CMSMask* pMSMaskFAX;

	// Generated message map functions
	//{{AFX_MSG(CCustomer)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMER_H__93483BA6_F0AB_4266_924C_F7B91FA7467F__INCLUDED_)
