#if !defined(AFX_CHOOSEDATE_H__4F343913_7BAF_4757_9420_E37E0D8CEB7C__INCLUDED_)
#define AFX_CHOOSEDATE_H__4F343913_7BAF_4757_9420_E37E0D8CEB7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseDate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseDate dialog

class CChooseDate : public CDialog
{
// Construction
public:
	CChooseDate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseDate)
	enum { IDD = IDD_CHOOSEDATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseDate)
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

#endif // !defined(AFX_CHOOSEDATE_H__4F343913_7BAF_4757_9420_E37E0D8CEB7C__INCLUDED_)
