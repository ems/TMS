//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_STOPCONNECTION_H__A697804F_E38B_48B0_B836_6405ED64D0AA__INCLUDED_)
#define AFX_STOPCONNECTION_H__A697804F_E38B_48B0_B836_6405ED64D0AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StopConnection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStopConnection dialog


typedef struct SCSORTStruct
{
  long NODESrecordID;
  char abbrName[NODES_ABBRNAME_LENGTH + 1];
  char intersection[NODES_INTERSECTION_LENGTH + 1];
} SCSORTDef;

class CStopConnection : public CDialog
{
// Construction
public:
//	CStopConnection(CWnd* pParent = NULL);   // standard constructor
	CStopConnection(CWnd* pParent = NULL, CONNECTIONSDef* pCONNECTIONS = NULL);

// Dialog Data
	//{{AFX_DATA(CStopConnection)
	enum { IDD = IDD_STOPCONNECTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStopConnection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CONNECTIONSDef* m_pCONNECTIONS;

  CListCtrl* pListCtrlFROM;
  CListCtrl* pListCtrlTO;
  CEdit*     pEditDISTANCE;
  CStatic*   pStaticMEASURE;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;

  SCSORTDef *m_pSCSORT;
  BOOL       m_bAllowSort;


	// Generated message map functions
	//{{AFX_MSG(CStopConnection)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnColumnclickFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickTo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STOPCONNECTION_H__A697804F_E38B_48B0_B836_6405ED64D0AA__INCLUDED_)
