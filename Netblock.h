//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef NETBLOCK_H
#define NETBLOCK_H

#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include "tod.h"

extern "C" {
#define externC
#include "TMSHeader.h"
#undef externC
}

#define Second(x)       ((tod_t)(x))
#define Minute(x)       (Second(x) * Second(60))
#define Hour(x)         (Second(x) * Second(60) * Second(60))

inline void fixMinMax( tod_t &min, tod_t &max )
{
    if( min == NO_TIME )
	min = Hour(0);
    if( max == NO_TIME )
	max = Hour(48);   
}

class Trip
{
public:
    Trip( TRIPINFODef *pt )
    {
        TRIPSrecordID      = pt->TRIPSrecordID;
        startTime          = pt->startTime;
        startNODESrecordID = pt->startNODESrecordID;
        endNODESrecordID   = pt->endNODESrecordID;
        BUSTYPESrecordID   = pt->BUSTYPESrecordID;
        ROUTESrecordID     = pt->ROUTESrecordID;
		
		tod_t min = pt->layoverMin;
		tod_t max = pt->layoverMax;
		fixMinMax( min, max );
		endTime = pt->endTime + min;
		layoverMax = max;
		
		fVisited = false;
		next = prev = NULL;
    }
    ~Trip() {}

    TmsMemMgrDeclare();

    tod_t getStartTime()  const { return startTime; }
    tod_t getEndTime()    const { return endTime; }
    tod_t getLayoverMax() const { return layoverMax; }
    tod_t getStartNode()  const { return startNODESrecordID; }
    tod_t getEndNode()    const { return endNODESrecordID; }
    tod_t getTripID()     const { return TRIPSrecordID; }
    tod_t getBusType()    const { return BUSTYPESrecordID; }
    tod_t getRoute()      const { return ROUTESrecordID; }
    tod_t getDuration()   const { return endTime - startTime; }

    bool  isVisited() const { return fVisited; }
    void setVisited() { fVisited = TRUE; }

    bool  compatible(const Trip &t) const
    {
        extern int BLallowInterlining;
        extern int BLignoreBusType;
		bool ret = true;
		if( !BLallowInterlining && ROUTESrecordID != t.ROUTESrecordID )
			ret = false;
		//if( !BLignoreBusType && BUSTYPESrecordID != t.BUSTYPESrecordID )
		//   ret = false;
		if( BUSTYPESrecordID != t.BUSTYPESrecordID )
			ret = false;
		return ret;
    }

    Trip    *next, *prev;       // pointers set after the solve to hook the trips together

private:
    tod_t  startTime, endTime;
    long    startNODESrecordID, endNODESrecordID;
    long    TRIPSrecordID;
    long    BUSTYPESrecordID;
    long    ROUTESrecordID;
    tod_t   layoverMax;

    bool    fVisited;
};

typedef skipList<Trip>  slTrip;
typedef skipListIter<Trip> slTripIter;

class TripEndNodeEndTimeCmp : public Comparison<Trip>
{
public:
    TripEndNodeEndTimeCmp() {}
    ~TripEndNodeEndTimeCmp() {}
    int compare( const Trip &t1, const Trip &t2 )
    {
		return t1.getEndNode() < t2.getEndNode() ? -1
				   : t1.getEndNode() > t2.getEndNode() ? 1
				   : t1.getEndTime() < t2.getEndTime() ? -1
				   : t1.getEndTime() > t2.getEndTime() ? 1
				   : 0;
    }
};

class TripStartNodeStartTimeCmp : public Comparison<Trip>
{
public:
    TripStartNodeStartTimeCmp() {}
    ~TripStartNodeStartTimeCmp() {}
    int compare( const Trip &t1, const Trip &t2 )
    {
		return t1.getStartNode() < t2.getStartNode() ? -1
				   : t1.getStartNode() > t2.getStartNode() ? 1
				   : t1.getStartTime() < t2.getStartTime() ? -1
				   : t1.getStartTime() > t2.getStartTime() ? 1
				   : 0;
    }
};

#endif // NETBLOCK_H