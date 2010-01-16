//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void TMSError(HWND hWnd, UINT fuStyle, UINT idResource, HANDLE hCtlFocus)
{
  char szErrorString[256];

  MessageBeep(fuStyle);
  LoadString(hInst, idResource, szErrorString, sizeof(szErrorString));
  MessageBox(hWnd, szErrorString, TMS, fuStyle);
  if(hCtlFocus != (HANDLE)NULL)
    SetFocus(hCtlFocus);
}

void NoConnectionErrorMessage(long fromNODESrecordID, long toNODESrecordID)
{
  char fromNodeName[NODES_ABBRNAME_LENGTH + 1];
  char toNodeName[NODES_ABBRNAME_LENGTH + 1];

//
//  From node
//
  NODESKey0.recordID = fromNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(fromNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(fromNodeName, NODES_ABBRNAME_LENGTH);
//
//  To node
//
  NODESKey0.recordID = toNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(toNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(toNodeName, NODES_ABBRNAME_LENGTH);
//
//  Error message
//
  LoadString(hInst, ERROR_037, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(szarString, szFormatString, fromNodeName, toNodeName);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, szarString, TMS, MB_ICONINFORMATION);

  return;
}
