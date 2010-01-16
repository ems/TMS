//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "tstring.h"
#include <ctype.h>

#ifdef MMenableMemoryManager
MMallocator *TString::allocators[numAllocators];
#endif // MMenableMemoryManager

void	TString::trim()
{
	if( s == NULL || *s == 0 )
		return;
	char *pStart = s;
	char *pEnd = s + strlen(s) - 1;
	const char *pLast = pEnd;

	if( isspace(*pStart) || isspace(*pEnd) )
	{
		while( isspace(*pStart) && *pStart != 0 ) pStart++;
		while( isspace(*pEnd) && pEnd > pStart ) pEnd--;
		pEnd++;

		if( pEnd != pLast )
			*pEnd = 0;

		TString	tmp(pStart);

		if( pEnd != pLast )
			*pEnd = ' ';

		operator=( tmp );
	}
}