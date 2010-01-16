//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDNODE_H__02CF55A1_A413_4F29_8BAA_D2FDAB53A643__INCLUDED_)
#define AFX_ADDNODE_H__02CF55A1_A413_4F29_8BAA_D2FDAB53A643__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddNode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddNode dialog

class CAddNode : public CDialog
{
// Construction
public:
	CAddNode(CWnd* pParent = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddNode)
	enum { IDD = IDD_ADDNODE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddNode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CEdit*     pEditABBRNAME;
  CEdit*     pEditLONGNAME;
  CEdit*     pEditAVLSTOPNAME;
  CEdit*     pEditINTERSECTION;
  CEdit*     pEditDESCRIPTION;
  CEdit*     pEditCAPACITY;
  CEdit*     pEditLONGITUDE;
  CEdit*     pEditLATITUDE;
  CEdit*     pEditLABELS;
  CStatic*   pStaticLABELS_TEXT;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;
  CComboBox* pComboBoxJURISDICTION;
  CEdit*     pEditNUMBER;
  CEdit*     pEditMAPCODES;
  CButton*   pButtonGARAGE;
  CButton*   pButtonSTOP;
  CButton*   pButtonSECURE;
  CButton*   pButtonAVLTRANSFERPOINT;
  CStatic*   pStaticLONGITUDE_TEXT;
  CStatic*   pStaticLATITUDE_TEXT;
  CListBox*  pListBoxSTOPFLAGSLIST;
  CStatic*   pStaticSTOPFLAGS;
  CButton*   pButtonTHISISARELIEFPOINT;
  CStatic*   pStaticOBSTOPNUMBER_TEXT;
  CEdit*     pEditOBSTOPNUMBER;
  CStatic*   pStaticIBSTOPNUMBER_TEXT;
  CEdit*     pEditIBSTOPNUMBER;

  long* m_pUpdateRecordID;
  BOOL  m_bWasGarage;

  void EnableStopFlags(BOOL bState);


	// Generated message map functions
	//{{AFX_MSG(CAddNode)
	afx_msg void OnStop();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnSelendokCommentcode();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnThisisareliefpoint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDNODE_H__02CF55A1_A413_4F29_8BAA_D2FDAB53A643__INCLUDED_)
