//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define LABS( x )	((x) < 0L ? -(x) : (x))

static	int	RosterCannotCombine( register RRDATADef *r1, register RRDATADef *r2 )
{
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
//
//  Hey, if they're the same, it's perfect!
//
  if(r1 == r2)
    return 0;
//
//  Don't combine unclassified runtypes
//
  if(r1->runtype == UNCLASSIFIED_RUNTYPE || r2->runtype == UNCLASSIFIED_RUNTYPE)
    return 1;
//
//  Go through and see if there's a valid match
//
//  Route
//
  if((ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEROUTE) &&
        r1->RGRPROUTESrecordID != r2->RGRPROUTESrecordID)
  return 1;
//
//  Run Number
//
  if((ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMERUNNUMBER) &&
        r1->runNumber != r2->runNumber)
    return 1;
//
//  Same start location
//
  if((ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMESTART) &&
        r1->onNODESrecordID != r2->onNODESrecordID)
    return 1;
//
//  Same end location
//
  if((ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEEND) &&
        r1->offNODESrecordID != r2->offNODESrecordID)
    return 1;
//
//  Runtype choices
//
//  0 - Must be the same
//
  if(ROSTERPARMS.runtypeChoice == 0)
  {
    if(r1->runtype != r2->runtype)
      return 1;
  }
//
//  1 - Can differ
//
  else if(ROSTERPARMS.runtypeChoice == 1)
  {
  }
//
//  2 - Can combine to other specific runtypes
//
  else
  {
    nI = (short int)LOWORD(r1->runtype);
    nJ = (short int)HIWORD(r1->runtype);
    nK = (short int)LOWORD(r2->runtype);
    nL = (short int)HIWORD(r2->runtype);
    if(!(ROSTERPARMS.match[nI][nJ][nL] & (2 << nK)))
      return 1;
  }

  return 0;
}

#define nePenalty	60L
#define CheckEqual( field, factor ) if(r1->field != r2->field) cost += nePenalty * factor

static long AdditionalPenalties( register RRDATADef *r1, register RRDATADef *r2 )
{
	// Just some quick external penalties that really will only make a difference,
	// if all other things are equal.
	// This helps to maintain stability as much as possible.
	// I just made up some relative penalty factors - this could be controlled from some
	// type of "equalizer" interface.
	register long	cost = 0;

	CheckEqual( runtype,			1 );
	CheckEqual( RGRPROUTESrecordID, 2 );
	CheckEqual( onNODESrecordID,	4 );
	CheckEqual( offNODESrecordID,	4 );

	return cost;
}

int RosterCosterCanCombine(int firstIndex, int firstService, int secondIndex, int secondService)
{
  register RRDATADef	*r1 = &m_pRRLIST[firstService].pData[firstIndex];
  register RRDATADef	*r2 = &m_pRRLIST[secondService].pData[secondIndex];

  return !RosterCannotCombine(r1, r2);
}

 // This is called when there is one or more holidays between work days.
long RosterCosterNonConsecutive(int firstIndex, int firstService, int secondIndex, int secondService)
{
  register RRDATADef	*r1 = &m_pRRLIST[firstService].pData[firstIndex];
  register RRDATADef	*r2 = &m_pRRLIST[secondService].pData[secondIndex];
  register long cost;

  if( RosterCannotCombine(r1, r2) )
	  return NO_RECORD;

  // Penalize for differences in onTime.
  cost = LABS(r1->onTime - r2->onTime);
  return cost + AdditionalPenalties(r1, r2);
}

// This is called for consecutive work days.
long RosterCoster(int firstIndex, int firstService, int secondIndex, int secondService)
{
  register RRDATADef	*r1 = &m_pRRLIST[firstService].pData[firstIndex];
  register RRDATADef	*r2 = &m_pRRLIST[secondService].pData[secondIndex];
  register long cost;

  if( RosterCannotCombine(r1, r2) )
	  return NO_RECORD;
//
//  Minimum Off Time
//
  if(86400L + r2->onTime - r1->offTime < ROSTERPARMS.minOffTime)
    return NO_RECORD;

  // Check if this is a night run and the following day must be a day off.
  if( r1->offTime > ROSTERPARMS.nightWorkAfter && (ROSTERPARMS.flags & ROSTERPARMS_FLAG_OFFDAYMUSTFOLLOW) != 0 )
	  return NO_RECORD;

  // Penalize for differences in onTime.
  cost = LABS(r1->onTime - r2->onTime);
//
//  Check Max variance
//
  if( ROSTERPARMS.startVariance > 0 && cost > ROSTERPARMS.startVariance / 2 )
	  return NO_RECORD;

   // Penalize for starting earlier the next day.
  if( r1->onTime > r2->onTime )
	  cost += (r1->onTime - r2->onTime) / 4;

  return cost + AdditionalPenalties(r1, r2);
}