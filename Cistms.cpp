//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
#include "cis.h"
#include "cistms.h"
#include "ServiceCalendar.h"
#include "StrTrim.h"

static	CIS	*cis = NULL;
CIS	*getCIS() { return cis; }
void resetCIS() { cis = NULL; }

extern "C"
{

void	CISfree()
{
	if( cis != NULL )
		delete cis;
	cis = NULL;
}

extern "C"
{
#include "TMSHeader.h"
}
//
//  CISBuild() - Stuff everything into the CIS
//

static int bReturn;

int CISbuild(int bTripPlan, int bShowStatus)
{
	return CISbuildService( bTripPlan, bShowStatus, NO_RECORD );
}

//-----------------------------------------------------------------------------------
#ifndef TRIP_PLANNER_SERVER	// Build the trip planner from the database rather than from files.
//-----------------------------------------------------------------------------------
int CISbuildService( int bTripPlan, int bShowStatus, long serviceIDIn )
{
	CISfree();
	cis = new CIS();

	if( !bTripPlan )
	{
		// Set parameters suitable for drivers - here we ignore a lot of quality issues about the plan
		// and concentrate on the shortest travel time.
		cis->setDriverParameters();
	}

  HCURSOR hSaveCursor;
  char dummy[256];
  long maxRoutes;
  long maxServices;
  long numRoutes;
  long numServices;
  int  rcode2;
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
  int  nM;
 
  long *pRouteRecordIDs = NULL;
  long *pServiceRecordIDs = NULL;
//
//  Display the status bar
//
  bReturn = FALSE;
  if(bShowStatus)
  {
    StatusBarStart(hWndMain, "Initializing...");
  }
//
//  Get the maximum number of routes
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  maxRoutes = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Get the maximum number of services
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  maxServices = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  ForAllLocations
//
  ident_t locationID;
  char    nodeName[NODES_INTERSECTION_LENGTH+1];
  double  longitude, latitude;
  
  if(bShowStatus)
  {
    StatusBarText("Setting up timepoints and stops...");
  }
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(bShowStatus)
    {
      if(StatusBarAbort())
      {
        goto cancel;
      }
    }
    locationID = NODES.recordID;
    strncpy(nodeName, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(nodeName, NODES_INTERSECTION_LENGTH);
    longitude = NODES.longitude;
    latitude = NODES.latitude;
	  if( longitude == 0.0 || latitude == 0.0 )
	  {
      sprintf(tempString, "Node \"%s\" is not geocoded\n\nTrip planning cannot function properly without all timepoints being geocoded", nodeName);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
      goto cancel;
   	}
    cis->addLocation( locationID, nodeName, longitude, latitude );
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  ForAllDirections
//
  ident_t directionID;
  char    directionName[DIRECTIONS_LONGNAME_LENGTH + 1];

  if(bShowStatus)
  {
    StatusBarText("Setting up directions...");
  }
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  while(rcode2 == 0)
  {
    directionID = DIRECTIONS.recordID;
    strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
    cis->addDirection( directionID, directionName );
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  }
//
//  ForAllServices
//
  ident_t serviceID;
  char    serviceName[SERVICES_NAME_LENGTH + 1];

  pServiceRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxServices); 
  if(pServiceRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto cancel;
  }
  if(bShowStatus)
  {
    StatusBarText("Setting up services...");
  }
  numServices = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  for(nI = 0; nI < maxServices; nI++)
  {
    if(bShowStatus)
    {
      if(StatusBarAbort())
      {
        goto cancel;
      }
    }
    serviceID = SERVICES.recordID;
//
// Check if this serviceID matches the one we were given.
//
	   if( serviceIDIn == NO_RECORD || (ident_t)serviceIDIn == serviceID )
	   {
		    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
		    trim(serviceName, SERVICES_NAME_LENGTH);
		    cis->addService( serviceID, serviceName );
		    pServiceRecordIDs[numServices++] = SERVICES.recordID;
	   }
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  }
//
//  ForAllVehicles
//
  ident_t vehicleID;
  char    vehicleName[BUSTYPES_NAME_LENGTH + 1];
  int     isSurface;
  int     isBicycle;
  int     isAccessible;

  if(bShowStatus)
  {
    StatusBarText("Setting up vehicle types...");
  }
  rcode2 = btrieve(B_GETFIRST, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
  while(rcode2 == 0)
  {
    if(bShowStatus)
    {
      if(StatusBarAbort())
      {
        goto cancel;
      }
    }
    vehicleID = BUSTYPES.recordID;
    strncpy(vehicleName, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
    trim(vehicleName, BUSTYPES_NAME_LENGTH);
    isSurface = (int)(BUSTYPES.flags & BUSTYPES_FLAG_SURFACE) != 0;
    isBicycle = (int)(BUSTYPES.flags & BUSTYPES_FLAG_CARRIER) != 0;
    isAccessible = (int)(BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE) != 0;
    cis->addVehicle( vehicleID, vehicleName, isSurface, isAccessible, isBicycle);
    rcode2 = btrieve(B_GETNEXT, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
  }
  cis->addVehicle( (ident_t)-1, "Default Vehicle", 1, 0, 0 );
//
//  ForAllRoutes
//
  ident_t routeID;
  char    routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char    routeName[ROUTES_NAME_LENGTH + 1];
  ident_t idService;
  ident_t idDirection;
  
  if(bShowStatus)
  {
    StatusBarText("Building route list...");
  }
  pRouteRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxRoutes); 
  if(pRouteRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto cancel;
  }
  numRoutes = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  for(nI = 0; nI < maxRoutes; nI++)
  {

    if(!(ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) || !bTripPlan)
    {
      pRouteRecordIDs[numRoutes++] = ROUTES.recordID;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  for(nI = 0; nI < numServices; nI++)
  {
    if(bShowStatus)
    {
      if(StatusBarAbort())
      {
        goto cancel;
      }
      StatusBar((long)nI, numServices);
    }
    for(nJ = 0; nJ < numRoutes; nJ++)
    {
      if(bShowStatus)
      {
        if(StatusBarAbort())
        {
          goto cancel;
        }
      }
      ROUTESKey0.recordID = pRouteRecordIDs[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        if(bShowStatus)
        {
          if(StatusBarAbort())
          {
            goto cancel;
          }
        }
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = pServiceRecordIDs[nI];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        if(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == pServiceRecordIDs[nI] &&
              PATTERNS.directionIndex == nK &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          routeID = ROUTES.recordID;
          strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(routeNumber, ROUTES_NUMBER_LENGTH);
          strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
          trim(routeName, ROUTES_NAME_LENGTH);
          sprintf(tempString, "%s - %s", routeNumber, routeName);
          idService = pServiceRecordIDs[nI];
          idDirection = ROUTES.DIRECTIONSrecordID[nK];
          cis->addRoute( routeID, tempString, idService, idDirection );
        }
      }  // for nK
    }  // for nJ
  }  // for nI
  if(bShowStatus)
  {
    StatusBar(numServices, numServices);
  }
//
//  ForAllTravel
//
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  typedef struct PATStruct
  {
    long PATTERNNAMESrecordID;
    int  numNodes;
    long NODESrecordIDs[200];
    long flags[200];
  } PATDef;
  PATDef  PAT[20];
  int     numPatterns;
  int     startNode;
  int     timePosition;
  int     intervening;
  long    prevPattern;
  long    thisPattern;
  long    totalCycles;
  long    dhd;
  float   distance;
  double  timeDifference;
  BOOL    bFound;
  
  tod_t  fromTime, fTime;
  ident_t fromLocID;
  tod_t  toTime, tTime;
  ident_t toLocID;
  ident_t idRoute;
  ident_t idVehicle;
//
//  Cycle through all the services
//
  totalCycles = numServices * numRoutes * 2;
  for(nI = 0; nI < numServices; nI++)
  {
    if(bShowStatus)
    {
      if(StatusBarAbort())
      {
        goto cancel;
      }
    }
    SERVICESKey0.recordID = pServiceRecordIDs[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Cycle through all the routes
//
    for(nJ = 0; nJ < numRoutes; nJ++)
    {
      if(bShowStatus)
      {
        if(StatusBarAbort())
          goto cancel;
      }
      ROUTESKey0.recordID = pRouteRecordIDs[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
//
//  Loop through both directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(tempString, "Setting up %s trip data\n%s - %s\n(%s)",
              serviceName, routeNumber, routeName, directionName);
        if(bShowStatus)
        {
          StatusBarText(tempString);
          StatusBar((long)((nI * numRoutes * 2) + (nJ * 2) + nK), totalCycles);
        }
//
//  Position into the patterns file
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        prevPattern = NO_RECORD;
        numPatterns = -1;
//
//  Read all the individual patterns (and nodes) for this service / route / direction
//
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK)
        {
          if(bShowStatus)
          {
            if(StatusBarAbort())
            {
              goto cancel;
            }
          }
          if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
          {
            prevPattern = PATTERNS.PATTERNNAMESrecordID;
            numPatterns++;
            PAT[numPatterns].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
            PAT[numPatterns].numNodes = 0;
          }
          PAT[numPatterns].NODESrecordIDs[PAT[numPatterns].numNodes] = PATTERNS.NODESrecordID;
          PAT[numPatterns].flags[PAT[numPatterns].numNodes] = PATTERNS.flags;
          PAT[numPatterns].numNodes++;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
        numPatterns++;
//
//  Position into the trips file
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
//
//  Cycle through all the trips
//
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nK)
        {
          if(bShowStatus)
          {
            if(StatusBarAbort())
            {
              goto cancel;
            }
          }
//
//  If this isn't an employee shuttle route, then
//  only consider trips that have been blocked
//
          if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) ||
                (TRIPS.standard.blockNumber > 0))
          {
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  If we're here on behalf of CISplanRelief, get the PO/PI info
//
            if(!bTripPlan)
            {
              if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
              {
                GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
                GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
                GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.timeOfDay = GTResults.firstNodeTime;
                dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
                if(dhd == NO_TIME)
                {
                  dhd = 0;
                }
                cis->addTravel(GTResults.firstNodeTime - dhd, TRIPS.standard.POGNODESrecordID,
                      GTResults.firstNodeTime, GTResults.firstNODESrecordID,
                      TRIPS.ROUTESrecordID, ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex], TRIPS.tripNumber, TRIPS.standard.blockNumber,
                      TRIPS.BUSTYPESrecordID, TRIPS.SERVICESrecordID);
              }
              if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
              {
                GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
                GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
                GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                GCTData.timeOfDay = GTResults.lastNodeTime;
                dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
                if(dhd == NO_TIME)
                {
                  dhd = 0;
                }
                cis->addTravel(GTResults.lastNodeTime, GTResults.lastNODESrecordID,
                      GTResults.lastNodeTime + dhd, TRIPS.standard.PIGNODESrecordID,
                      TRIPS.ROUTESrecordID, ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex], TRIPS.tripNumber, TRIPS.standard.blockNumber,
                      TRIPS.BUSTYPESrecordID, TRIPS.SERVICESrecordID);
              }
            }
//
//  Locate the pattern in PAT
//
            for(bFound = FALSE, nL = 0; nL < numPatterns; nL++)
            {
              if(PAT[nL].PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
              {
                thisPattern = nL;
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
			  	  {
				  	  sprintf(tempString, "Trip pattern %ld not found", TRIPS.PATTERNNAMESrecordID);
					    MessageBeep(MB_ICONSTOP);
					    MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
  					  goto cancel;
	  			  }
//
//  tripsTimes[] contains the times at the timepoints in this pattern
//
//  By definition, the first and last records in the pattern
//  are regular timepoints.  Start at the first, find the second,
//  and inperpolate the intervening times at the bus stops.
//
            startNode = 0;
            timePosition = 0;
            intervening = 0;
            for(nL = startNode + 1; nL < PAT[thisPattern].numNodes; nL++)
            {
              if(PAT[thisPattern].flags[nL] & PATTERNS_FLAG_BUSSTOP)
              {
                intervening++;
              }
              else
              {
                fromTime = GTResults.tripTimes[timePosition];
                toTime = GTResults.tripTimes[timePosition + 1];
                idRoute = ROUTES.recordID;
                idVehicle = TRIPS.BUSTYPESrecordID;
                idService = SERVICES.recordID;
                idDirection = ROUTES.DIRECTIONSrecordID[nK];
                if(intervening == 0)  // No intervening bus stops
                {
                  fromLocID = PAT[thisPattern].NODESrecordIDs[startNode];
                  toLocID   = PAT[thisPattern].NODESrecordIDs[startNode + 1];
                  if( fromTime > toTime )
                  {
                    char fromNode[NODES_ABBRNAME_LENGTH + 1];
                    char toNode[NODES_ABBRNAME_LENGTH + 1];
                    char szToTime[16];
 
                    NODESKey0.recordID = fromLocID;
                    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    if(rcode2 != 0)
                    {
                      sprintf(tempString, "Pattern node (from) %ld not found", fromLocID);
                      MessageBeep(MB_ICONSTOP);
                      MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
                      goto cancel;
                    }
                    strncpy(fromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    trim(fromNode, NODES_ABBRNAME_LENGTH);
                    NODESKey0.recordID = toLocID;
                    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                    if(rcode2 != 0)
                    {
                      sprintf(tempString, "Pattern node (to) %ld not found", toLocID);
                      MessageBeep(MB_ICONSTOP);
                      MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
                      goto cancel;
                    }
                    strncpy(toNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                    trim(toNode, NODES_ABBRNAME_LENGTH);
                    strcpy(szToTime, Tchar(toTime));  // Fucking sprintf.
                    sprintf(tempString, "Attempt to add travel from %s at %s to %s at %s",
                          fromNode, Tchar(fromTime), toNode, szToTime);
                    MessageBeep(MB_ICONSTOP);
                    MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
                    sprintf(tempString, "This has occurred on route %s - %s, on service %s",
                          routeNumber, routeName, serviceName);
                    MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
                    goto cancel;
                  }
                   
                  cis->addTravel(fromTime, fromLocID, toTime, toLocID,
                        idRoute, idDirection, TRIPS.tripNumber, TRIPS.standard.blockNumber, 
                        idVehicle, idService /*, idDirection*/ );
                }
                else
                {
                  timeDifference = (toTime - fromTime) / intervening;
                  for(nM = startNode; nM < nL; nM++)  // David, I changed <= to <.
                  {
                    if(nM == startNode)
                    {
                      fTime = fromTime;
                    }
                    else
                    {
                      fTime = tTime;
                    }

                    if(nM == nL - 1) // David, do you mean this?.
                    {
                      tTime = toTime;
                    }
                    else
				            {
//                      tTime = (long)(fTime + timeDifference);
//
//  Compute the next time relative to the start - this leads to greater accuracy.
//
//  On second thought, we could probably use the distance to get a better estimate -- FIXLATER.
//
                      tTime = (long)(fromTime + timeDifference * (nM-startNode) + 0.5);
                    }
//
//  David, I added these two lines.
//
                    fromLocID = PAT[thisPattern].NODESrecordIDs[nM];
                    toLocID   = PAT[thisPattern].NODESrecordIDs[nM + 1];

                    if( fTime > tTime )
				            {
					            sprintf(tempString,
                            "Attempt to add travel from NodeID %d to NodeID %d which leaves before it arrives.\n\nTrip planning cannot function properly without all timepoints being geocoded",
						                fromLocID, toLocID );
					            MessageBeep(MB_ICONSTOP);
                      MessageBox(NULL, tempString, "TMS", MB_ICONSTOP | MB_OK);
                      goto cancel;
                    }
                    cis->addTravel(fTime, fromLocID, tTime, toLocID,
                          idRoute, idDirection, TRIPS.tripNumber, TRIPS.standard.blockNumber,
                          idVehicle, idService /*, idDirection*/ );
                  }
                  intervening = 0;
                }
                startNode = nL;
                timePosition++;
              }
            }
          }  // if a shuttle or blocked
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
      }  // for nK
    }  // for nJ
  }  // for nI
  if(bShowStatus)
  {
    StatusBar(totalCycles, totalCycles);
    StatusBarEnd();
  }
  hSaveCursor = SetCursor(hCursorWait);
  cis->initSolve();
  SetCursor(hSaveCursor);

  bReturn = TRUE;
//
//  If he pressed Cancel, he gets here
//
  cancel:
    if(!bReturn)
    {
      CISfree();
      StatusBarEnd();
    }
//
//  Free allocated memory
//
  TMSHeapFree(pRouteRecordIDs);
  TMSHeapFree(pServiceRecordIDs);
//
// Read the mapinfo boundaries file.
//
	 if( bReturn )
  {
	   char	boundariesFile[1024];
	   strcpy( boundariesFile, szInstallDirectory );
	   strcat( boundariesFile, "\\boundaries.mif" );
	   ifstream	ifsMIF(boundariesFile);
	   if( ifsMIF.good() )
		    cis->initBoundaries( ifsMIF );
  }

  return(bReturn);
}

//-----------------------------------------------------------------------------------
#else 	// Build the trip planner from the .txt files rather than from the database.
//-----------------------------------------------------------------------------------

// For all services, call with serviceID == NO_RECORD.
int CISbuildService( int bTripPlan, int bShowStatus, long serviceIDIn )
{
	CFileException	fException;

	// Assumes that we are in the trip planner files directory.
	CISfree();
	cis = new CIS;

	if( !bTripPlan )
	{
		// Set parameters suitable for drivers - here we ignore a lot of quality issues about the plan
		// and concentrate on the shortest travel time.
		cis->setDriverParameters();
	}

  char *ptr;

  bReturn = FALSE;
//
//  ======================
//  Nodes File - Nodes.txt
//  ======================
//
//  Tab separated values
//
//  long     Nodes record ID
//  double   Latitude 
//  double   Longitude
//  char[64] Intersection/Address
//
  CStdioFile Nodes;
  CString    inputLine;
  ident_t locationID;
  char    nodeName[64 + 1];
  double  longitude, latitude;
  
  if( Nodes.Open("Nodes.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }

  Nodes.SeekToBegin();
  bool latLngEastingNorthingUndetermined = true;
  while(Nodes.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  RecordID
//
    ptr = strtok(tempString, "\t");
    locationID = atol(ptr);
//
//  Latitude
//
    ptr = strtok(NULL, "\t");
    latitude = (double)atof(ptr);
//
//  Longitude
//
    ptr = strtok(NULL, "\t");
    longitude = (double)atof(ptr);

//
//  Check for easting/northing rather than latitude/longitude.
//
	if( latLngEastingNorthingUndetermined )
	{
		if( latitude > 10000.0 || longitude > 10000.0 )
			m_bUseLatLong = false;
		else
			m_bUseLatLong = true;
		latLngEastingNorthingUndetermined = false;
	}

//
//  Intersection/Address
//
    ptr = strtok(NULL, "\r\n");
    strncpy(nodeName, ptr, 64);
    trim(nodeName, 64);
//
//  Add the location and cycle back
//
    cis->addLocation(locationID, nodeName, longitude, latitude);
  }
//
//  Done with Nodes.txt - close it
//
  if(Nodes.m_pStream)
  {
    Nodes.Close();
  }
//
//  ================================
//  Directions File - Directions.txt
//  ================================
//
//  Tab separated values
//
//  long     Directions record ID
//  char[16] Direction long name
//
  CStdioFile Directions;
  ident_t directionID;
  char    directionName[16 + 1];
  
  if( Directions.Open("Directions.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }

  Directions.SeekToBegin();
  while(Directions.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  RecordID
//
    ptr = strtok(tempString, "\t");
    directionID = atol(ptr);
//
//  Direction name
//
    ptr = strtok(NULL, "\r\n");
    strncpy(directionName, ptr, 16);
    trim(directionName, 16);
//
//  Add the direction and cycle back
//
    cis->addDirection(directionID, directionName);
  }
//
//  Done with Directions.txt - close it
//
  if(Directions.m_pStream)
  {
    Directions.Close();
  }
//
//  ============================
//  Services File - Services.txt
//  ============================
//
//  Tab separated values
//
//  long     Services record ID
//  char[32] Service name
//
  CStdioFile Services;
  ident_t serviceID;
  char    serviceName[32 + 1];

  if( Services.Open("Services.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }

  Services.SeekToBegin();
  while(Services.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  RecordID
//
    ptr = strtok(tempString, "\t");
    serviceID = atol(ptr);
//
//  Service name
//
    ptr = strtok(NULL, "\r\n");
    strncpy(serviceName, ptr, 16);
    trim(serviceName, 16);
//
//  Add the service and cycle back
//
    cis->addService(serviceID, serviceName);
  }
//
//  Done with Services.txt - close it
//
  if(Services.m_pStream)
  {
    Services.Close();
  }
//
//  ============================
//  Bustypes File - Bustypes.txt
//  ============================
//
//  Tab separated values
//
//  long     Bustypes record ID
//  char[32] Bustype name
//  char[1]  Surface vehicle (Y/N)
//  char[1]  Bicycle carrier (Y/N)
//  char[1]  Accessible (Y/N)
//
  CStdioFile Bustypes;
  ident_t vehicleID;
  char    vehicleName[32 + 1];
  int     isSurface;
  int     isBicycle;
  int     isAccessible;

  if( Bustypes.Open("Bustypes.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }

  Bustypes.SeekToBegin();
  while(Bustypes.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  RecordID
//
    ptr = strtok(tempString, "\t");
    vehicleID = atol(ptr);
//
//  Bustype name
//
    ptr = strtok(NULL, "\t");
    strncpy(vehicleName, ptr, 32);
    trim(vehicleName, 32);
//
//  Surface vehicle
//
    ptr = strtok(NULL, "\t");
    isSurface = (int)(*ptr == 'Y' ? 1 : 0);
//
//  Bicycle carrier
//
    ptr = strtok(NULL, "\t");
    isBicycle = (int)(*ptr == 'Y' ? 1 : 0);
//
//  Accessible
//
    ptr = strtok(NULL, "\r\n");
    isAccessible = (int)(*ptr == 'Y' ? 1 : 0);
//
//  Add the vehicle and cycle back
//
    cis->addVehicle(vehicleID, vehicleName, isSurface, isAccessible, isBicycle);
  }
//
//  Add the default (NO_RECORD) vehicle
//
  cis->addVehicle((ident_t)(-1), "Default Vehicle", 1, 0, 0);
//
//  Done with Bustypes.txt - close it
//
  if(Bustypes.m_pStream)
  {
    Bustypes.Close();
  }
//
//  ========================
//  Routes File - Routes.txt
//  ========================
//
//  Tab separated values
//
//  long     Routes record ID
//  long     Service record ID
//  long     Direction record ID
//  char[76] Route number/name
//
//  If this seems a bit odd, it's because routes are unloaded 
//  based on the presence of a BASE pattern for the route/ser/dir
//
  CStdioFile Routes;
  ident_t routeID;
  char    routeNumberAndName[76 + 1];
  
  if( Routes.Open("Routes.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }


  Routes.SeekToBegin();
  while(Routes.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  Routes Record ID
//
    ptr = strtok(tempString, "\t");
    routeID = atol(ptr);
//
//  Services Record ID
// 
    ptr = strtok(NULL, "\t");
    serviceID = atol(ptr);
//
//  Directions Record ID
//
    ptr = strtok(NULL, "\t");
    directionID = atol(ptr);
//
//  Route number/name
//
    ptr = strtok(NULL, "\r\n");
    strncpy(routeNumberAndName, ptr, 76);
    trim(routeNumberAndName, 76);
//
//  Add the route data and cycle back
//
    cis->addRoute(routeID, routeNumberAndName, serviceID, directionID );
  }
//
//  Done with Routes.txt - close it
//
  if(Routes.m_pStream)
  {
    Routes.Close();
  }
//
//  ======================
//  Trips File - Trips.txt
//  ======================
//
//  Tab separated values
//
//  long Routes record ID
//  long Services record ID
//  long Directions record ID
//  long Bustypes record ID
//  long from time
//  long from Nodes record ID
//  long to time
//  long to Nodes record ID
//  long trip number
//  long block number
//
  CStdioFile Trips;
  tod_t  fromTime;
  ident_t fromLocID;
  tod_t  toTime;
  ident_t toLocID;
  long    tripNumber;
  long    blockNumber;
  
  if( Trips.Open("Trips.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) == FALSE )
  {
	  CISfree();
	  AfxThrowFileException( fException.m_cause, fException.m_lOsError, fException.m_strFileName ); 
  }


  Trips.SeekToBegin();
  while(Trips.ReadString(inputLine))
  {
//
//  Parse the input string
//
    strcpy(tempString, inputLine);
//
//  Routes Record ID
//
    ptr = strtok(tempString, "\t");
    routeID = atol(ptr);
//
//  Services Record ID
// 
    ptr = strtok(NULL, "\t");
    serviceID = atol(ptr);
//
//  Directions Record ID
//
    ptr = strtok(NULL, "\t");
    directionID = atol(ptr);
//
//  Bustypes Record ID
//
    ptr = strtok(NULL, "\t");
    vehicleID = atol(ptr);
//
//  Trip number
//
    ptr = strtok(NULL, "\t");
    tripNumber = atol(ptr);
//
//  Block number
//
    ptr = strtok(NULL, "\t");
    blockNumber = atol(ptr);
//
//  From time
//
    ptr = strtok(NULL, "\t");
    fromTime = atol(ptr);
//
//  From Nodes Record ID
//
    ptr = strtok(NULL, "\t");
    fromLocID = atol(ptr);
//
//  To time
//
    ptr = strtok(NULL, "\t");
    toTime = atol(ptr);
//
//  To Nodes Record ID
//
    ptr = strtok(NULL, "\r\n");
    toLocID = atol(ptr);
//
//  Add the record and cycle back
//
    m_fromTime = fromTime;
    m_fromLocID = fromLocID;
    m_toTime = toTime;
    m_toLocID = toLocID;
    m_routeID = routeID;
	m_directionID = directionID;
    m_vehicleID = vehicleID;
    m_serviceID = serviceID;
    m_tripNumber = tripNumber;
    m_blockNumber = blockNumber;
    cis->addTravel(fromTime, fromLocID, toTime, toLocID, routeID, directionID, tripNumber, blockNumber, vehicleID, serviceID);
  }
//
//  Done with Trips.txt - close it
//
  if(Trips.m_pStream)
  {
    Trips.Close();
  }
//
//  Finalize the init
//
  cis->initSolve();
//
//  Read the boundaries file
//
  char boundariesFile[1024];
  strcpy(boundariesFile, "boundaries.mif");

  ifstream ifsMIF(boundariesFile);

  if(ifsMIF.good())
  {
    cis->initBoundaries(ifsMIF);
  }

//
//  ======================
//  ServiceCalendarDefaults.txt
//  ======================  
  do {
	std::ifstream ifs("ServiceCalendarDefaults.txt");
	if( !ifs.good() )
	{
		// If there is only one service, make it the default for all days of the week.
		ServiceCalendar	*sc = cis->getServiceCalendar();
		if( cis->getNumServices() == 1 )
		{
			const long serviceID = cis->getFirstServiceID();
			for( int d = 0; d < 7; ++d )
				sc->defaultServiceID[d] = serviceID;
		}
		else
		{
			// We have a missing file but also have multiple services.
			// Attempt to guess the service id for each day of the week, if we can...
			for( int d = 0; d < 7; ++d )
			{
				long idService = ServiceCalendar::UnknownID;	// Default.

				// If we fail to get a valid service id, make a best guess.
				// Attempt to guess the service based on the day of the week and likely internals names for the services.
				const bool	isWeekend = (d == 5 || d == 6);
				const bool	isSaturday = (d == 5);
				const bool	isSunday = (d == 6);
				const bool	isWeekday = !isWeekend;
				do
				{
					if( isWeekday )
					{
						if( (idService = cis->findServiceID("Weekday")) != -1 )		break;
						if( (idService = cis->findServiceID("Mon - Fri")) != -1 )	break;
						if( (idService = cis->findServiceID("Mon-Fri.")) != -1 )	break;
					}
					if( isSaturday )
					{
						if( (idService = cis->findServiceID("Saturday")) != -1 )	break;
						if( (idService = cis->findServiceID("Sat")) != -1 )			break;
						if( (idService = cis->findServiceID("Sat.")) != -1 )		break;
					}
					if( isSunday )
					{
						if( (idService = cis->findServiceID("Sunday")) != -1 )		break;
						if( (idService = cis->findServiceID("Sun")) != -1 )			break;
						if( (idService = cis->findServiceID("Sun.")) != -1 )		break;
					}
					if( isWeekend )
					{
						if( (idService = cis->findServiceID("Weekend")) != -1 )		break;
					}
				} while(0); // Not really a loop - just gives me a way to break.

				sc->defaultServiceID[d] = idService;
			}
		}
		break;
	}
	//----------------------------------------------------------------------
	// The file exists.  Read the contents.
	// Handle both the old style format and the new format.
	std::map< std::string, unsigned >	dayNum;
	dayNum.insert( std::make_pair("Monday",		0) );
	dayNum.insert( std::make_pair("Tuesday",	1) );
	dayNum.insert( std::make_pair("Wednesday",	2) );
	dayNum.insert( std::make_pair("Thursday",	3) );
	dayNum.insert( std::make_pair("Friday",		4) );
	dayNum.insert( std::make_pair("Saturday",	5) );
	dayNum.insert( std::make_pair("Sunday",		6) );

	ServiceCalendar	*sc = cis->getServiceCalendar();
	std::string	line, name;
	while(getline(ifs, line))
	{
		std::string::size_type p = 0, pNext;

		pNext = line.find('\t', p);
		if( p == std::string::npos )
			continue;

		name = line.substr( p, pNext );
		trimInPlace( name );
		if( name.empty() || name[0] == ';' )	// Ignore blank lines and lines starting with ';'.
			continue;

		const long idService = atol( line.c_str() + pNext );

		// Try to match by the day name.
		std::map< std::string, unsigned >::const_iterator day = dayNum.find(name);
		if( day != dayNum.end() )
			sc->defaultServiceID[day->second] = idService;
		else
		{
			// If we can't find the day name, try for a number.
			if( name.length() == 1 && 0 <= name[0] && name[0] <= '6' )
			{
				const long d = atol( name.c_str() );
				if( 0 <= d && d <= 6 )
					sc->defaultServiceID[d] = idService;
			}
		}
	}

	// Check for valid contents so we don't crash.
	for( int d = 0; d < 7; ++d )
	{
		if( sc->defaultServiceID[d] != ServiceCalendar::NoID &&
			!cis->isValidServiceID(sc->defaultServiceID[d]) )
			sc->defaultServiceID[d] = ServiceCalendar::UnknownID;
	}
  } while(false);
//
//  ======================
//  ServiceCalendarExceptions.txt
//  ======================
  do {
	std::ifstream ifs("ServiceCalendarExceptions.txt");
	if( !ifs.good() )
		break;
	ServiceCalendar	*sc = cis->getServiceCalendar();
	ServiceCalendar::Date	dSC;
	std::string	line, date;
	while(getline(ifs, line))
	{
		std::string::size_type p = 0, pNext;

		pNext = line.find('\t', p);
		if( p == std::string::npos )
			continue;

		date = line.substr( p, pNext );
		if( !dSC.set( date.c_str() ) )
			continue;

		long idService = atol( line.c_str() + pNext );
		// Check for valid contents so we don't crash.
		if( idService != ServiceCalendar::NoID &&
			!cis->isValidServiceID(idService) )
			idService = ServiceCalendar::UnknownID;

		sc->setServiceException( dSC.year, dSC.month, dSC.day, idService );
	}
  } while(false);

 //
//  Read the LatitudeLongitudeRef.txt file
//  This contains a latitude/longitude for the service area and is used to get the UTM (Easting/Northing) quadrant.
//
  CStdioFile LatLngRef;
  if( LatLngRef.Open("LatitudeLongitudeRef.txt", CFile::modeRead | CFile::typeText | CFile::shareDenyNone, &fException) != FALSE )
  {
	  LatLngRef.SeekToBegin();
	  
	  while(LatLngRef.ReadString(inputLine))
	  {
		  //
		  //  Parse the input string
		  //
		  strcpy(tempString, inputLine);
		  
		  double	lat,lng;
		  ptr = strtok(tempString, "\t");
		  lat = atof(ptr);
		  ptr = strtok(NULL, "\t");
		  lng = atof(ptr);
		  
		  // Set the utm reference with the given lat,lng.
		  TripPlanRequestReply::Coordinate::initUtmReference( lat, lng );
	  }
	  //
	  LatLngRef.Close();
  }

	return(TRUE);
}
#endif

//-----------------------------------------------------------------------------------------
static CISmessageMap *readMessageMap( CISmessageMap *&messageMap, const char *fname )
{
	// Read a message map from the install directory from the given file.
	if( messageMap )
		return messageMap;

	char	messageFile[1024];
	strcpy( messageFile, szInstallDirectory );
	strcat( messageFile, "\\" );
	strcat( messageFile, fname );
	std::ifstream	ifsMM( messageFile );

	// Create a new message map.
	messageMap = new CISmessageMap;
	if( ifsMM.good() )
	{
		// The file opened successfully.  Read the message map.
		messageMap->read( ifsMM );
	}
	else
	{
		// An error occured opening the file!
		// Just create an empty message map so the rest of the code does not crash.

		// Report file-not-found errors here.
		// Be warned - nothing will be output if the messageMap object cannot be read properly!
	}
	return messageMap;
}
static	inline	CISmessageMap *retrieveMessageMap( CISmessageMap *&messageMap, const char *fname )
{ return messageMap ? messageMap : readMessageMap(messageMap, fname); }

//-----------------------------------------------------------------------------------------
// Declare a number of message maps.
static CISmessageMap	*messageMap = NULL, *driversMessageMap = NULL;

// Declare some convenience functions for defererencing the message maps.
static	inline	CISmessageMap	*getMessageMap()
{ return retrieveMessageMap(messageMap, "English-text.txt"); }

static	inline	CISmessageMap	*getDriversMessageMap()
{ return retrieveMessageMap(driversMessageMap, "Drivers-text.txt"); }

//-----------------------------------------------------------------------------------------
void	resetMessageMap()
{
	// Causes the messageMap to be re-read from the file.  Useful if the file is changed and the
	// file needs to be re-read.  Also will become useful to allow the user to change languages
	// in the output.  Does not really handle multiple languages yet - FIXLATER.
	delete messageMap;
	messageMap = NULL;

	delete driversMessageMap;
	driversMessageMap = NULL;
}

char	*CISplan( const char *szLeave, const double leaveLongitude, const double leaveLatitude,
				  const char *szArrive, const double arriveLongitude, const double arriveLatitude,
				  const tod_t t, const int isLeave, const ident_t idService,
				  CISpoint **pCISpointArray, int *pNumCISpoints )
{
	// This function is complete crap and needs to be rewritten in the context of a new interface!!!
	//
	// The caller should pass in the CISresultCollection and display the results in a dialog box
	// so the user can see the attributes of the results.  After selecting the desired result,
	// the output could be formatted in the language of choice of the user by selecting from a set
	// of CISmessageMap objects that were pre-loaded and used to translate the message into the
	// required format.
	//
	// This could be so cool!!!  We just need to finish it.  FIXLATER.

	CISmessageMap	&mm = *getMessageMap();

	CIS::Point	leave(leaveLongitude, leaveLatitude);
	CIS::Point	arrive(arriveLongitude, arriveLatitude);
	// VehicleFlags takes 3 parameters as follows:
	// 1)	surface		- use surface vehicles only.
	// 2)	accessible	- accessible vehicles only.
	// 3)	bicycle		- use vehicles with bicycle only.
	// These are currently not used in the calling routine.

	// Plan the trip.
	const size_t numPlans = 1;	// number of possible plans to generate.
	CISresultCollection	resultCollection(numPlans);
	cis->plan(szLeave, leave, -1, szArrive, arrive, -1, t, isLeave, idService, CISvehicleFlags(0,0,0), resultCollection );

	// Format the results into our internal format.
	// If pCISpointArray is NULL, no points will be returned.
	char	szPlan[8192]; szPlan[0] = 0;

	// First, translate the trip plan into the internal format.
	// FIXLATER
	for( size_t i = 0; i < resultCollection.size(); ++i )
		resultCollection[i]->format( &szPlan[strlen(szPlan)], sizeof(szPlan) - strlen(szPlan), pCISpointArray, pNumCISpoints, mm );

//#define DIAGNOSTICS
#ifdef DIAGNOSTICS
	// Write out the internal format out so we can document it.
	ofstream	internalOFS("Internal.txt");
	internalOFS << szPlan;
#endif

	// Second, translate from the internal format into a readable format.
	static	char	szTranslate[8192];
	CIStranslate	trans;
	trans.translate( szTranslate, szPlan, mm );

#ifdef DIAGNOSTICS
	// Write out the translated format out so we can document it.
	ofstream	translateOFS("Translate.txt");
	translateOFS << szTranslate;
#endif
	return &szTranslate[0];
}

inline	tod_t	roundDownToNearestMinute( const tod_t t ) { return t - (t % 60); }

char	*CISplanReliefConnect(	tod_t *pStartTime, tod_t *pEndTime,
								tod_t *pDwellTime,
								const int description, /* if non-zero, a description of the plan will be returned. */
//								const ident_t fromNodeID, const ident_t toNodeID, // from and to nodes
								const long fromNode, const long toNode, // from and to nodes
								const tod_t t, const int planLeave, // time and whether this is a leave or arrive plan
								const ident_t idService )
{
	static char *noDescriptionString = "No description requested.";
	static	char	szTranslate[8192];
		
	if( !cis )
	{
		MessageBeep(MB_ICONSTOP);
		MessageBox(NULL, "Internal error: CIS Not initialized\nPlease contact Schedule Masters, Inc", TMS, MB_ICONSTOP | MB_OK);
		abort();	// Must initialize trip planner before calling!!!
	}
	
	if( t == NO_TIME || fromNode <= 0 || toNode <= 0 )
	{
		*pStartTime = -25 * 60 * 60;
		*pEndTime = 25 * 60 * 60;
		*pDwellTime = 2 * 25 * 60 * 60;
		return "";
	}
	// Translating the trips into readable format is expensive, and should avoided if not necessary.
	
	ident_t fromNodeID = fromNode;
	ident_t toNodeID = toNode;
	
	// Check the cache.
	if( description <= 0 )
	{
		CISdriverPlanCache::iterator dpe = cis->driverPlanCache.find( CISdriverPlanKey(fromNode, toNode, t, planLeave != 0, idService) );
		if( dpe != cis->driverPlanCache.end() )
		{
			const	CISdriverPlanData	&data = dpe.data();
			*pStartTime	= data.startTime;
			*pEndTime	= data.endTime;
			*pDwellTime	= data.dwellTime;
			return noDescriptionString;
		}
	}
	
	// Plan the trip.
	CISresultCollection	resultCollection(1);
	cis->plan( fromNodeID, toNodeID, t, planLeave, idService, resultCollection );
	
	if( resultCollection.size() == 0 || CIS::isError(resultCollection[0]->getStatus()) )
	{
		*pStartTime = -25 * 60 * 60;
		*pEndTime = 25 * 60 * 60;
		*pDwellTime = 2 * 25 * 60 * 60;
		if( !description )
			return noDescriptionString;
		else
			return "Infeasible.";
	}
	
	int bestIndex = 0;
	tod_t bestTravel = (50 * 60 * 60);
	tod_t thisTravel;
	
	// Pick the best trip plan (should always be the first one).
	for( int nI = 0; nI < (int)resultCollection.size(); nI++)
	{
		if(planLeave)
			thisTravel = resultCollection[nI]->getArriveTime() - t;
		else
			thisTravel = t - resultCollection[nI]->getLeaveTime();
		if(thisTravel < bestTravel)
		{
			bestTravel = thisTravel;
			bestIndex = nI;
		}
	}
	
	// Round down to the nearest minute.
	*pStartTime	= roundDownToNearestMinute(resultCollection[bestIndex]->getLeaveTime());
	*pEndTime	= roundDownToNearestMinute(resultCollection[bestIndex]->getArriveTime());
	if(*pStartTime == *pEndTime)
		*pDwellTime = 0;
	else
		*pDwellTime	= roundDownToNearestMinute(resultCollection[bestIndex]->getDwellTime());
	
	// Add this entry to the cache.
	cis->driverPlanCache.insert(CISdriverPlanKey(fromNode, toNode, t, planLeave != 0, idService),
								CISdriverPlanData(*pStartTime, *pEndTime, *pDwellTime) );
	
	if( description <= 0 )
		return noDescriptionString;
	
	// A description has been requested.
	// Format the results into our internal format.
	
	char	szPlan[8192]; szPlan[0] = 0;
	
	// First, translate the trip plan into the interval format.
	CISmessageMap	&mm = *getDriversMessageMap();	
	resultCollection[bestIndex]->format( szPlan, sizeof(szPlan), NULL, 0, mm );
	
	// Second, translate from the internal format into a readable format.
	CIStranslate	trans;
	trans.translate( szTranslate, szPlan, mm );
	
	register char *ptr = szTranslate;
	if( *ptr )
	{
		//
		//  Clean it up
		//
		ptr = &szTranslate[strlen(szTranslate) - 1];
		while(iscntrl(*ptr) || *ptr == ' ')
			ptr--;
		*(ptr + 1) = '\0';
		ptr = szTranslate;
		while(*ptr == 13 || *ptr == 10)
			ptr++;
	}
	return ptr;
}

} // extern "C"
