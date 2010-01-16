#if !defined(AFX_DISPLAYROSTER_H__8E5445DF_C916_41A5_A627_6E39CB636B1D__INCLUDED_)
#define AFX_DISPLAYROSTER_H__8E5445DF_C916_41A5_A627_6E39CB636B1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisplayRoster.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisplayRoster dialog

class CDisplayRoster : public CDialog
{
// Construction
public:
	CDisplayRoster(CWnd* pParent = NULL, long* pRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisplayRoster)
	enum { IDD = IDD_DISPLAYROSTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayRoster)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton*   pButtonALL;
  CButton*   pButtonWEEK1;
  CButton*   pButtonWEEK2;
  CButton*   pButtonWEEK3;
  CButton*   pButtonWEEK4;
  CButton*   pButtonWEEK5;
  CButton*   pButtonWEEK6;
  CButton*   pButtonWEEK7;
  CButton*   pButtonWEEK8;
  CButton*   pButtonWEEK9;
  CButton*   pButtonWEEK10;
  CButton*   pButtonWEEKS[10];
  CListCtrl* pListCtrlLIST;
  CButton*   pButtonIDASSIGN;

  ROSTERDef  m_ROSTER;
  long*      m_pRecordID;
  CString    m_sFmt;

  void       DisplayCurrent(BOOL bFirst);

	// Generated message map functions
	//{{AFX_MSG(CDisplayRoster)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDelete();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAll();
	afx_msg void OnAssign();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYROSTER_H__8E5445DF_C916_41A5_A627_6E39CB636B1D__INCLUDED_)
