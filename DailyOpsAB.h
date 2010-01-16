#if !defined(AFX_DAILYOPSAB_H__D108B0CA_710E_4633_BD61_5C036DACFD82__INCLUDED_)
#define AFX_DAILYOPSAB_H__D108B0CA_710E_4633_BD61_5C036DACFD82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsAB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAB dialog

class CDailyOpsAB : public CDialog
{
// Construction
public:
	CDailyOpsAB(CWnd* pParent = NULL, long date = NO_RECORD, BOOL bAutomatic = FALSE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsAB)
	enum { IDD = IDD_DAILYOPSAB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsAB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CListCtrl* pListCtrlLIST;
  CDTPicker* pDTPickerDATE;
  CButton*   pButtonSHOWOFF;

  long m_Date;
  BOOL m_bInitializing;
  BOOL m_bAutomatic;

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsAB)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnChangeDate();
	virtual void OnOK();
	afx_msg void OnPrint();
	afx_msg void OnShowoff();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSAB_H__D108B0CA_710E_4633_BD61_5C036DACFD82__INCLUDED_)
