//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

//
//  From a group of radio buttons, this function checks one of them, and unchecks the rest.
//

int nCwCheckRadioButtons(HWND hWnd, int *RGArray, int nIDButton, int numButtons)
{
  int nI;
  int nJ = 0;

//
// Turn all the Radiobuttons OFF and the nIDButton button ON
//

  for(nI = 0; nI < numButtons; nI++, RGArray++)
  {
    if((*RGArray) == nIDButton)
    {
      CheckDlgButton(hWnd, nIDButton, TRUE);
      nJ = nI;
    }
    else
      CheckDlgButton(hWnd, (*RGArray), FALSE);
  }

  return(nJ);
}

void DoTitle(char *title)
{
  strcpy(titleString, title);
  strcat(titleString, m_RouteNumber);
  strcat(titleString, " - ");
  strcat(titleString, m_RouteName);
  strcat(titleString, ", ");
  strcat(titleString, m_ServiceName);
  return;
}

static char returnString[36];

char *floatString(long a, long b, BOOL percent)
{
  float number;

  if(b == 0)
    strcpy(returnString, "---");
  else
  {
    number = ((float)a / (float)b) * (percent ? 100 : 1);
    sprintf(returnString, "%9.4f", number);
  }

  return(&returnString[0]);
}

void pad(char *string, int stringLength)
{
  int  nI;

  for(nI = 0; nI < stringLength; nI++)
  {
    if(string[nI] == '\0')
    {
      memset(&string[nI], ' ', stringLength - nI);
      break;
    }
  }

  return;
}

void trim(char *string, int stringLength)
{
  int  nI;

  for(nI = stringLength - 1; nI >= 0; nI--)
  {
    if(string[nI] != ' ')
    {
      string[nI + 1] = '\0';
      break;
    }
  }
  if(nI == -1)
    string[0] = '\0';

  return;
}

char *chhmmlb( long t ) // leading blanks
{
  static char szTime[9];

  if(t == NO_TIME)
    strcpy(szTime,"");
  else
  {
    if(t >= 0)
      sprintf( szTime, "%4ld:%02ld", t / 3600, (t % 3600) / 60 );
    else
    {
      t = -t;
      sprintf( szTime, "-%4ld:%02ld", t / 3600, (t % 3600) / 60 );
    }
    if(t % 60 != 0)
      strcat(szTime, "+");
  }

  return(&szTime[0]);
}


char *chhmm( long t )
{
  static char szTime[9];
  double decimalTime;
  int    hour, min;

  if(t == NO_TIME)
    strcpy(szTime,"");
  else
  {
    if(timeHHMMFormat == PREFERENCES_HHMM)
    {
      if(t >= 0)
        sprintf( szTime, "%ld:%02ld", t / 3600, (t % 3600) / 60 );
      else
      {
        t = -t;
        sprintf( szTime, "-%ld:%02ld", t / 3600, (t % 3600) / 60 );
      }
      if(t % 60 != 0)
      {
        strcat(szTime, "+");
      }
    }
    else
    {
      if(t < 0)
      {
        t += 86400L;
      }
      hour = t / 3600;
      min = (t % 3600) / 60;
      decimalTime = (double)(hour + ((double)min / 60));
      sprintf(szTime, "%5.2f", decimalTime);
    }
  }

  return(&szTime[0]);
}

char *chhmmnc(long t)
{
    static char        szTime[8];

    strcpy(szTime,"");
    if(t != NO_TIME)
      sprintf( szTime, "%2ld%02ld", t/3600, (t % 3600)/60 );
    return(&szTime[0]);
}

long cTime( char *szTimeIn )
{
#ifdef FIXLATER
    char  szHour[3], szMinute[3], szTime[6];
    long  hour, minute;

    if(strcmp(szTimeIn, "") == 0)
      return((long)NO_TIME);
//
//  For APX, normalize the length of the time string to hhmmQ
//  For Military, normalize the length of the time string to hhmm
//
    if(timeFormat == PREFERENCES_APX)
    {
      strcpy(szTime, "0000A");
      strcpy(&szTime[5-strlen(szTimeIn)], szTimeIn);
    }
    else
    {
      strcpy(szTime, "0000");
      strcpy(&szTime[4-strlen(szTimeIn)], szTimeIn);
    }
    strncpy(szHour, szTime, 2);
    szHour[2] = '\0';
    strncpy(szMinute, &szTime[2], 2);
    szMinute[2] = '\0';

    hour = atoi(szHour);
    minute = atoi(szMinute);
    if(timeFormat == PREFERENCES_APX)
    {
      switch( szTime[4] )
        {
          case 'A':
            if(hour == 12)
              hour-=12;
              break;

          case 'P':
            if(hour != 12)
              hour += 12;
            break;

          case 'X':
            hour += (hour == 12 ? 12 : 24);
            break;

      }
    }
#else
	long	hour, minute;
	int		remaining;
	char	APX, *p;

	if( !szTimeIn )
		return (long)NO_TIME;

	// Trim leading spaces.
	while( *szTimeIn && isspace(*szTimeIn) )
		++szTimeIn;

    if( !*szTimeIn )
      return (long)NO_TIME;

	// Skip trailing spaces.
	p = &szTimeIn[strlen(szTimeIn)-1];
	while( p >= szTimeIn && isspace(*p) )
		--p;
    if( p < szTimeIn )
      return (long)NO_TIME ;

	// Collect the APX character if present.
	if( !isdigit(*p) )
		APX = *p--;
	else
		APX = 'Q';

	// Extract the numbers from the string.
	// We rely on the fallthrough of the switch to do the work.
	remaining = p - szTimeIn + 1;
	p = szTimeIn;
	hour = 0;
	minute = 0;
	switch( remaining )
	{
	case 4:	hour = ((*p++) - '0') * 10;			// hhmm
	case 3: hour += (*p++) - '0';				//  hmm
	case 2: minute = ((*p++) - '0') * 10;		//   mm
	case 1: minute += (*p++) - '0';				//    m
	}

	if( timeFormat == PREFERENCES_APX )
	{
		switch( APX )
		{
		case 'A':
			if( hour == 12 )
				hour -= 12;
			break;
		case 'P':
			if( hour != 12 )
				hour += 12;
			break;
		case 'X':
			hour += (hour == 12 ? 12 : 24);
			break;
		}
	}
#endif

    return hour * 3600 + minute * 60;
}

char *Tchar(long time)
{
  static char szTime[64];
  long   hour, min;
  int    am, nextDay = FALSE;

  if(time == NO_TIME)
  {
    strcpy(szTime, "");
  }
  else
  {
    if(time < 0)
    {
      time += 86400L;
    }
    hour = time / 3600;
    if(hour >= 24)
    {
      nextDay = TRUE;
    }
    hour %= 24;
    min = (time % 3600) / 60;
    if(timeFormat == PREFERENCES_APX)
    {
      if(hour < 12)
      {
        am = TRUE;
      }
      else
      {
        if(hour != 12)
        {
          hour -= 12;
        }
        am = FALSE;
      }
      if(hour == 0)
      {
        hour = 12;
      }
      sprintf(szTime, "%2ld%02ld%c", hour, min, nextDay ? 'X' : am ? 'A' : 'P' );
    }
    else
    {
      if(nextDay && !m_bWrapTime)
      {
        hour += 24;
      }
      sprintf(szTime, "%02ld%02ld", hour, min);
    }
    if(time % 60 != 0)
    {
      strcat(szTime, "+");
    }
  }
  return &szTime[0];
}

char *TcharAMPMAM(long time)
{
  static char szTime[64];
  long   hour, min;
  int    am, nextDay = FALSE;

  if(time == NO_TIME)
    strcpy(szTime, "--");
  else
  {
    if(time < 0)
      time += 86400L;
    hour = time / 3600;
    if(hour >= 24)
      nextDay = TRUE;
    hour %= 24;
    min = (time % 3600) / 60;
    if(timeFormat == PREFERENCES_APX)
    {
      if(hour < 12)
        am = TRUE;
      else
      {
        if(hour != 12)
          hour -= 12;
        am = FALSE;
      }
      if(hour == 0)
        hour = 12;
      sprintf(szTime, "%2ld:%02ld %s", hour, min, nextDay ? "AM" : am ? "AM" : "PM" );
    }
    else
    {
      if(nextDay && !m_bWrapTime)
        hour += 24;
      sprintf(szTime, "%02ld%02ld", hour, min);
    }
  }
  return &szTime[0];
}

char *TcharNAP(long time, BOOL bUseColons)
{
  static char szTime[64];
  long   hour, min;
  int    nextDay = FALSE;
  char   *colon = bUseColons ? ":" : "";

  if(time == NO_TIME)
    szTime[0] = 0;
  else
  {
    if(time < 0)
      time += 86400L;
    hour = time / 3600;
    if(hour >= 24)
      nextDay = TRUE;
    hour %= 24;
    min = (time % 3600) / 60;
    if(timeFormat == PREFERENCES_APX)
    {
      if(hour > 12)
        hour -= 12;
      if(hour == 0)
        hour = 12;
      sprintf(szTime, "%2ld%s%02ld", hour, colon, min);
    }
    else
    {
      if(nextDay && !m_bWrapTime)
        hour += 24;
      sprintf(szTime, "%02ld%s%02ld", hour, colon, min);
    }
  }
  return &szTime[0];
}

long thhmm( char *szTimeIn )
{
//#ifdef FIXLATER
    char  *ptr;
    BOOL  bGotPeriod;
    long  hour, minute, seconds;
    float decimalMinutes, decimalTime;
  	register char	*p;
	  char	colon;

    if(strcmp(szTimeIn, "") == 0)
    {
      return((long)NO_TIME);
    }
//
//  Check for a period in the string
//
    ptr = szTimeIn;
    bGotPeriod = FALSE;
    while(*ptr != '\0')
    {
      if(*ptr == '.')
      {
        bGotPeriod = TRUE;
        break;
      }
      ptr++;
    }
//
//  Process accordingly
//
//  hh.decimal
//
    if(bGotPeriod)
    {
      decimalTime = (float)atof(szTimeIn);
      hour = (int)decimalTime;
      decimalMinutes = decimalTime - hour;
      return hour * 3600 + (long)((decimalMinutes * 60) * 60);
    }
//
//  hh:mm
//
    else
    {
/* 
      strcpy(szTime, "00:00");
      strcpy(&szTime[5 - strlen(szTimeIn)], szTimeIn);
      strncpy(szHour, szTime, 2);
      szHour[2] = '\0';
      strncpy(szMinute, &szTime[3], 2);
      szMinute[2] = '\0';
      hour = atoi(szHour);
      minute = atoi(szMinute);
      return hour * 3600 + minute * 60;
*/
    	if( !szTimeIn )
	    	return (long)NO_TIME;

	// Trim leading spaces.
    	while( *szTimeIn && isspace(*szTimeIn) )
    		++szTimeIn;

    	if( !*szTimeIn )
    		return (long)NO_TIME;

    	for( p = szTimeIn; *p; ++p )
		    if( *p == ':' )
    			break;
    	colon = *p;
    	p = szTimeIn;
    	hour = 0;
    	minute = 0;
    	if( colon == ':' )
    	{
    		for( ; *p != ':'; ++p )
		    	hour = hour * 10 + (*p - '0');
    		++p;	// Skip the colon.
    	}
      seconds = 0;
    	for( ; *p; ++p )
      {
        if(*p == '+')
        {
          seconds = 30;
        }
        else
        {
  		    minute = minute * 10 + (*p - '0');
        }
      }
      return hour * 3600 + minute * 60 + seconds;
    }

//#else
/*
	long	hour, minute;
	register char	*p;
	char	colon;

	if( !szTimeIn )
		return (long)NO_TIME;

	// Trim leading spaces.
	while( *szTimeIn && isspace(*szTimeIn) )
		++szTimeIn;

	if( !*szTimeIn )
		return (long)NO_TIME;

	for( p = szTimeIn; *p; ++p )
		if( *p == ':' )
			break;
	colon = *p;
	p = szTimeIn;
	hour = 0;
	minute = 0;
	if( colon == ':' )
	{
		for( ; *p != ':'; ++p )
			hour = hour * 10 + (*p - '0');
		++p;	// Skip the colon.
	}
	for( ; *p; ++p )
		minute = minute * 10 + (*p - '0');
  return hour * 3600 + minute * 60;
#endif
*/

}

char *PhoneString(long areaCode, long number)
{
  static char s[32];
  long firstThree;
  long lastFour;

  firstThree = number / 10000;
  lastFour = number - (firstThree * 10000);
  sprintf(s, "(%03ld) %03ld-%04ld", areaCode, firstThree, lastFour);

  return(&s[0]);
}

HGLOBAL TMSGlobalFree(HGLOBAL hMem)
{
  if(hMem == NULL)
    return(NULL);
  else
    return(GlobalFree(hMem));
}

char *Tdec(long time)
{
  static char szTime[64];
  double decimalTime;
  int    hour, min;

  if(time == NO_TIME)
    strcpy(szTime, "");
  else
  {
    if(time < 0)
    {
      time += 86400L;
    }
    hour = time / 3600;
    min = (time % 3600) / 60;
    decimalTime = (double)(hour + ((double)min / 60));
    sprintf(szTime, "%5.2f", decimalTime);
  }
  return &szTime[0];
}

void GetYMD(long input, long *year, long *month, long *day)
{
  *year = input / 10000;
  *month = (input - *year * 10000L) / 100;
  *day = (input - *year * 10000L - *month * 100L);
}

void GetHMSBizarre(long input, long *hours, long *minutes, long *seconds)
{
  *hours = input / 10000;
  *minutes = (input - *hours * 10000) / 100;
  *seconds = input - *hours * 10000 - *minutes * 100;
}

void GetHMS(long input, long *hours, long *minutes, long *seconds)
{
  *hours = input / 3600;
  *minutes = (input % 3600) / 60;
  *seconds = input - *hours * 3600 - *minutes * 60;
}

time_t TMSmktime(long date, long time)
{
  struct tm DateTime;
  long yyyy, mm, dd;
  long hour, min, sec;

  GetYMD(date, &yyyy, &mm, &dd);

  hour = time / 3600;
  min = (time % 3600) / 60;
  sec = time - ((hour * 3600) + (min * 60));

  DateTime.tm_sec = sec;
  DateTime.tm_min = min;
  DateTime.tm_hour = hour;
  DateTime.tm_mday = dd;
  DateTime.tm_mon = mm - 1;
  DateTime.tm_year = yyyy - 1900;

  return(mktime(&DateTime));
}

char *BuildOperatorString(long DRIVERSrecordID)
{
  static char retString[DRIVERS_BADGENUMBER_LENGTH + DRIVERS_LASTNAME_LENGTH + DRIVERS_FIRSTNAME_LENGTH + 6];
  char szString[DRIVERS_BADGENUMBER_LENGTH + DRIVERS_LASTNAME_LENGTH + DRIVERS_FIRSTNAME_LENGTH + 6];
  char *ptr;
  int  rcode2;

  DRIVERSKey0.recordID = DRIVERSrecordID;
//
//  Build up the operator's name
//
  if(DRIVERSrecordID != NO_RECORD)  // Record already read if NO_RECORD
  {
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    if(rcode2 != 0)
    {
      return("Driver not found");
    }
  }
  strncpy(szString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  trim(szString, DRIVERS_LASTNAME_LENGTH);
  strcpy(retString, szString);
  strcat(retString, ", ");
  strncpy(szString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  trim(szString, DRIVERS_FIRSTNAME_LENGTH);
  strcat(retString, szString);
  strncpy(szString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
  trim(szString, DRIVERS_BADGENUMBER_LENGTH);
  ptr = szString;
//
//  Clear any leading blanks from the badge number
//
  while(*ptr)
  {
    if(*ptr == ' ')
    {
      ptr++;
    }
    else
    {
      break;
    }
  }
  if(strcmp(ptr, "") != 0)
  {
    strcat(retString, " (");
    strcat(retString, ptr);
    strcat(retString, ")");
  }

  return(retString);
}

BOOL TMSHeapFree(void *pMemory)
{
  HANDLE hProcessHeap;

  if(pMemory != NULL)
  {
    hProcessHeap = GetProcessHeap();
    HeapFree(hProcessHeap, 0, pMemory);
    HeapCompact(hProcessHeap, 0);
    pMemory = NULL;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

BOOL VerifyUserAccessRights(long ThisType)
{
  if(!(m_UserAccessRights & (1L << ThisType)))
  {
    MessageBeep(MB_ICONSTOP);
    return(FALSE);
  }

  return(TRUE);
}

BOOL IsNumber(char *pStr)
{
  int nI;

  for(nI = 0; nI < (int)strlen(pStr); nI++)
  {
    if(pStr[nI] < '0' || pStr[nI] > '9')
    {
      return(FALSE);
    }
  }

  return(TRUE);
}
