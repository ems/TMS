#if !defined(AFX_FORCEPOPI_H__87009D62_B72F_4C83_87E7_54D0FB53BED7__INCLUDED_)
#define AFX_FORCEPOPI_H__87009D62_B72F_4C83_87E7_54D0FB53BED7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ForcePOPI.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CForcePOPI dialog

class CForcePOPI : public CDialog
{
// Construction
public:
	CForcePOPI(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CForcePOPI)
	enum { IDD = IDD_FORCEPOPI };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForcePOPI)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CButton*   pButtonTOCLOSEST;
  CButton*   pButtonTOGARAGE;
  CComboBox* pComboBoxGARAGELIST;

  PDISPLAYINFO m_pDI;

  long GetClosest(long NODESrecordID);
	// Generated message map functions
	//{{AFX_MSG(CForcePOPI)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangeGaragelist();
	afx_msg void OnToclosest();
	afx_msg void OnTogarage();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORCEPOPI_H__87009D62_B72F_4C83_87E7_54D0FB53BED7__INCLUDED_)
