//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "io.h"
}
#include "TMS.h"
#include "MainFrm.h"

//
//  Event handlers
//
void CF1Book::OnClickVcf1ctrl1(long nRow, long nCol) 
{
}

void CF1Book::OnRClickVcf1ctrl1(long nRow, long nCol) 
{
  CWnd* pParent = GetParent();
  
  if(pParent)
  {
    long displayIndex = GetWindowLong(pParent->m_hWnd, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
//      pParent->SendMessage(WM_COMMAND, (WPARAM)WM_CONTEXTMENU, (LPARAM)0);
    }
  }
}

void CF1Book::OnDblClickVcf1ctrl1(long nRow, long nCol) 
{
  CWnd* pParent = GetParent();
  
  if(pParent)
  {
    long displayIndex = GetWindowLong(pParent->m_hWnd, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
      if(nRow == 0)
      {
        SetSelection(-1, nCol, -1, nCol);
        pParent->SendMessage(WM_COMMAND, (WPARAM)ID_BROWSE_COLUMN_CHARACTERISTICS, (LPARAM)0);
      }
      else
      {
        SetSelection(nRow, -1, nRow, -1);

        pParent->SendMessage(WM_COMMAND, (WPARAM)ID_EDIT_UPDATE, (LPARAM)0);
      }
    }
  }
}

void CF1Book::OnMouseUpVcf1ctrl1(short Button, short Shift, long x, long y) 
{
  if(!(Button & VK_LBUTTON) | Shift)
    return;

  CWnd* pParent = GetParent();
  
  if(pParent)
  {
    long displayIndex = GetWindowLong(pParent->m_hWnd, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
      int height = pTD->F1.GetHdrHeight();
      if(y > height / 16)
        return;

      char szWidths[512];
      char szNodeWidth[32];
      int  nI;
      int  nJ;
      int  lastCol;
      int  width;
//
//  Get the last column of data.  If the table is empty, set it to numFields
//
      lastCol = pTD->F1.GetLastCol();
      if(lastCol == 0)
        lastCol = pTD->fieldInfo.numFields;
//
//  Loop through the columns and get the widths
//
      sprintf(tempString, "Width%ld", pTD->fileInfo.fileNumber);
      strcpy(szWidths, "");
//
//  nJ tracks whether of not we get a node display width.
//  These values are not recorded in the "Width" entry, as
//  the number of nodes can change from table to table.
//
      for(nJ = NO_RECORD, nI = 1; nI <= lastCol; nI++)
      {
        width = pTD->F1.GetColWidth(nI);
        if(pTD->fieldInfo.dataType[nI - 1] == TMSDATA_PATTERNCHECKS + DATATYPE_BIT ||
            pTD->fieldInfo.dataType[nI - 1] == TMSDATA_PATTERNTIMES + DATATYPE_BIT)
        {
          if(nJ == NO_RECORD)
            nJ = width;
        }
        else
        {
          itoa(width, szarString, 10);
          strcat(szWidths, szarString);
          strcat(szWidths, " ");
        }
      }
//
//  Did we get a node display width?
//
      if(nJ != NO_RECORD)
      {
        if(nJ < 1536)   // Too small
          nJ = 1536;
        nodeDisplayWidth = nJ;
        itoa(nodeDisplayWidth, szNodeWidth, 10);
        WritePrivateProfileString((LPSTR)userName, "NodeDisplayWidth", (LPSTR)szNodeWidth, (LPSTR)szDatabaseFileName);
      }
//
//  Write out the column widths
//
      WritePrivateProfileString((LPSTR)userName, (LPSTR)tempString, (LPSTR)szWidths, (LPSTR)szDatabaseFileName);
//
//  In the case of the Roster, get the column height of the rifst
//   
      if(pTD->fileInfo.fileNumber == TMS_ROSTER)
      {
        char szHeight[16];
        int  height;

        height = pTD->F1.GetRowHeight(1);
        itoa(height, szHeight, 10);
        WritePrivateProfileString((LPSTR)userName, "RosterRowHeight", (LPSTR)szHeight, (LPSTR)szDatabaseFileName);
      }
    }
  }
}
