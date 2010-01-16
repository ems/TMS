#if !defined(AFX_DAILYOPSRL_H__856C0534_2902_4373_801D_88023F638AE4__INCLUDED_)
#define AFX_DAILYOPSRL_H__856C0534_2902_4373_801D_88023F638AE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsRL.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRL dialog

class CDailyOpsRL : public CDialog
{
// Construction
public:
	CDailyOpsRL(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsRL)
	enum { IDD = IDD_DAILYOPSRL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsRL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsRL)
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

#endif // !defined(AFX_DAILYOPSRL_H__856C0534_2902_4373_801D_88023F638AE4__INCLUDED_)
