//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT24() - Download to Connexionz Real-Time Information System
//
//  This unload will typically be performed at the stand-alone level
//  by the TMS-Connexionz program, but is included here for the
//  purposes of completeness.
//
//  From QETXT.INI:
//
//  [TMSRPT24]
//  FILE=Connexionz.txt
//  FLN=0
//  TT=Fixed
//  Charset=ANSI
//  DS=.
//  FIELD1=Operator,VARCHAR,8,0,8,0,
//  FIELD2=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD3=RouteName,VARCHAR,64,0,64,0,
//  FIELD4=ServiceName,VARCHAR,32,0,32,0,
//  FIELD5=DirectionName,VARCHAR,16,0,16,0,
//  FIELD6=PatternName,VARCHAR,16,0,16,0,
//  FIELD7=TripNumber,NUMERIC,10,0,10,0,
//  FIELD8=BlockNumber,NUMERIC,10,0,10,0,
//  FIELD9=TripComment,VARCHAR,32,0,32,0,
//  FIELD10=SignCode,VARCHAR,32,0,32,0,
//  FIELD11=NumberOfNodes,NUMERIC,5,0,5,0,
//  FIELD12=NodeData,VARCHAR,256,0,256,0,
//
#define COMMENT_UNLOAD_LENGTH 32

typedef struct TMSRPT24PATStruct
{
  long PATTERNNAMESrecordID;
  int  numNodes;
  char abbrName[RECORDIDS_KEPT][NODES_ABBRNAME_LENGTH + 1];
} TMSRPT24PATDef;

BOOL FAR TMSRPT24(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  TMSRPT24PATDef PAT[50];
  int   thisPattern;
  int   numPatterns;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  char  dummy[256];
  char  outputString[512];
  char  commentsBuffer[sizeof(COMMENTS) + COMMENT_UNLOAD_LENGTH];
  char  *ptr;
  long  statbarTotal;
  long  saveTimeFormat;
  long  prevPattern;
  int   nI;
  int   nJ;
  int   currentRoute;
  int   currentService;
  int   numRoutes;
  int   numServices;
  int   rcode2;

  saveTimeFormat = timeFormat;
  timeFormat = PREFERENCES_MILITARY;
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\Connexionz.txt");
  hfOutputFile = _lcreat(tempString, 0);
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
            continue;
//
//  Read all the individual patterns (and nodes) for this service / route / direction
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          prevPattern = NO_RECORD;
          numPatterns = -1;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nI)
          {
            if(StatusBarAbort())
            {
              bKeepGoing = FALSE;
              goto deallocate;
            }
            if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
            {
              numPatterns++;
              prevPattern = PATTERNS.PATTERNNAMESrecordID;
              PAT[numPatterns].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
              PAT[numPatterns].numNodes = 0;
            }
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(PAT[numPatterns].abbrName[PAT[numPatterns].numNodes],
                    NODES.abbrName, NODES_ABBRNAME_LENGTH);
              PAT[numPatterns].abbrName[PAT[numPatterns].numNodes][NODES_ABBRNAME_LENGTH] = '\0';
              PAT[numPatterns].numNodes++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          numPatterns++;
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
//  Operator
//
            strcpy(tempString, "");
            if(ROUTES.COMMENTSrecordID != NO_RECORD)
            {
              recordLength[TMS_COMMENTS] = sizeof(commentsBuffer);
              COMMENTSKey0.recordID = ROUTES.COMMENTSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, &commentsBuffer, &COMMENTSKey0, 0);
              recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
              if(rcode2 == 0 || rcode2 == 22)
              {
                memcpy(&COMMENTS, &commentsBuffer, sizeof(COMMENTS));
                strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
                trim(tempString, COMMENTS_CODE_LENGTH);
              }
            }
            pad(tempString, COMMENT_UNLOAD_LENGTH);
            tempString[COMMENT_UNLOAD_LENGTH] = '\0';
            strcpy(outputString, tempString);
//
//  Route number
//
            strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
            tempString[ROUTES_NUMBER_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Route name
//
            strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
            tempString[ROUTES_NAME_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Service name
//
            strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
            tempString[SERVICES_NAME_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Direction name
//
            strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
            tempString[DIRECTIONS_LONGNAME_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Pattern name
//
            PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            tempString[PATTERNNAMES_NAME_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Trip number
//
            sprintf(tempString, "%10ld", TRIPS.tripNumber);
            strcat(outputString, tempString);
//
//  Block number
//
            sprintf(tempString, "%10ld", TRIPS.standard.blockNumber);
            strcat(outputString, tempString);
//
//  Trip Comment
//
//  (Max - 32 chars)
//
            strcpy(tempString, "");
            if(TRIPS.COMMENTSrecordID != NO_RECORD)
            {
              recordLength[TMS_COMMENTS] = sizeof(commentsBuffer);
              COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, &commentsBuffer, &COMMENTSKey0, 0);
              recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
              if(rcode2 == 0 || rcode2 == 22)
              {
                memcpy(&COMMENTS, &commentsBuffer, sizeof(COMMENTS));
                strncpy(tempString, &commentsBuffer[sizeof(COMMENTS)], COMMENT_UNLOAD_LENGTH);
                tempString[COMMENT_UNLOAD_LENGTH] ='\0';
                strupr(tempString);
                if(strncmp(tempString, "VIA", 3) != 0)
                  strcpy(tempString, "");
              }
            }
            pad(tempString, COMMENT_UNLOAD_LENGTH);
            tempString[COMMENT_UNLOAD_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Sign Code
//
            strcpy(tempString, "");
            if(TRIPS.SIGNCODESrecordID != NO_RECORD)
            {
              SIGNCODESKey0.recordID = TRIPS.SIGNCODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
              if(rcode2 == 0)
                strncpy(tempString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
            }
            pad(tempString, SIGNCODES_TEXT_LENGTH);
            tempString[SIGNCODES_TEXT_LENGTH] = '\0';
            strcat(outputString, tempString);
//
//  Nodes and node times
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Locate the pattern in PAT
//
            for(nJ = 0; nJ < numPatterns; nJ++)
            {
              if(PAT[nJ].PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
              {
                thisPattern = nJ;
                break;
              }
            }
//
//  Add the number of nodes to the output record
//
            sprintf(tempString, "%5d", PAT[thisPattern].numNodes);
            strcat(outputString, tempString);
//
//  tripTimes[] contains the times at the timepoints in this pattern
//
            for(nJ = 0; nJ < PAT[thisPattern].numNodes; nJ++)
            {
              strcat(outputString, PAT[thisPattern].abbrName[nJ]);
              strcat(outputString, Tchar(GTResults.tripTimes[nJ]));
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
//
//  Let him know
//
  strcpy(tempString, "Connexionz.txt was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);

  if(!bKeepGoing)
    return(FALSE);
//
//  All done
//
  return(TRUE);
}

