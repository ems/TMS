#if !defined(AFX_ROSTERORDER_H__73F6BF06_D63B_4474_8A91_72AD200CAC4E__INCLUDED_)
#define AFX_ROSTERORDER_H__73F6BF06_D63B_4474_8A91_72AD200CAC4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RosterOrder.h : header file
//

typedef struct DIVISIONDATAStruct
{
  int  numDivisions;
  long DIVISIONSrecordIDs[ROSTER_MAX_WEEKS + 1];
  char name[ROSTER_MAX_WEEKS + 1][DIVISIONS_NAME_LENGTH + 1];
} DIVISIONDATADef;
      

/////////////////////////////////////////////////////////////////////////////
// CRosterOrder dialog

class CRosterOrder : public CDialog
{
// Construction
public:
	CRosterOrder(CWnd* pParent = NULL, DIVISIONDATADef* pData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRosterOrder)
	enum { IDD = IDD_ROSTERORDER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRosterOrder)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  DIVISIONDATADef* m_pData;

  CStatic* pStaticWEEK_TITLE[ROSTER_MAX_WEEKS];
  CComboBox* pComboBoxPRIMARY;
  CComboBox* pComboBoxWEEK[ROSTER_MAX_WEEKS];

	// Generated message map functions
	//{{AFX_MSG(CRosterOrder)
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

#endif // !defined(AFX_ROSTERORDER_H__73F6BF06_D63B_4474_8A91_72AD200CAC4E__INCLUDED_)
