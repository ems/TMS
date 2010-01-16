#if !defined(AFX_COPYROSTER_H__5782CD68_51B9_40BA_BAC7_1CE79194FEFF__INCLUDED_)
#define AFX_COPYROSTER_H__5782CD68_51B9_40BA_BAC7_1CE79194FEFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyRoster.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyRoster dialog

class CCopyRoster : public CDialog
{
// Construction
public:
	CCopyRoster(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyRoster)
	enum { IDD = IDD_COPYROSTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyRoster)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CStatic*   pStaticDESTINATION;
  CButton*   pButtonWEEKBUTTON;
  CEdit*     pEditWEEK;
  CButton*   pButtonDIVISIONBUTTON;
  CComboBox* pComboBoxDIVISION;

	// Generated message map functions
	//{{AFX_MSG(CCopyRoster)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnEditchangeDivision();
	afx_msg void OnWeekbutton();
	afx_msg void OnDivisionbutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYROSTER_H__5782CD68_51B9_40BA_BAC7_1CE79194FEFF__INCLUDED_)
