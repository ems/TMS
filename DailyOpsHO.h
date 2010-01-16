#if !defined(AFX_DAILYOPSHO_H__29EC1886_D791_4207_8AF0_97F7F70089DB__INCLUDED_)
#define AFX_DAILYOPSHO_H__29EC1886_D791_4207_8AF0_97F7F70089DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsHO.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsHO dialog

class CDailyOpsHO : public CDialog
{
// Construction
public:
	CDailyOpsHO(CWnd* pParent = NULL, long* pDate = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsHO)
	enum { IDD = IDD_DAILYOPSHO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsHO)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long* m_pDate;

  CListCtrl* pListCtrlLIST;
  CButton*   pButtonIDREMOVE;
  CButton*   pButtonIDUPDATE;

  void Display();

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsHO)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnInsert();
	afx_msg void OnUpdate();
	afx_msg void OnRemove();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSHO_H__29EC1886_D791_4207_8AF0_97F7F70089DB__INCLUDED_)
