//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2007 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Load vehicle data from an external file into the buses table
//
BOOL FAR TMSRPT69(TMSRPTPassedDataDef *pPassedData)
{
  FILE *fp;
  int   rcode2;
  char  name[64];
  char  bustype[16];
  int   nLoadType;
//
//  Give the warning
//
  MessageBeep(MB_ICONQUESTION);
  LoadString(hInst, ERROR_360, tempString, TEMPSTRING_LENGTH);
  if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
    return(FALSE);
//
//  Open the text file
//
  fp = fopen("BUSES.TXT", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open BUSES.TXT", TMS, MB_OK);
    return(FALSE);
  }
//
//  Nuke the previous BUSES file
//
  rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey0, 0);
  while(rcode2 == 0)
  {
    btrieve(B_DELETE, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey0, 0);
  }
//
//  Read from the input file and write to the btrieve file
//
  BUSES.recordID = 0L;
//
//  But first, establish the type of buses load
//
//  nLoadType = GetPrivateProfileInt("Reports", "BusesLoadType", 1, TMSINIFile);
  nLoadType = 2;

//
//  Load type 1 - Public Transport Service Corporation (Trinidad)
//
  if(nLoadType == 1)
  {
    memset(&BUSES, 0x00, sizeof(BUSES));
    BUSES.recordID = 0L;
    BUSES.COMMENTSrecordID = NO_RECORD;
    pad(BUSES.reserved, BUSES_RESERVED_LENGTH);
    BUSES.flags = 0;
    while(fscanf(fp, "%s\t%s",
          &bustype, &name) > 0)
    {
      BUSES.recordID++;
      BUSES.BUSTYPESrecordID = atol(bustype);
      if(atol(name) < 1000)
      {
        BUSES.number[0] = ' ';
        strcpy(&BUSES.number[1], name);
      }
      else
      {
        strcpy(BUSES.number, name);
      }
      pad(BUSES.number, BUSES_NUMBER_LENGTH);
      btrieve(B_INSERT, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    }  
  }
//
//  Load type 2 - Santa Clarita
//
  if(nLoadType == 2)
  {
    memset(&BUSES, 0x00, sizeof(BUSES));
    BUSES.recordID = 0L;
    BUSES.COMMENTSrecordID = NO_RECORD;
    pad(BUSES.reserved, BUSES_RESERVED_LENGTH);
    BUSES.flags = 0;
    while(fscanf(fp, "%s\t%s",
         &name, &bustype) > 0)
    {
      BUSES.recordID++;
      BUSES.BUSTYPESrecordID = atol(bustype);
      if(atol(name) < 1000)
      {
        BUSES.number[0] = ' ';
        strcpy(&BUSES.number[1], name);
      }
      else
      {
        strcpy(BUSES.number, name);
      }
      pad(BUSES.number, BUSES_NUMBER_LENGTH);
      btrieve(B_INSERT, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    }  
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);

}

