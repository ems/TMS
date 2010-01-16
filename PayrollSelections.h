#if !defined(AFX_PAYROLLSELECTIONS_H__D83EADFF_D885_4F82_A480_0390CC681DC3__INCLUDED_)
#define AFX_PAYROLLSELECTIONS_H__D83EADFF_D885_4F82_A480_0390CC681DC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PayrollSelections.h : header file
//

#define PAYROLLSELECTIONS_FLAG_DAILY          0x0001
#define PAYROLLSELECTIONS_FLAG_PERIOD         0x0002
#define PAYROLLSELECTIONS_FLAG_CRYSTAL        0x0004
#define PAYROLLSELECTIONS_FLAG_TABDELIMITED   0x0008
#define PAYROLLSELECTIONS_FLAG_CSV            0x0010
#define PAYROLLSELECTIONS_FLAG_XML            0x0020
#define PAYROLLSELECTIONS_FLAG_CODES          0x0040
#define PAYROLLSELECTIONS_FLAG_COMBINEOUTPUT  0x0080
#define PAYROLLSELECTIONS_FLAG_SHOWRUNDETAILS 0x0100

typedef struct PayrollSelectionsStruct
{
  int   numSelected;
  long *pSelectedRecordIDs;
  long  startDateYYYYMMDD;
  int   daysInPeriod;
  int   startingRosterWeek;
  int   lastRosterWeek;
  char  delimiter;
  long  flags;
} PayrollSelectionsDef;

/////////////////////////////////////////////////////////////////////////////
// CPayrollSelections dialog

class CPayrollSelections : public CDialog
{
// Construction
public:
	CPayrollSelections(CWnd* pParent = NULL, PayrollSelectionsDef* pPS = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPayrollSelections)
	enum { IDD = IDD_PAYROLLSELECTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPayrollSelections)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton*   pButtonDAILYREPORT;
  CButton*   pButtonPERIODREPORT;
  CStatic*   pStaticDAYSINPERIOD_TEXT;
  CEdit*     pEditDAYSINPERIOD;
  CButton*   pButtonCRYSTAL;
  CButton*   pButtonCRYSTAL_TITLE;
  CButton*   pButtonSHOWRUNDETAILS;
  CButton*   pButtonFILE;
  CButton*   pButtonFILE_TITLE;
  CButton*   pButtonTABDELIMITED;
  CButton*   pButtonCSV;
  CEdit*     pEditDELIMITEDBY;
  CButton*   pButtonXML;
  CListBox*  pListBoxOPERATORLIST;
  CDTPicker* pDTPickerDATE;
  CEdit*     pEditSTARTINGROSTERWEEK;
  CEdit*     pEditLASTROSTERWEEK;
  CButton*   pButtonINCLUDECODES;
  CButton*   pButtonCOMBINEOUTPUT;

  PayrollSelectionsDef* m_pPS;

  void PopulateOperatorList();

	// Generated message map functions
	//{{AFX_MSG(CPayrollSelections)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnCrystal();
	afx_msg void OnFile();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnConsider();
	afx_msg void OnSelectall();
	afx_msg void OnChangeDaysinperiod();
	afx_msg void OnDailyreport();
	afx_msg void OnPeriodreport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAYROLLSELECTIONS_H__D83EADFF_D885_4F82_A480_0390CC681DC3__INCLUDED_)
