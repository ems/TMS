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

void CTMSApp::DisplayTripRow(CF1Book *F1, TableDisplayDef *pTD, TRIPSDef *TRIP, int row, int *col)
{
  GenerateTripDef GTResults;
//  COLORREF color = RGB(0, 255, 0);
  char    outString[16];
  long    nodeSeq[RECORDIDS_KEPT];
  long    previousTime;
  long    tempLong;
  int     MLP;
  int     nI;
  int     nJ;
  int     rcode2;
  int numNodes;
  int nK;
//
//  Get the font data
//
  COLORREF crColor;
  BOOL bBold;
  BOOL bItalic;
  BOOL bUnderline;
  BOOL bStrikeout;
  BOOL bOutline;
  BOOL bShadow;
  char szFontName[64];
  short int  nFontSize;
  CString s;
  BSTR bs = s.AllocSysString();

  F1->GetFont(&bs, &nFontSize, &bBold, &bItalic, &bUnderline, &bStrikeout, (long *)&crColor, &bOutline, &bShadow);
//
//  Generate the trip
//
  GenerateTrip(TRIP->ROUTESrecordID, TRIP->SERVICESrecordID,
        TRIP->directionIndex, TRIP->PATTERNNAMESrecordID,
        TRIP->timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the pattern nodes
//
  PATTERNSKey2.ROUTESrecordID = TRIP->ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = TRIP->SERVICESrecordID;
  PATTERNSKey2.directionIndex = TRIP->directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = TRIP->PATTERNNAMESrecordID;
  PATTERNSKey2.nodeSequence = 0;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  numNodes = 0;
  while(rcode2 == 0 &&
         PATTERNS.ROUTESrecordID == TRIP->ROUTESrecordID &&
         PATTERNS.SERVICESrecordID == TRIP->SERVICESrecordID &&
         PATTERNS.directionIndex == TRIP->directionIndex &&
         PATTERNS.PATTERNNAMESrecordID == TRIP->PATTERNNAMESrecordID)
  {
    if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
    {
      if(PATTERNS.flags & PATTERNS_FLAG_MLP)
        MLP = numNodes;
      nodeSeq[numNodes++] = PATTERNS.nodeSequence;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//
//  Move along the pattern
//
  nI = *col;
  nK = 0;
  previousTime = NO_TIME;
  for(nJ = nI; nJ < pTD->fieldInfo.cNumPatternNodes + nI; nJ++)
  {
    if(nK >= numNodes)
      break;
    if(pTD->fieldInfo.basePatternSeq[nJ] == nodeSeq[nK])
    {
      if(GTResults.tripTimes[nK] == NO_TIME)
        strcpy(outString, "");
      else
      {
        if(previousTime == NO_TIME || !m_bShowNodeTimes)
        {
          strcpy(outString, Tchar(GTResults.tripTimes[nK]));
        }
        else
        {
//          sprintf(outString, "%ld", (GTResults.tripTimes[nK] - previousTime) / 60);
          tempLong = GTResults.tripTimes[nK] - previousTime;
          sprintf(outString, "%d", tempLong / 60);
          if(tempLong % 60 != 0)
          {
            strcat(outString, "+");
          }
        }
        previousTime = GTResults.tripTimes[nK];
      }
      F1->SetTextRC(row + 1, nJ + 1, outString);
      if(nK == MLP)
      {
        F1->SetSelection(row + 1, nJ + 1, row + 1, nJ + 1);
        F1->SetFont(szFontName, -nFontSize,
              TRUE, TRUE, bUnderline, bStrikeout, crColor, bOutline, bShadow);
      }
      nK++;
    }
  }
  *col += pTD->fieldInfo.cNumPatternNodes - 1;
  return;
}

