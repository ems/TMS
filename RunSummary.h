//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_RUNSUMMARY_H__AF9D7EEE_9A12_4DE5_A4AC_462DAAF05385__INCLUDED_)
#define AFX_RUNSUMMARY_H__AF9D7EEE_9A12_4DE5_A4AC_462DAAF05385__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RunSummary.h : header file
//

typedef struct RUNSUMMARYStruct
{
  long numRuns;
  long platformTime;
  long reportTime;
  long turninTime;
  long travelTime;
  long makeUpTime;
  long overTime;
  long spreadOvertime;
  long paidBreaks;
  long payTime;
} RUNSUMMARYDef;


/////////////////////////////////////////////////////////////////////////////
// CRunSummary dialog

class CRunSummary : public CDialog
{
// Construction
public:
//	CRunSummary(CWnd* pParent = NULL);   // standard constructor
	CRunSummary(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);

// Dialog Data
	//{{AFX_DATA(CRunSummary)
	enum { IDD = IDD_RUNSUMMARY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunSummary)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  PDISPLAYINFO m_pDI;
  BOOL m_bShowUnclassified;
  int  m_Ratio;
  BOOL m_bReport;
  BOOL m_bTravel;
  BOOL m_bMakeup;
  BOOL m_bOvertime;
  BOOL m_bSpreadOT;
  BOOL m_bPaidBreaks;

  CListCtrl* pListCtrlLIST;
  CButton*   pButtonUNCLASSIFIED;
  CButton*   pButtonPAYTOPLATFORM;
  CButton*   pButtonPLATFORMTOPAY;
  CButton*   pButtonREPORT;
  CButton*   pButtonTRAVEL;
  CButton*   pButtonMAKEUP;
  CButton*   pButtonOVERTIME;
  CButton*   pButtonSPREADOT;
  CButton*   pButtonPAIDBREAKS;

  void       OutputRow(char* pszRuntype, RUNSUMMARYDef* pRS, BOOL bShowLines);
  void       DoCalc(void);
  void       DoDisplay();

	// Generated message map functions
	//{{AFX_MSG(CRunSummary)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnUnclassified();
	afx_msg void OnPaytoplatform();
	afx_msg void OnPlatformtopay();
	afx_msg void OnReport();
	afx_msg void OnTravel();
	afx_msg void OnMakeup();
	afx_msg void OnOvertime();
	afx_msg void OnPrint();
	afx_msg void OnSpreadot();
	afx_msg void OnPaidbreaks();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RUNSUMMARY_H__AF9D7EEE_9A12_4DE5_A4AC_462DAAF05385__INCLUDED_)
