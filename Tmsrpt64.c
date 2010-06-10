//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Update node data from an external file
//
BOOL FAR TMSRPT64(TMSRPTPassedDataDef *pPassedData)
{
  float longitude, latitude;
  FILE *fp;
  char  nodeName[64], location[64], lat[16], lon[16], number[16];
  char  longName[NODES_LONGNAME_LENGTH + 1];
  long  recID;
  int   rcode2;
  int   counter;
  int   num;
//
//  Open the text file
//
  fp = fopen("NodeChanges.txt", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open NodeChanges.txt", TMS, MB_OK);
    return(FALSE);
  }
//
//  Read from the input file and write to the btrieve file
//
  while(fgets(szarString, sizeof(szarString), fp))
  {
    if(strcmp(szarString, "") == 0)
    {
      continue;
    }
    strcpy(tempString, strtok(szarString, "\t\n"));
    recID = atol(tempString);
    strcpy(tempString, strtok(NULL, "\t\n"));
    latitude = (float)atof(tempString);
    strcpy(tempString, strtok(NULL, "\t\n"));
    longitude = (float)atof(tempString);
    NODESKey0.recordID = recID;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 == 0)
    {
      NODES.latitude = (float)latitude;
      NODES.longitude = (float)longitude;
      btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey2, 2);
    } 
    else
    {
      sprintf(tempString, "Node \"%s\" not found\n");
      MessageBox(NULL, tempString, TMS, MB_OK);
    } 
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);
/*
//
//  Open the text file
//
  fp = fopen("NodeUpdate.txt", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open NodeUpdate.txt", TMS, MB_OK);
    return(FALSE);
  }
//
//  Read from the input file and write to the btrieve file
//
  rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
  recID = AssignRecID(rcode2, NODES.recordID);
  while(fgets(szarString, sizeof(szarString), fp))
  {
    strcpy(tempString, strtok(szarString, "\n\t"));
    num = atol(tempString);
    strcpy(location, strtok(NULL, "\n\t"));
    strcpy(longName, strtok(NULL, "\n\t"));
    memset(&NODES, 0x00, sizeof(NODESDef));
    NODES.recordID = recID++;
    NODES.COMMENTSrecordID = NO_RECORD;
    NODES.JURISDICTIONSrecordID = NO_RECORD;

    sprintf(tempString, "%04d", num);
    strncpy(NODES.abbrName, tempString, NODES_ABBRNAME_LENGTH);

    strncpy(NODES.longName, longName, NODES_LONGNAME_LENGTH);
    pad(NODES.longName, NODES_ABBRNAME_LENGTH);

    strncpy(NODES.intersection, location, NODES_INTERSECTION_LENGTH);
    pad(NODES.intersection, NODES_DESCRIPTION_LENGTH);

    strncpy(NODES.description, location, NODES_DESCRIPTION_LENGTH);
    pad(NODES.description, NODES_DESCRIPTION_LENGTH);

    NODES.longitude = 0.0;
    NODES.latitude = 0.0;
    NODES.flags = NODES_FLAG_STOP;

    btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);
*/
/*
//
//  Open the text file
//
  fp = fopen("NodeUpdate.txt", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open NodeUpdate.txt", TMS, MB_OK);
    return(FALSE);
  }
//
//  Kill any stops in the Nodes table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      rcode2 = btrieve(B_DELETE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Kill any stops in the Patterns table
//
  rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  while(rcode2 == 0)
  {
    if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
    {
      rcode2 = btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    }

    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  }
//
//  Read from the input file and write to the btrieve file
//
  counter = 1;
  rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
  recID = AssignRecID(rcode2, NODES.recordID);
  while(fgets(tempString, TEMPSTRING_LENGTH, fp) > 0)
  {
    if(strcmp(tempString, "") == 0)
    {
      break;
    }
//    strcpy(number, strtok(tempString, "\t\n"));
//    strcpy(nodeName, strtok(NULL, "\t\n"));
    strcpy(nodeName, strtok(tempString, "\t\n"));
    strcpy(location, strtok(NULL, "\t\n"));
    strcpy(lat, strtok(NULL, "\t\n"));
    strcpy(lon, strtok(NULL, "\t\n"));
    
    memset(&NODES, 0x00, sizeof(NODESDef));

    NODES.recordID = ++recID;
    NODES.COMMENTSrecordID = NO_RECORD;
    NODES.JURISDICTIONSrecordID = NO_RECORD;
    sprintf(szarString, "%04d", counter);
    strncpy(NODES.abbrName, szarString, 4);
    sprintf(tempString, "Stop%s", szarString);
    strncpy(NODES.longName, tempString, 8);
    strcpy(NODES.intersection, nodeName);
    pad(NODES.intersection, NODES_INTERSECTION_LENGTH);
    if(strcmp(location, "None") == 0)
    {
      strcpy(NODES.description, nodeName);
      pad(NODES.description, NODES_INTERSECTION_LENGTH);
    }
    else
    {
      strcpy(NODES.description, location);
      pad(NODES.description, NODES_DESCRIPTION_LENGTH);
    }
    NODES.latitude = (float)atof(lat);
    NODES.longitude = (float)atof(lon);
    NODES.flags = NODES_FLAG_STOP;
    NODES.number = counter;

    rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    counter += 5;
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);

}
*/
/*
//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Update node data from an external file
//
BOOL FAR TMSRPT64(TMSRPTPassedDataDef *pPassedData)
{
  double easting, northing;
  FILE *fp;
  char  nodeAbbr[64];
  char  nodeDesc[64];
  long  recordID;
  int   rcode2;
//
//  Open the text file
//
  fp = fopen("Landmarks.txt", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open Landmarks.txt", TMS, MB_OK);
    return(FALSE);
  }
//
//  Read from the input file and write to the btrieve file
//
  rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
  recordID = AssignRecID(rcode2, NODES.recordID);
  while(fgets(tempString, TEMPSTRING_LENGTH, fp) != NULL)
  {
    strcpy(nodeAbbr, strtok(tempString, "|\n"));
    strcpy(nodeDesc, strtok(NULL, "|\n"));
    strcpy(szarString, strtok(NULL, "|\n"));
    easting = atof(szarString);
    strcpy(szarString, strtok(NULL, "|\n"));
    northing = atof(szarString);
    memset(&NODES, 0x00, sizeof(NODESDef));
    NODES.recordID = recordID++;
    NODES.COMMENTSrecordID = NO_RECORD;
    NODES.JURISDICTIONSrecordID = NO_RECORD;
    strncpy(NODES.abbrName, nodeAbbr, NODES_ABBRNAME_LENGTH);
    pad(NODES.abbrName, NODES_ABBRNAME_LENGTH);
    strncpy(NODES.longName, nodeAbbr, NODES_LONGNAME_LENGTH);
    pad(NODES.longName, NODES_ABBRNAME_LENGTH);
    strncpy(NODES.intersection, nodeDesc, NODES_INTERSECTION_LENGTH);
    pad(NODES.intersection, NODES_DESCRIPTION_LENGTH);
    strncpy(NODES.description, nodeDesc, NODES_DESCRIPTION_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
    NODES.longitude = (float)easting;
    NODES.latitude = (float)northing;
    NODES.flags = NODES_FLAG_STOP;
    btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  All done
//
  fclose(fp);
  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);
*/
}
