#if !defined(AFX_AUDITFILTER_H__B81A2D0C_B77D_4363_9F73_A3A435E27E9E__INCLUDED_)
#define AFX_AUDITFILTER_H__B81A2D0C_B77D_4363_9F73_A3A435E27E9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AuditFilter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAuditFilter dialog

class CAuditFilter : public CDialog
{
// Construction
public:
	CAuditFilter(CWnd* pParent = NULL, AUDITFILTERSELECTIONSDef *pSelections = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAuditFilter)
	enum { IDD = IDD_AUDITFILTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuditFilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  AUDITFILTERSELECTIONSDef *m_pSelections;
  CButton*   pButtonREGISTRATIONDATE;
  CButton*   pButtonOCCURRENCEDATE;
  CDTPicker* pDTPickerFROMDATE;
  CStatic*   pStaticUNTIL_TITLE;
  CDTPicker* pDTPickerTODATE;
  CButton*   pButtonFROMTODAY;
  CButton*   pButtonTOTODAY;
  CButton*   pButtonALL;
  CButton*   pButtonROSTER;
  CButton*   pButtonVEHICLEASSIGNMENT;
  CButton*   pButtonOPERATORCHECKINCHECKOUT;
  CButton*   pButtonOPERATORABSENCE;
  CButton*   pButtonOPENWORK;
  CButton*   pButtonALLOPERATORS;
  CButton*   pButtonOPERATORBUTTON;
  CComboBox* pComboBoxOPERATOR;
  CButton*   pButtonALLREASONS;
  CButton*   pButtonREASONBUTTON;
  CComboBox* pComboBoxREASON;

  void EnableOtherData(BOOL bEnable);
  void SetDateToToday(CDTPicker* pDTP);

	// Generated message map functions
	//{{AFX_MSG(CAuditFilter)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnAll();
	afx_msg void OnRoster();
	afx_msg void OnVehicleassignment();
	afx_msg void OnOperatorcheckincheckout();
	afx_msg void OnOperatorabsence();
	afx_msg void OnOpenwork();
	afx_msg void OnAlloperators();
	afx_msg void OnOperatorbutton();
	afx_msg void OnAllreasons();
	afx_msg void OnReasonbutton();
	afx_msg void OnSelchangeOperator();
	afx_msg void OnSelchangeReason();
	afx_msg void OnFromtoday();
	afx_msg void OnTotoday();
	afx_msg void OnRegistrationdate();
	afx_msg void OnOccurrencedate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDITFILTER_H__B81A2D0C_B77D_4363_9F73_A3A435E27E9E__INCLUDED_)
