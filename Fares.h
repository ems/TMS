#if !defined(AFX_FARES_H__79E636A1_AC81_401E_953F_6A4737A3E3A7__INCLUDED_)
#define AFX_FARES_H__79E636A1_AC81_401E_953F_6A4737A3E3A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Fares.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFares dialog

class CFares : public CDialog
{
// Construction
public:
	CFares(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFares)
	enum { IDD = IDD_FARES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFares)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CSDIGrid*  pSDIGridTYPES;
  CSDIGrid*  pSDIGridFARES;

	// Generated message map functions
	//{{AFX_MSG(CFares)
	virtual BOOL OnInitDialog();
	afx_msg void OnEstablish();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnPopulate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FARES_H__79E636A1_AC81_401E_953F_6A4737A3E3A7__INCLUDED_)
