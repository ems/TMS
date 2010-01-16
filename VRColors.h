//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_VRCOLORS_H__39628891_A936_4C01_B26E_BE7C683F42BE__INCLUDED_)
#define AFX_VRCOLORS_H__39628891_A936_4C01_B26E_BE7C683F42BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VRColors.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVRColors dialog

class CVRColors : public CDialog
{
// Construction
public:
//	CVRColors(CWnd* pParent = NULL);   // standard constructor
  CVRColors(CWnd* pParent = NULL, VRCOLORSDef* pVRColors = NULL);

// Dialog Data
	//{{AFX_DATA(CVRColors)
	enum { IDD = IDD_VRCOLORS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVRColors)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  VRCOLORSDef* m_pVRColors;
  CColorBtn m_cbCut;
  CColorBtn m_cbIllegal;
  CColorBtn m_cbFrozen;
  CColorBtn m_cbUncut;
	// Generated message map functions
	//{{AFX_MSG(CVRColors)
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VRCOLORS_H__39628891_A936_4C01_B26E_BE7C683F42BE__INCLUDED_)
