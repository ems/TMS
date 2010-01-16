//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVER03_H__AA6B26F9_8610_47C8_8C39_6624FBDE3A74__INCLUDED_)
#define AFX_ADDDRIVER03_H__AA6B26F9_8610_47C8_8C39_6624FBDE3A74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriver03.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriver03 dialog

class CAddDriver03 : public CPropertyPage
{
	DECLARE_DYNCREATE(CAddDriver03)

// Construction
public:
	CAddDriver03();
	~CAddDriver03();

// Dialog Data
	//{{AFX_DATA(CAddDriver03)
	enum { IDD = IDD_ADDDRIVER03 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAddDriver03)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAddDriver03)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVER03_H__AA6B26F9_8610_47C8_8C39_6624FBDE3A74__INCLUDED_)
