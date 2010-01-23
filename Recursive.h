//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef Recursive_H
#define Recursive_H

#include "rbtreemap.h"
#include "rbtreeset.h"
#include "HashFI.h"
#include "HashObjFI.h"
#include <limits.h>

extern "C"
{
#include "TMSHeader.h"
} // extern "C"

#include <string>
#include "RefCount.h"
#include "mm.h"

using namespace std;

extern const long InfiniteCost;

#define Number(a)	(sizeof(a) / sizeof((a)[0]))

#ifndef ForAllInCollection
#define ForAllInCollection( c, i ) \
	for( i = c.begin(); !(i == c.end()); ++i )
#endif // ForAllInCollection

class	Reverse
{
public:
	static	void	setBackward() { reverse = true; }
	static	void	setForward() { reverse = false; }

	static	void	flip() { reverse = (reverse ? false : true); }

	static	bool	reverse;
};

inline	tod_t	AdjustTime( const tod_t t )
{ return Reverse::reverse ? 48 * 60 * 60 - t : t; }

inline tod_t maxT( const tod_t a, const tod_t b ) { return a > b ? a : b; }
inline tod_t minT( const tod_t a, const tod_t b ) { return a < b ? a : b; }

class RPiece;
class RRun;
class RPiecePair;
class RPieceTriple;
class RecursiveCut;

class RPiece
{
	friend class RecursiveCut;
public:
	RPiece( int start, int end ) { init(start, end); }
	RPiece( const RPiece &p ) { init(p.iStart, p.iEnd); }
	MMdeclare();

	tod_t	startTime() const { return (*this)[0]; }
	tod_t	endTime() const { return (*this)[iMost()]; }

	tod_t	actualStartTime() const { return m_pRELIEFPOINTS[iStart].time; }
	tod_t	actualEndTime() const { return m_pRELIEFPOINTS[iEnd].time; }
	long	getFromNodesRecordID() const { return m_pRELIEFPOINTS[iStart].NODESrecordID; }	
	long	getToNodesRecordID() const { return m_pRELIEFPOINTS[iEnd].NODESrecordID; }	
	long	getFromTripsRecordID() const { return m_pRELIEFPOINTS[iStart].TRIPSrecordID; }	
	long	getToTripsRecordID() const { return m_pRELIEFPOINTS[iEnd].TRIPSrecordID; }

	long	blockNumber() const { return m_pRELIEFPOINTS[iStart].blockNumber; }

	long	serviceRecordID() const { return m_pRELIEFPOINTS[iStart].SGRPSERVICESrecordID; }

	long	firstNodeRecordID() const { return m_pRELIEFPOINTS[getI(0)].NODESrecordID; }
	long	lastNodeRecordID() const { return m_pRELIEFPOINTS[getI(iMost())].NODESrecordID; }

	tod_t	length() const { return endTime() - startTime(); }
	tod_t	spread( const RPiece &p ) const	{ return maxT(endTime(), p.endTime()) - minT(startTime(), p.startTime()); }
	tod_t	getBreak( const RPiece &p ) const
	{
		RPiece	p1(*this), p2(p);
		if( p1.startTime() > p2.startTime() ) { RPiece t(p1); p1 = p2; p2 = t; }
		tod_t	br = p2.startTime() - p1.endTime();
		return br > 0 ? br : 0;
	}

	RPiecePair	*forwardSplit( const tod_t minSize, const tod_t desSize, const tod_t maxSize, const tod_t minLeftover );
	RPiecePair	*forwardSplitLeftover( const tod_t minSize, const tod_t minLeftover );
	RPiecePair	*backwardSplit( const tod_t minSize, const tod_t desSize, const tod_t maxSize, const tod_t minLeftover )
	{
		Reverse::flip();
		RPiecePair	*pp = forwardSplit( minSize, desSize, maxSize, minLeftover );
		Reverse::flip();
		return pp;
	}
	RPiecePair	*backwardSplitLeftover( const tod_t minSize, const tod_t minLeftover )
	{
		Reverse::flip();
		RPiecePair	*pp = forwardSplitLeftover( minSize, minLeftover );
		Reverse::flip();
		return pp;
	}
	RPiecePair	*split( const int i );

	RPiecePair	*removePiece( const RPiece &p ) const;

	bool	intersectsInterval( const RPiece &p ) const
	{
		const	RPiece	&p1 = (*this)[0] < p[0] ? (*this) : p;
		const	RPiece	&p2 = (*this)[0] < p[0] ? p : (*this);
		return p1[p1.iMost()] >= p2[0];
	}
	bool intersects( const RPiece &p ) const
	{
		const	RPiece	&p1 = (*this)[0] < p[0] ? (*this) : p;
		const	RPiece	&p2 = (*this)[0] < p[0] ? p : (*this);
		return blockNumber() == p.blockNumber() && p1[p1.iMost()] > p2[0];
	}

	bool	isAdjacentTo( const RPiece &p ) const
	{ return iStart == p.iEnd || iEnd == p.iStart; }
	bool	mergePiece( const RPiece &p )
	{
		bool	status = true;
		if( iStart == p.iEnd )
			iStart = p.iStart;
		else if( iEnd == p.iStart )
			iEnd = p.iEnd;
		else
			status = false;
		return status;
	}

	RPiece	*advance( const int i ) const
	{
		return i >= iMost() ? NULL : new RPiece(getI(i), getI(iMost()));
	}
	RPiece	*retreat( const int i ) const
	{
		const int iAdjust = getI(-i);
		// Check that the relief point we backed up to is valid.
		return i >= 0 && i < m_numRELIEFPOINTS &&
			m_pRELIEFPOINTS[iAdjust].blockNumber == m_pRELIEFPOINTS[getI(0)].blockNumber &&
			(m_pRELIEFPOINTS[iAdjust].flags & (RPFLAGS_FIRSTINBLOCK | RPFLAGS_LASTINBLOCK))
				? new RPiece(iAdjust, getI(iMost())) : NULL;
	}

	void	install( const long runNumber, const long pieceNumber ) const;
	void	installCheck( const long runNumber, const long pieceNumber ) const;

	tod_t	length( const int i, const int j ) const
	{
		int	ii = getI(i), jj = getI(j);
		if( ii > jj ) { int t = ii; ii = jj; jj = t; }
		return m_pRELIEFPOINTS[jj].time - m_pRELIEFPOINTS[ii].time;
	}
	int		iMost() const { return iEnd - iStart; }

	bool	operator==( const RPiece &p ) const	{ return iStart == p.iStart && iEnd == p.iEnd; }
	bool	operator!=( const RPiece &p ) const	{ return iStart != p.iStart || iEnd != p.iEnd; }
	bool	operator<( const RPiece &p ) const	{ return iStart < p.iStart; }

	void	moveIStart( const int i = 1 )	{ iStart += i; }
	void	moveIEnd(  const int i = 1 )	{ iEnd += i; }

	int		getIStart() const { return iStart; }
	int		getIEnd() const { return iEnd; }

protected:
	void init( int start, int end )
	{
		if( start > end ) { int	t = start; start = end; end = t;}
		iStart = start;
		iEnd = end;
	}
	int		getI( const int i ) const { return Reverse::reverse ? iEnd - i: i + iStart; }
	tod_t	operator[]( const int i ) const { return AdjustTime(m_pRELIEFPOINTS[getI(i)].time); }

	int	findClosestLE( tod_t t ) const;
	int	iStart, iEnd;
};

class RPiecePair
{
public:
	RPiecePair( Ref<RPiece> aFirst, Ref<RPiece> aSecond )
	{
		if( aFirst->startTime() > aSecond->startTime() )
		{ Ref<RPiece> t(aFirst); aFirst = aSecond; aSecond = t; }
		first = aFirst;
		second = aSecond;
	}
	RPiecePair( Ref<RPiece> aFirst = (RPiece *)NULL )
	{
		first = aFirst;
	}
	RPiecePair( const RPiecePair &pp ) { first = pp.first; second = pp.second; }
	RPiecePair &operator=( const RPiecePair &pp ) { first = pp.first; second = pp.second; return *this; }
	MMdeclare();

	Ref<RPiece>	first, second;
};

template <class T>
T	Square( const T x ) { return x * x; }

struct RPieceElement
{
	// Compute the cost as a least squares sum of deviation from the beginning of the block
	// and the desired piece size.
	RPieceElement( tod_t desStartTime, tod_t desPieceSize, Ref<RPiece> aPiece, int aType ) : piece(aPiece), type(aType)
	{
		cost = Square(piece->startTime() - desStartTime) + Square(piece->length() - desPieceSize);
	}
	RPieceElement( const RPieceElement &e ) : piece(e.piece), cost(e.cost), type(e.type) {}
	RPieceElement &operator=( const RPieceElement &e ) { piece = e.piece; cost = e.cost; type = e.type; }
	bool	operator<( const RPieceElement &e ) const
	{
		return
			  type < e.type ? true : e.type < type ? false
			: cost < e.cost ? true : e.cost < cost ? false
			: piece->startTime() < e.piece->startTime();
	}

	Ref<RPiece>	piece;
	int		type;
	double	cost;
};

struct CmpRPieceElement
{
	bool operator()( const RPieceElement &e1, const RPieceElement &e2 ) const
	{ return e1 < e2; }
};

typedef RBTreeSet< RPieceElement, CmpRPieceElement > RPieceCollection;

class RRun
{
public:
	RRun( const long aRunNumber = -1 ) { init(aRunNumber); }
	RRun( const RRun &r ) { init(); assign(r); }
	~RRun() { delete [] pieces; }
	RRun	&assign( const RRun &r );
	RRun	&operator=( const RRun &r ) { return assign(r); }
	MMdeclare();

	void	addPiece( Ref<RPiece> p );
	Ref<RPiece>	getPiece( const int i ) const { return pieces[i]; }
	Ref<RPiece> &operator[]( const int i ) { return pieces[i]; }
	const Ref<RPiece> operator[]( const int i ) const { return pieces[i]; }

	Ref<RPiece> getLastPiece() const { return numPieces > 1 ? pieces[numPieces-1] : Ref<RPiece>(NULL); }
	int	getNumPieces() const { return numPieces; }
	long getRunNumber() const { return runNumber; }

	bool	operator<( const RRun &r ) const
	{
		const	tod_t	t1 = pieces[0]->actualStartTime();
		const	tod_t	t2 = r.pieces[0]->actualStartTime();
		return t1 < t2 ? true : t1 > t2 ? false :
			pieces[0]->blockNumber() < r.pieces[0]->blockNumber();
	}

	void	install( const long aRunNumber );
	void	install() { install(runNumber); }
	void	installCheck() const;
	bool	intersects( const RRun &r ) const;

	enum CostBiasType { PayPlatformRatio, PayPlatformDifference, None };
	long	getCost( const CostBiasType bias = PayPlatformDifference ) const
	{
		// Return the known cost if we have one.
		return costCur > 0 ? costCur : const_cast<RRun *>(this)->getCostCall(bias);
	}

	void	setCost( const long aCostCur ) { costCur = aCostCur; }
	void	resetCost() { costCur = -1; }

	long	getCostedRunType( const CostBiasType bias = PayPlatformDifference ) const
	{
		if( costCur <= 0 )
			getCost(bias);
		return costedRunType;
	}

	bool	valid() const { return getCost() < InfiniteCost; }

	tod_t	getSpreadTime() const
	{ return pieces[numPieces-1]->actualEndTime() - pieces[0]->actualStartTime(); }

	tod_t	getBreakTimeBefore( const int i ) const
	{ return pieces[i]->actualStartTime() - pieces[i-1]->actualEndTime(); }

	tod_t	getBreakTimeAfter( const int i ) const
	{ return pieces[i+1]->actualStartTime() - pieces[i]->actualEndTime(); }

	int		getPieceNumber( const Ref<RPiece> &p ) const
	{
		register int i;
		for( i = 0; i < numPieces; ++i )
			if( *pieces[i] == *p )
				break;
		return i;
	}

	static	void	resetCostCount() { costCount = 0; }
	static	size_t	getCostCount() { return costCount; }
	static	void	incCostCount() { ++costCount; }

private:
	void	init( const long aRunNumber = -1 )
	{
		numPieces = 0;
		pieces = NULL;
		costCur = -1,
		costedRunType = UNCLASSIFIED_RUNTYPE;
		runNumber = aRunNumber;
	}

	long	runNumber;
	int		numPieces;
	Ref<RPiece>	*pieces;
	long	costCur;
	long	costedRunType;

	long	getCostCall( const CostBiasType bias = PayPlatformDifference );	// Get the cost by calling the run coster.

	static	size_t	costCount;
};

struct	CmpPRun
{
	bool operator()( const Ref<RRun> &r1, const Ref<RRun> &r2 ) const
	{ return (*r1) < (*r2); }
};

struct TimeBlock
{
	TimeBlock( const tod_t aT, const long aBlock ) : t(aT), block(aBlock) {}
	TimeBlock( const Ref<RPiece> &p ) : t(p->startTime()), block(p->blockNumber()) {}
	bool	operator<( const TimeBlock &tb ) const
	{ return t < tb.t ? true : tb.t < t ? false : block < tb.block; }
	tod_t	t;
	long	block;
};

struct CmpTimeBlock
{
	bool operator()( const TimeBlock &tb1, const TimeBlock &tb2 ) const
	{ return tb1 < tb2; }
};

typedef RBTreeMap< TimeBlock, Ref<RPiece>, CmpTimeBlock > Pieces;
typedef RBTreeSet< Ref<RRun>, CmpPRun > Runs;
typedef HashFI< long, long >	RunTypeHash;

struct SFrame;

// Provide defaults for unspecified values.
inline tod_t	provideDefault( const tod_t x, const tod_t d ) { return (x == NO_TIME || x < 0) ? d : x; }

class RecursiveCut
{
public:
	RecursiveCut() {}
	~RecursiveCut() {}

	void	runcut();

	enum CutFlags
	{
		Entire		= (1<<0),
		FromFront	= (1<<1),
		FromEnd		= (1<<2),
		FromMiddle	= (1<<3)
	};

	void	getPieceTimeRange( tod_t &earliest, tod_t &latest ) const;

private:
	void	initPieces();

	// Get the travel time from the end of one piece to the beginning of the next.
	tod_t	getTravelTime( tod_t &arriveTime, const RPiece &from, const RPiece &to ) const;

	// Cut possibilities for the first piece.
	void	createCutPossibilities( RPieceCollection &pc, Ref<RPiece> original, const int cutFlags );
	// Cut possibilities for a non-first piece.
	void	createCutPossibilities( RPieceCollection &pc,
									const tod_t start, // Start time of the run.
									Ref<RPiece> prev,	// Previous piece of the run.
									const tod_t minBreak, const tod_t desBreak, const tod_t maxBreak,
									Ref<RPiece> original, const int cutFlags );
	bool	cutTrippersComplete( Ref<RPiece> pOriginal, const int numToCut );
	bool	cutTrippersComplete( Ref<RPiece> pOriginal );
	void	recursiveRuncut();
	enum RunDoneStatus { InsufficientPieces, PiecesMeet, Valid, Invalid };
	RunDoneStatus checkRunDone();
	void	pairwiseSwapImproveTwoPieceRuns();
	void	improveTwoPieceRuns();
	RRun	*getNextPiece( Ref<RPiece> p );

	Pieces	pieces;
	Pieces::iterator	insertPiece( const Ref<RPiece> &p )
	{
		return p() && p->length() > 0 ? pieces.insert( Pieces::data_type(TimeBlock(p), p) ).first : pieces.end();
	}
	void	erasePiece( const Ref<RPiece> &p )
	{
		if( p() )
			pieces.erase( TimeBlock(p) );
	}
	Pieces::iterator	findPiece( const Ref<RPiece> &p )
	{ return pieces.find( TimeBlock(p) ); }
	Pieces::iterator	firstPieceGE( const tod_t t )
	{ return pieces.findEqSuccessor( TimeBlock(t, NULL) ); }

	void	checkRuns() const;

	Runs	runs;

	//----------------------------------------------------------------------
	long	getRunType() const { return runType; }
	tod_t	getStartRunBeforeTime() const { return startRunBeforeTime; }  // This is already adjusted when set.
	bool	getSmartLookahead() const { return CUTPARMS.flags & CUTPARMSFLAGS_SMART != 0; }
	tod_t	getMinLeftover() const { return provideDefault(CUTPARMS.minLeftover, 0); }
	tod_t	getMaxSpreadTime() const { return provideDefault(curRunType->maxSpreadTime, 49*60*60); }
	int		getNumPieces() const { return curRunType->numPieces; }
	bool	getMultiPieceCanBeOne() const { return fMultiPieceCanBeOne; }
	void	push( SFrame &sf, const bool fBreakOfTry = false );
	Pieces::iterator	pop();

	// Adjust references to piece indexes depending on the direction.
	int		adjustPieceIndex( const int i ) const
	{ return Reverse::reverse ? curRunType->numPieces - 1 - i : i; }

	tod_t	_getRawMinPieceSize( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].minPieceSize; }
	tod_t	_getRawDesPieceSize( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].desPieceSize; }
	tod_t	_getRawMaxPieceSize( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].maxPieceSize; }

	tod_t	_getRawMinOnTime( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].minOnTime; }
	tod_t	_getRawMaxOnTime( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].maxOnTime; }
	tod_t	_getRawMinOffTime( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].minOffTime; }
	tod_t	_getRawMaxOffTime( const int i ) const { return curRunType->PIECE[adjustPieceIndex(i)].maxOffTime; }

	// Breaks have to be adjusted differently in the reverse direction.  The amount for the next break is
	// stored on the current piece info.  This means we have to subtract one from the index calculation
	// in the reverse direction.
	int		adjustBreakPieceIndex( const int i ) const
	{ return Reverse::reverse ? curRunType->numPieces - 2 - i : i; }

	tod_t	_getRawMinBreakTime( const int i ) const { return curRunType->PIECE[adjustBreakPieceIndex(i)].minBreakTime; }
	tod_t	_getRawDesBreakTime( const int i ) const { return curRunType->PIECE[adjustBreakPieceIndex(i)].desBreakTime; }
	tod_t	_getRawMaxBreakTime( const int i ) const { return curRunType->PIECE[adjustBreakPieceIndex(i)].maxBreakTime; }

	tod_t	_getRawMinBreakOf() const { return curRunType->minBreakOf; }
	tod_t	_getRawDesBreakOf() const { return curRunType->desBreakOf; }
	tod_t	_getRawMaxBreakOf() const { return curRunType->maxBreakOf; }

	tod_t	_getRawMinContig() const { return curRunType->minContig; }
	tod_t	_getRawDesContig() const { return curRunType->desContig; }
	tod_t	_getRawMaxContig() const { return curRunType->maxContig; }

	// Convenience functions for dereferencing piece attributes.
	tod_t	getMinPieceSize( const int i ) const { return provideDefault(_getRawMinPieceSize(i), 1); }
	tod_t	getDesPieceSize( const int i ) const { return provideDefault(_getRawDesPieceSize(i), (getMinPieceSize(i) + getMaxPieceSize(i)) / 2); }
	tod_t	getMaxPieceSize( const int i ) const { return provideDefault(_getRawMaxPieceSize(i), 49*60*60); }

	tod_t	getMinBreakTime( const int i ) const { return provideDefault(_getRawMinBreakTime(i), 0); }
	tod_t	getDesBreakTime( const int i ) const { return provideDefault(_getRawDesBreakTime(i), (getMinBreakTime(i) + getMaxBreakTime(i)) / 2); }
	tod_t	getMaxBreakTime( const int i ) const { return provideDefault(_getRawMaxBreakTime(i), 49*60*60); }

	// This gets tricky - depending on the direction we are cutting, we need to swap and transform the min and max
	// On and Off times.
	tod_t	getMinOnTime( const int i ) const
	{
		return !Reverse::reverse
			? provideDefault(_getRawMinOnTime(i), -49*60*60)
			: AdjustTime(provideDefault(_getRawMaxOnTime(i), 49*60*60));
	}
	tod_t	getMaxOnTime( const int i ) const
	{
		return !Reverse::reverse
			? provideDefault(_getRawMaxOnTime(i), 49*60*60)
			: AdjustTime(provideDefault(_getRawMinOnTime(i), -49*60*60));
	}
	tod_t	getMinOffTime( const int i ) const
	{
		return !Reverse::reverse
			? provideDefault(_getRawMinOffTime(i), -49*60*60)
			: AdjustTime(provideDefault(_getRawMaxOffTime(i), 49*60*60));
	}
	tod_t	getMaxOffTime( const int i ) const
	{
		return !Reverse::reverse
			? provideDefault(_getRawMaxOffTime(i), 49*60*60)
			: AdjustTime(provideDefault(_getRawMinOffTime(i), -49*60*60));
	}

	tod_t	getMinBreakOf() const { return provideDefault(_getRawMinBreakOf(), 0); }
	tod_t	getDesBreakOf() const { return provideDefault(_getRawDesBreakOf(), (getMinBreakOf() + getMaxBreakOf()) / 2); }
	tod_t	getMaxBreakOf() const { return provideDefault(_getRawMinBreakOf(), 0); }

	tod_t	getMinContig() const { return provideDefault(_getRawMinContig(), 0); }
	tod_t	getDesContig() const { return provideDefault(_getRawDesContig(), (getMinContig() + getMaxContig()) / 2); }
	tod_t	getMaxContig() const { return provideDefault(_getRawMaxContig(), 0); }

	bool	hasBreakOf() const { return curRunType->numPieces > 2 && provideDefault(curRunType->minBreakOf, 0); }

	tod_t	getDesTotal() const { return desTotal;}

	tod_t	getMinPlatformTime() const { return provideDefault(curRunType->minPlatformTime, 0); }
	tod_t	getMaxPlatformTime() const { return provideDefault(curRunType->maxPlatformTime, 49*60*60); }

	bool	fMultiPieceCanBeOne;
	tod_t	startRunBeforeTime;
	tod_t	desTotal;

	bool	fBreakOfHasOccurred;

	long	runType;
	RUNTYPEDef	*curRunType;

	RunTypeHash	runTypeHash;
};

#endif // Recursive_H