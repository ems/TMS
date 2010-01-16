//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT34() - Productivity Report (2)
//
//  From QETXT.INI:
//
//    [TMSRPT34]
//    FILE=TMSRPT34.TXT
//    FLN=0
//    TT=Tab
//    FIELD1=Sequence,NUMERIC,7,0,7,0,
//    FIELD2=Division,VARCHAR,32,0,32,0,
//    FIELD3=Route,VARCHAR,75,0,75,0,
//    FIELD4=Service,VARCHAR,32,0,32,0,
//    FIELD5=RunningTime,NUMERIC,8,0,8,0,
//    FIELD6=LayoverTime,NUMERIC,8,0,8,0,
//    FIELD7=RevenueDistance,NUMERIC,7,2,7,0,
//    FIELD8=DeadheadTime,NUMERIC,8,0,8,0,
//    FIELD9=DeadheadDistance,NUMERIC,7,2,7,0,
//    FIELD10=NumberOfTrips,NUMERIC,8,0,8,0,
//

#include "TMSHeader.h"
#include <math.h>

static char szSection[] = "Report33";

#define TMSRPT34_BYROUTEONLY_FLAG    0x0001
#define TMSRPT34_HHMM_FLAG           0x0002
#define TMSRPT34_INCLUDESCHOOL_FLAG  0x0004
#define TMSRPT34_INCLUDESHUTTLE_FLAG 0x0010
#define TMSRPT34_BEFORE_FLAG         0x0020
#define TMSRPT34_AFTER_FLAG          0x0040
#define TMSRPT34_ALLDAY_FLAG         0x0100
typedef struct TMSRPT34Struct
{
  long flags;
  long time;
} TMSRPT34Def;

//#define DEBUGTMSRPT34

BOOL  CALLBACK SPRFILTERMsgProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR TMSRPT34(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  TMSRPT34Def    RPT34PARMS;
  HANDLE hOutputFile;
#ifdef DEBUGTMSRPT34
  HANDLE hDebugFile;
#endif
  DWORD dwBytesWritten;
  TRIPSKey2Def tripSave;
  double totalRevenueDistance;
  double totalDeadheadDistance;
  float longitude;
  float latitude;
  float distance;
  double tripDistance;
  long  statbarTotal;
  long  numTrips;
  long  equivalentTravelTime;
  long  fromNode;
  long  fromTime;
  long  fromRoute;
  long  fromService;
  long  fromPattern;
  long  totalRunningTime;
  long  layoverTime;
  long  totalLayoverTime;
  long  deadheadTime;
  long  totalDeadheadTime;
  long  absoluteRecord;
  long  previousNode;
  long  flags;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bDoCalculatedDistance;
  BOOL  bFirstNode;
  BOOL  bPreviousWasStop;
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   sequence = 0;

  pPassedData->nReportNumber = 33;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.PATTERNNAMESrecordID = NO_RECORD;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS;

  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT34.txt");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Open the debug file
//
#ifdef DEBUGTMSRPT34
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\DEBUG34.txt");
  hDebugFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
//
//  So far so good - see what's to be filtered
//
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SPRFILTER),
        hWndMain, (DLGPROC)SPRFILTERMsgProc, (LPARAM)&RPT34PARMS);
  if(!bRC)
    goto deallocate;
//
//  Make sure the runtimes are up to date
//
  m_bEstablishRUNTIMES = TRUE;
//
//  Set up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
//
//  Get the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[0];
  rcode2 = btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  if(rcode2 != 0)
    strcpy(divisionName, "");
  else
  {
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
  }
//
//  Loop through the services
//
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    SERVICESKey0.recordID = REPORTPARMS.pServiceList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
      continue;
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through the routes
//
    for(nJ = 0; nJ < REPORTPARMS.numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      ROUTESKey0.recordID = REPORTPARMS.pRouteList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 != 0)
        continue;
//
//  Check on school/shuttle routes
//
      if((ROUTES.flags & ROUTES_FLAG_SCHOOL) &&
            !(RPT34PARMS.flags & TMSRPT34_INCLUDESCHOOL_FLAG))
        continue;
      if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) &&
            !(RPT34PARMS.flags & TMSRPT34_INCLUDESHUTTLE_FLAG))
        continue;
//
//  Build up the route number and name
//
      strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
      strcat(routeNumberAndName, " - ");
      strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(szarString, ROUTES_NAME_LENGTH);
      strcat(routeNumberAndName, szarString);
      StatusBarText(routeNumberAndName);
      StatusBar((long)(nI * REPORTPARMS.numRoutes + nJ), (long)statbarTotal);
      numTrips = 0;   
      totalRunningTime = 0;
      totalRevenueDistance = 0.0;
      totalLayoverTime = 0;
      totalDeadheadTime = 0;
      totalDeadheadDistance = 0.0;
//
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          continue;
//
//  Get the direction name if we're reporting by route/direction,
//  and reset the totals to zero while we're at it
//
        if(!(RPT34PARMS.flags & TMSRPT34_BYROUTEONLY_FLAG))
        {
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
          numTrips = 0;
          totalRunningTime = 0;
          totalRevenueDistance = 0.0;
          totalLayoverTime = 0;
          totalDeadheadTime = 0;
          totalDeadheadDistance = 0.0;
        }
//
//  Check to see if there are any non-geocoded nodes on this route/ser/dir.
//  If there are, we won't bother with distances.
//
        bDoCalculatedDistance = TRUE;
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nK &&
              PATTERNSKey2.PATTERNNAMESrecordID == basePatternRecordID)
        {
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          longitude = NODES.longitude;
          latitude = NODES.latitude;
          if(longitude == 0.0 || latitude == 0.0)
          {
            bDoCalculatedDistance = FALSE;
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  And cycle through the trips
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nK)
        {
          numTrips++;
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Check on any before and after constraints
//
          if((RPT34PARMS.flags & TMSRPT34_ALLDAY_FLAG) ||
                ((RPT34PARMS.flags & TMSRPT34_BEFORE_FLAG) && RPT34PARMS.time > GTResults.firstNodeTime) ||
                ((RPT34PARMS.flags & TMSRPT34_AFTER_FLAG ) && RPT34PARMS.time < GTResults.firstNodeTime))
          {
            totalRunningTime += (GTResults.lastNodeTime - GTResults.firstNodeTime);
            totalRevenueDistance += GTResults.tripDistance;
//
//  If we're calculating the distance, go through the pattern to get the distances,
//  since GenerateTrip only return distances betweeen timepoints.  Only do this, though,
//  if the distances weren't already hard-wired into the connections table.
//
            if(bDoCalculatedDistance && (GTResults.returnFlags & GENERATETRIP_FLAG_DISTANCECALC))
            {
              PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
              PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
              PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              PATTERNSKey2.directionIndex = TRIPS.directionIndex;
              PATTERNSKey2.nodeSequence = -1;
              rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              tripDistance = 0.0;
              bFirstNode = TRUE;
              while(rcode2 == 0 &&
                    PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                    PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                    PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
                    PATTERNS.directionIndex == TRIPS.directionIndex)
              {
                NODESKey0.recordID = PATTERNS.NODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                if(bFirstNode)
                {
                  bFirstNode = FALSE;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = previousNode;
                  GCTData.toNODESrecordID = PATTERNS.NODESrecordID;
                  GCTData.timeOfDay = GTResults.firstNodeTime;
                  flags = (NODES.flags & NODES_FLAG_STOP || bPreviousWasStop) ? GCT_FLAG_ATLEASTONESTOP : 0;
                  flags |= GCT_FLAG_RUNNINGTIME;
                  GetConnectionTime(flags, &GCTData, &distance);
                  if(distance < 0.0)  // this shouldn't happen - intercepted earlier
                  {
                    distance = -distance;
                  }
                  tripDistance += distance;
                }
                previousNode = NODES.recordID;
                bPreviousWasStop = NODES.flags & NODES_FLAG_STOP;
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              }
#ifdef DEBUGTMSRPT34
              sprintf(tempString, "%6ld\t%f\t%f\r\n", TRIPS.recordID, tripDistance, totalRevenueDistance);
              WriteFile(hDebugFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
#endif
//
//  This gets this report in synch with TMSRPT26.  Namely,
//  TMSRPT26 sends %6.2f to Crystal, whereas we do the
//  calculation here.  Without this, we're off on the distances.
//
              sprintf(tempString, "%6.2f", tripDistance);
              tripDistance = atof(tempString);
              totalRevenueDistance += tripDistance;;
            }
//              totalRevenueDistance += GTResults.tripDistance;
//
//  Check on P/O and P/I deadheads
//
//  Pull/Out
//
            if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
              GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
              GCTData.timeOfDay = GTResults.firstNodeTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(deadheadTime != NO_TIME)
              {
                totalDeadheadTime += deadheadTime;
                totalDeadheadDistance += distance;
              }
            }
//
//  Pull/In
//
            if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
              GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
              GCTData.timeOfDay = GTResults.lastNodeTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(deadheadTime != NO_TIME)
              {
                totalDeadheadTime += deadheadTime;
                totalDeadheadDistance += distance;
              }
            }
//
//  Repostion the trips record to get the next trip on the block
//  We'll assume "standard" blocks here - not "dropback"
//
            if(TRIPS.standard.blockNumber > 0)
            {
              fromNode = GTResults.lastNODESrecordID;
              fromTime = GTResults.lastNodeTime;
              fromRoute = TRIPS.ROUTESrecordID;
              fromService = TRIPS.SERVICESrecordID;
              fromPattern = TRIPS.PATTERNNAMESrecordID;
              tripSave.assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
              tripSave.RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
              tripSave.SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
              tripSave.blockNumber = TRIPS.standard.blockNumber;
              btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              memcpy(&absoluteRecord, &TRIPS, 4);
              rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
              if(rcode2 == 0 &&
                    TRIPS.standard.assignedToNODESrecordID == tripSave.assignedToNODESrecordID &&
                    TRIPS.standard.RGRPROUTESrecordID == tripSave.RGRPROUTESrecordID &&
                    TRIPS.standard.SGRPSERVICESrecordID == tripSave.SGRPSERVICESrecordID &&
                    TRIPS.standard.blockNumber == tripSave.blockNumber)
              {
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Check on node equivalence
//
                if(NodesEquivalent(fromNode, GTResults.firstNODESrecordID, &equivalentTravelTime))
                {
                  layoverTime = GTResults.firstNodeTime - fromTime;
                  deadheadTime = 0;
                }
//
//  Not equivalent - check on an intraline deadhead
//
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = fromPattern;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = fromRoute;
                  GCTData.fromSERVICESrecordID = fromService;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = fromNode;
                  GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                  GCTData.timeOfDay = fromTime;
//
//  If there's no deadhead time, set it to zero and call everything layover
//
                  deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  if(deadheadTime == NO_TIME)
                  {
                    deadheadTime = 0;
                    layoverTime = GTResults.firstNodeTime - fromTime;
                  }
//
//  There was a deadhead time.  Calculate the layover, and verify
//  that the two deadhead nodes are geocoded (for distance purposes)
//
                  else
                  {
                    layoverTime = (GTResults.firstNodeTime - fromTime - deadheadTime);
                    totalDeadheadDistance += distance;
                  }
                }
                totalLayoverTime += layoverTime;
                totalDeadheadTime += deadheadTime;
              }
//
//  Reposition the Trips record and get the next one
//
              memcpy(&TRIPS, &absoluteRecord, 4);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
        if(numTrips > 0 && !(RPT34PARMS.flags & TMSRPT34_BYROUTEONLY_FLAG))
        {
          sprintf(tempString, "%d\t%s\t\"%s %s\"\t%s\t%ld\t%ld\t%6.2f\t%ld\t%6.2f\t%ld\r\n",
                sequence, divisionName, routeNumberAndName, directionName, serviceName,
                totalRunningTime, totalLayoverTime, totalRevenueDistance, totalDeadheadTime,
                totalDeadheadDistance, numTrips);
          WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
          sequence++;
        }
      }  // nK
      if(numTrips > 0 && (RPT34PARMS.flags & TMSRPT34_BYROUTEONLY_FLAG))
      {
        sprintf(tempString, "%d\t%s\t\"%s\"\t%s\t%ld\t%ld\t%6.2f\t%ld\t%6.2f\t%ld\r\n",
              sequence, divisionName, routeNumberAndName, serviceName,
              totalRunningTime, totalLayoverTime, totalRevenueDistance, totalDeadheadTime,
              totalDeadheadDistance, numTrips);
        WriteFile(hOutputFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
        sequence++;
      }
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    CloseHandle(hOutputFile);
#ifdef DEBUGTMSRPT34
    CloseHandle(hDebugFile);
#endif
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
//  Write out the report name to TMS.INI so Crystal can pick it up
//
  LoadString(hInst, (RPT34PARMS.flags & TMSRPT34_BYROUTEONLY_FLAG) ? TEXT_259 : TEXT_260,
        tempString, TEMPSTRING_LENGTH);
  strcat(tempString, " - ");
//
//  Before/After/All Day
//
  if(RPT34PARMS.flags & TMSRPT34_ALLDAY_FLAG)
  {
    LoadString(hInst, TEXT_254, szarString, sizeof(szarString));
  }
  else
  {
    LoadString(hInst, (RPT34PARMS.flags & TMSRPT34_BEFORE_FLAG) ? TEXT_248 : TEXT_249,
          szarString, sizeof(szarString));
    strcat(szarString, " ");
    strcat(szarString, Tchar(RPT34PARMS.time));
  }
  strcat(tempString, szarString);
  strcat(tempString, ", ");
//
//  School trips
//
  LoadString(hInst, (RPT34PARMS.flags & TMSRPT34_INCLUDESCHOOL_FLAG) ? TEXT_250 : TEXT_251,
        szarString, sizeof(szarString));
  strcat(tempString, szarString);
  strcat(tempString, ", ");
//
//  Employee shuttles
//
  LoadString(hInst, (RPT34PARMS.flags & TMSRPT34_INCLUDESHUTTLE_FLAG) ? TEXT_252 : TEXT_253,
        szarString, sizeof(szarString));
  strcat(tempString, szarString);
//
//  Save it to the INI file
//
  WritePrivateProfileString("Reports", "CCReportName", tempString, TMSINIFile);

  return(TRUE);
}

BOOL CALLBACK SPRFILTERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static TMSRPT34Def *pRpt34Parms;
  static HANDLE hCtlBYROUTEONLY;
  static HANDLE hCtlBYROUTEANDDIRECTION;
  static HANDLE hCtlHHMM;
  static HANDLE hCtlDECIMAL;
  static HANDLE hCtlSCHOOL;
  static HANDLE hCtlSHUTTLE;
  static HANDLE hCtlNA;
  static HANDLE hCtlBEFORE;
  static HANDLE hCtlAFTER;
  static HANDLE hCtlTIME;
  long   time;
  long   tempLong;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Validate the pointer
//
      pRpt34Parms = (TMSRPT34Def *)lParam;
      if(pRpt34Parms == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlBYROUTEONLY = GetDlgItem(hWndDlg, SPRFILTER_BYROUTEONLY);
      hCtlBYROUTEANDDIRECTION = GetDlgItem(hWndDlg, SPRFILTER_BYROUTEANDDIRECTION);
      hCtlHHMM = GetDlgItem(hWndDlg, SPRFILTER_HHMM);
      hCtlDECIMAL = GetDlgItem(hWndDlg, SPRFILTER_DECIMAL);
      hCtlSCHOOL = GetDlgItem(hWndDlg, SPRFILTER_SCHOOL);
      hCtlSHUTTLE = GetDlgItem(hWndDlg, SPRFILTER_SHUTTLE);
      hCtlNA = GetDlgItem(hWndDlg, SPRFILTER_NA);
      hCtlBEFORE = GetDlgItem(hWndDlg, SPRFILTER_BEFORE);
      hCtlAFTER = GetDlgItem(hWndDlg, SPRFILTER_AFTER);
      hCtlTIME = GetDlgItem(hWndDlg, SPRFILTER_TIME);
//
//  Get whatever's in TMS.INI and populate the dialog
//
//  Flags
//
      tempLong = GetPrivateProfileInt(szSection, "flags", 0, TMSINIFile);
//
//  By route only or by route and direction
//
      if(tempLong & TMSRPT34_BYROUTEONLY_FLAG)
        SendMessage(hCtlBYROUTEONLY, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlBYROUTEANDDIRECTION, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Output time format
//
      if(tempLong & TMSRPT34_HHMM_FLAG)
        SendMessage(hCtlHHMM, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlDECIMAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Include school routes
//
      if(tempLong & TMSRPT34_INCLUDESCHOOL_FLAG)
        SendMessage(hCtlSCHOOL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Include shuttle routes
//
      if(tempLong & TMSRPT34_INCLUDESHUTTLE_FLAG)
        SendMessage(hCtlSHUTTLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Before/after times
//
      if(!(tempLong & TMSRPT34_BEFORE_FLAG) && !(tempLong & TMSRPT34_AFTER_FLAG))
      {
        SendMessage(hCtlNA, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        EnableWindow(hCtlTIME, FALSE);
      }
      else
      {
        time = GetPrivateProfileInt(szSection, "time", NO_TIME, TMSINIFile);
        if(tempLong & TMSRPT34_BEFORE_FLAG)
          SendMessage(hCtlBEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        else
          SendMessage(hCtlAFTER, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hCtlTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)Tchar(time));
      }
//
//  Done initializing
//
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0L);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  N/A - Before - After
//
        case SPRFILTER_NA:
        case SPRFILTER_BEFORE:
        case SPRFILTER_AFTER:
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            EnableWindow(hCtlTIME, FALSE);
            SendMessage(hCtlTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          }
          else
          {
            EnableWindow(hCtlTIME, TRUE);
          }
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Get the values from the dialog
//
//  If before or after is selected, then there has to be a time
//
          if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ||
                SendMessage(hCtlAFTER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            SendMessage(hCtlTIME, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            if(strcmp(tempString, "") == 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_281, hCtlTIME);
              break;
            }
          }
//
//  Build up the flags
//
          pRpt34Parms->flags = 0;
//
//  By route only or by route and direction
//
          if(SendMessage(hCtlBYROUTEONLY, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt34Parms->flags |= TMSRPT34_BYROUTEONLY_FLAG;
//
//  Output time format
//
          if(SendMessage(hCtlHHMM, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt34Parms->flags |= TMSRPT34_HHMM_FLAG;
//
//  Include school routes
//
          if(SendMessage(hCtlSCHOOL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt34Parms->flags |= TMSRPT34_INCLUDESCHOOL_FLAG;
//
//  Include shuttle routes
//
          if(SendMessage(hCtlSHUTTLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRpt34Parms->flags |= TMSRPT34_INCLUDESHUTTLE_FLAG;
//
//  Before/after times
//
          if(SendMessage(hCtlNA, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            pRpt34Parms->flags |= TMSRPT34_ALLDAY_FLAG;
            pRpt34Parms->time = NO_TIME;
          }
          else
          {
            SendMessage(hCtlTIME, WM_GETTEXT,
                  (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
            pRpt34Parms->time = cTime(tempString);
            if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              pRpt34Parms->flags |= TMSRPT34_BEFORE_FLAG;
            else
              pRpt34Parms->flags |= TMSRPT34_AFTER_FLAG;
          }
//
//  Write the values out to TMS.INI
//
//  Flags
//
          ltoa(pRpt34Parms->flags, tempString, 10);
          WritePrivateProfileString(szSection, "flags", tempString, TMSINIFile);
          if(pRpt34Parms->flags & TMSRPT34_HHMM_FLAG)
            WritePrivateProfileString("Reports", "OutputTime", "HoursMinutes", TMSINIFile);
          else
            WritePrivateProfileString("Reports", "OutputTime", "Decimal", TMSINIFile);
//
//  Before / after times
//
          ltoa(pRpt34Parms->time, tempString, 10);
          WritePrivateProfileString(szSection, "time", tempString, TMSINIFile);
//
//  All done
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
