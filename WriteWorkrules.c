//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

static char szWorkruleFile[256];

BOOL writeWorkrules(HWND hWnd)
{
  char *runtypes[] = {STRAIGHT_TEXT, TWOPIECE_TEXT, MULTIPIECE_TEXT,
        SWING_TEXT, TRIPPER_TEXT, ILLEGAL_TEXT};
  char tString[64];
  char *szRostering = "Rostering";
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
  FILE *fp;

//
//  Find the workrule file
//
//  First look in the database file to see if this user has his own special version.
//  Failing that, use the regular workrule file.
//
  GetPrivateProfileString(userName, "szWorkruleFile",
        "", tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  strcpy(szWorkruleFile, szDirectory);
  strcat(szWorkruleFile, (strcmp(tempString, "") == 0 ? WORKRULE_FILE : tempString));
//
//  Make sure it exists
//
  if((fp = fopen(szWorkruleFile, "r")) == NULL)
  {
    LoadString(hInst, ERROR_085, szarString, sizeof(szarString));
    sprintf(tempString, szarString, szWorkruleFile);
    MessageBeep(MB_ICONSTOP);
    MessageBox(hWnd, tempString, TMS, MB_ICONSTOP);
    return(FALSE);
  }
  fclose(fp);
//
//  Show the status bar
//
  StatusBarStart(hWndMain, "Verifying workrule file...");
  StatusBarText("Overtime rules...");
  StatusBar(0L, 0L);
//
//  Write the overtime calculation
//
  WritePrivateProfileString("OTCalc", "Pay",
        OVERTIME.payMinuteByMinute ? "MinuteByMinute" : "Bounded", szWorkruleFile);
  sprintf(tempString, "%ld", OVERTIME.roundedBounded);
  WritePrivateProfileString("OTCalc", "RoundedBounded", tempString, szWorkruleFile);
//
//  And the rounding
//
  sprintf(tempString, "%d", (int)OVERTIME.flags);
  WritePrivateProfileString("OTCalc", "Flags", tempString, szWorkruleFile);
//
//  Overtime workrules for a four-day week
//
  for(nI = 0; nI < MAXSERVICES; nI++)
  {
    if(nI == 0)
    {
      strcpy(szarString, "Overtime4");
//
//  OVERTIME.weeklyAfter[WEEK4DAYS]
//
      WritePrivateProfileString(szarString, "weeklyAfter",
            chhmm(OVERTIME.weeklyAfter[WEEK4DAYS]), szWorkruleFile);
//
//  OVERTIME.weeklyRate[WEEK4DAYS]
//
      gcvt(OVERTIME.weeklyRate[WEEK4DAYS], NUMDECS, tempString);
      WritePrivateProfileString(szarString, "weeklyRate", tempString, szWorkruleFile);
    }
    else
      sprintf(szarString, "Overtime4%d", nI);
//
//  OVERTIME.dailyAfter[WEEK4DAYS]
//
    WritePrivateProfileString(szarString, "dailyAfter",
          chhmm(OVERTIME.dailyAfter[WEEK4DAYS][nI]), szWorkruleFile);
//
//  OVERTIME.dailyRate[WEEK4DAYS]
//
    gcvt(OVERTIME.dailyRate[WEEK4DAYS][nI], NUMDECS, tempString);
    WritePrivateProfileString(szarString, "dailyRate", tempString, szWorkruleFile);
  }
//
//  Overtime workrules for a five-day week
//
  for(nI = 0; nI < MAXSERVICES; nI++)
  {
    if(nI == 0)
    {
      strcpy(szarString, "Overtime5");
//
//  OVERTIME.weeklyAfter[WEEK5DAYS]
//
      WritePrivateProfileString(szarString, "weeklyAfter",
            chhmm(OVERTIME.weeklyAfter[WEEK5DAYS]), szWorkruleFile);
//
//  OVERTIME.weeklyRate[WEEK5DAYS]
//
      gcvt(OVERTIME.weeklyRate[WEEK5DAYS], NUMDECS, tempString);
      WritePrivateProfileString(szarString, "weeklyRate", tempString, szWorkruleFile);
    }
    else
      sprintf(szarString, "Overtime5%d", nI);
//
//  OVERTIME.dailyAfter[WEEK5DAYS]
//
    WritePrivateProfileString(szarString, "dailyAfter",
          chhmm(OVERTIME.dailyAfter[WEEK5DAYS][nI]), szWorkruleFile);
//
//  OVERTIME.dailyRate[WEEK5DAYS]
//
    gcvt(OVERTIME.dailyRate[WEEK5DAYS][nI], NUMDECS, tempString);
    WritePrivateProfileString(szarString, "dailyRate", tempString, szWorkruleFile);
  }
//
//  Overtime workrules for a five-day week
//
  for(nI = 0; nI < MAXSERVICES; nI++)
  {
    if(nI == 0)
      strcpy(szarString, "SpreadOvertime");
    else
      sprintf(szarString, "SpreadOvertime%d", nI);
//
//  OVERTIME.spreadAfter
//
    WritePrivateProfileString(szarString, "spreadAfter",
          chhmm(OVERTIME.spreadAfter[nI]), szWorkruleFile);
//
//  OVERTIME.spreadRate
//
    gcvt(OVERTIME.spreadRate[nI], NUMDECS, tempString);
    WritePrivateProfileString(szarString, "spreadRate", tempString, szWorkruleFile);
  }
//
//  Overtime workrules for "Guarantee Time"
//
  strcpy(tempString, chhmm(OVERTIME.guarantee4DayWorkWeek));
  WritePrivateProfileString(szarString, "4DayWeekGuarantee", tempString, szWorkruleFile);
  strcpy(tempString, chhmm(OVERTIME.guarantee5DayWorkWeek));
  WritePrivateProfileString(szarString, "5DayWeekGuarantee", tempString, szWorkruleFile);
//
//  Write out the runtypes that are in use, their names
//  at this property, and their characteristics
//
  StatusBarText("Runtype data...");
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
//
//  Set up the name
//
      sprintf(szarString, "%s~%d", runtypes[nI], nJ);
//
//  If the runtype isn't in use, or isn't any more, make the entry blank
//  and make sure that the rest of the INI file gets cleaned up (out).
//
      if(!(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE))
      {
        WritePrivateProfileString("Runtypes", szarString, NULL, szWorkruleFile);
        WritePrivateProfileString(szarString, NULL, NULL, szWorkruleFile);
        for(nK = 0; nK < MAXPIECES; nK++)
        {
          sprintf(szarString, "%s~%d:%d", runtypes[nI], nJ, nK + 1);
          WritePrivateProfileString(szarString, NULL, NULL, szWorkruleFile);
        }
      }
//
//  The runtype exists
//
      else
      {
//
//  RUNTYPE[][].localName
//
        WritePrivateProfileString("Runtypes", szarString, RUNTYPE[nI][nJ].localName, szWorkruleFile);
//
//  RUNTYPE[][].numPieces
//
        itoa(RUNTYPE[nI][nJ].numPieces, tempString, 10);
        WritePrivateProfileString(szarString, "numPieces", tempString, szWorkruleFile);
//
//  RUNTYPE[][].basedUpon
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_4DAY ? 0 : 1, tempString, 10);
        WritePrivateProfileString(szarString, "basedUpon", tempString, szWorkruleFile);
//
//  RUNTYPE[][].minPlatformTime
//
        WritePrivateProfileString(szarString, "minPlatformTime",
              chhmm(RUNTYPE[nI][nJ].minPlatformTime), szWorkruleFile);
//
//  RUNTYPE[][].desPlatformTime
//
        WritePrivateProfileString(szarString, "desPlatformTime",
              chhmm(RUNTYPE[nI][nJ].desPlatformTime), szWorkruleFile);
//
//  RUNTYPE[][].maxPlatformTime
//
        WritePrivateProfileString(szarString, "maxPlatformTime",
              chhmm(RUNTYPE[nI][nJ].maxPlatformTime), szWorkruleFile);
//
//  RUNTYPE[][].minPayTime
//
        WritePrivateProfileString(szarString, "minPayTime",
              chhmm(RUNTYPE[nI][nJ].minPayTime), szWorkruleFile);
//
//  RUNTYPE[][].desPayTime
//
        WritePrivateProfileString(szarString, "desPayTime",
              chhmm(RUNTYPE[nI][nJ].desPayTime), szWorkruleFile);
//
//  RUNTYPE[][].maxPayTime
//
        WritePrivateProfileString(szarString, "maxPayTime",
              chhmm(RUNTYPE[nI][nJ].maxPayTime), szWorkruleFile);
//
//  RUNTYPE[][].minBreakOf
//
        WritePrivateProfileString(szarString, "minBreakOf",
              chhmm(RUNTYPE[nI][nJ].minBreakOf), szWorkruleFile);
//
//  RUNTYPE[][].desBreakOf
//
        WritePrivateProfileString(szarString, "desBreakOf",
              chhmm(RUNTYPE[nI][nJ].desBreakOf), szWorkruleFile);
//
//  RUNTYPE[][].maxBreakOf
//
        WritePrivateProfileString(szarString, "maxBreakOf",
              chhmm(RUNTYPE[nI][nJ].maxBreakOf), szWorkruleFile);
//
//  RUNTYPE[][].minContig
//
        WritePrivateProfileString(szarString, "minContig",
              chhmm(RUNTYPE[nI][nJ].minContig), szWorkruleFile);
//
//  RUNTYPE[][].desContig
//
        WritePrivateProfileString(szarString, "desContig",
              chhmm(RUNTYPE[nI][nJ].desContig), szWorkruleFile);
//
//  RUNTYPE[][].maxContig
//
        WritePrivateProfileString(szarString, "maxContig",
              chhmm(RUNTYPE[nI][nJ].maxContig), szWorkruleFile);
//
//  RUNTYPE[][].maxSpreadTime
//
        WritePrivateProfileString(szarString, "maxSpreadTime",
              chhmm(RUNTYPE[nI][nJ].maxSpreadTime), szWorkruleFile);
//
//  RUNTYPE[][].travelCounts
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_TRAVELCOUNTS ? 1 : 0, tempString, 10);
        WritePrivateProfileString(szarString, "travelCounts", tempString, szWorkruleFile);
//
//  RUNTYPE[][].bringUpTo
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_BRINGUPTO ? 1 : 0, tempString, 10);
        WritePrivateProfileString(szarString, "bringUpTo", tempString, szWorkruleFile);
//
//  RUNTYPE[][].paySpdOT
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_PAYSPDOT ? 1 : 0, tempString, 10);
        WritePrivateProfileString(szarString, "paySpdOT", tempString, szWorkruleFile);
//
//  RUNTYPE[][].crewOnly
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_CREWONLY ? 1:  0, tempString, 10);
        WritePrivateProfileString(szarString, "crewOnly", tempString, szWorkruleFile);
//
//  RUNTYPE[][].placeHolder
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_PLACEHOLDER ? 1:  0, tempString, 10);
        WritePrivateProfileString(szarString, "placeHolder", tempString, szWorkruleFile);
//
//  RUNTYPE[][].minLay and minLayTime and minLaySpan and minLayLabel
//
        itoa(RUNTYPE[nI][nJ].flags & RTFLAGS_MINLAY ? 1 : 0, tempString, 10);
        WritePrivateProfileString(szarString, "minLay", tempString, szWorkruleFile);
        WritePrivateProfileString(szarString, "minLayTime", chhmm(RUNTYPE[nI][nJ].minLayTime), szWorkruleFile);
        WritePrivateProfileString(szarString, "minLaySpan", chhmm(RUNTYPE[nI][nJ].minLaySpan), szWorkruleFile);
        WritePrivateProfileString(szarString, "minLayNotBefore", chhmm(RUNTYPE[nI][nJ].minLayNotBefore), szWorkruleFile);
        tempString[0] = RUNTYPE[nI][nJ].minLayLabel[0];
        tempString[1] = '\0';
        WritePrivateProfileString(szarString, "minLayLabel", tempString, szWorkruleFile);
//
//  RUNTYPE[][].paidBreak
//
        if(RUNTYPE[nI][nJ].flags & RTFLAGS_PAIDBREAK)
        {
          itoa(ADDRUNTYPE_LARGESTPAID, tempString, 10);
        }
        else if(RUNTYPE[nI][nJ].flags & RTFLAGS_LARGESTPAIDTHROUGHWHEN)
        {
          itoa(ADDRUNTYPE_LARGESTPAIDTHROUGHWHEN, tempString, 10);
        }
        else
        {
          itoa(ADDRUNTYPE_LARGESTNOTPAID, tempString, 10);
        }
        WritePrivateProfileString(szarString, "paidBreak", tempString, szWorkruleFile);
//
//  RUNTYPE[][].paidFrom
//
        WritePrivateProfileString(szarString, "paidFrom", chhmm(RUNTYPE[nI][nJ].paidFrom), szWorkruleFile);
//
//  RUNTYPE[][].paidTo
//
        WritePrivateProfileString(szarString, "paidTo",
              chhmm(RUNTYPE[nI][nJ].paidTo), szWorkruleFile);
//
//  RUNTYPE[][].paidToWhen
//
        WritePrivateProfileString(szarString, "paidToWhen",
              chhmm(RUNTYPE[nI][nJ].paidToWhen), szWorkruleFile);
//
//  RUNTYPE[][].paidBreakSmaller
//
        if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAID)
          itoa(ADDRUNTYPE_SMALLERPAID, tempString, 10);
        else if(RUNTYPE[nI][nJ].flags & RTFLAGS_SMALLERPAIDTHROUGHWHEN)
          itoa(ADDRUNTYPE_SMALLERPAIDTHROUGHWHEN, tempString, 10);
        else
          itoa(ADDRUNTYPE_SMALLERNOTPAID, tempString, 10);
        WritePrivateProfileString(szarString, "paidBreakSmaller", tempString, szWorkruleFile);
//
//  RUNTYPE[][].paidFromSmaller
//
        WritePrivateProfileString(szarString, "paidFromSmaller",
              chhmm(RUNTYPE[nI][nJ].paidFromSmaller), szWorkruleFile);
//
//  RUNTYPE[][].paidToSmaller
//
        WritePrivateProfileString(szarString, "paidToSmaller",
              chhmm(RUNTYPE[nI][nJ].paidToSmaller), szWorkruleFile);
//
//  RUNTYPE[][].paidToWhenSmaller
//
        WritePrivateProfileString(szarString, "paidToWhenSmaller",
              chhmm(RUNTYPE[nI][nJ].paidToWhenSmaller), szWorkruleFile);
//
//  Loop through the pieces
//
        for(nK = 0; nK < RUNTYPE[nI][nJ].numPieces; nK++)
        {
          sprintf(szarString, "%s~%d:%d", runtypes[nI], nJ, nK + 1);
//
//  RUNTYPE[][].PIECE[].minOnTime
//
          WritePrivateProfileString(szarString, "minOnTime",
                Tchar(RUNTYPE[nI][nJ].PIECE[nK].minOnTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].maxOnTime
//
          WritePrivateProfileString(szarString, "maxOnTime",
                Tchar(RUNTYPE[nI][nJ].PIECE[nK].maxOnTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].minOffTime
//
          WritePrivateProfileString(szarString, "minOffTime",
                Tchar(RUNTYPE[nI][nJ].PIECE[nK].minOffTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].maxOffTime
//
          WritePrivateProfileString(szarString, "maxOffTime",
                Tchar(RUNTYPE[nI][nJ].PIECE[nK].maxOffTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].minPayTime
//
          WritePrivateProfileString(szarString, "minPayTime",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].minPayTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].maxPayTime
//
          WritePrivateProfileString(szarString, "maxPayTime",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].maxPayTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].minPieceSize
//
          WritePrivateProfileString(szarString, "minPieceSize",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].minPieceSize), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].desPieceSize
//
          WritePrivateProfileString(szarString, "desPieceSize",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].desPieceSize), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].maxPieceSize
//
          WritePrivateProfileString(szarString, "maxPieceSize",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].maxPieceSize), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].minBreakTime
//
          WritePrivateProfileString(szarString, "minBreakTime",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].minBreakTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].desBreakTime
//
          WritePrivateProfileString(szarString, "desBreakTime",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].desBreakTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].maxBreakTime
//
          WritePrivateProfileString(szarString, "maxBreakTime",
                chhmm(RUNTYPE[nI][nJ].PIECE[nK].maxBreakTime), szWorkruleFile);
//
//  RUNTYPE[][].PIECE[].flags
//
          itoa(RUNTYPE[nI][nJ].PIECE[nK].flags & PIECEFLAGS_TRAVELINCLUDEDINMIN ?
                1 : 0, tempString, 10);
          WritePrivateProfileString(szarString, "travelIncludedInMin", tempString, szWorkruleFile);
          itoa(RUNTYPE[nI][nJ].PIECE[nK].flags & PIECEFLAGS_TRAVELINCLUDEDINMAX ?
                1 : 0, tempString, 10);
          WritePrivateProfileString(szarString, "travelIncludedInMax", tempString, szWorkruleFile);
          itoa(RUNTYPE[nI][nJ].PIECE[nK].flags & PIECEFLAGS_PAYONLYAPPLIES ?
                1:  0, tempString, 10);
          WritePrivateProfileString(szarString, "payOnlyApplies", tempString, szWorkruleFile);
        }
      }
    }
  }
//
//  Premiums
//
  StatusBarText("Premiums...");
  for(nI = 0; nI < m_numPremiums; nI++)
  {
    StatusBar((long)nI, (long)m_numPremiums);
    WritePremiums(nI);
  }
  StatusBar(0L, 0L);
//
//  And finally, nuke out any crap
//
  for(nI = m_numPremiums; nI < MAXPREMIUMS; nI++)
  {
    sprintf(szarString, "premium%d", nI);
    WritePrivateProfileString(szarString, NULL, NULL, szWorkruleFile);
  }
//
//  CUTPARMS
//
   StatusBarText("Runcut parameters...");
   strcpy(szarString, "Parameters");
//
//  "Smart" lookahead
//
   strcpy(tempString, ((CUTPARMS.flags & CUTPARMSFLAGS_SMART) ? szTRUE : szFALSE));
   WritePrivateProfileString(szarString, "smartLookahead", tempString, szWorkruleFile);
//
//  Improvements
//
   strcpy(tempString, ((CUTPARMS.flags & CUTPARMSFLAGS_IMPROVE) ? szTRUE : szFALSE));
   WritePrivateProfileString(szarString, "improvements", tempString, szWorkruleFile);
//
//  Check runtypes
//
   strcpy(tempString, ((CUTPARMS.flags & CUTPARMSFLAGS_CHECKRUNTYPE) ? szTRUE : szFALSE));
   WritePrivateProfileString(szarString, "checkRuntypes", tempString, szWorkruleFile);
//
//  "Ignore" relief points
//
   strcpy(tempString, ((CUTPARMS.flags & CUTPARMSFLAGS_IGNORE) ? szTRUE : szFALSE));
   WritePrivateProfileString(szarString, "ignoreReliefs", tempString, szWorkruleFile);
   sprintf(tempString, "%ld", CUTPARMS.ignoreMinutes);
   WritePrivateProfileString(szarString, "ignoreMinutes", tempString, szWorkruleFile);
//
//  Penalty flags.  These will also include the above three, but they're separated
//                  for the purposes of backward compatibility.
//
   ltoa(CUTPARMS.flags, tempString, 10);
   WritePrivateProfileString(szarString, "PenaltyFlags", tempString, szWorkruleFile);
//
//  Penalized Nodes.  These are the record IDs of those nodes
//  the set out as usable, but undesirable, cut points
//
   if(!(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT))
   {
     WritePrivateProfileString(szarString, "NumberOfPenalizedNodes", "0", szWorkruleFile);
     WritePrivateProfileString(szarString, "PenalizedNodes", "", szWorkruleFile);
   }
   else
   {
     itoa(CUTPARMS.numPenalizedNodes, tempString, 10);
     WritePrivateProfileString(szarString, "NumberOfPenalizedNodes", tempString, szWorkruleFile);
     strcpy(tempString, "");
     for(nI = 0; nI < CUTPARMS.numPenalizedNodes; nI++)
     {
       itoa(CUTPARMS.penalizedNODESrecordIDs[nI], tString, 10);
       strcat(tempString, tString);
       strcat(tempString, " ");
     }
     WritePrivateProfileString(szarString, "PenalizedNodes", tempString, szWorkruleFile);
   }
//
//  cutRuns
//
   itoa(CUTPARMS.cutRuns, tempString, 10);
   WritePrivateProfileString(szarString, "cutRuns", tempString, szWorkruleFile);
//
//  startTime
//
   WritePrivateProfileString(szarString, "startTime", Tchar(CUTPARMS.startTime), szWorkruleFile);
//
//  endTime
//
   WritePrivateProfileString(szarString, "endTime", Tchar(CUTPARMS.endTime), szWorkruleFile);
//
//  cutDirection
//
   itoa(CUTPARMS.cutDirection, tempString, 10);
   WritePrivateProfileString(szarString, "cutDirection", tempString, szWorkruleFile);
//
//  minLeftover
//
   WritePrivateProfileString(szarString, "minLeftover",
         chhmm(CUTPARMS.minLeftover), szWorkruleFile);
//
//  runtypes
//
   strcpy(tempString, "");
   for(nI = 0; nI < NUMRUNTYPES; nI++)
   {
     for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
     {
       if(CUTPARMS.runtypes[nI][nJ])
       {
         if(strcmp(tempString, "") != 0)
           strcat(tempString, " ");
         strcat(tempString, ltoa(MAKELONG(nI, nJ), tString, 10));
       }
     }
   }
   WritePrivateProfileString(szarString, "runtypes", tempString, szWorkruleFile);
//
//  labels
//
   WritePrivateProfileString(szarString, "labels", CUTPARMS.labels, szWorkruleFile);
//
//  ROSTERING STUFF
//
   StatusBarText("Rostering information...");
//
//  Service days
//
   for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
   {
     ltoa(ROSTERPARMS.serviceDays[nI], tempString, 10);
     sprintf(szarString, "ServiceDay%d", nI);
     WritePrivateProfileString(szRostering, szarString, tempString, szWorkruleFile);
   }
//
//  Flags
//
   ltoa(ROSTERPARMS.flags, tempString, 10);
   WritePrivateProfileString(szRostering, "flags", tempString, szWorkruleFile);
//
//  runtypeChoice
//
   itoa(ROSTERPARMS.runtypeChoice, tempString, 10);
   WritePrivateProfileString(szRostering, "runtypeChoice", tempString, szWorkruleFile);
//
//  match
//
//
   for(nI = 0; nI < NUMRUNTYPES; nI++)
   {
     for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
     {
       if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
       {
         strcpy(tempString, "");
         for(nK = 0; nK < NUMRUNTYPES; nK++)
         {
           for(nL = 0; nL < NUMRUNTYPESLOTS; nL++)
           {
             if(ROSTERPARMS.match[nI][nJ][nL] & (2 << nK))
             {
               strcat(tempString, ltoa(MAKELONG(nK, nL), tString, 10));
               strcat(tempString, " ");
             }
           }
         }
         ltoa(MAKELONG(nI, nJ), szarString, 10);
         WritePrivateProfileString(szRostering, szarString, tempString, szWorkruleFile);
       }
     }
   }
//
//  startVariance
//
   WritePrivateProfileString(szRostering, "startVariance",
         chhmm(ROSTERPARMS.startVariance), szWorkruleFile);
//
//  minOffTime
//
   WritePrivateProfileString(szRostering, "minOffTime",
         chhmm(ROSTERPARMS.minOffTime), szWorkruleFile);
//
//  workWeek
//
   sprintf(tempString, "%d", ROSTERPARMS.workWeek);
   WritePrivateProfileString(szRostering, "workWeek", tempString, szWorkruleFile);
//
//  Drivers to Consider
//
   strcpy(tempString, ltoa(driversToConsider, tempString, 10));
   WritePrivateProfileString(szRostering, "DriversToConsider", tempString, szWorkruleFile);
//
//  MinWork and MaxWork
//
   WritePrivateProfileString(szRostering, "minWork", chhmm(ROSTERPARMS.minWork), szWorkruleFile);
   WritePrivateProfileString(szRostering, "maxWork", chhmm(ROSTERPARMS.maxWork), szWorkruleFile);
//
//  maxNightWork
//
   itoa(ROSTERPARMS.maxNightWork, tempString, 10);
   WritePrivateProfileString(szRostering, "maxNightWork", tempString, szWorkruleFile);
//
// nightWorkAfter
//
   WritePrivateProfileString(szarString, "nightWorkAfter", Tchar(ROSTERPARMS.nightWorkAfter), szWorkruleFile);
//
//  Days off patterns
//
   strncpy(tempString, ROSTERPARMS.daysOffPatterns, 128);
   trim(tempString, 128);
   WritePrivateProfileString(szarString, "DaysOffPatterns", tempString, szWorkruleFile);
//
//  All done
//
  StatusBarEnd();
  m_bWorkrulesChanged = FALSE;
  return(TRUE);
}

void WritePremiums(int nI)
{
  sprintf(szarString, "premium%d", nI);
//
//  Output the premium entry
//
  WritePrivateProfileString("Premiums", szarString, PREMIUM[nI].localName, szWorkruleFile);
//
//  PREMIUM[].runtype
//
  ltoa(PREMIUM[nI].runtype, tempString, 10);
  WritePrivateProfileString(szarString, "runtype", tempString, szWorkruleFile);
//
//  PREMIUM[].time
//
  strcpy(tempString, chhmm(PREMIUM[nI].time));
  WritePrivateProfileString(szarString, "premiumAmount", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (was PREMIUM[].minutesCount)
//
  strcpy(tempString, ((PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "minutesCount", tempString, szWorkruleFile);
//
//  PREMIUM[].reportedAs
//
  itoa(PREMIUM[nI].reportedAs, tempString, 10);
  WritePrivateProfileString(szarString, "reportedAs", tempString, szWorkruleFile);
//
//  PREMIUM[].formattedDataType
//
  itoa(PREMIUM[nI].formattedDataType, tempString, 10);
  WritePrivateProfileString(szarString, "formattedDataType", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (was PREMIUM[].travelTimePremium)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_TRAVELTIMEPREMIUM) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "travelTimePremium", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (was PREMIUM[].bUseConnectionIfNoDynamic)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "UseConnectionIfNoDynamic", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (was PREMIUM[].dontPayIfPayingIntervening)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "dontPayIfPayingIntervening", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (PREMIUMFLAGS_RECORDONAB)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_RECORDONAB) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "recordOnAB", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (PREMIUMFLAGS_DONTTRAVELBEQC)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_DONTTRAVELBEQC) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "dontTravelBEqC", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (PREMIUMFLAGS_DONTINCORPORATE)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_DONTINCORPORATE) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "dontIncorporate", tempString, szWorkruleFile);
//
//  PREMIUM[].flags (PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE)
//
  strcpy(tempString,
        ((PREMIUM[nI].flags & PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE) ? szTRUE : szFALSE));
  WritePrivateProfileString(szarString, "travelBToCWhenIntInfeasible", tempString, szWorkruleFile);
//
//  PREMIUM[].payTravelTime
//
  itoa(PREMIUM[nI].payTravelTime, tempString, 10);
  WritePrivateProfileString(szarString, "payTravelTime", tempString, szWorkruleFile);
//
//  PREMIUM[].payHow
//
  itoa(PREMIUM[nI].payHow, tempString, 10);
  WritePrivateProfileString(szarString, "payHow", tempString, szWorkruleFile);
//
//  PREMIUM[].payHowPercent
//
  gcvt(PREMIUM[nI].payHowPercent, NUMDECS, tempString);
  WritePrivateProfileString(szarString, "payHowPercent", tempString, szWorkruleFile);
//
//  PREMIUM[].payHowMinutes
//
  ltoa(PREMIUM[nI].payHowMinutes, tempString, 10);
  WritePrivateProfileString(szarString, "payHowMinutes", tempString, szWorkruleFile);
//
//  PREMIUM[].ROUTESrecordId
//
  strcpy(tempString, ltoa(PREMIUM[nI].ROUTESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "routeRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].SERVICESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].SERVICESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "serviceRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].DIVISIONSrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].DIVISIONSrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "divisionRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].assignedToNODESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].assignedToNODESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "assignedToNODESRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].bustype
//
  strcpy(tempString, ltoa(PREMIUM[nI].bustype, tempString, 10));
  WritePrivateProfileString(szarString, "bustype", tempString, szWorkruleFile);
//
//  PREMIUM[].BUSTYPErecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].BUSTYPESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "bustypeRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].mealsPaid
//
  strcpy(tempString, ltoa(PREMIUM[nI].mealsPaid, tempString, 10));
  WritePrivateProfileString(szarString, "mealsPaid", tempString, szWorkruleFile);
//
//  PREMIUM[].onVerb
//
  strcpy(tempString, itoa(PREMIUM[nI].onVerb, tempString, 10));
  WritePrivateProfileString(szarString, "onVerb", tempString, szWorkruleFile);
//
//  PREMIUM[].onNoun
//
  strcpy(tempString, itoa(PREMIUM[nI].onNoun, tempString, 10));
  WritePrivateProfileString(szarString, "onNoun", tempString, szWorkruleFile);
//
//  PREMIUM[].nounNODESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].nounNODESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "nounNodeRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].forPieceNumber
//
  strcpy(tempString, itoa(PREMIUM[nI].forPieceNumber, tempString, 10));
  WritePrivateProfileString(szarString, "forPieceNumber", tempString, szWorkruleFile);
//
//  PREMIUM[].withPieceSize
//
  strcpy(tempString, ltoa(PREMIUM[nI].withPieceSize, tempString, 10));
  WritePrivateProfileString(szarString, "withPieceSize", tempString, szWorkruleFile);
//
//  PREMIUM[nI].select
//
  strcpy(tempString, itoa(PREMIUM[nI].select, tempString, 10));
  WritePrivateProfileString(szarString, "select", tempString, szWorkruleFile);
//
//  PREMIUM[].pieceTravel
//
  strcpy(tempString, itoa(PREMIUM[nI].pieceTravel, tempString, 10));
  WritePrivateProfileString(szarString, "pieceTravel", tempString, szWorkruleFile);
//
//  PREMIUM[].payBtoNODESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].payBtoNODESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "payBtoNodeRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].travelInBreak
//
  strcpy(tempString, itoa(PREMIUM[nI].travelInBreak, tempString, 10));
  WritePrivateProfileString(szarString, "travelInBreak", tempString, szWorkruleFile);
//
//  PREMIUM[].payWhen
//
  strcpy(tempString, itoa(PREMIUM[nI].payWhen, tempString, 10));
  WritePrivateProfileString(szarString, "payWhen", tempString, szWorkruleFile);
//
//  PREMIUM[].waitMinutes
//
  strcpy(tempString, ltoa(PREMIUM[nI].waitMinutes, tempString, 10));
  WritePrivateProfileString(szarString, "waitMinutes", tempString, szWorkruleFile);
//
//  PREMIUM[].paidTravel
//
  strcpy(tempString, itoa(PREMIUM[nI].paidTravel, tempString, 10));
  WritePrivateProfileString(szarString, "paidTravel", tempString, szWorkruleFile);
//
//  PREMIUM[].startEnd
//
  strcpy(tempString, itoa(PREMIUM[nI].startEnd, tempString, 10));
  WritePrivateProfileString(szarString, "startEnd", tempString, szWorkruleFile);
//
//  PREMIUM[].startLocation
//
  strcpy(tempString, ltoa(PREMIUM[nI].startLocation, tempString, 10));
  WritePrivateProfileString(szarString, "startLocation", tempString, szWorkruleFile);
//
//  PREMIUM[].startNODESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].startNODESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "startLocationRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].endLocation
//
  strcpy(tempString, ltoa(PREMIUM[nI].endLocation, tempString, 10));
  WritePrivateProfileString(szarString, "endLocation", tempString, szWorkruleFile);
//
//  PREMIUM[].endNODESrecordID
//
  strcpy(tempString, ltoa(PREMIUM[nI].endNODESrecordID, tempString, 10));
  WritePrivateProfileString(szarString, "endLocationRecID", tempString, szWorkruleFile);
//
//  PREMIUM[].beforeTime
//
  strcpy(tempString, ltoa(PREMIUM[nI].beforeTime, tempString, 10));
  WritePrivateProfileString(szarString, "beforeTime", tempString, szWorkruleFile);
//
//  Delete any old-style "NA" entries
//
  WritePrivateProfileString(szarString, "NASpecificRoute", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NASpecificService", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAWith", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAOn", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAPiece", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAStartEnd", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAMeals", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NATravelBetween", NULL, szWorkruleFile);
  WritePrivateProfileString(szarString, "NAWaitTime", NULL, szWorkruleFile);
}

