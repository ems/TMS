#if !defined(AFX_DAILYOPSBL_H__2B8473B5_F4B0_4AF6_97F4_50FA1DD40CD4__INCLUDED_)
#define AFX_DAILYOPSBL_H__2B8473B5_F4B0_4AF6_97F4_50FA1DD40CD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsBL.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBL dialog

class CDailyOpsBL : public CDialog
{
// Construction
public:
	CDailyOpsBL(CWnd* pParent = NULL, long* pLocationNODESrecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsBL)
	enum { IDD = IDD_DAILYOPSBL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsBL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CComboBox* pComboBoxLOCATION;

  long* m_pLocationNODESrecordID;

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsBL)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSBL_H__2B8473B5_F4B0_4AF6_97F4_50FA1DD40CD4__INCLUDED_)
