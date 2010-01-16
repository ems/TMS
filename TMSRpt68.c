// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>
//
//  Unpaid time report
//
//  From QETXT.INI:
//  
//  [TMSRPT68]
//  FILE=TMSRPT68.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Operator,VARCHAR,32,0,32,0,
//  FIELD2=Sequence,NUMERIC,8,0,8,0,
//  FIELD3=Date,VARCHAR,10,0,10,0,
//  FIELD4=AbsenceType,VARCHAR,8,0,8,0,
//  FIELD5=Reason,VARCHAR,32,0,32,0,
//  FIELD6=Action,VARCHAR,32,0,32,0,
//  FIELD7=Comments,VARCHAR,32,0,32,0,
//  FIELD8=DateOff,VARCHAR,10,0,10,0,
//

BOOL FAR TMSRPT68(TMSRPTPassedDataDef *pPassedData)
{
  UNPAIDTIMEPassedDataDef RParamData;
  HFILE hfOutputFile;
  BOOL  bFound;
  char  outputString[TEMPSTRING_LENGTH];
  char *pszReportName;
  char *pszOperator;
  char  dummy[256];
  long  thisDriver;
  long  maxDrivers;
  long  year;
  long  month;
  long  day;
  int   nI;
  int   rcode2;
  int   seq;
//
//  See what he wants to unload
//
  if(UnpaidTimeFilter(&RParamData) != IDOK)
  {
    return(FALSE);
  }
//
//  Back ok - set up the status bar
//
  pPassedData->nReportNumber = 67;
  pPassedData->numDataFiles = 1;
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
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT68.txt");
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
  StatusBar(-1L, -1L);
//
//  Cycle through the operators
//
  if(RParamData.DRIVERSrecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  else
  {
    rcode2 = btrieve(B_STAT, TMS_DRIVERS, &BSTAT, dummy, 0);
    thisDriver = 0;
    maxDrivers = BSTAT.numRecords;
    DRIVERSKey0.recordID = RParamData.DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  }
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      seq = 0;
      pszOperator = BuildOperatorString(DRIVERS.recordID);
      DAILYOPSKey3.DRIVERSrecordID = DRIVERS.recordID;
      DAILYOPSKey3.pertainsToDate = RParamData.date;
      DAILYOPSKey3.pertainsToTime = NO_TIME;
      DAILYOPSKey3.recordTypeFlag = 0;
      DAILYOPSKey3.recordFlags = 0;
      rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
      while(rcode2 == 0 &&
            DAILYOPS.DRIVERSrecordID == DRIVERS.recordID)// &&
//            DAILYOPS.pertainsToDate <= RParamData.date)
      {
        if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)  // Only interested in absences
        {
          if(!ANegatedRecord(DAILYOPS.recordID, 3))
          {
            if(DAILYOPS.DOPS.Absence.unpaidTime > 0)//  &&
//                  DAILYOPS.DOPS.Absence.reasonIndex != NO_RECORD &&
//                  m_AbsenceReasons[DAILYOPS.DOPS.Absence.reasonIndex].bCountsTowardUnpaidTime)
            {
//
//  Operator
//
              strcpy(outputString, pszOperator);
              strcat(outputString, "\t");
//
//  Sequence
//
              strcat(outputString, itoa(seq++, tempString, 10));
              strcat(outputString, "\t");
//
//  Date
//
              GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
              sprintf(tempString, "%ld/%02ld/%02ld", year, month, day);
              strcat(outputString, tempString);
              strcat(outputString, "\t");
//
//  Absence type
//
              strcat(outputString, "\t");
//
//  Reason
//
              if(DAILYOPS.DOPS.Absence.reasonIndex == NO_RECORD)
              {
                strcat(outputString, "Assignment cleared manually");
              }
              else
              {
                strcat(outputString, m_AbsenceReasons[DAILYOPS.DOPS.Absence.reasonIndex].szText);
              }
              strcat(outputString, "\t");
//
//  Action
//
              DISCIPLINEKey2.DRIVERSrecordID = DRIVERS.recordID;
              DISCIPLINEKey2.violationCategory = NO_RECORD;
              DISCIPLINEKey2.violationType = NO_RECORD;
              rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey2, 2);
              bFound = FALSE;
              while(rcode2 == 0 &&
                    DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
              {
                if(DISCIPLINE.DAILYOPSrecordID == DAILYOPS.recordID)
                {
                  bFound = TRUE;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey2, 2);
              }
              if(bFound)
              {
                if(DISCIPLINE.actionTaken != NO_RECORD)
                {
                  strcat(outputString, ltoa(DISCIPLINE.actionTaken, tempString, 10));
                  strcat(outputString, tempString);
                }
              }               
              strcat(outputString, "\t");
//
//  Comments
//
              if(bFound)
              {
                strncpy(tempString, DISCIPLINE.comments, DISCIPLINE_COMMENTS_LENGTH);
                trim(tempString, DISCIPLINE_COMMENTS_LENGTH);
                strcat(outputString, tempString);
              }
              else
              {
                OFFTIMEKey1.DAILYOPSrecordID = DAILYOPS.recordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey1, 1);
                if(rcode2 == 0)
                {
                  strncpy(tempString, OFFTIME.text, OFFTIME_TEXT_LENGTH);
                  trim(tempString, OFFTIME_TEXT_LENGTH);
                  strcat(outputString, tempString);
                }
              }
              strcat(outputString, "\t");
//
//  Unpaid time
//
              strcat(outputString, ltoa(DAILYOPS.DOPS.Absence.unpaidTime, tempString, 10));
              strcat(outputString, "\t");
              strcat(outputString, "\r\n");
//
//  Write it out
//
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }  // Unpaid time > 0
          }  // Not unregistered
        }  // Absence flag on DAILYOPSrecord
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
      }  // while this driver in DAILYOPS
    }  // Considering this driver type
//
//  If we were only doing one driver, we're done.  Otherwise, get the next one and go back.
//
    if(RParamData.DRIVERSrecordID != NO_RECORD)
    {
      StatusBar(100, 100);
      break;
    }
    DRIVERSKey0.recordID = DRIVERS.recordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    rcode2 = btrieve(B_GETPOSITION, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    thisDriver++;
    StatusBar(thisDriver, maxDrivers);
  }  // while on drivers
//
//  All done
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);

  return(TRUE);
}
