//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

void FreeAll(int subPool)
{
    HGLOBAL hMemory;

    while( piFreeArea[subPool] != NULL )
    {
        hMemory = piFreeArea[subPool]->hMem;
        piFreeArea[subPool] = piFreeArea[subPool]->piPrevious;
        TMSGlobalFree( hMemory );
    }
}
