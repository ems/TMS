#if !defined(AFX_ROSTERMERGE_H__E3D2FDDE_E00A_4184_9F6A_5367FB19BAFF__INCLUDED_)
#define AFX_ROSTERMERGE_H__E3D2FDDE_E00A_4184_9F6A_5367FB19BAFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RosterMerge.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRosterMerge dialog

class CRosterMerge : public CDialog
{
// Construction
public:
	CRosterMerge(CWnd* pParent = NULL, int recordIndex = NO_RECORD, long DIVISIONSrecordID = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRosterMerge)
	enum { IDD = IDD_ROSTERMERGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRosterMerge)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CListCtrl* pListCtrlRECORD;
  CListCtrl* pListCtrlWEEK1ROSTER;

  int  m_RecordIndex;
  long m_DIVISIONSrecordID;
	// Generated message map functions
	//{{AFX_MSG(CRosterMerge)
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

#endif // !defined(AFX_ROSTERMERGE_H__E3D2FDDE_E00A_4184_9F6A_5367FB19BAFF__INCLUDED_)
