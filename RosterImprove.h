
#ifndef RosterImprove_H
#define RosterImprove_H

#ifdef _MSC_VER
#pragma warning (disable: 4786 4290)
#define NOMINMAX
#endif // _MSC_VER

#include "tod.h"
#include <assert.h>
#include <list>
//#include <hash_map>
//#include <hash_set>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "FastRand.h"
#include "NetworkSimplex.h"

#include <limits>

template <typename Collection>
void delete_contents( Collection &c )
{
	register typename Collection::iterator i, iEnd;
	for( i = c.begin(), iEnd = c.end(); i != iEnd; ++i )
		delete *i;
	c.clear();
}

template <typename Collection>
void delete_contents_second( Collection &c )
{
	register typename Collection::iterator i, iEnd;
	for( i = c.begin(), iEnd = c.end(); i != iEnd; ++i )
		delete (*i).second;
	c.clear();
}

struct cmp_pointer
{
	template <typename Element>
	bool operator()( const Element *e1, const Element *e2 ) const { return *e1 < *e2; }
};

inline int bitCount( register unsigned int mask )
{
	register int count = 0;
	while( mask )
	{
		mask &= (mask-1);
		++count;
	}
	return count;
}

inline int dValid( int d ) { if( (d %= 7) < 0 ) d += 7; return d; }
inline int dPrev( const int d ) { return dValid(d-1); }
inline int dNext( const int d ) { return dValid(d+1); }

template <typename T>	T	square( const T x ) { return x * x; }

class RosterImprove
{
public:
	enum DayOfWeek { Mon, Tue, Wed, Thu, Fri, Sat, Sun, DayOfWeekMax };
	typedef long	id_t;
	typedef	double	cost_t;

	typedef std::vector<unsigned int>	UIntVec;

	class Work;		friend class Work;
	class Roster;	friend class Roster;

protected:
	typedef std::vector<unsigned int> Patterns;

	struct	Connection
	{
		Connection(	Work *aFrom, Work *aTo, const cost_t aCost ) : from(aFrom), to(aTo), cost(aCost), arc(NULL) {}
		Work	*from, *to;
		cost_t	cost;
		NetworkSimplex::Arc	*arc;
	};

	struct PWorkHash { size_t operator()( const Work *w ) const { return (size_t)w >> 2; } };
//	typedef std::hash_map<Work *, Connection, PWorkHash>	ConnectionHash;
	typedef std::map<Work *, Connection>	ConnectionHash;

public:
	class Work
	{
		friend class RosterImprove;
		friend class Roster;

	public:
		const id_t		id;
		const DayOfWeek	d;
		const tod_t		paidWork;
		const bool		isNightWork;

	protected:
		Work( const id_t aID, const DayOfWeek aD, const tod_t aPaidWork, const bool aIsNightWork = false ) :
				id(aID), d(aD), paidWork(aPaidWork), isNightWork(aIsNightWork),
				nIn(NULL), nOut(NULL), arc(NULL), inRoster(false), feasible(true),
				iPartition(-1) {}

		// Connections into this piece of work.
		// in[0] is the previous consecutive day, in[1] is with one day between, etc.
		// out[0] is the next consecutive day, out[1] is with one day between, etc.
		ConnectionHash		out[DayOfWeekMax - 1];

		bool	getCostTo( cost_t &cst, Work &w )
		{
			const int iOffset  = dValid(w.d - d) - 1;
			ConnectionHash::iterator c = out[iOffset].find(&w);
			if( c != out[iOffset].end() )
			{
				cst = (*c).second.cost;
				return true;
			}
			return false;
		}

		bool	operator<( const Work &w ) const	{ return id < w.id; }
		bool	operator==( const Work &w ) const	{ return id == w.id; }

		NetworkSimplex::Node	*nIn, *nOut;
		NetworkSimplex::Arc		*arc;
		bool			inRoster;
		bool			feasible;
		int				iPartition;
	};
	typedef std::map<id_t, Work *>	WorkDay;
	typedef std::vector<Work *>		WorkVec;
	typedef std::list<Work *>		WorkList;
//	typedef std::hash_set<Work *, PWorkHash>	WorkHash;
	typedef std::set<Work *>		WorkHash;

	typedef std::vector<int>		IntVec;

	class RosterCost
	{
	public:
		RosterCost() { reset(); }

		int		numInfeasibleConnections;
		int		nightWorkViolations;
		tod_t	lengthViolation, minViolation, maxViolation;
		tod_t	diffFromAverage;
		cost_t	cost;
		bool	isArtificial;	// true if this roster was maximally constructed of invalid work connections.

		void	reset()
		{
			numInfeasibleConnections = 0;
			nightWorkViolations = 0;
			lengthViolation = minViolation = maxViolation = 0;
			diffFromAverage = 0;
			cost = 0;
			isArtificial = false;
		}

		bool operator==( const RosterCost &rc ) const
		{
			return	numInfeasibleConnections == rc.numInfeasibleConnections &&
					nightWorkViolations == rc.nightWorkViolations &&
					lengthViolation == rc.lengthViolation &&
					diffFromAverage == rc.diffFromAverage &&
					cost == rc.cost &&
					isArtificial == rc.isArtificial;
		}
		bool operator!=( const RosterCost &rc ) const	{ return !operator==( rc ); }

		RosterCost	&operator+=( const RosterCost &rc )
		{
			if( !rc.isArtificial )
			{
				numInfeasibleConnections	+= rc.numInfeasibleConnections;
				nightWorkViolations			+= rc.nightWorkViolations;
				lengthViolation				+= rc.lengthViolation;
				diffFromAverage				+= rc.diffFromAverage;
				cost						+= rc.cost;
			}
			return *this;
		}

		RosterCost	&operator-=( const RosterCost &rc )
		{
			if( !rc.isArtificial )
			{
				numInfeasibleConnections	-= rc.numInfeasibleConnections;
				nightWorkViolations			-= rc.nightWorkViolations;
				lengthViolation				-= rc.lengthViolation;
				diffFromAverage				-= rc.diffFromAverage;
				cost						-= rc.cost;
			}
			return *this;
		}

		RosterCost	operator+( const RosterCost &rc )
		{
			if( isArtificial )		return rc;
			if( rc.isArtificial )	return *this;

			RosterCost rcNew = *this;
			rcNew += rc;
			return rcNew;
		}

		void	printOn( std::ostream &os )
		{
			os << "numInfeasibleConnections=" << numInfeasibleConnections
				<< " nightWorkViolations=" << nightWorkViolations
				<< " lengthViolation=" << lengthViolation
				<< " diffFromAverage=" << diffFromAverage
				<< " cost=" << std::setprecision(9) << cost
				<< std::endl;
		}
	};

	class Roster
	{
		friend class RosterImprove;

	public:
		// Status flags for this roster (set by RosterImprove::setFlags()).
		enum
		{
			violatesMinPaidWork			= (1<<0),
			violatesMaxPaidWork			= (1<<1),
			violatesNightWork			= (1<<2),
			violatesWorkDays			= (1<<3),
			violatesPatterns			= (1<<4),
			containsInvalidCombinations = (1<<5)
		};
		unsigned int flags;
		bool	getViolatesMinPaidWork() const			{ return 0 != (flags & violatesMinPaidWork); }
		bool	getViolatesMaxPaidWork() const			{ return 0 != (flags & violatesMaxPaidWork); }
		bool	getViolatesNightWork() const			{ return 0 != (flags & violatesNightWork); }
		bool	getViolatesWorkDays() const				{ return 0 != (flags & violatesWorkDays); }
		bool	getViolatesPatterns() const				{ return 0 != (flags & violatesPatterns); }
		bool	getContainsInvalidCombinations() const	{ return 0 != (flags & containsInvalidCombinations); }
		bool	isFeasible() const						{ return flags == 0; }

		RosterCost	cost;
		tod_t		paidWorkTotal;		// Total work.

		size_t		degreeTotal;		// Total connections of the work in this roster.

		Work		*work[7];
		mutable unsigned int pattern;

		bool	operator==( const Roster &r ) const
		{
			return  cost			== r.cost &&
					paidWorkTotal	== r.paidWorkTotal &&
					work[0]			== r.work[0] &&
					work[1]			== r.work[1] &&
					work[2]			== r.work[2] &&
					work[3]			== r.work[3] &&
					work[4]			== r.work[4] &&
					work[5]			== r.work[5] &&
					work[6]			== r.work[6];
		}
		bool	operator!=( const Roster &r ) const { return !operator==(r); }

		unsigned int computePattern() const
		{
			return pattern = (
				(work[0] ? (1<<0) : 0) |
				(work[1] ? (1<<1) : 0) |
				(work[2] ? (1<<2) : 0) |
				(work[3] ? (1<<3) : 0) |
				(work[4] ? (1<<4) : 0) |
				(work[5] ? (1<<5) : 0) |
				(work[6] ? (1<<6) : 0)
				);
		}
		unsigned int getPattern() const { return pattern ? pattern : computePattern(); }

		void	reset()
		{
			work[0] = NULL;
			work[1] = NULL;
			work[2] = NULL;
			work[3] = NULL;
			work[4] = NULL;
			work[5] = NULL;
			work[6] = NULL;
			pattern = 0; paidWorkTotal = 0, degreeTotal = 0; flags = 127;
		}

		int		getFirstWorkday() const
		{
			for( register int d = 0; d < 7; ++d )
				if( work[d] )
					break;
			return d;
		}
		int		getNextWorkday( register int dCur ) const
		{
			do { dCur = dValid(dCur+1); } while( !work[dCur] );

			return dCur;
		}

		bool	empty() const	{ getPattern(); return pattern == 0; }

		bool	hasOnlyOne() const
		{
			getPattern();
			return (pattern & (pattern - 1)) == 0;	// Check if the pattern is a power of 2.
		}

		size_t	size() const
		{
			return
				(work[0] ? 1 : 0) +
				(work[1] ? 1 : 0) +
				(work[2] ? 1 : 0) +
				(work[3] ? 1 : 0) +
				(work[4] ? 1 : 0) +
				(work[5] ? 1 : 0) +
				(work[6] ? 1 : 0);
		}

	protected:
		Roster() { reset(); }
	};
	typedef std::vector<Roster *>		RosterVec;
	typedef	std::list<Roster *>			RosterList;

	struct PRosterCmpGreater; friend PRosterCmpGreater;
	struct PRosterCmpGreater
	{
		PRosterCmpGreater( RosterImprove &aRT ) : rt(&aRT) {}

		RosterImprove	*rt;
		bool operator()( const Roster *r1, const Roster *r2 ) const { return rt->cmpLT(r2->cost, r1->cost); }
	};
	struct PRosterCmpDegreeTotalLT; friend PRosterCmpDegreeTotalLT;
	struct PRosterCmpDegreeTotalLT
	{
		PRosterCmpDegreeTotalLT( RosterImprove &aRT ) : rt(&aRT) {}

		RosterImprove	*rt;
		bool operator()( const Roster *r1, const Roster *r2 ) const
		{
			return r1->degreeTotal < r2->degreeTotal ? true
				:  r2->degreeTotal < r1->degreeTotal ? false
				:  rt->cmpLT(r1->cost, r2->cost);
		}
	};

public:

	RosterImprove();
	~RosterImprove();

	void	newWork( const DayOfWeek d, const id_t id, const tod_t paidWork, const bool isNightWork = false )
	{
		WorkDay::iterator w = workDay[d].find(id);
		if( w == workDay[d].end() )
			workDay[d].insert(std::make_pair(id, new Work(id, d, paidWork, isNightWork)));
	}

	typedef std::vector<id_t>	WorkIDVector;
	void	getWork( const DayOfWeek d, WorkIDVector &work )
	{
		work.clear();
		work.reserve( workDay[d].size() );
		register WorkDay::iterator w, wEnd;
		for( w = workDay[d].begin(), wEnd = workDay[d].end(); w != wEnd; ++w )
			work.push_back( (*w).first );
	}

	void	newConnection(	const DayOfWeek dFrom, const id_t idFrom,
							const DayOfWeek dTo, const id_t idTo,
							cost_t	cost )
	{
		if( dFrom == dTo )
			return;

		WorkDay::iterator wFrom	= workDay[dFrom].find(idFrom);
		if( wFrom == workDay[dFrom].end() )
			return;
		WorkDay::iterator wTo	= workDay[dTo].find(idTo);
		if( wTo == workDay[dTo].end() )
			return;

		const int iOffset  = dValid(dTo - dFrom) - 1;
		(*wFrom).second->out[iOffset].insert( std::make_pair((*wTo).second, Connection((*wFrom).second, (*wTo).second, cost)) );
	}

	void	setMinMaxPaidWork( const tod_t aMinPaidWork, const tod_t aMaxPaidWork )
	{
		minPaidWork = aMinPaidWork;
		maxPaidWork = aMaxPaidWork;
	}

	void	setMaxNightWork( const int aMaxNightWork = 7 ) { maxNightWork = aMaxNightWork; }

	void	setEqualize( const bool aEqualize ) { equalize = aEqualize; }

	void	setPreferExtrasOnWeekends( const bool aPreferExtrasOnWeekends ) { preferExtrasOnWeekends = aPreferExtrasOnWeekends; }

	// p is a bitmap of the work pattern where (1<<0) == Mon, (1<<1) == Tue etc.
	// Work Patterns do not have to have the same number of work days.
	void	setAllowedWorkPattern( const unsigned int p ) { patterns.push_back(p & 127); }

	void	solve();

	const RosterVec	&getRosters() const { return rosters; }

	void	write( std::ostream &os );
	void	read( std::istream &is );

protected:
#define CmpLTNum( n1, n2 )	if( (n1) < (n2) ) return true; if( (n2) < (n1) ) return false
	bool	cmpLT( const RosterCost &rc1, const RosterCost &rc2 ) const
	{
		CmpLTNum( rc1.numInfeasibleConnections,	rc2.numInfeasibleConnections );
		CmpLTNum( rc1.nightWorkViolations,		rc2.nightWorkViolations );
		CmpLTNum( rc1.lengthViolation,			rc2.lengthViolation );
		if( equalize )
		{
			CmpLTNum( rc1.diffFromAverage,		rc2.diffFromAverage );
		}
		return rc1.cost < rc2.cost;

	}
	bool	cmpLTFeasible( const RosterCost &rc1, const RosterCost &rc2 ) const
	{
		if( rc1.isArtificial || rc2.isArtificial )
			return true;

		CmpLTNum( rc1.numInfeasibleConnections,	rc2.numInfeasibleConnections );
		CmpLTNum( rc1.nightWorkViolations,		rc2.nightWorkViolations );
		return rc1.lengthViolation < rc2.lengthViolation;
	}
	bool	cmpLEFeasible( const RosterCost &rc1, const RosterCost &rc2 ) const
	{
		if( rc1.isArtificial || rc2.isArtificial )
			return true;

		CmpLTNum( rc1.numInfeasibleConnections,	rc2.numInfeasibleConnections );
		CmpLTNum( rc1.nightWorkViolations,		rc2.nightWorkViolations );
		return rc1.lengthViolation <= rc2.lengthViolation;
	}
#undef CmpLTNum

	void		update( Roster &r );

	void		partition();
	bool		trySwapRostersAll( Roster &r1, Roster &r2, const bool doAll = false );
	void		swapImproveSubproblem( WorkDay workDay[], RosterVec &rosters );

	void		solveSubproblem( WorkDay workDay[], RosterVec &rosters );

	tod_t		minPaidWork, maxPaidWork;	// minimum/maximum roster lengths
	bool		equalize;					// if true, try to make all rosters as close as possible to the average length
	Patterns	patterns;					// allowable patterns

	int			maxNightWork;				// maximum allowable night work/roster

	int			iPartitionMax;	// Number of partitions.
	tod_t		averageDayPaidWork;

	int			workDays;		// Expected work days for all patterns.

	bool		preferExtrasOnWeekends;

	WorkDay		workDay[DayOfWeekMax];
	RosterVec	rosters;
	RosterCost	costTotal;

	FastRand	fastRand;

	void		check( WorkDay workDay[], RosterVec &rosters );

	int			validPatterns[128];		// An array with 1 if that pattern (as index) is valid, 0 if not.
	unsigned int swapPermutation[127];	// Permutation sequence of least impact.
};

#endif // RosterImprove_H