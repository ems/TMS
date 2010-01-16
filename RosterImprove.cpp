
#include "RosterImprove.h"
#include "DOCRoster.h"

//#define DEBUG_PRINT

inline bool bitSet( const unsigned int mask, const int bit ) { return (mask & (1<<bit)) != 0; }

static bool bitsConsecutive( register unsigned int pattern )
{
	// Check if pattern is already a lsb-justified run of consecutive bits (i.e. is one less than a power of 2).
	if( ((pattern + 1) & pattern) == 0 )
		return true;

	// Rotate the first day-off to the first bit.
	while( pattern & 1 )
		pattern = ((pattern>>1) | (1<<6));

	// Skip over the string of days-off.
	while( !(pattern&1) )
		pattern >>= 1;

	// Check if the result is a lsb-justified run of consecutive bits (i.e. is one less than a power of 2).
	return ((pattern + 1) & pattern) == 0;
}

static bool cmpSwapPermutation( const unsigned int p1, const unsigned int p2 )
{
	const int b1 = bitCount(p1), b2 = bitCount(p2);
	if( b1 < b2 ) return true; if( b2 < b1 ) return false;

	const bool c1 = bitsConsecutive(p1), c2 = bitsConsecutive(p2);
	if( c1 & !c2 ) return true; if( !c1 & c2 ) return false;

	return p1 < p2;
}

static int getFirstWorkingDayFollowingDayOff( const unsigned int pCur )
{
	// Find the first working day following a day off.
	register int	dBegin = 0;

	while( bitSet(pCur, dValid(dBegin-1)) || !bitSet(pCur, dBegin) )
		++dBegin;

	return dBegin;
}

void	RosterImprove::update( Roster &r )
{
	r.cost.cost = 0;
	r.cost.numInfeasibleConnections = 0;
	r.cost.nightWorkViolations = 0;
	r.paidWorkTotal = 0;
	r.pattern = 0;
	r.flags = 0;
	r.cost.isArtificial = false;

	if( r.empty() )
	{
		if(      r.paidWorkTotal < minPaidWork )
		{
			r.cost.lengthViolation = r.cost.minViolation = minPaidWork - r.paidWorkTotal;
			r.cost.maxViolation = 0;
			r.flags |= Roster::violatesMinPaidWork;
		}

		if( !validPatterns[r.getPattern()] )
		{
			r.flags |= Roster::violatesPatterns;
			// FIXLATER -  assume that every pattern contains the same number of work days.

			if( !patterns.empty() && r.size() != (unsigned int)workDays )
				r.flags |= Roster::violatesWorkDays;
		}

		r.cost.isArtificial = true;
		return;
	}

	register int numNightWork = 0;
	register int dCur = r.getFirstWorkday(), dNext;
	const int dFirst = dCur;
	r.paidWorkTotal = r.work[dFirst]->paidWork;
	if( r.work[dFirst]->isNightWork )
		++numNightWork;

	cost_t	cost;
	while( (dNext = r.getNextWorkday(dCur)) != dFirst )
	{
		if( r.work[dNext]->isNightWork )
			++numNightWork;

		r.paidWorkTotal += r.work[dNext]->paidWork;
		if( r.work[dCur]->getCostTo(cost, *r.work[dNext]) )
			r.cost.cost += cost;
		else
			++r.cost.numInfeasibleConnections;
		dCur = dNext;
	}

	if( !r.hasOnlyOne() && r.work[dCur]->getCostTo(cost, *r.work[dFirst]) )
		r.cost.cost += cost;
	else
		++r.cost.numInfeasibleConnections;

	// Set flags to record the roster status.
	if(      r.paidWorkTotal < minPaidWork )
	{
		r.cost.lengthViolation = r.cost.minViolation = minPaidWork - r.paidWorkTotal;
		r.cost.maxViolation = 0;
		r.flags |= Roster::violatesMinPaidWork;
	}
	else if( r.paidWorkTotal > maxPaidWork )
	{
		r.cost.lengthViolation = r.cost.maxViolation = r.paidWorkTotal - maxPaidWork;
		r.cost.minViolation = 0;
		r.flags |= Roster::violatesMaxPaidWork;
	}
	else
	{
		r.cost.lengthViolation = r.cost.minViolation = r.cost.maxViolation = 0;
	}

	if( (r.cost.diffFromAverage = r.paidWorkTotal - averageDayPaidWork * r.size()) < 0 )
		r.cost.diffFromAverage = -r.cost.diffFromAverage;

	// Classify the feasibility of the roster.
	if( !validPatterns[r.getPattern()] )
	{
		r.flags |= Roster::violatesPatterns;
		r.cost.isArtificial = true;
		// FIXLATER -  assume that every pattern contains the same number of work days.

		if( !patterns.empty() && r.size() != (unsigned int)bitCount(patterns[0]) )
			r.flags |= Roster::violatesWorkDays;
	}

	if( r.cost.numInfeasibleConnections > 0 )
		r.flags |= Roster::containsInvalidCombinations;

	// Check for night work violations.
	if( numNightWork > maxNightWork )
	{
		r.flags |= Roster::violatesNightWork;
		r.cost.nightWorkViolations = numNightWork - maxNightWork;
	}
}

RosterImprove::~RosterImprove()
{
	for( int d = 0; d < DayOfWeekMax; ++d )
		delete_contents_second( workDay[d] );
	delete_contents( rosters );
}


RosterImprove::RosterImprove() : minPaidWork(0), maxPaidWork(24*60*60*100),
								workDays(0),
								maxNightWork(7),
								iPartitionMax(0), equalize(false), preferExtrasOnWeekends(false)
{
	// Initialize the swap permutations.
	unsigned int *pSwap = swapPermutation;
	for( unsigned int p = 1; p <= 126; ++p )
		*pSwap++ = p;
	*pSwap = 0;
	std::sort( swapPermutation, pSwap, &cmpSwapPermutation );
}

void RosterImprove::partition()
{
#ifdef FIXLATER
	register int d;

	// Reset all the work partitions.
	register WorkDay::iterator w, wEnd;
	register ConnectionHash::iterator c, cEnd;
	for( d = 0; d < DayOfWeekMax; ++d )
	{
		for( w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
			(*w).second->iPartition = 0;
	}
	iPartitionMax = 1;
#endif

	// Partition the work into disjoint sets.
	register int d;

	// Reset all the work partitions.
	register WorkDay::iterator w, wEnd;
	register ConnectionHash::iterator c, cEnd;
	for( d = 0; d < DayOfWeekMax; ++d )
	{
		for( w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
			(*w).second->iPartition = -1;
	}

	// Merge all the work into disjoint sets.
	
	for( iPartitionMax = 0; ; ++iPartitionMax )
	{
		// Find an unmatched piece of work.
		for( d = 0; d < DayOfWeekMax; ++d )
		{
			for( w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
			{
				if( (*w).second->iPartition < 0 )
					goto FoundUnmatched;
			}
		}

		break;

FoundUnmatched:

		// Follow a breadth-first search tree from the starting work.
		WorkList	bfs;
		(*w).second->iPartition = iPartitionMax;
		bfs.push_back( (*w).second );

		while( !bfs.empty() )
		{
			Work	&wFrom = **bfs.begin();
			bfs.erase( bfs.begin() );
			for( register int iOffset = 0; iOffset < 6; ++iOffset )
			{
				for( c = wFrom.out[iOffset].begin(), cEnd = wFrom.out[iOffset].end(); c != cEnd; ++c )
				{
					if( (*c).second.to->iPartition < 0 )
					{
						Work	&wTo = *(*c).second.to;
						wTo.iPartition = iPartitionMax;
						bfs.push_back( &wTo );
					}
				}
			}
		}
	}
}

bool		RosterImprove::trySwapRostersAll( Roster &r1, Roster &r2, const bool doAll )
{
	if( r1.cost.isArtificial && r2.cost.isArtificial )
		return false;

#define GOOD 0
#define COST 1
#define BAD	 2
	static const int actions[4][4] =
	{
		{COST,	BAD,	BAD,	BAD},
		{GOOD,	COST,	BAD,	BAD},
		{GOOD,	BAD,	COST,	BAD},
		{GOOD,	GOOD,	GOOD,	COST}
	};

	Roster		r1Best = r1, r2Best = r2, r1New, r2New;
	RosterCost	newCostTotal, bestCostTotal;

	bestCostTotal = r1Best.cost + r2Best.cost;

	// Consider all possible ways to swap work between these rosters.
	const unsigned int pMin = (doAll ? 1 : fastRand(125) + 1 /* (1<<fastRand(6))*/ );
	const unsigned int pMax = (doAll ? 126 : pMin);
	for( register unsigned int pCur = pMin; pCur <= pMax; ++pCur )
	{
		// Create new rosters according to this swap pattern.
		for( register int d = 0; d < DayOfWeekMax; ++d )
		{
			if( bitSet(pCur,d) )
			{
				r1New.work[d] = r2Best.work[d];
				r2New.work[d] = r1Best.work[d];
			}
			else
			{
				r1New.work[d] = r1Best.work[d];
				r2New.work[d] = r2Best.work[d];
			}
		}

		// Ignore rosters that have invalid patterns..
		if( !r1Best.cost.isArtificial && !validPatterns[r1New.getPattern()] )
			continue;
		if( !r2Best.cost.isArtificial && !validPatterns[r2New.getPattern()] )
			continue;

		// Compute the costs of the new rosters.
		update( r1New );
		update( r2New );

		switch( actions	[(r1Best.isFeasible() ? 0 : 2) + (r2Best.isFeasible() ? 0 : 1)]
						[(r1New.isFeasible()  ? 0 : 2) + (r2New.isFeasible()  ? 0 : 1)] )
		{
		case COST:
			newCostTotal = r1New.cost + r2New.cost;
			if( !cmpLT(newCostTotal, bestCostTotal) )
				break;

		case GOOD:
			r1Best = r1New;
			r2Best = r2New;
			bestCostTotal = r1Best.cost + r2Best.cost;
			break;

		case BAD:
			break;
		}
	}

	if( r1Best != r1 )
	{
		costTotal -= r1.cost;
		costTotal -= r2.cost;

		r1 = r1Best;
		r2 = r2Best;

		costTotal += r1.cost;
		costTotal += r2.cost;
		return true;
	}

	return false;

#undef GOOD
#undef COST
#undef BAD
}

void RosterImprove::swapImproveSubproblem( WorkDay workDay[], RosterVec &rosters )
{
	// Compute the total violation and cost of all the rosters.
	costTotal.reset();
	register RosterVec::iterator r, rEnd;
	for( r = rosters.begin(), rEnd = rosters.end(); r != rEnd; ++r )
		costTotal += (*r)->cost;

	const RosterCost zeroRosterCost;
	if( costTotal == zeroRosterCost )	// Nothing to do!
		return;

	// Then, improve the rosters as much as possible.

	const size_t numRosters = rosters.size();
	size_t iCount = 0;

	// Do random swaps to improve the solution.
	fastRand.seed( 0xed1 );
	size_t	lastImprovement = 0;
	register RosterVec::iterator r1, r2;

	if( numRosters > 1 )
	{
		for( ;; ++iCount )
		{
			size_t	i = fastRand( numRosters ), j;
			while( (j = fastRand(numRosters)) == i );

			if( trySwapRostersAll(*rosters[i], *rosters[j]) )
				lastImprovement = iCount;

#ifdef DEBUG_PRINT
			if( (iCount & ((1<<12)-1)) == 0 )
			{
				std::cerr << "R ";
				costTotal.printOn( std::cerr );
			}
#endif

			if( (iCount & ((1<<10)-1)) == 0 && costTotal == zeroRosterCost )
				break;

			if( iCount - lastImprovement > (1<<15) )
				break;
		}

		if( costTotal != zeroRosterCost )
		{
			// Do a final cleanup.  Try all pairs until there is no improvement.
#ifdef DEBUG_PRINT
			std::cerr << "Deterministic solve..." << std::endl;
#endif
			bool	success = false;
			rEnd = rosters.end();
			do
			{
				success = false;
				for( r1 = rosters.begin(); r1 != rEnd; ++r1 )
				{
					for( r2 = r1 + 1; r2 != rEnd; ++r2 )
					{
						if( trySwapRostersAll(**r1, **r2, true) )
						{
							success = true;
						}
#ifdef DEBUG_PRINT
						if( (iCount & ((1<<12)-1)) == 0 )
						{
							std::cerr << "D ";
							costTotal.printOn( std::cerr );
						}
#endif
						++iCount;
					}
				}
			} while( success );
		}
	}

#ifdef DEBUG_PRINT
	costTotal.printOn( std::cerr );
#endif
}

void RosterImprove::solveSubproblem( WorkDay workDay[], RosterVec &rosters )
{
	// Create an initial solution for the rosters.
	const tod_t averageMinWork = (tod_t)((double)minPaidWork / (double)workDays);
	const tod_t averageMaxWork = (tod_t)((double)maxPaidWork / (double)workDays);

	// First, figure out the best number of each pattern.
	// Get the total work per day (also, reset the inRoster flags).
	register int d;
	DOCroster	doc;
	for( d = 0; d < DayOfWeekMax; ++d )
	{
		register size_t count = 0;
		for( register WorkDay::iterator w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
		{
			(*w).second->inRoster = false;
			if( (*w).second->feasible )
				++count;
		}
		doc[d] = (int)count;
	}

	// Populate the Day Off Calculator data structures.
	DOCweekPatternCounts	weekPatterns;
	Patterns::iterator p, pEnd;
	register int iw;
	for( p = patterns.begin(), pEnd = patterns.end(), iw = 0; p != pEnd; ++p, ++iw )
		weekPatterns[iw].setPattern( *p );
	weekPatterns[iw].setPattern( 0 );

	weekPatterns.setExtrasOnWeekends( preferExtrasOnWeekends );

	// Declare a vector
	UIntVec	numUnmatched( iw ); std::fill( numUnmatched.begin(), numUnmatched.end(), 0 );

	// Solve it.
	int workTotal, extraMax, extraTotal;
	weekPatterns.setExtrasOnWeekends( true );
	doc.solve( workTotal, extraMax, extraTotal, weekPatterns );

	// Create a set of starting rosters according to the DOC solve (as best as we can).
	// Start with the least frequently used pattern.
	weekPatterns.sortIncreasingFrequency();
	for( iw = 0; weekPatterns[iw].getPattern(); ++iw )
	{
		size_t	pTargetCount = (size_t)weekPatterns[iw].getCount();
		if( pTargetCount == 0 )
			continue;

		const int BigM = 24 * 60 * 60 * 1000;		// Days of penalty to attempt to get the maximum number of rosters.

		const int pCur = weekPatterns[iw].getPattern();

		// Find the first working day following a day off.
		const int	dBegin = getFirstWorkingDayFollowingDayOff(pCur);

		// Create a map of all the work days.
		const int pDayTotal = bitCount(pCur);
		IntVec	pDays; pDays.reserve( pDayTotal );
		for( d = 0; d < 7; ++d )
		{
			const int dCur = dValid(dBegin + d);
			if( bitSet(pCur, dCur) )
				pDays.push_back( dCur );
		}
		const int dLast = pDays.back();

		// Create the nodes in the network.
		NetworkSimplex	network;
		NetworkSimplex::Node	*root = network.newNode();

		WorkDay::iterator w, wEnd;
		for( d = 0; d < pDayTotal; ++d )
		{
			// Split each workday into a pair of network simplex nodes.
			// By setting the upper bound on the arc to 1, we guarantee that this work will only be covered once.
			for( w = workDay[pDays[d]].begin(), wEnd = workDay[pDays[d]].end(); w != wEnd; ++w )
			{
				Work	&wCur = *(*w).second;
				if( wCur.feasible && !wCur.inRoster )
				{
					wCur.nIn  = network.newNode();
					wCur.nOut = network.newNode();
//					wCur.arc  = network.newArc( wCur.nIn, wCur.nOut, 0, 0, 1 );
					wCur.arc  = network.newArc( wCur.nIn, wCur.nOut,
												square(wCur.paidWork - averageMinWork),			// Cost
												0, 1 );											// Min, Max
				}
			}
		}

		// Connect the nodes together as specified by the pattern.
		for( d = 1; d < pDayTotal; ++d )
		{
			const int dCur = pDays[d-1], dNext = pDays[d];

			const int iOffset = dValid(dNext - dCur) - 1;
			for( w = workDay[dCur].begin(), wEnd = workDay[dCur].end(); w != wEnd; ++w )
			{
				Work	&wCur = *(*w).second;
				if( wCur.feasible && !wCur.inRoster )
				{
					for( register ConnectionHash::iterator c = wCur.out[iOffset].begin(), cEnd = wCur.out[iOffset].end(); c != cEnd; ++c )
					{
						if( (*c).second.to->feasible && !(*c).second.to->inRoster )
						{
							(*c).second.arc
								= network.newArc(	wCur.nOut, (*c).second.to->nIn,
//													(NetworkSimplex::cost_t)(*c).second.cost,
													0,											// Cost
													0, 1 );										// Min, Max
						}
					}
				}
			}
		}

		// Connect the nodes at the begining and end of the pattern to the root.
		for( w = workDay[pDays[0]].begin(), wEnd = workDay[pDays[0]].end(); w != wEnd; ++w )
		{
			if( (*w).second->feasible && !(*w).second->inRoster )
				network.newArc( root, (*w).second->nIn, -BigM, 0, 1 );
		}
		for( w = workDay[pDays[pDayTotal-1]].begin(), wEnd = workDay[pDays[pDayTotal-1]].end(); w != wEnd; ++w )
		{
			if( (*w).second->feasible && !(*w).second->inRoster )
				network.newArc( (*w).second->nOut, root, 0, 0, 1 );
		}

		// Solve the network to find the maximum number of rosters.
		network.solve();

		RosterVec	rostersOfPattern; rostersOfPattern.reserve( network.getInflow(root) );

		// Transform the network solution back into rosters.
		for( w = workDay[pDays[0]].begin(), wEnd = workDay[pDays[0]].end(); w != wEnd; ++w )
		{
			Work	*wCur = (*w).second;
			if( !wCur->feasible || wCur->inRoster || wCur->arc->getFlow() == 0 )
				continue;

			Roster	*rCur = new Roster;
			rCur->work[wCur->d] = wCur;
			rCur->paidWorkTotal = wCur->paidWork;
			rCur->degreeTotal = network.getOutDegree(wCur->nOut);
			const int dFirst = wCur->d;

			int	dCur = dBegin, dNext;

			register ConnectionHash::const_iterator c, cEnd;
			do
			{
				dNext = dCur;
				do dNext = dValid(dNext + 1); while( !bitSet(pCur, dNext) );

				const int iOffset = dValid(dNext - dCur) - 1;
				rCur->degreeTotal += network.getOutDegree(wCur->nOut);
				for( c = wCur->out[iOffset].begin(), cEnd = wCur->out[iOffset].end(); c != cEnd; ++c )
				{
					if( (*c).second.arc && (*c).second.arc->getFlow() > 0 )
					{
						rCur->work[(wCur = (*c).second.to)->d] = (*c).second.to;
						break;
					}
				}

				dCur = dNext;
			} while( dCur != dLast );

			update( *rCur );
			if( !rCur->getContainsInvalidCombinations() )
				rostersOfPattern.push_back( rCur );
			else
				delete rCur;
		}

		// Of all the rosters found, try to get a number equal to the initial recommendation.
		// Choose the rosters in order of the least possible ways to connect them (least committment approach).
		std::sort( rostersOfPattern.begin(), rostersOfPattern.end(), PRosterCmpDegreeTotalLT(*this) );
		if( rostersOfPattern.size() < pTargetCount )
		{
			// Keep track of the number of unmatched rosters.
			const size_t rSize = rostersOfPattern.size();
			numUnmatched[iw] = pTargetCount - rSize;
			pTargetCount = rSize;
		}

		// Delete the extra rosters we don't need.
		register RosterVec::iterator r, rEnd;
		for( r = rostersOfPattern.begin() + pTargetCount, rEnd = rostersOfPattern.end(); r != rEnd; ++r )
			delete *r;
		rostersOfPattern.erase( rostersOfPattern.begin() + pTargetCount, rostersOfPattern.end() );

		// Mark the work we are covering with these new rosters.
		for( r = rostersOfPattern.begin(), rEnd = rostersOfPattern.end(); r != rEnd; ++r )
		{
			for( register int d = 0; d < DayOfWeekMax; ++d )
				if( (*r)->work[d] )
					(*r)->work[d]->inRoster = true;
		}

		// Keep the rosters we want.
		std::copy( rostersOfPattern.begin(), rostersOfPattern.end(), std::back_inserter(rosters) );

		// Cleanup
		for( d = 1; d < pDayTotal; ++d )
		{
			const int dCur = pDays[d-1], dNext = pDays[d];

			const int iOffset = dValid(dNext - dCur) - 1;
			for( w = workDay[dCur].begin(), wEnd = workDay[dCur].end(); w != wEnd; ++w )
			{
				Work	&wCur = *(*w).second;
				wCur.nIn = NULL;
				wCur.nOut = NULL;
				wCur.arc = NULL;
				for( register ConnectionHash::iterator c = wCur.out[iOffset].begin(), cEnd = wCur.out[iOffset].end(); c != cEnd; ++c )
					(*c).second.arc = NULL;
			}
		}
	}

	// Create infeasible rosters based on the patterns.
	for( iw = 0; weekPatterns[iw].getPattern(); ++iw )
	{
		if( numUnmatched[iw] == 0 )
			continue;

		const int pCur = weekPatterns[iw].getPattern();

		// Create a map of all the work days for this pattern.
		const int pDayTotal = bitCount(pCur);
		IntVec	pDays; pDays.reserve( pDayTotal );
		for( d = 0; d < 7; ++d )
		{
			if( bitSet(pCur, d) )
				pDays.push_back( d );
		}

		// Create infeasible rosters to match the pattern count.
		for( ; numUnmatched[iw] > 0; --numUnmatched[iw] )
		{
			Roster	*rCur = new Roster;
			WorkDay::iterator w, wEnd;
			for( d = 0; d < pDayTotal; ++d )
			{
				for( w = workDay[pDays[d]].begin(), wEnd = workDay[pDays[d]].end(); w != wEnd; ++w )
				{
					if( !(*w).second->inRoster )
					{
						rCur->work[(*w).second->d] = (*w).second;
						(*w).second->inRoster = true;
						break;
					}
				}
			}
			update( *rCur );
			rosters.push_back( rCur );
		}
	}

	// Create artifical rosters to use of all remaining work days, even if they are infeasible.
	for( d = 0; d < 7; ++d )
	{
		WorkDay::iterator w, wEnd;
		for( w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
		{
			if( (*w).second->feasible && !(*w).second->inRoster )
			{
				Roster	*rCur = new Roster;
				rCur->work[(*w).second->d] = (*w).second;
				(*w).second->inRoster = true;

				for( register int d2 = 1; d2 < 7; ++d2 )
				{
					const int dCur = dValid(d + d2);
					WorkDay::iterator w2, w2End;
					for( w2 = workDay[dCur].begin(), w2End = workDay[dCur].end(); w2 != w2End; ++w2 )
					{
						if( (*w2).second->feasible && !(*w2).second->inRoster )
						{
							(*w2).second->inRoster = true;
							rCur->work[(*w2).second->d] = (*w2).second;
							break;
						}
					}
				}
				update( *rCur );
				rosters.push_back( rCur );
			}
		}
	}

	check( workDay, rosters );

	// Swap the rosters around to improve them.
	swapImproveSubproblem( workDay, rosters );

	check( workDay, rosters );
}

void RosterImprove::check( WorkDay workDay[], RosterVec &rosters )
{
#ifdef DEBUG_PRINT
	register int d;

	WorkDay	workDayRoster[7];

	RosterVec::const_iterator r, rEnd;
	for( r = rosters.begin(), rEnd = rosters.end(); r != rEnd; ++r )
	{
		for( d = 0; d < 7; ++d )
			if( (*r)->work[d] )
				workDayRoster[d].insert( std::make_pair((*r)->work[d]->id, (*r)->work[d]) );
	
	}

	for( d = 0; d < 7; ++d )
	{
		if( workDay[d] != workDayRoster[d] )
		{
			int aaa = 10, bbb = 0, ccc;
			ccc = aaa / bbb;
		}
	}
#endif // DEBUG_PRINT
}

void RosterImprove::solve()
{
	// Clean up the patterns.
	patterns.erase( std::remove(patterns.begin(), patterns.end(), (Patterns::value_type)0), patterns.end() );
	std::sort( patterns.begin(), patterns.end() );
	patterns.erase( std::unique(patterns.begin(), patterns.end()), patterns.end() );

	// Flag which patterns are in use.
	std::fill( validPatterns, validPatterns + 128, 0 );
	{
		for( register Patterns::const_iterator p = patterns.begin(), pEnd = patterns.end(); p != pEnd; ++p )
			validPatterns[*p] = 1;
	}

	// Clean up the rosters.
	delete_contents( rosters );

	// Set the workDays (FIXLATER).
	if( !patterns.empty() )
		workDays = bitCount(patterns[0]);

	// Partition the work into independent subproblems.
	partition();

	// Solve each partition and accumulate the results.
	for( int iPartition = 0; iPartition < iPartitionMax; ++iPartition )
	{
#ifdef DEBUG_PRINT
		std::cerr << "****Solving Parition " << iPartition << std::endl;
#endif
		WorkDay		workDayCur[DayOfWeekMax];

		averageDayPaidWork = 0;
		size_t	workTotal = 0;
		for( register int d = 0; d < DayOfWeekMax; ++d )
		{
			for( register WorkDay::iterator w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
			{
				if( (*w).second->iPartition == iPartition )
				{
					workDayCur[d].insert( *w );
					averageDayPaidWork += (*w).second->paidWork;
				}
			}
			workTotal += workDayCur[d].size();
		}
		averageDayPaidWork = (tod_t)((double)averageDayPaidWork / (double)workTotal);
		
		RosterVec	rostersCur;
		solveSubproblem( workDayCur, rostersCur );

		// Accumulate the results.
		rosters.reserve( rosters.size() + rostersCur.size() );
		std::copy( rostersCur.begin(), rostersCur.end(), std::back_inserter(rosters) );
	}

	// Record the status for each roster in its flags.
	for( register RosterVec::iterator r = rosters.begin(), rEnd = rosters.end(); r != rEnd; ++r )
		update( **r );
}

void RosterImprove::write( std::ostream &os )
{
	os << "MinPaidWork= " << minPaidWork << std::endl;
	os << "MaxPaidWork= " << maxPaidWork << std::endl;
	{
		os << "Patterns= " << std::endl;
		Patterns::iterator p, pEnd;
		for( p = patterns.begin(), pEnd = patterns.end(); p != pEnd; ++p )
			os << '\t' << *p << std::endl;
		os << '\t' << -1 << std::endl;
	}

	{
		os << "Work=" << std::endl;
		for( int d = 0; d < DayOfWeekMax; ++d )
		{
			for( WorkDay::iterator w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
				os << '\t' << d << ' ' << (*w).second->id << ' ' << (*w).second->paidWork << std::endl;
		}
		os << '\t' << -1 << std::endl;
	}

	{
		os << "Costs=" << std::endl;
		for( int d1 = 0; d1 < DayOfWeekMax; ++d1 )
		{
			for( WorkDay::iterator w1 = workDay[d1].begin(), w1End = workDay[d1].end(); w1 != w1End; ++w1 )
			{
				for( int d2 = 0; d2 < DayOfWeekMax - 1; ++d2 )
				{
					for( ConnectionHash::iterator c = (*w1).second->out[d2].begin(), cEnd = (*w1).second->out[d2].end(); c != cEnd; ++c )
						os << '\t' << d1 << ' ' << (*w1).second->id << ' ' <<  (*c).second.to->d << ' ' << (*c).second.to->id << ' ' << (*c).second.cost << std::endl;
				}
			}
		}
		os << '\t' << -1 << std::endl;
	}
}

void RosterImprove::read( std::istream &is )
{
	char	sz[128];
	is >> sz >> minPaidWork;
	is >> sz >> maxPaidWork;

	{
		// Read patterns;
		is >> sz;	// Skip the "Patterns=" line.
		int p;
		for( ;; )
		{
			is >> p;
			if( p == -1 )
				break;
			setAllowedWorkPattern( (unsigned int)p );
		}
	}

	{
		// Read Work patterns.
		is >> sz;	// Skip the "Work=" line.

		for( ;; )
		{
			int		d;			is >> d;
			if( d == -1 )
				break;

			id_t	id;			is >> id;
			tod_t	paidWork;	is >> paidWork;

			newWork( (DayOfWeek)d, id, paidWork );
		}
	}

	{
		// Read Costs.
		is >> sz;	// Skip the "Costs=" line.

		for( ;; )
		{
			int		d1;			is >> d1;
			if( d1 == -1 )
				break;

			id_t	id1;		is >> id1;
			int		d2;			is >> d2;
			id_t	id2;		is >> id2;
			cost_t	cost;		is >> cost;

			newConnection( (DayOfWeek)d1, id1, (DayOfWeek)d2, id2, cost );
		}
	}

}