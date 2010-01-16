//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "snbuild.h"
#include "snbuildp.h"
#include "Garages.h"

//#define BLOCKDEBUG

#ifdef D
#undef D
#endif

#ifdef BLOCKDEBUG
#include <fstream>
#define D(x)	x

using namespace std;

ofstream *blockLog = NULL;
#define log	(*blockLog)
static const char *logFile = "C:\\Projects\\tms\\block.txt";

class PLong
{
public:
	PLong( long aL = 0, int aWidth = 0 ) { l = aL; width = aWidth; }
	long	l;
	int		width;
};

static	ostream &operator<<( ostream &os, const PLong &fl )
{
	char	sz[64];
	char	*pch = &sz[sizeof(sz)];
	const   bool isNeg = (fl.l < 0);
	int		p = isNeg ? 1 : 0;
	long	l = isNeg ? -fl.l : fl.l;

	*(--pch) = 0;
	do
	{
		*(--pch) = '0' + (l % 10);
		l /= 10;
		p++;
	} while( l > 0 );

	for( ; p < fl.width; p++ )
		*(--pch) = '0';

	if( isNeg )
		*(--pch) = '-';

	return os << pch;
}

class PTime
{
public:
	PTime( long aT = 0 ) { t = aT; }
	long	t;
};

static	ostream &operator<<( ostream &os, const PTime &pt )
{
	long	hour   = pt.t / (60*60);
	long	minute = (pt.t / 60) % 60;
	long	second = pt.t % 60;

	os << PLong(hour, 2) << ':';
	os << PLong(minute, 2) << ':';
	os << PLong(second, 2);
	return os;
}

static ostream &operator<<( ostream &os, const SNtrip &t )
{
	os << PLong(t.getBlockNumber(), 4) << ' ';
	os << PLong(t.getTripID(), 4) << ' ';
	os << PLong(t.getStartNode(), 4) << ' ';
	os << PTime(t.getStartTime()) << ' ';
	os << PTime(t.getEndTime()) << ' ';
	os << PLong(t.getEndNode(), 4) << '\n';
	return os;
}

#else
#define D(x)
#endif

// Initialize the penalties for the blocker.
static const long tripPenalty = Hour(2*48);
static const long vehiclePenalty = Hour(50);
static const long penaltyMax = Hour(48);
static const long layoverMax = Hour(48);
static const double quadFactor = penaltyMax / ((double)layoverMax * (double)layoverMax);

static const long deadheadPenalty = Minute(0);  //was 10
static const long pullinPenalty = Minute(30);   //was 15
static const long pulloutPenalty = Minute(30);  //was 15

SNbuild::SNbuild()
{
}


SNbuild::~SNbuild()
{
	deleteMapData( tripNodes );
}

void SNbuild::addBusType( const long busType )
{
	if( !busTypes.insert(busType).second )
		return; // already added.

	D( log << "Adding busType: " << busType << '\n' );

	SNgarages::iterator ig;
	ForAllInCollection( garages, ig )
		(*ig).second->addBusType( busType );
}

void SNbuild::addGarage( const long id )
{
	if( !(garages.find(id) == garages.end()) )
   		return; // already added.

	D( log << "Adding garage: " << id << '\n' );

	SNdepot	*d = new SNdepot;

	// Add all the busTypes to this depot.
	SNbusTypes::iterator	ib;
	ForAllInCollection( busTypes, ib )
		(*d).addBusType( *ib );

	// Remember - a garage is also a tripNode.
	// It's virtual destructor will be called automatically when the tripNodes
	// are deleted.
	SNgarages::iterator g = garages.insert( SNgarage(id,d) ).first;
	tripNodes.insert( SNtripNode(id,(*g).second) );
}

inline long getTime( const SNevents::iterator &i ) { return (*i).first; }
inline SNtrip &getTrip( const SNevents::iterator &i ) { return *((*i).second.trip); }
inline SNnode *&getNode( const SNevents::iterator &i ) { return (*i).second.node; }
inline SNnode *&getNode( const SNevents::reverse_iterator &i ) { return (*i).second.node; }

inline SNevents &getLeave( const SNtripNodes::iterator &i ) { return (*i).second->leave; }
inline SNevents &getArrive( const SNtripNodes::iterator &i ) { return (*i).second->arrive; }

void SNbuild::addTrip( TRIPINFODef *pt )
{
	SNtrip	*t = new SNtrip(pt);
	SNtripNodes::iterator	itn;

	D( log << "Added trip: " << *t );

	if( (itn = tripNodes.find(t->getStartNode())) == tripNodes.end() )
		itn = tripNodes.insert( SNtripNode(t->getStartNode(),new SNlocation) ).first;

	SNevents::iterator	eStart
   		= getLeave(itn).insert( SNevent(t->getStartTime(), t) );
	getNode(eStart) = network.addNode( (ident_t)t );

	if( (itn = tripNodes.find(t->getEndNode())) == tripNodes.end() )
		itn = tripNodes.insert( SNtripNode(t->getEndNode(),new SNlocation) ).first;

	SNevents::iterator	eEnd
   		= getArrive(itn).insert( SNevent(t->getEndTime(), t) );
	getNode(eEnd) = network.addNode( (ident_t)t );

	network.addArc( getNode(eStart), getNode(eEnd), -tripPenalty );
}

void SNbuild::addWaits( const bool zeroCostLayovers )
{
	// For all compatible trips, connect arrive events to leave events
    // at the same location.
	SNtripNodes::iterator	from;
	ForAllInCollection( tripNodes, from )
	{
		SNevents::iterator	a;
		ForAllInCollection( getArrive(from), a )
		{
	       	SNtripNodes::iterator	to = tripNodes.find( getTrip(a).getEndNode() );

		   	SNevents::iterator l = getLeave(to).lower_bound( getTrip(a).getEarliestLeaveTime() );
       		SNevents::iterator lEnd = getLeave(to).upper_bound( getTrip(a).getLatestLeaveTime() );

			for( ; l != lEnd; ++l )
			{
				if( !getTrip(a).compatible( getTrip(l) ) )
					continue;
				long cost;
				if( !zeroCostLayovers )
				{
					const long waitTime = abs( (*l).first - (*a).first );
					const long penaltyTime = (long)((double)waitTime * (double)waitTime * quadFactor);
					cost = waitTime + penaltyTime;
				}
				else
					cost = SNzeroCostLayover;
				network.addArc( getNode(a), getNode(l), cost, 2 );
				D( log << "Added wait at node " << PLong(getTrip(a).getEndNode(),4)
					<< " from time " << PTime(getTime(a))
					<< " to time " << PTime(getTime(l)) << '\n' );
			}
		}
	}
}

void SNbuild::addDeadhead(const long startNODE, const long endNODE,
						  long start, long end, long dur )
{
	// For all compatible trips, connect arrive events to leave events at
    // different locations.
	fixMinMax( start, end );
	if( dur == NO_TIME )
		dur = Hour(48);

	SNtripNodes::iterator	from = tripNodes.find(startNODE);
	SNtripNodes::iterator	to   = tripNodes.find(endNODE);

	if( from == tripNodes.end() || to == tripNodes.end() )
		return;

	SNevents::iterator	a    = getArrive(from).lower_bound(start);
	SNevents::iterator  aEnd = getArrive(from).upper_bound(end);

	for( ; a != aEnd; ++a )
	{
   		SNevents::iterator	l  = getLeave(to).lower_bound(getTime(a)+dur);
		SNevents::iterator	lEnd
       		= getLeave(to).upper_bound(getTime(a) + dur + getTrip(a).getLayoverMax());

		for( ; l != lEnd; ++l )
		{
			if( !getTrip(a).compatible( getTrip(l) ) )
				continue;
			const long waitTime = abs( getTime(a) - getTime(l) - dur );
			const long penaltyTime = (long)((double)waitTime * (double)waitTime * quadFactor);
			// Add an extra 10 minute penalty to deadheads to ensure that
			// waiting time is prefered over taking a deadhead.
			const long cost = dur + waitTime + penaltyTime + deadheadPenalty;
			network.addArc( getNode(a), getNode(l), cost, 2 );
			D( log << "Added deadhead from node " << PLong(startNODE,4) << " to node " << PLong(endNODE,4)
				<< " at time " << PTime(getTrip(a).getStartTime()) << '\n');
		}
	}
}

void SNbuild::addEquivalence( const long startNODE, const long endNODE, long dur )
{
	// For all compatible trips, connect arrive events to leave events at
    // different locations for equivalences.

	if( dur == NO_TIME )
		dur = Hour(48);

	SNtripNodes::iterator	from = tripNodes.find(startNODE);
	SNtripNodes::iterator	to   = tripNodes.find(endNODE);

	if( from == tripNodes.end() || to == tripNodes.end() )
		return;

	SNevents::iterator	a;
	ForAllInCollection( getArrive(from), a )
	{
   		SNevents::iterator	l  = getLeave(to).lower_bound(getTime(a)+dur);
		SNevents::iterator	lEnd
       		= getLeave(to).upper_bound(getTime(a) + dur + getTrip(a).getLayoverMax());

		for( ; l != lEnd; ++l )
		{
			if( !getTrip(a).compatible( getTrip(l) ) )
				continue;
			const long waitTime = abs( getTime(a) - getTime(l) - dur );
			const long penaltyTime = (long)((double)waitTime * (double)waitTime * quadFactor);
			const long cost = dur + waitTime + penaltyTime + deadheadPenalty / 10;
			network.addArc( getNode(a), getNode(l), cost, 2 );
			D( log << "Added equivalent deadhead from node " << PLong(startNODE,4) << " to node " << PLong(endNODE,4)
				<< " at time " << PTime(getTrip(a).getStartTime()) << '\n');
		}
	}
}

void SNbuild::addEquivalences()
{
	CISnodeEquivalences	nodeEquivalences;

	CISnodeEquivalences::IDIDHash::iterator nFrom;
	ForAllInCollection( nodeEquivalences.nodes, nFrom )
	{
		CISnodeEquivalences::IDHash::iterator nTo;
		ForAllInCollection( *nFrom.data(), nTo )
			addEquivalence( *nFrom, *nTo, nTo.data() );
	}
}

inline SNbusPools &getBusPools( const SNgarages::iterator i ) { return (*i).second->busPools; }

inline long getBusType( const SNbusPools::iterator i ) { return (*i).first; }
inline SNevents &getEvents( const SNbusPools::iterator i ) { return *((*i).second); }

void SNbuild::addPullout(const long startNODE, const long endNODE,
						 long start, long end, long dur )
{
	fixMinMax( start, end );
	if( dur == NO_TIME )
		dur = Hour(48);

	SNgarages::iterator		garage = garages.find( startNODE );
	SNtripNodes::iterator 	to     = tripNodes.find( endNODE );

	if( garage == garages.end() || to == tripNodes.end() )
   		return;

  	SNevents::iterator	l  		= getLeave(to).lower_bound(start);
	SNevents::iterator	lEnd 	= getLeave(to).upper_bound(end);

	for( ; l != lEnd; ++l )
	{
		SNbusPools::iterator	p;
		ForAllInCollection( getBusPools(garage), p )
		{
       		if( !getTrip(l).compatibleVehicle(getBusType(p)) )
           		continue;
			// Try to find a pullout event at the bus holder.  Create it if one
			// does not exist.
			const long pulloutTime = getTrip(l).getStartTime() - dur;
			SNevents::iterator	pullout = getEvents(p).find( pulloutTime );
			if( pullout == getEvents(p).end() )
			{
           		pullout = getEvents(p).insert( SNevent(pulloutTime, SNnetNode()) );
				getNode(pullout) = network.addNode( (ident_t)(*garage).second );
			}
			// Penalize pullouts by an extra 5 minutes.
			network.addArc( getNode(pullout), getNode(l), dur + pulloutPenalty, 2 );
		}
    }
}

void SNbuild::addPullin(const long startNODE, const long endNODE,
						 long start, long end, long dur )
{
	fixMinMax( start, end );
	if( dur == NO_TIME )
		dur = Hour(48);

	SNtripNodes::iterator	from	= tripNodes.find( startNODE );
	SNgarages::iterator 	garage  = garages.find( endNODE );

	if( from == tripNodes.end() || garage == garages.end() )
   		return;

  	SNevents::iterator	a  		= getArrive(from).lower_bound(start);
	SNevents::iterator	aEnd 	= getArrive(from).upper_bound(end);

	for( ; a != aEnd; ++a )
	{
		SNbusPools::iterator p;
		ForAllInCollection( getBusPools(garage), p )
		{
       		if( !getTrip(a).compatibleVehicle(getBusType(p)) )
           		continue;
			const long pullinTime = getTrip(a).getEndTime() + dur;
			SNevents::iterator	pullin = getEvents(p).find( pullinTime );
			if( pullin == getEvents(p).end() )
			{
           		pullin = getEvents(p).insert( SNevent(pullinTime, SNnetNode()) );
				getNode(pullin) = network.addNode( (ident_t)(*garage).second );
			}
			// Penalize pullins by an extra 20 minutes.
			network.addArc( getNode(a), getNode(pullin), dur + pullinPenalty, 2 );
		}
	}
}

void	SNbuild::addFinal( const long vehicleMin, const long vehicleMax )
{
	source	= network.addNode( 3 );
	sink 	= network.addNode( 4 );

	// Add the cycle arc determining the cost and number of available vehicles.
	network.addArc( sink, source, vehiclePenalty, vehicleMax > 0 ? vehicleMax : 100000 );

	// If there is a minimum number of buses to use, make sure we use them.
	if( vehicleMin > 0 )
		network.addArc( sink, source, -vehiclePenalty, vehicleMin );

	SNgarages::iterator garage;
	ForAllInCollection( garages, garage )
	{
		SNbusPools::iterator p;
		ForAllInCollection( getBusPools(garage), p )
		{
       		if( getEvents(p).empty() )
           		continue;
	   		// Connect the bus holders to the source and sink.
			network.addArc( source, getNode(getEvents(p).begin()), 1, 100000 );
			network.addArc( getNode(getEvents(p).rbegin()), sink, 1, 100000 );

			// Connect all the bus holder events together.
			SNevents::iterator e = getEvents(p).begin();
			SNevents::iterator eNext = e;
			for( ++eNext; eNext != getEvents(p).end(); ++e, ++eNext )
           		network.addArc( getNode(e), getNode(eNext), 1, 100000 );
		}
	}
}

inline SNobj *validObj(ident_t t) { return (SNobj *)(t < 20 ? 0 : t); }

#ifdef BLOCKDEBUG
#include <strstrea.h>
char	*formatObj(const ident_t t)
{
	static	char	s[512];
	ostrstream	st(s, sizeof(s));

	SNtrip	*trip;
	SNobj	*o = validObj(t);
	switch( o->getType() )
	{
	case SNobj::trip:
		trip = static_cast<SNtrip *>(o);
		st << "trip: " << *trip;
		break;
	default:
		st << t;
		break;
	}
	st << '\0';
	return st.str();
}
#endif

void	SNbuild::decompose()
{
	register SNarc*	a, *aNext;
	register SNnode *from, *to;
	long	numTrip = 0L, blockNumber = 0L;
	bool	wasInGarage = true, tripSwitch = false;

	D( log << "Decompose:\n" );

	// Assign a block number to all trips.

	// This code is subtle.  The network structure forms a great cycle
	// where all trips are connected from a source node to a sink node
	// and the sink node is finally connected back to the source.
	// This allows us to keep following flows around the cycles until
	// there is no flow left.  All we need to do is keep track of whether
	// we came from a garage.
	for( from = source, wasInGarage = true; ; from = to )
	{
		// Find a non-zero out arc.
		for( a = from->firstOutArc(); a != NULL; a = aNext )
		{
			aNext = a->nextOutArc();
			if( a->getFlow() > 0 )
				break;
			else
				network.deleteArc( a ); // Delete arcs with 0 flow to improve performance on the next scan.
		}
		if( a == NULL )
			break;
		
		a->decrementFlow();
		to = a->getTo();
		SNobj	*o = validObj(to->getID());
		if( o != NULL && o->getType() == SNobj::trip )
		{
			if( wasInGarage )
			{
				// If our last visit was a garage, we are pulling out a new block.
				// Increment the block number.
				++blockNumber;
				wasInGarage = false;
			}
			// We actually traverse the trip twice - once for each from and to node
			// of the trip arc.
			// This toggle ensures that we only update the block number once for each
			// trip.
			if( (tripSwitch = !tripSwitch) == true )
			{
				// Assign the current block number to this trip..
				SNtrip *t = static_cast<SNtrip *>(o);
				t->blockNumber = blockNumber;

				D( log << *t );
				// Stamp this trip with this block number.
				m_pBLOCKSDATA[numTrip].TRIPSrecordID = t->getTripID();
				m_pBLOCKSDATA[numTrip].blockNumber = blockNumber;
				++numTrip;
			}
		}
		else
			wasInGarage = true; // Keep track that we have pulled into a garage. 
	}
}

//---------------------------------------------------------------------------

extern "C"
{
static	Garages	*gs = NULL;
static SNbuild *snb = NULL;
void	SNbuildProlog()
{
	D( blockLog = new ofstream(logFile) );

	snb = new SNbuild;

	// Make sure that all the garages are added before any trip nodes.
	if( gs == NULL )
		gs = new Garages;
	gs->init();
	for( int i = 0;i < gs->getNumGarages(); i++ )
		snb->addGarage( (*gs)[i] );
}

void SNbuildAddTrip( TRIPINFODef *pt )
{
	snb->addBusType(pt->BUSTYPESrecordID);
	snb->addTrip(pt);
}

void SNbuildAddDeadhead( long startNODE, long endNODE,
						  long start, long end, long dur )
{
	snb->addDeadhead(startNODE, endNODE, start, end, dur );
}

inline long	getNodeID( SNgarages::iterator &g ) { return (*g).first; }
inline long getTime( SNevents::iterator &e ) { return (*e).first; }

static void Crash() { int a = 10, b = 0, c = a / b; }

static void	SNbuildAddPullout(SNtripNodes::iterator &to, SNevents::iterator &event,
							  SNgarages::iterator &garage, SNbusPools::iterator &p,
							  const long dur )
{
	const long pulloutTime = getTime(event) - dur;
	SNevents::iterator	pullout = getEvents(p).find( pulloutTime );
	if( pullout == getEvents(p).end() )
	{
		pullout = getEvents(p).insert( SNevent(pulloutTime, SNnetNode()) );
		getNode(pullout) = snb->network.addNode( (ident_t)(*garage).second );
	}
	snb->network.addArc( getNode(pullout), getNode(event), dur + pulloutPenalty, 2 );
	D( log << "Added pullout from garage " << PLong(getNodeID(garage),4)
		<< " to trip " << PLong(getTrip(event).getTripID(),4) << '\n' );
	D( log.flush() );
}

static void SNbuildAddPullouts()
{
	SNtripNodes::iterator	to;
	ForAllInCollection( snb->tripNodes, to )
	{
		SNevents::iterator	event;
		ForAllInCollection( getLeave(to), event )
		{
			bool pulledOutSomewhere = false;
			SNgarages::iterator garage;
			ForAllInCollection( snb->garages, garage )
			{
				// Find the bus pool that is compatible with this trip.
				SNbusPools::iterator p = getBusPools(garage).find(getTrip(event).getBusType());
				if( p == getBusPools(garage).end() )
					continue;

				const long dur = gs->pulloutTime( getTrip(event).getTripID(),
												getTrip(event).getStartNode(),
												getNodeID(garage),
												getTime(event) );
				if( dur == NO_TIME )
					continue;
				SNbuildAddPullout( to, event, garage, p, dur );
				pulledOutSomewhere = true;
			}
			if( !pulledOutSomewhere )
			{
				// This trip does not pull out from anywhere.
				// Add a very long pullout from the first garage.
				garage = snb->garages.begin();
				SNbusPools::iterator p = getBusPools(garage).find(getTrip(event).getBusType());
				if( p == getBusPools(garage).end() )
					Crash();

				const long dur = Hour(4);
				D( log << "Default: " );
				SNbuildAddPullout( to, event, garage, p, dur );
			}
		}
	}
}

static void	SNbuildAddPullin( SNtripNodes::iterator &from, SNevents::iterator &event,
							 SNgarages::iterator &garage, SNbusPools::iterator &p,
							 const long dur )
{
	const long pullinTime = getTime(event) + dur;
	SNevents::iterator	pullin = getEvents(p).find( pullinTime );
	if( pullin == getEvents(p).end() )
	{
   		pullin = getEvents(p).insert( SNevent(pullinTime, SNnetNode()) );
		getNode(pullin) = snb->network.addNode( (ident_t)(*garage).second );
	}
	snb->network.addArc( getNode(event), getNode(pullin), dur + pullinPenalty, 2 );
	D( log << "Added pullin from trip " << PLong(getTrip(event).getTripID(), 4)
		<< " to garage " << PLong(getNodeID(garage), 4) << '\n' );
	D( log.flush() );
}

static void SNbuildAddPullins()
{
	SNtripNodes::iterator	from;
	ForAllInCollection( snb->tripNodes, from )
	{
		SNevents::iterator	event;
		ForAllInCollection( getArrive(from), event )
		{
			bool pulledInSomewhere = false;
			SNgarages::iterator garage;
			ForAllInCollection( snb->garages, garage )
			{
				// Find the bus pool that is compatible with this trip.
				SNbusPools::iterator p = getBusPools(garage).find(getTrip(event).getBusType());
				if( p == getBusPools(garage).end() )
					continue;

				const long dur = gs->pullinTime( getTrip(event).getTripID(),
												getTrip(event).getEndNode(),
												getNodeID(garage),
												getTime(event) );
				if( dur == NO_TIME )
					continue;
				SNbuildAddPullin( from, event, garage, p, dur );
				pulledInSomewhere = true;
			}
			if( !pulledInSomewhere )
			{
				// This trip does not pull in to anywhere.
				// Add a very long pullin to the first garage.
				garage = snb->garages.begin();
				SNbusPools::iterator p = getBusPools(garage).find(getTrip(event).getBusType());
				if( p == getBusPools(garage).end() )
					Crash();

				const long dur = Hour(4);
				D( log << "Default: " );
				SNbuildAddPullin( from, event, garage, p, dur );
			}
		}
	}
}

void SNbuildAddWaits( int zeroCostLayovers )
{
	snb->addWaits(zeroCostLayovers != 0);
}

void SNbuildEpilog( long vehicleMin, long vehicleMax )
{
	snb->addEquivalences();
	SNbuildAddPullouts();
	SNbuildAddPullins();

	snb->addFinal( vehicleMin, vehicleMax );

	D( log.flush() );

	snb->solve();
	snb->decompose();

	delete snb;
	snb = NULL;

#ifdef BLOCKDEBUG
	delete blockLog;
	blockLog = NULL;
#endif
}

} // extern "C"

