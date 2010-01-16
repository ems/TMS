//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_DISCIPLINE_H__1B366CB4_B692_41C3_8FC1_A9D563B5F20E__INCLUDED_)
#define AFX_DISCIPLINE_H__1B366CB4_B692_41C3_8FC1_A9D563B5F20E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Discipline.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDiscipline dialog

class CDiscipline : public CDialog
{
// Construction
public:
	CDiscipline(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDiscipline)
	enum { IDD = IDD_DISCIPLINE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscipline)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CComboBox* pComboBoxDRIVER;
  CListCtrl* pListCtrlHISTORY;
  CButton*   pButtonUPDATE;
  CButton*   pButtonNEW;
  CButton*   pButtonREPORT;
  CButton*   pButtonDELETE;

  long m_DRIVERSrecordID;

  void SetupDriverComboBox();
  void DisplayHistory(void);

	// Generated message map functions
	//{{AFX_MSG(CDiscipline)
	virtual BOOL OnInitDialog();
	afx_msg void OnClickHistory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNew();
	afx_msg void OnUpdate();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnSelendokDriver();
	afx_msg void OnDblclkHistory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnReport();
	afx_msg void OnConsider();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCIPLINE_H__1B366CB4_B692_41C3_8FC1_A9D563B5F20E__INCLUDED_)
