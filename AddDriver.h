//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVER_H__55B55987_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
#define AFX_ADDDRIVER_H__55B55987_0B69_11D6_A235_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriver.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriver dialog

class CAddDriver : public CDialog
{
// Construction
public:
	CAddDriver(CWnd* pParent = NULL, long *pUdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDriver)
	enum { IDD = IDD_ADDDRIVER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDriver)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CPropertySheet m_dlgPropSheet;
	CAddDriver01   m_Page01;
	CAddDriver02   m_Page02;
	CAddDriver03   m_Page03;
  CAddDriver04   m_Page04;
  
  long *m_pUpdateRecordID;
  long m_bInit;
  CBitmapCtrl m_bmpCtrl;

  CButton* pButtonNADT;
  CButton* pButtonDT;
  CComboBox* pComboBoxDRIVERTYPE;
  CButton* pButtonNADV;
  CButton* pButtonDV;
  CComboBox* pComboBoxDIVISION;
  CButton* pButtonCC;
  CButton* pButtonNACC;
  CComboBox* pComboBoxCOMMENTCODE;

  CEdit*     pEditLAST;
  CEdit*     pEditFIRST;
  CEdit*     pEditINITIALS;
  CEdit*     pEditADDRESS;
  CEdit*     pEditCITY;
  CStatic*   pStaticPROVINCE_TEXT;
  CEdit*     pEditPROVINCE;
  CStatic*   pStaticPOSTALCODE_TEXT;
  CEdit*     pEditPOSTALCODE;
  CMSMask*   pMSMaskPHONE;
  CMSMask*   pMSMaskFAX;
  CMSMask*   pMSMaskEMERGENCY;
  CEdit*     pEditEMERGENCYCONTACT;
  CEdit*     pEditEMAIL;
  CStatic*   pStaticSIN_TEXT;
  CEdit*     pEditSIN;
  CDTPicker* pDTPickerDATEOFBIRTH;
  CComboBox* pComboBoxSEX;
  CComboBox* pComboBoxRACE;

  CButton*   pButtonHIREDATEBOX;
  CDTPicker* pDTPickerHIREDATE;
  CButton*   pButtonTRANSITHIREDATEBOX;
  CDTPicker* pDTPickerTRANSITHIREDATE;
  CButton*   pButtonFULLTIMEDATEBOX;
  CDTPicker* pDTPickerFULLTIMEDATE;
  CButton*   pButtonPROMOTIONDATEBOX;
  CDTPicker* pDTPickerPROMOTIONDATE;
  CButton*   pButtonSENDATEBOX;
  CDTPicker* pDTPickerSENDATE;
  CButton*   pButtonTERMINATIONDATEBOX;
  CDTPicker* pDTPickerTERMINATIONDATE;
  CEdit*     pEditVACATION;
  CEdit*     pEditPENDINGVACATION;
  CEdit*     pEditPERSONAL;
  CEdit*     pEditSICK;
  CEdit*     pEditFLOAT;
  CEdit*     pEditSENSORT;
  CEdit*     pEditCREW;
  CEdit*     pEditBADGE;
  CStatic*   pStaticVACATIONDAYS;
  CStatic*   pStaticPENDINGVACATIONDAYS;
  CStatic*   pStaticPERSONALDAYS;
  CStatic*   pStaticSICKDAYS;
  CStatic*   pStaticFLOATDAYS;

  CEdit*     pEditLICENSENUMBER;
  CDTPicker* pDTPickerLICENSEEXPIRYDATE;
  CEdit*     pEditLICENSEPROVINCEOFISSUE;
  CEdit*     pEditLICENSETYPE;
  CDTPicker* pDTPickerPHYSICALEXPIRYDATE;
  CEdit*     pEditBITMAPPATH;

  CButton*   pButtonSUPERVISOR;
  CButton*   pButtonSAFETYSENSITIVE;
  CComboBox* pComboBoxEEOC;
  CComboBox* pComboBoxUNION;
  CComboBox* pComboBoxSECTION;
  CComboBox* pComboBoxPOSITION;

	// Generated message map functions
	//{{AFX_MSG(CAddDriver)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnCc();
	afx_msg void OnNacc();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnDv();
	afx_msg void OnSelendokDrivertype();
	afx_msg void OnNadt();
	afx_msg void OnSelendokDivision();
	afx_msg void OnNadv();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnDt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVER_H__55B55987_0B69_11D6_A235_0080C7294BDC__INCLUDED_)
