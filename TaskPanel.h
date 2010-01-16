//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#if !defined(AFX_TASKPANEL_H__007E5FF9_A62F_498D_B59D_F53A8C30BD37__INCLUDED_)
#define AFX_TASKPANEL_H__007E5FF9_A62F_498D_B59D_F53A8C30BD37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _XTP_INCLUDE_TASKPANEL
	#define CTaskPanelBase CXTPTaskPanel
#else
	#define CTaskPanelBase CWnd
#endif

class CTaskPanel : public CTaskPanelBase
{
// Construction
public:
	CTaskPanel();

// Attributes
public:
	BOOL Create(CWnd* pParentWnd);

  void OnUpdateReportSelections(CXTPTaskPanelGroup* pGroupReports, CXTPTaskPanelGroup* pGroupDownloads);
  void OnUpdateReportSelections();
	void SetTheme(XTPPaintTheme theme);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskPanel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTaskPanel();

private:
	CXTPTaskPanelGroup* m_pGroupReports;
 	CXTPTaskPanelGroup* m_pGroupDownloads;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTaskPanel)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKPANEL_H__007E5FF9_A62F_498D_B59D_F53A8C30BD37__INCLUDED_)
