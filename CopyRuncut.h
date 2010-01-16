#if !defined(AFX_COPYRUNCUT_H__50333F7A_571E_4A30_8B83_B0BB6DE6D754__INCLUDED_)
#define AFX_COPYRUNCUT_H__50333F7A_571E_4A30_8B83_B0BB6DE6D754__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyRuncut.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyRuncut dialog

class CCopyRuncut : public CDialog
{
// Construction
public:
	CCopyRuncut(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyRuncut)
	enum { IDD = IDD_COPYRUNCUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyRuncut)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CStatic* pStaticDESTINATION;
  CComboBox* pComboBoxSERVICE;
  CComboBox* pComboBoxDIVISION;

	// Generated message map functions
	//{{AFX_MSG(CCopyRuncut)
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

#endif // !defined(AFX_COPYRUNCUT_H__50333F7A_571E_4A30_8B83_B0BB6DE6D754__INCLUDED_)
