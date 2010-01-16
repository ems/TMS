//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "network.h"
#include <limits.h>
#include <stdio.h>

#ifdef WINDOWS_PRINT
extern "C" {
#include "TMSHeader.h"
#include "database.h"
#include "prototyp.h"
}
#endif

// *****************************************************************

SParc::SParc( SPnode *aFrom, SPnode *aTo, CostType aCost, CostType /* aMax */,
CostType aOrigCost )
: to(aTo), from(aFrom)
{
    Assert( aCost >= 0L );
    Assert( aFrom );
    Assert( aTo );

    cost = aCost;
    origCost = aOrigCost;
//    max = aMax;
    flow = 0;

    AddArcToOutList( aFrom, this );
    AddArcToInList( aTo, this );
}

/************************************************************/

void SPnode::removeFromNetwork()
{
    SParc *arc, *arcNext;

    for( arc = inList; arc != NULL; arc = arcNext )
    {
        arcNext = arc->inNext;

        SPnode  *from = (SPnode *)arc->from;
        SParc   *fromArcPrev = NULL;
        for( SParc *fromArc = from->outList;
             fromArc != NULL;
             fromArcPrev = fromArc, fromArc = fromArc->outNext )
        {
            if( fromArc->to == this )
            {
               if( fromArcPrev == NULL )
                   from->outList = fromArc->outNext;
               else
                   fromArcPrev->outNext = fromArc->outNext;
               break;
            }
        }
        delete arc;
    }
    inList = NULL;

    for( arc = outList; arc != NULL; arc = arcNext )
    {
        arcNext = arc->outNext;

        SPnode  *to = (SPnode *)arc->to;
        SParc   *toArcPrev = NULL;
        for( SParc *toArc = to->inList;
             toArc != NULL;
             toArcPrev = toArc, toArc = toArc->inNext )
        {
            if( toArc->from == this )
            {
               if( toArcPrev == NULL )
                   to->inList = toArc->inNext;
               else
                   toArcPrev->inNext = toArc->inNext;
               break;
            }
        }
        delete arc;
    }
    outList = NULL;
}

int SPnode::atEnd() const
{
    for( SParc *arc = outList; arc != NULL; arc = arc->outNext )
        if( arc->to->isWhite() || arc->to->isGray() )
            return 0;
    return 1;
}

int SPnode::atStart() const
{
    for( SParc *arc = inList; arc != NULL; arc = arc->inNext )
        if( arc->from->isWhite() || arc->to->isGray() )
            return 0;
    return 1;
}

/************************************************************/

SPnetwork::SPnetwork() : nodeSkipList( &nodeComparison, SL_UNIQUE )
{
    sink = source = NULL;
    for( long k = 0; k <= K+1; k++ )
	rgBucket[k].contents.reset();
}

SPnetwork::~SPnetwork()
{
    slNodeIter itr(&nodeSkipList);
    SPnode     *node;
    while( itr.first(&node) == SL_OK )
    {
        nodeSkipList.remove(node);
        delete node;
    }
}

void SPnetwork::check()
{
    slNodeIter itr(&nodeSkipList);
    SPnode     *node;
    slRetStatus ret;
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
    {
        SParc *arc;
        for( arc = node->outList; arc != NULL; arc = arc->outNext )
        {
            Assert( arc->from == node );
            Assert( arc->flow >= 0 );
        }
        for( arc = node->inList; arc != NULL; arc = arc->inNext )
        {
            Assert( arc->to == node );
            Assert( arc->flow >= 0 );
        }
    }
}

void SPnetwork::setNodesWhite()
{
    slNodeIter itr(&nodeSkipList);
    SPnode     *node;
    slRetStatus ret;
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        node->setWhite();
}

void
SPnetwork::dijkstraReset()
{
    SPnode        *node, *nodeNext;
    NodeDList	nodeList;

    // Put all the node lists back on the main list after resetting
    // the nodes.

    nodeList = rgBucket[K+1].contents;
    for( node = nodeDoneList.head; node != NULL; node = nodeNext )
    {
	nodeNext = node->next;

        node->reset();
	nodeDoneList.remove(node);
	nodeList.insert(node);
    }

    for( long k = 0; k <= K; k++ )
	for( node = rgBucket[k].contents.head; node != NULL; node = nodeNext )
	{
	    nodeNext = node->next;
	    
            node->reset();
	    rgBucket[k].contents.remove(node);
	    nodeList.insert(node);
	}

    rgBucket[K+1].contents = nodeList;
}

void
SPnetwork::dijkstraInitialize( SPnode *startNode )
{
    rgBucket[0].lower = 0;
    rgBucket[0].upper = 0;
    rgBucket[0].contents.reset();
    for( long k = 0; k <= K; k++ )
    {
	rgBucket[k].lower = (1L<<(k-1L));
	rgBucket[k].upper = (1L<<k)-1L;
	rgBucket[k].contents.reset();
    }

    // Assume that all the nodes have been places on the last bucket list.
    rgBucket[K+1].lower = (1L<<((K+1L)-1L));
    rgBucket[K+1].upper = (1L<<(K+1L))-1L;

    // Put the startNode node in the 0 bucket list.
    rgBucket[K+1].contents.remove(startNode);

    startNode->d = 0;
    startNode->assign = 0;
    startNode->arcToPred = NULL;
    rgBucket[0].contents.insert(startNode);

    // Assume all the rest of the nodes remain on the K+1 bucket list.
}

#define Reinsert( j ) \
    q = j->assign; \
    rgBucket[q].contents.remove(j); \
    for( ; q >= 0; q-- ) \
	if( rgBucket[q].lower <= j->d && j->d <= rgBucket[q].upper ) \
	    break; \
    Assert( q >= 0 ); \
    rgBucket[q].contents.insert(j); \
    j->assign = (int)q

SPnode	*
SPnetwork::findMin()
{
    long	k, p, q;
    DualType	dMin, up, u;
    SPnode	*node, *nodeNext;

    for( k = 0; ; k++ )
	if( k > K+1 )
	    return NULL;
	else if( !rgBucket[k].contents.isEmpty() )
	    break;

    if( !(k == 0 || k == 1) )
    {
	p = k;
	dMin = INFINITY;
	ForAllNodesDo( rgBucket[p].contents, node )
	    if( node->d < dMin )
		dMin = node->d;

        if( dMin == INFINITY )
            return NULL;

	rgBucket[0].lower = dMin;
	rgBucket[0].upper = dMin;
	up = rgBucket[p].upper;
	if( p == K )
	    for( k = 1; k <= K; k++ )
	    {
		rgBucket[k].lower = (1L<<(k-1L)) + dMin;
		rgBucket[k].upper = (1L<<k) + dMin + 1L;
	    }
	else
	    for( k = 1; k <= p; k++ )
	    {
		rgBucket[k].lower = (1L<<(k-1L)) + dMin;
		u = (1L<<k) + dMin - 1L;
		rgBucket[k].upper = Min(u, up);
	    }
	for( node = rgBucket[p].contents.head; node!=NULL; node = nodeNext )
	{
	    nodeNext = node->next;
	    Reinsert( node );
	}
    }
    if( !rgBucket[0].contents.isEmpty() )
	return rgBucket[0].contents.head;
    else
	return rgBucket[1].contents.head;
}

//#define SLOW_SOLVE
#ifndef SLOW_SOLVE
void
SPnetwork::shortestPathTree()
{
    SPnode	*i, *j;
    SParc	*arc;
    long	q;
    SPnode	*startNode = source;

    if( startNode == NULL )
	return;

    dijkstraInitialize( startNode );
    for( ;; )
    {
	if( (i = findMin()) == NULL )
	    break;

	rgBucket[i->assign].contents.remove(i);
	i->assign = -1;

        // Save the node on the done node list.
	nodeDoneList.insert(i);

        int in;
        ForAllArcsOfNode( i, arc, in )
	{
	    if( Residual(i, arc) == 0 )
		continue;

	    j = ToNode(i, arc);

	    Assert( Cost(i,arc) >= 0L );

            Assert( i->d >= 0L );
	    const long dNew = i->d + Cost(i, arc);
            Assert( dNew >= 0L );
	    if( j->d > dNew )
	    {
		j->d = dNew;
		j->arcToPred = arc;
		Reinsert( j );
	    }
        }
    }
}
#else

class DualCmp : public Comparison<SPnode>
{
public:
    DualCmp() {}
    ~DualCmp() {}
    int compare(const SPnode &n1, const SPnode &n2)
    {
        return n1.d < n2.d ? -1 : n1.d > n2.d ? 1 : 0;
    }
};

void  SPnetwork::shortestPathTree()
{
    DualCmp     dualCmp;
    slNodeList  dualList(&dualCmp);
    slNodeIter  itr(&nodeSkipList);
    slNodeIter  ditr(&dualList);
    slRetStatus ret;
    SPnode      *i;

    // First, initialize all the nodes in the network.
    for( ret = itr.first(&i); ret == SL_OK; ret = itr.next(&i) )
    {
        i->d = INFINITY;
        i->arcToPred = NULL;
    } 

    // Put the source node into the reachable list.
    source->d = 0L;
    dualList.insert(source);

    while( ditr.first(&i) == SL_OK )
    {
	// Remove the next nearest node.
        dualList.remove(i);

	// For all nodes reachable from this node,
	// update their shortest paths, and update
	// their positions in the reachable list if
	// necessary.
	int	in;
        SParc   *arc;
	ForAllArcsOfNode( i, arc, in )
	{
	    if( Residual(i, arc) == 0 )
		continue;

	    SPnode *j = ToNode(i, arc);

	    Assert( Cost(i,arc) >= 0 );

            Assert( i->d >= 0L );
	    const long dNew = i->d + Cost(i, arc);
            Assert( dNew >= 0L );
	    if( j->d > dNew )
	    {
                dualList.remove(j);
		j->d = dNew;
		j->arcToPred = arc;
                dualList.insert(j);
	    }
	}
    }
    dualList.clear();
}
#endif

void	SPnetwork::netSolve()
{
//#define DEBUG
#ifdef DEBUG
    long    numPivots = 0L;
    long    trips = 0L;
#endif

    Assert( source );
    Assert( sink );
                            
    // Remove all unreachable nodes from the network.
    SPnode    *node, *nodeNext;
    for( node = rgBucket[K+1].contents.head; node != NULL; node = nodeNext )
    {
        nodeNext = node->next;
        if( node == source || node == sink )
            continue;
        if( node->inList == NULL || node->outList == NULL )
        {
            node->removeFromNetwork();
            rgBucket[K+1].contents.remove(node);
            nodeSkipList.remove(node);
            delete node;
        }
#ifdef DEBUG
        trips++;
#endif
    }
#ifdef DEBUG
    trips /= 2;
#endif

    if( source->outList == NULL )
        return;         // Nothing to do - network is NULL.

    check();

    // Augment on all shortest paths until the sink node in unreachable.
    for( ;; )
    {
	shortestPathTree();

	// No augmenting path - time to quit.
	if( sink->arcToPred == NULL )
	    break;

	// Augment along the shortest path.
	// First, determine the maximum augmenting amount.
	CostType	aug = LONG_MAX;
	for( SPnode *node = sink; node->arcToPred != NULL; node = GetPred(node) )
	    if( !IsForwardArc(node, node->arcToPred) )
		aug = Min(aug, node->arcToPred->getMax() - node->arcToPred->flow);
	    else
		aug = Min(aug, node->arcToPred->flow);
        Assert( node == source );
	Assert( aug > 0 );

	// Then, augment along the path.
	for( node = sink; node->arcToPred != NULL; node = GetPred(node) )
	    if( !IsForwardArc(node, node->arcToPred) )
		node->arcToPred->flow += aug;
	    else
		node->arcToPred->flow -= aug;
        Assert( node == source );

	// Rescale the costs so that we get rid of negative costs.
	// This allows us to use Dijkstra's alg. to find subsequent
	// least-cost augmenting paths.
	slNodeIter	itr(&nodeSkipList);
	slRetStatus	ret;
        SParc           *arc;
	for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
            ForAllInArcsOfNode(node, arc)
                arc->cost = arc->cost + arc->from->d - arc->to->d;

#ifdef DEBUG
  printf( "netSolve: pivots=%ld maxPivots=%ld\n",
          ++numPivots, trips );
#endif
#ifndef SLOW_SOLVE
        // Setup for the next shortest path solve.
	SPnetwork::dijkstraReset();
#endif
    }
}

void    SPnetwork::forAllNonZeroArcs( arcFunc a )
{
    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
	ForAllOutArcsOfNode( node, arc )
            if( arc->flow > 0 )
                (*a)(arc->from->id, arc->to->id, arc->cost, arc->getMax(), arc->flow );
}

void	SPnetwork::setSourceSink( const IdType idSource, const IdType idSink )
{
    source = findNodeFromID(idSource);
    Assert( source );

    sink = findNodeFromID(idSink);
    Assert( sink );
}

#ifdef STREAM_PRINT
void	SPnetwork::netPrint( ostream &ostr )
{
    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    CostType	totalCost = 0L;
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
	ForAllOutArcsOfNode( node, arc )
	{
	    totalCost += (long)arc->origCost * (long)arc->flow;
	    ostr << arc->from->id << ' ' << arc->to->id << ' ' << arc->flow << '\n';
	}
    ostr << "Total Cost  : " << totalCost << '\n';

    CostType	totalFlow = 0L;
    ForAllInArcsOfNode( sink, arc )
	totalFlow += arc->flow;

    ostr << "Maximum Flow: " << totalFlow << '\n';
}

void	SPnetwork::printLP( ostream &ostr )
{
    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    ostr << "minimize\n";
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        ForAllOutArcsOfNode( node, arc )
            if( arc->cost > 0 )
                ostr << "+ " << arc->cost << " x" << arc->from->id << 't' << arc->to->id << '\n';
    ostr << "- 1000000 x" << sink->id << 't' << source->id << '\n';

    ostr << "subject to\n";
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
    {
	ForAllInArcsOfNode( node, arc )
	    ostr << "+ x" << arc->from->id << 't' << arc->to->id << '\n';
        if( node == source )
            ostr << "+ x" << sink->id << 't' << source->id << '\n';
	ForAllOutArcsOfNode( node, arc )
	    ostr << "- x" << arc->from->id << 't' << arc->to->id << '\n';
        if( node == sink )
            ostr << "- x" << sink->id << 't' << source->id << '\n';
        ostr << "= 0\n";
    }

    ostr << "bounds\n";
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        ForAllOutArcsOfNode( node, arc )
            ostr << arc->from->id << 't' << arc->to->id << " <= 1\n";
}

ostream &
operator << ( ostream &ostr, SPnetwork &net )
{
    slNodeIter itr(&net.nodeSkipList);
    slRetStatus ret;
    SPnode	*node;
    SParc	*arc;

    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
	ForAllOutArcsOfNode(node, arc)
	    ostr << node->id << ' ' << arc->to->id << ' ' << arc->cost << ' ' << arc->getMax() << '\n';

    ostr << "sourcesink " << net.source->id << ' ' << net.sink->id << '\n';

    return ostr;
}
#endif

#ifdef WINDOWS_PRINT
void SPnetwork::windowsPrintLP()
{
    OFSTRUCT ofStruct;
    HFILE hFile;
    char   szTmp[1024];

    hFile = OpenFile("c:\\windows\\network.lp", &ofStruct, OF_CREATE | OF_READWRITE);
    if(hFile == HFILE_ERROR)
        return;

    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    sprintf( szTmp, "min\n" );
    _lwrite( hFile, szTmp, strlen(szTmp) );
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        ForAllOutArcsOfNode( node, arc )
            if( arc->cost > 0 )
            {
                sprintf( szTmp, "+ %ld f%ldt%ld\n",
                          arc->cost,
                          arc->from->id,
                          arc->to->id );
                _lwrite( hFile, szTmp, strlen(szTmp) );
            }
    sprintf( szTmp, "- %ld f%ldt%ld\n",
              1000000L,
              sink->id,
              source->id );
    _lwrite( hFile, szTmp, strlen(szTmp) );

    sprintf( szTmp, "st\n" );
    _lwrite( hFile, szTmp, strlen(szTmp) );
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
    {
	ForAllInArcsOfNode( node, arc )
        {
            sprintf( szTmp, "+ f%ldt%ld\n",
                      arc->from->id, arc->to->id );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }
        if( node == source )
        {
            sprintf( szTmp, "+ f%ldt%ld\n",
                      sink->id, source->id );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }
	ForAllOutArcsOfNode( node, arc )
        {
            sprintf( szTmp, "- f%ldt%ld\n",
                      arc->from->id, arc->to->id );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }
        if( node == sink )
        {
            sprintf( szTmp, "- f%ldt%ld\n",
                      sink->id, source->id );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }
        sprintf( szTmp, "= 0\n" );
        _lwrite( hFile, szTmp, strlen(szTmp) );
    }

    sprintf( szTmp, "bounds\n" );
    _lwrite( hFile, szTmp, strlen(szTmp) );
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        ForAllOutArcsOfNode( node, arc )
        {
            sprintf( szTmp, "0 <= f%ldt%ld <= 1\n",
                      arc->from->id, arc->to->id );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }

    _lclose(hFile);
}

void    SPnetwork::windowsPrintNet()
{
    OFSTRUCT ofStruct;
    HFILE hFile;
    char   szTmp[1024];

    hFile = OpenFile("c:\\windows\\network.txt", &ofStruct, OF_CREATE | OF_READWRITE);
    if(hFile == HFILE_ERROR)
        return;

    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        ForAllOutArcsOfNode( node, arc )
        {
            sprintf( szTmp, "%ld %ld %ld %ld\n",
                      arc->from->id,
                      arc->to->id,
                      arc->cost,
                      arc->getMax() );
            _lwrite( hFile, szTmp, strlen(szTmp) );
        }
    sprintf( szTmp, "sourcesink %ld %ld",
              source->id,
              sink->id );
    _lwrite( hFile, szTmp, strlen(szTmp) );

    _lclose(hFile);
}
#endif
