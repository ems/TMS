#if !defined(AFX_ROSTERWEEK_H__2483B572_0598_478C_A8B7_FE362A959B33__INCLUDED_)
#define AFX_ROSTERWEEK_H__2483B572_0598_478C_A8B7_FE362A959B33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RosterWeek.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRosterWeek dialog

class CRosterWeek : public CDialog
{
// Construction
public:
	CRosterWeek(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRosterWeek)
	enum { IDD = IDD_ROSTERWEEK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRosterWeek)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CEdit* pEditWEEK;
	// Generated message map functions
	//{{AFX_MSG(CRosterWeek)
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

#endif // !defined(AFX_ROSTERWEEK_H__2483B572_0598_478C_A8B7_FE362A959B33__INCLUDED_)
