#if !defined(AFX_DAILYOPSXB_H__30F78D41_D17C_460A_A93A_CCDED4710A67__INCLUDED_)
#define AFX_DAILYOPSXB_H__30F78D41_D17C_460A_A93A_CCDED4710A67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsXB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsXB dialog

class CDailyOpsXB : public CDialog
{
// Construction
public:
	CDailyOpsXB(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsXB)
	enum { IDD = IDD_DAILYOPSXB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsXB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton*  pButtonALL;
  CButton*  pButtonJUSTUNROSTERED;
  CListBox* pListBoxUNASSIGNED;
  CListBox* pListBoxDAYTIME;
  CListBox* pListBoxNIGHTTIME;

  void Move(CListBox* pFrom, CListBox* pTo);
  void DisplayOperators();

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsXB)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnAddtoday();
	afx_msg void OnAddtonight();
	afx_msg void OnRemovefromday();
	afx_msg void OnRemovefromnight();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnAll();
	afx_msg void OnJustunrostered();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSXB_H__30F78D41_D17C_460A_A93A_CCDED4710A67__INCLUDED_)
