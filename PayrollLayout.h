#if !defined(AFX_PAYROLLLAYOUT_H__E7A5723D_40F0_43F6_93A9_65878F475233__INCLUDED_)
#define AFX_PAYROLLLAYOUT_H__E7A5723D_40F0_43F6_93A9_65878F475233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PayrollLayout.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPayrollLayout dialog

class CPayrollLayout : public CDialog
{
// Construction
public:
	CPayrollLayout(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPayrollLayout)
	enum { IDD = IDD_PAYROLLLAYOUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPayrollLayout)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListCtrl* pListCtrlLIST;
  CListCtrl* pListCtrlOUTPUTORDER;
  CButton*   pButtonUSED;
  CButton*   pButtonUNUSED;
  CEdit*     pEditPOSITION;

  BOOL m_bInitInProgress;
  int  m_Index;
  int  m_SelectedRow;

  void RefreshOutputOrder();


	// Generated message map functions
	//{{AFX_MSG(CPayrollLayout)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUnused();
	afx_msg void OnUsed();
	virtual void OnOK();
	afx_msg void OnChangePosition();
	afx_msg void OnKillfocusPosition();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAYROLLLAYOUT_H__E7A5723D_40F0_43F6_93A9_65878F475233__INCLUDED_)
