#if !defined(AFX_RECOST_H__28067F38_3F50_49ED_A10F_1DA45486CD58__INCLUDED_)
#define AFX_RECOST_H__28067F38_3F50_49ED_A10F_1DA45486CD58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Recost.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecost dialog

class CRecost : public CDialog
{
// Construction
public:
	CRecost(CWnd* pParent = NULL, OPENWORKDef* pOpenWork = NULL, long PertainsToDate = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRecost)
	enum { IDD = IDD_RECOST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecost)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton*   pButtonAUTOMATIC;
  CButton*   pButtonRUNTYPE;
  CComboBox* pComboBoxRUNTYPELIST;
  CButton*   pButtonVALUE;
  CEdit*     pEditVALUEDATA;

  OPENWORKDef* m_pOpenWork;
  long         m_PertainsToDate;

	// Generated message map functions
	//{{AFX_MSG(CRecost)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnRuntype();
	afx_msg void OnValue();
	virtual void OnOK();
	afx_msg void OnAutomatic();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECOST_H__28067F38_3F50_49ED_A10F_1DA45486CD58__INCLUDED_)
