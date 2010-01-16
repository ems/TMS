//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_ADDCONNECTION_H__4191E1F3_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
#define AFX_ADDCONNECTION_H__4191E1F3_0926_11D6_A22D_0080C7294BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddConnection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddConnection dialog

class CAddConnection : public CDialog
{
// Construction
public:     // standard constructor
	CAddConnection(CWnd* pParent = NULL, long * pUpdateRecordID = NULL, PDISPLAYINFO pDI = NULL);

// Dialog Data
	//{{AFX_DATA(CAddConnection)
	enum { IDD = IDD_ADDCONNECTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddConnection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  PDISPLAYINFO m_pDI;
  BOOL         m_bInit;
  long         *m_pUpdateRecordID;
  int          m_numServices;
  int          m_numRoutes;

  void       DisplayActiveRoute(CComboBox *pCB, long routeRecordID);
  void       DisplayActiveService(CComboBox *pCB, long serviceRecordID);
  BOOL       DisplayActivePattern(CComboBox *pCBRte, CComboBox *pCBSer,
                   CComboBox *pCBPat, long patternnamesRecordID, int controlID); 

  CComboBox* pComboBoxFROMNODE;
  CComboBox* pComboBoxTONODE;
  CEdit*     pEditCONNECTIONTIME;
  CEdit*     pEditDISTANCE;
  CButton*   pButtonAPPLYTOALL;
  CButton*   pButtonFROMALLROUTES;
  CComboBox* pComboBoxFROMROUTE;
  CButton*   pButtonFROMALLSERVICES;
  CComboBox* pComboBoxFROMSERVICE;
  CButton*   pButtonFROMALLPATTERNS;
  CComboBox* pComboBoxFROMPATTERN;
  CButton*   pButtonTOALLROUTES;
  CComboBox* pComboBoxTOROUTE;
  CButton*   pButtonTOALLSERVICES;
  CComboBox* pComboBoxTOSERVICE;
  CButton*   pButtonTOALLPATTERNS;
  CComboBox* pComboBoxTOPATTERN;
  CEdit*     pEditFROMTIMEOFDAY;
  CEdit*     pEditTOTIMEOFDAY;
  CButton*   pButtonONEWAY;
  CButton*   pButtonTWOWAY;
  CButton*   pButtonRUNNINGTIME;
  CButton*   pButtonTRAVELTIME;
  CButton*   pButtonDEADHEADTIME;
  CButton*   pButtonEQUIV;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;
  CStatic*   pStaticCOUNTER;

	// Generated message map functions
	//{{AFX_MSG(CAddConnection)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnFromallpatterns();
	afx_msg void OnFromallroutes();
	afx_msg void OnFromallservices();
	afx_msg void OnToallpatterns();
	afx_msg void OnToallroutes();
	afx_msg void OnToallservices();
	virtual void OnOK();
	afx_msg void OnEditchangeFromroute();
	afx_msg void OnEditchangeFromservice();
	afx_msg void OnEditchangeToroute();
	afx_msg void OnEditchangeToservice();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDCONNECTION_H__4191E1F3_0926_11D6_A22D_0080C7294BDC__INCLUDED_)
