//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCFRONTEND_H__561EB691_C506_11D3_9A69_00A04B03FF52__INCLUDED_)
#define AFX_CCFRONTEND_H__561EB691_C506_11D3_9A69_00A04B03FF52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCFrontEnd.h : header file
//

typedef struct CCFESORTStruct
{
  char szReferenceNumber[COMPLAINTS_REFERENCENUMBER_LENGTH + 1];
  char szName[32];
  char szStatus[16];
  long flags;
} CCFESORTDef;

/////////////////////////////////////////////////////////////////////////////
// CCCFrontEnd dialog

class CCCFrontEnd : public CDialog
{
// Construction
public:
	CCCFrontEnd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCFrontEnd)
	enum { IDD = IDD_CUSTOMERCOMMENT_FRONTEND };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCFrontEnd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

 protected:
   BOOL m_bDisplayCancelled;

   CButton*   pButtonNEW;
   CButton*   pButtonEXISTING;
   CListCtrl* pListCtrlLIST;
   CButton*   pButtonTHISISA;
   CWnd*      pWndDATETIME;
   CWnd*      pWndRECEIVEDBY;
   CButton*   pButtonCOMPLAINT;
   CButton*   pButtonCOMMENDATION;
   CButton*   pButtonSERVICEREQUEST;
   CButton*   pButtonMISCELLANEOUS;
   CStatic*   pStaticJUMPTO_TEXT;
   CEdit*     pEditJUMPTO;
   CButton*   pButtonDISPLAYCANCELLED;
   CButton*   pButtonREOPEN;
 
   void PopulateListCtrl();
   int  ProcessSelectedItem();


   int  numComplaints;
   int  maxRefer;
   CCFEDef CCFE;
   CCFESORTDef *pCCFESORT;
   BOOL bAllowSort;
   char cPrefix[4];


	// Generated message map functions
	//{{AFX_MSG(CCCFrontEnd)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual BOOL OnInitDialog();
	afx_msg void OnNew();
	afx_msg void OnExisting();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeJumpto();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReports();
	afx_msg void OnDisplaycancelled();
	afx_msg void OnReopen();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCFRONTEND_H__561EB691_C506_11D3_9A69_00A04B03FF52__INCLUDED_)
