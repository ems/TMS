//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void AllocationError(char *szFileName, int nLineNumber, BOOL bReAlloc)
{
  char tempString[256];
  int  nID = bReAlloc ? ERROR_268 : ERROR_267;

  LoadString(hInst, nID, szFormatString, sizeof(szFormatString));
  sprintf(tempString, szFormatString, szFileName, nLineNumber, GetLastError());
  MessageBeep(MB_ICONSTOP);
  MessageBox(NULL, tempString, TMS, MB_OK);

  return;
}

void AssertNotification(char *szFileName, int nLineNumber)
{
/*
  char tempString[256];

  sprintf(tempString, "Assert at line %d in %s", nLineNumber, szFileName);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK | MB_ICONINFORMATION);

  return;
*/
}