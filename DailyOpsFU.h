#if !defined(AFX_DAILYOPSFU_H__C4128442_0C40_4934_B086_828ACE4160B5__INCLUDED_)
#define AFX_DAILYOPSFU_H__C4128442_0C40_4934_B086_828ACE4160B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOpsFU.h : header file
//

typedef struct DAILYOPSFUPassedDataStruct
{
  long flags;
  long date;
  long returnValue;
  int  numInBlockInfo;
  BlockInfoDef *BlockInfo;
  int  numInOpenWork;
  OPENWORKDef *OpenWork;
} DAILYOPSFUPassedDataDef;

#define DAILYOPSFU_FLAG_CHANGEDATE     0x0001
#define DAILYOPSFU_FLAG_CHANGECONSIDER 0x0002
#define DAILYOPSFU_FLAG_CHANGEWEEK     0x0004
#define DAILYOPSFU_FLAG_CHANGECOLOR    0x0008

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsFU dialog

class CDailyOpsFU : public CDialog
{
// Construction
public:
	CDailyOpsFU(CWnd* pParent = NULL, DAILYOPSFUPassedDataDef* pPassedData = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDailyOpsFU)
	enum { IDD = IDD_DAILYOPSFU };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOpsFU)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  DAILYOPSFUPassedDataDef* m_pPassedData;
	// Generated message map functions
	//{{AFX_MSG(CDailyOpsFU)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSetdate();
	afx_msg void OnAudittrail();
	afx_msg void OnHolidays();
	afx_msg void OnFinished();
	afx_msg void OnConsider();
	afx_msg void OnRosterweek();
	afx_msg void OnRidership();
	afx_msg void OnFares();
	afx_msg void OnColors();
	afx_msg void OnCheckinlocations();
	afx_msg void OnExtraboard();
	afx_msg void OnPayrolllayout();
	afx_msg void OnGenerateavldata();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPSFU_H__C4128442_0C40_4934_B086_828ACE4160B5__INCLUDED_)
