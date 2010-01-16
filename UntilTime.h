#if !defined(AFX_UNTILTIME_H__4479F0AB_AC53_4EE3_9E84_97D229F94550__INCLUDED_)
#define AFX_UNTILTIME_H__4479F0AB_AC53_4EE3_9E84_97D229F94550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UntilTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUntilTime dialog

class CUntilTime : public CDialog
{
// Construction
public:
	CUntilTime(CWnd* pParent = NULL, long* pTime = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUntilTime)
	enum { IDD = IDD_UNTILTIME };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUntilTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long* m_pTime;

  CDTPicker* pDTPickerTIME;

	// Generated message map functions
	//{{AFX_MSG(CUntilTime)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNTILTIME_H__4479F0AB_AC53_4EE3_9E84_97D229F94550__INCLUDED_)
