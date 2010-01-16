//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#include <limits.h>
#include "cover.h"
#include "PairingHeap.h"
#include <fstream>

using namespace std;

#define Assert( c ) \
do { if( !(c) ) { int a = 0, b; b = 10 / a; } } while(0)

static const double Infinity = (double)INT_MAX;

double Column::contribution()
{
	register unsigned int under = 0;
	rowHT::iterator r;
	ForAllInCollection( rows, r )
		if( (*r)->isUnderCovered() )
			under += (*r)->bOriginal - (*r)->bCurrent;

	return under == 0 ? Infinity : (double)cost / (double)under;
};

class	IncreasingContrib
{
public:
	bool	operator()( const Column &c1, const Column &c2 ) const 
	{ return c1.lastContribution < c2.lastContribution; }
};

int	Cover::solveCover( const unsigned int NMax )
{
	// Reset any existing solution.
	unsigned int exposed = 0;
	idRowHT::iterator r;
	ForAllInCollection( rows, r )
	{
		r.data()->init();
		exposed += r.data()->bOriginal;
	}
	solution.clear();

	// Update all contributions and put them into the heap.
	PairingHeap<Column, IncreasingContrib> colHeap( columns.size() );
	idColumnHT::iterator c;
	ForAllInCollection( columns, c )
	{
		c.data()->value = 0;
		c.data()->lastContribution = c.data()->contribution();
		colHeap.insert( c.data() );
	}

	// Initialize the best pick list.
	unsigned int N = columns.size() < NMax ? columns.size() : NMax;

	Column	**nBest = new Column * [N];
	register unsigned int i;
	for( i = 0; i < N; i++ )
		nBest[i] = colHeap.extractTop();

	while( exposed > 0 )
	{
		// Pick from the top N to give some more randomness.
		i = rand() % N;
		for( ;; )
		{
			const double newContribution = nBest[i]->contribution();
			if( nBest[i]->lastContribution != newContribution )
			{
				nBest[i]->lastContribution = newContribution;
				colHeap.insert( nBest[i] );
				nBest[i] = colHeap.extractTop();
			}
			else
				break;
		}
		register Column *cBest = nBest[i];
		if( cBest->cost >= Infinity )
		{
			// If we are hitting infinities, half the number of choices
			// and continue.
			if( N > 1 )
			{
				for( i = N/2; i < N; i++ )
					colHeap.insert( nBest[i] );
				N /= 2;
			}
			continue;
		}

		// Put this column into the solution.
		const unsigned int lastExposed = exposed;
		if( ++cBest->value == 1 )
			solution.insertKey( cBest );
		rowHT::iterator r;
		ForAllInCollection( cBest->rows, r )
		{
			if( (*r)->isUnderCovered() )
				exposed--;
			(*r)->cover( cBest );
		}

		// Check if we should retract any previous columns.
		ForAllInCollection( cBest->rows, r )
		{
			if( (*r)->bCurrent < (*r)->bOriginal )
				continue;
			columnHT::iterator c, cNext;
			for( c = (*r)->solutionColumns.begin();
				!(c == (*r)->solutionColumns.end());
				c = cNext )
			{
				cNext = c; ++cNext;

				if( (*c) == cBest )
					continue;

				// Count how many more exposures will occur if we remove this column.
				register Column *cCur = *c;
				rowHT::iterator rCur;
				register unsigned int deltaExposed = 0;
				ForAllInCollection( cCur->rows, rCur )
					if( (*rCur)->bCurrent <= (*rCur)->bOriginal )
						deltaExposed++;

				// If the total number of exposed is still less than when we added
				// the new column, remove this column.
				if( exposed + deltaExposed < lastExposed )
				{
					// Remove this column from the solution.
					if( --(cCur->value) == 0 )
						solution.remove( cCur );
					ForAllInCollection( cCur->rows, rCur )
						(*rCur)->expose( cCur );

					exposed += deltaExposed;
				}
			}
		}
	}

	// Update the cost.
	cost = 0;
	columnHT::iterator col;
	ForAllInCollection( solution, col )
		cost += (*col)->value * (*col)->cost;

#ifdef FIXLATER
	// Check the solution
	idRowHT::iterator row;
	ForAllInCollection( rows, row )
	{
		Row *r = row.data();
		Assert( !r->isUnderCovered() );
	}

	// Print out the solution.
	ForAllInCollection( solution, col )
		cerr << 'x' << (*col)->id << "=" << (*col)->value << ' ';
	cerr << '\n';

	ForAllInCollection( rows, row )
	{
		Row *r = row.data();
		cerr << "b=" << r->bOriginal << " c=" << r->bCurrent << '\n';
	}
	cerr << "cost=" << cost << '\n';
#endif

	// Cleanup
	delete [] nBest;

	return 0; // Optimal.
}

int	Cover::solve()
{
	// Try pure greedy.
	if( solveCover(1) )
		return 1; // Infeasible;

#ifdef GRASP_SOLVE
	const unsigned int NMax = 10;
	int bestSeed = -1;

	unsigned int bestCost = cost;
//	cerr << "g\t" << bestCost << '\n';

	for( int i = 0; i < 5; i++ )
	{
		cerr << '.';
		srand( i );
		solveCover( NMax );
		if( cost < bestCost )
		{
			bestCost = cost;
			bestSeed = i;
//			cerr << '\n' << i << '\t' << bestCost << '\n';
		}
	}

	// Restore the best solution.
	if( bestSeed < 0 )
		solveCover( 1 );
	else
	{
		srand( bestSeed );
		solveCover( NMax );
	}
#endif

	return 0; // Optimal
}