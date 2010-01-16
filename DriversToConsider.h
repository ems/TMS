#if !defined(AFX_DRIVERSTOCONSIDER_H__B0E01EFB_D44A_40FC_AE23_0BCCB8D74D74__INCLUDED_)
#define AFX_DRIVERSTOCONSIDER_H__B0E01EFB_D44A_40FC_AE23_0BCCB8D74D74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DriversToConsider.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDriversToConsider dialog

class CDriversToConsider : public CDialog
{
// Construction
public:
	CDriversToConsider(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDriversToConsider)
	enum { IDD = IDD_DRIVERSTOCONSIDER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDriversToConsider)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListBox* pListBoxTYPES;

	// Generated message map functions
	//{{AFX_MSG(CDriversToConsider)
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

#endif // !defined(AFX_DRIVERSTOCONSIDER_H__B0E01EFB_D44A_40FC_AE23_0BCCB8D74D74__INCLUDED_)
