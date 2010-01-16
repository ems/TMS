//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef SNSOLVE_H
#define SNSOLVE_H

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
typedef	int	flow_t;
typedef unsigned long 	ident_t;

#define SNInfiniteCost	INT_MAX
#define SNInfiniteFlow	INT_MAX

#define SNBigM		(SNInfiniteFlow >> 16)

class SNnode;
class SNnetwork;
class SNbasis;

class SNarc
{
public:
  friend class SNnode;
  friend class SNnetwork;
  friend class SNbasis;

  enum idenType { L, U, T };

  flow_t	getFlow() const;
  void		decrementFlow();

  cost_t  getCost() const		{ return cost; }
  void	  setCost( const cost_t aCost ) { cost = aCost; }
  SNnode  *getTo() const		{ return to; }
  SNnode  *getFrom() const 		{ return from; }
  SNarc	  *nextOutArc() const 		{ return outNext; }
  SNarc	  *nextInArc() const 		{ return inNext; }
private:
  SNarc( SNnode *aFrom, SNnode *aTo, cost_t aCost, flow_t aUpper = 1, idenType aIdent = L )
    {
      from = aFrom;
      to = aTo;
      cost = aCost;
      upper = aUpper;
      ident = aIdent;

      next = prev = NULL;
      outNext = outPrev = NULL;
    }

  SNnode	*from, *to;

  SNarc	*next, *prev;
  SNarc	*outNext, *outPrev;
  SNarc *inNext, *inPrev;

  cost_t	cost;
  flow_t	upper;
  flow_t	flowCur;

  int		ident;

  cost_t	pivotPotential() const;
  cost_t  	getCostContribution() const;
};

class SNnode
{
public:
  friend class SNnetwork;
  friend class SNarc;
  friend class SNbasis;

private:

  // The following functions are here because the compile will not inline them if they
  // are defined further down in the class definition.
  SNnode	*pred() const
    { return basic_arc->to == this ? basic_arc->from : basic_arc->to; }

  void removeChild( SNnode *n )
    {
      if( n->left_sibling == NULL )
	    child = n->right_sibling;
      else
	    n->left_sibling->right_sibling = n->right_sibling;
      if( n->right_sibling != NULL )
	    n->right_sibling->left_sibling = n->left_sibling;
    }

  void addChild( SNnode *n )
    {
      if( child != NULL )
	    child->left_sibling = n;
      n->left_sibling = NULL;
      n->right_sibling = child;
      child = n;
    }

  void changeBasicArc( SNarc *a )
    {
      SNnode	*pCur = pred();
      pCur->removeChild( this );

      basic_arc = a;
      pCur = pred();
      pCur->addChild( this );
    }

  void init()
    {
      flow = 0;
      potential = 0;
      child = NULL;
      level = 0;
      left_sibling = NULL;
      right_sibling = NULL;

      basic_arc = NULL;

      color = white;		// topo DFS flag.

      c = NULL;
      aout = NULL;
      tnext = NULL;
    }

public:
  enum SNnodeColor { white, grey, black };

  void setBasicArc( SNarc *a, const flow_t f = 0 )
    {
      basic_arc = a;
      basic_arc->ident = SNarc::T;

      flow = f;
      pred()->addChild( this );
	  level = pred()->level + 1;
    }

  SNarc	*firstOutArc() const { return outList; }
  SNarc *firstInArc() const { return inList; }

  ident_t	getID() const { return id; }

  SNarc	*getCycleArc() const { return aout; }

  SNnodeColor	getColor() const { return color; }

private:
  SNnode( ident_t aID );
  ~SNnode() {}

  ident_t	id;
  SNnodeColor	color;		// state for topo sort.
  SNnode	*tnext;		// temporary next for topological sort

  cost_t	potential;	// simplex potential

  int		level;

  SNnode	*child;		// first child node
  SNnode	*right_sibling; // right sibling node
  SNnode	*left_sibling;  // left sibling node

  SNarc		*basic_arc;	// basic arc between this node the the predecessor

  flow_t	flow;		// flow along basic arc

  SNnode	*c;		// used for DFS without stack

  SNarc		*outList;	// list of arcs out of this node
  SNarc		*inList;
  SNarc		*aout;		// pointer to next out arc for DFS without stack

  SNnode	*next;		// next node in graph
  SNnode	*prev;		// previous node in graph

  int	basicArcPointsUp() const  { return basic_arc->from == this; }
  int	basicArcPointDown() const { return basic_arc->to == this; }

  flow_t	residualForward() const
    {
      return basic_arc->to == this
		? basic_arc->upper - flow
		: flow;
    }
  flow_t	residualBackward() const
    {
      return basic_arc->to == this
		? flow
		: basic_arc->upper - flow;
    }

  cost_t	basicArcPotential() const
    {
      return basic_arc->to == this
		? basic_arc->cost
		: -basic_arc->cost;
    }

  void	updateResidualForward( flow_t &resMin );
  void	updateResidualBackward( flow_t &resMin );

  void	insertInArc( SNarc *a )
    {
      a->inPrev = NULL;
      if( inList )
		inList->inPrev = a;
      a->inNext = inList;
      inList = a;
    }
  void	insertOutArc( SNarc *a )
    {
      a->outPrev = NULL;
      if( outList )
		outList->outPrev = a;
      a->outNext = outList;
      outList = a;
    }
  void	removeInArc( SNarc *a )
    {
      if( a->inPrev != NULL )
		a->inPrev->inNext = a->inNext;
      else
		inList = a->inNext;
      if( a->inNext != NULL )
		a->inNext->inPrev = a->inPrev;
    }
  void	removeOutArc( SNarc *a )
    {
      if( a->outPrev != NULL )
		a->outPrev->outNext = a->outNext;
      else
		outList = a->outNext;
      if( a->outNext != NULL )
		a->outNext->outPrev = a->outPrev;
    }
};

inline flow_t	SNarc::getFlow() const
{
  return ident == L ? 0 : ident == U ? upper
    : (from->basic_arc == this ? from : to)->flow;
}

inline cost_t	SNarc::pivotPotential() const
{
  return ident == L
    ? from->potential + cost - to->potential
    : to->potential - cost - from->potential;
}

inline cost_t	SNarc::getCostContribution() const
{
  return ident == L ? 0
    :    ident == U ? upper * cost
    :    (from->basic_arc == this ? from : to)->flow * cost;
}

inline void SNarc::decrementFlow()
{
	if( ident == U ) --upper;
	else --(from->basic_arc == this ? from : to)->flow;
}

class SNnetwork
{
friend class SNbasis;
public:
  SNnetwork();
  ~SNnetwork();

  void setRoot( SNnode *aRoot ) { root = aRoot; }
  SNnode	*getRoot() const { return root; }

  SNnode	*addNode( ident_t id = 0 );
  void		deleteNode( SNnode *n );
  SNarc		*addArc( SNnode *aFrom, SNnode *aTo, cost_t aCost,
			 flow_t aUpper = 1, SNarc::idenType aIdent = SNarc::L );
  void		deleteArc( SNarc *a );

  SNarc		*findArc( SNnode *from, SNnode *to ) const;
  void		deleteArcSolve( SNarc *a );
  void		deleteArc( SNnode *from, SNnode *to );
  void		recostArcSolve( SNarc *a, cost_t newCost );
  void 		recostArc( SNarc *a, cost_t newCost ) { a->cost = newCost; }

  void 		findZeroFlowBasis( SNnode *s, SNnode *t );
  void 		solve();
  cost_t 	getCost() const { return cost; }
  int		getNumPivots() const { return numPivots; }
  int		getNumArcs() const { return numArcs; }
  int		getNumNodes() const { return numNodes; }

  void	saveSolution();
  void	restoreSolution();

  void	        basisEpilog();
#if defined(DIAGNOSTICS)
  void		printBasis();
  void		printNetwork() const;
#endif

private:
#if defined(DIAGNOSTICS)
  void		checkBasis();
  void		checkStrongBasis();
#endif

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
  SNarc		*arcFreeList;
  SNnode	*nodeFreeList;

  void		solveValidDual();

  void		topoSort( SNnode *s, SNnode *t );

  SNnode	*findApex( flow_t &resMin, SNarc *a ) const;
  SNnode	*cycleFlow(int &cutOnFromPath, const flow_t resMin, const SNnode *apex,
			   const SNarc *a );
  SNarc		*findPivotArc();
  void		updateDuals( register SNnode *n );

  void		pivot( SNarc *a );

  int		nullPivot( SNarc *a );
  void		markTree( SNnode *n, const SNnode::SNnodeColor color );

  SNnode	*root;
  SNnode	*source, *sink;

  cost_t	cost;

  SNarc		*arcList;
  int		numArcs;

  int		numPivots;

  SNnode	*nodeList;
  SNnode	*nodeTList;
  int		numNodes;

  int		K, J;		// multiple partial pricing parameters.

  SNarc		**candidates;
  int		numCandidates;
  int		candidatesMax;

  SNarc		*nextPosition;
  int		lookahead;

  struct SNarcUpper
  {
  	SNarc	*arc;
   flow_t upper;
  };
  SNarcUpper	*arcUpper;
  int				numArcUpper;

  struct SNnodeFlow
  {
  	SNnode *node;
   flow_t	flow;
  };
  SNnodeFlow	*nodeFlow;
  int			numNodeFlow;
};

class SNbasis
{
public:
  SNbasis( SNnetwork *aNetwork = NULL );
  ~SNbasis();

  void capture( SNnetwork *aNetwork );
  void restore();

private:
  SNnetwork *network;

  struct ArcInfo
  {
    SNarc	*arc;
    int		ident;
  };

  struct NodeInfo
  {
    SNnode	*node;
    SNarc	*basic_arc;
    flow_t	flow;
    int		level;
    int		potential;
  };

  ArcInfo	*arcInfo;
  NodeInfo	*nodeInfo;
};

#endif // SNSOLVE_H

