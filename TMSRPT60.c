//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Discipline Report
//
//  From QETXT.INI:
//
//    [TMSRPT60]
//    FILE=TMSRPT60.txt
//    FLN=0
//    TT=Tab
//    Charset=ANSI
//    DS=.
//    FIELD1=Date,VARCHAR,32,0,32,0,
//    FIELD2=Time,VARCHAR,32,0,32,0,
//    FIELD3=Sequence,NUMERIC,8,0,8,0,
//    FIELD4=Operator,VARCHAR,32,0,32,0,
//    FIELD5=Text,VARCHAR,256,0,256,0,
//
BOOL FAR TMSRPT60(TMSRPTPassedDataDef *pPassedData)
{
  DISCIPLINEFILTERSELECTIONSDef DisciplineSelections;
  HANDLE hOutputFile = NULL;
  DWORD  dwBytesWritten;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  long  year, month, day;
  long  DRIVERSrecordID;
  long  seq;
  long  category;
  long  type;
  long  action;
  int   rcode2;
  int   nI;

  pPassedData->nReportNumber = 59;
  pPassedData->numDataFiles = 1;
  DRIVERSrecordID = pPassedData->DRIVERSrecordID;
//
//  See what's to be filtered
//
  bRC = DisciplineFilter(&DisciplineSelections);

  if(bRC != IDOK)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT60.txt");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  StatusBarStart(hWndMain, "Processing selections...");
  StatusBar(-1L, -1L);
  bKeepGoing = TRUE;
//
//  Set up the driver
//
  DRIVERSKey0.recordID = DRIVERSrecordID;
  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  trim(tempString, DRIVERS_LASTNAME_LENGTH);
  strcpy(szDriver, tempString);
  strcat(szDriver, ", ");
  strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
  strcat(szDriver, tempString);
//
//  Set up the flags for retrieval
//
  DISCIPLINEKey1.DRIVERSrecordID = DRIVERSrecordID;
  DISCIPLINEKey1.dateOfOffense = DisciplineSelections.fromDate;
  DISCIPLINEKey1.timeOfOffense = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
  seq = 0;
  while(rcode2 == 0 &&
        DISCIPLINE.DRIVERSrecordID == DRIVERSrecordID)
  {
    if(DISCIPLINE.dateOfOffense > DisciplineSelections.toDate)
    {
      break;
    }
    category = DISCIPLINE.violationCategory;
    type = DISCIPLINE.violationType;
    action = DISCIPLINE.actionTaken;
//
//  Category
//
    if(category != NO_RECORD)
    {
      GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
      sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
      strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
      strcat(outputString, "\t");
      sprintf(tempString, "%ld\t", seq++);
      strcat(outputString, tempString);
      strcat(outputString, szDriver);
      strcat(outputString, "\tCategory: ");
      strcat(outputString, m_ViolationCategories[category]);
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Type
//
    if(category != NO_RECORD && type != NO_RECORD)
    {
      GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
      sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
      strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
      strcat(outputString, "\t");
      sprintf(tempString, "%ld\t", seq++);
      strcat(outputString, tempString);
      strcat(outputString, szDriver);
      strcat(outputString, "\t    Type: ");
      strcat(outputString, m_Violations[category][type].text);
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
//
//  Action
//
      sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
      strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
      strcat(outputString, "\t");
      sprintf(tempString, "%ld\t", seq++);
      strcat(outputString, tempString);
      strcat(outputString, szDriver);
      strcat(outputString, "\t  Action: ");
      if(action == NO_RECORD)
      {
        strcat(outputString, " None taken.");
      }
      else
      {
        strcat(outputString, m_ViolationActions[action]);
      }
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Comments
//
    GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
    sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
    strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
    strcat(outputString, "\t");
    sprintf(tempString, "%ld\t", seq++);
    strcat(outputString, tempString);
    strcat(outputString, szDriver);
    strcat(outputString, "\t   Notes: ");
    strncpy(tempString, DISCIPLINE.comments, DISCIPLINE_COMMENTS_LENGTH);
    trim(tempString, DISCIPLINE_COMMENTS_LENGTH);
    for(nI = 0; nI < (int)strlen(tempString); nI++)
    {
      if(tempString[nI] == 13)
      {
        tempString[nI    ] = ' ';
        tempString[nI + 1] = ' ';
        break;
      }
    }
    strcat(outputString, tempString);
    strcat(outputString, "\r\n");
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
//
//  Suspended
//
    if(DISCIPLINE.suspensionDate > 0)
    {
//
//  Date, time, sequence, driver
//
      GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
      sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
      strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
      strcat(outputString, "\t");
      sprintf(tempString, "%ld\t", seq++);
      strcat(outputString, tempString);
      strcat(outputString, szDriver);
      strcat(outputString, "\t");
      GetYMD(DISCIPLINE.suspensionDate, &year, &month, &day);
      sprintf(tempString, "Suspension date: %02ld-%s-%04ld", day, szMonth[month - 1], year);
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
      sprintf(tempString, "Suspension duration: %5.2f", DISCIPLINE.suspensionDuration);
      GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
      sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
      strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
      strcat(outputString, "\t");
      sprintf(tempString, "%ld\t", seq++);
      strcat(outputString, tempString);
      strcat(outputString, szDriver);
      strcat(outputString, "\t");
      sprintf(tempString, "Suspension duration: %5.2f\r\n", DISCIPLINE.suspensionDuration);
      strcat(outputString, tempString);
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Dropoff date
//
    GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
    sprintf(outputString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
    strcat(outputString, Tchar(DISCIPLINE.timeOfOffense));
    strcat(outputString, "\t");
    sprintf(tempString, "%ld\t", seq++);
    strcat(outputString, tempString);
    strcat(outputString, szDriver);
    strcat(outputString, "\t Dropoff: ");
    GetYMD(DISCIPLINE.dropoffDate, &year, &month, &day);
    sprintf(tempString, "%02ld-%s-%04ld\t", day, szMonth[month - 1], year);
    strcat(outputString, tempString);
    strcat(outputString, "\r\n");
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
//
//  Get the next record
//
    rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
  }
//
//  Free allocated memory
//
  deallocate:
    StatusBarEnd();
    if(hOutputFile != NULL)
    {
      CloseHandle(hOutputFile);
    }
    SetCursor(hCursorArrow);

  return(bKeepGoing);
}
