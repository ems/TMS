
#include "NetworkSimplex.h"

//#define VALIDATE_BASIS

#include <iostream>
#include <assert.h>

NetworkSimplex::flow_t	NetworkSimplex::getViolation( const Node *n ) const
{
	if( root )
	{
		register const Arc *a;

		if( n->b >= 0 )
		{
			// Find the arc pointing to the root.
			for( a = n->out; a->to != root; a = a->outNext )
				continue;
			if( a->flow > 0 )
				return a->flow;
		}
		else
		{
			// Find the arc pointing from the root.
			for( a = n->in; a->from != root; a = a->inNext )
				continue;
			if( a->flow > 0 )
				return -a->flow;
		}
	}
	return 0;
}

void NetworkSimplex::deleteNode( register Node *n )
{
	while( n->out )		{ deleteArc(n->out); }
	while( n->in )		{ deleteArc(n->in); }

	list_remove(nodeList, n, prev, next);

	n->~Node(); nodePool.free( n );

	--N;
}

void	NetworkSimplex::clear()
{
	// Purge the memory pools (faster).
	arcPool.purge_memory();
	nodePool.purge_memory();

	nodeList = NULL;

	root = NULL;
	N = A = 0;
	J = K = 0;

	lastPivotNode = NULL;
	lastPivotArc = NULL;

	numPivots = numDegeneratePivots = 0;
	fixedCost = cost = 0;

	bigFlow = -1;
	bigCost = -1;
}

void	NetworkSimplex::printBasis( std::ostream &os )
{
	register Node	*n, *child;

	n = root;
	n->childCur = n->children;
	os << (void *)n << ' ' << n->depth << ' ' << (cost_t)n->pi << std::endl;

	for( ;; )
	{
		while( child = n->childCur )
		{
			// Follow this child down the tree.
			n->childCur = child->siblingNext;
			n = child;
			n->childCur = n->children;

			for( register int i = 0; i < n->depth; ++i )
				os << ' ';
			os << (void *)n << ' ' << n->depth << ' ' << (cost_t)n->pi << std::endl;
		}

		// Pop up to the parent and check its remaining children.
		if( n == root )
			break;
		n = n->parent();
	}
}

void	NetworkSimplex::validateBasis()
{
	register Node	*n, *child;

	n = root;
	n->childCur = n->children;

	for( ;; )
	{
		while( child = n->childCur )
		{
			assert( child->parent() == n );
			assert( child->depth = n->depth + 1 );
			assert( child->pi ==
				n->pi + (child->treeArc->from == n ? -child->treeArc->cost : child->treeArc->cost) );

			// Check for strong basis conditions.
			if( child->treeArc->atMin() )
				assert( child->treeArc->from == child );
			if( child->treeArc->atMax() )
				assert( child->treeArc->to == child );

			// Follow this child down the tree.
			n->childCur = child->siblingNext;
			n = child;
			n->childCur = n->children;
		}

		// Pop up to the parent and check its remaining children.
		if( n == root )
			break;
		n = n->parent();
	}
}

void	NetworkSimplex::updateBasis( register Node *top )
{
	register	Node	*n, *child;

	n = top;
	n->childCur = n->children;
	for( ;; )
	{
		while( child = n->childCur )
		{
			// Follow this child down the tree.
			n->childCur = child->siblingNext;
			child->updateTreeChild( n );
			n = child;
			n->childCur = n->children;
		}

		// Pop up to the parent and check its remaining children.
		if( n == top )
			break;
		n = n->parent();
	}
}

void	NetworkSimplex::pivot( NetworkSimplex::Arc *pivotArc )
{
	register	Node *from, *to;

	++numPivots;

	if( pivotArc->atMin() )
	{
		from = pivotArc->from;		// Flow in forward direction.
		to = pivotArc->to;
	}
	else
	{
		from = pivotArc->to;		// Flow in reverse direction.
		to = pivotArc->from;
	}

	const cost_t arcPotential = pivotArc->getPotential();

	// Find the apex of the cycle.
	// Determine the limiting arc to leave the tree to preserve a strong basis.
	// "Select the leaving arc as the last blocking arc encountered in traversing the pivot cycle W
	// along its orientatation starting at apex w."
	Node	*apex;
	Arc		*leaveArc = NULL;
	flow_t	capacityMin = pivotArc->max;
	bool	leaveArcOnFromBranch;
	{
		register flow_t capacityCur, fromCapacityMin = capacityMin, toCapacityMin = capacityMin;
		register Node *cycleFrom = from, *cycleTo = to;
		Arc		*fromLeaveArc = NULL, *toLeaveArc = NULL;

		for( ; cycleFrom->depth > cycleTo->depth; cycleFrom = cycleFrom->parent() )
			if( (capacityCur = cycleFrom->treeArc->getResCapacityTo(cycleFrom)) < fromCapacityMin )
			{
				fromCapacityMin = capacityCur;
				fromLeaveArc = cycleFrom->treeArc;
			}

		for( ; cycleTo->depth > cycleFrom->depth; cycleTo = cycleTo->parent() )
			if( (capacityCur = cycleTo->treeArc->getResCapacityFrom(cycleTo)) <= toCapacityMin )
			{
				toCapacityMin = capacityCur;
				toLeaveArc = cycleTo->treeArc;
			}

		for( ; cycleFrom != cycleTo; cycleFrom = cycleFrom->parent(), cycleTo = cycleTo->parent() )
		{
			if( (capacityCur = cycleFrom->treeArc->getResCapacityTo(cycleFrom)) < fromCapacityMin )
			{
				fromCapacityMin = capacityCur;
				fromLeaveArc = cycleFrom->treeArc;
			}
			if( (capacityCur = cycleTo->treeArc->getResCapacityFrom(cycleTo)) <= toCapacityMin )
			{
				toCapacityMin = capacityCur;
				toLeaveArc = cycleTo->treeArc;
			}
		}

		apex = cycleFrom;

		if( fromCapacityMin < capacityMin || toCapacityMin <= capacityMin )
		{
			if( fromCapacityMin < toCapacityMin )
			{
				leaveArcOnFromBranch = true;
				capacityMin = fromCapacityMin;
				leaveArc = fromLeaveArc;
			}
			else // toCapacityMin <= fromCapacityMin
			{
				leaveArcOnFromBranch = false;
				capacityMin = toCapacityMin;
				leaveArc = toLeaveArc;
			}
		}
	}

	// Push flow around the cycle if this is not a degenerate pivot.
	if( capacityMin > 0 )
	{
		register Node	*n;
		for( n = from; n != apex; n = n->parent() )
			n->treeArc->incFlowTo( n, capacityMin );
		for( n = to; n != apex; n = n->parent() )
			n->treeArc->incFlowFrom( n, capacityMin );

		pivotArc->incFlowFrom( from, capacityMin );

		cost += capacityMin * arcPotential;
	}
	else
		++numDegeneratePivots;

	// Repair the basis if necessary.
	if( leaveArc )
	{
		register Arc	*aPrev = pivotArc, *aNext;
		register Node	*n, *nParent;
		register Node	*leaveTop = leaveArc->from->depth < leaveArc->to->depth ? leaveArc->from : leaveArc->to;
		for( n = leaveArcOnFromBranch ? from : to; n != leaveTop; n = nParent )
		{
			nParent = n->parent();
			aNext = n->treeArc;
			n->reparent( aPrev );
			aPrev = aNext;
		}
		pivotArc->inTree = true;
		leaveArc->inTree = false;

		// Update the basis.
		updateBasis( leaveArcOnFromBranch ? to : from );
#ifdef VALIDATE_BASIS
		validateBasis();
#endif
	}
}

NetworkSimplex::Arc	*NetworkSimplex::findPivotArc()
{
	// Use Multiple Partial Pricing as described in "Solving Large-Scale Real-World Minimum-Cost Flow Problems by
	// a Network Simplex Method", by Andreas Lobel, February 1996.
	//
	// We do not explicitly implement candidate lists as described in the paper, rather, we just we
	// just scan the next K arcs in the network.

	register Arc *aBest = NULL;		// Pivot arc with the greatest reduced cost violation.
	register cost_t	potentialBest = 0, potentialCur;

	// Purge invalid candidates and record the best candidate so far.
	register ArcVector::iterator aiDest = candidates.begin();
	{
		for( register ArcVector::iterator ai = aiDest; *ai; ++ai )
		{
			if( !(*ai)->inTree && (potentialCur = (*ai)->getPotential()) < 0 && (*ai)->max > 0 )
			{
				if( potentialCur < potentialBest )
				{
					potentialBest = potentialCur;
					aBest = (*ai);
				}
				*aiDest++ = *ai;
			}
		}
	}

	// While where are fewer than J candidates to choose from, scan the next K arcs.
	// Also stop when we have scanned all the arcs in the network.
	for( size_t count = 0; aiDest < candidates.begin() + J && count < A; count += K )
	{
		for( register size_t k = 0; k < K; ++k )
		{
			register Arc *a = lastPivotArc;
			for( lastPivotArc = lastPivotArc->outNext; !lastPivotArc; lastPivotArc = lastPivotNode->out )
			{
				if( !(lastPivotNode = lastPivotNode->next) )
					lastPivotNode = nodeList;
			}

			if( !a->inTree && (potentialCur = a->getPotential()) < 0 && a->max > 0 )
			{
				if( potentialCur < potentialBest )
				{
					potentialBest = potentialCur;
					aBest = a;
				}
				*aiDest++ = a;
			}
		}
	}

	// Terminate the candidate list and return the arc that violates the reduced cost criteria the most.
	*aiDest = NULL;
	return aBest;
}

void	NetworkSimplex::initCandidateList()
{
	if(		A	<= 10000 )	{	K = 30;		J = 5;	}
	else if(A	<= 100000 )	{	K = 50;		J = 10;	}
	else					{	K = 200;	J = 20;	}

	if( K > A )
		K = A;
	if( J > K )
		J = K / 2 > 0 ? K / 2 : 1;

	candidates.resize( K + J + 1 );	// Account for a possible terminating NULL.
	candidates[0] = NULL;

	for( lastPivotNode = nodeList; lastPivotNode; lastPivotNode = lastPivotNode->next )
	{
		if( lastPivotArc = lastPivotNode->out )
			break;
	}
}

void	NetworkSimplex::createBasis()
{
	removeBasis();

	cost = 0;

	register Node *n;
	register Arc *a;

	// Compute the limits for the artificial arcs.
	bigCost = 1;
	bigFlow = 1;
	for(n = nodeList; n; n = n->next )
	{
		if( n->b < 0 )
			bigFlow -= n->b;
		for( a = n->out; a; a = a->outNext )
			if( a->cost > 0 )
				bigCost += a->cost;
	}

	// Create an artificial node for the root.
	root = new (nodePool.malloc()) Node( 0 );

	for( n = nodeList; n; n = n->next )
	{
		for( a = n->out; a; a = a->outNext )
		{
			a->flow = 0;
			a->inTree = false;
		}

		// Connect each node to and from the root.

		if( n->b >= 0 )
		{
			Arc	*toRoot = newArc( n, root, BigCost(), 0, BigFlow() );
			n->setParent( toRoot );
			toRoot->flow = n->b;
			cost += toRoot->cost * toRoot->flow;
		}
		else
		{
			Arc	*fromRoot = newArc( root, n, BigCost(), 0, BigFlow() );
			n->setParent( fromRoot );
			fromRoot->flow = -n->b;
			cost += fromRoot->cost * fromRoot->flow;
		}
	}

	++N;
	list_push_front( nodeList, root, prev, next );
}

void	NetworkSimplex::solve()
{
	if( !root )
		createBasis();
	else
		updateBasis( root );

	numPivots = numDegeneratePivots = 0;

	initCandidateList();
	register	Arc	*a;
	while( a = findPivotArc() )
		pivot( a );
}

NetworkSimplex::cost_t	NetworkSimplex::computeCost() const
{
	register cost_t costCur = 0;

	for( register const Node *n = nodeList; n; n = n->next )
	{
		if( n != root )
		{
			for( register const Arc *a = n->out; a; a = a->outNext )
				if( a->to != root )
					costCur += a->getFlow() * a->cost;
		}
	}

	return costCur;
}

bool	NetworkSimplex::isFeasible() const
{
	if( !root )
		return false;

	register const Arc *a;

	// Check if any artificial arcs have flow.
	for( a = root->out; a; a = a->outNext )
	{
		if( a->flow > 0 )
			return false;
	}

	for( a = root->in; a; a = a->inNext )
	{
		if( a->flow > 0 )
			return false;
	}

	return true;
}