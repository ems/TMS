//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "mbpsolve.h"
#include <limits.h>
#include <string.h>
#include <new.h>

#ifdef DIAGNOSTICS
extern ofstream *bpLogFile;
#define log	(*bpLogFile)
#endif

//#define PRINT_NETWORK

#define Crash() do { int a = 1, b = 0; a = a / b; } while(0)

char *nodeName( const MBPnode *n )
{
	static char s[64];
	char *pch = s + sizeof(s);
	unsigned long l = (unsigned long)n;

	*(--pch) = 0;
	do
	{
		*(--pch) = "0123456789"[l % 10];
		l /= 10;
	} while( l > 0 );
	return pch;
}

MBPnode::MBPnode( ident_t aID, cost_t aCost, int aSet )
{
  set = aSet;
  id = aID;
  cost = aCost;
  backwardCost = forwardCost = 0;
  bpNode = NULL;
  outList = inList = NULL;
  dummy = false;
  next = prev = NULL;
  forwardMate = NULL;
  backwardMate = NULL;
  init();
}

MBParc	*MBPnetwork::findArc( MBPnode *from, MBPnode *to ) const
{
  register MBParc *a;
  for( a = from->outList; a != NULL; a = a->outNext )
    if( a->to == to )
      break;
  return a;
}

MBPnode	*MBPnetwork::addNode( ident_t id, cost_t cost, int set )
{
  void	*buf;
  if( nodeFreeList != NULL )
    {
      buf = (void *)nodeFreeList;
      nodeFreeList = nodeFreeList->next;
    }
  else if( (buf = nodeMemList->getBuf()) == NULL )
    {
      new memobj( nodeMemList, sizeof(MBPnode) );
      buf = nodeMemList->getBuf();
    }


  MBPnode *n = new (buf) MBPnode( id, cost, set );
  if( nodeList[set] != NULL )
    nodeList[set]->prev = n;
  n->next = nodeList[set];
  n->prev = NULL;
  nodeList[set] = n;
  ++numSetNodes[set];
  ++numNodes;
  return n;
}

void MBPnetwork::deleteNode( register MBPnode *n )
{
  while( n->outList )
    deleteArc( n->outList );
  while( n->inList )
    deleteArc( n->inList );
  if( n->prev != NULL )
    n->prev->next = n->next;
  else
    nodeList[n->set] = n->next;
  if( n->next != NULL )
    n->next->prev = n->prev;

  // Destroy the node, but recover the memory.
  n->~MBPnode();
  n->next = nodeFreeList;
  nodeFreeList = n;

  --numNodes;
}

MBParc		*MBPnetwork::addArc( MBPnode *aFrom, MBPnode *aTo, cost_t aCost )
{
  if( aFrom->set != aTo->set-1 )
	  Crash();

  void	*buf;
  if( arcFreeList != NULL )
    {
      buf = (void *)arcFreeList;
      arcFreeList = arcFreeList->next;
    }
  else if( (buf = arcMemList->getBuf()) == NULL )
    {
      new memobj( arcMemList, sizeof(MBParc) );
      buf = arcMemList->getBuf();
    }

  MBParc *a = new (buf) MBParc(aFrom, aTo, aCost );
  
  aFrom->insertOutArc(a);
  aTo->insertInArc(a);
  
  a->next = arcList;
  a->prev = NULL;
  if( arcList != NULL )
    arcList->prev = a;
  arcList = a;
  numArcs++;
  return a;
}

void	MBPnetwork::deleteArc( register MBParc *a )
{
  // Remove the arc from the graph.
  a->to->removeInArc( a );
  a->from->removeOutArc( a );
  
  if( a->prev != NULL )
    a->prev->next = a->next;
  else
    arcList = a->next;
  if( a->next != NULL )
    a->next->prev = a->prev;

  // Destory the arc, but recover the memory.
  a->~MBParc();
  a->next = arcFreeList;
  arcFreeList = a;

  --numArcs;
}

void MBPnetwork::deleteArc( MBPnode *from, MBPnode *to )
{
  for( register MBParc *a = from->outList; a != NULL; a = a->outNext )
    if( a->to == to )
      {
		deleteArc( a );
		return;
      }
}


struct MemObj
{
  MemObj	*next;
};

MBPnetwork::MBPnetwork( const int aNumSets, const cost_t aTarget )
{
  numSets = aNumSets;
  target = aTarget;
  arcMemList = NULL;
  new memobj( arcMemList, sizeof(MBParc) );
  nodeMemList = NULL;
  new memobj( nodeMemList, sizeof(MBPnode) );
  arcFreeList = NULL;
  nodeFreeList = NULL;

  arcList = NULL;
  numArcs = 0;
  numNodes = 0;

  nodeList = new MBPnode * [numSets];
  numSetNodes = new int[numSets];
  for( int set = 0; set < numSets; set++ )
  {
	nodeList[set] = NULL;
	numSetNodes[set] = 0;
  }

  cost = 0;

  numPivots = 0;
  costPairs = NULL;
}


MBPnetwork::~MBPnetwork()
{
  memobj	*mem, *memNext;
  for( mem = arcMemList; mem != NULL; mem = memNext )
    {
      memNext = mem->next;
      delete mem;
    }
  for( mem = nodeMemList; mem != NULL; mem = memNext )
    {
      memNext = mem->next;
      delete mem;
    }
  delete [] nodeList;
  delete [] numSetNodes;
  delete [] costPairs;
}

cost_t	MBPnetwork::getDummyCost( const int i ) const
{
	register MBPnode *n;
	register cost_t	averageCost = 0;

	// Compute the average cost at level i.
	for( n = nodeList[i]; n != NULL; n = n->next )
		averageCost += n->cost;
	averageCost = (cost_t)((double)averageCost / (double)numSetNodes[i] + 0.5);
	return averageCost;
}

void	MBPnetwork::addDummyNodes()
{
	// Find the set with the maximum number of nodes.
	register int i, j;
	register MBPnode *n;
	register MBParc *a;

	int	elementMax = numSetNodes[0];
	for( i = 1; i < numSets; i++ )
		if( numSetNodes[i] > elementMax )
			elementMax = numSetNodes[i];

	// Add dummy nodes until all sets have the same number.
	int	*needsDummies = new int[numSets];
	for( i = 0; i < numSets; i++ )
	{
		if( elementMax == numSetNodes[i] )
		{
			needsDummies[i] = 0;
			continue;
		}
		needsDummies[i] = 1;

		// Give each dummy node an average cost.
		const cost_t	averageCost = getDummyCost( i );
		for( j = numSetNodes[i]; j < elementMax; j++ )
		{
			n = addNode( -10000 - j, averageCost, i );
			n->dummy = true;
		}
	}

	// Connect dummy nodes to subsequent sets.
	for( i = 0; i < numSets - 1; i++ )
	{
		if( !needsDummies[i] )
			continue;

		// Compute the average cost of a connection.
		cost_t averageCost = 0;
		int count = 0;
		for( n = nodeList[i]; n != NULL; n = n->next )
			for( a = n->outList; a != NULL; a = a->outNext )
		    {
				averageCost += a->cost;
				++count;
			}
		// Connect up the dummy nodes.
		averageCost = (cost_t)((double)averageCost / (double)count + 0.5);
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			if( !n->dummy )
				continue;
			for( register MBPnode *to = nodeList[i+1]; to != NULL; to = to->next )
				addArc( n, to, averageCost );
		}
	}

	// Cleanup
	delete [] needsDummies;
}

void	MBPnetwork::phase1()
{
	// Find a feasible solution.
	register MBPnode	*n;
	register MBParc		*a;

	for( n = nodeList[0]; n != NULL; n = n->next )
	{
		n->backwardCost = n->cost;
		n->backwardMate = NULL;
	}
	for( n = nodeList[numSets-1]; n != NULL; n = n->next )
	{
		n->forwardCost = n->cost;
		n->forwardMate = NULL;
	}

	BPnetwork	bp;
	int i;
	for( i = 0; i < numSets - 1; i++ )
	{
		D( log << "phase1: solving for stages: " << i << " " << (i+1) << '\n' );

		bp.reset();
		// Construct a bottleneck problem from sets i and i+1.
		for( n = nodeList[i+1]; n != NULL; n = n->next )
		{
			n->bpNode = bp.addNode( (ident_t)(n), 1 );
			n->backwardMate = NULL;
		}

		// Connect the bottleneck problems with cost = backwardCost(i) + cost(i+1)
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			n->bpNode = bp.addNode( (ident_t)(n), 0 );
			n->forwardMate = NULL;

			for( a = n->outList; a != NULL; a = a->outNext )
				a->bpArc = bp.addArc( n->bpNode, a->to->bpNode,
										n->backwardCost + a->to->cost,
										(ident_t)(a) );
		}

		// Try to find a feasible solution to the problem.
		BPnode	*bpNode;
		while( (bpNode = bp.findInitialSolution()) != NULL )
		{
			// If a node cannot be feasible matched, change it to a dummy node.
			const cost_t averageCost = getDummyCost( i );
			MBPnode *from = (MBPnode *)bpNode->getID(), *to;

			from->dummy = true;
			from->backwardCost = from->backwardCost - from->cost + averageCost;
			from->cost = averageCost;

			// Cleanup the graph.
			while( from->firstOutArc() != NULL )
				deleteArc( from->firstOutArc() );

			// Connect the new dummy node to everything.
			for( to = nodeList[i+1]; to != NULL; to = to->next )
				addArc( from, to, averageCost );

			// Synchronize the subproblem.
			while( bpNode->firstOutArc() != NULL )
				bp.deleteArc( bpNode->firstOutArc() );
			for( a = from->outList; a != NULL; a = a->outNext )
				a->bpArc = bp.addArc( from->bpNode, a->to->bpNode,
										from->backwardCost + a->to->cost,
										(ident_t)(a) );
		}
		// Minimize the maximum connection.
		bp.minimizeBottleneck();
		++numPivots;

		// Install the generated solution.
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			a = (MBParc *)(n->bpNode->mateArc()->getID());
			n->setForwardMate( a );
			a->to->backwardCost = n->backwardCost + a->to->cost;
		}
	}

	// Compute the forward costs.
	register MBPnode *from;
	for( i = numSets - 1; i > 0; i-- )
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			from = n->backwardMate->from;
			from->forwardCost = n->forwardCost + from->cost;
		}

	// Find the maximum cost.
	n = nodeList[0];
	cost = n->forwardCost;
	for( n = n->next; n != NULL; n = n->next )
		if( n->forwardCost > cost )
			cost = n->forwardCost;
	D( printSolution() );
}

void	MBPnetwork::phase2()
{
  // Attempt to improve the solution by swapping arcs between stages.
  register MBPnode	*n;
  register MBParc		*a;
  BPnetwork	bp;

  int	improvementMade;
  do
    {
      improvementMade = 0;

      // Attempt to shorten the longest path between 2 stages.
      for( int i = 0; i < numSets - 1 /* && !improvementMade */; i++  )
	{
	  D( log << "phase2: solving for stages: " << i << " " << (i+1) << '\n' );

	  // Construct a bottleneck problem from sets i and i+1.
	  bp.reset();

	  cost = INT_MIN;
	  register cost_t mateCost;
	  MBPnode	*nMax = NULL;
	  for( n = nodeList[i+1]; n != NULL; n = n->next )
	    {
	      n->bpNode = bp.addNode( (ident_t)(n), 1 );

	      if( (mateCost = n->backwardMate->from->backwardCost + n->forwardCost) > cost )
		{
		  cost = mateCost;
		  nMax = n->backwardMate->from;
		}
	    }

	  for( n = nodeList[i]; n != NULL; n = n->next )
	    {
	      n->bpNode = bp.addNode( (ident_t)(n), 0 );

				// Connect the bottleneck problems with cost = backwardCost(i) + forwardCost(i+1)
	      for( a = n->outList; a != NULL; a = a->outNext )
		if( (mateCost = n->backwardCost + a->to->forwardCost) <= cost )
		  a->bpArc = bp.addArc( n->bpNode, a->to->bpNode,
					mateCost,
					(ident_t)(a) );

				// Install the current solution.
	      n->bpNode->setMate( n->forwardMate->bpArc );
	    }

	  ++numPivots;
	  // Attempt to pivot out the arc connecting the longest path.
	  if( bp.improveAtNode(nMax->bpNode) )
	    {
	      improvementMade = 1;
	      ++numPhase2Improvements;

				// Install the new solution.
	      for( n = nodeList[i]; n != NULL; n = n->next )
		{
		  // Check for a change.
		  a = (MBParc *)(n->bpNode->mateArc()->getID());
		  if( a == n->forwardMate )
		    continue;

		  n->setForwardMate( a );

		  // Update the backward costs of the changed path.
		  register MBPnode	*nCur;
		  for( nCur = a->to; ; nCur = nCur->forwardMate->to )
		    {
		      nCur->backwardCost = nCur->cost + nCur->backwardMate->from->backwardCost;
		      if( nCur->set == numSets - 1 )
			break;
		    }
		
		  // Update the forward costs of the changed path.
		  for( nCur = a->from; ; nCur = nCur->backwardMate->from )
		    {
		      nCur->forwardCost = nCur->cost + nCur->forwardMate->to->forwardCost;
		      if( nCur->set == 0 )
			break;
		    }
		}
	    }
	}
    } while( improvementMade );
  D( printSolution() );
}

struct Path
{
	MBPnode	*x, *y;			// The matched nodes before and after the start and end nodes of
							// a subpath.
	BPnode	*bpNode;
};

struct SubPath
{
	Path	*path;			// The original path that contains this subpath.
	MBPnode	*start, *end;	// The start and end nodes of the subpath.
	cost_t	cost;
	BPnode	*bpNode;
};

struct PathLink
{
	Path	*path;
	SubPath	*subPath;
	MBParc	*aFrom, *aTo;	// Arcs splicing the path to the subpath.

	BParc	*bpArc;
	cost_t getCost() const
	{ return aFrom->getFrom()->getBackwardCost() +
			subPath->cost +
			aTo->getTo()->getForwardCost(); }
};

struct hashPair
{
  MBParc	*a;
  MBPnode	*n;
};

inline int getHash( register MBPnode *n, register int hashSize )
{
  return (int)((((unsigned long)n)>>2) % hashSize);
}

void	MBPnetwork::phase3()
{
  // Attempt to improve the solution by swapping sub-paths.
  register MBPnode	*n;
  register int i;
  BPnetwork	bp;
  Path	*paths = new Path[numSetNodes[0]];
  SubPath	*subPaths = new SubPath[numSetNodes[0]];
  int		numSubPaths;
  PathLink *pathLinks = new PathLink[numSetNodes[0] * numSetNodes[0]];
  int		numPathLinks;

  // Find a prime number to hash with.
  int	hashSize = numSetNodes[0] * 2;
  int	isPrime = 0;
  do
    {
      isPrime = 1;
      if( (hashSize & 1) == 0 )
	  hashSize++;
      register int j = (hashSize >> 1) - 1;
      if( (j & 1) == 0 )
	j--;
      for( ; j >= 3; j -= 2 )
	if( hashSize % j == 0 )
	  {
	    isPrime = 0;
	    hashSize += 2;
	    break;
	  }
    } while( !isPrime );
  hashPair	*xHash = new hashPair[hashSize];
  hashPair	*yHash   = new hashPair[hashSize];

  int	improvementMade;
  do
    {
      improvementMade = 0;

      // Find the current longest path.
      n = nodeList[0];
      cost = n->forwardCost;
      for( n = n->next; n != NULL; n = n->next )
	if( n->forwardCost > cost )
	  cost = n->forwardCost;

      // Attempt to find some paths to swap.
      // Look for swaps as follows:
      // For all sets, look for subpaths of increasing length to swap.
      // If we find an improvement, keep going to avoid duplicating work.
      for( int d = 2; d < numSets - 1 && d < 5 /* && !improvementMade */; d++ )
	{
	  for( int k = 0; k + d <= numSets - 1 /* && !improvementMade */; k++ )
	    {
				// Construct a bottleneck problem with subpaths connecting sets k and k+d.
	      D( log << "phase3: k=" << k << " d=" << d << '\n' );
	      D( printSolution() );

				// First, construct all the subpaths from k to k+d.
	      numSubPaths = 0;
	      MBPnode	*start, *end;
	      cost_t	subPathCost;
	      for( n = nodeList[k]; n != NULL; n = n->next )
		{
		  start = end = n->forwardMate->to;
		  subPathCost = start->cost;
		  while( end->set < k + d - 1 )
		    {
		      end = end->forwardMate->to;
		      subPathCost += end->cost;
		    }
		  Path	*p = &paths[numSubPaths];
		  SubPath *s = &subPaths[numSubPaths];

		  p->x = start->backwardMate->from;
		  p->y = end->forwardMate->to;

		  s->path = p;
		  s->start = start;
		  s->end = end;
		  s->cost = subPathCost;

		  D( log << "phase3: path[" << numSubPaths << "].x=" << p->x->id );
		  D( log << " y=" << p->y->id << '\n' );
		  D( log << "phase3: subPath[" << numSubPaths << "].start=" << s->start->id );
		  D( log << " end=" << s->end->id );
		  D( log << " cost=" << s->cost << '\n' );
		  numSubPaths++;
		}

				// Create all the path links based on the sub paths.

	      numPathLinks = 0;
	      for( i = 0; i < numSubPaths; i++ )
		{
		  Path	*p = &paths[i];

		  register int k;
		  for( k = 0; k < hashSize; k++ )
		    {
		      xHash[k].n = NULL;
		      yHash[k].n = NULL;
		    }
		  for( register MBParc *aTo = p->x->outList; aTo != NULL; aTo = aTo->outNext )
		    {
		      k = getHash(aTo->to, hashSize);
		      while( xHash[k].n != NULL )
			if( ++k == hashSize )
			  k = 0;
		      xHash[k].n = aTo->to;
		      xHash[k].a = aTo;
		    }
		  for( register MBParc *aFrom = p->y->inList; aFrom != NULL; aFrom = aFrom->inNext )
		    {
		      k = getHash(aFrom->from, hashSize);
		      while( yHash[k].n != NULL )
			if( ++k == hashSize )
			  k = 0;
		      yHash[k].n = aFrom->from;
		      yHash[k].a = aFrom;
		    }

		  for( register int j = 0; j < numSubPaths; j++ )
		    {
		      SubPath *s = &subPaths[j];
		      
#ifndef FIXLATER
		      k = getHash(s->start, hashSize);
		      while( xHash[k].n != s->start && xHash[k].n != NULL )
			if( ++k == hashSize )
			  k = 0;
		      if( xHash[k].n == NULL )
			continue;
		      MBParc	*aFrom = xHash[k].a;

		      k = getHash(s->end, hashSize);
		      while( yHash[k].n != s->end && yHash[k].n != NULL )
			if( ++k == hashSize )
			  k = 0;
		      if( yHash[k].n == NULL )
			continue;
		      MBParc	*aTo = yHash[k].a;

		      PathLink	*pl = &pathLinks[numPathLinks];
		      pl->aFrom	= aFrom;
		      pl->aTo	= aTo;
		      pl->path  = p;
		      pl->subPath= s;
		      if( pl->getCost() <= cost )
			numPathLinks++;

#else
		      for( register MBParc *aFrom = s->start->inList; aFrom != NULL; aFrom = aFrom->inNext )
			{
			  if( aFrom->from != p->x )
			    continue;

			  for( register MBParc *aTo = s->end->outList; aTo != NULL; aTo = aTo->outNext )
			    {
			      if( aTo->to != p->y )
				continue;

			      PathLink	*pl = &pathLinks[numPathLinks];
			      pl->aFrom	= aFrom;
			      pl->aTo	= aTo;
			      pl->path    = p;
			      pl->subPath= s;
			      if( pl->getCost() <= cost )
				numPathLinks++;
			    }
			}
#endif
		    }
		}

	      if( numPathLinks <= numSetNodes[0] )
		continue;

				// Create a matching problem between the existing paths and the subPaths.
	      bp.reset();
	      for( i = 0; i < numSubPaths; i++ )
		{
		  SubPath *s = &subPaths[i];
		  Path	*p = s->path;
		  p->bpNode = bp.addNode( (ident_t)(p), 0 );
		  s->bpNode = bp.addNode( (ident_t)(s), 1 );
		}

				// Connect the bottleneck problems with cost = backwardCost(i) + forwardCost(i+1)
				// Also, install the current solution.
	      for( i = 0; i < numPathLinks; i++ )
		{
		  PathLink	*pl = &pathLinks[i];
		  pl->bpArc = bp.addArc( pl->path->bpNode, pl->subPath->bpNode,
					 pl->getCost(), (ident_t)(pl) );

		  // Check if this path is connecting the two nodes.
		  if( pl->path == pl->subPath->path )
		    pl->path->bpNode->setMate( pl->bpArc );
		}

				// Solve the bottleneck problem.
	      D( bp.printNetwork() );
	      bp.minimizeBottleneck();
	      ++numPivots;

	      if( bp.getCost() < cost )
		{
		  // Install the improved solution.
		  D( log << "phase3: found an improvement: oldCost=" << cost << " newCost=" << bp.getCost() << '\n' );
		  improvementMade = 1;
		  ++numPhase3Improvements;
		  cost = bp.getCost();

		  for( i = 0; i < numSubPaths; i++ )
		    {
		      Path		*p = &paths[i];
		      PathLink	*pl = (PathLink *)(p->bpNode->mateArc()->getID());
		      if( pl->aFrom == p->x->forwardMateArc() )
			continue;
		      SubPath		*s = pl->subPath;

		      D( log << "phase3: updating path starting from: " << p->x->id );
		      D( log << " ending at: " << p->y->id << '\n' );
		      D( log << "phase3: connecting them to: " << s->start->id );
		      D( log << " and " << s->end->id << '\n' );
		      p->x->setForwardMate( pl->aFrom );
		      p->y->setBackwardMate( pl->aTo );

		      // Update the forward and backward costs of the changed path.
		      register MBPnode	*nCur;

		      for( nCur = pl->subPath->end; ; nCur = nCur->backwardMate->from )
			{
			  nCur->forwardCost = nCur->cost + nCur->forwardMate->to->forwardCost;
			  if( nCur->set == 0 )
			    break;
			}
		      for( nCur = pl->subPath->start; ; nCur = nCur->forwardMate->to )
			{
			  nCur->backwardCost = nCur->cost + nCur->backwardMate->from->backwardCost;
			  if( nCur->set == numSets - 1 )
			    break;
			}
		    }
		  D( checkSolution() );
		  D( log << "phase3: improved solution:\n" );
		  D( printSolution() );
		}
	    }
	}
    } while( improvementMade );

  // Cleanup
  delete [] xHash;
  delete [] yHash;

  delete [] pathLinks;
  delete [] subPaths;
  delete [] paths;
  D( printSolution() );
}

static int cmpCostPairs( const void *v1, const void *v2 )
{
	cost_t	c1 = ((MBPcostPair *)v1)->cost;
	cost_t	c2 = ((MBPcostPair *)v2)->cost;
	return c1 < c2 ? -1 : c1 > c2 ? 1 : 0;
}

void	MBPnetwork::collectPaths()
{
	int	i;
	register MBPnode	*n, *nCur;

	// For all the bottleneck paths, sort them by increasing cost.
	costPairs = new MBPcostPair[numSetNodes[0]];

	const cost_t dummyPenalty = 8 * 60 * 60;
	for( i = 0, n = nodeList[0]; n != NULL; n = n->next, i++ )
	{
		costPairs[i].node = n;

		// First, penalize the roster for deviation from the target.
		costPairs[i].cost = target - n->forwardCost;
		if( costPairs[i].cost < 0 )
			costPairs[i].cost = -costPairs[i].cost;

		// Then, penalize it for the number of dummy nodes.
		for( nCur = n; nCur != NULL; nCur = nCur->forwardMateNode() )
			if( nCur->dummy )
				costPairs[i].cost += dummyPenalty;
	}

	qsort( costPairs, numSetNodes[0], sizeof(costPairs[0]), cmpCostPairs );
}

void	MBPnetwork::solveImprove( const int level )
{
	// Solve the problem given an existing starting solution.
	numPivots = 0;
	numPhase2Improvements = 0;
	numPhase3Improvements = 0;

	int	i;
	register MBPnode	*n;

	// Initialize everything.
	for( n = nodeList[0]; n != NULL; n = n->next )
		n->backwardCost = n->cost;
	for( n = nodeList[numSets-1]; n != NULL; n = n->next )
		n->forwardCost = n->cost;

	// Compute the forward costs.
	register MBPnode *from, *to;
	for( i = numSets - 1; i > 0; i-- )
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			from = n->backwardMate->from;
			from->forwardCost = n->forwardCost + from->cost;
		}
	// Compute the backward costs.
	for( i = 0; i < numSets - 1; i++ )
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			to = n->forwardMate->to;
			to->backwardCost = n->backwardCost + to->cost;
		}

	// Find the maximum cost.
	n = nodeList[0];
	cost = n->forwardCost;
	for( n = n->next; n != NULL; n = n->next )
		if( n->forwardCost > cost )
			cost = n->forwardCost;

	if( level > 0 )
		phase2();
	D( log.flush() );

	if( level > 1 )
		phase3();
	D( log.flush() );

	collectPaths();
}

void	MBPnetwork::solve( const int level )
{
	numPivots = 0;
	numPhase2Improvements = 0;
	numPhase3Improvements = 0;

	addDummyNodes();

	phase1();
	D( log.flush() );

	if( level > 0 )
		phase2();
	D( log.flush() );

	if( level > 1 )
		phase3();
	D( log.flush() );

	collectPaths();
}


//-----------------------------------------------------------------------
#ifdef DIAGNOSTICS
void MBPnetwork::printNetwork() const
{
	log << "*******************************************\n";
	register MBPnode *n;
	register MBParc *a;

	for( int i = 0; i < numSets - 1; i++ )
		for( n = nodeList[i]; n != NULL; n = n->next )
		{
			log << n->id << " (" << n->cost << "):\n";
			for( a = n->outList; a != NULL; a = a->outNext )
				log << '\t' << a->to->id << " (" << a->to->cost << ")\tcost: " << a->cost << '\n';

		}
	printSolution();
}

void	MBPnetwork::checkSolution() const
{
	D( log.flush() );
	register MBPnode *n, *nCur;
	cost_t	totalCost;
	for( n = nodeList[0]; n != NULL; n = n->next )
	{
		totalCost = 0;
		nCur = n;
		do
		{
			totalCost += nCur->cost;
			if( nCur->backwardCost != totalCost )
				Crash();
		} while( (nCur = nCur->forwardMateNode()) != NULL );
	}
	for( n = nodeList[numSets-1]; n != NULL; n = n->next )
	{
		totalCost = 0;
		nCur = n;
		do
		{
			totalCost += nCur->cost;
			if( nCur->forwardCost != totalCost )
				Crash();
		} while( (nCur = nCur->backwardMateNode()) != NULL );
	}
}

void	MBPnetwork::printSolution() const
{
	log << "*******************************************\n";
	register MBPnode *n;

	cost_t	costMin = INT_MAX;
	cost_t	costMax = INT_MIN;

	for( n = nodeList[0]; n != NULL; n = n->next )
	{
		cost_t	totalCost = 0;
		register MBPnode *nCur = n;
		do
		{
			log << nCur->id << " (" << nCur->cost << ") ";
			totalCost += nCur->cost;
		} while( (nCur = nCur->forwardMateNode()) != NULL );
		log << ": " << totalCost << '\n';
		if( totalCost < costMin )
			costMin = totalCost;
		if( totalCost > costMax )
			costMax = totalCost;
	}

	log << "min=" << costMin << " max=" << costMax << " %max deviation="
		<< (100.0 * (costMax - costMin) / ((costMin + costMax) / 2.0)) << '\n';
	log << "*******************************************\n";
	log.flush();
}
#endif // DIAGNOSTICS
