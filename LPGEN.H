//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef LPGen_H
#define LPGen_H

#include <rundef.H>
#include <rblock.H>
#include <limits.h>

#ifdef UNIX
const double INFINITY = DBL_MAX / 1000000.0;
#else
const double INFINITY = (double)LONG_MAX*0.8;
#endif

class aggPiece
{
friend class LPGen;
friend ostream &operator<<( ostream &os, const aggPiece &a );
    aggPiece();
    aggPiece( ReliefPoint *rp );
    aggPiece( const aggPiece &p1, const aggPiece &p2 );

    void addRelief( ReliefPoint *rp )
    { reliefs[numReliefs++] = rp; }
    ReliefPoint *getRelief( const int i ) const { return reliefs[i]; }
    void append( const aggPiece &a );
    ReliefPoint	*from() const { return reliefs[0]->pred; }
    ReliefPoint	*to() const { return reliefs[numReliefs-1]; }
    tod_t length() const { return to()->getT() - from()->getT(); }
    void addToList( aggPiece *&head )
    {
	if( head != NULL )
	    head->prev = this;
	next = head;
	head = this;
	prev = NULL;
    }
    void removeFromList( aggPiece *&head )
    {
	if( prev != NULL )
	    prev->next = next;
	else
	    head = next;
	if( next != NULL )
	    next->prev = prev;
    }
    void markAllDone();
    void normalize();		// fix up any spurious run references.

    ReliefPoint	*reliefs[6];
    int		numReliefs;
    int		breakPoint;

    int		done;
    aggPiece	*prev, *next;

    void init()
    {
	numReliefs = 0;
	done = 0;
	breakPoint = -1;
	prev = next = NULL;
    }
    ostream	&printRun( ostream &os, const long r );
    void        installRun( const LPGen &lpg ) const;
};

typedef int (LPGen::*MatchCostFunc)( long &cost,
				     ReliefPoint *from1, ReliefPoint *to1,
				     ReliefPoint *from2, ReliefPoint *to2 );

class LPGen
{
friend class aggPiece;
public:
    LPGen();
    ~LPGen();
    void addBlock(  RBlock *rb ) { rb->addToList(rbHead, rbTail); }
    void addRunDef( RunDef *rd ) { rd->addToList(rdHead, rdTail); }

    void setMaxDrivers( const double aDrivers ) { maxDrivers = aDrivers; }
    void setMinDrivers( const double aDrivers ) { minDrivers = aDrivers; }
    void solveLP( tod_t aStart, tod_t aEnd,
		  const tod_t aMinLeftoverPiece, const tod_t aMultiple );
    long getNumPatterns( tod_t aStart, tod_t aEnd, const tod_t aMultiple );
    double getIdealRuncutCost() const { return idealRuncutCost; }
    void	cutBlocks();
    void	cutRuns();

    double getCost( tod_t start, tod_t length ) const;
    void decCount( RunChunk *&i, tod_t start, tod_t length );
    void incCount(  RunChunk *i ) { if(i != NULL) i->workCount += 1.0; }

private:
    tod_t start, end;  // horizon of all the pieces of work
    tod_t multiple;
    tod_t minLeftoverPiece;
    tod_t minLeftoverPieceSave;
    RunChunk *find( tod_t start, tod_t length ) const;
    int isPotentialTwoPiece( ReliefPoint *from, ReliefPoint *to );
    int	isValidOnePiece( ReliefPoint *from, ReliefPoint *to );
    int isValidRun( const aggPiece &a );

    LPGen::matchAgg( aggPiece *&toDo, MatchCostFunc mcf );
    int getPairCost( long &cost,
		     ReliefPoint *from1, ReliefPoint *to1,
		     ReliefPoint *from2, ReliefPoint *to2 );
    int getShortPairCost( long &cost,
			  ReliefPoint *from1, ReliefPoint *to1,
			  ReliefPoint *from2, ReliefPoint *to2 );
    int getSlagMatchCost( long &cost,
			  ReliefPoint *from1, ReliefPoint *to1,
			  ReliefPoint *from2, ReliefPoint *to2 );

    RBlock	*rbHead, *rbTail; // list of blocks to cut
    RunDef	*rdHead, *rdTail; // list of run definitions

//    RunPattern	*rpHead, *rpTail; // list of used ideal run patterns

    RunChunk	***chunkIndex;    // array of chunks in each bucket.
#if !defined(__WIN32__)
    long	huge *chunkElements; // the number of chunk types in each bucket.
#else
    long *chunkElements;
#endif
    long	chunkIndexSize;	// size of the chunk index.

    double	minDrivers;	// minimum drivers that must be used.
    double	maxDrivers;	// number of drivers available.

    // the shortest and longest chunk durations
    // (this improves search performance).
    tod_t	tShortest, tLongest;

    double	idealRuncutCost;
};

#endif // LPGen_H
