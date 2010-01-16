//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "snsolve.H"
#include <limits.h>
#include <string.h>
#include <new.h>

#ifdef DIAGNOSTICS
extern ofstream *blockLog;
#define log	(*blockLog)
#endif

//#define PRINT_BASIS
//#define PRINT_NETWORK

#define Crash() do { int a = 1, b = 0; a = a / b; } while(0)

char *nodeName( const SNnode *n )
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

SNnode::SNnode( ident_t aID )
{
  id = aID;
  outList = inList = NULL;
  next = prev = NULL;
  init();
}

SNarc	*SNnetwork::findArc( SNnode *from, SNnode *to ) const
{
  register SNarc *a;
  for( a = from->outList; a != NULL; a = a->outNext )
    if( a->to == to )
      break;
  return a;
}

SNnode	*SNnetwork::addNode( ident_t id )
{
  void	*buf;
  if( nodeFreeList != NULL )
    {
      buf = (void *)nodeFreeList;
      nodeFreeList = nodeFreeList->next;
    }
  else if( (buf = nodeMemList->getBuf()) == NULL )
    {
      new memobj( nodeMemList, sizeof(SNnode) );
      buf = nodeMemList->getBuf();
    }


  SNnode *n = new (buf) SNnode( id );
  if( nodeList != NULL )
    nodeList->prev = n;
  n->next = nodeList;
  n->prev = NULL;
  nodeList = n;
  ++numNodes;
  return n;
}

void SNnetwork::deleteNode( register SNnode *n )
{
  while( n->outList )
    deleteArc( n->outList );
  while( n->inList )
    deleteArc( n->inList );
  if( n->prev != NULL )
    n->prev->next = n->next;
  else
    nodeList = n->next;
  if( n->next != NULL )
    n->next->prev = n->prev;

  // Destroy the node, but recover the memory.
  n->~SNnode();
  n->next = nodeFreeList;
  nodeFreeList = n;

  --numNodes;
}

SNarc		*SNnetwork::addArc( SNnode *aFrom, SNnode *aTo, cost_t aCost,
				    flow_t aUpper, SNarc::idenType aIdent )
{
  void	*buf;
  if( arcFreeList != NULL )
    {
      buf = (void *)arcFreeList;
      arcFreeList = arcFreeList->next;
    }
  else if( (buf = arcMemList->getBuf()) == NULL )
    {
      new memobj( arcMemList, sizeof(SNarc) );
      buf = arcMemList->getBuf();
    }

  SNarc *a = new (buf) SNarc(aFrom, aTo, aCost, aUpper, aIdent);
  
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

void	SNnetwork::deleteArc( register SNarc *a )
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
  a->~SNarc();
  a->next = arcFreeList;
  arcFreeList = a;

  --numArcs;
}

void SNnetwork::deleteArc( SNnode *from, SNnode *to )
{
  for( register SNarc *a = from->outList; a != NULL; a = a->outNext )
    if( a->to == to )
      {
		deleteArc( a );
		return;
      }
}

void SNnetwork::recostArcSolve( SNarc *a, cost_t newCost )
{
  a->cost = newCost;
  if( a->ident != SNarc::T )
    {
      // Arc is not in the basis tree.
      // If there is no pivot potential from changing the cost, then
      // the network is OK and we don't have to do anything.
      if( a->pivotPotential() < 0 )
	{
	  pivot( a );		// kick start the first pivot we know we need to do.
	  solveValidDual();	// do any other pivots that are necessary.
	}
    }
  else
    {
      // Arc is in the basis tree.
      // Minimally update the dual and resolve.
      updateDuals( a->from->basic_arc == a ? a->from : a->to );
      solveValidDual();
    }
}

void SNnetwork::deleteArcSolve( SNarc *a )
{
  if( a->ident == SNarc::L )
    {
      deleteArc( a );	// No flow, safe to delete.
      return;
    }
  else if( a->ident == SNarc::U )
    {
      // The arc is at its upper bound.  Attempt to pivot it out of the solution.
      recostArcSolve( a, SNBigM );
      // Did we manage to pivot it out of the basis?
      if( a->ident == SNarc::L )
	{
	  deleteArc( a );
	  return;
	}
      if( a->getFlow() > 0 )
	{
	  // The network would go infeasible without this arc.
	  // Just return for now... FIXLATER.
	  return;
	}
    }

  // The arc is in the basis.  Try to get it out.
  if( a->getFlow() > 0 )
    {
      a->cost = SNBigM;
      updateDuals( a->from->basic_arc == a ? a->from : a->to );
      solveValidDual();
      // Did we manage to pivot it out of the basis?
      if( a->ident == SNarc::L )
	{
	  deleteArc( a );
	  return;
	}
    }

  // The arc is in the basis, but has zero flow.  We need to pivot it out.
  if( !nullPivot(a) )
    {
      // Null pivot failed - We need a new basis.
      deleteArc( a );
      findZeroFlowBasis( source, sink );
      solve();
    }
}

//-----------------------------------------------------------------------------------

void SNnetwork::markTree( register SNnode *n, const SNnode::SNnodeColor color )
{
  // This is a bit fun.  We do a depth-first search
  // on the graph without doing recursion.
  // The current child node is maintained in each node (in field c)
  // so the path down the graph actually acts as its own stack.

  register SNnode *base = n;		// Save the base of the tree.
  register SNnode *t;			// Temporary register.

 enterNode:
  n->color = color;
  n->c = n->child;

 processChild:
  if( (t = n->c) != NULL )
    {
      n = t;
      goto enterNode;
    }

  // leaveNode:
  if( n != base )
    {
      n = n->pred();
      n->c = n->c->right_sibling;
      goto processChild;
    }
}

int SNnetwork::nullPivot( SNarc *a )
{
  // Mark the non-root part of the tree black.
  markTree( a->from, SNnode::black );

  // Try to find a bridge arc from the root tree to the non-root tree.
  register SNarc *bridge;
  for( bridge = a->from->outList; bridge != NULL; bridge = bridge->outNext )
    if( bridge->to->color == SNnode::white && bridge->ident == SNarc::L )
      break;

  // Cleanup
  markTree( a->from, SNnode::white );

  // If there is no bridge, we give up.
  if( bridge == NULL )
    return 0;

  // Reparent the 'from' node with the bridge arc.
  // This preserves a strong basis.
  a->from->changeBasicArc( bridge );

  updateDuals( bridge->from );
  // It is not necessary to resolve because we did not cycle any flow.
  return 1;
}

void SNnetwork::updateDuals( SNnode *subtreeRoot )
{
	D( ofs << "Updating duals starting at: " << nodeName(subtreeRoot) << '\n' );

	register	SNnode	*parent, *child = subtreeRoot;

	// Update the subtreeRoot with the arc that connects it to the main tree.
	if( child->basic_arc == NULL )
    {
		child->level = 0;
		child->potential = 0;
    }
	else
    {
		parent = child->pred();
		child->potential = parent->potential + child->basicArcPotential();
		child->level = parent->level + 1;
    }
	
	parent = child;
	register	SNnode	*tail = parent; parent->c = NULL;
	do
	{
		for( child = parent->child; child != NULL; child = child->right_sibling )
		{
			// Update this child's potential and level.
			child->potential = parent->potential + child->basicArcPotential();
			child->level = parent->level + 1;

			// Put this child onto the BFS list.
			tail->c = child; child->c = NULL; tail = child;
		}
	} while( (parent = parent->c) != NULL );

	D( checkBasis() );
	D( checkStrongBasis() );
}

inline	void	SNnode::updateResidualForward( flow_t &resMin )
{
  flow_t	resCur;
  if( basic_arc->from == this )
    {
      // forward flow
      resCur = basic_arc->upper - flow;
      if( resCur < resMin )
	resMin = resCur;
    }
  else
    {
      // backward flow
      resCur = flow;
      if( resCur < resMin )
	resMin = resCur;
    }
}

inline	void	SNnode::updateResidualBackward( flow_t &resMin )
{
  register flow_t	resCur;
  if( basic_arc->to == this )
    {
      // forward flow
      resCur = basic_arc->upper - flow;
      if( resCur < resMin )
	resMin = resCur;
    }
  else
    {
      // backward flow
      resCur = flow;
      if( resCur < resMin )
	resMin = resCur;
    }
}

SNnode	*SNnetwork::findApex( flow_t &resMin, SNarc *a ) const
{
  // Find the apex root by comparing the node levels.
  // If one level is smaller than the other, move up the tree
  // on that side.
  //
  // If the levels are equal, either we have
  // found the apex root, or we are at the same level on the way.

  resMin = a->upper;

  register SNnode	*from, *to;

  if( a->ident == SNarc::L )
    {
      // Forward flow.
      from = a->from;
      to = a->to;
    }
  else
    {
      // Backward flow.
      from = a->to;
      to = a->from;
    }

  // Crawl up the branches of the tree.

  if( from->level > to->level )
    {
      // Move up the 'from' branch as far as necessary.
      do {
	       from->updateResidualBackward( resMin );
	       from = from->pred();
      } while( from->level > to->level );
    }
  else if( to->level > from->level )
    {
      // Move up the 'to' branch as far as necessary.
      do {
	        to->updateResidualForward( resMin );
	        to = to->pred();
      } while( to->level > from->level );
    }

  // Move up both branches until we find the apex.
  while( from != to )
    {
      from->updateResidualBackward( resMin );
      from = from->pred();

      to->updateResidualForward( resMin );
      to = to->pred();
    }
  return from;
}

SNnode *SNnetwork::cycleFlow(int &cutOnFromPath, const flow_t resMin, const SNnode *apex,
			     const SNarc *a )
{
  D( log << "resMin=" << resMin << '\n' );

  cost += a->pivotPotential() * resMin;

  register SNnode	*from, *to;

  if( a->ident == SNarc::L )
    {
      // Forward flow.
      from = a->from;
      to = a->to;
    }
  else
    {
      // Backward flow.
      from = a->to;
      to = a->from;
    }

  // Crawl up each branch of the tree and update the flows.

  SNnode	*fromCutNode = NULL;
  while( from != apex )
    {
      if( from->basic_arc->to == from )
	{
	  from->flow += resMin;
	  if( from->flow == from->basic_arc->upper && fromCutNode == NULL )
	    fromCutNode = from;
	  from = from->basic_arc->from;
	}
      else
	{
	  from->flow -= resMin;
	  if( from->flow == 0 && fromCutNode == NULL )
	    fromCutNode = from;
	  from = from->basic_arc->to;
	}
    }

  SNnode	*toCutNode = NULL;
  while( to != apex )
    {
      if( to->basic_arc->from == to )
	{
	  to->flow += resMin;
	  if( to->flow == to->basic_arc->upper )
	    toCutNode = to;
	  to = to->basic_arc->to;
	}
      else
	{
	  to->flow -= resMin;
	  if( to->flow == 0 )
	    toCutNode = to;
	  to = to->basic_arc->from;
	}
    }

  if( toCutNode != NULL )
    {
      cutOnFromPath = 0;
      return toCutNode;
    }
  cutOnFromPath = 1;
  return fromCutNode;
}

void	SNnetwork::pivot( SNarc *a )
{
  D( log << "Entering arc: (" << nodeName(a->from) << ',' );
  D( log << nodeName(a->to) << ") " << (a->ident == SNarc::U ? 'U' : 'L') );
  D( log << " cost=" << a->cost << " potential=" << a->pivotPotential() << '\n' );

  flow_t	resMin;		// minimum residual

  // Cycle as much flow around as possible.
  int cutOnFromPath;
  SNnode	*apex = findApex( resMin, a );
  SNnode	*cutNode = cycleFlow( cutOnFromPath, resMin, apex, a );

  if( cutNode == NULL )
    {
      // No pivot required.
      // The entering arc was the limiting arc.
      a->ident = (a->ident == SNarc::L ? SNarc::U : SNarc::L);
      return;
    }

  // Update the tree structure.
  SNnode	*from, *to;
  flow_t	flow;
  if( a->ident == SNarc::L )
    {
      // Forward flow.
      from = a->from;
      to = a->to;
      flow = resMin;
    }
  else
    {
      // Backward flow.
      from = a->to;
      to = a->from;
      flow = a->upper - resMin;
    }

  if( resMin == a->upper &&
      ((cutNode->flow == 0                         && cutNode->basic_arc->from == cutNode) ||
       (cutNode->flow == cutNode->basic_arc->upper && cutNode->basic_arc->to   == cutNode)) )
    {
      // No pivot required.
      // The entering arc is at a bound,
      // and the existing basis is still strong.
      a->ident = (flow == 0 ? SNarc::L : SNarc::U);
      return;
    }

  //  D( log << "Leaving arc: (" << (cutNode->basic_arc)->from->getID() << ','
  //     << (cutNode->basic_arc)->to->getID() << ")\n" );
  D( log << "Leaving arc: (" << nodeName((cutNode->basic_arc)->from) << ',' );
  D( log << nodeName((cutNode->basic_arc)->to) << ")\n" );

  // Update the arc flags.
  cutNode->basic_arc->ident = (cutNode->flow == 0 ? SNarc::L : SNarc::U);
  a->ident = SNarc::T;

  // Reparent the nodes around the cycle.
  SNnode	*n = cutOnFromPath ? from : to;

  SNarc	*basic_arc = a;
  for( ;; )
    {
      SNnode	*next = n->pred();
      SNarc	*next_basic_arc = n->basic_arc;
      flow_t	next_flow = n->flow;

      n->changeBasicArc( basic_arc );
      n->flow = flow;

      if( n == cutNode )
		break;
      n = next;
      basic_arc = next_basic_arc;
      flow = next_flow;
    }

  D( checkStrongBasis() );

  // Update the duals on the modified subtree.
  updateDuals( cutOnFromPath ? from : to );

  D( log.flush() );
}

SNarc	*SNnetwork::findPivotArc()
{
  register SNarc	**a, **dest;
  register int		i, j;
  cost_t	bestPotential, potential;
  SNarc	*bestArc;

  a = dest = candidates;
  bestPotential = 0;
  bestArc = NULL;
  // Find the arc with the greatest potential.
  for( i = 0; i < numCandidates; i++, a++ )
    {
      if( (*a)->ident != SNarc::T )
	{
	  potential = (*a)->pivotPotential();
	  if( potential < bestPotential )
	    {
	      bestArc = *a;
	      bestPotential = potential;
	    }
	  // Only keep this arc if it has a negative potential.
	  if( potential < 0 )
	    (*dest++) = *a;
	}
    }
  numCandidates = dest - candidates;

  // If there are at least J arcs, just take the best pivot found so far.
  if( numCandidates >= J && bestArc != NULL )
    return bestArc;

  i = 0;
  a = dest;
  // We need to add more arcs to the search list.
  do
    {
      // Scan the next list chunk in an attempt to get the required number of arcs.
      for(j = 0;
	  j < lookahead;
	  j++, nextPosition = (nextPosition->next == NULL ? arcList : nextPosition->next) )
	{
	  if( nextPosition->ident == SNarc::T )
	    continue;
	  potential = nextPosition->pivotPotential();
	  if( potential < 0 )
	    {
		// Update the best arc found.
	      if( potential < bestPotential )
		{
		  bestArc = nextPosition;
		  bestPotential = potential;
		}

		// Add this new arc to the list.
	      (*a++) = nextPosition;
	      numCandidates++;

		// If we fill up the list, return what we have.
	      if( numCandidates >= (K + J) )
		return bestArc;
	    }
	}

      // If looking in the next chunk gets us enough arcs in the buffer,
      // return what we have.
      if( numCandidates > J )
	return bestArc;

      i += lookahead;
    } while( i <= numArcs );

  // If we checked everything, just return the best one so far.
  if( numCandidates > 0 )
    return bestArc;

  // Else, the list is truly empty and we are done.
#ifdef DIAGNOSTICS
  {
    // Check that there are absolutly no pivot arcs left.
    updateDuals( root );
    register SNarc	*a;
    for( a = arcList; a != NULL; a = a->next )
      if( a->ident != SNarc::T )
	{
	  potential = a->pivotPotential();
	  if( potential < 0 )
	    Crash();
	}
  }
#endif
  return NULL;
}

void	SNnetwork::solveValidDual()
{
  if( numArcs <= 10000 )
    {
      K = 30;
      J = 5;
    }
  else if( numArcs <= 100000 )
    {
      K = 50;
      J = 10;
    }
  else
    {
      K = 200;
      J = 20;
    }
  if( K + J >= candidatesMax )
    {
      delete [] candidates;
      candidatesMax = K + J + 64;
      candidates = new SNarc * [candidatesMax];
    }
  numCandidates = 0;
  nextPosition = arcList;

  lookahead = numArcs / K;
  if( lookahead < 1 )
    lookahead = 1;

  D( checkBasis() );
  D( checkStrongBasis() );

  numPivots = 0;
  SNarc	*a;
  while( (a = findPivotArc()) != NULL )
    {
      D( log << "Basis before:\n" );
      D( printBasis() );
      D( checkBasis() );
      D( checkStrongBasis() );

      pivot( a );
      ++numPivots;

      D( log << "Basis after:\n" );
      D( printBasis() );
      D( checkBasis() );
      D( checkStrongBasis() );
	  D( log.flush() );
    }

  numCandidates = 0;
}

void	SNnetwork::solve()
{
  updateDuals( root );
  solveValidDual();
}

void SNnetwork::basisEpilog()
{
  D( checkStrongBasis() );
  cost = 0;
  for( register SNarc *a = arcList; a != NULL; a = a->next )
    cost += a->getCostContribution();
}

void SNnetwork::topoSort( SNnode *s, SNnode *t )
{
  // This is a bit fun.  We do a depth-first search
  // on the graph without doing recursion.
  // Here we use the c field to record the parent,
  // and use the aout to record the next out arc.

  nodeTList = NULL;

  register SNnode *n;
  register SNnode *p;				// Temporary register.

  // Make sure we don't go beyond the t node, so mark it as done.
  t->color = SNnode::black;

  // Start the DFS at the s node.
  n = s;
  n->c = NULL;

 enterNode:
  n->color = SNnode::grey;
  n->aout = n->outList;

 processChild:
  while( n->aout != NULL )
    {
      p = n->aout->to;
      n->aout = n->aout->outNext;

      if( p->color == SNnode::white )
	{
	  p->c = n;	// Save the parent.
	  n = p;	// Enter this child.
	  goto enterNode;
	}
    }

  // leaveNode:
  n->color = SNnode::black;	// Flag this node as done.
  // Put this node on the topological sort list.
  n->tnext = nodeTList;
  nodeTList = n;

  if( (n = n->c) != NULL )	// Crawl back up the stack.
    goto processChild;
}

void	SNnetwork::findZeroFlowBasis( SNnode *s, SNnode *t )
{
  source = s;
  sink = t;

  register SNnode	*n;
  register SNarc	*a;

  // Reset the basis and DFS flag for all nodes.
  for( n = nodeList; n != NULL; n = n->next )
    n->init();

  // Init the arc status.
  for( a = arcList; a != NULL; a = a->next )
    a->ident = SNarc::L;

	// Do a breadth-first search from the sink connecting all
	// nodes to their parents.
	sink->tnext = NULL;
	register SNnode	*head = sink, *tail = sink;
	do
	{
		for( a = head->inList; a != NULL; a = a->inNext )
		{
			if( a->from->basic_arc != NULL || a->from == sink )
				continue;

			a->from->setBasicArc( a );

			tail->tnext = a->from;
			tail = a->from;
			tail->tnext = NULL;
		}
		// Follow the next node in the BFS list.
		n = head;
		head = head->tnext;
		n->tnext = NULL; // Cleanup
	} while( head != NULL );

  // Make the sink the root.
  setRoot( sink );

  D( checkBasis() );
  D( checkStrongBasis() );

  basisEpilog();
}

#if defined(DIAGNOSTICS)
extern char *formatObj( const ident_t t );

void	SNnetwork::checkBasis()
{
 register SNnode *n;

  for( n = nodeList; n != NULL; n = n->next )
    if( n->basic_arc == NULL )
      {
	    // Check that the only node without a basic arc is the root.
	    if( n != root )
		{
			D( log << "bad node is: " << formatObj(n->id) << '\n' );
			D( log.flush() );
			Crash();
		}
      }
    else
      {
	    // Check that a node's level is greater than its parent's.
	    if( n->level <= n->pred()->level )
	      Crash();
      }
}

void	SNnetwork::checkStrongBasis()
{
  // This is a bit fun.  We do a depth-first search
  // on the graph without doing recursion.
  // The current child node is maintained in each node (in field c)
  // so the path down the graph actually acts as its own stack.

  register SNnode *n = root;
  register SNnode *base = root;		// Save the base of the tree.

 enterNode:
  n->c = n->child;

 processChild:
  if( n->c != NULL )
    {
      n = n->c;
      if( n->flow == 0 )
	{
	  // Check that a basic arc with 0 flow points upwards towards root.
	  if( n->basic_arc->from != n )
	    {
	      D( log << "Error: zero flow basic arc at node: " << n->getID() << " not upward pointing\n" );
	      Crash();
	    }
	}
      else if( n->flow == n->basic_arc->upper )
	{
	  // Check that a basic arc at UB points downwards away from root.
	  if( n->basic_arc->to != n )
	    {
	      D( log << "Error: At capacity basic arc at node: " << n->getID() << " not downward pointing\n" );
	      Crash();
	    }
	}
      goto enterNode;
    }

  // leaveNode:
  if( n != base )
    {
      n = n->pred();
      n->c = n->c->right_sibling;
      goto processChild;
    }
}

void	SNnetwork::printBasis()
{
#ifdef PRINT_BASIS
  static int level = -1;

  if( level == -1 )
    log << "Pivots: " << numPivots << '\n';

  log << "------------------------------------------------------\n";

  int		i;

  // This is a bit fun.  We do a depth-first search
  // on the graph without doing recursion.
  // The current child node is maintained in each node (in field c)
  // so the path down the graph actually acts as its own stack.

  register SNnode *n = root;

 enterNode:
  level++;

  for( i = 0; i < level; i++ )
    log << "    ";

  log << nodeName(n) << ':' << n->level
       << ':' << n->flow << ':' << n->potential << '\n';
  n->c = n->child;

 processChild:
  if( n->c != NULL )
    {
      n = n->c;
      goto enterNode;
    }

 // leaveNode:
  if( --level >= 0 )
    {
      n = n->pred();
      n->c = n->c->right_sibling;
      goto processChild;
    }
#endif
}

void	SNnetwork::printNetwork() const
{
#ifdef PRINT_NETWORK
  log << "***********************\n";
  register SNarc *a;
  for( a = arcList; a != NULL; a = a->next )
    log << nodeName(a->from) << '\t' << a->cost << '\t' << nodeName(a->to) << '\n';
#endif
}

#endif // DIAGNOSTICS

struct MemObj
{
  MemObj	*next;
};

SNnetwork::SNnetwork()
{
  arcMemList = NULL;
  new memobj( arcMemList, sizeof(SNarc) );
  nodeMemList = NULL;
  new memobj( nodeMemList, sizeof(SNnode) );
  arcFreeList = NULL;
  nodeFreeList = NULL;

  arcList = NULL;
  numArcs = 0;
  numNodes = 0;

  nodeList = NULL;
  nodeTList = NULL;

  root = source = sink = NULL;
  cost = 0;

  K = J = 0;

  candidates = NULL;
  numCandidates = 0;
  candidatesMax = 0;

  nextPosition = NULL;
  lookahead = 0;

  numPivots = 0;

  arcUpper = NULL;
  nodeFlow = NULL;
}

void SNnetwork::saveSolution()
{
	register SNarc	*arc;

   for( arc = arcList; arc != NULL; arc = arc->next )
   	if( arc->ident == SNarc::U )
      	numArcUpper++;
   if( numArcUpper > 0 )
   {
   	arcUpper = new SNarcUpper[numArcUpper];
	   numArcUpper = 0;
	   for( arc = arcList; arc != NULL; arc = arc->next )
   		if( arc->ident == SNarc::U )
      	{
      		arcUpper[numArcUpper].arc = arc;
	         arcUpper[numArcUpper].upper = arc->upper;
   	      numArcUpper++;
      	}
   }

   register SNnode	*node;
   for( node = nodeList; node != NULL; node = node->next )
   	if( node->flow > 0 )
	   	numNodeFlow++;
   if( numNodeFlow > 0 )
   {
  		nodeFlow = new SNnodeFlow[numNodeFlow];
   	numNodeFlow = 0;
   	for( node = nodeList; node != NULL; node = node->next )
      	if( node->flow > 0 )
	   	{
   			nodeFlow[numNodeFlow].node = node;
     		 	nodeFlow[numNodeFlow].flow = node->flow;
      		numNodeFlow++;
   		}      
   }
}

void SNnetwork::restoreSolution()
{
	register int i;
   for( i = 0; i < numArcUpper; i++ )
   	arcUpper[i].arc->upper = arcUpper[i].upper;

   for( i = 0; i < numNodeFlow; i++ )
   	nodeFlow[i].node->flow = nodeFlow[i].flow;

   delete [] nodeFlow;
   nodeFlow = NULL;
   numNodeFlow = 0;

	delete [] arcUpper;
	arcUpper = NULL;
   numArcUpper = 0;
}

SNnetwork::~SNnetwork()
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
  delete [] candidates;
}

//-----------------------------------------------------------------------------

SNbasis::SNbasis( SNnetwork *aNetwork )
{
  network = aNetwork;
  arcInfo = NULL;
  nodeInfo = NULL;
  if( network )
    capture( network );
}

SNbasis::~SNbasis()
{
  delete [] arcInfo;
  delete [] nodeInfo;
}

void	SNbasis::capture( SNnetwork *aNetwork )
{
  delete [] arcInfo;
  delete [] nodeInfo;
  arcInfo = NULL;
  nodeInfo = NULL;

  network = aNetwork;

  if( network == NULL || network->numArcs == 0 )
    return;

  register SNarc *a;

  register ArcInfo *ai = arcInfo = new ArcInfo [network->numArcs];
  for( a = network->arcList; a != NULL; a = a->next, ai++ )
    {
      ai->arc = a;
      ai->ident = a->ident;
    }

  register SNnode *n;
  register NodeInfo *ni = nodeInfo = new NodeInfo [network->numNodes];
  for( n = network->nodeList; n != NULL; n = n->next, ni++ )
    {
      ni->node = n;
      ni->basic_arc = n->basic_arc;
      ni->flow = n->flow;
      ni->level = n->level;
      ni->potential = n->potential;
    }
}

void SNbasis::restore()
{
  register SNnode *n;

  // Reset the basis.
  for( n = network->nodeList; n != NULL; n = n->next )
    n->child = n->right_sibling = n->left_sibling = NULL;

  // Restore the node basis information.
  register NodeInfo *ni;
  NodeInfo *niEnd = nodeInfo + network->numNodes;
  for( ni = nodeInfo; ni != niEnd; ni++ )
    {
      n = ni->node;
      n->setBasicArc( ni->basic_arc );
      n->flow = ni->flow;
      n->level = ni->level;
      n->potential = ni->potential;
    }

  // Restore the arc basis information.
  register SNarc   *a;
  register ArcInfo *ai;
  ArcInfo  *aiEnd = arcInfo + network->numArcs;
  for( ai = arcInfo; ai != aiEnd; ai++ )
    {
      a = ai->arc;
      a->ident = ai->ident;
    }
}

