#if !defined(AFX_PICKDATE_H__8A6ABE33_9F39_4EC1_8543_AC388DA1FF58__INCLUDED_)
#define AFX_PICKDATE_H__8A6ABE33_9F39_4EC1_8543_AC388DA1FF58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PickDate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPickDate dialog

class CPickDate : public CDialog
{
// Construction
public:
	CPickDate(CWnd* pParent = NULL, long* pDate = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPickDate)
	enum { IDD = IDD_PICKDATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickDate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CDTPicker* pDTPickerDATE;

  long* m_pDate;

	// Generated message map functions
	//{{AFX_MSG(CPickDate)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICKDATE_H__8A6ABE33_9F39_4EC1_8543_AC388DA1FF58__INCLUDED_)
