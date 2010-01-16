//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT86() - Version 2 of the download to Connexionz Real-Time Information System
//
//  [tripschedule.csv]
//  ColNameHeader = False
//  CharacterSet = ANSI
//  Format = CsvDelimited
//  col1=company_id Text
//  col2=service_id Text
//  col3=trip_no Long
//  col4=block_no Long
//  col5=duty_no Long
//  col6=route_pattern Text
//  col7=sign_code Text
//  col8=timepoints Text
//              
#define COMPANY_ID         "MVT"
#define OUTPUT_FOLDER_NAME "Connexionz Interface"

BOOL FAR TMSRPT86(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound;
  char  dummy[256];
  char  outputString[512];
  long  statbarTotal;
  long  saveTimeFormat;
  long  lastTripNumber;
  long  maxTrips;
  long  patternNodes[MAXTRIPTIMES];
  long  tempLong;
  int   nI;
  int   nJ;
  int   currentRoute;
  int   currentService;
  int   numRoutes;
  int   numServices;
  int   rcode2;
  int   numNodes;

  saveTimeFormat = timeFormat;
  timeFormat = PREFERENCES_MILITARY;
//
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  LoadString(hInst, nI, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  Open the output file
//
  hfOutputFile = _lcreat("tripschedule.csv", 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
//
//  Get the number of routes and services
//  for the benefit of the status bar
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    goto deallocate;
  }
  numRoutes = BSTAT.numRecords;
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    goto deallocate;
  }
  numServices = BSTAT.numRecords;
//
//  maxTrips
//
  rcode2 = btrieve(B_STAT, TMS_TRIPS, &BSTAT, dummy, 0);
  maxTrips = rcode2 == 0 ? BSTAT.numRecords : 0;
//
//  Verify that all trips in the system are numbered
//
  StatusBarText("Checking if all trips are numbered");
  rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  nI = 0;
  bFound = FALSE;
  lastTripNumber = NO_RECORD;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto deallocate;
    }
    StatusBar((long)nI, (long)maxTrips);
    if(TRIPS.tripNumber <= 0)
    {
      MessageBeep(MB_ICONQUESTION);
      LoadString(hInst, ERROR_344, tempString, TEMPSTRING_LENGTH);
      if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OK) != IDOK)
      {
        goto deallocate;
      }
      bFound = TRUE;
      break;
    }
    if(TRIPS.tripNumber > lastTripNumber)
    {
      lastTripNumber = TRIPS.tripNumber;
    }
    nI++;
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  }
  if(bFound)
  {
    StatusBarText("Renumbering Trips");
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    nI = 1;
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
      StatusBar((long)nI, (long)maxTrips);
      TRIPS.tripNumber = (long)nI;
      nI++;
      rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
    lastTripNumber = nI;
  }
//
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = numRoutes * numServices;
  currentRoute = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
//
//  Do not process employee shuttles
//
    if(!(ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE))
    {
//
//  Loop through the services
//
      currentService = 1;
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
//
//  Loop through all the trips on this route/service/direction
//
        for(nI = 0; nI < 2; nI++)
        {
          if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          {
            continue;
          }
//
//  Get the trips
//
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
          TRIPSKey1.directionIndex = nI;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nI)
          {
            DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
            btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
//
//  Display where we are in the status bar
//
            strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(tempString, ROUTES_NUMBER_LENGTH);
            strcat(tempString, " - ");
            strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
            trim(szarString, ROUTES_NAME_LENGTH);
            strcat(tempString, szarString);
            strcat(tempString, "\n");
            strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(szarString, SERVICES_NAME_LENGTH);
            strcat(tempString, szarString);
            strcat(tempString, " - ");
            strncpy(szarString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
            trim(szarString, DIRECTIONS_LONGNAME_LENGTH);
            strcat(tempString, szarString);
            StatusBarText(tempString);
            StatusBar((long)(currentRoute * (numServices - 1) + currentService), (long)statbarTotal);
            if(StatusBarAbort())
            {
              bKeepGoing = FALSE;
              goto deallocate;
            }
//
//  Build up the output string
//
//  Company Id
//
            sprintf(outputString, "%s,", COMPANY_ID);
//
//  Service name
//
            strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(tempString, SERVICES_NAME_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  Trip number
//
            sprintf(tempString, "%ld,", TRIPS.tripNumber);
            strcat(outputString, tempString);
//
//  Block number
//
            sprintf(tempString, "%ld,", TRIPS.standard.blockNumber);
            strcat(outputString, tempString);
//
//  Duty number (currently unused)
//
            strcat(outputString, ",");
//
//  Pattern name
//
            PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            trim(tempString, PATTERNNAMES_NAME_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  Sign code
//
            SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
            if(rcode2 != 0)
            {
              strcpy(tempString, "");
            }
            else
            {
              strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
              trim(tempString, SIGNCODES_CODE_LENGTH);
            }
            strcat(outputString, tempString);
            strcat(outputString, ",");
//
//  Nodes times
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the pattern nodes
//
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            numNodes = 0;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                patternNodes[numNodes] = PATTERNS.NODESrecordID;
                numNodes++;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
//
//  tripTimes[] contains the times at the timepoints in this pattern
//
//  We're only interested in departure times, so if we
//  run into an arrive/leave, skip over the arrive time
//
            for(nJ = 0; nJ < numNodes; nJ++)
            {
              if(nJ < numNodes - 1)
              {
                if(NodesEquivalent(patternNodes[nJ], patternNodes[nJ + 1], &tempLong))
                {
                  continue;
                }
              }
              strcat(outputString, Tchar(GTResults.tripTimes[nJ]));
              strcat(outputString, " ");
            }
//
//  End of record
//
            strcat(outputString, "\r\n");
//
//  Write it out
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Next trip
//
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  Next direction
//
        }  // nI
//
//  Next service
//
        currentService++;
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
//
//  Not a shuttle
//
    }
//
//  Next route
//
    currentRoute++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    timeFormat = saveTimeFormat;
    _lclose(hfOutputFile);
    StatusBarEnd();
    chdir("..");
//
//  Let him know
//
  strcpy(tempString, "Download of tripschedule.csv complete\n");
  MessageBox(hWndMain, tempString, TMS, MB_OK);

  if(!bKeepGoing)
  {
    return(FALSE);
  }
//
//  All done
//
  return(TRUE);
}

