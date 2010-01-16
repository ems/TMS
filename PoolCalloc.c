//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void *PoolCalloc( int num, int size, int subPool )
{
    HGLOBAL  hMemory;
    POOLINFO *piPrev;
    int      sTotalSize;
    char     *pArea;

    sTotalSize = num * size;

    // Round to the nearest larger multiple of 4.

    sTotalSize += 3; sTotalSize -= sTotalSize & 3;
    if( piFreeArea[subPool] != NULL )
      if( piFreeArea[subPool]->sOffset + sTotalSize <= piFreeArea[subPool]->sLength )
      {
        pArea = &piFreeArea[subPool]->start[piFreeArea[subPool]->sOffset];
        piFreeArea[subPool]->sOffset += sTotalSize;
        return( pArea );
      }
    piPrev = piFreeArea[subPool];
    hMemory = GlobalAlloc( GPTR, POOLSIZE );
    if(hMemory == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      return NULL;
    }
    piFreeArea[subPool] = (POOLINFO *)hMemory;
    piFreeArea[subPool]->piPrevious = piPrev;
    piFreeArea[subPool]->sOffset = 0;
    piFreeArea[subPool]->sLength = POOLSIZE - sizeof(POOLINFO) + 1;
    piFreeArea[subPool]->hMem = hMemory;
    pArea = &piFreeArea[subPool]->start[0];
    piFreeArea[subPool]->sOffset += sTotalSize;
    return( pArea );
}
