#if !defined(AFX_ROSTERPARMS_H__FB1110A1_6158_4CCE_8F42_F87FF689E32C__INCLUDED_)
#define AFX_ROSTERPARMS_H__FB1110A1_6158_4CCE_8F42_F87FF689E32C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RosterParms.h : header file
//

#define MAXRUNTYPES NUMRUNTYPES * NUMRUNTYPESLOTS

/////////////////////////////////////////////////////////////////////////////
// CRosterParms dialog

class CRosterParms : public CDialog
{
// Construction
public:
	CRosterParms(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRosterParms)
	enum { IDD = IDD_ROSTERPARMS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRosterParms)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CButton*  pButtonSAMEROUTE;
  CButton*  pButtonSAMERUNNUMBER;
  CButton*  pButtonSAMESTART;
  CButton*  pButtonSAMEEND;
  CButton*  pButtonSAMERUNTYPE;
  CButton*  pButtonDIFFRUNTYPE;
  CButton*  pButtonRUNTYPERULES;
  CListBox* pListBoxRUNTYPE;
  CStatic*  pStaticCOMBINETITLE;
  CListBox* pListBoxCOMBINE;
  CEdit*    pEditSTARTTIME;
  CEdit*    pEditOFFTIME;
  CEdit*    pEditWORKWEEK;
  CButton*  pButtonBIDBYDRIVERTYPE;
  CEdit*    pEditDAYSOFFPATTERNSTEXT;
  CButton*  pButtonIDEDIT;
  CButton*  pButtonEQUALIZE;
  CButton*  pButtonFORCEEXTRASTOWEEKDAYS;
  CButton*  pButtonFORCEEXTRASTOWEEKENDS;
  CEdit*    pEditMINWORK;
  CEdit*    pEditMAXWORK;
  CButton*  pButtonSTARTSAFTER;
  CButton*  pButtonENDSAFTER;
  CEdit*    pEditNIGHTWORKAFTER;
  CEdit*    pEditMAXNIGHTWORK;
  CButton*  pButtonOFFDAYMUSTFOLLOW;

  char   m_LocalMatch[NUMRUNTYPES][NUMRUNTYPESLOTS][NUMRUNTYPESLOTS];
  char	 m_DaysOffPatterns[128];	// Temporary storage for days off patterns.

	// Generated message map functions
	//{{AFX_MSG(CRosterParms)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSameruntype();
	afx_msg void OnDiffruntype();
	afx_msg void OnRuntyperules();
	afx_msg void OnSelchangeRuntype();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnEdit();
	afx_msg void OnSelchangeCombine();
	afx_msg void OnChangeWorkweek();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROSTERPARMS_H__FB1110A1_6158_4CCE_8F42_F87FF689E32C__INCLUDED_)
