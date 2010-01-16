//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2007 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Load vehicle data from an external file into the buses table
//
BOOL FAR TMSRPT82(TMSRPTPassedDataDef *pPassedData)
{
  FILE *fp;
  BOOL  bFound;
  char  badgeNumber[64], sickTime[64], vacationTime[64];
  int   nLoadType;
  int   rcode2;
//
//  Give the warning
//
  MessageBeep(MB_ICONQUESTION);
  LoadString(hInst, ERROR_372, tempString, TEMPSTRING_LENGTH);
  if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
    return(FALSE);
//
//  Open the text file
//
  strcpy(tempString, szDatabaseRoot);
  strcat(tempString, "\\Common\\banks.txt");
  fp = fopen(tempString, "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open BANKS.TXT\n\nPlease check for it in the TMSData\\Common folder", TMS, MB_OK);
    return(FALSE);
  }
//
//  Read from the input file and write to the btrieve file
//
  BUSES.recordID = 0L;
//
//  But first, establish the type of buses load
//
  nLoadType = GetPrivateProfileInt("Reports", "DriverBankLoadType", 1, TMSINIFile);
  nLoadType = 1;

//
//  Load type 1 - Alexandria Dash
//
  if(nLoadType == 1)
  {
    while(fscanf(fp, "%s\t%s\t%s", &badgeNumber, &sickTime, &vacationTime) > 0)
    {
      bFound = FALSE;
      rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      while(rcode2 == 0)
      {
        strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
        if(strcmp(badgeNumber, tempString) == 0)
        {
          DRIVERS.sickTime = (long)(atof(sickTime) * 3600);
          DRIVERS.vacationTime = (long)(atof(vacationTime) * 3600);
          rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
          bFound = TRUE;
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
      if(!bFound)
      {
        sprintf(tempString, "Driver \"%s\" not found in Drivers Table\n\nContinue?", badgeNumber);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(hWndMain, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
        {
          break;
        }
      }
    }  
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);

}

