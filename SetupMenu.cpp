//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include "MainFrm.h"
#include "ResizingDialog.h"
#include "DailyOpsHeader.h"
#include "RosterParms.h"
#include "RosterWeek.h"
#include "PremiumDefinition.h"

//
//  ID_SETUP_RUNS_PARAMETERS
//
void CTMSApp::OnSetupRunsParameters()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_PARAMETERS), hWndMain, (DLGPROC)PARAMETERSMsgProc);

  return;
}

//
//  ID_SETUP_RUNS_RELIEF_POINTS
//
void CTMSApp::OnSetupRunsReliefpoints()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_RELIEFPOINTS), hWndMain, (DLGPROC)RELIEFPOINTSMsgProc);

  return;
}

//
//  ID_SETUP_RUNS_OVERTIME
//
void CTMSApp::OnSetupRunsOvertime()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_OVERTIME), hWndMain, (DLGPROC)OVERTIMEMsgProc);

  return;
}

//
//  ID_SETUP_RUNS_RUNTYPES
//
void CTMSApp::OnSetupRunsRuntypes()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_RUNTYPES), hWndMain, (DLGPROC)RUNTYPESMsgProc);

  return;
}

//
//  ID_SETUP_RUNS_PREMIUMS
//
void CTMSApp::OnSetupRunsPremiums()
{
  CMDIFrameWnd* pMDIFrameWnd = (CMDIFrameWnd *)m_pMainWnd->GetTopLevelFrame();

  if(pMDIFrameWnd == NULL)
  {
    return;
  }

  CMDIChildWnd* pChild = pMDIFrameWnd->MDIGetActive();

  if(pChild == NULL)
  {
    return;
  }

  HWND hActiveWindow = pChild->m_hWnd;
  if(hActiveWindow)
  {
    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;

      CPremiumDefinition dlg(pChild, &DI);

      if(dlg.DoModal() == IDOK)
//      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WORKRULES),
//              pTD->pane->m_hWnd, (DLGPROC)WORKRULESMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_SETUP_ROSTER_PARAMETERS
//
void CTMSApp::OnSetupRosterParameters()
{
  CRosterParms dlg;

  dlg.DoModal();
}

//
//  ID_SETUP_ROSTER_SERVICE_DAYS
//
void CTMSApp::OnSetupRosterServiceDays()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_SERVICEDAYS), hWndMain, (DLGPROC)SERVICEDAYSMsgProc);
  return;
}

//
//  ID_SETUP_DRIVERSTOCONSIDER
//
void CTMSApp::OnSetupRosterDriverstoconsider() 
{
  CDriversToConsider dlg;

  dlg.DoModal();
}
