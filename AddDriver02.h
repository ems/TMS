//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDDRIVER02_H__95CABB1C_72C9_4BF3_8FAB_CFF8464E6268__INCLUDED_)
#define AFX_ADDDRIVER02_H__95CABB1C_72C9_4BF3_8FAB_CFF8464E6268__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriver02.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriver02 dialog

class CAddDriver02 : public CPropertyPage
{
	DECLARE_DYNCREATE(CAddDriver02)

// Construction
public:
	CAddDriver02();
	~CAddDriver02();

// Dialog Data
	//{{AFX_DATA(CAddDriver02)
	enum { IDD = IDD_ADDDRIVER02 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAddDriver02)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAddDriver02)
	virtual BOOL OnInitDialog();
	afx_msg void OnHiredatebox();
	afx_msg void OnTransithiredatebox();
	afx_msg void OnFulltimedatebox();
	afx_msg void OnPromotiondatebox();
	afx_msg void OnSendatebox();
	afx_msg void OnTerminationdatebox();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

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
  
  void ClearDateField(CDTPicker *pDTP);
  void SetDateField(CDTPicker *pDTP);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVER02_H__95CABB1C_72C9_4BF3_8FAB_CFF8464E6268__INCLUDED_)
