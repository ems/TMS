//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#pragma warning (disable: 4786)

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "tms.h"

#include <math.h>
#include "MainFrm.h"

void CTMSApp::DisplayPatternRow(CF1Book *F1, TableDisplayDef *pTD, PATTERNSDef *PAT, int row, int *col)
{
  char    outString[16];
  long    recordID;
  long    patternNameRecordID;
  int     nI;
  int     nJ;
  int     rcode2 = 0;

  recordID = PAT->recordID;
  patternNameRecordID = PAT->PATTERNNAMESrecordID;
  nI = *col;
  while(rcode2 == 0 && nI < pTD->fieldInfo.numFields &&
        PAT->ROUTESrecordID == pTD->fileInfo.routeRecordID &&
        PAT->SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
        PAT->PATTERNNAMESrecordID == patternNameRecordID &&
        PAT->directionIndex == pTD->fileInfo.directionIndex)
  {
    if(!(PAT->flags & PATTERNS_FLAG_BUSSTOP))
    {
      for(nJ = nI; nJ < pTD->fieldInfo.cNumPatternNodes + nI; nJ++)
      {
        if(pTD->fieldInfo.basePatternSeq[nJ] == PAT->nodeSequence)
        {
          if(PAT->flags & PATTERNS_FLAG_MLP)
            strcpy(outString, "Max");
          else
            strcpy(outString, " * ");
          F1->SetTextRC(row + 1, nJ + 1, outString);
          break;
        }
      }
      nI++;
      recordID = PAT->recordID;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, PAT, &PATTERNSKey2, 2);
  }
  *col = nI;
  PAT->recordID = recordID;

  return;
}
