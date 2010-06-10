
#ifndef GraspRuncut_H
#define GraspRuncut_H

#include "BinPackRuncut.h"
#include "FastRand.h"

namespace BinPackRuncut
{

struct Solution
{
	Solution() : cost(0.0), uncutTotal(0), workTotal(1) {}

	RunList		runs;
	PieceList	uncut;
	double		cost;
	tod_t		uncutTotal;
	tod_t		workTotal;

	double		getWorkCutRatio() const	{ return (double)uncutTotal / (double)workTotal; }
	size_t		getNumRuns() const		{ return runs.size(); }

	Solution &operator=( const Runcut &r )
	{
		runs = r.runs;
		uncut = r.uncut;
		cost = r.cost;
		uncutTotal = r.uncutTotal;
		workTotal = r.workTotal;
		return *this;
	}

	template < typename RHS >
	int	cmp( const RHS &s ) const
	{
		return MC::multi_cmp(	uncutTotal, MC::less, s.uncutTotal,
								cost,		MC::less, s.cost );
	}
	template < typename RHS > bool	operator<( const RHS &s ) const	{ return cmp(s) < 0; }

	bool	operator==( const Solution &s ) const
	{
		return cost == s.cost && uncutTotal == s.uncutTotal && runs == s.runs;
	}

	void	swap( Solution &s )
	{
		std::swap( uncutTotal,	s.uncutTotal );
		std::swap( workTotal,	s.workTotal );
		std::swap( cost,		s.cost );
		std::swap( runs,		s.runs );
		std::swap( uncut,		s.uncut );
	}
};
}

namespace std
{
    template<>
    inline void swap(BinPackRuncut::Solution& a, BinPackRuncut::Solution& b) { a.swap(b); }
}

namespace BinPackRuncut
{

class GraspRuncut : public Runcut
{
public:
	GraspRuncut() : iterationMax(4), iterationCur(0) {}

	struct ChooseGrasp : public PieceChooser
	{
		ChooseGrasp( const int seed = 0xed ) : r(seed) {}
		FastRand	r;

		Piece	&operator()( Piece &firstPiece, PieceVector &secondPieces )
		{ return secondPieces[r(secondPieces.size())]; }
	};
	void	solve();

	size_t		iterationCur;
	size_t	getIteration() const { return iterationCur; }

	size_t		iterationMax;
	Solution	sBest;
};

} // namespace BinPackRuncut
#endif // GraspRuncut_H