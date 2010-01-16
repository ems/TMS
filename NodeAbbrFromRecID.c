//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"


int compare(void *a, void *b)
{
  NODEABBRSDef *pa = (NODEABBRSDef *)a;
  NODEABBRSDef *pb = (NODEABBRSDef *)b;
  
  return pa->recordID < pb->recordID ? -1	: pa->recordID > pb->recordID ? 1 : 0;
}


char *NodeAbbrFromRecID(long recordID)
{
  static char szAbbr[NODES_ABBRNAME_LENGTH + 1];
  NODEABBRSDef *pResult;  
  
  if(recordID == NO_RECORD)
    return("");

  pResult = (NODEABBRSDef *)bsearch(&recordID, m_pNodeAbbrs, m_numNodeAbbrs,
        sizeof(NODEABBRSDef), (int (*)(const void*, const void*))compare);
  
  return pResult->abbrName;
}

