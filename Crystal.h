//{{AFX_INCLUDES()
#include "crystalreportviewer11.h"
//}}AFX_INCLUDES
#if !defined(AFX_CRYSTAL_H__B666A244_4FD1_40B6_A4BF_30433F308CBE__INCLUDED_)
#define AFX_CRYSTAL_H__B666A244_4FD1_40B6_A4BF_30433F308CBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Crystal.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCrystal dialog

class CCrystal : public CDialog
{
// Construction
public:
	CCrystal(CWnd* pParent = NULL, char *pszReportPath = NULL, char *pszReportName = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCrystal)
	enum { IDD = IDD_CRYSTAL };
	CCrystalReportViewer11	m_Viewer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrystal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  char *m_pszReportPath;
  char *m_pszReportName;
  IApplicationPtr m_Application;
  IReportPtr m_Report;
  VARIANT dummy;

	// Generated message map functions
	//{{AFX_MSG(CCrystal)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRYSTAL_H__B666A244_4FD1_40B6_A4BF_30433F308CBE__INCLUDED_)
