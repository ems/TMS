//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_MANUALCUT_H__5D4DB91A_0042_4A8F_A86D_68984D221ED1__INCLUDED_)
#define AFX_MANUALCUT_H__5D4DB91A_0042_4A8F_A86D_68984D221ED1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ManualCut.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CManualCut dialog

class CManualCut : public CDialog
{
// Construction
public:
	CManualCut(CWnd* pParent = NULL, int firstIndex = NO_RECORD, int lastIndex = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CManualCut)
	enum { IDD = IDD_MANUALCUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CManualCut)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  COSTDef m_COST;
  long m_CutAsRuntype;
  int  m_firstIndex;
  int  m_lastIndex;
  int  m_PreviousStartPosition;
  int  m_PreviousEndPosition;

  CListCtrl* pListCtrlPOTENTIALRELIEFS;
  CComboBox* pComboBoxSTARTRELIEF;
  CComboBox* pComboBoxENDRELIEF;
  CComboBox* pComboBoxRUNTYPE;
  CStatic*   pStaticREPORTAT;
  CStatic*   pStaticONAT;
  CStatic*   pStaticONLOC;
  CStatic*   pStaticOFFAT;
  CStatic*   pStaticOFFLOC;
  CStatic*   pStaticPLATFORMTIME;
  CStatic*   pStaticREPORTTIME;
  CStatic*   pStaticSTARTTRAVEL;
  CStatic*   pStaticENDTRAVEL;
  CStatic*   pStaticTOTALTRAVEL;
  CStatic*   pStaticTURNINTIME;
  CStatic*   pStaticMAKEUPTIME;
  CStatic*   pStaticOVERTIME;
  CStatic*   pStaticSPREADTIME;
  CStatic*   pStaticPAYTIME;
  CStatic*   pStaticPLATFORMTOPAY;
  CStatic*   pStaticPAYTOPLATFORM;
  CButton*   pButtonTRAVELINSTRUCTIONS;
  CButton*   pButtonWHY;

  void DisplayRunCharacteristics();

	// Generated message map functions
	//{{AFX_MSG(CManualCut)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeRuntype();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnShowdirections();
	afx_msg void OnWhy();
	afx_msg void OnSelchangeStartrelief();
	afx_msg void OnSelchangeEndrelief();
	afx_msg void OnClickPotentialreliefs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickPotentialreliefs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANUALCUT_H__5D4DB91A_0042_4A8F_A86D_68984D221ED1__INCLUDED_)
