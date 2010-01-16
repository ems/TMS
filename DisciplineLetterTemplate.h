#if !defined(AFX_DISCIPLINELETTERTEMPLATE_H__1C4DAFC0_854D_46EB_890C_0F9298A12CB5__INCLUDED_)
#define AFX_DISCIPLINELETTERTEMPLATE_H__1C4DAFC0_854D_46EB_890C_0F9298A12CB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisciplineLetterTemplate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DisciplineLetterTemplate dialog

class CDisciplineLetterTemplate : public CDialog
{
// Construction
public:
	CDisciplineLetterTemplate(CWnd* pParent = NULL, char* pszFile = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisciplineLetterTemplate)
	enum { IDD = IDD_LETTERTEMPLATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisciplineLetterTemplate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CListBox* pListBoxFILES;
  CEdit*    pEditPREVIEW;

  char *m_pszFile;

	// Generated message map functions
	//{{AFX_MSG(CDisciplineLetterTemplate)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkFiles();
	afx_msg void OnSelchangeFiles();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCIPLINELETTERTEMPLATE_H__1C4DAFC0_854D_46EB_890C_0F9298A12CB5__INCLUDED_)
