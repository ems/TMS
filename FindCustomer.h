//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_FINDCUSTOMER_H__6C9303DD_1682_4526_B43F_F0B79C4A493D__INCLUDED_)
#define AFX_FINDCUSTOMER_H__6C9303DD_1682_4526_B43F_F0B79C4A493D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindCustomer.h : header file
//

#define FINDCUSTOMER_NUMCOLUMNS 10

typedef struct FCSORTStruct
{
  long recordID;
  char lastName[CUSTOMERS_LASTNAME_LENGTH + 1];
  char firstName[CUSTOMERS_FIRSTNAME_LENGTH + 1];
  char streetAddress[CUSTOMERS_STREETADDRESS_LENGTH + 1];
  char city[CUSTOMERS_CITY_LENGTH + 1];
  char state[CUSTOMERS_STATE_LENGTH + 1];
  char ZIP[CUSTOMERS_ZIP_LENGTH + 1];
  char dayPhone[16];
  char nightPhone[16];
  char faxPhone[16];
  char emailAddress[CUSTOMERS_EMAILADDRESS_LENGTH];
} FCSORTDef;

/////////////////////////////////////////////////////////////////////////////
// CFindCustomer dialog

class CFindCustomer : public CDialog
{
// Construction
public:
//	CFindCustomer(CWnd* pParent = NULL);   // standard constructor
  CFindCustomer(CWnd* pParent = NULL, long* pRecordID = NULL);

// Dialog Data
	//{{AFX_DATA(CFindCustomer)
	enum { IDD = IDD_FINDCUSTOMER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindCustomer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CEdit*     pEditTEXT;
  CListCtrl* pListCtrlLIST;

  long*      m_pRecordID;
  int        m_maxFound;
  FCSORTDef* m_pFCSORT;
  BOOL       m_bAllowSort;

	// Generated message map functions
	//{{AFX_MSG(CFindCustomer)
	afx_msg void OnGo();
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDCUSTOMER_H__6C9303DD_1682_4526_B43F_F0B79C4A493D__INCLUDED_)
