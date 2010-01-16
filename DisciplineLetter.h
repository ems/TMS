#if !defined(AFX_DISCIPLINELETTER_H__9229BC56_A609_4813_8EDB_27BFCED22A0D__INCLUDED_)
#define AFX_DISCIPLINELETTER_H__9229BC56_A609_4813_8EDB_27BFCED22A0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisciplineLetter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisciplineLetter dialog

class CDisciplineLetter : public CDialog
{
// Construction
public:
	CDisciplineLetter(CWnd* pParent = NULL, long* pDRIVERSrecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisciplineLetter)
	enum { IDD = IDD_LETTERPREVIEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisciplineLetter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CEdit* pEditTEXT;

  long *m_pDRIVERSrecordID;

	// Generated message map functions
	//{{AFX_MSG(CDisciplineLetter)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnTemplate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCIPLINELETTER_H__9229BC56_A609_4813_8EDB_27BFCED22A0D__INCLUDED_)
