#if !defined(AFX_ASSIGNDRIVER_H__2E49606D_D521_4137_8030_52939372DF60__INCLUDED_)
#define AFX_ASSIGNDRIVER_H__2E49606D_D521_4137_8030_52939372DF60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AssignDriver.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAssignDriver dialog

class CAssignDriver : public CDialog
{
// Construction
public:
	CAssignDriver(CWnd* pParent = NULL, long* pUpdateRecordID = NULL, ROSTERDef* pROSTER = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAssignDriver)
	enum { IDD = IDD_ASSIGNDRIVER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAssignDriver)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListBox*  pListBoxDRIVER;
  long*      m_pUpdateRecordID;
  ROSTERDef* m_pROSTER;

	// Generated message map functions
	//{{AFX_MSG(CAssignDriver)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnDblclkDriver();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASSIGNDRIVER_H__2E49606D_D521_4137_8030_52939372DF60__INCLUDED_)
