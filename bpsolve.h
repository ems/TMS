//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef BPSOLVE_H
#define BPSOLVE_H

#include <stdlib.h>
#include <limits.h>

//#define DIAGNOSTICS
#if defined(DIAGNOSTICS)
#include <fstream>
using namespace std;

#define D(x)	x

#else

#define	D(x)

#endif

typedef int 	cost_t;
typedef	int		flow_t;
typedef unsigned long 	ident_t;

#define BPinfiniteCost	(INT_MAX - 256)

class BPnode;
class BPnetwork;

class BParc
{
public:
  friend class BPnode;
  friend class BPnetwork;

  cost_t  getCost() const		{ return cost; }
  void	  setCost( const cost_t aCost ) { cost = aCost; }
  BPnode  *getTo() const		{ return to; }
  BPnode  *getFrom() const 		{ return from; }
  BParc	  *nextOutArc() const 	{ return outNext; }
  BParc	  *nextInArc() const 	{ return inNext; }

  ident_t	getID() const		{ return id; }
private:
  void construct( BPnode *aFrom, BPnode *aTo, cost_t aCost, ident_t aID = 0 )
    {
      from = aFrom;
      to = aTo;
      cost = aCost;
      id = aID;

      next = prev = NULL;
      outNext = outPrev = NULL;
    }
  BParc( BPnode *aFrom, BPnode *aTo, cost_t aCost, ident_t aID = 0 )
    {
      construct( aFrom, aTo, aCost, aID );
    }

  BPnode	*from, *to;
  ident_t	id;

  BParc	*next, *prev;
  BParc	*outNext, *outPrev;
  BParc *inNext, *inPrev;

  cost_t	cost;
};

class BPnode
{
public:
  friend class BPnetwork;
  friend class BParc;

private:

  // The following functions are here because the compile will not inline them if they
  // are defined further down in the class definition.
  void init()
    {
      mate = NULL;

      snext = NULL;
      tnext = NULL;
    }
public:
  void setMate( BParc *a  )
    { mate = a; mateNode()->mate = a; }

  BPnode	*mateNode() const
  { return mate == NULL ? NULL : mate->to == this ? mate->from : mate->to; }
  BParc	*mateArc() const
    { return mate; }
  cost_t	mateCost() const { return mate ? mate->cost : BPinfiniteCost; }
  int		matched() const { return mate != NULL; }

  BParc	*firstOutArc() const { return outList; }
  BParc *firstInArc() const { return inList; }

  ident_t	getID() const { return id; }

private:
  void construct( ident_t aID, int aSet );
  BPnode( ident_t aID, int aSet ) { construct( aID, aSet); }
  ~BPnode() {}

  ident_t	id;
  int		set;
  BPnode	*tnext;		// temporary next for topological sort

  BParc		*mate;		// arc that is the mate of this node.

  BPnode	*snext;		// used for BFS reset list.

  BParc		*outList;	// list of arcs out of this node
  BParc		*inList;

  BPnode	*next;		// next node in graph
  BPnode	*prev;		// previous node in graph

  BParc		*arcToParent;
  BPnode	*getParent() const
  { return arcToParent->to == this ? arcToParent->from : arcToParent->to; }
  bool		visited() const { return snext != NULL; }

  void	insertInArc( BParc *a )
    {
      a->inPrev = NULL;
      if( inList )
		inList->inPrev = a;
      a->inNext = inList;
      inList = a;
    }
  void	insertOutArc( BParc *a )
    {
      a->outPrev = NULL;
      if( outList )
		outList->outPrev = a;
      a->outNext = outList;
      outList = a;
    }
  void	removeInArc( BParc *a )
    {
      if( a->inPrev != NULL )
		a->inPrev->inNext = a->inNext;
      else
		inList = a->inNext;
      if( a->inNext != NULL )
		a->inNext->inPrev = a->inPrev;
    }
  void	removeOutArc( BParc *a )
    {
      if( a->outPrev != NULL )
		a->outPrev->outNext = a->outNext;
      else
		outList = a->outNext;
      if( a->outNext != NULL )
		a->outNext->outPrev = a->outPrev;
    }
};

class BPnetwork
{
public:
  BPnetwork();
  ~BPnetwork();

  BPnode	*addNode( ident_t id, int set );
  void		deleteNode( BPnode *n );
  BParc		*addArc( BPnode *aFrom, BPnode *aTo, cost_t aCost, ident_t aID = 0 );
  void		deleteArc( BParc *a );
  void		deleteArc( BPnode *from, BPnode *to );

  BParc		*findArc( BPnode *from, BPnode *to ) const;

  BPnode    *findInitialSolution();
  void 		minimizeBottleneck();
  void		solve()
  {
	  findInitialSolution();
	  minimizeBottleneck();
  }
  bool		improveAtNode( BPnode *n )
  {
	  BPnode *toHead = findAugmentingPath(n, n->mateArc()->cost);
	  return toHead == NULL ? false : (++numPivots, augmentOnPath(toHead), true);
  }
  void		reset();

  cost_t 	getCost() const { return cost; }
  int		getNumPivots() const { return numPivots; }
  int		getNumArcs() const { return numArcs; }
  int		getNumNodes() const { return numNodes; }

  void	        basisEpilog();
#if defined(DIAGNOSTICS)
  void		printNetwork() const;
  void		printSolution() const;
#endif

private:
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
  BParc		*arcFreeList;
  BPnode	*nodeFreeList;

  BPnode	*findAugmentingPath( BPnode *root, const cost_t costMax );
  BPnode	*findMaximumNode();
  void		augmentOnPath( BPnode *n );

  cost_t	cost;

  BParc		*arcList;
  long		numArcs;

  long		numPivots;

  BPnode	*nodeList[2];
  long		numNodes;
};

#endif // BPSOLVE_H

