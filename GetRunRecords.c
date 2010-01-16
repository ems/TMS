//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

int sort_runs(const void *a, const void *b)
{
  RunRecordDef *pa, *pb;
  pa = (RunRecordDef *)a;
  pb = (RunRecordDef *)b;
  
  if(pa->blockNumber < pb->blockNumber)
  {
    return(-1);
  }
  else if(pa->blockNumber > pb->blockNumber)
  {
    return(1);
  }
  else
  {
    return(pa->startTime < pb->startTime ? -1 : pa->startTime > pb->startTime ? 1 : 0);
  }
}


int GetRunRecords(long DIVISIONSrecordID, long SERVICESrecordID)
{
  long onTime;
  long offTime;
  int  rcode2;

  m_numRunRecords = 0;
  RUNSKey1.DIVISIONSrecordID = DIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = SERVICESrecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == DIVISIONSrecordID &&
        RUNS.SERVICESrecordID == SERVICESrecordID)
  {
    if(StatusBarAbort())
    {
      TMSHeapFree(m_pRunRecordData);
      return(NO_RECORD);
    }
    LoadString(hInst, TEXT_042, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, RUNS.runNumber);
    StatusBarText(tempString);
    m_pRunRecordData[m_numRunRecords].recordID = RUNS.recordID;
    m_pRunRecordData[m_numRunRecords].runNumber = RUNS.runNumber;
    m_pRunRecordData[m_numRunRecords].pieceNumber = RUNS.pieceNumber;
    TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    m_pRunRecordData[m_numRunRecords].blockNumber = TRIPS.standard.blockNumber;
    RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
          RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
    m_pRunRecordData[m_numRunRecords].startNODESrecordID = RUNS.start.NODESrecordID;
    m_pRunRecordData[m_numRunRecords].startTRIPSrecordID = RUNS.start.TRIPSrecordID;
    m_pRunRecordData[m_numRunRecords].startTime = onTime;
    m_pRunRecordData[m_numRunRecords].endNODESrecordID = RUNS.end.NODESrecordID;
    m_pRunRecordData[m_numRunRecords].endTRIPSrecordID = RUNS.end.TRIPSrecordID;
    m_pRunRecordData[m_numRunRecords].endTime = offTime;
    m_numRunRecords++;
    if(m_numRunRecords >= m_maxRunRecords)
    {
      m_maxRunRecords += 100;
      m_pRunRecordData = (RunRecordDef *)HeapReAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, m_pRunRecordData, sizeof(RunRecordDef) * m_maxRunRecords); 
      if(m_pRunRecordData == NULL)
      {
        AllocationError(__FILE__, __LINE__, TRUE);
        return(NO_RECORD);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
  qsort((void *)m_pRunRecordData, m_numRunRecords, sizeof(RunRecordDef), sort_runs);

  return(m_numRunRecords);
}
