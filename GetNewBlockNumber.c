//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

long GetNewBlockNumber(DISPLAYINFO *pDI)
{
  long lastBlock = 0;
  int  rcode2;
  int  keyNumber;
  BLOCKSDef *pTRIPSChunk;

  if(pDI == NULL)
  {
    return 0;
  }

  keyNumber = pDI->fileInfo.keyNumber;
  pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
//
//  First, find the highest existing block number on this route and service
//
  lastBlock = 0;
  TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
  TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
  TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
  TRIPSKey2.blockNumber = 2147483647;
  TRIPSKey2.blockSequence = NO_TIME;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
  if(rcode2 == 0 && pTRIPSChunk->blockNumber > lastBlock &&
        pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
        pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID)
  {
    lastBlock = pTRIPSChunk->blockNumber;
  }
//
//  Now cycle through any blocks that might have a
//  garage as the assignedToNODESrecordID
//
  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
  {
    TRIPSKey2.assignedToNODESrecordID = NODES.recordID;
    TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
    TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
    TRIPSKey2.blockNumber = 2147483647;
    TRIPSKey2.blockSequence = NO_TIME;
    rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
    if(rcode2 == 0 && pTRIPSChunk->blockNumber > lastBlock)
    {
      lastBlock = pTRIPSChunk->blockNumber;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
//
//  All done
//
  return(lastBlock);
}
