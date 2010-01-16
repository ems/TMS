#if !defined(AFX_UNPAIDTIMEFILTER_H__2F476FC8_1A4A_419E_A034_9D54D63FCFB2__INCLUDED_)
#define AFX_UNPAIDTIMEFILTER_H__2F476FC8_1A4A_419E_A034_9D54D63FCFB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnpaidTimeFilter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUnpaidTimeFilter dialog

class CUnpaidTimeFilter : public CDialog
{
// Construction
public:
	CUnpaidTimeFilter(CWnd* pParent = NULL, UNPAIDTIMEPassedDataDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnpaidTimeFilter)
	enum { IDD = IDD_UNPAIDTIMEFILTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnpaidTimeFilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  UNPAIDTIMEPassedDataDef* m_pPassedData;

  CDTPicker* pDTPickerFROMDATE;
  CButton*   pButtonALLOPERATORS;
  CButton*   pButtonOPERATORBUTTON;
  CComboBox* pComboBoxOPERATOR;

  void SetupDriverList();

	// Generated message map functions
	//{{AFX_MSG(CUnpaidTimeFilter)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAlloperators();
	afx_msg void OnOperatorbutton();
	afx_msg void OnConsider();
	afx_msg void OnSelendokOperator();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNPAIDTIMEFILTER_H__2F476FC8_1A4A_419E_A034_9D54D63FCFB2__INCLUDED_)
