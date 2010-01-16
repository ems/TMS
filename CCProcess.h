//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CCPROCESS_H__1D9A139E_4DC4_469A_8925_07650CE67865__INCLUDED_)
#define AFX_CCPROCESS_H__1D9A139E_4DC4_469A_8925_07650CE67865__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCProcess.h : header file
//

#define CCPROCESS_LOGFLAGS_ALL            0xFFFF
#define CCPROCESS_LOGFLAGS_CUSTOMER       0x0002
#define CCPROCESS_LOGFLAGS_HISTORY        0x0004
#define CCPROCESS_LOGFLAGS_DATETIME       0x0008
#define CCPROCESS_LOGFLAGS_WEATHER        0x0010
#define CCPROCESS_LOGFLAGS_BUSNUMBER      0x0020
#define CCPROCESS_LOGFLAGS_LOCATION       0x0040
#define CCPROCESS_LOGFLAGS_ROUTE          0x0080
#define CCPROCESS_LOGFLAGS_SEX            0x0100
#define CCPROCESS_LOGFLAGS_DESCRIPTION    0x0200
#define CCPROCESS_LOGFLAGS_DRIVERNAME     0x0400
#define CCPROCESS_LOGFLAGS_SPEEDING       0x0800
#define CCPROCESS_LOGFLAGS_ATTENTION      0x1000
#define CCPROCESS_LOGFLAGS_TIMEDIFFERENCE 0x2000
#define CCPROCESS_LOGFLAGS_TAKENBY        0x4000

/////////////////////////////////////////////////////////////////////////////
// CCCProcess dialog

class CCCProcess : public CDialog
{
// Construction
public:
//	CCCProcess(CWnd* pParent = NULL);   // standard constructor
  CCCProcess(CWnd* pParent = NULL, CCFEDef* pCCFE = NULL, char* pCurrentLog = NULL);

// Dialog Data
	//{{AFX_DATA(CCCProcess)
	enum { IDD = IDD_CCPROCESS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCProcess)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CCFEDef* m_pCCFE;
  char*    m_pCurrentLog;
  char     m_previousLog[COMPLAINTS_VARIABLE_LENGTH];

  CEdit*     pEditPREVIOUSLOG;
  CEdit*     pEditCURRENTLOG;
  CButton*   pButtonPRINTLETTER;
  CButton*   pButtonPRINTLOG;
  CButton*   pButtonPRINTFORREVIEW;
  CButton*   pButtonEMAIL;
  CButton*   pButtonSUPERVISOR;
  CListCtrl* pListCtrlLIST;
  CListBox*  pListBoxTYPICAL;

  void PrintLog(BOOL bFromControl);
  void PrintLetter();
  void PrintForReview();
  void SetupLog(int flags);
  void SaveRecord();
	// Generated message map functions
	//{{AFX_MSG(CCCProcess)
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEmail();
	afx_msg void OnDblclkTypical();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBack();
	afx_msg void OnSupervisor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCPROCESS_H__1D9A139E_4DC4_469A_8925_07650CE67865__INCLUDED_)
