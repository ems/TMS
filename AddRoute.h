#if !defined(AFX_ADDROUTE_H__3350B95B_53CC_45F5_98E5_BC87C4BA754E__INCLUDED_)
#define AFX_ADDROUTE_H__3350B95B_53CC_45F5_98E5_BC87C4BA754E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddRoute.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddRoute dialog

class CAddRoute : public CDialog
{
// Construction
public:
	CAddRoute(CWnd* pParent = NULL, long *pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddRoute)
	enum { IDD = IDD_ADDROUTE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddRoute)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CEdit*     pEditNUMBER;
  CEdit*     pEditNAME;
  CButton*   pButtonBIDIRECTIONAL;
  CButton*   pButtonCIRCULAR;
  CComboBox* pComboBoxOUTBOUND;
  CComboBox* pComboBoxINBOUND;
  CButton*   pButtonSHUTTLE;
  CButton*   pButtonSCHOOL;
  CButton*   pButtonRGRP;
  CButton*   pButtonOBBAYNA;
  CButton*   pButtonOBBAY;
  CComboBox* pComboBoxOBBAYLIST;
  CButton*   pButtonIBGROUPBOX;
  CButton*   pButtonIBBAYNA;
  CButton*   pButtonIBBAY;
  CComboBox* pComboBoxIBBAYLIST;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;

  long* m_pUpdateRecordID;

	// Generated message map functions
	//{{AFX_MSG(CAddRoute)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBidirectional();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnCircular();
	afx_msg void OnIbbayna();
	afx_msg void OnIbbay();
	afx_msg void OnSelendokIbbaylist();
	afx_msg void OnObbayna();
	afx_msg void OnObbay();
	afx_msg void OnSelendokObbaylist();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDROUTE_H__3350B95B_53CC_45F5_98E5_BC87C4BA754E__INCLUDED_)
