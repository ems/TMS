#if !defined(AFX_ADDHOLIDAY_H__D6BF8FD0_AA00_49F0_9471_903D3443DA40__INCLUDED_)
#define AFX_ADDHOLIDAY_H__D6BF8FD0_AA00_49F0_9471_903D3443DA40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddHoliday.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddHoliday dialog

class CAddHoliday : public CDialog
{
// Construction
public:
	CAddHoliday(CWnd* pParent = NULL, long* pRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddHoliday)
	enum { IDD = IDD_ADDHOLIDAY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddHoliday)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  long* m_pRecordID;

  CDTPicker* pDTPickerDATE;
  CEdit*     pEditDESCRIPTION;
  CListBox*  pListBoxSERVICE;

	// Generated message map functions
	//{{AFX_MSG(CAddHoliday)
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

#endif // !defined(AFX_ADDHOLIDAY_H__D6BF8FD0_AA00_49F0_9471_903D3443DA40__INCLUDED_)
