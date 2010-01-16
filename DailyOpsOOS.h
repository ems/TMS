#if !defined(AFX_DAILYOPSOOS_H__992FF859_F254_401E_BAE5_8B5AF979A1CA__INCLUDED_)
#define AFX_DAILYOPSOOS_H__992FF859_F254_401E_BAE5_8B5AF979A1CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsOOS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsOOS dialog

class CDailyOpsOOS : public CDialog
{
// Construction
public:
	CDailyOpsOOS(CWnd* pParent = NULL, DAILYOPSBSPassedDataDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsOOS)
	enum { IDD = IDD_DAILYOPSOOS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsOOS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CComboBox* pComboBoxREASON;
  CDTPicker* pDTPickerTIME;
  CComboBox* pComboBoxLOCATION;
  CEdit*     pEditPAX;
  CButton*   pButtonMARKOOS;

  DAILYOPSBSPassedDataDef* m_pPassedData;

  void OnNow();

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsOOS)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSOOS_H__992FF859_F254_401E_BAE5_8B5AF979A1CA__INCLUDED_)
