//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_VISUALRUNCUTTER_H__9CD9A5F4_FF4C_478E_B438_79B96CEEDB87__INCLUDED_)
#define AFX_VISUALRUNCUTTER_H__9CD9A5F4_FF4C_478E_B438_79B96CEEDB87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VisualRuncutter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVisualRuncutter dialog

class CVisualRuncutter : public CResizingDialog
{
// Construction
public:
//	CVisualRuncutter(CWnd* pParent = NULL);   // standard constructor
  CVisualRuncutter(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);

// Dialog Data
	//{{AFX_DATA(CVisualRuncutter)
	enum { IDD = IDD_VR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVisualRuncutter)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//
//  Functions
//
  void Cleanup(BOOL bSaveAndDestroy);
  void MoveCutRunsWindow();
  void OnDisplayrunswindow();
  void SaveCutRunsRect();
  void SaveDialogRect();
  void EstablishViewingArea();
  BOOL SaveRuncut();
  void HighlightBlock(CPaintDC *pDC, int color, int start, int end);
  void UpdateDisplayStatus();
//
//  Passed display information
//
  PDISPLAYINFO m_pDI;
//
//  Controls
//
  CFlatSB* pFlatSBSCROLLBAR;
  CButton* pButtonOK;
  CButton* pButtonSAVE;
  CButton* pButtonCANCEL;
  CButton* pButtonHELP;
  CStatic* pStaticDISPLAYSTATUS;
//
//  Colors
//
  VRCOLORSDef m_VRColors;
//
//  The others
//

#define VR_FLAG_ALLBLOCKS           0x0001
#define VR_FLAG_SUBSET              0x0002
#define VR_FLAG_ILLEGAL             0x0004

#define VR_FLAG_SORTEDBYBLOCKNUMBER 0x0001
#define VR_FLAG_SORTEDBYBLOCKLENGTH 0x0002
#define VR_FLAG_SORTEDBYONTIME      0x0004
#define VR_FLAG_SORTEDBYOFFTIME     0x0008

  CObArray   m_BlockPropertiesDialogs;
  CUIntArray m_BlockPropertiesNumbers;
  CObArray   m_RunSliderDialogs;
  CUIntArray m_RunSliderBlockNumbers;
  CUIntArray m_DisplayList;
  CCutRuns m_CutRunsDlg;
  CPoint   m_LButtonPoint;
  CPoint   m_RButtonPoint;
  CMenu m_menu;
  CPen  m_Pens[LASTPEN];
  BOOL  m_bDisplayingCutRuns;
  BOOL  m_bDisplayReliefPointHashMarks;

  RECT  m_CutRunsRect;
  RECT  m_rc;
  RECT  m_rcRepaint;
  RECT  m_rcDialog;
  long  m_CurrentSortFlag;
  long  m_CurrentDisplayFlag;
  long  m_earliestTime;
  long  m_latestTime;
  long  m_timeInc;
  long  m_firstDisplayed;
  int   m_nCaptionSize;
  int   m_LastIndex;
  int   m_numInDisplayList;

  void  AbortInit();

	// Generated message map functions
	//{{AFX_MSG(CVisualRuncutter)
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnPaint();
	afx_msg void OnChangeScrollbar();
	afx_msg void OnScrollScrollbar();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnParameters();
	afx_msg void OnRuntypes();
	afx_msg void OnWorkrules();
	afx_msg void OnCutRecursive();
	afx_msg void OnCutMincost();
	afx_msg void OnCutCrewcut();
	afx_msg void OnProperties();
	afx_msg void OnSelectall();
	afx_msg void OnDeselectall();
	afx_msg void OnColors();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDisplayAllblocks();
	afx_msg void OnDisplayLeftovers();
	afx_msg void OnClose();
	afx_msg void OnSortBlocknumber();
	afx_msg void OnSortOfftime();
	afx_msg void OnSortOntime();
	afx_msg void OnSortBlocklength();
	afx_msg void OnJumptorun();
	afx_msg void OnCutWhatsleft();
	afx_msg void OnDisplayIllegal();
	afx_msg void OnRunslider();
	afx_msg void OnDisplayReliefhashmarks();
	afx_msg void OnRecursive();
	afx_msg void OnMincost();
	afx_msg void OnUndoruns();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISUALRUNCUTTER_H__9CD9A5F4_FF4C_478E_B438_79B96CEEDB87__INCLUDED_)
