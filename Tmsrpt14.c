//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

typedef struct TMSRPT14RDataStruct
{
  long recordID;
  long runNumber;
  long cutAsRuntype;
} TMSRPT14RDataDef;

//
//  Unrostered runs report
//
BOOL FAR TMSRPT14(TMSRPTPassedDataDef *pPassedData)
{
  TMSRPT14RDataDef *pTMSRPT14RData[ROSTER_MAX_DAYS];
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound;
  long  numRuns[ROSTER_MAX_DAYS];
  char szOutputString[128];
  char szDay[16];
  char dummy[256];
  long maxRuns;
  int   days[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011,
                                 TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;

  pPassedData->nReportNumber = 13;
  pPassedData->numDataFiles = 1;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Open the output file
//
  StatusBarText("Opening output file...");
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt14.txt");
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
//  Initialize the TMSRPT14RData and TMSRPT14RDataData structures
//
  rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_266, (HANDLE)NULL);
    goto deallocate;
  }
  maxRuns = BSTAT.numRecords;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    numRuns[nI] = 0;
    pTMSRPT14RData[nI] = (TMSRPT14RDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TMSRPT14RDataDef) * maxRuns); 
    if(pTMSRPT14RData[nI] == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      goto deallocate;
    }
  }
//
//  Read the runs into TMSRPT14RData
//
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    LoadString(hInst, days[nI], szDay, sizeof(szDay));
    sprintf(tempString, "Reading %s runs...", szDay); 
    StatusBarText(tempString);
    if(StatusBarAbort())
    {
      goto deallocate;
    }
    if(ROSTERPARMS.serviceDays[nI] == NO_RECORD)
    {
      continue;
    }
    RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
    RUNSKey1.SERVICESrecordID = ROSTERPARMS.serviceDays[nI];
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 && 
          RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
          RUNS.SERVICESrecordID == ROSTERPARMS.serviceDays[nI])
    {
      if(RUNS.pieceNumber == 1)
      {
        pTMSRPT14RData[nI][numRuns[nI]].recordID = RUNS.recordID;
        pTMSRPT14RData[nI][numRuns[nI]].runNumber = RUNS.runNumber;
        pTMSRPT14RData[nI][numRuns[nI]].cutAsRuntype = RUNS.cutAsRuntype;
        numRuns[nI]++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
  }
//
//  Go through the roster.  Each time a run number shows up, kill it
//  from the list inside TMSRPT14RData.
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
  while(rcode2 == 0)
  {
    StatusBarText("Cycling through the roster...");
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
      {
        continue;
      }
      for(nJ = 0; nJ < numRuns[nI]; nJ++)
      {
        if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI] == pTMSRPT14RData[nI][nJ].recordID)
        {
          pTMSRPT14RData[nI][nJ].recordID = NO_RECORD;
          break;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
  }
//
//  Dump out the remaining runs
//
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    if(StatusBarAbort())
    {
      goto deallocate;
    }
    LoadString(hInst, days[nI], szDay, sizeof(szDay));
    sprintf(tempString, "Writing output for day %s...", szDay); 
    StatusBarText(tempString);
    for(bFound = FALSE, nJ = 0; nJ < numRuns[nI]; nJ++)
    {
      if(pTMSRPT14RData[nI][nJ].recordID != NO_RECORD)
      {
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
    {
      sprintf(szOutputString, "%10s\t", szDay);
      strcat(szOutputString, "     0\tNo runs\r\n");
      _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
    }
    else
    {
      for(; nJ < numRuns[nI]; nJ++)
      {
        sprintf(szOutputString, "%10s\t", szDay);
        if(pTMSRPT14RData[nI][nJ].recordID != NO_RECORD)
        {
          sprintf(tempString, "%6ld\t", pTMSRPT14RData[nI][nJ].runNumber);
          strcat(szOutputString, tempString);
          nK = (short int)LOWORD(pTMSRPT14RData[nI][nJ].cutAsRuntype);
          nL = (short int)HIWORD(pTMSRPT14RData[nI][nJ].cutAsRuntype);
          if(nK >= 0 && nK < NUMRUNTYPES && nL >= 0 && nL < NUMRUNTYPESLOTS)
          {
            strcat(szOutputString, RUNTYPE[nK][nL].localName);
          }
          strcat(szOutputString, "\r\n");
          _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
        }
      }
    }
  }
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      TMSHeapFree(pTMSRPT14RData[nI]);
    }
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
