#if !defined(AFX_ADDEXTRATRIP_H__9663CED9_99BA_454C_B84B_829808A9F8C6__INCLUDED_)
#define AFX_ADDEXTRATRIP_H__9663CED9_99BA_454C_B84B_829808A9F8C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddExtraTrip.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddExtraTrip dialog

class CAddExtraTrip : public CDialog
{
// Construction
public:
	CAddExtraTrip(CWnd* pParent = NULL, DAILYOPSDef* pDAILYOPS = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddExtraTrip)
	enum { IDD = IDD_ADDEXTRATRIP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddExtraTrip)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  DAILYOPSDef *m_pDAILYOPS;
  long m_ROUTESrecordID;
  long m_SERVICESrecordID;
  long m_directionIndex;
  long m_PATTERNNAMESrecordID;
  int  m_MLPIndex;

  void PopulatePatternName();

  CComboBox* pComboBoxROUTE;
  CComboBox* pComboBoxSERVICE;
  CButton*   pButtonOUTBOUND;
  CButton*   pButtonINBOUND;
  CComboBox* pComboBoxPATTERNNAME;
  CListBox*  pListBoxPATTERNNODES;
  CEdit*     pEditTIMEATMLP;
  CEdit*     pEditDEADHEADS;

	// Generated message map functions
	//{{AFX_MSG(CAddExtraTrip)
	virtual BOOL OnInitDialog();
	afx_msg void OnOutbound();
	afx_msg void OnInbound();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelchangeRoute();
	afx_msg void OnSelchangeService();
	afx_msg void OnSelchangePatternname();
	afx_msg void OnSelchangePatternnodes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDEXTRATRIP_H__9663CED9_99BA_454C_B84B_829808A9F8C6__INCLUDED_)
