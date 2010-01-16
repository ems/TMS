#if !defined(AFX_DAILYOPSRS_H__C8A98890_4B30_464E_B155_6EC1D5C1E745__INCLUDED_)
#define AFX_DAILYOPSRS_H__C8A98890_4B30_464E_B155_6EC1D5C1E745__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsRS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRS dialog

typedef struct RPStruct
{
  long TRIPSrecordID;
  long NODESrecordID;
  long time;
  long flags;
  long startDRIVERSrecordID;
  long endDRIVERSrecordID;
  long DAILYOPSrecordID;
} RPDef;

#define RPMAX                 500
#define RP_FLAGS_STARTOFPIECE 0x0001
#define RP_FLAGS_ENDOFPIECE   0x0002
#define RP_FLAGS_SPLITATEND   0x0004
#define RP_FLAGS_SPLITATSTART 0x0008
#define RP_FLAGS_SPLITMIDDLE  0x0010
#define RP_FLAGS_INSERTED     0x0020


class CDailyOpsRS : public CDialog
{
// Construction
public:
	CDailyOpsRS(CWnd* pParent = NULL, RunSplitDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsRS)
	enum { IDD = IDD_DAILYOPSRS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsRS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CListCtrl* pListCtrlPOTENTIALSPLITS;
  CListCtrl* pListCtrlCURRENTSPLITS;
  CComboBox* pComboBoxSTARTSPLIT;
  CComboBox* pComboBoxENDSPLIT;
  CButton*   pButtonIDREMOVE;
  CButton*   pButtonIDCANCEL;

  RunSplitDef* m_pPassedData;
  RPDef m_RP[500];
  int   m_firstIndex;
  int   m_lastIndex;
  int   m_PreviousStartPosition;
  int   m_PreviousEndPosition;
  int   m_numRP;
  BOOL  m_bCrewOnly;

  void DisplayPotentials();
  void DisplayCurrents(int startIndex, int endIndex);
  void DisplayDropdowns();

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsRS)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnClickPotentialsplits(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickPotentialsplits(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeStartsplit();
	afx_msg void OnSelchangeEndsplit();
	afx_msg void OnClickCurrentsplits(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPreview();
	afx_msg void OnDelete();
	afx_msg void OnInsert();
	afx_msg void OnRemoveall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSRS_H__C8A98890_4B30_464E_B155_6EC1D5C1E745__INCLUDED_)
