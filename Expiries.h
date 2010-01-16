#if !defined(AFX_EXPIRIES_H__5710DA9A_DEE8_48E7_B248_295BB90978BD__INCLUDED_)
#define AFX_EXPIRIES_H__5710DA9A_DEE8_48E7_B248_295BB90978BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Expiries.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExpiries dialog

class CExpiries : public CDialog
{
// Construction
public:
	CExpiries(CWnd* pParent = NULL, BOOL* pLicense = NULL, BOOL* pMedical = NULL, int* pDays = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpiries)
	enum { IDD = IDD_EXPIRIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpiries)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton* pButtonCHECKLICENSE;
  CButton* pButtonCHECKMEDICAL;
  CEdit*   pEditDAYS;

  BOOL *m_pLicense;
  BOOL *m_pMedical;
  int  *m_pDays;

	// Generated message map functions
	//{{AFX_MSG(CExpiries)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnConsider();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPIRIES_H__5710DA9A_DEE8_48E7_B248_295BB90978BD__INCLUDED_)
