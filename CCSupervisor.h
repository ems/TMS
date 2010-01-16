//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCSUPERVISOR_H__99AD486C_3E14_4539_B5C9_D3AD1AD89BAF__INCLUDED_)
#define AFX_CCSUPERVISOR_H__99AD486C_3E14_4539_B5C9_D3AD1AD89BAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCSupervisor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCSupervisor dialog

class CCCSupervisor : public CDialog
{
// Construction
public:
  CCCSupervisor(CWnd* pParent = NULL, COMPLAINTSDef* pCOMPLAINTS = NULL, CCFEDef* pCCFE = NULL);
 
  static void PopulateComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox);

// Dialog Data
	//{{AFX_DATA(CCCSupervisor)
	enum { IDD = IDD_CCSUPERVISOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCSupervisor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  COMPLAINTSDef* m_pCOMPLAINTS;
  char m_szDateTime[CCFE_DATETIME_LENGTH];
  char m_szReceivedBy[CCFE_RECEIVEDBY_LENGTH];

  CComboBox* pComboBoxDEPARTMENT;
  CComboBox* pComboBoxASSIGNEDTO;
  CComboBox* pComboBoxCLASSIFICATIONCODE;
  CComboBox* pComboBoxCATEGORY;
  CButton*   pButtonDAMAGE;
  CButton*   pButtonVIOLATION;
  CComboBox* pComboBoxEMPLOYEENAME;
  CDTPicker* pDTPickerDATEPRESENTED;
  CButton*   pButtonSIMILARCOMMENTS;
  CButton*   pButtonPHONE;
  CButton*   pButtonLETTER;
  CButton*   pButtonEMAIL;
  CEdit*     pEditCONVERSATION;
  CComboBox* pComboBoxACTIONCODE;
  CButton*   pButtonNOVIOLATION;
  CButton*   pButtonINFORMATION;
  CButton*   pButtonINSUFFICIENT;
  CButton*   pButtonCOMMENDATION;
  CButton*   pButtonOTHER;
  CEdit*     pEditOTHERTEXT;
  CButton*   pButtonOPERATIONS;
  CButton*   pButtonMAINTENANCE;
  CButton*   pButtonFINANCE;
  CButton*   pButtonDEVELOPMENT;
  CButton*   pButtonSPECIALSERVICES;
  CButton*   pButtonCOMMUNITYRELATIONS;
  CDTPicker* pDTPickerRETAINUNTIL;
  CStatic*   pStaticDATECLOSED;

  void SaveRecord();
  void ClearOtherText();

	// Generated message map functions
	//{{AFX_MSG(CCCSupervisor)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnFinalize();
	afx_msg void OnOther();
	afx_msg void OnNoviolation();
	afx_msg void OnInformation();
	afx_msg void OnInsufficient();
	afx_msg void OnCommendation();
	afx_msg void OnPrint();
	afx_msg void OnScan();
	afx_msg void OnView();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCSUPERVISOR_H__99AD486C_3E14_4539_B5C9_D3AD1AD89BAF__INCLUDED_)
