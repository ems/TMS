//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

extern "C"
{
#include "TMSHeader.h"
}
#include <math.h>
#include "Garages.h"

int Garages::init()
{
    numGarages = 0;
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    int rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
    while(rcode2 == 0 && (NODES.flags & NODES_FLAG_GARAGE) &&
		numGarages < (sizeof(garages) / sizeof(garages[0])) )
    {
      garages[numGarages++] = NODES.recordID;
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }

	if( numGarages == 0 )
	{
		// Define at least one default garage.
		garages[numGarages++] = -1;
	}
    return numGarages;
}

long Garages::pulloutTime( 	const long tripID,
					const long nodeID,
					const long garageID,
					const long timeOfDay ) const
{
  // Get the time from the end of this trip to the depot.
  if( !isValid() || nodeID == garageID )
	  return 0;

  GetConnectionTimeDef GCTData;
  float distance;

  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.fromNODESrecordID = garageID;
  GCTData.toNODESrecordID = nodeID;
  GCTData.timeOfDay = timeOfDay;
  const long timeToDepot = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
  distance = (float)fabs((double)distance);
  return timeToDepot;
}

long Garages::pullinTime( const long tripID,
				   const long nodeID,
				   const long garageID,
				   const long timeOfDay ) const
{
  // Get the time from the end of this trip to the depot.
  if( !isValid() || nodeID == garageID )
	  return 0;

  GetConnectionTimeDef GCTData;
  float distance;

  TRIPSKey0.recordID = tripID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
  GCTData.fromNODESrecordID = nodeID;
  GCTData.toNODESrecordID = garageID;
  GCTData.timeOfDay = timeOfDay;
  const long timeToDepot = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
  distance = (float)fabs((double)distance);
  return timeToDepot;
}
