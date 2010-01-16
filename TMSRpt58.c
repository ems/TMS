//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT58() - Place holder to Discipline Letter
//

BOOL FAR TMSRPT58(TMSRPTPassedDataDef *pPassedData)
{
  MessageBox(hWndMain, "Please run this report from within the Discipline Module", TMS, MB_OK);
//
//  All done
//
  return(FALSE);
}
