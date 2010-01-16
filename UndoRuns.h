#if !defined(AFX_UNDORUNS_H__0270E263_BB69_44A9_99CE_A99B5DD2FDC5__INCLUDED_)
#define AFX_UNDORUNS_H__0270E263_BB69_44A9_99CE_A99B5DD2FDC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UndoRuns.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUndoRuns dialog

class CUndoRuns : public CDialog
{
// Construction
public:
	CUndoRuns(CWnd* pParent = NULL,  CUIntArray *pRunNumbers = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUndoRuns)
	enum { IDD = IDD_UNDORUNS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUndoRuns)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  
  CUIntArray *m_pRunNumbers;

  CListCtrl *pListCtrlLIST;


	// Generated message map functions
	//{{AFX_MSG(CUndoRuns)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelectall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNDORUNS_H__0270E263_BB69_44A9_99CE_A99B5DD2FDC5__INCLUDED_)
