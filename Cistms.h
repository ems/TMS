//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef cistms_H
#define cistms_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tod.h"

typedef unsigned long ident_t;

typedef struct CISpoint CISpoint;
struct CISpoint
{
	tod_t	t;						// time of this point.
	long	routeID;				// route id (NO_RECORD for the start and end of the trip).
	const	char *routeName;		// name of the route (NULL for the start and end of the trip).
	double	longitude, latitude;	// location.
};

void	CISfree();
int 	CISbuildService( int bTripPlan,   // TRUE if initializing for trip planner, FALSE if for CISplanRelief
                      int bShowStatus, // TRUE if status bar to be displayed, FALSE if not
                      long serviceID );	// Builds a search structure for the given service.
										                  	// If serviceID == NO_RECORD, all services are built.
int 	CISbuild(int bTripPlan,    // TRUE if initializing for trip planner, FALSE if for CISplanRelief
              int bShowStatus); // TRUE if status bar to be displayed, FALSE if not
                  							// Builds a search structure for all services.
									          		// Equivalent to CISbuildService( int, int, NO_RECORD )

// All the stops on the route will be returned in the pCISpointArray if it is non-NULL.
// The number of points will be returned in numCISpoints.
// Entries 0 and numCISpoints-1 (that is, the first and last points) will be initialized
// with the leave and arrive coordinates with routeID set to NO_RECORD.
//
// This array is allocated with malloc and must be freed afterwards by the caller.
//
// This is a BOGUS way to call this routine - the user interface needs to take care of
// the multiple language and multiple trip plan capabilities!!!
//
// FIXLATER.

char	*CISplan( const char *szLeave, const double leaveLongitude, const double leaveLatitude,
				  const char *szArrive, const double arriveLongitude, const double arriveLatitude,
				  const tod_t t, const int isLeave, const ident_t idService,
				  CISpoint **pCISpointArray, int *numCISpoints );

char	*CISplanReliefConnect(	tod_t *pStartTime, tod_t *pEndTime,	/* start and end time of trip */
								tod_t *pDwellTime,						/* waiting time after/before given time */
								const int description, /* if non-zero, a description of the plan will be returned. */
//								const ident_t fromNodeID, const ident_t toNodeID, // from and to nodes
								const long fromNode, const long toNode, // from and to nodes
								const tod_t t, const int planLeave, // time and whether this is a leave or arrive plan
								const ident_t idService );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // cistms_H