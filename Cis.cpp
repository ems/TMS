//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <afx.h>
#include "cis.h"
#include "cistms.h"
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <strstream>
#include <winbase.h>

#define DHeapSetIndex
#include "dheap.h"
#include "HashObjFI.h"
#include "List.h"
#include "ParseStream.h"

using namespace TripPlanRequestReply;

static bool fTripPlanLog = false;	// Controlled by registry entry TripPlanLog

//#define PRINT_SP_SEARCH
//#define PRINT_TRANSFER_GROUPS
#include <fstream>
#include "TFormat.h"

using namespace std;

MMinitMax( CISvehicle,		16 );
MMinitMax( CISdirection,	16 );
MMinitMax( CISroute,		64 );
MMinitMax( CISservice,		8 );
MMinitMax( CISlocation,		256 );
MMinitMax( CIStransferGroup,128 );
MMinitMax( CIScEvent,		2048 );
MMinitMax( CISarc,			2048 );
MMinitMax( CISarcList::CISarcElement, 256 );
MMinitMax( CISsolution,		16 );
MMinitMax( CISresult,		16 );
MMinit( CIStranslate::SymbolValueElement );

typedef HashObjFI< QTree::Point, CISlocationCollection >	PointLocationHash;

template < class Collection >
void	deleteHashContents( Collection &c )
{
	Collection::iterator i;
	ForAllInCollection( c, i )
		delete i.data();
	c.clear();
}

class formatTime
{
public:
	formatTime( const tod_t aT ) : t(aT) {}
	operator const char *();

	enum Format { APX, AMPM, Military };

	static	void	setFormat( const CISmessageMap &mm );

private:
	static	Format	format;
	tod_t	t;
	char	szTime[32];
};

formatTime::Format	formatTime::format = formatTime::AMPM;

formatTime::operator const char *()
{
	tod_t	hours	= t / (60 * 60);
	tod_t	minutes = (t / 60) % 60;
	tod_t	seconds = t % 60;
	
	char	*ampm = "";

	switch( format )
	{
	case AMPM:
		if( hours < 12 )
			ampm = "am";
		else
			ampm = "pm";
		if( hours >= 13 )
			hours -= 12;
		else if( hours == 0 )
			hours = 12;
		sprintf( szTime, "%2d:%02d%s", hours, minutes, ampm );
		break;

	case APX:
		if( hours < 12 )
			ampm = "A";
		else
			ampm = "P";
		if( hours >= 13 )
			hours -= 12;
		else if( hours == 0 )
		{
			hours = 12;
			ampm = "X";
		}
		sprintf( szTime, "%2d%02d%s", hours, minutes, ampm );
		break;

	case Military:
		sprintf( szTime, "%2d:%02d", hours, minutes );
		break;
	}

	return szTime;
}

// Create a class to quote small strings.
class qt
{
public:
	qt( const char *str )			{ sprintf(sz, "\"%s\"", str); }
	qt( const int i )				{ sprintf(sz, "\"%d\"", i ); }
	qt( const unsigned int i )		{ sprintf(sz, "\"%u\"", i ); }
	qt( const long t )				{ sprintf(sz, "\"%ld\"", (long)t ); }
	qt( const double d )			{ sprintf(sz, "\"%f\"", d ); }

	operator const char *() const	{ return sz; }
	const char *str() const			{ return sz; }
private:
	char	sz[1024];
};

inline ostream &operator<<( ostream &os, const qt &q ) { return os << q.str(); }

template < class Collection >
void	os_release( Collection &c )
{
	if( c.size() > 0 )
	{
		// Put all the elements in a list.
		SList< Collection::value_type >	list;
		Collection::iterator i;
		ForAllInCollection( c, i )
			list.insert( *i );

		// Clear the collection.
		c.clear();

		// Delete the elements on the list.
		while( !list.empty() )
			delete list.remove();
	}
}

static	inline void	SetIndex( CIScEvent *&s, const size_t i ) { s->iheap = i; }

CIScEvent::~CIScEvent()
{
	register CISarc *a, *aNext;
	for( a = outList; a != NULL; a = aNext )
	{
		aNext = a->outNext;
		delete a;
	}
}

CISlocation::~CISlocation()
{
	os_release( shelter );
	for( CIScRouteIDEventSet::iterator r = routeCurb.begin(), rEnd = routeCurb.end(); r != rEnd; ++r )
		os_release( r->second );
}

CIStransferGroup &CIStransferGroup::operator |= ( CIStransferGroup &g )
{
	if( &g != this )
	{
		CISlocationCollection::iterator i;
		ForAllInCollection( g.locations, i )
		{
			i.data()->transferGroup = this;
			locations.insert( i.data()->getID(), i.data() );
		}
		g.locations.clear();
	}
	return *this;
}

void CISnodeEquivalences::init()
{
#ifndef TRIP_PLANNER_SERVER
		// Insert all the node equivalences into the hash table.
	int	rcode2;
	for(rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
		rcode2 == 0;
		rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0) )
	{
		if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
			insert( CONNECTIONS.fromNODESrecordID, CONNECTIONS.toNODESrecordID, CONNECTIONS.connectionTime );
	}
#endif
#ifdef NOT_NECESSARY
		/*
		// Node equivalences are not used in regular trip planning - there is no need to read the data.
//
//  ====================================
//  Equivalences File - Equivalences.txt
//  ====================================
//
//  Tab separated values
//
//  long     From nodes record ID
//  long     To nodes record ID
//  long     Connection time in minutes
//
  CStdioFile Equivalences;
  CString    inputLine;
  char      *ptr;
  ident_t    fromLocID, toLocID;
  tod_t     connectionTime;

  Equivalences.Open("Web-Based Trip Planner Data Files\\Equivalences.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone);

  Equivalences.SeekToBegin();
  while(Equivalences.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  From nodes record ID
//
    ptr = strtok(tempString, "\t");
    fromLocID = atol(ptr);
//
//  To nodes record ID
//
    ptr = strtok(NULL, "\t");
    toLocID = atol(ptr);
//
//  Connection time
//
    ptr = strtok(NULL, "\r\n");
    connectionTime = atol(ptr);
//
//  Add the equivalence and cycle back
//
    insert(fromLocID, toLocID, connectionTime);
	}
//
//  Done with Equivalences.txt - close it
//
  if(Equivalences.m_pStream)
  {
    Equivalences.Close();
  }
  */
#endif // NOT_NECESSARY
}

CISnodeEquivalences::~CISnodeEquivalences()
{
	IDIDHash::iterator n;
	ForAllInCollection( nodes, n )
		delete n.data();
}

//---------------------------------------------------------------------------------------

inline CISarc	*CISservice::connect( CIScEvent *from, CIScEvent *to, 
                                      const CISroute *route, const ident_t idTrip, const ident_t idBlock,
                                      const CISvehicle *vehicle, const CISservice::ArcType arcType )
{
	Assert( from->t <= to->t );

	CISarc	*a = new CISarc( from, to, route, vehicle, idTrip, idBlock, arcType );
	a->addToOutList( from->outList );
	a->addToInList( to->inList );
	if( route && !routes.contains(route->getID()) )
		routes.insert( route->getID(), const_cast<CISroute *>(route) );	// Keep track of all the routes in this service.
	return a;
}

void	CISarcList::reverse()
{
	tail = head;
	register CISarcElement *prev = NULL, *next, *cur;
	for( cur = head; cur != NULL; cur = next )
	{
		next = cur->next;
		cur->next = prev;
		prev = cur;
	}
	head = prev;
}

void	CISservice::addTravel( CIS &cis, const tod_t fromTime, CISlocation *fromLoc, const tod_t toTime, CISlocation *toLoc,
                CISroute *route, const ident_t idTrip, const ident_t idBlock,
                CISvehicle *vehicle )
{
  Assert( fromTime <= toTime );

	// Check if the locations are present.
	// If not, add them to this service's locations.
	CISlocationCollection::iterator il;

	if( (il = locations.find(fromLoc->getID())) == locations.end() )
		il = locations.insert( fromLoc->getID(), new CISlocation(*fromLoc) );
	CISlocation	*from = il.data();

	if( (il = locations.find(toLoc->getID())) == locations.end() )
		il = locations.insert( toLoc->getID(), new CISlocation(*toLoc) );
	CISlocation	*to = il.data();

	// Get the curb events.
	CIScEventSet::iterator curbItr;

	CIScEvent		*fromCurb, *toCurb;

	CIScEvent		curbSearch;
	curbSearch.route = route;
	curbSearch.service = this;

	// Check if the "from" location has events for this route - add it if necessary.
	curbSearch.t = fromTime;
	CIScRouteIDEventSet::iterator res = from->routeCurb.find(route->getID());
	if( res == from->routeCurb.end() )
		res = from->routeCurb.insert( std::make_pair(route->getID(), CIScEventSet()) ).first;

	// Check if the "from" location's route has an event for the "from" time - add it if necessary.
	curbItr = (*res).second.find(&curbSearch);
	if( curbItr != (*res).second.end() )
		fromCurb = *curbItr;
	else
	{
		fromCurb = *(*res).second.insert( new CIScEvent(fromTime, from, route, this) ).first;
		numCIScEvents++;
	}

	// Check if the "to" location has events for this route - add it if necessary.
	curbSearch.t = toTime;
	res = to->routeCurb.find(route->getID());
	if( res == to->routeCurb.end() )
		res = to->routeCurb.insert( std::make_pair(route->getID(), CIScEventSet()) ).first;

	// Check if the "to" location's route has an event for the "to" time - add it if necessary.
	curbItr = (*res).second.find(&curbSearch);
	if( curbItr != (*res).second.end() )
		toCurb = *curbItr;
	else
	{
		toCurb = *(*res).second.insert( new CIScEvent(toTime, to, route, this) ).first;
		numCIScEvents++;
	}

	connect( fromCurb, toCurb, route, idTrip, idBlock, vehicle, CISservice::Travel );
}

void	CIS::addTravel( const tod_t fromTime, const ident_t fromLoc, const tod_t toTime, const ident_t toLoc,
                    const ident_t idCISroute, const ident_t idCISdirection, const ident_t idTrip, const ident_t idBlock,
                    const ident_t idCISvehicle, const ident_t idService )
{
	// The locations must be present or this will crash (and rightly so!)
	CISlocation	*from	 = locations.find(fromLoc).data();
	CISlocation	*to		 = locations.find(toLoc).data();

	CISroute	*route	 = routes.find(combineRouteDirectionID(idCISroute,idCISdirection)).data();
	CISvehicle	*vehicle = vehicles.find(idCISvehicle).data();
	CISservice  *service = services.find(idService).data();

#ifdef PRINT_SP_SEARCH
  if( os )
  {
#ifdef FIXLATER
    (*os) << "fromTime=" << formatTime(fromTime) << " ";
    (*os) << "fromLocID=" << fromLoc << " ";
    (*os) << "toTime=" << formatTime(fromTime) << " ";
    (*os) << "toLocID=" << toLoc << " ";
    (*os) << "routeID=" << idCISroute << " ";
    (*os) << "directionID=" << idCISdirection << " ";
    (*os) << "tripID=" << idTrip << " ";
    (*os) << "blockID=" << idBlock << " ";
    (*os) << "vehicleID=" << idCISvehicle << " ";
    (*os) << "serviceID=" << idService << "\n";
#endif
    (*os) << "svc=" << qt(service->getName()) << " ";
    (*os) << "rte=" << qt(route->getName()) << " ";
    (*os) << "dir=" << qt(route->getDirection()->getName()) << " ";
    (*os) << "blk=" << idBlock << " ";
    (*os) << "veh=" << qt(vehicle->getName()) << " ";
    (*os) << "frmT=" << qt(formatTime(fromTime)) << " ";
    (*os) << "frm=" << qt(from->getName()) << " ";
    (*os) << "toT=" << qt(formatTime(fromTime)) << " ";
    (*os) << "to=" << qt(to->getName()) << " ";
    (*os) << "trp=" << idTrip << "\n";
 }
#endif // PRINT_SP_SEARCH
	service->addTravel( *this, fromTime, from, toTime, to, route, idTrip, idBlock, vehicle );
}

class SquareSpiral
{
	// Traces an expanding square spiral.
	enum Direction { Right, Down, Left, Up };

public:
	SquareSpiral( const int startingSize = 0 ) { reset(startingSize); }

	void	reset( const int aSize = 0 )
	{
		size = aSize;
		if( size < 0 )
			size = -size;
		direction = (size > 0 ? Right : Up);
		x = -size;
		y = size;
	}

	int	getX() const	{ return x; }
	int	getY() const	{ return y; }
	void getXY( int &aX, int &aY ) const	{ aX = x; aY = x; }

	int	getSize() const { return size; }

	void	next()
	{
		switch( direction )
		{
		case Right:
			if( ++x > size ) { x = size; y = size - 1; direction = Down; }
			break;
		case Down:
			if( --y < -size ) { y = -size; x = size - 1; direction = Left; }
			break;
		case Left:
			if( --x < -size ) { x = -size; y = -size + 1; direction = Up; }
			break;
		case Up:
			if( ++y >= size ) reset( size + 1 );
			break;
		}
	}

protected:
	int	x, y, size;
	Direction direction;
};

void	CISservice::initSolve( CIS &cis )
{
	// Check if there are no locations for this service.
	if( locations.empty() )
		return;

	// Build the qtrees.
	// Find the limits.
	CISlocationCollection::iterator locItr = locations.begin();
	int	x, y;
	locItr.data()->getCoord( x, y );
	int	xMin = x, xMax = xMin;
	int yMin = y, yMax = yMin;
	for( ++locItr; locItr != locations.end(); ++locItr )
	{
		locItr.data()->getCoord( x, y );

		if(		 x < xMin)	xMin = x;
		else if( x > xMax)	xMax = x;

		if(		 y < yMin)	yMin = y;
		else if( y > yMax)	yMax = y;
	}
	// Create a quad tree with expanded limits to get the boundary queries.
	const int dx = (xMax - xMin), dy = (yMax - yMin);
	qtree.initRange( xMin - dx, yMin - dy, xMax + dx, yMax + dy );

	// Keep track of points with the same lat/long.
	PointLocationHash	pl(locations.size());
	ForAllInCollection( locations, locItr )
	{
		QTree::Point	p( locItr.data()->getPoint() );
		PointLocationHash::iterator found = pl.find( p );
		if( found == pl.end() )		// If it matches another point, this is an equivalence.
			found = pl.insert( p, CISlocationCollection() );

		// Register this location with this coordinate.
		found.data().insert( *locItr, locItr.data() );
	}

	// Insert the points into the quadtree.
	ForAllInCollection( locations, locItr )
	{
		SquareSpiral	ss;

		// If any points collide, check in an expanding spiral until we find a unique position.
		locItr.data()->getCoord( x, y );
		while( qtree.contains(QTree::Point(x + ss.getX(), y + ss.getY())) )
			ss.next();

		qtree.insert( QTree::Point(x + ss.getX(), y + ss.getY()), (ident_t)(locItr.data()) );
	}

	// Determine all the locations in the same transfer group.
	ident_t	transferGroupID = 0;
	register CISlocation *loc, *locCur;

	// First, consider all locations that have the same lat/long.
	PointLocationHash::iterator plItr;
	ForAllInCollection( pl, plItr )
	{
		// Don't bother with unique locations.
		if( plItr.data().size() == 1 )
			continue;

		char	name[16];
		sprintf( name, "tg%d", transferGroupID );
		CIStransferGroup	*transferGroup = new CIStransferGroup( transferGroupID++, name );
		transferGroups.insert( transferGroup->getID(), transferGroup );

		// Put all locations that share the same lat/long into the same transfer group.
		ForAllInCollection( plItr.data(), locItr )
			transferGroup->insert( locItr.data() );
	}

	// Consider all locations within the radius of transferDistance.
	register int numPointIDs, i;
	const int maxPointIDs = 64;
	QTree::PointID	pointIDs[maxPointIDs];
	ForAllInCollection( locations, locItr )
	{
		loc = locItr.data();
		if( (numPointIDs = qtree.findInRadius(loc->getPoint(), cis.transferDistance, pointIDs, maxPointIDs)) <= 1 )
			continue;

		// Check if any of the found points already belong to a transfer group.
		CIStransferGroup	*transferGroup = NULL;
		for( i = 0; i < numPointIDs; i++ )
		{
			locCur = (CISlocation *)pointIDs[i].id;
			if( locCur->transferGroup != NULL )
			{
				transferGroup = locCur->transferGroup;
				break;
			}
		}
		// Create a new transfer group if there isn't one already.
		if( transferGroup == NULL )
		{
			char	name[16];
			sprintf( name, "tg%d", transferGroupID );
			transferGroup = new CIStransferGroup( transferGroupID++, name );
			transferGroups.insert( transferGroup->getID(), transferGroup );
		}

		// Merge all the locations and transfer groups together.
		// This will also net the locations with the same lat/long since we have already combined them
		// into their own transfer groups.
		for( i = 0; i < numPointIDs; i++ )
		{
			locCur = (CISlocation *)pointIDs[i].id;
			if( locCur->transferGroup == NULL )
				transferGroup->insert( locCur );
			else if( locCur->transferGroup != transferGroup )
			{
				// When we find a location that is already part of a transfer group, we merge
				// that transfer group into ours.
				CIStransferGroup	*oldTransferGroup = locCur->transferGroup;
				*transferGroup |= *oldTransferGroup;
				transferGroups.remove( oldTransferGroup->getID() );
				delete oldTransferGroup;
			}
		}
	}

#ifdef PRINT_TRANSFER_GROUPS
	{
		ofstream ofs( "transfers.txt" );
		CIStransferGroupCollection::iterator tg;
		ForAllInCollection( transferGroups, tg )
		{
			ofs << tg.data()->getName() << '\n';
			CISlocationCollection::iterator lo;
			ForAllInCollection( tg.data()->locations, lo )
				ofs << '\t' << lo.data()->getName()
					<< "\t(" << lo.data()->getLatitude() << ", " << lo.data()->getLongitude() << ")\n";
		}
	}
#endif

	// Connect all the curb events to shelter events.
	CIScRouteIDEventSet::iterator resItr;
	CIScEventSet::iterator	curbItr;
	CIScEvent shelterSearch, *newShelterEvent;
	ForAllInCollection( locations, locItr )
	{
		loc = locItr.data();

		ForAllInCollection( loc->routeCurb, resItr )
		{
			ForAllInCollection( (*resItr).second, curbItr )
			{
				const bool fHasDepartures = (*curbItr)->hasDepartures();
				const bool fHasArrivals = (*curbItr)->hasArrivals();

				if( fHasDepartures )
				{
					// Attempt to add a new shelter leave event.
					newShelterEvent = new CIScEvent((*curbItr)->t - cis.entryTime, loc);
					std::pair<CIScEventSet::iterator, bool> ib = loc->shelter.insert( newShelterEvent );
					if( ib.second )
						numCIScEvents++;			// add was successful
					else
						delete newShelterEvent;		// add was not successful - delete the new event.

					// Connect the shelter to the leave event for the trip.
					connect( *ib.first, *curbItr, CISservice::Enter );
				}

				if( fHasArrivals )
				{
					// Attempt to add a new shelter arrive event.
					newShelterEvent = new CIScEvent((*curbItr)->t + cis.exitTime, loc);
					std::pair<CIScEventSet::iterator, bool> ib = loc->shelter.insert( newShelterEvent );
					if( ib.second )
						numCIScEvents++;			// add was successful
					else
						delete newShelterEvent;		// add was not successful - delete the new event.

					// Connect the arrive event for the trip to the shelter.
					connect( *curbItr, *ib.first, CISservice::Exit );
				}

				if( fHasArrivals && !fHasDepartures )
				{
					register const CISarc *a;
					for( a = (*curbItr)->inList; a; a = a->inNext )
					{
						if( a->isTravel() )
							break;
					}
					const ident_t idBlockIn = a->idBlock;

					CIScEventSet::iterator curbItrNext = curbItr;
					if( ++curbItrNext != (*resItr).second.end() && (*curbItrNext)->hasDepartures() )
					{
						for( a = (*curbItrNext)->outList; a; a = a->outNext )
						{
							if( a->isTravel() && a->idBlock == idBlockIn )
							{
								// Add a special wait arc for the dwell time.
								connect( *curbItr, *curbItrNext, CISservice::Dwell );
								break;
							}
						}
					}
				}
			}
		}
	}

	// Add all the wait and transfer arcs.
	CIScEventSet::iterator shelterItr;
	ForAllInCollection( locations, locItr )
	{
		loc = locItr.data();

		// Create the transfers for this location.
		if( loc->transferGroup != NULL )
		{
			QTree::Point from( loc->getPoint() );
			ForAllInCollection( loc->shelter, shelterItr )
			{
				// Only consider transfers from shelter events that have arrivals.
				if( !(*shelterItr)->hasArrivals() )
					continue;

				// Attempt to transfer to all cevents next to our location.
				CISlocationCollection::iterator litr;
				ForAllInCollection( loc->transferGroup->locations, litr )
				{
					locCur = litr.data();
					if( locCur == loc ) continue;	// Don't transfer to ourselves.

					// Compute how long it would take to walk between the locations.
					// Assume that equivalent nodes require the given transfer time.
					int transferTime = 0;
					if( !cis.nodeEquivalences.isEquivalent(loc->getID(), locCur->getID()) )
					{
						const int walkingDistance = from.distance( locCur->getPoint() );
						transferTime = cis.walkingDistanceToTime(walkingDistance);
						// Make sure transfer time is at least a few minutes between locations.
						if( transferTime < cis.transferTimeMin ) transferTime = cis.transferTimeMin;
					}
					else
						transferTime = cis.nodeEquivalences.getConnectionTime(loc->getID(), locCur->getID());

					// Find the earliest event at the "to" location we can walk to.
					shelterSearch.t = (*shelterItr)->t + transferTime;
					CIScEventSet::iterator found = locCur->shelter.lower_bound( &shelterSearch );
					while( found != locCur->shelter.end() && (*found)->t < shelterSearch.t )
						++found;
					if( found == locCur->shelter.end() )
						continue;

					// Connect the transfer.
					connect( *shelterItr, *found, CISservice::WalkTransfer );
				}
			}
		}

		// Create the wait arcs at the shelter for this location.
		shelterItr = loc->shelter.begin();
		if( shelterItr == loc->shelter.end() )
			continue; // This location has no shelter events!
		CIScEventSet::iterator nextShelterItr = shelterItr;
		for( ++nextShelterItr; !(nextShelterItr == loc->shelter.end()); shelterItr = nextShelterItr, ++nextShelterItr )
			connect( *shelterItr, *nextShelterItr, CISservice::Wait );
	}
}

void	CIS::initSolve()
{
	if( driverPlan )
	{
		// Make sure that all nodes with the same lat/long are also entered as node equivalences.
		CISlocationCollection::iterator loc1, loc2, locEnd = locations.end();
		for( loc1 = locations.begin(); loc1 != locEnd; ++loc1 )
		{
			for( loc2 = loc1, ++loc2; loc2 != locEnd; ++loc2 )
				if( loc1.data()->eqLongitudeLatitude(*loc2.data()) )
					nodeEquivalences.insert( *loc1, *loc2, 0 );
		}

		// Override any default equivalences with user-specified data.
		nodeEquivalences.init();	
	}

	NMax = 0;
	CISserviceCollection::iterator si;
	ForAllInCollection( services, si )
	{
		si.data()->initSolve( *this );
		if( si.data()->numCIScEvents > NMax )
			NMax = si.data()->numCIScEvents;
	}
	// Allocate the shortest path heap.
	NMax /= 10;
	if( NMax < 2048 ) NMax = 2048;
	heap = new CIScEvent * [NMax];
	N = 0;
	highestN = NMax;
}

void CIS::spReset()
{
	register size_t i;

	// Reset all the nodes still in the heap.
	for( i = 0; i < N; i++ )
		heap[i]->spReset();

	// Reset all the nodes processed through the heap.
	for( i = highestN; i < NMax; i++ )
		heap[i]->spReset();

	// Reset the size indices.
	highestN = NMax;
	N = 0;
}

inline CIScEvent *CIS::extractMin()
{
	// Remove and save the top element.
	register CIScEvent	*s = heap[0];
	DHeapRemoveTop( heap, N, CIScEventCmpD() );
	return heap[--highestN] = s;
}

void	CIS::resizeHeap()
{
	// Double the size of the heap.
	register size_t i;
	const	size_t	newNMax = NMax * 2;

	CIScEvent	**newHeap = new CIScEvent * [newNMax];
	for( i = 0; i < N; i++ )
		newHeap[i] = heap[i];
	for( i = highestN; i < NMax; i++ )
		newHeap[newNMax - (NMax - i)] = heap[i];

	delete [] heap;
	heap = newHeap;
	highestN = newNMax - (NMax - highestN);
	NMax = newNMax;
}

inline	void CIS::insert( CIScEvent *s )
{
	if( N+4 > highestN ) resizeHeap();
	heap[N++] = s;
	DHeapInsert( heap, N, CIScEventCmpD() );
}

inline	void CIS::insertInitial( CIScEvent *s )
{
	if( N+4 > NMax ) resizeHeap();
	heap[N++] = s;
}

void	CIS::spLeave( const CISarcFlags arcFlags )
{
#ifdef PRINT_SP_SEARCH
	static	bool	enablePrint = true;
	ofstream	ofs;
	if( enablePrint )
		ofs.open("spLeave.txt");
#endif // PRINT_SP_SEARCH

	DHeapBuild( heap, N, CIScEventCmpD() );

	register CIScEvent *sMin, *to;
	register CISarc  *a;
	register utility_t  d;
	utility_t	dFinal = CIScEvent::dMax();
	CIScEvent	*parentFinal = NULL;

	while( N > 0 )
	{
		sMin = extractMin();

#ifdef PRINT_SP_SEARCH
		if( enablePrint )
		{
			ofs << "'" << sMin->location->getName()
				<< "'\t" << (const char *)tFormat(sMin->t)
				<< " " << (sMin->arcToParent ? sMin->arcToParent->getTypeName() : "")
				<< " d=" << sMin->d << '\n';
			if( sMin->location->isPotentialLeave )
				ofs << "\t" << "isPotentialLeave\n";
		}
#endif // PRINT_SP_SEARCH

		// Scan the connection to the final destination.
		if( sMin->location->isPotentialArrive )
		{
#ifdef PRINT_SP_SEARCH
			if( enablePrint )
				ofs << "\tthis is a potential arrive location\n";
#endif // PRINT_SP_SEARCH

			// Compensate for the extra transferPenalty that will be levied on the first enter arc.
			// Do not apply walking disutility for driver plans - use the exact time as specified in the node equivalence.
			if( (d = sMin->d
					+ (driverPlan ? sMin->location->arriveWalkingTime : walkingUtility(sMin->location->arriveWalkingTime))
					- transferPenalty / 2) < dFinal )
			{
#ifdef PRINT_SP_SEARCH
				if( enablePrint )
					ofs << "\tbest arrive utility so far: "  << d << '\n';
#endif // PRINT_SP_SEARCH
				parentFinal = sMin;
				CISsolution *ss;
				if( solutionHeap.size() < solutionMax )
				{
					// We don't have enough plans yet - keep 'em comin.
					solutionHeap.insert( ss = new CISsolution(parentFinal, d) );
					if( solutionHeap.size() == solutionMax )
						dFinal = d;
				}
				else
				{
					// Replace the top of the plan heap.
					ss = solutionHeap.extractTop();
					ss->reset( parentFinal, dFinal = d );
					solutionHeap.insert( ss );
				}
#ifdef PRINT_SP_SEARCH
				ofs << "\t\tBest Path\n";
				for( CIScEvent *e = ss->parentFinal; e; e = e->parent() )
				{
					ofs << "\t\t" << (const char *)tFormat(e->t) << ": " << e->location->getName();
					if( e->arcToParent && e->arcToParent->isTravel() )
					{
						CISarc *a = e->arcToParent;
						ofs << ", " << a->route->getName() << ", " << a->idBlock << ", " << a->idTrip;
					}
					ofs << '\n';
				}
#endif // PRINT_SP_SEARCH
			}
#ifdef PRINT_SP_SEARCH
			else
			{
				if( enablePrint )
					ofs << "\tarrive utility " << d << " >= best arrive utility so far: " << dFinal << '\n';
			}
#endif // PRINT_SP_SEARCH
		}

		// Scan the potential leave, transfer and wait connections at this event.
		for( a = sMin->outList; a != NULL; a = a->outNext )
		{
			if( !a->matches(arcFlags) )
				continue;
			to = a->to;
#ifdef PRINT_SP_SEARCH
			if( enablePrint )
				ofs << "\t'" << to->location->getName()
					<< "'\t" << (const char *)tFormat(to->t)
					<< " " << (sMin->arcToParent ? sMin->arcToParent->getTypeName() : "")
					<< " (" << elapsedUtility(a) << ")";
#endif // PRINT_SP_SEARCH
			const utility_t teu = leaveTransferExpireUtility(a);
			if( (d = sMin->d + elapsedUtility(a) + teu) >= dFinal )
			{
#ifdef PRINT_SP_SEARCH
				if( enablePrint )
					ofs << ' ' << d << " >= best known time of " << dFinal << '\n';
#endif // PRINT_SP_SEARCH
				continue;
			}
			if( d < to->d )
			{
				if( to->d == CIScEvent::dMax() )
				{
					to->setDArcToParent(d, a);
					insert( to );
				}
				else
				{
					to->setDArcToParent(d, a);
					DHeapPromote( heap, to->iheap, CIScEventCmpD() );
				}
				// Reset the time the last fare was paid if necessary
				// or keep track of the time since the last fare was paid.
				to->tCur = (teu > (utility_t)0 ? 0 : a->from->tCur + a->getElapsed());

#ifdef PRINT_SP_SEARCH
				if( enablePrint )
					ofs << " updated from " << to->d << " to " << d << "\n";
#endif // PRINT_SP_SEARCH
			}
#ifdef PRINT_SP_SEARCH
			else
				if( enablePrint )
					ofs << ' ' << d << " >= " << to->d << "\n";
#endif // PRINT_SP_SEARCH
		}
	}
}

void	CIS::spArrive( const CISarcFlags arcFlags )
{
	DHeapBuild( heap, N, CIScEventCmpD() );

	register CIScEvent *sMin, *from;
	register CISarc  *a;
	register utility_t  d;
	utility_t	dFinal = CIScEvent::dMax();
	CIScEvent	*parentFinal = NULL;

	while( N > 0 )
	{
		sMin = extractMin();

		// Scan the connection to the starting destination.
		if( sMin->location->isPotentialLeave )
		{
			// Compensate for the extra transferPenalty that will be levied on the first enter arc.
			if( (d = sMin->d
					+ (driverPlan ? sMin->location->leaveWalkingTime : walkingUtility(sMin->location->leaveWalkingTime))
					- transferPenalty/2) < dFinal )
			{
				parentFinal = sMin;
				CISsolution *ss;
				if( solutionHeap.size() < solutionMax )
				{
					// We don't have enough plans yet - keep 'em comin.
    				solutionHeap.insert( ss = new CISsolution(parentFinal, d) );
					if( solutionHeap.size() == solutionMax )
						dFinal = d;
				}
				else
				{
					// Replace the top of the plan heap.
					ss = solutionHeap.extractTop();
					ss->reset( parentFinal, dFinal = d );
					solutionHeap.insert( ss );
				}
			}
		}
		
		// Scan the potential leave, transfer and wait connections at this event.
		for( a = sMin->inList; a != NULL; a = a->inNext )
		{
			if( !a->matches(arcFlags) )
				continue;
			from = a->from;
			if( (d = sMin->d + elapsedUtility(a)) >= dFinal )
				continue;
			if( d < from->d )
			{
				if( from->d == CIScEvent::dMax() )
				{
					from->setDArcToParent(d, a);
					insert( from );
				}
				else
				{
					from->setDArcToParent(d, a);
					DHeapPromote( heap, from->iheap, CIScEventCmpD() );
				}
				from->tCur = a->to->tCur + a->getElapsed();	// Keep track of the total elapsed time.
			}
		}
	}
}

struct SegmentCrossesBoundary
{
	SegmentCrossesBoundary( const QTree::Point &aOrigin, Trapezoidal &aTrapezoidal )
		: origin(aOrigin.x, aOrigin.y), t(aTrapezoidal) {}

	const Trapezoidal::Point	origin;
	const Trapezoidal	&t;

	// Return true if the segment formed from the origin and the given destination cross a boundary line.
	bool operator()( const QTree::PointID &destination ) const
	{
		return const_cast<Trapezoidal &>(t).
			crossesExistingSegment( origin, Trapezoidal::Point(destination.point.x, destination.point.y) );
	}
};

void	CIS::filterPoints( const QTree::Point &p, QTree::PointID *closest, size_t &numClosest )
{
	if( numClosest == 0 )
		return;

	// Check if any points would cross boundaries and remove them.
	if( !boundaries.empty() )
		numClosest = std::remove_if( closest, closest + numClosest, SegmentCrossesBoundary(p, boundaries) ) - closest;

#ifdef FIXLATER
	// Remove points that are too far to walk to.
	// Assume that the points are ordered by increasing distance.
	register size_t i;
	register coord_t dCur, dLast;
	for( i = 1; i < numClosest; ++i )
	{
		// If the point is within walking distance, continue.
		dCur = p.distance(closest[i].point);
		if( dCur <= walkingRadius )
			continue;

		// If the point is relatively close to the previous point
		// but outside the walking distance, keep it.
		dLast = p.distance(closest[i-1].point);
		if( (double)(dCur - dLast) <= dLast/10.0 )
			continue;
	}
	if( i < numClosest )
		numClosest = i;
#endif
}

// If t is < 0, the plan is assumed to leave as early as possible.
// If t is >= 48 hours, the plan is assumed to arrive as late as possible.
// If 0 <= t < 48 hours, the leaveFlag is used to determine whether the plan
// is a leave or arrive plan.

CIS::PlanStatus CIS::plan( const char *szLeave, const Point leave, const long fromNODESrecordID,
						const char *szArrive, const Point arrive, const long toNODESrecordID,
						const tod_t t,
						const int leaveFlag,
						const ident_t idService,
						const CISvehicleFlags vehicleFlags,
						CISresultCollection &resultCollection /* return */ )
{
#ifdef PRINT_SP_SEARCH
	ofstream	ofs;
	ofs.open("spLocations.txt");
#endif // PRINT_SP_SEARCH

	solutionHeap.clear();
	solutionMax = resultCollection.sizeMax();	// We know the collection will have at least 1 element.

	resultCollection.reset();
	resultCollection.setCIS( this );
	resultCollection.setStatus( good );
	resultCollection.setHasPass( hasPass = 0 );	// FIXLATER
	resultCollection.setDriverPlan( driverPlan );
	if( driverPlan )
		resultCollection.setNODESrecordID( fromNODESrecordID, toNODESrecordID );

	resultCollection.setLeave( leave, szLeave );
	resultCollection.setArrive( arrive, szArrive );

	// Set trip flags and times.
	const tod_t	leaveLateTime = 48 * 60 * 60;	// 48 hours
	const bool		leaveAsEarlyAsPossible = (t < 0);
	const bool		arriveAsLateAsPossible = (t >= leaveLateTime);
	const bool		isLeave  = (leaveAsEarlyAsPossible ? true : arriveAsLateAsPossible ? false : leaveFlag);
	const bool		isArrive = !isLeave;
	const tod_t	leaveTime	= leaveAsEarlyAsPossible ? -leaveLateTime : (isLeave ?  t : -1024);
	const tod_t	arriveTime	= arriveAsLateAsPossible ?  leaveLateTime : (isArrive ? t : -1024);

	// Determine the plan type.
	if( leaveAsEarlyAsPossible )
	{
		resultCollection.setType( leaveEarliest );
		resultCollection.setT( 0 );
	}
	else if( arriveAsLateAsPossible )
	{
		resultCollection.setType( CIS::leaveLatest );
		resultCollection.setT( 48 * 60 * 60 );
	}
	else
	{
		if( isArrive )
			resultCollection.setType( CIS::arriveAt );
		else
			resultCollection.setType( CIS::leaveAt );
		resultCollection.setT( t );
	}

	// Determine service and arc flags.
	// Check if we are told there is no service today.
	if( idService == (ident_t)ServiceCalendar::NoID )
	{
		resultCollection.setService( NULL );
		resultCollection.setStatus( noService );
		return noService;
	}
	// If we are passed in an unrecognizable service and there is only one service we have, use the one we know.
	CISserviceCollection::iterator si = services.find(idService);
	if( si == services.end() && services.size() == 1 )
		service = services.begin().data();
	else
		service = si.data();
	resultCollection.setService( service  );
	const CISarcFlags	arcFlags( vehicleFlags );

	register size_t i;

	// Convert long/lat to meters to preserve sanity.
	int	x, y;
	Distance::CoordFromLongLat( x, y, leave.longitude, leave.latitude );
	QTree::Point leavePoint( x, y );

	Distance::CoordFromLongLat( x, y, arrive.longitude, arrive.latitude );
	QTree::Point arrivePoint( x, y );

	// Compute what it would take to walk between the locations.
	size_t	directWalkingTime;
	{
		if( driverPlan && nodeEquivalences.isEquivalent(fromNODESrecordID, toNODESrecordID) )
			directWalkingTime = nodeEquivalences.getConnectionTime(fromNODESrecordID, toNODESrecordID);
		else
		{
			const size_t walkingDistance = leavePoint.distance( arrivePoint );
			if( walkingDistance <= directWalkingDistanceMax )
				directWalkingTime = walkingDistanceToTime(walkingDistance);
			else
				directWalkingTime = infeasibleWalkingTime;
		}
		resultCollection.setDirectWalkingTime( directWalkingTime );
	}

	// Find the closest locations to leave from and arrive to.
	QTree::PointID	closestLeave[closestPointMax], closestArrive[closestPointMax];
	size_t	closestLeaveNum = 0, closestArriveNum = 0;
	if( !driverPlan )
	{
		// Do a proximity search to find the nearest stops.
		closestLeaveNum  = service->qtree.findNearestN(  leavePoint, closestLeave,  closestPointMax );
		closestArriveNum = service->qtree.findNearestN( arrivePoint, closestArrive, closestPointMax );

		// Filter out the locations that are too far or cross boundaries.
		filterPoints( leavePoint, closestLeave, closestLeaveNum );
		filterPoints( arrivePoint, closestArrive, closestArriveNum );
	}
	else
	{
		// Only consider the given nodes and its equivalences.
		closestLeave[closestLeaveNum++].set( leavePoint, (ident_t)service->locations.find(fromNODESrecordID).data() );
		CISnodeEquivalences::IDIDHash::iterator n = nodeEquivalences.nodes.find(fromNODESrecordID);
		if( n != nodeEquivalences.nodes.end() )
		{
			CISnodeEquivalences::IDHash::iterator	e;
			ForAllInCollection( *n.data(), e )
			{
				CISlocationCollection::iterator locFound = service->locations.find(*e);
				if( locFound != service->locations.end() )
				{
					if( closestLeaveNum >= closestPointMax )
						break;
					closestLeave[closestLeaveNum++].set( locFound.data()->getPoint(), (ident_t)locFound.data() );
				}
			}
		}

		closestArrive[closestArriveNum++].set( arrivePoint, (ident_t)service->locations.find(toNODESrecordID).data() );
		n = nodeEquivalences.nodes.find(toNODESrecordID);
		if( n != nodeEquivalences.nodes.end() )
		{
			CISnodeEquivalences::IDHash::iterator	e;
			ForAllInCollection( *n.data(), e )
			{
				CISlocationCollection::iterator locFound = service->locations.find(*e);
				if( locFound != service->locations.end() )
				{
					if( closestArriveNum >= closestPointMax )
						break;
					closestArrive[closestArriveNum++].set( locFound.data()->getPoint(), (ident_t)locFound.data() );
				}
			}
		}
	}

	do // Not really a loop, just allows me to escape to report errors without gotos.
	{
		// Set the location flags of the leave and arrive points.
		// Also set the walking time to and from the point.
		for( i = 0; i < closestLeaveNum; i++ )
		{
			CISlocation	*loc = (CISlocation *)closestLeave[i].id;
			loc->isPotentialLeave = true;
			loc->leaveWalkingTime = driverPlan
				? nodeEquivalences.getConnectionTime( loc->getID(), fromNODESrecordID )
				: walkingDistanceToTime(leavePoint.distance(closestLeave[i].point));
		}
		for( i = 0; i < closestArriveNum; i++ )
		{
			CISlocation	*loc = (CISlocation *)closestArrive[i].id;
			loc->isPotentialArrive = true;
			loc->arriveWalkingTime = driverPlan
				? nodeEquivalences.getConnectionTime( loc->getID(), toNODESrecordID )
				: walkingDistanceToTime(arrivePoint.distance(closestArrive[i].point));
		}

		// Ensure there are no points that are both potential leave and potential arrive.
		if( closestArriveNum > 1 && closestLeaveNum > 1 )
		{
			HashFI< CISlocation *, CISlocation * > arrive, leave;

			// Allocate all arrive locations on the basis of proximity.
			for( i = 0; i < closestArriveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestArrive[i].id;
				if( loc->isPotentialLeave &&
					loc->leaveWalkingTime < loc->arriveWalkingTime )
					leave.insertKey( loc );
				else
					arrive.insertKey( loc );
			}
			// Allocate all leave locations on the basis of proximity.
			for( i = 0; i < closestLeaveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestLeave[i].id;
				if( loc->isPotentialArrive &&
					loc->arriveWalkingTime <= loc->leaveWalkingTime )
					arrive.insertKey( loc );
				else
					leave.insertKey( loc );
			}
			// Ensure that there is at least one leave or arrive location.
			if( arrive.empty() )
			{
				// Keep the closest arrive location.
				CISlocation	*locBest = NULL;
				for( i = 0; i < closestArriveNum; i++ )
				{
					CISlocation	*loc = (CISlocation *)closestArrive[i].id;
					if( locBest == NULL || locBest->arriveWalkingTime > loc->arriveWalkingTime )
						locBest = loc;
				}
				leave.remove( locBest );
				arrive.insertKey( locBest );
			}
			else if( leave.empty() )
			{
				// Keep the closest leave location.
				CISlocation	*locBest = NULL;
				for( i = 0; i < closestLeaveNum; i++ )
				{
					CISlocation	*loc = (CISlocation *)closestLeave[i].id;
					if( locBest == NULL || locBest->leaveWalkingTime > loc->leaveWalkingTime )
						locBest = loc;
				}
				arrive.remove( locBest );
				leave.insertKey( locBest );
			}

			// Make the closest leave and arrive arrays consistent with the location allocations.
			register size_t j;
			for( i = j = 0; i < closestLeaveNum; ++i )
			{
				CISlocation	*loc = (CISlocation *)closestLeave[i].id;
				if( leave.contains(loc) )
					closestLeave[j++] = closestLeave[i];
				else
					loc->isPotentialLeave  = false;
			}
			closestLeaveNum = j;

			for( i = j = 0; i < closestArriveNum; ++i )
			{
				CISlocation	*loc = (CISlocation *)closestArrive[i].id;
				if( arrive.contains(loc) )
					closestArrive[j++] = closestArrive[i];
				else
					loc->isPotentialArrive  = false;
			}
			closestArriveNum = j;
		}

		// Check for errors regarding leave and arrive stops.
		if( closestLeaveNum == 0 )
			resultCollection.setStatus( noLeaveStop );
		if( closestArriveNum == 0 )
			resultCollection.setStatus( (resultCollection[0]->status == noLeaveStop ? noLeaveArriveStop : noArriveStop) );
		if( closestLeaveNum == 0 || closestArriveNum == 0 )
			break; // Error

		if( isLeave )
		{
			// Find the earliest wait events at the leave locations.
			for( i = 0; i < closestLeaveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestLeave[i].id;
				CIScEventSet::iterator found;

				if( !leaveAsEarlyAsPossible )
				{
					// Find the first event at or after the expected arrival time at the stop.
					CIScEvent	search( leaveTime + loc->leaveWalkingTime );

					found = loc->shelter.lower_bound(&search);
					while( found != loc->shelter.end() && (*found)->t < search.t )
						++found;
				}
				else // leave as early as possible
				{
					// If we are leaving as early as possible, take the first leave event.
					found = loc->shelter.begin();
				}

				// If no event meets our criteria, check the next leave location.
				if( found == loc->shelter.end() )
					continue;

				// Add all events up to walkingUtility(walkingTime) after the first event to the heap.
				// Make sure we penalize the walking and waiting time properly.
				CIScEvent	*efound = *found;
				CIScEvent	*ecur = efound;
				const tod_t dtMax = leaveAsEarlyAsPossible ? 48 * 60 * 60 : (tod_t)walkingUtility(directWalkingTime);
				do
				{
					const tod_t waitingTime = ecur->t - leaveTime - loc->leaveWalkingTime;
					ecur->d = (utility_t)waitingTime + walkingUtility(loc->leaveWalkingTime);
					ecur->tCur = 0;
					insertInitial( ecur );

					if( ++found == loc->shelter.end() )
						break;
					ecur = *found;
				} while( ecur->t - efound->t <= dtMax );
			}
			if( N == 0 )
			{
				resultCollection.setStatus( leaveTooLate );
				break; // Error.
			}
		}
		else // isArrive
		{
			// Find the latest wait events at the arrive locations.
			for( i = 0; i < closestArriveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestArrive[i].id;
				CIScEventSet::iterator found;

				if( loc->shelter.begin() == loc->shelter.end() )
					continue;

				// Find the first event at or before the expected leave time at the stop.
				if( !arriveAsLateAsPossible )
				{
					if( (*loc->shelter.begin())->t > arriveTime - loc->arriveWalkingTime )
						continue;

					const tod_t tSearch = (tod_t)(arriveTime - walkingUtility(directWalkingTime));
					if( tSearch <= (*loc->shelter.begin())->t )
						found = loc->shelter.begin();
					else
					{
						CIScEvent	search( tSearch );
						found = loc->shelter.lower_bound( &search );
						if( found == loc->shelter.end() )
							continue;
						while( found != loc->shelter.begin() && (*found)->t > search.t )
							--found;
					}
				}
				else // arrive as late as possible.
				{
					// This trip is to arrive as late as possible.
					// Get the last wait event we can find.
					if( loc->shelter.begin() == loc->shelter.end() )
						continue;
					const tod_t tSearch = (tod_t)((*loc->shelter.rbegin())->t - walkingUtility(directWalkingTime));
					if( tSearch <= (*loc->shelter.begin())->t )
						found = loc->shelter.begin();
					else
					{
						CIScEvent	search( tSearch );
						found = loc->shelter.lower_bound( &search );
						if( found == loc->shelter.end() )
							continue;
						while( found != loc->shelter.begin() && (*found)->t > search.t )
							--found;
					}
				}

				if( found == loc->shelter.end() )
					continue;

				// Add all events to the heap.
				// Make sure we penalize the walking and waiting time properly.
				CIScEvent	*efound = *found;
				CIScEvent	*ecur = efound;
				do
				{
					const tod_t	waitingTime = arriveTime - ecur->t - loc->arriveWalkingTime;
					ecur->d = (utility_t)waitingTime + walkingUtility(loc->arriveWalkingTime);
					ecur->tCur = 0;
					insertInitial( ecur );

					if( ++found == loc->shelter.end() )
						break;
					ecur = *found;
				} while( ecur->t + loc->arriveWalkingTime <= arriveTime );
			}
			if( N == 0 )
			{
				resultCollection.setStatus( leaveTooEarly );
				break; // Error
			}
		}

#ifdef PRINT_SP_SEARCH
		{
			size_t i;

			ofs << "Potential leave locations:\n";
			for( i = 0; i < closestLeaveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestLeave[i].id;
				ofs << "\t'" << loc->getName() << "'\n";
			}

			ofs << "Potential arrive locations:\n";
			for( i = 0; i < closestArriveNum; i++ )
			{
				CISlocation	*loc = (CISlocation *)closestArrive[i].id;
				ofs << "\t'" << loc->getName() << "'\n";
			}
		}
#endif // PRINT_SP_SEARCH

		// Find the shortest path.
		if( isLeave )
			spLeave( arcFlags );
		else
			spArrive( arcFlags );

		// If the arrive or leave destination was not reached, then it is too late/early to get there.
		if( solutionHeap.size() == 0 )
		{
			if( isLeave )
				resultCollection.setStatus( leaveTooLate );
			else // isArrive
				resultCollection.setStatus( leaveTooEarly );
			break; // Error
		}

		// Save the paths in order of increasing cost.
		// Remember - the most costly paths will be first off the solutionHeap.
		i = solutionHeap.size();
		do
		{
			CISsolution *s = solutionHeap.extractTop();
			Ref<CISresult>	r = resultCollection.results[--i];
			register CISarc *a;
			if( isLeave )
			{
				for( a = s->parentFinal->arcToParent; a != NULL; a = a->from->arcToParent )
					r->arcList.insert( a );
			}
			else
			{
				for( a = s->parentFinal->arcToParent; a != NULL; a = a->to->arcToParent )
					r->arcList.insert( a );
			}
			r->dFinal = s->dFinal;

			// Compute the metrics for the trip plan.
			r->init();

			delete s;
		} while( i != 0 );
	} while(0);	// break-without-goto loop.

	if( isArrive )
	{
		// Check if there are any trips that pay multiple fares.
		const size_t iMac = solutionHeap.size();
		HashFI< CISresult *, size_t > multiFare;
		for( i = 0; i < iMac; ++i )
			if( resultCollection.results[i]->getNumFares() > 1 )
				multiFare.insert( resultCollection.results[i].data(), i );
		if( !multiFare.empty() )
		{
			// Attempt to find a trip that takes a little more time but
			// avoids an expired transfer.
		}
	}

	// Cleanup
	// Reset the shortest path search.
	spReset();

	// Reset the location flags.
	for( i = 0; i < closestLeaveNum; i++ )
		((CISlocation *)(closestLeave[i].id ))->isPotentialLeave  = 0;
	for( i = 0; i < closestArriveNum; i++ )
		((CISlocation *)(closestArrive[i].id))->isPotentialArrive = 0;

	// Return the general result.
	resultCollection.setValidMax();
	return resultCollection[0]->status;
}

CIS::PlanStatus	CIS::plan( const ident_t fromNodeID, const ident_t toNodeID, const tod_t t, const int planLeave,
						  const ident_t idService,
							CISresultCollection &resultCollection /* return */ )
{
	// Find the locations of the ids.
	CISlocation	*fromLoc = locations.find(fromNodeID).data();
	CISlocation	*toLoc = locations.find(toNodeID).data();

	// Call the trip planner.
	PlanStatus ret = plan( fromLoc->getName(), Point(fromLoc->getLongitude(), fromLoc->getLatitude()), fromNodeID,
							toLoc->getName(), Point(toLoc->getLongitude(), toLoc->getLatitude()), toNodeID,
							t, planLeave, idService, CISvehicleFlags(0,0,0), resultCollection );

	return ret;
}

class trimString
{
public:
	trimString( const char *aS ) : szIn(aS), szOut(NULL) {}
	~trimString() { delete [] szOut; }
	operator const char *();
private:
	const	char	*szIn;
	char			*szOut;
};

trimString::operator const char *()
{
	if( szIn == NULL || *szIn == 0 )
		szOut = NULL;
	else
	{
		szOut = new char[strlen(szIn)*2+1];
		register const char *p = szIn;
		while( isspace(*p) )
			p++;

		register char *q = szOut;
		while( *p != 0 )
		{
			if( *p == '\'' )	// Escape all backquotes.
				*q++ = '\\';
			*q++ = *p++;
		}

		*q = 0;
		while( --q >= szOut && isspace(*q) )
			*q = 0;
	}
	return szOut ? szOut : "";
}

char	*CIS::compassDirection( char *szDirection, size_t, const char *szFormat, const QTree::Point &p1, const QTree::Point &p2 )
{
	// Only commit to a direction if we are some distance away.
	// This avoids accuracy problems when the points are close together.
	if( p1.distance(p2) < 40 )
		*szDirection = 0;
	else
		sprintf( szDirection, szFormat, Distance::compassDirection(p1.x, p1.y, p2.x, p2.y) );
	return szDirection;
}

static	void	AddPoint( CISpoint **pCISpointArray, int *pNumCISpoints,
						 const tod_t t, const CIS::Point point,
						 const long routeID, const char *routeName = NULL )
{
	if( pCISpointArray != NULL )
	{
		register CISpoint *p = &((*pCISpointArray)[(*pNumCISpoints)++]);
		p->t = t;
		p->longitude = point.longitude;
		p->latitude = point.latitude;
		p->routeID = routeID;
		p->routeName = routeName;
	}
}

static	void	AddTravel( CISpoint **pCISpointArray, int *pNumCISpoints,
						  const tod_t fromTime,	const CIS::Point from,
						  const tod_t toTime,		const CIS::Point to,
						  const long routeID, const char *routeName )
{
	if( pCISpointArray != NULL )
	{
		// Check if we need to add the from point.
		register CISpoint *p, *b;
		p = &((*pCISpointArray)[(*pNumCISpoints)++]);
		b = p - 1;

		if( *pNumCISpoints == 1 ||
			b->t != fromTime ||
			b->longitude != from.longitude ||
			b->latitude != from.latitude ||
			b->routeID != routeID )
		{
			p->t = fromTime;
			p->longitude = from.longitude;
			p->latitude = from.latitude;
			p->routeID = routeID;
			p->routeName = routeName;
			p = &((*pCISpointArray)[(*pNumCISpoints)++]);
		}

		p->t = toTime;
		p->longitude = to.longitude;
		p->latitude = to.latitude;
		p->routeID = routeID;
		p->routeName = routeName;
	}
}

static	void	CreateWalk( CISpoint **pCISpointArray, int *pNumCISpoints,
						   const tod_t fromTime,	const CIS::Point from,
						   const tod_t toTime,		const CIS::Point to )
{
	if( pCISpointArray != NULL )
	{
		*pCISpointArray = (CISpoint *)malloc( sizeof(CISpoint) * 2 );
		*pNumCISpoints = 0;
		
		// Check if we need to add the from point.
		register CISpoint *p;
		p = &((*pCISpointArray)[(*pNumCISpoints)++]);
		p->t = fromTime;
		p->latitude = from.latitude;
		p->longitude = from.longitude;
		p->routeID = -1;
		
		p = &((*pCISpointArray)[(*pNumCISpoints)++]);
		p->t = toTime;
		p->latitude = to.latitude;
		p->longitude = to.longitude;
		p->routeID = -1;
	}
}

CISservice::~CISservice()
{
	deleteHashContents( transferGroups );
	deleteHashContents( locations );
}

void	CIS::initBoundaries( istream &isMIF )
{
	ParseMIF	pm( isMIF );
	register ParseMIF::Line	*line;
	for( line = pm.getLHead(); line != NULL; line = line->next )
	{
		int	x1, y1, x2, y2;
		Distance::CoordFromLongLat( x1, y1, line->long1, line->lat1 );
		Distance::CoordFromLongLat( x2, y2, line->long2, line->lat2 );
		boundaries.addSegment( Trapezoidal::Point(x1,y1), Trapezoidal::Point(x2,y2) );
	}
}

void CIS::setDriverParameters()
{
	setDriverPlan(			true );
	setWalkingFactor(		6.60 );	// Increased to fix a walking from stop problem with CTRAN.
//	setWalkingFactor(		1.20 );	// Regular value
	setTransferWaitFactor(	1.10 );
	setTravelFactor(		1.15 );
	setTransferTimeMin(		0 );
//	setTransferPenalty(		10 * 60);
	setTransferPenalty(		5 * 60);
	setAdditionalFarePenalty( 0 );
	setDirectWalkingDistanceMax( 300 );
	setEntryTime(			0 );
	setExitTime(			0 );
	setHasPass(				false );
}

CIS::CIS() : os(0), serviceCalendar(NULL)
{
  fTripPlanLog = false;
//  fTripPlanLog = true;


	// Initialize the heap.
	NMax = 0;
	heap = NULL;

	// Initalize the number of solutions.
	solutionMax = 0;

	// Intialize the trip preferences so we don't crash (this may get changed later).
	transferDistance = 60;	// meters between locations to be considered in the same transfer group.
	transferTimeMin = 60*1;	// minimum seconds required for a transfer between locations.
	transferPenalty = 60*5; // minimum disutility for changing vehicles on a trip.

	additionalFarePenalty = 60*10; // disutility for paying an additional fare.

	entryTime = 60;			// seconds to go from the shelter to the bus.
	exitTime = 60;			// seconds to go from the bus to the shelter.

	travelFactor = 1.1;		// penalty for actually riding a vehicle.
	transferWaitFactor = 1.2; // penalty for waiting at a transfer point in the system.
	walkingFactor = 1.6;	  // penalty for walking time.

//	walkingSpeed = 1.0;		// slow walking speed (meters/second).
	walkingSpeed = 1.5;		// faster walking speed (meters/second).
	walkingRadius = (int)(30 * 60 * walkingSpeed);	// maximum allowed meters of walking to a stop (30 minutes of walking).

	hasPass = 0;			// has pass flag.

	directWalkingDistanceMax = UINT_MAX;	// maximum direct walking distance.

	// Override everything with what the drivers are doing.
	setDriverParameters();
	driverPlan = 0;			// not planning for driver relief points.
	setDirectWalkingDistanceMax( UINT_MAX );	// allow infinite walking for non-drivers.
	saveParametersAsDefault();	// save the default parameters so we can restore them later.

#ifdef PRINT_SP_SEARCH
		os = new ofstream("addTravel.txt");
#endif 
}

CIS::~CIS()
{
	deleteHashContents( locations );
	deleteHashContents( routes );
	deleteHashContents( directions );
	deleteHashContents( services );
	deleteHashContents( vehicles );
	delete [] heap;

  if( os )
    delete os;

  delete serviceCalendar;
}

char	*CISresult::getTimestamp( char *szTimestamp, const size_t tsSize )
{
	// Create a timestamp.
	time_t timeNow = time(NULL);
	struct tm tmNow;
	memcpy( &tmNow, localtime(&timeNow), sizeof(tmNow) );
#ifdef AMPM
	const int isAM = (tmNow.tm_hour < 12);
	if( tmNow.tm_hour >= 13 )
		tmNow.tm_hour -= 12;
	else if( tmNow.tm_hour == 0 )
		tmNow.tm_hour = 12;
	sprintf( &szTimestamp[strftime(szTimestamp, tsSize, "%B %d, %Y", &tmNow)], " %d:%02d%s",
		tmNow.tm_hour, tmNow.tm_min, isAM ? "am" : "pm" );
#else
	sprintf( &szTimestamp[strftime(szTimestamp, tsSize, "%B %d, %Y", &tmNow)], " %d:%02d",
		tmNow.tm_hour, tmNow.tm_min );
#endif

	return szTimestamp;
}

void	CISresult::init()
{
	// Convert lat/long to meters to preserve sanity.
	int	x, y;
	Distance::CoordFromLongLat( x, y, leave.longitude, leave.latitude );
	QTree::Point leavePoint( x, y );
	
	Distance::CoordFromLongLat( x, y, arrive.longitude, arrive.latitude );
	QTree::Point arrivePoint( x, y );
		
	CISarcList::iterator a;

	// Reverse the order of the arcs if necessary.
	if( !arcList.empty() && (*arcList.begin())->from->t >= (*arcList.last())->to->t )
		arcList.reverse();

	// Check if the trip plan includes a bus trip somewhere.
	bool	hasTravelArc = false;
	ForAllInCollection( arcList, a )
	{
		if( (*a)->isTravel() )
		{
			hasTravelArc = true;
			break;
		}
	}

	// If we are leaving as late or early as possible, adjust the total utility
	// to be from leave to arrival.
	if( type == CIS::leaveEarliest || type == CIS::leaveLatest )
	{
		if( hasTravelArc )
		{
			dFinal = (float)cis->entryTime;
			CISarcList::iterator aStart = arcList.end(), aEnd;
			ForAllInCollection( arcList, a )
			{
				if( (*a)->isTravel() )
				{
					if( aStart == arcList.end() )
						aStart = a;
					aEnd = a;
				}
				dFinal += cis->elapsedUtility(*a);
			}
			dFinal += driverPlan
				? cis->nodeEquivalences.getConnectionTime( (*aStart)->from->location->getID(), fromNODESrecordID )
				: cis->walkingUtility(cis->walkingDistanceToTime((*aStart)->from->location->getPoint().distance(leavePoint)));
			dFinal += driverPlan
				? cis->nodeEquivalences.getConnectionTime( (*aEnd)->to->location->getID(), toNODESrecordID )
				: cis->walkingUtility(cis->walkingDistanceToTime((*aEnd)->to->location->getPoint().distance(arrivePoint)));
		}
		else
			dFinal = 7.0 * 24 * 60 * 60;	// If there is no path, ensure we suggest the direct walking alternative.
	}
	
	// Compute the trip metrics:
	// numTransfers
	// walkingDistance
	// walkingTime
	// dwellTime
	// travelTime

	// Check if it would be better to walk.
	if( !hasTravelArc || cis->walkingUtility(directWalkingTime) < dFinal )
	{
		// Update the trip metrics.
		status = (directWalkingTime == CIS::infeasibleWalkingTime ? CIS::infeasible : CIS::betterToWalk);
		numTransfers = 0;
		walkingDistance = (size_t)leavePoint.distance( arrivePoint );
		travelTime = walkingTime = cis->walkingDistanceToTime(walkingDistance);
		
		// No need to keep the path since we are walking.
		arcList.clear();
		
		dFinal = cis->walkingUtility(directWalkingTime);
		
		// No need to pay anything if we are walking.
		numFares = 0;
		
		// Compute the arrive and leave times based on the walking time.
		switch( type )
		{
		case CIS::leaveAt:
			leaveTime = t;
			arriveTime = t + directWalkingTime;
			break;
		case CIS::arriveAt:
			leaveTime = t - directWalkingTime;
			arriveTime = t;
			break;
		case CIS::leaveEarliest:
			leaveTime = 0;
			arriveTime = directWalkingTime;
			break;
		case CIS::leaveLatest:
			arriveTime = 48 * 60 * 60;
			leaveTime = arriveTime - directWalkingTime;
			break;
		}
		dwellTime = 0;	// Since the customer is not taking the bus, dwell time is 0.
	}
	else
	{
		numTransfers = 0;
		if( arcList.empty() )
		{
			travelTime = 0;
			dwellTime = 0;
		}
		else
		{
			CISarc	*firstTrip = NULL, *lastTrip = NULL;

			// Account for walking from the leave location.
			const size_t leaveWalkingDistance = driverPlan ? 0 : (size_t)((*arcList.begin())->from->location->getPoint().distance(leavePoint));
			walkingDistance = leaveWalkingDistance;
			
			tod_t	tCur = 0;
			ForAllInCollection( arcList, a )
			{
				tCur += (*a)->getElapsed();

				if( (*a)->isTravel() )
				{
					if( firstTrip == NULL )
						firstTrip = *a;

					if( lastTrip )
					{
						if( isTransfer( lastTrip->idTrip, lastTrip->route->getID(), lastTrip->idBlock,
										(*a)->idTrip,     (*a)->route->getID(),     (*a)->idBlock) )
						{
							if( tCur > service->transferTimeMax )
							{
								numFares++;
								tCur = 0;
							}
							numTransfers++;
						}
					}
					lastTrip = *a;
				}
				else
				{
					// Account for walking between transfer points.
					if( !(*a)->isTravel() &&
						!cis->nodeEquivalences.isEquivalent((*a)->from->location->getID(), (*a)->to->location->getID()))
						walkingDistance += (size_t)((*a)->from->location->getPoint().distance( (*a)->to->location->getPoint() ));
				}

			}

			// Account for when the trip plan is just a transfer arc.
			if( firstTrip == NULL )
				firstTrip = lastTrip = *arcList.begin();

			// Account for walking to the arrive location.
			const size_t arriveWalkingDistance = driverPlan ? 0 : (size_t)(lastTrip->to->location->getPoint().distance(arrivePoint));
			walkingDistance += arriveWalkingDistance;
			travelTime = lastTrip->to->t - firstTrip->from->t;
			travelTime += cis->walkingDistanceToTime(walkingDistance);

			// Account for the connection time to/from the arrive/leave locations.
			const tod_t leaveConnectionTime = driverPlan
				? cis->nodeEquivalences.getConnectionTime(fromNODESrecordID, firstTrip->from->location->getID())
				: 0;
			const tod_t arriveConnectionTime = driverPlan
				? cis->nodeEquivalences.getConnectionTime(toNODESrecordID, lastTrip->to->location->getID())
				: 0;
			travelTime += leaveConnectionTime + arriveConnectionTime;
			
			// Compute actual arrive and leave times.
			leaveTime = firstTrip->from->t - (driverPlan
												? leaveConnectionTime
												: cis->walkingDistanceToTime(leaveWalkingDistance));
			arriveTime = lastTrip->to->t + (driverPlan
												? arriveConnectionTime
												: cis->walkingDistanceToTime(arriveWalkingDistance));
			
			// Compute the total walking time based on the walking distance.
			walkingTime = cis->walkingDistanceToTime(walkingDistance);
			
			// Compute the dwell time based on the type of trip plan.
			switch( type )
			{
			case CIS::leaveAt:	dwellTime = leaveTime - t;	break;
			case CIS::arriveAt:	dwellTime = t - arriveTime;	break;
			default:
				dwellTime = 0;
				break;
			}
		}
	}
}

void	formatTime::setFormat( const CISmessageMap &mm )
{
	// Get the time format.
	TString value(trimString(mm.find("!TimeFormat")));
	format = AMPM;
	if(		 value == "APX" )		format = APX;
	else if( value == "Military" )	format = Military;
}

inline const char	*CISresult::getDirection( const char *szDirection, const CISmessageMap &mm ) const
{
	const char *sz = mm.find(szDirection);
	return sz != NULL ? sz : &szDirection[1];
}

void	CISresult::setDistanceFactor( const CISmessageMap &mm )
{
	// Get the distance units.
	distanceFactor = 1.0;	// Default is meters.
	decimals = 0;			// Default is no decimals.
	distanceUnit = "m";		// Default unit.
	TString	value(trimString(mm.find("!DistanceUnits")));
	if(		 value == "Km" || value == "km" )			{ distanceUnit = "km"; decimals = 3; distanceFactor = 1.0 / 1000.0; }
	else if( value == "Feet" || value == "ft" )			{ distanceUnit = "ft"; decimals = 0; distanceFactor = 39.37 / 12.0; }
	else if( value == "Yards" || value == "yds" )		{ distanceUnit = "yards"; decimals = 0; distanceFactor = 39.37 / 36.0; }
	else if( value == "Miles" || value == "miles" )		{ distanceUnit = "miles"; decimals = 3; distanceFactor = 0.6214 / 1000.0; }
}

const char *CISresult::formatDistance( const size_t distance ) const
{
	static	char	szDistance[16];
	if( decimals == 0 )
		sprintf( szDistance, "%d", (int)(distance * distanceFactor + 0.5) );
	else
		sprintf( szDistance, "%.*f", decimals, (distance * distanceFactor) );
	return &szDistance[0];
}

inline tod_t	secondsToMinutes( const tod_t s ) { return s / 60; }	// Do not round to the nearest minute (for consistency).

char	*CISresult::format( char *szPlan, size_t szPlanSize, CISpoint **pCISpointArray, int *pNumCISpoints, const CISmessageMap &mm )
{
	std::ofstream tripPlanLog;
	if( fTripPlanLog )
		tripPlanLog.open("TripPlanLog.csv");

	if( pCISpointArray != NULL )
	{
		*pCISpointArray = NULL;
		*pNumCISpoints = 0;
	}

	formatTime::setFormat( mm );
	setDistanceFactor( mm );

	char	szDirection[64];
	ostrstream	os(szPlan, szPlanSize);	// Yes, I really want to use ostrstream here with the specified buffers.
	const char nl = '\n';
    const char nlLog = '\n';
	const char tab = '\t';
	const char sep = ',';

	// Convert lat/long to meters to preserve sanity.
	int	x, y;
	Distance::CoordFromLongLat( x, y, leave.longitude, leave.latitude );
	QTree::Point leavePoint( x, y );

	Distance::CoordFromLongLat( x, y, arrive.longitude, arrive.latitude );
	QTree::Point arrivePoint( x, y );

	char	szNow[64];
	getTimestamp( szNow, sizeof(szNow) );

	// Create a header.
	os << "<?xml version=\"1.0\"?>" << nl;
	os <<  "<TRIPPLAN" << nl
			<<	tab << "timestamp=" << qt(szNow) << nl
//			<<	tab << "format=" << qt("Text") << nl
//			<<	tab << "property=" << qt(szProperty) << nl
			<<	tab << "language=" << qt("English") << nl
			<<	tab << "distanceUnit=" << qt(distanceUnit.getStr())
			<< '>' << nl;
	os << "<TRIP" << nl
			<< tab << "from=" << qt(trimString(sLeave.getStr())) << nl
			<< tab << "to=" << qt(trimString(sArrive.getStr())) << nl
//			<< tab << "travelDate=" << qt(szTravelDate) << nl
			<< tab << "service=" << qt(trimString(service->getName())) << nl
			<< '>' << nl;

	os << "<PROLOG>" << nl;

	if( fTripPlanLog )
	{
		tripPlanLog << "TimeStamp"		<< sep << szNow << nlLog;
		tripPlanLog << "DistanceUnit"	<< sep << distanceUnit.getStr() << nlLog;
		tripPlanLog << "Service"		<< sep << service->getName() << nlLog;
		tripPlanLog << "From"			<< sep << sLeave.getStr() << nlLog;
		tripPlanLog << "To"				<< sep << sArrive.getStr() << nlLog;
		tripPlanLog << nlLog;
	}

	int	 leaveFlag;
	switch( type )
	{
	case CIS::leaveEarliest:
		os << tab << "<leaveEarliest/>" << nl;
		leaveFlag = 1;
		if( fTripPlanLog ) tripPlanLog << "Leave"		<< sep << "earliest" << nlLog;
		break;
	case CIS::leaveLatest:
		os << tab << "<leaveLatest/>" << nl;
		leaveFlag = 0;
		if( fTripPlanLog ) tripPlanLog << "Leave"		<< sep << "latest" << nlLog;
		break;
	case CIS::leaveAt:
		os << tab << "<leaveAt time=" << qt(formatTime(t)) << "/>" << nlLog;
		leaveFlag = 1;
		if( fTripPlanLog ) tripPlanLog << "Leave"		<< sep << "Time=" << tFormat(t) << nlLog;
		break;
	case CIS::arriveAt:
		os << tab << "<arriveAt time=" << qt(formatTime(t)) << "/>" << nlLog;
		leaveFlag = 0;
		if( fTripPlanLog ) tripPlanLog << "Arrive"		<< sep << "Time=" << tFormat(t) << nlLog;
		break;
	}
	os << tab << (hasPass ? "<hasPass/>" : "<hasNoPass/>") << nl;
	os << "</PROLOG>" << nl;

	os << "<INSTRUCTIONS>" << nl;

	do // not really a loop - just give me a chance to break if I have errors.
	{
		CISarcList::iterator a;

		// Check for errors.
		if( fTripPlanLog ) tripPlanLog << "Status" << sep;
		switch( status )
		{
		case CIS::noLeaveStop:
			os << tab << "<noFromStop/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "noFromStop" << nlLog;
			break;
		case CIS::noArriveStop:
			os << tab << "<noToStop/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "noToStop" << nlLog;
			break;
		case CIS::noLeaveArriveStop:
			os << tab << "<noFromStop/>" << nl;
			os << tab << "<noToStop/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "noFromStop/noToStop" << nlLog;
			break;
		case CIS::leaveTooEarly:
			os << tab << "<leaveTooEarly/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "leaveTooEarly" << nlLog;
			break;
		case CIS::leaveTooLate:
			os << tab << "<leaveTooLate/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "leaveTooLate" << nlLog;
			break;
		case CIS::infeasible:
			os << tab << "<infeasible/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "infeasible" << nlLog;
			break;
		}
		// Check if the trip plan actually goes somewhere.
		if( status == CIS::good )
		{
			for( a = arcList.begin(); a != arcList.end(); ++a )
				if( (*a)->isTravel() )
					break;
			if( a == arcList.end() )
				status = CIS::betterToWalk;
		}
		if( status == CIS::betterToWalk )
		{
			cis->compassDirection(szDirection, sizeof(szDirection), "!%s", leavePoint, arrivePoint);
			os << tab << "<walkDirectly"
				<< " direction=" << qt(trimString(getDirection(szDirection, mm)))
				<< "/>" << nl;
			if( fTripPlanLog ) tripPlanLog << "Walk Directly" << nlLog;
			CreateWalk( pCISpointArray, pNumCISpoints,
				leaveFlag ? t : t - directWalkingTime, leave,
				leaveFlag ? t + directWalkingTime : t, arrive );
			if( fTripPlanLog ) tripPlanLog << "betterToWalk" << nlLog;
		}
		if( status != CIS::good )
			break;
		else
			if( fTripPlanLog ) tripPlanLog << "good" << nlLog;

		if( pCISpointArray != NULL )
		{
			// Count the number of travel arcs.
			for( a = arcList.begin(); a != arcList.end(); ++a )
				if( (*a)->isTravel() )
					(*pNumCISpoints)++;
			*pNumCISpoints = (*pNumCISpoints) * 2 + 2;	// Add extra to account for transfers.
			*pCISpointArray = (CISpoint *)malloc( sizeof(CISpoint) * (*pNumCISpoints) );
			*pNumCISpoints = 0;
		}

		if( fTripPlanLog )
		{
			tripPlanLog << "CumTime" << sep << "ElapsedTime" << sep << "Type"
			<< sep << "From"  << sep << "FromTime"
			<< sep << "To" << sep << "ToTime"
			<< sep << "RouteID" << sep << "RouteName" << sep << "TripID" << sep << "BlockID" << nlLog;
		}

		// Report the path.
		tod_t	tCur = 0;
		size_t	curTransfer = numTransfers;
		CISarc	*lastTravel = NULL, *firstTravel = NULL;
		for( a = arcList.begin(); a != arcList.end(); ++a )
		{
			if( fTripPlanLog )
			{
				if( (*a)->isTravel() )
				{
					tripPlanLog << tFormat(tCur) << sep << tFormat((*a)->getElapsed()) << sep << "Travel"
						<< sep << (*a)->from->location->getName() << sep << tFormat((*a)->from->t)
						<< sep << (*a)->to->location->getName() << sep << tFormat((*a)->to->t)
						<< sep << (*a)->route->getID() << sep << (*a)->route->getName() << sep << (*a)->idTrip << sep << (*a)->idBlock << nlLog;
				}
				else
				{
					tripPlanLog << tFormat(tCur) << sep << tFormat((*a)->getElapsed()) << sep << "Wait"
						<< sep << (*a)->from->location->getName() << sep << tFormat((*a)->from->t)
						<< sep << (*a)->to->location->getName() << sep << tFormat((*a)->to->t)
						<< sep << sep << sep << sep << nlLog;
				}
			}

			tCur += (*a)->getElapsed();
			if( !(*a)->isTravel() )
				continue;

			if( firstTravel == NULL )
				firstTravel = *a;

      // Check if this travel is on the same vehicle.
			if( lastTravel != NULL &&
					!isTransfer(lastTravel->idTrip, lastTravel->route->getID(), lastTravel->idBlock,
                      (*a)->idTrip, (*a)->route->getID(), (*a)->idBlock) )
			{
				AddTravel( pCISpointArray, pNumCISpoints,
					(*a)->from->t, CIS::Point((*a)->from->location->longitude, (*a)->from->location->latitude),
					(*a)->to->t, CIS::Point((*a)->to->location->longitude, (*a)->to->location->latitude),
					(*a)->route->getID(), (*a)->route->getName() );
				lastTravel = *a;
				continue;
			}

			if( lastTravel == NULL )
			{
				// We just left.
				const QTree::Point p((*a)->from->location->getPoint());
				const tod_t etd = (*a)->from->t - (driverPlan
					? cis->nodeEquivalences.getConnectionTime(fromNODESrecordID, firstTravel->from->location->getID())
					: cis->walkingDistanceToTime(p.distance(leavePoint)));
				os << tab << "<leave time=" << qt(formatTime(etd)) << "/>" << nl;
				cis->compassDirection(szDirection, sizeof(szDirection), "!%s", leavePoint, (*a)->from->location->getPoint());
				if( etd != (*a)->from->t && szDirection[0] != 0 ) // No direction will be given if we are at the stop already.
				{
					os << tab << "<walkToStop"
							<< " time=" << qt(formatTime(etd))
							<< " direction=" << qt(trimString(getDirection(szDirection, mm)))
							<< " stop=" << qt(trimString((*a)->from->location->getName()))
							<< "/>" << nl;
				}
				AddPoint( pCISpointArray, pNumCISpoints, etd, leave, -1 );
				if( !hasPass && numTransfers > 1 )
					os << tab << "<getTransfer/>" << nl;
			}
			else
			{
				// We must have got off our last vehicle to get on another one.
				os << tab << "<exitVehicle"
						<< " time=" << qt(formatTime(lastTravel->to->t))
						<< " stop=" << qt(trimString(lastTravel->to->location->getName()))
						<< "/>" << nl;
				os << tab << "<transfer/>" << nl;
				if( lastTravel->to->location != (*a)->from->location )
				{
					os << tab << "<walkToTransfer"
						<< " time=" << qt(formatTime(lastTravel->to->t + cis->exitTime))
						<< " stop=" << qt(trimString((*a)->from->location->getName()))
						<< "/>" << nl;
				}
			}
			os << tab << "<enterVehicle"
				<< " time=" << qt(formatTime((*a)->from->t))
				<< " route=" << qt(trimString((*a)->getRoute()->getName()))
				<< " stop=" << qt(trimString((*a)->from->location->getName()))
				<< " direction=" << qt(trimString(getDirection(cis->compassDirection(szDirection, sizeof(szDirection), "!%s",
											(*a)->from->location->getPoint(),
											(*a)->to->location->getPoint()), mm)))
				<< "/>" << nl;

			--curTransfer;
			// Check if we need to pay an additional fare.
			if( !hasPass && tCur > service->transferTimeMax )
			{
				os << tab << "<payAdditionalFare/>" << nl;
				// Check if we still need to get another transfer.
				if( curTransfer > 1 )
					os << tab << "<getTransfer/>" << nl;
				tCur = 0;
			}

			AddTravel( pCISpointArray, pNumCISpoints,
				(*a)->from->t, CIS::Point((*a)->from->location->longitude, (*a)->from->location->latitude),
				(*a)->to->t, CIS::Point((*a)->to->location->longitude, (*a)->to->location->latitude),
				(*a)->route->getID(), (*a)->route->getName() );
			lastTravel = *a;
		}
		os << tab << "<exitVehicle"
				<< " time=" << qt(formatTime(lastTravel->to->t))
				<< " stop=" << qt(trimString(lastTravel->to->location->getName()))
				<< "/>" << nl;

		const tod_t eta = lastTravel->to->t +
			(driverPlan && cis->nodeEquivalences.isEquivalent(toNODESrecordID, lastTravel->to->location->getID())
			?  cis->nodeEquivalences.getConnectionTime(toNODESrecordID, lastTravel->to->location->getID())
			: cis->walkingDistanceToTime(lastTravel->to->location->getPoint().distance(arrivePoint)));
		cis->compassDirection(szDirection, sizeof(szDirection), "!%s",
												lastTravel->to->location->getPoint(), arrivePoint);
		if( eta != lastTravel->to->t && szDirection[0] != 0 )
		{
			os << tab << "<walkFromStop"
				<< " time=" << qt(formatTime(lastTravel->to->t))
				<< " stop=" << qt(trimString(lastTravel->to->location->getName()))
				<< " direction=" << qt(trimString(getDirection(szDirection, mm)))
				<< "/>" << nl;
		}

		os << tab << "<arrive time=" << qt(formatTime(eta)) << "/>" << nl;
		AddPoint( pCISpointArray, pNumCISpoints, eta, arrive, -1 );
	} while(0);
	os << "</INSTRUCTIONS>" << nl;

	// Update the foot of the report.
	os << "<EPILOG>" << nl;
	if( status == CIS::good )
	{
		if( walkingTime > 60 )	// Do not bother walking time less than 1 minute.
			os << tab << "<totalWalking distance=" << qt(formatDistance(walkingDistance)) << " "
				<< "minutes=" << qt(secondsToMinutes(walkingTime)) << "/>" << nl;
		if( numTransfers > 0 )
			os << tab << "<transfers num=" << qt(numTransfers) << "/>" << nl;
		os << tab << "<travelTime minutes=" << qt(secondsToMinutes(travelTime)) << "/>" << nl;
		os << tab << "<travelPlusDwellTime minutes=" << qt(secondsToMinutes(travelTime+dwellTime)) << "/>" << nl;
	}
	os << "</EPILOG>" << nl;

	// Finish off the trip description.
	os << "</TRIP>" << nl;

	// Finish off the plan.
	os << "</TRIPPLAN>" << nl;

	// Make sure the string is terminated.
	os << ends;

	if( fTripPlanLog )
	{
		std::ofstream tripPlanLogXML("TripPlanLog.xml");
		tripPlanLogXML << os.str() << nl;
	}

	return os.str();
}

//---------------------------------------------------------------------------------------------------------------

void	CISmessageMap::read( std::istream &is )
{
	char	szLine[1024], *pLine;
	char	szToken[1024], *pToken = NULL;
	char	szSubstitution[4096], *pSubstitution = szSubstitution;

	while( is.getline(szLine, sizeof(szLine)) )
	{
		// Skip comments.
		if( szLine[0] == '#' )
			continue;

		// Trim any trailing whitespace.
		const int lineLen = strlen(szLine);
		if( lineLen > 0 )
			for( pLine = &szLine[lineLen-1]; pLine > szLine && isspace(*pLine); --pLine )
				*pLine = 0;

		// Check if the first character is a '<'.  This indicates a token.
		if( szLine[0] == '<' )
		{
			// This is a token.
			// If we already have a token on the go, add it to the hash table.
			if( pToken != NULL )
				strHash.insert( szToken, szSubstitution );

			// Grab this token.
			for( pLine = &szLine[1], pToken = szToken; *pLine && *pLine != '>'; ++pLine )
				*pToken++ = *pLine;
			*pToken = 0;

			// Reset its substituion.
			pSubstitution = szSubstitution;
			*pSubstitution = 0;
			continue;
		}

		// A space or empty line in the first position indicates part of the subsitution of this token.
		if( szLine[0] == 0 || isspace(szLine[0]) )
		{
			// Trim leading spaces.
			for( pLine = szLine; *pLine && isspace(*pLine); ++pLine )
				continue;
			// Append to the substitution.
			for( ; *pLine; ++pLine )
				*pSubstitution++ = *pLine;

			// Check for a line continuation, that is, no newline is to be added to the substitution.
			if( pSubstitution > szSubstitution && *(pSubstitution-1) == '_' )
				--pSubstitution;
			else
			{
				// Add a new line.
				*pSubstitution++ = '\r';
				*pSubstitution++ = '\n';
			}
			*pSubstitution = 0;
		}
	}
	if( pToken != NULL )
		strHash.insert( szToken, szSubstitution );
}

CIStranslate::CIStranslate() : iStack(0), symbolStack(NULL)
{
	register size_t	i, k;
	char	szSymbol[64];
	char	szValue[256];

	// Insert convenience symbols.
	symbols.insert( "tab", "\t" );
	for( i = 1; i <= 8; i++ )
	{
		sprintf( szSymbol, "tab%d", i );
		for( k = 0; k < i; k++ )
			szValue[k] = '\t';
		szValue[k] = 0;
		symbols.insert( szSymbol, szValue );
	}
	symbols.insert( "sp", " " );
	for( i = 1; i <= 20; i++ )
	{
		sprintf( szSymbol, "sp%d", i );
		for( k = 0; k < i; k++ )
			szValue[k] = ' ';
		szValue[k] = 0;
		symbols.insert( szSymbol, szValue );
	}
	symbols.insert( "nl", "\r\n" );
	for( i = 1; i <= 20; i++ )
	{
		sprintf( szSymbol, "nl%d", i );
		for( k = 0; k < i; k++ )
		{
			szValue[k*2] = '\r';
			szValue[k*2+1] = '\n';
		}
		szValue[k*2] = 0;
		symbols.insert( szSymbol, szValue );
	}
}

inline int	fastStrCmp( register const char *p, register const char *q )
{
	// Inline the comparisons of the first 4 characters.
	return  *p <   *q ? -1 : *p > *q ? 1 : *p == 0 ? 0
		: *++p < *++q ? -1 : *p > *q ? 1 : *p == 0 ? 0
		: *++p < *++q ? -1 : *p > *q ? 1 : *p == 0 ? 0
		: *++p < *++q ? -1 : *p > *q ? 1 : *p == 0 ? 0
		: strcmp( ++p, ++q );
}

char	*CIStranslate::translate( char *szOut, const char *szIn, const CISmessageMap &mm )
{
	istrstream iss((char *)szIn);
	ParseStream	is(iss);
	char	c;
	register char	*q, *pOut = szOut;
	char	szTokenBuf[128], *szToken = szTokenBuf;
	char	szSymbol[128];
	char	szValue[2048];
	char	szSubstitution[256];
	SymbolValueElement	*symbolStackPtr[8];
	SymbolValueElement	*symbolStack = NULL;
	size_t	iStack = 0;

	*szToken++ = '/';	// Initialize a '/' prefix on the token.
	while( is.good() )
	{
		// Try to find a token.
		is.eatwhite();
		if( !is.get(c) || c != '<' )
			break;

		// Check the first char of the token.
		q = szToken;
		if( !is.get(c) ) break;
		*q++ = c;
		if( c == '!' )		// Check for a comment.
		{
			if( !is.get(c) ) break;
			if( (*q++ = c) == '-' )
			{
				if( !is.get(c) ) break;
				if( (*q++ = c) == '-' )
				{
					// This is the start of a comment.  Handle nested comments.
					size_t	nestedCount = 1;
					char	lookBehind[5];
					strcpy( lookBehind, "<!--" );
					for( ;; )
					{
						register char *z = lookBehind, *y = z + 1;
						*z++ = *y++; *z++ = *y++; *z++ = *y++;
						if( !is.get(lookBehind[3]) )
							break;
						if( fastStrCmp(lookBehind, "<!--") == 0 )
							++nestedCount;
						else if( fastStrCmp(&lookBehind[1], "-->") == 0 )
							if( --nestedCount == 0 )
								break;
					}
					continue;	// Skip the comment.
				}
			}
		}
		else if( c == '?' )	// Check for a special tag.
		{
			char	c1;
			if( !is.get(c1) ) break;
			while( is.get(c).good() )
			{
				if( c1 == '?' && c == '>' )
					break;
				c1 = c;
			}
			continue; // Skip the special tag.
		}

		// Collect the rest of the token and its labels.
		// Handle the syntax of '/>' to terminate a tag without variable attributes.
#define isTokenTerminator(c)	((c) == '>' || (c) == '/')
		while( is.get(c) && !isspace(c) && !isTokenTerminator(c) ) *q++ = c; *q = 0;

		// Keep stack pointers to the last symbols and operations.
		if( szToken[0] != '/' )
			symbolStackPtr[iStack++] = symbolStack;

		while( !isTokenTerminator(c) && (is.eatwhite(), is.get(c) && !isTokenTerminator(c)) )
		{
			// Collect all the labels.
			q = szSymbol;
			do *q++ = c; while( is.get(c) && c != '=' ); *q = 0;
			if( is.get(c) && (c == '\'' || c == '"') )	// Check for quoted attributes.
			{
				const char quoteChar = c;
				for( q = szValue; is.get(c) && c != quoteChar; )
				{
					if( c == '\\' )
					{
						is.get(c);
						if( isdigit(c) )
						{
							// Parse an octal digit.
							int	v = 0;
							register int i = 0;
							do v = (v<<3) + c - '0'; while( ++i < 3 && is.get(c) );
							*q++ = (char)v;
						}
						else
						{
							// Parse an escaped character.
							switch( c )
							{
							case 'n':	*q++ = '\n';	break;
							case 'r':	*q++ = '\r';	break;
							case 't':	*q++ = '\t';	break;
							default:
								*q++ = c;
							}
						}
					}
					else
						*q++ = c;	// Just take the character.
				}
				is.get(c);	// Skip the ending quote.
			}
			else
			{
				// This is an unquoted attribute - technically impossible in strict xml.
				q = szValue;
				do *q++ = c; while( is.get(c) && !isTokenTerminator(c) && !isspace(c) );
			}
			*q = 0;

			// Register the new symbol.
			registerSymbolValue(szSymbol, szValue);
		}

		// Check for a terminated token and skip the '/' if present.
		const bool	labelIsTerminated = (c == '/' ? (is.get(c), true) : false);
		is.get(c); // skip the ending '>'.

		// Dispatch the correct read routine based on the token.
		for( ;; )
		{
			TString	sTranslation(mm.find(szToken));
			if( !sTranslation.empty() )
			{
				for( register const char *pTranslation = sTranslation.getStr(); *pTranslation != 0; ++pTranslation )
				{
					if( *pTranslation == '$' && *(pTranslation+1) == '(' )
					{
						// Parse a substitution.
						register char	*pSubstitution = szSubstitution;
						for( pTranslation += 2; *pTranslation != 0 && *pTranslation != ')'; ++pTranslation )
							*pSubstitution++ = *pTranslation;
						*pSubstitution = 0;
						TString	value;
						TStringHash::iterator found = symbols.find(szSubstitution);
						if( found != symbols.end() )
						{
							strcpy( pOut, found.data().getStr() );
							pOut += found.data().length();
						}
						else
						{
							*pOut++ = '*'; *pOut++ = '*'; *pOut++ = '*'; *pOut++ = '*';
							strcpy( pOut, szSubstitution ); pOut += strlen( szSubstitution );
							*pOut++ = '=';
							strcpy(	pOut, "Undefined" ); pOut += strlen("Undefined");
							*pOut++ = '*'; *pOut++ = '*'; *pOut++ = '*'; *pOut++ = '*';
						}
					}
					else
						*pOut++ = *pTranslation;
				}
			}

			// If the label is terminated, create a terminating token.
			if( labelIsTerminated && szToken[0] != '/' )
			{
				// Modify the token to be a terminating one.  This allows the callback mechanism to
				// work exactly the same if the user had input a real </token> label.
				--szToken;
				continue;
			}
			else
				break;
		}

		if( szToken[0] == '/' )
		{
			// This is a terminating label.
			if( labelIsTerminated )
				++szToken;	// Remove the '/' prefix.

			// Clean up the symbols pushed from the last label.
			--iStack;
			while( symbolStackPtr[iStack] != symbolStack )
			{
				SymbolValueElement	*e = symbolStack;
				symbolStack = symbolStack->next;
				if( !e->value.empty() )					// restore the previous symbol value
					symbols.insert( e->symbol, e->value );
				else
					symbols.remove( e->symbol );		// remove the symbol
				delete e;
			}
		}
	}

	// Clean up any remaining symbols or operators.
	while( symbolStack != NULL )
	{
		SymbolValueElement	*e = symbolStack;
		symbolStack = symbolStack->next;
		delete e;
	}

	*pOut = 0;
	return szOut;
}


TripPlan *CISresult::toTripPlan()
{
	if( !(status == CIS::betterToWalk || status == CIS::good) )
		return NULL;

	TripPlan	*tp = new TripPlan();

	// Convert lat/long to meters to preserve sanity.
	int	x, y;
	Distance::CoordFromLongLat( x, y, leave.longitude, leave.latitude );
	QTree::Point leavePoint( x, y );

	Distance::CoordFromLongLat( x, y, arrive.longitude, arrive.latitude );
	QTree::Point arrivePoint( x, y );

	CISarcList::iterator a;

	// Check if the trip plan actually goes somewhere.
	if( status == CIS::good )
	{
		for( a = arcList.begin(); a != arcList.end(); ++a )
			if( (*a)->isTravel() )
				break;
		if( a == arcList.end() )
			status = CIS::betterToWalk;
	}
	if( status == CIS::betterToWalk )
	{
		tp->numberOfTransfers = 0;
		tp->totalWalkingMeters = leavePoint.distance(arrivePoint);
		tp->totalWalkingMinutes = secondsToMinutes(cis->walkingDistanceToTime(tp->totalWalkingMeters));
		tp->travelMinutes = tp->totalWalkingMinutes;
		tp->travelPlusDwellMinutes = tp->travelMinutes;
		tp->instructions.push_back( new WalkDirectlyInstruction(Direction(CIS::compassPoint(leavePoint, arrivePoint)), tp->totalWalkingMeters ));
		return tp;
	}

	// Report the path.
	tod_t	tCur = 0;
	size_t	curTransfer = numTransfers;
	CISarc	*lastTravel = NULL, *firstTravel = NULL;
	for( a = arcList.begin(); a != arcList.end(); ++a )
	{
		tCur += (*a)->getElapsed();
		if( !(*a)->isTravel() )
			continue;

		if( firstTravel == NULL )
			firstTravel = *a;

		// Record the from and to stops of this travel arc (remove duplicates later).
		tp->stops.push_back(
			TimeStopRoute(
				(*a)->from->t,
				Stop( (*a)->from->location->getID(), (*a)->from->location->getName(),
					Coordinate((*a)->from->location->latitude, (*a)->from->location->longitude)),
				Route((*a)->route->getName(),(*a)->route->getDirection()->getName())));

		tp->stops.push_back(
			TimeStopRoute(
				(*a)->to->t,
				Stop( (*a)->to->location->getID(), (*a)->to->location->getName(),
					Coordinate((*a)->to->location->latitude, (*a)->to->location->longitude)),
				Route((*a)->route->getName(),(*a)->route->getDirection()->getName())));

		// Check if this travel is on the same vehicle and route.
		// If so, continue.
		if( lastTravel != NULL )
		{
			if( !isTransfer(lastTravel->idTrip, lastTravel->route->getID(), lastTravel->idBlock,
							(*a)->idTrip, (*a)->route->getID(), (*a)->idBlock) )
			{
				lastTravel = *a;
				continue;
			}
			// Check if the route changes but the block number (vehicle) stays the same.
			// If so, only create an informational instruction as the rider does not need to transfer to another vehicle.
			if( isRouteChange(lastTravel->route->getID(), lastTravel->idBlock,
							   (*a)->route->getID(), (*a)->idBlock) )
			{
				tp->instructions.push_back(
					new RouteChangesInstruction(
						(*a)->from->t,
						Stop((*a)->from->location->getID(), (*a)->from->location->getName(),
							Coordinate((*a)->from->location->latitude, (*a)->from->location->longitude)),
						Route((*a)->getRoute()->getName(),(*a)->getRoute()->getDirection()->getName())) );
				lastTravel = *a;
				continue;
			}
		}

		if( lastTravel == NULL )
		{
			// We just left.
			const QTree::Point p((*a)->from->location->getPoint());
			const coord_t dist = p.distance(leavePoint);
			const tod_t etd = (*a)->from->t - (driverPlan
				? cis->nodeEquivalences.getConnectionTime(fromNODESrecordID, firstTravel->from->location->getID())
				: cis->walkingDistanceToTime(dist));

			tp->instructions.push_back(
				new LeaveFromLocationInstruction(
					etd,
					Coordinate(leave.latitude, leave.longitude),sLeave.getStr()));

			if( etd != (*a)->from->t )
			{
				tp->instructions.push_back(
					new WalkToStopInstruction(
						    etd,
						    Stop( firstTravel->from->location->getID(), firstTravel->from->location->getName(),
							    Coordinate(firstTravel->from->location->latitude, firstTravel->from->location->longitude)),
						    Direction(CIS::compassPoint(leavePoint, p)),
							dist ));
			}
		}
		else
		{
			// Record a transfer.
			// Exit the last vehicle.
			tp->instructions.push_back(
				new ExitVehicleInstruction(
						lastTravel->to->t,
						Stop(lastTravel->to->location->getID(), lastTravel->to->location->getName(),
							Coordinate(lastTravel->to->location->latitude, lastTravel->to->location->longitude)),
						Stop(lastTravel->from->location->getID(), lastTravel->from->location->getName(),
							Coordinate(lastTravel->from->location->latitude, lastTravel->from->location->longitude))));

			// Record a transfer.
			tp->instructions.push_back( new TransferInstruction() );

			// Record walking to the transfer stop if necessary.
			bool isEquivalent = (lastTravel->to->location == (*a)->from->location ||
								lastTravel->to->location->eqLongitudeLatitude(*(*a)->from->location));
			if( !isEquivalent )
			{
				tp->instructions.push_back(
					new WalkToTransferStopInstruction(
						Stop((*a)->from->location->getID(), (*a)->from->location->getName(),
							Coordinate((*a)->from->location->latitude, (*a)->from->location->longitude)),
						Direction(CIS::compassPoint(lastTravel->to->location->getPoint(), (*a)->from->location->getPoint())),
						lastTravel->to->location->getPoint().distance((*a)->from->location->getPoint())));
			}
		}

		// Record entering a vehicle.
		EnterVehicleInstruction *evi =
				new EnterVehicleInstruction((*a)->from->t,
						Stop((*a)->from->location->getID(), (*a)->from->location->getName(),
							Coordinate((*a)->from->location->latitude, (*a)->from->location->longitude)),
						Route((*a)->getRoute()->getName(),(*a)->getRoute()->getDirection()->getName()),
						Stop((*a)->to->location->getID(), (*a)->to->location->getName(),
							Coordinate((*a)->to->location->latitude, (*a)->to->location->longitude)));

		// Find the next leave times of this route from this location.
		{
			CISlocation *fromLoc = (*a)->from->location;
			CIScRouteIDEventSet::iterator resItr = fromLoc->routeCurb.find( (*a)->route->getID() );
			CIScEventSet	&es = resItr->second;

			size_t	i = 0;
			register CIScEventSet::const_iterator e = es.find( (*a)->from ), eEnd = es.end();
			for( ++e; e != eEnd; ++e )
			{
				if( (*e)->route == (*a)->route )
				{
					evi->nextLeaveTime[i].t = (*e)->t;
					if( ++i >= EnterVehicleInstruction::numNextLeaveTimes )
						break;
				}
			}
		}
		tp->instructions.push_back( evi );

		lastTravel = *a;
	}

	// Exit the last vehicle.
	tp->instructions.push_back(
		new ExitVehicleInstruction(
				lastTravel->to->t,
				Stop(lastTravel->to->location->getID(), lastTravel->to->location->getName(),
					Coordinate(lastTravel->to->location->latitude, lastTravel->to->location->longitude)),
				Stop(lastTravel->from->location->getID(), lastTravel->from->location->getName(),
					Coordinate(lastTravel->from->location->latitude, lastTravel->from->location->longitude))));

	// Compute estimated time of arrival.
	const coord_t dist = lastTravel->to->location->getPoint().distance(arrivePoint);
	const tod_t eta = lastTravel->to->t +
		(driverPlan && cis->nodeEquivalences.isEquivalent(toNODESrecordID, lastTravel->to->location->getID())
		?  cis->nodeEquivalences.getConnectionTime(toNODESrecordID, lastTravel->to->location->getID())
		: cis->walkingDistanceToTime(dist));

	// Record walking to the destination if necessary.
	if( eta != lastTravel->to->t )
	{
		tp->instructions.push_back(
			new WalkFromStopInstruction(
					lastTravel->to->t,
					Stop( lastTravel->to->location->getID(), lastTravel->to->location->getName(),
						Coordinate(lastTravel->to->location->latitude, lastTravel->to->location->longitude)),
					Direction(CIS::compassPoint(lastTravel->to->location->getPoint(), arrivePoint)),
					dist));
	}

	// Arrive at the destination, and we are done.
	tp->instructions.push_back(
		new ArriveAtLocationInstruction(
				eta,
				Coordinate(arrive.latitude, arrive.longitude),
				sArrive.getStr() ));

	// Clean up any stops for the same location, time and route.
	tp->removeDuplicateStops();

	// Record details.
	tp->totalWalkingMeters = (int)walkingDistance;
	tp->totalWalkingMinutes = secondsToMinutes(walkingTime);
	tp->numberOfTransfers = numTransfers;
	tp->travelMinutes = secondsToMinutes(travelTime);
	tp->travelPlusDwellMinutes = secondsToMinutes(travelTime+dwellTime);

	return tp;
}

TripPlanReply *CISresultCollection::toTripPlanReply()
{
	TripPlanReply	*tpr = new TripPlanReply();

	CISresult	&result = *results[0];

	tpr->fromName = result.sLeave.getStr();
	tpr->fromCoord.set( result.leave.latitude, result.leave.longitude );
	tpr->toName = result.sArrive.getStr();
	tpr->toCoord.set( result.arrive.latitude, result.arrive.longitude );

	switch( result.type )
	{
	case CIS::leaveEarliest:	tpr->timeOption = TimeOption::LeaveEarliest;	break;
	case CIS::leaveLatest:		tpr->timeOption = TimeOption::LeaveLatest;		break;
	case CIS::leaveAt:			tpr->timeOption = TimeOption::LeaveAtTime;  tpr->timeOption = result.t; break;
	case CIS::arriveAt:			tpr->timeOption = TimeOption::ArriveAtTime; tpr->timeOption = result.t; break;
	}

	tpr->service = (result.service ? result.service->getName() : "NoService");

	bool	success = true;
	switch( result.status )
	{
	case CIS::noLeaveStop:		tpr->error = Error::NoLeaveStop;				success = false; break;
	case CIS::noArriveStop:		tpr->error = Error::NoArriveStop;				success = false; break;
	case CIS::noLeaveArriveStop:tpr->error = Error::NoLeaveArriveStop;			success = false; break;
	case CIS::leaveTooEarly:	tpr->error = Error::LeaveTooEarly;				success = false; break;
	case CIS::leaveTooLate:		tpr->error = Error::LeaveTooLate;				success = false; break;
	case CIS::noService:		tpr->error = Error::NoService;					success = false; break;
	case CIS::infeasible:		tpr->error = Error::Infeasible;					success = false; break;
	}

	if( !success )
		return tpr;

	for( size_t i = 0; i < max; ++i )
	{
		TripPlan	*tp = results[i]->toTripPlan();
		if( tp )
			tpr->tripPlans.push_back( tp );
	}

	return tpr;
}

TiXmlDocument	*CISrouteTimesReply::toXmlDoc() const
{
	TiXmlDocument *doc = new TiXmlDocument();
	doc->LinkEndChild( new TiXmlDeclaration( "1.0", "ISO-8859-1", "yes" ) );

	TiXmlElement	*element = new TiXmlElement( "TMSwebRouteTimesReply" );
	doc->LinkEndChild( element );

	TiXmlElement	*child = new TiXmlElement("Service");
	child->LinkEndChild( new TiXmlText(service->getName()) );
	element->LinkEndChild( child );

	{
		Route	routeXml( route->getName(), route->getDirection()->getName() );
		element->LinkEndChild( routeXml.toXml() );
	}

	child = new TiXmlElement("Direction");
	child->LinkEndChild( new TiXmlText(route->getDirection()->getName()) );
	element->LinkEndChild( child );

	TiXmlElement	*stopTimes = new TiXmlElement("StopTimes");
	element->LinkEndChild( stopTimes );

	CISlocationTimesVector::const_iterator loc, locEnd;
	for( loc = locationTimes.begin(), locEnd = locationTimes.end(); loc != locEnd; ++loc )
	{
		TiXmlElement	*stopTime = new TiXmlElement("StopTime");
		stopTimes->LinkEndChild( stopTime );

		Stop	stop( (*loc).location->getID(), (*loc).location->getName(), Coordinate((*loc).location->getLatitude(), (*loc).location->getLongitude()) );
		stopTime->LinkEndChild( stop.toXml() );

		TiXmlElement	*times = new TiXmlElement("Times");
		stopTime->LinkEndChild( times );

		std::string	timeStr;
		TimeVector::const_iterator tv, tvEnd;
		for( tv = (*loc).times.begin(), tvEnd = (*loc).times.end(); tv != tvEnd; ++tv )
		{
			if( (*tv) < 0 )
				timeStr.append( "-," );
			else
			{
				const int	hour   = *tv / (60 * 60);
				const int	minute = (*tv % (60 * 60)) / 60;
				const int	second = *tv % 60;
				char  sz[16];
				sprintf( sz, "%02d%02d%02d,", hour, minute, second );
				timeStr.append( sz );
			}
		}
		timeStr.erase( timeStr.end() - 1 );	// Remove the last separator.
		times->LinkEndChild( new TiXmlText(timeStr) );
	}

	// Add all the connections.
	TiXmlElement	*connections = new TiXmlElement("Connections");
	element->LinkEndChild( connections );
	CISfromToVector::const_iterator ft, ftEnd;
	for( ft = fromToConnections.begin(), ftEnd = fromToConnections.end(); ft != ftEnd; ++ft )
	{
		TiXmlElement	*c = new TiXmlElement("Connection");
		connections->LinkEndChild( c );

		TiXmlElement	*from = new TiXmlElement("FromID");
		c->LinkEndChild( from );
		from->LinkEndChild( new TiXmlText( toStr((int)ft->first->getID())) );

		TiXmlElement	*to = new TiXmlElement("ToID");
		c->LinkEndChild( to );
		to->LinkEndChild( new TiXmlText( toStr((int)ft->second->getID())) );
	}

	return doc;
}


