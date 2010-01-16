#if !defined(AFX_EVENTREPORT_H__6FF42317_2757_4385_8D0E_CD7DE91D401C__INCLUDED_)
#define AFX_EVENTREPORT_H__6FF42317_2757_4385_8D0E_CD7DE91D401C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventReport.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventReport dialog

class CEventReport : public CDialog
{
// Construction
public:
	CEventReport(CWnd* pParent = NULL, EVENTREPORTPassedDataDef *pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEventReport)
	enum { IDD = IDD_EVENTREPORT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventReport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CComboBox* pComboBoxDIVISION;
  CDTPicker* pDTPickerDATE;
  CButton*   pButtonREPORTTIME;
  CButton*   pButtonSTARTOFPIECETRAVEL;
  CButton*   pButtonSTARTOFPIECE;
  CButton*   pButtonENDOFPIECE;
  CButton*   pButtonENDOFPIECETRAVEL;
  CButton*   pButtonEXTRABOARD;
  
  EVENTREPORTPassedDataDef *m_pPassedData;

	// Generated message map functions
	//{{AFX_MSG(CEventReport)
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

#endif // !defined(AFX_EVENTREPORT_H__6FF42317_2757_4385_8D0E_CD7DE91D401C__INCLUDED_)
