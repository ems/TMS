//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

typedef struct SERStruct
{
  long recordID;
  char name[SERVICES_NAME_LENGTH + 1];
} SERDef;

//
//  Load connection data from a "rules" file into the connections table
//
BOOL FAR TMSRPT41(TMSRPTPassedDataDef *pPassedData)
{
  SERDef SERS[20];
  FILE *fp;
  char fromNode[NODES_ABBRNAME_LENGTH + 1];
  char toNode[NODES_ABBRNAME_LENGTH + 1];
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char serviceName[SERVICES_NAME_LENGTH + 1];
  char patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char inputLine[256];
  char *pch;
  long toDelete[1000];
  long previousFromNode;
  long previousToNode;
  long fromNodeRecordID;
  long toNodeRecordID;
  long routeRecordID;
  long serviceRecordID;
  long patternNameRecordID;
  long fromTimeOfDay;
  long toTimeOfDay;
  long connectionTime;
  int  rcode2;
  int  nI;
  int  numServices;
  int  numToDelete;
//
//  Open the text file
//
  fp = fopen("RULES.TXT", "r");
  if(fp == NULL)
  {
    MessageBox(NULL, "Failed to open RULES.TXT", TMS, MB_OK);
    goto cleanup;
  }
//
//  Get the list of all the services
//
  numServices = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  while(rcode2 == 0)
  {
    SERS[numServices].recordID = SERVICES.recordID;
    strncpy(SERS[numServices].name, SERVICES.name, SERVICES_NAME_LENGTH);
    numServices++;
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  }
//
//  Set up the previous nodes
//
  previousFromNode = NO_RECORD;
  previousToNode = NO_RECORD;
//
//  Cycle through the input file
//
  while(fgets(inputLine, sizeof(inputLine), fp))
  {
//
//  From node
//
    pch = strtok(inputLine, "\t\n");
    strcpy(fromNode, pch);
    pad(fromNode, NODES_ABBRNAME_LENGTH);
    strncpy(NODESKey2.abbrName, fromNode, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
    if(rcode2 != 0)
    {
      MessageBox(NULL, fromNode, TMS, MB_OK);
      continue;
    }
    fromNodeRecordID = NODES.recordID;
//
//  To node
//
    pch = strtok(NULL, "\t\n");
    strcpy(toNode, pch);
    pad(toNode, NODES_ABBRNAME_LENGTH);
    strncpy(NODESKey2.abbrName, toNode, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
    if(rcode2 != 0)
    {
      MessageBox(NULL, toNode, TMS, MB_OK);
      continue;
    }
    toNodeRecordID = NODES.recordID;
//
//  Route number
//
    pch = strtok(NULL, "\t\n");
    strcpy(routeNumber, pch);
    if(strncmp(routeNumber, "-1", 2) == 0)
      routeRecordID = NO_RECORD;
    else
    {
      pad(routeNumber, ROUTES_NUMBER_LENGTH);
      strncpy(ROUTESKey1.number, routeNumber, ROUTES_NUMBER_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      if(rcode2 != 0)
        continue;
      routeRecordID = ROUTES.recordID;
    }
//
//  Service name / recordID
//
    pch = strtok(NULL, "\t\n");
    strcpy(serviceName, pch);
    pad(serviceName, SERVICES_NAME_LENGTH);
    for(serviceRecordID = NO_RECORD, nI = 0; nI < numServices; nI++)
    {
      if(strncmp(serviceName, SERS[nI].name, SERVICES_NAME_LENGTH) == 0)
      {
        serviceRecordID = SERS[nI].recordID;
        break;
      }
    }
    if(serviceRecordID == NO_RECORD)
      continue;
//
//  If the node pair is different from the previous one, nuke
//  the existing connections on this service before adding 
//
    if(fromNodeRecordID != previousFromNode ||
          toNodeRecordID != previousToNode)
    {
      CONNECTIONSKey1.fromNODESrecordID = fromNodeRecordID;
      CONNECTIONSKey1.toNODESrecordID = toNodeRecordID;
      CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      numToDelete = 0;
      while(rcode2 == 0 &&
            CONNECTIONS.fromNODESrecordID == fromNodeRecordID &&
            CONNECTIONS.toNODESrecordID == toNodeRecordID)
      {
        if(CONNECTIONS.fromSERVICESrecordID == serviceRecordID)
        {
          toDelete[numToDelete] = CONNECTIONS.recordID;
          numToDelete++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      }
      for(nI = 0; nI < numToDelete; nI++)
      {
        CONNECTIONSKey0.recordID = toDelete[nI];
        rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        if(rcode2 == 0)
        {
          rcode2 = btrieve(B_DELETE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        }
      }
      previousFromNode = fromNodeRecordID;
      previousToNode = toNodeRecordID;
    }
//
//  Pattern
//
    pch = strtok(NULL, "\t\n");
    strcpy(patternName, pch);
    pad(patternName, PATTERNNAMES_NAME_LENGTH);
    if(strncmp(patternName, basePatternName, PATTERNNAMES_NAME_LENGTH) == 0)
      patternNameRecordID = NO_RECORD;
    else
    {
      strncpy(PATTERNNAMESKey1.name, patternName, PATTERNNAMES_NAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
      if(rcode2 != 0)
        continue;
      patternNameRecordID = PATTERNNAMES.recordID;
    }
//
//  From time of day
//
    pch = strtok(NULL, "\t\n");
    fromTimeOfDay = atol(pch);
//
//  To time of day
//
    pch = strtok(NULL, "\t\n");
    toTimeOfDay = atol(pch);
//
//  Connection time
//
    pch = strtok(NULL, "\t\n");
    connectionTime = atol(pch);
//
//  Build up the CONNECTIONS record
//
    rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
    CONNECTIONS.COMMENTSrecordID = NO_RECORD;
    CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
    CONNECTIONS.fromROUTESrecordID = routeRecordID;
    CONNECTIONS.fromSERVICESrecordID = serviceRecordID;
    CONNECTIONS.fromPATTERNNAMESrecordID = patternNameRecordID;
    CONNECTIONS.toNODESrecordID = toNodeRecordID;
    CONNECTIONS.toROUTESrecordID = routeRecordID;
    CONNECTIONS.toSERVICESrecordID = serviceRecordID;
    CONNECTIONS.toPATTERNNAMESrecordID = patternNameRecordID;
    CONNECTIONS.connectionTime = connectionTime;
    CONNECTIONS.fromTimeOfDay = fromTimeOfDay;
    CONNECTIONS.toTimeOfDay = toTimeOfDay;
    CONNECTIONS.distance = NO_RECORD;
    CONNECTIONS.flags = CONNECTIONS_FLAG_RUNNINGTIME;
    CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
//
//  Add it and loop back
//
    btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
//
//  Cleanup
//
  cleanup:
    if(fp != NULL)
    {
      fclose(fp);
    }

  m_bEstablishRUNTIMES = TRUE;

  MessageBox(NULL, "Done!", TMS, MB_OK);

  return(TRUE);
}

