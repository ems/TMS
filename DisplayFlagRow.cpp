//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "tms.h"

#include <math.h>
#include "MainFrm.h"

void CTMSApp::DisplayFlagRow(CF1Book *F1, TableDisplayDef *pTD, AllFilesDef *ALLFILES, int row, int *col)
{
  char    outString[16];
  long    flags;
  long    toCheck[32];
  int     nI;
  int     nJ;
  int     nK;


  if(pTD == NULL)
    return;
  if(pTD->fileInfo.fileNumber == TMS_ROUTES)
  {
    flags = ALLFILES->ROUTES.flags;
    toCheck[0] = ROUTES_FLAG_EMPSHUTTLE;
    toCheck[1] = ROUTES_FLAG_SCHOOL;
  }
  else if(pTD->fileInfo.fileNumber == TMS_NODES)
  {
    flags = ALLFILES->NODES.flags;
    toCheck[0] = NODES_FLAG_GARAGE;
    toCheck[1] = NODES_FLAG_STOP;
    toCheck[2] = NODES_FLAG_SECURE;
  }
  else if(pTD->fileInfo.fileNumber == TMS_CONNECTIONS)
  {
    flags = ALLFILES->CONNECTIONS.flags;
    toCheck[0] = CONNECTIONS_FLAG_TWOWAY;
    toCheck[1] = CONNECTIONS_FLAG_RUNNINGTIME;
    toCheck[2] = CONNECTIONS_FLAG_TRAVELTIME;
    toCheck[3] = CONNECTIONS_FLAG_DEADHEADTIME;
    toCheck[4] = CONNECTIONS_FLAG_EQUIVALENT;
    toCheck[5] = CONNECTIONS_FLAG_AVLEQUIVALENT;
    toCheck[6] = CONNECTIONS_FLAG_STOPSTOP;

  }
  else if(pTD->fileInfo.fileNumber == TMS_BUSTYPES)
  {
    flags = ALLFILES->BUSTYPES.flags;
    toCheck[0] = BUSTYPES_FLAG_ACCESSIBLE;
    toCheck[1] = BUSTYPES_FLAG_CARRIER;
    toCheck[2] = BUSTYPES_FLAG_SURFACE;
    toCheck[3] = BUSTYPES_FLAG_RAIL;
  }
  else if(pTD->fileInfo.fileNumber == TMS_BUSES)
  {
    flags = ALLFILES->BUSES.flags;
    toCheck[0] = BUSES_FLAG_RETIRED;
  }
  else
  {
    return;
  }
//
//  Move along the flags  cNumFlagFields set in GetFlagNames()
//
  nI = *col;
  nK = 0;
  for(nJ = nI; nJ < pTD->fieldInfo.cNumFlagFields + nI; nJ++)
  {
    sprintf(outString, (flags & toCheck[nK] ? CONST_YES : CONST_NO));
    F1->SetTextRC(row + 1, nJ + 1, outString);
    nK++;
  }
  *col += pTD->fieldInfo.cNumFlagFields - 1;
  return;
}

