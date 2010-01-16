//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "bpsolve.h"
#include <limits.h>
#include <string.h>
#include <new.h>

#ifdef DIAGNOSTICS
extern ofstream *bpLogFile;
#define log	(*bpLogFile)
#endif

//#define PRINT_NETWORK

#define Crash() do { int a = 1, b = 0; a = a / b; } while(0)

char *nodeName( const BPnode *n )
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

void BPnode::construct( ident_t aID, int aSet )
{
  set = aSet;
  id = aID;
  outList = inList = NULL;
  next = prev = NULL;
  mate = NULL;
  init();
}

BParc	*BPnetwork::findArc( BPnode *from, BPnode *to ) const
{
  register BParc *a;
  for( a = from->outList; a != NULL; a = a->outNext )
    if( a->to == to )
      break;
  return a;
}

BPnode	*BPnetwork::addNode( ident_t id, int set )
{
  register void	*buf;
  if( nodeFreeList != NULL )
    {
      buf = (void *)nodeFreeList;
      nodeFreeList = nodeFreeList->next;
    }
  else if( (buf = nodeMemList->getBuf()) == NULL )
    {
      new memobj( nodeMemList, sizeof(BPnode) );
      buf = nodeMemList->getBuf();
    }


  register BPnode *n = (BPnode *)(buf);
  n->construct( id, set );
  if( nodeList[set] != NULL )
    nodeList[set]->prev = n;
  n->next = nodeList[set];
  n->prev = NULL;
  nodeList[set] = n;
  ++numNodes;
  return n;
}

void BPnetwork::deleteNode( register BPnode *n )
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
  n->~BPnode();
  n->next = nodeFreeList;
  nodeFreeList = n;

  --numNodes;
}

BParc		*BPnetwork::addArc( BPnode *aFrom, BPnode *aTo, cost_t aCost, ident_t aID )
{
  if( aFrom->set != aTo->set-1 )
	  Crash();

  register void	*buf;
  if( arcFreeList != NULL )
    {
      buf = (void *)arcFreeList;
      arcFreeList = arcFreeList->next;
    }
  else if( (buf = arcMemList->getBuf()) == NULL )
    {
      new memobj( arcMemList, sizeof(BParc) );
      buf = arcMemList->getBuf();
    }

  register BParc *a = (BParc *)buf;
  a->construct( aFrom, aTo, aCost, aID );
  
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

void	BPnetwork::deleteArc( register BParc *a )
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
  a->~BParc();
  a->next = arcFreeList;
  arcFreeList = a;

  --numArcs;
}

void BPnetwork::deleteArc( BPnode *from, BPnode *to )
{
  for( register BParc *a = from->outList; a != NULL; a = a->outNext )
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

BPnetwork::BPnetwork()
{
  arcMemList = NULL;
  new memobj( arcMemList, sizeof(BParc), 1024 );
  nodeMemList = NULL;
  new memobj( nodeMemList, sizeof(BPnode) );
  arcFreeList = NULL;
  nodeFreeList = NULL;

  arcList = NULL;
  numArcs = 0;
  numNodes = 0;

  for( int set = 0; set < 2; set++ )
	nodeList[set] = NULL;

  cost = 0;

  numPivots = 0;
}

void	BPnetwork::reset()
{
  // Reset everything to get ready for another solve.
  register BParc *a, *aNext;
  register BPnode *n, *nNext;
  register int set;

  // Put all the arcs on the free list.
  if( arcFreeList == NULL )
	arcFreeList = arcList;
  else
  {
	  for( a = arcList; a != NULL; a = aNext )
	  {
		  aNext = a->next;
		  a->next = arcFreeList;
		  arcFreeList = a;
	  }
  }
  arcList = NULL;

  // Put all the nodes on the free list.
  for( set = 0; set < 2; set++ )
  {
	for( n = nodeList[set]; n != NULL; n = nNext )
	{
		nNext = n->next;
		n->next = nodeFreeList;
		nodeFreeList = n;
	}
	nodeList[set] = NULL;
  }

  cost = 0;

  numArcs = 0;
  numNodes = 0;
  numPivots = 0;
}

BPnetwork::~BPnetwork()
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
}

//-----------------------------------------------------------------------

BPnode	*BPnetwork::findAugmentingPath( BPnode *root, const cost_t costMax )
{
	D( log << "findAugmentingPath: starting at: " << root->id << '\n' );

	register BPnode *n, *to;
	register BPnode	*head, *tail, *shead = NULL;
	register BParc *a;
	BPnode	*toHead = NULL;	// head of an alternating path terminated by root.
	BPnode	*mateNode = root->mateNode();
	BParc	*mateArc = root->mateArc();

	// Grow an alternating path tree from the given root.
	root->arcToParent = NULL;
	head = tail = root;
	head->tnext = NULL;

	// Initialize the reset list.
	// This also marks the root as visited.
	shead = root->snext = root;
	do
	{
		n = head;
		
		if( n->set == 0 )
		{
			for( a = n->outList; a != NULL; a = a->outNext )
			{
				// Skip arcs with costs exceeding costMax and nodes that
				// we have already visited.
				if( a->cost >= costMax || a->to->visited() || a == mateArc )
					continue;

				to = a->to;

				// Setup the return path.
				to->arcToParent = a;

				// Check for termination.
				// This happens in 2 cases:
				// 1: We find an unmatched node.
				//    We assume that we started with an unmatched node.
				//    This will grow the cardinality
				//    of the matching by 1.
				// 2: We find another alternating path to the mate node of the root
				//    that only uses arcs with costs lower than costMax.
				if( !to->matched() || to == mateNode )
				{
					// We have an augmenting path.
					toHead = to;
					goto Cleanup;
				}

				// Put the node on the reset list.
				// This also marks the node as visited.
				to->snext = shead;
				shead = to;

				// Put the node on the BFS list.
				tail->tnext = to;
				to->tnext = NULL;
				tail = to;
			}
		}
		else // n->set == 1
		{
			BPnode	*from = n->mate->from;
			// Setup the return path.
			from->arcToParent = n->mate;

			// Put the node on the reset list.
			// This also marks it as visited.
			from->snext = shead;
			shead = from;

			// Put the node on the BFS list.
			tail->tnext = from;
			from->tnext = NULL;
			tail = from;
		}

		head = head->tnext;
	} while( head != NULL );

Cleanup:
	// Reset the nodes we visited in the alternating search tree.
	register BPnode *snext = shead;
	do
	{
		n = snext;
		D( log << "findAugmentingPath: cleaning: " << n->id << '\n' );

		snext = n->snext;
		n->snext = NULL;
	} while( n != root );

	D( toHead != NULL
			? (log << "findAugmentingPath: "
				<< "found augmenting path starting at " << toHead->id << '\n')
			: (log << "findAugmentingPath: "
				<< "no augmenting path\n") );
	return toHead;
}

void	BPnetwork::augmentOnPath( BPnode *toHead )
{
	register BPnode *n = toHead;
	for( ;; )
	{
		D( log << "augmentOnPath: augmenting node: " << n->id << '\n' );
		// Change the mate of the set1 node.
		n->setMate( n->arcToParent );

		// Move to a corresponding set0 node.
		n = n->arcToParent->from;

		// Check for termination.
		if( n->arcToParent == NULL )
			break;

		// Move back to a set1 node again.
		n = n->arcToParent->to;
	}
}

BPnode *BPnetwork::findMaximumNode()
{
	// Find the node with the maximum cost mate.
	register BPnode *n = nodeList[0];
	cost = n->mateCost();
	BPnode	*nodeMax = n;
	for( n = n->next; n != NULL; n = n->next )
		if( n->mateCost() > cost )
		{
			cost = n->mateCost();
			nodeMax = n;
		}
	return nodeMax;
}

BPnode *BPnetwork::findInitialSolution()
{
	if( numNodes == 0 )
		return NULL;

	BPnode *n, *toHead;

	for( n = nodeList[0]; n != NULL; n = n->next )
		n->init();
	for( n = nodeList[1]; n != NULL; n = n->next )
		n->init();

	for( n = nodeList[0]; n != NULL; n = n->next )
	{
		if( (toHead = findAugmentingPath(n, BPinfiniteCost + 1)) != NULL )
		{
			augmentOnPath( toHead );
			++numPivots;
			D( printSolution() );
		}
		else
			return n; // Return the infeasible node.
	}
	return NULL;
}

void BPnetwork::minimizeBottleneck()
{
	if( numNodes == 0 )
		return;

	BPnode	*n, *toHead;
	for( ;; )
	{
		n = findMaximumNode();
		D( log << "minimizeBottleneck: bottleneck: " << cost << '\n' );
		if( (toHead = findAugmentingPath(n, n->mateCost())) != NULL )
		{
			augmentOnPath( toHead );
			++numPivots;
			D( printSolution() );
		}
		else
			break;
	}
}

#ifdef DIAGNOSTICS
void BPnetwork::printNetwork() const
{
	log << "*******************************************\n";
	register BPnode *n;
	register BParc *a;

	for( n = nodeList[0]; n != NULL; n = n->next )
	{
		log << n->id << ":\n";
		for( a = n->outList; a != NULL; a = a->outNext )
			log << '\t' << a->to->id << "\tcost: " << a->cost << '\n';

	}
	printSolution();
}

void	BPnetwork::printSolution() const
{
	log << "*******************************************\n";
	register BPnode *n;

	for( n = nodeList[0]; n != NULL; n = n->next )
		if( n->mateNode() != NULL )
			log << n->id << "\tmatches\t" << n->mateNode()->getID()
				<< "\tcost: " << n->mateArc()->getCost() << '\n';
	log.flush();
}
#endif // DIAGNOSTICS
