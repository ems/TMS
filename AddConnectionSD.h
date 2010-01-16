#if !defined(AFX_ADDCONNECTIONSD_H__F53A12DC_1B44_44CE_BA09_E4C1578507A8__INCLUDED_)
#define AFX_ADDCONNECTIONSD_H__F53A12DC_1B44_44CE_BA09_E4C1578507A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddConnectionSD.h : header file
//

typedef struct AddConnectionSDDataStruct
{
  int  numRecs;
  long recordIDs[96];  // Assumes nothing stupider that 15 minute increments all day
  PDISPLAYINFO pDI;
} AddConnectionSDDataDef;
/////////////////////////////////////////////////////////////////////////////
// CAddConnectionSD dialog

class CAddConnectionSD : public CDialog
{
// Construction
public:
	CAddConnectionSD(CWnd* pParent = NULL, AddConnectionSDDataDef* = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddConnectionSD)
	enum { IDD = IDD_ADDCONNECTIONSD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddConnectionSD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CComboBox* pComboBoxFROMNODE;
  CComboBox* pComboBoxTONODE;
  CEdit*     pEditDISTANCE;
  CButton*   pButtonALLROUTES;
  CComboBox* pComboBoxROUTE;
  CButton*   pButtonALLSERVICES;
  CComboBox* pComboBoxSERVICE;
  CButton*   pButtonALLPATTERNS;
  CComboBox* pComboBoxPATTERN;
  CButton*   pButtonRUNNINGTIME;
  CButton*   pButtonTRAVELTIME;
  CButton*   pButtonDEADHEADTIME;
  CButton*   pButtonONEWAY;
  CButton*   pButtonTWOWAY;
  CSDIGrid*  pSDIGridGRID;
  CButton*   pButtonROUNDUP;
  CButton*   pButtonROUNDDOWN;
  CButton*   pButtonHALFMINUTES;
  CButton*   pButtonFULLMINUTES;
  CButton*   pButtonIDCALCULATE;

  AddConnectionSDDataDef* m_pAddConnectionSDData;

	// Generated message map functions
	//{{AFX_MSG(CAddConnectionSD)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangeFromnode();
	afx_msg void OnSelchangeTonode();
	afx_msg void OnChangeDistance();
	afx_msg void OnAllpatterns();
	afx_msg void OnSelchangePattern();
	afx_msg void OnRoundup();
	afx_msg void OnRounddown();
	afx_msg void OnHalfminutes();
	afx_msg void OnFullminutes();
	afx_msg void OnCalculate();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnAllroutes();
	afx_msg void OnAllservices();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDCONNECTIONSD_H__F53A12DC_1B44_44CE_BA09_E4C1578507A8__INCLUDED_)
