#if !defined(AFX_DATERANGE_H__05AC7499_75AC_4017_ACD6_674D31CD3C20__INCLUDED_)
#define AFX_DATERANGE_H__05AC7499_75AC_4017_ACD6_674D31CD3C20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DateRange.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDateRange dialog

class CDateRange : public CDialog
{
// Construction
public:
	CDateRange(CWnd* pParent = NULL, long* pFromDate = NULL, long* pToDate = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDateRange)
	enum { IDD = IDD_DATERANGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDateRange)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerTODATE;

  long* m_pFromDate;
  long* m_pToDate;

  void SetDateToToday(CDTPicker* pDTP);

	// Generated message map functions
	//{{AFX_MSG(CDateRange)
	virtual BOOL OnInitDialog();
	afx_msg void OnFromtoday();
	afx_msg void OnTotoday();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATERANGE_H__05AC7499_75AC_4017_ACD6_674D31CD3C20__INCLUDED_)
