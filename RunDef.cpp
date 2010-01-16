//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <rundef.H>
#include <stdlib.h>
#include <string>

using namespace std;

#ifndef Min
#define Min(a,b)        (((a)<(b))?(a):(b))
#endif
#ifndef Max
#define Max(a,b)        (((a)>(b))?(a):(b))
#endif

// Initialize the static members.
int RunDef::countOnly = 0;
tod_t RunDef::multiple = (tod_t)0;
tod_t RunDef::start = (tod_t)0;
tod_t RunDef::end = (tod_t)0;
tod_t RunDef::startOrig = (tod_t)0;
int RunDef::level = 0;
RunChunk *RunDef::chunks[RunDef::chunkMax] =
{
NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL
};

MMinit( RunComponent );
MMinitMax( RunPattern, 1024 );
MMinitMax( RunChunk, 4096 );
MMinit( RunDef );

RunComponent::RunComponent( int aFWork, tod_t aLenMin, tod_t aLenMax,
                                tod_t aLenDes )
{
    workCount = aFWork;
    lenMin = aLenMin;
    lenMax = aLenMax;
    lenDes = aLenDes;
    next = NULL;
}

RunChunk::RunChunk( int aFWork, tod_t aStart, tod_t aLength )
{
    workCount = aFWork;
    start = aStart;
    length = aLength;
    next = NULL;
}

RunChunk::RunChunk( const RunChunk &rc )
{
    workCount = rc.workCount;
    start = rc.start;
    length = rc.length;
    next = NULL;
}

RunPattern::RunPattern()
{
    head = tail = NULL;
    next = NULL;
    workCount = 0;
    id = 0;
}

RunPattern::~RunPattern()
{
    // Delete all the chunks associated with this pattern.
    RunChunk	*rc, *rcNext;
    for( rc = head; rc != NULL; rc = rcNext )
    {
		rcNext = rc->next;
		delete rc;
    }
}

ostream &operator<<( ostream &os, const RunChunk &rc )
{
    os << '\t';
    os << asTime(rc.start) << '\t';
    os << asTime(rc.start + rc.length) << '\t';
    os << asTime(rc.length) << "\n";
    return os;
}

ostream &operator<<( ostream &os, const RunPattern &rp )
{
    RunChunk	*rc;
    for( rc = rp.head; rc != NULL; rc = rc->next )
		os << *rc;
    return os;
}
//------------------------------------------------------------

RunDef::RunDef( tod_t aSpreadMax,
		tod_t aWorkMin,
		tod_t aWorkMax,
		tod_t aMustStartBefore,
		tod_t aMustStartAfter,
		tod_t aMustEndBefore,
		tod_t aMustEndAfter )
{
    spreadMax = aSpreadMax;
    workMin = aWorkMin;
    workMax = aWorkMax;
    mustStartBefore = aMustStartBefore;
    mustStartAfter  = aMustStartAfter;
    mustEndBefore   = aMustEndBefore;
    mustEndAfter    = aMustEndAfter;

    cHead = cTail = NULL;
    pHead = pTail = NULL;
    next = NULL;
    numPieces = 0;
    isPenalty = 0;

    runType = 0;
}

RunDef::~RunDef()
{
    // Delete all the patterns generated from this definition.
    RunPattern	*rp, *rpNext;
    for( rp = firstPattern(); rp != NULL; rp = rpNext )
    {
		rpNext = rp->next;
		delete rp;
    }

    // Delete all the components associated with this run definition.
    RunComponent	*rc, *rcNext;
    for( rc = firstComponent(); rc != NULL; rc = rcNext )
    {
		rcNext = rc->next;
		delete rc;
    }

    // Clean up the static chunks array if as needed.
    for( int i = 0; chunks[i] != NULL; i++ )
    {
		delete chunks[i];
		chunks[i] = NULL;
    }
}

void RunDef::genPattern( const RunComponent *rc, const tod_t s )
{
    // Check if we are out of range.
    if( s > end ) return;

    // Check if we respect spread constraint.
    if( s - startOrig >= spreadMax ) return;

    // Check the starting constraints.
    if( level < 0 )
    {
		if( mustStartBefore && s > mustStartBefore ) return;
		if( mustStartAfter  && s < mustStartAfter ) return;
    }

    // Check the ending constraints.
    if( rc == NULL )
    {
		if( mustEndBefore && s >= mustEndBefore ) return;
		if( mustEndAfter  && s <  mustEndAfter )  return;
    }

    // Generate a pattern if we are at the last chunk.
    if( rc == NULL && level >= 0 )
    {
		// Check the total work constraints.
		tod_t workTotal = 0;
		int	i;
		for( i = 0; i <= level; i++ )
			if( chunks[i]->workCount > 0 )
				workTotal += chunks[i]->length;

		if( workMin && workTotal < workMin ) return;
		if( workMax && workTotal > workMax ) return;

		if( !countOnly )
		{
			// We are at the last component - generate a new pattern.
			// Ignore the break chunks as they are not necessary for the pattern.
			RunPattern	*rp = new RunPattern;
			rp->addToList( pHead, pTail );
			rp->owner = this;
			for( i = 0; i <= level; i++ )
			if( chunks[i]->workCount > 0 )
				rp->add( * (new RunChunk(*chunks[i])) );
		}
		numPatterns++;
		return;
    }

    ++level;

    if( chunks[level] == NULL )
		chunks[level] = new RunChunk( rc->workCount, 0, 0 );
    else
		chunks[level]->workCount = rc->workCount;
    
    tod_t	 t    = greaterMult( rc->lenMin, multiple );
    const tod_t tMax = lesserMult(  rc->lenMax, multiple );
    for( ; t <= tMax; t += multiple )
    {
		chunks[level]->start = s;
		chunks[level]->length = t;
		genPattern( rc->next, s + t );
    }

    --level;
}

size_t RunDef::genPatterns( const tod_t aStart,
			    const tod_t aEnd,
			    const tod_t aMultiple )
{
    multiple = aMultiple;

    // Round the start and end times to the nearest multiple.
    start = nearestMult( aStart, multiple );
    end   = nearestMult( aEnd,   multiple );

    // Delete any existing patterns.
    RunPattern	*rp, *rpNext;
    for( rp = firstPattern(); rp != NULL; rp = rpNext )
    {
		rpNext = rp->next;
		delete rp;
    }
    pHead = pTail = NULL;
    numPatterns = 0;

    // Check if we need to do anything.
    if( mustStartBefore != 0 && mustStartBefore < aStart )
		return 0;
    if( mustEndAfter    != 0 && aEnd < mustEndAfter )
		return 0;

    // Determine some tighter bounds on generating the patterns.
    RunComponent	*rc;
    tod_t		sumMin = 0;
    tod_t		sumMax = 0;
    for( rc = cHead; rc != NULL; rc = rc->next )
    {
		sumMin += rc->lenMin;
		sumMax += rc->lenMax;
    }
    sumMax = Min( spreadMax, sumMax );

    start = aStart;
    end   = aEnd;
    tod_t startMax = end - sumMin;

    if( mustEndAfter != 0 )
    {
		start = Max( aStart, mustEndAfter - sumMax );
    }
    if( mustStartBefore != 0 )
    {
		end = Min( aEnd, mustStartBefore + sumMax );
		startMax = mustStartBefore;
    }

    start = nearestMult( start, multiple );
    end   = nearestMult( end,   multiple );

    // For all starting times, attempt to generate a pattern.
    level = -1;
    for( startOrig = start; startOrig <= startMax; startOrig += multiple )
		genPattern( cHead, startOrig );
    return numPatterns;
}

size_t	RunDef::countPatterns( const tod_t aStart,
			       const tod_t aEnd,
			       const tod_t multiple)
{
    countOnly = 1;
    return genPatterns( aStart, aEnd, multiple );
}

size_t	RunDef::generatePatterns( const tod_t aStart,
				  const tod_t aEnd,
				  const tod_t multiple)
{
    countOnly = 0;
    return genPatterns( aStart, aEnd, multiple );
}

ostream &operator<<( ostream &os, const RunDef &rd )
{
    os << "Number of patterns: " << rd.numPatterns << '\n';

    RunPattern	*rp;
    for( rp = rd.pHead; rp != NULL; rp = rp->getNext() )
    {
		os << *rp;
		os << '\n';
    }
    return os;
}
