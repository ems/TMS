//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef rundef_H
#define rundef_H

#include <tod.h>
#include <iostream>
#include "mm.h"

using namespace std;

#ifndef UNIX
#define srand48(s)      srand((int)s)
#define drand48()       ((double)rand() / ((double)(RAND_MAX) + 1.0))
#endif

class RunComponent
{
public:
    RunComponent( int aFWork, tod_t aLenMin, tod_t aLenMax, tod_t aLenDes );
	MMdeclare();

    void addToList( RunComponent *&head, RunComponent *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }

    // a flag indicating whether this is work or break
    int		workCount;

    tod_t		lenMin, lenMax;	// Min and max length of this component.
    tod_t      lenDes;         // Desired length of this component.
    RunComponent	*next;
};

//-------------------------------------------------------------------
class RunPattern;
class RunChunk
{
public:
    RunChunk( int aFWork, tod_t aStart, tod_t aLength );
    RunChunk( const RunChunk &rc );
	MMdeclare();

    void addToList( RunChunk *&head, RunChunk *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }
    int work() const { return (int)workCount; }

    int operator<( const RunChunk &rc ) const
    { return start != rc.start ? start < rc.start : length < rc.length; }
    int operator<=( const RunChunk &rc ) const
    { return start != rc.start ? start < rc.start : length <= rc.length; }
    int operator==( const RunChunk &rc ) const
    { return start == rc.start && length == rc.length; }
    int operator>=( const RunChunk &rc ) const
    { return start != rc.start ? start > rc.start : length >= rc.length; }
    int operator>( const RunChunk &rc ) const
    { return start != rc.start ? start > rc.start : length > rc.length; }
    int operator!=( const RunChunk &rc ) const
    { return start != rc.start || length != rc.length; }

    // a flag indicating whether this is work or break
    // later, used as a count of how many of these chunks are required.
    double 	workCount;

    tod_t	start, length;
    RunChunk	*next;
    RunPattern	*owner;		// a pointer to the head of this pattern
};
ostream &operator<<( ostream &os, const RunChunk &rc );

class RunDef;
class RunPattern
{
friend class RunDef;
public:
    RunPattern();
    ~RunPattern();
	MMdeclare();
    void add( int workCount, tod_t start, tod_t length )
    {
	RunChunk *rc = new RunChunk( workCount, start, length );
	rc->addToList( head, tail );
    }
    void add( RunChunk &rc )
    {
	rc.addToList( head, tail );
    }
    RunChunk *firstChunk() const { return head; }
    void addToList( RunPattern *&head, RunPattern *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }
    RunPattern	*getNext() const { return next; }
    RunChunk	*head, *tail;
private:
    RunPattern	*next;

public:
    double	workCount;		// used by the lp solver
    size_t	id;
    RunDef	*owner;
};
ostream &operator<<( ostream &os, const RunPattern &rp );

//-------------------------------------------------------------------
class RunDef
{
public:
    RunDef( tod_t aSpreadMax = 12L*60L*60L,
	    tod_t aWorkMin = 0L,
	    tod_t aWorkMax = 0L,
	    tod_t aMustStartBefore = 0L,
	    tod_t aMustStartAfter = 0L,
	    tod_t aMustEndBefore = 0L,
	    tod_t aMustEndAfter = 0L);
    ~RunDef();
	MMdeclare();
    void addWork( tod_t lenMin, tod_t lenMax, tod_t lenDes )
    {
	RunComponent *rc = new RunComponent( 1, lenMin, lenMax, lenDes );
	rc->addToList( cHead, cTail );
	numPieces++;
    }
    void addBreak( tod_t lenMin, tod_t lenMax, tod_t lenDes )
    {
	RunComponent *rc = new RunComponent( 0, lenMin, lenMax, lenDes );
	rc->addToList( cHead, cTail );
    }
    size_t	generatePatterns( const tod_t aStart,	// earliest start work time
				  const tod_t aEnd,	// latest end work time
				  const tod_t multiple);// full day time multiple
    size_t	countPatterns( const tod_t aStart,	// earliest start work time
			       const tod_t aEnd,	// latest end work time
			       const tod_t multiple);// full day time multiple
    RunComponent *firstComponent() { return cHead; }
    RunPattern *firstPattern() { return pHead; }
    size_t	getNumPatterns() const { return numPatterns; }
    void addToList( RunDef *&head, RunDef *&tail )
    {
	if( head == NULL )
	    head = this;
	else
	    tail->next = this;
	tail = this;
	next = NULL;
    }
    // getNumWorking gets the number of working components assuming alternating
    // breaks.
    size_t	getNumWorking() const { return numPieces / 2 + 1; }
    
    //-----------------------------------------------------------------
    size_t	genPatterns( const tod_t aStart,	// earliest start work time
			     const tod_t aEnd,		// latest end work time
			     const tod_t multiple);	// full day time multiple
    void	genPattern( const RunComponent *rc, const tod_t s );

    // Run constraints.
    tod_t		spreadMax;
    tod_t		mustStartBefore, mustEndAfter;
    tod_t		mustStartAfter,  mustEndBefore;
    tod_t		workMin, workMax;

    RunComponent	*cHead, *cTail;
    RunPattern		*pHead, *pTail;
    size_t		numPatterns;
    size_t		numPieces;
    int			isPenalty; // if true, this is only a penalty rundef.
    RunDef		*next;

    long                runType;    // Used by TMS.

    static int		countOnly;

    static tod_t	multiple, start, end; // current data for pattern
    static tod_t	startOrig;    	// current start time of this pattern

    static int		level;
    enum { pieceMax = 16, chunkMax = pieceMax*2 }; // the largest number of pieces (including breaks)
    static RunChunk	*chunks[chunkMax];
};
ostream &operator<<( ostream &os, const RunDef &rd );

//-----------------------------------------------------------------------------

inline tod_t nearestMult( tod_t t, const tod_t m )
{
    t += m / 2;
    return t - (t % m);
}

inline tod_t lesserMult( tod_t t, const tod_t m )
{
    return t - (t % m);
}

inline tod_t greaterMult( tod_t t, const tod_t m )
{
    t += m - 1;
    return t - (t % m);
}

#if !defined(__WIN32__)
inline tod_t Min( const tod_t a, const tod_t b )
{
    return a < b ? a : b;
}

inline tod_t Max( const tod_t a, const tod_t b )
{
    return a > b ? a : b;
}
#endif

const char *asTime( tod_t s );

#endif // rundef_H
