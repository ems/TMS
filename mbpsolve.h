//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef MBPSOLVE_H
#define MBPSOLVE_H

#include <stdlib.h>
#include <limits.h>
#include "bpsolve.h"

//#define DIAGNOSTICS
#if defined(DIAGNOSTICS)
#include <fstream>
using namespace std;

#define D(x)	x

#else

#define	D(x)

#endif

#define MBPinfiniteCost	(INT_MAX - 256)

class MBPnode;
class MBPnetwork;

class MBParc
{
public:
  friend class MBPnode;
  friend class MBPnetwork;

  cost_t	getCost() const			{ return cost; }
  void		setCost( const cost_t aCost ) { cost = aCost; }
  MBPnode	*getTo() const			{ return to; }
  MBPnode	*getFrom() const 		{ return from; }
  MBParc	*nextOutArc() const 	{ return outNext; }
  MBParc	*nextInArc() const 		{ return inNext; }
private:
  MBParc( MBPnode *aFrom, MBPnode *aTo, cost_t aCost )
    {
      from = aFrom;
      to = aTo;
      cost = aCost;
	  bpArc = NULL;

      next = prev = NULL;
      outNext = outPrev = NULL;
    }

  MBPnode	*from, *to;
  BParc		*bpArc;

  MBParc	*next, *prev;
  MBParc	*outNext, *outPrev;
  MBParc	*inNext, *inPrev;

  cost_t	cost;
};

class MBPnode
{
public:
  friend class MBPnetwork;
  friend class MBParc;

private:

  // The following functions are here because the compile will not inline them if they
  // are defined further down in the class definition.
  void init()
    {
      forwardMate = NULL;
      backwardMate = NULL;

      tnext = NULL;
    }

  void setForwardMate( MBParc *a  )
    { forwardMate = a; a->to->backwardMate = a; }
  void setBackwardMate( MBParc *a  )
    { backwardMate = a; a->from->forwardMate = a; }

public:
	void	setForwardMate( MBPnode *n )
	{
		for( register MBParc *a = outList; a; a = a->outNext )
			if( a->to == n )
			{
				setForwardMate( a );
				break;
			}
	}
	void	setBackwardMate( MBPnode *n )	{ n->setForwardMate( this ); }

  MBPnode	*forwardMateNode() const
  { return forwardMate == NULL ? NULL : forwardMate->to; }
  MBPnode	*backwardMateNode() const
  { return backwardMate == NULL ? NULL : backwardMate->from; }
  MBParc	*forwardMateArc() const
    { return forwardMate; }
  MBParc	*backwardMateArc() const
    { return backwardMate; }
  cost_t	forwardMateCost() const { return forwardMate ? forwardMate->cost : MBPinfiniteCost; }
  cost_t	backwardMateCost() const { return backwardMate ? backwardMate->cost : MBPinfiniteCost; }
  bool		forwardMatched() const { return forwardMate != NULL; }
  bool		backwardMatched() const { return backwardMate != NULL; }

  MBParc	*firstOutArc() const { return outList; }
  MBParc	*firstInArc() const { return inList; }

  ident_t	getID() const { return id; }
  cost_t	getCost() const { return cost; }
  cost_t	getForwardCost() const { return forwardCost; }
  cost_t	getBackwardCost() const { return backwardCost; }

  bool		isDummy() const { return dummy; }

private:
  MBPnode( ident_t aID, cost_t aCost, int aSet );
  ~MBPnode() {}

  ident_t	id;
  int		set;
  cost_t	cost, backwardCost, forwardCost;
  bool		dummy;
  MBPnode	*tnext;		// temporary next for topological sort

  MBParc	*forwardMate;
  MBParc	*backwardMate;

  MBParc	*outList;	// list of arcs out of this node
  MBParc	*inList;

  MBPnode	*next;		// next node in graph
  MBPnode	*prev;		// previous node in graph

  BPnode	*bpNode;

  void	insertInArc( MBParc *a )
    {
      a->inPrev = NULL;
      if( inList )
		inList->inPrev = a;
      a->inNext = inList;
      inList = a;
    }
  void	insertOutArc( MBParc *a )
    {
      a->outPrev = NULL;
      if( outList )
		outList->outPrev = a;
      a->outNext = outList;
      outList = a;
    }
  void	removeInArc( MBParc *a )
    {
      if( a->inPrev != NULL )
		a->inPrev->inNext = a->inNext;
      else
		inList = a->inNext;
      if( a->inNext != NULL )
		a->inNext->inPrev = a->inPrev;
    }
  void	removeOutArc( MBParc *a )
    {
      if( a->outPrev != NULL )
		a->outPrev->outNext = a->outNext;
      else
		outList = a->outNext;
      if( a->outNext != NULL )
		a->outNext->outPrev = a->outPrev;
    }
};

struct MBPcostPair
{
	MBPnode	*node;
	cost_t	cost;
};

class MBPnetwork
{
public:
  MBPnetwork( const int aNumSets, const cost_t aTarget );
  ~MBPnetwork();

  MBPnode	*addNode( ident_t id, cost_t cost, int set );
  void		deleteNode( MBPnode *n );
  MBParc	*addArc( MBPnode *aFrom, MBPnode *aTo, cost_t aCost );
  void		deleteArc( MBParc *a );
  void		deleteArc( MBPnode *from, MBPnode *to );

  MBParc	*findArc( MBPnode *from, MBPnode *to ) const;

  cost_t 	getCost() const { return cost; }
  int		getNumPivots() const { return numPivots; }
  int		getNumArcs() const { return numArcs; }
  int		getNumNodes() const { return numNodes; }
  int		getNumPhase2Improvements() const { return numPhase2Improvements; }
  int		getNumPhase3Improvements() const { return numPhase3Improvements; }

#if defined(DIAGNOSTICS)
  void		checkSolution() const;
  void		printNetwork() const;
  void		printSolution() const;
#endif

  void		solve( const int level = 2 );
  void		solveImprove( const int level = 2 );
  MBPnode	*getBestNode( const int i ) const { return costPairs[i].node; }

private:
	void	collectPaths();
  struct memobj
  {
    memobj( memobj *&list, size_t aSize, size_t aNum = 128 )
      {
		i = 0;
		num = aNum;
		size = aSize;
		buf = new char [num * size];
	
		next = list;
		list = this;
      }
    ~memobj() { delete [] buf; }
    
    void *getBuf() { return i < num ? (void *)(&buf[(i++) * size]) : NULL; }
    
    memobj	*next;
    char	*buf;
    size_t	i;
    size_t	num, size;
  };

  memobj	*arcMemList, *nodeMemList;
  MBParc	*arcFreeList;
  MBPnode	*nodeFreeList;

  cost_t	getDummyCost( const int i ) const;
  void		addDummyNodes();
  void 		phase1();
  void 		phase2();
  void 		phase3();

  cost_t	subpathCost( const MBPnode *from, const MBPnode *to ) const
  {
	return to->forwardCost - from->forwardMate->to->forwardCost;
  }

  cost_t	cost;	// best cost so far

  cost_t	target;

  MBParc	*arcList;
  int		numArcs;

  int		numPivots;
  int		numPhase2Improvements, numPhase3Improvements;

  MBPnode	**nodeList;
  int		*numSetNodes;
  int		numSets;
  int		numNodes;

  MBPcostPair	*costPairs;
};

#endif // MBPSOLVE_H

