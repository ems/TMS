//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_CUTRUNS_H__647AA6E9_AF22_468F_875C_1997E94D19DF__INCLUDED_)
#define AFX_CUTRUNS_H__647AA6E9_AF22_468F_875C_1997E94D19DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CutRuns.h : header file
//

#define VRC_COLUMN_RUNTYPE       0x00000001
#define VRC_COLUMN_RUNNUMBER     0x00000002
#define VRC_COLUMN_PIECENUMBER   0x00000004
#define VRC_COLUMN_BLOCKNUMBER   0x00000008
#define VRC_COLUMN_REPORTAT      0x00000010
#define VRC_COLUMN_REPORTTIME    0x00000020
#define VRC_COLUMN_STARTTRAVEL   0x00000040
#define VRC_COLUMN_ONAT          0x00000080
#define VRC_COLUMN_ONLOC         0x00000100
#define VRC_COLUMN_OFFAT         0x00000200
#define VRC_COLUMN_OFFLOC        0x00000400
#define VRC_COLUMN_ENDTRAVEL     0x00000800
#define VRC_COLUMN_TURNINTIME    0x00001000
#define VRC_COLUMN_PLATFORMTIME  0x00002000
#define VRC_COLUMN_TOTALTRAVEL   0x00004000
#define VRC_COLUMN_PAIDBREAKS    0x00008000
#define VRC_COLUMN_MAKEUPTIME    0x00010000
#define VRC_COLUMN_OVERTIME      0x00020000
#define VRC_COLUMN_SPREADPREMIUM 0x00040000
#define VRC_COLUMN_PAYTIME       0x00080000
#define VRC_COLUMN_PAYTOPLATFORM 0x00100000
#define VRC_COLUMN_PLATFORMTOPAY 0x00200000
#define VRC_COLUMN_SPREADTIME    0x00400000

/////////////////////////////////////////////////////////////////////////////
// CCutRuns dialog

class CCutRuns : public CDialog
{
// Construction
public:
//	CCutRuns(CWnd* pParent = NULL);   // standard constructor
	CCutRuns(CWnd* pParent = NULL, VRCOLORSDef* pVRColors = NULL);   // standard constructor
  void ShowRuns(VRCOLORSDef* pVRColors);
  void SetRunAtTop(long runNumber);
  void SayHello(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CCutRuns)
	enum { IDD = IDD_CUTRUNS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCutRuns)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

  CWnd *m_pVRDlg;
  VRCOLORSDef* m_pVRColors;
  VRCOLORSDef m_VRColors;
  void DisplayRun(int *position, RUNLISTDATADef *pRLD, RUNLISTDef *pRL, COSTDef *pC, BOOL bSetTopIndex);

  CListCtrl* pListCtrlLIST;
  CImageList m_ImageList;
  CMenu m_ContextMenu;
  CMenu m_ColumnMenu;
  RECT  m_rc;
  long  m_ColumnMask;
  int   m_nCaptionSize;



	// Generated message map functions
	//{{AFX_MSG(CCutRuns)
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDeselectall();
	afx_msg void OnFreeze();
	afx_msg void OnHookruns();
	afx_msg void OnImprove();
	afx_msg void OnProfile();
	afx_msg void OnProperties();
	afx_msg void OnRenumber();
	afx_msg void OnSelectall();
	afx_msg void OnShift();
	afx_msg void OnSummary();
	afx_msg void OnSwap();
	afx_msg void OnUndorun();
	afx_msg void OnUnhook();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReassignruntype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUTRUNS_H__647AA6E9_AF22_468F_875C_1997E94D19DF__INCLUDED_)
