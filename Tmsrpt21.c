//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT21() - Book Pages
//
#include "TMSHeader.h"
#include <math.h>

#define MAXCOMMENTS 50

//
//  From QETXT.INI:
//
//  [TMSRPT21]
//  FILE=Tmsrpt21.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Division,VARCHAR,32,0,32,0,
//  FIELD3=Service,VARCHAR,32,0,32,0,
//  FIELD4=Route,VARCHAR,75,0,75,0,
//  FIELD5=Direction,VARCHAR,16,0,16,0,
//  FIELD6=Text,VARCHAR,168,0,168,0,
//

BOOL FAR TMSRPT21(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  TRIPSKey2Def   tripSave;
  float distance;
  HFILE hfOutputFile;
  BOOL  bGotOne;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  titleString[2][512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  longNames[RECORDIDS_KEPT][NODES_LONGNAME_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
  char  *tempString2;
  long  BASEPatternNodeList[RECORDIDS_KEPT];
  long  commentList[MAXCOMMENTS];
  long  subPatternNodeList[RECORDIDS_KEPT];
  long  POTime;
  long  PITime;
  long  deadheadTime;
  long  runNumber[2];
  long  reliefNode;
  long  absoluteRecord;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   nN;
  int   numComments;
  int   numLines;
  int   numNodes;
  int   numBASENodes;
  int   rcode2;
  int   seq;

  pPassedData->nReportNumber = 20;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_COMMENTS | RPFLAG_SERVICES | RPFLAG_DIVISIONS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numRoutes == 0 ||
        REPORTPARMS.numServices == 0 ||
        REPORTPARMS.numDivisions == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt21.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Loop through all the divisions
//
  bKeepGoing = TRUE;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numDivisions; nI++)
  {
    DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[nI];
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Loop through all the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Get all the runs for this division and service
//
      GetRunRecords(REPORTPARMS.pDivisionList[nI], REPORTPARMS.pServiceList[nJ]);
      if(m_numRunRecords == NO_RECORD)
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Loop through all the routes
//
      for(nK = 0; nK < REPORTPARMS.numRoutes; nK++)
      {
//
//  Loop through the directions
//
        for(nL = 0; nL < 2; nL++)
        {
          ROUTESKey0.recordID = REPORTPARMS.pRouteList[nK];
          rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          if(ROUTES.DIRECTIONSrecordID[nL] == NO_RECORD)
          {
            continue;
          }
          strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
          strcat(routeNumberAndName, " - ");
          strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
          trim(tempString, ROUTES_NAME_LENGTH);
          strcat(routeNumberAndName, tempString);
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nL];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Get the BASE pattern nodes for this route, service, and direction
//  Due to output limitations, the maximum number of nodes in one direction is 16
//
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nK];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          PATTERNSKey2.directionIndex = nL;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = 0;
          numBASENodes = 0;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nL &&
                PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              BASEPatternNodeList[numBASENodes++] = PATTERNS.NODESrecordID;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }  // while rcode2 == 0 on the pattern
//
//  Store the long names
//
          for(nM = 0; nM < numBASENodes; nM++)
          {
            NODESKey0.recordID = BASEPatternNodeList[nM];
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(longNames[nM], NODES.longName, NODES_LONGNAME_LENGTH);
            longNames[nM][NODES_LONGNAME_LENGTH] = '\0';
          }
//
//  Set up the title strings
//
          for(nM = 0; nM < 2; nM++)
          {
            sprintf(titleString[nM], "\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"",
                  divisionName, serviceName, routeNumberAndName, directionName);
            if(nM == 0)
            {
              strcat(titleString[nM], "Block   Run     Trip    Comment   P/O    ");
              for(nN = 0; nN < numBASENodes; nN++) 
              {
                strncpy(tempString, longNames[nN], 4);
                tempString[4] = '\0';
                strcat(titleString[nM], tempString);
                strcat(titleString[nM], "   ");
              }
              strcat(titleString[nM], "P/I    Run    Next     Next\"\r\n");
            }
            else
            {
              strcat(titleString[nM], "Number  Number  Number  Code      Time   ");
              for(nN = 0; nN < numBASENodes; nN++)
              {
                strncpy(tempString, &longNames[nN][4], 4);
                tempString[4] = '\0';
                strcat(titleString[nM], tempString);
                strcat(titleString[nM], "   ");
              }
              strcat(titleString[nM], "Time   Number Route    Trip\"\r\n"); 
            }
          }
//
//  Go through all the trips for this route, service, and direction
//
          numComments = 0;
          numLines = 42;
          TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nK];
          TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          TRIPSKey1.directionIndex = nL;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                TRIPS.directionIndex == nL)
          {
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this a pull-out?
//
            POTime = NO_TIME;
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
                POTime = GTResults.firstNodeTime - deadheadTime;
              }
            }
//
//  Get the run number(s) on the trip
//
            runNumber[0] = 0;
            runNumber[1] = 0;
            for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
            {
              if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
              {
                break;
              }
              if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                    (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                     GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                     GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
              {
                if(bGotOne)
                {
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  reliefNode = m_pRunRecordData[nM].startNODESrecordID;
                  break;
                }
                else
                {
                  runNumber[0] = m_pRunRecordData[nM].runNumber;
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  reliefNode = NO_RECORD;
                  bGotOne = TRUE;
                }
              }
            }
//
//  Get the comment code of the trip
//
            if(TRIPS.COMMENTSrecordID == NO_RECORD)
            {
              memset(commentCode, ' ', COMMENTS_CODE_LENGTH);
            }
            else
            {
              COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
              btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
              strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
              for(bGotOne = FALSE, nM = 0; nM < numComments; nM++)
              {
                if(commentList[nM] == COMMENTS.recordID)
                {
                  bGotOne = TRUE;
                  break;
                }
              }
              if(!bGotOne && numComments < MAXCOMMENTS)
              {
                commentList[numComments++] = COMMENTS.recordID;
              }
            }
            commentCode[COMMENTS_CODE_LENGTH] = '\0';
//
//  See if we have to send out the row headers again
//
            if(numLines > 41)
            {
              for(nM = 0; nM < 2; nM++)
              {
                sprintf(outputString, "%ld\t%s", seq++, titleString[nM]);
                _lwrite(hfOutputFile, outputString, strlen(outputString));
              }
              numLines = 2;
            }
//
//  Build the first part of the output string
//
            sprintf(outputString, "%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"",
                  seq++, divisionName, serviceName, routeNumberAndName, directionName);
            if(TRIPS.tripNumber <= 0)
            {
              sprintf(tempString, "%6ld  %6ld          %s ",
                    TRIPS.standard.blockNumber, runNumber[0], commentCode);
            }
            else
            {
              sprintf(tempString, "%6ld  %6ld  %6ld  %s ",
                    TRIPS.standard.blockNumber, runNumber[0], TRIPS.tripNumber, commentCode);
            }
            strcat(outputString, tempString);
            if(POTime == NO_TIME)
            {
              strcpy(tempString, "     ");
            }
            else
            {
              if(timeFormat == PREFERENCES_MILITARY)
              {
                strcat(outputString, " ");
              }
              strcpy(tempString, Tchar(POTime));
            }
            strcat(outputString, tempString);
            strcat(outputString, "  ");
//
//  Get the trip times
//
//  If necessary, get the pattern nodes on the trip
//
            if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
            {
              for(nM = 0; nM < numBASENodes; nM++)
              {
                if(timeFormat == PREFERENCES_MILITARY)
                {
                  strcat(outputString, " ");
                }
                strcat(outputString, Tchar(GTResults.tripTimes[nM]));
                if(BASEPatternNodeList[nM] == reliefNode)
                {
                  strcat(outputString, "* ");
                }
                else
                {
                  strcat(outputString, "  ");
                }
              }
            }
//
//  Not the BASE pattern - get the pattern and fill in the times
//
            else
            {
              PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
              PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
              PATTERNSKey2.directionIndex = TRIPS.directionIndex;
              PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              PATTERNSKey2.nodeSequence = 0;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              numNodes = 0;
              while(rcode2 == 0 &&
                    PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                    PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                    PATTERNS.directionIndex == TRIPS.directionIndex &&
                    PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
              {
                if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                {
                  subPatternNodeList[numNodes++] = PATTERNS.NODESrecordID;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
              }
//
//  Move along the pattern
//
              nN = 0;
              for(nM = 0; nM < numBASENodes; nM++)
              {
                if(nN >= numNodes)
                {
                  break;
                }
                if(BASEPatternNodeList[nM] == subPatternNodeList[nN])
                {
                  if(timeFormat == PREFERENCES_MILITARY)
                  {
                    strcat(outputString, " ");
                  }
                  strcat(outputString, Tchar(GTResults.tripTimes[nN]));
                  if(BASEPatternNodeList[nM] == reliefNode)
                  {
                    strcat(outputString, "* ");
                  }
                  else
                  {
                    strcat(outputString, "  ");
                  }
                  nN++;
                }
                else
                {
                  strcat(outputString, "       ");
                }
              }
//
//  Fill in some blanks if the last pattern node
//  isn't the same as the last BASE pattern node
//
              if(subPatternNodeList[numNodes - 1] != BASEPatternNodeList[numBASENodes - 1])
              {
                nN = 1;
                for(nM = numBASENodes - 2; nM >= 0 ; nM--)
                {
                  if(subPatternNodeList[numNodes - 1] == BASEPatternNodeList[nM])
                  {
                    break;
                  }
                  nN++;
                }
                for(nM = 0; nM < nN; nM++)
                {
                  strcat(outputString, "       ");
                }
              }
            }
//
//  Is this a pull-in?
//
            PITime = NO_TIME;
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
                PITime = GTResults.lastNodeTime + deadheadTime;
              }
            }
            if(PITime == NO_TIME)
            {
              strcat(outputString, "     ");
            }
            else
            {
              if(timeFormat == PREFERENCES_MILITARY)
              {
                strcat(outputString, " ");
              }
              strcat(outputString, Tchar(PITime));
            }
            strcat(outputString, "  ");
//
//  And the "To" run number
//
            sprintf(tempString, "%6ld  ", runNumber[1]);
            strcat(outputString, tempString);
//
//  Repostion the trips record to get the next route/trip
//  We'll assume "standard" blocks here - not "dropback"
//
            if(TRIPS.standard.blockNumber > 0)
            {
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
                ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
                btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                tempString[ROUTES_NUMBER_LENGTH] = '\0';
                strcat(outputString, tempString);
                strcat(outputString, " ");
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                strcat(outputString, Tchar(GTResults.firstNodeTime));
              }
            }
            strcat(outputString, "\"\r\n");
//
//  Write out the record and loop back
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            numLines++;
//
//  Reposition the Trips record and get the next one
//
            if(TRIPS.standard.blockNumber > 0)
            {
              memcpy(&TRIPS, &absoluteRecord, 4);
              btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }  // while TRIPS loop
//
//  Spit out the comments for this route/ser/direction
//
          for(nM = 0; nM < numComments; nM++)
          {
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            COMMENTSKey0.recordID = commentList[nM];
            btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(szarString, COMMENTS_CODE_LENGTH);
            strcat(szarString, ":");
            sprintf(outputString, "%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\r\n",
                  seq++, divisionName, serviceName,
                  routeNumberAndName, directionName, szarString);
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            szarString[0] = 0;
            for( tempString2 = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n");
                 tempString2;
                 tempString2 = strtok(NULL, "\r\n") )
            {
              strcpy(szarString, tempString2 );
              sprintf(outputString, "%ld\t\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t\"  %s\"\r\n",
                    seq++, divisionName, serviceName,
                    routeNumberAndName, directionName, szarString);
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
          }
        }  //  nL loop on directions
      }  //  nK loop on routes
    }  // nJ loop on services
  }  // nI loop on divisions
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}
