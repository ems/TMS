#if !defined(AFX_PEGBOARDCOLORS_H__C8C0734E_10CD_4366_A581_5B14B4B034A7__INCLUDED_)
#define AFX_PEGBOARDCOLORS_H__C8C0734E_10CD_4366_A581_5B14B4B034A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PegboardColors.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPegboardColors dialog

class CPegboardColors : public CDialog
{
// Construction
public:
	CPegboardColors(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPegboardColors)
	enum { IDD = IDD_DAILYOPSPC };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPegboardColors)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CColorBtn m_cbAvailable;
  CColorBtn m_cbAssigned;
  CColorBtn m_cbOutOfService;
  CColorBtn m_cbShortShift;
  CColorBtn m_cbCharterReserve;
  CColorBtn m_cbSightseeingReserve;
	// Generated message map functions
	//{{AFX_MSG(CPegboardColors)
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

#endif // !defined(AFX_PEGBOARDCOLORS_H__C8C0734E_10CD_4366_A581_5B14B4B034A7__INCLUDED_)
