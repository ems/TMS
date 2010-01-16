//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  GetLLEN.c
//
//  Returns: Latitude / Longitude / Easting / Northing
//           based on passed argument
//
#include "TMSHeader.h"

#define LOADNODE_VALUE 1000

float GetLLEN(float inputValue, char *returnString)
{
  union
  {
    char String[4];
    long Integer;
  } LLEN;

  memcpy(&LLEN.String, &inputValue, 4);
  if(m_bUseLatLong)
  {
    sprintf(returnString, "%12.6f", (float)LLEN.Integer / 10000);
    return((float)LLEN.Integer/10000);
  }
  else
  {
    sprintf(returnString, "%9.3f", (float)LLEN.Integer / LOADNODE_VALUE);
    return((float)LLEN.Integer / LOADNODE_VALUE);  // Note: Correspond to LoadNode.mb
  }
}