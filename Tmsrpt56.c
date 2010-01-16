//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define PM1159 23 * 3600 + 59 * 60
//
//  Load absence / discipline data 
//
BOOL FAR TMSRPT56A(TMSRPTPassedDataDef *);
BOOL FAR TMSRPT56D(TMSRPTPassedDataDef *);

BOOL FAR TMSRPT56(TMSRPTPassedDataDef *pPassedData)
{
  MessageBeep(MB_ICONQUESTION);
  LoadString(hInst, TEXT_318, tempString, TEMPSTRING_LENGTH);
//
//  Load absence data?
//
  if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
    return(TMSRPT56A(pPassedData));
  }
//
//  Load discipline data?
//
  else
  {
    LoadString(hInst, TEXT_319, tempString, TEMPSTRING_LENGTH);
    if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
    {
      return(TMSRPT56D(pPassedData));
    }
    else
    {
      return(FALSE);
    }
  }
}

//
//  Load absence data into DAILYOPS
//
BOOL FAR TMSRPT56A(TMSRPTPassedDataDef *pPassedData)
{
  HFILE hfErrorLog;
  FILE *fp;
  time_t tempTime;
  float hoursLost;
  float suspensionDays;
  BOOL bFound;
  char inputLine[256];
  char szComment[3][64];
  char *pch;
  char absenceReasonCode;
  long dateOfEntry;
  long DRIVERSrecordID;
  long reasonIndex;
  long missoutNumber;
  long dropoffDate;
  int  rcode2;

  fp = NULL;
//
//  Issue the warning
//
  LoadString(hInst, ERROR_339, tempString, TEMPSTRING_LENGTH);
  MessageBeep(MB_ICONINFORMATION);
  if(MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_YESNO) != IDYES)
  {
    goto cleanup;
  }
//
//  Open the text file
//
  fp = fopen("OLDMASTR.TXT", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open OLDMASTR.TXT", TMS, MB_OK);
    goto cleanup;
  }
//
//  Open the error log
//
  strcpy(tempString, "error.log");
  hfErrorLog = _lcreat(tempString, 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    goto cleanup;
  }
//
//  Cycle through the input file
//
  while(fgets(inputLine, sizeof(inputLine), fp))
  {
//
//  Set up the DAILYOPS record
//
//  RecordID
//
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
//
//  Date
//
    pch = strtok(inputLine, "\t\n");
    dateOfEntry = atol(pch);
//
//  Employee.  There could be multiple entries for fired/rehired employees,
//  so check the date range to make sure we've got the right one.
//
    pch = strtok(NULL, "\t\n");
    strcpy(tempString, pch);
    pad(tempString, DRIVERS_BADGENUMBER_LENGTH);
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    DRIVERSrecordID = NO_RECORD;
    while(rcode2 == 0)
    {
      if(strncmp(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH) == 0)
      {
        if(dateOfEntry >= DRIVERS.hireDate && DRIVERS.hireDate != 0)
        {
          if(dateOfEntry <= DRIVERS.terminationDate || DRIVERS.terminationDate == 0)
          {
            DRIVERSrecordID = DRIVERS.recordID;
          }
        }
        break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
    if(DRIVERSrecordID == NO_RECORD)
    {
      sprintf(tempString, "Did not find DRIVERSrecordID for absence recorded %ld\r\n", dateOfEntry);
      _lwrite(hfErrorLog, tempString, strlen(tempString));
      continue;
    }
//
//  Absence reason code
//
    pch = strtok(NULL, "\t\n");
    absenceReasonCode = *pch;
//
//  From Tony Givhan:
//
//  M = Missout.  Are given for late call outs and reports.  A late call is calling out
//  under an hour before report time (a half hour if the report time is before 6 AM).
//  A late report is reporting to the Dispatch window or other report location late.
//  One (1) M for reporting more than 6 minutes at the window, and more than 4 minutes
//  late on street report.  One-half (.5) M for a late report under 6 minutes at the
//  Dispatch window and under 4 minutes late on street report.  All time attributed to
//  missouts accumulates as unpaid time (U for full-time and N for part-time).  All missouts
//  are posted for drivers to see.  Three (3) missouts within 90 consecutive calendar days
//  results in a written reprimand and a counseling session.  Four (4) missouts within 90
//  consecutive calendar days results in a two day suspension.  Five (5) missouts within 90
//  consecutive calendar days results in a five day suspension.  Sixth (6) within 90
//  consecutive calendar days results in termination.  Missouts drop off after 90 days.
//  No one can get more than two missouts on any day.  If a person who has missouts on
//  their record avoids getting one for 90 days the first two drop off and a additional
//  one for each consecutive 30 days without a missout.
//
//  Q = Occurrence.  Employees can call out anytime up to an hour (a half hour if
//  the report time is before 6 AM) before work and still get paid if they have
//  paid time (annual leave) in their bank. Anytime that a call out from work is
//  given after noon the day before up to an hour before work a person is given a Q.
//  This is with pay so they would also be given a V = Vacation or rather paid annual
//  leave. After accumulating 3 Q's in a quarter (calendar quarter, not just any three
//  months), the forth call out within this time frame is without pay (U =unpaid time
//  for full-time drivers; N=unpaid time for part-time drivers), even if they have paid
//  time in their bank.  So Q's track untimely call outs, i.e., call outs after noon the
//  day before.  After getting 3 Q's in a calendar quarter a person no longer gets paid 
//  for these types of call outs.
//
//  O - More than 2 hours late without any justifiable reason
//
//  C - Callout - allowed to call before noon the previous day to miss this day's work  (not a discipline issue)
//
    if(absenceReasonCode == 'M' || absenceReasonCode == 'Q' || absenceReasonCode == 'O')
    {
      DAILYOPS.recordTypeFlag |= DAILYOPS_FLAG_ABSENCETODISCIPLINE;
    }
//
//  "Number" (only used do distinguish 1/2 from full miss-outs)
//
    pch = strtok(NULL, "\t\n");
    missoutNumber = atol(pch);
//
//  Set the absence reason
//
    pch = strtok(NULL, "\t");
    reasonIndex = atol(pch);
//
//  Start of fix for bad data
//
    if(reasonIndex > 2)
    {
      reasonIndex -= 1;
    }
//
//  End of fix for bad data
//

//
//  Suspension days
//
    pch = strtok(NULL, "\t\n");
    suspensionDays = (float)atof(pch);
//
//  Hours lost
//
    pch = strtok(NULL, "\t\n");
    hoursLost = (float)atof(pch);
//
//  Dropoff date
//
    pch = strtok(NULL, "\t\n");
    dropoffDate = atol(pch);
//
//  Comment 1 (Action)
//
    pch = strtok(NULL, "\t\n");
    if(pch == NULL)
    {
      strcpy(szComment[1], "");
    }
    else
    {
      strcpy(szComment[1], pch);
    }
//
//  Comment 2 (Comments)
//
    if(pch != NULL)
    {
      pch = strtok(NULL, "\t\n");
    }
    if(pch == NULL)
    {
      strcpy(szComment[2], "");
    }
    else
    {
      strcpy(szComment[2], pch);
    }
//
//  In the old system absence reason codes of 'U' and 'N' flagged an absence as unpaid.
//
//  Deal with them later.
//
    if(absenceReasonCode != 'N' && absenceReasonCode != 'U')
    {
//
//  Set up the DAILYOPS record
//
//  RecordID
//
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
//
//  Date/time of entry
//
      DAILYOPS.entryDateAndTime = TMSmktime(dateOfEntry, 0);
//
//  Flags and UserID
//
      DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
      DAILYOPS.userID = 0;
//
//  Driver
//
      DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
//
//  Default the offense date/time
//
      DAILYOPS.pertainsToDate = dateOfEntry;
      DAILYOPS.pertainsToTime = 0;
      DAILYOPS.DOPS.Absence.untilDate = dateOfEntry;
      DAILYOPS.DOPS.Absence.untilTime = PM1159;
//
//  Set up the balance of the absence portion.  Assume paid.
//  This record will be modified if we run into an 'N' or 'U' record later
//
      DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEREGISTER | DAILYOPS_FLAG_ABSENCEPAID;
//
//  Absence reason
//
      DAILYOPS.DOPS.Absence.reasonIndex = reasonIndex;
//
//  The following aren't used on a "register"
//
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
//
//  Time lost?
//
      DAILYOPS.DOPS.Absence.timeLost = (long)(hoursLost * 3600);
//
//  Add the record
//
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
//
//  The reason(s) for the absence get jammed into OFFTIME
//
//  Set up the OFFTIME record
//
//  RecordID
//
      rcode2 = btrieve(B_GETLAST, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
      OFFTIME.recordID = AssignRecID(rcode2, OFFTIME.recordID);
//
//  DailyOps recordID
//
      OFFTIME.DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Merge the comments
//
      strcpy(tempString, szComment[1]);
      strcat(tempString, "");
      strcat(tempString, szComment[2]);
//
//  And write it out
//
      pad(tempString, OFFTIME_TEXT_LENGTH);
      strncpy(OFFTIME.text, tempString, OFFTIME_TEXT_LENGTH);
      OFFTIME.flags = 0;
      rcode2 = btrieve(B_INSERT, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
//
//  Absence codes of 'M', 'Q', and 'O' are discipline issues too
//
      if(absenceReasonCode == 'M' || absenceReasonCode == 'Q' || absenceReasonCode == 'O')
      {
//
//  Set up the DISCIPLINE record
//
//  RecordID
//
        rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
        DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
//
//  Comment code
//
        DISCIPLINE.COMMENTSrecordID = NO_RECORD;
//
//  Driver
//
        DISCIPLINE.DRIVERSrecordID = DRIVERSrecordID;
//
//  Date/Time of Entry
//
        DISCIPLINE.entryDateAndTime = TMSmktime(dateOfEntry, 0);
//
//  Date/Time of Offense
//
        DISCIPLINE.dateOfOffense = dateOfEntry;
        DISCIPLINE.timeOfOffense = 0;
//
//  Violation category (hardcoded to "8" (Absence))
//
        DISCIPLINE.violationCategory = 8;
//
//  Violation number
//
        if(absenceReasonCode == 'M')
        {
          if(missoutNumber == 12)
          {
            DISCIPLINE.violationType = 0;
          }
          else
          {
            DISCIPLINE.violationType = 1;
          }
        }
        else if(absenceReasonCode == 'Q')
        {
          DISCIPLINE.violationType = 2;
        }
        else if(absenceReasonCode == 'O')
        {
          DISCIPLINE.violationType = 3;
        }
//
//  Dropoff date
//
        DISCIPLINE.dropoffDate = (dropoffDate == 19000100 ? 0 : dropoffDate);
//
//  Comments
//
        strcpy(tempString, szComment[1]);
        strcat(tempString, "\r\n");
        strcat(tempString, szComment[2]);
        pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
        strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
//
//  Flags and UserID
//
        DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
        DISCIPLINE.userID = 0;
//
//  Suspension date and duration
//
        if(suspensionDays == 0)
        {
          DISCIPLINE.suspensionDate = 0;
          DISCIPLINE.suspensionDuration = 0.0;
        }
        else
        {
          DISCIPLINE.suspensionDate = dateOfEntry;
          DISCIPLINE.suspensionDuration = suspensionDays;
          DISCIPLINE.flags |= DISCIPLINE_FLAG_SUSPENSION;
        }
//
//  Action taken
//
        DISCIPLINE.actionTaken = NO_RECORD;
//
//  Reported by
//
        DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
//
//  DAILYOPSrecordID (formerly Reserved)
//
        DISCIPLINE.DAILYOPSrecordID = NO_RECORD;
//
//  Write it out
//
        rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
      }
    }
//
//  Deal with 'N' and 'U'
//
    else
    {
//
//  Locate and update the original record
//
      tempTime = TMSmktime(dateOfEntry, 0);
      bFound = FALSE;
      DAILYOPSKey5.entryDateAndTime = tempTime;
      DAILYOPSKey5.pertainsToDate = 0;
      DAILYOPSKey5.pertainsToTime = 0;
      DAILYOPSKey5.recordTypeFlag = 0;
      DAILYOPSKey5.recordFlags = 0;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey5, 5);
      while(rcode2 == 0 &&
            DAILYOPS.entryDateAndTime == tempTime)
      {
        if(DAILYOPS.DRIVERSrecordID == DRIVERSrecordID &&
              (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEPAID))
        {
          DAILYOPS.recordFlags &= ~DAILYOPS_FLAG_ABSENCEPAID;
          rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          bFound = TRUE;
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      }
//
//  Was it there?
//
      if(!bFound)
      {
        strncpy(szarString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(szarString, DRIVERS_BADGENUMBER_LENGTH);
        sprintf(tempString, "Did not find a record to update paid status for absence recorded %ld for driver %s\r\n", dateOfEntry, szarString);
        _lwrite(hfErrorLog, tempString, strlen(tempString));
      }
    }
  }
//
//  All done
//
  MessageBox(NULL, "Done!", TMS, MB_OK);

  cleanup:
    if(fp != NULL)
    {
      fclose(fp);
    }
    _lclose(hfErrorLog);
    return(TRUE);
}

//
//  Load discipline data
//
BOOL FAR TMSRPT56D(TMSRPTPassedDataDef *pPassedData)
{
  FILE *fp;
  char inputLine[256];
  char szComment[3][64];
  char *pch;
  long dateOfEntry;
  int  rcode2;

  fp = NULL;
//
//  Issue the warning
//
  LoadString(hInst, ERROR_339, tempString, TEMPSTRING_LENGTH);
  MessageBeep(MB_ICONINFORMATION);
  if(MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_YESNO) != IDYES)
  {
    goto cleanup;
  }
//
//  Open the text file
//
  fp = fopen("Other Discipline.txt", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open Other Discipline.txt", TMS, MB_OK);
    goto cleanup;
  }
/*
//
//  Nuke the existing portion of the file prior to 20051013
//
  rcode2 = btrieve(B_GETFIRST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
  while(rcode2 == 0)
  {
    if(DISCIPLINE.dateOfEntry < 20051013)
    {
      if(!(DISCIPLINE.flags & DISCIPLINE_FLAG_ABSENCERELATED))
      {
        rcode2 = btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
  }
*/
//
//  Cycle through the input file
//
  while(fgets(inputLine, sizeof(inputLine), fp))
  {
    strcpy(szarString, inputLine);
//
//  Set up the DISCIPLINE record
//
//  RecordID
//
    pch = strtok(inputLine, "\t\n");
    DISCIPLINEKey0.recordID = atol(pch);
    rcode2 = btrieve(B_GETEQUAL, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
    memset(&DISCIPLINE, 0x00, sizeof(DISCIPLINEDef));
    if(rcode2 == 0)
    {
      rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
      DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
    }
    else
    {
      DISCIPLINE.recordID = atol(pch);
    }
//
//  Comments recordID
//
    DISCIPLINE.COMMENTSrecordID = NO_RECORD;
//
//  Reported by (supervisor)
//
    pch = strtok(NULL, "\t\n");
    if(strcmp(pch, "") == 0)
    {
      DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
    }
    else
    {
      DISCIPLINE.reportedByDRIVERSrecordID = atol(pch);
    }
//
//  Date/time of entry
//
    pch = strtok(NULL, "\t\n");
    if(strcmp(pch, "") == 0)
    {
      dateOfEntry = 19800101;
    }
    else
    {
      dateOfEntry = atol(pch);
    }
    DISCIPLINE.entryDateAndTime = TMSmktime(dateOfEntry, 0);
//
//  Default the offense date/time
//
    DISCIPLINE.dateOfOffense = dateOfEntry;
    DISCIPLINE.timeOfOffense = 0;
//
//  Employee
//
    pch = strtok(NULL, "\t\n");
    DISCIPLINE.DRIVERSrecordID = atol(pch);
//
//  Action taken
//
    pch = strtok(NULL, "\t\n");
    DISCIPLINE.actionTaken = atol(pch);
//
//  Violation Category
//
    pch = strtok(NULL, "\t\n");
    DISCIPLINE.violationCategory = atol(pch);
//
//  Violation Type
//
    pch = strtok(NULL, "\t\n");
    DISCIPLINE.violationType = atol(pch);
//
//  Comment 1
//
    pch = strtok(NULL, "\t\n");
    strcpy(szComment[0], pch);
//
//  Comment 2
//
    pch = strtok(NULL, "\t\n");
    if(pch == NULL)
    {
      strcpy(szComment[1], "");
    }
    else
    {
      strcpy(szComment[1], pch);
    }
    strcpy(tempString, szComment[0]);
    strcat(tempString, "\r\n");
    strcat(tempString, szComment[1]);
    pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
    strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
//
//  Dropoff date (needs to be checked after load)
//
    DISCIPLINE.dropoffDate = DISCIPLINE.dateOfOffense + 10000;  // Default to a year
//
//  Flags
//
    DISCIPLINE.flags |= DISCIPLINE_FLAG_FIXEDFUNKYTIME;
    DISCIPLINE.flags |= DISCIPLINE_FLAG_ACTUALACTION;
//
//  DAILYOPSrecordID (formerly Reserved)
//
    DISCIPLINE.DAILYOPSrecordID = NO_RECORD;
//
//  Add the record
//
    rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
  }
//
//  All done
//
  MessageBox(NULL, "Done!", TMS, MB_OK);

  cleanup:
    if(fp != NULL)
    {
      fclose(fp);
    }
    return(TRUE);
}

