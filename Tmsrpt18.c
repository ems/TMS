//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Load driver data from an external file into the drivers table
//
BOOL FAR TMSRPT18(TMSRPTPassedDataDef *pPassedData)
{
  FILE *fp;
  char  inputString[512];
  char *pch;
  int   rcode2;
  char  lname[64], fname[64], inits[64];
  char  dob[16];
  char  expiryDate[16];
  long  badge, sdate, hdate;
  long  sdatePrev, ssort;
  long  dtype;
  int   nLoadType;
  float vacationHours, sickHours;
  long  badgeNumbers[500], DRIVERSrecordIDs[500];
  int   numDrivers;
  BOOL  bFound;
  int   nI;
  long  recordID;
  long  seniorityDate;
  int   senioritySort;
  long  driverType;
//
//  Establish the type of deadhead load
//
  nLoadType = GetPrivateProfileInt("Reports", "DriverLoadType", 1, TMSINIFile);
  nLoadType = 19;
  if(nLoadType == 14)
  {
    fp = fopen("Driver Hours.txt", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Driver Hours.txt", TMS, MB_OK);
      return(FALSE);
    }
  }
  else
  {
//
//  Give the warning
//
    MessageBeep(MB_ICONQUESTION);
    LoadString(hInst, ERROR_238, tempString, TEMPSTRING_LENGTH);
    if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
    {
      return(FALSE);
    }
//
//  Open the text file
//
    fp = fopen("DRIVERS.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DRIVERS.TXT", TMS, MB_OK);
      return(FALSE);
    }
//
//  Nuke the previous DRIVERS file
//
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    while(rcode2 == 0)
    {
      btrieve(B_DELETE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
//
//  Read from the input file and write to the btrieve file
//
    DRIVERS.recordID = 0L;
  }
//
//  Load type 1 - GRATA
//
  if(nLoadType == 1)
  {
    while(fscanf(fp, "%218c", inputString) > 0)  //218 'cause of EOL
    {
      strncpy(DRIVERS.lastName, &inputString[0], DRIVERS_LASTNAME_LENGTH);
      strncpy(DRIVERS.firstName, &inputString[32], DRIVERS_FIRSTNAME_LENGTH);
      strncpy(DRIVERS.initials, &inputString[64], DRIVERS_INITIALS_LENGTH);
      strncpy(DRIVERS.badgeNumber, &inputString[68], DRIVERS_BADGENUMBER_LENGTH);
      strncpy(tempString, &inputString[84], 5);
      tempString[5] = '\0';
      DRIVERS.crewNumber = atol(tempString);
      strncpy(DRIVERS.streetAddress, &inputString[89], DRIVERS_STREETADDRESS_LENGTH);
      strncpy(DRIVERS.city, &inputString[121], DRIVERS_CITY_LENGTH); 
      strncpy(DRIVERS.province, &inputString[153], DRIVERS_PROVINCE_LENGTH);
      strncpy(DRIVERS.postalCode, &inputString[155], DRIVERS_POSTALCODE_LENGTH);
      strncpy(tempString, &inputString[165], 4);
      tempString[4] = '\0';
      DRIVERS.telephoneArea = atol(tempString);
      strncpy(tempString, &inputString[169], 8);
      tempString[8] = '\0';
      DRIVERS.telephoneNumber = atol(tempString);
      strncpy(tempString, &inputString[177], 9);
      tempString[9] = '\0';
      DRIVERS.hireDate = atol(tempString);
      strncpy(tempString, &inputString[186], 9);
      tempString[9] = '\0';
      DRIVERS.seniorityDate = atol(tempString);
      strncpy(tempString, &inputString[195], 3);
      tempString[3] = '\0';
      DRIVERS.senioritySort = (short int)atoi(tempString);
      strncpy(tempString, &inputString[198], 3);
      tempString[3] = '\0';
      DRIVERS.vacationTime = (short int)atoi(tempString);
      strncpy(DRIVERS.SIN, &inputString[201], DRIVERS_SIN_LENGTH);
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 2 - Brampton transit
//
  else if(nLoadType == 2)
  {
    memset(&DRIVERS, ' ', sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    while(fscanf(fp, "%57c", inputString) > 0)  //57 'cause of EOL
    {
      strncpy(DRIVERS.lastName, &inputString[0], 32);
      strncpy(DRIVERS.badgeNumber, &inputString[32], 3);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strncpy(tempString, &inputString[36], 9);
      tempString[9] = '\0';
      DRIVERS.hireDate = atol(tempString);
      strncpy(tempString, &inputString[45], 9);
      tempString[9] = '\0';
      DRIVERS.seniorityDate = atol(tempString);
      strncpy(tempString, &inputString[54], 3);
      tempString[3] = '\0';
      DRIVERS.senioritySort = (short int)atoi(tempString);
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 3 - ATC/Vancom Denver
//
  else if(nLoadType == 3)
  {
    memset(&DRIVERS, ' ', sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fscanf(fp, "%s\t%s\t%ld\t%ld", &lname, &fname, &badge, &sdate) > 0)
    {
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      ltoa(badge, tempString, 10);
      strcpy(DRIVERS.badgeNumber, tempString);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.hireDate = sdate;
      DRIVERS.seniorityDate = sdate;
      if(sdate != sdatePrev)
      {
        ssort = 1;
        sdatePrev = sdate;
      }
      else
      {
        ssort++;
      }
      DRIVERS.senioritySort = (short int)ssort;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 4 - Alexandria DASH
//
  else if(nLoadType == 4)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fgets(inputString, sizeof(inputString), fp))
    {
      strcpy(tempString, strtok(inputString, "\n\t"));
      strcpy(DRIVERS.badgeNumber, tempString);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);

      strcpy(lname, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      strcpy(fname, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      strcpy(tempString, strtok(NULL, "\n\t"));
      DRIVERS.hireDate = atol(tempString);

      strcpy(tempString, strtok(NULL, "\n\t"));
      DRIVERS.seniorityDate = atol(tempString);

      strcpy(tempString, strtok(NULL, "\n\t"));
      DRIVERS.dateOfBirth = atol(tempString);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.streetAddress, tempString);
      pad(tempString, DRIVERS_STREETADDRESS_LENGTH);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.city, tempString);
      pad(tempString, DRIVERS_CITY_LENGTH);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.province, tempString);
      pad(tempString, DRIVERS_PROVINCE_LENGTH);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strcpy(DRIVERS.postalCode, tempString);
      pad(tempString, DRIVERS_POSTALCODE_LENGTH);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strncpy(szarString, tempString, 3);
      szarString[3] = 0;
      DRIVERS.telephoneArea = atol(szarString);

      strncpy(szarString, &tempString[4], 3);
      szarString[3] = 0;
      DRIVERS.telephoneNumber = atol(szarString) * 10000;

      strncpy(szarString, &tempString[8], 4);
      szarString[4] = 0;
      DRIVERS.telephoneNumber += atol(szarString);

      strcpy(tempString, strtok(NULL, "\n\t"));
      strncpy(szarString, tempString, 3);
      szarString[3] = 0;
      DRIVERS.faxArea = atol(szarString);

      strncpy(szarString, &tempString[4], 3);
      szarString[3] = 0;
      DRIVERS.faxNumber = atol(szarString) * 10000;

      strncpy(szarString, &tempString[8], 4);
      szarString[4] = 0;
      DRIVERS.faxNumber += atol(szarString);

      DRIVERS.senioritySort = (short int)1;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = 1;
      pad(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
      pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
      pad(DRIVERS.bitmapFileExt, DRIVERS_BITMAPFILEEXT_LENGTH);
      pad(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
      pad(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
      pad(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 5 - Redbus
//
  else if(nLoadType == 5)
  {
    memset(&DRIVERS, ' ', sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fgets(inputString, sizeof(inputString), fp))
    {
      pch = strtok(inputString, "\t\n");
      strcpy(lname, pch);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(fname, pch);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(tempString, pch);
      strcpy(DRIVERS.badgeNumber, tempString);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);

      DRIVERS.seniorityDate = 0;
      DRIVERS.hireDate = 0;
      DRIVERS.senioritySort = (short int)1;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 6 - Madison County Transit
//
  else if(nLoadType == 6)
  {
    memset(&DRIVERS, ' ', sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fscanf(fp, "%ld\t%s\t%s\t%s\t%ld\t%ld\t%ld",
          &badge, &fname, &inits, &lname, &sdate, &ssort, &dtype) > 0)
    {
      ltoa(badge, tempString, 10);
      strcpy(DRIVERS.badgeNumber, tempString);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.initials, inits);
      pad(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.seniorityDate = sdate;
      DRIVERS.hireDate = sdate;
      DRIVERS.senioritySort = (int)ssort;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = dtype;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 7 - Olympia Transit
//
  else if(nLoadType == 7)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fscanf(fp, "%ld\t%s\t%s\t%s\t%ld\t%ld",
          &badge, &lname, &fname, &inits, &sdate, &hdate) > 0)
    {
      ltoa(badge, tempString, 10);
      strcpy(DRIVERS.badgeNumber, tempString);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.initials, inits);
      pad(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.seniorityDate = sdate;
      DRIVERS.hireDate = hdate;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
      pad(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
      pad(DRIVERS.city, DRIVERS_CITY_LENGTH);
      pad(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
      pad(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
      pad(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
      pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
      pad(DRIVERS.bitmapFileExt, DRIVERS_BITMAPFILEEXT_LENGTH);
      pad(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
      pad(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
      pad(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 8 - Kingston Transit
//
  else if(nLoadType == 8)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fscanf(fp, "%s\t%s\t%s\t%ld\t%ld\t%ld",
          &DRIVERS.badgeNumber, &lname, &fname, &sdate, &ssort, &DRIVERS.DRIVERTYPESrecordID) > 0)
    {
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.initials, "");
      pad(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      DRIVERS.seniorityDate = sdate;
      DRIVERS.senioritySort = ssort;
      DRIVERS.hireDate = sdate;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      pad(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
      pad(DRIVERS.city, DRIVERS_CITY_LENGTH);
      pad(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
      pad(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
      pad(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
      pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
      pad(DRIVERS.bitmapFileExt, DRIVERS_BITMAPFILEEXT_LENGTH);
      pad(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
      pad(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
      pad(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 9 - City of Kalamazoo Metro Transit
//
  else if(nLoadType == 9)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0;
    while(fscanf(fp, "%225c", inputString) > 0)  //225 'cause of EOL
    {
      DRIVERS.recordID++;

      DRIVERS.COMMENTSrecordID = NO_RECORD;

      DRIVERS.DIVISIONSrecordID = NO_RECORD;

      strncpy(DRIVERS.lastName, &inputString[0], 19);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      strncpy(DRIVERS.firstName, &inputString[19], 14);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      memset(&DRIVERS.initials, ' ', DRIVERS_INITIALS_LENGTH);

      strncpy(DRIVERS.badgeNumber, &inputString[33], 6);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);

      DRIVERS.crewNumber = 0;

      strncpy(DRIVERS.streetAddress, &inputString[39], 30);
      pad(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);

      strncpy(DRIVERS.city, &inputString[69], 16);
      pad(DRIVERS.city, DRIVERS_CITY_LENGTH);

      strncpy(DRIVERS.province, &inputString[85], 4);
      pad(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);

      strncpy(DRIVERS.postalCode, &inputString[89], 7);
      pad(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);

      strncpy(tempString, &inputString[96], 9);
      DRIVERS.telephoneNumber = atol(tempString);

      strncpy(DRIVERS.SIN, &inputString[105], 12);
      pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);

      strncpy(tempString, &inputString[117], 3);
      DRIVERS.DRIVERTYPESrecordID = atol(tempString);

      strncpy(tempString, &inputString[120], 9);
      DRIVERS.dateOfBirth = atol(tempString);

      strncpy(tempString, &inputString[129], 9);
      DRIVERS.hireDate = atol(tempString);

      strncpy(tempString, &inputString[138], 9);
      DRIVERS.fullTimeDate = atol(tempString);

      strncpy(tempString, &inputString[147], 9);
      DRIVERS.seniorityDate = atol(tempString);

      strncpy(tempString, &inputString[156], 9);
      DRIVERS.promotionDate = atol(tempString);

      strncpy(tempString, &inputString[165], 9);
      DRIVERS.terminationDate = atol(tempString);

      strncpy(DRIVERS.licenseNumber, &inputString[174], 18);
      pad(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
    
      strncpy(tempString, &inputString[192], 9);
      DRIVERS.licenseExpiryDate = atol(tempString);

      strncpy(tempString, &inputString[201], 3);
      DRIVERS.labourUnion = (char)atoi(tempString);

      strncpy(tempString, &inputString[204], 3);
      DRIVERS.section = (char)atoi(tempString);

      strncpy(tempString, &inputString[207], 3);
      DRIVERS.sex = (char)atoi(tempString);

      strncpy(tempString, &inputString[210], 3);
      DRIVERS.race = (char)atoi(tempString);

      strncpy(tempString, &inputString[213], 3);
      DRIVERS.flags = (char)atoi(tempString);

      strncpy(tempString, &inputString[216], 8);
      DRIVERS.transitHireDate = atol(tempString);

      DRIVERS.telephoneArea = 269;
      DRIVERS.faxArea = 0;
      DRIVERS.faxNumber = 0;
      memset(&DRIVERS.emailAddress, ' ', DRIVERS_EMAILADDRESS_LENGTH);
      DRIVERS.senioritySort = 1;
      DRIVERS.vacationTime = 0;
      DRIVERS.personalTime = 0;
      DRIVERS.sickTime = 0;
      DRIVERS.floatTime = 0;

      memset(&DRIVERS.bitmapFileExt, ' ', DRIVERS_BITMAPFILEEXT_LENGTH);

      memset(&DRIVERS.licenseProvinceOfIssue, ' ', DRIVERS_PROVINCE_LENGTH);

      memset(&DRIVERS.licenseType, ' ', DRIVERS_LICENSETYPE_LENGTH);

      DRIVERS.EEOC = 0;

      DRIVERS.position = 0;

      memset(&DRIVERS.reserved1, ' ', DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, ' ', DRIVERS_RESERVED2_LENGTH);

      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
  }
//
//  Load type 11 - Barbados Transport Board
//
  else if(nLoadType == 11)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    DRIVERS.seniorityDate = 0;
    DRIVERS.senioritySort = 0;
    DRIVERS.hireDate = 0;
    DRIVERS.COMMENTSrecordID = NO_RECORD;
    DRIVERS.DIVISIONSrecordID = NO_RECORD;
    pad(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
    pad(DRIVERS.city, DRIVERS_CITY_LENGTH);
    pad(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
    pad(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
    pad(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
    pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
    pad(DRIVERS.bitmapFileExt, DRIVERS_BITMAPFILEEXT_LENGTH);
    pad(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
    pad(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
    pad(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
    while(fscanf(fp, "%s\t%s\t%s",
          &DRIVERS.badgeNumber, &lname, &fname) > 0)
    {
      DRIVERS.recordID++;
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.initials, inits);
      pad(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 12 - Public Transport Service Corporation (Trinidad)
//
  else if(nLoadType == 12)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    DRIVERS.seniorityDate = 0;
    DRIVERS.senioritySort = 0;
    DRIVERS.hireDate = 0;
    DRIVERS.COMMENTSrecordID = NO_RECORD;
    DRIVERS.DIVISIONSrecordID = NO_RECORD;
    pad(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
    pad(DRIVERS.city, DRIVERS_CITY_LENGTH);
    pad(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
    pad(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
    pad(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
    pad(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
    pad(DRIVERS.bitmapFileExt, DRIVERS_BITMAPFILEEXT_LENGTH);
    pad(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
    pad(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
    while(fscanf(fp, "%s\t%s\t%s\t%s\t%s\t%s",
          &fname, &lname, &dob, &DRIVERS.badgeNumber, &DRIVERS.licenseNumber, &expiryDate) > 0)
    {
      DRIVERS.recordID++;
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.initials, "");
      pad(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      DRIVERS.dateOfBirth = atol(dob);
      DRIVERS.licenseExpiryDate = atol(expiryDate);
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 13 - Ocean County
//
  else if(nLoadType == 13)
  {
    memset(&DRIVERS, ' ', sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fscanf(fp, "%s\t%s\t%ld",
          &lname, &fname, &badge) > 0)
    {
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      ltoa(badge, tempString, 10);
      strcpy(DRIVERS.badgeNumber, tempString);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.seniorityDate = 19800101;
      DRIVERS.hireDate = 19800101;
      DRIVERS.fullTimeDate = 19800101;
      DRIVERS.dateOfBirth = 19800101;
      DRIVERS.licenseExpiryDate = 19800101;
      DRIVERS.promotionDate = 19800101;
      DRIVERS.terminationDate = 19800101;
      DRIVERS.transitHireDate = 19800101;
      DRIVERS.senioritySort = (int)ssort;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = 1;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 14 - DASH Driver's Sick and Holiday data
//
  else if(nLoadType == 14)
  {
    rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    numDrivers = 0;
    while(rcode2 == 0)
    {
      DRIVERSrecordIDs[numDrivers] = DRIVERS.recordID;
      badgeNumbers[numDrivers] = atol(DRIVERS.badgeNumber);
      numDrivers++;
      rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
    while(fscanf(fp, "%ld\t%f\t%f", &badge, &sickHours, &vacationHours) > 0)
    {
      for(bFound = FALSE, nI = 0; nI < numDrivers; nI++)
      {
        if(badgeNumbers[nI] == badge)
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        sprintf(tempString, "Driver %ld not found", badge);
        MessageBeep(MB_ICONSTOP);
        MessageBox(NULL, tempString, TMS, MB_OK | MB_ICONSTOP);
        break;
      }
      DRIVERSKey0.recordID = DRIVERSrecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      DRIVERS.sickTime = (long)(sickHours * 3600);
      DRIVERS.vacationTime = (long)(vacationHours * 3600);
      btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
  }
//
//  Load type 15 - Santa Rosa
//
  else if(nLoadType == 15)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fgets(inputString, sizeof(inputString), fp))
    {
      pch = strtok(inputString, "\t\n");
      DRIVERS.DRIVERTYPESrecordID = atol(pch);

      pch = strtok(NULL, " ");
      strcpy(fname, pch);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(lname, pch);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(tempString, pch);
      strcpy(DRIVERS.badgeNumber, tempString);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);

      pch = strtok(NULL, "\t\n");
      DRIVERS.seniorityDate = atol(pch);

      pch = strtok(NULL, "\t\n");
      DRIVERS.hireDate = atol(pch);

      DRIVERS.senioritySort = (short int)1;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 16 - Santa Clarita
//
  else if(nLoadType == 16)
  {
    recordID = 1;
    while(fscanf(fp, "%s\t%s\t%ld\t%d",
          &lname, &fname, &seniorityDate, &senioritySort) > 0)
    {
      memset(&DRIVERS, 0x00, sizeof(DRIVERS));
      DRIVERS.recordID = recordID;
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      strcpy(DRIVERS.badgeNumber, "");
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.seniorityDate = seniorityDate;
      DRIVERS.senioritySort = senioritySort;
      DRIVERS.hireDate = DRIVERS.seniorityDate;
      DRIVERS.fullTimeDate = 19800101;
      DRIVERS.dateOfBirth = 19800101;
      DRIVERS.licenseExpiryDate = 19800101;
      DRIVERS.promotionDate = 19800101;
      DRIVERS.terminationDate = 19800101;
      DRIVERS.transitHireDate = 19800101;
      DRIVERS.physicalExpiryDate = 19800101;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = 1;
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      recordID++;
    }  
  }
//
//  Load type 17 - Lynchburg, VA
//
  else if(nLoadType == 17)
  {
    recordID = 1;
    while(fscanf(fp, "%ld\t%ld\t%s\t%s\t%s\t%ld\t%d",
          &badge, &driverType, &lname, &fname, &inits, &seniorityDate, &senioritySort) > 0)
    {
      memset(&DRIVERS, 0x00, sizeof(DRIVERS));
      DRIVERS.recordID = recordID;
      sprintf(tempString, "%ld", badge);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      strncpy(DRIVERS.badgeNumber, tempString, DRIVERS_BADGENUMBER_LENGTH);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      strcpy(DRIVERS.initials, inits);
      pad(DRIVERS.lastName, DRIVERS_INITIALS_LENGTH);
      DRIVERS.seniorityDate = seniorityDate;
      DRIVERS.senioritySort = senioritySort;
      DRIVERS.hireDate = DRIVERS.seniorityDate;
      DRIVERS.fullTimeDate = 19800101;
      DRIVERS.dateOfBirth = 19800101;
      DRIVERS.licenseExpiryDate = 19800101;
      DRIVERS.promotionDate = 19800101;
      DRIVERS.terminationDate = 19800101;
      DRIVERS.transitHireDate = 19800101;
      DRIVERS.physicalExpiryDate = 19800101;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = driverType;
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      recordID++;
    }  
  }
//
//  Load type 18 - CBS - New Zealand
//
  else if(nLoadType == 18)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fgets(inputString, sizeof(inputString), fp))
    {
      pch = strtok(inputString, "\t\n");
      strcpy(lname, pch);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(fname, pch);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      DRIVERS.DRIVERTYPESrecordID = atol(pch);

      DRIVERS.senioritySort = (short int)1;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  Load type 19 - Leopard - New Zealand
//
  else if(nLoadType == 19)
  {
    memset(&DRIVERS, 0x00, sizeof(DRIVERS));
    DRIVERS.recordID = 0L;
    sdatePrev = NO_RECORD;
    ssort = 1;
    while(fgets(inputString, sizeof(inputString), fp))
    {
      pch = strtok(inputString, "\t\n");
      strcpy(lname, pch);
      strcpy(DRIVERS.lastName, lname);
      pad(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(fname, pch);
      strcpy(DRIVERS.firstName, fname);
      pad(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);

      pch = strtok(NULL, "\t\n");
      strcpy(DRIVERS.badgeNumber, pch);
      pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);

      pch = strtok(NULL, "\t\n");
      DRIVERS.hireDate = atol(pch);
      DRIVERS.seniorityDate = DRIVERS.hireDate;

      DRIVERS.senioritySort = (short int)1;
      DRIVERS.vacationTime = 0;
      DRIVERS.crewNumber = 0;
      DRIVERS.telephoneArea = 0;
      DRIVERS.telephoneNumber = 0;
      DRIVERS.recordID++;
      DRIVERS.COMMENTSrecordID = NO_RECORD;
      DRIVERS.DIVISIONSrecordID = NO_RECORD;
      DRIVERS.DRIVERTYPESrecordID = 1;
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = 0L;
      btrieve(B_INSERT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }  
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);

}

