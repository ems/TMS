//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// TMS.h : main header file for the TMS application
//

#if !defined(AFX_TMS_H__401DAA16_25A3_4D4F_B302_6183789B7682__INCLUDED_)
#define AFX_TMS_H__401DAA16_25A3_4D4F_B302_6183789B7682__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "InfoBar.h"
#include "Picture.h"
#include "F1Book.h"
#include "F1RangeRef.h"
#include "F1ObjPos.h"
#include "F1Rect.h"
#include "F1ReplaceResults.h"
#include "F1ODBCQuery.h"
#include "F1ODBCConnect.h"
#include "F1BookView.h"
#include "F1FileSpec.h"
#include "F1NumberFormat.h"
#include "F1CellFormat.h"
#include "F1FindReplaceInfo.h"
#include "F1PageSetup.h"

#include "crystalreportviewer11.h"
#include "Crystal.h"


/////////////////////////////////////////////////////////////////////////////
// CTMSApp:
// See TMS.cpp for the implementation of this class
//
//
//  External or not
//
#if !defined(EXTERN)
#define EXTERN extern
#endif

EXTERN CString strCompanyName;
EXTERN CString strProductName;
EXTERN CString strLegalCopyright;
EXTERN CString strProductVersion;
EXTERN CString strReleaseDate;

char *GetDatabaseString(char *inputString, long inputLength);
void PutDatabaseString(char *destination, CString inputString, long inputLength);

//
//  Text Files
//
#include "ExternalTextFiles.h"
//
//  Interaction with the "About Box"
//
#define ABOUTBOX_FLAG_ABOUT 0x0001
#define ABOUTBOX_FLAG_OPEN  0x0002

EXTERN long m_aboutFlag;

#define ABOUTBOX_RESULT_OPEN 0x0001
#define ABOUTBOX_RESULT_NEW  0x0002
#define ABOUTBOX_RESULT_OK   0x0004
#define ABOUTBOX_RESULT_EXIT 0x0008

EXTERN long m_aboutResult;

//
//  Database status
//
EXTERN BOOL m_bDatabaseOpen;

//
//  Global classname for all TMS-created child windows
//
EXTERN LPCTSTR m_lpszClass;
//
//  Interaction with open tables
//
#define MAXOPENTABLES 20

typedef struct TableDisplayStruct
{
  CWnd*     pane;
  CString   title;
  int       menuType;
  CF1Book   F1;
  FIELDINFO fieldInfo;
  FILEINFO  fileInfo;
  long      baggage;
  long      otherData;
  long      flags;
} TableDisplayDef;

EXTERN TableDisplayDef m_TableDisplay[MAXOPENTABLES];

//
//  Handy handles
//
EXTERN HWND m_hwndDlgBar;
EXTERN HWND m_wndBar;

//
//  Size of the client rect (for Crystal)
//
EXTERN CRect m_ClientRect;

EXTERN IApplicationPtr m_Application;
EXTERN IReportPtr      m_Report;
EXTERN CCrystalReportViewer11 m_Viewer;



class CTMSApp : public CWinApp
{
private:
  BOOL TextConvert(void);
  BOOL TextLoad(void);
public:
	CMultiDocTemplate* m_pDocTemplate;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CTMSApp();

  void SetInfobarText(CString);
  static void GetTMSData(TableDisplayDef *, long);
  static void EstablishDataFields(TableDisplayDef *);
  static void DisplayPatternRow(CF1Book *, TableDisplayDef *, PATTERNSDef *, int, int *);
  static void DisplayTripRow(CF1Book *, TableDisplayDef *, TRIPSDef *, int, int *);
  static void DisplayFlagRow(CF1Book *, TableDisplayDef *, AllFilesDef *, int, int *);
  static int  RenderField(CF1Book *, TableDisplayDef *, int, DATABASEDef *, int, int, char *);
  static BOOL GetNodeNames(TableDisplayDef *, int);
  static BOOL GetFlagNames(TableDisplayDef *, int);
  static void SetRowColor(CF1Book*, int, COLORREF);
  static void SetDefaultPageSetup(int pane);
  static BOOL CALLBACK EnumForChangedConnections(HWND, LPARAM);
  static BOOL CALLBACK EnumForChangedTrips(HWND, LPARAM);
//
//  File menu
//
  int  OnFileOpen();
  int  OnFileNew();
  void OnFileClose();
  void SetupBalanceOfMainFrame();

//
//  Globals
//
  void TMSRpt(void);
  void DetermineActiveReports(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTMSApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation
	//{{AFX_MSG(CTMSApp)
	afx_msg void OnAppAbout();
	afx_msg void OnHelpContents();
	afx_msg void OnEditUpdate();
	afx_msg void OnBrowseColumnCharacteristics();
	afx_msg void OnBrowseColumnSort();
	afx_msg void OnCommandsAnalyzeRun();
	afx_msg void OnCommandsAssignGarages();
	afx_msg void OnCommandsAssignTripNumbers();
	afx_msg void OnCommandsAssignVehicleType();
	afx_msg void OnCommandsBlockSummary();
	afx_msg void OnCommandsBlockTrips();
	afx_msg void OnCommandsCopyPatterns();
	afx_msg void OnCommandsCopyTrips();
	afx_msg void OnCommandsCopyroster();
	afx_msg void OnCommandsCopyruncut();
	afx_msg void OnCommandsCutRuns();
	afx_msg void OnCommandsDeleteRuns();
	afx_msg void OnCommandsEstablishrostertemplate();
	afx_msg void OnCommandsGraphicalTrips();
	afx_msg void OnCommandsHookBlocks();
	afx_msg void OnCommandsNodeFinder();
	afx_msg void OnCommandsPopi();
	afx_msg void OnCommandsRenumberBlocks();
	afx_msg void OnCommandsRenumberRoster();
	afx_msg void OnCommandsRenumberRuns();
	afx_msg void OnCommandsRenumberRunsFromRoster();
	afx_msg void OnCommandsRosterRuns();
	afx_msg void OnCommandsRunSummary();
	afx_msg void OnCommandsSelectExistingConnections();
	afx_msg void OnCommandsSetLayover();
	afx_msg void OnCommandsSortTrips();
	afx_msg void OnCommandsUnhookTrips();
	afx_msg void OnEditAddRunComment();
	afx_msg void OnEditAssignDriverToRoster();
	afx_msg void OnEditCopy();
	afx_msg void OnEditDelete();
	afx_msg void OnEditExtraboardEditor();
	afx_msg void OnEditFindBlock();
	afx_msg void OnEditInsert();
	afx_msg void OnEditInsertStopConnection();
	afx_msg void OnEditReassignRuntype();
	afx_msg void OnEditRemoveDriverFromRoster();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditUpdateRangeCustomerComments();
	afx_msg void OnEditUpdateRangeLayover();
	afx_msg void OnEditUpdateRangeOperatorComments();
	afx_msg void OnEditUpdateRangePatterns();
	afx_msg void OnEditUpdateRangeShift();
	afx_msg void OnEditUpdateRangeSigncodes();
	afx_msg void OnEditUpdateRangeVehicles();
	afx_msg void OnEditUpdateTripNumber();
	afx_msg void OnRunCrystalReports();
	afx_msg void OnRunMapinfo();
	afx_msg void OnSetupRosterDriverstoconsider();
	afx_msg void OnSetupRosterParameters();
	afx_msg void OnSetupRosterServiceDays();
	afx_msg void OnSetupRunsOvertime();
	afx_msg void OnSetupRunsParameters();
	afx_msg void OnSetupRunsPremiums();
	afx_msg void OnSetupRunsReliefpoints();
	afx_msg void OnSetupRunsRuntypes();
	afx_msg void OnViewTripRunningTimes();
	afx_msg void OnViewCompressedBlocks();
	afx_msg void OnFilePageSetup();
	afx_msg void OnFilePrint();
	afx_msg void OnEditInsertSdConnection();
	afx_msg void OnCommandsOptimalhookBlocks();
	afx_msg void OnEditCreateplaceholderruns();
	afx_msg void OnEditUpdateRangeTripnumbers();
	afx_msg void OnCommandsForcePopi();
	//}}AFX_MSG
  void OnEditChangeTripData(WPARAM which);
	void OnWindowCloseallwindows();
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMS_H__401DAA16_25A3_4D4F_B302_6183789B7682__INCLUDED_)
