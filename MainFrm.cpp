//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#include "stdafx.h"
#include <afxole.h>
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "CCFrontEnd.h"
}

#include "TMS.h"

#include "Discipline.h"
#include "DailyOpsHeader.h"

#include <dos.h>
#include <direct.h>
#include "BitmapDialog.h"
#include <version.h>
#include "cshuser.h"

#include "WinReg.h"
#include "mapinfow.h"   // ADDED FOR INTEGRATED MAPPING SUPPORT

#include "MainFrm.h"
#include "ChildFrm.h"

#include "CustomControls.h"
#include "Themes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void * CDailyOps::pObject;
EXTERN void	LaunchProg( const CString &url );


CMainFrame* CMainFrame::m_pInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_FILE_DIRECTIONS, OnFileDirections)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_WINDOW_CLOSEALLWINDOWS, OnWindowCloseallwindows)
	ON_COMMAND(ID_WINDOW_CLOSEWINDOW, OnWindowClosewindow)
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_ROUTES, OnFileRoutes)
	ON_COMMAND(ID_FILE_SERVICES, OnFileServices)
	ON_COMMAND(ID_FILE_JURISDICTIONS, OnFileJurisdictions)
	ON_COMMAND(ID_FILE_DIVISIONS, OnFileDivisions)
	ON_COMMAND(ID_FILE_NODESANDSTOPS, OnFileNodesandstops)
	ON_COMMAND(ID_FILE_PATTERNS, OnFilePatterns)
	ON_COMMAND(ID_FILE_CONNECTIONTIMES, OnFileConnectiontimes)
	ON_COMMAND(ID_FILE_BUSTYPES, OnFileBustypes)
	ON_COMMAND(ID_FILE_BUSES, OnFileBuses)
	ON_COMMAND(ID_FILE_SIGNCODES, OnFileSigncodes)
	ON_COMMAND(ID_FILE_COMMENTS, OnFileComments)
	ON_COMMAND(ID_FILE_TRIPS, OnFileTrips)
	ON_COMMAND(ID_FILE_STANDARDBLOCKS, OnFileStandardblocks)
	ON_COMMAND(ID_FILE_DROPBACKBLOCKS, OnFileDropbackblocks)
	ON_COMMAND(ID_FILE_RUNS, OnFileRuns)
	ON_COMMAND(ID_FILE_CREWONLYRUNS, OnFileCrewonlyruns)
	ON_COMMAND(ID_FILE_DRIVERTYPES, OnFileDrivertypes)
	ON_COMMAND(ID_FILE_DRIVERS, OnFileDrivers)
	ON_COMMAND(ID_FILE_DAILYOPERATIONS, OnFileDailyoperations)
	ON_COMMAND(ID_FILE_BIDDINGROSTERING, OnFileBiddingrostering)
	ON_COMMAND(ID_FILE_DISCIPLINE, OnFileDiscipline)
	ON_COMMAND(ID_FILE_TRIPPLANNING, OnFileTripplanning)
	ON_COMMAND(ID_FILE_COMPLAINTS, OnFileComplaints)
	ON_COMMAND(ID_FILE_TIMECHECKS, OnFileTimechecks)
	ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI_RANGE(ID_THEME_DEFAULT, ID_THEME_RIBBON, OnUpdateTheme)
	ON_COMMAND_RANGE(ID_THEME_DEFAULT, ID_THEME_RIBBON, OnSwitchTheme)
	ON_COMMAND(ID_THEME_CUSTOM, OnThemeCustom)
	ON_COMMAND(ID_VIEW_OFFICE2007FRAME, OnViewOffice2007Frame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OFFICE2007FRAME, OnUpdateViewOffice2007Frame)
	ON_COMMAND(ID_VIEW_TEXTBELOWICONS, OnTextBelow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTBELOWICONS, OnUpdateTextBelow)
	ON_WM_CLOSE()
	ON_COMMAND_RANGE(ID_SAVEAS_XML, ID_SAVEAS_BINARY, OnSaveAs)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SAVEAS_XML, ID_SAVEAS_BINARY, OnUpdateSaveAs)
	ON_COMMAND(XTP_ID_CUSTOMIZE, OnCustomize)
	ON_XTP_EXECUTE(ID_EDIT_STATE, OnEditState)
	ON_UPDATE_COMMAND_UI(ID_EDIT_STATE, OnUpdateEditState)
	ON_XTP_EXECUTE(ID_FILE_NEW, OnFileNew)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TASKPANEL_NEWDOCUMENT, ID_TASKPANEL_SEARCH, OnUpdateTaskPanelCaption)
	ON_MESSAGE(WM_XTP_CONTROLRBUTTONUP, OnControlRButtonUp)
	ON_XTP_EXECUTE_RANGE(ID_TASKPANEL_NEWDOCUMENT, ID_TASKPANEL_SEARCH, OnTaskPanelCaption)
	ON_XTP_CREATECONTROL()
	ON_XTP_CREATECOMMANDBAR()
	ON_MESSAGE(XTPWM_TASKPANEL_NOTIFY, OnTaskPanelNotify)
  ON_CBN_SELCHANGE(RSDTOOLBAR_ROUTE, OnChangeRSDRoute)
  ON_CBN_SELCHANGE(RSDTOOLBAR_SERVICE, OnChangeRSDService)
  ON_CBN_SELCHANGE(RSDTOOLBAR_DIVISION, OnChangeRSDDivision)
  ON_CBN_SELCHANGE(RSDTOOLBAR_ROSTERWEEK, OnChangeRSDRosterWeek)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


static UINT uHideCmds[] =
{
	ID_FILE_CLOSE,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
	ID_FILE_PRINT_PREVIEW,
	ID_FILE_PRINT_SETUP,
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
	ID_WINDOW_CASCADE,
	ID_WINDOW_TILE_HORZ,
	ID_WINDOW_ARRANGE,
	ID_SCROLL_ITEM7,
	ID_SCROLL_ITEM8,
	ID_SCROLL_ITEM9,
	ID_SCROLL_ITEM10,
	ID_SCROLL_ITEM11,
	ID_SCROLL_ITEM12,
	ID_EDIT_FIND,
};

LPCTSTR lpszProfileName = _T("CommandBars_ver11");

#define IDR_COLORTEXT_POPUP 1000
#define IDR_COLORBACK_POPUP 1001

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nState = 0;
	m_nIDSaveAs = ID_SAVEAS_XML;
	m_pInstance = this;

	m_nCurrentTheme = ID_THEME_OFFICE_XP;

	m_bOffice2007Frame = 2;

  m_pPanel = NULL;

  m_dwWindowStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Initialize command bars.  Command bars must be created and initialized
	// after all standard MFC control bars have been created.
	if (!InitCommandBars())
		return -1;

	// Set the default theme to Office XP.
	CXTPPaintManager::SetTheme(xtpThemeOfficeXP);

	// Get a pointer to the command bars object.
	CXTPCommandBars* pCommandBars = GetCommandBars();

	// create the menubar.
	CXTPCommandBar* pMenuBar = pCommandBars->SetMenu(_T("Menu Bar"), IDR_MAINFRAME); //IDR_CUSTOMTYPE);
	pMenuBar->SetFlags(xtpFlagIgnoreSetMenuMessage);

	// create the standard toolbar.
	CXTPToolBar* pStandardBar = (CXTPToolBar*)pCommandBars->Add(_T("Standard"), xtpBarTop);
	if (!pStandardBar || !pStandardBar->LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create pStandardBar\n");
		return -1;
	}
	
  if (!m_dlgBar.Create(this, IDR_MAINFRAME, CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
		return -1;
  m_hwndDlgBar = m_dlgBar.m_hWnd;
	if (!m_wndReBar.Create(this) || !m_wndReBar.AddBar(&m_dlgBar))
		return -1;


	// create the theme toolbar.
	CXTPToolBar* pThemeBar = (CXTPToolBar*)pCommandBars->Add(_T("Theme Bar"), xtpBarTop);
	if (!pThemeBar || !pThemeBar->LoadToolBar(IDR_SWITCH))
	{
		TRACE0("Failed to create pThemeBar\n");
		return -1;
	}

	CXTPDialogBar* pBar = (CXTPDialogBar*)pCommandBars->Add(_T("Tables and Tasks"), xtpBarLeft, RUNTIME_CLASS(CXTPDialogBar));
	pBar->SetBarID(200);
	pBar->EnableDocking(xtpFlagAlignAny|xtpFlagFloating|xtpFlagStretched);
	pBar->SetCaption(_T("Tables and Tasks"));
	pBar->ModifyStyle(0, WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

	m_wndTaskPanel.Create(pBar);
	m_wndTaskPanel.SetOwner(this);
	pBar->SetChild(&m_wndTaskPanel);
//	pBar->SetCaptionPopup(IDR_MENU_TASKPANEL);
	m_wndTaskPanel.SetTheme(XTPPaintManager()->BaseTheme());

	pCommandBars->GetCommandBarsOptions()->animationType = xtpAnimateWindowsDefault;
	pCommandBars->GetCommandBarsOptions()->ShowKeyboardCues(xtpKeyboardCuesShowWindowsDefault);

	// Load the saved toolbar layout.
	LoadCommandBars(lpszProfileName);

	// Initialize intelligent menus.
	pCommandBars->GetCommandBarsOptions()->bAlwaysShowFullMenus = FALSE;
	pCommandBars->HideCommands(uHideCmds, _countof(uHideCmds));

	// Toggle floating by double click
	pCommandBars->GetCommandBarsOptions()->bDblClickFloat = TRUE;
	
	pCommandBars->GetToolTipContext()->SetStyle(xtpToolTipOffice);

	// Change size of popup icons.
	// pCommandBars->GetCommandBarsOptions()->szPopupIcons = CSize(20, 20);
//
//  Global classname for all TMS-created child windows
//
  m_lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, hCursorArrow, (HBRUSH)(COLOR_WINDOW+1), hIconTMS);
//
//  Save the size of the client rect for reference in calls to Crystal's API
//
  GetClientRect(&m_ClientRect);

	return 0;
}

void CMainFrame::DockRightOf(CXTPToolBar* pBarToDock, CXTPToolBar* pBarOnLeft)
{
	RecalcLayout(TRUE);
	CXTPWindowRect rcBar(pBarOnLeft);
	rcBar.OffsetRect(rcBar.Width(), 0);
	GetCommandBars()->DockCommandBar(pBarToDock, rcBar, pBarOnLeft->GetDockBar());
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style &= ~FWS_ADDTOTITLE; 

	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


void CMainFrame::SetCustomTheme(CXTPPaintManager* pTheme)
{
	CXTPPaintManager::SetCustomTheme(pTheme);

	m_wndTaskPanel.SetTheme(XTPPaintManager()->BaseTheme());
	RecalcLayout(FALSE);
	GetCommandBars()->RedrawCommandBars();
	RedrawWindow();
}

void CMainFrame::OnThemeCustom()
{

}

void CMainFrame::OnUpdateTheme(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrentTheme ? TRUE : FALSE);
}
void CMainFrame::OnSwitchTheme(UINT nTheme)
{

	EnableOffice2007Frame(NULL);

	switch (nTheme)
	{
		case ID_THEME_DEFAULT: SetCustomTheme(new CXTPDefaultTheme()); break;
		case ID_THEME_OFFICE_XP: SetCustomTheme(new CXTPOfficeTheme()); break;
		case ID_THEME_OFFICE_2003: SetCustomTheme(new CXTPOffice2003Theme()); break;
		case ID_THEME_WINDOWS_XP: SetCustomTheme(new CXTPNativeXPTheme()); break;
		case ID_THEME_RIBBON: SetCustomTheme(new CXTPRibbonTheme()); break;
		case ID_THEME_OFFICE2007: SetCustomTheme(new CXTPOffice2007Theme()); break;

		case ID_THEME_MAC: SetCustomTheme(new CCustomMacTheme()); break;
		case ID_THEME_GRAY: SetCustomTheme(new CCustomGrayTheme()); break;
		case ID_THEME_ICY: SetCustomTheme(new CCustomIcyTheme()); break;
		case ID_THEME_GRAD: SetCustomTheme(new CCustomGrayTheme()); break;
		case ID_THEME_BEVEL: SetCustomTheme(new CCustomBevelTheme()); break;
		case ID_THEME_FANCY: SetCustomTheme(new CCustomFancyTheme()); break;
	}

	if (nTheme == ID_THEME_RIBBON || nTheme == ID_THEME_OFFICE2007)
	{
		if (m_bOffice2007Frame == 2 || m_bOffice2007Frame == 1)
		{
			EnableOffice2007Frame(GetCommandBars());
		}
	}

	m_nCurrentTheme = nTheme;
}

void CMainFrame::OnViewOffice2007Frame()
{
	m_bOffice2007Frame = m_bOffice2007Frame + 1;
	if (m_bOffice2007Frame == 3) m_bOffice2007Frame = 0;

	OnSwitchTheme(m_nCurrentTheme);
}

void CMainFrame::OnUpdateViewOffice2007Frame(CCmdUI* pCmdUI)
{
	if (m_nCurrentTheme == ID_THEME_RIBBON || m_nCurrentTheme == ID_THEME_OFFICE2007)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);

	}
	pCmdUI->SetCheck(m_bOffice2007Frame);

}


BOOL GetAppDataFolder(CString& strAppDataFolder)
{
	OSVERSIONINFO versionInfo = { sizeof(OSVERSIONINFO) };
	if (!(GetVersionEx(&versionInfo) && versionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT && versionInfo.dwMajorVersion >= 5))
		return FALSE;

	LPITEMIDLIST pidlAppData = NULL;
	IMalloc * pMalloc = NULL;
	BOOL bResult = FALSE;

	HRESULT hr = SHGetMalloc(&pMalloc);
	if (SUCCEEDED(hr))
	{
		TCHAR   szPath[MAX_PATH];
		if (SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlAppData) == NOERROR )
		{
			if (SHGetPathFromIDList( pidlAppData, szPath))
			{
				strAppDataFolder = szPath;
				bResult = TRUE;
			}
		}

		pMalloc->Free(pidlAppData);
		pMalloc->Release();

	}
	return TRUE;
}

void CMainFrame::OnClose()
{
	CString strAppDataFolder;

	if (GetAppDataFolder(strAppDataFolder))
	{
		CreateDirectory(strAppDataFolder + _T("\\") + AfxGetApp()->m_pszAppName, NULL);

		// Set customization path as "\Documents and Settings\user\Application Data\CustomThemes\CommandBars_ver6"
		GetCommandBars()->SetCustomizationDataFileName(strAppDataFolder + _T("\\") + AfxGetApp()->m_pszAppName + _T("\\") + lpszProfileName);
	}


	SaveCommandBars(lpszProfileName);

	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnCustomize()
{
	CXTPCustomizeSheet cs(GetCommandBars());

	CXTPCustomizeOptionsPage pageOptions(&cs);
	cs.AddPage(&pageOptions);

	CXTPCustomizeCommandsPage* pCommands = cs.GetCommandsPage();
	pCommands->AddCategories(IDR_MAINFRAME); //IDR_CUSTOMTYPE);
	pCommands->GetControls(_T("File"))->Add(xtpControlButton, ID_FILE_OPEN, _T(""), 1);

	pCommands->AddToolbarCategory(_T("Themes"), IDR_SWITCH);

	pCommands->InsertAllCommandsCategory();
	pCommands->InsertBuiltInMenus(IDR_MAINFRAME); //IDR_CUSTOMTYPE);
	pCommands->InsertNewMenuCategory();

//	CXTPControls* pControls = pCommands->GetControls(_T("Test"));
//	ASSERT(pControls);
//  pControls->Add(xtpControlButton, ID_FILE_NEW);
//	pControls->Add(xtpControlPopup, ID_TOOLS_HEX);
//	pControls->Add(xtpControlButtonPopup, ID_TOOLS_HEX);
//	pControls->Add(xtpControlSplitButtonPopup, ID_TOOLS_HEX);

//	CXTPControl* pControlState = GetCommandBars()->FindControl(xtpControlComboBox, ID_EDIT_STATE, FALSE, TRUE);
//	if (pControlState) pControls->AddClone(pControlState, 4);

//	CXTPControl* pControlNew = GetCommandBars()->FindControl(xtpControlEdit, ID_FILE_NEW, FALSE, TRUE);
//	if (pControlNew) pControls->AddClone(pControlNew, 6);

	cs.DoModal();
}


int CMainFrame::OnCreateCommandBar(LPCREATEBARSTRUCT lpCreatePopup)
{
	if (lpCreatePopup->bTearOffBar && (lpCreatePopup->nID == IDR_COLORTEXT_POPUP || lpCreatePopup->nID == IDR_COLORBACK_POPUP))
	{
		CXTPToolBar* pToolBar = DYNAMIC_DOWNCAST(CXTPToolBar, lpCreatePopup->pCommandBar);
		ASSERT(pToolBar);

		// Prevent docking.
		pToolBar->EnableDocking(xtpFlagFloating);
		// Prevent customization
		pToolBar->EnableCustomization(FALSE);
		return TRUE;
	}
	return FALSE;
}

int CMainFrame::OnCreateControl(LPCREATECONTROLSTRUCT lpCreateControl)
{
	if (lpCreateControl->bToolBar == FALSE)
	{
		if (lpCreateControl->controlType == xtpControlPopup && lpCreateControl->strCaption == _T("&Open..."))
		{
			lpCreateControl->controlType = xtpControlSplitButtonPopup;
			lpCreateControl->nID = ID_FILE_OPEN;
			return TRUE;
		}
		if (lpCreateControl->controlType == xtpControlButton && lpCreateControl->nID == ID_TOOLS_HEX)
		{
			lpCreateControl->controlType = xtpControlSplitButtonPopup;
			return TRUE;
		}

		if (lpCreateControl->controlType == xtpControlButton && lpCreateControl->nID == ID_EDIT_FIND)
		{
			CXTPControlComboBox* pComboFind = (CXTPControlComboBox*)CXTPControlComboBox::CreateObject();
			pComboFind->SetDropDownListStyle();
			pComboFind->SetEditHint(_T("Click to find string"));

			pComboFind->AddString(_T("Line1"));
			pComboFind->AddString(_T("Line2"));
			pComboFind->AddString(_T("Line3"));

			pComboFind->SetWidth(150);
			pComboFind->SetCaption(_T("Find"));
			pComboFind->SetStyle(xtpComboLabel);

			lpCreateControl->pControl = pComboFind;
			return TRUE;

		}
	}
	else
	{
		CXTPToolBar* pToolBar = DYNAMIC_DOWNCAST(CXTPToolBar, lpCreateControl->pCommandBar);
		if (!pToolBar)
			return FALSE;

		if (lpCreateControl->nID == ID_FILE_PRINT_EX && pToolBar->GetBarID() == IDR_LEFT)
		{
			lpCreateControl->controlType = xtpControlSplitButtonPopup;
			lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
			return TRUE;
		}
		if (lpCreateControl->nID == IDR_MENU_POPUP && pToolBar->GetBarID() == IDR_LEFT)
		{
			lpCreateControl->controlType = xtpControlPopup;
			lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
			return TRUE;
		}
		if (lpCreateControl->nID == ID_TOOLS_HEX && pToolBar->GetBarID() == IDR_MAINFRAME)
		{
			lpCreateControl->controlType = xtpControlSplitButtonPopup;
			return TRUE;
		}
		if (lpCreateControl->nID == ID_FILE_NEW && pToolBar->GetBarID() == IDR_COMBOBOX_BAR)
		{
			lpCreateControl->controlType = xtpControlEdit;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_THEME_CUSTOM)
		{
			lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
			lpCreateControl->controlType = xtpControlPopup;
			return TRUE;
		}
		if (lpCreateControl->nID == ID_VIEW_OFFICE2007FRAME)
		{
			lpCreateControl->controlType = xtpControlCheckBox;
			return TRUE;
		}

		if (pToolBar->GetBarID() == IDR_SWITCH && (lpCreateControl->nID >= ID_THEME_DEFAULT && lpCreateControl->nID <= ID_THEME_RIBBON))
		{
			lpCreateControl->buttonStyle = xtpButtonCaption;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_EDIT_FIND && pToolBar->GetBarID() == IDR_COMBOBOX_BAR)
		{
			CXTPControlComboBox* pComboFind = (CXTPControlComboBox*)CXTPControlComboBox::CreateObject();
			pComboFind->SetDropDownListStyle();
			pComboFind->SetEditHint(_T("Click to find string"));

			pComboFind->AddString(_T("Line1"));
			pComboFind->AddString(_T("Line2"));
			pComboFind->AddString(_T("Line3"));

			pComboFind->SetWidth(150);
			pComboFind->SetStyle(xtpButtonIconAndCaption);

			lpCreateControl->pControl = pComboFind;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_COMBO_MASKED)
		{
			CXTPControlComboBox* pComboMasked = (CXTPControlComboBox*)CControlComboBoxEx::CreateObject();
			pComboMasked->SetDropDownListStyle();

			pComboMasked->AddString(_T("12:00:20:30"));
			pComboMasked->AddString(_T("11:03:12:30"));
			pComboMasked->AddString(_T("00:00:12:30"));
			pComboMasked->AddString(_T("10:00:30:30"));

			pComboMasked->SetWidth(130);
			pComboMasked->SetStyle(xtpComboLabel);
			pComboMasked->SetFlags(xtpFlagManualUpdate);

			lpCreateControl->pControl = pComboMasked;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_COMBO_OWNERDRAW)
		{
			CXTPControlComboBox* pComboCustomDraw = new CControlComboBoxCustomDraw();

			pComboCustomDraw->SetDropDownListStyle();
			pComboCustomDraw->SetWidth(130);
			pComboCustomDraw->SetDropDownWidth(230);
			pComboCustomDraw->SetStyle(xtpComboLabel);

			lpCreateControl->pControl = pComboCustomDraw;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_COMBO_POPUP)
		{
			CXTPControlComboBox* pComboPopup = (CXTPControlComboBox*)CControlComboBoxPopup::CreateObject();

			CXTPPopupBar* pPopupBar = CXTPPopupBar::CreatePopupBar(GetCommandBars());
			CMenu menu;
			menu.LoadMenu(IDR_MENU_SAVEAS);

			pPopupBar->LoadMenu(menu.GetSubMenu(0));

			pComboPopup->SetCommandBar(pPopupBar);

			pPopupBar->InternalRelease();



			pComboPopup->SetWidth(130);
			pComboPopup->SetStyle(xtpComboLabel);
			pComboPopup->SetFlags(xtpFlagManualUpdate);

			lpCreateControl->pControl = pComboPopup;
			return TRUE;
		}


		if (lpCreateControl->nID == ID_EDIT_STATE && pToolBar->GetBarID() == IDR_COMBOBOX_BAR)
		{
			CXTPControlComboBox* pComboState = (CXTPControlComboBox*)CXTPControlComboBox::CreateObject();
			pComboState->AddString(_T("Debug"));
			pComboState->AddString(_T("Release"));
			pComboState->AddString(_T("Configuration Manager"));
			pComboState->SetDropDownWidth(150);
			lpCreateControl->pControl = pComboState;
			return TRUE;
		}

		if (lpCreateControl->nID == ID_COMBO_DISABLED && pToolBar->GetBarID() == IDR_COMBOBOX_BAR)
		{
			CXTPControlComboBox* pComboDisable = (CXTPControlComboBox*)CXTPControlComboBox::CreateObject();
			pComboDisable->SetStyle(xtpComboLabel);
			lpCreateControl->pControl = pComboDisable;
			return TRUE;

		}

		if (lpCreateControl->nID == ID_BUTTONLINE && pToolBar->GetBarID() == IDR_CUSTOM)
		{
			CXTPControlPopup* pButtonLine = CXTPControlPopup::CreateControlPopup(xtpControlButtonPopup);
			pButtonLine->SetFlags(xtpFlagManualUpdate);

			CXTPPopupBar* pLineBar = (CXTPPopupBar*)CXTPPopupToolBar::CreateObject();

			pLineBar->GetControls()->Add(new CControlButtonLine(0.25), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(0.5), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(0.75), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(1), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(1.5), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(2.25), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(3), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(4.5), 0, _T(""), -1, FALSE);
			pLineBar->GetControls()->Add(new CControlButtonLine(6), 0, _T(""), -1, FALSE);

			pButtonLine->SetCommandBar(pLineBar);
			pLineBar->InternalRelease();

			lpCreateControl->pControl = pButtonLine;

			return TRUE;

		}
		if (lpCreateControl->nID == ID_BUTTONSTYLE && pToolBar->GetBarID() == IDR_CUSTOM)
		{
			CXTPControlPopup* pButtonStyle = CXTPControlPopup::CreateControlPopup(xtpControlButtonPopup);
			pButtonStyle->SetFlags(xtpFlagManualUpdate);

			CXTPPopupBar* pStyleBar = (CXTPPopupBar*)CXTPPopupToolBar::CreateObject();

			pStyleBar->GetControls()->Add(new CControlButtonStyle(0), 0, _T(""), -1, FALSE);
			pStyleBar->GetControls()->Add(new CControlButtonStyle(1), 0, _T(""), -1, FALSE);
			pStyleBar->GetControls()->Add(new CControlButtonStyle(2), 0, _T(""), -1, FALSE);
			pStyleBar->GetControls()->Add(new CControlButtonStyle(3), 0, _T(""), -1, FALSE);
			pStyleBar->GetControls()->Add(new CControlButtonStyle(4), 0, _T(""), -1, FALSE);
			pStyleBar->GetControls()->Add(new CControlButtonStyle(5), 0, _T(""), -1, FALSE);

			pButtonStyle->SetCommandBar(pStyleBar);
			pStyleBar->InternalRelease();

			lpCreateControl->pControl = pButtonStyle;

			return TRUE;
		}

		if (lpCreateControl->nID == ID_BUTTONARROW && pToolBar->GetBarID() == IDR_CUSTOM)
		{
			CXTPControlPopup* pButtonArrow = CXTPControlPopup::CreateControlPopup(xtpControlButtonPopup);
			pButtonArrow->SetFlags(xtpFlagManualUpdate);

			CXTPPopupBar* pArrowBar = (CXTPPopupBar*)CXTPPopupToolBar::CreateObject();

			pArrowBar->GetControls()->Add(new CControlButtonArrow(1), 0, _T(""), -1, FALSE);
			pArrowBar->GetControls()->Add(new CControlButtonArrow(2), 0, _T(""), -1, FALSE);
			pArrowBar->GetControls()->Add(new CControlButtonArrow(3), 0, _T(""), -1, FALSE);
			pArrowBar->GetControls()->Add(new CControlButtonArrow(4), 0, _T(""), -1, FALSE);
			CXTPControlButton* pOther =
				(CXTPControlButton*)pArrowBar->GetControls()->Add(new CControlButtonArrow(0), 0, _T(""), -1, FALSE);


			pOther->SetCaption(_T("Other Arrow..."));
			pOther->SetBeginGroup(TRUE);
			pOther->SetStyle(xtpButtonCaption);

			pButtonArrow->SetCommandBar(pArrowBar);
			pArrowBar->InternalRelease();

			lpCreateControl->pControl = pButtonArrow;

			return TRUE;
		}

		if (lpCreateControl->nID == ID_BUTTON_TEXT && pToolBar->GetBarID() == IDR_CUSTOM)
		{
			CXTPControlPopupColor* pPopupColor = new CXTPControlPopupColor();

			CXTPPopupBar* pColorBar = (CXTPPopupBar*)CXTPPopupToolBar::CreateObject();

			pColorBar->GetControls()->Add(new CXTPControlButtonColor(), ID_TEXT_AUTO);
			pColorBar->GetControls()->Add(new CXTPControlColorSelector(), ID_SELECTOR_TEXT);
			pColorBar->GetControls()->Add(new CXTPControlButtonColor(), ID_TEXT_MORE);

			pPopupColor->SetCommandBar(pColorBar);
			pColorBar->InternalRelease();

			pColorBar->SetTearOffPopup(_T("Text Color"), IDR_COLORTEXT_POPUP, 0);
			pColorBar->EnableCustomization(FALSE);

			lpCreateControl->pControl = pPopupColor;

			return TRUE;
		}

		if (lpCreateControl->nID == ID_BUTTON_BACK && pToolBar->GetBarID() == IDR_CUSTOM)
		{
			CXTPControlPopupColor* pPopupColorBack = new CXTPControlPopupColor();

			CXTPPopupBar* pColorBar = (CXTPPopupBar*)CXTPPopupToolBar::CreateObject();

			pColorBar->GetControls()->Add(new CXTPControlButtonColor(), ID_BACK_NONE);
			pColorBar->GetControls()->Add(new CXTPControlColorSelector(), ID_SELECTOR_BACK);
			pColorBar->GetControls()->Add(new CXTPControlButtonColor(), ID_BACK_MORE);

			pPopupColorBack->SetCommandBar(pColorBar);
			pColorBar->InternalRelease();

			pColorBar->SetTearOffPopup(_T("Back Color"), IDR_COLORBACK_POPUP, 0);
			pColorBar->EnableCustomization(FALSE);

			lpCreateControl->pControl = pPopupColorBack;

			return TRUE;
		}
	}
	return FALSE;

}


void CMainFrame::OnEditState(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMXTPCONTROL* tagNMCONTROL = (NMXTPCONTROL*)pNMHDR;

	CXTPControlComboBox* pControl = (CXTPControlComboBox*)tagNMCONTROL->pControl;
	if (pControl->GetType() == xtpControlComboBox)
	{
		int nState = pControl->GetCurSel();
		if (nState != 2)
		{
			m_nState = nState;
		}
		else
		{
			MessageBox(_T("Configuration Manager"));
			pControl->SetCurSel(m_nState);
		}
		*pResult = TRUE; // Handled
	}
}

void CMainFrame::OnUpdateEditState(CCmdUI* pCmdUI)
{
	CXTPControlComboBox* pStateCombo = DYNAMIC_DOWNCAST(CXTPControlComboBox, CXTPControl::FromUI(pCmdUI));

	if (pStateCombo && !pStateCombo->HasFocus())
	{
		pStateCombo->SetCurSel(m_nState);
	}

	pCmdUI->Enable(TRUE);
}


void CMainFrame::OnFileNew(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMXTPCONTROL* tagNMCONTROL = (NMXTPCONTROL*)pNMHDR;

	CXTPControlComboBox* pControl = (CXTPControlComboBox*)tagNMCONTROL->pControl;
	if (pControl->GetType() == xtpControlEdit)
	{
		CString str = ((CXTPControlEdit*)pControl)->GetEditText();

		CDocument* pDocument = ((CTMSApp*)AfxGetApp())->m_pDocTemplate->OpenDocumentFile(NULL);
		if (pDocument != NULL && str.GetLength() > 0)
		{
			pDocument->SetTitle(str);
		}

		*pResult = TRUE; // Handled
	}
}


BOOL CMainFrame::OnTaskPanelCaption(UINT /*nID*/, NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	CXTPControl* pControl = ((NMXTPCONTROL*)pNMHDR)->pControl;
	CXTPDialogBar* pBar = DYNAMIC_DOWNCAST(CXTPDialogBar, GetCommandBars()->GetToolBar(200));
	ASSERT(pBar);

	pBar->SetCaption(pControl->GetCaption());
	return 1;
}

void CMainFrame::OnUpdateTaskPanelCaption(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	CXTPDialogBar* pBar = DYNAMIC_DOWNCAST(CXTPDialogBar, GetCommandBars()->GetToolBar(200));
	ASSERT(pBar);

	CXTPControl* pControl = CXTPControl::FromUI(pCmdUI);
	if (pControl)
	{
		pCmdUI->SetCheck(pControl->GetCaption() == pBar->GetCaption()? TRUE: FALSE);
	}

}



void CMainFrame::OnTextBelow()
{
	GetCommandBars()->GetCommandBarsOptions()->bShowTextBelowIcons =
		!GetCommandBars()->GetCommandBarsOptions()->bShowTextBelowIcons;

	// or
	// GetCommandBars()->GetAt(1)->ShowTextBelowIcons(TRUE);

	GetCommandBars()->RedrawCommandBars();
}

void CMainFrame::OnUpdateTextBelow(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetCommandBars()->GetCommandBarsOptions()->bShowTextBelowIcons);
}


void CMainFrame::OnSaveAs(UINT nID)
{
	m_nIDSaveAs = nID;
}

void CMainFrame::OnUpdateSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nIDSaveAs? TRUE: FALSE);
}

LRESULT CMainFrame::OnControlRButtonUp(WPARAM, LPARAM lParam)
{	
	CXTPControl* pControl = (CXTPControl*)lParam;

	UNUSED_ALWAYS(pControl);

	return FALSE;

}
//
//  OnChangeRSDRoute() - Process a change in the toolbar "Route" dropdown
//
void CMainFrame::OnChangeRSDRoute() 
{
  CWnd* pROUTE = m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE);
  
  if(pROUTE)
  {
    int nI = pROUTE->SendMessage(CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
  
    if(nI == CB_ERR)
    {
      return;
    }
    m_RouteRecordID = pROUTE->SendMessage(CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
    sprintf(tempString, "%ld", m_RouteRecordID);
    if(WritePrivateProfileString((LPSTR)userName, (LPSTR)"Route", (LPSTR)tempString, (LPSTR)szDatabaseFileName) == 0)
    {
      DWORD dw = GetLastError(); 
 
      sprintf(tempString, "GetLastError returned %u\n", dw); 
      MessageBox(tempString, TMS, MB_OK); 
    }
    ROUTESKey0.recordID = m_RouteRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(m_RouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(m_RouteNumber, ROUTES_NUMBER_LENGTH);
    strncpy(m_RouteName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(m_RouteName, ROUTES_NAME_LENGTH);
  }
}

//
//  OnChangeRSDService() - Process a change in the toolbar "Service" dropdown
//
void CMainFrame::OnChangeRSDService()
{
  CWnd* pSERVICE = m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE);
  
  if(pSERVICE)
  {
    int nI = pSERVICE->SendMessage(CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
  
    if(nI == CB_ERR)
      return;
    m_ServiceRecordID = pSERVICE->SendMessage(CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
    ltoa(m_ServiceRecordID, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"Service", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
    SERVICESKey0.recordID = m_ServiceRecordID;
    btrieve(B_GETEQUAL,TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(m_ServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(m_ServiceName, SERVICES_NAME_LENGTH);
    m_ServiceNumber = SERVICES.number;
  }
}

//
//  OnChangeRSDDivision() - Process a change in the toolbar "Division" dropdown
//
void CMainFrame::OnChangeRSDDivision()
{
  CWnd* pDIVISION = m_dlgBar.GetDlgItem(RSDTOOLBAR_DIVISION);
  
  if(pDIVISION)
  {
    int nI = pDIVISION->SendMessage(CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if(nI == CB_ERR)
      return;
    m_DivisionRecordID = pDIVISION->SendMessage(CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
    ltoa(m_DivisionRecordID, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"Division", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
    DIVISIONSKey0.recordID = m_DivisionRecordID;
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(m_DivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(m_DivisionName, DIVISIONS_NAME_LENGTH);
    m_DivisionNumber = DIVISIONS.number;
  }
}

//
//  OnChangeRSDRosterWeek() - Process a change in the toolbar "Roster Week" dropdown
//
void CMainFrame::OnChangeRSDRosterWeek()
{
  CWnd* pROSTERWEEK = m_dlgBar.GetDlgItem(RSDTOOLBAR_ROSTERWEEK);
  
  if(pROSTERWEEK)
  {
    int nI = pROSTERWEEK->SendMessage(CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if(nI == CB_ERR)
    {
      return;
    }
    m_RosterWeek = pROSTERWEEK->SendMessage(CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
    ltoa(m_RosterWeek, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"RosterWeek", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
  }
}

//
//  Set the "Page Setup" defaults
//
//
//  Set the "Page Setup" defaults
//
//  These are the special codes text string can contain.
//  By default, text is centered unless &L or &R is specified.
//
//  &L	Left-aligns the characters that follow.
//  &C	Centers the characters that follow.
//  &R	Right-aligns the characters that follow.
//  &A	Prints the sheet name.
//  &D	Prints the current date.
//  &T	Prints the current time.
//  &F	Prints the workbook name.
//  &P	Prints the page number.
//  &P+number	Prints the page number plus number.
//  &P-number	Prints the page number minus number.
//  &&	Prints an ampersand.
//  &N	Prints the total number of pages in the document.
//
//  The following font codes must appear before other codes and text or they are ignored.
//  The alignment codes (e.g., &L, &C, and &R) restart each section; new font codes can be
//  specified after an alignment code.
//
//  &B	Use a bold font.
//  &I	Use an italic font.
//  &U	Underline the header.
//  &S	Strikeout the header.
//  &O	Ignored (Mac specific).
//  &H	Ignored (Mac specific).
//  &"fontname"	Use the specified font.
//  &nn	Use the specified font size - must be a two digit number.
//
void CTMSApp::SetDefaultPageSetup(int pane)
{
  TableDisplayDef *pTD = &m_TableDisplay[pane];
  if(pTD)
  {
    pTD->F1.SetPrintColHeading(TRUE);
    pTD->F1.SetPrintRowHeading(TRUE);
//
//  Page header
//
    sprintf(tempString, "&\"Tahoma\"&L&06%s - The Master Scheduler&R&06Printed &D at &T&C&06%s", szPropertyName, m_TableDisplay[pane].title);
    pTD->F1.SetPrintHeader(tempString);
//
//  Page footer
//
    sprintf(tempString, "&\"Tahoma\"&L&06%s - Effective: %s (%s)&R&06Page &P of &N",
          szDatabaseDescription, szEffectiveDate, szDatabaseFileName);
    pTD->F1.SetPrintFooter(tempString);
  }
}

void CMainFrame::OnUpdateReportSelections()
{
  m_wndTaskPanel.OnUpdateReportSelections();
}

//
//  OnTaskPanelNotify()
//
//  Process a selection on the Task Panel
//
LRESULT CMainFrame::OnTaskPanelNotify(WPARAM wParam, LPARAM lParam)
{
  CString s;
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS MSJet Text";
  TMSRPTPassedDataDef PassedData;
  DWORD  tempStringLength = TEMPSTRING_LENGTH;
  DWORD  type = REG_SZ;
  HKEY  hKey;
  FILE  *fp;
  long  result;
  char  szReportName[TMSRPT_REPORTNAME_LENGTH];
  char  szReportFileName[TMSRPT_REPORTFILENAME_LENGTH];
  char  szReportDataFile[TMSRPT_MAX_REPORTOUTPUTFILES][TMSRPT_REPORTFILENAME_LENGTH];
  int   nReportNumber;
  int   numDataFiles;
  int   thisReport;
  int   nI, nJ;

  if(GetAvailableTableSlot() == NO_RECORD)
  {
    MessageBox("You need to close some other tables first", TMS, MB_OK);
    return 0;
  }

	switch(wParam)
  {
    case XTP_TPN_CLICK:
		{
			CXTPTaskPanelGroupItem* pItem = (CXTPTaskPanelGroupItem*)lParam;
			TRACE(_T("Click Event: pItem.Caption = %s, pItem.ID = %i\n"), pItem->GetCaption(), pItem->GetID());
      switch(pItem->GetID())
      {
        case ID_FILE_DIRECTIONS:
          OnFileDirections();
          break;

        case ID_FILE_ROUTES:
          OnFileRoutes();
          break;

        case ID_FILE_SERVICES:
          OnFileServices();
          break;

        case ID_FILE_JURISDICTIONS:
          OnFileJurisdictions();
          break;

        case ID_FILE_DIVISIONS:
          OnFileDivisions();
          break;

        case ID_FILE_NODESANDSTOPS:
          OnFileNodesandstops();
          break;

        case ID_FILE_PATTERNS:
          OnFilePatterns();
          break;

        case ID_FILE_CONNECTIONTIMES:
          OnFileConnectiontimes();
          break;

        case ID_FILE_BUSTYPES:
          OnFileBustypes();
          break;

        case ID_FILE_BUSES:
          OnFileBuses();
          break;

        case ID_FILE_SIGNCODES:
          OnFileSigncodes();
          break;

        case ID_FILE_COMMENTS:
          OnFileComments();
          break;

        case ID_FILE_TRIPS:
          OnFileTrips();
          break;

        case ID_FILE_STANDARDBLOCKS:
          OnFileStandardblocks();
          break;

        case ID_FILE_DROPBACKBLOCKS:
          OnFileDropbackblocks();
          break;

        case ID_FILE_RUNS:
          OnFileRuns();
          break;

        case ID_FILE_CREWONLYRUNS:
          OnFileCrewonlyruns();
          break;

        case ID_FILE_DRIVERTYPES:
          OnFileDrivertypes();
          break;

        case ID_FILE_DRIVERS:
          OnFileDrivers();
          break;

        case ID_FILE_BIDDINGROSTERING:
          OnFileBiddingrostering();
          break;

        case ID_FILE_DAILYOPERATIONS:
          OnFileDailyoperations();
          break;

        case ID_FILE_DISCIPLINE:
          OnFileDiscipline();
          break;

        case ID_FILE_TRIPPLANNING:
          OnFileTripplanning();
          break;

        case ID_FILE_COMPLAINTS:
          OnFileComplaints();
          break;

        case ID_FILE_TIMECHECKS:
          OnFileTimechecks();
          break;

        default:  // Handle the dynamic list of reports/downloads here
          s = pItem->GetCaption();
          for(thisReport = NO_RECORD, nI = 0; nI < m_LastReport; nI++)
          {
            if(strcmp(TMSRPT[nI].szReportName, s) == 0)
            {
              thisReport = nI;
              break;
            }
          }
//
//  Report not found
//
          if(thisReport == NO_RECORD)
          {
            break;
          }
//
//  Report found
//
          CMainFrame* pFrameWnd = (CMainFrame *)AfxGetApp()->m_pMainWnd->GetTopLevelFrame();
          memset(&PassedData, 0x00, sizeof(PassedData));
//
//  Call the report.  If it comes back as "Cancelled", break
//
          PassedData.nReportNumber = TMSRPT[thisReport].originalReportNumber;
          if(!(TMSRPT[thisReport].lpRoutine)(&PassedData))
          {
            break;
          }
//
//  If the "Do not use Crystal" flag comes back, skip the Crystal section
//
          if(PassedData.flags & TMSRPT_PASSEDDATAFLAG_DONOTUSECRYSTAL)
          {
            break;
          }
//
//  The ATC/Vancom data load affects the Routes Table - refresh it
//
          if(thisReport == 28)
          {
            HWND hWndRSDToolBar = pFrameWnd->m_dlgBar;
       
            SetUpRouteList(hWndRSDToolBar, RSDTOOLBAR_ROUTE, 1);
            SetUpServiceList(hWndRSDToolBar, RSDTOOLBAR_SERVICE, 1);
            SetUpDivisionList(hWndRSDToolBar, RSDTOOLBAR_DIVISION, 1);
          }
          nReportNumber = PassedData.nReportNumber;
          numDataFiles = PassedData.numDataFiles;
//
//  Get the template
//
          for(nI = 0; nI < TMSRPT[thisReport].numReports; nI++)
          {
            strcpy(szReportName, TMSRPT[thisReport].szReportName);
            strcpy(szReportFileName, TMSRPT[thisReport].szReportFileName[nI]);
//
//  Make sure the template exists
//
//  If it was NULL to start with, it means that this report doesn't use a template - so just leave
//
            if(strcmp(szReportFileName, "") == 0)
            {
              return 0;
            }
            fp = fopen(szReportFileName, "r");
            if(fp == NULL)
            {
              s.LoadString(ERROR_197);
              sprintf(tempString, s, szReportFileName);
              MessageBeep(MB_ICONSTOP);
              MessageBox(tempString, TMS, MB_OK);
              return 0;
            }
//
//  It does exist
//
            fclose(fp);
            for(nJ = 0; nJ < TMSRPT_MAX_REPORTOUTPUTFILES; nJ++)
            {
              strcpy(szReportDataFile[nJ], PassedData.szReportDataFile[nJ]);
            }
            result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
            if(result != 0)
            {
              LoadString(hInst, ERROR_240, szFormatString, sizeof(szFormatString));
              sprintf(tempString, szFormatString, sRegKey);
              MessageBeep(MB_ICONSTOP);
              MessageBox(tempString, TMS, MB_OK);
              return 0;
            }
//
//  Set the registry entries so Crystal knows where to look
//
            strcpy(szarString, szReportsTempFolder);
            RegSetValueEx(hKey, "DefaultDir", 0, type, (LPBYTE)szarString, strlen(szarString));
//
//  Copy the Schema.INI file from the install directory into the temporary report folder.
//  When we do this each time, we ensure that he's getting the most recent version
//
            char fileName[2][MAX_PATH];

            for(nJ = 0; nJ < 2; nJ++)
            {
              strcpy(fileName[nJ], nJ == 0 ? szInstallDirectory : szarString);
              strcat(fileName[nJ], "\\Schema.INI");
            }
            if(GetFileAttributes(fileName[0]) == 0xFFFFFFFF)  // Couldn't find INI file in szInstallDirectory
            {
              TMSError(NULL, MB_ICONSTOP, ERROR_218, (HANDLE)NULL);
              return 0;
            }
//
//  Found the INI file
//
            CopyFile(fileName[0], fileName[1], FALSE);
//
//  Fire up the status bar
//
            StatusBarStart(hWndMain, "Activating Crystal Interface");
            StatusBarText("Processing...");
//
//  Bring up the Crystal Dialog
//
            CCrystal dlg(this, szReportFileName, szReportName);
            dlg.DoModal();
            StatusBarEnd();
          }
        }
        break;
//
//  End of Reports and Downloads/Uploads cases
//
		}
		break;

	case XTP_TPN_RCLICK:
		{
			CXTPTaskPanelItem* pItem = (CXTPTaskPanelItem*)lParam;
			TRACE(_T("RClick Event: pItem.Caption = %s, pItem.ID = %i\n"), pItem->GetCaption(), pItem->GetID());

		}
		break;
	}

	return 0;
}

//
//  OnContextMenu()
//
//  Handle menu display and selection from a child window
//
void CMainFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  if(m_ContextMenu != NULL)
  {
    return;
  }

  CWnd* pChildWnd = pWnd->GetWindow(GW_CHILD);

  if(pChildWnd == NULL)
  {
    return;
  }
  long displayIndex = GetWindowLong(pChildWnd->m_hWnd, GWL_USERDATA);
  TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

  m_ContextMenu.LoadMenu(pTD->menuType);
  CMenu* pMenu = m_ContextMenu.GetSubMenu(0);
  if(pTD->fileInfo.fileNumber == TMS_TRIPS)
  {
    if(pMenu)
    {
      CMenu* pMenu2 = pMenu->GetSubMenu(3);
      if(pMenu2)
      {
        if(m_bShowNodeTimes)
        {
          pMenu2->CheckMenuItem(0, MF_BYPOSITION | MF_CHECKED);
        }
        else
        {
          pMenu2->CheckMenuItem(0, MF_BYPOSITION | MF_UNCHECKED);
        }
      }
    }
  }
  else if(pTD->fileInfo.fileNumber == TMS_STANDARDBLOCKS ||
        pTD->fileInfo.fileNumber == TMS_DROPBACKBLOCKS)
  {
    if(pMenu)
    {
      CMenu* pMenu2 = pMenu->GetSubMenu(3);
      if(pMenu2)
      {
        if(m_bCompressedBlocks)
        {
          pMenu2->CheckMenuItem(0, MF_BYPOSITION | MF_CHECKED);
        }
        else
        {
          pMenu2->CheckMenuItem(0, MF_BYPOSITION | MF_UNCHECKED);
        }
      }
    }
  }
  pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, pChildWnd, NULL);
  m_ContextMenu.DestroyMenu();
}

//
//  GetAvailableTableSlot()
//
//  Return the first available position in m_TableDisplay
//
int CMainFrame::GetAvailableTableSlot()
{
  int nI;
//
//
//  See what's available
//
  for(nI = 0; nI < MAXOPENTABLES; nI++)
  {
    if(m_TableDisplay[nI].pane == NULL)
    {
      return(nI);
    }
  }

  return(NO_RECORD);
}

//
//  OnWindowCloseallwindows()
//
//  Close all active MDI children and reset the internals in m_TableDisplay
//
void CMainFrame::OnWindowCloseallwindows() 
{
  int nI;

  for(nI = 0; nI < MAXOPENTABLES; nI++)
  {
    CChildFrame* pWnd = (CChildFrame *)m_TableDisplay[nI].pane;
    
    if(pWnd != NULL)
    {
      if(IsWindow(pWnd->m_hWnd))
      {
        pWnd->MDIDestroy();
      }
    }
    
    m_TableDisplay[nI].flags = 0;
    m_TableDisplay[nI].pane = NULL;
    if(IsWindow(m_TableDisplay[nI].F1))
    {
      m_TableDisplay[nI].F1.DestroyWindow();
    }
  }
}

//
//  OnWindowClosewindow()
//
//  Close the active MDI child and free up a slot in m_TableDisplay
//

void CMainFrame::OnWindowClosewindow() 
{
  CChildFrame* pChildWnd = (CChildFrame *)MDIGetActive();

  if(pChildWnd == NULL)
  {
    return;
  }

  HWND hActiveWindow = pChildWnd->m_hWnd;
  if(hActiveWindow)
  {
    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];
  
    if(pTD)
    {
      pTD->flags = 0;
      pTD->pane = NULL;
      pChildWnd->MDIDestroy();
      if(IsWindow(pTD->F1))
      {
        pTD->F1.DestroyWindow();
      }
    } 
  }
}

//
//  OnFileDirections()
//
//  Process the request to display the Directions Table
//
void CMainFrame::OnFileDirections() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_DIRECTIONS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_DIRECTIONS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}



void CMainFrame::OnFileRoutes() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_ROUTES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_ROUTES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileServices() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_SERVICES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_SERVICES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileJurisdictions() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_JURISDICTIONS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_JURISDICTIONS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileDivisions() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_DIVISIONS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_DIVISIONS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileNodesandstops() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_NODESANDSTOPS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_NODES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFilePatterns() 
{
  char dirAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  int  rcode2;
  int  nI;

  if(m_RouteRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_010, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE));
    return;
  }

  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  ROUTESKey0.recordID = m_RouteRecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  if(rcode2 != 0)
  {
  }
  else
  {
    CString sTableName;
    int tableSlot = GetAvailableTableSlot();
    int firstAssigned = tableSlot;

    for(nI = 0; nI < 2; nI++)
    {
      if(nI == 1)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          break;
        }
        tableSlot = GetAvailableTableSlot();
        if(tableSlot == NO_RECORD)
        {
          break;
        }
      }
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
      rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      if(rcode2 != 0)
      {
        break;
      }
      strncpy(dirAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(dirAbbr, DIRECTIONS_ABBRNAME_LENGTH);
      sTableName.Format("%s Patterns: Route %s - %s, Service %s", dirAbbr, m_RouteNumber, m_RouteName, m_ServiceName);

      CChildFrame* pChildWnd = new CChildFrame;
      pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

      pChildWnd->SetIcon(hIconTMS, TRUE);
      m_TableDisplay[tableSlot].pane = pChildWnd;
      m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
      m_TableDisplay[tableSlot].title = sTableName;
      m_TableDisplay[tableSlot].menuType = IDR_PATTERNS_CONTEXT;
      m_TableDisplay[tableSlot].fileInfo.routeRecordID = m_RouteRecordID;
      m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
      m_TableDisplay[tableSlot].fileInfo.directionIndex = nI;
      m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
      m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_PATTERNS;
      m_TableDisplay[tableSlot].fileInfo.keyNumber = 2;
      m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
      m_TableDisplay[tableSlot].flags = 0;
      SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
      CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
    }
    m_TableDisplay[firstAssigned].pane->SetFocus();
    MDITile(MDITILE_HORIZONTAL);

    CRect r;
    GetWindowRect(&r);
    InvalidateRect(r, TRUE);
  }
}

void CMainFrame::OnFileConnectiontimes() 
{
  if(m_RouteRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_010, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE));
    return;
  }

  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  
  sTableName.Format("Connection Times (Route/Service default to Route %s - %s, Service %s)", m_RouteNumber, m_RouteName, m_ServiceName);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_CONNECTIONS_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = m_RouteRecordID;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_CONNECTIONS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  CONN.numConnections = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
//
//  Go directly to the "Pick Connections" dialog box
//  if there are any connections in the database
//
  int rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  if(rcode2 == 0)
  {
    DISPLAYINFO DI;
    DI.fileInfo.routeRecordID = m_TableDisplay[tableSlot].fileInfo.routeRecordID;
    DI.fileInfo.serviceRecordID = m_TableDisplay[tableSlot].fileInfo.serviceRecordID;;
    if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PICKCONNECTION), hWndMain, (DLGPROC)PICKCONNECTIONMsgProc, (LPARAM)&DI))
    {
      CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
    }
  }
}

void CMainFrame::OnFileBustypes() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_BUSTYPES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_BUSTYPES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileBuses() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_BUSES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_BUSES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileSigncodes() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_SIGNCODES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_SIGNCODES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileComments() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_COMMENTS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_COMMENTS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileTrips() 
{
  char dirAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  int  rcode2;
  int  nI;

  if(m_RouteRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_010, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE));
    return;
  }

  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  ROUTESKey0.recordID = m_RouteRecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  if(rcode2 != 0)
  {
  }
  else
  {
    CString sTableName;
    int tableSlot = GetAvailableTableSlot();
    int firstAssigned = tableSlot;

    for(nI = 0; nI < 2; nI++)
    {
      if(nI == 1)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          break;
        }
        tableSlot = GetAvailableTableSlot();
        if(tableSlot == NO_RECORD)
        {
          break;
        }
      }
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
      rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      if(rcode2 != 0)
      {
        break;
      }
      strncpy(dirAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(dirAbbr, DIRECTIONS_ABBRNAME_LENGTH);
      sTableName.Format("%s Trips: Route %s - %s, Service %s", dirAbbr, m_RouteNumber, m_RouteName, m_ServiceName);

      CChildFrame* pChildWnd = new CChildFrame;
      pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

      pChildWnd->SetIcon(hIconTMS, TRUE);
      m_TableDisplay[tableSlot].pane = pChildWnd;
      m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
      m_TableDisplay[tableSlot].title = sTableName;
      m_TableDisplay[tableSlot].menuType = IDR_TRIPS_CONTEXT;
      m_TableDisplay[tableSlot].fileInfo.routeRecordID = m_RouteRecordID;
      m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
      m_TableDisplay[tableSlot].fileInfo.directionIndex = nI;
      m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
      m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_TRIPS;
      m_TableDisplay[tableSlot].fileInfo.keyNumber = 2;
      m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
      m_TableDisplay[tableSlot].flags = 0;
      m_numConnectionAlerts = 0;
      SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
      CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
    }
    CRect r;
    m_TableDisplay[firstAssigned].pane->SetFocus();
    MDITile(MDITILE_HORIZONTAL);
  }
}

void CMainFrame::OnFileStandardblocks() 
{
  if(m_RouteRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_010, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE));
    return;
  }

  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.Format("Standard Blocks: Route Group %s - %s, Service Group %s", m_RouteNumber, m_RouteName, m_ServiceName);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_BLOCKS_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = m_RouteRecordID;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_STANDARDBLOCKS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 2;
  m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileDropbackblocks() 
{
  if(m_RouteRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_010, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE));
    return;
  }

  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.Format("Dropback Blocks: Route Group %s - %s, Service Group %s", m_RouteNumber, m_RouteName, m_ServiceName);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_BLOCKS_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = m_RouteRecordID;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_DROPBACKBLOCKS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 3;
  m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileRuns() 
{
  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  if(m_DivisionRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_154, m_dlgBar.GetDlgItem(RSDTOOLBAR_DIVISION));
    return;
  }


//
//  Runs are special - prior to displaying the window, warn him if
//  there are any unblocked trips anywhere, and offer up the list if
//  wants to see it.  Note that this check has to be replicated in
//  restored() so that he doesn't come back into the system without
//  any warnings.
//
  int response = IDNO;
  int rcode2;

  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  if(rcode2 != 0)  // No Garages
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
    return;
  }
  
  BOOL bFirst = TRUE;
//
//  Only check standard blocks.
//
  while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
  {
    TRIPSKey2.assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
    TRIPSKey2.RGRPROUTESrecordID = NO_RECORD;
    TRIPSKey2.SGRPSERVICESrecordID = NO_RECORD;
    TRIPSKey2.blockNumber = 0;
    TRIPSKey2.blockSequence = NO_TIME;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  If we got a record with an RGRP == NO_RECORD, it mean that there's at
//  least one trip in the system that hasn't been blocked, so let him know about it.
//
    CString s;

    if(rcode2 == 0 && TRIPS.standard.RGRPROUTESrecordID == NO_RECORD)
    {
      s.LoadString(ERROR_079);
      MessageBeep(MB_ICONINFORMATION);
      response = MessageBox(s, TMS, MB_ICONINFORMATION | MB_YESNOCANCEL);
//
//  React to the response
//
      if(response == IDYES)
      {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UNBLOCKEDT), hWndMain, (DLGPROC)UNBLOCKEDTMsgProc);
        s.LoadString(ERROR_119);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
        {
          return;
        }
      }
    }
    if(bFirst)
    {
      bFirst = FALSE;
    }
    else
    {
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }
  }
//
//  And proceed
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuildService(FALSE, TRUE, m_ServiceRecordID))
      {
        return;
      }
    }
    else
    {
      InitTravelMatrix(m_ServiceRecordID, TRUE);
    }
  }

  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.Format("Runs: %s Division, %s", m_DivisionName, m_ServiceName);
  
  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_RUNS_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = m_DivisionRecordID;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_RUNS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileCrewonlyruns() 
{
  if(m_ServiceRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_011, m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE));
    return;
  }

  if(m_DivisionRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_154, m_dlgBar.GetDlgItem(RSDTOOLBAR_DIVISION));
    return;
  }

  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuildService(FALSE, TRUE, m_ServiceRecordID))
      {
        return;
      }
    }
    else
    {
      InitTravelMatrix(m_ServiceRecordID, TRUE);
    }
  }

  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.Format("Crew-only runs: %s Division, %s", m_DivisionName, m_ServiceName);
  
  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_CREWONLY_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = m_ServiceRecordID;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = m_DivisionRecordID;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_CREWONLY;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileDrivertypes() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_DRIVERTYPES);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_DRIVERTYPES;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileDrivers() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_DRIVERS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_DRIVERS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileBiddingrostering() 
{
  if(m_DivisionRecordID == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_154, m_dlgBar.GetDlgItem(RSDTOOLBAR_DIVISION));
    return;
  }

  if(m_RosterWeek == NO_RECORD)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_365, m_dlgBar.GetDlgItem(RSDTOOLBAR_ROSTERWEEK));
    return;
  }

  long serviceRecordID;
  int  rcode2;

  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuild(FALSE, TRUE))
      {
        return;
      }
    }
    else
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        serviceRecordID = SERVICES.recordID;
        InitTravelMatrix(serviceRecordID, TRUE);
        SERVICESKey0.recordID = serviceRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
    }
  }
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.Format("Roster: %s Division, Week %ld", m_DivisionName, m_RosterWeek + 1);
  
  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].menuType = IDR_ROSTER_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = m_DivisionRecordID;
  m_TableDisplay[tableSlot].fileInfo.rosterWeek = m_RosterWeek;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_ROSTER;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = FILE_CHECKRSDD;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}

void CMainFrame::OnFileDailyoperations() 
{
  long serviceRecordID;
  int  rcode2;

  if(!m_bEnableDailyOps)
  {
    return;
  }

  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuild(FALSE, TRUE))
      {
        return;
      }
    }
    else
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        serviceRecordID = SERVICES.recordID;
        InitTravelMatrix(serviceRecordID, TRUE);
        SERVICESKey0.recordID = serviceRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
    }
  }

  CDailyOps dlg;

  dlg.DoModal();
}

void CMainFrame::OnFileDiscipline() 
{
  CDiscipline dlg;

  dlg.DoModal();
}

void CMainFrame::OnFileTripplanning() 
{
  char szCommand[256];
  
  if(m_bAllowTripPlanning)
  {
    if(m_bUseMapInfo)
    {
      sprintf(szCommand, "Set Application Window %ld", this->m_hWnd);
      mapinfo.Do(szCommand);
      sprintf(szCommand, "Set Next Document Parent %ld Style 1", this->m_hWnd);
      mapinfo.Do(szCommand);
      if(strcmp(szStreetFile, "") != 0)
      {
        strcpy(szCommand, "Open Table \"");
        strcat(szCommand, szStreetFile);
        strcat(szCommand, "\" as StreetFile interactive");
        mapinfo.Do(szCommand);
        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRIPPLAN), this->m_hWnd, (DLGPROC)TRIPPLANMsgProc, (LPARAM)0);
        mapinfo.Do("Close All");
      }
    }
    else
    {
      LaunchProg(m_szTripPlannerURL);
    }
  }
  else
  {
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRIPPLAN), this->m_hWnd, (DLGPROC)TRIPPLANMsgProc, (LPARAM)0);
  }
}

void CMainFrame::OnFileComplaints() 
{
  CCCFrontEnd dlg;
  
  dlg.DoModal(); 
}

void CMainFrame::OnFileTimechecks() 
{
  int tableSlot = GetAvailableTableSlot();

  CString sTableName;
  sTableName.LoadString(TABLE_TIMECHECKS);

  CChildFrame* pChildWnd = new CChildFrame;
  pChildWnd->Create(m_lpszClass, sTableName, m_dwWindowStyle, rectDefault, this);

  pChildWnd->SetIcon(hIconTMS, TRUE);
  m_TableDisplay[tableSlot].pane = pChildWnd;
  m_TableDisplay[tableSlot].pane->InvalidateRect(NULL, TRUE);
  m_TableDisplay[tableSlot].title = sTableName;
  m_TableDisplay[tableSlot].menuType = IDR_GENERIC_CONTEXT;
  m_TableDisplay[tableSlot].fileInfo.routeRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.serviceRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.directionIndex = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.divisionRecordID = NO_RECORD;
  m_TableDisplay[tableSlot].fileInfo.fileNumber = TMS_TIMECHECKS;
  m_TableDisplay[tableSlot].fileInfo.keyNumber = 1;
  m_TableDisplay[tableSlot].fileInfo.flags = 0;
  m_TableDisplay[tableSlot].flags = 0;
  SetWindowLong(m_TableDisplay[tableSlot].pane->m_hWnd, GWL_USERDATA, (long)tableSlot);
  CTMSApp::GetTMSData(&m_TableDisplay[tableSlot], GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
}
//
//  File | Preferences
//
void CMainFrame::OnFilePreferences() 
{
  if(!DialogBox(hInst, MAKEINTRESOURCE(IDD_PREFERENCES), hWndMain, (DLGPROC)PREFERENCESMsgProc))
    return;
//
//  Determine which reports are (still/now) active
//
  int nI, nJ, nK;

  for(nI = 0; nI < m_LastReport; nI++)
  {
    nJ = nI / 32;
    nK = 1 << (nI % 32);
    if(TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE)
    {
      bmReportIsActive[nJ] |= nK;
    }
    else
    {
      if(bmReportIsActive[nJ] & nK)
      {
        bmReportIsActive[nJ] -= nK;
      }
    }
  }
//
//  Write the bitmasks out to the registry
//
  CWinApp* pApp = AfxGetApp();
  
  for(nI = 0; nI < TMSRPT_MAX_BITMASKS; nI++)
  {
    sprintf(tempString, "ActiveReportMask%02d", nI + 1);
    pApp->WriteProfileInt("Reports", tempString, bmReportIsActive[nI]);
  }
//
//  Redisplay the reports folder
//
  OnUpdateReportSelections();
}
