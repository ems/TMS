#if !defined(AFX_DAILYOPSAL_H__A1C609C0_8E9B_4AF6_AAC2_32F4754539D7__INCLUDED_)
#define AFX_DAILYOPSAL_H__A1C609C0_8E9B_4AF6_AAC2_32F4754539D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsAL.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAL dialog

class CDailyOpsAL : public CDialog
{
// Construction
public:
	CDailyOpsAL(CWnd* pParent = NULL, DAILYOPSALDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsAL)
	enum { IDD = IDD_DAILYOPSAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsAL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CStatic*   pStaticWHO;
  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerUNTILDATE;
  CButton*   pButtonALLDAY;
  CButton*   pButtonSPECIFICTIMES;
  CStatic*   pStaticFROMTIME_TITLE;
  CDTPicker* pDTPickerFROMTIME;
  CButton*   pButtonIDNOW1;
  CStatic*   pStaticUNTILTIME_TITLE;
  CDTPicker* pDTPickerUNTILTIME;
  CButton*   pButtonIDNOW2;
  CListBox*  pListBoxREASON;
  CEdit*     pEditTIMEUSED;
  CButton*   pButtonIDCALCULATE;
  CEdit*     pEditNOTES;

  DAILYOPSALDef* m_pPassedData;

  void SetTimeToNow(CDTPicker* pDTP);


	// Generated message map functions
	//{{AFX_MSG(CDailyOpsAL)
	afx_msg void OnAllday();
	afx_msg void OnSpecifictimes();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnNow1();
	afx_msg void OnNow2();
	afx_msg void OnCalculate();
	afx_msg void OnChangeFromdate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSAL_H__A1C609C0_8E9B_4AF6_AAC2_32F4754539D7__INCLUDED_)
