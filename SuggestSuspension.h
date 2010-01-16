#if !defined(AFX_SUGGESTSUSPENSION_H__B1AEC0B4_D668_4716_A48A_788D713DBBCB__INCLUDED_)
#define AFX_SUGGESTSUSPENSION_H__B1AEC0B4_D668_4716_A48A_788D713DBBCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SuggestSuspension.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSuggestSuspension dialog

class CSuggestSuspension : public CDialog
{
// Construction
public:
	CSuggestSuspension(CWnd* pParent = NULL, long* pDRIVERSrecordID = NULL, long* pStartDate = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSuggestSuspension)
	enum { IDD = IDD_SUGGESTSUSPENSION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuggestSuspension)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  BOOL IsHeAwayToday(long date);

  CListCtrl* pListCtrlLIST;

  long *m_pStartDate;
  long *m_pDRIVERSrecordID;

	// Generated message map functions
	//{{AFX_MSG(CSuggestSuspension)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUGGESTSUSPENSION_H__B1AEC0B4_D668_4716_A48A_788D713DBBCB__INCLUDED_)
