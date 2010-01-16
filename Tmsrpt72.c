// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to RouteMatch
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "RouteMatch Interface"

BOOL FAR TMSRPT72(TMSRPTPassedDataDef *pPassedData)
{
  MIFLINESDef          MIFLINES[TMSRPT61_MAXMIFLINES];
  MIFPASSEDDATADef     MIFPASSEDDATA;
  HFILE hfOutputFile;
  HFILE hfErrorLog;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  long  SERVICESrecordID;
  char  outputString[1024];
  char  outputStringSave[1024];
  char *pszReportName;
  char  outputFileName[64];
  char *ptr;
  int   nI, nJ;
  int   rcode2;
  int   seq;
  int   mask[] = {2, 4, 8, 16, 32, 64, 1};
  int   numMIFLINES;
  int   MIFPosition;
  int   MIFStart;

  bFinishedOK = FALSE;
//
//  Maintain the distance measure
//
  distanceMeasureSave = distanceMeasure;
  distanceMeasure = PREFERENCES_MILES;
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
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    goto done;
  }
  bGotError = FALSE;
  pPassedData->nReportNumber = 71;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  ==============================
//  Download file - FixedRoute.csv
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "FixedRoute.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "ID,Direction,AgencyID,ExistenceStartDate,ExistenceEndDate,InternalName,");
  strcat(outputString, "RouteColor,ServiceBoundaryColor,WeeklyRecurrence,LongLatSequence,Timepoint,");
  strcat(outputString, "Longitude,Latitude\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));

//
//  Get the services from ROSTERPARMS
//
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    if(ROSTERPARMS.serviceDays[nI] == NO_RECORD)
    {
      continue;
    }
    SERVICESrecordID = ROSTERPARMS.serviceDays[nI];
//
//  Cycle through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    while(rcode2 == 0)
    {
//
//  Direction
//
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
//
//  There must be a BASE pattern on this route/ser/dir
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
        PATTERNSKey2.directionIndex = nJ;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        if(rcode2 != 0 ||
              PATTERNS.ROUTESrecordID != ROUTES.recordID ||
              PATTERNS.SERVICESrecordID != SERVICESrecordID ||
              PATTERNS.directionIndex != nJ ||
              PATTERNS.PATTERNNAMESrecordID != basePatternRecordID)
        {
          continue;
        }
//
//  ID
//
        sprintf(outputString, "%ld,", ROUTES.recordID);
//
//  Direction
//
        strcat(outputString, (nJ == 0 ? "0," : "1,"));
//
//  AgencyID
//
        strcat(outputString, "1,");
//
//  ExistenceStartDate
//
        sprintf(tempString, "%ld,", effectiveDate);
        strcat(outputString, tempString);
//
//  ExistenceEndDate
//
        strcat(outputString, "99991231,");
//
//  InternalName
//
        strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szarString, ROUTES_NUMBER_LENGTH);
        strcat(szarString, " - ");
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(szarString, tempString);
        strcat(outputString, szarString);
        strcat(outputString, ",");
//
//  RouteColor
//
        strcat(outputString, ",");
//
//  ServiceBoundaryColor
//
        strcat(outputString, ",");
//
//  WeeklyRecurrence
//
        sprintf(tempString, "%d,", mask[nI]);
        strcat(outputString, tempString);
        strcpy(outputStringSave, outputString);
//  
//  Establish the route tracings file name
//
        strcpy(MIFPASSEDDATA.szMIFFileName, szRouteTracingsFolder);
        strcat(MIFPASSEDDATA.szMIFFileName, "\\rt");
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        if(isalpha(tempString[strlen(tempString) - 1]))
        {
          tempString[strlen(tempString) - 1] -= 16;
        }
        else
        {
          strcat(tempString, "0");
        }
        sprintf(szarString, "%08ld%d", atol(tempString), nJ);
        strcat(MIFPASSEDDATA.szMIFFileName, szarString);
        strcat(szarString, "\r\n");
        _lwrite(hfErrorLog, szarString, strlen(szarString));
//
//  Build the MIFLINES structure based on the BASE pattern
//
        strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szarString, ROUTES_NUMBER_LENGTH);
        sprintf(tempString, "*INF - Route is %s\r\n", szarString);
        _lwrite(hfErrorLog, tempString, strlen(tempString));
        MIFPASSEDDATA.ROUTESrecordID = ROUTES.recordID;
        MIFPASSEDDATA.SERVICESrecordID = 1L;
        MIFPASSEDDATA.directionIndex = nJ;
        MIFPASSEDDATA.PATTERNNAMESrecordID = basePatternRecordID;
        MIFPASSEDDATA.hfErrorLog = hfErrorLog;
        numMIFLINES = MIFDigest(&MIFPASSEDDATA, &MIFLINES[0]);
        if(numMIFLINES >= TMSRPT61_MAXMIFLINES)
        {
          sprintf(tempString, "*ERR - Number of MIF Lines exceeds %d\r\n", TMSRPT61_MAXMIFLINES);
          _lwrite(hfErrorLog, tempString, strlen(tempString));
          bGotError = TRUE;
        }
//#define DUMPLINES

#ifdef DUMPLINES
        sprintf(tempString, "*Inf - Dump of line arrangement in map\r\n");
        _lwrite(hfErrorLog, tempString, strlen(tempString));
        for(nJ = 0; nJ < numMIFLINES; nJ++)
        {
          pFrom = &MIFLINES[nJ].from;
          pTo = &MIFLINES[nJ].to;
//
//  From Node
//
          if(pFrom->NODESrecordID == NO_RECORD)
          {
            strcpy(szFromNode, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szFromNode, NODES_ABBRNAME_LENGTH);
          }
//
//  To Node
//
          if(pTo->NODESrecordID == NO_RECORD)
          {
            strcpy(szToNode, "");
          }
          else
          {
            NODESKey0.recordID = pTo->NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szToNode, NODES_ABBRNAME_LENGTH);
          }
//
//  From stop
//
          if(pFrom->associatedStopNODESrecordID == NO_RECORD)
          {
            strcpy(szFromStop, "");
          }
          else
          {
            NODESKey0.recordID = pFrom->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szFromStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szFromStop, NODES_LONGNAME_LENGTH);
          }
//
//  To stop
//
          if(pTo->associatedStopNODESrecordID == NO_RECORD)
          {
            strcpy(szToStop, "");
          }
          else
          {
            NODESKey0.recordID = pTo->associatedStopNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szToStop, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szToStop, NODES_LONGNAME_LENGTH);
          }
          sprintf(tempString, "%4d,\"%4s\",\"%4s\",%10.7f,%10.7f,\"%4s\",\"%4s\",%10.7f,%10.7f\r\n", nJ,
                szFromNode, szFromStop, pFrom->longitude, pFrom->latitude,
                szToNode, szToStop, pTo->longitude, pTo->latitude);
          _lwrite(hfErrorLog, tempString, strlen(tempString));
        }
#endif
//
//  Cycle through the BASE pattern
//
        seq = 1;
        MIFStart = 0;
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
        PATTERNSKey2.directionIndex = nJ;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICESrecordID &&
              PATTERNS.directionIndex == nJ &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          strcpy(outputString, outputStringSave);
//
//  LongLatSequence
//
          sprintf(tempString, "%d,", seq++);
          strcat(outputString, tempString);
//
//  Timepoint/Stop
//
          strcat(outputString, "Y,");
//
//  Longitude
//
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          sprintf(tempString, "%12.6f,", NODES.longitude);
          ptr = tempString;
          while(*ptr != ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
//
//  Latitude
//
          sprintf(tempString, "%12.6f,", NODES.latitude);
          while(*ptr != ' ')
          {
            ptr++;
          }
          strcat(outputString, ptr);
          strcat(outputString, "\r\n");
//
//  Write out the record
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Go through MIFLINES until we hit a different node
//
          for(MIFPosition = MIFStart + 1; MIFPosition < numMIFLINES; MIFPosition++)
          {
            if(MIFPosition >= numMIFLINES)
            {
              break;
            }
            if(MIFLINES[MIFPosition + 1].from.NODESrecordID != PATTERNS.NODESrecordID)
            {
              MIFStart = MIFPosition + 1;
              break;
            }
            strcpy(outputString, outputStringSave);
//
//  LongLatSequence
//
            sprintf(tempString, "%d,", seq++);
            strcat(outputString, tempString);
//
//  Timepoint/Stop
//
            strcat(outputString, "N,");
//
//  Longitude
//
            sprintf(tempString, "%12.6f,", MIFLINES[MIFPosition].from.longitude);
            ptr = tempString;
            while(*ptr != ' ')
            {
              ptr++;
            }
            strcat(outputString, ptr);
//
//  Latitude
//
            sprintf(tempString, "%12.6f,", MIFLINES[MIFPosition].from.latitude);
            while(*ptr != ' ')
            {
              ptr++;
            }
            strcat(outputString, ptr);
            strcat(outputString, "\r\n");
//
//  Write out the record
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  Get the next pattern record
//
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while on patterns
      }  // nJ (directions)
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    }  // while on routes
  }  // nI 

//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  done:
    StatusBarEnd();
  
  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  if(bGotError)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_350, (HANDLE)NULL);
  }
  _lclose(hfErrorLog);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(NULL, "Would you like to see the error log?", TMS, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
  {
    WinExec("Notepad error.log", SW_SHOWNORMAL); 
  }
  chdir("..");  // Back to where we started from
  _lclose(hfOutputFile);
  distanceMeasure = distanceMeasureSave;
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  return(bFinishedOK);
}

