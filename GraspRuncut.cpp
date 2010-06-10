
#include "GraspRuncut.h"

namespace BinPackRuncut
{

void	GraspRuncut::solve()
{
	iterationCur = 0;
	Runcut::solve();
	sBest = *this;

	for( size_t i = 1; i < iterationMax && keepGoing(); ++i )
	{
		++iterationCur;
		coreSolve( ChooseGrasp(0xeded + i), 3 );
		if( !(sBest < *this) )
			sBest = *this;
	}
}

};