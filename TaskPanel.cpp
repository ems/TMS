//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//

#include "stdafx.h"
#include <afxole.h>
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include <dos.h>
#include <direct.h>
#include "BitmapDialog.h"
#include <version.h>
#include "cshuser.h"
#include "mapinfow.h"   // ADDED FOR INTEGRATED MAPPING SUPPORT

#include "MainFrm.h"
#include "TaskPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTaskPanel

CTaskPanel::CTaskPanel()
{
}

CTaskPanel::~CTaskPanel()
{
}


BEGIN_MESSAGE_MAP(CTaskPanel, CTaskPanelBase)
	//{{AFX_MSG_MAP(CTaskPanel)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTaskPanel message handlers

BOOL CTaskPanel::Create(CWnd* pParentWnd)
{
#ifdef _XTP_INCLUDE_TASKPANEL
	if (!CXTPTaskPanel::Create(WS_CHILD|WS_VISIBLE, CRect(0, 0, 0, 0),  pParentWnd, 0))
		return FALSE;

//	CXTPImageManager* pImageManager = XTPImageManager();

//	pImageManager->InternalAddRef();
//	SetImageManager(pImageManager);


//	SetExpandable(FALSE);

//
//  Tables
//
	CXTPTaskPanelGroup* pGroupTables = AddGroup(ID_TASKPANEL_TABLES);
	pGroupTables->SetCaption(_T("Tables"));
	pGroupTables->SetExpanded(TRUE);
	pGroupTables->SetSpecialGroup();


	CXTPTaskPanelGroupItem* pTableItem = pGroupTables->AddLinkItem(ID_FILE_DIRECTIONS, 0);
	pTableItem->SetCaption(_T("Directions"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_ROUTES, 1);
	pTableItem->SetCaption(_T("Routes"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_SERVICES, 2);
	pTableItem->SetCaption(_T("Services"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_JURISDICTIONS, 3);
	pTableItem->SetCaption(_T("Jurisdictions"));
	
  pTableItem = pGroupTables->AddLinkItem(ID_FILE_DIVISIONS, 4);
	pTableItem->SetCaption(_T("Divisions"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_NODESANDSTOPS, 5);
	pTableItem->SetCaption(_T("Nodes and Stops"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_PATTERNS, 6);
	pTableItem->SetCaption(_T("Patterns"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_CONNECTIONTIMES, 7);
	pTableItem->SetCaption(_T("Connection Times"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_BUSTYPES, 8);
	pTableItem->SetCaption(_T("Bus Types"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_BUSES, 9);
	pTableItem->SetCaption(_T("Buses"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_SIGNCODES, 10);
	pTableItem->SetCaption(_T("Sign Codes"));

	pTableItem = pGroupTables->AddLinkItem(ID_FILE_COMMENTS, 11);
	pTableItem->SetCaption(_T("Comments"));
//
//  Trips and blocks
//
	CXTPTaskPanelGroup* pGroupTrips = AddGroup(ID_TASKPANEL_TRIPSBLOCKS);
	pGroupTrips->SetCaption(_T("Trips and Blocks"));
	pGroupTrips->SetExpanded(FALSE);

	CXTPTaskPanelGroupItem* pTripsItem = pGroupTrips->AddLinkItem(ID_FILE_TRIPS, 12);
	pTripsItem->SetCaption(_T("Trips"));

	pTripsItem = pGroupTrips->AddLinkItem(ID_FILE_STANDARDBLOCKS, 13);
	pTripsItem->SetCaption(_T("Standard Blocks"));

	pTripsItem = pGroupTrips->AddLinkItem(ID_FILE_DROPBACKBLOCKS, 14);
	pTripsItem->SetCaption(_T("Dropback Blocks"));
//
//  Runs
//
	CXTPTaskPanelGroup* pGroupRuns = AddGroup(ID_TASKPANEL_RUNCUTTING);
	pGroupRuns->SetCaption(_T("Runs"));
	pGroupRuns->SetExpanded(FALSE);

	CXTPTaskPanelGroupItem* pRunsItem = pGroupRuns->AddLinkItem(ID_FILE_RUNS, 15);
	pRunsItem->SetCaption(_T("Runs"));

	pRunsItem = pGroupRuns->AddLinkItem(ID_FILE_CREWONLYRUNS, 16);
	pRunsItem->SetCaption(_T("Crew Only Runs"));
//
//  Workforce Management
//
	CXTPTaskPanelGroup* pGroupWorkforceManagement = AddGroup(ID_TASKPANEL_WORKFORCEMANAGEMENT);
	pGroupWorkforceManagement->SetCaption(_T("Workforce Management"));
	pGroupWorkforceManagement->SetExpanded(FALSE);

	CXTPTaskPanelGroupItem* pWorkforceManagementItem = pGroupWorkforceManagement->AddLinkItem(ID_FILE_DRIVERTYPES, 17);
	pWorkforceManagementItem->SetCaption(_T("Driver Types"));

	pWorkforceManagementItem = pGroupWorkforceManagement->AddLinkItem(ID_FILE_DRIVERS, 18);
	pWorkforceManagementItem->SetCaption(_T("Drivers"));

	pWorkforceManagementItem = pGroupWorkforceManagement->AddLinkItem(ID_FILE_BIDDINGROSTERING, 19);
	pWorkforceManagementItem->SetCaption(_T("Bidding / Rostering"));

	pWorkforceManagementItem = pGroupWorkforceManagement->AddLinkItem(ID_FILE_DAILYOPERATIONS, 20);
	pWorkforceManagementItem->SetCaption(_T("Daily Operations"));
	
  pWorkforceManagementItem = pGroupWorkforceManagement->AddLinkItem(ID_FILE_DISCIPLINE, 21);
	pWorkforceManagementItem->SetCaption(_T("Discipline"));
//
//  Customer Service
//
	CXTPTaskPanelGroup* pGroupCustomerService = AddGroup(ID_TASKPANEL_CUSTOMERSERVICE);
	pGroupCustomerService->SetCaption(_T("Customer Service"));
	pGroupCustomerService->SetExpanded(FALSE);

	CXTPTaskPanelGroupItem* pCustomerServiceItem = pGroupCustomerService->AddLinkItem(ID_FILE_TRIPPLANNING, 22);
	pCustomerServiceItem->SetCaption(_T("Trip Planning"));

	pCustomerServiceItem = pGroupCustomerService->AddLinkItem(ID_FILE_COMPLAINTS, 23);
	pCustomerServiceItem->SetCaption(_T("Customer Comments"));
//
//  Planning
//
	CXTPTaskPanelGroup* pGroupPlanning = AddGroup(ID_TASKPANEL_PLANNING);
	pGroupPlanning->SetCaption(_T("Planning"));
	pGroupPlanning->SetExpanded(FALSE);

	CXTPTaskPanelGroupItem* pPlanningItem = pGroupPlanning->AddLinkItem(ID_FILE_TIMECHECKS, 24);
	pPlanningItem->SetCaption(_T("Time Checks"));
//
//  Reports and downloads
//
	m_pGroupReports = AddGroup(ID_TASKPANEL_REPORTS);
	m_pGroupReports->SetCaption(_T("Reports"));
	m_pGroupReports->SetExpanded(FALSE);

 	m_pGroupDownloads = AddGroup(ID_TASKPANEL_DOWNLOADSUPLOADS);
	m_pGroupDownloads->SetCaption(_T("Uploads and Downloads"));
	m_pGroupDownloads->SetExpanded(FALSE);

  OnUpdateReportSelections();

//	SetAnimation(xtpTaskPanelAnimationNo);

#else
	if (!CWnd::Create(_T("STATIC"), _T("Tables"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|SS_CENTER, CRect(0, 0, 0, 0),  pParentWnd, 0))
		return FALSE;
#endif

	GetImageManager()->SetIcons(IDB_SMALL_IMAGELIST, 0, 0, CSize(16, 16));
	GetImageManager()->SetIcon(IDI_SYSTEM_TASKS, IDI_SYSTEM_TASKS, CSize(32, 32));


	return TRUE;

}

void CTaskPanel::SetTheme(XTPPaintTheme theme)
{
#ifdef _XTP_INCLUDE_TASKPANEL
	CXTPTaskPanel::SetTheme(theme == xtpThemeOfficeXP? xtpTaskPanelThemeOfficeXPPlain:
		theme == xtpThemeOffice2003? xtpTaskPanelThemeOffice2003Plain: theme == xtpThemeOffice2000? xtpTaskPanelThemeOffice2000Plain:
		xtpTaskPanelThemeNativeWinXPPlain);
#else
	UNREFERENCED_PARAMETER(theme);
#endif

}
void CTaskPanel::OnUpdateReportSelections()
{
  OnUpdateReportSelections(m_pGroupReports, m_pGroupDownloads);
}

void CTaskPanel::OnUpdateReportSelections(CXTPTaskPanelGroup* pGroupReports, CXTPTaskPanelGroup* pGroupDownloads)
{
//
//  Delete any previous Reports and Uploads/Downloads folders
//
	pGroupReports->GetItems()->Clear();
	CXTPTaskPanelGroupItem* pReportItem;
//
//  Folder: Reports
//
//  ...and create them
//
  int nI, nJ;
  int position = 25;
  int startPosition = position;


  for(nJ = 0, nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE)
    {
      if(TMSRPT[nI].flags & TMSRPTFLAG_REPORT)
      {
      	pReportItem = pGroupReports->AddLinkItem(ID_REPORT00 + nI, 26);
    	  pReportItem->SetCaption(TMSRPT[nI].szReportName);
        position++;
      }
    }
  }
//
//  Make sure there's at least one entry in the folder
//
  if(position == startPosition)
  {
    pReportItem = pGroupReports->AddLinkItem(NO_RECORD, position);
    pReportItem->SetCaption("(None Selected)");
    position++;
  }
//
//  Folder: Downloads/Uploads
//
  pGroupDownloads->GetItems()->Clear();
	CXTPTaskPanelGroupItem* pDownloadItem;
//
//  Downloads first 
//
  startPosition = position;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE)
    {
      if(TMSRPT[nI].flags & TMSRPTFLAG_DOWNLOAD)
      {
       	pDownloadItem = pGroupDownloads->AddLinkItem(ID_REPORT00 + nI, 27);
   	    pDownloadItem->SetCaption(TMSRPT[nI].szReportName);
        position++;
      }
    }
  }
//
//  Now the uploads
//
  startPosition = position;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].flags & TMSRPTFLAG_ACTIVE)
    {
      if(TMSRPT[nI].flags & TMSRPTFLAG_UPLOAD)
      {
       	pDownloadItem = pGroupDownloads->AddLinkItem(ID_REPORT00 + nI, 28);
   	    pDownloadItem->SetCaption(TMSRPT[nI].szReportName);
        position++;
      }
    }
  }
//
//  Make sure there's at least one entry in the folder
//
  if(position == startPosition)
  {
    pDownloadItem = pGroupDownloads->AddLinkItem(NO_RECORD, position);
    pDownloadItem->SetCaption("(None Selected)");
  }
}
