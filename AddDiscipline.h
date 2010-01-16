#if !defined(AFX_ADDDISCIPLINE_H__FA7E6FB2_FEB7_4ABB_AC5C_789DDE745AE4__INCLUDED_)
#define AFX_ADDDISCIPLINE_H__FA7E6FB2_FEB7_4ABB_AC5C_789DDE745AE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDiscipline.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDiscipline dialog

class CAddDiscipline : public CDialog
{
// Construction
public:
	CAddDiscipline(CWnd* pParent = NULL,
        long* pDRIVERSrecordID = NULL, long* pUpdateRecordID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDiscipline)
	enum { IDD = IDD_ADDDISCIPLINE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDiscipline)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton*   pButtonDRIVER;
  CDTPicker* pDTPickerENTRYDATE;
  CDTPicker* pDTPickerENTRYTIME;
  CDTPicker* pDTPickerOFFENCEDATE;
  CDTPicker* pDTPickerOFFENCETIME;
  CComboBox* pComboBoxREPORTEDBY;
  CComboBox* pComboBoxCATEGORY;
  CComboBox* pComboBoxVIOLATION;
  CListCtrl* pListCtrlACTIONLIST;
  CComboBox* pComboBoxACTUALACTION;
  CEdit*     pEditCOMMENTS;
  CDTPicker* pDTPickerSUSPENSIONDATE;
  CButton*   pButtonNOSUSPENSION;
  CButton*   pButtonDEFERSUSPENSION;
  CButton*   pButtonSUSPENSIONDETAILS;
  CEdit*     pEditDURATION;
  CDTPicker* pDTPickerDROPOFFDATE;
  CButton*   pButtonNA;
  CButton*   pButtonCC;
  CComboBox* pComboBoxCOMMENTCODE;
  CButton*   pButtonSUGGEST;
  CButton*   pButtonHOURS;
  CButton*   pButtonDAYS;
  CButton*   pButtonSUSPENDEDWITHPAY;

  long* m_pUpdateRecordID;
  long* m_pDRIVERSrecordID;

	// Generated message map functions
	//{{AFX_MSG(CAddDiscipline)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnSelchangeCategory();
	virtual void OnOK();
	afx_msg void OnSelchangeViolation();
	afx_msg void OnClickActionlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNosuspension();
	afx_msg void OnCc();
	afx_msg void OnNa();
	afx_msg void OnSelendokCommentcode();
	afx_msg void OnLetter();
	afx_msg void OnSelendokActualaction();
	afx_msg void OnSuggest();
	afx_msg void OnDefersuspension();
	afx_msg void OnSuspensiondetails();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDISCIPLINE_H__FA7E6FB2_FEB7_4ABB_AC5C_789DDE745AE4__INCLUDED_)
