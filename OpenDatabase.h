#if !defined(AFX_OPENDATABASE_H__32876331_89D1_445E_A250_910892DE9A14__INCLUDED_)
#define AFX_OPENDATABASE_H__32876331_89D1_445E_A250_910892DE9A14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpenDatabase.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpenDatabase dialog

class COpenDatabase : public CDialog
{
// Construction
public:
	COpenDatabase(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpenDatabase)
	enum { IDD = IDD_DATABASE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenDatabase)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CStatic*   pStaticFILENAME;
  CEdit*     pEditLONGDESCRIPTION;
  CEdit*     pEditSHORTDESCRIPTION;
  CDTPicker* pDTPickerEFFECTIVEDATE;
  CDTPicker* pDTPickerUNTILDATE;
  CStatic*   pStaticLASTUPDATEDATE;
  CStatic*   pStaticLASTUPDATEDBY;
  CButton*   pButtonLL;
  CButton*   pButtonEN;
  CStatic*   pStaticMESSAGE;
  long       m_CommonFlag;

	// Generated message map functions
	//{{AFX_MSG(COpenDatabase)
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

#endif // !defined(AFX_OPENDATABASE_H__32876331_89D1_445E_A250_910892DE9A14__INCLUDED_)
