//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
} // extern "C"

#include "TMS.h"
#include "mapinfow.h"   // ADDED FOR INTEGRATED MAPPING SUPPORT

//
//  Command Info Parameters
//
//  Do not use these #defined values in CommandInfo calls, as they
//  will crash KERNEL32.DLL hard.  Use the numbers instead.
//
//  CMD_INFO_X          1
//  CMD_INFO_Y          2
//  CMD_INFO_FIND_RC    3
//  CMD_INFO_FIND_ROWID 4
//  
extern "C" {

BOOL GetLongLat(char *searchString, float *x, float *y, BOOL bUseStreet, HWND hWnd)
{
  BOOL bResult;
  char szCommand[256];

  sprintf(szCommand, "Set Application Window %ld", hWnd);
  mapinfo.Do(szCommand);
  sprintf(szCommand, "Set Next Document Parent %ld Style 1", hWnd);
  mapinfo.Do(szCommand);
//
//  Bermuda - Earth Projection 8, 104, \"m\", -63, 0, 0.9996, 500000, 0
//  Kingston - Nonearth Units "m" Bounds (359500, 4894300)(405800, 4926200)
//
  if(!m_bUseLatLong)
  {
    strcpy(szCommand, "Set Coordsys ");
    strcat(szCommand, szCoordsys);
    mapinfo.Do(szCommand);
  }
  if(bUseStreet)
  {
    sprintf(szCommand, "Find Using StreetFile(%s)", szStreetColumn);
  }
  else
  {
    strcpy(szCommand, "Find Using LandmarkFile(Name)");
  }
  mapinfo.Do(szCommand); 
  sprintf(szCommand, "Find \"%s\" Interactive", searchString);
  mapinfo.Do(szCommand);
  bResult = atoi(mapinfo.Eval("CommandInfo(3)")) >= 1;
  if(bResult)
  {
    *x = (float)atof(mapinfo.Eval("CommandInfo(1)"));
    *y = (float)atof(mapinfo.Eval("CommandInfo(2)"));
  }

  return(bResult);
}

} // extern "C"