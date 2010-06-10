//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef cis_H
#define cis_H
 
// Disable the pesky stl warnings.
#pragma warning(disable : 4786 4114 4800)

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

extern "C"
{
#include "TMSHeader.h"
}

#include "multi_cmp.h"
#include "hash_buf.h"
#include <string.h>
#include <fstream>
#include <limits.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "tod.h"
#include "qtree.h"
#include "distance.h"
#include "Hash.h"
#include "HashFI.h"
#include "HashObj.h"
#include "TString.h"
#include "PairingHeap.h"
#include "parseMIF.h"
#include "RefCount.h"
#include "Trapezoid.h"
#include "TripPlanRequestReply.h"
#include "BinSearch.h"
#include "StrStripCmp.h"
#include "Stripe.h"
#include "ServiceCalendar.h"

#define ForAllInCollection( c, i ) \
	for( (i) = (c).begin(); !((i) == (c).end()); ++(i) )

#ifndef Assert
#define Assert( c )		do { if( !(c) ) { int xxx1 = 10, xxx2 = 0, xxx3; xxx3 = xxx1 / xxx2; } } while(0)
#endif

typedef std::vector<std::string>	StringVector;
typedef std::vector<ident_t>		IDVector;

class IdentHash
{
public:
	IdentHash( const ident_t aID ) : id(aID) {}
	IdentHash( const IdentHash &aIH ) : id(aIH.id) {}

	IdentHash &operator=( const ident_t aID ) { id = aID; return *this; }
	IdentHash &operator=( const IdentHash &aIH ) { id = aIH.id; return *this; }

	size_t	hash() const { return id; }

	operator ident_t() const { return id; }

	bool operator==( const IdentHash &ih ) const { return id == ih.id; }

	ident_t	id;
};

inline bool isTransfer(const ident_t fromTripID, const ident_t fromRouteID, const ident_t fromBlockID,
                       const ident_t   toTripID, const ident_t   toRouteID, const ident_t   toBlockID )
{
  if( fromRouteID != toRouteID )
    return true;
  if( fromBlockID != toBlockID )
    return true;
  if( fromTripID != toTripID )
    return true;

  return false;  
}

inline bool isRouteChange(const ident_t fromRouteID, const ident_t fromBlockID,
						  const ident_t   toRouteID, const ident_t   toBlockID )
{
  if( fromRouteID != toRouteID && fromBlockID == toBlockID )
    return true;

  return false;  
}

// This class is used in the path hash table.
class FromToTimeCISservice
{
public:
	FromToTimeCISservice( const long aFrom, const long aTo, const tod_t aT, const long aCISservice ) : from(aFrom), to(aTo), t(aT), service(aCISservice) {}
	FromToTimeCISservice( const FromToTimeCISservice &ftt ) { operator=(ftt); }

	int operator==( const FromToTimeCISservice &ftt ) const
	{ return from == ftt.from && to == ftt.to && t == ftt.t && service == ftt.service; }

	FromToTimeCISservice &operator=( const FromToTimeCISservice &ftt )
	{
		from = ftt.from;
		to = ftt.to;
		t = ftt.t;
		service = ftt.service;
		return *this;
	}

	size_t hash() const
	{
		return multi_hash( from, to, t, service );
	}

private:
	long	from, to, service;
	tod_t	t;
};

class CISresult;
typedef HashObj<FromToTimeCISservice, Ref<CISresult> >	FromToTimeCISserviceHash;

class CIS;
class CIScEvent;
class CISlocation;
class CISvehicle;
class CISdirection;
class CISroute;
class CISservice;
class CIStransferGroup;
class CISarc;
class CISsolution;
struct CIScmpSolutionDecreasing;
class CISresult;
class CISresultCollection;
class CISmessageMap;

typedef std::vector<CISarc *>	CISarcVector;
typedef Hash< ident_t, CISlocation * > CISlocationCollection;
typedef std::map<ident_t, std::set<ident_t> > CISlocationMapSet;
typedef std::vector<CISlocation *> CISlocationVector;
typedef Hash< ident_t, CISservice * > CISserviceCollection;
typedef std::vector<CISservice *>	CISserviceVector;
typedef Hash< ident_t, CIStransferGroup * > CIStransferGroupCollection;
typedef Hash< ident_t, CISdirection * > CISdirectionCollection;
typedef Hash< ident_t, CISvehicle * > CISvehicleCollection;
typedef Hash< ident_t, CISroute * > CISrouteCollection;
typedef std::vector< CISroute * >	CISrouteVector;
typedef HashObj<TString, TString> TStringHash;

typedef std::vector< std::pair<ident_t, ident_t> >    PathifyConnections;
void    pathify( PathifyConnections &pc );
void	getPathifyConnections( PathifyConnections &pc, CISlocationMapSet &fromTo );

class ID
{
public:
	ID( const ident_t aID ) : id(aID) {}
	ID( const ID &aID ) : id(aID.id) {}

	ident_t	getID() const { return id; }

	bool operator< ( const ID &i ) const { return id <  i.id; }
	bool operator<=( const ID &i ) const { return id <= i.id; }
	bool operator==( const ID &i ) const { return id == i.id; }
	bool operator> ( const ID &i ) const { return id >  i.id; }
	bool operator>=( const ID &i ) const { return id >= i.id; }

	ID &operator=( const ID &i ) { id = i.id; return *this; }

protected:
	ident_t	id;
};

//---------------------------------------------------------------------------------

class IDName : public ID
{
public:
	IDName( const ident_t id, const char *aName ) : ID(id), name(aName) {}
	IDName( const IDName &idn ) : ID(idn.id), name(idn.name) {}
	IDName &operator=( const IDName &idn )
	{
		ID::operator=( idn );
		name = idn.name;
		return *this;
	}
	const char *getName() const { return name.getStr(); }
protected:
	TString	name;
};

//---------------------------------------------------------------------------------

class CISdirection : public IDName
{
public:
	CISdirection( const ident_t aID, const char *aName ) : IDName(aID, aName) {}
	MMdeclare();
};

//---------------------------------------------------------------------------------

struct CISvehicleFlags
{
	CISvehicleFlags( const bool isSurface = true, const bool isAccessible = false, const bool isBicycle = false )
	{
		surface =		(isSurface ? 1 : 0);
		accessible =	(isAccessible ? 1 : 0);
		bicycle =		(isBicycle ? 1 : 0);
	}
	unsigned int surface:1;
	unsigned int accessible:1;
	unsigned int bicycle:1;
};

class CISvehicle : public IDName
{
public:
	CISvehicle( const ident_t aID, const char *aName, const bool isSurface = true, const bool isAccessible = false, const bool isBicycle = false ) :
	  IDName(aID, aName), vehicleFlags(isSurface, isAccessible, isBicycle)
	  {}
	MMdeclare();

	bool isSurface() const { return vehicleFlags.surface; }
	bool isAccessible() const { return vehicleFlags.accessible; }
	bool isBicycle() const { return vehicleFlags.bicycle; }
	const	CISvehicleFlags	&getFlags() const { return vehicleFlags; }
protected:
	const	CISvehicleFlags	vehicleFlags;
};

//---------------------------------------------------------------------------------

class CISroute : public IDName
{
	// Route combines with direction.
public:
	CISroute( const ident_t id, const char *aName, const CISservice *aCISservice, const CISdirection *aCISdirection ) :
	  IDName(id, aName), service(aCISservice), direction(aCISdirection) {}
	MMdeclare();

	size_t serviceIndex() const;
	const	CISdirection	*getDirection() const { return direction; }

	StripeVector	stripes;		// Cache for timetable info.
	IDVector		idLocations;	// Cache for vector of location ids in this route.
	CISlocationMapSet fromTo;		// Cache for all fromLoc/toLoc connections in this route.
protected:
	const CISservice *service;
	const CISdirection *direction;
};

class CISrouteCmp
{
public:
	bool operator()( const CISroute *r1, const CISroute *r2 ) const
	{
		const int i = StrStripCmp(r1->getName(), r2->getName()); 
		if( i != 0 )
			return i == -1;
		return StrStripCmp(r1->getDirection()->getName(), r2->getDirection()->getName()) == -1; 
	}
};
//---------------------------------------------------------------------------------

class CISservice : public IDName
{
	friend class CIS;
	friend class CISresult;
public:
	enum ArcType
	{
		Wait,
		Travel,
		Dwell,
		Enter,
		Exit,
		WalkTransfer
	};

	CISservice( const ident_t aID, const char *aName, const tod_t aTransferTimeMax = 48*60*60 ) :
	  IDName(aID, aName), numCIScEvents(0), transferTimeMax(aTransferTimeMax) {}
	~CISservice();
	MMdeclare();

	void	addRoute( const ident_t id, const char *aName, const CISdirection *aCISdirection )
	{ if( !routes.contains(id) ) routes.insert( id, new CISroute(id, aName, this, aCISdirection) ); }
	const CISroute	*getRoute( const ident_t id )
	{ return routes.find(id).data(); }
	void	addTravel( CIS &cis, const tod_t fromTime, CISlocation *fromLoc, const tod_t toTime, CISlocation *to,
      CISroute *route, const ident_t idTrip, const ident_t idBlock,
      CISvehicle *vehicle );

	CISlocation	*getLocation( const ident_t idLocation ) { return locations[idLocation]; }

protected:
	void	initSolve( CIS &cis );
	CISarc		*connect( CIScEvent *from, CIScEvent *to,
            const CISroute *route, const ident_t idTrip, ident_t idBlock,
            const CISvehicle *vehicle, const ArcType arcType );

	CISarc		*connect( CIScEvent *from, CIScEvent *to, const ArcType arcType )
	{
		return connect( from, to, NULL, -1, -1, NULL, arcType );
	}

	CISlocationCollection	locations;			// locations used by this service.
	CIStransferGroupCollection	transferGroups;	// transfer groups used by this service.

	CISrouteCollection		routes;
	CISrouteVector			routesByName;
	QTree	qtree;							// quad tree of locations used in this service.

	size_t	numCIScEvents;					// number of events for this service.

	tod_t	transferTimeMax;				// maximum time allowed on a transfer.
};

class CISserviceCmp
{
public:
	bool operator()( const CISservice *s1, const CISservice *s2 ) const
	{ return StrStripCmp(s1->getName(), s2->getName()) == -1; }
};

//---------------------------------------------------------------------------------

struct CISarcFlags
{
	enum { surfaceShift = 0, accessibleShift, bicycleShift };

	CISarcFlags()
	{
		// By default, turn everything on so we will work with transfers and waits.
		mask = ((1<<16)-1);
	}
	CISarcFlags( const CISvehicleFlags &vehicleFlags )
	{
		mask = 0;
		if( vehicleFlags.surface )
			mask |= (1<<surfaceShift);
		if( vehicleFlags.accessible )
			mask |= (1<<accessibleShift);
		if( vehicleFlags.bicycle )
			mask |= (1<<bicycleShift);
	}
	CISarcFlags( const CISarcFlags &af ) : mask(af.mask) {}
	CISarcFlags &operator=( const CISarcFlags &af )
	{ mask = af.mask; return *this; }
		
	int matches( const CISarcFlags &af ) const
	{ return (mask & af.mask) == af.mask; }

	unsigned int	mask;
};

class CISarc
{
public:

	CISarc( CIScEvent *aFrom, CIScEvent *aTo, const CISroute *aCISroute, const CISvehicle *aCISvehicle,
		const ident_t aIDTrip, const ident_t aIDBlock, const CISservice::ArcType aType ) :
		from(aFrom), to(aTo), route(aCISroute), idTrip(aIDTrip), idBlock(aIDBlock), type(aType)
	{
		if( route != NULL && aCISvehicle != NULL )
			arcFlags = CISarcFlags(aCISvehicle->getFlags());
	}
	MMdeclare();

	bool isWait()		const	{ return type == CISservice::Wait; }
	bool isTravel()		const	{ return type == CISservice::Travel; }
	bool isTransfer()	const	{ return type == CISservice::Exit || type == CISservice::Enter; }
	bool isExit()		const	{ return type == CISservice::Exit; }
	bool isEntry()		const	{ return type == CISservice::Enter; }

	const char *getTypeName() const
	{
		switch( type )
		{
		case CISservice::Wait:			return "Wait";
		case CISservice::Travel:		return "Travel";
		case CISservice::Dwell:			return "Dwell";
		case CISservice::Enter:			return "Enter";
		case CISservice::Exit:			return "Exit";
		case CISservice::WalkTransfer:	return "WalkTransfer";
		}
		return "Unknown";
	};

	bool	matches( const CISarcFlags &af ) const { return arcFlags.matches(af); }

	tod_t getElapsed() const;

	const	char	*compassDirection() const;

	CIScEvent	*from, *to;
	const CISroute	*route;
    const ident_t     idTrip, idBlock; // Used to differentiate when a transfer occurs.
	CISarcFlags	arcFlags;
	CISservice::ArcType		type;

	void	addToInList( CISarc *&inList )
	{
		inNext = inList;
		inList = this;
	}
	void	addToOutList( CISarc *&outList )
	{
		outNext = outList;
		outList = this;
	}

	const CISroute *getRoute() const { return route; }
	const CISdirection *getDirection() const { return route->getDirection(); }

	CISarc		*inNext, *outNext;
};

class CISarcList
{
	struct CISarcElement
	{
		CISarcElement( CISarc *aA ) : a(aA) {}

		MMdeclare();
		CISarc			*a;
		CISarcElement	*next;

		void	addToListTail( CISarcElement *&head, CISarcElement *&tail )
		{
			if( head == NULL )
				head = tail = this;
			else
			{
				tail->next = this;
				tail = this;
			}
			next = NULL;
		}
	};
public:
	class iterator
	{
	public:
		friend class CISarcList;
		iterator() : e(NULL) {}
		iterator( CISarcElement *aE ) : e(aE) {}
		iterator( iterator &i ) : e(i.e) {}

		iterator &operator++() { e = e->next; return *this; }
		iterator operator++(int) { iterator i(*this); operator++(); return i; }
		int operator==( const iterator &i ) const { return e == i.e; }
		int operator!=( const iterator &i ) const { return e != i.e; }

		CISarc	*operator*() const { return e->a; }
	private:
		CISarcElement	*e;
	};

	CISarcList() : head(NULL), tail(NULL), numElements(0) {}
	CISarcList( const CISarcList &al ) : head(NULL), tail(NULL), numElements(0) { operator=(al); }
	~CISarcList() { clear(); }

	CISarcList	&operator=( const CISarcList &al )
	{
		clear();
		register	CISarcElement	*a;
		for( a = al.head; a != NULL; a = a->next )
			insert( a->a );
		return *this;
	}
	CISarcList	&destructiveAssign( CISarcList &al )
	{
		clear();
		head = al.head;
		tail = al.tail;
		numElements = al.numElements;

		al.head = al.tail = NULL;
		al.numElements = 0;
		return *this;
	}

	iterator	begin() const { return iterator(head); }
	iterator	end() const { return iterator(NULL); }
	iterator	last() const { return iterator(tail); }

	void	insert( CISarc *a ) { numElements++; (new CISarcElement(a))->addToListTail(head, tail); }
	void	reverse();
	void	clear()
	{
		register	CISarcElement	*a, *aNext;
		for( a = head; a != NULL; a = aNext )
		{
			aNext = a->next;
			delete a;
		}
		head = tail = NULL;
		numElements = 0;
	}
	size_t	size() const { return numElements; }
	int	empty() const { return numElements == 0; }

private:
	CISarcElement	*head, *tail;
	size_t		numElements;
};

//---------------------------------------------------------------------------------

typedef float utility_t;

class CIScEvent
{
public:
	CIScEvent( const tod_t aT = 0 )		// Used for searching.
	{
		t = aT;
		location = NULL;
		inList = outList = NULL;
		route = NULL;
		service = NULL;

		arcToParent = NULL;
		d = dMax();
		iheap = 0;
	}
	CIScEvent( const tod_t aT, CISlocation *aCISlocation, CISroute *aCISroute = NULL, CISservice *aCISservice = NULL )
	{
		t = aT;
		location = aCISlocation;
		inList = outList = NULL;
		route = aCISroute;
		service = aCISservice;

		arcToParent = NULL;
		d = dMax();
		iheap = 0;
	}
	~CIScEvent();
	MMdeclare();

	CIScEvent	*parent() const
	{
		return (CIScEvent *)(arcToParent ? (arcToParent->from == this ? arcToParent->to : arcToParent->from) : NULL);
	}

	int	cmp( const CIScEvent &s ) const
	{
		return MC::multi_cmp(	t, MC::less, s.t,
								(long)(route ? route->getID() : -1L), MC::less, (long)(s.route ? s.route->getID() : -1L) );
	}
	bool	operator< ( const CIScEvent &s ) const { return cmp(s) <  0; }
	bool	operator<=( const CIScEvent &s ) const { return cmp(s) <= 0; }
	bool	operator==( const CIScEvent &s ) const { return cmp(s) == 0; }
	bool	operator!=( const CIScEvent &s ) const { return cmp(s) != 0; }
	bool	operator>=( const CIScEvent &s ) const { return cmp(s) >= 0; }
	bool	operator> ( const CIScEvent &s ) const { return cmp(s) >  0; }

	tod_t		t;					// time of this event.

	CISlocation		*location;		// location of this event.
	CISroute		*route;			// route of this event.
	CISservice		*service;		// if an arrival or departure, which service.  If wait or transfer, NULL.

	CISarc			*inList, *outList;

	bool hasArrivals() const { return inList != NULL; }
	bool hasDepartures() const { return outList != NULL; }
	bool isTravel() const { return route != NULL; }

	utility_t	d;				// utility distance in shortest path heap.
	tod_t		tCur;			// actual time in the shortest path tree.
	size_t		iheap;			// index in shortest path heap.
	CISarc		*arcToParent;	// arc to parent in shortest path.

	static utility_t dMax() { return (utility_t)(24 * 60 * 60 * 6 * 7); /* 6 weeks should do it */ }

	void	spReset()
	{
		arcToParent = NULL;
		d = dMax();
	}

	void	setDArcToParent( const utility_t aD, CISarc *aArcToParent )
	{
		d = aD;
		arcToParent = aArcToParent;
	}

	CISarc	*findTravelOutArc() const;
	CISarc	*findTravelInArc() const;
};

inline tod_t CISarc::getElapsed()  const { return to->t - from->t; }

inline	CISarc	*CIScEvent::findTravelOutArc() const
{
	register CISarc *a = NULL;
	for( a = outList; a; a = a->outNext )
		if( a->isTravel() )
			break;
	return a;
}

inline	CISarc	*CIScEvent::findTravelInArc() const
{
	register CISarc *a = NULL;
	for( a = inList; a; a = a->inNext )
		if( a->isTravel() )
			break;
	return a;
}

struct CISarcCmpFromTime
{
	bool operator()( const CISarc *a1, CISarc *a2 ) const
	{
		return a1->from->t < a2->from->t;
	}
};

struct CIScEventCmpTimeCISroute
{
	bool operator()( const CIScEvent *s1, const CIScEvent *s2 ) const	{ return *s1 < *s2; }
};

struct CIScEventCmpTime
{
	bool operator()( const CIScEvent *s1, const CIScEvent *s2 ) const
	{ Assert(s1); Assert(s2); return s1->t < s2->t; }
};

struct CIScEventCmpD
{
	bool operator()( const CIScEvent *s1, const CIScEvent *s2 ) const
	{ Assert(s1); Assert(s2); return s1->d < s2->d; }
};

typedef std::set<CIScEvent *, CIScEventCmpTime> CIScEventSet;
typedef std::map<ident_t, CIScEventSet>	CIScRouteIDEventSet;
typedef std::vector<CIScEvent *> CIScEventVector;
typedef std::vector<CIScEventVector> CIScEventVectorVector;

//---------------------------------------------------------------------------------

class CISlocation : public IDName
{
public:
	CISlocation( const ident_t aID, const char *aName, const double aLongitude, const double aLatitude ) :
	  IDName(aID, aName)
	  {
		  longitude = (float)aLongitude;
		  latitude = (float)aLatitude;
		  
		  // Reset the leave/arrive flags and times.
		  isPotentialLeave = isPotentialArrive = false;
		  leaveWalkingTime = arriveWalkingTime = 0;
		  
		  transferGroup = NULL;	// by default, this location is not part of any transfer group.
	  }
	  CISlocation( const CISlocation &loc ) : IDName(loc)
	  {
		  longitude = loc.longitude;
		  latitude = loc.latitude;
		  
		  // Reset the leave/arrive flags and times.
		  isPotentialLeave = isPotentialArrive = false;
		  leaveWalkingTime = arriveWalkingTime = 0;
		  
		  transferGroup = NULL;	// by default, this location is not part of any transfer group.
	  }
	  ~CISlocation();
	  MMdeclare();
	  double getLongitude() const { return (double)longitude; }
	  double getLatitude()  const { return (double)latitude; }
	  
	  void	getCoord( int &x, int &y ) const
	  { Distance::CoordFromLongLat( x, y, (double)longitude, (double)latitude ); }
	  
	  bool  eqLongitudeLatitude( const CISlocation &loc ) const
	  { return longitude == loc.longitude && latitude == loc.latitude; }
	  
	  const	QTree::Point	getPoint() const
	  { int x, y; getCoord(x, y); return QTree::Point(x, y); }
	  
	  float	longitude, latitude;
	  
	  CIScRouteIDEventSet		routeCurb;	// model all curb arrive/leave events for all routes.
	  CIScEventSet				shelter;	// model shelter enter/leave events.

	  // Flags indicating this location is a potential leave or arrive.
	  bool	isPotentialLeave, isPotentialArrive;
	  tod_t	leaveWalkingTime;	// Walking time from the leave point to this location.
	  tod_t	arriveWalkingTime;	// Walking time from this location to the arrive point.
	  
	  CIStransferGroup	*transferGroup;
	  
private:
	CISlocation &operator=( const CISlocation &loc ); // not to be called.
};

struct CISlocationNameCmp
{
	bool operator()( const CISlocation *loc1, const CISlocation *loc2 ) const
	{
		return StrStripCmp(loc1->getName(), loc2->getName()) < 0;
	}
};

inline  const char *CISarc::compassDirection() const
{
	return Distance::compassDirection(	from->location->getLongitude(),	from->location->getLatitude(),
										to->location->getLongitude(),	to->location->getLatitude() );
}

class CIStransferGroup : public IDName
{
public:
	CIStransferGroup( const ident_t aID, const char *aName ) : IDName(aID, aName)
	{}
	MMdeclare();

	void	insert( CISlocation *location )
	{
		if( location->transferGroup != NULL )
			location->transferGroup->locations.remove(location->getID());
		location->transferGroup = this;
		locations.insert( location->getID(), location );
	}
	CIStransferGroup &operator |= ( CIStransferGroup &g );

	CISlocationCollection	locations;
};

extern "C" { typedef struct CISpoint CISpoint; }

class CISsolution
{
	friend class CIS;
	friend struct CIScmpSolutionDecreasing;

	CISsolution( CIScEvent *aParentFinal, const utility_t aDFinal ) : parentFinal(aParentFinal), dFinal(aDFinal) {}
	CISsolution( const CISsolution &cs ) { parentFinal = cs.parentFinal; dFinal = cs.dFinal; }
	CISsolution &operator=( const CISsolution &cs ) { parentFinal = cs.parentFinal; dFinal = cs.dFinal; return *this; }

	void	reset() { parentFinal = NULL; dFinal = (utility_t)INT_MAX/2; }
	void	reset( CIScEvent *aParentFinal, const utility_t aDFinal )
	{
		parentFinal = aParentFinal;
		dFinal = aDFinal;
	}

	MMdeclare();

	CIScEvent		*parentFinal;
	utility_t	dFinal;
};

struct CIScmpSolutionDecreasing
{
	bool	operator()( const CISsolution &s1, const CISsolution &s2 ) const
	{ return s1.dFinal > s2.dFinal; }
};

//---------------------------------------------------------------------------------------
// Define our own NodeEquivalence object to speed access.
// Basically all we do here is read the connections once and insert the equivalent ones
// into a hash table for fast access later.
class CISnodeEquivalences
{
public:
	typedef	HashFI< long, long >		IDHash;
	typedef HashFI< long, IDHash * >	IDIDHash;

	IDIDHash	nodes;

	void	init();
	~CISnodeEquivalences();

	bool	isEquivalent( const long fromNODESrecordID, const long toNODESrecordID ) const
	{
		IDIDHash::iterator n;
		return fromNODESrecordID == toNODESrecordID ||
			((n = nodes.find(fromNODESrecordID)) != nodes.end() && n.data()->contains(toNODESrecordID));
	}

	long getConnectionTime( const long fromNODESrecordID, const long toNODESrecordID ) const
	{
		if( fromNODESrecordID != toNODESrecordID )
		{

			IDIDHash::iterator n = nodes.find(fromNODESrecordID);
			if( n != nodes.end() )
				return n.data()->get( toNODESrecordID, 0 );
		}
		return 0;
	}

	bool	contains( const long fromNODESrecordID, const long toNODESrecordID ) const
	{
		IDIDHash::iterator n = nodes.find(fromNODESrecordID);
		return n != nodes.end() && n.data()->contains(toNODESrecordID);
	}

	void	insert( const long fromNODESrecordID, const long toNODESrecordID, const long connectionTime )
	{
		if( fromNODESrecordID != toNODESrecordID )
		{
			IDIDHash::iterator n = nodes.find(fromNODESrecordID);
			if( n == nodes.end() )
				n = nodes.insert( fromNODESrecordID, new IDHash );
			n.data()->insert( toNODESrecordID, connectionTime );

			n = nodes.find(toNODESrecordID);
			if( n == nodes.end() )
				n = nodes.insert( toNODESrecordID, new IDHash );
			n.data()->insert( fromNODESrecordID, connectionTime );
		}
	}
};

typedef std::vector<tod_t>		TimeVector;
struct CISlocationTimes
{
	CISlocationTimes( CISlocation *aLocation ) : location(aLocation) {}

	CISlocation	*location;
	TimeVector	times;

	tod_t		operator[]( const size_t i ) const { return times[i]; }
	tod_t		&operator[]( const size_t i ) { return times[i]; }
	size_t		size() const { return times.size(); }
};
typedef std::vector<CISlocationTimes>		CISlocationTimesVector;

typedef std::vector< std::pair<CISlocation *, CISlocation *> >	CISfromToVector;
class CISrouteTimesReply
{
public:
	CISservice		*service;
	CISroute		*route;
	CISlocationTimesVector	locationTimes;
	CISfromToVector	fromToConnections;

	const CISlocationTimes		&operator[]( const size_t i ) const { return locationTimes[i]; }
	CISlocationTimes			&operator[]( const size_t i ) { return locationTimes[i]; }
	size_t		size() const { return locationTimes.size(); }

	TiXmlDocument	*toXmlDoc() const;
};

struct CISdriverPlanKey
{
	CISdriverPlanKey( const ident_t aFromNode, const ident_t aToNode, const tod_t aT, const bool aPlanLeave, const ident_t aIdService )
		: fromNode(aFromNode), toNode(aToNode), t(aT), planLeave(aPlanLeave), idService(aIdService) {}

	ident_t	fromNode, toNode;
	tod_t	t;
	bool	planLeave;
	ident_t	idService;

	bool	operator==( const CISdriverPlanKey &dpk ) const
	{
		return
			fromNode	== dpk.fromNode &&
			toNode		== dpk.toNode &&
			t			== dpk.t &&
			planLeave	== dpk.planLeave &&
			idService	== dpk.idService;
	}

	size_t	hash() const { return multi_hash(fromNode, toNode, t, planLeave, idService); }
};
struct CISdriverPlanData
{
	CISdriverPlanData( const tod_t aStartTime = -25*60*60, const tod_t aEndTime = 0, const tod_t aDwellTime = 0 )
		: startTime(aStartTime), endTime(aEndTime), dwellTime(aDwellTime) {}
	tod_t startTime, endTime, dwellTime;
};

typedef HashObj< CISdriverPlanKey, CISdriverPlanData >	CISdriverPlanCache;

class CIS
{
public:
	friend class CISresult;
	friend class CISservice;
	enum
	{
		closestPointMax = 32,
		infeasibleWalkingTime = INT_MAX / 1024
	};

	class Point
	{
	public:
		Point( const double aLongitude, const double aLatitude ) : longitude(aLongitude), latitude(aLatitude) {}
		Point() : longitude(0.0), latitude(0.0) {}
		Point &operator=( const Point &p ) { longitude = p.longitude; latitude = p.latitude; return *this; }
		double longitude, latitude;
	};

	struct Parameters
	{
		double	walkingFactor;
		double	travelFactor;
		tod_t	transferPenalty;
		tod_t	additionalFarePenalty;
		size_t	directWalkingDistanceMax;
		int		hasPass;
		int		driverPlan;
	};

	void	getParameters( Parameters &p ) const
	{
		p.walkingFactor			= walkingFactor;
		p.travelFactor			= travelFactor;
		p.transferPenalty		= transferPenalty;
		p.additionalFarePenalty = additionalFarePenalty;
		p.directWalkingDistanceMax = directWalkingDistanceMax;
		p.hasPass				= hasPass;
		p.driverPlan			= driverPlan;
	}
	void	setParameters( const Parameters &p )
	{
		walkingFactor			= p.walkingFactor;
		travelFactor			= p.travelFactor;
		transferPenalty			= p.transferPenalty;
		additionalFarePenalty	= p.additionalFarePenalty;
		directWalkingDistanceMax = p.directWalkingDistanceMax;
		hasPass					= p.hasPass;
		driverPlan				= p.driverPlan;
	}
	void	restoreDefaultParameters()	{ setParameters( defaultParameters ); }
	void	saveParametersAsDefault()	{ getParameters( defaultParameters ); }
	void	setDefaultParameters( const Parameters &p )	 { defaultParameters = p; }
	void	setDriverParameters();

	CIS();
	~CIS();

	// Allow the penalty parameters to be manipulated.
	// Utility factor for waiting in the system.
	void	setTransferWaitFactor( const double f ) { transferWaitFactor = f; }
	double	getTransferWaitFactor() const { return transferWaitFactor; }

	// Utility factor for walking time.
	void	setWalkingFactor( const double f ) { walkingFactor = f; }
	double	getWalkingFactor() const { return walkingFactor; }

	// Utility factor for travelling on a vehicle.
	void	setTravelFactor( const double f ) { travelFactor = f; }
	double	getTravelFactor() const { return travelFactor; }

	// The minimum time required for a transfer (only used if combined waiting/walking time is small).
	void	setTransferTimeMin( const tod_t t ) { transferTimeMin = t; }
	tod_t	getTransferTimeMin() const { return transferTimeMin; }

	// A fixed time penalty for planning a transfer.
	void	setTransferPenalty( const tod_t t ) { transferPenalty = t; }
	tod_t	getTransferPenalty() const { return transferPenalty; }

	// A fixed time penalty for planning an additional fare (only used if rider does not have a bus pass).
	void	setAdditionalFarePenalty( const tod_t t ) { additionalFarePenalty = t; }
	tod_t	getAdditionalFarePenalty() const { return additionalFarePenalty; }

	// A fixed time to board a vehicle.
	void	setEntryTime( const tod_t t ) { entryTime = t; }
	tod_t	getEntryTime() const { return entryTime; }

	// A fixed time to alight a vehicle.
	void	setExitTime( const tod_t t ) { exitTime = t; }
	tod_t	getExitTime() const { return exitTime; }

	// Indicates the rider has a bus pass - if so, multiple fares will not be considered.
	void	setHasPass( const bool p ) { hasPass = p; }
	bool	getHasPass() const { return hasPass; }

	void	setDriverPlan( const bool f ) { driverPlan = f; }
	bool	getDriverPlan() const { return driverPlan; }

	void	setDirectWalkingDistanceMax( const size_t d ) { directWalkingDistanceMax = d; }
	size_t	getDirectWalkingdistanceMax() const { return directWalkingDistanceMax; }

	// Access functions for building the search graph.
	void	addDirection( const ident_t aID, const char *aName )
	{
		if( !directions.contains(aID) )
		{
			CISdirection *direction = new CISdirection( aID, aName );
			directions.insert( aID, direction );
		}
	}
	void	addService( const ident_t aID, const char *aName )
	{
		if( !services.contains(aID) )
		{
			CISservice *service = new CISservice( aID, aName );
			services.insert( aID, service );
		}
	}
	void	addVehicle( const ident_t aID, const char *aName,
				const bool aIsSurface = true, const bool aIsAccessible = false, const bool aIsBicycle = false )
	{
		if( !vehicles.contains(aID) )
		{
			CISvehicle		*vehicle = new CISvehicle( aID, aName, aIsSurface, aIsAccessible, aIsBicycle );
			vehicles.insert( aID, vehicle );
		}
	}

#define idRouteShift	7	// Handle a maximum of 128 directions.
	ident_t	combineRouteDirectionID( const ident_t idRoute, const ident_t idDirection ) const
	{ return (idRoute << idRouteShift) + idDirection; }

	void splitRouteDirectionID( ident_t &idRoute, ident_t &idDirection, const ident_t idRouteDirection) const
	{
		idRoute = (idRouteDirection >> idRouteShift);
		idDirection = idRouteDirection & ((1<<idRouteShift) - 1);
	}
#undef idRouteShift

	void	addRoute( const ident_t aID, const char *aName, const ident_t idCISservice, const ident_t idCISdirection )
	{
		CISservice		*service	= services.find(idCISservice).data();
		CISdirection	*direction	= directions.find(idCISdirection).data();
		if( !routes.contains(combineRouteDirectionID(aID, idCISdirection)) )
		{
			CISroute		*route		= new CISroute( combineRouteDirectionID(aID, idCISdirection), aName, service, direction );
			routes.insert( route->getID(), route );
		}
	}
	void	addLocation( const ident_t aID, const char *aName, const double longitude, const double latitude )
	{
		if( latitude != 0.0 || longitude != 0.0 )
		{
			if( !locations.contains(aID) )
			{
				CISlocation	*location = new CISlocation( aID, aName, longitude, latitude );
				locations.insert( aID, location );
			}
		}
	}

	void	addTravel( const tod_t fromTime, const ident_t fromLoc, const tod_t toTime, const ident_t toLoc,
        const ident_t idCISroute, const ident_t idCISdirection, const ident_t idTrip, const ident_t idBlock,
        const ident_t idCISvehicle, const ident_t idCISservice );
	void	initSolve();

	void	initBoundaries( istream &isMIF );

	// Plan returns an ascii description.
	// If cisPointArray is NULL, it will be ignored.
	enum PlanStatus
	{
		unknownStatus, good, betterToWalk, noLeaveStop, noArriveStop, noLeaveArriveStop,
		leaveTooEarly, leaveTooLate,
		noService,  // No bus service that day.
		infeasible	// This can only happen if there is no service between the points and
					// the distance between the points exceeds directWalkingDistanceMax.
	};
	static inline int isError(const PlanStatus p)
	{
		return !(p == good || p == betterToWalk);
	}
	enum PlanType
	{
		unknownType, leaveAt, arriveAt, leaveEarliest, leaveLatest
	};

	bool	isValidServiceID( const ident_t serviceID ) const { return services.contains(serviceID); }

	ident_t	getServiceID( const char *name )
	{
		CISserviceCollection::iterator	s;
		ForAllInCollection( services, s )
		{
			if( StrStripCmp(s.data()->getName(), name) == 0 )
				return s.data()->getID();
		}
		return -1;
	}
	ident_t	getFirstServiceID()
	{
		CISserviceCollection::iterator	s = services.begin();
		return s.data()->getID();
	}
	ident_t	findServiceID( const char *name )
	{
		CISserviceCollection::iterator	s;
		ForAllInCollection( services, s )
		{
			if( strstr(name, s.data()->getName()) )
				return s.data()->getID();
		}
		return -1;
	}
	ident_t	getRouteDirectionID( const ident_t serviceID, const char *name, const char *direction )
	{
		CISserviceCollection::iterator	s = services.find( serviceID );
		if( s == services.end() )
			return -1;
		CISrouteCollection::iterator r, rEnd;
		for( r = s.data()->routes.begin(), rEnd = s.data()->routes.end(); r != rEnd; ++r )
		{
			if( StrStripCmp(r.data()->getName(), name) == 0 && StrStripCmp(r.data()->getDirection()->getName(), direction) == 0 )
				return *r;
		}
		return -1;
	}

	ServiceCalendar	*getServiceCalendar() { if( !serviceCalendar ) serviceCalendar = new ServiceCalendar(); return serviceCalendar; }

protected:
		struct LocationNameCmp
		{
			bool operator()( const CISlocation *loc, const char *name ) const
			{ return StrStripCmp(loc->getName(), name) < 0; }
		};
		struct RouteNameCmp
		{
			bool operator()( const CISroute *r, const char *name ) const
			{ return StrStripCmp(r->getName(), name) < 0; }
		};
public:
	void	getLocations( CISlocationVector &locv, const char *name )
	{
		locv.clear();
		if( locationsByName.size() != locations.size() )
		{
			locationsByName.clear();
			locationsByName.reserve( locations.size() );
			CISlocationCollection::iterator loc;
			ForAllInCollection( locations, loc )
				locationsByName.push_back( loc.data() );
			std::sort( locationsByName.begin(), locationsByName.end(), CISlocationNameCmp() );
		}

		register CISlocationVector::iterator found
			= binSearchItr(locationsByName.begin(), locationsByName.end(), name, LocationNameCmp() );
		while( found != locationsByName.begin() && StrStripCmp((*(found-1))->getName(), name) == 0 )
			--found;

		while( found != locationsByName.end() && StrStripCmp((*found)->getName(), name) == 0 )
		{
			locv.push_back( *found );
			++found;
		}
	}

	void getLocationIDs( std::vector<ident_t> &idv, const char *name )
	{
		idv.clear();
		CISlocationVector	locv;
		getLocations( locv, name );
		if( !locv.empty() )
		{
			idv.reserve( locv.size() );
			for( register CISlocationVector::const_iterator loc = locv.begin(), locEnd = locv.end(); loc != locEnd; ++loc )
				idv.push_back( (*loc)->getID() );
		}
	}

	const CISlocation	*getLocation( const ident_t idLocation )
	{
		CISlocationCollection::iterator loc = locations.find(idLocation);
		return loc != locations.end() ? loc.data() : NULL;
	}

	CISservice *getService( const ident_t idService )	 { return services[idService]; }

	void	getServices( CISserviceVector &sv )
	{
		sv.clear();
		sv.reserve( services.size() );
		CISserviceCollection::iterator	s;
		ForAllInCollection( services, s )
			sv.push_back( s.data() );
		std::sort( sv.begin(), sv.end(), CISserviceCmp() );
	}
	
	size_t	getNumServices() const { return services.size(); }

	const char *getServiceName( const ident_t idService )	const { return services.find(idService).data()->getName(); }
	void	getRoutes( StringVector	&rv, const ident_t serviceID )
	{
		rv.clear();
		CISserviceCollection::iterator	s = services.find( serviceID );
		if( s == services.end() )
			return;
		rv.reserve( s.data()->routes.size() );

		CISrouteCollection::iterator r, rEnd;
		for( r = s.data()->routes.begin(), rEnd = s.data()->routes.end(); r != rEnd; ++r )
			rv.push_back( r.data()->getName() );

		std::sort( rv.begin(), rv.end() );
		rv.erase( std::unique(rv.begin(), rv.end()), rv.end() );
	}
	void	getDirections( StringVector	&dv, const ident_t serviceID, const char *routeName )
	{
		dv.clear();
		CISserviceCollection::iterator	s = services.find( serviceID );
		if( s == services.end() )
			return;
		CISservice *service = s.data();

		if( service->routesByName.size() != service->routes.size() )
		{
			service->routesByName.clear();
			service->routesByName.reserve( service->routes.size() );
			CISrouteCollection::iterator r;
			ForAllInCollection( service->routes, r )
				service->routesByName.push_back( r.data() );
			std::sort( service->routesByName.begin(), service->routesByName.end(), CISrouteCmp() );
		}

		register CISrouteVector::iterator found
			= binSearchItr(service->routesByName.begin(), service->routesByName.end(), routeName, RouteNameCmp() );

		if( found == service->routesByName.end() )
			return;

		while( found != service->routesByName.begin() && StrStripCmp((*(found-1))->getName(), routeName) == 0 )
			--found;

		dv.reserve( 4 );
		while( found != service->routesByName.end() && StrStripCmp((*found)->getName(), routeName) == 0 )
		{
			dv.push_back( (*found)->getDirection()->getName() );
			++found;
		}

		std::sort( dv.begin(), dv.end() );
		dv.erase( std::unique(dv.begin(), dv.end()), dv.end() );
	}
	void	getRouteStops( CISlocationVector &locv, const ident_t serviceID, const ident_t routeDirectionID );
	void	getFromToStops( CISlocationMapSet &fromTo, const ident_t serviceID, const ident_t routeDirectionID );

	PlanStatus	plan(	const char *szLeave, const Point leave,
						const long fromNODESrecordID,	// only used in driverPlan mode
						const char *szArrive, const Point arrive,
						const long toNODESrecordID,		// only used in driverPlan mode
						const tod_t t,
						const int leaveFlag,
						const ident_t idCISservice,
						const CISvehicleFlags vehicleFlags,
						CISresultCollection	&resultCollection /* return */ );

	PlanStatus	plan(	const ident_t fromNodeID, const ident_t toNodeID, const tod_t t,
						const int planLeave, const ident_t idCISservice,
						CISresultCollection &result /* return */ );

	CISrouteTimesReply	*getRouteTimesReply( const ident_t serviceID, const ident_t routeDirectionID );
	void				getRouteStopTimes( TripPlanRequestReply::TimeOfDayVector &todv, const ident_t serviceID, const ident_t routeDirectionID, const ident_t locationID );

	CISdriverPlanCache	driverPlanCache;

private:
	ServiceCalendar	*serviceCalendar;

	Parameters	defaultParameters;

	typedef std::vector<QTree::PointID>	QTreePointIDVector;
	void	filterPoints( const QTree::Point &p, QTree::PointID *closest, size_t &numClosest );

	PairingHeap< CISsolution, CIScmpSolutionDecreasing >	solutionHeap;
	size_t	solutionMax;

	int	transferDistance;
	tod_t	transferTimeMin;
	tod_t	entryTime, exitTime;
	tod_t	transferPenalty;
	tod_t  additionalFarePenalty;

	int walkingRadius;		// walking radius used to find stops.
	double walkingSpeed;	// walking speed (meters/sec)

	double	travelFactor;
	double	transferWaitFactor;
	double	walkingFactor;

	size_t	directWalkingDistanceMax;	// maximum direct walking distance (meters) that will be considered.

	utility_t walkingUtility( const tod_t walkingTime ) const { return (utility_t)(walkingTime * walkingFactor); }

	utility_t leaveTransferExpireUtility( const CISarc *a ) const
	{
		// If the customer does not have a transit pass and
		// if this is an entry arc and the transfer time has expired, account for the additional fare. 
		return !hasPass &&
			a->from->tCur >= service->transferTimeMax &&
			a->from->location == a->to->location &&
			a->from->route == NULL && a->to->route != NULL ? (utility_t)additionalFarePenalty : (utility_t)0;
	}

	utility_t elapsedUtility( const CISarc *a ) const
	{
		register utility_t u = (utility_t)a->getElapsed();
		switch( a->type )
		{
		case CISservice::Wait:
			u = (utility_t)(u * transferWaitFactor);
			break;
		case CISservice::Travel:
		case CISservice::Dwell:
			u = (utility_t)(u * travelFactor);
			break;
		case CISservice::Enter:
		case CISservice::Exit:
			u += (utility_t)transferPenalty / 2;
			break;
		case CISservice::WalkTransfer:
			// Penalize the walking and waiting time.
			const	tod_t	walkingTime
				= (tod_t)(a->from->location->getPoint().distance( a->to->location->getPoint() ) * walkingSpeed);
			const	tod_t	waitingTime = (tod_t)u - walkingTime;
			u = (utility_t)(waitingTime * transferWaitFactor + walkingTime * walkingFactor) +
				// Add a tiny bias to make transfers earlier - all things being equal.
				(utility_t)(1.0 / (48.0*60.0*60.0) * a->to->t);
			break;
		}
		return u;
	}
	char	*compassDirection( char *szDirection, size_t, const char *szFormat, const QTree::Point &p1, const QTree::Point &p2 );
	static TripPlanRequestReply::Direction::CompassPoint compassPoint( const QTree::Point &p1, const QTree::Point &p2 )
	{
		switch( Distance::compassPoint(p1.x, p1.y, p2.x, p2.y) )
		{
		case Distance::Northeast:	return TripPlanRequestReply::Direction::Northeast;
		case Distance::North:		return TripPlanRequestReply::Direction::North;
		case Distance::Northwest:	return TripPlanRequestReply::Direction::Northwest;
		case Distance::West:		return TripPlanRequestReply::Direction::West;
		case Distance::Southwest:	return TripPlanRequestReply::Direction::Southwest;
		case Distance::South:		return TripPlanRequestReply::Direction::South;
		case Distance::Southeast:	return TripPlanRequestReply::Direction::Southeast;
		case Distance::East:		return TripPlanRequestReply::Direction::East;
		}
		return TripPlanRequestReply::Direction::Directly;
	}

	CISservice	*service;	// CISservice being searched.
	void	spLeave( const CISarcFlags arcFlags );
	void	spArrive( const CISarcFlags arcFlags );

	CISlocationCollection	locations;
	CISlocationVector		locationsByName; // Used to find locations by name quickly.
	CISdirectionCollection	directions;
	CISvehicleCollection	vehicles;
	CISserviceCollection	services;
	CISrouteCollection		routes;

	// Shortest path heap functions.
	bool	hasPass;	// flag indicating the user has a transit pass.
	void	spReset();
	void	buildHeap();
	CIScEvent	*extractMin();
	void	insert( CIScEvent *s );
	void	insertInitial( CIScEvent *s );

	void	resizeHeap();
	CIScEvent	**heap;

  ofstream  *os;  // Used for debugging.

	size_t	N;		  // number of elements in the heap.
	size_t	highestN; // highest index for elements visited in the heap.
	size_t	NMax;	  // size of the heap.

	// A flag to indicate we are planning reliefs for drivers only.
	bool	driverPlan;

	// Blocking lines.
	Trapezoidal	boundaries;

  // Keep track of node equivalences as defined in the database.
  // Also, automatically create node equivalences between nodes that have the same lat/long.
	CISnodeEquivalences	nodeEquivalences;

	tod_t	walkingDistanceToTime( const size_t distance ) const { return (tod_t)(distance / walkingSpeed); }
	size_t	walkingTimeToDistance( const tod_t t ) const { return (size_t)(t * walkingSpeed); }
};

class CISresult
{
	friend class CIS;
	friend class CISresultCollection;
public:
	CISresult() { reset(); }
	MMdeclare();

	void	addCISarc( CISarc *a ) { arcList.insert(a); }

	void	reset()
	{
		// Plan/cost data.
		cis = NULL;
		driverPlan = false;
		fromNODESrecordID = toNODESrecordID = -1;
		sLeave = sArrive = "";
		status = CIS::unknownStatus;
		type = CIS::unknownType;
		dFinal = (utility_t)CIS::infeasibleWalkingTime;
		arcList.clear();
		t = leaveTime = arriveTime = directWalkingTime = 0;
		creationTime = 0;
		hasPass = false;

		// Metrics.
		travelTime = 0;
		numTransfers = 0;
		walkingDistance = 0;
		walkingTime = 0;
		dwellTime = 0;
		numFares = 1;

		// Formatting
		distanceUnit = "m";
		distanceFactor = 1.0;
		decimals = 0;
	}

	char	*format( char *szPlan, size_t szPlanSize, CISpoint **pCISpointArray, int *pNumCISpoints, const CISmessageMap &mm );
	static char	*getTimestamp( char *szTimestamp, const size_t tsSize );

	TripPlanRequestReply::TripPlan *toTripPlan();
	
	tod_t	getDirectWalkingTime() const { return directWalkingTime; }
	tod_t	getWalkingTime() const { return walkingTime; }
	tod_t	getDwellTime() const { return dwellTime; }
	tod_t	getTravelTime() const { return travelTime; }
	tod_t	getLeaveTime() const { return leaveTime; }
	tod_t	getArriveTime() const { return arriveTime; }

	size_t	getNumTransfers() const { return numTransfers; }
	size_t	getWalkingDistance() const { return walkingDistance; }
	size_t	getNumFares() const { return numFares; }

	void	setType( const CIS::PlanType t ) { type = t; }
	void	setT( const tod_t aT ) { t = aT; }
	void	setHasPass( const int aHasPass = 1 ) { hasPass = aHasPass; }
	void	setStatus( const CIS::PlanStatus s ) { status = s; }
	CIS::PlanStatus	getStatus() const { return status; }
	void	setService( CISservice *s ) { service = s; }

	utility_t	getUtility() const { return dFinal; }	// Return final fitness of the trip plan.

private:
	void	init();

	const	char	*getDirection( const char *szDirection, const CISmessageMap &mm ) const;

	TString	distanceUnit;	// units of distance (m, km, ft, yards, miles).
	double	distanceFactor;	// distance multiple.
	int		decimals;		// number of decimals in distance.

	void	setDistanceFactor( const CISmessageMap &mm );
	const	char	*formatDistance( const size_t distance ) const;

	TString	sLeave, sArrive;	// names of leave and arrive points.
	CIS::Point	leave, arrive;	// locations of leave and arrive points.

	CIS		*cis;			// customer information instance.

	CIS::PlanStatus	status;	// status of trip plan
	CIS::PlanType	type;	// type of trip plan

	bool	driverPlan;		// True if this is a driver plan result, False if a regular trip plan.

	CISservice		*service;	// requested service

	bool		hasPass;	// indicates the customer has a bus pass.
	utility_t	dFinal;		// utility of trip.
	tod_t	t;				// requested leave or arrive time.  Not valid if request is leaveEarliest or leaveLatest.
	tod_t	directWalkingTime;	// time to walk directly from origin to destination
	tod_t	walkingTime;		// total walking time in the plan
	tod_t	dwellTime;			// amount of time early if this is an arrive plan, amount of time to wait if this is a leave plan
	tod_t	leaveTime, arriveTime;	// actual leave and arrive time of planned trip
	time_t	creationTime;	// time this request was generated

	// Metrics.
	tod_t	travelTime;			// total travel time of the trip.
	size_t	numTransfers;		// number of transfers
	size_t	walkingDistance;	// total walking distance required of this trip.
	size_t	numFares;			// fares required for the trip

	long	fromNODESrecordID, toNODESrecordID;	// used if this is a driver relief.

	// CISarcs used in the trip.
	CISarcList	arcList;
};

class CISresultCollection
{
	friend class CIS;
public:
	CISresultCollection( size_t aMax = 4 )
	{
		if( aMax < 1 )
			aMax = 1;

		results = new Ref<CISresult> [max = aMax];
		validMax = 0;
		for( register size_t i = 0; i < max; ++i )
			results[i] = new CISresult;
	}
	~CISresultCollection()
	{
		for( register size_t i = 0; i < max; ++i )
			results[i] = NULL;
		delete [] results;
	}
	void	reset() { for( register size_t i = 0; i < max; i++ ) results[i]->reset(); }
	void	setDriverPlan( const int aDriverPlan )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->driverPlan = (aDriverPlan != 0);
	}
	void	setLeave( const CIS::Point &aLeave, const TString &aSLeave )
	{
		for( register size_t i = 0; i < max; i++ )
		{
			results[i]->leave = aLeave;
			results[i]->sLeave = aSLeave;
		}
	}
	void	setArrive( const CIS::Point &aArrive, const TString &aSArrive )
	{
		for( register size_t i = 0; i < max; i++ )
		{
			results[i]->arrive = aArrive;
			results[i]->sArrive = aSArrive;
		}
	}
	void	setHasPass( const int aHasPass )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->hasPass = aHasPass;
	}
	void	setType( const CIS::PlanType aType )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->type = aType;
	}
	void	setCIS( CIS *aCIS )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->cis = aCIS;
	}
	void	setStatus( const CIS::PlanStatus aStatus )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->status = aStatus;
	}
	void	setT( const tod_t aT )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->t = aT;
	}
	void	setDirectWalkingTime( const size_t aDirectWalkingTime )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->directWalkingTime = aDirectWalkingTime;
	}
	void	setService( CISservice *aCISservice )
	{
		for( register size_t i = 0; i < max; i++ )
			results[i]->service = aCISservice;
	}
	void	setNODESrecordID( long aFromNODESrecordID, long aToNODESrecordID )
	{
		for( register size_t i = 0; i < max; i++ )
		{
			results[i]->fromNODESrecordID	= aFromNODESrecordID;
			results[i]->toNODESrecordID		= aToNODESrecordID;
		}
	}

	void	set( const size_t i, const Ref<CISresult> r ) { results[i] = r; }

	Ref<CISresult>	operator[]( const size_t i ) const { return results[i]; }
	size_t	size() const { return validMax; }
	size_t	sizeMax() const { return max; }

	TripPlanRequestReply::TripPlanReply *toTripPlanReply();

private:
	Ref<CISresult>	*results;
	size_t		validMax, max;
	void	setValidMax()
	{
		if( CIS::isError(results[0]->getStatus()) )	// make sure the first error is reported.
			validMax = 1;
		else
		{
			for( validMax = 1; validMax < max; ++validMax)
				if( results[validMax]->getStatus() == CIS::unknownStatus )
					break;
		}
	}
};

class CISmessageMap
{
public:
	CISmessageMap() {}

	void	read( std::istream &is );
	void	insert( const TString &key, const TString &value ) { strHash.insert(key, value); }
	void	remove( const TString &key ) { strHash.remove(key); }
	const	char	*find( const TString &key ) const
	{
		TStringHash::iterator i = strHash.find(key);
		return i != strHash.end() ? i.data().getStr() : NULL;
	}

private:
	CISmessageMap &operator=( const CISmessageMap &mm ); // stays private.
	TStringHash	strHash;
};

class CIStranslate
{
public:
	CIStranslate();
	char	*translate( char *szOut, const char *szIn, const CISmessageMap &mm );

private:
	struct	SymbolValueElement
	{
		SymbolValueElement( const TString &aSymbol, const TString &aValue = TString(NULL) ) : symbol(aSymbol), value(aValue), next(NULL) {}

		TString	symbol, value;
		SymbolValueElement	*next;

		void	addToList( SymbolValueElement *&head )
		{
			next = head;
			head = this;
		}

		MMdeclare();
	};

	TStringHash	symbols;
	size_t	iStack;
	SymbolValueElement	*symbolStackPtr[8];
	SymbolValueElement	*symbolStack;

	void	registerSymbolValue( const TString &symbol, const TString &value )
	{
		TStringHash::iterator s = symbols.find(symbol);
		if( s != symbols.end() )
			(new SymbolValueElement(*s, s.data()))->addToList(symbolStack);
		else
			(new SymbolValueElement(symbol))->addToList(symbolStack);
		symbols.insert( symbol, value );
	}

};

#endif // cis_H
