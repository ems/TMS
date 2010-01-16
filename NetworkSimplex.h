
#ifndef NetworkSimplex_H
#define NetworkSimplex_H

#ifdef _MSC_VER		// Protect against Microsoft min/max macros.
#define NOMINMAX
#endif // _MSC_VER

#include <vector>
#include <ostream>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define Use64Bit

//-----------------------------------------------------------------

#define slist_push_back( head, tail, element, nextField ) \
(element->nextField = NULL, (tail ? tail->nextField = element : head = element), tail = element)

#define slist_remove_front( head, tail, nextField ) \
((head = head->nextField) ? head : tail = NULL)

//-----------------------------------------------------------------

#define list_push_front( head, element, prevField, nextField ) \
(element->prevField = NULL, element->nextField = head, (head ? head->prevField = element : NULL), head = element)

#define list_remove( head, element, prevField, nextField ) \
( (element->nextField ? element->nextField->prevField = element->prevField : NULL), \
  (element->prevField ? element->prevField->nextField = element->nextField : head = element->nextField) )

//-----------------------------------------------------------------

class NetworkSimplex
{
protected:
	class FastMemPool
	{
	protected:
		struct	Element { Element *next; };

	public:
		FastMemPool( const size_t aObjSize ) :
			objSize(aObjSize >= sizeof(Element *) ? aObjSize : sizeof(Element *)),
				freeList(NULL), iPool(0), numObjsInPool(1) {}

		void	*malloc()
		{
			register void *r;
			if( freeList )
			{
				r = (void *)freeList;
				freeList = freeList->next;
			}
			else
			{
				if( numObjsInPool == (1<<iPool) )
				{
					++iPool;
					pool[iPool] = new char [(1<<iPool) * objSize];
					numObjsInPool = 0;
				}
				r = (void *)(pool[iPool] + (numObjsInPool++ * objSize));
			}
			return r;

		}

		void	free( register void *obj )
		{
			if( obj )
			{
				((Element *)obj)->next = freeList;
				freeList = ((Element *)obj);
			}
		}

		void	purge_memory()
		{
			while( iPool )
			{
				delete [] pool[iPool];
				--iPool;
			}
			numObjsInPool = 1;
			freeList = NULL;
		}

	protected:
		Element	*freeList;			// Object free list.
		const size_t	objSize;	// Size of an object in the pool.
		size_t	iPool;				// Current pool index.  The size of each pool is (1<<iPool).
		size_t	numObjsInPool;		// Number of objects allocated in current pool.
		char	*pool[32];			// Pools increase in powers of two.  pool[0] is not allocated.
	};

public:

#ifdef Use64Bit
	typedef	__int64		cost_t;
	typedef	__int64		flow_t;
#else
	typedef	int		cost_t;
	typedef	int		flow_t;
#endif

	class	Arc;	friend class Arc;
	class	Node;	friend class Node;

	class Arc
	{
		friend class Node;
		friend class NetworkSimplex;
	public:
		Arc( Node *from, Node *to, cost_t aCost, flow_t aMin, flow_t aMax );

		flow_t	getFlow() const { return flow + min; }
		cost_t	getContribution() const { return cost * getFlow(); }
		cost_t	getCost() const { return cost; }

		flow_t	getMin() const { return min; }
		flow_t	getMax() const { return max + min; }
		void	setMax( const flow_t aMax ) { max = aMax - min; }
		void	setMin( const flow_t aMax );

		Node	*getFromNode() const	{ return from; }
		Node	*getToNode() const		{ return to; }

		Node	*getNext( const Node *n ) const { return from == n ? to : from; }
		bool	isFrom( const Node *n ) const { return n == from; }
		bool	isTo( const Node *n ) const { return n == to; }

	protected:
		~Arc();
		Node	*getFrom() const { return atMin() ? from : to; }
		Node	*getTo() const { return atMin() ? to : from; }

		bool	atMax() const { return flow == max; }
		bool	atMin() const { return flow == 0; }

		cost_t	getPotential() const;
		cost_t	getTreeCost( const Node *parent ) const;
		flow_t	getResCapacityFrom( const Node *aFrom ) const	{ return from == aFrom	? max - flow : flow; }
		flow_t	getResCapacityTo( const Node *aTo ) const		{ return to == aTo		? max - flow : flow; }


		void	incFlowFrom( const Node *aFrom, const flow_t capacity )
		{ if( from == aFrom )	flow += capacity; else flow -= capacity; }
		void	incFlowTo( const Node *aTo, const flow_t capacity )
		{ if( to == aTo )		flow += capacity; else flow -= capacity; }

		Node	*from, *to;
		cost_t	cost;
		flow_t	flow;
		flow_t	min, max;
		bool	inTree;

		void	from_to_insert(),	from_to_remove();

		Arc		*inPrev,  *inNext;
		Arc		*outPrev, *outNext;
	};
	typedef std::vector<Arc *> ArcVector;

	class Node
	{
		friend class Arc;
		friend class NetworkSimplex;
	public:
		Node( const flow_t aB = 0 ) : b(aB), pi(0), depth(0), out(NULL), in(NULL), treeArc(NULL),
			children(NULL), childCur(NULL), siblingPrev(NULL), siblingNext(NULL) {}

	protected:
		~Node()	{}

		Node	*parent() const { return treeArc->from == this ? treeArc->to : treeArc->from; }

		int		depth;

		Arc		*treeArc;

		Node	*children, *childCur;
		Node	*siblingPrev, *siblingNext;
		Node	*prev, *next;

		void	updateTreeChild( register Node *p )
		{
			depth = p->depth + 1;
			if( treeArc->from == p )
				pi = p->pi - treeArc->cost;
			else
				pi = p->pi + treeArc->cost;
		}

		void	updateTreeChild()	{ updateTreeChild(parent()); }

		void	reparent( Arc *newTreeArc )
		{
			Node	*p = parent();
			list_remove( p->children, this, siblingPrev, siblingNext );
			treeArc = newTreeArc;
			p = parent();
			list_push_front( p->children, this, siblingPrev, siblingNext );
		}

		void	setParent( Arc *newTreeArc )
		{
			newTreeArc->inTree = true;
			treeArc = newTreeArc;
			Node *p = parent();
			list_push_front( p->children, this, siblingPrev, siblingNext );
			updateTreeChild( p );
		}

		cost_t	pi;					// dual value
		flow_t	b;					// +b = source node, -b = sink node.
		Arc	*out, *in;
	};

public:
	NetworkSimplex() : nodeList(NULL), nodePool(sizeof(Node)), arcPool(sizeof(Arc)) { clear(); }
	~NetworkSimplex()	{ clear(); }

	virtual void	clear();

	Arc	*newArc( Node *from, Node *to, cost_t aCost = 0, flow_t aMin = 0, flow_t aMax = 1 );

	void eraseArc( Arc *a );
	void changeArcCost( Arc *a, const cost_t newCost );
	void changeArcMinMax( Arc *a, const flow_t aMin, const flow_t aMax );
	bool wouldCausePivot( const Arc *a ) const;

	Node *newNode( const flow_t b = 0 )
	{
		// b is positive for a source node, negative for a sink node, and zero for a transship node.
		removeBasis();
		Node *n = new (nodePool.malloc()) Node( b );
		++N;
		list_push_front(nodeList, n, prev, next);
		return n;
	}

	void	eraseNode( Node *n )
	{
		removeBasis();
		deleteNode( n );
	}

	void	changeNodeB( Node *n, const flow_t aB )
	{
		if( n->b != aB )
		{
			removeBasis();
			n->b = aB;
		}
	}

	virtual flow_t	getViolation( const Node *n ) const;

	flow_t	getInflow( const Node *n ) const
	{
		register flow_t	f = 0;
		for( register Arc *a = n->in; a; a = a->inNext )
			f += a->getFlow();
		return f;
	}

	flow_t	getOutflow( const Node *n ) const
	{
		register flow_t	f = 0;
		for( register Arc *a = n->out; a; a = a->outNext )
			f += a->getFlow();
		return f;
	}

	size_t	getOutDegree( const Node *n ) const
	{
		register size_t	d = 0;
		for( register Arc *a = n->out; a; a = a->outNext )
			if( a->to != root )
				++d;
		return d;
	}

	size_t	getInDegree( const Node *n ) const
	{
		register size_t	d = 0;
		for( register Arc *a = n->in; a; a = a->inNext )
			if( a->from != root )
				++d;
		return d;
	}

	virtual void	createBasis();
	virtual void	removeBasis()
	{
		if( root )	// Explicitly remove the basis.  Should not be necessary as this is done automatically.
		{
			deleteNode( root );
			root = NULL;
			cost = 0;
		}
	}
	virtual void	printBasis( std::ostream &os );
	virtual void	solve();

	virtual bool	hasBasis() const { return root != NULL; }
	virtual bool	isFeasible() const;

	flow_t	BigFlow() { return bigFlow; }
	cost_t	BigCost() { return bigCost; }

	cost_t	getCost() const { return cost + fixedCost; }
	virtual cost_t	computeCost() const;

	size_t	getNumPivots() const { return numPivots; }
	size_t	getNumDegeneratePivots() const { return numDegeneratePivots; }

protected:

	void deleteArc( Arc *a );
	void deleteNode( Node *n );

	size_t		A;			// number of arcs
	size_t		N;			// number of nodes

	size_t		J, K;		// Refill and maximum size for candidate pivot rule.
	ArcVector	candidates;	// Pivot candidates.

	size_t		numPivots, numDegeneratePivots;

	Node		*lastPivotNode;
	Arc			*lastPivotArc;
	Arc			*findPivotArc();
	void		initCandidateList();

	void		pivot( Arc *a );
	void		updateBasis( Node *p );

	virtual void	validateBasis();

	Node		*root;
	Node		*nodeList;

	cost_t		cost, fixedCost;

	// Computed in createBasis.
	flow_t		bigFlow;
	cost_t		bigCost;

	FastMemPool arcPool, nodePool;
};


inline void	NetworkSimplex::Arc::setMin( const flow_t aMin )
{
	const	flow_t	aMax = getMax();

	from->b -= (aMin - min);
	to->b	+= (aMin - min);

	min = aMin;

	setMax( aMax );
}

inline void	NetworkSimplex::Arc::from_to_insert()
{
	list_push_front( from->out, this, outPrev, outNext );
	list_push_front( to->in,    this, inPrev,  inNext );
}

inline void	NetworkSimplex::Arc::from_to_remove()
{
	list_remove( from->out, this, outPrev, outNext );
	list_remove( to->in,    this, inPrev,  inNext );
}

inline NetworkSimplex::Arc::Arc( Node *aFrom, Node *aTo, cost_t aCost, flow_t aMin, flow_t aMax ) :
	from(aFrom), to(aTo), cost(aCost), min(0), max(0), flow(0), inTree(false)
{
	from_to_insert();
	setMax( aMax );
	setMin( aMin );
}

inline NetworkSimplex::Arc::~Arc()
{
	setMin( 0 );
	from_to_remove();
}

inline NetworkSimplex::cost_t	NetworkSimplex::Arc::getPotential() const
{ return flow == 0 ? cost + to->pi - from->pi : from->pi - cost - to->pi; }

//-------------------------------------------------------------------------------------------------------------

inline NetworkSimplex::Arc	*NetworkSimplex::newArc( Node *from, Node *to, cost_t aCost, flow_t aMin, flow_t aMax )
{
	++A;
	fixedCost += aMin * aCost;
	if( aMin > 0 )
		removeBasis();
	return new (arcPool.malloc()) Arc(from, to, aCost, aMin, aMax);
}

inline void NetworkSimplex::deleteArc( Arc *a )
{
	a->from_to_remove();
	a->~Arc(); arcPool.free( a );
	--A;
}
	
inline void NetworkSimplex::eraseArc( Arc *a )
{
	if( a->inTree || (a->max > 0 && a->atMax()) )
		removeBasis();
	deleteArc( a );
}

inline void	NetworkSimplex::changeArcCost( Arc *a, const cost_t newCost )
{
	fixedCost += (newCost - a->cost) * a->min;
	cost += (newCost - a->cost) * a->flow;
	a->cost = newCost;
}

inline bool NetworkSimplex::wouldCausePivot( const Arc *a ) const
{ return !hasBasis() || a->inTree || a->getPotential() < 0; }

inline void	NetworkSimplex::changeArcMinMax( Arc *a, const flow_t aMin, const flow_t aMax )
{
	if( a->getMin() != aMin )
		fixedCost += (aMin - a->getMin()) * a->cost;

	if( !hasBasis() )
	{
		a->setMax( aMax );
		a->setMin( aMin );
	}
	else if( !a->inTree && a->getMin() == aMin && a->getMax() != aMax && a->flow == 0 )
	{
		// OK to change arc max if it is not in the tree and is at a lower bound.
		a->setMax( aMax );
	}
	else
	{
		removeBasis();
		a->setMax( aMax );
		a->setMin( aMin );
	}
}

#if defined(Use64Bit) && _MSC_VER < 1300
inline std::ostream &operator<<( std::ostream &os, __int64 i )
{
	char	szOut[64];
	register char *p = &szOut[sizeof(szOut)];

	const bool neg = (i < 0 ? (i *= -1i64, true) : false);

	*--p = 0;
	do
	{
		*--p = '0' + (char)(i%10);
		i /= 10;
	} while( i );

	if( neg )
		*--p = '-';

	return os << p;
}
#endif

#endif // NetworkSimplex_H