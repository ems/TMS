// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  License and Medical Expiries
//
//  From Schema.ini:
//
//  [TMSRPT80.TXT]
//  ColNameHeader=False
//  Format=TabDelimited
//  CharacterSet=ANSI
//  Col1 = Sequence Integer
//  Col2 = Text Char
//
#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
}  // extern "C"
#include "Expiries.h"
#include "DailyOpsHeader.h"

int  TMSRPT80Ex(TMSRPTPassedDataDef *);

extern "C" {
BOOL FAR TMSRPT80(TMSRPTPassedDataDef *pPassedData)
{
  return TMSRPT80Ex(pPassedData);
}
} // extern "C"

#include "TMS.h"

CTime ConvertYYYYMMDD(long year, long month, long day)
{
  return(CTime(year, month, day, 0, 0, 0));
}

BOOL FAR TMSRPT80Ex(TMSRPTPassedDataDef *pPassedData)
{
  CString s;
  HFILE hfOutputFile;
  BOOL  bCheck[2];
  BOOL  bFinishedOK;
  BOOL  bFound;
  BOOL  bGotOne;
  char  dummy[256];
  char  outputString[512];
  char  szCheckType[2][16] = {"License", "Medical"};
  long  year, month, day;
  long  *pDate;
  int   numDrivers;
  int   nI;
  int   rcode2;
  int   seq;
  int   daysForward;

  bFinishedOK = FALSE;

  CExpiries dlg(NULL, &bCheck[0], &bCheck[1], &daysForward);
  CTime today = CTime::GetCurrentTime();
  CTime futureDate = today;
//
//  Open the output file
//
  pPassedData->nReportNumber = 79;
  pPassedData->numDataFiles = 1;
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt80.txt");
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
//  Get the number of drivers
//
  rcode2 = btrieve(B_STAT, TMS_JURISDICTIONS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    LoadString(hInst, ERROR_368, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  numDrivers = BSTAT.numRecords;
//
//  Get the filter
//
  if(dlg.DoModal() == IDCANCEL)
  {
    goto deallocate;
  }
  futureDate += CTimeSpan(daysForward, 0, 0, 0);
//
//  List out the driver types under consideration
//
  bGotOne = FALSE;

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  while(rcode2 == 0)
  {
    if(driversToConsider & (1 << (DRIVERTYPES.number - 1)))
    {
      bGotOne = TRUE;
      break;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  }
//
//  Must be looking at at least one driver type
//
  if(!bGotOne)
  {
    s.LoadString(ERROR_369);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, s, TMS, MB_ICONSTOP);
    goto deallocate;
  }
//
//  List the driver types
//
  seq = 1;
  sprintf(outputString, "%d\t\"The following driver types are under consideration:\"\r\n", seq++);
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  while(rcode2 == 0)
  {
    if(driversToConsider & (1 << (DRIVERTYPES.number - 1)))
    {
      strncpy(tempString, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
      trim(tempString, DRIVERTYPES_NAME_LENGTH);
      sprintf(outputString, "%d\t\"  %s\"\r\n", seq++, tempString);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  }
//
//  Loop through the checks
//
//  bCheck[0] = License
//  bCheck[1] = Medical
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  for(nI = 0; nI < 2; nI++)
  {
    if(!bCheck[nI])
    {
      continue;
    }
    pDate = (nI == 0 ? &DRIVERS.licenseExpiryDate : &DRIVERS.physicalExpiryDate);
    StatusBar((long)nI, 2L);
    sprintf(outputString, "%d\r\n", seq++);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    sprintf(outputString, "%d\t\"%s\"\r\n", seq++, szCheckType[nI]);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    sprintf(outputString, "%d\r\n", seq++);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Check invalid expiry dates
//
//  Standard validations:
//     1970 < Year < 2038
//     1 < Month < 12
//     1 < Day < 31
//
    sprintf(outputString, "%d\t\"  The following drivers have invalid %s expiry dates (yyyy/mm/dd):\"\r\n", seq++, szCheckType[nI]);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    bFound = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    while(rcode2 == 0)
    {
      if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        GetYMD(*pDate, &year, &month, &day);
        if((year < 1970 || year > 2038) || (month < 1 || month > 12) || (day < 1 || day > 31))
        {
          sprintf(tempString, BuildOperatorString(NO_RECORD));
          sprintf(outputString, "%d\t\"    %s (%04ld/%02ld/%02ld)\"\r\n", seq++, tempString, year, month, day);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          bFound = TRUE;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    }
    if(!bFound)
    {
      sprintf(outputString, "%d\t\"    None.\"\r\n", seq++);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
//
//  Check for expired
//
    sprintf(outputString, "%d\r\n", seq++);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    sprintf(outputString, "%d\t\"  The following drivers have expired %s dates:\"\r\n", seq++, szCheckType[nI]);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    bFound = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    while(rcode2 == 0)
    {
      if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        GetYMD(*pDate, &year, &month, &day);
        if((year >= 1970 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
        {
          CTime thisDate(year, month, day, 0, 0, 0);

          if(thisDate < today)
          {
            sprintf(tempString, BuildOperatorString(NO_RECORD));
            sprintf(outputString, "%d\t\"    %s (%s)\"\r\n", seq++, tempString, thisDate.Format(_T("%B %d, %Y")));
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            bFound = TRUE;
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    }
    if(!bFound)
    {
      sprintf(outputString, "%d\t\"    None.\"\r\n", seq++);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
//
//  Check for those expiring in the next "daysForward" days
//
    sprintf(outputString, "%d\r\n", seq++);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    sprintf(outputString,
          "%d\t\"  The following drivers will have their %s expire in the next %d days:\"\r\n", seq++, szCheckType[nI], daysForward);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    bFound = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    while(rcode2 == 0)
    {
      if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        GetYMD(*pDate, &year, &month, &day);
        if((year >= 1970 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
        {
          CTime thisDate(year, month, day, 0, 0, 0);

          if(thisDate >= today && thisDate <= futureDate)
          {
            sprintf(tempString, BuildOperatorString(NO_RECORD));
            sprintf(outputString, "%d\t\"    %s (%s)\"\r\n", seq++, tempString, thisDate.Format(_T("%B %d, %Y")));
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            bFound = TRUE;
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    }
    if(!bFound)
    {
      sprintf(outputString, "%d\t\"    None.\"\r\n", seq++);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
    }
  }
//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  deallocate:
    _lclose(hfOutputFile);
    StatusBarEnd();
  
  return(bFinishedOK);
}
