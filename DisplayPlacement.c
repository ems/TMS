//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void DisplayPlacement(HWND hWnd, int iCtl, PDISPLAYINFO pDI, long flags)
{
  HANDLE hCtl;
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char routeName[ROUTES_NAME_LENGTH + 1];
  char serviceName[SERVICES_NAME_LENGTH + 1];
  char directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char divisionName[DIVISIONS_NAME_LENGTH + 1];

  if(pDI == NULL)
    return;
//
//  Set the window and get the format string
//
  hCtl = GetDlgItem(hWnd, iCtl);
  SendMessage(hCtl, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
//
//  Get the particulars
//
//  Route
//
  if(flags & PLACEMENT_ROUTE)
  {
    ROUTESKey0.recordID = pDI->fileInfo.routeRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
  }
//
//  Service
//
  if(flags & PLACEMENT_SERVICE)
  {
    SERVICESKey0.recordID = pDI->fileInfo.serviceRecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
  }
//
//  Direction
//
  if(flags & PLACEMENT_DIRECTION)
  {
    if(!(flags & PLACEMENT_ROUTE))
    {
      ROUTESKey0.recordID = pDI->fileInfo.routeRecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    }
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[pDI->fileInfo.directionIndex];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
  }
//
//  Division
//
  if(flags & PLACEMENT_DIVISION)
  {
    DIVISIONSKey0.recordID = pDI->fileInfo.divisionRecordID;
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
  }
//
//  Display them
//
//  The possibilities are:
//
//  1.  Rte/Ser/Dir
//  2.  Rte/Ser
//  3.  Ser/Div
//  4.  Dir
//  5.  Div
//
  strcpy(tempString, "");
//
//  Case 1: Rte/Ser/Dir
//
  
  if(flags & PLACEMENT_ROUTE && flags & PLACEMENT_SERVICE && flags & PLACEMENT_DIRECTION)
    sprintf(tempString, szFormatString, routeNumber, routeName, serviceName, directionName);
//
//  Case 2: Rte/Ser
//
  else if(flags & PLACEMENT_ROUTE && flags & PLACEMENT_SERVICE)
    sprintf(tempString, szFormatString, routeNumber, routeName, serviceName);
//
//  Case 3: Ser/Div
//
  else if(flags & PLACEMENT_SERVICE && flags & PLACEMENT_DIVISION)
    sprintf(tempString, szFormatString, serviceName, divisionName);
//
//  Case 4: Dir
//
  else if(flags & PLACEMENT_DIRECTION)
    sprintf(tempString, szFormatString, directionName);
//
//  Case 5: Div
//
  else if(flags & PLACEMENT_DIVISION)
    sprintf(tempString, szFormatString, pDI->fileInfo.rosterWeek + 1, divisionName);
//
//  Send the string to the control
//
  SendMessage(hCtl, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  All done
//
  return;
}
