//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2005 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  ProcessAbsenceRules() - Like the routine name says
//
#include "TMSHeader.h"

long YMDO(long, long, long, long);

BOOL ProcessAbsenceRules(DAILYOPSDef *pDAILYOPS, BOOL bRegister)
{
  DAILYOPSDef localDAILYOPS;
  HCURSOR hSaveCursor;
  float totalMissouts;
  long unpaidTimeTotal;
  long paidTime[2];
  long unpaidTime;
  BOOL  bRecordAbsence;
  BOOL  bCheckUnpaidTime;
  BOOL  bPaid;
  long  year, month, day;
  long  startDate;
  long  endDate;
  long  quarterStart;
  long  timeLost;
  long  tempLong;
  char  szPaidTime[2][16];
  char  szUnpaidTime[16];
  int   rcode2;
  int   QCount;
  int   OCount;

  enum KUnion {NotRecorded, ATU, KMEA, NBU, AFSCME};  // Kalamazoo Unions

  int   reasonIndex = pDAILYOPS->DOPS.Absence.reasonIndex;
  int   ruleNumber;
  int   localReasonIndex;
  int   localRuleNumber;
  
  if(reasonIndex != NO_RECORD)
  {
    ruleNumber = m_AbsenceReasons[reasonIndex].ruleNumber;
  }
  else
  {
    ruleNumber = NO_RECORD;
  }
//
//  Get the driver record
//
  DRIVERSKey0.recordID = pDAILYOPS->DRIVERSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
//
//  Switch on all the rule numbers
//
  bRecordAbsence = FALSE;
  bCheckUnpaidTime = FALSE;
  if(bRegister)
  {
    paidTime[0] = NO_TIME;
    paidTime[1] = NO_TIME;
    unpaidTime = NO_TIME;
  }
  else
  {
    paidTime[0] = pDAILYOPS->DOPS.Absence.paidTime[0];
    paidTime[1] = pDAILYOPS->DOPS.Absence.paidTime[1];
    unpaidTime = pDAILYOPS->DOPS.Absence.unpaidTime;
  }
  timeLost = pDAILYOPS->DOPS.Absence.timeLost;
//
//  Switch
//
  switch(ruleNumber)
  {
//
//  *******************************************
//  *** Start Kalamazoo Metro Transit Rules ***
//  *******************************************
//
//  Kalamazoo rule 0 - Sick
// 
//  ATU gets its time from the vacation/sick pool.  If the pool drops below zero,
//  then the time is unpaid.  Other unions get the time from sick time.  If
//  sick time drops below zero, then the time comes out of vacation.  If vacation
//  drops below zero, then the time is unpaid.  Unpaid time is subject to discipline.
//  
//
    case 0:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        else if(DRIVERS.labourUnion == ATU)
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        else
        {
          if(DRIVERS.sickTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.sickTime;
            if(DRIVERS.vacationTime >= (pDAILYOPS->DOPS.Absence.timeLost - paidTime[0]))
            {
              paidTime[1] = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
              unpaidTime = 0;
            }
            else
            {
              paidTime[1] = DRIVERS.vacationTime;
              unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0] - paidTime[1];
            }
          }
          if(paidTime[1] == NO_TIME)
          {
            sprintf(tempString,
                  "Paid time from sick time: %s\nUnpaid time: 0.00\n\n", chhmm(paidTime[0]));
          }
          else
          {
            strcpy(szPaidTime[0], chhmm(paidTime[0]));
            strcpy(szPaidTime[1], chhmm(paidTime[1]));
            strcpy(szUnpaidTime, chhmm(unpaidTime));
            sprintf(tempString,
                  "Paid time from sick time: %s\nPaid time from vacation time: %s\nUnpaid time: %s\n\n",
                  szPaidTime[0], szPaidTime[1], szUnpaidTime);
          }
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime -= paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime -= paidTime[0];
          if(paidTime[1] != NO_TIME)
          {
            DRIVERS.vacationTime -= paidTime[1];
          }
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime += paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime += paidTime[0];
          DRIVERS.vacationTime += paidTime[1];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  Flag unpaid time, if necessary
//
      if(unpaidTime > 0)
      {
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 1 - Personal Leave - all unpaid - not subject to discipline
//
    case 1: 
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
//
//  Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 2 - Annual Leave
//
//  ATU gets its time from the vacation/sick pool.  If the pool drops below zero,
//  then the time is unpaid.  Other unions get the time from vacation time.
//  If vacation drops below zero, then the time is unpaid.
//
    case 2: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        else if(DRIVERS.labourUnion == ATU)
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        else
        {
          if(DRIVERS.vacationTime >= (pDAILYOPS->DOPS.Absence.timeLost - paidTime[0]))
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString,
                "Paid time from vacation time: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == ATU || DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.vacationTime -= paidTime[0];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  Flag unpaid time, if necessary
//
      if(unpaidTime > 0)
      {
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 3 - * UNUSED *
//
    case 3: 
      bRecordAbsence = FALSE;
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 4 - FMLA
//
//  ATU gets it time from the vacation/sick pool.  If the pool drops below zero,
//  then the time is unpaid.  Other unions get the time from sick time.  If
//  sick time drops below zero, then the time comes out of vacation.  If vacation
//  drops below zero, then the time is unpaid.  Unpaid time doesn't count in the yearly total.
//
    case 4:
//
//  Register
//
      if(bRegister)
      { 
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        else if(DRIVERS.labourUnion == ATU)
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        else
        {
          if(DRIVERS.sickTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.sickTime;
            if(DRIVERS.vacationTime >= (pDAILYOPS->DOPS.Absence.timeLost - paidTime[0]))
            {
              paidTime[1] = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
              unpaidTime = 0;
            }
            else
            {
              paidTime[1] = DRIVERS.vacationTime;
              unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0] - paidTime[1];
            }
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          if(paidTime[1] != NO_TIME)
          {
            sprintf(tempString,
                  "Paid time from sick time: %s\nUnpaid time: %s\n\n",
                   szPaidTime[0], szUnpaidTime);
          }
          else
          {
            strcpy(szPaidTime[1], chhmm(paidTime[1]));
            sprintf(tempString,
                  "Paid time from sick time: %s\nPaid time from vacation time: %s\nUnpaid time: %s\n\n",
                  szPaidTime[0], szPaidTime[1], szUnpaidTime);
          }
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime -= paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime -= paidTime[0];
          if(paidTime[1] != NO_TIME)
          {
            DRIVERS.vacationTime -= paidTime[1];
          }
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime += paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime += paidTime[0];
          DRIVERS.vacationTime += paidTime[1];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);

      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 5 - Vacation
//
//  ATU gets it time from the vacation/sick pool.  If the pool drops below zero,
//  then the time is unpaid.  Other unions get the time from vacation time.
//  If vacation drops below zero, then the time is unpaid.
//
    case 5: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        else if(DRIVERS.labourUnion == ATU)
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        else
        {
          if(DRIVERS.vacationTime >= (pDAILYOPS->DOPS.Absence.timeLost - paidTime[0]))
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString,
                "Paid time from vacation time: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == ATU || DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.vacationTime -= paidTime[0];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  Flag unpaid time, if necessary
//
      if(unpaidTime > 0)
      {
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 6 - Floating Holiday
//
//  Comes off the floating holiday
//
    case 6: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        else
        {
          if(DRIVERS.floatTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            strcpy(tempString, "There isn't enough time left to register a floating holiday");
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
            break;
          }
          sprintf(tempString, "Paid time from floating holiday: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.floatTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        DRIVERS.floatTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 7 - Suspension
//
    case 7: 
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 8 - Medical Leave
//
//  ATU gets it time from the vacation/sick pool.  If the pool drops below zero,
//  then the time is unpaid.  Other unions get the time from sick time.  If
//  sick time drops below zero, then the time comes out of vacation.  If vacation
//  drops below zero, then the time is unpaid.  There's no unpaid time check in rule 8.
//  
//
    case 8:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        else if(DRIVERS.labourUnion == ATU)
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
        }
        else
        {
          if(DRIVERS.sickTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.sickTime;
            if(DRIVERS.vacationTime >= (pDAILYOPS->DOPS.Absence.timeLost - paidTime[0]))
            {
              paidTime[1] = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
              unpaidTime = 0;
            }
            else
            {
              paidTime[1] = DRIVERS.vacationTime;
              unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0] - paidTime[1];
            }
          }
          if(paidTime[1] != NO_TIME)
          {
            sprintf(tempString,
                  "Paid time from sick time: %s\nUnpaid time: 0:00\n\n", chhmm(paidTime[0]));
          }
          else
          {
            strcpy(szPaidTime[0], chhmm(paidTime[0]));
            strcpy(szPaidTime[1], chhmm(paidTime[1]));
            strcpy(szUnpaidTime, chhmm(unpaidTime));
            sprintf(tempString,
                  "Paid time from sick time: %s\nPaid time from vacation time: %s\nUnpaid time: %s\n\n",
                  szPaidTime[0], szPaidTime[1], szUnpaidTime);
          }
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime -= paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime -= paidTime[0];
          if(paidTime[1] != NO_TIME)
          {
            DRIVERS.vacationTime -= paidTime[1];
          }
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        if(DRIVERS.labourUnion == ATU)
        {
          DRIVERS.vacationTime += paidTime[0];
        }
        else if(DRIVERS.labourUnion == KMEA ||
              DRIVERS.labourUnion == NBU || DRIVERS.labourUnion == AFSCME)
        {
          DRIVERS.sickTime += paidTime[0];
          DRIVERS.vacationTime += paidTime[1];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 9 - Personal Day
//
    case 9: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != KMEA && DRIVERS.labourUnion != NBU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        if(DRIVERS.labourUnion == KMEA)
        {
          if(DRIVERS.sickTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
            sprintf(tempString, "Paid time from sick time: %s\n\n", chhmm(paidTime[0]));
          }
          else
          {
            strcpy(tempString, "There isn't enough time left to register a personal day");
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
            break;
          }
        }
        else
        {
          if(DRIVERS.personalTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
            sprintf(tempString, "Paid time from personal time: %s\n\n", chhmm(paidTime[0]));
          }
          else
          {
            strcpy(tempString, "There isn't enough time left to register a personal day");
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
            break;
          }
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        if(DRIVERS.labourUnion == KMEA)
        {
          DRIVERS.sickTime -= paidTime[0];
        }
        else
        {
          DRIVERS.personalTime -= paidTime[0];
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        if(DRIVERS.labourUnion == KMEA)
        {
          DRIVERS.sickTime += paidTime[0];
        }
        else
        {
          DRIVERS.personalTime += paidTime[0];
        }
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 10 - Critical Illness
//
    case 10: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU &&
              DRIVERS.labourUnion != KMEA && DRIVERS.labourUnion != AFSCME)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 11 - Funeral Leave
//
    case 11: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 12 - 1/2 Miss Out
//  Kalamazoo rule 13 - Miss Out
//
//  Missouts are given for late call outs and reports.  A late call is calling out
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
    case 12: 
    case 13: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
//
//  Calculate the total number of missouts in the past 90 days
//
//
//  Key 3 : DRIVERSrecordID : "Pertains to" Date : "Pertains to" Time : Record type : Record Flags
//
        hSaveCursor = SetCursor(hCursorWait);
        GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
        startDate = YMDO(year, month, day, -90);
        DAILYOPSKey3.DRIVERSrecordID = DRIVERS.recordID;
        DAILYOPSKey3.pertainsToDate = startDate;
        DAILYOPSKey3.pertainsToTime = NO_TIME;
        DAILYOPSKey3.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
        DAILYOPSKey3.recordFlags = 0;
        totalMissouts = 0;
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey3, 3);
        while(rcode2 == 0 &&
              localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
              localDAILYOPS.pertainsToDate <= pDAILYOPS->pertainsToDate)
        {
          if(!ANegatedRecord(localDAILYOPS.recordID, 3) && !ANegatingRecord(&localDAILYOPS))
          {
            if(localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
            {
              localReasonIndex = localDAILYOPS.DOPS.Absence.reasonIndex;
              if(localReasonIndex != NO_RECORD)
              {
                localRuleNumber = m_AbsenceReasons[localReasonIndex].ruleNumber;
//                if(localDAILYOPS.DOPS.Absence.untilDate <= pDAILYOPS->DOPS.Absence.untilDate &&
//                      (localRuleNumber == 12 || localRuleNumber == 13))
                if(localRuleNumber == 12 || localRuleNumber == 13)
                {
                  totalMissouts += (float)(localRuleNumber == 12 ? 0.5 : 1.0);
                }
              }
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey3, 3);
        }
        SetCursor(hSaveCursor);
//
//  Display the numbers
//
        totalMissouts += (float)(ruleNumber == 12 ? 0.5 : 1.0);
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = TRUE;
        LoadString(hInst, ABSENCE_012, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(tempString, szFormatString, totalMissouts);
        MessageBeep(MB_ICONINFORMATION);
        MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
//
//  One or more missout in the last 90 days - write out a discipline record too
//
        if(totalMissouts >= 1)
        {
          LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONQUESTION);
//
//  Verify that's what he wants to do
//
          if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
          {
            rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
            DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
            DISCIPLINE.COMMENTSrecordID = NO_RECORD;
            DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
            DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
            memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
            DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
            DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
            DISCIPLINE.violationCategory = 8;  // Attendance
            DISCIPLINE.violationType = (ruleNumber == 12 ? 0 : 1);  // 1/2 Missout vs. Missout
            GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
            DISCIPLINE.dropoffDate = YMDO(year, month, day, 90);
            LoadString(hInst, TEXT_327, szFormatString, SZFORMATSTRING_LENGTH);
            sprintf(tempString, szFormatString, totalMissouts);
            pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
            strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
            DISCIPLINE.suspensionDate = 0;
            DISCIPLINE.suspensionDuration = 0;
            DISCIPLINE.actionTaken = NO_RECORD;
            DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
            DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
            DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
            DISCIPLINE.userID = pDAILYOPS->userID;
            rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          }
//
//  No - he's backed off
//
          else
          {
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_339, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
//
//  And check the unpaid time
//
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 14 - Occurrence
//
//  Employees can call out anytime up to an hour (a half hour if the report time is before 6 AM)
//  before work and still get paid if they have paid time (annual leave) in their bank.  Anytime
//  that a call out from work is given after noon the day before up to an hour before work a
//  person is given a "Q".
//
//  This is with pay so they would also be given a V = Vacation or rather paid annual
//  leave. After accumulating 3 Q's in a quarter (calendar quarter, not just any three
//  months), the fourth call out within this time frame is without pay (U =unpaid time
//  for full-time drivers; N=unpaid time for part-time drivers), even if they have paid
//  time in their bank.  So Q's track untimely call outs, i.e., call outs after noon the
//  day before.  After getting 3 Q's in a calendar quarter a person no longer gets paid 
//  for these types of call outs.
//
//  Monthly quarters: 1 = Jan, Feb, Mar; 2 = Apr, May, Jun; 3 = Jul, Aug, Sep; 4 = Oct, Nov, Dec
//  Anything in quarter 1 goes to Jan, quarter 2 to Apr, quarter 3 to Jul, and quarter 4 to Oct.
//
    case 14:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        hSaveCursor = SetCursor(hCursorWait);
        GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
        if(month >= 10)
        {
          quarterStart = 10;
        }
        else if(month >= 7)
        {
          quarterStart = 7;
        }
        else if(month >= 4)
        {
          quarterStart = 4;
        }
        else
        {
          quarterStart = 1;
        }
        startDate = year * 10000 + quarterStart * 100 + 1;
        endDate = year * 10000 + (quarterStart + 2) * 100 + 31;
        DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
        DAILYOPSKey1.pertainsToDate = startDate;
        DAILYOPSKey1.pertainsToTime = NO_TIME;
        DAILYOPSKey1.recordFlags = 0;
        QCount = 0;
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        while(rcode2 == 0 &&
              (localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
        {
          if(!ANegatedRecord(localDAILYOPS.recordID, 1) && !ANegatingRecord(&localDAILYOPS))
          {
            if(localDAILYOPS.pertainsToDate >= startDate &&
                  localDAILYOPS.pertainsToDate <= endDate &&
                  localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
                  localDAILYOPS.DOPS.Absence.reasonIndex == reasonIndex)
            {
              QCount++;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        }
        SetCursor(hSaveCursor);
//
//  Now that we have a count of Qs this quarter, let him know if it's
//  paid or unpaid, and fix any of the "paid" flags that might need fixing.
//
        bPaid = (QCount < 3 && DRIVERS.vacationTime - pDAILYOPS->DOPS.Absence.timeLost >= 0);
        MessageBeep(MB_ICONQUESTION);
        if(bPaid)
        { 
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          LoadString(hInst, ABSENCE_009, tempString, TEMPSTRING_LENGTH);
          if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
          {
            if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_NOTSUREIFPAID)
            {
              pDAILYOPS->recordFlags &= ~DAILYOPS_FLAG_NOTSUREIFPAID;
            }
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
            DRIVERS.vacationTime -= pDAILYOPS->DOPS.Absence.timeLost;
            rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
            bRecordAbsence = TRUE;
            bCheckUnpaidTime = FALSE;
          }
        }
        else
        {
          if(QCount >= 3)
          {
            LoadString(hInst, ABSENCE_011, tempString, TEMPSTRING_LENGTH);
            if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
            {
              LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
              MessageBeep(MB_ICONQUESTION);
              if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
              {
                rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
                DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
                DISCIPLINE.COMMENTSrecordID = NO_RECORD;
                DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
                DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
                memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
                DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
                DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
                DISCIPLINE.violationCategory = 8;  // Attendance
                DISCIPLINE.violationType = 2;      // Occurrence
                GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
                DISCIPLINE.dropoffDate = YMDO(year, month, day, 90);
                LoadString(hInst, TEXT_330, szFormatString, SZFORMATSTRING_LENGTH);
                sprintf(tempString, szFormatString, QCount);
                pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
                strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
                DISCIPLINE.suspensionDate = 0;
                DISCIPLINE.suspensionDuration = 0;
                DISCIPLINE.actionTaken = NO_RECORD;
                DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
                DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
                DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
                DISCIPLINE.userID = pDAILYOPS->userID;
                rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
                paidTime[0] = 0;
                unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
                bRecordAbsence = TRUE;
                bCheckUnpaidTime = TRUE;
              }
            }
          }
          else
          {
            LoadString(hInst, ABSENCE_010, szFormatString, SZFORMATSTRING_LENGTH);
            sprintf(tempString, szFormatString, chhmm(DRIVERS.vacationTime));
            if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
            {
              bRecordAbsence = TRUE;
              bCheckUnpaidTime = TRUE;
              unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
            }
          }
//
//  Fix the flags, if necessary
//
          if(bRecordAbsence)
          {
            if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_ABSENCEPAID)
            {
              pDAILYOPS->recordFlags &= ~DAILYOPS_FLAG_ABSENCEPAID;
            }
            else if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_NOTSUREIFPAID)
            {
              pDAILYOPS->recordFlags &= ~DAILYOPS_FLAG_NOTSUREIFPAID;
            }
          }
        }
      }
//
//  Unregister
//
      else
      {
        if(paidTime[0] > 0)
        {
          DRIVERS.vacationTime += paidTime[0];
          rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        }
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_337, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
//
//  And check the unpaid time
//
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 15 - Run Split
//
//  * Was Run Split - Now Unused *
//
    case 15:
      bRecordAbsence = FALSE;
      bCheckUnpaidTime = FALSE;
/*
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  Check on unpaid time
//
      bCheckUnpaidTime = TRUE;
*/
      break;
//
//  Kalamazoo rule 16 - Meeting
//
    case 16: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        }
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 17 - City Doctor
//
    case 17: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 18 - Physical Therapy
//
    case 18: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 19 - Union Business
//
//  Only ATU - can be paid or unpaid
//
    case 19:
//
//  Register
//
      if(bRegister)
      { 
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        LoadString(hInst, TEXT_317, tempString, TEMPSTRING_LENGTH);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        else
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 20 - Occasion
//
//  An occasion occurs when an employee is more that two hours late without any justifiable reason.
//  Three or more occasions in 12 months results in a disciplinary letter and possible suspension.
//
    case 20: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        hSaveCursor = SetCursor(hCursorWait);
        GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
        startDate = (year - 1) * 10000 + month * 100 + day;
        endDate = pDAILYOPS->pertainsToDate;
        DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
        DAILYOPSKey1.pertainsToDate = startDate;
        DAILYOPSKey1.pertainsToTime = NO_TIME;
        DAILYOPSKey1.recordFlags = 0;
        OCount = 1;  // Today's occasion
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        while(rcode2 == 0 &&
              (localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
        {
          if(!ANegatedRecord(localDAILYOPS.recordID, 1) && !ANegatingRecord(&localDAILYOPS))
          {
            if(localDAILYOPS.pertainsToDate >= startDate &&
                  localDAILYOPS.pertainsToDate <= endDate &&
                  localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
                  localDAILYOPS.DOPS.Absence.reasonIndex == reasonIndex)
            {
              OCount++;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        }
        SetCursor(hSaveCursor);
//
//  Now that we have a count of Os for the past year, see if we have to make a discipline entry
//
        MessageBeep(MB_ICONINFORMATION);
        LoadString(hInst, ABSENCE_013, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(tempString, szFormatString, OCount);
        MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
//        if(OCount >= 3)
//        {
          LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONQUESTION);
          if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
          {
            rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
            DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
            DISCIPLINE.COMMENTSrecordID = NO_RECORD;
            DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
            DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
            memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
            DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
            DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
            DISCIPLINE.violationCategory = 8;  // Attendance
            DISCIPLINE.violationType = 3;      // Occasion
            GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
            DISCIPLINE.dropoffDate = (year + 1) * 10000 + month * 100 + day;
            LoadString(hInst, TEXT_331, szFormatString, SZFORMATSTRING_LENGTH);
            sprintf(tempString, szFormatString, OCount);
            pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
            strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
            DISCIPLINE.suspensionDate = 0;
            DISCIPLINE.suspensionDuration = 0;
            DISCIPLINE.actionTaken = NO_RECORD;
            DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
            DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
            DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
            DISCIPLINE.userID = pDAILYOPS->userID;
            rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          }
//        }
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = TRUE;
        if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_ABSENCEPAID)
        {
          pDAILYOPS->recordFlags &= ~DAILYOPS_FLAG_ABSENCEPAID;
        }
        else if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_NOTSUREIFPAID)
        {
          pDAILYOPS->recordFlags &= ~DAILYOPS_FLAG_NOTSUREIFPAID;
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_337, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
//
//  And check the unpaid time
//
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Kalamazoo rule 21 - Workers' Compensation
//
    case 21:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 22 - Accident
//
    case 22: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU)
        {
          LoadString(hInst, TEXT_336, tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, "Did this accident occur today?",
              TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        else
        {
          if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
          {
            paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
            unpaidTime = 0;
          }
          else
          {
            paidTime[0] = DRIVERS.vacationTime;
            unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
          }
          strcpy(szPaidTime[0], chhmm(paidTime[0]));
          strcpy(szUnpaidTime, chhmm(unpaidTime));
          sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n",
                szPaidTime[0], szUnpaidTime);
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 23 - Donated Hours
//
    case 23: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Donated hours from vacation time: %s\n\n", chhmm(paidTime[0]));
        }
        else
        {
          strcpy(tempString, "There isn't enough time left to donate this many hours");
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          bRecordAbsence = FALSE;
          bCheckUnpaidTime = FALSE;
          break;
        }
        LoadString(hInst, TEXT_335, tempString, TEMPSTRING_LENGTH);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.vacationTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = FALSE;
      }
//
//  Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 24 - Emergency Leave
//
    case 24: 
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  Kalamazoo rule 25 - Jury Duty
//
    case 25: 
      if(bRegister)
      {
        if(DRIVERS.labourUnion != ATU && DRIVERS.labourUnion != KMEA &&
              DRIVERS.labourUnion != NBU && DRIVERS.labourUnion != AFSCME)
        {
          paidTime[0] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
          sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
          pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        }
        else
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
          sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        }
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  *****************************************
//  *** End Kalamazoo Metro Transit Rules ***
//  *****************************************
//
//
//  *******************************
//  *** Start Bermuda PTB Rules ***
//  *******************************
//
//  PTB Rules
//
//  PTB rule 0 - Sick (Certified)
//
    case 26: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 1 - Sick (Uncertified)
//
    case 27: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 2 - Sick/disabled (long term)
//
    case 28: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 3 - Personal leave
//
    case 29: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 4 - Day in lieu
//
    case 30: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 5 - Vacation
//
    case 31: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        bRecordAbsence = TRUE;
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 6 - Weather
//
    case 32: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 7 - Suspension
//
    case 33: 
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 8 - Compassionate leave
//
    case 34: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 9 - Union meeting/business
//
    case 35: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 10 - Doctor's appointment
//
    case 36: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 11 - Accident
//
    case 37: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 12 - Emergency leave
//
    case 38: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 13 - Jury duty
//
    case 39: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 14 - Regiment duty
//
    case 40: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 15 - Subpoena
//
    case 41: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 16 - Maternity leave
//
    case 42: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 17 - Paternity leave
//
    case 43: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 18 - Adoption leave
//
    case 44: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  PTB rule 19 - Failed
//
    case 45: 
      if(bRegister)
      {
        LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
        MessageBeep(MB_ICONQUESTION);
//
//  Verify that's what he wants to do
//
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
          rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
          DISCIPLINE.COMMENTSrecordID = NO_RECORD;
          DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
          DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
          memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
          DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
          DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
          DISCIPLINE.violationCategory = 8;  // Attendance
          DISCIPLINE.violationType = ruleNumber;
          GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
          DISCIPLINE.dropoffDate = YMDO(year, month, day, 90);
          strcpy(tempString, "");
          pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
          strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
          DISCIPLINE.suspensionDate = 0;
          DISCIPLINE.suspensionDuration = 0;
          DISCIPLINE.actionTaken = NO_RECORD;
          DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
          DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
          DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
          DISCIPLINE.userID = pDAILYOPS->userID;
          rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          bRecordAbsence = TRUE;
        }
//
//  No - he's backed off
//
        else
        {
          bRecordAbsence = FALSE;
        }
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  *****************************
//  *** End Bermuda PTB Rules ***
//  *****************************
//
//  ***********************************
//  *** Start Alexandria DASH Rules ***
//  ***********************************
//
//  DASH Rule 0 - Injury (Workman's Comp)
//
    case 46: 
      if(bRegister)
      {
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH Rule 1 - Sick
//
    case 47: 
      if(bRegister)
      {
        if(DRIVERS.sickTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.sickTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0];
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szPaidTime[1], chhmm(paidTime[1]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from sick time: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.sickTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.sickTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 2 - Vacation
//
    case 48:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.vacationTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.vacationTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 3 - Military leave (all unpaid)
//
    case 49:
//
//  Register
//
      if(bRegister)
      {
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 4 - FMLA
//
    case 50:
//
//  Start off by chewing vacation time, and when that runs out, use up to 
//  twelve weeks of sick time, and when that runs out, it's not paid
//
//  One day = 8 hours, one week = 40 hours, 12 weeks = 480 hours => 28800 minutes => 1728000 seconds 
//
//  Register
//
      if(bRegister)
      { 
        if(DRIVERS.vacationTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          paidTime[1] = 0;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.vacationTime;
          paidTime[1] = min(pDAILYOPS->DOPS.Absence.timeLost - paidTime[1], 1728000);
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - paidTime[0] - paidTime[1];
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szPaidTime[1], chhmm(paidTime[1]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from vacation pool: %s\nPaid time from sick pool: %s\nUnpaid time: %s\n\n",
              szPaidTime[0], szPaidTime[1], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.vacationTime -= paidTime[0];
        DRIVERS.sickTime -= paidTime[1];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        DRIVERS.sickTime += paidTime[1];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH Rule 5 - Miss-out
//
//  A missout occurs when an employee does not report on time to the Dispatch window, 
//  unless otherwise specifically directed.  Part-time operators will be charges with 
//  a missout if they do not notify TMS of their unavailability.  An operator who
//  refuses an assignment will be charged with a missout.  If an operator has a missout,
//  he or she will automatically forfeit his or her assignment and be dropped to the bottom
//  of the extra board and will be required to work if needed.
//
//  Missouts in a 180-day calendar period:
//
//  1st - Counseling
//  2nd - Written warning
//  3rd - Written warning
//  4th - Suspension (1 day)
//  5th - Suspension (3 day)
//  6th - Suspension (5 day)
//  7th - Dismissal
//
//  The Company, in its discretion, may elect not to charge a missout that it determines
//  was caused by circumstances beyond the operator's control
//
    case 51: 
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
//
//  Calculate the total number of missouts in the past 180 days
//
        hSaveCursor = SetCursor(hCursorWait);
        GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
        startDate = YMDO(year, month, day, -180);
        DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
        DAILYOPSKey1.pertainsToDate = startDate;
        DAILYOPSKey1.pertainsToTime = NO_TIME;
        DAILYOPSKey1.recordFlags = 0;
        totalMissouts = 0;
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        while(rcode2 == 0 &&
              localDAILYOPS.pertainsToDate <= pDAILYOPS->pertainsToDate &&
              (localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
        {
          if(!ANegatedRecord(localDAILYOPS.recordID, 1) && !ANegatingRecord(&localDAILYOPS))
          {
            localReasonIndex = localDAILYOPS.DOPS.Absence.reasonIndex;
            if(localReasonIndex == NO_RECORD)
            {
              localRuleNumber = NO_RECORD;
            }
            else
            {
              localRuleNumber = m_AbsenceReasons[localReasonIndex].ruleNumber;
            }
            if(localDAILYOPS.DOPS.Absence.untilDate <= pDAILYOPS->DOPS.Absence.untilDate &&
                  localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
                  (localRuleNumber == 50))
            {
              totalMissouts += 1;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
        }
        SetCursor(hSaveCursor);
//
//  Display the numbers
//
        totalMissouts += 1;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = TRUE;
        LoadString(hInst, ABSENCE_015, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(tempString, szFormatString, totalMissouts);
        MessageBeep(MB_ICONINFORMATION);
        MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
//
//  One or more missouts in the last 180 days - write out a discipline record too
//
        if(totalMissouts >= 1)
        {
          LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONQUESTION);
//
//  Verify that's what he wants to do
//
          if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
          {
            rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
            DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
            DISCIPLINE.COMMENTSrecordID = NO_RECORD;
            DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
            DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
            memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
            DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
            DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
            DISCIPLINE.violationCategory = 8;  // Attendance
            DISCIPLINE.violationType = pDAILYOPS->DOPS.Absence.reasonIndex;
            GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
            DISCIPLINE.dropoffDate = YMDO(year, month, day, 180);
            LoadString(hInst, TEXT_327, szFormatString, SZFORMATSTRING_LENGTH);
            sprintf(tempString, szFormatString, totalMissouts);
            pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
            strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
            DISCIPLINE.suspensionDate = 0;
            DISCIPLINE.suspensionDuration = 0;
            DISCIPLINE.actionTaken = NO_RECORD;
            DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
            DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
            DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
            DISCIPLINE.userID = pDAILYOPS->userID;
            rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          }
//
//  No - he's backed off
//
          else
          {
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_339, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 6 - Distinguished Driver (paid)
//
    case 52:
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        paidTime[1] = 0;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 7 - Report off
//
    case 53:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.personalTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.personalTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.personalTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.personalTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 8 - Jury Duty (Register as unpaid)
//
    case 54:
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n(Payroll will handle pay differential)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 9 - Subpoena
//
    case 55:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.personalTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.personalTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.personalTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.personalTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 10 - Bereavement
//
    case 56:
//
//  Start off by chewing up to three days of personal time, and when that runs out, it's not paid
//
//  One day = 8 hours, three day = 24 hours, 24 hours => 1440 minutes => 86400 seconds 
//
//  Register
//
      if(bRegister)
      { 
        tempLong = min(pDAILYOPS->DOPS.Absence.timeLost, 86400);
        if(DRIVERS.personalTime >= tempLong)
        {
          paidTime[0] = tempLong;
          paidTime[1] = 0;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - tempLong;
        }
        else
        {
          paidTime[0] = DRIVERS.personalTime;
          paidTime[1] = 0;
          unpaidTime = tempLong - paidTime[0];
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szPaidTime[1], chhmm(paidTime[1]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from vacation pool: %s\nPaid time from sick pool: %s\nUnpaid time: %s\n\n",
              szPaidTime[0], szPaidTime[1], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.personalTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
//  Unregister
//
      else
      {
        DRIVERS.vacationTime += paidTime[0];
        DRIVERS.sickTime += paidTime[1];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 11 - Personal Time
//
    case 57:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.personalTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.personalTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.vacationTime;
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.personalTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.personalTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 12 - Float Time
//
    case 58:
//
//  Register
//
      if(bRegister)
      {
        if(DRIVERS.floatTime >= pDAILYOPS->DOPS.Absence.timeLost)
        {
          paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
          unpaidTime = 0;
        }
        else
        {
          paidTime[0] = DRIVERS.floatTime;
          unpaidTime = pDAILYOPS->DOPS.Absence.timeLost - DRIVERS.floatTime;
        }
        strcpy(szPaidTime[0], chhmm(paidTime[0]));
        strcpy(szUnpaidTime, chhmm(unpaidTime));
        sprintf(tempString, "Paid time from pool: %s\nUnpaid time: %s\n\n", szPaidTime[0], szUnpaidTime);
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        DRIVERS.personalTime -= paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        bRecordAbsence = TRUE;
      }
//
// Unregister
//
      else
      {
        DRIVERS.personalTime += paidTime[0];
        rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 13 - Leave of Absence (unpaid)
//
    case 59:
//
//  Register
//
      if(bRegister)
      {
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 14 - Charter (paid)
//
    case 60:
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        paidTime[1] = 0;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 15 - Suspension (unpaid)
//
    case 61:
//
//  Register
//
      if(bRegister)
      {
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 16 - Training (paid)
//  DASH rule 17 - Meeting (paid)
//  DASH rule 18 - Paid Administrative Leave (paid)
//  DASH rule 19 - Extraboard SDO
//  DASH rule 20 - Traded time
//  DASH rule 21 - Short-term disability
//
    case 62:
    case 63:
    case 64:
    case 207:
    case 208:
    case 209:
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        paidTime[1] = 0;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  DASH rule 22 - AWOL
//
    case 210:
//
//  Register
//
      if(bRegister)
      {
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  *********************************
//  *** End Alexandria DASH Rules ***
//  *********************************
//
//
//  ***********************************
//  *** Start Bridgeport GBTA Rules ***
//  ***********************************
//
//
//  GBTA
//
    case 65:  // 	Accident
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 66:  // 	Bereavement
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 67:  // 	Birthday
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 68:  // 	BOOT
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 69:  // 	Disability
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 70:  // 	Excused
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 71:  // 	Excused holiday
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 72:  // 	FMLA
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 73:  // 	Holiday
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 74:  // 	Investigation
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 75:  // 	Jury Duty
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 76:  // 	Keystone training
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 77:  // 	Late
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 78:  // 	Leave of absence
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 79:  // 	Medical certificate
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 80:  // 	Meeting
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 81:  // 	Military leave
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 82:  // 	On the job injury
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 83:  // 	Pension business
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 84:  // 	Personal business
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 85:  // 	Personal holiday
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 86:  // 	Sick
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 87:  // 	Sleep over
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 88:  // 	Suspension
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 89:  // 	Training
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 90:  // 	Unexcused absence
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 91:  // 	Union business
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 92:  // 	Vacation
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 93:  // 	Vacation day
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;

    case 94:  // 	Worker's compensation
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        paidTime[1] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Time used: %s\n(Payroll will decide paid/unpaid)\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
//
// Unregister (nothing to do)
//
      else
      {
      }
//
//  No check on unpaid time
//
      bCheckUnpaidTime = FALSE;
      break;
//
//  *********************************
//  *** End Bridgeport GBTA Rules ***
//  *********************************
//
//
//  ************************************
//  *** Start Grand Rapids ITP Rules ***
//  ************************************
//
    case 95:  // Absent
      bRecordAbsence = TRUE;
      break;

    case 96:  // Absent - no work offered
      bRecordAbsence = TRUE;
      break;

    case 97:  // Accident
      bRecordAbsence = TRUE;
      break;

    case 98:  // Accident report
      bRecordAbsence = TRUE;
      break;

    case 99:  // Additional training
      bRecordAbsence = TRUE;
      break;

    case 100: // Unused
      bRecordAbsence = TRUE;
      break;

    case 101: // Assigned to dispatch
      bRecordAbsence = TRUE;
      break;

    case 102: // AWOL
      bRecordAbsence = TRUE;
      break;

    case 103: // Bereavement
      bRecordAbsence = TRUE;
      break;

    case 104: // Business meeting
      bRecordAbsence = TRUE;
      break;

    case 105: // Disciplinary time off
      bRecordAbsence = TRUE;
      break;

    case 106: // Disciplinary time off (points)
      bRecordAbsence = TRUE;
      break;

    case 107: // Driver paid for run/time scheduled but not worked
      bRecordAbsence = TRUE;
      break;

    case 108: // Driver trainer
      bRecordAbsence = TRUE;
      break;

    case 109: // Drug test (awaiting results)
      bRecordAbsence = TRUE;
      break;

    case 110: // Drug test (random/post-accident)
      bRecordAbsence = TRUE;
      break;

    case 111: // FMLA
      bRecordAbsence = TRUE;
      break;

    case 112: // Floating holiday
      bRecordAbsence = TRUE;
      break;

    case 113: // Goodwill light duty
      bRecordAbsence = TRUE;
      break;

    case 114: // Granted time off
      bRecordAbsence = TRUE;
      break;

    case 115: // Injury
      bRecordAbsence = TRUE;
      break;

    case 116: // Jury duty
      bRecordAbsence = TRUE;
      break;

    case 117: // Late arrival
      bRecordAbsence = TRUE;
      break;

    case 118: // Late call-in
      bRecordAbsence = TRUE;
      break;

    case 119: // Light duty
      bRecordAbsence = TRUE;
      break;

    case 120: // Military leave
      bRecordAbsence = TRUE;
      break;

    case 121: // Missing (< 2 hours)
      bRecordAbsence = TRUE;
      break;

    case 122: // Other (negative drug screen)
      bRecordAbsence = TRUE;
      break;

    case 123: // Part-time guaranteed day off
      bRecordAbsence = TRUE;
      break;

    case 124: // Personal court appearance
      bRecordAbsence = TRUE;
      break;

    case 125: // Personal day off
      bRecordAbsence = TRUE;
      break;

    case 126: // Personal leave (> 1 day)
      bRecordAbsence = TRUE;
      break;

    case 127: // Personal time off granted
      bRecordAbsence = TRUE;
      break;

    case 128: // Protect
      bRecordAbsence = TRUE;
      break;

    case 129: // Relieve driver of another bus
      bRecordAbsence = TRUE;
      break;

    case 130: // Road supervisor
      bRecordAbsence = TRUE;
      break;

    case 131: // Route timing
      bRecordAbsence = TRUE;
      break;

    case 132: // Short term disability
      bRecordAbsence = TRUE;
      break;

    case 133: // Statutory holiday
      bRecordAbsence = TRUE;
      break;

    case 134: // Union business granted
      bRecordAbsence = TRUE;
      break;

    case 135: // Vacation
      bRecordAbsence = TRUE;
      break;

    case 136: // Witness testify in court
      bRecordAbsence = TRUE;
      break;

    case 137: // Workers' compensation
      bRecordAbsence = TRUE;
      break;
//
//  **********************************
//  *** End Grand Rapids ITP Rules ***
//  **********************************
//
//  ********************************
//  *** Start Wood Buffalo Rules ***
//  ********************************
//
    case 138: // Sick certified (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 139: // Sick (Not certified) (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 140:  // Sick/Disabled (Sun Life) (paid by Sun Life)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 141: // Sick/Disabled (WCB) (paid by WCB)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 142:  // Personal Leave (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 143:  // Day in lieu (paid as per lieu hours)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 144:  // Vacation (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 145:  // Weather (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 146: // Suspension (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 147:  // Compassionate leave (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 148:  // Union Business/Meeting (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 149:  // Union Business/Meeting (Un-Paid) (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 150:  // Doctor's appointment (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 151:  // Collision (Non Discipline) (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 152:  // Emergency leave (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 153:  // Jury duty (paid) 
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 154:  // Court/Subpeona (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 155:  // Maternity/Paternity/Adoption (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 156:  // No show (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 157:  // Alternate work (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 158:  // Substance Testing (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 159:  // Car Trouble (unpaid) 
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 160:  // Seasonal layoff (unpaid)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 161:  // Driver trainer (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 162:  // Training (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
    
    case 163:  // Bereavement (paid)
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
//
//  ******************************
//  *** End Wood Buffalo Rules ***
//  ******************************
//
//  *********************************
//  *** Start Santa Clarita Rules ***
//  *********************************
//
    case 164:  // Sick (PTO)
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 165:  // FMLA
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 166:  // Vacation
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 167:  // Approved PTO
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 168:  // Suspension
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 169:  // Medical leave
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 170:  // Emergency PTO
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 171:  // Bereavement
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 172:  // 1/2 Miss-out
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 173:  // Miss-out
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 174:  // No call / no show
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 175:  // Meeting
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
    case 176:  // City doctor
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 177:  // Union meeting
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 178:  // Worker's comp
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 179:  // Accident
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
    case 180:  // Jury duty
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 181:  // Tardy
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 182:  // Injury
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

    case 183:  // Light duty
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
//
//  *******************************
//  *** End Santa Clarita Rules ***
//  *******************************
//
//  ************************
//  *** Start GLTC Rules ***
//  ************************
//
//  Miss-out: 2 or more in a 30 day period generates a discipline record
//
    case 193:  // Miss-out	
//
//  Register
//
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
//
//  Calculate the total number of missouts in the past 30 days
//
//
//  Key 3 : DRIVERSrecordID : "Pertains to" Date : "Pertains to" Time : Record type : Record Flags
//
        hSaveCursor = SetCursor(hCursorWait);
        GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
        startDate = YMDO(year, month, day, -30);
        DAILYOPSKey3.DRIVERSrecordID = DRIVERS.recordID;
        DAILYOPSKey3.pertainsToDate = startDate;
        DAILYOPSKey3.pertainsToTime = NO_TIME;
        DAILYOPSKey3.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
        DAILYOPSKey3.recordFlags = 0;
        totalMissouts = 0;
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey3, 3);
        while(rcode2 == 0 &&
              localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
              localDAILYOPS.pertainsToDate <= pDAILYOPS->pertainsToDate)
        {
          if(!ANegatedRecord(localDAILYOPS.recordID, 3) && !ANegatingRecord(&localDAILYOPS))
          {
            if(localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
            {
              localReasonIndex = localDAILYOPS.DOPS.Absence.reasonIndex;
              if(localReasonIndex != NO_RECORD)
              {
                localRuleNumber = m_AbsenceReasons[localReasonIndex].ruleNumber;
                if(localRuleNumber == 193)
                {
                  totalMissouts++;
                }
              }
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey3, 3);
        }
        SetCursor(hSaveCursor);
//
//  Display the numbers
//
        totalMissouts ++;  // today's miss-out
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = TRUE;
        LoadString(hInst, ABSENCE_012, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(tempString, szFormatString, totalMissouts);
        MessageBeep(MB_ICONINFORMATION);
        MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
//
//  Two or more missout in the last 30 days - write out a discipline record too
//
        if(totalMissouts >= 2)
        {
          LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
          MessageBeep(MB_ICONQUESTION);
//
//  Verify that's what he wants to do
//
          if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
          {
            rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
            DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
            DISCIPLINE.COMMENTSrecordID = NO_RECORD;
            DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
            DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
            memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
            DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
            DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
            DISCIPLINE.violationCategory = 4;  // Attendance
            DISCIPLINE.violationType = 1;  // Missout
            GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
            DISCIPLINE.dropoffDate = YMDO(year, month, day, 90);
            LoadString(hInst, TEXT_327, szFormatString, SZFORMATSTRING_LENGTH);
            sprintf(tempString, szFormatString, totalMissouts);
            pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
            strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
            DISCIPLINE.suspensionDate = 0;
            DISCIPLINE.suspensionDuration = 0;
            DISCIPLINE.actionTaken = NO_RECORD;
            DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
            DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
            DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
            DISCIPLINE.userID = pDAILYOPS->userID;
            rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
          }
//
//  No - he's backed off
//
          else
          {
            bRecordAbsence = FALSE;
            bCheckUnpaidTime = FALSE;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_339, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
//
//  And check the unpaid time
//
        bCheckUnpaidTime = TRUE;
      }
      break;
//
//  Unpaid
//
    case 185:  // FMLA	
    case 187:  // Furlough	
    case 188:  // Suspension	
    case 189:  // Generic Unpaid	
    case 192:  // LOA	
    case 194:  // AWOL	
    case 197:  // Union meeting	
    case 198:  // Worker's comp	
    case 200:  // Jury duty	
    case 201:  // Court - Personal	
      if(bRegister)
      {
        paidTime[0] = 0;
        unpaidTime = pDAILYOPS->DOPS.Absence.timeLost;
        sprintf(tempString, "Unpaid time: %s\n\n", chhmm(unpaidTime));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        pDAILYOPS->recordFlags |= DAILYOPS_FLAG_DONTCOUNTASUNPAID;
        bRecordAbsence = TRUE;
        bCheckUnpaidTime = FALSE;
      }
      else  // Unregister
      {
      }
      bCheckUnpaidTime = FALSE;
      break;
//
//  Paid
//
    case 184:  // Sick                      B01SIC
    case 186:  // Vacation	                B01VAC
    case 190:  // Administrative leave      B01HR
    case 191:  // Bereavement               B01BER
    case 195:  // Company meeting           B01HR
    case 196:  // Company medical appt      B01HR
    case 199:  // Accident	                B01HR
    case 202:  // Company business         	B01HR
    case 203:  // Vacation - Odd	          B01VAC
    case 204:  // Vacation - Sick pay	      B01VAC
    case 205:  // Birthday	                B01HOL
    case 206:  // Court - Company business	B01HR
      if(bRegister)
      {
        paidTime[0] = pDAILYOPS->DOPS.Absence.timeLost;
        unpaidTime = 0;
        sprintf(tempString, "Paid time: %s\n\n", chhmm(paidTime[0]));
        LoadString(hInst, TEXT_335, szarString, SZARSTRING_LENGTH);
        strcat(tempString, szarString);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
        {
          return(FALSE);
        }
        bRecordAbsence = TRUE;
      }
      else
      {
      }
      bCheckUnpaidTime = FALSE;
      break;

//
//  **********************
//  *** End GLTC Rules ***
//  **********************
//
//
//  The all-encompassing 99999 rule: Just record the absence - no associated rule
//

    case 99999:
      bRecordAbsence = TRUE;
      break;
  }
//
//  Switch on a specific rule numbers so we get the property right
//
//  Switch
//
  switch(ruleNumber)
  {
//
//  Kalamazoo rules
// 
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
//
//  Register
//
//  He wanted to proceed, so record the absence
//
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
//
//  Check the unpaid time to see if a discipline record needs to be sent
//
        if(bCheckUnpaidTime)
        {
          hSaveCursor = SetCursor(hCursorWait);
          GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
          startDate = (year - 1) * 10000 + month * 100 + day;
          endDate = pDAILYOPS->pertainsToDate;
          unpaidTimeTotal = 0;
          DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
          DAILYOPSKey1.pertainsToDate = startDate;
          DAILYOPSKey1.pertainsToTime = NO_TIME;
          DAILYOPSKey1.recordFlags = 0;
          rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
          while(rcode2 == 0 &&
                (localDAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
          {
            if(!ANegatedRecord(localDAILYOPS.recordID, 1) && !ANegatingRecord(&localDAILYOPS))
            {
              if(localDAILYOPS.pertainsToDate >= startDate &&
                    localDAILYOPS.pertainsToDate <= endDate &&
                    localDAILYOPS.DRIVERSrecordID == DRIVERS.recordID &&
                    localDAILYOPS.DOPS.Absence.reasonIndex == reasonIndex)
              {
                if(!(localDAILYOPS.recordFlags & DAILYOPS_FLAG_DONTCOUNTASUNPAID))
                {
                  unpaidTimeTotal += localDAILYOPS.DOPS.Absence.unpaidTime;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &localDAILYOPS, &DAILYOPSKey1, 1);
          }
          SetCursor(hSaveCursor);
//
//  Now that we have a total for unpaid time for the past year, see if we have to make a discipline entry
//
          LoadString(hInst, ABSENCE_014, szFormatString, SZFORMATSTRING_LENGTH);
          sprintf(tempString, szFormatString, chhmm(unpaidTimeTotal));
          MessageBeep(MB_ICONINFORMATION);
          MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
          if(unpaidTimeTotal >= (58 * 60 * 60))
          {
            LoadString(hInst, ABSENCE_001,tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONQUESTION);
            if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
              rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
              DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
              DISCIPLINE.COMMENTSrecordID = NO_RECORD;
              DISCIPLINE.DRIVERSrecordID = pDAILYOPS->DRIVERSrecordID;
              DISCIPLINE.entryDateAndTime = pDAILYOPS->entryDateAndTime;
              memset(&DISCIPLINE.unused, 0x00, DISCIPLINE_UNUSED_LENGTH);
              DISCIPLINE.dateOfOffense = pDAILYOPS->pertainsToDate;
              DISCIPLINE.timeOfOffense = pDAILYOPS->pertainsToTime;
              DISCIPLINE.violationCategory = 8;  // Attendance
              DISCIPLINE.violationType = 4;      // Exceeding unpaid time threshold
              GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
              DISCIPLINE.dropoffDate = (year + 1) * 10000 + month * 100 + day;
              LoadString(hInst, TEXT_332, szFormatString, SZFORMATSTRING_LENGTH);
              sprintf(tempString, szFormatString, chhmm(unpaidTimeTotal));
              pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
              strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
              DISCIPLINE.suspensionDate = 0;
              DISCIPLINE.suspensionDuration = 0;
              DISCIPLINE.actionTaken = NO_RECORD;
              DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
              DISCIPLINE.DAILYOPSrecordID = pDAILYOPS->recordID;
              DISCIPLINE.flags = DISCIPLINE_FLAG_ABSENCERELATED;
              DISCIPLINE.userID = pDAILYOPS->userID;
              rcode2 = btrieve(B_INSERT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
              DAILYOPSKey0.recordID = DAILYOPS.recordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
              if(rcode2 == 0)
              {
                DAILYOPS.recordFlags |= DAILYOPS_FLAG_ABSENCETODISCIPLINE;
                rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
              }
            }
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Kalamazoo rules
//
//  Bermuda rules
//
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
//
//  Register
//
//  He wanted to proceed, so record the absence
//
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;
//
//  Alexandria DASH rules
//
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 207:
    case 208:
    case 209:
    case 210:
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Alexandria DASH rules
//
//  GBTA Rules
//
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Bridgeport GBTA rules
//
//  ITP/The Rapid (GRATA) Rules
//
    case 95:  // Absent
    case 96:  // Absent - no work offered
    case 97:  // Accident
    case 98:  // Accident report
    case 99:  // Additional training
    case 100: // Unused
    case 101: // Assigned to dispatch
    case 102: // AWOL
    case 103: // Bereavement
    case 104: // Business meeting
    case 105: // Disciplinary time off
    case 106: // Disciplinary time off (points)
    case 107: // Driver paid for run/time scheduled but not worked
    case 108: // Driver trainer
    case 109: // Drug test (awaiting results)
    case 110: // Drug test (random/post-accident)
    case 111: // FMLA
    case 112: // Floating holiday
    case 113: // Goodwill light duty
    case 114: // Granted time off
    case 115: // Injury
    case 116: // Jury duty
    case 117: // Late arrival
    case 118: // Late call-in
    case 119: // Light duty
    case 120: // Military leave
    case 121: // Missing (< 2 hours)
    case 122: // Other (negative drug screen)
    case 123: // Part-time guaranteed day off
    case 124: // Personal court appearance
    case 125: // Personal day off
    case 126: // Personal leave (> 1 day)
    case 127: // Personal time off granted
    case 128: // Protect
    case 129: // Relieve driver of another bus
    case 130: // Road supervisor
    case 131: // Route timing
    case 132: // Short term disability
    case 133: // Statutory holiday
    case 134: // Union business granted
    case 135: // Vacation
    case 136: // Witness testify in court
    case 137: // Workers' compensation
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
      break;  // End of ITP/The Rapid Rules
//
//  Wood Buffalo Rules
//
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Wood Buffalo Rules
//
//  Santa Clarita Rules
//
    case 164:  // Sick (PTO)
    case 165:  // FMLA
    case 166:  // Vacation
    case 167:  // Approved PTO
    case 168:  // Suspension
    case 169:  // Medical leave
    case 170:  // Emergency PTO
    case 171:  // Bereavement
    case 172:  // 1/2 Miss-out
    case 173:  // Miss-out
    case 174:  // No call / no show
    case 175:  // Meeting
    case 176:  // City doctor
    case 177:  // Union meeting
    case 178:  // Worker's comp
    case 179:  // Accident
    case 180:  // Jury duty
    case 181:  // Tardy
    case 182:  // Injury
    case 183:  // Light duty
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Santa Clarita Rules
//
//  GLTC Rules
//
    case 184:  // Sick (PTO)
    case 185:  // FMLA
    case 186:  // Vacation
    case 187:  // Furlough
    case 188:  // Suspension
    case 189:  // Medical leave
    case 190:  // LOA - Paid
    case 191:  // Bereavement
    case 192:  // LOA - Unpaid
    case 193:  // Miss-out
    case 194:  // AWOL
    case 195:  // Meeting
    case 196:  // Medical appointment
    case 197:  // Union meeting
    case 198:  // Worker's comp
    case 199:  // Accident
    case 200:  // Jury duty
    case 201:  // Court
    case 202:  // Company business
    case 203:  // Vacation - Odd
    case 204:  // Vacation - Sick
    case 205:  // Birthday
    case 206:  // Court - Company business
    case 211:  // Request Off
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;  // End of Santa Clarita Rules
//
//  99999 - Generic "record absence" rule
//
    case 99999:
      if(bRegister)
      {
        if(bRecordAbsence)
        {
          if(paidTime[0] == NO_TIME)
          {
            paidTime[0] = 0;
          }
          if(paidTime[1] == NO_TIME)
          {
            paidTime[1] = 0;
          }
          if(unpaidTime == NO_TIME)
          {
            unpaidTime = 0;
          }
          pDAILYOPS->DOPS.Absence.paidTime[0] = paidTime[0];
          pDAILYOPS->DOPS.Absence.paidTime[1] = paidTime[1];
          pDAILYOPS->DOPS.Absence.unpaidTime = unpaidTime;
          pDAILYOPS->DOPS.Absence.timeLost = paidTime[0] + paidTime[1] + unpaidTime;
          if(paidTime[0] > 0 || paidTime[1] > 0)
          {
            pDAILYOPS->recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
          }
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, pDAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
//
//  Unregister
//
      else
      {
//
//  If a discipline record was generated, locate and delete it
//
        DISCIPLINEKey1.DRIVERSrecordID = DRIVERS.recordID;
        DISCIPLINEKey1.dateOfOffense = NO_RECORD;
        DISCIPLINEKey1.timeOfOffense = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        while(rcode2 == 0 &&
              DISCIPLINE.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(DISCIPLINE.DAILYOPSrecordID == pDAILYOPS->DAILYOPSrecordID)
          {
            btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
            LoadString(hInst, TEXT_338, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONINFORMATION);
            MessageBox(NULL, tempString, TMS, MB_ICONINFORMATION | MB_OK);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
        }
      }
      break;
//
//  Fall-through
//
    default:
      break;
  }
//
//  All done
//
  return(TRUE);
}
