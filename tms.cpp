//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//  TMS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <afxole.h>
#include <afxsock.h>
extern "C" {
#define EXTERN
#include "TMSHeader.h"
#include "cistms.h"
#include "CCFrontEnd.h"
}

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "TMS.h"
#include "AVLInterface.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include <dos.h>
#include <direct.h>
#include "BitmapDialog.h"
#include <version.h>
#include "cshuser.h"
#include "mapinfow.h"   // ADDED FOR INTEGRATED MAPPING SUPPORT
#include "cderr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMSApp

BEGIN_MESSAGE_MAP(CTMSApp, CWinApp)
	//{{AFX_MSG_MAP(CTMSApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_EDIT_UPDATE, OnEditUpdate)
	ON_COMMAND(ID_BROWSE_COLUMN_CHARACTERISTICS, OnBrowseColumnCharacteristics)
	ON_COMMAND(ID_BROWSE_COLUMN_SORT, OnBrowseColumnSort)
	ON_COMMAND(ID_COMMANDS_ANALYZE_RUN, OnCommandsAnalyzeRun)
	ON_COMMAND(ID_COMMANDS_ASSIGN_GARAGES, OnCommandsAssignGarages)
	ON_COMMAND(ID_COMMANDS_ASSIGN_TRIP_NUMBERS, OnCommandsAssignTripNumbers)
	ON_COMMAND(ID_COMMANDS_ASSIGN_VEHICLE_TYPE, OnCommandsAssignVehicleType)
	ON_COMMAND(ID_COMMANDS_BLOCK_SUMMARY, OnCommandsBlockSummary)
	ON_COMMAND(ID_COMMANDS_BLOCK_TRIPS, OnCommandsBlockTrips)
	ON_COMMAND(ID_COMMANDS_COPY_PATTERNS, OnCommandsCopyPatterns)
	ON_COMMAND(ID_COMMANDS_COPY_TRIPS, OnCommandsCopyTrips)
	ON_COMMAND(ID_COMMANDS_COPYROSTER, OnCommandsCopyroster)
	ON_COMMAND(ID_COMMANDS_COPYRUNCUT, OnCommandsCopyruncut)
	ON_COMMAND(ID_COMMANDS_CUT_RUNS, OnCommandsCutRuns)
	ON_COMMAND(ID_COMMANDS_DELETE_RUNS, OnCommandsDeleteRuns)
	ON_COMMAND(ID_COMMANDS_ESTABLISHROSTERTEMPLATE, OnCommandsEstablishrostertemplate)
	ON_COMMAND(ID_COMMANDS_GRAPHICAL_TRIPS, OnCommandsGraphicalTrips)
	ON_COMMAND(ID_COMMANDS_HOOK_BLOCKS, OnCommandsHookBlocks)
	ON_COMMAND(ID_COMMANDS_NODE_FINDER, OnCommandsNodeFinder)
	ON_COMMAND(ID_COMMANDS_POPI, OnCommandsPopi)
	ON_COMMAND(ID_COMMANDS_RENUMBER_BLOCKS, OnCommandsRenumberBlocks)
	ON_COMMAND(ID_COMMANDS_RENUMBER_ROSTER, OnCommandsRenumberRoster)
	ON_COMMAND(ID_COMMANDS_RENUMBER_RUNS, OnCommandsRenumberRuns)
	ON_COMMAND(ID_COMMANDS_RENUMBER_RUNS_FROM_ROSTER, OnCommandsRenumberRunsFromRoster)
	ON_COMMAND(ID_COMMANDS_ROSTER_RUNS, OnCommandsRosterRuns)
	ON_COMMAND(ID_COMMANDS_RUN_SUMMARY, OnCommandsRunSummary)
	ON_COMMAND(ID_COMMANDS_SELECT_EXISTING_CONNECTIONS, OnCommandsSelectExistingConnections)
	ON_COMMAND(ID_COMMANDS_SET_LAYOVER, OnCommandsSetLayover)
	ON_COMMAND(ID_COMMANDS_SORT_TRIPS, OnCommandsSortTrips)
	ON_COMMAND(ID_COMMANDS_UNHOOK_TRIPS, OnCommandsUnhookTrips)
	ON_COMMAND(ID_EDIT_ADD_RUN_COMMENT, OnEditAddRunComment)
	ON_COMMAND(ID_EDIT_ASSIGN_DRIVER_TO_ROSTER, OnEditAssignDriverToRoster)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_EXTRABOARD_EDITOR, OnEditExtraboardEditor)
	ON_COMMAND(ID_EDIT_FIND_BLOCK, OnEditFindBlock)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_EDIT_INSERT_STOP_CONNECTION, OnEditInsertStopConnection)
	ON_COMMAND(ID_EDIT_REASSIGN_RUNTYPE, OnEditReassignRuntype)
	ON_COMMAND(ID_EDIT_REMOVE_DRIVER_FROM_ROSTER, OnEditRemoveDriverFromRoster)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_CUSTOMER_COMMENTS, OnEditUpdateRangeCustomerComments)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_LAYOVER, OnEditUpdateRangeLayover)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_OPERATOR_COMMENTS, OnEditUpdateRangeOperatorComments)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_PATTERNS, OnEditUpdateRangePatterns)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_SHIFT, OnEditUpdateRangeShift)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_SIGNCODES, OnEditUpdateRangeSigncodes)
	ON_COMMAND(ID_EDIT_UPDATE_RANGE_VEHICLES, OnEditUpdateRangeVehicles)
	ON_COMMAND(ID_EDIT_UPDATE_TRIP_NUMBER, OnEditUpdateTripNumber)
	ON_COMMAND(ID_RUN_CRYSTAL_REPORTS, OnRunCrystalReports)
	ON_COMMAND(ID_RUN_MAPINFO, OnRunMapinfo)
	ON_COMMAND(ID_SETUP_ROSTER_DRIVERSTOCONSIDER, OnSetupRosterDriverstoconsider)
	ON_COMMAND(ID_SETUP_ROSTER_PARAMETERS, OnSetupRosterParameters)
	ON_COMMAND(ID_SETUP_ROSTER_SERVICE_DAYS, OnSetupRosterServiceDays)
	ON_COMMAND(ID_SETUP_RUNS_OVERTIME, OnSetupRunsOvertime)
	ON_COMMAND(ID_SETUP_RUNS_PARAMETERS, OnSetupRunsParameters)
	ON_COMMAND(ID_SETUP_RUNS_PREMIUMS, OnSetupRunsPremiums)
	ON_COMMAND(ID_SETUP_RUNS_RELIEFPOINTS, OnSetupRunsReliefpoints)
	ON_COMMAND(ID_SETUP_RUNS_RUNTYPES, OnSetupRunsRuntypes)
	ON_COMMAND(ID_VIEW_TRIP_RUNNING_TIMES, OnViewTripRunningTimes)
	ON_COMMAND(ID_VIEW_COMPRESSED_BLOCKS, OnViewCompressedBlocks)
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_EDIT_INSERT_SD_CONNECTION, OnEditInsertSdConnection)
	ON_COMMAND(ID_COMMANDS_OPTIMALHOOK_BLOCKS, OnCommandsOptimalhookBlocks)
	ON_COMMAND(ID_EDIT_CREATEPLACEHOLDERRUNS, OnEditCreateplaceholderruns)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_WINDOW_CLOSEALLWINDOWS, OnWindowCloseallwindows)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTMSApp construction

CTMSApp::CTMSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

static const CLSID clsid =
{ 0x56fa6ee7, 0xc024, 0x452b, { 0x90, 0xa, 0x66, 0x8c, 0x32, 0x25, 0x8c, 0xd5 } };

/////////////////////////////////////////////////////////////////////////////
// The one and only CTMSApp object

CTMSApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTMSApp initialization

BOOL CTMSApp::InitInstance()
{
  CString s;

  m_bWorkrulesChanged = FALSE;
//
//  Initialize the common control library
//
  InitCommonControls();
//
//  Call the parent class method
//
  CWinApp::InitInstance();
//
//  Perform socket initialization
//	
  if(!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
//
//  Initialize OLE libraries
//
	if(!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
//
//  Enable us as a control container
//
	AfxEnableControlContainer();
//
//  Carry on
//  
//  Standard initialization
//
#if _MSC_VER <= 1200 // MFC 6.0 or earlier
#ifdef _AFXDLL
	Enable3dControls();         // Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif
#endif // MFC 6.0 or earlier

//
//  Get module details from VERSIONINFO
//
	DWORD dwVerInfoSize;
	DWORD dwHnd;
	void* pBuffer; 
	VS_FIXEDFILEINFO *pFixedInfo; // pointer to fixed file info structure
	LPVOID  lpVersion;            // String pointer to 'version' text
	UINT    uVersionLen;          // Current length of full version string
	TCHAR szGetName[500]; 

	dwVerInfoSize = GetFileVersionInfoSize("TMS.EXE", &dwHnd); 
  if(!dwVerInfoSize)
  {
    AfxMessageBox("Failed on GetFileVersionInfoSize\n\nTMS will now exit.");
    return FALSE;
  }

  pBuffer = malloc(dwVerInfoSize); 
  if(pBuffer == NULL)
  {
    AfxMessageBox("Failed on malloc of dwVerInfoSize\n\nTMS will now exit.");
    return FALSE;
  }

  GetFileVersionInfo("TMS.EXE", dwHnd, dwVerInfoSize, pBuffer); 
  VerQueryValue(pBuffer, _T("\\"), (void**)&pFixedInfo, (UINT *)&uVersionLen);
//
//  Company Name
//
  lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\CompanyName");  
  if(VerQueryValue(pBuffer, szGetName, (void**)&lpVersion, (UINT *)&uVersionLen) != 0)
  {
    strCompanyName = (LPTSTR)lpVersion;
  }
  else
  {
    free(pBuffer); 
    AfxMessageBox("Failed on VerQueryValue of CompanyName\n\nTMS will now exit.");
    return FALSE;
  }
//
//  Product Name
//
  lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\ProductName");  
  if(VerQueryValue(pBuffer, szGetName, (void**)&lpVersion, (UINT *)&uVersionLen) != 0)
  {
    strProductName = (LPTSTR)lpVersion;
  }
  else
  {
    free(pBuffer); 
    AfxMessageBox("Failed on VerQueryValue of ProductName\n\nTMS will now exit.");
    return FALSE;
  }
//
//  Legal Copyright
//
  lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\LegalCopyright");  
  if(VerQueryValue(pBuffer, szGetName, (void**)&lpVersion, (UINT *)&uVersionLen) != 0)
  {
    strLegalCopyright = (LPTSTR)lpVersion;
  }
//
//  Product Version
//
  strProductVersion.Format ("Version: %u.%u.%02u.%03u",
        HIWORD (pFixedInfo->dwProductVersionMS), LOWORD(pFixedInfo->dwProductVersionMS),
        HIWORD (pFixedInfo->dwProductVersionLS), LOWORD(pFixedInfo->dwProductVersionLS));
//
//  Release Date
//
  lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\FileDescription");  
  if(VerQueryValue(pBuffer, szGetName, (void**)&lpVersion, (UINT *)&uVersionLen) != 0)
  {
    strReleaseDate = (LPTSTR)lpVersion;
  }
 
  free(pBuffer); 
//
//  Establish userID for DailyOps;
//
  m_UserID = 0;
//
//  Get stuff out of the registry
//
  SetRegistryKey(strCompanyName);
//
//  Absolutely the easiest and most brainless
//  way to prevent multiple instances of TMS
//
  CStdioFile file;
  BOOL bAllowMultipleSessions = GetProfileInt("General", "AllowMultipleSessions", 0);

  if(!bAllowMultipleSessions)
  {

    if(!file.Open("C:\\Temp\\OneInstance", CFile::modeCreate | CFile::shareExclusive))
    {
      AfxMessageBox("TMS is already running\nor is in the process of starting up");
      return FALSE;
    }
  }
//
//  Database root
//
  s = GetProfileString("General", "DatabaseRoot");
  if(s == "")
  {
    s = "C:\\";
  }
  strcpy(szDatabaseRoot, s);
//
//  See if we're using the Strategic Mapping system here
//
  m_bUseStrategicMapping = GetProfileInt("General", "StrategicMapping", 0);
  if(m_bUseStrategicMapping)
  {
    m_AddAssignment = SMAddAssignment;
    m_DeleteAssignment = SMDelAssignment;
    m_AddDriver = SMAddDriver;
    m_AddBus = SMAddBus;
  }
//
//  See if we're using the Connexionz system here
//
  m_bUseConnexionz = GetProfileInt("General", "Connexionz", 0);
  if(m_bUseConnexionz)
  {
    m_AddAssignment = CNZAddAssignment;
    m_DeleteAssignment = CNZDelAssignment;
    m_AddDriver = CNZAddDriver;
    m_AddBus = CNZAddBus;
  }
//
//  Don't bother with MapInfo initialization if the registry
//  tells us that there's a web-based version available
//
  m_bUseMapInfo = GetProfileInt("Trip Planner", "UseMapInfo", 1);
  s = GetProfileString("Trip Planner", "URL");
  m_bAllowTripPlanning = TRUE;
//
//  MapInfo additions
//
//  If MapInfo isn't present, set m_bAllowTripPlanning to FALSE
//  to disable input on the address section of the trip planner dialog
//
  if(m_bUseMapInfo || s == "")
  {
    if(!mapinfo.CreateDispatch("MapInfo.Application"))
    {
      m_bAllowTripPlanning = FALSE;
    }
    else
    {
      AfxOleGetMessageFilter()->SetMessagePendingDelay(-1);
      AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);
    }
  }
//
//  Not using MapInfo for the trip planner - set up the URL
//
  else
  {
    strcpy(m_szTripPlannerURL, s);
  }
//
//  Property name
//
  s = GetProfileString("General", "PropertyName");
  if(s == "")
  {
    s = strCompanyName;
  }
  strcpy(szPropertyName, s);
//
//  Get the install directory
//
  s = GetProfileString("General", "InstallDirectory");
  if(s == "")
  {
    s = "C:\\Program Files\\Schedule Masters, Inc\\The Master Scheduler";
    WriteProfileString("General", "InstallDirectory", s);
  }
  strcpy(szInstallDirectory, s);
//
//  Get this machine's rights mask
//
  m_UserAccessRights = GetProfileInt("General", "Rights", 0);
//
//  Ensure that if bidding is set, he's got access to the roster and driver files
//
  if(m_UserAccessRights & RIGHTS_BIDDING)
  {
    m_UserAccessRights |= RIGHTS_ROSTER;
    m_UserAccessRights |= RIGHTS_DRIVERS;
    m_UserAccessRights |= RIGHTS_DRIVERTYPES;
  }
//
//  Does this guy have access to Daily Ops?
//
  m_bEnableDailyOps = GetProfileInt("Daily Operations", "Enable", 0);
//
//  If so, give him access to bus types, buses, driver types, and drivers
//
  if(m_bEnableDailyOps)
  {
    m_UserAccessRights |= RIGHTS_DAILYOPS;
    m_UserAccessRights |= RIGHTS_DRIVERTYPES;
    m_UserAccessRights |= RIGHTS_DRIVERS;
    m_UserAccessRights |= RIGHTS_BUSTYPES;
    m_UserAccessRights |= RIGHTS_BUSES;
  }
//
//  Are we monitoring checkins?
//
  m_bMonitorCheckins = GetProfileInt("Daily Operations", "Operator Check-In", 1);
//
//  Is there more than one machine?
//
  m_bMultipleDispatchMachines = GetProfileInt("Daily Operations", "MultipleDispatchMachines", 0);
//
//  Is there a magic number to offset the pegboard display?
//
  m_PegboardMagicNumber = GetProfileInt("Daily Operations", "PegboardDisplayOffset", 55);
//
//  Pegboard Colors
//
//  Can't use GetProfileInt here, as we're dealing in DWORDS
//
  HKEY  hKey;
  DWORD dwBufLen = sizeof(DWORD);
  DWORD dwRetVal;
  LONG  lRet;
  BOOL  bGotColors[6];

  memset(&bGotColors, 0x00, sizeof(BOOL) * 6);

  if(RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Schedule Masters, Inc.\\TMS - The Master Scheduler\\Daily Operations"),
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    lRet = RegQueryValueEx(hKey, TEXT("PBColorAvailable"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[0] = TRUE;
      m_PBCOLORS.crAvailable = (COLORREF)dwRetVal;
    }
   
    lRet = RegQueryValueEx(hKey, TEXT("PBColorAssigned"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[1] = TRUE;
      m_PBCOLORS.crAssigned = (COLORREF)dwRetVal;
    }
    
    lRet = RegQueryValueEx(hKey, TEXT("PBColorOutOfService"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[2] = TRUE;
      m_PBCOLORS.crOutOfService = (COLORREF)dwRetVal;
    }
    
    lRet = RegQueryValueEx(hKey, TEXT("PBColorShortShift"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[3] = TRUE;
      m_PBCOLORS.crShortShift = (COLORREF)dwRetVal;
    }
   
    lRet = RegQueryValueEx(hKey, TEXT("PBColorCharterReserve"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[4] = TRUE;
      m_PBCOLORS.crCharterReserve = (COLORREF)dwRetVal;
    }
   
    lRet = RegQueryValueEx(hKey, TEXT("PBColorSightseeingReserve"), NULL, NULL, (LPBYTE)&dwRetVal, &dwBufLen);
    if(lRet == ERROR_SUCCESS)
    {
      bGotColors[5] = TRUE;
      m_PBCOLORS.crSightseeingReserve = (COLORREF)dwRetVal;
    }
    RegCloseKey(hKey);
  }
//
//  If we don't have colors, or if they've come back as black, set up the default colors
//
  if(!bGotColors[0] || m_PBCOLORS.crAvailable == RGB(0, 0, 0))
  {
    m_PBCOLORS.crAvailable = RGB(0, 255, 0);            // Green
  }
  if(!bGotColors[1] || m_PBCOLORS.crAssigned == RGB(0, 0, 0))
  {
    m_PBCOLORS.crAssigned = RGB(255, 255, 0);           // Yellow
  }
  if(!bGotColors[2] || m_PBCOLORS.crOutOfService == RGB(0, 0, 0))
  {
    m_PBCOLORS.crOutOfService = RGB(255, 0, 0);         // Red
  }
  if(!bGotColors[3] || m_PBCOLORS.crShortShift == RGB(0, 0, 0))
  {
    m_PBCOLORS.crShortShift = RGB(0, 0, 255);           // Blue
  }
  if(!bGotColors[4] || m_PBCOLORS.crCharterReserve == RGB(0, 0, 0))
  {
    m_PBCOLORS.crCharterReserve = RGB(255, 128, 0);     // Orange
  }
  if(!bGotColors[5] || m_PBCOLORS.crSightseeingReserve == RGB(0, 0, 0))
  {
    m_PBCOLORS.crSightseeingReserve = RGB(255, 0, 255); // Pink
  }
//
//  Backwards compatibility for stuff not moved from TMS.INI yet
//
  GetWindowsDirectory((LPSTR)TMSINIFile, sizeof(TMSINIFile));
  strcat(TMSINIFile, "\\TMS.INI");
//
//  Default directory (folder where last database was opened)
//
  s = GetProfileString("General", "DefaultDirectory");
  if(s == "")
  {
    s = "C:\\";
  }
  chdir(s);
//
//  Discipline Letter Templates
//
  strcpy(szDisciplineLetterTemplateFolder, szInstallDirectory);
  strcat(szDisciplineLetterTemplateFolder, "\\Discipline Letter Templates");
//
//  Crystal Reports
//
  s = GetProfileString("Reports", "Path");
  strcpy(szCrystalReportsPath, s);
//
//  Report templates
//
  strcpy(szReportTemplateFolder, szInstallDirectory);
  strcat(szReportTemplateFolder, "\\MSJet Report Templates");
//
//  Initialize the TMS reports
//
  TMSRpt();
//
//  Mapping program
// 
  s = GetProfileString("Mapping", "Path");
  strcpy(szMappingProgram, s);
//
//  Street map
//
  s = GetProfileString("Mapping", "StreetFile");
  strcpy(szStreetFile, s);
//
//  Street column
//
  s = GetProfileString("Mapping", "StreetColumn");
  if(s == "")
  {
    s = "Street";
  }
  strcpy(szStreetColumn, s);
//
//  Landmark file
//
  s = GetProfileString("Mapping", "LandmarkFileText");
  if(s == "")
  {
    s = GetProfileString("Mapping", "LandmarkFile");
  }
  strcpy(szLandmarkFile, s);
//
//  Use Lat/Long
//
  m_bUseLatLong = GetProfileInt("Mapping", "UseLatLong", 1);
//
//  Coordsys
//
  s = GetProfileString("Mapping", "Coordsys");
  strcpy(szCoordsys, s);
//
//  Route Tracings Folder
//
  s = GetProfileString("Mapping", "RouteTracings");
  strcpy(szRouteTracingsFolder, s);
//
//  UTM Zone
//
  s = GetProfileString("Mapping", "UTMZone");
  strcpy(szUTMZone, s);
//
//  Enable context-sensitive help
//
//  CSHInitialize("context.hlp", FALSE);
//
//  Establish Help file
//
  strcpy(szarHelpFile, szInstallDirectory);
  strcat(szarHelpFile, "\\TMS.HLP");
//
//  Set up hourglass and arrow cursors, and the TMS icon
//
  hCursorWait  = LoadStandardCursor(IDC_WAIT);
  hCursorArrow = LoadStandardCursor(IDC_ARROW);
  hIconTMS     = LoadIcon(IDR_MAINFRAME);
//
//  And who we are
//
  s.LoadString(TEXT_065);
  strcpy(TMS, s);
//
//  Connection alerts
//
  m_numConnectionAlerts = 0;
//
//  Create the main MDI Frame window
//
	CMainFrame* pMainFrame = new CMainFrame;

	if(!pMainFrame->LoadFrame(IDR_MAINFRAME))
  {
    AfxMessageBox("Failed on LoadFrame of IDR_MAINFRAME\n\nTMS will now exit.");
		return FALSE;
  }
	m_pMainWnd = pMainFrame;
//
//  Zero out the table displays
//
  int nI;

  for(nI = 0; nI < MAXOPENTABLES; nI++)
  {
    m_TableDisplay[nI].pane = NULL;
  }
//
//  Allocate space for the RUNTIMES array
//
  m_pRUNTIMES = (CONNECTIONSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CONNECTIONSDef) * MAXCONNECTIONS); 
  if(m_pRUNTIMES == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }
//
//  Allocate space for the text portion of the COMMENTS and COMPLAINTS files
//
  pCommentText = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, COMMENTS_TOTAL_LENGTH); 
  if(pCommentText == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }

  pComplaintText = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, COMPLAINTS_TOTAL_LENGTH); 
  if(pComplaintText == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }
//
//  Zero out numInTM[] - The number of entries in the dynamic travels matrix
//
  memset(numInTM, 0x00, sizeof(numInTM));
//
//  Estable the TRUE and FALSE strings
//
  s.LoadString(TEXT_053);
  strcpy(szTRUE, s);
  s.LoadString(TEXT_054);
  strcpy(szFALSE, s);
//
//  Load up the month name abbrviations array
//
  int nJ;
  
  for(nI = TEXT_105, nJ = 0; nI <= TEXT_116; nI++)
  {
    LoadString(hInst, nI, tempString, TEMPSTRING_LENGTH);
    tempString[3] = '\0';
    strcpy(szMonth[nJ], tempString);
    nJ++;
  }
//
//  Set up the btrieve file record lengths
//
  recordLength[FILE_DDF] = sizeof(FILEDDF);
  recordLength[FIELD_DDF] = sizeof(FIELDDDF);
  recordLength[INDEX_DDF] = sizeof(INDEXDDF);
  recordLength[TMS_ATTRIBUTES] = sizeof(ATTRIBUTES);
  recordLength[TMS_CREATED] = sizeof(CREATED);
  recordLength[TMS_TABLEVIEW] = sizeof(TABLEVIEW);
  recordLength[TMS_DIRECTIONS] = sizeof(DIRECTIONS);
  recordLength[TMS_ROUTES] = sizeof(ROUTES);
  recordLength[TMS_SERVICES] = sizeof(SERVICES);
  recordLength[TMS_JURISDICTIONS] = sizeof(JURISDICTIONS);
  recordLength[TMS_NODES] = sizeof(NODES);
  recordLength[TMS_DIVISIONS] = sizeof(DIVISIONS);
  recordLength[TMS_PATTERNS] = sizeof(PATTERNS);
  recordLength[TMS_PATTERNNAMES] = sizeof(PATTERNNAMES);
  recordLength[TMS_CONNECTIONS] = sizeof(CONNECTIONS);
  recordLength[TMS_BUSTYPES] = sizeof(BUSTYPES);
  recordLength[TMS_COMMENTS] = sizeof(COMMENTS);
  recordLength[TMS_SIGNCODES] = sizeof(SIGNCODES);
  recordLength[TMS_TRIPS] = sizeof(TRIPS);
  recordLength[TMS_RUNS] = sizeof(RUNS);
  recordLength[TMS_ROSTER] = sizeof(ROSTER);
  recordLength[TMS_DRIVERS] = sizeof(DRIVERS);
  recordLength[TMS_DRIVERTYPES] = sizeof(DRIVERTYPES);
  recordLength[TMS_BLOCKNAMES] = sizeof(BLOCKNAMES);
  recordLength[TMS_CUSTOMERS] = sizeof(CUSTOMERS);
  recordLength[TMS_COMPLAINTS] = sizeof(COMPLAINTS);
  recordLength[TMS_ROUTINGS] = sizeof(ROUTINGS);
  recordLength[TMS_TIMECHECKS] = sizeof(TIMECHECKS);
  recordLength[TMS_BUSES] = sizeof(BUSES);
  recordLength[TMS_DISCIPLINE] = sizeof(DISCIPLINE);
  recordLength[TMS_CREWONLY] = sizeof(CREWONLY);
  recordLength[TMS_DAILYOPS] = sizeof(DAILYOPS);
  recordLength[TMS_OFFTIME] = sizeof(OFFTIME);
  recordLength[TMS_RIDERSHIP] = sizeof(RIDERSHIP);
//
//  Btrieve client setup
//
  memset(clientID.networkAndNode, 0, sizeof(clientID.networkAndNode));
  strcpy(clientID.applicationID, "MT" );  // must be greater than "AA" 
  clientID.threadID = 51;//MY_THREAD_ID;
//
//  Set up the default display font
//
  DWORD dwBaseUnits = GetDialogBaseUnits();
  lfCurrent.lfHeight = HIWORD(dwBaseUnits);
  lfCurrent.lfWidth = 0;
  lfCurrent.lfEscapement = 0;
  lfCurrent.lfOrientation = 0;
  lfCurrent.lfItalic = 0;
  lfCurrent.lfUnderline = 0;
  lfCurrent.lfStrikeOut = 0;
  lfCurrent.lfCharSet = ANSI_CHARSET;
  lfCurrent.lfOutPrecision = 0;
  lfCurrent.lfClipPrecision = 0;
  lfCurrent.lfQuality = DRAFT_QUALITY;
  lfCurrent.lfPitchAndFamily = DEFAULT_PITCH + FF_SWISS;
  lfCurrent.lfFaceName[0] = 0;
  lfCurrent.lfWeight = FW_BOLD;
  hBFont = CreateFontIndirect(&lfCurrent);
  lfCurrent.lfWeight = FW_REGULAR;
  hMFont = CreateFontIndirect(&lfCurrent);
  rgbColor = RGB(0, 0, 0);
//
//  Set up the Base Pattern Name
//
  strcpy(basePatternName, BASE_PATTERN_NAME);
//
//  External text files
//
//  TextConvert() will load up the access database if necessary
//
  TextConvert();
//
//  TextLoad() takes the access database into core
//
  TextLoad();
//
//  Make the pointer to incore node abbreviations NULL
//
  m_pNodeAbbrs = NULL;
//
//  And make sure that databaseFileName is "" in case he leaves without opening anything
//
  strcpy(szDatabaseFileName, "");
//
//  The one and only window has been initialized, so show and update it
//
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
//
//  Set the application title
//
  s.LoadString(AFX_IDS_APP_TITLE);
  m_pMainWnd->SetWindowText(s);
//
//  Display the greeting dialog
//
  while(TRUE)
  {
    m_bDatabaseOpen = FALSE;
    m_aboutFlag = ABOUTBOX_FLAG_OPEN;
    OnAppAbout();

    if(m_aboutResult & ABOUTBOX_RESULT_EXIT)
    {
      TMSError(m_pMainWnd->m_hWnd, MB_ICONSTOP, ERROR_362, (HANDLE)NULL);
      return FALSE;
    }
  
    m_aboutFlag = ABOUTBOX_FLAG_ABOUT;
//
//  Open a database
//
    if(m_aboutResult & ABOUTBOX_RESULT_OPEN)
    {
      if(OnFileOpen() == IDCANCEL)
      {
        continue;
      }
      m_bDatabaseOpen = TRUE;
      break;
    }
//
//  Create a new database
//
    else
    {
      if(OnFileNew() == IDCANCEL)
      {
        continue;
      }
      m_bDatabaseOpen = TRUE;
      break;
    }
  }
//
//  Read the workrules
//
  if(!ReadWorkrules(m_pMainWnd->m_hWnd))
  {
    TMSError(m_pMainWnd->m_hWnd, MB_ICONSTOP, ERROR_084, (HANDLE)NULL);
    return FALSE;
  }
//
//  Allocate space for the m_pRELIEFPOINTS structure
//
  m_maxRELIEFPOINTS = 1024;
  m_pRELIEFPOINTS = (RELIEFPOINTSDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RELIEFPOINTSDef) * m_maxRELIEFPOINTS); 
  if(m_pRELIEFPOINTS == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }
//
//  Allocate space for m_pBLOCKDATA
//
  m_maxBLOCKDATA = 128;
  m_pBLOCKDATA = (BLOCKDATADef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BLOCKDATADef) * m_maxBLOCKDATA); 
  if(m_pBLOCKDATA == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }
//
//  Allocate space for m_pRunRecordData
//
  m_maxRunRecords = 200;
  m_pRunRecordData = (RunRecordDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RunRecordDef) * m_maxRunRecords); 
  if(m_pRunRecordData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return FALSE;
  }

//
//  Set pointers to NULL for structures to be allocated later
//
  m_maxPICKEDTRIPS = 512;
  m_pPICKEDTRIPS = NULL;
//
//  Default to considering all runtypes in the RunCoster
//
  m_bCheckUnderConsideration = FALSE;
//
//  Reset the application title
//
  s.LoadString(AFX_IDS_APP_TITLE);
  s += ": \"";
  s += szDatabaseDescription;
  s += "\", located in folder ";
  s += szDatabaseFileName;
  m_pMainWnd->SetWindowText(s);
  
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// CTMSApp message handlers


BOOL CTMSApp::PreTranslateMessage(MSG* pMsg)
{
	return CWinApp::PreTranslateMessage(pMsg);
}

int CTMSApp::ExitInstance() 
{
//
//  Delete some font objects
//
  DeleteObject(hBFont);
  DeleteObject(hMFont);
//
//  Temp: Strategic Mapping
//
  if(m_bUseStrategicMapping)
  {
    if(BusAssignmentTransactions.m_pStream)
    {
      BusAssignmentTransactions.Close();
    }
  }
//
//  Close out the grid (if necessary)
//
  COleMessageFilter* pFilter = AfxOleGetMessageFilter();
  ULONG lCount = pFilter->m_xMessageFilter.Release();

  if(lCount<=1)
  {
    pFilter->m_xMessageFilter.AddRef();
  }
//
//  Free m_pRUNTIMES and pCommentText
//
  TMSHeapFree(m_pRUNTIMES);
  TMSHeapFree(pCommentText);
  TMSHeapFree(pComplaintText);
//
//  Free m_pRELIEFPOINTS and m_pBLOCKDATA and m_pRunRecordData
//
  TMSHeapFree(m_pRELIEFPOINTS); 
  TMSHeapFree(m_pBLOCKDATA);
  TMSHeapFree(m_pRunRecordData); 
//
//  Free the trip planner
//
  CISfree();
//
//  Save the date and time into the .TMS file
//
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
  
  strcpy(tempString, strTime);
  WritePrivateProfileString("Description", "LastAccessDate", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
//
//  Save the Pegboard Colors into the registry
//
  HKEY hKey;
  DWORD dwBufLen = sizeof(COLORREF);
  LONG lRet;

  lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Schedule Masters, Inc.\\TMS - The Master Scheduler\\Daily Operations"), 0, KEY_ALL_ACCESS, &hKey);
  if(lRet == ERROR_SUCCESS)
  {
    lRet = RegSetValueEx(hKey, TEXT("PBColorAvailable"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crAvailable, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet1 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    lRet = RegSetValueEx(hKey, TEXT("PBColorAssigned"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crAssigned, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet2 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    lRet = RegSetValueEx(hKey, TEXT("PBColorOutOfService"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crOutOfService, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet3 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    lRet = RegSetValueEx(hKey, TEXT("PBColorShortShift"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crShortShift, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet4 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    lRet = RegSetValueEx(hKey, TEXT("PBColorCharterReserve"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crCharterReserve, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet5 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    lRet = RegSetValueEx(hKey, TEXT("PBColorSightseeingReserve"), NULL, REG_DWORD, (CONST BYTE *)&m_PBCOLORS.crSightseeingReserve, dwBufLen);
    if(lRet != ERROR_SUCCESS)
    {
      sprintf(tempString, "lRet6 = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(NULL, tempString, TMS, MB_OK);
    }
    RegCloseKey(hKey);
  }
  else
  {
    sprintf(tempString, "lRet = %ld on RegOpenKeyEx\nPlease contact Schedule Masters, Inc.", lRet);
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(NULL, tempString, TMS, MB_OK);
  }
//
//  Reset Btrieve (this also closes all the open TMS data files)
//
  btrieve(B_RESET, 0, 0, 0, 0);
  btrieve(B_STOP,  0, 0, 0, 0);
// ADDED FOR INTEGRATED MAPPING SUPPORT
  if(m_bAllowTripPlanning && m_bUseMapInfo)
  {
    mapinfo.ReleaseDispatch();
  }
// END OF ADDITION FOR INTEGRATED MAPPING SUPPORT

  _CrtDumpMemoryLeaks();

  return CWinApp::ExitInstance();
}

BOOL CTMSApp::OnIdle(LONG lCount) 
{
  HCURSOR hSaveCursor;
//
//  Write out the workrules file
//
  if(m_bWorkrulesChanged)
  {
    hSaveCursor = SetCursor(hCursorWait);
    writeWorkrules(hWndMain);
    SetCursor(hSaveCursor);
  }
	
	return CWinApp::OnIdle(lCount);
}

void CTMSApp::OnHelpContents() 
{
 ::WinHelp(hWndMain, szarHelpFile, HELP_FINDER, 0L);
}


//
//  OnWindowCloseallwindows()
//
//  Close all active MDI children and reset the internals in m_TableDisplay
//
void CTMSApp::OnWindowCloseallwindows() 
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
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CBitmapDialog 
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// App command to run the dialog
void CTMSApp::OnAppAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

CAboutDlg::CAboutDlg() : CBitmapDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CBitmapDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(ABOUTBOX_BUTTON1, OnButton1)
	ON_BN_CLICKED(ABOUTBOX_BUTTON2, OnButton2)
	ON_BN_CLICKED(ABOUTBOX_BUTTON3, OnButton3)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static	LONG	GetRegKey( HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY	hkey;
	LONG	retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if( retval == ERROR_SUCCESS )
	{
		long	datasize = MAX_PATH;
		TCHAR	data[MAX_PATH];
		RegQueryValue( hkey, NULL, data, &datasize );
		lstrcpy( retdata, data);
		RegCloseKey(hkey);
	}
	return retval;
}

void	LaunchProg( const CString &url )
{
	TCHAR key[MAX_PATH + MAX_PATH];

	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_NORMAL);
	if( (UINT)result <= HINSTANCE_ERROR )
	{
		if( GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS )
		{
			lstrcat(key, _T("\\shell\\open\\command"));
			if( GetRegKey(HKEY_CLASSES_ROOT, key, key) == ERROR_SUCCESS )
			{
				TCHAR	*pos = _tcsstr(key, _T("\"%1\""));
				if( pos == NULL )
				{
					// No quotes found.
					pos = strstr(key, _T("%1"));
					if( pos == NULL )
						pos = key + lstrlen(key) - 1;
					else
						*pos = '\0';
				}
				else
					*pos = '\0';

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE)WinExec(key, SW_SHOWNORMAL);
			}
		}
	}
}
//
//  "About" Dialog Handling
//

BOOL CAboutDlg::OnInitDialog()
{
	CBitmapDialog::OnInitDialog();

  SetBitmap(IDB_WATCH, CBitmapDialog::BITMAP_TOPLEFT);

	CString str;

  CStatic* pStaticPRODUCTNAME = (CStatic *)GetDlgItem(ABOUTBOX_PRODUCTNAME);
  CStatic* pStaticLEGALCOPYRIGHT = (CStatic *)GetDlgItem(ABOUTBOX_LEGALCOPYRIGHT);
  CStatic* pStaticVERSION = (CStatic *)GetDlgItem(ABOUTBOX_VERSION);
  CStatic* pStaticRELEASEDATE = (CStatic *)GetDlgItem(ABOUTBOX_RELEASEDATE);
  CButton* pButtonBUTTON1 = (CButton *)GetDlgItem(ABOUTBOX_BUTTON1);
  CButton* pButtonBUTTON2 = (CButton *)GetDlgItem(ABOUTBOX_BUTTON2);
  CButton* pButtonBUTTON3 = (CButton *)GetDlgItem(ABOUTBOX_BUTTON3);

  pStaticPRODUCTNAME->SetWindowText(strProductName);
  pStaticLEGALCOPYRIGHT->SetWindowText(strLegalCopyright);
  pStaticVERSION->SetWindowText(strProductVersion);
  pStaticRELEASEDATE->SetWindowText(strReleaseDate);
//
//  See why we're here
//
//  He hit Help | About
//
  if(m_aboutFlag & ABOUTBOX_FLAG_ABOUT)
  {
    SetWindowText("About The Master Scheduler");
    pButtonBUTTON1->SetWindowText("Send us email");
    pButtonBUTTON2->SetWindowText("Visit us on the web");
    pButtonBUTTON3->SetWindowText("&OK");
  }
	
  return TRUE;

}

void CAboutDlg::OnButton1() 
{
  if(m_aboutFlag & ABOUTBOX_FLAG_ABOUT)
	  LaunchProg( "mailto:support@themasterscheduler.com" );
  else
  {
    m_aboutResult = ABOUTBOX_RESULT_OPEN;
    CBitmapDialog::OnOK();
  }
}
    
void CAboutDlg::OnButton2() 
{
  if(m_aboutFlag & ABOUTBOX_FLAG_ABOUT)
    LaunchProg( "http://www.themasterscheduler.com" );
  else
  {
    m_aboutResult = ABOUTBOX_RESULT_NEW;
    CBitmapDialog::OnOK();
  }
}

void CAboutDlg::OnButton3() 
{
  if(m_aboutFlag & ABOUTBOX_FLAG_ABOUT)
    m_aboutResult = ABOUTBOX_RESULT_OK;
  else
    m_aboutResult = ABOUTBOX_RESULT_EXIT;

	CBitmapDialog::OnOK();
}

void CAboutDlg::OnClose() 
{
	OnButton3();
}
