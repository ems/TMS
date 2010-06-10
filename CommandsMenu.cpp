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
#include "TripGraph.h"
#include "RunSummary.h"
#include "CopyRoster.h"
#include "CopyRuncut.h"
#include "BlockPairs.h"
#include "ForcePOPI.h"

//
//  ID_COMMANDS_NODE_FINDER
//
//  Node Finder
//

void CTMSApp::OnCommandsNodeFinder()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_NODEFINDER), hWndMain, (DLGPROC)NODEFINDERMsgProc);

  return;
}


//
//  ID_COMMANDS_COPY_PATTERNS
//
//  Copy Patterns
//
void CTMSApp::OnCommandsCopyPatterns()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_PATTERNS))
      {
        return;
      }

      DISPLAYINFO DI;
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COPYPATTERNS),
            pTD->pane->m_hWnd, (DLGPROC)COPYPATTERNSMsgProc, (LPARAM)&DI))
      {
        long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

        GetTMSData(pTD, flags);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_SELECT_EXISTING_CONNECTIONS
//
//  Choose existing connections to display
//
void CTMSApp::OnCommandsSelectExistingConnections()
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
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fileInfo.routeRecordID = m_TableDisplay[displayIndex].fileInfo.routeRecordID;
      DI.fileInfo.serviceRecordID = m_TableDisplay[displayIndex].fileInfo.serviceRecordID;;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PICKCONNECTION),
            pTD->pane->m_hWnd, (DLGPROC)PICKCONNECTIONMsgProc, (LPARAM)&DI))
      {
        long flags = GETTMSDATA_FLAG_BUILD;

        GetTMSData(pTD, flags);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_COPY_TRIPS
//
//  Copy Trips
//
void CTMSApp::OnCommandsCopyTrips()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COPYTRIPS),
            pTD->pane->m_hWnd, (DLGPROC)COPYTRIPSMsgProc, (LPARAM)&DI))
      {
        long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

        m_bEstablishRUNTIMES = TRUE;
        GetTMSData(pTD, flags);
      }
    }
  }

  return;
}
//
//  ID_COMMANDS_SORT_TRIPS
//
//  Sort Trips
//
void CTMSApp::OnCommandsSortTrips()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fieldInfo = pTD->fieldInfo;
      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SORTTRIP),
            pTD->pane->m_hWnd, (DLGPROC)SORTTRIPMsgProc, (LPARAM)&DI))
      {
        pTD->baggage = DI.baggage;
        pTD->otherData = DI.otherData;

        CString s;

        char nodeName[NODES_ABBRNAME_LENGTH + 1];
        long nodeTime;
        int  rcode2;
        int  nI;
        int  nJ;
        int  atNodeColumn = pTD->otherData + 1; // Add one 'cause the grid starts at 1 not 0
        int  selectedSortPoint = pTD->baggage;
//
//  Loop through the grid
//
        HCURSOR hSaveCursor = SetCursor(hCursorWait);
        int     lastRow = pTD->F1.GetLastRow();

        for(nI = 1; nI <= lastRow; nI++)
        {
          TRIPSKey0.recordID = (long)pTD->F1.GetNumberRC(nI, 1);
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Switch on the radio button selection
//
          switch(selectedSortPoint)
          {
//
//  SORTTRIP_TRIPMLP - There are about 4,000,000,000 different
//                     ways that this could be done better
//
            case SORTTRIP_TRIPMLP:
              NODESKey0.recordID = NO_RECORD;
              PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
              PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
              PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
              PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              PATTERNSKey2.nodeSequence = NO_RECORD;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              while(rcode2 == 0 &&
                    PATTERNS.ROUTESrecordID == pTD->fileInfo.routeRecordID &&
                    PATTERNS.SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
                    PATTERNS.directionIndex == pTD->fileInfo.directionIndex &&
                    PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
              {
                if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                {
                  NODESKey0.recordID = PATTERNS.NODESrecordID;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              }
              if(NODESKey0.recordID == NO_RECORD)  // for safety's sake...
                atNodeColumn = pTD->fieldInfo.firstNodeColumn;
              else
              {
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                trim(nodeName, NODES_ABBRNAME_LENGTH);

                int nJmax = pTD->fieldInfo.firstNodeColumn + pTD->fieldInfo.cNumPatternNodes - 1;

                for(nJ = pTD->fieldInfo.firstNodeColumn; nJ < nJmax; nJ++)
                {
                  s = pTD->F1.GetColText(nJ);
                  if(strcmp(nodeName, s) == 0)
                  {
                    atNodeColumn = nJ;
                    break;
                  }
                }
              }
              strcpy(tempString, pTD->F1.GetTextRC(nI, atNodeColumn));
              if((nodeTime = cTime(tempString)) != NO_TIME)
                TRIPS.tripSequence = nodeTime;
              break;
//
//  SORTTRIP_FIRSTTIME
//
            case SORTTRIP_FIRSTTIME:
              for(nJ = pTD->fieldInfo.firstNodeColumn;
                  nJ < pTD->fieldInfo.firstNodeColumn + pTD->fieldInfo.cNumPatternNodes; nJ++)
              {
                strcpy(tempString, pTD->F1.GetTextRC(nI, nJ));
                if((nodeTime = cTime(tempString)) != NO_TIME)
                {
                  TRIPS.tripSequence = nodeTime;
                  break;
                }
              }
              break;
//
//  SORTTRIP_BASEMLP
//  SORTTRIP_ATNODE
//
            case SORTTRIP_BASEMLP:
            case SORTTRIP_ATNODE:
              strcpy(tempString, pTD->F1.GetTextRC(nI, atNodeColumn));
//
//  No problem if not NO_TIME
//
              if((nodeTime = cTime(tempString)) != NO_TIME)
              {
                TRIPS.tripSequence = nodeTime;
              }
//
//  It was NO_TIME.  We have to get a "pretend" time
//
              else
              {
                GenerateTripDef GTResults;
                TRIPSDef        localTRIPS;
                long thisPatternMLPSeq;
                long basePatternMLPSeq;
                BOOL bDoTheSwitch;
//
//  Set up a local version of the trips record
//
                localTRIPS = TRIPS;
//
//  Find out if the BASE pattern MLP is different from the "atNodeColumn"
//
                PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                PATTERNSKey2.nodeSequence = NO_RECORD;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                basePatternMLPSeq = NO_RECORD;
                while(rcode2 == 0 &&
                      PATTERNS.ROUTESrecordID == pTD->fileInfo.routeRecordID &&
                      PATTERNS.SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
                      PATTERNS.directionIndex == pTD->fileInfo.directionIndex &&
                      PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
                {
                  if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                  {
                    basePatternMLPSeq = PATTERNS.nodeSequence;
                    break;
                  }
                  rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                }
//
//  Get the MLP of the pattern of this trip
//
                if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
                {
                  thisPatternMLPSeq = basePatternMLPSeq;
                }
                else
                {
                  PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                  PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                  PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                  PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  PATTERNSKey2.nodeSequence = NO_RECORD;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  while(rcode2 == 0 &&
                        PATTERNS.ROUTESrecordID == pTD->fileInfo.routeRecordID &&
                        PATTERNS.SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
                        PATTERNS.directionIndex == pTD->fileInfo.directionIndex &&
                        PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
                  {
                    if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                    {
                      thisPatternMLPSeq = PATTERNS.nodeSequence;
                      break;
                    }
                    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  }
                }
//
//  If the base pattern MLP is different from the trip we're looking
//  at, temporarily modify the BASE pattern to set the MLP to what we have
//
                bDoTheSwitch = (basePatternMLPSeq != thisPatternMLPSeq &&
                      basePatternMLPSeq != NO_RECORD);
                if(bDoTheSwitch)
                {
                  PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                  PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                  PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                  PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                  PATTERNSKey2.nodeSequence = basePatternMLPSeq;
                  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNS.flags &= ~PATTERNS_FLAG_MLP;
                  rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                  PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                  PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                  PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                  PATTERNSKey2.nodeSequence = thisPatternMLPSeq;
                  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNS.flags |= PATTERNS_FLAG_MLP;
                  rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                }
//
//  Generate a fake trip
//
                localTRIPS.PATTERNNAMESrecordID = basePatternRecordID;
                GenerateTrip(localTRIPS.ROUTESrecordID, localTRIPS.SERVICESrecordID,
                      localTRIPS.directionIndex, localTRIPS.PATTERNNAMESrecordID,
                      localTRIPS.timeAtMLP, 0, &GTResults);
                TRIPS.tripSequence = GTResults.tripTimes[atNodeColumn - pTD->fieldInfo.firstNodeColumn - 1];
//
//  Restore the base pattern
//
                if(bDoTheSwitch)
                {
                  PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                  PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                  PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                  PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                  PATTERNSKey2.nodeSequence = basePatternMLPSeq;
                  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNS.flags |= PATTERNS_FLAG_MLP;
                  rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
                  PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
                  PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
                  PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
                  PATTERNSKey2.nodeSequence = thisPatternMLPSeq;
                  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                  PATTERNS.flags &= ~PATTERNS_FLAG_MLP;
                  rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
                }
              }
              break;
          }
          btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        }
        SetCursor(hSaveCursor);
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_GRAPHICAL_TRIPS
//
//  Display trips graphically
//
void CTMSApp::OnCommandsGraphicalTrips()
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
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fileInfo = pTD->fileInfo;

      CTripGraph dlg(pChild, &DI);
  
      dlg.DoModal();
    }
  }
  return;
}

//
//  ID_COMMANDS_ASSIGN_TRIP_NUMBERS
//
//  Assign Trip Numbers
//
void CTMSApp::OnCommandsAssignTripNumbers()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;
      int ThisType = pTD->fileInfo.fileNumber;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ASSIGNTRIP),
            pTD->pane->m_hWnd, (DLGPROC)ASSIGNTRIPMsgProc, (LPARAM)&DI))
      GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
    }
  }

  return;
}

//
//  ID_COMMANDS_SET_LAYOVER
//
//  Set the default layover
//

void CTMSApp::OnCommandsSetLayover()
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
      int ThisType = pTD->fileInfo.fileNumber;

      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LAYOVER),
            pTD->pane->m_hWnd, (DLGPROC)LAYOVERMsgProc, (LPARAM)ThisType);
      return;
    }
  }
}

//
//  ID_COMMANDS_BLOCK_TRIPS
//
//  Block Trips
//
void CTMSApp::OnCommandsBlockTrips()
{
  if(StandardLayover.minimum.type == NO_RECORD || StandardLayover.maximum.type == NO_RECORD)
  {
    TMSError(NULL, MB_ICONINFORMATION, ERROR_030, (HANDLE)NULL);
    ::SendMessage(hWndMain, WM_COMMAND, ID_COMMANDS_SET_LAYOVER, 0L);
  }
  if(StandardLayover.minimum.type == NO_RECORD || StandardLayover.maximum.type == NO_RECORD)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_031, (HANDLE)NULL);
    return;
  }
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
      int ThisType = pTD->fileInfo.fileNumber;

//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.otherData = pTD->otherData;
      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDBLOCK),
              pTD->pane->m_hWnd, (DLGPROC)ADDBLOCKMsgProc, (LPARAM)&DI))
      {
        pTD->otherData = DI.otherData;
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}
//
//  ID_COMMANDS_HOOK_BLOCKS
//
void CTMSApp::OnCommandsHookBlocks()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HOOKBLOCKS),
            hWndMain, (DLGPROC)HOOKBLOCKSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

void CTMSApp::OnCommandsOptimalhookBlocks() 
{
  if(StandardLayover.minimum.type == NO_RECORD || StandardLayover.maximum.type == NO_RECORD)
  {
    TMSError(NULL, MB_ICONINFORMATION, ERROR_030, (HANDLE)NULL);
    ::SendMessage(hWndMain, WM_COMMAND, ID_COMMANDS_SET_LAYOVER, 0L);
  }
  if(StandardLayover.minimum.type == NO_RECORD || StandardLayover.maximum.type == NO_RECORD)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_031, (HANDLE)NULL);
    return;
  }
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
      int ThisType = pTD->fileInfo.fileNumber;

//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.otherData = pTD->otherData;
      DI.fileInfo = pTD->fileInfo;

      CBlockPairs dlg(pChild, &DI);

      if(dlg.DoModal() == IDOK)
      {
        pTD->otherData = DI.otherData;
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}
//
//  ID_COMMANDS_UNHOOK_TRIPS
//

void CTMSApp::OnCommandsUnhookTrips()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UNHOOKTRIPS),
            hWndMain, (DLGPROC)UNHOOKTRIPSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_POPI
//
void CTMSApp::OnCommandsPopi()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PULLOUTPULLIN),
            hWndMain, (DLGPROC)PULLOUTPULLINMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_ASSIGN_GARAGES
//
void CTMSApp::OnCommandsAssignGarages()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ASSREMGARAGE),
            hWndMain, (DLGPROC)ASSREMGARAGEMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_RENUMBER_BLOCKS
//
void CTMSApp::OnCommandsRenumberBlocks()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      specificBlock = NO_RECORD;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENUMBLOCKS),
            hWndMain, (DLGPROC)RENUMBLOCKSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_ASSIGN_VEHICLE_TYPE
//
void CTMSApp::OnCommandsAssignVehicleType()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_TRIPS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ASSIGNVEHICLETYPE),
            hWndMain, (DLGPROC)ASSIGNVEHICLETYPEMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_BLOCK_SUMMARY
//
void CTMSApp::OnCommandsBlockSummary()
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
      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BLOCKSUMMARY),
            hWndMain, (DLGPROC)BLOCKSUMMARYMsgProc, (LPARAM)&DI);
    }
  }

  return;
}

//
//  ID_COMMANDS_CUT_RUNS
//
void CTMSApp::OnCommandsCutRuns()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_RUNS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDRUN),
            pTD->pane->m_hWnd, (DLGPROC)ADDRUNMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_FORCE_POPI
//

void CTMSApp::OnCommandsForcePopi() 
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_RUNS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      CForcePOPI dlg(pChild, &DI);
  
      if(dlg.DoModal())
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_DELETE_RUNS
//
void CTMSApp::OnCommandsDeleteRuns()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_RUNS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;

      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DELETERUNS),
            pTD->pane->m_hWnd, (DLGPROC)DELETERUNSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_RENUMBER_RUNS
//
void CTMSApp::OnCommandsRenumberRuns()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_RUNS))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      specificRun = NO_RECORD;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENUMRUNS),
              pTD->pane->m_hWnd, (DLGPROC)RENUMRUNSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_COPYRUNCUT
//
void CTMSApp::OnCommandsCopyruncut() 
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
//
//  Verify his rights
//
    if(!VerifyUserAccessRights(TMS_RUNS))
    {
      return;
    }

    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD)
    {
      CCopyRuncut dlg;

      if(dlg.DoModal() == IDOK)
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_RUN_SUMMARY
//
void CTMSApp::OnCommandsRunSummary()
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
      CRunSummary dlg(pChild, &DI);

      dlg.DoModal();
    }
  }

  return;
}

//
//  ID_COMMANDS_ANALYZE_RUN
//
void CTMSApp::OnCommandsAnalyzeRun()
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
      ANALYZERUNPassedDataDef ANALYZERUNPassedData;
      long row[2];
      long col[2];
      int  nI;
      int  nJ;
      int  numRanges;
      int  rcode2;

      numRanges = pTD->F1.GetSelectionCount();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          RUNSKey0.recordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 == 0 && RUNS.pieceNumber == 1)
          {
            ANALYZERUNPassedData.flags = ANALYZERUN_FLAGS_CHILDWND;
            ANALYZERUNPassedData.longValue = RUNS.recordID;
            if(!DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ANALYZERUN),
                  pTD->pane->m_hWnd, (DLGPROC)ANALYZERUNMsgProc, (LPARAM)&ANALYZERUNPassedData))
              break;
          }
        }
      }
    }
  }

  return;
}
//
//  ID_COMMANDS_ROSTER_RUNS
//
void CTMSApp::OnCommandsRosterRuns()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_ROSTER))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDROSTER),
            pTD->pane->m_hWnd, (DLGPROC)ADDROSTERMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_RENUMBER_ROSTER
//
void CTMSApp::OnCommandsRenumberRoster()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_ROSTER))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENUMROSTER),
            pTD->pane->m_hWnd, (DLGPROC)RENUMROSTERMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}

//
//  ID_COMMANDS_RENUMBER_RUNS_FROM_ROSTER
//
void CTMSApp::OnCommandsRenumberRunsFromRoster()
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
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(TMS_ROSTER))
      {
        return;
      }

      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENRUNS),
            pTD->pane->m_hWnd, (DLGPROC)RENRUNSMsgProc, (LPARAM)&DI))
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }
}

//
//  ID_COMMANDS_COPY_ROSTER
//
void CTMSApp::OnCommandsCopyroster() 
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
//
//  Verify his rights
//
    if(!VerifyUserAccessRights(TMS_ROSTER))
    {
      return;
    }

    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
      CCopyRoster dlg;

      if(dlg.DoModal() == IDOK)
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }

  return;
}


//
//  ID_COMMANDS_ESTABLISHROSTERTEMPLATE
//  
void CTMSApp::OnCommandsEstablishrostertemplate() 
{
  CString s;

  s.LoadString(TEXT_265);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(NULL, s, TMS, MB_OK | MB_YESNO | MB_DEFBUTTON2) != IDYES)
  {
    return;
  }

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
//
//  Verify his rights
//
    if(!VerifyUserAccessRights(TMS_DAILYOPS))
    {
      return;
    }

    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
//
//  Establish "now"
//
      CTime time = CTime::GetCurrentTime();
      long  datenow = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
      long  timenow = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
      BOOL  bFound;
      int   rcode2;
      int   nI;
//
//  Reset the "next" daytime and nighttime extraboard rotation number
//
//  Daytime
//
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
      DAILYOPS.entryDateAndTime = time.GetTime();
      DAILYOPS.pertainsToDate = datenow;
      DAILYOPS.pertainsToTime = timenow;
      DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_ROSTERSETNEXTDAYTIME;
      DAILYOPS.userID = m_UserID;
      DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
      memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
      DAILYOPS.DOPS.RosterTemplate.nextDaytime = 1;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
//
//  Nighttime
//
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
      DAILYOPS.entryDateAndTime = time.GetTime();
      DAILYOPS.pertainsToDate = datenow;
      DAILYOPS.pertainsToTime = timenow;
      DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_ROSTERSETNEXTNIGHTTIME;
      DAILYOPS.userID = m_UserID;
      DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
      memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
      DAILYOPS.DOPS.RosterTemplate.nextNighttime = 1;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
//
//  Cycle through the active roster
//
//  Database verify: 11-Jan-07
//
      ROSTERKey1.DIVISIONSrecordID = pTD->fileInfo.divisionRecordID;
      ROSTERKey1.rosterNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      while(rcode2 == 0 &&
            ROSTER.DIVISIONSrecordID == pTD->fileInfo.divisionRecordID)
      {
        rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
        DAILYOPS.entryDateAndTime = time.GetTime();
        DAILYOPS.pertainsToDate = datenow;
        DAILYOPS.pertainsToTime = timenow;
        DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
        DAILYOPS.recordFlags = DAILYOPS_FLAG_ROSTERESTABLISH;
        DAILYOPS.userID = m_UserID;
        DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
        DAILYOPS.DAILYOPSrecordID = NO_RECORD;
        memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
        DAILYOPS.DOPS.RosterTemplate.ROSTERrecordID = ROSTER.recordID;
        DAILYOPS.DOPS.RosterTemplate.ROSTERDIVISIONSrecordID = pTD->fileInfo.divisionRecordID;
        DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID = NO_RECORD;
        for(bFound = FALSE, nI = 0; nI < 7; nI++)
        {
          if(ROSTER.WEEK[pTD->fileInfo.rosterWeek].RUNSrecordIDs[nI] != NO_RECORD)
          {
            RUNSKey0.recordID = ROSTER.WEEK[pTD->fileInfo.rosterWeek].RUNSrecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            if(rcode2 == 0)
            {
              DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID = RUNS.DIVISIONSrecordID;
              bFound = TRUE;
              break;
            }
          }
        }
        if(bFound)
        {
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      }
    }
  }
}
