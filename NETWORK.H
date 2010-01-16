//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef HD_NETWORK
#define HD_NETWORK

// #define STREAM_PRINT
#ifdef STREAM_PRINT
#include <iostream.h>
#endif

/************************************************************/
// Define the base datatypes of network costs and node identifiers.
#define	IdType		unsigned long  // data type of node identifiers.
#define	CostType	long  // data type of arc costs.

#define DualType        CostType /* data type of dual costs */

#define K               (20L)/* = ceiling(log2 C) where C is the largest distance
			      * between nodes in the network.
			      * a value of 20 means this system will handle distances
			      * up to 2^20 = 1,048,576.
			      * Smaller values of K will result in slightly
			      * better performance. */
/************************************************************/
#define Boolean         int

#define INFINITY (1L << (K+1))

#ifndef True
#define True            1
#endif
#ifndef False
#define False           0
#endif

#ifndef Min
#define Min(a,b)        (((a)<(b))?(a):(b))
#endif
#ifndef Max
#define Max(a,b)        (((a)>(b))?(a):(b))
#endif

#include "skiplist.h"

static int AssertFunc( const void *c )
{
    int assertA = 0;
    return !c ? (10 / assertA) : 1;
}
#ifndef Assert
#define Assert(c)       AssertFunc( (void *)(c) )
#endif

//--------------------------------------------------------------------------------

class SParc;
class SPnode;

#define AddArcToOutList( n, x )  (x)->outNext = (n)->outList; (n)->outList = (x)
#define AddArcToInList( n, x )  (x)->inNext = (n)->inList; (n)->inList = (x)

#define ForAllOutArcsOfNode( n, a ) \
for( (a) = (n)->outList; (a) != NULL; (a) = (a)->outNext )

#define ForAllInArcsOfNode( n, a ) \
for( (a) = (n)->inList; (a) != NULL; (a) = (a)->inNext )

#define ForAllArcsOfNode( n, a, in ) \
for( (a) = ((n)->inList ? ((in) = True, (n)->inList) : ((in) = False, (n)->outList)); \
     (a) != NULL; \
     (a) = ((in) \
     ? ((a)->inNext  == NULL ? ((in) = False, (n)->outList) : (a)->inNext) \
      : ((a)->outNext == NULL ? NULL : (a)->outNext)) )

#define IsForwardArc(n, a) ((n) == (a)->from)

#define Residual(n, a) \
(IsForwardArc(n,a) ? ((a)->getMax() - (a)->flow) : (a)->flow)

#define Cost(n, a) (IsForwardArc(n,a) ? (a)->cost : -(a)->cost)

#define ToNode(n, a) \
(SPnode *)((IsForwardArc(n,a) ? (a)->to : (a)->from))

#define GetPred(n) \
(SPnode *)(((n)->arcToPred == NULL ? NULL \
: (IsForwardArc(n, (n)->arcToPred) ? (n)->arcToPred->to : (n)->arcToPred->from)))


//--------------------------------------------------------------------------------

#define ForAllNodesDo( h, n ) \
	for( (n) = (h).head; (n) != NULL; (n) = ((n)->next) )

//--------------------------------------------------------------------------------

class SPnetwork;

class SParc
{
friend class SPnetwork;
friend class SPnode;
#ifdef STREAM_PRINT
friend ostream &operator << ( ostream &, SPnetwork & );
#endif

 public:
    SParc( SPnode *from, SPnode *to, CostType aCost = 0, CostType aMax = 1L,
           CostType aOrigCost = 0L );
    ~SParc() {}
    IdType      getFromID() const;
    IdType      getToID() const;

 private:
    CostType    cost;           /* cost of arc */
// The following fields, although useful, they take up unnecessary memory.
    CostType    origCost;       /* original cost - unaffected by scaling */

//    CostType    flow;           /* flow along arc */
//    CostType    max;            /* max flow along arc */

    int           flow;
    CostType    getMax() const { return 1L; }

    SPnode      *to;            /* "To" node of arc */
    SPnode      *from;          /* "From" node of arc */
    SParc       *inNext;        /* next arc in IN arc list */
    SParc       *outNext;       /* next arc in OUT arc list */
};

class SPnode
{
    friend class SPnetwork;
    friend class NodeComparison;
    friend class DualCmp;
    friend class SParc;
    friend class NodeDList;
#ifdef STREAM_PRINT
    friend ostream &operator << ( ostream &, SPnetwork & );
#endif

public:
    SPnode( IdType aId ) : id(aId)
    {
	outList = NULL;
        inList = NULL;
	prev = NULL;
        next = NULL;
	reset();
    }
    ~SPnode() { removeFromNetwork(); }
    IdType getID() const { return id; }

    void setWhite() { assign = 0; }
    void setGray() { assign = 1; }
    void setBlack() { assign = 2; }
    int isWhite() const { return assign == 0; }
    int isGray() const { return assign == 1; }
    int isBlack() const { return assign == 2; }
    int atEnd() const;
    int atStart() const;

private:
    const IdType id;            /* id of this node */
    int         assign;         /* index of current bucket the node is in */
    DualType    d;              /* value of dual variable */

    SParc       *outList;       /* an out list of arcs leaving the node */
    SParc       *inList;        /* an in list of arcs leaving the node */
    SParc       *arcToPred;     /* arc that points to the predecessor */

    SPnode      *prev, *next;   /* pointers for doubly linked list */

    SPnode      *getPred() const
    {
	return arcToPred == NULL ? NULL
	    : arcToPred->from == this ? arcToPred->to : arcToPred->from;
    }

    void        reset()
    {
	assign = (int)(K+1);
	d = INFINITY;
	arcToPred = NULL;
    }

    void removeFromNetwork();
};

inline IdType SParc::getFromID() const { return from->id; }
inline IdType SParc::getToID() const { return to->id; }

//-----------------------------------------------------------------

class NodeDList
{
friend class SPnetwork;
friend class Bucket;
private:
    NodeDList() { head = NULL; }

    void insert( SPnode *n )
    {
	Assert( n );

	n->next = head;
	if( head != NULL )
	    head->prev = n;

	n->prev = NULL;
	head = n;
    }

    void remove( SPnode *n )
    {
	Assert( n );

	if( n->prev != NULL )
	    n->prev->next = n->next;
	else
	    head = n->next;

	if( n->next != NULL )
	    n->next->prev = n->prev;
    }

    void reset() { head = NULL; }

    NodeDList &operator = (NodeDList &list )
    {
	head = list.head;
	list.reset();

	return *this;
    }

    Boolean isEmpty() { return head == NULL; }

    SPnode        *head;
};

/************************************************************/

// Declare a skiplist 
class NodeComparison : public Comparison<SPnode>
{
public:
    NodeComparison() {};
    int compare(const SPnode &n1, const SPnode &n2)
    {
	return n1.id < n2.id
	    ? -1
	    : n1.id > n2.id ? 1 : 0;
    }
};


typedef skipList<SPnode> slNodeList;
typedef skipListIter<SPnode> slNodeIter;

class Bucket
{
friend class SPnetwork;
#ifdef STREAM_PRINT
friend ostream &operator << ( ostream &, SPnetwork & );
#endif
private:
    CostType    lower, upper;   // range of bucket
    NodeDList   contents;       // doubly-linked list of nodes in this bucket
    Bucket() { lower = upper = 0L; }
};

typedef void (*arcFunc)( IdType from, IdType to, CostType cost, CostType max, CostType flow );

class Gene;
class GarageTravelTime;
class SPnetwork
{
#ifdef STREAM_PRINT
friend ostream &operator << ( ostream &, SPnetwork & );
#endif
public:
    SPnetwork();
    ~SPnetwork();

    void check();

    // called to add an arc to the network.
    void        addArc(const IdType idFrom,
		       const IdType idTo,
		       const CostType aCost = 1L,
		       const CostType aMax = 1L,
                       const CostType aOrigCost = 1L )
    {
	SPnode  *from = getNode(idFrom);
	SPnode  *to   = getNode(idTo);
	if( from != to )
	    new SParc( from, to, aCost, aMax, aOrigCost );
    }

    // called to set the source and sink nodes in the network.
    void        setSourceSink( const IdType idSource, const IdType idSink );

    // solve the min cost network.
    void        netSolve();

    // use a heuristic to solve the runblock problem simultaneously
    double      blockCut(skipList<SPnode> &blockCutList,
                         GarageTravelTime &gtt,
                         Gene &g );
    // this routine attempts to reorder the wait arcs to get a better
    // solution
    void        bestBlockCut();

    skipList<SPnode> *getForwardPiece( SPnode *n );
    skipList<SPnode> *getBackwardPiece( SPnode *n );
    void        setNodesWhite();

    // print the flow value on all arcs.
#ifdef STREAM_PRINT
    void        netPrint( ostream &ostr );
    void        printLP( ostream &ostr );
#endif

//#define WINDOWS_PRINT
#ifdef WINDOWS_PRINT
    void        windowsPrintNet();
    void        windowsPrintLP();
#endif

    void        forAllNonZeroArcs( arcFunc a );

private:
    void        shortestPathTree();
    void        dijkstraReset();

    void        dijkstraInitialize( SPnode *startNode );
    SPnode      *getNode( const IdType id )
    {
	SPnode  *node = findNodeFromID( id );
	if( node == NULL )
	{
	    node = new SPnode(id);
	    rgBucket[K+1].contents.insert(node);
	    nodeSkipList.insert(node);
	}
	return node;
    }

    SPnode *findNodeFromID( IdType id )
    {
	SPnode  *nodeRet, nodeSearch(id);

	return nodeSkipList.find(&nodeSearch, &nodeRet) == SL_OK 
	    ? nodeRet
	    : NULL;
    }

    void Reinsert( SPnode *j );

    NodeComparison      nodeComparison;
    slNodeList  nodeSkipList;

    Bucket      rgBucket[K+2];  // array of node buckets
    NodeDList   nodeDoneList;

    SPnode              *findMin();

    SPnode              *source, *sink;
};

#endif // HD_NETWORK
