
#include "CrossEntropyRuncut.h"

namespace BinPackRuncut
{

Piece	&CrossEntropyRuncut::ChooseCrossEntropy::operator()( Piece &firstPiece, PieceVector &secondPieces )
{
	if( countsCum.size() < secondPieces.size() + 1 )
		countsCum.resize( secondPieces.size() * 2 + 1 );

	register SizeTVector::iterator c = countsCum.begin();
	*c++ = 0;

	Run	run;
	run.piece[0] = firstPiece;
	for( register PieceVector::const_iterator p = secondPieces.begin(), pEnd = secondPieces.end(); p != pEnd; ++p )
	{
		run.piece[1] = *p;
		*c++ = *(c-1) + cer->getCount(run) + 1;
	}

	--c;

	size_t R = r(*c);
	size_t i = std::lower_bound( countsCum.begin(), c, R ) - countsCum.begin();
	if( R < countsCum[i] )
		--i;

	return secondPieces[i];
}

void CrossEntropyRuncut::solve()
{
	ppc.clear();

	iterationCur = 0;

	size_t i;
	population.clear();
	population.resize( populationSize );
	SolutionVector::iterator p, pEnd;
	for( p = population.begin(), pEnd = population.end(); p != pEnd && keepGoing(); ++p )
	{
		coreSolve( GraspRuncut::ChooseGrasp(p - population.begin()), 5 );
		*p = *this;
		if( *p < sBest )
			sBest = *p;
		++iterationCur;
	}

	const size_t	populationTop = size_t(populationSize / 10);
	sBest = population.front();

	// Update the weights based on the best individuals.
	std::nth_element( population.begin(), population.begin() + populationTop, population.end() );
	for( p = population.begin(), pEnd = population.begin() + populationTop; p != pEnd && keepGoing(); ++p )
	{
		updatePPC( *p );
		if( *p < sBest )
			sBest = *p;
	}

	size_t	noImprovementCount = 0;
	for( i = 0; i < iterationMax && noImprovementCount < 10 && keepGoing(); ++i )
	{
		// Create a new population based on the weights.
		for( p = population.begin(), pEnd = population.end(); p != pEnd && keepGoing(); ++p )
		{
			coreSolve( ChooseCrossEntropy(this, i*iterationMax + (p - population.begin())), 5 );
			*p = *this;
			if( !keepGoing() )
				return;
		}

		// Update the weights based on the best individuals.
		std::nth_element( population.begin(), population.begin() + populationTop, population.end() );
		for( p = population.begin(), pEnd = population.begin() + populationTop; p != pEnd && keepGoing(); ++p )
		{
			updatePPC( *p );
			if( *p < sBest )
			{
				sBest = *p;
				noImprovementCount = 0;
			}
			else
				++noImprovementCount;
		}
		++iterationCur;
	}
}

};
