//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

char *GetDatabaseString(char *inputString, long inputLength)
{
  static char returnString[256];

  strncpy(returnString, inputString, inputLength);
  trim(returnString, inputLength);

  return (&returnString[0]);
}

void PutDatabaseString(char *destination, CString inputString, long inputLength)
{
  strncpy(destination, (LPCSTR)inputString, inputLength);
  pad(destination, inputLength);
}

