//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ONTIMEPERFORMANCE_H__780379E3_B015_42F4_BC40_EA5BF8F26A0D__INCLUDED_)
#define AFX_ONTIMEPERFORMANCE_H__780379E3_B015_42F4_BC40_EA5BF8F26A0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OnTimePerformance.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance dialog

class COnTimePerformance : public CDialog
{
// Construction
public:
	COnTimePerformance(CWnd* pParent = NULL, TMSRPT36PassedDataDef *pPD = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COnTimePerformance)
	enum { IDD = IDD_ONTIMEPERF };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COnTimePerformance)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void SetUpTextComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox);

  TMSRPT36PassedDataDef *m_pPD;
  char m_szDays[7][16];

  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerTODATE;
  CButton*   pButtonALLSERVICES;
  CButton*   pButtonJUSTSERVICE;
  CComboBox* pComboBoxJUSTSERVICELIST;
  CButton*   pButtonALLDAYS;
  CButton*   pButtonJUSTDAY;
  CComboBox* pComboBoxJUSTDAYLIST;
  CButton*   pButtonALLROUTES;
  CButton*   pButtonJUSTROUTE;
  CComboBox* pComboBoxJUSTROUTELIST;
  CButton*   pButtonOB;
  CButton*   pButtonIB;
  CButton*   pButtonBOTHDIRECTIONS;
  CButton*   pButtonALLTIMEPOINTS;
  CButton*   pButtonJUSTTIMEPOINT;
  CComboBox* pComboBoxJUSTTIMEPOINTLIST;
  CButton*   pButtonALLDAY;
  CButton*   pButtonFROM;
  CEdit*     pEditFROMTIME;
  CEdit*     pEditTOTIME;
  CButton*   pButtonALLOPERATORS;
  CButton*   pButtonJUSTOPERATOR;
  CComboBox* pComboBoxJUSTOPERATORLIST;
  CButton*   pButtonALLBUSTYPES;
  CButton*   pButtonJUSTBUSTYPE;
  CComboBox* pComboBoxJUSTBUSTYPELIST;
  CButton*   pButtonDONOTEXCLUDE;
  CButton*   pButtonJUSTEXCLUDE;
  CComboBox* pComboBoxJUSTEXCLUDELIST;
  CButton*   pButtonALLPASSENGERLOADS;
  CButton*   pButtonJUSTPASSENGERLOAD;
  CComboBox* pComboBoxJUSTPASSENGERLOADLIST;
  CButton*   pButtonALLROADSUPERVISORS;
  CButton*   pButtonJUSTROADSUPERVISOR;
  CComboBox* pComboBoxJUSTROADSUPERVISORLIST;

	// Generated message map functions
	//{{AFX_MSG(COnTimePerformance)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnHelp();
	virtual void OnCancel();
	afx_msg void OnAllservices();
	afx_msg void OnAlldays();
	afx_msg void OnAlltimepoints();
	afx_msg void OnAllroutes();
	afx_msg void OnAllday();
	afx_msg void OnAlloperators();
	afx_msg void OnDonotexclude();
	afx_msg void OnAllbustypes();
	afx_msg void OnAllpassengerloads();
	afx_msg void OnAllroadsupervisors();
	afx_msg void OnSelchangeJustbustypelist();
	afx_msg void OnSelchangeJustdaylist();
	afx_msg void OnSelchangeJustexcludelist();
	afx_msg void OnSelchangeJustoperatorlist();
	afx_msg void OnSelchangeJustpassengerloadlist();
	afx_msg void OnSelchangeJustroadsupervisorlist();
	afx_msg void OnSelchangeJustroutelist();
	afx_msg void OnSelchangeJustservicelist();
	afx_msg void OnSelchangeJusttimepointlist();
	afx_msg void OnJustbustype();
	afx_msg void OnJustday();
	afx_msg void OnJustexclude();
	afx_msg void OnJustoperator();
	afx_msg void OnJustpassengerload();
	afx_msg void OnJustroadsupervisor();
	afx_msg void OnJustroute();
	afx_msg void OnJustservice();
	afx_msg void OnJusttimepoint();
	afx_msg void OnFrom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ONTIMEPERFORMANCE_H__780379E3_B015_42F4_BC40_EA5BF8F26A0D__INCLUDED_)
