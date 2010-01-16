#if !defined(AFX_SUSPENSIONPENDING_H__15F20D99_6B82_4C12_AE25_4A45A5600622__INCLUDED_)
#define AFX_SUSPENSIONPENDING_H__15F20D99_6B82_4C12_AE25_4A45A5600622__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SuspensionPending.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSuspensionPending dialog

class CSuspensionPending : public CDialog
{
// Construction
public:
	CSuspensionPending(CWnd* pParent = NULL, BOOL bSuspensionPending = TRUE, long numRecordIDs = 0, long* pRecordIDs = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSuspensionPending)
	enum { IDD = IDD_SUSPENSIONPENDING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuspensionPending)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListCtrl* pListCtrlLIST;

  long  m_numRecordIDs;
  long* m_pRecordIDs;
  BOOL  m_bSuspensionPending;

	// Generated message map functions
	//{{AFX_MSG(CSuspensionPending)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	afx_msg void OnPrint();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUSPENSIONPENDING_H__15F20D99_6B82_4C12_AE25_4A45A5600622__INCLUDED_)
