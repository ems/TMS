#if !defined(AFX_DAILYOPSSO_H__08AAC9B0_C6A9_441D_A6A3_DBD7B87DC38B__INCLUDED_)
#define AFX_DAILYOPSSO_H__08AAC9B0_C6A9_441D_A6A3_DBD7B87DC38B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsSO.h : header file
//


typedef struct AvailableDriversStruct
{
  long DRIVERSrecordID;
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 2 + DRIVERS_BADGENUMBER_LENGTH + 1 + 1];
  char szDriverType[DRIVERTYPES_NAME_LENGTH + 1];
  long seniorityDate;
  long senioritySort;
  long rosteredTime;
  long openWorkTime;
  long totalTime;  // Just 'cause it's a sort field
  long flags;
  char szFlags[64];
  long time[2];
  long runNumber;
  int  extraboardSequence;
} AvailableDriversDef;

#define DAILYOPSSO_FLAGS_RUNSPLIT   0x0001
#define DAILYOPSSO_FLAGS_SHOWHOURS  0x0002

#define DAILYOPSSO_MAXAVILABLEDRIVERS  250

typedef struct DailyOpsSOPassedDataStruct
{
  RosteredDriversDef* pRD;
  int  numRD;
  long date;
  long runNumber;
  long runStartTime;
  long runEndTime;
  long payTime;
  long flags;
  long selectedOperator;
  ABSENTDef* pAbsentList;
  int  numAbsent;

} DailyOpsSOPassedDataDef;

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSO dialog

class CDailyOpsSO : public CDialog
{
// Construction
public:
	CDailyOpsSO(CWnd* pParent = NULL, 
              DailyOpsSOPassedDataDef* pPassedData = NULL);// standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsSO)
	enum { IDD = IDD_DAILYOPSSO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsSO)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void DisplayAvailableDrivers();
  void DetermineServiceDay(long date, BOOL bNeedDateWithService);
  void EstablishOpenWork(long startDate);
  void CalculateAssignedTime(long startDate);

  AvailableDriversDef m_AvailableDrivers[DAILYOPSSO_MAXAVILABLEDRIVERS];
  DailyOpsSOPassedDataDef* m_pPassedData;

  int   m_Index;
  int   num_AvailableDrivers;
  OPENWORKDef m_OpenWork[7][250];
  int  m_numInOpenWork[7];
  OPENWORKDef m_Assignments[7][250];
  int  m_numAssignments[7];
  long m_Today;
  long m_SERVICESrecordIDInEffect;
  int  m_NextDaytimeIndex;
  int  m_NextNighttimeIndex;

  CStatic*   pStaticDESCRIPTION;
  CListCtrl* pListCtrlLIST;
  CButton*   pButtonSHOWINELIGIBLE;

	// Generated message map functions
	//{{AFX_MSG(CDailyOpsSO)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPrint();
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowineligible();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSSO_H__08AAC9B0_C6A9_441D_A6A3_DBD7B87DC38B__INCLUDED_)
