//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT73() - Ridership - Daily Detail
//
//
//  From QETXT.INI:
//
//  [TMSRPT73]
//  FILE=TMSRPT73.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD2=RouteName,VARCHAR,64,0,64,0,
//  FIELD3=Date,VARCHAR,16,0,16,0,
//  FIELD4=Cash0,NUMERIC,8,2,8,0,
//  FIELD5=Cash1,NUMERIC,8,2,8,0,
//  FIELD6=Cash2,NUMERIC,8,2,8,0,
//  FIELD7=Cash3,NUMERIC,8,2,8,0,
//  FIELD8=Fares0,NUMERIC,8,0,8,0,
//  FIELD9=Fares1,NUMERIC,8,0,8,0,
//  FIELD10=Fares2,NUMERIC,8,0,8,0,
//  FIELD11=Fares3,NUMERIC,8,0,8,0,
//  FIELD12=MilesOut,NUMERIC,9,1,9,1,
//  FIELD13=MilesIn,NUMERIC,9,1,9,1,
//

#include "TMSHeader.h"

#define TMSRPT73_MAXROUTES 100

typedef struct CombinedStruct
{
  long  ROUTESrecordID;
  long  RUNSrecordID;
  int   numFares[4];
  float fareValue[4];
  float mileageOut;
  float mileageIn;
} CombinedDef;

BOOL GetDateRange(long *, long *);

#define NUMCOLS 6

BOOL FAR TMSRPT73(TMSRPTPassedDataDef *pPassedData)
{
  CombinedDef Combined[TMSRPT73_MAXROUTES];
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound;
  char  outputString[512];
  long  tempLong;
  long  fromDate, toDate;
  long  year, month,day;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   numCombined;

  pPassedData->nReportNumber = 72;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  if(!GetDateRange(&fromDate, &toDate))
  {
    return(FALSE);
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT73.txt");
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
//  Go through the audit trail to get the ridership numbers
//
  StatusBarStart(hWndMain, "Unloading Ridership Data");
  for(tempLong = fromDate; tempLong <= toDate; tempLong++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    for(nI = 0; nI < TMSRPT73_MAXROUTES; nI++)
    {
      Combined[nI].ROUTESrecordID = NO_RECORD;
      Combined[nI].RUNSrecordID = NO_RECORD;
      for(nJ = 0; nJ < 4; nJ++)
      {
        Combined[nI].numFares[nJ] = 0;
        Combined[nI].fareValue[nJ] = 0;
      }
      Combined[nI].mileageOut = 0.0;
      Combined[nI].mileageIn  = 0.0;
    }
    numCombined = 0;
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = tempLong;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == tempLong)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETRIDERSHIP)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          TRIPSKey0.recordID = DAILYOPS.DOPS.Ridership.R.TRIPSrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          for(nI = 0; nI < numCombined; nI++)
          {
            if(Combined[nI].ROUTESrecordID == TRIPS.ROUTESrecordID)
            {
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
          {
            nI = numCombined;
            Combined[nI].ROUTESrecordID = TRIPS.ROUTESrecordID;
            Combined[nI].RUNSrecordID = DAILYOPS.DOPS.Ridership.R.RUNSrecordID;
            numCombined++;
          }
          for(nJ = 0; nJ < 4; nJ++)
          {
            Combined[nI].numFares[nJ] += DAILYOPS.DOPS.Ridership.R.numFares[nJ];
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
//
//  Get the fare values for each of the routes
//
    for(nI = 0; nI < numCombined; nI++)
    {
      for(bFound = FALSE, nJ = 0; nJ < m_numFareTypesByRoute; nJ++)
      {
        if(Combined[nI].ROUTESrecordID == m_FareTypesByRoute[nJ].ROUTESrecordID)
        {
          bFound = TRUE;
          break;
        }
      }
      if(bFound)
      {
        for(nK = 0; nK < 4; nK++)
        {
          Combined[nI].fareValue[nK] = m_FareTypesByRoute[nJ].fareValue[nK];
        }
      } 
    }
//
//  Get the mileage for this run
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = tempLong;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == tempLong)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETMILEAGE)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          for(nI = 0; nI < numCombined; nI++)
          {
            if(Combined[nI].RUNSrecordID == DAILYOPS.DOPS.Ridership.M.RUNSrecordID)
            {
              bFound = TRUE;
              break;
            }
          }
          if(bFound)
          {
            Combined[nI].mileageOut = DAILYOPS.DOPS.Ridership.M.mileageOut;
            Combined[nI].mileageIn = DAILYOPS.DOPS.Ridership.M.mileageIn;
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
//
//  Loop through Combined
//
    for(nI = 0; nI < numCombined; nI++)
    {
//
//  Route number
//
      ROUTESKey0.recordID = Combined[nI].ROUTESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      strcpy(outputString, tempString);
      strcat(outputString, "\t");
//
//  Route name
//
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  Date
//
      GetYMD(tempLong, &year, &month, &day);
      sprintf(tempString, "%02ld/%02ld/%ld", month, day, year);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  Cash received
//
      for(nJ = 0; nJ < 4; nJ++)
      {
        sprintf(tempString, "%6.02f", Combined[nI].numFares[nJ] * Combined[nI].fareValue[nJ]);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
      }
//
//  Number of fares
//
      for(nJ = 0; nJ < 4; nJ++)
      {
        sprintf(tempString, "%d", Combined[nI].numFares[nJ]);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
      }
//
//  Mileage
//
      sprintf(tempString, "%8.1f", Combined[nI].mileageOut);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
      sprintf(tempString, "%8.1f", Combined[nI].mileageIn);
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
//
//  Write it out
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
  } // tempLong

  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    _lclose(hfOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}

