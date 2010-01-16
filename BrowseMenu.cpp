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
//  BROWSE Menu
//
//  ID_BROWSE_COLUMN_CHARACTERISTICS
//


void CTMSApp::OnBrowseColumnCharacteristics()
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
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETCOL),
            pTD->pane->m_hWnd, (DLGPROC)SETCOLMsgProc, (LPARAM)pTD->fileInfo.fileNumber))
      {
        long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

        GetTMSData(pTD, flags);
      }
    }
  }

  return;
}
//
//  ID_BROWSE_COLUMN_SORT
//

void CTMSApp::OnBrowseColumnSort()
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

      DI.fieldInfo = pTD->fieldInfo;
      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SORTBYCOLUMN),
            m_TableDisplay[displayIndex].pane->m_hWnd, (DLGPROC)SORTBYCOLUMNMsgProc, (LPARAM)&DI))
      {
        pTD->fieldInfo = DI.fieldInfo;
        pTD->fileInfo = DI.fileInfo;
        sprintf(szarString, "SortColumn%d", pTD->fileInfo.fileNumber);
        sprintf(tempString, "%ld", pTD->fieldInfo.sortFieldId);
        WritePrivateProfileString(userName, szarString, tempString, szDatabaseFileName);
        if(pTD->fieldInfo.sortFieldId == 0) // TMS Default
        {
          long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

          GetTMSData(pTD, flags);
        }
        else
        {
          int nI;
          for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
          {
            if(abs(pTD->fieldInfo.sortFieldId) == pTD->fieldInfo.fieldId[nI])
            {
              VARIANT v;
              long    lastRow = pTD->F1.GetLastRow();
          
              v.vt = VT_I2;
              v.iVal = nI + 1;
              if(pTD->fieldInfo.sortFieldId < 0)
                v.iVal = -v.iVal;
              pTD->F1.Sort(-1, -1, -1, -1, TRUE, v);
              break;
            }
          }
        }
      }
    }
  }

  return;
}
