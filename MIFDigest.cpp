//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
void EN2LL(int, double, double, char*, double *, double *);
}  // extern "C"

int  SetupMIFLINES(MIFPASSEDDATADef *, MIFLINESDef *);

extern "C" {
int MIFDigest(MIFPASSEDDATADef *pMIFPASSEDDATA, MIFLINESDef *pMIFLINES)
{
  return SetupMIFLINES(pMIFPASSEDDATA, pMIFLINES);
}
} // extern "C"

#include "TMS.h"

static	const	char	*wSpace = " \t\r\n";

int SetupMIFLINES(MIFPASSEDDATADef *pMIFPASSEDDATA, MIFLINESDef *pMIFLINES)
{
  MIFLINESDef MIFLINES[TMSRPT61_MAXMIFLINES];
  CString s;
  double  dValue1, dValue2, dValue3, dValue4;
  double  Lat, Long;
  double  distance;
  double  leastDistance;
  BOOL    bDATAFound;
  BOOL    bCoordSysNonEarthFound;
  BOOL    bBreak;
  BOOL    bFound;
  char   *ptr;
  long    firstNODESrecordID;
  long    firstStopNumber;
  long    firstStopNODESrecordID;
  long    toNODESrecordID;
  long    toStopNumber;
  long    toStopNODESrecordID;
  long    NODESrecordID;
  long    previousNODESrecordID;
  long    travelTime;
  int     nI, nJ, nK;
  int     numMultiples;
  int     numSegments;
  int     numMIFLINES;
  int     rcode2;
  int     leastDistanceIndex;
  int     MIFPointer;
//
//  Establish and open the .mif file
//
  CString MIFFileName;
  CStdioFile MIFFile;

  MIFFileName = pMIFPASSEDDATA->szMIFFileName;
//
//  Look for this pattern name attached to the szMIFFileName
//  to see if we have a spcialized trace file
//
  PATTERNNAMESKey0.recordID = pMIFPASSEDDATA->PATTERNNAMESrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
  if(rcode2 != 0)
  {
    sprintf(tempString, "***ERR Pattern not found in PATTERNNAMES : RecordID = %ld\r\n", pMIFPASSEDDATA->PATTERNNAMESrecordID);
    _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
    return(NO_RECORD);
  }
  else
  {
    strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
    trim(tempString, PATTERNNAMES_NAME_LENGTH);
    MIFFileName += tempString;
    MIFFileName += ".mif";
    sprintf(tempString, "***INF Opening Trace File : \"%s\"\r\n", MIFFileName);
    _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
//
//  The specialized trace file isn't there - look for the regular one
//
    if(MIFFile.Open(MIFFileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) == 0)
    {
      sprintf(tempString, "***INF Could not open Trace File : \"%s\"\r\n", MIFFileName);
      _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
      MIFFileName = pMIFPASSEDDATA->szMIFFileName;
      MIFFileName  += ".mif";
      sprintf(tempString, "***INF Opening Trace File : \"%s\"\r\n", MIFFileName);
      _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
      if(MIFFile.Open(MIFFileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) == 0)
      {
        s.LoadString(ERROR_349);
        sprintf(szarString, s, MIFFileName);
        MessageBeep(MB_ICONINFORMATION);
        MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONINFORMATION);
        sprintf(tempString, "***ERR Could not open Trace File : \"%s\"\r\n", MIFFileName);
        _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
        return(NO_RECORD);
      }
    }
  }
//
//  Cycle through the .MIF file
//
  bDATAFound = FALSE;
  bCoordSysNonEarthFound = FALSE;
  numMIFLINES = 0;
  MIFFile.SeekToBegin();
  while(MIFFile.ReadString(s))
  {
    s.MakeUpper();
//
//  Scan through until we hit "DATA" and/or "CoordSys"
//
    if(!bDATAFound)
    {
//
//  Is "CoordSys NonEarth" present?
//
      if(!bCoordSysNonEarthFound)
      {
        nI = s.Find("COORDSYS NONEARTH", 0);
        if(nI != -1)
        {
          bCoordSysNonEarthFound = TRUE;
          continue;
        }
      }
//
//  Check for "DATA"
//
      nI = s.Find("DATA", 0);
      if(nI != -1)
      {
        bDATAFound = TRUE;
      }
      continue;
    }
//
//  "DATA" Encountered.  Now look to see what lines are involved.
//
//  "PLINE"
//
    nI = s.Find("PLINE", 0);
    if(nI != -1)
    {
//
//  There are two possibilities in a PLINE statement:
//     1) It's followed (on the same line) by the number of points to follow
//     2) It's followed (on the same line) by the keyword "MULTIPLE" and then the number of sections,
//        with each section starting with the number of points to follow
// 
//  Look for "MULTIPLE"
//
      strcpy(tempString, s);
      nI = s.Find("MULTIPLE", 0);
      if(nI != -1)
      {
        if((ptr = strtok(tempString, wSpace)) == NULL)  // Get rid of "PLINE"
        {
          continue;
        }
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get rid of "MULTIPLE"
        {
          continue;
        }
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the number of multiples
        {
          continue;
        }
        numMultiples = atol(ptr);
        if(!MIFFile.ReadString(s))
        {
          break;
        }
        numSegments = atol(s);
      }
//
//  No "MULTIPLE" - get the number of segments
//
      else
      {
        numMultiples = 1;
        if((ptr = strtok(tempString, wSpace)) == NULL)  // Get rid of "PLINE"
        {
          continue;
        }
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the number of segments
        {
          continue;
        }
        numSegments = atol(ptr);
      }
//
//  Loop through the segments and multiples
//
      bBreak = FALSE;
      for(nI = 0; nI < numMultiples; nI++)
      {
        if(bBreak)
        {
          break;
        }
        for(nJ = 0; nJ < numSegments; nJ++)
        {
          if(!MIFFile.ReadString(s))
          {
            bBreak = TRUE;
            break;
          }
          strcpy(tempString, s);
          if((ptr = strtok(tempString, wSpace)) == NULL)  // Take the first numeric
          {
            continue;
          }
          dValue1 = (double)atof(ptr);
          if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the second numeric
          {
            continue;
          }
          dValue2 = (double)atof(ptr);
//
//  Got two doubles - do we have to convert them to Lat/Long?
//
          if(bCoordSysNonEarthFound)
          {
            EN2LL(23, dValue1, dValue2, szUTMZone, &Lat, &Long);
          }
          else
          {
            Long = dValue1;
            Lat  = dValue2;
//            Long = dValue2;
//            Lat  = dValue1;
          }
//
//  Save Lat and Long to the data structure
//
          MIFLINES[numMIFLINES].flags = nJ == 0 ? MIFLINES_FLAG_START : 0;
          if(nJ != numSegments - 1)
          {
            MIFLINES[numMIFLINES].from.longitude = Long;
            MIFLINES[numMIFLINES].from.latitude = Lat;
          }
          if(nJ > 0)
          {
            MIFLINES[numMIFLINES - 1].to.longitude = Long;
            MIFLINES[numMIFLINES - 1].to.latitude = Lat;
          }
          if(nJ != numSegments - 1)
          {
            numMIFLINES++;
            if(numMIFLINES >= TMSRPT61_MAXMIFLINES)
            {
              sprintf(tempString, "***ERR TMSRPT61_MAXMIFLINES too small (1)\r\n");
              _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
              return(NO_RECORD);
            }
          }
        }
        if(numMultiples > 1)
        {
          if(!MIFFile.ReadString(s))
          {
            bBreak = TRUE;
          }
          else
          {
            numSegments = atol(s);
          }
        }
      }
    }
//
//  "LINE"
//
    else
    {
      nI = s.Find("LINE", 0);
      if(nI != -1)
      {
        strcpy(tempString, s);
        if((ptr = strtok(tempString, wSpace)) == NULL)  // Get rid of "LINE"
        {
          continue;
        }
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Take the first numeric
        {
          continue;
        }
        dValue1 = (double)atof(ptr);
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the second numeric
        {
          continue;
        }
        dValue2 = (double)atof(ptr);
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the third numeric
        {
          continue;
        }
        dValue3 = (double)atof(ptr);
        if((ptr = strtok(NULL, wSpace)) == NULL)  // Get the fourth numeric
        {
          continue;
        }
        dValue4 = (double)atof(ptr);
//
//  Do we convert?
//
        if(bCoordSysNonEarthFound)
        {
          EN2LL(23, dValue1, dValue2, szUTMZone, &Lat, &Long);
        }
        else
        {
          Long = dValue1;
          Lat  = dValue2;
        }
        MIFLINES[numMIFLINES].flags = MIFLINES_FLAG_START;
        MIFLINES[numMIFLINES].from.longitude = Long;
        MIFLINES[numMIFLINES].from.latitude = Lat;
        if(bCoordSysNonEarthFound)
        {
          EN2LL(23, dValue3, dValue4, szUTMZone, &Lat, &Long);
        }
        else
        {
          Long = dValue3;
          Lat  = dValue4;
        }
        MIFLINES[numMIFLINES].to.longitude = Long;
        MIFLINES[numMIFLINES].to.latitude = Lat;
        numMIFLINES++;
        if(numMIFLINES >= TMSRPT61_MAXMIFLINES)
        {
          sprintf(tempString, "***ERR TMSRPT61_MAXMIFLINES too small (2)\r\n");
          _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
          return(NO_RECORD);
        }
      }
    }
  }
//
//  Establish the output lines
//
//  Get the first node on the pattern
//
  PATTERNSKey2.ROUTESrecordID = pMIFPASSEDDATA->ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = pMIFPASSEDDATA->SERVICESrecordID;
  PATTERNSKey2.directionIndex = pMIFPASSEDDATA->directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = pMIFPASSEDDATA->PATTERNNAMESrecordID;
  PATTERNSKey2.nodeSequence = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  if(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == pMIFPASSEDDATA->ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == pMIFPASSEDDATA->SERVICESrecordID &&
        PATTERNS.directionIndex == pMIFPASSEDDATA->directionIndex &&
        PATTERNS.PATTERNNAMESrecordID == pMIFPASSEDDATA->PATTERNNAMESrecordID)
  {
    NODESrecordID = NodeEquivalenced(PATTERNS.NODESrecordID);
    NODESKey0.recordID = NODESrecordID == NO_RECORD ? PATTERNS.NODESrecordID : NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    firstNODESrecordID = NODES.recordID;
    firstStopNumber = (pMIFPASSEDDATA->directionIndex == 0 ? NODES.OBStopNumber : NODES.IBStopNumber);
    if(firstStopNumber <= 0)
    {
      firstStopNumber = NODES.number;
    }
    if(firstStopNumber > 0)
    {
      bFound = FALSE;
      rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
      while(rcode2 == 0)
      {
        if((NODES.flags & NODES_FLAG_STOP))
        {
          if(NODES.number == firstStopNumber)
          {
            firstStopNODESrecordID = NODES.recordID;
            bFound = TRUE;
            break;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
      if(!bFound)
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      }
//
//  Change to stop number (as derived from long name) : Bob Antonisse, 2-Aug-07
//
      else
      {
        strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
        trim(tempString, NODES_LONGNAME_LENGTH);
        firstStopNumber = atol(&tempString[4]);
      }
    }
    Long = NODES.longitude;
    Lat = NODES.latitude;
  }
  else
  {
    sprintf(tempString, "***ERR Pattern without nodes\r\n");
    _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
    return(NO_RECORD);
  }
//
//  Locate the closest MIFLINE to the starting node
//  and string the rest of the lines together in order
//
  for(nI = 0; nI < numMIFLINES; nI++)
  {
    leastDistance = 10000;
    leastDistanceIndex = NO_RECORD;
    for(nJ = 0; nJ < numMIFLINES; nJ++)
    {
      if(MIFLINES[nJ].flags & MIFLINES_FLAG_USED)
      {
        continue;
      }
      distance = GreatCircleDistance(Long, Lat, MIFLINES[nJ].from.longitude, MIFLINES[nJ].from.latitude);
      if(distance < leastDistance)
      {
        leastDistance = distance;
        leastDistanceIndex = nJ;
      }
    }
    if(leastDistanceIndex == NO_RECORD)
    {
      sprintf(tempString, "***ERR Least distance index remained at NO_RECORD in search for starting node\r\n");
      _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
      return(NO_RECORD);
    }
    pMIFLINES[nI] = MIFLINES[leastDistanceIndex];
    Long = MIFLINES[leastDistanceIndex].to.longitude;
    Lat = MIFLINES[leastDistanceIndex].to.latitude;
    MIFLINES[leastDistanceIndex].flags = MIFLINES_FLAG_USED;
  }
  if(pMIFPASSEDDATA->ROUTESrecordID == 16)
  {
    nI = 1;
  }
//
//  Set all the lines to be contiguous (no "pen up")
//  and set all the nodes to NO_RECORD
//

  for(nI = 1; nI < numMIFLINES; nI++)
  {
    pMIFLINES[nI].from.latitude  = pMIFLINES[nI - 1].to.latitude;
    pMIFLINES[nI].from.longitude = pMIFLINES[nI - 1].to.longitude;
  }

//
//  Nodes and associated stops refer to the "from" element of the
//  first line kept in MIFLINES, and to the "to" element of each
//  subsequent line.
//
//  We got the first node in the pattern earlier
//
  pMIFLINES[0].from.latitude = NODES.latitude;
  pMIFLINES[0].from.longitude = NODES.longitude;
  pMIFLINES[0].from.flags = 0;
  pMIFLINES[0].from.NODESrecordID = firstNODESrecordID;
  pMIFLINES[0].from.associatedStopNODESrecordID = firstStopNODESrecordID;
  pMIFLINES[0].from.associatedStopNumber = firstStopNumber;
  pMIFLINES[0].to.flags = 0;
  pMIFLINES[0].to.NODESrecordID = NO_RECORD;
  pMIFLINES[0].to.associatedStopNODESrecordID = NO_RECORD;
  pMIFLINES[0].to.associatedStopNumber = NO_RECORD;
//
//  Clear out the (yet) unused portions of the structure
//
  for(nI = 1; nI < numMIFLINES; nI++)
  {
    pMIFLINES[nI].from.flags = 0;
    pMIFLINES[nI].from.NODESrecordID = NO_RECORD;
    pMIFLINES[nI].from.associatedStopNODESrecordID = NO_RECORD;
    pMIFLINES[nI].from.associatedStopNumber = NO_RECORD;
    pMIFLINES[nI].to.flags = 0;
    pMIFLINES[nI].to.NODESrecordID = NO_RECORD;
    pMIFLINES[nI].to.associatedStopNODESrecordID = NO_RECORD;
    pMIFLINES[nI].to.associatedStopNumber = NO_RECORD;
  }
//
//  And record the nodes/stops as encountered
//
  pMIFPASSEDDATA->NODESrecordIDs[0] = PATTERNS.NODESrecordID;
  pMIFPASSEDDATA->numNodes = 1;
//
//  Put the rest of the pattern nodes/stops into the MIFLINES structure
//
  MIFPointer = 0;
  previousNODESrecordID = PATTERNS.NODESrecordID;
  rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  while(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == pMIFPASSEDDATA->ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == pMIFPASSEDDATA->SERVICESrecordID &&
        PATTERNS.directionIndex == pMIFPASSEDDATA->directionIndex &&
        PATTERNS.PATTERNNAMESrecordID == pMIFPASSEDDATA->PATTERNNAMESrecordID)
  {
    pMIFPASSEDDATA->NODESrecordIDs[pMIFPASSEDDATA->numNodes] = PATTERNS.NODESrecordID;
    pMIFPASSEDDATA->numNodes++;
//
//  If we encounter a mid-trip layover, it gets a special record.
//
//  From Orbital:
//    "To declare a mid-trip layover, the pattern in the VTP File must contain exactly one record where the
//    'from timepoint name' and 'to timepoint name' fields are set to exactly the same non-null value.
//    In the example case, they should both be set to 'LTCA'.  It wouldn't work to set one to 'LTCA' and
//    the other to 'LTCD' because our software would interpret this as two distinct timepoints.  The following
//    field pairs must also match:  'from timepoint id' = 'to timepoint id', 'from stop name' = 'to stop name',
//    'from stop id' = 'to stop id', 'from longitude' = 'to longitude', 'from latitude' = 'to latitude'.
//
    if(NodesEquivalent(previousNODESrecordID, PATTERNS.NODESrecordID, &travelTime))
    {
      pMIFLINES[MIFPointer - 1].flags |= MIFLINES_FLAG_NEXTISSAMELOCATION;  // MIFPointer can't be 0
    }
//
//  Not a mid-trip layover
//
    else
    {
//
//  Isolate the node
//
      if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        toNODESrecordID = NO_RECORD;
        toStopNODESrecordID = NODES.recordID;
        toStopNumber = (pMIFPASSEDDATA->directionIndex == 0 ? NODES.OBStopNumber : NODES.IBStopNumber);
        if(toStopNumber <= 0)
        {
//          toStopNumber = NODES.number;
          strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
          trim(tempString, NODES_LONGNAME_LENGTH);
          toStopNumber = atol(&tempString[4]);
        }
      }
      else
      {
        NODESrecordID = NodeEquivalenced(PATTERNS.NODESrecordID);
        NODESKey0.recordID = NODESrecordID == NO_RECORD ? PATTERNS.NODESrecordID : NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        toNODESrecordID = NODES.recordID;
        toStopNumber = (pMIFPASSEDDATA->directionIndex == 0 ? NODES.OBStopNumber : NODES.IBStopNumber);
        toStopNODESrecordID = NO_RECORD;
        if(toStopNumber <= 0)
        {
          toStopNumber = NODES.number;
        }
        if(toStopNumber > 0)
        {
          bFound = FALSE;
          rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
          while(rcode2 == 0)
          {
            if((NODES.flags & NODES_FLAG_STOP) && NODES.number == toStopNumber)
            {
              toStopNODESrecordID = NODES.recordID;
              bFound = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
          }
          if(!bFound)
          {
            NODESrecordID = NodeEquivalenced(PATTERNS.NODESrecordID);
            NODESKey0.recordID = NODESrecordID == NO_RECORD ? PATTERNS.NODESrecordID : NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          }
          else
          {
            strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(tempString, NODES_LONGNAME_LENGTH);
            toStopNumber = atol(&tempString[4]);
          }
        }
      }
//
//  Find the closest endpoint to the node/stop
//
      if(pMIFPASSEDDATA->ROUTESrecordID == 21)
      {
        int nx = 1;
      }
      leastDistance = 10000;
      leastDistanceIndex = NO_RECORD;
      for(nI = MIFPointer; nI < numMIFLINES; nI++)
      {
        distance = GreatCircleDistance(NODES.longitude, NODES.latitude, pMIFLINES[nI].to.longitude, pMIFLINES[nI].to.latitude);
        if(distance < leastDistance)
        {
          leastDistance = distance;
          leastDistanceIndex = nI;
        }
      }
      if(leastDistanceIndex == NO_RECORD)
      {
        sprintf(tempString, "***ERR Least distance index remained at NO_RECORD in search for subsequent nodes\r\n");
        _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
        strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(szarString, NODES_ABBRNAME_LENGTH);
        sprintf(tempString, "***INF This function failed at or before the search for %s on the pattern\r\n", szarString);
        _lwrite(pMIFPASSEDDATA->hfErrorLog, tempString, strlen(tempString));
        return(NO_RECORD);
      }
//
//  Set the "to" portion of the line to the located node's coordinates
//
      pMIFLINES[leastDistanceIndex].to.flags = PATTERNS.flags;
      pMIFLINES[leastDistanceIndex].to.latitude = NODES.latitude;
      pMIFLINES[leastDistanceIndex].to.longitude = NODES.longitude;
      pMIFLINES[leastDistanceIndex].to.NODESrecordID = toNODESrecordID;
      pMIFLINES[leastDistanceIndex].to.associatedStopNumber = toStopNumber;
      pMIFLINES[leastDistanceIndex].to.associatedStopNODESrecordID = toStopNODESrecordID;
//
//  If we're not on the last one, set the "from" portion of
//  the next line to the located node's coordinates
//
      if(leastDistanceIndex < numMIFLINES - 1)
      {
        pMIFLINES[leastDistanceIndex + 1].from = pMIFLINES[leastDistanceIndex].to;
      }
//
//  Reset the MIFPointer to the next line and cycle back
//
      MIFPointer = leastDistanceIndex + 1;
    }
    previousNODESrecordID = PATTERNS.NODESrecordID;
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//
//  Go through MIFLINES and fill in the "blanks" where lines connect to just other lines
//
//  Do "from" first
//
  long setIndex = 0;

  nI = -1;
  while(nI < numMIFLINES - 1)
  {
    nI++;
    if(nI >= numMIFLINES)
    {
      break;
    }
    if(pMIFLINES[nI].from.NODESrecordID == NO_RECORD &&
          pMIFLINES[nI].from.associatedStopNODESrecordID == NO_RECORD)
    {
      for(bFound = FALSE, nJ = nI; nJ < numMIFLINES; nJ++)
      {
        if(pMIFLINES[nJ].from.NODESrecordID == NO_RECORD &&
              pMIFLINES[nJ].from.associatedStopNODESrecordID == NO_RECORD)
        {
          continue;
        }
        bFound = TRUE;
        break;
      }
      if(!bFound)
      {
        nJ = numMIFLINES;
      }
      for(nK = nI; nK < nJ; nK++)
      {
        pMIFLINES[nK].from.NODESrecordID = pMIFLINES[setIndex].from.NODESrecordID;
        pMIFLINES[nK].from.associatedStopNODESrecordID = pMIFLINES[setIndex].from.associatedStopNODESrecordID;
        pMIFLINES[nK].from.associatedStopNumber = pMIFLINES[setIndex].from.associatedStopNumber;
        pMIFLINES[nK].from.flags = pMIFLINES[setIndex].from.flags;
      }
      nI = nK - 1;
    }
    setIndex = nI;
  }
//
//  Now do "to"
//
  nI = -1;
  while(nI < numMIFLINES - 1)
  {
    nI++;
    if(nI >= numMIFLINES)
    {
      break;
    }
    if(pMIFLINES[nI].to.NODESrecordID == NO_RECORD &&
          pMIFLINES[nI].to.associatedStopNODESrecordID == NO_RECORD)
    {
      for(nJ = nI; nI < numMIFLINES - 1; nJ++)
      {
        if(pMIFLINES[nJ].to.NODESrecordID == NO_RECORD &&
              pMIFLINES[nJ].to.associatedStopNODESrecordID == NO_RECORD)
        {
          continue;
        }
        for(nK = nI; nK < nJ; nK++)
        {
          pMIFLINES[nK].to.NODESrecordID = pMIFLINES[nJ].to.NODESrecordID;
          pMIFLINES[nK].to.associatedStopNODESrecordID = pMIFLINES[nJ].to.associatedStopNODESrecordID;
          pMIFLINES[nK].to.associatedStopNumber = pMIFLINES[nJ].to.associatedStopNumber;
          pMIFLINES[nK].to.flags = pMIFLINES[nJ].to.flags;
        }
        nI = nK - 1;
        break;
      }
    }
  }
//
//
//  Close the file
//
  MIFFile.Close();
//
//  All done
//
//  Return MIFPointer instead of numMIFLINES in case we short turned
//
  return(MIFPointer);
}
