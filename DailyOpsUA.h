#if !defined(AFX_DAILYOPSUA_H__54C71CE7_C320_4FCC_84DD_0DF9D948AE06__INCLUDED_)
#define AFX_DAILYOPSUA_H__54C71CE7_C320_4FCC_84DD_0DF9D948AE06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsUA.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsUA dialog

class CDailyOpsUA : public CDialog
{
// Construction
public:
	CDailyOpsUA(CWnd* pParent = NULL, long* pDAILYOPSrecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsUA)
	enum { IDD = IDD_DAILYOPSUA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsUA)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  long *m_pDAILYOPSrecordID;
  long  m_DRIVERSrecordID;
  BOOL  m_bGotOfftime;

  void SetTimeToNow(CDTPicker* pDTP);

  CButton*   pButtonNAME;
  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerFROMTIME;
  CButton*   pButtonIDNOW1;
  CDTPicker* pDTPickerUNTILDATE;
  CDTPicker* pDTPickerUNTILTIME;
  CButton*   pButtonIDNOW2;
  CComboBox* pComboBoxREASON;
  CEdit*     pEditNOTES;
  CEdit*     pEditTIMEUSED;
  CButton*   pButtonPAID;
  CButton*   pButtonNOTPAID;
  CButton*   pButtonNOTSUREIFPAID;
	// Generated message map functions
	//{{AFX_MSG(CDailyOpsUA)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnNow1();
	afx_msg void OnNow2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSUA_H__54C71CE7_C320_4FCC_84DD_0DF9D948AE06__INCLUDED_)
