//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// Disable bogus stl warnings.
#pragma warning(disable : 4786 4114 4699)

#include "Recursive.h" 

#if _MSC_VER >= 1200  // Version 6.0 and above
#include <map>
#include <set>
#include <algorithm>
#else
namespace std
{
#include <map.h>
#include <set.h>
#include <algorithm.h>
}
#endif  // Version 6.0 and above

static	const	RRun::CostBiasType	bias = RRun::PayPlatformDifference;
static	const	tod_t	bigTime = 3*24*60*60;

typedef std::map< long, Ref<RRun>, std::less<long> > RunNumberRuns;

struct RunPieceNumber
{
	RunPieceNumber( Ref<RRun> aRun, Ref<RPiece> aPiece )
		: run(aRun), pieceNumber(-1), pieceOrig(*aPiece), pieceBest(new RPiece(*aPiece)) {}

	void	init()	{ pieceNumber = run->getPieceNumber(pieceBest); run->getCost(bias); }

	Ref<RRun>			run;
	int					pieceNumber;
	const RPiece		pieceOrig;	// Keep track of the original piece.
	Ref<RPiece>			pieceBest;

	bool	isChanged() const { return pieceOrig != *pieceBest; }

	void	setChange( ImprovedPieceChangeDef &ipc ) const
	{
		ipc.runNumber		= run->getRunNumber();
		ipc.pieceNumber		= run->getPieceNumber(pieceBest);
		ipc.oldStartRelief	= pieceOrig.getIStart();
		ipc.oldEndRelief	= pieceOrig.getIEnd();
		ipc.newStartRelief	= pieceBest->getIStart();
		ipc.newEndRelief	= pieceBest->getIEnd();
	}

	bool operator==( const  ImprovedPieceChangeDef &ipc ) const
	{
		return	ipc.runNumber == run->getRunNumber() &&
				ipc.pieceNumber == pieceNumber;
	}

	bool operator<( const RunPieceNumber &rpn ) const
	{
		return	run->getRunNumber() < rpn.run->getRunNumber() ? true
			:	run->getRunNumber() > rpn.run->getRunNumber() ? false
			:	pieceNumber < rpn.pieceNumber;
	}
};

struct ImprovedPieceChangeDefLt
{
	bool operator()( const ImprovedPieceChangeDef &ipc1, const ImprovedPieceChangeDef &ipc2 ) const
	{
		return	ipc1.runNumber < ipc2.runNumber ? true
			:	ipc1.runNumber > ipc2.runNumber ? false
			:	ipc1.pieceNumber < ipc2.pieceNumber;
	}
};

struct ImprovedPieceChangeDefEq
{
	bool operator()( const ImprovedPieceChangeDef &ipc1, const ImprovedPieceChangeDef &ipc2 ) const
	{
		return ipc1.runNumber == ipc2.runNumber && ipc1.pieceNumber == ipc2.pieceNumber;
	}
};

struct PieceBoundary
{
	PieceBoundary(	Ref<RRun> aFromRun, Ref<RPiece> aFromPiece,
					Ref<RRun> aToRun,	Ref<RPiece> aToPiece )
		:	from(aFromRun, aFromPiece),
			to(aToRun, aToPiece) {}

	RunPieceNumber	from, to;

	bool	operator<( const PieceBoundary &pb ) const
	{ return from.pieceOrig.blockNumber() < pb.from.pieceOrig.blockNumber() ? true :
			 from.pieceOrig.blockNumber() > pb.from.pieceOrig.blockNumber() ? false :
			 from.pieceOrig.endTime() < pb.from.pieceOrig.endTime(); }

	long	improve();

	bool	isChanged() const
	{ return from.isChanged(); }

	bool	consecutiveTo( const PieceBoundary &pb ) const
	{
		return	to.pieceBest->getIEnd() == pb.from.pieceBest->getIStart() ||
				pb.to.pieceBest->getIEnd() == from.pieceBest->getIStart();
	}

	MMdeclare();
};

MMinit( PieceBoundary );

template <class T>
struct refLess
{
	bool operator()( const Ref<T> &r1, const Ref<T> &r2 ) const
	{ return *r1 < *r2; }
};

typedef std::set< Ref<PieceBoundary>, refLess<PieceBoundary> >	PieceBoundaries;

static	inline	bool	good(const RUNTYPEDef &runInfo, const long runType, const RRun &r,
							const RPiece &p, const int pieceNumber, const RPiece &pOrig )
{
	if( !(p.getIStart() >= 0 && p.getIStart() < m_numRELIEFPOINTS) ) return false;
	if( !(p.getIEnd() >= 0 && p.getIEnd() < m_numRELIEFPOINTS) ) return false;
	if( !(m_pRELIEFPOINTS[p.getIStart()].blockNumber == m_pRELIEFPOINTS[pOrig.getIStart()].blockNumber) ) return false;
	if( !(m_pRELIEFPOINTS[p.getIEnd()].blockNumber == m_pRELIEFPOINTS[pOrig.getIEnd()].blockNumber) ) return false;
	if( !(p.length() >= provideDefault(runInfo.PIECE[pieceNumber].minPieceSize,0)) ) return false;
	if( !(p.length() <= provideDefault(runInfo.PIECE[pieceNumber].maxPieceSize,bigTime)) ) return false;
	if( !(r.getSpreadTime() <= provideDefault(runInfo.maxSpreadTime,bigTime)) ) return false;
	if( !(r.getCostedRunType(bias) == runType) ) return false;
	return true;
}

long	PieceBoundary::improve()
{
	const	int	searchSpreadMax = 8;

	// Attempt to improve these runs by swapping some relief points.
	const	long	fromRunType	= from.run->getCostedRunType();
	if( fromRunType < 0 )
		return 0;

	const	long	toRunType	= to.run->getCostedRunType();
	if( toRunType < 0 )
		return 0;

	const	RUNTYPEDef	&fromRunInfo = RUNTYPE[LOWORD(fromRunType)][HIWORD(fromRunType)];
	RRun	fromRunCur(*from.run);

	const	RUNTYPEDef	&toRunInfo = RUNTYPE[LOWORD(toRunType)][HIWORD(toRunType)];
	RRun	toRunCur(*to.run);

	const long	costOld = fromRunCur.getCost(bias) + toRunCur.getCost(bias);

	long	costBest = costOld;

	Ref<RPiece>	fromPieceBest	= from.pieceBest;
	Ref<RPiece>	toPieceBest		= to.pieceBest;
	for( int i = 1; i < searchSpreadMax; i = i > 0 ? -i : -i + 1 )
	{
		do // Not really a loop, just gives me a way to break if something goes wrong without a goto.
		{
			// Adjust the piece in the first run.
			fromRunCur[from.pieceNumber] = new RPiece(*from.pieceBest);
			fromRunCur[from.pieceNumber]->moveIEnd(i);
			fromRunCur.resetCost();
			if( !good(fromRunInfo, fromRunType, fromRunCur, *fromRunCur[from.pieceNumber], from.pieceNumber, from.pieceOrig) )
				break;

			// Adjust the piece in the second run.
			toRunCur[to.pieceNumber] = new RPiece(*to.pieceBest);
			toRunCur[to.pieceNumber]->moveIStart(i);
			toRunCur.resetCost();
			if( !good(toRunInfo, toRunType, toRunCur, *toRunCur[to.pieceNumber], to.pieceNumber, to.pieceOrig) )
				break;

			// If the cost is better, record the adjustments.
			const long	costCur = fromRunCur.getCost(bias) + toRunCur.getCost(bias);
			if( costCur < costBest )
			{
				costBest		= costCur;
				fromPieceBest	= fromRunCur[from.pieceNumber];
				toPieceBest		= toRunCur[to.pieceNumber];
			}
		} while(0);
	}

	// Record the best pieces found so far.
	// Make them part of the runs.
	if( costBest < costOld )
	{
		(*from.run)[from.pieceNumber]= from.pieceBest	= fromPieceBest;
		from.run->resetCost();
		(*to.run)[to.pieceNumber]	 = to.pieceBest		= toPieceBest;
		to.run->resetCost();
	}

	return costOld - costBest;
}

static	void	InitRuns( RunNumberRuns &runs, PieceBoundaries &pbs )
{
	Reverse::setForward();

	runs.clear();
	pbs.clear();

    int	i;

	// Record all unfrozen run numbers.
	for( i = 0; i < MAXRUNSINRUNLIST; ++i )
	{
		const RUNLISTDATADef	&r = RUNLISTDATA[i];
		if( r.runNumber == NO_RUNTYPE ||
			r.runNumber == UNCLASSIFIED_RUNTYPE ||
			r.frozenFlag == TRUE )
			continue;
		runs.insert( RunNumberRuns::value_type(r.runNumber, new RRun(r.runNumber)) );
	}

	if( runs.empty() )
		return;

	Ref<RRun>	lastRun;
	Ref<RPiece>	lastPiece;
	register int	j;
	for( i = 0; i < m_numRELIEFPOINTS; i = j )
	{
		// Find the end of this piece.
		j = i;
		const	long	runNumber = m_pRELIEFPOINTS[i].start.runNumber;
		if( runNumber == NO_RECORD )
		{
			do ++j; while( j < m_numRELIEFPOINTS && m_pRELIEFPOINTS[j].start.runNumber == NO_RECORD );
			continue;
		}

		const	long	pieceNumber = m_pRELIEFPOINTS[i].start.pieceNumber;
		do ++j; while( j < m_numRELIEFPOINTS &&
						m_pRELIEFPOINTS[j].start.runNumber == runNumber &&
						m_pRELIEFPOINTS[j].start.pieceNumber == pieceNumber );

		// Ignore uncut pieces or frozen runs.
		RunNumberRuns::iterator rnri = runs.find(runNumber);
		if( rnri == runs.end() )
		{
			lastRun = NULL;
			lastPiece = NULL;
			continue;
		}

		// Add this piece to the run.
		Ref<RRun>	thisRun = (*rnri).second;
		Ref<RPiece>	thisPiece = new RPiece(i,j);
		thisRun->addPiece( thisPiece );

		// Record this as a piece boundary.
		if( lastPiece() &&
			lastPiece->blockNumber() == thisPiece->blockNumber() &&
			lastPiece->actualEndTime() == thisPiece->actualStartTime() )
		{
			pbs.insert( new PieceBoundary(lastRun, lastPiece, thisRun, thisPiece) );
		}

		lastRun = thisRun;
		lastPiece = thisPiece;
	}

	// Intialize the boundaries after all the pieces have been added.
	// Remove any invalid runs.
	PieceBoundaries::iterator pbi, pbiNext;
	for( pbi = pbs.begin(); pbi != pbs.end(); pbi = pbiNext )
	{
		pbiNext = pbi; ++pbiNext;

		(*pbi)->from.init();
		(*pbi)->to.init();

		bool	success = true;
		if( !(*pbi)->from.run->valid() )
		{
			runs.erase( (*pbi)->from.run->getRunNumber() );
			success = false;
		}
		if( !(*pbi)->to.run->valid() )
		{
			runs.erase( (*pbi)->to.run->getRunNumber() );
			success = false;
		}
		if( !success )
			pbs.erase( pbi );
	}
}

static ImprovedPieceChangeDef	*ipcd = NULL;
static int ipcdCount = 0;

extern "C"
{

void	GetImprovedPieceChanges(ImprovedPieceChangeDef **ipc, int *numIPC)
{
	*ipc = ipcd;
	*numIPC = ipcdCount;
}

void	FreeImprovedPieceChanges()
{
	delete ipcd;
	ipcd = NULL;
	ipcdCount = 0;
}

int	RuncutImprove( long *savings )
{
	size_t	ipcdCount = 0;
	*savings = 0;
	
	RunNumberRuns	runs;
	PieceBoundaries	pbs;

	InitRuns( runs, pbs );

	if( pbs.empty() )
		return 0;

	RunNumberRuns::iterator	rnri;
	long	costBefore = 0;
	for( rnri = runs.begin(); rnri != runs.end(); ++rnri )
		costBefore += (*rnri).second->getCost(bias);

	PieceBoundaries::iterator pb;
	long	improved = 0;
	bool	success = false, improvementMade = false;
	do
	{
		success = false;

		for( pb = pbs.begin(); pb != pbs.end(); ++pb )
		{
			const long improvedCur = (*pb)->improve();
			if( improvedCur )
			{
				improved += improvedCur;
				success = true;
				improvementMade = true;
			}
		}
	} while( success );

	long	costAfter = 0;
	if( improvementMade )
	{
		for( rnri = runs.begin(); rnri != runs.end(); ++rnri )
		{
			costAfter += (*rnri).second->getCost(bias);
			(*rnri).second->install();
		}

		for( pb = pbs.begin(); pb != pbs.end(); ++pb )
			if( (*pb)->isChanged() )
				ipcdCount += 2;

		delete ipcd;
		ipcd = new ImprovedPieceChangeDef[ipcdCount];

		size_t	i = 0;
		for( pb = pbs.begin(); pb != pbs.end(); ++pb )
		{
			if( (*pb)->isChanged() )
			{
				(*pb)->from.setChange(	ipcd[i++] );
				(*pb)->to.setChange(	ipcd[i++] );
			}
		}

		// Remove any multiply counted changes.
		// This has the nice effect of sequencing the changes by run and piece number.
		std::sort( ipcd, ipcd + ipcdCount, ImprovedPieceChangeDefLt() );
		ipcdCount = std::unique( ipcd, ipcd + ipcdCount, ImprovedPieceChangeDefEq() ) - ipcd;
	}
	else
	{
		costAfter = costBefore;
		FreeImprovedPieceChanges();
	}

	*savings = costBefore - costAfter;
	return ipcdCount;
}

}