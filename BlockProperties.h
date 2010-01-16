//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_BLOCKPROPERTIES_H__3C914637_68B3_4F33_A823_38DEF66A1A4A__INCLUDED_)
#define AFX_BLOCKPROPERTIES_H__3C914637_68B3_4F33_A823_38DEF66A1A4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlockProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBlockProperties dialog

class CBlockProperties : public CResizingDialog
{
// Construction
public:
//	CBlockProperties(CWnd* pParent = NULL);   // standard constructor
	CBlockProperties(CWnd* pParent = NULL, long blockNumber = -1, CCutRuns *pCRDlg = NULL);   // standard constructor

  BOOL Create(UINT nID, CWnd * pWnd)
  {
    return CDialog::Create(nID, pWnd);
  }

  BOOL ShowReliefPoints(void);

// Dialog Data
	//{{AFX_DATA(CBlockProperties)
	enum { IDD = IDD_BLOCKPROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListCtrl *pListCtrlLIST;
  CStatic   *pStaticTRIPINFO;
  CListCtrl *pListCtrlTRIPDATA;
  CImageList *m_pImageList;
  long m_blockNumber;
  CCutRuns *m_pCRDlg;
  CWnd *m_pVRDlg;

	// Generated message map functions
	//{{AFX_MSG(CBlockProperties)
	afx_msg void OnHelp();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnCloseButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKPROPERTIES_H__3C914637_68B3_4F33_A823_38DEF66A1A4A__INCLUDED_)
