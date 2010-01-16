#if !defined(AFX_ROSTER_H__DC4058F4_34E0_4E3C_A2F9_F88CA13C2153__INCLUDED_)
#define AFX_ROSTER_H__DC4058F4_34E0_4E3C_A2F9_F88CA13C2153__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Roster.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoster dialog

class CRoster : public CDialog
{
// Construction
public:
	CRoster(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRoster)
	enum { IDD = IDD_ROSTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoster)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  PDISPLAYINFO m_pDI;
  HMENU m_hMenu1;
  HMENU m_hMenu2;
  long  m_serviceDays[ROSTER_MAX_DAYS];
  int   m_dayBits[ROSTER_MAX_DAYS];

  CListCtrl* pListCtrlROSTERED;
  CListCtrl* pListCtrlUNROSTERED;

  void DisplayPanels(BOOL bDoUNROSTERED, BOOL bDoROSTERED);
  int  GetSelections(CListCtrl* pListCtrl, CUIntArray* indexesIntoCOMBINED);

	// Generated message map functions
	//{{AFX_MSG(CRoster)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDblclkRostered(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkUnrostered(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnParameters();
	afx_msg void OnSave();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnManual();
	afx_msg void OnAutomatic();
	afx_msg void OnTakerun();
	afx_msg void OnUndorostered();
	afx_msg void OnUndounrostered();
	afx_msg void OnColumnclickUnrostered(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickRostered(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSwapunrostered();
	afx_msg void OnSwaprostered();
	afx_msg void OnMerge();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROSTER_H__DC4058F4_34E0_4E3C_A2F9_F88CA13C2153__INCLUDED_)
