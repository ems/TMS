#if !defined(AFX_ADDDRIVER04_H__84EC5D17_A7D2_42F7_8013_47E50E4F24B3__INCLUDED_)
#define AFX_ADDDRIVER04_H__84EC5D17_A7D2_42F7_8013_47E50E4F24B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDriver04.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDriver04 dialog

class CAddDriver04 : public CPropertyPage
{
	DECLARE_DYNCREATE(CAddDriver04)

// Construction
public:
	CAddDriver04();
	~CAddDriver04();

// Dialog Data
	//{{AFX_DATA(CAddDriver04)
	enum { IDD = IDD_ADDDRIVER04 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAddDriver04)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAddDriver04)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDRIVER04_H__84EC5D17_A7D2_42F7_8013_47E50E4F24B3__INCLUDED_)
