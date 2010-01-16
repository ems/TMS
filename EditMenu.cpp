//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "CCFrontEnd.h"
}

#include "TMS.h"
#include "MainFrm.h"
#include "AddDialogs.h"
#include "StopConnection.h"
#include "FindBlock.h"
#include "AssignDriver.h"
#include "DisplayRoster.h"
#include "PlaceHolder.h"
//
//  EDIT Menu
//
//  ID_EDIT_COPY
//
void CTMSApp::OnEditCopy()
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
      pTD->F1.EditCopy();
    }
  }
}

//
//  ID_EDIT_INSERT
//
void CTMSApp::OnEditInsert()
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
      int nRc;
      int ThisType = pTD->fileInfo.fileNumber;

//
//  Verify his rights
//
      if(!VerifyUserAccessRights(ThisType))
      {
        return;
      }

      updateRecordID = NO_RECORD;
      inboundUpdate = FALSE;
      nRc = 0;
//
//  Directions
//
      if(ThisType == TMS_DIRECTIONS)
      {
        CAddDirection dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Routes
//
      else if(ThisType == TMS_ROUTES)
      {
        CAddRoute dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
        if(nRc)
        {
          SetUpRouteList(m_hwndDlgBar, RSDTOOLBAR_ROUTE, m_RouteRecordID);
        }
      }
//
//  Services
//
      else if(ThisType == TMS_SERVICES)
      {
        nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDSERVICE), pTD->pane->m_hWnd, (DLGPROC)ADDSERVICEMsgProc);
        if(nRc)
        {
          SetUpServiceList(m_hwndDlgBar, RSDTOOLBAR_SERVICE, m_ServiceRecordID);
        }
      }
//
//  Jurisdictions
//
      else if(ThisType == TMS_JURISDICTIONS)
      {
        nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDJURISDICTION), pTD->pane->m_hWnd, (DLGPROC)ADDJURISDICTIONMsgProc);
      }
//
//  Divisions
//
      else if(ThisType == TMS_DIVISIONS)
      {
        CAddDivision dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
        if(nRc)
        {
          SetUpDivisionList(m_hwndDlgBar, RSDTOOLBAR_DIVISION, m_DivisionRecordID);
        }
      }
//
//  Nodes
//
      else if(ThisType == TMS_NODES)
      {
        CAddNode dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Patterns
//
      else if(ThisType == TMS_PATTERNS)
      {
        DI.fileInfo = pTD->fileInfo;
        nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDPATTERN),
              pTD->pane->m_hWnd, (DLGPROC)ADDPATTERNMsgProc, (LPARAM)&DI);
      }
//
//  Connections
//
      else if(ThisType == TMS_CONNECTIONS)
      {
        long row[2];
        long col[2];

        pTD->F1.GetSelection(0, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] == F1_MAXCOL)
          pTD->baggage = (long)pTD->F1.GetNumberRC(row[0], 1);
        else
          pTD->baggage = NO_RECORD;

        DISPLAYINFO DI;

        DI.fileInfo = pTD->fileInfo;
        DI.baggage = pTD->baggage;
        CAddConnection dlg(pTD->pane, &updateRecordID, &DI);
        nRc = dlg.DoModal();
        if(nRc == IDOK)
        {
          pTD->fileInfo = DI.fileInfo;
          pTD->baggage = DI.baggage;
        }
      }
//
//  Bustypes
//
      else if(ThisType == TMS_BUSTYPES)
      {
        CAddBustype dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Buses
//
      else if(ThisType == TMS_BUSES)
      {
        CAddBus dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Sign Codes
//
      else if(ThisType == TMS_SIGNCODES)
      {
        nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDSIGNCODE), pTD->pane->m_hWnd, (DLGPROC)ADDSIGNCODEMsgProc);
      }
//
//  Comments
//
      else if(ThisType == TMS_COMMENTS)
      {
        CAddComment dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Trips
//
      else if(ThisType == TMS_TRIPS)
      {
        DI.fileInfo = pTD->fileInfo;
        nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDTRIP),
              pTD->pane->m_hWnd, (DLGPROC)ADDTRIPMsgProc, (LPARAM)&DI);
      }
//
//  Driver types
//
      else if(ThisType == TMS_DRIVERTYPES)
      {
        CAddDriverType dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Drivers
//
      else if(ThisType == TMS_DRIVERS)
      {
        CAddDriver dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Timechecks
//
      else if(ThisType == TMS_TIMECHECKS)
      {
        DI.fileInfo = pTD->fileInfo;
        CAddTimecheck dlg(NULL, &updateRecordID, &DI);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  Crew only runs
//
      else if(ThisType == TMS_CREWONLY)
      {
        DI.fileInfo = pTD->fileInfo;
        CAddCrewOnly dlg(NULL, &updateRecordID, &DI);
        nRc = (dlg.DoModal() == IDOK);
      }
//
//  IDOK out of the dialog
//
      if(nRc == IDOK)
      {
//
//  When returning from the "ADD" dialog, updateRecordID is assigned in the dialog
//  routine and is the recordID of the new record.  Now we have to figure out where
//  in the display grid the new record goes.
//
        AllFilesDef ALLFILES;
        union
        {
          AllKey0Def  ALLKey0;
          AllKey1Def  ALLKey1;
          AllKey2Def  ALLKey2;
        } ALLKeys;
        long   recordID;
        long   prevRecordID = NO_RECORD;
        long   nextRecordID = NO_RECORD;
        int    fileNumber= pTD->fileInfo.fileNumber;
        int    keyNumber = pTD->fileInfo.keyNumber;
        int    nI;
        int    rcode2;
        long   insertAt;
        long   numRows;
//
//  Did we change connections?
//
        if(ThisType == TMS_CONNECTIONS)
        {
          m_bEstablishRUNTIMES = TRUE;
        }
//
//  On the same key as the table display, find the previous and next record
//  ID of the new record.
//
        ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
        btrieve(B_GETEQUAL, fileNumber, &ALLFILES, &ALLKeys, 0);
        if(fileNumber == TMS_PATTERNS || fileNumber == TMS_TRIPS || fileNumber == TMS_TIMECHECKS)
        {
          long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

          GetTMSData(pTD, flags);
        }
        else
        {
          if(keyNumber != 0)
          {
            btrieve(B_GETPOSITION, fileNumber, &ALLFILES, &ALLKeys, keyNumber);
            btrieve(B_GETDIRECT, fileNumber, &ALLFILES, &ALLKeys, keyNumber);
          }
          rcode2 = btrieve(B_GETPREVIOUS, fileNumber, &ALLFILES, &ALLKeys, keyNumber);
//
//  If we got a previous record, make sure it fits the criteria of the table
//  (pTD->fileInfo.flags & FILE_CHECKRSDD) lets us know if we have to check by file number
//
          if(rcode2 == 0)
          {
            prevRecordID = ALLFILES.DATABASE.recordID;
            if(pTD->fileInfo.flags & FILE_CHECKRSDD)
            {
              if(fileNumber == TMS_PATTERNS)
              {
                if(ALLFILES.PATTERNS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
                      ALLFILES.PATTERNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
                      ALLFILES.PATTERNS.directionIndex != pTD->fileInfo.directionIndex)
                  prevRecordID = NO_RECORD;
              }
              else if(fileNumber == TMS_TRIPS)
              {
                if(ALLFILES.TRIPS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
                      ALLFILES.TRIPS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
                      ALLFILES.TRIPS.directionIndex != pTD->fileInfo.directionIndex)
                  prevRecordID = NO_RECORD;
              }
            }
            btrieve(B_GETNEXT, fileNumber, &ALLFILES, &ALLKeys, keyNumber);
          }
          rcode2 = btrieve(B_GETNEXT, fileNumber, &ALLFILES, &ALLKeys, keyNumber);
//
//  If we got a next record, make sure it fits the criteria of the table
//  (pTD->fileInfo.flags & FILE_CHECKRSDD) lets us know if we have to check by file number
//
          if(rcode2 == 0)
          {
            nextRecordID = ALLFILES.DATABASE.recordID;
            if(pTD->fileInfo.flags & FILE_CHECKRSDD)
            {
              if(fileNumber == TMS_PATTERNS)
              {
                if(ALLFILES.PATTERNS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
                      ALLFILES.PATTERNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
                      ALLFILES.PATTERNS.directionIndex != pTD->fileInfo.directionIndex)
                  nextRecordID = NO_RECORD;
              }
              else if(fileNumber == TMS_TRIPS)
              {
                if(ALLFILES.TRIPS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
                      ALLFILES.TRIPS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
                      ALLFILES.TRIPS.directionIndex != pTD->fileInfo.directionIndex)
                  nextRecordID = NO_RECORD;
              }
            }
          }
//
//  How many rows are there now?
//
          numRows = pTD->F1.GetLastRow();
//
//  If previous is NO_RECORD, it goes in row 1
//
          if(prevRecordID == NO_RECORD)
          {
            if(numRows != 0)
              pTD->F1.InsertRange(1, -1, 1, -1, F1ShiftVertical);
            insertAt = 1;
          }
//
//  If next is NO_RECORD, it goes after the last row and we don't have to do anything
//
          else if(nextRecordID == NO_RECORD)
          {
            insertAt = numRows + 1;
          }
//
//  Both prev and next exist, so find out where to put it
//
          else
          {
            for(insertAt = NO_RECORD, nI = 1; nI <= numRows; nI++)
            {
              recordID = (long)pTD->F1.GetNumberRC(nI, 1);
              if(recordID == prevRecordID)
              {
                insertAt = nI + 1;
                break;
              }
            }
//
//  If we didn't find its predecessor, look for nextRecordID
//
            if(insertAt == NO_RECORD)
            {
              for(nI = 1; nI <= numRows; nI++)
              {
                recordID = (long)pTD->F1.GetNumberRC(nI, 1);
                if(recordID == nextRecordID)
                {
                  insertAt = nI;
                  break;
                }
              }
            }
//
//  If insertAt is still NO_RECORD, make it the first row
//  This should never happen, but who the hell knows.
//
            if(insertAt == NO_RECORD)
              insertAt = 1;
//
//  Shift the grid down
//
            pTD->F1.InsertRange(insertAt, -1, insertAt, -1, F1ShiftVertical);
          }
//
//  Adjust MaxRow
//
          pTD->F1.SetMaxRow((long)numRows + 1);
//
//  And display the new record
//
          int  ctlFlags = 0;
          char outString[256];

          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          if(fileNumber != TMS_COMMENTS)
            btrieve(B_GETEQUAL, fileNumber, &ALLFILES, &ALLKeys, 0);
          else
          {
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            if(rcode2 == 0)
            {
              memcpy(&ALLFILES, pCommentText, COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH);
              memset(&ALLFILES.hackedCommentText[COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH], 0x00, 1);
            }
          }
          for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
          {
            if(pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNCHECKS + DATATYPE_BIT)
            {
              DisplayPatternRow(&pTD->F1, pTD, &ALLFILES.PATTERNS, insertAt - 1, &nI);
              ALLKeys.ALLKey0.PATTERNSKey0.recordID = ALLFILES.PATTERNS.recordID;
              btrieve(B_GETEQUAL, TMS_PATTERNS, &ALLFILES, &ALLKeys, 0);
              btrieve(B_GETPOSITION, TMS_PATTERNS, &ALLFILES, &ALLKeys, 0);
              btrieve(B_GETDIRECT, TMS_PATTERNS, &ALLFILES, &ALLKeys, 2);
            }
            else if(pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNTIMES + DATATYPE_BIT)
            {
              DisplayTripRow(&pTD->F1, pTD, &ALLFILES.TRIPS, insertAt - 1, &nI);
            }
            else if(pTD->fieldInfo.dataType[nI] == TMSDATA_FLAGS + DATATYPE_BIT)
            {
              DisplayFlagRow(&pTD->F1, pTD, &ALLFILES, insertAt - 1, &nI);
            }
            else
            {
              RenderField(&pTD->F1, pTD, nI, &ALLFILES.DATABASE, insertAt - 1, ctlFlags, outString);
              pTD->F1.SetTextRC(insertAt, nI + 1, outString);
            }
          }
        }
//
//  Enumerate any open children to see if we have to update the trips
//
        if(ThisType == TMS_CONNECTIONS)
        {
          CONNECTIONSKey0.recordID = updateRecordID;
          btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedConnections, (LPARAM)&CONNECTIONS);
        }
      }
    }
  }
}


//
//  ID_EDIT_INSERT_STOP_CONNECTION
//
void CTMSApp::OnEditInsertStopConnection()
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
      if(!VerifyUserAccessRights(TMS_CONNECTIONS))
      {
        return;
      }

      memset(&CONNECTIONS, 0x00, sizeof(CONNECTIONSDef));
      CONNECTIONS.recordID = NO_RECORD;
      CONNECTIONS.COMMENTSrecordID = NO_RECORD;
      CStopConnection dlg(NULL, &CONNECTIONS);

      if(dlg.DoModal() != IDOK)
      {
        return;
      }
//
//  Figure out where in the display grid the new record goes.
//
      long recordID = CONNECTIONS.recordID;
      long prevRecordID = NO_RECORD;
      long nextRecordID = NO_RECORD;
      long insertAt;
      long numRows;
      int  nI;
      int  rcode2;
//
//  On the same key as the table display, find the previous and next record
//  ID of the new record.
//
      CONNECTIONSKey0.recordID = recordID;
      btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      rcode2 = btrieve(B_GETPREVIOUS, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      if(rcode2 == 0)
      {
        prevRecordID = CONNECTIONS.recordID;
        btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      if(rcode2 == 0)
        nextRecordID = CONNECTIONS.recordID;
//
//  How many rows are there now?
//
      numRows = pTD->F1.GetLastRow();
//
//  If previous is NO_RECORD, it goes in row 1
//
      if(prevRecordID == NO_RECORD)
      {
        if(numRows != 0)
          pTD->F1.InsertRange(1, -1, 1, -1, F1ShiftVertical);
        insertAt = 1;
      }
//
//  If next is NO_RECORD, it goes after the last row and we don't have to do anything
//
      else if(nextRecordID == NO_RECORD)
      {
        insertAt = numRows + 1;
      }
//
//  Both prev and next exist, so find out where to put it
//
      else
      {
        for(insertAt = NO_RECORD, nI = 1; nI <= numRows; nI++)
        {
          recordID = (long)pTD->F1.GetNumberRC(nI, 1);
          if(recordID == prevRecordID)
          {
            insertAt = nI + 1;
            break;
          }
        }
//
//  If we didn't find its predecessor, look for nextRecordID
//
        if(insertAt == NO_RECORD)
        {
          for(nI = 1; nI <= numRows; nI++)
          {
            recordID = (long)pTD->F1.GetNumberRC(nI, 1);
            if(recordID == nextRecordID)
            {
              insertAt = nI;
              break;
            }
          }
        }
      }
//
//  If insertAt is still NO_RECORD, make it the first row
//  This should never happen, but who the hell knows.
//
      if(insertAt == NO_RECORD)
        insertAt = 1;
//
//  Shift the grid down
//
      pTD->F1.InsertRange(insertAt, -1, insertAt, -1, F1ShiftVertical);
//
//  Adjust MaxRow
//
      pTD->F1.SetMaxRow((long)numRows + 1);
//
//  And display the new record
//
      int  ctlFlags = 0;
      char outString[256];
      DATABASEDef dat;

      CONNECTIONSKey0.recordID = recordID;
      btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      memcpy(&dat, &CONNECTIONS, sizeof(CONNECTIONSDef));
      for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
      {
        RenderField(&pTD->F1, pTD, nI, &dat, insertAt - 1, ctlFlags, outString);
        pTD->F1.SetTextRC(insertAt, nI + 1, outString);
      }
    }
  }
}

//
//  ID_EDIT_INSERT_SD_CONNECTION
//
void CTMSApp::OnEditInsertSdConnection() 
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
      if(!VerifyUserAccessRights(TMS_CONNECTIONS))
      {
        return;
      }

      int nI, nJ;
      AddConnectionSDDataDef AddConnectionSDData;

      AddConnectionSDData.numRecs = 0;
      for(nI = 0; nI < 96; nI++)
      {
        AddConnectionSDData.recordIDs[nI] = NO_RECORD;
      }
      
      DISPLAYINFO DI;

      DI.fileInfo = pTD->fileInfo;
      AddConnectionSDData.pDI = &DI;

      CAddConnectionSD dlg(NULL, &AddConnectionSDData);

      if(dlg.DoModal() != IDOK)
      {
        return;
      }
//
//  Put the new stuff at the end.
//
      long insertAt;
      long numRows;
//
//  How many rows are there now?
//
      numRows = pTD->F1.GetLastRow();
      insertAt = numRows + 1;
//
//  Display the new record(s)
//
      int  ctlFlags = 0;
      char outString[256];
      AllFilesDef ALLFILES;
      union
      {
        AllKey0Def  ALLKey0;
        AllKey1Def  ALLKey1;
        AllKey2Def  ALLKey2;
      } ALLKeys;

      for(nI = 0; nI < AddConnectionSDData.numRecs; nI++)
      {
//
//  Shift the grid down
//
        pTD->F1.InsertRange(insertAt, -1, insertAt, -1, F1ShiftVertical);
//
//  Adjust MaxRow
//
        pTD->F1.SetMaxRow((long)insertAt);
//
//  And display the new record
//
        ALLKeys.ALLKey0.DATABASEKey0.recordID = AddConnectionSDData.recordIDs[nI];
        btrieve(B_GETEQUAL, TMS_CONNECTIONS, &ALLFILES, &ALLKeys, 0);
        for(nJ = 0; nJ < pTD->fieldInfo.numFields; nJ++)
        {
          if(pTD->fieldInfo.dataType[nJ] == TMSDATA_FLAGS + DATATYPE_BIT)
          {
            DisplayFlagRow(&pTD->F1, pTD, &ALLFILES, insertAt - 1, &nJ);
          }
          else
          {
            RenderField(&pTD->F1, pTD, nJ, &ALLFILES.DATABASE, insertAt - 1, ctlFlags, outString);
            pTD->F1.SetTextRC(insertAt, nJ + 1, outString);
          }
        }
        insertAt++;
        numRows++;
      }
    }
  }
}
//
//  ID_EDIT_UPDATE
//
void CTMSApp::OnEditUpdate()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  char    outString[256];
  long    row[2];
  long    col[2];
  int     nRc;
  int     numRanges;
  int     numRecords;
  int     numRows;
  int     nI;
  int     nJ;
  int     rcode2;
  DISPLAYINFO DI;

  long *pRecordIDs = NULL;
  int  *pRowNumbers = NULL;

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
//  A double-click on the Runs Table fires up Analyze Run
//
      if(ThisType == TMS_RUNS)
      {
        OnCommandsAnalyzeRun();
        return;
      }
//
//  Verify his rights
//
      if(!VerifyUserAccessRights(ThisType))
      {
        return;
      }
//
//  Fall through to update
//
//  Allocate space for the recordIDs
//
      numRows = pTD->F1.GetLastRow();
      pRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * numRows); 
      if(pRecordIDs == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        return;
      }
      pRowNumbers = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * numRows); 
      if(pRowNumbers == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        TMSHeapFree(pRecordIDs);
        return;
      }
//
//  Figure out the ranges
//
      numRanges = pTD->F1.GetSelectionCount();
      numRecords = 0;
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          pRecordIDs[numRecords] = (long)pTD->F1.GetNumberRC(nJ, 1L);
          pRowNumbers[numRecords] = nJ;
          numRecords++;
        }
      }
//
//  Process the update(s)
//
      for(nI = 0; nI < numRecords; nI++)
      {
        nRc = 0;
        updateRecordID = pRecordIDs[nI];
//
//  Directions
//
        if(ThisType == TMS_DIRECTIONS)
        {
          CAddDirection dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Routes
//
        else if(ThisType == TMS_ROUTES)
        {
        CAddRoute dlg(NULL, &updateRecordID);
        nRc = (dlg.DoModal() == IDOK);
          if(nRc)
          {
            SetUpRouteList(m_hwndDlgBar, RSDTOOLBAR_ROUTE, m_RouteRecordID);
          }
        }
//
//  Services
//
        else if(ThisType == TMS_SERVICES)
        {
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDSERVICE), pTD->pane->m_hWnd, (DLGPROC)ADDSERVICEMsgProc);
          if(nRc)
          {
            SetUpServiceList(m_hwndDlgBar, RSDTOOLBAR_SERVICE, m_ServiceRecordID);
          }
        }
//
//  Jurisdictions
//
        else if(ThisType == TMS_JURISDICTIONS)
        {
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDJURISDICTION), pTD->pane->m_hWnd, (DLGPROC)ADDJURISDICTIONMsgProc);
        }
//
//  Divisions
//
        else if(ThisType == TMS_DIVISIONS)
        {
          CAddDivision dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
          if(nRc)
          {
            SetUpDivisionList(m_hwndDlgBar, RSDTOOLBAR_DIVISION, m_DivisionRecordID);
          }
        }
//
//  Nodes
//
        else if(ThisType == TMS_NODES)
        {
          CAddNode dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Patterns
//
        else if(ThisType == TMS_PATTERNS)
        {
          DI.fileInfo = pTD->fileInfo;
          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDPATTERN),
                pTD->pane->m_hWnd, (DLGPROC)ADDPATTERNMsgProc, (LPARAM)&DI);
        }
//
//  Connections
//
        else if(ThisType == TMS_CONNECTIONS)
        {
          CONNECTIONSKey0.recordID = pRecordIDs[nI];
          btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          if(CONNECTIONS.flags & CONNECTIONS_FLAG_STOPSTOP)
          {
            CStopConnection dlg(NULL, &CONNECTIONS);
            nRc = dlg.DoModal();
          }
          else
          {
            DI.fileInfo = pTD->fileInfo;
            DI.baggage = pTD->baggage;
            CAddConnection dlg(NULL, &updateRecordID, &DI);
            nRc = dlg.DoModal();
          }
        }
//
//  Bustypes
//
        else if(ThisType == TMS_BUSTYPES)
        {
          CAddBustype dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Buses
//
        else if(ThisType == TMS_BUSES)
        {
          CAddBus dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Sign Codes
//
        else if(ThisType == TMS_SIGNCODES)
        {
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDSIGNCODE), pTD->pane->m_hWnd, (DLGPROC)ADDSIGNCODEMsgProc);
        }
//
//  Comments
//
        else if(ThisType == TMS_COMMENTS)
        {
          CAddComment dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Trips
//
        else if(ThisType == TMS_TRIPS || ThisType == TMS_STANDARDBLOCKS || ThisType == TMS_DROPBACKBLOCKS)
        {
          DI.fileInfo = pTD->fileInfo;
          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDTRIP),
                pTD->pane->m_hWnd, (DLGPROC)ADDTRIPMsgProc, (LPARAM)&DI);
        }
//
//  Driver types
//
        else if(ThisType == TMS_DRIVERTYPES)
        {
          CAddDriverType dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Drivers
//
        else if(ThisType == TMS_DRIVERS)
        {
          CAddDriver dlg(NULL, &updateRecordID);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  Roster
//
        else if(ThisType == TMS_ROSTER)
        {
          ROSTERKey0.recordID = updateRecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          if(rcode2 == 0)
          {
            CDisplayRoster dlg(NULL, &updateRecordID);
            nRc = (dlg.DoModal() == IDOK);
          }
        }
//
//  Timechecks
//
        else if(ThisType == TMS_TIMECHECKS)
        {
          DI.fileInfo = pTD->fileInfo;
          CAddTimecheck dlg(NULL, &updateRecordID, &DI);
          nRc = (dlg.DoModal() == IDOK);
        }
//
//  "Crew Only" runs
//
        else if(ThisType == TMS_CREWONLY)
        {
          CREWONLYKey0.recordID = updateRecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          if(rcode2 == 0)
          {
            short int nType = (short int)LOWORD(CREWONLY.cutAsRuntype);
            short int nSlot = (short int)HIWORD(CREWONLY.cutAsRuntype);

            if(RUNTYPE[nType][nSlot].flags & RTFLAGS_PLACEHOLDER)
            {
              MessageBeep(MB_ICONSTOP);
              MessageBox(NULL, "You can't update a \"Placeholder\" run.\n\nPlease delete and reinsert", TMS, MB_OK | MB_ICONSTOP);
              nRc = FALSE;
            }
            else
            {
              DI.fileInfo = pTD->fileInfo;
              CAddCrewOnly dlg(NULL, &updateRecordID, &DI);
              nRc = (dlg.DoModal() == IDOK);
            }
          }
        }
//
//  OK?
//
//  Display the changed row
//
        if(nRc)
        {
//
//  Enumerate any open children to see if we have to update the trips
//
          if(ThisType == TMS_CONNECTIONS)
          {
            m_bEstablishRUNTIMES = TRUE;
            CONNECTIONSKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedConnections, (LPARAM)&CONNECTIONS);
          }
//
//  Enumerate any open children to see if we have to update the blocks
//
          else if(ThisType == TMS_TRIPS)
          {
            EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedTrips, (LPARAM)updateRecordID);
          }
          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          if(ThisType != TMS_COMMENTS)
          {
            btrieve(B_GETEQUAL, ThisType, &ALLFILES, &ALLKeys, 0);
          }
          else
          {
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            if(rcode2 == 0)
            {
              memcpy(&ALLFILES, pCommentText, COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH);
              memset(&ALLFILES.hackedCommentText[COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH], 0x00, 1);
            }
          }
          if(ThisType == TMS_PATTERNS || ThisType == TMS_TRIPS ||
                ThisType == TMS_STANDARDBLOCKS || ThisType == TMS_DROPBACKBLOCKS)
          {
            long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

            GetTMSData(pTD, flags);
          }
          else
          {
            if(ThisType == TMS_ROSTER)
            {
              memset(&RosterData, 0x00, sizeof(RosterData));
              RosterData[3] = ALLFILES.ROSTER.WEEK[pTD->fileInfo.rosterWeek].flags;
            }
            for(nJ = 0; nJ < pTD->fieldInfo.numFields; nJ++)
            {
              if(pTD->fieldInfo.dataType[nJ] == TMSDATA_FLAGS + DATATYPE_BIT)
              {
                DisplayFlagRow(&pTD->F1, pTD, &ALLFILES, pRowNumbers[nI] - 1, &nJ);
              }
              else
              {
                RenderField(&pTD->F1, pTD, nJ, &ALLFILES.DATABASE, pRowNumbers[nI], 0, outString);
                pTD->F1.SetTextRC(pRowNumbers[nI], nJ + 1, outString);
              }
            }
          }
        }
        TMSHeapFree(pRecordIDs);
        TMSHeapFree(pRowNumbers);
        if(ThisType == TMS_CONNECTIONS)
        {
          m_bEstablishRUNTIMES = TRUE;
        }
      }
    }
  }

  return;
}

//
//  IDM_E_DELETE
//  IDM_E_UNHOOK
//
#define MAXTRIPS 500
void CTMSApp::OnEditDelete()
{
  HCURSOR saveCursor;
  BOOL    bDoDelete;
  BOOL    firstTime;
  long    assignedToNODESrecordID;
  long    RGRPROUTESrecordID;
  long    SGRPSERVICESrecordID;
  long    blockNumber;
  long    recordToDelete;
  int     rcode2;
  short int numRanges;
  int     numRows;
  int     nI, nJ, nK;
  int     numTrips;
  long    row[2];
  long    col[2];
  long    tripRecordIDs[MAXTRIPS];
  BLOCKSDef   *pTRIPSChunk;
  int         keyNumber;

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
      if(!VerifyUserAccessRights(ThisType))
      {
        return;
      }

      saveCursor = ::SetCursor(hCursorWait);
      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
//
//  Blocks get unhooked
//
      if(ThisType == TMS_STANDARDBLOCKS || ThisType == TMS_DROPBACKBLOCKS)
      {
        keyNumber = pTD->fileInfo.keyNumber;
        pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
        for(nI = 0; nI < numRanges; nI++)
        {
          pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
          if(col[1] != F1_MAXCOL)
            continue;
          if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
            row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
          for(nJ = row[0]; nJ <= row[1]; nJ++)
          {
            TRIPSKey0.recordID = (long)pTD->F1.GetNumberRC(nJ, 1);
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(!m_bCompressedBlocks)
            {
              pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
              pTRIPSChunk->POGNODESrecordID = NO_RECORD;
              pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
              pTRIPSChunk->RGRPROUTESrecordID = NO_RECORD;
              pTRIPSChunk->SGRPSERVICESrecordID = NO_RECORD;
              pTRIPSChunk->blockNumber = 0;
              pTRIPSChunk->blockSequence = NO_TIME;
//
//  If standard blocks get undone, dropback get undone automatically
//
              if(ThisType == TMS_STANDARDBLOCKS)
              {
                TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
                TRIPS.dropback.POGNODESrecordID = NO_RECORD;
                TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
                TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
                TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
                TRIPS.dropback.blockNumber = 0;
                TRIPS.dropback.blockSequence = NO_TIME;
              }
//
//  Fix the time at the MLP if this trip was "service wrapped"
//
              if(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_PLUS)
              {
                TRIPS.timeAtMLP -= 86400;
                TRIPS.tripSequence = TRIPS.timeAtMLP;
                TRIPS.flags -= TRIPS_FLAG_SERVICEWRAP_PLUS;
              }
              else if(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_MINUS)
              {
                TRIPS.timeAtMLP += 86400;
                TRIPS.tripSequence = TRIPS.timeAtMLP;
                TRIPS.flags -= TRIPS_FLAG_SERVICEWRAP_MINUS;
              }
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
//
//  Undoing blocks in compressed display mode
//
            else
            {
//
//  Get the pertinent values from this trip
//
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
              RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
              SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
              blockNumber = pTRIPSChunk->blockNumber;
//
//  Position to the first trip in the block
//
              TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
              TRIPSKey2.RGRPROUTESrecordID = RGRPROUTESrecordID;
              TRIPSKey2.SGRPSERVICESrecordID = SGRPSERVICESrecordID;
              TRIPSKey2.blockNumber = blockNumber;
              TRIPSKey2.blockSequence = NO_TIME;
              rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
//
//  Get all the trips in this block
//
              numTrips = 0;
              while(rcode2 == 0 &&
                    pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                    pTRIPSChunk->RGRPROUTESrecordID == RGRPROUTESrecordID &&
                    pTRIPSChunk->SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
                    pTRIPSChunk->blockNumber == blockNumber)
              {
                tripRecordIDs[numTrips++] = TRIPS.recordID;
                rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              }
//
//  Update all the trips in this block
//
              for(nK = 0; nK < numTrips; nK++)
              {
                TRIPSKey0.recordID = tripRecordIDs[nK];
                btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
                pTRIPSChunk->POGNODESrecordID = NO_RECORD;
                pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
                pTRIPSChunk->RGRPROUTESrecordID = NO_RECORD;
                pTRIPSChunk->SGRPSERVICESrecordID = NO_RECORD;
                pTRIPSChunk->blockNumber = 0;
                pTRIPSChunk->blockSequence = NO_TIME;
//
//  If standard blocks get undone, dropback get undone automatically
//
                if(ThisType == TMS_STANDARDBLOCKS)
                {
                  TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
                  TRIPS.dropback.POGNODESrecordID = NO_RECORD;
                  TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
                  TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
                  TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
                  TRIPS.dropback.blockNumber = 0;
                  TRIPS.dropback.blockSequence = NO_TIME;
                }
                btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              }
            }
          }
        }
        EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedConnections, (LPARAM)NULL);
      }
//
//  Everyone else gets deleted
//
      else
      {
//
//  Did we delete connections?
//
        bDoDelete = TRUE;
        if(ThisType == TMS_CONNECTIONS)
        {
          m_bEstablishRUNTIMES = TRUE;
        }
//
//  Warn him on the roster
//
        else if(ThisType == TMS_ROSTER)
        {
          CString s;
          
          s.LoadString(TEXT_400);
          MessageBeep(MB_ICONQUESTION);
          bDoDelete = (MessageBox(m_pMainWnd->m_hWnd, s, TMS, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES);
        }
        if(bDoDelete)
        {
          for(firstTime = TRUE, nI = 0; nI < numRanges; nI++)
          {
            if(!bDoDelete)
            {
              break;
            }
            pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
            if(col[1] != F1_MAXCOL)
            {
              continue;
            }
            if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
            {                     // pressing above row 1 and GetSelection returns MaxRow in row[1]
              row[1] = numRows;
            }
            for(nJ = row[0]; nJ <= row[1]; nJ++)
            {
              recordToDelete = (long)pTD->F1.GetNumberRC(nJ, 1);
              if((bDoDelete = DeleteRecord(ThisType, recordToDelete, TRUE, firstTime, pTD->pane->m_hWnd)) == FALSE)
              {
                break;
              }
              firstTime = FALSE;
            }
          }
        }
      }
//
//  Adjust the display
//
      if(bDoDelete)
      {
        for(nI = numRanges - 1; nI >= 0; nI--)
        {
          pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
          if(col[1] != F1_MAXCOL)
            continue;
          if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
            row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
          pTD->F1.DeleteRange(row[0], col[0], row[1], col[1], F1ShiftVertical);
        }
        row[0] = pTD->F1.GetLastRow();
        if(row[0] != 0)
        {
          pTD->F1.SetMaxRow(row[0]);
        }
        pTD->F1.SetSelection(1, 1, 1, 1);
        ::SetCursor(saveCursor);
      }
//
//  Enumerate any open children to see if we have to update the trips
//
      if(ThisType == TMS_CONNECTIONS)
      {
        EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedConnections, (LPARAM)NULL);
      }
//
//  Or the Blocks, Runs, and Roster
//
      else if(ThisType == TMS_TRIPS || ThisType == TMS_RUNS || ThisType == TMS_ROSTER)
      {
        EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedTrips, (LPARAM)NULL);
      }
//
//  Refresh the garage list
//
      numGaragesInGarageList = 0;
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      while(rcode2 == 0 && (NODES.flags & NODES_FLAG_GARAGE))
      {
        garageList[numGaragesInGarageList++] = NODES.recordID;
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
//
//  If we deleted a route, service, or division, then update the RSDTOOLBAR.
//
      if(ThisType == TMS_ROUTES)
      {
        SetUpRouteList(m_hwndDlgBar, RSDTOOLBAR_ROUTE, m_RouteRecordID);
      }
      if(ThisType == TMS_SERVICES)
      {
        SetUpServiceList(m_hwndDlgBar, RSDTOOLBAR_SERVICE, m_ServiceRecordID);
      }
      if(ThisType == TMS_DIVISIONS)
      {
        SetUpDivisionList(m_hwndDlgBar, RSDTOOLBAR_DIVISION, m_DivisionRecordID);
      }
    }
  }
}

//
//  ID_EDIT_UPDATE_TRIP_NUMBER
//
void CTMSApp::OnEditUpdateTripNumber()
{
  BOOL         bUpdated;
  int          nRc;
  int          numRanges;
  long         row[2];
  long         col[2];
  int          nI;
  int          nJ;
  int          numRows;

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

      bUpdated = FALSE;
      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();

      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_RENUMBER), pTD->pane->m_hWnd, (DLGPROC)CHANGETRIPNUMBERMsgProc);
          if(!nRc)
            break;
          bUpdated = TRUE;
        }
      }
      if(bUpdated)
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }
}

//
//  ID_EDIT_UPDATE_RANGE_PATTERNS
//  ID_EDIT_UPDATE_RANGE_OPERATOR_COMMENTS
//  ID_EDIT_UPDATE_RANGE_CUSTOMER_COMMENTS
//  ID_EDIT_UPDATE_RANGE_SIGNCODES
//  ID_EDIT_UPDATE_RANGE_VEHICLES
//  ID_EDIT_UPDATE_RANGE_LAYOVER
//  ID_EDIT_UPDATE_RANGE_SHIFT
//
void CTMSApp::OnEditUpdateRangePatterns()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_PATTERNS);
}
void CTMSApp::OnEditUpdateRangeOperatorComments()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_OPERATOR_COMMENTS);
}
void CTMSApp::OnEditUpdateRangeCustomerComments()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_CUSTOMER_COMMENTS);
}
void CTMSApp::OnEditUpdateRangeSigncodes()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_SIGNCODES);
}
void CTMSApp::OnEditUpdateRangeVehicles()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_VEHICLES);
}
void CTMSApp::OnEditUpdateRangeLayover()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_LAYOVER);
}
void CTMSApp::OnEditUpdateRangeShift()
{
  OnEditChangeTripData(ID_EDIT_UPDATE_RANGE_SHIFT);
}
void CTMSApp::OnEditChangeTripData(WPARAM which)
{
  typedef struct PASSEDDATAStruct
  {
    long ROUTESrecordID;
    long SERVICESrecordID;
    long directionIndex;
    long *pRecordIDs;
    int  numRecords;
  } PASSEDDATADef;

  PASSEDDATADef PassedData;
  BOOL    bRC;
  int     numRanges;
  long    row[2];
  long    col[2];
  int     nI;
  int     nJ;
  int     numRows;

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

      PassedData.ROUTESrecordID = pTD->fileInfo.routeRecordID;
      PassedData.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
      PassedData.directionIndex = pTD->fileInfo.directionIndex;
      numRows = pTD->F1.GetLastRow();
      PassedData.pRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * numRows); 
      if(PassedData.pRecordIDs == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        return;
      }
      numRanges = pTD->F1.GetSelectionCount();
      PassedData.numRecords = 0;
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
        {
          continue;
        }
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
        {                     // pressing above row 1 and GetSelection returns MaxRow in row[1]
          row[1] = numRows;
        }
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          PassedData.pRecordIDs[PassedData.numRecords++] = (long)pTD->F1.GetNumberRC(nJ, 1);
        }
      }
      if(which == ID_EDIT_UPDATE_RANGE_PATTERNS)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGEPATTERNS),
              pTD->pane->m_hWnd, (DLGPROC)CHANGEPATTERNSMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_OPERATOR_COMMENTS)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGEOCOMMENTS),
              pTD->pane->m_hWnd, (DLGPROC)CHANGEOCOMMENTSMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_CUSTOMER_COMMENTS)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGECCOMMENTS),
              pTD->pane->m_hWnd, (DLGPROC)CHANGECCOMMENTSMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_SIGNCODES)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGESIGNCODES),
              pTD->pane->m_hWnd, (DLGPROC)CHANGESIGNCODESMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_VEHICLES)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGEBUSTYPES),
              pTD->pane->m_hWnd, (DLGPROC)CHANGEBUSTYPESMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_LAYOVER)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGELAYOVER),
              pTD->pane->m_hWnd, (DLGPROC)CHANGELAYOVERMsgProc, (LPARAM)&PassedData);
      }
      else if(which == ID_EDIT_UPDATE_RANGE_SHIFT)
      {
        bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGETRIPSHIFT),
              pTD->pane->m_hWnd, (DLGPROC)CHANGETRIPSHIFTMsgProc, (LPARAM)&PassedData);
      }
      if(bRC)
      {
        long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

        GetTMSData(pTD, flags);
      }
      TMSHeapFree(PassedData.pRecordIDs);
    }
  }
}


//
//  ID_EDIT_FIND_BLOCK
//
void CTMSApp::OnEditFindBlock()
{
  BOOL bUpdated;
  long row[2];
  long col[2];
  long TRIPSrecordID;
  int  numRanges;
  int  nI;
  int  nJ;
  int  numRows;

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

      bUpdated = FALSE;
      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();

      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          TRIPSrecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          CFindBlock dlg(NULL, TRIPSrecordID);
          if(dlg.DoModal() != IDOK) 
            break;
          bUpdated = TRUE;
        }
      }
      if(bUpdated)
      {
        EnumChildWindows(m_pMainWnd->m_hWnd, (WNDENUMPROC)EnumForChangedTrips, (LPARAM)NULL);
        GetTMSData(pTD, GETTMSDATA_FLAG_CLEAR);
      }
    }
  }
}

//
//  ID_EDIT_ADD_RUN_COMMENT
//
void CTMSApp::OnEditAddRunComment()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  char    outString[256];
  int     nRc;
  int     numRanges;
  int     numRows;
  long    row[2];
  long    col[2];
  int     nI;
  int     nJ;
  int     nK;

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

      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDRUNCOMMENT),
                pTD->pane->m_hWnd, (DLGPROC)ADDRUNCOMMENTMsgProc);
          if(!nRc)
            break;
          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          GetRunElements(pTD->pane->m_hWnd, &ALLFILES.RUNS, &PROPOSEDRUN, &COST, TRUE);
          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          for(nK = 0; nK < pTD->fieldInfo.numFields; nK++)
          {
            RenderField(&pTD->F1, pTD, nK, &ALLFILES.DATABASE, nJ, 0, outString);
            pTD->F1.SetTextRC(nJ, nK + 1, outString);
          }
        }
      }
    }
  }
}

//
//  ID_EDIT_REASSIGN_RUNTYPE
//
void CTMSApp::OnEditReassignRuntype()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  char    outString[256];
  int     nRc;
  int     numPieces;
  int     numRanges;
  int     numRows;
  long    row[2];
  long    col[2];
  int     nI;
  int     nJ;
  int     nK;
  int     nL;

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

      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_ALTERRUNTYPE),
                pTD->pane->m_hWnd, (DLGPROC)ALTERRUNTYPEMsgProc);
          if(!nRc)
            break;
          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          numPieces = GetRunElements(pTD->pane->m_hWnd, &ALLFILES.RUNS, &PROPOSEDRUN, &COST, TRUE);
          ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          for(nK = 0; nK < numPieces; nK++)
          {
            if(LOWORD(RUNSVIEW[0].runType) == ILLEGAL_INDEX ||
                  RUNSVIEW[0].runType == NO_RUNTYPE || RUNSVIEW[numPieces - 1].payTime == 0L)
              pTD->F1.SetFontColor(PALETTERGB(255, 0, 0));
            else
              pTD->F1.SetFontColor(PALETTERGB(0, 0, 0));
            for(nL = 0; nL < pTD->fieldInfo.numFields; nL++)
            {
              RenderField(&pTD->F1, pTD, nL, &ALLFILES.DATABASE, nJ, nK, outString);
              pTD->F1.SetTextRC(nJ + nK, nL + 1, outString);
            }
            if(nK != numPieces - 1)
            {
              btrieve(B_GETPOSITION, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
              btrieve(B_GETDIRECT, TMS_RUNS, &ALLFILES, &ALLKeys, 1);
              btrieve(B_GETNEXT, TMS_RUNS, &ALLFILES, &ALLKeys, 1);
            }
          }
        }
      }
    }
  }
}

//
//  ID_EDIT_EXTRABOARD_EDITOR
//
void CTMSApp::OnEditExtraboardEditor()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  EXTRABOARDEDITORPARAMDef EBP;
  char    outString[256];
  int     nRc;
  int     numPieces;
  int     numRanges;
  int     numRows;
  long    row[2];
  long    col[2];
  long    firstPieceRecordID;
  long    pieceNumber;
  long    divisionRecordID;
  long    serviceRecordID;
  long    runNumber;
  int     nI;
  int     nJ;
  int     nK;
  int     nL;
  short int nType;
  short int nSlot;

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

      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          ALLKeys.ALLKey0.DATABASEKey0.recordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          runNumber = ALLFILES.RUNS.runNumber;
          pieceNumber = ALLFILES.RUNS.pieceNumber - 1;
          if(ALLFILES.RUNS.pieceNumber == 1)
            firstPieceRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          else
          {
            divisionRecordID = ALLFILES.RUNS.DIVISIONSrecordID;
            serviceRecordID = ALLFILES.RUNS.SERVICESrecordID;
            ALLKeys.ALLKey1.RUNSKey1.DIVISIONSrecordID = ALLFILES.RUNS.DIVISIONSrecordID;
            ALLKeys.ALLKey1.RUNSKey1.SERVICESrecordID = ALLFILES.RUNS.SERVICESrecordID;
            ALLKeys.ALLKey1.RUNSKey1.runNumber = ALLFILES.RUNS.runNumber;
            ALLKeys.ALLKey1.RUNSKey1.pieceNumber = 1;
            btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 1);
            firstPieceRecordID = ALLFILES.RUNS.recordID;
          }
          numPieces = GetRunElements(pTD->pane->m_hWnd, &ALLFILES.RUNS, &PROPOSEDRUN, &COST, TRUE);
          if(COST.TOTAL.makeUpTime <= 0)
            TMSError(NULL, MB_ICONINFORMATION, ERROR_246, (HANDLE)NULL);
          if(COST.runtype == NO_RUNTYPE || COST.runtype == UNCLASSIFIED_RUNTYPE)
            TMSError(NULL, MB_ICONINFORMATION, ERROR_247, (HANDLE)NULL);
          EBP.runNumber = runNumber;
          EBP.pieceNumber = pieceNumber + 1;
          EBP.availableTime = COST.TOTAL.makeUpTime;
          if(COST.TOTAL.makeUpTime > 0)
          {
            for(nK = 0; nK < numPieces; nK++)
            {
              if(RUNSVIEW[nK].flags & RUNS_FLAG_EXTRABOARDPRIOR)
                EBP.availableTime -=
                      RUNSVIEW[nK].startOfPieceExtraboardEnd -
                      RUNSVIEW[nK].startOfPieceExtraboardStart;
              if(RUNSVIEW[nK].flags & RUNS_FLAG_EXTRABOARDAFTER)
                EBP.availableTime -=
                      RUNSVIEW[nK].endOfPieceExtraboardEnd -
                      RUNSVIEW[nK].endOfPieceExtraboardStart;
              if(EBP.availableTime <= 0)
              {
                EBP.availableTime = 0;
                TMSError(NULL, MB_ICONINFORMATION, ERROR_248, (HANDLE)NULL);
              }
            }
          }
          EBP.pieceStart = RUNSVIEW[pieceNumber].runOnTime -
                COST.TRAVEL[pieceNumber].startTravelTime;
          EBP.pieceEnd = RUNSVIEW[pieceNumber].runOffTime +
                COST.TRAVEL[pieceNumber].endTravelTime;
          if(RUNSVIEW[pieceNumber].flags & RUNS_FLAG_EXTRABOARDPRIOR)
          {
            EBP.startStart = RUNSVIEW[pieceNumber].startOfPieceExtraboardStart;
            EBP.startEnd = RUNSVIEW[pieceNumber].startOfPieceExtraboardEnd;
          }
          else
          {
            EBP.startStart = NO_TIME;
            EBP.startEnd = NO_TIME;
          }
          if(RUNSVIEW[pieceNumber].flags & RUNS_FLAG_EXTRABOARDAFTER)
          {
            EBP.endStart = RUNSVIEW[pieceNumber].endOfPieceExtraboardStart;
            EBP.endEnd = RUNSVIEW[pieceNumber].endOfPieceExtraboardEnd;
          }
          else
          {
            EBP.endStart = NO_TIME;
            EBP.endEnd = NO_TIME;
          }
          EBP.flags = RUNSVIEW[pieceNumber].flags;
          nType = (short int)LOWORD(COST.runtype);
          nSlot = (short int)HIWORD(COST.runtype);
          EBP.maxPieceSize = RUNTYPE[nType][nSlot].PIECE[pieceNumber].maxPieceSize;
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXTRABOARDEDITOR),
                pTD->pane->m_hWnd, (DLGPROC)EXTRABOARDEDITORMsgProc, (LPARAM)&EBP);
          if(!nRc)
            break;
          ALLKeys.ALLKey0.DATABASEKey0.recordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          ALLFILES.RUNS.flags = EBP.flags;
          ALLFILES.RUNS.prior.startTime = EBP.startStart;
          ALLFILES.RUNS.prior.endTime = EBP.startEnd;
          ALLFILES.RUNS.after.startTime = EBP.endStart;
          ALLFILES.RUNS.after.endTime = EBP.endEnd;
          btrieve(B_UPDATE, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          ALLKeys.ALLKey0.DATABASEKey0.recordID = firstPieceRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          numPieces = GetRunElements(pTD->pane->m_hWnd, &ALLFILES.RUNS, &PROPOSEDRUN, &COST, TRUE);
          ALLKeys.ALLKey0.DATABASEKey0.recordID = firstPieceRecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
          for(nK = 0; nK < numPieces; nK++)
          {
            if(LOWORD(RUNSVIEW[0].runType) == ILLEGAL_INDEX ||
                  RUNSVIEW[0].runType == NO_RUNTYPE ||
                  RUNSVIEW[numPieces - 1].payTime == 0L)
              pTD->F1.SetFontColor(PALETTERGB(255, 0, 0));
            else
              pTD->F1.SetFontColor(PALETTERGB(0, 0, 0));
            for(nL = 0; nL < pTD->fieldInfo.numFields; nL++)
            {
              RenderField(&pTD->F1, pTD, nL, &ALLFILES.DATABASE, nJ, nK, outString);
              pTD->F1.SetTextRC(nJ + nK - pieceNumber, nL + 1, outString);
            }
            if(nK != numPieces - 1)
            {
              btrieve(B_GETPOSITION, TMS_RUNS, &ALLFILES, &ALLKeys, 0);
              btrieve(B_GETDIRECT, TMS_RUNS, &ALLFILES, &ALLKeys, 1);
              btrieve(B_GETNEXT, TMS_RUNS, &ALLFILES, &ALLKeys, 1);
            }
          }
        }
      }
    }
  }
}

//
//  ID_EDIT_ASSIGN_DRIVER_TO_ROSTER
//
void CTMSApp::OnEditAssignDriverToRoster()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  char    outString[256];
  long    row[2];
  long    col[2];
  int     numRanges;
  int     numRows;
  int     nI;
  int     nJ;
  int     nK;
  int     rcode2;

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

      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          ROSTERKey0.recordID = updateRecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          if(rcode2 == 0 && ROSTER.DRIVERSrecordID == NO_RECORD)
          {
            CAssignDriver dlg(NULL, &updateRecordID, NULL);
            if(dlg.DoModal() != IDOK) 
              break;
            ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_ROSTER, &ALLFILES, &ALLKeys, 0);
            memset(&RosterData, 0x00, sizeof(RosterData));
            RosterData[3] = ALLFILES.ROSTER.WEEK[pTD->fileInfo.rosterWeek].flags;
            for(nK = 0; nK < pTD->fieldInfo.numFields; nK++)
            {
              if(pTD->fieldInfo.dataType[nK] == TMSDATA_FLAGS + DATATYPE_BIT)
                DisplayFlagRow(&pTD->F1, pTD, &ALLFILES, nJ - 1, &nK);
              else
              {
                RenderField(&pTD->F1, pTD, nK, &ALLFILES.DATABASE, nJ, 0, outString);
                pTD->F1.SetTextRC(nJ, nK + 1, outString);
              }
            }
          }
        }
      }
    }
  }
}

//
//  ID_EDIT_REMOVE_DRIVER_FROM_ROSTER
//
void CTMSApp::OnEditRemoveDriverFromRoster()
{
  AllFilesDef ALLFILES;
  union
  {
    AllKey0Def  ALLKey0;
    AllKey1Def  ALLKey1;
    AllKey2Def  ALLKey2;
  } ALLKeys;
  long row[2];
  long col[2];
  char outString[256];
  int  numRanges;
  int  numRows;
  int  nI;
  int  nJ;
  int  nK;
  int  rcode2;

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

      numRanges = pTD->F1.GetSelectionCount();
      numRows = pTD->F1.GetLastRow();
      for(nI = 0; nI < numRanges; nI++)
      {
        pTD->F1.GetSelection(nI, &row[0], &col[0], &row[1], &col[1]);
        if(col[1] != F1_MAXCOL)
          continue;
        if(row[1] > numRows)  // This fixes the problem that occurs when he selects all by
          row[1] = numRows;   // pressing above row 1 and GetSelection returns MaxRow in row[1]
        for(nJ = row[0]; nJ <= row[1]; nJ++)
        {
          updateRecordID = (long)pTD->F1.GetNumberRC(nJ, 1);
          ROSTERKey0.recordID = updateRecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          if(rcode2 == 0)
          {
            ROSTER.DRIVERSrecordID = NO_RECORD;
            btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
            ALLKeys.ALLKey0.DATABASEKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_ROSTER, &ALLFILES, &ALLKeys, 0);
            memset(&RosterData, 0x00, sizeof(RosterData));
            RosterData[3] = ALLFILES.ROSTER.WEEK[pTD->fileInfo.rosterWeek].flags;
            for(nK = 0; nK < pTD->fieldInfo.numFields; nK++)
            {
              if(pTD->fieldInfo.dataType[nK] == TMSDATA_FLAGS + DATATYPE_BIT)
                DisplayFlagRow(&pTD->F1, pTD, &ALLFILES, nJ - 1, &nK);
              else
              {
                RenderField(&pTD->F1, pTD, nK, &ALLFILES.DATABASE, nJ, 0, outString);
                pTD->F1.SetTextRC(nJ, nK + 1, outString);
              }
            }
          }
        }
      }
    }
  }
}

//
//  ID_EDIT_SELECT_ALL
//
void CTMSApp::OnEditSelectAll()
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
      long lastRow;

      lastRow = pTD->F1.GetLastRow();
      pTD->F1.SetSelection(1, -1, lastRow, -1);
    }
  }
}

//
//  ID_EDIT_CREATEPLACEHOLDERRUNS
//

void CTMSApp::OnEditCreateplaceholderruns() 
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

//
//  Verify his rights
//
      if(!VerifyUserAccessRights(ThisType))
      {
        return;
      }
//
//  Fire up the dialog
//
      long updateRecordID;

      CPlaceHolder dlg(NULL, &updateRecordID);

      if(dlg.DoModal() == IDOK)
      {
        CREWONLY.recordID = updateRecordID;
        btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);

        long flags = GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR;

        GetTMSData(pTD, flags);
      }
    }
  }
}
