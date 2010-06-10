#if !defined(AFX_DAILYOPSBP_H__2E36E167_A879_4D77_A07F_9D614D040AF1__INCLUDED_)
#define AFX_DAILYOPSBP_H__2E36E167_A879_4D77_A07F_9D614D040AF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsBP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBP dialog

class CDailyOpsBP : public CDialog
{
// Construction
public:
	CDailyOpsBP(CWnd* pParent = NULL, TripInfoDef* pTI = NULL, int numInTripInfo = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsBP)
	enum { IDD = IDD_DAILYOPSBP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsBP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  
  CListCtrl* pListCtrlTRIPLIST;
  CButton*   pButtonIDMODIFY;
  
  TripInfoDef* m_pTI;
  int          m_numInTI;

  void DisplayRow(int rowNumber);

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsBP)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnModify();
	afx_msg void OnClickTriplist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTriplist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSBP_H__2E36E167_A879_4D77_A07F_9D614D040AF1__INCLUDED_)
