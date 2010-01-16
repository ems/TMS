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

//
//  ID_VIEW_TRIP_RUNNING_TIMES
//
//  Change the trips display to show running times
//
void CTMSApp::OnViewTripRunningTimes()
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
      m_bShowNodeTimes = !m_bShowNodeTimes;
      EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedConnections, (LPARAM)NULL);
    }
  }

  return;
}

//
//  ID_VIEW_COMPRESSED_BLOCKS
//
//  Change the blocks display to show single line entries
//
void CTMSApp::OnViewCompressedBlocks()
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
      long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1;
      m_bCompressedBlocks = !m_bCompressedBlocks;
      GetTMSData(pTD, flags);
    }
  }

  return;
}
