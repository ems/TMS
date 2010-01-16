//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL ReadWorkrules(HWND hWnd)
{
  char *runtypes[] = {STRAIGHT_TEXT, TWOPIECE_TEXT, MULTIPIECE_TEXT,
        SWING_TEXT, TRIPPER_TEXT, ILLEGAL_TEXT};
  char *timePeriods[] = {"AM", "MD", "PM", "OW", ""};
  char tString[64];
  char *szRostering = "Rostering";
  int  nI;
  int  nJ;
  int  nK;
  int  rcode2;
  char *token;
  long tempLong;
  FILE *fp;

//
//  Find the workrule file
//
//  First look in the database file to see if this user has his own special version.
//  Failing that, use the regular workrule file.
//
  GetPrivateProfileString(userName, "WorkruleFile",
        "", tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  strcpy(szWorkruleFile, szDirectory);
  strcat(szWorkruleFile, (strcmp(tempString, "") == 0 ? WORKRULE_FILE : tempString));
//
//  Make sure it exists
//
  if((fp = fopen(szWorkruleFile, "r")) == NULL)
  {
    sprintf(tempString, "Couldn't find parameter file \"%s\"", szWorkruleFile);
    MessageBeep(MB_ICONSTOP);
    MessageBox(hWnd, tempString, TMS, MB_ICONSTOP);
    return(FALSE);
  }
  fclose(fp);
//
//  Get the overtime calculation
//
  GetPrivateProfileString("OTCalc", "Pay", "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
  OVERTIME.payMinuteByMinute = strcmp(tempString, "Bounded") != 0;
  OVERTIME.roundedBounded = GetPrivateProfileInt("OTCalc", "RoundedBounded", 300, szWorkruleFile);
//
//  And the flags
//
  OVERTIME.flags = (char)GetPrivateProfileInt("OTCalc", "Flags", OTF_ROUNDDOWN, szWorkruleFile);
//
//  For backward compatibility, make sure a value
//  is present in the flags for the daily/spread overlap
//
  if(!(OVERTIME.flags & OTF_PAYLOWER) && !(OVERTIME.flags & OTF_PAYHIGHER))
    OVERTIME.flags |= OTF_PAYBOTH;
//
//  Get the overtime workrules for a four-day week
//
  for(nI = 0; nI < MAXSERVICES; nI++)
  {
    if(nI == 0)
    {
      strcpy(szarString, "Overtime4");
//
//  OVERTIME.weeklyAfter[WEEK4DAYS]
//
      GetPrivateProfileString(szarString, "weeklyAfter",
            "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
      OVERTIME.weeklyAfter[WEEK4DAYS] = thhmm(tempString);
//
//  OVERTIME.weeklyRate[WEEK4DAYS]
//
      GetPrivateProfileString(szarString, "weeklyRate",
            "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
      OVERTIME.weeklyRate[WEEK4DAYS] = (float)atof(tempString);
    }
    else
      sprintf(szarString, "Overtime4%d", nI);
//
//  OVERTIME.dailyAfter[WEEK4DAYS]
//
    GetPrivateProfileString(szarString, "dailyAfter",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.dailyAfter[WEEK4DAYS][nI] = thhmm(tempString);
//
//  OVERTIME.dailyRate[WEEK4DAYS]
//
    GetPrivateProfileString(szarString, "dailyRate",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.dailyRate[WEEK4DAYS][nI] = (float)atof(tempString);
  }
//
//  Get the overtime workrules for a five-day week
//
  for(nI = 0; nI < MAXSERVICES; nI++)
  {
    if(nI == 0)
    {
      strcpy(szarString, "Overtime5");
//
//  OVERTIME.weeklyAfter[WEEK5DAYS]
//
      GetPrivateProfileString(szarString, "weeklyAfter",
            "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
      OVERTIME.weeklyAfter[WEEK5DAYS] = thhmm(tempString);
//
//  OVERTIME.weeklyRate[WEEK5DAYS]
//
      GetPrivateProfileString(szarString, "weeklyRate",
            "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
      OVERTIME.weeklyRate[WEEK5DAYS] = (float)atof(tempString);
    }
    else
      sprintf(szarString, "Overtime5%d", nI);
//
//  OVERTIME.dailyAfter[WEEK5DAYS]
//
    GetPrivateProfileString(szarString, "dailyAfter",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.dailyAfter[WEEK5DAYS][nI] = thhmm(tempString);
//
//  OVERTIME.dailyRate[WEEK5DAYS]
//
    GetPrivateProfileString(szarString, "dailyRate",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.dailyRate[WEEK5DAYS][nI] = (float)atof(tempString);
  }
//
//  Get the overtime workrules for spread time
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
    GetPrivateProfileString(szarString, "spreadAfter",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.spreadAfter[nI] = thhmm(tempString);
//
//  OVERTIME.spreadRate
//
    GetPrivateProfileString(szarString, "spreadRate",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.spreadRate[nI] = (float)atof(tempString);
  }
//
//  Get the weekly guarantee times
//
    GetPrivateProfileString(szarString, "4DayWeekGuarantee",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.guarantee4DayWorkWeek = thhmm(tempString);
    GetPrivateProfileString(szarString, "5DayWeekGuarantee",
          "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
    OVERTIME.guarantee5DayWorkWeek = thhmm(tempString);

//
//  Get the runtypes that are in use, their names at this property, and their characteristics
//
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      sprintf(szarString, "%s~%d", runtypes[nI], nJ);
      GetPrivateProfileString("Runtypes", szarString,
            "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
//
//  Nuke out all of the runtype piece attributes
//
      if(nI == 5 && nJ == 29)
      {
        nK = 0;
      }
      for(nK = 0; nK < MAXPIECES; nK++)
      {
        RUNTYPE[nI][nJ].PIECE[nK].minOnTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].maxOnTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].minOffTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].maxOffTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].minPieceSize = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].desPieceSize = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].maxPieceSize = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].minBreakTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].desBreakTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].maxBreakTime = NO_TIME;
        RUNTYPE[nI][nJ].PIECE[nK].flags = 0;
      }
//
//  Does this runtype exist here?
//
      if(strcmp(tempString, "") != 0)
      {
        RUNTYPE[nI][nJ].flags |= RTFLAGS_INUSE;
//
//  RUNTYPE[][].localName
//
        strcpy(RUNTYPE[nI][nJ].localName, tempString);
//
//  RUNTYPE[][].numPieces
//
        GetPrivateProfileString(szarString, "numPieces",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].numPieces = atoi(tempString);
//
//  RUNTYPE[][].basedUpon
//
        GetPrivateProfileString(szarString, "basedUpon",
              "1", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].flags |= atoi(tempString) == 0 ? RTFLAGS_4DAY : RTFLAGS_5DAY;
//
//  RUNTYPE[][].minPlatformTime
//
        GetPrivateProfileString(szarString, "minPlatformTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].minPlatformTime = thhmm(tempString);
//
//  RUNTYPE[][].desPlatformTime
//
        GetPrivateProfileString(szarString, "desPlatformTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].desPlatformTime = thhmm(tempString);
//
//  RUNTYPE[][].maxPlatformTime
//
        GetPrivateProfileString(szarString, "maxPlatformTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].maxPlatformTime = thhmm(tempString);
//
//  RUNTYPE[][].minPayTime
//
        GetPrivateProfileString(szarString, "minPayTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].minPayTime = thhmm(tempString);
//
//  RUNTYPE[][].desPayTime
//
        GetPrivateProfileString(szarString, "desPayTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].desPayTime = thhmm(tempString);
//
//  RUNTYPE[][].maxPayTime
//
        GetPrivateProfileString(szarString, "maxPayTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].maxPayTime = thhmm(tempString);
//
//  RUNTYPE[][].minBreakOf
//
        GetPrivateProfileString(szarString, "minBreakOf",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].minBreakOf = thhmm(tempString);
//
//  RUNTYPE[][].desBreakOf
//
        GetPrivateProfileString(szarString, "desBreakOf",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].desBreakOf = thhmm(tempString);
//
//  RUNTYPE[][].maxBreakOf
//
        GetPrivateProfileString(szarString, "maxBreakOf",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].maxBreakOf = thhmm(tempString);
//
//  RUNTYPE[][].minContig
//
        GetPrivateProfileString(szarString, "minContig",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].minContig = thhmm(tempString);
//
//  RUNTYPE[][].minContig
//
        GetPrivateProfileString(szarString, "desContig",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].desContig = thhmm(tempString);
//
//  RUNTYPE[][].minContig
//
        GetPrivateProfileString(szarString, "maxContig",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].maxContig = thhmm(tempString);
//
//  RUNTYPE[][].maxSpreadTime
//
        GetPrivateProfileString(szarString, "maxSpreadTime",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].maxSpreadTime = thhmm(tempString);
//
//  RUNTYPE[][].travelCounts
//
        GetPrivateProfileString(szarString, "travelCounts",
              "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
          RUNTYPE[nI][nJ].flags |= RTFLAGS_TRAVELCOUNTS;
//
//  RUNTYPE[][].bringUpTo
//
        GetPrivateProfileString(szarString, "bringUpTo",
              "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
          RUNTYPE[nI][nJ].flags |= RTFLAGS_BRINGUPTO;
//
//  RUNTYPE[][].paySpdOT
//
        GetPrivateProfileString(szarString, "paySpdOT",
              "1", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
          RUNTYPE[nI][nJ].flags |= RTFLAGS_PAYSPDOT;
//
//  RUNTYPE[][].crewOnly
//
        GetPrivateProfileString(szarString, "crewOnly",
              "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
          RUNTYPE[nI][nJ].flags |= RTFLAGS_CREWONLY;
//
//  RUNTYPE[][].placeHolder
//
        GetPrivateProfileString(szarString, "placeHolder",
              "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
          RUNTYPE[nI][nJ].flags |= RTFLAGS_PLACEHOLDER;
//
//  RUNTYPE[][].minLay and minLayTime and minLaySpan and minLayLabel
//
        GetPrivateProfileString(szarString, "minLay",
              "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString))
        {
          RUNTYPE[nI][nJ].flags |= RTFLAGS_MINLAY;
          GetPrivateProfileString(szarString, "minLayTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].minLayTime = thhmm(tempString);
          GetPrivateProfileString(szarString, "minLaySpan",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].minLaySpan = thhmm(tempString);
          GetPrivateProfileString(szarString, "minLayNotBefore",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].minLayNotBefore = thhmm(tempString);
          GetPrivateProfileString(szarString, "minLayLabel",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].minLayLabel[0] = tempString[0];
        }
//
//  RUNTYPE[][].paidBreak
//
        GetPrivateProfileString(szarString, "paidBreak",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString) == ADDRUNTYPE_LARGESTPAID)
          RUNTYPE[nI][nJ].flags |= RTFLAGS_PAIDBREAK;
        else if(atoi(tempString) == ADDRUNTYPE_LARGESTPAIDTHROUGHWHEN)
          RUNTYPE[nI][nJ].flags |= RTFLAGS_LARGESTPAIDTHROUGHWHEN;
//
//  RUNTYPE[][].paidFrom
//
        GetPrivateProfileString(szarString, "paidFrom",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidFrom = thhmm(tempString);
//
//  RUNTYPE[][].paidTo
//
        GetPrivateProfileString(szarString, "paidTo",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidTo = thhmm(tempString);
//
//  RUNTYPE[][].paidToWhen
//
        GetPrivateProfileString(szarString, "paidToWhen",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidToWhen = thhmm(tempString);
//
//  RUNTYPE[][].paidBreakSmaller
//
        GetPrivateProfileString(szarString, "paidBreakSmaller",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        if(atoi(tempString) == ADDRUNTYPE_SMALLERPAID)
          RUNTYPE[nI][nJ].flags |= RTFLAGS_SMALLERPAID;
        else if(atoi(tempString) == ADDRUNTYPE_SMALLERPAIDTHROUGHWHEN)
          RUNTYPE[nI][nJ].flags |= RTFLAGS_SMALLERPAIDTHROUGHWHEN;
//
//  RUNTYPE[][].paidFromSmaller
//
        GetPrivateProfileString(szarString, "paidFromSmaller",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidFromSmaller = thhmm(tempString);
//
//  RUNTYPE[][].paidToSmaller
//
        GetPrivateProfileString(szarString, "paidToSmaller",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidToSmaller = thhmm(tempString);
//
//  RUNTYPE[][].paidToWhenSmaller
//
        GetPrivateProfileString(szarString, "paidToWhenSmaller",
              "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
        RUNTYPE[nI][nJ].paidToWhenSmaller = thhmm(tempString);
//
//  Loop through the pieces
//
        for(nK = 0; nK < RUNTYPE[nI][nJ].numPieces; nK++)
        {
          sprintf(szarString, "%s~%d:%d", runtypes[nI], nJ, nK + 1);
//
//  RUNTYPE[][].PIECE[].minOnTime
//
          GetPrivateProfileString(szarString, "minOnTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].minOnTime = cTime(tempString);
//
//  RUNTYPE[][].PIECE[].maxOnTime
//
          GetPrivateProfileString(szarString, "maxOnTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].maxOnTime = cTime(tempString);
//
//  RUNTYPE[][].PIECE[].minOffTime
//
          GetPrivateProfileString(szarString, "minOffTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].minOffTime = cTime(tempString);
//
//  RUNTYPE[][].PIECE[].maxOffTime
//
          GetPrivateProfileString(szarString, "maxOffTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].maxOffTime = cTime(tempString);
//
//  RUNTYPE[][].PIECE[].minPayTime
//
          GetPrivateProfileString(szarString, "minPayTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].minPayTime = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].maxPayTime
//
          GetPrivateProfileString(szarString, "maxPayTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].maxPayTime = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].minPieceSize
//
          GetPrivateProfileString(szarString, "minPieceSize",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].minPieceSize = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].desPieceSize
//
          GetPrivateProfileString(szarString, "desPieceSize",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].desPieceSize = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].maxPieceSize
//
          GetPrivateProfileString(szarString, "maxPieceSize",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].maxPieceSize = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].minBreakTime
//
          GetPrivateProfileString(szarString, "minBreakTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].minBreakTime = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].desBreakTime
//
          GetPrivateProfileString(szarString, "desBreakTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].desBreakTime = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].maxBreakTime
//
          GetPrivateProfileString(szarString, "maxBreakTime",
                "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          RUNTYPE[nI][nJ].PIECE[nK].maxBreakTime = thhmm(tempString);
//
//  RUNTYPE[][].PIECE[].flags
//
          GetPrivateProfileString(szarString, "travelIncludedInMin",
                "1", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          if(atoi(tempString))
            RUNTYPE[nI][nJ].PIECE[nK].flags |= PIECEFLAGS_TRAVELINCLUDEDINMIN;
          GetPrivateProfileString(szarString, "travelIncludedInMax",
                "1", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          if(atoi(tempString))
            RUNTYPE[nI][nJ].PIECE[nK].flags |= PIECEFLAGS_TRAVELINCLUDEDINMAX;
          GetPrivateProfileString(szarString, "payOnlyApplies",
                "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
          if(atoi(tempString))
            RUNTYPE[nI][nJ].PIECE[nK].flags |= PIECEFLAGS_PAYONLYAPPLIES;
        }
      }
    }
  }
//
//  Premiums
//
//  There can be up to MAXPREMIUMS entries in the [Premiums] section,
//  all labelled as premium0, premium1, premium2, and so on.  These
//  entries hold the section headers for the rest of the premium data.
//
   m_numPremiums = 0;
   for(nI = 0; nI < MAXPREMIUMS; nI++)
   {
     PREMIUM[nI].flags = 0L;
     sprintf(szarString, "premium%d", nI);
//
//  Get the premium entry
//
     GetPrivateProfileString("Premiums", szarString,
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, "") == 0)
       break;
//
//  PREMIUM[].localName
//
     strcpy(PREMIUM[nI].localName, tempString);
//
//  PREMIUM[].runtype
//
     GetPrivateProfileString(szarString, "runtype",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].runtype = (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].time
//
     GetPrivateProfileString(szarString, "premiumAmount",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].time = thhmm(tempString);
//
//  PREMIUM[].flags (was PREMIUM[].minutesCount)
//
     GetPrivateProfileString(szarString, "minutesCount",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_TIMECOUNTSINOT;
//
//  PREMIUM[].reportedAs
//
     GetPrivateProfileString(szarString, "reportedAs",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].reportedAs = (strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString));
//
//  PREMIUM[].formattedDataType
//
     GetPrivateProfileString(szarString, "formattedDataType",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].formattedDataType = (strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString));
//
//  If formattedDataType is missing, point the premium to the appropriate "formattedDataType"
//
     if(PREMIUM[nI].formattedDataType == NO_RECORD)
     {
       ATTRIBUTESKey0.Id = PREMIUM[nI].reportedAs;
       rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
       PREMIUM[nI].formattedDataType = (rcode2 == 0 ? ATTRIBUTES.formattedDataType : NO_RECORD);
     }
//
//  PREMIUM[].flags (was PREMIUM[].travelTimePremium)
//
     GetPrivateProfileString(szarString, "travelTimePremium",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_TRAVELTIMEPREMIUM;
//
//  PREMIUM[].flags (was PREMIUM[].bUseConnectionIfNoDynamic)
//
     GetPrivateProfileString(szarString, "UseConnectionIfNoDynamic",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
//
//  PREMIUM[].flags (was PREMIUM[].dontPayIfPayingIntervening)
//
     GetPrivateProfileString(szarString, "dontPayIfPayingIntervening",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING;
//
//  PREMIUM[].flags (RECORDONAB)
//
     GetPrivateProfileString(szarString, "recordOnAB",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_RECORDONAB;
//
//  PREMIUM[].flags (DONTTRAVELBEQC)
//
     GetPrivateProfileString(szarString, "dontTravelBEqC",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_DONTTRAVELBEQC;
//
//  PREMIUM[].flags (DONTINCORPORATE)
//
     GetPrivateProfileString(szarString, "dontIncorporate",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_DONTINCORPORATE;
//
//  PREMIUM[].flags (TRAVELBTOCWHENINTINFEASIBLE)
//
     GetPrivateProfileString(szarString, "travelBToCWhenIntInfeasible",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].flags |= PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE;
//
//  PREMIUM[].payTravelTime
//
     GetPrivateProfileString(szarString, "payTravelTime",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].payTravelTime = (strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString));
//
//  PREMIUM[].payHow
//
     GetPrivateProfileString(szarString, "payHow",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].payHow =  (strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString));
//
//  PREMIUM[].payHowPercent
//
     GetPrivateProfileString(szarString, "payHowPercent",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].payHowPercent = (float)atof(tempString);
//
//  PREMIUM[].payHowMinutes
//
     GetPrivateProfileString(szarString, "payHowMinutes",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].payHowMinutes = atol(tempString);
//
//  PREMIUM[].NASpecificRoute
//
//  This is for backward compatibility.  NASpecificRoute is
//  no longer used - routeRecID = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NASpecificRoute",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].ROUTESrecordID = NO_RECORD;
//
//  PREMIUM[].ROUTESrecordId
//
     else
     {
       GetPrivateProfileString(szarString, "routeRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].ROUTESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
     }
//
//  PREMIUM[].NASpecificService
//
//  This is for backward compatibility.  NASpecificService is
//  no longer used - serviceRecID = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NASpecificService",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].SERVICESrecordID = NO_RECORD;
//
//  PREMIUM[].SERVICESrecordID
//
     else
     {
       GetPrivateProfileString(szarString, "serviceRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].SERVICESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
     }
//
//  PREMIUM[].DIVISIONSrecordID (no backward compatibility required)
//
     GetPrivateProfileString(szarString, "divisionRecID",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].DIVISIONSrecordID =
           (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].assignedToNODESrecordID (no backward compatibility required)
//
     GetPrivateProfileString(szarString, "assignedtoNODESRecID",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].assignedToNODESrecordID =
           (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].NAWith
//
//  This is for backward compatibility.  NAWith is
//  no longer used - bustype = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NAWith",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
     {
       PREMIUM[nI].bustype = NO_RECORD;
       PREMIUM[nI].BUSTYPESrecordID = NO_RECORD;
     }
//
//  PREMIUM[].bustype
//
     else
     {
       GetPrivateProfileString(szarString, "bustype",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].bustype = atoi(tempString);
//
//  PREMIUM[].BUSTYPESDrecordID
//
       GetPrivateProfileString(szarString, "bustypeRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].BUSTYPESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
     }
//
//  PREMIUM[].NAOn
//
//  This is for backward compatibility.  NAOn is
//  no longer used - onVerb = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NAOn",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
     {
       PREMIUM[nI].onVerb = NO_RECORD;
       PREMIUM[nI].onNoun = NO_RECORD;
       PREMIUM[nI].nounNODESrecordID = NO_RECORD;
     }
//
//  PREMIUM[].onVerb
//
     else
     {
       GetPrivateProfileString(szarString, "onVerb",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].onVerb = atoi(tempString);
//
//  PREMIUM[].onNoun
//
       GetPrivateProfileString(szarString, "onNoun",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].onNoun = atoi(tempString);
//
//  PREMIUM[].nounNODESrecordID
//
       GetPrivateProfileString(szarString, "nounNodeRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].nounNODESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
     }
//
//  PREMIUM[].NAPiece
//
//  This is for backward compatibility.  NAPiece is
//  no longer used - forPieceNumber = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NAPiece",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].forPieceNumber = NO_RECORD;
//
//  PREMIUM[].forPieceNumber
//
     else
     {
       GetPrivateProfileString(szarString, "forPieceNumber",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].forPieceNumber = atoi(tempString);
     }
//
//  PREMIUM[].withPieceSize
//
     GetPrivateProfileString(szarString, "withPieceSize",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].withPieceSize = atol(tempString);
//
//  PREMIUM[].select - If not found, assume no subsets, and adjust if one is found
//
     GetPrivateProfileString(szarString, "select",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     PREMIUM[nI].select = atoi(tempString);
//
//  PREMIUM[].NAMeals
//
//  This is for backward compatibility.  NAMeals is
//  no longer used - mealsPaid = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NAMeals",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].mealsPaid = NO_RECORD;
//
//  PREMIUM[].mealsPaid
//
     else
     {
       if(PREMIUM[nI].select == 0)
         PREMIUM[nI].select = PREMIUMS_PAIDMEALS;
       GetPrivateProfileString(szarString, "mealsPaid",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].mealsPaid = strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString);
       if(PREMIUM[nI].mealsPaid == 0)
         PREMIUM[nI].mealsPaid = NO_RECORD;
     }
//
//  PREMIUM[].NATravelBetween
//
//  This is for backward compatibility.  NATravelBetween is
//  no longer used - pieceTravel = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NATravelBetween",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].pieceTravel = NO_RECORD;
//
//  PREMIUM[].pieceTravel
//
//  There is backwards compaitibility here too.
//  pieceTravel has been split out into pieceTravel and travelFrom.
//
//  28-May-2001 - travelFrom merged back into pieceTravel
//
     else
     {
       if(PREMIUM[nI].select == 0)
         PREMIUM[nI].select = PREMIUMS_PAIDTRAVEL;
       GetPrivateProfileString(szarString, "pieceTravel",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       nJ = atoi(tempString);
       if(nJ == 0)
         nJ = BCTRAVEL_NA;
       PREMIUM[nI].pieceTravel = nJ;
       GetPrivateProfileString(szarString, "travelFrom",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       if(strcmp(tempString, "") != 0)
       {
         PREMIUM[nI].pieceTravel = atoi(tempString);
         WritePrivateProfileString(szarString, "travelFrom", "", szWorkruleFile);
       }
//
//  PREMIUM[].payBtoNODESrecordID
//
       GetPrivateProfileString(szarString, "payBtoNodeRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].payBtoNODESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].travelInBreak
//
//  There is backwards compaitibility here too.
//  travelInBreak is no longer "Yes" or "No".  It is now checked or unchecked,
//  and as such, old values must be inspected and reset (if necessary) within TMS.
//
       GetPrivateProfileString(szarString, "travelInBreak",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       nJ = atoi(tempString);
       if(nJ == NO_RECORD)
         PREMIUM[nI].travelInBreak = NO_RECORD;
       else
         PREMIUM[nI].travelInBreak = nJ >= TRAVELINC_YES ? TRAVELINC_YES : TRAVELINC_NO;
//
//  PREMIUM[].payWhen
//
       GetPrivateProfileString(szarString, "payWhen",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].payWhen = atoi(tempString);
     }
//
//  PREMIUM[].NAWaitTime
//
//  This is for backward compatibility.  NAWaitTime is
//  no longer used - waitMinutes = NO_TIME is used instead
//
     GetPrivateProfileString(szarString, "NAWaitTime",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].waitMinutes = NO_TIME;
//
//  PREMIUM[].waitMinutes
//
     else
     {
       if(PREMIUM[nI].select == 0)
         PREMIUM[nI].select = PREMIUMS_WAITTIME;
       GetPrivateProfileString(szarString, "waitMinutes",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].waitMinutes = atol(tempString);
     }
//
//  PREMIUM[].NAStartEnd
//
//  This is for backward compatibility.  NAStartEnd is
//  no longer used - startEnd = NO_RECORD is used instead
//
     GetPrivateProfileString(szarString, "NAStartEnd",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       PREMIUM[nI].startEnd = NO_RECORD;
//
//  PREMIUM[].startEnd
//
     else
     {
       if(PREMIUM[nI].select == 0)
         PREMIUM[nI].select = PREMIUMS_PAIDSTARTEND;
       GetPrivateProfileString(szarString, "startEnd",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].startEnd = strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString);
       if(PREMIUM[nI].startEnd != NO_RECORD)
         PREMIUM[nI].select = PREMIUMS_PAIDSTARTEND;

//
//  PREMIUM[].startLocation
//
       GetPrivateProfileString(szarString, "startLocation",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].startLocation = atoi(tempString);
//
//  PREMIUM[].startNODESrecordID
//
       GetPrivateProfileString(szarString, "startLocationRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].startNODESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].endLocation
//
       GetPrivateProfileString(szarString, "endLocation",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].endLocation = atoi(tempString);
//
//  PREMIUM[].endNODESrecordID
//
       GetPrivateProfileString(szarString, "endLocationRecID",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].endNODESrecordID =
             (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
//
//  PREMIUM[].paidTravel
//
       GetPrivateProfileString(szarString, "paidTravel",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       PREMIUM[nI].paidTravel = atoi(tempString);
//
//  PREMIUM[].beforeTime
//
       GetPrivateProfileString(szarString, "beforeTime",
             "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
       if(strcmp(tempString, "") == 0)
         ltoa(NO_TIME, tempString, 10);
       PREMIUM[nI].beforeTime = atol(tempString);
     }
//
//  Add one to m_numPremiums and loop back
//
     m_numPremiums++;
   }
//
//  Get the current parameters
//
   strcpy(szarString, "Parameters");
   CUTPARMS.flags = 0L;
//
//  "Smart" lookahead
//
   GetPrivateProfileString(szarString, "smartLookahead",
         szFALSE, tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   if(strcmp(tempString, szTRUE) == 0)
     CUTPARMS.flags |= CUTPARMSFLAGS_SMART;
//
//  Improvements
//
   GetPrivateProfileString(szarString, "improvements",
         szTRUE, tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   if(strcmp(tempString, szTRUE) == 0)
     CUTPARMS.flags |= CUTPARMSFLAGS_IMPROVE;
//
//  Check runtypes
//
   GetPrivateProfileString(szarString, "checkRuntypes",
         szTRUE, tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   if(strcmp(tempString, szTRUE) == 0)
   {
     CUTPARMS.flags |= CUTPARMSFLAGS_CHECKRUNTYPE;
   }
//
//  "Ignore" reliefs
//
   GetPrivateProfileString(szarString, "ignoreReliefs",
         szFALSE, tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   if(strcmp(tempString, szTRUE) == 0)
     CUTPARMS.flags |= CUTPARMSFLAGS_IGNORE;
   GetPrivateProfileString(szarString, "ignoreMinutes",
         "0", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   CUTPARMS.ignoreMinutes = atoi(tempString);
//
//  Penalty flags
//
   GetPrivateProfileString(szarString, "PenaltyFlags",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   tempLong = atol(tempString);
   CUTPARMS.flags |= tempLong;
//
//  Penalized Nodes.  These are the record IDs of those nodes
//  the set out as usable, but undesirable, cut points
//
   if(!(CUTPARMS.flags & CUTPARMSFLAGS_PEN_RUNSCUTAT))
   {
     CUTPARMS.numPenalizedNodes = 0;
   }
   else
   {
     CUTPARMS.numPenalizedNodes =
           GetPrivateProfileInt(szarString, "NumberOfPenalizedNodes", 0, szWorkruleFile);
     GetPrivateProfileString(szarString, "PenalizedNodes",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(CUTPARMS.numPenalizedNodes > 0)
     {
       token = strtok(tempString, " ");
       if(token != NULL)
       {
         strcpy(tString, token);
         for(nI = 0; nI < CUTPARMS.numPenalizedNodes; nI++)
         {
           CUTPARMS.penalizedNODESrecordIDs[nI] = atol(tString);
           token = strtok(NULL, " \n");
           if(token != NULL)
             strcpy(tString, token);
           else
             break;
         }
       }
     }
   }
//
//  cutRuns radio group
//
   GetPrivateProfileString(szarString, "cutRuns",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   CUTPARMS.cutRuns = atoi(tempString);
//
//  startTime
//
   GetPrivateProfileString(szarString, "startTime",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   CUTPARMS.startTime = cTime(tempString);
//
//  endTime
//
   GetPrivateProfileString(szarString, "endTime",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   CUTPARMS.endTime = cTime(tempString);
//
//  cutDirection radio group
//
   GetPrivateProfileString(szarString, "cutDirection",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   if(strcmp(tempString, "") == 0)
     CUTPARMS.cutDirection = PARAMETERS_TMSDECIDES;
   else
     CUTPARMS.cutDirection = atoi(tempString);
//
//  minLeftover
//
   GetPrivateProfileString(szarString, "minLeftover",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   CUTPARMS.minLeftover = thhmm(tempString);
//
//  runtypes
//
   GetPrivateProfileString(szarString, "runtypes",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   memset(CUTPARMS.runtypes, FALSE, sizeof(CUTPARMS.runtypes));
   token = strtok(tempString, " ");
   while(token != 0)
   {
     tempLong = atol(token);
     nI = LOWORD(tempLong);
     nJ = HIWORD(tempLong);
     CUTPARMS.runtypes[nI][nJ] = TRUE;
     token = strtok(NULL, " ");
   }
//
//  labels
//
   GetPrivateProfileString(szarString, "labels",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   strcpy(CUTPARMS.labels, tempString);
//
//  ROSTERING STUFF
//
//  Service days
//
   for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
   {
     sprintf(szarString, "ServiceDay%d", nI);
     GetPrivateProfileString(szRostering, szarString,
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     ROSTERPARMS.serviceDays[nI] = (strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString));
   }
//
//  flags
//
//  New style (2001-03-15) - flags recorded as is.
//  If ROSTERPARMS.flags comes back as NO_RECORD,
//  keep the following for backward compatibility.
//
   ROSTERPARMS.flags = GetPrivateProfileInt(szRostering, "flags", NO_RECORD, szWorkruleFile);
   if(ROSTERPARMS.flags == NO_RECORD)
   {
     ROSTERPARMS.flags = 0;
//
//  sameRoute
//
     GetPrivateProfileString(szRostering, "sameRoute",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMEROUTE;
//
//  sameRunNumber
//
     GetPrivateProfileString(szRostering, "sameRunNumber",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMERUNNUMBER;
//
//  sameStart
//
     GetPrivateProfileString(szRostering, "sameStart",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMESTART;
//
//  sameEnd
//
     GetPrivateProfileString(szRostering, "sameEnd",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMEEND;
//
//  bidByDriverType
//
     GetPrivateProfileString(szRostering, "bidByDriverType",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     if(strcmp(tempString, szTRUE) == 0)
       ROSTERPARMS.flags |= ROSTERPARMS_FLAG_BIDBYDRIVERTYPE;
   }  // ROSTERPARMS.flags
//
//  runtypeChoice
//
   GetPrivateProfileString(szRostering, "runtypeChoice",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.runtypeChoice = (strcmp(tempString, "") == 0 ? 0 : atoi(tempString));
//
//  match
//
   memset(ROSTERPARMS.match, 0x00, sizeof(ROSTERPARMS.match));
   for(nI = 0; nI < NUMRUNTYPES; nI++)
   {
     for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
     {
       if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
       {
         ltoa(MAKELONG(nI, nJ), szarString, 10);
         GetPrivateProfileString(szRostering, szarString,
               "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
         token = strtok(tempString, " ");
         while(token != 0)
         {
           tempLong = atol(token);
           ROSTERPARMS.match[nI][nJ][HIWORD(tempLong)] |= (2 << LOWORD(tempLong));
           token = strtok(NULL, " ");
         }
       }
     }
   }
//
//  startVariance
//
   GetPrivateProfileString(szRostering, "startVariance",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.startVariance = thhmm(tempString);
//
//  minOffTime
//
   GetPrivateProfileString(szRostering, "minOffTime",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.minOffTime = thhmm(tempString);
//
//  workWeek
//
   GetPrivateProfileString(szRostering, "workWeek",
         "5", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.workWeek = atoi(tempString);
//
//  MinWork and MaxWork
//
   GetPrivateProfileString(szRostering, "minWork",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.minWork = thhmm(tempString);
   GetPrivateProfileString(szRostering, "maxWork",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.maxWork = thhmm(tempString);
//
//  maxNightWork
//
   GetPrivateProfileString(szRostering, "maxNightWork",
         "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   ROSTERPARMS.maxNightWork = (strcmp(tempString, "") == 0 ? NO_RECORD : atoi(tempString));
//
// nightWorkAfter
//
   if(ROSTERPARMS.maxNightWork == NO_RECORD)
   {
     ROSTERPARMS.nightWorkAfter = NO_TIME;
   }
   else
   {
     GetPrivateProfileString(szarString, "nightWorkAfter",
           "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
     ROSTERPARMS.nightWorkAfter = cTime(tempString);
   }
//
//  Days off patterns
//
   GetPrivateProfileString(szarString, "DaysOffPatterns", "", tempString, TEMPSTRING_LENGTH, szWorkruleFile);
   pad(tempString, 128);
   strncpy(ROSTERPARMS.daysOffPatterns, tempString, 128);
//
//  Are dynamic travels in use?
//
  bUseDynamicTravels = FALSE;
  for(nI = 0; nI < m_numPremiums; nI++)
  {
    if(bUseDynamicTravels)
      break;
    if(PREMIUM[nI].payTravelTime == WORKRULES_ASPERSCHEDULE ||
          PREMIUM[nI].payTravelTime == WORKRULES_ASPERSCHEDULEDWELL)
    {
      bUseDynamicTravels = TRUE;
      nJ = GetPrivateProfileInt((LPSTR)userName, (LPSTR)"TMSGeneratedTravelTimes",
            (int)PREFERENCES_USETRIPPLANNER, (LPSTR)szDatabaseFileName);
      bUseCISPlan = (nJ == PREFERENCES_USETRIPPLANNER);
    }
  }
//
//  Drivers to consider
//
  driversToConsider = GetPrivateProfileInt(szRostering,
        (LPSTR)"DriversToConsider", (int)0, (LPSTR)szWorkruleFile);
//
//  All done
//
  return(TRUE);
}
