#if !defined(AFX_PLACEHOLDER_H__BE5D46BD_A352_49CD_99AF_D5DBC8F91660__INCLUDED_)
#define AFX_PLACEHOLDER_H__BE5D46BD_A352_49CD_99AF_D5DBC8F91660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlaceHolder.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlaceHolder dialog

class CPlaceHolder : public CDialog
{
// Construction
public:
	CPlaceHolder(CWnd* pParent = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlaceHolder)
	enum { IDD = IDD_PLACEHOLDER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaceHolder)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CEdit*     pEditHOWMANY;
  CComboBox* pComboBoxSTARTNODE;
  CComboBox* pComboBoxENDNODE;
  CEdit*     pEditSTARTAT;
  CEdit*     pEditINCREMENT;

  long*  m_pUpdateRecordID;

	// Generated message map functions
	//{{AFX_MSG(CPlaceHolder)
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

#endif // !defined(AFX_PLACEHOLDER_H__BE5D46BD_A352_49CD_99AF_D5DBC8F91660__INCLUDED_)
