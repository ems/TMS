//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDCOMMENT_H__4191E1F2_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
#define AFX_ADDCOMMENT_H__4191E1F2_0926_11D6_A22D_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddComment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddComment dialog

class CAddComment : public CDialog
{
// Construction
public:
	CAddComment(CWnd* pParent = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddComment)
	enum { IDD = IDD_ADDCOMMENT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddComment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long*     m_pUpdateRecordID;
  BOOL      m_bInit;

  CEdit*    pEditCODE;
  CEdit*    pEditTEXT;

	// Generated message map functions
	//{{AFX_MSG(CAddComment)
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

#endif // !defined(AFX_ADDCOMMENT_H__4191E1F2_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
