//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCREPORTS_H__9A41963B_0954_4177_8376_38F159C73B57__INCLUDED_)
#define AFX_CCREPORTS_H__9A41963B_0954_4177_8376_38F159C73B57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCReports.h : header file
//

#define ODBC_CCMRPT01_COMPLAINT_LENGTH   12
#define ODBC_CCMRPT01_DATE_LENGTH        10
#define ODBC_CCMRPT01_CLASSIFICATION_LENGTH     20
#define ODBC_CCMRPT01_ROUTENUMBER_LENGTH  8
#define ODBC_CCMRPT01_LOCATION_LENGTH    16
#define ODBC_CCMRPT01_EMPLOYEE_LENGTH    16
#define ODBC_CCMRPT01_CUSTOMER_LENGTH    16
#define ODBC_CCMRPT01_REFERREDTO_LENGTH  16
#define ODBC_CCMRPT01_ACTION_LENGTH      16
#define ODBC_CCMRPT01_SORTFIELD_LENGTH   30

union CCMRPT02SortFields
{
  char szCatchAll[ODBC_CCMRPT01_CLASSIFICATION_LENGTH + 1];  // Longest element
  char referenceNumber[ODBC_CCMRPT01_COMPLAINT_LENGTH + 1];
  long incidentDate;
  char classification[ODBC_CCMRPT01_CLASSIFICATION_LENGTH + 1];
  char routeNumber[ODBC_CCMRPT01_ROUTENUMBER_LENGTH + 1];
  char location[ODBC_CCMRPT01_LOCATION_LENGTH + 1];
  char employee[ODBC_CCMRPT01_EMPLOYEE_LENGTH + 1];
  char customer[ODBC_CCMRPT01_CUSTOMER_LENGTH + 1];
  char referredTo[ODBC_CCMRPT01_REFERREDTO_LENGTH + 1];
  char action[ODBC_CCMRPT01_ACTION_LENGTH + 1];
};

typedef struct CCMRPT02Struct
{
  long  recordID;
  union CCMRPT02SortFields SortFields;
} CCMRPT02Def;

#define CCREPORTS_LISTING 0x0000
#define CCREPORTS_SUMMARY 0x0001
#define CCREPORTS_DETAIL  0x0002

/////////////////////////////////////////////////////////////////////////////
// CCCReports dialog

class CCCReports : public CDialog
{
// Construction
public:
	CCCReports(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCReports)
	enum { IDD = IDD_CCREPORTS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCReports)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CButton* pButtonBYREFERENCENUMBER;
  CButton* pButtonBYCUSTOMER;
  CButton* pButtonBYROUTE;
  CButton* pButtonBYCATEGORY;
  CButton* pButtonBYCLASSIFICATIONCODE;
  CButton* pButtonBYDETERMINATIONCODE;
  CButton* pButtonBYEMPLOYEE;
  CButton* pButtonBYDEPARTMENT;
  CButton* pButtonUNRESOLVED;
  CComboBox* pComboBoxREFERENCENUMBER;
  CComboBox* pComboBoxCUSTOMER;
  CComboBox* pComboBoxROUTE;
  CComboBox* pComboBoxCATEGORY;
  CComboBox* pComboBoxCLASSIFICATIONCODE;
  CComboBox* pComboBoxDETERMINATIONCODE;
  CComboBox* pComboBoxEMPLOYEE;
  CComboBox* pComboBoxDEPARTMENT;
  CComboBox* pComboBoxCB;
  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerUNTILDATE;
  CComboBox* pComboBoxSORTBY;
  CEdit*     pEditLOG;

  BOOL         m_flags;
  HFILE        m_hfOutputFile;
  CCMRPT02Def* m_pCCMRPT02;
  int          m_numCCMRPT02;
  int          m_sortIndex;

  void DisableDropdowns(void);
  void GenerateOutputLine(COMPLAINTSDef *pCOMPLAINTS);
  void GenerateOutputLine01(COMPLAINTSDef *pCOMPLAINTS);
  void GenerateOutputLine02();
  void StoreRelevantData(COMPLAINTSDef *pCOMPLAINTS);
  BOOL AllocateSpace();

	// Generated message map functions
	//{{AFX_MSG(CCCReports)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnBycustomer();
	afx_msg void OnByroute();
	afx_msg void OnBycategory();
	afx_msg void OnByclassificationcode();
	afx_msg void OnBydeterminationcode();
	afx_msg void OnByemployee();
	afx_msg void OnBydepartment();
	afx_msg void OnUnresolved();
	afx_msg void OnDetail();
	afx_msg void OnSummary();
	afx_msg void OnByreferencenumber();
	afx_msg void OnListing();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCREPORTS_H__9A41963B_0954_4177_8376_38F159C73B57__INCLUDED_)
