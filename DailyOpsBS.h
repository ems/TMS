#if !defined(AFX_DAILYOPSBS_H__B6F7DA4F_F46C_451E_AD16_B157A5445049__INCLUDED_)
#define AFX_DAILYOPSBS_H__B6F7DA4F_F46C_451E_AD16_B157A5445049__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsBS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBS dialog

class CDailyOpsBS : public CDialog
{
// Construction
public:
	CDailyOpsBS(CWnd* pParent = NULL, DAILYOPSBSPassedDataDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsBS)
	enum { IDD = IDD_DAILYOPSBS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsBS)
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
	//{{AFX_MSG(CDailyOpsBS)
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

#endif // !defined(AFX_DAILYOPSBS_H__B6F7DA4F_F46C_451E_AD16_B157A5445049__INCLUDED_)
