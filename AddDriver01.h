//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVER01_H__1587DAC1_8B05_4293_9D7E_8AF48D1223F1__INCLUDED_)
#define AFX_ADDDRIVER01_H__1587DAC1_8B05_4293_9D7E_8AF48D1223F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriver01.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriver01 dialog

class CAddDriver01 : public CPropertyPage
{
	DECLARE_DYNCREATE(CAddDriver01)

// Construction
public:
	CAddDriver01();
	~CAddDriver01();

// Dialog Data
	//{{AFX_DATA(CAddDriver01)
	enum { IDD = IDD_ADDDRIVER01 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAddDriver01)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CBitmapCtrl m_bmpCtrl;

	// Generated message map functions
	//{{AFX_MSG(CAddDriver01)
	virtual BOOL OnInitDialog();
	afx_msg void OnPhoto();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVER01_H__1587DAC1_8B05_4293_9D7E_8AF48D1223F1__INCLUDED_)
