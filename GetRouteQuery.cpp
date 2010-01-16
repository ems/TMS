
#include "CIS.h"
#include <set>

void	getPathifyConnections( PathifyConnections &pc, CISlocationMapSet &fromTo )
{
	pc.clear();

	register size_t cTotal = 0;
	register CISlocationMapSet::iterator from, fromEnd;
	for( from = fromTo.begin(), fromEnd = fromTo.end(); from != fromEnd; ++from )
		cTotal += from->second.size();

	// Organize the connections reasonably.
	pc.reserve( cTotal );
	for( from = fromTo.begin(), fromEnd = fromTo.end(); from != fromEnd; ++from )
	{
		register std::set<ident_t>::iterator to, toEnd;
		for( to = from->second.begin(), toEnd = from->second.end(); to != toEnd; ++to )
			pc.push_back( std::make_pair(from->first, *to) );
	}
	pathify( pc );
}


class IDGraph
{
public:
	IDGraph() : hasCycle(false) {}

	enum	VisitedState
	{
		White,	// Unseen.
		Grey,	// On the current path (used to detect cycles).
		Black	// Fully fathomed previously.
	};

protected:
	struct Node;
	friend struct Node;
	typedef HashFI<ident_t, Node *> Nodes;

	struct Node
	{
		Node( const ident_t aID ) : id(aID) { reset(); }

		void	reset()
		{
			visitedState = White;
			parent = NULL;
		}

		const ident_t	id;

		Node			*parent;		// Used for backtracking in DFS.
		Nodes::iterator child;			// Current child in DFS.
		VisitedState	visitedState;

		Nodes			out;			// Out "arcs" to connecting nodes.
	};

	Nodes	nodes;
	bool	hasCycle;	// Will be set true after topoSort if the graph has a cycle.

public:
	void	connect( const ident_t fromID, const ident_t toID )
	{
		Nodes::iterator from = nodes.find(fromID);
		if( from == nodes.end() )
			from = nodes.insert( fromID, new Node(fromID) );

		Nodes::iterator to   = nodes.find(toID);
		if( to == nodes.end() )
			to   = nodes.insert( toID,   new Node(toID  ) );

		from.data()->out.insert( toID, to.data() );
	}

	void	newNode( const ident_t id )
	{
		if( !contains(id) )
			nodes.insert( id, new Node(id) );
	}

	bool	contains( const ident_t id ) const { return nodes.contains(id); }

	VisitedState	getVisitedState( const ident_t id )
	{
		Nodes::iterator n = nodes.find(id);
		return n == nodes.end() ? White : n.data()->visitedState;
	}

	void	setVisitedState( const ident_t id, const VisitedState vs )
	{
		Nodes::iterator n = nodes.find(id);
		n.data()->visitedState = vs;
	}

	void	getOutConnections( IDVector &out, const ident_t id )
	{
		out.clear();
		Nodes::iterator n = nodes.find(id);
		if( n != nodes.end() )
		{
			out.reserve( n.data()->out.size() );
			for( register Nodes::iterator o = n.data()->out.begin(), oEnd = n.data()->out.end(); o != oEnd; ++o )
				out.push_back( *o );
		}
	}

	void	topoSort( IDVector &s, ident_t suggestedStart = 0 )
	{
		s.clear();
		s.reserve( nodes.size() );
		hasCycle = false;

		Nodes::iterator n, nEnd;
		for( n = nodes.begin(), nEnd = nodes.end(); n != nEnd; ++n )
			n.data()->reset();

		if( !nodes.contains(suggestedStart) )
			suggestedStart = *nodes.begin();

		bool	firstTime = true;

		for( n = nodes.find(suggestedStart), nEnd = nodes.end(); n != nEnd; ++n )
		{
			if( n.data()->visitedState != White )
				continue;

			// Start the depth-first search.
			register Node *nCur = n.data();
			nCur->parent = NULL;
			nCur->visitedState = Grey;
			nCur->child = nCur->out.begin();

			do
			{
				// Try to extend the path.
				while( nCur->child != nCur->out.end() )
				{
					register Node *nChild = nCur->child.data(); ++nCur->child;
					switch( nChild->visitedState )
					{
					case White:
						break;
					case Grey:
						hasCycle = true;
						// Cycle can be recovered with as follows:
						//
						// IDVector idv;
						// for( Node *nCycle = nCur; nCycle != nChild; nCycle = nCycle->parent )
						//	  idv.push_back( nCycle );
						// idv.push_back( nChild );
						//
						// Fallthrough.
					case Black:
						continue;
					}

					// Found an unvisited child.  Extend the DFS path.
					nChild->parent = nCur;
					nChild->visitedState = Grey;
					nChild->child = nChild->out.begin();
					nCur = nChild;
				}

				// All the children of this node have been visited.
				// Record its position in the topological sort.
				s.push_back( nCur->id );

				// Backtrack up the path.
				nCur->visitedState = Black;
			} while( nCur = nCur->parent );

			if( firstTime )
			{
				n = nodes.begin();
				firstTime = false;
			}
		}

		// In the DFS, the furthest nodes will be added first.
		// Reverse the order so that tha furthest nodes are last.
		std::reverse( s.begin(), s.end() );
	}

	void clear()
	{
		register Nodes::iterator n, nEnd;
		for( n = nodes.begin(), nEnd = nodes.end(); n != nEnd; ++n )
			delete n.data();
		nodes.clear();
	}
	
	bool isCyclic() const { return hasCycle; }

	~IDGraph() { clear(); }
};

typedef HashFI<CISarc *, CISarc *>	CISarcCollection;
typedef	HashObj<IdentHash, TimeVector>	LocIDTimes;
typedef std::vector< std::vector<size_t> >	IndexVectorVector;

CISrouteTimesReply	*CIS::getRouteTimesReply( const ident_t serviceID, const ident_t routeDirectionID )
{
	CISserviceCollection::iterator	sItr = services.find( serviceID );
	if( sItr == services.end() )
		return NULL;
	CISservice *service = sItr.data();
	CISrouteCollection::iterator	rItr = service->routes.find( routeDirectionID );
	if( rItr == service->routes.end() )
		return NULL;
	CISroute *route = rItr.data();

	// Check if the timetable is already cached.  This sames a lot of processing.
	if( route->stripes.empty() )
	{
		// Collect all the leave events for this route at each location.
		LocIDTimes	locIDTimes;

		IDGraph	idGraph;

		ident_t	earliestLocID = 0;
		tod_t	tEarliest = 24*60*60*3;

		CISlocationCollection::iterator loc, locEnd;
		for( loc = service->locations.begin(), locEnd = service->locations.end(); loc != locEnd; ++loc )
		{
			CISlocation *location = loc.data();
			CIScRouteIDEventSet::iterator resItr = location->routeCurb.find(routeDirectionID);
			if( resItr == location->routeCurb.end() )
				continue;
			const ident_t fromLocID = location->getID();
			CIScEventSet::const_iterator event, eventEnd;
			for( event = resItr->second.begin(), eventEnd = resItr->second.end(); event != eventEnd; ++event )
			{
				register CISarc *a = (*event)->findTravelOutArc();
				if( !a )
					continue;

				if( (*event)->t < tEarliest )
				{
					tEarliest = (*event)->t;
					earliestLocID = fromLocID;
				}

				// Add this link to the graph so we can figure out the location sequence with toposort.
				const ident_t toLocID = a->to->location->getID();
				idGraph.connect( fromLocID, toLocID );

				// Add the time this route left the location.
				LocIDTimes::iterator lid;

				if( (lid = locIDTimes.find(fromLocID)) == locIDTimes.end() )
					lid = locIDTimes.insert( fromLocID, TimeVector() );
				lid.data().push_back( a->from->t );		

				// Ensure that the 'to' location and its time is recorded.
				if( (lid = locIDTimes.find(toLocID)) == locIDTimes.end() )
					lid = locIDTimes.insert( toLocID, TimeVector() );
				lid.data().push_back( a->to->t );		
			}
		}

		// Topologically sort the location graph to get the stop sequence for this route.
		IDVector	idLocations;
		idGraph.topoSort( idLocations, earliestLocID );

		// Create a link between the location ids and its position in the route.
		IDVector::const_iterator i, iEnd;
		HashFI<ident_t, size_t> idIndex;
		for( i = idLocations.begin(), iEnd = idLocations.end(); i != iEnd; ++i )
			idIndex.insert( *i, (unsigned int)(i - idLocations.begin()) );

		// Populate the paths of single vehicles through the route.
		StripeVector		stripes;
		CISarcCollection	visitedArcs;

		for( i = idLocations.begin(), iEnd = idLocations.end(); i != iEnd; ++i )
		{
			const ident_t fromLocID = *i;
			CISlocation	*fromLoc = service->locations[fromLocID];
			CIScEventSet	&es = fromLoc->routeCurb[routeDirectionID];
			CIScEventSet::const_iterator event, eventEnd;
			for( event = es.begin(), eventEnd = es.end(); event != eventEnd; ++event )
			{
				register CISarc *a = (*event)->findTravelOutArc();
				if( !a || visitedArcs.contains(a) )
					continue;

				visitedArcs.insertKey( a );

				// Start a new stripe.
				stripes.push_back( Stripe() );
				Stripe	&stripe = stripes.back();
				stripe.push( i - idLocations.begin(), (*event)->t );

				// Create a path following the closest connection times.
				tod_t	tFrom = a->to->t;
				const ident_t toID = a->to->location->getID();
				const size_t iTo = idIndex[toID];
				CISlocation	*curLoc = service->locations[toID];
				if( iTo <= stripe.getLastILoc() )
				{
					// We have a loop of one stop.  Ignore the stripe?  Better than crashing...
					stripes.erase( stripes.end()-1, stripes.end() );
					continue;
				}
				stripe.push( iTo, a->to->t );

				bool	pathExtended;
				do
				{
					pathExtended = false;

					// Only search for a leave connection up to the next arrive time.
					// Find the next arrival time for this route.
					tod_t	tNextArrival = 2 * 24 * 60 * 60;
					CIScEvent	eSearch(tFrom, curLoc, route, service);
					CIScEventSet	&esCur = curLoc->routeCurb[routeDirectionID];
					CIScEventSet::const_iterator eventBegin = esCur.lower_bound(&eSearch), eventEnd = esCur.end();
					CIScEventSet::const_iterator event;
					for( event = eventBegin; event != eventEnd; ++event )
					{
						if( (*event)->t <= tFrom )
							continue;
						register CISarc *a = (*event)->findTravelInArc();
						if( !a || visitedArcs.contains(a) )
							continue;

						tNextArrival = (*event)->t;
						break;
					}
					// Find the next leave connection.  Do not search past the next arrival time.
					for( event = eventBegin; event != eventEnd; ++event )
					{
						register CISarc *a = (*event)->findTravelOutArc();
						if( !a || a->to->t < tFrom || visitedArcs.contains(a) )
							continue;
						if( (*event)->t >= tNextArrival ) // Do not steal the the next vehicle's leave connection.
							break;

						visitedArcs.insertKey( a );
						const ident_t toID = a->to->location->getID();
						const size_t iTo = idIndex[toID];
						if( iTo <= stripe.getLastILoc() )	// We have a loop - don't know what to do with this - start a new stripe?
							break;
						stripe.push( iTo, a->to->t );
						curLoc = service->locations[toID];
						tFrom = a->to->t;
						pathExtended = true;
						break;
					}
				} while( pathExtended );
			}
		}

		// Sequence the stripes into increasing time by location.
		std::sort( stripes.begin(), stripes.end() );

		// Check if consecutive stripes can be merged.
		register size_t c;
		for( c = 0; c < stripes.size() - 1; ++c )
		{
			bool	merged = false;
			const size_t cNext = c + 1;
			Stripe	&sCur = stripes[c], &sNext = stripes[cNext];

			if( !merged )
			{
				if( sCur.getLastILoc() == sNext.getFirstILoc() &&
					sCur.getLastTime() < sNext.getFirstTime() )
				{
					sCur.appendAllButFirst( sNext );
					merged = true;
				}
			}

			if( !merged )
			{
				if( sNext.iLocTimes[1].t >= sCur.getLastTime() && sCur.onlyFirstLocInCommon(sNext) )
				{
					sCur.appendAllButFirst( sNext );
					merged = true;
				}
			}

			if( merged )
				stripes.erase( stripes.begin() + cNext );
		}

		// Cache the timetable in the route so we can access it faster later.
		route->stripes.swap( stripes );
		route->idLocations.swap( idLocations );

		// Get all the connections in the route.	
		getFromToStops( route->fromTo, serviceID, routeDirectionID );
	}

	// Build the return structure.
	const StripeVector	&stripes = route->stripes;
	const IDVector		&idLocations = route->idLocations;

	CISrouteTimesReply *rq = new CISrouteTimesReply();
	rq->route = route;
	rq->service = service;
	const size_t rows = idLocations.size();
	const size_t cols = stripes.size();
	rq->locationTimes.reserve( rows );

	// Initialize everything to invalid times.
	for( register size_t r = 0; r < rows; ++r )
	{
		rq->locationTimes.push_back( CISlocationTimes(service->locations[idLocations[r]]) );
		TimeVector &tv = rq->locationTimes.back().times;
		tv.resize( cols );
		std::fill( tv.begin(), tv.end(), -1 );
	}

	// Fill in the times for each path.
	for( register size_t c = 0; c < cols; ++c )
	{
		register Stripe::ILocTimeVector::const_iterator il, ilEnd;
		for( il = stripes[c].iLocTimes.begin(), ilEnd = stripes[c].iLocTimes.end(); il != ilEnd; ++il )
			rq->locationTimes[il->iLoc][c] = il->t;
	}

	// Transform the connections into a reasonable list of arcs.
	PathifyConnections	connections;
	getPathifyConnections( connections, route->fromTo );
	rq->fromToConnections.reserve( connections.size() );
	{
		for( PathifyConnections::const_iterator c = connections.begin(), cEnd = connections.end(); c != cEnd; ++c )
			rq->fromToConnections.push_back( std::make_pair(service->locations[c->first], service->locations[c->second]) );
	}

	return rq;
}

void	CIS::getRouteStopTimes( TripPlanRequestReply::TimeOfDayVector &todv, const ident_t serviceID, const ident_t routeDirectionID, const ident_t locationID )
{
	todv.clear();

	CISserviceCollection::iterator	sItr = services.find( serviceID );
	if( sItr == services.end() )
		return;
	CISservice *service = sItr.data();
	CISrouteCollection::iterator	rItr = service->routes.find( routeDirectionID );
	if( rItr == service->routes.end() )
		return;
	CISroute *route = rItr.data();
	CISlocationCollection::iterator locItr = service->locations.find( locationID );
	if( locItr == service->locations.end() )
		return;
	CISlocation *location = locItr.data();

	CIScRouteIDEventSet::iterator resItr = location->routeCurb.find(routeDirectionID);
	if( resItr == location->routeCurb.end() )
		return;

	register CIScEventSet::const_iterator event, eventEnd;
	for( event = resItr->second.begin(), eventEnd = resItr->second.end(); event != eventEnd; ++event )
	{
		if( (*event)->findTravelOutArc() )
			todv.push_back( (*event)->t );
	}
}

void	CIS::getRouteStops( CISlocationVector &locv, const ident_t serviceID, const ident_t routeDirectionID )
{
	locv.clear();

	CISserviceCollection::iterator	sItr = services.find( serviceID );
	if( sItr == services.end() )
		return;
	CISservice *service = sItr.data();
	CISrouteCollection::iterator	rItr = service->routes.find( routeDirectionID );
	if( rItr == service->routes.end() )
		return;
	CISroute *route = rItr.data();

	IDGraph	idGraph;	// Add all the locations to the graph so we can figure out the location sequence.

	{
		CISlocationCollection::iterator loc, locEnd;
		for( loc = service->locations.begin(), locEnd = service->locations.end(); loc != locEnd; ++loc )
		{
			CIScRouteIDEventSet::iterator resItr = loc.data()->routeCurb.find(routeDirectionID);
			if( resItr == loc.data()->routeCurb.end() )
				continue;

			CIScEventSet::const_iterator event, eventEnd;
			for( event = resItr->second.begin(), eventEnd = resItr->second.end(); event != eventEnd; ++event )
			{
				register const CISarc *a = (*event)->findTravelOutArc();
				if( a )
					idGraph.connect( a->from->location->getID(), a->to->location->getID() );
			}
		}
	}

	IDVector	idLocations;
	idGraph.topoSort( idLocations );
	locv.reserve( idLocations.size() );
	for( register IDVector::const_iterator loc = idLocations.begin(), locEnd = idLocations.end(); loc != locEnd; ++loc )
		locv.push_back( service->locations.find(*loc).data() );
}

void	CIS::getFromToStops( CISlocationMapSet &fromToMapSet, const ident_t serviceID, const ident_t routeDirectionID )
{
	fromToMapSet.clear();

	CISserviceCollection::iterator	sItr = services.find( serviceID );
	if( sItr == services.end() )
		return;
	CISservice *service = sItr.data();
	CISrouteCollection::iterator	rItr = service->routes.find( routeDirectionID );
	if( rItr == service->routes.end() )
		return;
	CISroute *route = rItr.data();

	CISlocationCollection::iterator loc, locEnd;
	for( loc = service->locations.begin(), locEnd = service->locations.end(); loc != locEnd; ++loc )
	{
		CIScRouteIDEventSet::iterator resItr = loc.data()->routeCurb.find(routeDirectionID);
		if( resItr == loc.data()->routeCurb.end() )
			continue;
		register CIScEventSet::const_iterator event, eventEnd;
		for( event = resItr->second.begin(), eventEnd = resItr->second.end(); event != eventEnd; ++event )
		{
			register const CISarc *a = (*event)->findTravelOutArc();
			if( a )
			{
				CISlocationMapSet::iterator ft = fromToMapSet.find(a->from->location->getID());
				if( ft == fromToMapSet.end() )
					ft = fromToMapSet.insert( std::make_pair(a->from->location->getID(), std::set<ident_t>() ) ).first;
				ft->second.insert( a->to->location->getID() );
			}
		}
	}
}
