#if !defined(AFX_DAILYOPSSS_H__F035C08B_DD6C_40BF_9721_AF4CB2DAD0FE__INCLUDED_)
#define AFX_DAILYOPSSS_H__F035C08B_DD6C_40BF_9721_AF4CB2DAD0FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsSS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSS dialog

class CDailyOpsSS : public CDialog
{
// Construction
public:
	CDailyOpsSS(CWnd* pParent = NULL, long DRIVERSrecordID = NO_RECORD, unsigned short int flags = 0, BOOL* pbChanged = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsSS)
	enum { IDD = IDD_DAILYOPSSS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsSS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void DisplayAbsenceRow(int iItem, DAILYOPSDef* pDAILYOPS);
  void Display();

  long  m_DRIVERSrecordID;
  long  m_StartDate;
  BOOL* m_pbChanged;
  unsigned short int m_RecordTypeFlag;

  CListCtrl* pListCtrlLIST;
  CButton*   pButtonIDREMOVE;
  CButton*   pButtonIDUPDATE;
  CButton*   pButtonJUSTFROMTHELASTYEAR;
  CButton*   pButtonALL;

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsSS)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRemove();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdate();
	afx_msg void OnJustfromthelastyear();
	afx_msg void OnAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSSS_H__F035C08B_DD6C_40BF_9721_AF4CB2DAD0FE__INCLUDED_)
