//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "Recursive.h"
#include "match.H"
#include "cistms.h"
#include "TimeInterval.h"

#ifndef Assert
#define Assert(c)	do { if( !(c) ) { int xxx1 = 10, xxx2 = 0, xxx3; xxx3 = xxx1 / xxx2; } } while(0)
#endif // Assert

//#define DEBUG_RUNS
#ifdef DEBUG_RUNS
#include <fstream>
#include "Timer.h"
#include "TFormat.h"

using namespace std;

static ostream	&PrintRPiece( ostream &os, const RPiece *p )
{
	os << p->blockNumber() << '\t' << tFormat(p->startTime()) << '\t' << tFormat(p->endTime()) << "\tlength=" << tFormat(p->length());
	os.flush();
	return os;
}

inline ostream	&operator<<( ostream &os, const RPiece *p )	{ return PrintRPiece(os, p); }
inline ostream	&operator<<( ostream &os, const RPiece &p )	{ return PrintRPiece(os, &p); }
inline ostream	&operator<<( ostream &os, Ref<RPiece> p )	{ return PrintRPiece(os, p.data()); }

static ofstream	*pofs;
#define ofs	(*pofs)

#endif // DEBUG_RUNS

//----------------------------------------------------------------------------------------------------
const long InfiniteCost = 1000 * 60 * 60;

bool	Reverse::reverse = 0;

MMinit( RPiecePair );
MMinit( RPiece );
MMinit( RRun );

int	RPiece::findClosestLE( tod_t t ) const
{
	t += (*this)[0];

	register int left = -1, right = iMost()+1, mid;
	while( right - left > 1 )
	{
		mid = ((left + right) >> 1);
		(t < (*this)[mid] ? left : right) = mid;
	}
	if( left < 0 ) ++left;
	if( (left == 0 && (*this)[left] > t) || left == iMost()+1 )
		return -1;
	if( (*this)[left] > t )
		--left;
	return left;
}

RPiecePair	*RPiece::forwardSplit( const tod_t minSize, const tod_t desSize, const tod_t maxSize, const tod_t minLeftover )
{
	if( length() < minSize + minLeftover )
		return NULL;
	
	register int	i = findClosestLE( desSize );
	if( i < 0 )
		return NULL;
	
	// Make sure we have something >= minSize.
	if( length(0, i) < minSize )
	{
		while( length(0, i) < minSize && i < iMost() )
			++i;
		// Check if it is possible to make cut in the middle.
		if( length(0, i) > maxSize )
			return NULL;
	}
	
	// Check for a leftover violation.
	if( length(i, iMost()) < minLeftover )
	{
		while( length(i, iMost()) < minLeftover && i > 0 )
			i--;
		
		if( length(0, i) >= minSize && length(i, iMost()) >= minLeftover )
			return new RPiecePair(	new RPiece(getI(0), getI(i)),
			new RPiece(getI(i), getI(iMost())) );
		else
			return NULL;
	}
	
#ifndef Abs
#define Abs(a)  ((a) > 0 ? (a) : -(a) )
#endif
	
	// Check if a larger piece is a better choice.
	for( ;; )
	{
		const	int		iNext = i+1;
		if( iNext > iMost() ||
			length(iNext, iMost()) < minLeftover ||
			length(0,iNext) > maxSize )
			break;
		if( Abs(length(0,i) - desSize) > Abs(length(0,iNext) - desSize) )
			i = iNext;
		else
			break;
	}
	
	return new RPiecePair(	new RPiece(getI(0), getI(i)),
		new RPiece(getI(i), getI(iMost())) );
}

RPiecePair	*RPiece::forwardSplitLeftover( const tod_t minSize, const tod_t minLeftover )
{
	if( length() < minSize + minLeftover )
		return NULL;

	register int	i = findClosestLE( minSize );
	while( length(0, i) < minSize )
		++i;
	if( length(i, iMost()) < minLeftover )
		return NULL;

	return new RPiecePair(	new RPiece(getI(0), getI(i)),
							new RPiece(getI(i), getI(iMost())) );
}

RPiecePair	*RPiece::split( const int i )
{
	// Split the piece at the specified relief point index.
	if( i < iMost() )
		return new RPiecePair(  new RPiece(getI(0), getI(i)),
								new RPiece(getI(i), getI(iMost())) );
	else
		return new RPiecePair( new RPiece(getI(0), getI(i)) );
}

RPiecePair	*RPiece::removePiece( const RPiece &p ) const
{
#ifdef DEBUG_RUNS
	if( blockNumber() != p.blockNumber() )
	{
		ofs << "removePiece: pieces to not belong to the same block!\n";
		ofs << '\t'; PrintRPiece( ofs, this ); ofs << '\n';
		ofs << '\t'; PrintRPiece( ofs, &p ); ofs << '\n';
	}
#endif // DEBUG_RUNS
	Assert( blockNumber() == p.blockNumber() );

	if( !(iStart <= p.iStart && p.iEnd <= iEnd) )
		return NULL;

	RPiecePair	*pp = new RPiecePair;
	if( iStart == p.iStart && iEnd == p.iEnd )
		;
	else if( iStart == p.iStart )
		pp->first = new RPiece( p.iEnd, iEnd );
	else if( iEnd == p.iEnd )
		pp->first = new RPiece( iStart, p.iStart );
	else
	{
		pp->first = new RPiece( iStart, p.iStart );
		pp->second = new RPiece( p.iEnd, iEnd );
	}
	return pp;
}

//-------------------------------------------------------------------------------

void	RRun::addPiece( Ref<RPiece> p )
{
	register int		i;

	Ref<RPiece>	*piecesNew = new Ref<RPiece> [numPieces + 1];
	for( i = 0; i < numPieces; i++ )
		piecesNew[i] = pieces[i];
	delete [] pieces;
	pieces = piecesNew;
	pieces[numPieces++] = p;

	// Make sure the pieces remain in order.
	for( i = numPieces-2; i >= 0; i-- )
		if( pieces[i]->actualStartTime() > pieces[i+1]->actualStartTime() )
		{
			Ref<RPiece> t(pieces[i]);
			pieces[i] = pieces[i+1];
			pieces[i+1] = t;
		}
	costCur = -1;	// Invalidate the current known cost.
}

RRun	&RRun::assign( const RRun &r )
{
	if( numPieces != r.numPieces )
	{
		delete [] pieces;
		pieces = r.numPieces ? new Ref<RPiece> [r.numPieces] : NULL;
		numPieces = r.numPieces;
	}

	for( register int i = 0; i < r.numPieces; ++i )
		pieces[i] = r.pieces[i];

	costCur = r.costCur;
	runNumber = r.runNumber;

	return *this;
}

size_t RRun::costCount = 0;

long	RRun::getCostCall( const CostBiasType bias )
{
	if( costCur > 0 )	// Return the known cost if we have one.
		return costCur;

	incCostCount();
	register	int	k;
	// Add the pieces to the PROPOSEDRUN structure.
	PROPOSEDRUN	pr;
	for( k = 0; k < numPieces; k++ )
	{
		RPiece &p = *getPiece( k );
		pr.piece[k].fromTime = p.actualStartTime();
		pr.piece[k].fromNODESrecordID = p.getFromNodesRecordID();
		pr.piece[k].fromTRIPSrecordID = p.getFromTripsRecordID();

		pr.piece[k].toTime = p.actualEndTime();
		pr.piece[k].toNODESrecordID = p.getToNodesRecordID();
		pr.piece[k].toTRIPSrecordID = p.getToTripsRecordID();
	}
	pr.numPieces = numPieces;

	COSTDef	cost;
	costedRunType = RunCoster( &pr, NO_RUNTYPE, &cost );
	if( costedRunType == UNCLASSIFIED_RUNTYPE )
		return costCur = InfiniteCost;

	switch( bias )
	{
	case PayPlatformRatio:
		{
			double costRatio = (cost.TOTAL.platformTime > 0 ? cost.TOTAL.payTime / cost.TOTAL.platformTime : 10.0);
			if( costRatio < 1.0 )
				costRatio = 1.0;
			costCur = (long)(costRatio * 1000.0);
		}
		break;

	case PayPlatformDifference:
		{
			// Ignore the run type as long as it is valid.
//			long costDifference = cost.TOTAL.payTime - (numPieces > 1 ? cost.TOTAL.platformTime : 0);
			long costDifference = cost.TOTAL.payTime - cost.TOTAL.platformTime;
			if( costDifference < 0 )
				costDifference = 1;
			costCur = costDifference;
		}
		break;

	case None:
		costCur = cost.TOTAL.payTime;
		break;
	}
	
	return costCur;
}	
//-------------------------------------------------------------------------------

static inline double dSquare( const double x ) { return x * x; }
static inline long   lSquare( const long   x ) { return x * x; }
static inline long     lCube( const long   x ) { return x * x * x; }

void	RecursiveCut::pairwiseSwapImproveTwoPieceRuns()
{
	// Collect the two piece runs and remove them from the run list.
	Runs	twoPieceRuns;
	Runs::iterator rItr;
	for( rItr = runs.begin(); rItr != runs.end(); ++rItr )
		if( (*rItr)->getNumPieces() == 2 )
			twoPieceRuns.insert( *rItr );

	if( twoPieceRuns.empty() )
		return;

	// Try pairwise swapping the runs to improve the overall runcut.
	Runs::iterator r1, r2;
	bool	success;
	do
	{
		success = false;
		for( r1 = twoPieceRuns.begin(); r1 != twoPieceRuns.end(); ++r1 )
		{
			for( r2 = r1, ++r2; r2 != twoPieceRuns.end(); ++r2 )
			{
				Ref<RPiece>	a = (*r1)->getPiece(0), b = (*r1)->getPiece(1);
				Ref<RPiece>	c = (*r2)->getPiece(0), d = (*r2)->getPiece(1);

				// Skip intersecting pieces.
				if( a->intersectsInterval(*d) || c->intersectsInterval(*b) )
					continue;

				// Form new runs and cost them.
				RRun		r1New; r1New.addPiece(a); r1New.addPiece(d);
				if( r1New.getCost() >= InfiniteCost || !runTypeHash.contains(r1New.getCostedRunType()) )
					continue;

				RRun		r2New; r2New.addPiece(c); r2New.addPiece(b);
				if( r2New.getCost() >= InfiniteCost || !runTypeHash.contains(r2New.getCostedRunType()) )
					continue;

				// If the total cost of the new runs beats the old ones, keep them.
				const long oldCost = (*r1)->getCost() + (*r2)->getCost();
				const long newCost = r1New.getCost() + r2New.getCost();
				if( oldCost < newCost )
					continue;
				// If the new runs cost the same but are more equal, keep them.
				if( oldCost == newCost &&
					dSquare((*r1)->getCost()) + dSquare((*r2)->getCost()) <=
					dSquare(r1New.getCost()) + dSquare(r2New.getCost()) )
					continue;

				// This assignment preserves the start times of the runs.
				*(*r1) = r1New;
				*(*r2) = r2New;
				success = true;
			}
		}
	} while( success );
}

void	RecursiveCut::improveTwoPieceRuns()
{
	// Remember - the matcher works on indices from 1..N, not 0..N-1.
	Runs	twoPieceRuns;
	Runs::iterator rItr;
	for( rItr = runs.begin(); rItr != runs.end(); ++rItr )
		if( (*rItr)->getNumPieces() == 2 )
			twoPieceRuns.insert( *rItr );

	if( twoPieceRuns.empty() )
		return;

	const size_t iPieceMac = twoPieceRuns.size()*2;
	MatchGraph	mg( iPieceMac );
	Ref<RPiece>	*pieces = new Ref<RPiece>[iPieceMac];
	register size_t	i = 0, j;
	for( rItr = twoPieceRuns.begin(); rItr != twoPieceRuns.end(); ++rItr )
	{
		// Add all its pieces to our own array.
		// Also, initialize the match graph with the current feasible match.
		for( j = 0; j < 2; j++ )
		{
			pieces[i] = (*rItr)->getPiece(j);
			++i;
		}
		// Add the current feasible match.
		mg.AddEdge( i-1, i, (*rItr)->getCost() );
	}

	// Generate all the possible matches.
	for( i = 0; i < iPieceMac; i++ )
	{
		// If i is even, the next piece is its former mate.
		// We can skip it because we already added the cost arc.
		for( j = (i&1) ? i+1 : i+2; j < iPieceMac; j++ )
		{
			if( pieces[i]->intersectsInterval(*pieces[j]) )
				continue;

			// Check if the run formed by swapping the pieces is valid.
			RRun	runTmp;
			runTmp.addPiece( pieces[i] );
			runTmp.addPiece( pieces[j] );
			if( runTmp.getCost() >= InfiniteCost || !runTypeHash.contains(runTmp.getCostedRunType()) )
				continue;

			// Check if this new run costs as a type we are cutting.
			mg.AddEdge( i+1, j+1, runTmp.getCost() );
		}
	}

	// Rematch the pieces into 2 piece runs.
	mg.Weighted_Match(0);

	// Check if the matching succeeded.
	for( i = 0; i < iPieceMac; i++ )
	{
		const int iMate = mg.Mate(i+1)-1;
		if( iMate < 0 )
			break;
	}

	if( i == iPieceMac )		// Success!!!
	{
		// Remove the old runs from the run list.
		for( rItr = twoPieceRuns.begin(); rItr != twoPieceRuns.end(); ++rItr )
			runs.remove( *rItr );

		// Install the new, optimized matched runs.
		for( i = 0; i < iPieceMac; i++ )
		{
			if( !pieces[i] )	// Skip pieces that have already been matched.
				continue;

			const int iMate = mg.Mate(i+1)-1;

			if( iMate < 0 )
				continue;

			Ref<RRun>	r( new RRun );
			r->addPiece( pieces[i] );
			r->addPiece( pieces[iMate] );
			runs.insert( r );

			pieces[i] = NULL;	// Mark these pieces as matched.
			pieces[iMate] = NULL;
		}
	}
	else						// Match failed.
		pairwiseSwapImproveTwoPieceRuns();

	// Cleanup
	delete [] pieces;
}

void	RecursiveCut::initPieces()
{
	// Clean out the pieces structure.
	pieces.erase( pieces.begin(), pieces.end() );

	// Insert uncut blocks into the runcutter as pieces.
	register int i = 0;
	while( i < m_numRELIEFPOINTS )
	{
		if( m_pRELIEFPOINTS[i].start.runNumber != NO_RECORD &&
          m_pRELIEFPOINTS[i].start.runNumber != SECURELOCATION_RUNNUMBER)
		{
			++i;
			continue;
		}

		const int iStart = i++;
		const long blockNumber = m_pRELIEFPOINTS[iStart].blockNumber;

		while( i < m_numRELIEFPOINTS &&
			m_pRELIEFPOINTS[i].end.runNumber == NO_RECORD &&
			m_pRELIEFPOINTS[i].blockNumber == blockNumber )
			++i;

		// Only add cuttable pieces.
		const RPiece	p(iStart, i-1);
		if( p.length() < getMinLeftover() )
			continue;
		insertPiece( new RPiece(p) );
	}
}

void	RecursiveCut::getPieceTimeRange( tod_t &earliest, tod_t &latest ) const
{
	earliest = 10 * 24 * 60 * 60;
	latest = -earliest;
	Pieces::iterator piece;
	for( piece = pieces.begin(); piece != pieces.end(); ++piece )
	{
		if( (*piece).second->actualStartTime() < earliest )
			earliest = (*piece).second->actualStartTime();
		if( (*piece).second->actualEndTime() > latest )
			latest = (*piece).second->actualEndTime();
	}
}

struct	SFrame
{
	enum CutType { Entire, FromFront, FromBack, FromMiddle };

	SFrame( Ref<RPiece> aOriginal = (RPiece *)NULL,
		Ref<RPiece> aPiece = (RPiece *)NULL, 
		Ref<RPiecePair> aRemaining = (RPiecePair *)NULL,
		RecursiveCut::CutFlags aType = RecursiveCut::Entire )
		:	original(aOriginal),
			piece(aPiece),
			remaining(aRemaining),
			type(aType),
			fOldBreakOfHasOccurred(false),
			fNewBreakOfHasOccurred(false) {}
	SFrame &operator=( const SFrame &sp )
	{
		original = sp.original;
		piece = sp.piece;
		remaining = sp.remaining;
		type = sp.type;
		fOldBreakOfHasOccurred = sp.fOldBreakOfHasOccurred;
		fNewBreakOfHasOccurred = sp.fNewBreakOfHasOccurred;
		return *this;
	}
	void	reset()
	{
		original = NULL;
		piece = NULL;
		remaining = NULL;
		type = RecursiveCut::Entire;
		fOldBreakOfHasOccurred = false;
		fNewBreakOfHasOccurred = false;
	}
	void	setOldBreakOfHasOccurred( const bool aB ) { fOldBreakOfHasOccurred = aB; }
	bool	getOldBreakOfHasOccurred() const { return fOldBreakOfHasOccurred; }
	void	setNewBreakOfHasOccurred( const bool aB ) { fNewBreakOfHasOccurred = aB; }
	bool	getNewBreakOfHasOccurred() const { return fNewBreakOfHasOccurred; }
	bool	getBreakOfHasOccurred() const { return fOldBreakOfHasOccurred != fNewBreakOfHasOccurred; }

	tod_t	cutPenalty() const;

	Ref<RPiece>	original, piece;
	Ref<RPiecePair> remaining;
	RecursiveCut::CutFlags	type;
	bool	fOldBreakOfHasOccurred, fNewBreakOfHasOccurred;	// Keep track of the old and new breakOf flag.
};

tod_t	SFrame::cutPenalty() const
{
	switch( type )
	{
	case Entire:		return 0;
	case FromFront:		return 1 * 60 * 60;
	case FromBack:		return 1 * 60 * 60;
	case FromMiddle:	return 2 * 60 * 60;
	}
	return 0;
}

const static int	beamSearchMax = 10;		// Maximum number of trys before we declare a winner.
static	int			beamSearchCount = 0;	// Current count of trys.
static	bool		fFoundValidRun = false;	// Flag indicating whether a valid run is found.
static inline bool	keepSearching()	{ return (!fFoundValidRun || beamSearchCount < beamSearchMax) && !StatusBarAbort(); }
static	long		costBest;				// Best known cost so far.

static	tod_t	platformCur = 0, minTotalFollowing = 0;
static	int	iPiece = 0;
static	SFrame	bestSF[16], curSF[16];		// Piece stacks of best and current runs.

void	RecursiveCut::createCutPossibilities( RPieceCollection &pc, Ref<RPiece> original, const int cutFlags )
{
	if( StatusBarAbort() )
		return;

	// Handle the case when this is the first piece of the run.
	tod_t minPieceSize = getMinPieceSize(0);
	tod_t desPieceSize = getDesPieceSize(0);
	tod_t maxPieceSize = getMaxPieceSize(0);

	register int	i, j;

	if( getNumPieces() > 1 )
	{
		tod_t	tMaxTotal = 0, tMinTotal = 0, tRemaining;
		for( i = 1; i < getNumPieces(); ++i )
		{
			tMaxTotal += getMaxPieceSize(i);
			tMinTotal += getMinPieceSize(i);
		}

		// Check what the *real* minPieceSize can be.
		tRemaining = getMinPlatformTime() - tMaxTotal;
		if( tRemaining > minPieceSize )
			minPieceSize = tRemaining;
		if( desPieceSize < minPieceSize )
			desPieceSize = minPieceSize;

		// Check if this is really hosed up.
		if( minPieceSize > maxPieceSize )
			return;

		// Check what the *real* maxPieceSize can be.
		tRemaining = getMaxPlatformTime() - tMinTotal;
		if( tRemaining < maxPieceSize )
			maxPieceSize = tRemaining;
		if( desPieceSize > maxPieceSize )
			desPieceSize = maxPieceSize;

		// Check if this is really hosed up.
		if( minPieceSize > maxPieceSize )
			return;
	}

	// Check if we should take the whole piece.
	if( (cutFlags & Entire) &&
			original->length() <= maxPieceSize &&
			original->length() >= minPieceSize &&
			original->startTime() <= getStartRunBeforeTime() &&
			TimeInterval(getMinOnTime(0), getMaxOnTime(0)).contains(original->startTime()) &&
			TimeInterval(getMinOffTime(0), getMaxOffTime(0)).contains(original->endTime()) )
	{
		pc.insert( RPieceElement(original->startTime(), original->length(), original, Entire) );
	}

	// Generate all the possibilities from the beginning.
	if( (cutFlags & FromFront) &&
		original->startTime() <= getStartRunBeforeTime() &&
		TimeInterval(getMinOnTime(0), getMaxOnTime(0)).contains(original->startTime()) )
	{
		const	tod_t	desStartTime = original->startTime();
		i = 0;
		for( j = i+1; j <= original->iMost(); ++j )
		{
			if( original->length(j, original->iMost()) < getMinLeftover() ) break;
			if( original->length(i, j) > maxPieceSize )						break;
			if( (*original)[j] > getMaxOffTime(0) )							break;
			if( original->length(i, j) < minPieceSize )						continue;
			if( (*original)[j] < getMinOffTime(0) )							continue;
			pc.insert( RPieceElement(desStartTime, getDesPieceSize(0), new RPiece(original->getI(i), original->getI(j)), FromFront) );
		}
	}

	// Generate all the possibilities from the middle.
	if( cutFlags & FromMiddle )
	{
		const	tod_t	desStartTime = original->startTime();
		for( i = 1; i < original->iMost(); ++i )
		{
			if( (*original)[i] > getStartRunBeforeTime() )					break;
			if( (*original)[i] > getMaxOnTime(iPiece) )						break;
			if( original->length(i, original->iMost()) < getMinLeftover() + minPieceSize )	break;
			if( (*original)[i] < getMinOnTime(iPiece) )						continue;
			if( original->length(0, i) < getMinLeftover() )					continue;
			for( j = i+1; j <= original->iMost(); ++j )
			{
				if( original->length(j, original->iMost()) < getMinLeftover() ) break;
				if( original->length(i, j) > maxPieceSize )						break;
				if( (*original)[j] > getMaxOffTime(0) )							break;
				if( original->length(i, j) < minPieceSize )						continue;
				if( (*original)[j] < getMinOffTime(0) )							continue;
				pc.insert( RPieceElement(desStartTime, getDesPieceSize(0), new RPiece(original->getI(i), original->getI(j)), FromMiddle) );
			}
		}
	}

	// Generate all the possibilities from the end.
	if( (cutFlags & FromEnd) &&
		TimeInterval(getMinOffTime(0), getMaxOffTime(0)).contains(original->endTime()) )
	{
		const	tod_t	desStartTime = original->endTime() - desPieceSize;
		j = original->iMost();
		for( i = j-1; i > 0; --i )
		{
			if( original->length(0, i) < getMinLeftover() )					break;
			if( original->length(i, j) > maxPieceSize )						break;
			if( (*original)[i] < getMinOnTime(0) )							break;
			if( (*original)[i] > getMaxOnTime(0) )							continue;
			if( (*original)[i] > getStartRunBeforeTime() )					continue;
			if( original->length(i, j) < minPieceSize )						continue;
			pc.insert( RPieceElement(desStartTime, getDesPieceSize(0), new RPiece(original->getI(i), original->getI(j)), FromEnd) );
		}
	}
}

tod_t	RecursiveCut::getTravelTime( tod_t &arriveTime, const RPiece &from, const RPiece &to ) const
{
	const	long	serviceID = from.serviceRecordID();
	long	lastNodeRecordID, firstNodeRecordID, equivalentTravelTime;
	tod_t	firstNodeTime;
	int		planLeave;
	if( !Reverse::reverse )
	{
		lastNodeRecordID = from.lastNodeRecordID();
		firstNodeRecordID = to.firstNodeRecordID();
		firstNodeTime = to.startTime();
		planLeave = 0;
	}
	else
	{
		lastNodeRecordID = to.firstNodeRecordID();
		firstNodeRecordID = from.lastNodeRecordID();
		firstNodeTime = AdjustTime(from.endTime());
		planLeave = 1;
	}

	tod_t	travelTime = 0, startTime = -1, endTime = -1;
	arriveTime = from.endTime();

	if( lastNodeRecordID != firstNodeRecordID )
	{
		tod_t	dwellTime;
    if(NodesEquivalent(lastNodeRecordID, firstNodeRecordID, &equivalentTravelTime))
    {
      travelTime = equivalentTravelTime;
      dwellTime = 0;
    }
    else
    {
  		if( bUseCISPlan )
	  	{
			  // If the serviceID is not valid, the trip planner will use the service it has.
  			CISplanReliefConnect(&startTime, &endTime, &dwellTime, 0,
  			lastNodeRecordID, firstNodeRecordID,
	  		firstNodeTime, planLeave, serviceID);
        if(startTime == -25 * 60 * 60)
        {
          travelTime = NO_TIME;
        }
        else
        {
    			travelTime = endTime - startTime;
        }
    		arriveTime = planLeave ? startTime : endTime;
	  	}
		  else if( bUseDynamicTravels )
  		{
#ifdef FIXLATER
	  		// I need to get the serviceID here correctly!
		  	travelTime = GetDynamicTravelTime(!planLeave, lastNodeRecordID,
			      	firstNodeRecordID, serviceID, lastNodeTime, &dwellTime);
  			arriveTime = planLeave ? (lastNodeTime + travelTime) : (lastNodeTime - travelTime);
#endif
      }
		}
		arriveTime = AdjustTime( arriveTime );
	}
	
	// Penalize invalid travel times.
	if( travelTime < 0 )
		travelTime = 4 * 24 * 60 * 60;

#ifdef DEBUG_RUNS
	ofs << "getTravelTime: arriveTime: " << tFormat(arriveTime) << " travelTime: " << tFormat(travelTime) << '\n';
#endif // DEBUG_RUNS

	return travelTime;
}


void	RecursiveCut::createCutPossibilities( RPieceCollection &pc,
												const tod_t start, // Start time of the run.
												Ref<RPiece> prev,	// Previous piece in the run.
												const tod_t minBreak, const tod_t desBreak, const tod_t maxBreak,
												Ref<RPiece> original, const int cutFlags )
{
	if( StatusBarAbort() )
		return;

	// Handle the case when this is *not* the first piece of the run.
	tod_t minPieceSize = getMinPieceSize(iPiece);
	tod_t desPieceSize = getDesPieceSize(iPiece);
	tod_t maxPieceSize = getMaxPieceSize(iPiece);

	register int	i, j;

	if( getNumPieces() > 1 )
	{
		tod_t	tMaxTotal = 0, tMinTotal = 0, tBefore = 0, tRemaining;
		for( i = 0; i < getNumPieces(); ++i )
		{
			if( i < iPiece )
				tBefore += curSF[i].piece->length();
			else if( i > iPiece )
			{
				tMaxTotal += getMaxPieceSize(i);
				tMinTotal += getMinPieceSize(i);
			}
		}

		// Check what the *real* minPieceSize can be.
		tRemaining = getMinPlatformTime() - tBefore - tMaxTotal;
		if( tRemaining > minPieceSize )
			minPieceSize = tRemaining;
		if( desPieceSize < minPieceSize )
			desPieceSize = minPieceSize;

		// Check if this is really hosed up.
		if( minPieceSize > maxPieceSize )
			return;

		// Check what the *real* maxPieceSize can be.
		tRemaining = getMaxPlatformTime() - tBefore - tMinTotal;
		if( tRemaining < maxPieceSize )
			maxPieceSize = tRemaining;
		if( desPieceSize > maxPieceSize )
			desPieceSize = maxPieceSize;

		// Check if this is really hosed up.
		if( minPieceSize > maxPieceSize )
			return;
	}

	// Check if we should try to take the whole piece.
	const tod_t	sepTime = original->startTime() - prev->endTime();
	tod_t	travelTime = -1, arriveTime = -1;
	if( (cutFlags & Entire) &&
		sepTime >= minBreak &&
		original->endTime() - start <= getMaxSpreadTime() &&
		original->length() >= minPieceSize &&
		original->length() <= maxPieceSize &&
		TimeInterval(getMinOnTime(iPiece), getMaxOnTime(iPiece)).contains(original->startTime()) &&
		TimeInterval(getMinOffTime(iPiece), getMaxOffTime(iPiece)).contains(original->endTime()) &&
		(travelTime = getTravelTime(arriveTime, *prev, *original)) + minBreak <= sepTime &&
		arriveTime <= original->startTime() &&
		sepTime - travelTime <= maxBreak )
	{
		const	tod_t	desStartTime = prev->endTime() + desBreak;
		pc.insert( RPieceElement(desStartTime, getDesPieceSize(iPiece), original, Entire) );
	}

	// Generate all the possibilities from the beginning.
	if( (cutFlags & FromFront) &&
		sepTime >= minBreak &&
		TimeInterval(getMinOnTime(iPiece), getMaxOnTime(iPiece)).contains(original->startTime()) &&
		(travelTime >= 0 ? travelTime : travelTime = getTravelTime(arriveTime, *prev, *original)) + minBreak <= sepTime &&
		arriveTime <= original->startTime() &&
		sepTime - travelTime <= maxBreak )
	{
		const	tod_t	desStartTime = prev->endTime() + desBreak;

		i = 0;
		for( j = i+1; j <= original->iMost(); ++j )
		{
			if( original->length(j, original->iMost()) < getMinLeftover() ) break;
			if( (*original)[j] - start > getMaxSpreadTime() )				break;
			if( original->length(i, j) > maxPieceSize )						break;
			if( (*original)[j] > getMaxOffTime(iPiece) )					break;
			if( original->length(i, j) < minPieceSize )						continue;
			if( (*original)[j] < getMinOffTime(iPiece) )					continue;
			pc.insert( RPieceElement(desStartTime, getDesPieceSize(iPiece), new RPiece(original->getI(i), original->getI(j)), FromFront) );
		}
	}

	// Generate all the possibilities from the middle.
	if( (cutFlags & FromMiddle) )
	{
		for( i = 1; i < original->iMost(); ++i )
		{
			if( (*original)[i] > getMaxOnTime(iPiece) )						break;
			if( original->length(i, original->iMost()) < getMinLeftover() + minPieceSize )	break;
			if( (*original)[i] < getMinOnTime(iPiece) )						continue;
			if( original->length(0, i) < getMinLeftover() )					continue;
			const tod_t	sepTime = (*original)[i] - prev->endTime();
			if( sepTime < minBreak )										continue;
			if( (travelTime = getTravelTime(arriveTime, *prev, RPiece(original->getI(i), original->getI(original->iMost())))) + minBreak > sepTime ) continue;
			if( arriveTime > (*original)[i] )								continue;
			if( sepTime - travelTime > maxBreak )							break;
			for( j = i+1; j <= original->iMost(); ++j )
			{
				if( original->length(j, original->iMost()) < getMinLeftover() ) break;
				if( (*original)[j] - start > getMaxSpreadTime() )				break;
				if( original->length(i, j) > maxPieceSize )						break;
				if( (*original)[j] > getMaxOffTime(iPiece) )					break;
				if( original->length(i, j) < minPieceSize )						continue;
				if( (*original)[j] < getMinOffTime(iPiece) )					continue;
				pc.insert( RPieceElement(prev->endTime() + desBreak + travelTime,
							getDesPieceSize(iPiece),
							new RPiece(original->getI(i), original->getI(j)), FromMiddle) );
			}
		}
	}

	// Generate all the possibilities from the end.
	if( (cutFlags & FromEnd) &&
		original->endTime() - start <= getMaxSpreadTime() &&
		TimeInterval(getMinOffTime(iPiece), getMaxOffTime(iPiece)).contains(original->endTime()) )
	{
		const	tod_t	desStartTime = original->endTime() - desPieceSize;

		j = original->iMost();
		for( i = j-1; i >= 0; --i )
		{
			const tod_t	sepTime = (*original)[i] - prev->endTime();
			if( sepTime < minBreak )										break;
			if( original->length(0, i) < getMinLeftover() )					break;
			if( original->length(i, j) > maxPieceSize )						break;
			if( (*original)[i] < getMinOnTime(iPiece) )						break;
			if( (*original)[i] > getMaxOnTime(iPiece) )						continue;
			if( original->length(i, j) < minPieceSize )						continue;
			if( (travelTime = getTravelTime(arriveTime, *prev, RPiece(original->getI(i), original->getI(original->iMost())))) + minBreak > sepTime ) continue;
			if( arriveTime > (*original)[i] )								continue;
			if( sepTime - travelTime > maxBreak )							continue;
			pc.insert( RPieceElement(desStartTime, getDesPieceSize(iPiece), new RPiece(original->getI(i), original->getI(j)), FromEnd) );
		}
	}
}

void	RecursiveCut::push( SFrame &sf, const bool breakOfTry )
{
	curSF[iPiece] = sf;

#ifdef DEBUG_RUNS
#ifdef FIXLATER
	ofs << "push:\n";
	ofs <<		"\tpiece          ="; PrintRPiece(ofs,curSF[iPiece].piece); ofs << '\n';
	if( curSF[iPiece].remaining() && curSF[iPiece].remaining->first() )
	{
		ofs <<	"\tremainingFirst =" << curSF[iPiece].remaining->first << '\n';
	}
	if( curSF[iPiece].remaining() && curSF[iPiece].remaining->second() )
	{
		ofs <<	"\tremainingSecond=" << curSF[iPiece].remaining->second << '\n';
	}
	ofs <<		"\toriginal       =" << curSF[iPiece].original << '\n';
#endif // FIXLATER
	ofs.flush();
#endif // DEBUG_RUNS

		// Stack the BreakOf flag so it will be restored when we backtrack.
	curSF[iPiece].setOldBreakOfHasOccurred( fBreakOfHasOccurred );
	if( !fBreakOfHasOccurred )
		fBreakOfHasOccurred = breakOfTry;
	curSF[iPiece].setNewBreakOfHasOccurred( fBreakOfHasOccurred );
	if( iPiece != getNumPieces()-1 )
	{
		erasePiece( sf.original );
		if( sf.remaining() )
		{
			insertPiece( sf.remaining->first );
			insertPiece( sf.remaining->second );
		}
	}
}
Pieces::iterator	RecursiveCut::pop()
{
#ifdef DEBUG_RUNS
#ifdef FIXLATER
	ofs << "pop:\n";
	ofs <<		"\tpiece          ="; PrintRPiece(ofs,curSF[iPiece].piece); ofs << '\n';
	if( curSF[iPiece].remaining() && curSF[iPiece].remaining->first() )
	{
		ofs <<	"\tremainingFirst =" << curSF[iPiece].remaining->first << '\n';
	}
	if( curSF[iPiece].remaining() && curSF[iPiece].remaining->second() )
	{
		ofs <<	"\tremainingSecond=" << curSF[iPiece].remaining->second << '\n';
	}
	ofs <<		"\toriginal       =" << curSF[iPiece].original << '\n';
#endif // FIXLATER
	ofs.flush();
#endif // DEBUG_RUNS

	SFrame	&sf = curSF[iPiece];

	// Restore the BreakOf flag.
	fBreakOfHasOccurred = curSF[iPiece].getOldBreakOfHasOccurred();
	Pieces::iterator i;
	if( iPiece != getNumPieces()-1 )
	{
		if( sf.remaining() )
		{
			erasePiece( sf.remaining->first );
			erasePiece( sf.remaining->second );
		}
		i = insertPiece( sf.original );
	}
	else
		i = findPiece( sf.original );

	sf.original = NULL;
	sf.remaining = NULL;

	return i;
}

RecursiveCut::RunDoneStatus		RecursiveCut::checkRunDone()
{
  int nI, nJ;

	// Check if any pieces are consecutive when they should not be.
	if( !getMultiPieceCanBeOne() &&
		iPiece > 0 &&
		curSF[iPiece-1].piece->endTime() == curSF[iPiece].piece->startTime() &&
		curSF[iPiece-1].piece->blockNumber() == curSF[iPiece].piece->blockNumber() )
		return PiecesMeet;
	
	// Check if we have enough pieces to cost out.
	if( iPiece != getNumPieces()-1 )
		return InsufficientPieces;
	
	// Check if this is a valid run.
	// Add the pieces to a RRun to ensure they are in the right order.
	tod_t	platformTime = 0;
	tod_t	cutPenaltyTotal = 0;
	RRun	r;
	register int k;
	for( k = 0; k < getNumPieces(); k++ )
	{
		r.addPiece( curSF[k].piece );
		cutPenaltyTotal += curSF[k].cutPenalty();
		platformTime += curSF[k].piece->length();
	}

#ifdef DEBUG_RUNS
	{
		static	int	checkRunDoneCount = 0;
		ofs << (++checkRunDoneCount) << ": checkRunDone: checking pieces:\n";
		for( int ip = 0; ip <= iPiece; ++ip )
			ofs << '\t' << curSF[ip].piece << '\n';
		ofs << "\tplatformTime=" << tFormat(platformTime) << '\n';
	}
#endif // DEBUG_RUNS

	// Do a quick check of the platform time before we bother the run coster.
	if( platformTime < getMinPlatformTime() || platformTime > getMaxPlatformTime() )
	{
#ifdef DEBUG_RUNS
		ofs << "\tRun does not meet minPlatformTime or maxPlatformTime criteria.\n";
#endif // DEBUG_RUNS
		return Invalid;
	}

	// Add the pieces to the PROPOSEDRUN structure.
	PROPOSEDRUN	pr;
	for( k = 0; k < getNumPieces(); k++ )
	{
		RPiece &p = *r.getPiece( k );
		pr.piece[k].fromTime = p.actualStartTime();
		pr.piece[k].fromNODESrecordID = p.getFromNodesRecordID();
		pr.piece[k].fromTRIPSrecordID = p.getFromTripsRecordID();
		
		pr.piece[k].toTime = p.actualEndTime();
		pr.piece[k].toNODESrecordID = p.getToNodesRecordID();
		pr.piece[k].toTRIPSrecordID = p.getToTripsRecordID();
	}
	pr.numPieces = getNumPieces();
	
	// Penalize the deviation from desired pay time.
	COSTDef	cost;
	const long	costedRunType = RunCoster( &pr, NO_RUNTYPE, &cost );
	RRun::incCostCount();
	if( RRun::getCostCount() % 20 == 0 )
	{
		sprintf(tempString, "Tries: %d  Runs cut: %d", RRun::getCostCount(), (int)runs.size() );
		StatusBarText(tempString);
	}

	if( costedRunType == runType && cost.TOTAL.payTime != 0 )
	{
#ifdef DEBUG_RUNS
		ofs << "\tRun is feasible!\n";
#endif // DEBUG_RUNS
		++beamSearchCount;

//		long weightedCost = cost.TOTAL.payTime - (getNumPieces() > 1 ? cost.TOTAL.platformTime : 0) + cutPenaltyTotal;
		long weightedCost = cost.TOTAL.payTime - cost.TOTAL.platformTime + cutPenaltyTotal;
		
		if( weightedCost < 0L )
			weightedCost = 0L;
		//
		//  Penalize travel time
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_TRAVEL)
		{
			long totalTravel = 0L;
			for(k = 0; k < getNumPieces(); k++)
			{
				totalTravel += cost.TRAVEL[k].startTravelTime + cost.TRAVEL[k].endTravelTime;
			}
			weightedCost += lSquare(totalTravel);
		}
		//
		//  Penalize MakeUp time
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_MAKEUP)
		{
			weightedCost += lSquare(lSquare(cost.TOTAL.makeUpTime));
		}
		//
		//  Penalize Overtime
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_OVERTIME)
		{
			weightedCost += lSquare(cost.TOTAL.overTime);
		}
		//
		//  Penalize Paid Breaks
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_PAIDBREAKS)
		{
			weightedCost += lSquare(cost.TOTAL.paidBreak);
		}
		//
		//  Penalize Spread Overtime
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_SPDOT)
		{
			weightedCost += lSquare(cost.spreadOvertime);
		}
		//
		//  Penalize cuts at particular relief points
		//
		if(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT)
		{
      for(nI = 0; nI < CUTPARMS.numPenalizedNodes; nI++)
      {
        for(nJ = 0; nJ < pr.numPieces; nJ++)
        {
          if(CUTPARMS.penalizedNODESrecordIDs[nI] == pr.piece[nJ].fromNODESrecordID)
            weightedCost += cost.TOTAL.platformTime;
          if(CUTPARMS.penalizedNODESrecordIDs[nI] == pr.piece[nJ].toNODESrecordID)
            weightedCost += cost.TOTAL.platformTime;
        }
      }
		}
		
		// Penalize the deviation from desired.
		double workVariance = 0.0;
		double breakVariance = 0.0;
		for( k = 0; k < getNumPieces(); k++ )
		{
			workVariance += dSquare( curSF[k].piece->length() - getDesPieceSize(k) );
			if( k + 1 < getNumPieces() )
				breakVariance += dSquare( curSF[k+1].piece->startTime() - curSF[k].piece->endTime() -
				curSF[k+1].getBreakOfHasOccurred() ? getDesBreakOf() : getDesBreakTime(k) );
		}
		
		// Scale the total deviation so it does not interfere too much with the cost.
		weightedCost += (long)((workVariance + breakVariance) / dSquare(12*60*60));
		
		// Save the best run found so far.
		if( !fFoundValidRun || weightedCost < costBest )
		{
			costBest = weightedCost;
			fFoundValidRun = true;
			for( k = 0; k < getNumPieces(); k++ )
				bestSF[k] = curSF[k];
		}
		return Valid;
	}
    else
	{
#ifdef DEBUG_RUNS
		ofs << "\tRun is INFEASIBLE: " << runcosterReason << '\n';
#endif // DEBUG_RUNS
        return Invalid;
	}
}

RRun	*RecursiveCut::getNextPiece( Ref<RPiece> p )
{
	// Enter the recursion.
	platformCur += p->length();
	minTotalFollowing -= getMinPieceSize(iPiece);
	++iPiece;

#define regularBreakTry()	(iTry == 0)
#define breakOfTry()		(iTry == 1)
#define	contiguous()		(p->endTime() - curSF[0].piece->startTime())

	// Define the stratgies to use in order to find a match.
	static int	cutStrategy[] = { Entire, FromFront, FromEnd, FromMiddle };

	// Try to find a match in order of the strateges we are looking for.
	if( iPiece < getNumPieces() )
	{
		tod_t	desLength = (getDesTotal() - platformCur) / (getNumPieces() - iPiece);
		if( desLength < getMinPieceSize(iPiece) )
			desLength = getMinPieceSize(iPiece);
		else if( desLength > getMaxPieceSize(iPiece) )
			desLength = getMaxPieceSize(iPiece);

		for( int c = 0; c < Number(cutStrategy) && keepSearching(); ++c )
		{
			for( int iTry = 0; iTry < 2 && keepSearching(); ++iTry )
			{
				tod_t	minBreak, desBreak, maxBreak;
				if( regularBreakTry() )
				{
					// Check if we have exceeded the maximum contiguous work time without the BreakOf.
					if( hasBreakOf() && !fBreakOfHasOccurred && contiguous() > getMaxContig() )
						break;
					minBreak = getMinBreakTime(iPiece-1);
					desBreak = getDesBreakTime(iPiece-1);
					maxBreak = getMaxBreakTime(iPiece-1);
				}
				else // breakOfTry()
				{
					if( !hasBreakOf() || fBreakOfHasOccurred )
						break;
					minBreak = getMinBreakOf();
					desBreak = getDesBreakOf();
					maxBreak = getMaxBreakOf();
				}

				Pieces::iterator pItr;
				ForAllInCollection( pieces, pItr )
				{
					if( !keepSearching() )
						break;

					Ref<RPiece>	pCur((*pItr).second);
					if( pCur->startTime() - curSF[iPiece-1].piece->endTime() > maxBreak )
						break;

					RPieceCollection	pieceCollection;
					createCutPossibilities( pieceCollection,
										curSF[0].piece->startTime(),
										curSF[iPiece-1].piece,
										minBreak, desBreak, maxBreak,
										pCur, cutStrategy[c] );
					RPieceCollection::iterator peItr;
					ForAllInCollection( pieceCollection, peItr )
					{
						Ref<RPiece>	pePiece((*peItr).piece);
						push( SFrame(pCur, pePiece, Ref<RPiecePair>(pCur->removePiece(*pePiece))), breakOfTry() );
						if( checkRunDone() == InsufficientPieces )
							getNextPiece( curSF[iPiece].piece );
						pItr = pop();
						if( !keepSearching() )
							break;
					}
				}
			}
		}
	}

	// Back out of the recursion.
	iPiece--;
	minTotalFollowing += getMinPieceSize(iPiece);
	platformCur -= p->length();

	// Check for success.
	if( iPiece == 0 && fFoundValidRun )
	{
		// Create a run from the pieces.
		RRun *r = new RRun;
		for( int i = 0; i < getNumPieces(); i++ )
		{
			// Update the remaining pieces to reflect the removed piece.
			if( bestSF[i].original() )
			{
				erasePiece( bestSF[i].original );
				if( bestSF[i].remaining() )
				{
					insertPiece( bestSF[i].remaining->first );
					insertPiece( bestSF[i].remaining->second );
				}
			}
			// Add this piece to the run.
			r->addPiece( bestSF[i].piece );
		}
		r->setCost( costBest );
		return r;
	}

	return NULL;
}

#ifdef DEBUG_RUNS
static void printReliefPoint( const long i )
{
#ifdef FIXLATER
	ofs << i << ":\t"
		<< m_pRELIEFPOINTS[i].start.recordID << '\t'
		<< m_pRELIEFPOINTS[i].start.runNumber << '\t'
		<< m_pRELIEFPOINTS[i].start.pieceNumber << '\t'

		<< m_pRELIEFPOINTS[i].end.recordID << '\t'
		<< m_pRELIEFPOINTS[i].end.runNumber << '\t'
		<< m_pRELIEFPOINTS[i].end.pieceNumber << '\t'
		<< tFormat(m_pRELIEFPOINTS[i].time) << '\n';
	ofs.flush();
#endif // FIXLATER
}
#endif

void	RPiece::install( const long runNumber, const long pieceNumber ) const
{
	m_pRELIEFPOINTS[iStart].start.recordID = NO_RECORD;
	m_pRELIEFPOINTS[iStart].start.runNumber = runNumber;
	m_pRELIEFPOINTS[iStart].start.pieceNumber = pieceNumber;

#ifdef DEBUG_RUNS
	printReliefPoint(iStart);
#endif // DEBUG_RUNS

	for( register int i = iStart + 1; i < iEnd; i++ )
	{
	    m_pRELIEFPOINTS[i].start.recordID = NO_RECORD;
		m_pRELIEFPOINTS[i].start.runNumber = runNumber;
		m_pRELIEFPOINTS[i].start.pieceNumber = pieceNumber;

		m_pRELIEFPOINTS[i].end.recordID = NO_RECORD;
		m_pRELIEFPOINTS[i].end.runNumber = runNumber;
		m_pRELIEFPOINTS[i].end.pieceNumber = pieceNumber;

#ifdef DEBUG_RUNS
		printReliefPoint(i);
#endif // DEBUG_RUNS
	}

	m_pRELIEFPOINTS[iEnd].end.recordID = NO_RECORD;
	m_pRELIEFPOINTS[iEnd].end.runNumber = runNumber;
	m_pRELIEFPOINTS[iEnd].end.pieceNumber = pieceNumber;

#ifdef DEBUG_RUNS
	printReliefPoint(iEnd);
#endif // DEBUG_RUNS
}

void	RRun::install( const long aRunNumber )
{
	runNumber = aRunNumber;

	// Install the pieces in increasing time order.
	for( long k = 0; k < numPieces; k++ )
		pieces[k]->install( runNumber, k+1 );
}

void	RPiece::installCheck( const long runNumber, const long pieceNumber ) const
{
#ifdef DEBUG_RUNS
	Assert( m_pRELIEFPOINTS[iStart].start.runNumber == runNumber );
	Assert( m_pRELIEFPOINTS[iStart].start.pieceNumber == pieceNumber );

	for( register int i = iStart + 1; i < iEnd; i++ )
	{
		Assert( m_pRELIEFPOINTS[i].start.runNumber == runNumber );
		Assert( m_pRELIEFPOINTS[i].start.pieceNumber == pieceNumber );

		Assert( m_pRELIEFPOINTS[i].end.runNumber == runNumber );
		Assert( m_pRELIEFPOINTS[i].end.pieceNumber == pieceNumber );
	}

	Assert( m_pRELIEFPOINTS[iEnd].end.runNumber == runNumber );
	Assert( m_pRELIEFPOINTS[iEnd].end.pieceNumber == pieceNumber );
#endif // DEBUG_RUNS
}

void	RRun::installCheck() const
{
#ifdef DEBUG_RUNS
	// Install the pieces in increasing time order.
	for( long k = 0; k < numPieces; k++ )
		pieces[k]->installCheck( runNumber, k+1 );
#endif // DEBUG_RUNS
}

bool	RRun::intersects( const RRun &r ) const
{
	for( register int i = 0; i < numPieces; ++i )
		for( register int j = 0; j < r.numPieces; ++j )
			if( pieces[i]->intersects(*r.pieces[j]) )
			{
				Assert( 0 );
				return true;
			}
	return false;
}

void	RecursiveCut::checkRuns() const
{
#ifdef DEBUG_RUNS
	// Check that pieces do not intersect with themselves.
	{
		for( Pieces::iterator pItr1 = pieces.begin(); pItr1 != pieces.end(); ++pItr1 )
		{
			const	tod_t	lengthCur = (*pItr1).second->length();
			const	tod_t	lengthMin = getMinLeftover();
			Assert( lengthCur >= lengthMin );

			Pieces::iterator pItr2 = pItr1;
			for( ++pItr2; pItr2 != pieces.end(); ++pItr2 )
			{
				if( (*pItr1).second->intersects( *(*pItr2).second) )
				{
					ofs << "Intersection between pieces:\n";
					ofs << '\t' << (*pItr1).second << '\n';
					ofs << '\t' << (*pItr2).second << '\n';
				}
				Assert( !(*pItr1).second->intersects( *(*pItr2).second) );
			}
		}
	}

	// Check that the runs do not intersect with any remaining pieces.
	{
		for( Pieces::iterator pItr = pieces.begin(); pItr != pieces.end(); ++pItr )
			for( Runs::iterator rItr = runs.begin(); rItr != runs.end(); ++rItr )
				for( int i = 0; i < (*rItr)->getNumPieces(); ++i )
				{
					if( (*rItr)->getPiece(i)->intersects(*(*pItr).second) )
					{
						ofs << "Intersection between run and pieces:\n";
						ofs << '\t' << (*rItr)->getPiece(i) << '\n';
						ofs << '\t' << (*pItr).second << '\n';
					}
					Assert( !(*rItr)->getPiece(i)->intersects(*(*pItr).second) );
				}
	}

	// Check that the runs do not intersect with themselves.
	{
		for( Runs::iterator rItr1 = runs.begin(); rItr1 != runs.end(); ++rItr1 )
		{
			Runs::iterator rItr2 = rItr1;
			for( ++rItr2; rItr2 != runs.end(); ++rItr2 )
			{
				Assert( !(*rItr1)->intersects(*(*rItr2)) );
			}
		}
	}
#endif // DEBUG_RUNS
}

bool	RecursiveCut::cutTrippersComplete( Ref<RPiece> pOriginal, const int numToCut )
{
	if( StatusBarAbort() )
		return false;

	// Try to cut the runs simultaneously from the piece.

	// Set of a series of adjustments we are prepared to make to find a feasible run.
	static	const int	adjustment[9] = { 0, -1, 1, -2, 2, -3, 3, -4, 4 };

	static	RRun **totalRuns = NULL;
	static	int		iTotalRunsMax = 0;

	if( numToCut > iTotalRunsMax )
	{
		delete [] totalRuns;
		totalRuns = new RRun * [iTotalRunsMax = numToCut * 2];
	}
	register int i;
	for( i = 0; i < numToCut; ++i )
		totalRuns[i] = NULL;
	int	iTotalRuns = 0;

	// Try to cut so there are no leftovers.
	bool	success = true;
	register int a = 0, b;
	for( i = 0; i < numToCut && success; a = b, ++i )
	{
		success = false;

		if( i == numToCut - 1 )
			b = pOriginal->iMost();
		else
		{
			const	tod_t	idealCur = pOriginal->length(a, pOriginal->iMost()) / (numToCut - i);
			if( a >= pOriginal->iMost() )
				break;
			for( b = a + 1; b < pOriginal->iMost() && pOriginal->length(a, b) < idealCur; ++b )
				continue;
			if( b > pOriginal->iMost() )
				break;
		}

		// Try to find a valid run around the ideal.
		for( int j = 0; j < Number(adjustment); ++j )
		{
			// Check if we are trying to adjust the last one - not allowed!
			if( adjustment[j] != 0 && i == numToCut - 1 )
				break;

			const int bNew = b + adjustment[j];

			// Check if we are in bounds.
			if( bNew <= a )											continue;
			if( bNew > pOriginal->iMost() )							continue;

			// Check that this run meets all the criteria.
			if( (*pOriginal)[a] > getStartRunBeforeTime() )			break;
			if( pOriginal->length(a, bNew) < getMinPieceSize(0) )	continue;
			if( pOriginal->length(a, bNew) > getMaxPieceSize(0) )	continue;

			// Check if this run costs out.
			RRun	*r = new RRun;
			r->addPiece( new RPiece(pOriginal->getI(a), pOriginal->getI(bNew)) );
			if( r->getCostedRunType() != runType )	{ delete r; continue; }

			// Party time!  We got this far!
			totalRuns[iTotalRuns++] = r;
			success = true;
			b = bNew;
			break;
		}
	}

	if( i == numToCut && a == pOriginal->iMost() && success )
	{
		// We successfully cut this piece with no leftover.
		for( i = 0; i < iTotalRuns; ++i )
		{
			runs.insert( totalRuns[i] );
			// Install this run under a temporary number so we do not try to cut it again.
			totalRuns[i]->install( (1<<30)-1 );
		}
		erasePiece( pOriginal );
	}
	else
	{
		// Cleanup and leave the original piece intact.
		for( i = 0; i < iTotalRuns; ++i )
			delete totalRuns[i];
	}

	return success;
}

bool	RecursiveCut::cutTrippersComplete( Ref<RPiece> pOriginal )
{
	if( StatusBarAbort() )
		return false;

	// Check for the special case when we have to cut the entire block and leave nothing behind.
	// This is indicated when we are trying to a run with one piece, the minLeftover is less than minPieceSize,
	// and the piece cannot be cut as one run already.
	if( getNumPieces() != 1 )						return false;
	if( getMinLeftover() < getMinPieceSize(0) )		return false;
	if( pOriginal->length() <= getMaxPieceSize(0) )	return false;

	// Check if any of the pieces would violate the user's given time boundary.
	if( pOriginal->endTime() - getMaxPieceSize(0) > getStartRunBeforeTime() )
		return false;

	// Try to figure out how many runs to cut from this piece.
	int	numToCut = pOriginal->length() / getDesPieceSize(0);
	if( numToCut < 2 )	// We have to try for at least two.
		numToCut = 2;
	tod_t	idealPieceSize = pOriginal->length() / numToCut;
	if( idealPieceSize > getMaxPieceSize(0) )
	{
		++numToCut;
		idealPieceSize = pOriginal->length() / numToCut;
		if( idealPieceSize < getMinPieceSize(0) )
			return false;	// There is no way to perfectly cut this piece.
	}
	else if( idealPieceSize < getMinPieceSize(0) )
	{
		if( --numToCut < 2 )
			return 0;
		idealPieceSize = pOriginal->length() / numToCut;
		if( idealPieceSize > getMaxPieceSize(0) )
			return false;	// There is no way to perfectly cut this piece.
	}

	// Try varying adjustments to the number of trippers to cut from this piece.
	// We are trying to find the right one that leaves nothing left over.
	bool	success = false;
	for( int failCount = 0, j = 0; failCount < 2 && !StatusBarAbort(); j = j >= 0 ? -j-1 : -j )
	{
		++failCount;

		const int numToCutCur = numToCut + j;
		if( numToCutCur < 2 )		continue;

		idealPieceSize = pOriginal->length() / numToCutCur;

		if( idealPieceSize < getMinPieceSize(0) )	continue;
		if( idealPieceSize > getMaxPieceSize(0) )	continue;

		if( success = cutTrippersComplete(pOriginal, numToCutCur) )
			break;

		failCount = 0;
	}

	return success;
}

struct	FirstPiece
{
	FirstPiece( const long aBlock, const tod_t aTime ) : block(aBlock), t(aTime) {}

	bool operator==( const FirstPiece &sp ) const { return block == sp.block && t == sp.t; }

	size_t	hash() const { return (block << 16) ^ t; }

	long	block;
	tod_t	t;
};

typedef HashObjFI<FirstPiece, int>	FirstPieceCollection;

void	RecursiveCut::recursiveRuncut()
{
	initPieces();
	checkRuns();
	size_t	lastRunsCut = 0;

	FirstPieceCollection	cannotBeFirstPiece;
	while( !StatusBarAbort() )
	{
		// Tell the user what is going on.
		if( lastRunsCut != runs.size() )
		{
			sprintf(tempString, "Tries: %d  Runs cut: %d", RRun::getCostCount(), (int)runs.size() );
			StatusBarText(tempString);
			lastRunsCut = runs.size();
		}
					
		// Try to find a valid first piece.
		Ref<RPiece>	pOriginal;
		Pieces::iterator pItr;
		ForAllInCollection( pieces, pItr )
		{
			pOriginal = (*pItr).second;
			if( !cannotBeFirstPiece.contains(FirstPiece(pOriginal->blockNumber(), pOriginal->startTime())) )
				break;
		}
		if( pItr == pieces.end() || StatusBarAbort() )
			break;

		// Consider the first piece.

		// Reset the search count.
		beamSearchCount = 0;

		// Only cut up to the given StartRunBeforeTime.
		if( pOriginal->startTime() > getStartRunBeforeTime() )
			break;

		// If it is too small, ignore it.
		if( pOriginal->length() < getMinPieceSize(0) )
		{
			erasePiece( pOriginal );
			continue;
		}

		// Check if we are cutting trippers and we were able to cut the entire piece.
		if( cutTrippersComplete(pOriginal) )
			continue;

		// Try to cut this piece into a run.
		RRun	*r = NULL;

		// Try cutting this run the usual recursive way.
		if( r == NULL )
		{
			// First, make a list of prioritized potential pieces to try.
			RPieceCollection	pieceCollection;
			createCutPossibilities(pieceCollection, pOriginal, Entire | FromFront | FromMiddle | FromEnd);
			
			RPieceCollection::iterator peItr;
			ForAllInCollection( pieceCollection, peItr )
			{
				if( StatusBarAbort() )
					break;

				// Now we have the first piece of the run.  See if we can find matching pieces.
				Ref<RPiece>	pePiece = (*peItr).piece;
				push( SFrame(pOriginal, pePiece, Ref<RPiecePair>(pOriginal->removePiece(*pePiece))) );
				
				// Prepare stats for subsequent cuts.
				minTotalFollowing = 0;
				for( register int k = 0; k < getNumPieces(); k++ )
					minTotalFollowing += getMinPieceSize(k);
				fBreakOfHasOccurred = false;
				
				// Check if we are already done on the first cut.
				if( fFoundValidRun = (getNumPieces() == 1 && checkRunDone() == Valid) )
					bestSF[0] = curSF[0];
				
				// Get subsequent pieces if there are any to get.  If the current run is valid,
				// this will create a run.
				if( (r = getNextPiece(pePiece)) != NULL )
				{
					runs.insert( r );
					r->install( (1<<30)-1 );	// Install this run under a temporary number so we do not try to cut it again.
					
#ifdef DEBUG_RUNS
					MessageBeep( MB_OK );
					ofs << "******** Cut run: " << runs.size() << '\n';
					ofs.flush();
#endif // DEBUG_RUNS
					
					break;						// Keep this run.
				}
				
				// We have failed to cut this piece.
				pop();
				checkRuns();
			}
		}
		
		if( r == NULL )
		{
			// We have failed to cut from this block.
			// Restore the run pieces.
#ifdef DEBUG_RUNS
			ofs << "Cannot cut first piece from block: " << pOriginal->blockNumber() << '\n';
#endif // DEBUG_RUNS

			// We have proven that this block cannot contain the first piece of a run.
			// Flag it so it cannot be a first piece again.
			cannotBeFirstPiece.insert( FirstPiece(pOriginal->blockNumber(), pOriginal->startTime()), 1 );
		}

		checkRuns();
	}

	// Reset the piece stacks.
	for( int i = 0; i < getNumPieces(); i++ )
	{
		bestSF[i].reset();
		curSF[i].reset();
	}
}

void	RecursiveCut::runcut()
{
	runTypeHash.clear();
	RRun::resetCostCount();

	if( CUTPARMS.cutRuns == 1 )	// User has selected to end after a certain time.
	{
		// Cut from the back to finish after the endTime.
		Reverse::setBackward();
		startRunBeforeTime = AdjustTime(CUTPARMS.endTime);
	}
	else						// User has selected to start before a certain time.
	{
		// Cut from the front to commence before the startTime.
		Reverse::setForward();
		startRunBeforeTime = AdjustTime(CUTPARMS.startTime);
	}

	fMultiPieceCanBeOne = false;

	register int	i, j, k;
	// First, register all the runTypes we will be cutting in this run.
	for( i = 0; i < NUMRUNTYPES; ++i )
	{
		for( j = 0; j < NUMRUNTYPESLOTS; ++j )
		{
			if( (CUTPARMS.runtypes[i][j])
					&& (RUNTYPE[i][j].flags & RTFLAGS_INUSE))
			{
                runType	= MAKELONG(i,j);
				runTypeHash.insertKey( runType ); // Keep track of the runtypes we added.
            }
		}
	}

	// Now, actually cut the runs the user is looking for.
	for( i = 0; i < NUMRUNTYPES && !StatusBarAbort(); ++i )
	{
		for( j = 0; j < NUMRUNTYPESLOTS && !StatusBarAbort(); ++j )
		{
			if( (CUTPARMS.runtypes[i][j])
					&& (RUNTYPE[i][j].flags & RTFLAGS_INUSE))
			{
                runType	= MAKELONG(i,j);
				curRunType = &(RUNTYPE[i][j]);
				
				if( getNumPieces() == 1 )
					fMultiPieceCanBeOne = TRUE;
				else
					fMultiPieceCanBeOne = FALSE;

				desTotal = 0;
				for( k = 0; k < getNumPieces(); ++k )
					desTotal += getDesPieceSize(k);
				recursiveRuncut();
            }
		}
	}

    if( !StatusBarAbort() && (CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE))
	    improveTwoPieceRuns();
	
	Runs::iterator rItr;
	for( rItr = runs.begin(); rItr != runs.end(); ++rItr )
	{
#ifdef DEBUG_RUNS
		ofs << "----------------------------------------------------------------\n";
		ofs << m_GlobalRunNumber << ":\n";
		long platTime = 0;
		for( register int k = 0; k < (*rItr)->getNumPieces(); ++k )
		{
			ofs << '\t' << (*rItr)->getPiece(k)->blockNumber()
				<< '\t' << tFormat((*rItr)->getPiece(k)->actualStartTime())
				<< '\t' << tFormat((*rItr)->getPiece(k)->actualEndTime())
				<< '\t' << tFormat((*rItr)->getPiece(k)->actualEndTime() - (*rItr)->getPiece(k)->actualStartTime()) << '\n';
			platTime += (*rItr)->getPiece(k)->actualEndTime() - (*rItr)->getPiece(k)->actualStartTime();
		}
		ofs << "\tplatTime=" << tFormat(platTime) << '\n';
		ofs.flush();
#endif // DEBUG_RUNS
		
		(*rItr)->install( m_GlobalRunNumber++ );
	}
	
#ifdef DEBUG_RUNS
	for( rItr = runs.begin(); rItr != runs.end(); ++rItr )
		(*rItr)->installCheck();
#endif // DEBUG_RUNS
  
	runs.erase( runs.begin(), runs.end() );
}

extern "C"
{

void RecursiveRuncut()
{
#ifdef DEBUG_RUNS
	pofs = new ofstream( "runDebug.txt" );
	Timer	timer;
#endif // DEBUG_RUNS

	long	GlobalRunNumberSave;
	do
	{
		// Keep calling the runcutter as long as it is successful.  FIXLATER.
		GlobalRunNumberSave = m_GlobalRunNumber;

#ifdef DEBUG_RUNS
		ofs << "RecursiveRuncut: about to call runcut.  m_GlobalRunNumber=" << m_GlobalRunNumber << '\n';
#endif // DEBUG_RUNS

		RecursiveCut rc; rc.runcut();

#ifdef DEBUG_RUNS
		ofs << "RecursiveRuncut: called runcut.  m_GlobalRunNumber=" << m_GlobalRunNumber << '\n';
#endif // DEBUG_RUNS

		
		break; //*****************************************************************
	} while( GlobalRunNumberSave != m_GlobalRunNumber && !StatusBarAbort() );

#ifdef DEBUG_RUNS
	ofs << "Runcut time: " << timer.elapsed() << '\n';
	delete pofs; pofs = NULL;
#endif // DEBUG_RUNS

//
//  Try to improve the runs that we have cut.
//
	long savings;
    if( !StatusBarAbort() && (CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE) )
      RuncutImprove( &savings );
}

void ChainsawRuncut()
{
	// Cut all runs in small increments from both directions.
	const int		cutRunsSave = CUTPARMS.cutRuns;
	const tod_t	endTimeSave = CUTPARMS.endTime;
	const tod_t	startTimeSave = CUTPARMS.startTime;

	const	tod_t	inc = 10 * 60; // increments of every 10 minutes.

	// Get the earliest and latest range to cut.
	tod_t	earliest, latest;
	{
		RecursiveCut rc;
		rc.getPieceTimeRange( earliest, latest );
	}

	long	GlobalRunNumberSave;
	while( earliest <= latest && !StatusBarAbort() )
	{
		CUTPARMS.cutDirection = PARAMETERS_BACK;
		CUTPARMS.endTime = latest;
		do
		{
			// Keep calling the runcutter as long as it is successful.  FIXLATER.
			GlobalRunNumberSave = m_GlobalRunNumber;
			RecursiveCut rc; rc.runcut();
		} while( GlobalRunNumberSave != m_GlobalRunNumber && !StatusBarAbort() );

		CUTPARMS.cutDirection = PARAMETERS_FRONT;
		CUTPARMS.startTime = earliest;

		if( StatusBarAbort() )
			break;

		do
		{
			// Keep calling the runcutter as long as it is successful.  FIXLATER.
			GlobalRunNumberSave = m_GlobalRunNumber;
			RecursiveCut rc; rc.runcut();
		} while( GlobalRunNumberSave != m_GlobalRunNumber && !StatusBarAbort() );

		earliest += inc;
		latest -= inc;
	}

	CUTPARMS.endTime = endTimeSave;
	CUTPARMS.startTime = startTimeSave;
	CUTPARMS.cutDirection = cutRunsSave;

//
//  Try to improve the runs that we have cut.
//
	long savings;
    if( !StatusBarAbort() && (CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE) )
      RuncutImprove( &savings );
}
} // extern "C"