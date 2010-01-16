//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#if !defined(AFX_MAINFRM_H__04D2F152_F10A_4AC1_AA87_556FC6E4887B__INCLUDED_)
#define AFX_MAINFRM_H__04D2F152_F10A_4AC1_AA87_556FC6E4887B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vcf15.h"
#include "TaskPanel.h"

class CMainFrame : public CXTPMDIFrameWnd, CXTPOffice2007FrameHook
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	static CMainFrame* m_pInstance;

// Attributes
public:
	CImageList		  imaLarge, imaSmall;
  CInfoBar        m_InfoBar;
  CDialogBar      m_dlgBar;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CTaskPanel m_wndTaskPanel;
	CXTPStatusBar  m_wndStatusBar;
  CXTPTaskPanelGroup* m_pPanel;
	int m_nState;
  DWORD m_dwWindowStyle;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar* m_pDlgBar;
  CMenu       m_ContextMenu;
  int  GetAvailableTableSlot();

public:
	UINT m_nIDSaveAs;

private:
	void SetCustomTheme(CXTPPaintManager* pTheme);
	void DockRightOf(CXTPToolBar* pBarToDock, CXTPToolBar* pBarOnLeft);
  void OnUpdateReportSelections();

// Generated message map functions
protected:
	afx_msg void OnInitMenu(CMenu* pMenu);
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnFileDirections();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWindowCloseallwindows();
	afx_msg void OnWindowClosewindow();
	afx_msg void OnClose();
	afx_msg void OnFileRoutes();
	afx_msg void OnFileServices();
	afx_msg void OnFileJurisdictions();
	afx_msg void OnFileDivisions();
	afx_msg void OnFileNodesandstops();
	afx_msg void OnFilePatterns();
	afx_msg void OnFileConnectiontimes();
	afx_msg void OnFileBustypes();
	afx_msg void OnFileBuses();
	afx_msg void OnFileSigncodes();
	afx_msg void OnFileComments();
	afx_msg void OnFileTrips();
	afx_msg void OnFileStandardblocks();
	afx_msg void OnFileDropbackblocks();
	afx_msg void OnFileRuns();
	afx_msg void OnFileCrewonlyruns();
	afx_msg void OnFileDrivertypes();
	afx_msg void OnFileDrivers();
	afx_msg void OnFileDailyoperations();
	afx_msg void OnFileBiddingrostering();
	afx_msg void OnFileDiscipline();
	afx_msg void OnFileTripplanning();
	afx_msg void OnFileComplaints();
	afx_msg void OnFileTimechecks();
	afx_msg void OnFilePreferences();
	//}}AFX_MSG
  int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnCustomize();
	int OnCreateControl(LPCREATECONTROLSTRUCT lpCreateControl);
	int OnCreateCommandBar(LPCREATEBARSTRUCT lpCreatePopup);
	void OnEditState(NMHDR* pNMHDR, LRESULT* pResult);
	void OnFileNew(NMHDR* pNMHDR, LRESULT* pResult);
	void OnUpdateEditState(CCmdUI* pCmdUI);
	void OnTextBelow();
	void OnUpdateTextBelow(CCmdUI* pCmdUI);
	void OnSaveAs(UINT nID);
	void OnUpdateSaveAs(CCmdUI* pCmdUI);
	BOOL OnTaskPanelCaption(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
	void OnUpdateTaskPanelCaption(CCmdUI* pCmdUI);
	LRESULT OnControlRButtonUp(WPARAM, LPARAM lParam);
	long OnOutbarNotify(WPARAM wParam, LPARAM lParam);
	void OnThemeCustom();
	void OnUpdateTheme(CCmdUI* pCmdUI);
	void OnSwitchTheme(UINT nTheme);
	void OnUpdateViewOffice2007Frame(CCmdUI* pCmdUI);
	void OnViewOffice2007Frame();
  void OnChangeRSDRoute(void);
  void OnChangeRSDService(void);
  void OnChangeRSDDivision(void);
  void OnChangeRSDRosterWeek(void);
	void OnUpdateEditCopy(CCmdUI* pCmdUI);
	void OnUpdateAllowUpdate(CCmdUI* pCmdUI);
	void OnUpdateAllowUpdateDelete(CCmdUI* pCmdUI);
	void OnUpdateTableOpen(CCmdUI* pCmdUI);
	void OnUpdateDatabaseOpen(CCmdUI* pCmdUI);
	void OnViewCompressedBlocks();
	void OnUpdateCommandsCopyroster(CCmdUI* pCmdUI);
	void OnUpdateCommandsCopyruncut(CCmdUI* pCmdUI);
  LRESULT OnTaskPanelNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	UINT m_nCurrentTheme;
	BOOL m_bOffice2007Frame;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__04D2F152_F10A_4AC1_AA87_556FC6E4887B__INCLUDED_)
