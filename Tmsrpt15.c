//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define NUMNUMBERS 5
#define NUMSUBS    3
//
//  Load deadhead data from an external file into the connections table
//
BOOL FAR TMSRPT15(TMSRPTPassedDataDef *pPassedData)
{
  FILE *fp;
  float tempFloat;
  char fromNode[NODES_ABBRNAME_LENGTH + 1];
  char toNode[NODES_ABBRNAME_LENGTH + 1];
  char timeOfDay[6];
  char inputLine[512];
  char *ptr;
  char szTime[16];
  char szDist[16];
  char szRoute[ROUTES_NUMBER_LENGTH + 1];
  long fromNodeRecordID;
  long toNodeRecordID;
  long stopNum;
  long recID;
  int  time;
  int  times[8];
  int  rcode2;
  int  service;
  long fromTime;
  long toTime;
  int  dir;
  int  nLoadType;
  int  nI, nJ;
  int  ddd, mm;
  float ss;
  int  action;
  int  index;
  long sub;
  int  numEntries;
  long number;
  long originalRecordID;
  long numbers[NUMNUMBERS] = {700000, 700005, 700021, 700035, 700110};
  long recordIDs[NUMNUMBERS][NUMSUBS] = { 1, 39, -1,
                                          4, 41, 42,
                                         10, 40, -1,
                                         16, 43, 44,
                                         23, 45, 46};
//
//  Establish the type of deadhead load
//
  nLoadType = GetPrivateProfileInt("Reports", "DeadheadLoadType", 1, TMSINIFile);
  nLoadType = 25;
//
//  Load type 1
//
  if(nLoadType == 1)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%s\t%s\t%d", &fromNode, &toNode, &time) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60;
            CONNECTIONS.fromTimeOfDay = NO_TIME;
            CONNECTIONS.toTimeOfDay = NO_TIME;
            CONNECTIONS.flags = CONNECTIONS_FLAG_TWOWAY |
                  CONNECTIONS_FLAG_TRAVELTIME;
            if(CONNECTIONS.connectionTime == 0)
              CONNECTIONS.flags |= CONNECTIONS_FLAG_RUNNINGTIME | CONNECTIONS_FLAG_EQUIVALENT;
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Load type 2
//
  else if(nLoadType == 2)
  {
//
//  Open the text file
//
    fp = fopen("TRV.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open TRV.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%d %s %s %d", &service, &fromNode, &toNode, &time) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey1.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            if(service == 0)
              CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            else
              CONNECTIONS.fromSERVICESrecordID = (service * 1L);
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            if(service == 0)
              CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            else
              CONNECTIONS.toSERVICESrecordID = (service * 1L);
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60;
            CONNECTIONS.fromTimeOfDay = NO_TIME;
            CONNECTIONS.toTimeOfDay = NO_TIME;
            CONNECTIONS.flags = CONNECTIONS_FLAG_TWOWAY | CONNECTIONS_FLAG_TRAVELTIME;
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Load type 3
//
  else if(nLoadType == 3)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%d %s %s %ld %ld %d %d", &service, &fromNode, &toNode,
                                 &fromTime, &toTime, &time, &dir) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey1.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            if(service == 0)
              CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            else
              CONNECTIONS.fromSERVICESrecordID = (service * 1L);
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            if(service == 0)
              CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            else
              CONNECTIONS.toSERVICESrecordID = (service * 1L);
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60L;
            if(fromTime == 99999L)
              CONNECTIONS.fromTimeOfDay = NO_TIME;
            else
              CONNECTIONS.fromTimeOfDay = fromTime;
            if(toTime == 99999L)
              CONNECTIONS.toTimeOfDay = NO_TIME;
            else
              CONNECTIONS.toTimeOfDay = toTime;
            CONNECTIONS.flags = CONNECTIONS_FLAG_DEADHEADTIME;
            if(dir == 2)
              CONNECTIONS.flags |= CONNECTIONS_FLAG_TWOWAY; 
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Type 4 (DHD.TXT and TRV.TXT for Las Vegas)
//
  else if(nLoadType == 4)
  {
//
//  Open the deadhead text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%s %s %d %d %d %d %d %d %d %d", &fromNode, &toNode,
          &times[0], &times[1], &times[2], &times[3],
          &times[4], &times[5], &times[6], &times[7]) > 0)
    {
      strcpy(NODESKey2.abbrName, fromNode);
      pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
      if(rcode2 == 0)
      {
        fromNodeRecordID = NODES.recordID;
        strcpy(NODESKey2.abbrName, toNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          toNodeRecordID = NODES.recordID;
          for(nI = 0; nI < 2; nI++)
          {
            for(nJ = 0; nJ < 4; nJ++)
            {
              rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
              CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
              switch(nJ)
              {
                case 0:
                  CONNECTIONS.fromTimeOfDay = 0;      // 1200A
                  CONNECTIONS.toTimeOfDay = 25140;    //  659A
                  break;
                case 1:
                  CONNECTIONS.fromTimeOfDay = 25200;  //  700A
                  CONNECTIONS.toTimeOfDay = 39540;    // 1059A
                  break;
                case 2:
                  CONNECTIONS.fromTimeOfDay = 39600;  // 1100A
                  CONNECTIONS.toTimeOfDay = 68340;    //  659P
                  break;
                case 3:
                  CONNECTIONS.fromTimeOfDay = 68400;  //  700P
                  CONNECTIONS.toTimeOfDay = T1159P;   // 1159P
                  break;
              }
              CONNECTIONS.fromNODESrecordID = nI == 0 ? fromNodeRecordID : toNodeRecordID;
              CONNECTIONS.fromROUTESrecordID = NO_RECORD;
              CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
              CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
              CONNECTIONS.toNODESrecordID = nI == 0 ? toNodeRecordID : fromNodeRecordID;
              CONNECTIONS.toROUTESrecordID = NO_RECORD;
              CONNECTIONS.toSERVICESrecordID = NO_RECORD;
              CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
              CONNECTIONS.connectionTime = times[(nI * 4) + nJ] * 60;
              CONNECTIONS.flags = CONNECTIONS_FLAG_TWOWAY |
                                  CONNECTIONS_FLAG_TRAVELTIME | CONNECTIONS_FLAG_DEADHEADTIME;
              CONNECTIONS.distance = (float)NO_RECORD;
              CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
              rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            }
          }
        }
      }
    }
  }
//
//  Load type 5
//
  else if(nLoadType == 5)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%s %s %d %s", &fromNode, &toNode, &time, &timeOfDay) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            CONNECTIONS.fromSERVICESrecordID = 1;
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            CONNECTIONS.toSERVICESrecordID = 1;
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60;
            CONNECTIONS.fromTimeOfDay = cTime(timeOfDay);
            CONNECTIONS.toTimeOfDay = cTime(timeOfDay);
            CONNECTIONS.flags = CONNECTIONS_FLAG_DEADHEADTIME;
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Load type 6 (Stops)
//
  else if(nLoadType == 6)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    stopNum = 1;
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = 1;
      NODES.flags = NODES_FLAG_STOP;
//
//  New number
//
      sprintf(tempString, "%04ld", stopNum);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", stopNum++);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Legacy number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Easting
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.longitude = (float)atof(tempString);
//
//  Northing
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Stop type
//
      ptr = strtok(NULL, "\t");
      if(ptr != NULL)
      {
        if(strcmp(ptr, "Shelter") == 0)
        {
          NODES.stopFlags |= (1 << 2);
        }
        else if(strcmp(ptr, "Reg Pole") == 0)
        {
          NODES.stopFlags |= (1 << 3);
        }
        else if(strcmp(ptr, "Strapped to Pole") == 0)
        {
          NODES.stopFlags |= (1 << 4);
        }
        else if(strcmp(ptr, "Strapped to Light Pole") == 0)
        {
          NODES.stopFlags |= (1 << 5);
        }
        else if(strcmp(ptr, "Strapped to Hydro Pole") == 0)
        {
          NODES.stopFlags |= (1 << 6);
        }
        else if(strcmp(ptr, "Anchor") == 0)
        {
          NODES.stopFlags |= (1 << 7);
        }
//
//  Snow route
//
        ptr = strtok(NULL, "\t");
        if(ptr != NULL)
        {
          if(strcmp(ptr, "Plow") == 0)
          {
            NODES.stopFlags |= (1 << 8);
          }
          else if(strcmp(ptr, "Priority") == 0)
          {
            NODES.stopFlags |= (1 << 9);
          }
          else if(strcmp(ptr, "None") == 0)
          {
            NODES.stopFlags |= (1 << 10);
          }
//
//  Shelter Type
//
          ptr = strtok(NULL, "\t");
          if(ptr != NULL)
          {
            if(strcmp(ptr, "TBT") == 0)
            {
              NODES.stopFlags |= (1 << 11);
            }
            else if(strcmp(ptr, "DayTech") == 0)
            {
              NODES.stopFlags |= (1 << 12);
            }
            else if(strcmp(ptr, "None") == 0)
            {
              NODES.stopFlags |= (1 << 12);
            }
          }
        }
      }
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 7 (Stops)
//
  else if(nLoadType == 7)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = 1;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
      sprintf(tempString, "%04ld", NODES.number);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", NODES.number);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 8 (AVL Node Name)
//
  else if(nLoadType == 8)
  {
//
//  Open the text file
//
    fp = fopen("Nodes.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Nodes.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODESKey0.recordID = atol(tempString);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 == 0)
      {
//
//  AVL Node Names
//
        strcpy(tempString, strtok(NULL, "\t\n"));
        pad(tempString, NODES_AVLSTOPNAME_LENGTH);
        strncpy(NODES.AVLStopName, tempString, NODES_AVLSTOPNAME_LENGTH);
//
//  Insert
// 
        rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
      }
    }
  }
//
//  Load type 9 (Stops)
//
  else if(nLoadType == 9)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
      sprintf(tempString, "%04ld", NODES.number);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", NODES.number);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Bench
//
      strcpy(tempString, strtok(NULL, "\t"));
      if(tempString[0] == 'T')
      {
        NODES.flags |= 0x0004;
      }
//
//  Shelter
//
      strcpy(tempString, strtok(NULL, "\t"));
      if(tempString[0] == 'T')
      {
        NODES.flags |= 0x0008;
      }
//
//  Garbage
//
      strcpy(tempString, strtok(NULL, "\t"));
      if(tempString[0] == 'T')
      {
        NODES.flags |= 0x0010;
      }
//
//  Lat (in modified degrees minutes seconds)
//
      strcpy(tempString, strtok(NULL, "\t"));
      tempFloat = (float)atof(tempString);
      ddd = (int)tempFloat;
      mm = (int)((tempFloat - ddd) * 100);
      ss = ((tempFloat - ddd) - ((float)mm / 100)) * 10000;
      NODES.latitude = ddd + ((float)mm / 60) + (float)(ss / 3600);
//
//  Lon (in modified degrees minutes seconds)
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      tempFloat = (float)atof(tempString);
      ddd = (int)tempFloat;
      mm = (int)((tempFloat - ddd) * 100);
      ss = ((tempFloat - ddd) - ((float)mm / 100)) * 10000;
      NODES.longitude = ddd + ((float)mm / 60) + (float)(ss / 3600);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 10 (Stops)
//
  else if(nLoadType == 10)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = 1;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
      sprintf(tempString, "%04ld", NODES.number);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", NODES.number);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Lon
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      NODES.longitude = (float)atof(tempString);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 11 (Stops - Update lat/long)
//
  else if(nLoadType == 11)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      stopNum = atol(tempString);
//
//  Long name
//
      strcpy(tempString, strtok(NULL, "\t"));
      if(strncmp(tempString, "Stop", 4) == 0)
      {
        strncpy(NODESKey2.abbrName, &tempString[4], 4);
      }
      else
      {
        strcpy(NODESKey2.abbrName, tempString);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
      }
//
//  Get the record
//
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
      if(rcode2 != 0)
      {
        sprintf(szarString, "Could not find %s", tempString);
        MessageBox(NULL, szarString, TMS, MB_OK);
      }
      else
      {
//
//  Lat
//
        strcpy(tempString, strtok(NULL, "\t"));
        NODES.latitude = (float)atof(tempString);
//
//  Lon
//
        strcpy(tempString, strtok(NULL, "\t\n"));
        NODES.longitude = (float)atof(tempString);
//
//  Number
//
        if(strncmp(tempString, "Stop", 4) == 0)
        {
          NODES.number = stopNum;
        }

//
//  Update
// 
        rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey2, 2);
      }
    }
  }
//
//  Load type 12 (Stops)
//
  else if(nLoadType == 12)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      NODES.number = atol(inputLine);
      sprintf(tempString, "%04ld", NODES.number);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", NODES.number);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 13 (Stops)
//
  else if(nLoadType == 13)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = 1;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
      sprintf(tempString, "%04ld", NODES.number);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Stop%04ld", NODES.number);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Lon
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      NODES.longitude = (float)atof(tempString);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 14 (Stops - Update lat/long)
//
  else if(nLoadType == 14)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
//
//  RecordID
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODESKey0.recordID = atol(tempString);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 == 0)
      {
//
//  Lat
//
        strcpy(tempString, strtok(NULL, "\t"));
        NODES.latitude = (float)atof(tempString);
//
//  Lon
//
        strcpy(tempString, strtok(NULL, "\t\n"));
        NODES.longitude = (float)atof(tempString);
//
//  Update
// 
        rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey2, 2);
      }
    }
  }
//
//  Load type 15 (Stops)
//
  else if(nLoadType == 15)
  {
//
//  Open the text file
//
    fp = fopen("NODES.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open NODES.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
//
//  Abbr and long name
//
      strcpy(tempString, strtok(NULL, "\t"));
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Comments RecordID
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.COMMENTSrecordID = atol(tempString);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Lon
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      NODES.longitude = (float)atof(tempString);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 16 (Stops)
//
  else if(nLoadType == 16)
  {
//
//  Open the text file
//
    fp = fopen("stops.txt", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open stops.txt", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
//      NODES.flags = NODES_FLAG_STOP;
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      NODES.number = atol(tempString);
//
//  Abbr and long name
//
      sprintf(tempString, "%ld     ", NODES.number);
      pad(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      pad(tempString, NODES_LONGNAME_LENGTH);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//      sprintf(szarString, "Prda%s", tempString);
//      strncpy(NODES.longName, szarString, NODES_LONGNAME_LENGTH);
//
//  Lon
//
      strcpy(szarString, strtok(NULL, "\t"));
      NODES.longitude = (float)(atof(szarString) / 1000000);
//
//  Lat
//
      strcpy(szarString, strtok(NULL, "\t"));
      NODES.latitude = (float)(atof(szarString) / 1000000);
//
//  Address
//
      strcpy(szarString, strtok(NULL, "\t\n"));
      strcat(tempString, " - ");
      strcat(tempString, szarString);
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 17 (Stops)
//
  else if(nLoadType == 17)
  {
//
//  Open the text file
//
    fp = fopen("stops.txt", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open stops.txt", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
//
//  Number
//
      strcpy(tempString, strtok(inputLine, "\t"));
      strncpy(NODESKey2.abbrName, tempString, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
      if(rcode2 == 0)
      {
        action = B_UPDATE;
        memset(NODES.AVLStopName, 0x00, NODES_AVLSTOPNAME_LENGTH);
      }
      else
      {
        rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
        recID = AssignRecID(rcode2, NODES.recordID);
        memset(&NODES, 0x00, sizeof(NODESDef));
        NODES.recordID = recID++;
        NODES.COMMENTSrecordID = NO_RECORD;
        NODES.JURISDICTIONSrecordID = NO_RECORD;
        NODES.flags = NODES_FLAG_STOP;
        NODES.number = atol(tempString);
        strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
        action = B_INSERT;
      }
//
//  Long name
//
      sprintf(szarString, "Stop%s", tempString);
      strncpy(NODES.longName, szarString, NODES_LONGNAME_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Description
//
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Lon
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.longitude = (float)atof(tempString);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      NODES.latitude = (float)atof(tempString);
//
//  Insert/Update
// 
      rcode2 = btrieve(action, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 18 (Travel times)
//
  else if(nLoadType == 18)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%s\t%s\t%d", &fromNode, &toNode, &time) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60;
            CONNECTIONS.fromTimeOfDay = NO_TIME;
            CONNECTIONS.toTimeOfDay = NO_TIME;
            CONNECTIONS.flags = CONNECTIONS_FLAG_TWOWAY | CONNECTIONS_FLAG_TRAVELTIME;
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Load type 19 (Deadhead times)
//
  else if(nLoadType == 19)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      strcpy(fromNode, strtok(inputLine, "\t"));
      strcpy(toNode, strtok(NULL, "\t"));
      strcpy(szTime, strtok(NULL, "\t"));
      strcpy(szDist, strtok(NULL, "\t\n"));
      strcpy(NODESKey2.abbrName, fromNode);
      pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
      if(rcode2 != 0)
      {
        sprintf(tempString, "Couldn't find fromNode \"%s\"", fromNode);
        MessageBox(NULL, tempString, TMS, MB_OK);
      }
      else
      {
        fromNodeRecordID = NODES.recordID;
        strcpy(NODESKey2.abbrName, toNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          numEntries = 1;
          sub = NODES.recordID;
          index = NO_RECORD;
        }
        else
        {
          numEntries = NO_RECORD;
          for(nI = 0; nI < NUMNUMBERS; nI++)
          {
            if(atol(toNode) == numbers[nI])
            {
              numEntries = 3;
              sub = recordIDs[nI][0];
              index = nI;
              break;
            }
          }
          if(numEntries == NO_RECORD)
          {
            sprintf(tempString, "Couldn't find toNode \"%s\"", toNode);
            MessageBox(NULL, tempString, TMS, MB_OK);
          }
        }
        for(nI = 0; nI < numEntries; nI++)
        {
          if(nI > 0)
          {
            sub = recordIDs[index][nI];
            if(sub == NO_RECORD)
            {
              break;
            }
          }
          toNodeRecordID = sub;
          rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
          CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
          CONNECTIONS.fromROUTESrecordID = NO_RECORD;
          CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
          CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
          CONNECTIONS.toNODESrecordID = toNodeRecordID;
          CONNECTIONS.toROUTESrecordID = NO_RECORD;
          CONNECTIONS.toSERVICESrecordID = NO_RECORD;
          CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
          CONNECTIONS.fromTimeOfDay = NO_TIME;
          CONNECTIONS.toTimeOfDay = NO_TIME;
          CONNECTIONS.connectionTime = (long)((float)atof(szTime) * 60);
          CONNECTIONS.flags = CONNECTIONS_FLAG_TRAVELTIME | CONNECTIONS_FLAG_DEADHEADTIME;
          CONNECTIONS.distance = (float)atof(szDist);
          CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
          rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        }
      }
    }
  }
//
//  Load type 20 (Connection times)
//
  else if(nLoadType == 20)
  {
//
//  Open the text file
//
    fp = fopen("Connections.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Connections.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      strcpy(szRoute, strtok(inputLine, "\t"));
      strcpy(fromNode, strtok(NULL, "_"));
      strcpy(toNode, strtok(NULL, "\t"));
      strcpy(szDist, strtok(NULL, "\t"));
      strcpy(szTime, strtok(NULL, "\t\n"));
//
//  Route
//
      strcpy(ROUTESKey1.number, szRoute);
      pad(ROUTESKey1.number, ROUTES_NUMBER_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      if(rcode2 != 0)
      {
        sprintf(tempString, "Couldn't find Route \"%s\"", szRoute);
        MessageBox(NULL, tempString, TMS, MB_OK);
      }
      else
      {
//
//  From node
//
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 != 0)
        {
          sprintf(tempString, "Couldn't find fromNode \"%s\"", fromNode);
          MessageBox(NULL, tempString, TMS, MB_OK);
        }
        else
        {
          fromNodeRecordID = NODES.recordID;
//
//  To node
//
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 != 0)
          {
            sprintf(tempString, "Couldn't find toNode \"%s\"", toNode);
            MessageBox(NULL, tempString, TMS, MB_OK);
          }
          else
          {
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = ROUTES.recordID;
            CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = NODES.recordID;
            CONNECTIONS.toROUTESrecordID = ROUTES.recordID;
            CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = (long)(atol(&szTime[0]) * 60);
            if(szTime[1] == '+')
            {
              CONNECTIONS.connectionTime += 30;
            }
            CONNECTIONS.fromTimeOfDay = NO_TIME;
            CONNECTIONS.toTimeOfDay = NO_TIME;
            CONNECTIONS.flags = CONNECTIONS_FLAG_RUNNINGTIME;
            CONNECTIONS.distance = (float)atof(szDist);
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Load type 21 (Stops)
//
  else if(nLoadType == 21)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = 1;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number (Abbr Name)
//
      strcpy(tempString, strtok(inputLine, "\t"));
      stopNum = atol(tempString);
      pad(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
//
//  Long name
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_LONGNAME_LENGTH);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Legacy number
//
      NODES.number = stopNum;
//
//  Description
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Long
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      NODES.longitude = (float)atof(tempString);
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 22 (Stops)
//
  else if(nLoadType == 22)
  {
//
//  Open the text file
//
    fp = fopen("Stops.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open Stops.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
      NODES.flags = NODES_FLAG_STOP;
//
//  RecordID of original node
//
      strcpy(tempString, strtok(inputLine, "\t"));
      originalRecordID = atol(tempString);
//
//  Number (Abbr Name)
//
      strcpy(tempString, strtok(NULL, "\t"));
      stopNum = atol(tempString);
      pad(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
//
//  Long name
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_LONGNAME_LENGTH);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Description
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Address
//
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Long
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.longitude = (float)atof(tempString);
//
//  Number
//
      strcpy(tempString, strtok(NULL, "\t\n"));
      number = atol(tempString) + 9000;
      NODES.number = number;
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Update the node record to associate it with this stop
//
      NODESKey0.recordID = originalRecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      NODES.OBStopNumber = number;
      NODES.IBStopNumber = number;
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
  }
//
//  Load type 23 (Node update and stop insert)
//
  else if(nLoadType == 23)
  {
//
//  Open the text file
//
    fp = fopen("NodeUpdates.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open NodeUpdates.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
//
//  New or existing?
//
      strcpy(tempString, strtok(inputLine, "\t"));
//
//  New - therefore a stop
//
      if(strncmp(tempString, "Prda", 4) == 0)
      {
        memset(&NODES, 0x00, sizeof(NODESDef));
        NODES.recordID = recID++;
        NODES.COMMENTSrecordID = NO_RECORD;
        NODES.JURISDICTIONSrecordID = NO_RECORD;
        NODES.flags = NODES_FLAG_STOP;
//
//  Abbr and number
//
        strncpy(NODES.abbrName, &tempString[4], NODES_ABBRNAME_LENGTH);
        NODES.number = atol(&tempString[4]);
//
//  Long name
//
        strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Description and Intersection
//
        strcpy(tempString, strtok(NULL, "\t"));
        strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
        pad(NODES.description, NODES_DESCRIPTION_LENGTH);
        strncpy(NODES.intersection, NODES.description, NODES_DESCRIPTION_LENGTH);
//
//  Lat/long
//
        strcpy(tempString, strtok(NULL, "\t"));
        NODES.latitude = (float)atof(tempString);
        strcpy(tempString, strtok(NULL, "\t\n"));
        NODES.longitude = (float)atof(tempString);
//
//  Insert
// 
        rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
//
//  Existing
//
      else
      {
//
//  Get the record
//
        strncpy(NODESKey2.abbrName, tempString, NODES_ABBRNAME_LENGTH);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
//
//  Description
//
          strcpy(tempString, strtok(NULL, "\t"));
          pad(tempString, NODES_DESCRIPTION_LENGTH);
          strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Address
//
          strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Lat
//
          strcpy(tempString, strtok(NULL, "\t"));
          NODES.latitude = (float)atof(tempString);
//
//  Long
//
          strcpy(tempString, strtok(NULL, "\t"));
          NODES.longitude = (float)atof(tempString);
//
//  Number
//
          strcpy(tempString, strtok(NULL, "\t"));
          number = atol(tempString);
          NODES.number = number;
//
//  OB Number and IB Number
//
          strcpy(tempString, strtok(NULL, "\t"));
          number = atol(tempString);
          NODES.OBStopNumber = number;
          strcpy(tempString, strtok(NULL, "\t\n"));
          number = atol(tempString);
          NODES.IBStopNumber = number;
//
//  Update
//
          rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
        }
      }
    }
  }
//
//  Load type 24 (Stops and Timepoints)
//
  else if(nLoadType == 24)
  {
//
//  Open the text file
//
    fp = fopen("TimepointUpdates.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open TimepointUpdates.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    recID = AssignRecID(rcode2, NODES.recordID);
    while(fgets(inputLine, sizeof(inputLine), fp))
    {
      memset(&NODES, 0x00, sizeof(NODESDef));
      NODES.recordID = recID++;
      NODES.COMMENTSrecordID = NO_RECORD;
      NODES.JURISDICTIONSrecordID = NO_RECORD;
      NODES.flags = NODES_FLAG_STOP;
//
//  Number (Abbr Name)
//
      strcpy(tempString, strtok(inputLine, "\t"));
      pad(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);
//
//  Long name
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_LONGNAME_LENGTH);
      strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
//
//  Description
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
//
//  Address
//
      strcpy(tempString, strtok(NULL, "\t"));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
//
//  Lat
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.latitude = (float)atof(tempString);
//
//  Long
//
      strcpy(tempString, strtok(NULL, "\t"));
      NODES.longitude = (float)atof(tempString);
//
//  Stop or node?
//
//  Stop
//
      if(NODES.abbrName[0] == '3')
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        NODES.number = atol(tempString);
      }
//
//  Node
//
      else
      {
//
//  Number
//
        strcpy(tempString, strtok(NULL, "\t\n"));
        number = atol(tempString);
        NODES.number = number;
//
//  OB Number and IB Number
//
        strcpy(tempString, strtok(NULL, "\t"));
        number = atol(tempString);
        NODES.OBStopNumber = number;
        strcpy(tempString, strtok(NULL, "\t\n"));
        number = atol(tempString);
        NODES.IBStopNumber = number;
      }
//
//  Insert
// 
      rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 != 0)
      {
        strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(szarString, NODES_ABBRNAME_LENGTH);
        sprintf(tempString, "Insert failed code %d on add of %s", rcode2, szarString);
        MessageBox(NULL, tempString, TMS, MB_OK);
      }
    }
  }
//
//  Load type 25
//
  if(nLoadType == 25)
  {
//
//  Open the text file
//
    fp = fopen("DHD.TXT", "r");
    if(fp == NULL)
    {
      MessageBox(NULL, "Failed to open DHD.TXT", TMS, MB_OK);
      goto cleanup;
    }
//
//  Read from the input file and write to the btrieve file
//
    while(fscanf(fp, "%s\t%s\t%d", &fromNode, &toNode, &time) > 0)
    {
      if(time != -1)
      {
        strcpy(NODESKey2.abbrName, fromNode);
        pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 == 0)
        {
          fromNodeRecordID = NODES.recordID;
          strcpy(NODESKey2.abbrName, toNode);
          pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
          rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
          if(rcode2 == 0)
          {
            toNodeRecordID = NODES.recordID;
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
            CONNECTIONS.fromROUTESrecordID = NO_RECORD;
            CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
            CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.toNODESrecordID = toNodeRecordID;
            CONNECTIONS.toROUTESrecordID = NO_RECORD;
            CONNECTIONS.toSERVICESrecordID = NO_RECORD;
            CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
            CONNECTIONS.connectionTime = time * 60;
            CONNECTIONS.fromTimeOfDay = NO_TIME;
            CONNECTIONS.toTimeOfDay = NO_TIME;
            CONNECTIONS.flags = CONNECTIONS_FLAG_DEADHEADTIME;
            CONNECTIONS.distance = (float)NO_RECORD;
            CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
            rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
  }
//
//  Cleanup
//
  cleanup:
    if(fp != NULL)
      fclose(fp);
 
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);
}
