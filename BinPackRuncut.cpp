
#include "BinPackRuncut.h"

#include <iostream>

namespace BinPackRuncut
{

void	Block::resetPartitions( const tod_t pieceSizeMin, const tod_t pieceSizeMax )
{
	// Initialize all structure.
	pieces.clear();

	register ReliefVector::iterator r, rEnd;
	for( r = relief.begin(), rEnd = relief.end(); r != rEnd; ++r )
		r->children.clear();

	// Recursively partition into pieces.
	partitionRecursive( pieceSizeMin, pieceSizeMax, 0 );

	// Organize all the pieces as a set.
	for( r = relief.begin(), rEnd = relief.end(); r != rEnd; ++r )
	{
		const size_t i = r - relief.begin();
		const tod_t t = r->t;
		// Reverse children so we insert the longest ones first.
		// This preserves the search order by increasing time, decreasing length.
		std::reverse( r->children.begin(), r->children.end() );
		for( register SizeTVector::iterator j = r->children.begin(), jEnd = r->children.end(); j != jEnd; ++j )
			pieces.push_back( Piece(this, i, *j) );
		r->children.clear();
	}

	// std::cerr << "Pieces: " << id << std::endl; for( PieceList::iterator p = pieces.begin(), pEnd = pieces.end(); p != pEnd; ++p ) std::cerr << *p << std::endl;
}

void	Block::partitionRecursive( const tod_t pieceSizeMin, const tod_t pieceSizeMax, const size_t i )
{
	register size_t jLast = relief.size() - 1;
#ifdef FIXLATER
	register tod_t	W = workTime(i, jLast);
	if( pieceSizeMin <= W && W <= pieceSizeMax )
	{
		relief[i].children.push_back( jLast );
		return;
	}
#endif
	for( register size_t j = i + 1; j <= jLast; ++j )
	{
		register tod_t	W = workTime( i, j );
		if( W > pieceSizeMax )									break;
		if( W < pieceSizeMin )									continue;

		// Check if we would leave a leftover piece on the end that is too small.
		if( j != jLast && workTime(j, jLast) < pieceSizeMin )
		{
			j = jLast - 1;
			continue;
		}

		relief[i].children.push_back( j );
		if( relief[j].children.empty() )
			partitionRecursive( pieceSizeMin, pieceSizeMax, j );
	}
}

//-----------------------------------------------------------------------------
bool	Runcut::hasOverlap( const Piece &piece )
{
	// Check if there is at least one overlapping piece.
	register PieceList::iterator p = piece.b->pieces.begin(), pEnd = piece.b->pieces.end();
	for( ; p != pEnd && p->i < piece.j; ++p )
	{
		if( p->overlaps(piece) )
			return true;
	}
	return false;
}

void	Runcut::removeOverlapping( const Piece &piece )
{
	// Find all the overlaping pieces and remove them.
	register PieceList::iterator p = piece.b->pieces.begin(), pEnd = piece.b->pieces.end();
	while( p != pEnd && p->i < piece.j )
	{
		if( p->overlaps(piece) )
		{
			PieceList::iterator pCur(p);
			++p;
			piece.b->pieces.erase( pCur );
		}
		else
			++p;
	}
}

void	Runcut::findBestSecondPieces( PieceVector &secondPiecesBest, const Piece &firstPiece )
{
	secondPiecesBest.clear();

	// Check if the first node is feasible.
	if( infeasibleFirst.contains(firstPiece.first().nodeID) )
		return;

	secondPiecesBest.reserve( choicesMax );

	const tod_t firstPieceworkLength = firstPiece.length();
	const tod_t firstPieceFirstT = firstPiece.first().t;
	const tod_t firstPieceLastT = firstPiece.last().t;

	for( BlockList::iterator b = blocks.begin(), bEnd = blocks.end(); b != bEnd; ++b )
	{
		for( PieceList::iterator p = b->pieces.begin(), pEnd = b->pieces.end(); p != pEnd; ++p )
		{
			Piece	&secondPiece = *p;

			// Check min/max break.
			const tod_t	breakTime = secondPiece.first().t - firstPieceLastT;
			if( breakTime < breakMin )		continue;
			if( breakTime > breakMax )		break;

			// Check spreadtime.
			const tod_t spreadTime = secondPiece.last().t - firstPieceFirstT;
			if( spreadTime > spreadMax )	continue;

			// Check worktime.
			const tod_t workTime = firstPieceworkLength + secondPiece.length();
			if( workTime < platformMin )	continue;
			if( workTime > platformMax )	continue;

			// Check that the endpoints are compatible.
			if( startEndMustMatch && firstPiece.first().nodeID != secondPiece.last().nodeID ) continue;

			// Check that the relief points are at compatible locations.
			const tod_t tConnect = getConnectionTime(firstPiece.last().nodeID, secondPiece.first().nodeID);
			if( tConnect < 0 ) continue;						// Locations are not compatible.
			if( breakTime - tConnect < breakMin ) continue;		// Check for min break with connection time.

			// Check if last nodeID in the run is feasible.
			if( infeasibleLast.contains(secondPiece.last().nodeID) )
				continue;

			// Do a final feasibility check.
			if( !checkFeasible(firstPiece, secondPiece) )
			{
				// Check if we discover that this can never be the first location.
				if( infeasibleFirst.contains(firstPiece.first().nodeID) )
					return;
				continue;
			}

			// At this point the piece is a feasible match.

			// Check if we need more candidates.
			if( secondPiecesBest.size() < choicesMax )
			{
				secondPiecesBest.push_back( secondPiece );
				if( secondPiecesBest.size() == choicesMax )
					std::make_heap( secondPiecesBest.begin(), secondPiecesBest.end() );
				continue;
			}

			// Check if this is a better piece than the worst candidate we have.
			if( secondPiece < secondPiecesBest.front() )
			{
				std::pop_heap( secondPiecesBest.begin(), secondPiecesBest.end() );
				secondPiecesBest.back() = secondPiece;
				std::push_heap( secondPiecesBest.begin(), secondPiecesBest.end() );
				continue;
			}

			// Check if we do not need to consider any more possibilities in this block.
			break;
		}
	}

	// Make sure the heap is built when we don't have the maximum candidates.
	if( secondPiecesBest.size() < choicesMax )
		std::make_heap( secondPiecesBest.begin(), secondPiecesBest.end() );
}

void	Runcut::getUncutWork()
{
	BlockList::iterator b, bEnd;

	for( b = emptyBlocks.begin(), bEnd = emptyBlocks.end(); b != bEnd; ++b )
		b->resetHeadTail();

	// Set the head and tail flags on all the known pieces.
	for( RunList::iterator r = runs.begin(), rEnd = runs.end(); r != rEnd; ++r )
	{
		for( size_t i = 0; i < 2; ++i )
		{
			Piece			&p = r->piece[i];
			ReliefVector	&relief = p.b->relief;

			relief[p.i].head = true;
			for( register size_t k = p.i + 1; k < p.j; ++k )
			{
				relief[k].head = true;
				relief[k].tail = true;
			}
			relief[p.j].tail = true;
		}
	}

	// Collect the uncut pieces.
	for( b = emptyBlocks.begin(), bEnd = emptyBlocks.end(); b != bEnd; ++b )
	{
		for( register size_t i = 0, iEnd = b->relief.size(); i < iEnd; ++i )
		{
			if( !b->relief[i].head || !b->relief[i].tail )
			{
				register size_t j;
				for( j = i + 1; j < iEnd && (!b->relief[j].head || !b->relief[j].tail); ++j )
					continue;
				--j;
				uncut.push_back( Piece(&*b, i, j) );
				i = j;
			}
		}
	}
}

void Runcut::coreSolve( PieceChooser &pieceChooser, const size_t numChoices )
{
	// Initialize.
	choicesMax = numChoices;
	runs.clear();
	uncut.clear();
	cost = 0.0;
	uncutTotal = 0;
	workTotal = 0;

	blocks.splice( blocks.end(), emptyBlocks );
	blocks.sort();

	// Generate all the block partitions.
	BlockList::iterator b, bEnd;
	for( b = blocks.begin(), bEnd = blocks.end(); b != bEnd; ++b )
	{
		if( !b->relief.empty() )
			workTotal += b->relief.back().t - b->relief.front().t;
		b->resetPartitions( pieceSizeMin, pieceSizeMax );
	}

	tod_t	tLast = 0;
	PieceVector	secondPiecesBest;
	while( keepGoing() )
	{
		// Get the best first piece.
		Piece	firstPiece;
		for( b = blocks.begin(), bEnd = blocks.end(); b != bEnd; )
		{
			if( b->pieces.empty() )
			{
				BlockList::iterator bCur(b);
				++b;
				emptyBlocks.splice( emptyBlocks.end(), blocks, bCur );
				continue;
			}

			if( firstPiece.empty() || b->pieces.front() < firstPiece )
				firstPiece = b->pieces.front();
			++b;
		}

		if( firstPiece.empty() )
			break;

		// Remove this piece from the block as it will either be matched or discarded.
		firstPiece.b->pieces.pop_front();

		tCur = firstPiece.first().t;
		if( tCur < tLast )
		{
			int aaa = 10;
		}
		tLast = tCur;

		// std::cerr << "Attempting firstPiece: " << firstPiece << std::endl;

		// Try to find a good second piece.
		findBestSecondPieces( secondPiecesBest, firstPiece );
		if( !secondPiecesBest.empty() )
		{
			Piece secondPiece = pieceChooser(firstPiece, secondPiecesBest);

			// std::cerr << "Found second piece: " << secondPiece << std::endl;

			// Remove all pieces that overlap the pieces.
			removeOverlapping( firstPiece );
			removeOverlapping( secondPiece );

			// Record the run.
			Run	r;
			r.piece[0] = firstPiece;
			r.piece[1] = secondPiece;
			r.cost = runCost( r );
			runs.push_back( r );
			cost += r.cost;
		}
		else
		{
			// std::cerr << "No second piece found." << std::endl;
		}
	}

	getUncutWork();
	for( register PieceList::iterator p = uncut.begin(), pEnd = uncut.end(); p != pEnd; ++p )
		uncutTotal += p->length();

	cost += (regularCostPerHour + additionalPenaltyCostPerHour) * uncutTotal;
}

}; // namespace BinPackRuncut