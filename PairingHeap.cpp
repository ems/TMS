//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "PairingHeap.h"

MMinit( PairNode );

void PairingHeapTreeArrayResize( PairNode **&treeArray, int &treeArraySize )
{
	register int	treeArraySizeOld = treeArraySize;

	if( treeArraySize == 0 )
		treeArraySize = 64;
	else
		treeArraySize <<= 1;

	PairNode **treeArrayNew = new PairNode * [treeArraySize];
	for( register int i = 0; i < treeArraySizeOld; i++ )
		treeArrayNew[i] = treeArray[i];
	delete [] treeArray;
	treeArray = treeArrayNew;
}

