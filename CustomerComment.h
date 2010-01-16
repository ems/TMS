//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CUSTOMERCOMMENT_H__6FF3AEEC_7347_4D7A_AE21_4522A49DEECF__INCLUDED_)
#define AFX_CUSTOMERCOMMENT_H__6FF3AEEC_7347_4D7A_AE21_4522A49DEECF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomerComment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomerComment dialog

class CCustomerComment : public CDialog
{
// Construction
public:
//	CCustomerComment(CWnd* pParent = NULL);   // standard constructor
  CCustomerComment(CWnd* pParent = NULL, CCFEDef* pCCFE = NULL);

// Dialog Data
	//{{AFX_DATA(CCustomerComment)
	enum { IDD = IDD_CUSTOMERCOMMENT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomerComment)
	public:
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CCFEDef*   m_pCCFE;
  long       m_customerRecordID;
  char       m_newEntry[COMPLAINTS_VARIABLE_LENGTH];

  CWnd*      pGroupBoxTRACKINGNUMBER;
  CStatic*   pStaticRECEIVEDBY;
  CButton*   pButtonIDUPDATECUSTOMER;
  CButton*   pButtonIDFIND;
  CStatic*   pStaticNAME;
  CStatic*   pStaticADDRESS;
  CStatic*   pStaticCITYSTATE;
  CStatic*   pStaticZIP;
  CStatic*   pStaticDAYPHONE;
  CStatic*   pStaticNIGHTPHONE;
  CStatic*   pStaticFAX;
  CStatic*   pStaticEMAIL;
  CButton*   pButtonHISTORYNO;
  CButton*   pButtonHISTORYYES;
  CStatic*   pStaticHISTORYHOWMANYTIMES;
  CButton*   pButtonHISTORYNOTSURE;
  CButton*   pButtonHISTORYTHISMANY;
  CEdit*     pEditHISTORYTHISMANYNUMBER;
  CDTPicker* pDTPickerDETAILDATE;
  CDTPicker* pDTPickerDETAILTIME;
  CEdit*     pEditVEHICLENUMBER;
  CEdit*     pEditLOCATION;
  CComboBox* pComboBoxROUTE;
  CStatic*   pStaticDIRECTION_TEXT;
  CButton*   pButtonNADIRECTION;
  CButton*   pButtonOUTBOUND;
  CButton*   pButtonINBOUND;
  CComboBox* pComboBoxWEATHER;
  CButton*   pButtonEMPLOYEEMALE;
  CButton*   pButtonEMPLOYEEFEMALE;
  CButton*   pButtonNAEMPLOYEE;
  CComboBox* pComboBoxEMPLOYEENAME;
  CEdit*     pEditEMPLOYEEDESCRIPTION;
  CButton*   pButtonNASPEEDING;
  CButton*   pButtonSPEEDINGNO;
  CButton*   pButtonSPEEDINGYES;
  CEdit*     pEditSPEED;
  CButton*   pButtonNAATTENTION;
  CButton*   pButtonATTENTIONNO;
  CButton*   pButtonATTENTIONYES;
  CButton*   pButtonNATIMEDIFFERENCE;
  CButton*   pButtonLESSTHANTWO;
  CButton*   pButtonTWOTOFIVE;
  CButton*   pButtonMORETHANFIVE;

  void PopulateCustomerFields();
  void BlankCustomerFields();

	// Generated message map functions
	//{{AFX_MSG(CCustomerComment)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnNew();
	afx_msg void OnFind();
	afx_msg void OnHistoryno();
	afx_msg void OnHistoryyes();
	afx_msg void OnHistorynotsure();
	afx_msg void OnHistorythismany();
	afx_msg void OnSelchangeRoute();
	afx_msg void OnNaspeeding();
	afx_msg void OnSpeedingyes();
	afx_msg void OnUpdatecustomer();
	afx_msg void OnProcess();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMERCOMMENT_H__6FF3AEEC_7347_4D7A_AE21_4522A49DEECF__INCLUDED_)
