//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

// Disable bogus stl warnings.
#pragma warning(disable : 4786 4114)


//#define NOMINMAX
#include "stdafx.h"
#undef min
#undef max
#include "snsolve.h"

#include <new.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include "cis.h"

#include <map>
#include <set>

using namespace std;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define Second(x)       ((tod_t)(x))
#define Minute(x)       ((tod_t)(x) * Second(60))
#define Hour(x)         ((tod_t)(x) * Minute(60))

#ifndef NO_TIME
#define	NO_TIME	((tod_t)-1)
#endif

inline void fixMinMax( tod_t &min, tod_t &max )
{
    if( min == NO_TIME )
		min = Hour(0);
	if( max == NO_TIME )
		max = Hour(48);
}

class SNobj
{
public:
	enum SNobjType { trip, depot, location };

	SNobj( const SNobjType aType ) : type(aType) {}
	SNobjType getType() const { return type; }
protected:
	const SNobjType	type;
};

class SNtrip : public SNobj
{
public:
    SNtrip( TRIPINFODef *pt, SNobjType type = SNobj::trip ) : SNobj(type)
    {
        TRIPSrecordID      = pt->TRIPSrecordID;
        startTime          = pt->startTime;
        startNODESrecordID = pt->startNODESrecordID;
        endNODESrecordID   = pt->endNODESrecordID;
        BUSTYPESrecordID   = pt->BUSTYPESrecordID;
        ROUTESrecordID     = pt->ROUTESrecordID;
		blockNumber		   = NO_RECORD;

		long min = pt->layoverMin, max = pt->layoverMax;
		fixMinMax( min, max );
		endTime = pt->endTime + min;
		latestLeaveTime = pt->endTime + max;

		visited = false;
    }
    ~SNtrip() {}

    long getStartTime()  const { return startTime; }
    long getEndTime()    const { return endTime; }
    long getStartNode()  const { return startNODESrecordID; }
    long getEndNode()    const { return endNODESrecordID; }
    long getTripID()     const { return TRIPSrecordID; }
	long getBlockNumber() const{ return blockNumber; }
    long getBusType()    const { return BUSTYPESrecordID; }
    long getRoute()      const { return ROUTESrecordID; }
    long getDuration()   const { return endTime - startTime; }
	long getLatestLeaveTime() const { return latestLeaveTime; }
	long getEarliestLeaveTime() const { return endTime; }
	long getLayoverMax	() const	{ return latestLeaveTime - endTime; }

    bool  isVisited() const { return visited; }
    void setVisited() { visited = true; }

    bool  compatibleVehicle( const long busType ) const
    { return BUSTYPESrecordID == busType; }

    bool  compatible(const SNtrip &t) const
    {
        extern int BLallowInterlining;
        bool ret = true;
		if( !BLallowInterlining && ROUTESrecordID != t.ROUTESrecordID )
			ret = false;
		if( BUSTYPESrecordID != t.BUSTYPESrecordID )
			ret = false;
		return ret;
    }

	long	blockNumber;

private:
    tod_t   startTime, endTime;
    long    startNODESrecordID, endNODESrecordID;
    long    TRIPSrecordID;
    long    BUSTYPESrecordID;
    long    ROUTESrecordID;
    tod_t   latestLeaveTime;

	bool	visited;
};

// An SNnetNode pairs a trip with an actual network node.
class SNnetNode
{
public:
	SNnetNode( SNtrip *aTrip = NULL )
	{
		node = NULL;
		trip = aTrip;
	}

	SNnetNode &operator=( const SNnetNode &e )
    { trip = e.trip; node = e.node; return *this; }

	SNtrip	*trip;
	SNnode	*node;
};

// An SNevent is a time and an SNnetNode.
// An SNevents is a map of time and SNnetNode pairs.
typedef std::multimap< long, SNnetNode, std::less<long> > SNevents;
typedef SNevents::value_type SNevent;

template <class M>
void deleteMapData( M &m )
{
	for( M::iterator i = m.begin(); i != m.end(); ++i )
   		delete (*i).second;
	m.erase( m.begin(), m.end() );
}

class SNlocation : public SNobj
{
public:
	SNlocation(SNobjType type = SNobj::location) : SNobj(type) {}
	virtual ~SNlocation()
	{
		// Only delete the leave trips - this ensures that the trip will
		// only be deleted once.
		for( SNevents::iterator i = leave.begin(); i != leave.end(); ++i )
   			delete (*i).second.trip;
		leave.erase( leave.begin(), leave.end() );
	}
	SNevents	leave;
	SNevents	arrive;
};

typedef std::map< long, SNlocation *, std::less<long> > SNtripNodes;
typedef SNtripNodes::value_type SNtripNode;

typedef std::map< long, SNevents *, std::less<long> > SNbusPools;
typedef SNbusPools::value_type SNbusPool;

class SNdepot : public SNlocation
{
public:
	SNdepot(SNobjType type = SNobj::depot) : SNlocation(type) {}

	virtual ~SNdepot() { deleteMapData(busPools); }

	void	addBusType( const long busType )
	{ busPools.insert( SNbusPool(busType, new SNevents())); }

	SNbusPools	busPools;
};

typedef std::map< long, SNdepot *, std::less<long> > SNgarages;
typedef SNgarages::value_type SNgarage;

typedef std::set< long, std::less<long> > SNbusTypes;

class SNbuild
{
public:
	SNbuild();
	~SNbuild();

	void	addBusType( const long busType );
	void	addGarage( const long id );
	void	addTrip( TRIPINFODef *pt );
	void	addWaits( const bool zeroCostLayovers );
	void	addDeadhead( const long startNODE, const long endNODE, long start, long end, long dur );

	void	addEquivalence( const long startNODE, const long endNODE, long dur );
	void	addEquivalences();

	void	addPullout( const long startNODE, const long endNODE, long start, long end, long dur );
	void	addPullin( const long startNODE, const long endNODE, long start, long end, long dur );
    void	addFinal( const long vehicleMin = 0, const long vehicleMax = 0 ); // minimum and maximum number of vehicles to use.

	bool	isGarage( const long id ) const { return garages.find(id) != garages.end(); }

	void	solve()
	{
		network.findZeroFlowBasis( sink, source );
		network.solve();
	}
	void	decompose();

	SNnode		*sink, *source;
	SNbusTypes	busTypes;
	SNgarages	garages;
	SNtripNodes tripNodes;

	SNnetwork	network;
};

