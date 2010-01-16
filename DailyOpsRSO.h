#if !defined(AFX_DAILYOPSRSO_H__A1ECC759_4D99_43F0_AE73_69698C361053__INCLUDED_)
#define AFX_DAILYOPSRSO_H__A1ECC759_4D99_43F0_AE73_69698C361053__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsRSO.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRSO dialog

class CDailyOpsRSO : public CDialog
{
// Construction
public:
	CDailyOpsRSO(CWnd* pParent = NULL, long* pNODESrecordID = NULL, long* pTime = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsRSO)
	enum { IDD = IDD_DAILYOPSRSO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsRSO)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CComboBox* pComboBoxNEWNODELIST;
  CEdit*     pEditNEWTIME;

  long* m_pNODESrecordID;
  long* m_pTime;

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsRSO)
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

#endif // !defined(AFX_DAILYOPSRSO_H__A1ECC759_4D99_43F0_AE73_69698C361053__INCLUDED_)
