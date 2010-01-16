//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2008 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT78() - Bus Condition Report
//
//  
#include "TMSHeader.h"

BOOL FAR TMSRPT78(TMSRPTPassedDataDef *pPassedData)
{
  HFILE hfOutputFile;
  BOOL  bKeepGoing;
 
  pPassedData->nReportNumber = 78;
  pPassedData->numDataFiles = 1;
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt79.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    _lclose(hfOutputFile);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}



