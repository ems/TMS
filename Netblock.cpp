//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "netblock.h"
#include "snbuildp.h"
#include <math.h>

TmsMemMgrInit(Trip);

// Flags to control bus interconnections
int     BLallowInterlining = FALSE;
int     BLignoreBusType    = FALSE;

// Comparison functions for the trip index skiplists
static TripEndNodeEndTimeCmp        tripEndNodeEndTimeCmp;
static TripStartNodeStartTimeCmp    tripStartNodeStartTimeCmp;

// Trip index skiplists:
//
// endNodeEndTime       - trips sorted by end node id and end time.
// startNodeStartTime   - trips sorted by start node id and start time.
//
// These indexes are used to quickly build the connection network.
static slTrip   *endNodeEndTime = NULL;
static slTrip   *startNodeStartTime = NULL;

// This is the mincost maxflow network that is built from the trip
// and deadhead information.  The network is solved in network.cpp.
static SPnetwork *network = NULL;

static tod_t layoverMax;
static tod_t tripStartMin, tripEndMax;

//#define WINDOWS_PRINT
#ifdef WINDOWS_PRINT
static void printTrips()
{
    OFSTRUCT ofStruct;
    HFILE hFile;
    char   szTmp[1024];

    hFile = OpenFile("c:\\windows\\trips.txt", &ofStruct, OF_CREATE | OF_READWRITE);
    if(hFile == HFILE_ERROR)
        return;

    slTripIter	itr(startNodeStartTime);
    slRetStatus	ret;
    Trip	*trip;

    for( ret = itr.first(&trip); ret == SL_OK; ret = itr.next(&trip) )
    {
        sprintf( szTmp, "%ld %ld %ld %ld %ld %ld %ld %ld\n",
                  trip->getTripID(),
                  trip->getStartNode(),
                  trip->getStartTime(),
                  trip->getEndNode(),
                  trip->getEndTime(),
                  trip->getLayoverMax(),
                  trip->getBusType(),
                  trip->getRoute() );
        _lwrite( hFile, szTmp, strlen(szTmp) );
    }

    _lclose(hFile);
}

static const char *logFile = "c:\\windows\\tmslog.txt";
static long msgCount = 0;
static tod_t logStartTime;

static void initLog()
{
    OFSTRUCT ofStruct;
    HFILE hFile;

    // Clean up the old log file.
    hFile = OpenFile(logFile, &ofStruct, OF_CREATE | OF_WRITE);
    if(hFile == HFILE_ERROR)
        return;
    _lclose(hFile);

    msgCount = 0L;
    logStartTime = time(NULL);
}

static void logMsg( const char *sz )
{
    HFILE hFile;
    char   szTmp[1024];

    // Open the log file again.
    hFile = _lopen(logFile, WRITE);
    if(hFile == HFILE_ERROR)
        return;

    // Seek to the end of the file.
    _llseek( hFile, 0L, 2 );

    // Get the current date and time
    sprintf( szTmp, "%ld\t%ld\t%s\n", ++msgCount,
                        time(NULL) - logStartTime, sz );
    _lwrite( hFile, szTmp, strlen(szTmp) );

    // Close the file so we don't lose the entry.
    _lclose(hFile);
}
#define log(msg)     logMsg(msg)
#else
#define initLog()
#define log(msg)
#endif

const unsigned long SourceID = 1L;
const unsigned long SinkID   = 2L;

static void hookTrips( unsigned long fromID, unsigned long toID,
                       long cost, long max, long flow )
{
    // Ignore connections to the Source or Sink nodes.
    if( fromID == SourceID || toID == SinkID )
        return;

    // Get the trips from the IDs.
    Trip        *from = (Trip *)fromID;
    Trip        *to = (Trip *)(toID - 1L);

    // Link the trips together on the same chain.
    from->next = to;
    to->prev = from;
}

static void InstallSolution()
{
    network->forAllNonZeroArcs( hookTrips );
}

static void Decompose()
{
    slRetStatus ret;
    Trip        *t;
    slTripIter  iter(startNodeStartTime);
    long        iBlock = 0L;
    long        numTrip = 0L;

    // Initialize each trip to its own block - this helps us detect
    // errors later if any occur.
    for( ret = iter.first(&t); ret == SL_OK; ret = iter.next(&t) )
    {
      m_pBLOCKSDATA[numTrip].TRIPSrecordID = t->getTripID();
   		m_pBLOCKSDATA[numTrip].blockNumber = ++iBlock;
   		numTrip++;
    }
    iBlock = 0L;
    numTrip = 0L;

    // Decompose the maxflow mincost network solution into
    // block numbers.
    //
    // After InstallSolution is called, all trips will be linked
    // together on their optimal chains.  All we have to do now
    // is to traverse each chain and give it the same block
    // number.
    //
    // The basic idea is to find an "unvisited" trip, trace back
    // to the head of its chain, and then traverse the whole chain,
    // visiting as we go.
    for( ret = iter.first(&t); ret == SL_OK; ret = iter.next(&t) )
    {
        // Skip any visited trips.
	     if( t->isVisited() )
	       continue;

	     // Find the first trip in the block.
	     while( t->prev )
	       t = t->prev;

	     // Traverse all the trips in this chain and install them in the m_pBLOCKSDATA array.
	     iBlock++;
	     for( ; t != NULL; t = t->next )
	     {
	        // Stamp this trip with this block number.
	        m_pBLOCKSDATA[numTrip].TRIPSrecordID = t->getTripID();
	        m_pBLOCKSDATA[numTrip].blockNumber = iBlock;
	        numTrip++;

	        // Visit each trip so we don't use it again.
	        t->setVisited();
	    }
    }
}

//----------------------------------------------------------------------

class GarageTravelTime
{
public:
	GarageTravelTime() { numGarages = 0; bestGarage = NO_RECORD; }
	int init(); // Returns the number of garages.
	long pulloutTime( const long tripID,
			  const long nodeID,
			  const long timeOfDay );
	long pullinTime( const long tripID,
			 const long nodeID,
			 const long timeOfDay );
        long findBestGarage( const long tripID,
                             const long nodeID,
                             const long timeOfDay );
        void findAllGarages( skipList<long> &garageList,
                             const long tripID,
                             const long nodeID,
                             const long timeOfDay );
        void setBestGarage( const long aBestGarage )
        { bestGarage = aBestGarage; }

private:
   long garages[10];
	long bestGarage;
	int numGarages;
};

int GarageTravelTime::init()
{
    numGarages = 0;
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    int rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
    while(rcode2 == 0 && (NODES.flags & NODES_FLAG_GARAGE) && numGarages < 10)
    {
      garages[numGarages++] = NODES.recordID;
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }
    return numGarages;
}

long GarageTravelTime::findBestGarage( 	const long tripID,
					const long nodeID,
					const long timeOfDay )
{
  GetConnectionTimeDef GCTData;
  float distance;

  long     deadheadTime;
  long     bestTime = NO_TIME;
  bestGarage = NO_RECORD;

  // Get the connection time from the depot to this location
  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  for(int nJ = 0; nJ < numGarages; nJ++)
  {
    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
    GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
    GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
    GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
    GCTData.fromNODESrecordID = garages[nJ];
    GCTData.toNODESrecordID = nodeID;
    GCTData.timeOfDay = timeOfDay;
    if((deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance)) == NO_TIME)
      continue;
    distance = (float)fabs((double)distance);
    if(bestTime == NO_TIME || deadheadTime < bestTime)
    {
      bestGarage = garages[nJ];
      bestTime = deadheadTime;
    }
  }
  return bestGarage;
}

void GarageTravelTime::findAllGarages(  skipList<long> &garageList,
                                        const long tripID,
					const long nodeID,
					const long timeOfDay )
{
  GetConnectionTimeDef GCTData;
  float distance;

  // Get the connection time from the depot to this location
  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  for(int nJ = 0; nJ < numGarages; nJ++)
  {
    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
    GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
    GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
    GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
    GCTData.fromNODESrecordID = garages[nJ];
    GCTData.toNODESrecordID = nodeID;
    GCTData.timeOfDay = timeOfDay;
    if(GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance) == NO_TIME)
      continue;
    distance = (float)fabs((double)distance);

    long *l = new long;
    *l = garages[nJ];
    garageList.insert( l );
  }
}

long GarageTravelTime::pulloutTime( 	const long tripID,
					const long nodeID,
					const long timeOfDay )
{
  GetConnectionTimeDef GCTData;
  float distance;

  // Get the time from the end of this trip to the depot.
  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.fromNODESrecordID = bestGarage;
  GCTData.toNODESrecordID = nodeID;
  GCTData.timeOfDay = timeOfDay;
  const long timeToDepot = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
  distance = (float)fabs((double)distance);
  return timeToDepot;
}

long GarageTravelTime::pullinTime( const long tripID,
				   const long nodeID,
				   const long timeOfDay )
{
  GetConnectionTimeDef GCTData;
  float distance;

  // Get the time from the end of this trip to the depot.
  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.fromNODESrecordID = nodeID;
  GCTData.toNODESrecordID = bestGarage;
  GCTData.timeOfDay = timeOfDay;
  const long timeToDepot = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
  distance = (float)fabs((double)distance);
  return timeToDepot;
}

//^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^

// limits on total daily work (includes break and pullin/pullout time)
static long workDayMin = Hour(6) + Minute(30);
static long workDayDesired = Hour(8);
static long workDayMax = Hour(9) + Minute(30);

// limits on break time.
static long breakMin = Minute(30);
static long breakDesired = Minute(45);
static long breakMax = Minute(60);

// limits on continuous work time.
static long pieceSizeMin = Hour(2);
static long pieceSizeDesired = Hour(3) + Minute(30);
//static long pieceSizeMax = Hour(5);
static long pieceSizeMax = Hour(6) + Minute(30);

// allowances for taking vehicles in and out of the garage
static long pulloutAllowance = Minute(10);
static long pullinAllowance = Minute(10);

class WaitArcCmpIncreasing : public Comparison<SParc>
{
public:
        WaitArcCmpIncreasing() {}
        ~WaitArcCmpIncreasing() {}
        int compare( const SParc &a1, const SParc &a2 )
        {
                const Trip *t1 = (const Trip *)a1.getFromID();
                const Trip *t2 = (const Trip *)a2.getFromID();
                return    t1->getStartTime() < t2->getStartTime() ? -1
                        : t1->getStartTime() > t2->getStartTime() ? 1
                        : a1.getFromID() < a2.getFromID() ? -1
                        : a1.getFromID() > a2.getFromID() ? 1
                        : a1.getToID() < a2.getToID() ? -1
                        : a1.getToID() > a2.getToID() ? 1
                        : 0;
       }
};

class NodeCmpIncreasing : public Comparison<SPnode>
{
public:
        NodeCmpIncreasing() {}
        ~NodeCmpIncreasing() {}
        int compare( const SPnode &n1, const SPnode &n2 )
        {
                const Trip *t1 = (const Trip *)n1.getID();
                const Trip *t2 = (const Trip *)n2.getID();
                return    t1->getStartTime() < t2->getStartTime() ? -1
                        : t1->getStartTime() > t2->getStartTime() ? 1
                        : n1.getID() < n2.getID() ? -1
                        : n1.getID() > n2.getID() ? 1
                        : 0;
        }
};

class NodeCmpDecreasing : public Comparison<SPnode>
{
public:
        NodeCmpDecreasing() {}
        ~NodeCmpDecreasing() {}
        int compare( const SPnode &n1, const SPnode &n2 )
        {
                const Trip *t1 = (const Trip *)n1.getID();
                const Trip *t2 = (const Trip *)n2.getID();
                return    t1->getEndTime() > t2->getEndTime() ? -1
                        : t1->getEndTime() < t2->getEndTime() ? 1
                        : n1.getID() < n2.getID() ? -1
                        : n1.getID() > n2.getID() ? 1
                        : 0;
        }
};

class GarageCmp : public Comparison<long>
{
public:
        GarageCmp() {}
        ~GarageCmp() {}
        int compare( const long &g1, const long &g2 )
        {
                return g1 < g2 ? -1 : g1 > g2 ? 1 : 0;
        }
};

static inline ltBreakMin( const long t )
{
        return t < breakMin;
//        return 1;
}

skipList<SPnode> *SPnetwork::getForwardPiece( SPnode *n )
{
    NodeCmpDecreasing nodeCmpDecreasing;
    skipList<SPnode> nodeList(&nodeCmpDecreasing, SL_UNIQUE);
    skipListIter<SPnode> nodeIter(&nodeList);
    slRetStatus ret;
    Trip *t = (Trip *)n->getID();

    const long breakStart = t->getEndTime();

    // First, create a list of all upstream work up to the maximum work time.
    n->arcToPred = NULL;
    n->next = n->prev = NULL;
    SPnode      *tail = n;
    SParc       *arc;
    SPnode      *node;
    for( node = n; node != NULL; node = node->next )
        ForAllInArcsOfNode( node, arc )
            if( ltBreakMin(arc->origCost) && arc->from->isWhite() )
            {
                SPnode *nFrom = arc->from;
                Trip *from = (Trip *)nFrom->getID();
                if( breakStart - from->getStartTime() < pieceSizeMax )
                {
                    nodeList.insert( nFrom );
                    nFrom->arcToPred = NULL;
                    nFrom->d = INFINITY;
                    nFrom->setGray();

                    nFrom->next = nFrom->prev = NULL;
                    tail->next = nFrom;
                    tail = nFrom;
                }
            }

    // Now, do a dynamic programing step to get a continuous stream of
    // valid work.
    n->d = 0L;
    nodeList.insert( n );
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        ForAllInArcsOfNode( node, arc )
            if( ltBreakMin(arc->origCost) && arc->from->isGray() )
                if( arc->cost + node->d < arc->from->d )
                {
                    arc->from->d = arc->cost + node->d;
                    arc->from->arcToPred = arc;
                }

    // Next, remove any invalid pieces of work and avoid orphan trips.
    skipList<SPnode> *pnodeListKeep = new skipList<SPnode>(&nodeCmpDecreasing, SL_UNIQUE);
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
    {
        const Trip *t = (Trip *)node->getID();
        const long work = breakStart - t->getStartTime();
        if( pieceSizeMin <= work && work <= pieceSizeMax && node->atStart() )
            pnodeListKeep->insert( node );
    }

    if( pnodeListKeep->entries() == 0 )
    {
        // If none of the nodes are at the start, return the ones within valid duration
        for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        {
            const Trip *t = (Trip *)node->getID();
            const long work = breakStart - t->getStartTime();
            if( pieceSizeMin <= work && work <= pieceSizeMax )
                pnodeListKeep->insert( node );
        }
    }
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        node->setWhite();

    return pnodeListKeep;
}

skipList<SPnode> *SPnetwork::getBackwardPiece( SPnode *n )
{
    NodeCmpIncreasing nodeCmpIncreasing;
    skipList<SPnode> nodeList(&nodeCmpIncreasing, SL_UNIQUE);
    skipListIter<SPnode> nodeIter(&nodeList);
    slRetStatus ret;
    Trip *t = (Trip *)n->getID();

    const long breakEnd = t->getStartTime();

    // First, create a list of all downstream work up to the maximum work time.
    n->arcToPred = NULL;
    n->next = n->prev = NULL;
    SPnode      *tail = n;
    SParc       *arc;
    SPnode      *node;
    for( node = n; node != NULL; node = node->next )
        ForAllOutArcsOfNode( node, arc )
            if( ltBreakMin(arc->origCost) && arc->to->isWhite() )
            {
                SPnode *nTo = arc->to;
                Trip *to = (Trip *)nTo->getID();
                if( to->getEndTime() - breakEnd < pieceSizeMax )
                {
                    nodeList.insert( nTo );
                    nTo->arcToPred = NULL;
                    nTo->d = INFINITY;
                    nTo->setGray();

                    nTo->next = nTo->prev = NULL;
                    tail->next = nTo;
                    tail = nTo;
                }
            }

    // Now, do a dynamic programing step to get a continuous stream of
    // valid work.
    n->d = 0L;
    nodeList.insert( n );
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        ForAllOutArcsOfNode( node, arc )
            if( ltBreakMin(arc->origCost) && arc->to->isGray() )
                if( node->d + arc->cost < arc->to->d )
                {
                    arc->to->d = node->d + arc->cost;
                    arc->to->arcToPred = arc;
                }

    // For all the valid pieces, check that we do not create orphan trips.
    skipList<SPnode> *pnodeListKeep = new skipList<SPnode>(&nodeCmpIncreasing, SL_UNIQUE);
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
    {
        const Trip *t = (Trip *)node->getID();
        const long work = t->getEndTime() - breakEnd;
        if( pieceSizeMin <= work && work <= pieceSizeMax && node->atEnd() )
            pnodeListKeep->insert( node );
    }

    if( pnodeListKeep->entries() == 0 )
    {
        // If none of the nodes are at the end, return the ones within valid duration
        for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        {
            const Trip *t = (Trip *)node->getID();
            const long work = t->getEndTime() - breakEnd;
            if( pieceSizeMin <= work && work <= pieceSizeMax )
                pnodeListKeep->insert( node );
        }
    }

    // Mark these nodes as used.
    for( ret = nodeIter.first(&node); ret == SL_OK; ret = nodeIter.next(&node) )
        node->setWhite();

    return pnodeListKeep;
}

static inline long Abs( const long v )
{
    return v < 0 ? -v : v;
}

static inline double Abs( const double v )
{
    return v < 0.0 ? -v : v;
}

static inline double Sqr( const long v )
{
    return (double)v * (double)v;
}

static inline double frand( const double max = 1.0, const double min = 0.0 )
{
    return ((double)rand() / (double)(RAND_MAX+1)) * (max - min) + min;
}

class Gene
{
public:
        Gene( const long i )
        {
            cost = 1000000000.0;
            size = used = i;
            arcs = new SParc * [size];
        }
        Gene( const Gene& g )
        {
            cost = g.cost;
            size = used = g.size;
            arcs = new SParc * [size];
            setArcs( g.arcs );
        }
        Gene &operator=( const Gene &g )
        {
            cost = g.cost;
            used = g.used;
            setArcs( g.arcs );
            return *this;
        }
        void resetCost() { cost = 1000000000.0; }
        int operator==( const Gene &g ) const
        {
            return cost == g.cost && used == g.used && cmpArcs(g);
        }

        ~Gene() { delete [] arcs; }
        SParc *setArc( const long i, SParc *a ) { return arcs[i] = a; }
        SParc *getArc( const long i ) const { return arcs[i]; }
        double getCost() const { return cost; }
        double setCost( const double c ) { return cost = c; }
        long getSize() const { return size; }
        void setUsed( const long u ) { used = u; }
        long getUsed() const { return used; }

        void randomize( const long start = 0L );
        void orderMutateUnused();
        void orderMutateUsed();

        static void positionCrossover( Gene &c1, Gene &c2,
                                        const Gene &p1, const Gene &p2 );
private:
        void setArcs( SParc *a[] );
        int cmpArcs( const Gene &g ) const;
        double  cost;
        SParc   **arcs;
        long    size;
        long    used;  // the number of arcs at the beginning that are actually used.
};

int     Gene::cmpArcs( const Gene &g ) const
{
    for( long i = 0L; i < size; i++ )
        if( arcs[i] != g.arcs[i] )
            return 0;
    return 1;
}

void    Gene::setArcs( SParc *a[] )
{
    for( long i = 0L; i < size; i++ )
        arcs[i] = a[i];
}

void    Gene::randomize( const long start )
{
    for( long i = start; i < size; i++ )
    {
        const long j = (long)frand(size, start);
        SParc *t = arcs[i];
        arcs[i] = arcs[j];
        arcs[j] = t;
    }
    used = size;
}

void    Gene::positionCrossover( Gene &c1, Gene &c2,
                                const Gene &p1, const Gene &p2 )
{
    const long used = Max(p1.getUsed(), p2.getUsed());
    const long size = p1.getSize();
    WaitArcCmpIncreasing        arcCmp;
    skipList<SParc>     c1List(&arcCmp);
    skipList<SParc>     c2List(&arcCmp);

    // Set the probability of chromosome swapping to be between
    // a specified range.
    const float probMin = (float)0.3;
    const float probMax = (float)0.5;
    double prob = frand(probMax, probMin);

    // Perform the initial crossover.
    long i;
    for( i = 0L; i < used; i++ )
        if( frand() < prob )
        {
            c1.setArc( i, p2.getArc(i) );
            c1List.insert(p2.getArc(i) );

            c2.setArc( i, p1.getArc(i) );
            c2List.insert(p1.getArc(i) );
        }
        else
        {
            c1.setArc( i, NULL );
            c2.setArc( i, NULL );
        }

    // Initialize the rest of the children.
    for( ; i < size; i++ )
    {
        c1.setArc( i, NULL );
        c2.setArc( i, NULL );
    }

    // Fill in the blanks in each gene with arcs in the same order
    // they occur in their respective parents.
    long iSrc, iDest;
    for( iSrc = iDest = 0L; iSrc < size; iSrc++ )
    {
        if( c1List.find(p1.getArc(iSrc)) == SL_OK )
            continue;
        for( ;; iDest++ )
            if( c1.getArc(iDest) == NULL )
                break;
        c1.setArc(iDest, p1.getArc(iSrc));
    }
    for( iSrc = iDest = 0L; iSrc < size; iSrc++ )
    {
        if( c2List.find(p2.getArc(iSrc)) == SL_OK )
            continue;
        for( ;; iDest++ )
            if( c2.getArc(iDest) == NULL )
                break;
        c2.setArc(iDest, p2.getArc(iSrc));
    }
    c1.randomize( used );
    c2.randomize( used );
}

void    Gene::orderMutateUnused()
{
    long i, j;
    if( used >= size )
    {
        // Just swap any two breaks.
        i = (long)frand(size);
        do{ j = (long)frand(size); } while( i == j );
    }
    else
    {
        // Swap out one of the used waits.
        i = (long)frand(used);
        j = (long)frand(size,used);

        // Randomize the remaining ones.
        randomize( used );
    }
    SParc *t = arcs[i];
    arcs[i] = arcs[j];
    arcs[j] = t;
}

void    Gene::orderMutateUsed()
{
    long i, j;

    // Swap a pair of used waits.
    if( used > 1 )
    {
        i = (long)frand(used);
        do{ j = (long)frand(used); } while( i == j );

        SParc *t = arcs[i];
        arcs[i] = arcs[j];
        arcs[j] = t;
    }

    // Randomize the remaining ones.
    randomize( used );
}

class   GeneCmpIncreasing : public Comparison<Gene>
{
public:
        GeneCmpIncreasing() {}
        ~GeneCmpIncreasing() {}
        int compare( const Gene &g1, const Gene &g2 )
        {
            return g1.getCost() < g2.getCost() ? -1
                : g1.getCost() > g2.getCost() ? 1
                : &g1 > &g2 ? 1 : &g1 < &g2 ? -1 : 0;
        }
};

class   GeneCmpDecreasing : public Comparison<Gene>
{
public:
        GeneCmpDecreasing() {}
        ~GeneCmpDecreasing() {}
        int compare( const Gene &g1, const Gene &g2 )
        {
            return g1.getCost() > g2.getCost() ? -1
                : g1.getCost() < g2.getCost() ? 1
                : &g1 > &g2 ? 1 : &g1 < &g2 ? -1 : 0;
        }
};

static long costCount = 0L;

double      SPnetwork::blockCut(skipList<SPnode> &blockCutList,
                                GarageTravelTime &gtt,
                                Gene &g )
{
    blockCutList.clear();
    setNodesWhite();

    double totalCost = 0.0;
    long numBlockCut = 0;       // Number of 2 piece blocks cut.
    long numPieceCut = 0;       // Number of single pieces cut.

    long used = 0L;
    for( long i = 0L; i < g.getSize(); i++ )
    {
        SParc   *arc = g.getArc(i);

        // Check if the from and to trips of this arc are already allocated.
        if( arc->from->isBlack() || arc->to->isBlack() )
            continue;

        skipList<SPnode> *forward = getForwardPiece(arc->from);
        if( forward->entries() == 0 )
        {
            delete forward;
            continue;
        }
        skipList<SPnode> *backward = getBackwardPiece(arc->to);
        if( backward->entries() == 0 )
        {
            delete forward;
            delete backward;
            continue;
        }
        const long startBreak = ((Trip *)arc->from->getID())->getEndTime();
        const long endBreak = ((Trip *)arc->to->getID())->getStartTime();

        // Now, find a list of all the depots that the pieces could
        // pull in or pull out of.
        GarageCmp       garageCmp;
        skipList<long> garageList(&garageCmp, SL_UNIQUE);
        SPnode  *node;
        slRetStatus fRet;
        skipListIter<SPnode> forwardIter(forward);
        for(    fRet = forwardIter.first(&node);
                fRet == SL_OK;
                fRet = forwardIter.next(&node) )
        {
            Trip *tStart = (Trip *)node->getID();
            gtt.findAllGarages( garageList,
                                tStart->getTripID(),
                                tStart->getStartNode(),
                                tStart->getStartTime() );
        }
        slRetStatus bRet;
        skipListIter<SPnode> backwardIter(backward);
        for(    bRet = backwardIter.first(&node);
                bRet == SL_OK;
                bRet = backwardIter.next(&node) )
        {
            Trip *tEnd = (Trip *)node->getID();
            gtt.findAllGarages( garageList,
                                tEnd->getTripID(),
                                tEnd->getEndNode(),
                                tEnd->getEndTime() );
        }
        SPnode  *bestStart = NULL;
        SPnode  *bestEnd = NULL;
        double  bestCost = 1000000000.0;
        long    bestGarage = 0;

        slRetStatus gRet;
        skipListIter<long> garageIter(&garageList);
        long    *garage;
        for(    gRet = garageIter.first(&garage);
                gRet == SL_OK;
                gRet = garageIter.next(&garage) )
        {
            gtt.setBestGarage( *garage );
            SPnode *nStart;
            for( fRet = forwardIter.first(&nStart);
                 fRet == SL_OK;
                 fRet = forwardIter.next(&nStart) )
            {
                 Trip *tStart = (Trip *)nStart->getID();
                 const long pullout =
                        gtt.pulloutTime(tStart->getTripID(),
                                        tStart->getStartNode(),
                                        tStart->getStartTime());
                 if( pullout == NO_TIME )
                     continue;
                 const long lenStart = startBreak - tStart->getStartTime() + pullout;
                 if( lenStart > pieceSizeMax )
                     continue;

                 SPnode *nEnd;
                 for( bRet = backwardIter.first(&nEnd);
                      bRet == SL_OK;
                      bRet = backwardIter.next(&nEnd) )
                 {
                     Trip *tEnd = (Trip *)nEnd->getID();
                     const long pullin =
                                gtt.pullinTime(tEnd->getTripID(),
                                               tEnd->getEndNode(),
                                               tEnd->getEndTime());
                     if( pullin == NO_TIME )
                         continue;
                     const long lenEnd = tEnd->getEndTime() - endBreak + pullin;
                     if( lenEnd > pieceSizeMax )
                         continue;

                     // Check the daily work time constraints.
                     const long workDay = pullout +
                               tEnd->getEndTime() - tStart->getStartTime() +
                               pullin + pullinAllowance + pulloutAllowance;
                     if( workDay < workDayMin || workDay > workDayMax )
                         continue;

                     // Pick the best garage between the work pair.
                     const long workDayDeviation
                        = Abs(workDay - workDayDesired);
                     const long forwardPieceDeviation
                        = Abs(lenStart + pulloutAllowance - pieceSizeDesired);
                     const long backwardPieceDeviation
                        = Abs(lenEnd + pullinAllowance- pieceSizeDesired);
                     const long breakDeviation
                        = Abs(endBreak - startBreak - breakDesired);
                     const double cost
                        = (Sqr(workDayDeviation)
                        + Sqr(forwardPieceDeviation) / 10.0
                        + Sqr(backwardPieceDeviation) / 10.0
                        + Sqr(breakDeviation) / 100.0) / 100.0;
                     if( cost < bestCost )
                     {
                        bestCost = cost;
                        bestStart = nStart;
                        bestEnd = nEnd;
                        bestGarage = *garage;
                     }
                 }
             }
        }
        // Clean up.
        garageList.clearAndDelete();
        delete forward;
        delete backward;

        // We have a match!  Mark these nodes so they will not be used again.
        if( bestStart != NULL && bestEnd != NULL )
        {
            // Mark the used trips so they will not be selected again.
            for( node = bestStart; node != NULL; node = node->getPred() )
                node->setBlack(); // mark this node as done

            // Backpatch the second piece of work to form the complete block.
            SParc *prev = NULL;
            SPnode *next;
            for( node = bestEnd; node != NULL; node = next )
            {
                node->setBlack(); // mark this node as done
                next = node->getPred();
                SParc *cur = node->arcToPred;
                node->arcToPred = prev;
                prev = cur;
            }
            arc->from->arcToPred = arc;
            blockCutList.insert( bestStart );

            // Keep track of the best garage for this trip.
            bestStart->d = bestGarage;
            bestEnd->d = bestGarage;

            totalCost += bestCost;
            numBlockCut++;

            // Swap all the used breaks to the beginning of the gene list.
            if( used != i )
            {
                SParc *t = g.getArc(i);
                g.setArc( i, g.getArc(used) );
                g.setArc( used, t );
                used++;
            }
        }
    }

    // Try to make as many pieces as possible out of what is left.
    // First, collect the unmatched trips.
    NodeCmpIncreasing   nodeCmpIncreasing;
    skipList<SPnode> nodeInc(&nodeCmpIncreasing, SL_UNIQUE);
    slNodeIter itr(&nodeSkipList);
    SPnode     *node;
    slRetStatus ret;
    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
        if( node->isWhite() )
            nodeInc.insert( node );

    // For each unmatched trip, attempt to create a valid minimum piece of work.
    skipListIter<SPnode> nodeIncIter(&nodeInc);
    SPnode      *bestStart;
    for( ret = nodeIncIter.first(&bestStart);
         ret == SL_OK;
         ret = nodeIncIter.next(&bestStart) )
    {
        if( !bestStart->isWhite() )  // Skip blocked nodes.
            continue;

        skipList<SPnode> *backward = getBackwardPiece(bestStart);
        if( backward->entries() == 0 )
        {
            delete backward;
            continue;
        }

        // Find all the garages available to this starting trip.
        GarageCmp       garageCmp;
        skipList<long> garageList(&garageCmp, SL_UNIQUE);
        Trip *tStart = (Trip *)bestStart->getID();
        gtt.findAllGarages( garageList,
                            tStart->getTripID(),
                            tStart->getStartNode(),
                            tStart->getStartTime() );
        // Find all the garages available to the ending trips.
        slRetStatus bRet;
        skipListIter<SPnode> backwardIter(backward);
        for(    bRet = backwardIter.first(&node);
                bRet == SL_OK;
                bRet = backwardIter.next(&node) )
        {
            Trip *tEnd = (Trip *)node->getID();
            gtt.findAllGarages( garageList,
                                tEnd->getTripID(),
                                tEnd->getEndNode(),
                                tEnd->getEndTime() );
        }

        // Determine the best garage combination.
        SPnode  *bestEnd = NULL;
        double  bestCost = 1000000000.0;
        long    bestGarage = 0;

        slRetStatus gRet;
        skipListIter<long> garageIter(&garageList);
        long    *garage;
        for(    gRet = garageIter.first(&garage);
                gRet == SL_OK;
                gRet = garageIter.next(&garage) )
        {
            gtt.setBestGarage( *garage );
            SPnode *nStart = node;
            Trip *tStart = (Trip *)nStart->getID();
            const long pullout =
                   gtt.pulloutTime(tStart->getTripID(),
                                   tStart->getStartNode(),
                                   tStart->getStartTime());
            if( pullout == NO_TIME )
                continue;
            const long lenStart = pullout;

            SPnode *nEnd;
            for( bRet = backwardIter.first(&nEnd);
                 bRet == SL_OK;
                 bRet = backwardIter.next(&nEnd) )
            {
                Trip *tEnd = (Trip *)nEnd->getID();
                const long pullin =
                           gtt.pullinTime(tEnd->getTripID(),
                                          tEnd->getEndNode(),
                                          tEnd->getEndTime());
                if( pullin == NO_TIME )
                    continue;
                const long lenEnd = tEnd->getEndTime() - tStart->getStartTime() + pullin;

                // Pick the best garage between the work pair.
                const long backwardPieceDeviation
                   = Abs(lenEnd + pullout + pullinAllowance - pieceSizeDesired);
                const double cost
                   = (Sqr(backwardPieceDeviation) / 10.0) / 100.0;
                if( cost < bestCost )
                {
                    bestCost = cost;
                    bestEnd = nEnd;
                    bestGarage = *garage;
                 }
             }
        }

        // Clean up.
        garageList.clearAndDelete();
        delete backward;

        // We have a best piece!  Mark these nodes so they will not be used again.
        if( bestEnd != NULL )
        {
            // Mark the used trips so they will not be selected again.

            // Backpatch the second piece of work to form the complete block.
            SParc *prev = NULL;
            SPnode *next;
            for( node = bestEnd; node != NULL; node = next )
            {
                node->setBlack(); // mark this node as done
                next = node->getPred();
                SParc *cur = node->arcToPred;
                node->arcToPred = prev;
                prev = cur;
            }
            blockCutList.insert( bestStart );

            // Keep track of the best garage for this trip.
            bestStart->d = bestGarage;
            bestEnd->d = bestGarage;

            totalCost += bestCost;
            numPieceCut++;
        }
    }

    costCount++; // Increment the cost routine counter.
    g.setUsed( used ); // Set the number of used waits

    // Bias the cost so that fewer blocks and pieces are favored.
    const double fullDayFactor = 2.0;
    const double halfDayFactor = 1.0;
    return g.setCost(totalCost
                - numBlockCut * workDayDesired * fullDayFactor
                - numPieceCut * pieceSizeDesired * halfDayFactor );
}

#ifdef WINDOWS_PRINT
static void logRange( const Gene &best, const Gene &worst, const char *reason )
{
    char        szTmp[512];
    sprintf( szTmp, "%ld\t%ld\t%ld\t%ld\t%s",
                        costCount,
                        (long)worst.getCost(),
                        (long)best.getCost(), best.getUsed(), reason );
    log( szTmp );
}
#else
#define logRange(best, worst, reason)
#endif

class Population
{
public:
    Population( const int s, const Gene &seed );
    ~Population();
    void randomizeAll( SPnetwork *network, Gene &best, Gene &bestBreed,
                        skipList<SPnode> &blockCutList, GarageTravelTime &gtt );
    void chooseParents( Gene * &p1, Gene * &p2 );
    void chooseChild( Gene &child ) { child = *pop[(int)frand(popSize)]; }
    int addIfBetter( SPnetwork *network,
                     Gene &better, Gene &best, Gene &bestBreed,
                     skipList<SPnode> &blockCutList,
                     GarageTravelTime &gtt,
                     const char *reason );
    Gene *getWorst() { Gene *worst; worstList.first(&worst); return worst; }
private:
    bool isDuplicate( const Gene &g ) const;
    GeneCmpIncreasing   cmpIncreasing;
    GeneCmpDecreasing   cmpDecreasing;
    skipList<Gene>      worstList;
    skipList<Gene>      bestList;
    const int popSize;
    Gene **pop;
};

Population::Population( const int s, const Gene &seed ) :
        popSize(s),
        worstList(&cmpDecreasing, SL_UNIQUE),
        bestList(&cmpIncreasing, SL_UNIQUE)
{
    pop = new Gene * [popSize];
    for( int p = 0; p < popSize; p++ )
        pop[p] = new Gene(seed);
}

Population::~Population()
{
    for( int p = 0; p < popSize; p++ )
        delete pop[p];
    delete [] pop;
}

void Population::randomizeAll( SPnetwork *network, Gene &best, Gene &bestBreed,
        skipList<SPnode> &blockCutList, GarageTravelTime &gtt )
{
    Gene        bestSave(best);

    // Initialize this population
    bestList.clear();
    worstList.clear();
    for( int p = 0; p < popSize; p++ )
    {
        pop[p]->randomize();
        bestSave = *pop[p];
        network->blockCut( blockCutList, gtt, *pop[p] );
        bestList.insert( pop[p] );
        worstList.insert( pop[p] );
        if( pop[p]->getCost() < best.getCost() )
        {
            best = bestSave;
            best.setCost( pop[p]->getCost() );
            bestBreed = *pop[p];
            logRange( bestBreed, *getWorst(), "random" );
        }
    }
}

bool Population::isDuplicate( const Gene &g ) const
{
    for( int p = 0; p < popSize; p++ )
        if( g == *pop[p] )
            return false;
    return 0;
}

void Population::chooseParents( Gene * &p1, Gene * &p2 )
{
    // First, get the total of all costs.
    Gene    *worst = getWorst();
    double costTotal = 0.0;
    for( int p = 0; p < popSize; p++ )
        costTotal += Abs(pop[p]->getCost() - worst->getCost());

    // Then, compute a cost threshold.
    double costThresh = frand(costTotal);
    double costAccum;
    slRetStatus     ret;
    skipListIter<Gene> bestIter(&bestList);
    costAccum = 0.0;

    // Run through the gene until the total exceeds the threshold
    // and choose this as parent 1.
    for( ret = bestIter.first(&p1); ret == SL_OK; ret = bestIter.next(&p1) )
    {
        costAccum += Abs(p1->getCost() - worst->getCost());
        if( costAccum >= costThresh )
            break;
    }
    if( ret != SL_OK ) p1 = worst;

    // Choose another cost threshold.
    costThresh = frand(costTotal);
    costAccum = 0.0;

    // Run through the gene until the total exceeds the threshold
    // and choose this as parent 2.
    for( ret = bestIter.first(&p2); ret == SL_OK; ret = bestIter.next(&p2) )
    {
        costAccum += Abs(p2->getCost() - worst->getCost());
        if( costAccum >= costThresh )
            break;
    }
    if( ret != SL_OK ) p2 = worst;

    // Make sure we have unique parents.
    while( p1 == p2 ) p2 = pop[(int)frand(popSize)];
}

int Population::addIfBetter( SPnetwork *network,
                        Gene &better, Gene &best, Gene &bestBreed,
                        skipList<SPnode> &blockCutList,
                        GarageTravelTime &gtt, const char *reason )
{
    Gene bestSave( better );
    network->blockCut( blockCutList, gtt, better );

    // Replace the worst member if necessary.
    Gene *worst = getWorst();
    if( better.getCost() < worst->getCost() )
    {
        // Ignore duplicate children in the population.
        if( !isDuplicate(better) )
        {
            bestList.remove(worst);
            worstList.remove(worst);
            *worst = better;
            logRange( best, *worst, reason );
            bestList.insert(worst);
            worstList.insert(worst);
        }
    }
    if( better.getCost() < best.getCost() )
    {
        best = bestSave;
        best.setCost( better.getCost() );
        bestBreed = better;
        logRange( best, *worst, reason );
        return 1;
    }
    return 0;
}

void SPnetwork::bestBlockCut()
{
    costCount = 0L; // Initialize the cost counter.

    // Intialize the garage information.
    GarageTravelTime    gtt;
    if( gtt.init() == 0 )
        return;

    // First, create a list of all the wait arcs that could be breaks.
    WaitArcCmpIncreasing waitArcCmpIncreasing;
    skipList<SParc> waitList(&waitArcCmpIncreasing, SL_UNIQUE);

    NodeComparison     nodeCmp;
    slNodeList  blockCutList(&nodeCmp, SL_UNIQUE);

    slNodeIter	itr(&nodeSkipList);
    slRetStatus	ret;
    SPnode	*node;
    SParc	*arc;

    for( ret = itr.first(&node); ret == SL_OK; ret = itr.next(&node) )
    {
        const Trip *t = (Trip *)node->getID();
        // Skip potential breaks that are too early in the day.
        if( t->getEndTime() - tripStartMin < pieceSizeMin )
            continue;
        // Skip potential breaks if there is no possibility of earlier work.
        if( node->inList == NULL && t->getDuration() < pieceSizeMin )
            continue;
	ForAllOutArcsOfNode( node, arc )
        {
            // Check if this could be a valid wait arc.
            if( arc->origCost < breakMin || arc->origCost > breakMax )
                continue;

            const SPnode *toNode = arc->to;
            const Trip *t = (Trip *)toNode->getID();
            // Skip potential breaks that are too late in the day.
            if( tripEndMax - t->getStartTime() < pieceSizeMin )
                continue;

            // Skip potential breaks if there is no possibility of later work.
            if( toNode->outList == NULL && t->getDuration() < pieceSizeMin )
                continue;

            // Otherwise, add this arc to the list of potentials.
            waitList.insert( arc );
        }
    }

    Gene        start( waitList.entries() );
    skipListIter<SParc> waitIter(&waitList);
    long i;
    for( i = 0L, ret = waitIter.first(&arc); ret == SL_OK;
         ret = waitIter.next(&arc) )
         start.setArc( i++, arc );
    Gene        best(start), bestBreed(best);
    Gene        child1(best), child2(best);
    Gene        &child = child1;

    // Create the population to run the genetic algorithm.
    const int popSize = waitList.entries() / 10;
    Population pop(popSize, best);

    // Pick the number of iterations.
    const long iterMax = (long)(waitList.entries() * 1.6);
    long        g; // Generation counter.

    // Choose a probability that a crossover will occur with the
    // best individual.
    const float bestProb = (float)0.005;

    // Do a number of tries in an attempt to get the best solution.
    int tries;
    Gene        bestOverall(best);
    bestOverall.resetCost();
    for( tries = 0; tries < 5; tries++ )
//    for( tries = 0; tries < 1; tries++ )
    {
        // Seed the generator for reproducability.
        srand( tries );

        // Initialize this population
        best.resetCost();
        pop.randomizeAll( this, best, bestBreed, blockCutList, gtt );

        // Perform the generations.
        for( g = 0L; g < iterMax; g++ )
        {
            if( frand() < 0.25 )
            {
                // Do a mutation.
                // First, randomly pick a member.
                pop.chooseChild( child );

                // Randomly pick a mutation.
                const int mutateUsed = (frand() < 0.5);
                if( mutateUsed )
                    child.orderMutateUsed();
                else
                    child.orderMutateUnused();
                pop.addIfBetter(this, child, best, bestBreed, blockCutList, gtt,
                             mutateUsed ? "mutationUsed" : "mutationUnused");
            }
            else
            {
                // Do a position oriented crossover.
                // Use the roulette wheel approach to pick parents.
                // This biases the breeding with the best individuals.
                Gene *p1, *p2;
                pop.chooseParents( p1, p2 );

                // Every so often, breed with the best individual.
                Gene::positionCrossover( child1, child2, *p1,
                        frand() < bestProb ? bestBreed : *p2 );
                pop.addIfBetter(this, child1, best, bestBreed, blockCutList, gtt,
                                "crossover1" );
                pop.addIfBetter(this, child2, best, bestBreed, blockCutList, gtt,
                                "crossover2" );
            }
        } // end Generations loop
        if( bestOverall.getCost() > best.getCost() )
            bestOverall = best;
    } // end Tries loop

    // Install the blockCut solution.
    blockCut( blockCutList, gtt, bestOverall );
    logRange( bestOverall, *pop.getWorst(), "best" );
    skipListIter<SPnode> blockCutIter(&blockCutList);
    for(ret = blockCutIter.first(&node); ret == SL_OK;
        ret = blockCutIter.next(&node) )
    {
        Trip    *prev = NULL;
        for( ; node != NULL; node = node->getPred() )
        {
            Trip *t = (Trip *)node->getID();
            t->prev = t->next = NULL;
            if( prev != NULL )
                prev->next = t;
            t->prev = prev;
            prev = t;
        }
    }
}

static  slRetStatus getFirstTripStartingAfter( long t, long n, slTripIter &i )
{
    // Find the first trip that starts at the same location as
    // the from trip.
    TRIPINFODef findTripInfo;
    memset( &findTripInfo, 0, sizeof(findTripInfo) );
    findTripInfo.startNODESrecordID = n;
    findTripInfo.endNODESrecordID = n;
    findTripInfo.startTime = t;
    findTripInfo.endTime = t;
    findTripInfo.layoverMin = Hour(0);
    findTripInfo.layoverMax = Hour(24);
    Trip    findTrip(&findTripInfo);

    // The skiplist will find the closest trip before
    // the endTime - we want the closest trip after the
    // end time so we need to skip some of the leading trips.
    slTripIter  itr(startNodeStartTime);
    Trip        *trip;
    slRetStatus ret = itr.findFirstClosest(&findTrip, &trip);
    if( ret != SL_OK )
        ret = itr.first(&trip);
    while( ret == SL_OK &&
           tripStartNodeStartTimeCmp.compare(*trip, findTrip) < 0 )
        ret = itr.next(&trip);

    i = itr;
    if( ret == SL_OK &&
        trip->getStartNode() == n &&
        trip->getStartTime() >= t )
        return SL_OK;

    return SL_NOT_FOUND;
}

static  slRetStatus getFirstTripEndingAfter( long t, long n, slTripIter &i )
{
    // Find the first trip that starts at the same location as
    // the from trip.
    TRIPINFODef findTripInfo;
    memset( &findTripInfo, 0, sizeof(findTripInfo) );
    findTripInfo.startNODESrecordID = n;
    findTripInfo.endNODESrecordID = n;
    findTripInfo.startTime = t;
    findTripInfo.endTime = t;
    findTripInfo.layoverMin = Hour(0);
    findTripInfo.layoverMax = Hour(24);
    Trip    findTrip(&findTripInfo);

    // The skiplist will find the closest trip before
    // the startTime - we want the closest trip after the
    // start time so we need to skip some of the leading trips.
    slTripIter  itr(endNodeEndTime);
    Trip        *trip;
    slRetStatus ret = itr.findFirstClosest(&findTrip, &trip);
    if( ret != SL_OK )
        ret = itr.first(&trip);
    while( ret == SL_OK &&
           tripEndNodeEndTimeCmp.compare(*trip, findTrip) < 0 )
        ret = itr.next(&trip);

    i = itr;
    if( ret == SL_OK &&
        trip->getEndNode() == n &&
        trip->getEndTime() >= t )
        return SL_OK;

    return SL_NOT_FOUND;
}


extern "C" {

long   numWaitArcs;

const double penaltyMax = Minute(2);
void    BLGenerateWaitArcs( int zeroCostLayovers )
{

	if( !m_bNetworkMode )
	{
		SNbuildAddWaits( zeroCostLayovers );
		return;
	}
    numWaitArcs = 0;

    slTripIter  fromItr(startNodeStartTime);
    slTripIter  toItr(startNodeStartTime);
    slRetStatus fromRet, toRet;
    Trip        *from, *to;

    // Compute a small quadratic penalty for the wait time of trips.
    // This ensures that if possible, the blocker will tend to use
    // lots of small waiting times and avoid longer periods of waiting.
    const double quadFactor = layoverMax > 0
                              ? penaltyMax / ((double)layoverMax * (double)layoverMax)
                              : 0;

    // For all trips, connect each trip to all other trips
    // at the same end node that leave after the trip's end
    // time (plus min wait time) and leave before the trip's
    // maximum wait time.
    for( fromRet = fromItr.first(&from); fromRet == SL_OK; fromRet = fromItr.next(&from) )
    {
        toRet = getFirstTripStartingAfter( from->getEndTime(), from->getEndNode(), toItr );
        if( toRet != SL_OK )
            continue;
		
		for( toRet = toItr.current(&to); toRet == SL_OK; toRet = toItr.next(&to) )
		{
			if( to->getStartNode() != from->getEndNode() ||
				to->getStartTime() - from->getEndTime() > from->getLayoverMax() )
				break;
			if( from->compatible(*to) )
            {
				long cost, waitTime;
				if( !zeroCostLayovers )
				{
					waitTime = Abs(to->getStartTime() - from->getEndTime());
					const long penaltyTime = (long)((double)waitTime * (double)waitTime * quadFactor);
					cost = waitTime + penaltyTime;
				}
				else
				{
					cost = SNzeroCostLayover;
					waitTime = SNzeroCostLayover;
				}
                if( !m_bNetworkMode )
					network->addArc( (unsigned long)from, ((unsigned long)to)+1L, cost, 1L );
                else
                    network->addArc( (unsigned long)from, (unsigned long)to, cost, 1L, waitTime );
                numWaitArcs++;
            }
		}
    }
}

static void    GenerateDeadheads( const long startNODE, const long endNODE, long start, long end, long dur )
{
    if( start == NO_TIME ) start = Hour(0);
    if( end == NO_TIME )   end = Hour(48);
    if( dur == NO_TIME )   dur = Hour(48);

    slTripIter  fromItr(endNodeEndTime);
    slTripIter  toItr(startNodeStartTime);
    slRetStatus fromRet, toRet;
    Trip        *from, *to;

    fromRet = getFirstTripEndingAfter(start, startNODE, fromItr );
    if( fromRet != SL_OK )
        return;

    // Compute a small quadratic penalty for the wait time of trips.
    // This ensures that if possible, the blocker will tend to use
    // lots of small waiting times and avoid longer periods of waiting.
    const double quadFactor = layoverMax > 0
                              ? penaltyMax / ((double)layoverMax * (double)layoverMax)
                              : 0;

    // For all trips ending at the startNode between the start and end times,
    // connect each one to all trips starting at the endNode between the start
    // and end times, and so that the layover time before the second trip is less
    // than the layover max of the first trip.
    for( fromRet = fromItr.current(&from); fromRet == SL_OK; fromRet = fromItr.next(&from) )
    {
	if( from->getEndNode() != startNODE ||
	    from->getEndTime() > end )
	    break;

        toRet = getFirstTripStartingAfter( from->getEndTime() + dur,
                                           endNODE,
                                           toItr );
        if( toRet != SL_OK )
            continue;

	for( toRet = toItr.current(&to); toRet == SL_OK; toRet = toItr.next(&to) )
	{
	    if( to->getStartNode() != endNODE ||
		to->getStartTime() - from->getEndTime() - dur > from->getLayoverMax() )
		break;
	    if( from->compatible(*to) )
            {
                // Define the cost of a deadhead as the sum of:
                // 1) the duration of the deadhead (driving time)
                // 2) the waiting time incurred at the end of the deadhead.
                // 3) a quadradic scaling factor the favors shorter waiting times.
                // 4) a 10 minute constant penalty for taking deadheads.  The
                //    idea here is that we would rather wait an extra 10
                //    minutes than take a deadhead at all.
                const long waitTime = Abs(to->getStartTime() - from->getEndTime()) - dur;
                const long penaltyTime = (long)((double)waitTime * (double)waitTime * quadFactor);
                const long cost = dur + waitTime + penaltyTime + Minute(10);
                if( !m_bNetworkMode )
	            network->addArc( (unsigned long)from, ((unsigned long)to)+1L,
		       	             cost, 1L );
                else
                    network->addArc( (unsigned long)from,
                                     (unsigned long)to, cost, 1L, waitTime );
                numWaitArcs++;
            }
	}
    }
}

void BLGenerateDeadheadArcs( DEADHEADINFODef *deadhead )
{
	if( m_bNetworkMode )
	{
		GenerateDeadheads( deadhead->fromNODESrecordID,
		       deadhead->toNODESrecordID,
		       deadhead->fromTimeOfDay,
		       deadhead->toTimeOfDay,
		       deadhead->deadheadTime );
		if( deadhead->flags & CONNECTIONS_FLAG_TWOWAY )
			GenerateDeadheads( deadhead->toNODESrecordID,
			   deadhead->fromNODESrecordID,
			   deadhead->fromTimeOfDay,
			   deadhead->toTimeOfDay,
			   deadhead->deadheadTime );
	}
	else
	{
		SNbuildAddDeadhead( deadhead->fromNODESrecordID,
		       deadhead->toNODESrecordID,
		       deadhead->fromTimeOfDay,
		       deadhead->toTimeOfDay,
		       deadhead->deadheadTime );
		if( deadhead->flags & CONNECTIONS_FLAG_TWOWAY )
			SNbuildAddDeadhead( deadhead->toNODESrecordID,
			   deadhead->fromNODESrecordID,
			   deadhead->fromTimeOfDay,
			   deadhead->toTimeOfDay,
			   deadhead->deadheadTime );
	}
}

void BLInitNetwork()
{
	if( m_bNetworkMode )
	{
		endNodeEndTime = new slTrip(&tripEndNodeEndTimeCmp);
		startNodeStartTime = new slTrip(&tripStartNodeStartTimeCmp);
		network = new SPnetwork;

		layoverMax = 0L;

		tripStartMin = Hour(48);
		tripEndMax = Hour(0);
	}
	else
	{
		SNbuildProlog();
	}
}

void    BLSetOptions( BOOL allowInterliningFlag /* ,
                      BOOL ignoreBusTypeFlag */ )
{
    BLallowInterlining = allowInterliningFlag;
//    BLignoreBusType    = ignoreBusTypeFlag;
    BLignoreBusType = TRUE;
}

void    BLCleanup( void )
{
	if( startNodeStartTime == NULL )
		return;

	slTripIter itr(startNodeStartTime);
	Trip       *t;
	while( itr.first(&t) == SL_OK )
	{
		startNodeStartTime->remove(t);
		endNodeEndTime->remove(t);
		delete t;
	}

	delete endNodeEndTime;
	delete startNodeStartTime;
	endNodeEndTime = startNodeStartTime = NULL;
	delete network;
	network = NULL;
}

void BLGenerateTripArc( TRIPINFODef *ptrip )
{
	if( m_bNetworkMode )
	{
		Assert( endNodeEndTime );
		Assert( startNodeStartTime );

		Trip    *t = new Trip(ptrip);
		endNodeEndTime->insert( t );
		startNodeStartTime->insert( t );

		layoverMax = Max( layoverMax, t->getLayoverMax() );
		tripStartMin = Min( tripStartMin, t->getStartTime() );
		tripEndMax = Max( tripEndMax, t->getEndTime() );

		// Add the arcs form the super source node to the super sink
		// node in the mincost-maxflow network.
		// We use the convention that the "from" network node ids will
		// be equal to the address of the trip, the "to" network node
		// ids will be equal to the address of the trip + 1.
		if( !m_bNetworkMode )
		{
			network->addArc( SourceID, (unsigned long)t, Hour(0), 1L );
			network->addArc( ((unsigned long)t)+1L, SinkID, Hour(0), 1L );
		}
	}
	else
	{
		SNbuildAddTrip( ptrip );
	}
}

int sblock(long vehicleMin, long vehicleMax)
{
    initLog();

    if( m_numTripsForBlocker <= 0 )
		return 0;

#ifdef FIXLATER
    printTrips();
#endif

    if( !m_bNetworkMode )
    {
		SNbuildEpilog( vehicleMin, vehicleMax );
    }
    else
	{
        network->bestBlockCut();
		Decompose();
		BLCleanup();
	}

    return 0;
}

} // extern "C"
