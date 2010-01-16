#if !defined(AFX_PREMIUMDEFINITION_H__53F94CF7_3CB6_4475_B367_11CCEAEBBBA6__INCLUDED_)
#define AFX_PREMIUMDEFINITION_H__53F94CF7_3CB6_4475_B367_11CCEAEBBBA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PremiumDefinition.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPremiumDefinition dialog

class CPremiumDefinition : public CDialog
{
// Construction
public:
	CPremiumDefinition(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPremiumDefinition)
	enum { IDD = IDD_WORKRULES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPremiumDefinition)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  PDISPLAYINFO m_pDI;

  CButton*   pButtonALLRUNTYPES;
  CButton*   pButtonONERUNTYPE;
  CComboBox* pComboBoxRUNTYPES;
  CListBox*  pListBoxPREMIUMLIST;
  CEdit*     pEditPREMIUMTIME;
  CButton*   pButtonTIMECOUNTSINOT;
  CButton*   pButtonDONTPAY;
  CButton*   pButtonDONTINCORPORATE;
  CButton*   pButtonIDNEWPREMIUM;
  CButton*   pButtonIDSETPREMIUM;

  CButton*   pButtonCLASSIFIEDASREPORTTIME;
  CButton*   pButtonCLASSIFIEDASTRAVELTIME;
  CButton*   pButtonCLASSIFIEDASTURNINTIME;
  CButton*   pButtonCLASSIFIEDASPAIDBREAK;

  CButton*   pButtonTRAVELTIMEONLY;

  CButton*   pButtonACTUALTRAVEL;
  CButton*   pButtonTIMEENTERED;
  CButton*   pButtonASINCONNECTIONS;
  CButton*   pButtonASPERSCHEDULE;
  CButton*   pButtonASPERSCHEDULEDWELL;
  CButton*   pButtonUSECONNIFNOSCHED;

  CButton*   pButtonPAIDTRAVEL;
  CEdit*     pEditPAYFLATMINUTES;
  CEdit*     pEditPERCENTOFACTUAL;
  CEdit*     pEditSTAGGEREDPERCENT;
  CEdit*     pEditSTAGGEREDMINUTES;
  CButton*   pButtonPAYACTUAL;
  CButton*   pButtonPAYFLAT;
  CButton*   pButtonPAYPERCENTAGE;
  CButton*   pButtonPAYSTAGGERED;
  CStatic*   pStaticSTATIC1;
  CStatic*   pStaticSTATIC2;

void ShowPremiums(int selection);
void EnableTravelSection(BOOL bEnable);

	// Generated message map functions
	//{{AFX_MSG(CPremiumDefinition)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnNewpremium();
	afx_msg void OnSetpremium();
	afx_msg void OnSelchangePremiumlist();
	afx_msg void OnDblclkPremiumlist();
	afx_msg void OnOneruntype();
	afx_msg void OnSelchangeRuntypes();
	afx_msg void OnAllruntypes();
	afx_msg void OnUpdate();
	virtual void OnOK();
	afx_msg void OnHelp();
	afx_msg void OnClassifiedasreporttime();
	afx_msg void OnClassifiedastraveltime();
	afx_msg void OnClassifiedasturnintime();
	afx_msg void OnClassifiedaspaidbreak();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREMIUMDEFINITION_H__53F94CF7_3CB6_4475_B367_11CCEAEBBBA6__INCLUDED_)
