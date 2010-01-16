//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include "RefCount.h"

#ifdef MMenableMemoryManager
// Declare this to be the same layout and size as in RefCount.h, but without the template.
// This allows us to share the RefCount allocator for all type of references.
struct RefTemplate
{
	int		count;
	void	*data;
};
#	ifdef MMsharedAllocators
		MMallocator *refAllocator = MMallocatorFactory::getAllocator( sizeof(RefTemplate), 1024 );
#	else
		MMallocator *refAllocator = new MMallocator( sizeof(RefTemplate), 1024 );
#	endif
#endif // MMenableMemoryManager
