//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//  From QETXT.INI:
//
//  [TMSRPT62]
//  FILE=Tmsrpt62.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Division,VARCHAR,32,0,32,0,
//  FIELD3=RunNumber,VARCHAR,16,0,16,0,
//  FIELD4=Monday,VARCHAR,16,0,16,0,
//  FIELD5=Tuesday,VARCHAR,16,0,16,0,
//  FIELD6=Wednesday,VARCHAR,16,0,16,0,
//  FIELD7=Thursday,VARCHAR,16,0,16,0,
//  FIELD8=Friday,VARCHAR,16,0,16,0,
//  FIELD9=Saturday,VARCHAR,16,0,16,0,
//  FIELD10=Sunday,VARCHAR,16,0,16,0,
//
typedef struct TMSRPT62RDataStruct
{
  long runNumber;
  long rosterNumber[7];
  long flags;
} TMSRPT62RDataDef;

int sort_TMSRPT62(const void *a, const void *b)
{
  TMSRPT62RDataDef *pa, *pb;
  
  pa = (TMSRPT62RDataDef *)a;
  pb = (TMSRPT62RDataDef *)b;
  
  return(pa->runNumber < pb->runNumber ? -1 : pa->runNumber > pb->runNumber ? 1 : 0);
}

#define TMSRPT62_CREWONLY 0x0001
//
//  Unrostered runs report
//
BOOL FAR TMSRPT62(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound;
  BOOL  bRC;
  char outputString[256];
  char szDivisionName[DIVISIONS_NAME_LENGTH + 1];
  char dummy[256];
  long maxRuns;
  long runNumber;
  long flag;
  int  days[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011,
                                TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int  nI;
  int  nJ;
  int  rcode2;
  int  numRuns;
  int  seq;

  TMSRPT62RDataDef *pTMSRPT62RData = NULL;

  pPassedData->nReportNumber = 61;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.flags = RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS;

  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of divisions
//
  if(REPORTPARMS.numDivisions == 0)  // This should never happen
  {
    goto deallocate;
  }
//
//  Open the output file
//
  StatusBarStart(NULL, "Roster Reconciliation");
  StatusBarText("Opening output file...");
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT62.txt");
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
//  Initialize the TMSRPT62RData structure
//
  rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_266, (HANDLE)NULL);
    goto deallocate;
  }
  maxRuns = BSTAT.numRecords;
  numRuns = 0;
  pTMSRPT62RData = (TMSRPT62RDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TMSRPT62RDataDef) * maxRuns); 
  if(pTMSRPT62RData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
  for(nI = 0; nI < maxRuns; nI++)
  {
    pTMSRPT62RData[nI].runNumber = NO_RECORD;
    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      pTMSRPT62RData[nI].rosterNumber[nJ] = NO_RECORD;
    }
    pTMSRPT62RData[nI].flags = 0;
  }
//
//  Read the roster into TMSRPT62RData
//
  ROSTERKey1.DIVISIONSrecordID = REPORTPARMS.DIVISIONSrecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == REPORTPARMS.DIVISIONSrecordID)
  {
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
      {
        continue;
      }
      if(ROSTER.WEEK[m_RosterWeek].flags & (1 << nI))  // Crew only?
      {
        CREWONLYKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI];
        btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        runNumber = CREWONLY.runNumber;
        flag = TMSRPT62_CREWONLY;
      }
      else
      {
        RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI];
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        runNumber = RUNS.runNumber;
        flag = 0;
      }
      bFound = FALSE;
      for(nJ = 0; nJ < numRuns; nJ++)
      {
        if(pTMSRPT62RData[nJ].runNumber == runNumber && (pTMSRPT62RData[nJ].flags == flag))
        {
          pTMSRPT62RData[nJ].rosterNumber[nI] = ROSTER.rosterNumber;
          pTMSRPT62RData[nJ].flags = flag;
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        pTMSRPT62RData[numRuns].runNumber = RUNS.runNumber;
        pTMSRPT62RData[numRuns].rosterNumber[nI] = ROSTER.rosterNumber;
        numRuns++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Sort the structure on run number
//
  qsort((void *)pTMSRPT62RData, numRuns, sizeof(TMSRPT62RDataDef), sort_TMSRPT62);
//
//  Get the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.DIVISIONSrecordID;
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(szDivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(szDivisionName, DIVISIONS_NAME_LENGTH);
//
//  Dump out the data
//
  seq = 0;
  for(nI = 0; nI < numRuns; nI++)
  {
    if(StatusBarAbort())
    {
      goto deallocate;
    }
    StatusBar((long)(nI + 1), (long)numRuns);
    sprintf(outputString, "%d\t%s\t%ld", seq++, szDivisionName, pTMSRPT62RData[nI].runNumber);
    if(pTMSRPT62RData[nI].flags & TMSRPT62_CREWONLY)
    {
      strcat(outputString, "*");
    }
    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      if(pTMSRPT62RData[nI].rosterNumber[nJ] == NO_RECORD)
      {
        strcat(outputString, "\t---");
      }
      else
      {
        sprintf(tempString, "\t%ld", pTMSRPT62RData[nI].rosterNumber[nJ]);
        strcat(outputString, tempString);
      }
    }
    strcat(outputString, "\r\n");
    _lwrite(hfOutputFile, outputString, strlen(outputString));
  }
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(pTMSRPT62RData);
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
