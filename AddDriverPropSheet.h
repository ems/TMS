//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVERPROPSHEET_H__94FD10CB_2D9A_43B4_8628_6D3F9C624B70__INCLUDED_)
#define AFX_ADDDRIVERPROPSHEET_H__94FD10CB_2D9A_43B4_8628_6D3F9C624B70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriverPropSheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriverPropSheet

class CAddDriverPropSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CAddDriverPropSheet)

// Construction
public:
	CAddDriverPropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CAddDriverPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDriverPropSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAddDriverPropSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAddDriverPropSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVERPROPSHEET_H__94FD10CB_2D9A_43B4_8628_6D3F9C624B70__INCLUDED_)
