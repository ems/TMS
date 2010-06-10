
#ifndef CrossEntropyRuncut_H
#define CrossEntropyRuncut_H

#include "GraspRuncut.h"
#include "HashObj.h"

namespace BinPackRuncut
{

typedef HashObj<Run, size_t>	PiecePairCount;
typedef std::vector<Solution>	SolutionVector;

class CrossEntropyRuncut : public Runcut
{
public:
	typedef bool	(*Callback)( void * );

	CrossEntropyRuncut() : iterationMax(1000), populationSize(100), iterationCur(0) {}

	struct ChooseCrossEntropy : public PieceChooser
	{
		ChooseCrossEntropy( CrossEntropyRuncut *aCER, const int seed = 0xed ) : cer(aCER), r(seed) {}

		Piece	&operator()( Piece &firstPiece, PieceVector &secondPieces );

		CrossEntropyRuncut	*cer;
		FastRand	r;
		SizeTVector	countsCum;	// Temporary used by chooseCrossEntropy;
	};

	void	solve();

	Solution	sBest;
	SolutionVector	population;

	size_t		iterationCur;
	size_t	getIteration() const { return iterationCur; }

	size_t		iterationMax;
	size_t		populationSize;

	PiecePairCount	ppc;
	void		updatePPC( const Solution &s )
	{
		for( register RunList::const_iterator r = s.runs.begin(), rEnd = s.runs.end(); r != rEnd; ++r )
		{
			PiecePairCount::iterator p = ppc.find( *r );
			if( p == ppc.end() )
				ppc.insert( *r, 1 );
			else
				p.data()++;
		}
	}
	size_t	getCount( const Run &r )
	{
		PiecePairCount::iterator i = ppc.find(r);
		return i == ppc.end() ? 0 : i.data();
	}
};

} // BinPackRuncut

#endif // CrossEntropyRuncut_H