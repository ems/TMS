// CalendarDate.cpp : implementation file
//

#include "stdafx.h"

int   GetDate(long *, long *, long *);
char *GetDateWithOffset(long, long, long, int);
static char szDate[16];

extern "C"{

#include "TMSHeader.h"

int CalendarDate(long *dd, long *mm, long *yyyy)
{
  return(GetDate(dd, mm, yyyy));
}

char *CalendarDateWithOffset(long dd, long mm, long yyyy, int offset)
{
  return(GetDateWithOffset(dd, mm, yyyy, offset));
}

}  // EXTERN C

#include "tms.h"
#include "dtpicker.h"
#include "PickDate.h"

int GetDate(long *dd, long *mm, long *yyyy)
{
  long date;

  while(TRUE)
  {
    date = NO_RECORD;
    CPickDate dlg(NULL, &date);

    if(dlg.DoModal() != IDOK)
    {
      return(NO_RECORD);
    }

    GetYMD(date, yyyy, mm, dd);
  
    CTime spanDate(*yyyy, *mm, *dd, 0, 0, 0);

    if(spanDate.GetDayOfWeek() == 2)
    {
      return(TRUE);
    }
    
    MessageBox(NULL, "Please select a Monday", TMS, MB_OK);
  }
}

char *GetDateWithOffset(long dd, long mm, long yyyy, int offset)
{
  CTime offsetDate(yyyy, mm, dd, 0, 0, 0);
  offsetDate += CTimeSpan(offset, 0, 0, 0);
  strcpy(szDate, offsetDate.Format(_T("%d/%m/%Y")));
  return(szDate);
}