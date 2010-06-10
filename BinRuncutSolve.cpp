
extern "C"
{
#include "TMSHeader.h"
#include "rc.h"
}
#include "tod.h"
#include "CrossEntropyRuncut.h"
#include "HashObj.h"

using namespace BinPackRuncut;

struct RunTypeWrapper
{
	tod_t	provideDefault( const tod_t x, const tod_t d ) const { return (x == NO_TIME || x < 0) ? d : x; }

	long	runType;
	RUNTYPEDef	*curRunType;

	RunTypeWrapper( long aRunType, RUNTYPEDef *aCurRunType ) : runType(aRunType), curRunType(aCurRunType)
	{
	}

	long	getRunType() const { return runType; }
	tod_t	getStartRunBeforeTime() const { return provideDefault(CUTPARMS.startTime, 49*60*60); }
	bool	getSmartLookahead() const { return CUTPARMS.flags & CUTPARMSFLAGS_SMART != 0; }
	tod_t	getMinLeftover() const { return provideDefault(CUTPARMS.minLeftover, 0); }
	tod_t	getMaxSpreadTime() const { return provideDefault(curRunType->maxSpreadTime, 49*60*60); }
	int		getNumPieces() const { return curRunType->numPieces; }
	bool	getMultiPieceCanBeOne() const { return curRunType->numPieces == 1; }

	tod_t	_getRawMinPieceSize( const int i ) const { return curRunType->PIECE[i].minPieceSize; }
	tod_t	_getRawDesPieceSize( const int i ) const { return curRunType->PIECE[i].desPieceSize; }
	tod_t	_getRawMaxPieceSize( const int i ) const { return curRunType->PIECE[i].maxPieceSize; }

	tod_t	_getRawMinOnTime( const int i ) const { return curRunType->PIECE[i].minOnTime; }
	tod_t	_getRawMaxOnTime( const int i ) const { return curRunType->PIECE[i].maxOnTime; }
	tod_t	_getRawMinOffTime( const int i ) const { return curRunType->PIECE[i].minOffTime; }
	tod_t	_getRawMaxOffTime( const int i ) const { return curRunType->PIECE[i].maxOffTime; }

	tod_t	_getRawMinBreakTime( const int i ) const { return curRunType->PIECE[i].minBreakTime; }
	tod_t	_getRawDesBreakTime( const int i ) const { return curRunType->PIECE[i].desBreakTime; }
	tod_t	_getRawMaxBreakTime( const int i ) const { return curRunType->PIECE[i].maxBreakTime; }

	tod_t	_getRawMinBreakOf() const { return curRunType->minBreakOf; }
	tod_t	_getRawDesBreakOf() const { return curRunType->desBreakOf; }
	tod_t	_getRawMaxBreakOf() const { return curRunType->maxBreakOf; }

	tod_t	_getRawMinContig() const { return curRunType->minContig; }
	tod_t	_getRawDesContig() const { return curRunType->desContig; }
	tod_t	_getRawMaxContig() const { return curRunType->maxContig; }

	// Convenience functions for dereferencing piece attributes.
	tod_t	getMinOnTime( const int i ) const	{ return provideDefault(_getRawMinOnTime(i), -49*60*60); }
	tod_t	getMaxOnTime( const int i ) const	{ return provideDefault(_getRawMaxOnTime(i), 49*60*60); }
	tod_t	getMinOffTime( const int i ) const	{ return provideDefault(_getRawMinOffTime(i), -49*60*60); }
	tod_t	getMaxOffTime( const int i ) const	{ return provideDefault(_getRawMaxOffTime(i), 49*60*60); }

	tod_t	getMinPayTime() const { return provideDefault(curRunType->minPayTime, 8*60*60); }
	tod_t	getMaxPayTime() const { return provideDefault(curRunType->maxPayTime, 10*60*60); }

	tod_t	getMinPieceSize( const int i ) const { return provideDefault(_getRawMinPieceSize(i), getMinLeftover()); }
	tod_t	getDesPieceSize( const int i ) const { return provideDefault(_getRawDesPieceSize(i), (getMinPieceSize(i) + getMaxPieceSize(i)) / 2); }
	tod_t	getMaxPieceSize( const int i ) const { return provideDefault(_getRawMaxPieceSize(i), getMaxPayTime()); }

	tod_t	getMinBreakTime( const int i ) const { return provideDefault(_getRawMinBreakTime(i), 0); }
	tod_t	getDesBreakTime( const int i ) const { return provideDefault(_getRawDesBreakTime(i), (getMinBreakTime(i) + getMaxBreakTime(i)) / 2); }
	tod_t	getMaxBreakTime( const int i ) const { return provideDefault(_getRawMaxBreakTime(i), 49*60*60); }

	tod_t	getMinBreakOf() const { return provideDefault(_getRawMinBreakOf(), 0); }
	tod_t	getDesBreakOf() const { return provideDefault(_getRawDesBreakOf(), (getMinBreakOf() + getMaxBreakOf()) / 2); }
	tod_t	getMaxBreakOf() const { return provideDefault(_getRawMinBreakOf(), 0); }

	tod_t	getMinContig() const { return provideDefault(_getRawMinContig(), 0); }
	tod_t	getDesContig() const { return provideDefault(_getRawDesContig(), (getMinContig() + getMaxContig()) / 2); }
	tod_t	getMaxContig() const { return provideDefault(_getRawMaxContig(), 0); }

	bool	hasBreakOf() const { return curRunType->numPieces > 2 && provideDefault(curRunType->minBreakOf, 0); }

	tod_t	getMinPlatformTime() const { return provideDefault(curRunType->minPlatformTime, 0); }
	tod_t	getMaxPlatformTime() const { return provideDefault(curRunType->maxPlatformTime, 49*60*60); }
};

//typedef	BinPackRuncut::CrossEntropyRuncut	Runcutter;
typedef	BinPackRuncut::GraspRuncut	Runcutter;
typedef HashObj< BinPackRuncut::Run, long >	RunHT;
typedef std::vector<long>	RunTypes;
struct RuncutContext
{
	RuncutContext() : runcutter(NULL), lastRunCount(10000), lastRunCost(0.0), lastIteration(1000),
		callsCached(0), callsFeasible(0), callsInfeasible(0), lastCallsTotal(0) {}

	void	clear()
	{
		runTypes.clear();
		runCache.clear();
		runcutter = NULL;
	}

	RunTypes	runTypes;

	bool		isValidRuntype( const long runType ) const
	{ return std::binary_search(runTypes.begin(), runTypes.end(), runType); }

	RunHT		runCache;
	Runcutter	*runcutter;

	size_t		lastIteration;
	size_t		lastRunCount;
	double		lastRunCost;

	size_t		callsCached, callsFeasible, callsInfeasible;
	size_t		lastCallsTotal;
};

static	bool	getRunCost( void *data, BinPackRuncut::Run &r, BinPackRuncut::Runcut &runcut )
{
	RuncutContext	&rc = *((RuncutContext *)data);

	// Check the cache.
	// If we has seen this run before, it is infeasible is it has a cost of -1, otherwise feasible.
	RunHT::iterator h = rc.runCache.find(r);
	if( h != rc.runCache.end() )
	{
		++rc.callsCached;
		return h.data() != -1;
	}

	// Not in the cache.  Call the RunCoster (expensive).
	PROPOSEDRUNDef	pr;
	pr.numPieces = 2;
	for( size_t i = 0; i < 2; ++i )
	{
		const BinPackRuncut::Piece &pSrc = r.piece[i];
		PROPOSEDRUNPIECESDef &pDest = pr.piece[i];
		memset( &pDest, 0, sizeof(PROPOSEDRUNPIECESDef) );

		pDest.fromNODESrecordID	= pSrc.first().nodeID;
		pDest.fromTime			= pSrc.first().t;
		pDest.fromTRIPSrecordID = m_pRELIEFPOINTS[pSrc.first().iRef].TRIPSrecordID;

		pDest.toNODESrecordID	= pSrc.last().nodeID;
		pDest.toTime			= pSrc.last().t;
		pDest.toTRIPSrecordID	= m_pRELIEFPOINTS[pSrc.last().iRef].TRIPSrecordID;
	}

    // Cost out this run.
	COSTDef			cost;
    const long	runType = RunCoster( &pr, NO_RUNTYPE, &cost );
    if( cost.TOTAL.payTime != 0 && cost.runtype != UNCLASSIFIED_RUNTYPE && rc.isValidRuntype(runType) )
	{
		rc.runCache.insert( r, cost.TOTAL.payTime );
		++rc.callsFeasible;
		return true;
	}

	rc.runCache.insert( r, -1 );
	++rc.callsInfeasible;
	return false;
}

template < typename C > inline void assignMin( C &x, const C &y ) { if( y < x ) x = y; }
template < typename C > inline void assignMax( C &x, const C &y ) { if( x < y ) x = y; }

static	bool	AbortCallback( void *data )
{
	RuncutContext	&rc = *((RuncutContext *)data);

	if( rc.lastRunCost != rc.runcutter->sBest.cost ||
		rc.lastRunCount != rc.runcutter->sBest.runs.size() ||
		rc.lastIteration != rc.runcutter->getIteration() ||
		rc.callsCached + rc.callsFeasible + rc.callsInfeasible - rc.lastCallsTotal > 200 )
	{
		size_t	runsCur = rc.runcutter->sBest.runs.size();
		double	costCur = rc.runcutter->sBest.cost;
		if( runsCur == 0 )
		{
			runsCur = rc.runcutter->runs.size();
			costCur = rc.runcutter->cost;
		}

		rc.lastRunCount = rc.runcutter->sBest.runs.size();
		rc.lastRunCost  = rc.runcutter->sBest.cost;
		rc.lastIteration = rc.runcutter->getIteration();
		rc.lastCallsTotal = rc.callsCached + rc.callsFeasible + rc.callsInfeasible;

		const int	hour = rc.runcutter->tCur / (60*60), min = (rc.runcutter->tCur / 60) % 60;

		char	sz[512];
		sprintf( sz, "Scan: %02d%02d  Tries: %d  Runs cut: %d\nSuccess: %d / %d (%.1f%%)\nCache hits: %d",
			hour, min, rc.lastCallsTotal, runsCur,
			rc.callsFeasible, rc.callsFeasible + rc.callsInfeasible, 100.0 * (double)rc.callsFeasible / (double)(rc.callsFeasible + rc.callsInfeasible+1),
			rc.callsCached );
		StatusBarText( sz );
	}

	return !StatusBarAbort();
}

extern "C"
{
	
void BinRuncutSolve()
{
	Runcutter	runcutter;

	const tod_t	tInitMin	= 49*60*60;
	const tod_t tInitMax	=  1*60*60;

	runcutter.pieceSizeMin	= tInitMin;
	runcutter.pieceSizeMax	= tInitMax;

	runcutter.spreadMax		= tInitMax;

	runcutter.breakMin		= tInitMin;
	runcutter.breakMax		= tInitMax;

	runcutter.workPayMin	= tInitMin;

	runcutter.platformMin   = tInitMin;
	runcutter.platformMax   = tInitMax;

	runcutter.startEndMustMatch = true;
	//runcutter.startEndMustMatch = false;

	runcutter.iterationMax = 1;

	RuncutContext	rc;
	rc.runcutter = &runcutter;
	int i, j;
	for( i = 0; i < NUMRUNTYPES; ++i )
	{
		for( j = 0; j < NUMRUNTYPESLOTS; ++j )
		{
			if( (CUTPARMS.runtypes[i][j]) && (RUNTYPE[i][j].flags & RTFLAGS_INUSE))
			{
                RunTypeWrapper rtw( MAKELONG(i,j), &(RUNTYPE[i][j]) );
				if( rtw.getNumPieces() != 2 )
					continue;

				assignMin( runcutter.pieceSizeMin,	rtw.getMinPieceSize(0) );
				assignMax( runcutter.pieceSizeMax,	rtw.getMaxPieceSize(0) );

				assignMax( runcutter.spreadMax,		rtw.getMaxSpreadTime() );

				assignMin( runcutter.breakMin,		rtw.getMinBreakTime(0) );
				assignMax( runcutter.breakMax,		rtw.getMaxBreakTime(0) );

				assignMin( runcutter.workPayMin,	rtw.getMinPayTime() );

				assignMin( runcutter.platformMin,	rtw.getMinPlatformTime() );
				assignMax( runcutter.platformMax,	rtw.getMaxPlatformTime() );
				
				rc.runTypes.push_back( rtw.runType );
            }
		}
	}

	std::sort( rc.runTypes.begin(), rc.runTypes.end() );

	// Insert all the node equivalences.
	int	rcode2;
	for(rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
		rcode2 == 0;
		rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0) )
	{
		if( CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT )
		{
			runcutter.setEquivalent( CONNECTIONS.fromNODESrecordID, CONNECTIONS.toNODESrecordID, CONNECTIONS.connectionTime );
			if( CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY )
				runcutter.setEquivalent( CONNECTIONS.toNODESrecordID, CONNECTIONS.fromNODESrecordID, CONNECTIONS.connectionTime );
		}
	}

	size_t	iPiece = 0;
	RELIEFPOINTSDef *r = m_pRELIEFPOINTS, *rEnd = m_pRELIEFPOINTS + m_numRELIEFPOINTS;
	while( r != rEnd )
    {
		// Find the start of some unmatched relief points.
        for( ; r != rEnd && IsFromReliefPointMatched(*r); ++r )
			continue;

		if( r == rEnd )
			break;

		// Create another uncut piece.
		const long blockNumber = r->blockNumber;
		BinPackRuncut::Block	&blockCur = runcutter.newBlock( iPiece++ );

		// Add this first relief point.
		blockCur.append( r->time, r->NODESrecordID, r - m_pRELIEFPOINTS );

		// Add the remaining relief points until we get a previously matched "to" point.
		for( ++r; r != rEnd && r->blockNumber == blockNumber && !IsToReliefPointMatched(*r); ++r )
			blockCur.append( r->time, r->NODESrecordID, r - m_pRELIEFPOINTS );
    }

	runcutter.setCallback( &AbortCallback, &rc );
	runcutter.setFeasibleCallback( &getRunCost, &rc );
	runcutter.solve();

	rc.clear();

	for( BinPackRuncut::RunList::iterator run = runcutter.sBest.runs.begin(), runEnd = runcutter.sBest.runs.end(); run != runEnd; ++run )
	{
		// Filter out whether to record this run.
		if( CUTPARMS.cutRuns == 1 )	// User has selected to end after a certain time.
		{
			if( run->piece[1].last().t < CUTPARMS.endTime )
				continue;
		}
		else						// User has selected to start before a certain time.
		{
			if( run->piece[1].first().t > CUTPARMS.startTime )
				continue;
		}

		// Install all the pieces of the run.
		for( int pieceNumber = 1; pieceNumber <= 2; ++pieceNumber )
		{
			BinPackRuncut::Piece	&pieceCur = run->piece[pieceNumber-1];
			BinPackRuncut::Block	&blockCur = *pieceCur.b;
			for( register int iRelief = pieceCur.i, iReliefEnd = pieceCur.j; iRelief <= iReliefEnd; ++iRelief )
			{
				r = &m_pRELIEFPOINTS[blockCur[iRelief].iRef];

				if( iRelief != pieceCur.j )
				{
					r->start.recordID = NO_RECORD;
					r->start.runNumber = m_GlobalRunNumber;
					r->start.pieceNumber = pieceNumber;
				}

				if( iRelief != pieceCur.i )
				{
					r->end.recordID = NO_RECORD;
					r->end.runNumber = m_GlobalRunNumber;
					r->end.pieceNumber = pieceNumber;
				}
			}
        }
        ++m_GlobalRunNumber;
    }
}
} // extern "C"
