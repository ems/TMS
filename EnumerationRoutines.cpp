//
//  EnumForChangedConnections()
//
//  Enumeration callback to deal with open tables that could be affected by a
//  change to the Connections Table.
//
//  If the Blocks, Runs, and/or Roster Table are open when a connection is added,
//  updated, or deleted, then the table simply gets redrawn.  If a trips table is
//  open, then the timepoints of the base pattern displayed in that table are
//  checked to see if and update is necessary.
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "tms.h"

#include "MainFrm.h"
#include "ChildFrm.h"

BOOL CALLBACK CTMSApp::EnumForChangedConnections(HWND hWndChild, LPARAM lParam)
{
  CONNECTIONSDef *pCONNECTIONS;
  BOOL bFound;
  int  nI;
  int  nJ;

//
//  Get the class name of the child window.  We don't
//  want to deal with those that don't matter.
//
  GetClassName(hWndChild, tempString, sizeof(tempString));
  if(strcmp(tempString, m_lpszClass) != 0)
  {
    return(TRUE);  // To continue the enumeration
  }
//
//  Locate this window
//
  for(bFound = FALSE, nI = 0; nI < MAXOPENTABLES; nI++)
  {
    if(m_TableDisplay[nI].pane == NULL)  
    {
      continue;
    }
    if(m_TableDisplay[nI].pane->m_hWnd == hWndChild)
    {
      bFound = TRUE;
      break;
    }
  }
  if(!bFound)
  {
    return(TRUE);
  }
//
//  Check the pointers and see if there's anything on the table
//
  TableDisplayDef *pTD = &m_TableDisplay[nI];

  if(!pTD || !pTD->F1 || pTD->fileInfo.position == NO_POSITION)
  {
    return(TRUE);
  }
//
//  Blocks, Runs, and/or the Roster - perform a redraw
//
  if(pTD->fileInfo.fileNumber == TMS_STANDARDBLOCKS ||
        pTD->fileInfo.fileNumber == TMS_DROPBACKBLOCKS ||
        pTD->fileInfo.fileNumber == TMS_RUNS ||
        pTD->fileInfo.fileNumber == TMS_ROSTER)
  {
    GetTMSData(pTD, GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
  }
//
//  Outbound and inbound trips
//
  else if(pTD->fileInfo.fileNumber == TMS_TRIPS)
  {
    pCONNECTIONS = (CONNECTIONSDef *)lParam;
    if(pCONNECTIONS == NULL)  // If NULL, just redraw.
    {
      GetTMSData(pTD, GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
    }
    else  // Otherwise, loop through the timepoints on the screen
    {
      int start = pTD->fieldInfo.firstNodeColumn;
      int   end = pTD->fieldInfo.firstNodeColumn + pTD->fieldInfo.cNumPatternNodes; // - 1;
 
      for(bFound = FALSE, nI = start; nI <  end - 1; nI++)
      {
        for(nJ = nI + 1; nJ < end; nJ++)
        {
          if((pTD->fieldInfo.recordID[nI] == CONNECTIONS.fromNODESrecordID &&
                pTD->fieldInfo.recordID[nJ] == CONNECTIONS.toNODESrecordID) ||
             (pTD->fieldInfo.recordID[nI] == CONNECTIONS.toNODESrecordID &&
                pTD->fieldInfo.recordID[nJ] == CONNECTIONS.fromNODESrecordID))
          {
            bFound = TRUE;
            break;
          }
        }
        if(bFound)
        {
          break;
        }
      }
      if(bFound)
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
      }
    }
  }

  return(TRUE);
}


BOOL CALLBACK CTMSApp::EnumForChangedTrips(HWND hWndChild, LPARAM lParam)
{
  long updateRecordID = (long)lParam;
  BOOL bFound;
  int  nI;

//
//  Get the class name of the child window.  We don't
//  want to deal with those that don't matter.
//
  GetClassName(hWndChild, tempString, sizeof(tempString));
  if(strcmp(tempString, m_lpszClass) != 0)
  {
    return(TRUE);  // To continue the enumeration
  }
//
//  Locate this window
//
  for(bFound = FALSE, nI = 0; nI < MAXOPENTABLES; nI++)
  {
    if(m_TableDisplay[nI].pane == NULL)
    {
      continue;
    }
    if(m_TableDisplay[nI].pane->m_hWnd == hWndChild)
    {
      bFound = TRUE;
      break;
    }
  }
  if(!bFound)
  {
    return(TRUE);
  }
//
//  Check the pointers and see if there's anything on the table
//
  TableDisplayDef *pTD = &m_TableDisplay[nI];

  if(!pTD || !pTD->F1 || pTD->fileInfo.position == NO_POSITION)
  {
    return(TRUE);
  }
//
//  Blocks, Runs, and/or the Roster - perform a redraw
//
  if(pTD->fileInfo.fileNumber == TMS_STANDARDBLOCKS ||
        pTD->fileInfo.fileNumber == TMS_DROPBACKBLOCKS ||
        pTD->fileInfo.fileNumber == TMS_RUNS ||
        pTD->fileInfo.fileNumber == TMS_ROSTER)
  {
//
//  When updateRecordID is NULL, it means that we're here on a delete
//
    if(updateRecordID == NULL)
    {
      GetTMSData(pTD, GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
    }
//
//  We're here on an update (updateRecordID != NULL)
//
    else
    {
      TRIPSKey0.recordID = updateRecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(TRIPS.standard.RGRPROUTESrecordID == pTD->fileInfo.routeRecordID &&
            TRIPS.standard.SGRPSERVICESrecordID == pTD->fileInfo.serviceRecordID)
      {
        GetTMSData(pTD, GETTMSDATA_FLAG_BUILD | GETTMSDATA_FLAG_CLEAR | GETTMSDATA_FLAG_SETROW1);
      }
    }
  }

  return(TRUE);  // Return TRUE to continue the enumeration
}
