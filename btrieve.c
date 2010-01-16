//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

int btrieve(int opCode, int fileNumber, void *dataBuffer, void *keyBuffer, int keyNumber)
{
  int  rcode2;
  BTI_WORD dataBufferLength;

  dataBufferLength = (opCode == B_STAT ? sizeof(BSTAT) : recordLength[fileNumber]);
  rcode2 = BTRVID((BTI_WORD)opCode, positionBlock[fileNumber],
        dataBuffer, &dataBufferLength, keyBuffer, (BTI_SINT)keyNumber, (BTI_BUFFER_PTR)&clientID);

  return (rcode2);
}
