#if !defined(AFX_SELECTPATTERNS_H__3CDF66F8_0D14_4E97_B0B2_3EF410067FE6__INCLUDED_)
#define AFX_SELECTPATTERNS_H__3CDF66F8_0D14_4E97_B0B2_3EF410067FE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectPatterns.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectPatterns dialog

class CSelectPatterns : public CDialog
{
// Construction
public:
	CSelectPatterns(CWnd* pParent = NULL, PATSELDef* pPATSEL = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectPatterns)
	enum { IDD = IDD_SELECTPATTERNS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPatterns)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  PATSELDef* m_pPATSEL;
  CListCtrl* pListCtrlOUTBOUND;
  CListCtrl* pListCtrlINBOUND;

	// Generated message map functions
	//{{AFX_MSG(CSelectPatterns)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPATTERNS_H__3CDF66F8_0D14_4E97_B0B2_3EF410067FE6__INCLUDED_)
