//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define NUMINPUTFILES       3
#define MAXDIVISIONS       20
#define MAXPATTERNNODES   100
#define MAXPATTERNS        50
#define TMSRPT29_ROUTES     0
#define TMSRPT29_NODES      1
#define TMSRPT29_HEADWAYS   2

typedef struct PINFOStruct
{
  long PATTERNNAMESrecordID;
  BOOL bInUse[MAXPATTERNNODES];
} PINFODef;

typedef struct CINFOStruct
{
  long recordID;
  long fromNode;
  long toNode;
  long fromTime;
  long toTime;
  long connectionTime;
} CINFODef;

typedef struct UNIQUEStruct
{
  long connectionTime;
  long numTimes;
} UNIQUEDef;

long DeterminePattern(long, long, long, char *);
BOOL CreateConnections(long, long, char *);
BOOL ATCDivision(char *);
BOOL CondenseConnections(void);
void CondensePass2(int, CINFODef *);


static PINFODef PINFO[MAXPATTERNS];

static int  numDivisions;
static int  divisionList[MAXDIVISIONS];
static long basePatternNodes[MAXPATTERNNODES];
static int  numBasePatternNodes;
static int  numPatterns;


//
//  Load driver data from an external file into the drivers table
//
BOOL FAR TMSRPT29(TMSRPTPassedDataDef *pPassedData)
{
  BOOL  bOK;
  BOOL  bFileOpen[NUMINPUTFILES];
  BOOL  bFound;
  FILE *fp[NUMINPUTFILES];
  char  blanks[80];
  char  dummy[256];
  char  inputString[256];
  char  inputFiles[NUMINPUTFILES][16];
  char  nodeNumbers[256];
  char *ptr;
  char  routeNumber[ROUTES_NAME_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  long  directionIndex;
  long  firstTime;
  long  nodeSequence;
  long  PATTERNSrecordID;
  long  ROUTESrecordID;
  long  SERVICESrecordID;
  long  tempLong;
  long  oldCount;
  long  newCount;
  int   nI;
  int   nJ;
  int   rcode2;

  bOK = TRUE;
  memset(blanks, ' ', 80);
  for(nI = 0; nI < NUMINPUTFILES; nI++)
  {
    bFileOpen[nI] = FALSE;
  }
//
//  Give the warning
//
  LoadString(hInst, ERROR_278, tempString, TEMPSTRING_LENGTH);
  if(MessageBox(NULL, tempString, TMS, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDNO)
  {
    bOK = FALSE;
    goto done;
  }
//
//  Get the list of divisions we're going to load
//
//  Number of divisions
//
  numDivisions = GetPrivateProfileInt("TMSRPT29", "numDivisions", 0, TMSINIFile);
  if(numDivisions == 0)
  {
    LoadString(hInst, ERROR_273, tempString, TEMPSTRING_LENGTH);
    MessageBox(NULL, tempString, TMS, MB_OK);
    bOK = FALSE;
    goto done;
  }
//
//  Did we exceed the max?
//
  if(numDivisions > MAXDIVISIONS)
  {
    LoadString(hInst, ERROR_274, tempString, TEMPSTRING_LENGTH);
    MessageBox(NULL, tempString, TMS, MB_OK);
    bOK = FALSE;
    goto done;
  }
//
//  Loop through the divisions?
//
  for(nI = 0; nI < numDivisions; nI++)
  {
    sprintf(tempString, "DivisionNumber%02d", nI);
    divisionList[nI] = GetPrivateProfileInt("TMSRPT29", tempString, NO_RECORD, TMSINIFile);
    if(divisionList[nI] == NO_RECORD)
    {
      LoadString(hInst, ERROR_275, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBox(NULL, szarString, TMS, MB_OK);
      bOK = FALSE;
      goto done;
    }
    sprintf(tempString, "DivisionName%02d", nI);
    GetPrivateProfileString("TMSRPT29", tempString, "",
          szarString, sizeof(szarString), TMSINIFile);
//
//  Set up the Divisions Table
//
    rcode2 = btrieve(B_GETLAST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    DIVISIONS.recordID = AssignRecID(rcode2, DIVISIONS.recordID);
    DIVISIONS.COMMENTSrecordID = NO_RECORD;
    DIVISIONS.number = divisionList[nI];
    strncpy(DIVISIONS.name, szarString, DIVISIONS_NAME_LENGTH);
    pad(DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    memset(DIVISIONS.reserved, 0x00, DIVISIONS_RESERVED_LENGTH);
    DIVISIONS.flags = 0;
    rcode2 = btrieve(B_INSERT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
//
//  Add the division information as a comment
//
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETLAST, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    COMMENTS.recordID = AssignRecID(rcode2, COMMENTS.recordID);
    strncpy(COMMENTS.code, szarString, COMMENTS_CODE_LENGTH);
    pad(COMMENTS.code, COMMENTS_CODE_LENGTH);
    memset(COMMENTS.reserved, 0x00, COMMENTS_RESERVED_LENGTH);
    COMMENTS.flags = 0;
    memcpy(pCommentText, &COMMENTS, COMMENTS_FIXED_LENGTH);
    strcpy(&pCommentText[COMMENTS_FIXED_LENGTH], szarString);
    btrieve(B_INSERT, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  }
//
//  Establish the file names
//
  strcpy(inputFiles[TMSRPT29_ROUTES], "ROUTES.TXT");
  strcpy(inputFiles[TMSRPT29_NODES], "NODES.TXT");
  strcpy(inputFiles[TMSRPT29_HEADWAYS], "HEADWAYS.TXT");
//
//  Set up the status bar
//
  pPassedData->nReportNumber = 28;
  pPassedData->numDataFiles = 1;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Verify that all the files are present
//
  for(nI = 0; nI < NUMINPUTFILES; nI++)
  {
    fp[nI] = fopen(inputFiles[nI], "r");
    if(fp[nI] == NULL)
    {
      LoadString(hInst, ERROR_204, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, inputFiles[nI]);
      MessageBox(NULL, tempString, TMS, MB_OK);
      bOK = FALSE;
      goto done;
    }
    bFileOpen[nI] = TRUE;
  }
//
//  Routes
//
//  Cols   Data
//  ~~~~   ~~~~
//   1-4   (Alphanumeric) Route number
//    5    ","
//   6-37  Route name
//   38    ","
//   39    Service day (1)
//   40    ","
//   41    Service day (2)
//   42    ","
//   43    Service day (3)
//   44    ","
//  45-52  Outbound direction name
//   53    ","
//  54-61  Inbound direction name
//   62    ","
//  63-66  Internal route number / numeric equivalent (for block numbers)
//
  StatusBarText("Loading Route Data");
  while(fgets(inputString, sizeof(inputString), fp[TMSRPT29_ROUTES]))
  {
    if(StatusBarAbort())
    {
      bOK = FALSE;
      goto done;
    }
    memset(&ROUTES, 0x00, sizeof(ROUTESDef));
//
//  ROUTES.recordID
//
    strncpy(tempString, &inputString[62], 4);
    tempString[4] = '\0';
    ROUTES.recordID = atol(tempString);
//
//  ROUTES.COMMENTSrecordID
//
    ROUTES.COMMENTSrecordID = NO_RECORD;
//
//  ROUTES.DIRECTIONSrecordID[]
//
    for(nI = 0; nI < 2; nI++)
    {
      if(nI == 0)
        strncpy(tempString, &inputString[44], 8);
      else
        strncpy(tempString, &inputString[53], 8);
      tempString[8]= '\0';
      pad(tempString, DIRECTIONS_LONGNAME_LENGTH);
//
//  If it's blank, it means the route is unidirectional
//
      if(strncmp(tempString, blanks, DIRECTIONS_LONGNAME_LENGTH) == 0)
      {
        ROUTES.DIRECTIONSrecordID[nI] = NO_RECORD;
      }
//
//  See if the direction exists already
//
      else
      {
        strncpy(DIRECTIONSKey1.longName, tempString, DIRECTIONS_LONGNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
        if(rcode2 == 0)
        {
          ROUTES.DIRECTIONSrecordID[nI] = DIRECTIONS.recordID;
        }
//
//  No - create a DIRECTIONS record and insert it.
//
//  tempString still contains the long direction name
//
        else
        {
//
//  DIRECTIONS.RecordID
//
          rcode2 = btrieve(B_GETLAST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          DIRECTIONS.recordID = AssignRecID(rcode2, DIRECTIONS.recordID);
//
//  DIRECTIONS.COMMENTSrecordID
//
          DIRECTIONS.COMMENTSrecordID = NO_RECORD;
//
//  DIRECTIONS.abbrName
//
          DIRECTIONS.abbrName[0] = tempString[0];
          DIRECTIONS.abbrName[1] = (tempString[1] == '-' ? 'B' : tempString[1]);
          pad(DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
//
//  DIRECTIONS.longName
//
          strncpy(DIRECTIONS.longName, tempString, DIRECTIONS_LONGNAME_LENGTH);
//
//  DIRECTIONS.reserved
//
          memset(&DIRECTIONS.reserved, 0x00, DIRECTIONS_RESERVED_LENGTH);
//
//  DIRECTIONS.flags
//
          DIRECTIONS.flags = 0;
//
//  DIRECTIONS Insert
//
          rcode2 = btrieve(B_INSERT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          if(rcode2 != 0)
          {
            LoadString(hInst, ERROR_205, szFormatString, sizeof(szFormatString));
            sprintf(tempString, szFormatString, rcode2);
            MessageBox(NULL, tempString, TMS, MB_OK);
            bOK = FALSE;
            goto done;
          }
          ROUTES.DIRECTIONSrecordID[nI] = DIRECTIONS.recordID;
        }
      }
    }
//
//  ROUTES.number
//
    strncpy(tempString, &inputString[0], 4);
    tempString[4] = '\0';
    strcpy(ROUTES.number, tempString);
    pad(ROUTES.number, ROUTES_NUMBER_LENGTH);
//
//  ROUTES.name
//
    strncpy(tempString, &inputString[5], 32);
    tempString[32] = '\0';
    strcpy(ROUTES.name, tempString);
    pad(ROUTES.name, ROUTES_NAME_LENGTH);
//
//  ROUTES bays
//
    ROUTES.OBBayNODESrecordID = NO_RECORD;
    ROUTES.IBBayNODESrecordID = NO_RECORD;
//
//  ROUTES Insert
//
    rcode2 = btrieve(B_INSERT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_206, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, rcode2);
      MessageBox(NULL, tempString, TMS, MB_OK);
      bOK = FALSE;
      goto done;
    }
  }
//
//  Nodes
//
//  Cols   Data
//  ~~~~   ~~~~
//   1-3   Blank
//   4-7   Node number / name
//    8    ","
//   9-12  On street
//   13    ","
//  14-17  At street
//   18    ","
//  19-22  Address prefix (1)
//   23    ","
//  24-27  Address prefix (2)
//   28    ","
//  29-    Address / Intersection
//
  StatusBarText("Loading Node Data");
  while(fgets(inputString, sizeof(inputString), fp[TMSRPT29_NODES]))
  {
    if(StatusBarAbort())
    {
      bOK = FALSE;
      goto done;
    }
    memset(&NODES, 0x00, sizeof(NODESDef));
//
//  NODES.recordID
//
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    NODES.recordID = AssignRecID(rcode2, NODES.recordID);
//
//  NODES.COMMENTSrecordID
//
    NODES.COMMENTSrecordID = NO_RECORD;
//
//  NODES.JURISDICTIONSrecordID
//
    NODES.JURISDICTIONSrecordID = NO_RECORD;
//
//  NODES.abbrName
//
    strncpy(NODES.abbrName, &inputString[3], 4);
//
//  NODES.longName
//
    strncpy(NODES.longName, &inputString[8], 4);
    strncpy(&NODES.longName[4], &inputString[13], 4);
//
//  NODES.intersection
//
    strcpy(tempString, "");
//
//  There are extra information fields in the input stream.
//  If they're not blank, prefix the intersection field with them
//
    if(strncmp(&inputString[18], blanks, 4) != 0)
    {
      strncpy(szarString, &inputString[18], 4);
      szarString[4] = '\0';
      strcat(tempString, szarString);
      strcat(tempString, " ");
    }
    if(strncmp(&inputString[23], blanks, 4) != 0)
    {
      strncpy(szarString, &inputString[23], 4);
      szarString[4] = '\0';
      strcat(tempString, szarString);
      strcat(tempString, " ");
    }
//
//  Now the rest of the intersection
//
//  There's crap at the end of the record, so get rid of it
//
    strcat(tempString, &inputString[28]);
    for(nI = strlen(tempString) - 1; nI > 0; nI--)
    {
      if(tempString[nI] == 10)  // ^J ??
      {
        tempString[nI] = ' ';
        break;
      }
    }
    strcpy(NODES.intersection, tempString);
    pad(NODES.intersection, NODES_INTERSECTION_LENGTH);
//
//  NODES.reliefLabels
//
    strncpy(NODES.reliefLabels, blanks, NODES_RELIEFLABELS_LENGTH);
//
//  NODES.longitude and NODES.latitude
//
    tempLong = 0L;
    memcpy(&NODES.longitude, &tempLong, sizeof(long));
    memcpy(&NODES.latitude, &tempLong, sizeof(long));
//
//  NODES.number
//
    strncpy(tempString, &inputString[3], 4);
    NODES.number = atol(tempString);
//
//  NODES.reserved
//
    memset(&NODES.reserved, 0x00, NODES_RESERVED_LENGTH);
//
//  NODES.flags
//
    NODES.flags = 0;
//
//  NODES Insert
//
    rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_230, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, rcode2);
      MessageBox(NULL, tempString, TMS, MB_OK);
      bOK = FALSE;
      goto done;
    }
  }
//
//  Headways
//
//  Header Record
//  
//  Cols   Data
//  ~~~~   ~~~~
//    1    Blank
//   2-6   "ROUTE"
//    7    Blank
//   8-11  (Alphanumeric) Route number
//   12    Blank
//  13-20  Direction name
//  21-23  " : "
//  24-80  Route name
//  
//  Nodes on the Route Record
//  
//  Cols   Data
//  ~~~~   ~~~~
//   1-7   ",,,,,,,"
//   8-11  Node number
//   12    ","
//  13-16  Node number
//   17    ","
//  
//  ...continues in i4,1x format until "\0"
//  
//  Trip records
//  
//  Cols   Data
//  ~~~~   ~~~~
//  1-2   Service number (1=Wk, 2=Sa, 3=Su)
//    3    ","
//   4-5   Division number
//    6    ","
//   7-10  (Alphanumeric) Route number
//   11    ","
//  12-19  Direction name
//   20    ","
//  21-25  Block number (rrrnn, where "rrr" is the internal route #)
//   26    ","
//  27-30  Trip start (in minutes after midnight)
//   31    ","
//  32-35  Trip end (in minutes after midnight)
//   36    ","
//  37-40  (Military) Time at timepoint
//  41-42  " ,"
//  
//  ...repeats Number of Nodes on the Routes Record times.
//  
//  If there's no time at a timepoint, the field contains "  - "
//  
  StatusBarText("Loading Headway Data");
  ptr = fgets(inputString, sizeof(inputString), fp[TMSRPT29_HEADWAYS]);
  while(ptr)
  {
    if(StatusBarAbort())
    {
      bOK = FALSE;
      goto done;
    }
//
//  Figure out the type of record we've just read
//
//  Header record
//
    if(strncmp(inputString, " ROUTE", 6) == 0)
    {
//
//  Header: Determine the route
//
      strncpy(tempString, &inputString[7], 4);
      tempString[4] = '\0';
      strcpy(ROUTESKey1.number, tempString);
      pad(ROUTESKey1.number, ROUTES_NUMBER_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      if(rcode2 != 0)
      {
        LoadString(hInst, ERROR_231, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, tempString);
        MessageBox(NULL, szarString, TMS, MB_OK);
        bOK = FALSE;
        goto done;
      }
      ROUTESrecordID = ROUTES.recordID;
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Header: Determine the direction
//
      strncpy(tempString, &inputString[12], 8);
      tempString[8] = '\0';
      pad(tempString, DIRECTIONS_LONGNAME_LENGTH);
      strncpy(DIRECTIONSKey1.longName, tempString, DIRECTIONS_LONGNAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
      if(rcode2 != 0)
      {
        LoadString(hInst, ERROR_232, szFormatString, sizeof(szFormatString));
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(szarString, szFormatString, tempString, routeNumber);
        MessageBox(NULL, szarString, TMS, MB_OK);
        bOK = FALSE;
        goto done;
      }
      if(DIRECTIONS.recordID == ROUTES.DIRECTIONSrecordID[0])
        directionIndex = 0;
      else if(DIRECTIONS.recordID == ROUTES.DIRECTIONSrecordID[1])
        directionIndex = 1;
      else
      {
        LoadString(hInst, ERROR_233, szFormatString, sizeof(szFormatString));
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(szarString, szFormatString, tempString, routeNumber);
        MessageBox(NULL, szarString, TMS, MB_OK);
        bOK = FALSE;
        goto done;
      }
//
//  Get the next record
//
      ptr = fgets(inputString, sizeof(inputString), fp[TMSRPT29_HEADWAYS]);
    }
//
//  Nodes on the route record.  The record terminates with \j\0
//
    else if(strncmp(inputString, ",,,,,,,", 7) == 0)
    {
//
//  Save the current input string.  We actually have to read the next record
//  (which is the first trip record) in order to determine what service
//  we're on.
//
      strcpy(nodeNumbers, &inputString[7]);
      ptr = fgets(inputString, sizeof(inputString), fp[TMSRPT29_HEADWAYS]);
      if(!ptr)
      {
        LoadString(hInst, ERROR_243, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, routeNumber);
        MessageBox(NULL, szarString, TMS, MB_OK);
        bOK = FALSE;
        goto done;
      }
//
//  Determine the service
//
      strncpy(tempString, inputString, 2);
      tempString[2] = '\0';
      SERVICESKey1.number = atol(tempString);
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      if(rcode2 != 0)
      {
        LoadString(hInst, ERROR_260, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, routeNumber);
        MessageBox(NULL, szarString, TMS, MB_OK);
        bOK = FALSE;
        goto done;
      }
      SERVICESrecordID = SERVICES.recordID;
//
//  Loop through the node numbers so we can build a BASE pattern
//
      nJ = 0;
      rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      PATTERNSrecordID = AssignRecID(rcode2, PATTERNS.recordID);
      nodeSequence = 10000;
      numBasePatternNodes = 0;
      while(nodeNumbers[nJ] != 10)
      {
        memset(&PATTERNS, 0x00, sizeof(PATTERNSDef));
//
//  PATTERNS.recordID
//
        PATTERNS.recordID = PATTERNSrecordID++;
//
//  PATTERNS.COMMENTSrecordID
//
        PATTERNS.COMMENTSrecordID = NO_RECORD;
//
//  PATTERNS.ROUTESrecordID
//
        PATTERNS.ROUTESrecordID = ROUTESrecordID;
//
//  PATTERNS.SERVICESrecordID
//
        PATTERNS.SERVICESrecordID = SERVICESrecordID;
//
//  PATTERNS.directionIndex
//
        PATTERNS.directionIndex = directionIndex;
//
//  PATTERNS.PATTERNNAMESrecordID
//
        PATTERNS.PATTERNNAMESrecordID = basePatternRecordID;
//
//  PATTERNS.NODESrecordID
//
        strncpy(tempString, &nodeNumbers[nJ], 4);
        tempString[4] = '\0';
        strncpy(NODESKey2.abbrName, tempString, 4);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
        if(rcode2 != 0)
        {
          LoadString(hInst, ERROR_269, szFormatString, sizeof(szFormatString));
          sprintf(szarString, szFormatString, tempString, routeNumber);
          MessageBox(NULL, szarString, TMS, MB_OK);
          bOK = FALSE;
          goto done;
        }
        PATTERNS.NODESrecordID = NODES.recordID;
//
//  PATTERNS.nodeSequence
//
        PATTERNS.nodeSequence = nodeSequence;
        nodeSequence += 10000;
//
//  PATTERNS.reserved
//
        memset(&PATTERNS.fromText, 0x00, PATTERNS_FROMTEXT_LENGTH);
        memset(&PATTERNS.toText, 0x00, PATTERNS_TOTEXT_LENGTH);
        memset(&PATTERNS.reserved, 0x00, PATTERNS_RESERVED_LENGTH);
//
//  PATTERNS.flags
//
//  Force the first timepoint to be the MLP
//
        PATTERNS.flags = (nJ == 0 ? PATTERNS_FLAG_MLP : 0);
//
//  PATTERNS: Insert
//
        rcode2 = btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
        if(rcode2 != 0)
        {
          LoadString(hInst, ERROR_271, szFormatString, sizeof(szFormatString));
          sprintf(tempString, szFormatString, rcode2);
          MessageBox(NULL, tempString, TMS, MB_OK);
          bOK = FALSE;
          goto done;
        }
//
//  Save the base pattern internally
//
        basePatternNodes[numBasePatternNodes] = PATTERNS.NODESrecordID;
        numBasePatternNodes++;
//
//  Get the next timepoint
//
        nJ += 5;
      }
//
//  Zero out numPatterns for the benefit of the headway records to follow
//
      numPatterns = 0;
    }
//
//  Trip records.  The first trip record was fetched above.
//
    else
    {
//
//  Only load trips belonging to the required divisions
//
      if(ATCDivision(&inputString[3]))
      {
//
//  Do we need to add a comment to the route?
//
        ROUTESKey0.recordID = ROUTESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if(rcode2 == 0 && ROUTES.COMMENTSrecordID == NO_RECORD)
        {
          DIVISIONSKey1.number = atol(&inputString[3]);
          rcode2 = btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
          if(rcode2 == 0)
          {
            strncpy(COMMENTSKey1.code, DIVISIONS.name, COMMENTS_CODE_LENGTH);
            pad(COMMENTSKey1.code, COMMENTS_CODE_LENGTH);
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey1, 1);
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            if(rcode2 == 0)
            {
              ROUTES.COMMENTSrecordID = COMMENTS.recordID;
              rcode2 = btrieve(B_UPDATE, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            }
          }
        }
//
//  Get the first time
//
        strncpy(tempString, &inputString[26], 4);
        tempString[4] = '\0';
        firstTime = atol(tempString) * 60;
//
//  TRIPS.recordID
//
        rcode2 = btrieve(B_GETLAST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        TRIPS.recordID = AssignRecID(rcode2, TRIPS.recordID);
//
//  TRIPS.COMMENTSrecordID
//
        TRIPS.COMMENTSrecordID = NO_RECORD;
//
//  TRIPS.ROUTESrecordID
//
        TRIPS.ROUTESrecordID = ROUTESrecordID;
//
//  TRIPS.SERVICESrecordID
//
        TRIPS.SERVICESrecordID = SERVICESrecordID;
//
//  TRIPS.BUSTYPESrecordID
//
        TRIPS.BUSTYPESrecordID = NO_RECORD;
//
//  TRIPS.PATTERNNAMESrecordID
//
        TRIPS.PATTERNNAMESrecordID =
              DeterminePattern(ROUTESrecordID, SERVICESrecordID, directionIndex, inputString);
        if(TRIPS.PATTERNNAMESrecordID == NO_RECORD)
        {
          bOK = FALSE;
          goto done;
        }
//
//  TRIPS.timeAtMLP and tripSequence
//
        TRIPS.timeAtMLP = firstTime;
        TRIPS.tripSequence = firstTime;
//
//  TRIPS.SIGNCODESrecordID
//
        TRIPS.SIGNCODESrecordID = NO_RECORD;
//
//  TRIPS.directionIndex
//
        TRIPS.directionIndex = directionIndex;
//
//  TRIPS.tripNumber
//
        TRIPS.tripNumber = NO_RECORD;
//
//  TRIPS.shift.*
//
        TRIPS.shift.negativeMax = NO_TIME;
        TRIPS.shift.positiveMax = NO_TIME;
        TRIPS.shift.actual = NO_TIME;
//
//  TRIPS.standard.*
//
//  TRIPS.standard.blockNumber
//
//  Figure out where it's blocked.  This is a verification process.  We
//  know the recordID of the route 'cause it was set up above as the
//  "Internal Route Number".  Never hurts to check, though.
//
        strncpy(tempString, &inputString[20], 3);
        tempString[3] = '\0';
        ROUTESKey0.recordID = atol(tempString);
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if(rcode2 != 0)
        {
          LoadString(hInst, ERROR_270, szFormatString, sizeof(szFormatString));
          sprintf(szarString, szFormatString, rcode2);
          MessageBox(NULL, szarString, TMS, MB_OK);
          bOK = FALSE;
          goto done;
        }
        strncpy(tempString, &inputString[20], 5);
        tempString[5] = '\0';
        TRIPS.standard.blockNumber = atol(tempString);
//
//  TRIPS.standard.BLOCKNAMESrecordID
//
        TRIPS.standard.BLOCKNAMESrecordID = NO_RECORD;
//
//  TRIPS.standard.POGNODESrecordID and PIGNODESrecordID
//
        TRIPS.standard.POGNODESrecordID = NO_RECORD;
        TRIPS.standard.PIGNODESrecordID = NO_RECORD;
//
//  TRIPS.standard.RGRPROUTESrecordID
//
        TRIPS.standard.RGRPROUTESrecordID = ROUTES.recordID;
//
//  TRIPS.standard.SGRPSERVICES
//
        TRIPS.standard.SGRPSERVICESrecordID = SERVICESrecordID;
//
//  TRIPS.standard.assignedToNODESrecordID
//
        TRIPS.standard.assignedToNODESrecordID = NO_RECORD;
//
//  TRIPS.standard.blockSequence
//
        TRIPS.standard.blockSequence = firstTime;
//
//  TRIPS.standard.layoverMin and layoverMax
//
        TRIPS.standard.layoverMin = NO_TIME;
        TRIPS.standard.layoverMax = NO_TIME;
//
//  TRIPS.dropback.*
//
        TRIPS.dropback.blockNumber = 0;
        TRIPS.dropback.BLOCKNAMESrecordID = NO_RECORD;
        TRIPS.dropback.POGNODESrecordID = NO_RECORD;
        TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
        TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
        TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
        TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
        TRIPS.dropback.blockSequence = NO_TIME;
        TRIPS.dropback.layoverMin = NO_TIME;
        TRIPS.dropback.layoverMax = NO_TIME;
//
//  TRIPS.customerCOMMENTSrecordID
//
        TRIPS.customerCOMMENTSrecordID = NO_RECORD;
//
//  TRIPS.reserved
//
        memset(&TRIPS.reserved, 0x00, TRIPS_RESERVED_LENGTH);
//
//  TRIPS.flags
//
        TRIPS.flags = 0;
//
//  TRIPS: Insert
//
        rcode2 = btrieve(B_INSERT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        if(rcode2 != 0)
        {
          LoadString(hInst, ERROR_272, szFormatString, sizeof(szFormatString));
          sprintf(tempString, szFormatString, routeNumber);
          MessageBox(NULL, tempString, TMS, MB_OK);
          bOK = FALSE;
          goto done;
        }
//
//  Create the connections
//
        if(!CreateConnections(ROUTESrecordID, SERVICESrecordID, inputString))
        {
          bOK = FALSE;
          goto done;
        }
      }
//
//  Get the next record
//
      ptr = fgets(inputString, sizeof(inputString), fp[TMSRPT29_HEADWAYS]);
    }
  }
//
//  Eliminate the patterns from any route that doesn't have trips on it
//
//  Cycle through all the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
      return(FALSE);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Cycle through all the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
        return(FALSE);
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
      sprintf(tempString, "Eliminating Unused Patterns - %s\n%s - %s",
            serviceName, routeNumber, routeName);
      StatusBarText(tempString);
//
//  Loop through the (possibly) two directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          continue;
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_TIME;
//
//  See if there are any trips on the rte/ser/dir
//
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        if(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
          continue;  // Yes
        }
//
//  No.  Delete any pattern data on this rte/ser/dir
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nI)
        {
          btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2,2 );
        }
      }  // nI
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Now eliminate any unused nodes
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    PATTERNSKey1.NODESrecordID = NODES.recordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey1, 1);
    if(rcode2 != 0)
    {
      btrieve(B_DELETE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Now eliminate any unused routes
//
//
//  Cycle through all the routes
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
      return(FALSE);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
//
//  Cycle through all the services
//
    bFound = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
        return(FALSE);
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
      sprintf(tempString, "Eliminating Unused Routes - %s\n%s - %s",
            serviceName, routeNumber, routeName);
      StatusBarText(tempString);
//
//  Loop through the (possibly) two directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          continue;
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_TIME;
//
//  See if there are any trips on the rte/ser/dir
//
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        if(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
          bFound = TRUE;
          break;
        }
      }
//
//  If there was, break
//
      if(bFound)
        break;
//
//  See if there are any blocks on this rte/ser
//
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
      TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
      TRIPSKey2.blockNumber = 0;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      if(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
            TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
            TRIPS.standard.SGRPSERVICESrecordID == SERVICES.recordID)
      {
        bFound = TRUE;
        break;
      }
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  If no reference was found, delete the route
//
    if(!bFound)
      btrieve(B_DELETE, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Condense the connections table
//
  rcode2 = btrieve(B_STAT, TMS_CONNECTIONS, &BSTAT, dummy, 0);
  oldCount = BSTAT.numRecords;
  if(!CondenseConnections())
  {
    bOK = FALSE;
    goto done;
  }
  rcode2 = btrieve(B_STAT, TMS_CONNECTIONS, &BSTAT, dummy, 0);
  newCount = BSTAT.numRecords;
  sprintf(tempString, "Connections reduced from %ld to %ld", oldCount, newCount);
  MessageBox(NULL, tempString, TMS, MB_OK);
//
//  All done
//
  done:
    for(nI = 0; nI < NUMINPUTFILES; nI++)
    {
      if(bFileOpen[nI])
      {
        fclose(fp[nI]);
      }
    }
    StatusBarEnd();
    if(bOK)
    {
      MessageBox(NULL, "Done!", TMS, MB_OK);
    }

  return(TRUE);

}

//
//  Determine Pattern
//
long DeterminePattern(long ROUTESrecordID, 
      long SERVICESrecordID, long directionIndex, char *inputString)
{
  BOOL bFirst;
  BOOL bInUse[MAXPATTERNNODES];
  BOOL bNotBase;
  BOOL bGotPattern;
  long PATTERNNAMESrecordID;
  long PATTERNSrecordID;
  int  nI;
  int  nJ;
  int  pos;
  int  rcode2;
//
//  Figure out where this thing goes through
//
  pos = 36;
  for(nI = 0; nI < numBasePatternNodes; nI++)
  {
    bInUse[nI] = (strncmp(&inputString[pos], "  -  ", 5) != 0) ? TRUE : FALSE;
    pos += 6;
  }
//
//  See if it's the BASE pattern
//
  bNotBase = FALSE;
  for(nI = 0; nI < numBasePatternNodes; nI++)
  {
    if(!bInUse[nI])
    {
      bNotBase = TRUE;
      break;
    }
  }
  if(!bNotBase)
  {
    return(basePatternRecordID);
  }
//
//  Not the base - have we recorded this one yet?
//
  for(nI = 0; nI < numPatterns; nI++)
  {
    bGotPattern = TRUE;  // Assume success
    for(nJ = 0; nJ < numBasePatternNodes; nJ++)
    {
      if(PINFO[nI].bInUse[nJ] != bInUse[nJ])
      {
        bGotPattern = FALSE;
        break;
      }
    }
    if(bGotPattern)
    {
      return(PINFO[nI].PATTERNNAMESrecordID);
    }
  }
//
//  Well, we're here, so it means no, we haven't recorded this pattern yet
//
//  What to call it?  (NB01, SB04, and so on)
//
  ROUTESKey0.recordID = ROUTESrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[directionIndex];
  rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
  trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
  sprintf(szarString, "%s%02d", tempString, numPatterns + 1);
//
//  Got the name.  Is it in PATTERNNAMES already?
//
  strcpy(PATTERNNAMESKey1.name, szarString);
  pad(PATTERNNAMESKey1.name, PATTERNNAMES_NAME_LENGTH);
  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  if(rcode2 != 0)
  {
    rcode2 = btrieve(B_GETLAST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
    PATTERNNAMES.recordID = AssignRecID(rcode2, PATTERNNAMES.recordID);
    PATTERNNAMES.COMMENTSrecordID = NO_RECORD;
    strcpy(PATTERNNAMES.name, szarString);
    pad(PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
    memset(&PATTERNS.fromText, 0x00, PATTERNS_FROMTEXT_LENGTH);
    memset(&PATTERNS.toText, 0x00, PATTERNS_TOTEXT_LENGTH);
    memset(PATTERNNAMES.reserved, 0x00, PATTERNNAMES_RESERVED_LENGTH);
    PATTERNNAMES.flags = 0;
    rcode2 = btrieve(B_INSERT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_276, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, rcode2);
      MessageBox(NULL, szarString, TMS, MB_OK);
      return(NO_RECORD);
    }
  }
  PATTERNNAMESrecordID = PATTERNNAMES.recordID;
//
//  Build the new pattern and add it to the database
//
  rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  PATTERNSrecordID = AssignRecID(rcode2, PATTERNS.recordID);
  bFirst = TRUE;
  for(nI = 0; nI < numBasePatternNodes; nI++)
  {
    if(!bInUse[nI])
      continue;
    memset(&PATTERNS, 0x00, sizeof(PATTERNSDef));
    PATTERNS.recordID = PATTERNSrecordID++;
    PATTERNS.COMMENTSrecordID = NO_RECORD;
    PATTERNS.ROUTESrecordID = ROUTESrecordID;
    PATTERNS.SERVICESrecordID = SERVICESrecordID;
    PATTERNS.directionIndex = directionIndex;
    PATTERNS.PATTERNNAMESrecordID = PATTERNNAMESrecordID;
    PATTERNS.NODESrecordID = basePatternNodes[nI];
    PATTERNS.nodeSequence = (long)(nI + 1) * 10000L;
    memset(&PATTERNS.fromText, 0x00, PATTERNS_FROMTEXT_LENGTH);
    memset(&PATTERNS.toText, 0x00, PATTERNS_TOTEXT_LENGTH);
    memset(&PATTERNS.reserved, 0x00, PATTERNS_RESERVED_LENGTH);
    PATTERNS.flags = (bFirst ? PATTERNS_FLAG_MLP : 0);
    bFirst = FALSE;
    rcode2 = btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_271, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, rcode2);
      MessageBox(NULL, tempString, TMS, MB_OK);
      return(NO_RECORD);
    }
  }
  PINFO[numPatterns].PATTERNNAMESrecordID = PATTERNNAMESrecordID;
  memcpy(&PINFO[numPatterns].bInUse, &bInUse, sizeof(BOOL) * MAXPATTERNNODES);
  numPatterns++;

  return(PATTERNNAMESrecordID);
}

//
//  Create Connections
//
BOOL CreateConnections(long ROUTESrecordID, long SERVICESrecordID, char *inputString)
{
  BOOL bInUse[MAXPATTERNNODES];
  long CONNECTIONSrecordID;
  long tempLong;
  long firstTime;
  long secondTime;
  long hours;
  long minutes;
  int  nI;
  int  nJ;
  int  pos;
  int  rcode2;
//
//  Figure out where this thing goes through (patterns redux)
//
  pos = 36;
  for(nI = 0; nI < numBasePatternNodes; nI++)
  {
    bInUse[nI] = (strncmp(&inputString[pos], "  -  ", 5) != 0) ? TRUE : FALSE;
    pos += 6;
  }
//
//  Now parse out the times
//
  nI = 0;
  nJ = 1;
  while(nI < numBasePatternNodes - 1 && nJ < numBasePatternNodes)
  {
//
//  Find the first node
//
    if(!bInUse[nI])
    {
      nI++;
      continue;
    }
//
//  Find the second node
//
    for(nJ = nI + 1; nJ < numBasePatternNodes; nJ++)
    {
      if(bInUse[nJ])
        break;
    }
//
//  Leave if we've fallen of the end
//
    if(nJ >= numBasePatternNodes)
      break;
//
//  Get the times and convert them to seconds
//
//  First time
//
    tempLong = atol(&inputString[(nI * 6) + 36]);
    hours = (tempLong / 100);
    minutes = tempLong - (hours * 100);
    firstTime = (hours * 3600) + minutes * 60;
//
//  Second time
//
    tempLong = atol(&inputString[(nJ * 6) + 36]);
    hours = (tempLong / 100);
    minutes = tempLong - (hours * 100);
    secondTime = (hours * 3600) + minutes * 60;
//
//  Set up the Connections record
//
//
//  CONNECTIONS.recordID
//
    rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    CONNECTIONSrecordID = AssignRecID(rcode2, CONNECTIONS.recordID);
    memset(&CONNECTIONS, 0x00, sizeof(CONNECTIONSDef));
    CONNECTIONS.recordID = CONNECTIONSrecordID;
//
//  CONNECTIONS.COMMENTSrecordID
//
    CONNECTIONS.COMMENTSrecordID = NO_RECORD;
//
//  CONNECTIONS.from stuff
//
    CONNECTIONS.fromNODESrecordID = basePatternNodes[nI];
    CONNECTIONS.fromROUTESrecordID = ROUTESrecordID;
    CONNECTIONS.fromSERVICESrecordID = SERVICESrecordID;
    CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
//
//  CONNECTIONS.to stuff
//
    CONNECTIONS.toNODESrecordID = basePatternNodes[nJ];
    CONNECTIONS.toROUTESrecordID = ROUTESrecordID;
    CONNECTIONS.toSERVICESrecordID = SERVICESrecordID;
    CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
//
//  CONNECTIONS.connectionTime
//
    CONNECTIONS.connectionTime = secondTime - firstTime;
//
//  CONNECTIONS.fromTimeOfDay and toTimeOfDay
//
    if(firstTime > 86400)
      firstTime -= 86400;
    if(secondTime > 86400)
      secondTime -= 86400;
    CONNECTIONS.fromTimeOfDay = firstTime;
    CONNECTIONS.toTimeOfDay = secondTime;
//
//  CONNECTIONS.distance
//
    CONNECTIONS.distance = 0.0;
//
//  CONNECTIONS.flags
//
    CONNECTIONS.flags = CONNECTIONS_FLAG_RUNNINGTIME;
//
//  CONNECTIONS.ROUTINGSrecordID
//
    CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
//
//  CONNECTIONS Insert
//
    rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_277, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, rcode2);
      MessageBox(NULL, tempString, TMS, MB_OK);
      return(FALSE);
    }
//
//  Get the next pair
//
    nI++;
  }
  return(TRUE);
}

//
//  ATC Division
//
BOOL ATCDivision(char *division)
{
  long divnum;
  int  nI;

  divnum = atol(division);
  for(nI = 0; nI < numDivisions; nI++)
  {
    if(divnum == divisionList[nI])
      return(TRUE);
  }
  return(FALSE);
}

//
//  Condense Connections
//
BOOL CondenseConnections(void)
{
  CINFODef CINFO[250];
  int  numConn;
  int  rcode2;
  long fromNode;
  long toNode;
  long saveRecID;
  char serviceName[SERVICES_NAME_LENGTH + 1];
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char routeName[ROUTES_NAME_LENGTH + 1];

  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
      return(FALSE);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
        return(FALSE);
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
      sprintf(tempString, "Condensing Connections - %s\n%s - %s",
            serviceName, routeNumber, routeName);
      StatusBarText(tempString);
      rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      while(rcode2 == 0)
      {
        fromNode = CONNECTIONS.fromNODESrecordID;
        toNode = CONNECTIONS.toNODESrecordID;
        numConn = 0;
        while(rcode2 == 0 &&
              fromNode == CONNECTIONS.fromNODESrecordID &&
              toNode == CONNECTIONS.toNODESrecordID)
        {
          if(CONNECTIONS.fromSERVICESrecordID == SERVICES.recordID &&
                CONNECTIONS.fromROUTESrecordID == ROUTES.recordID)
          {
            CINFO[numConn].recordID = CONNECTIONS.recordID;
            CINFO[numConn].fromNode = CONNECTIONS.fromNODESrecordID;
            CINFO[numConn].toNode = CONNECTIONS.toNODESrecordID;
            CINFO[numConn].fromTime = CONNECTIONS.fromTimeOfDay;
            CINFO[numConn].toTime = CONNECTIONS.toTimeOfDay;
            CINFO[numConn].connectionTime = CONNECTIONS.connectionTime;
            numConn++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
        }
        if(rcode2 == 0)
        {
          saveRecID = CONNECTIONS.recordID;
          CondensePass2(numConn, &CINFO[0]);
          CONNECTIONSKey0.recordID = saveRecID;
          rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          if(rcode2 == 0)
          {
            btrieve(B_GETPOSITION, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            btrieve(B_GETDIRECT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
          }
        }
      }
      CondensePass2(numConn, &CINFO[0]);
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
  return(TRUE);
}

void CondensePass2(int numConn, CINFODef *pCINFO)
{
  UNIQUEDef UNIQUE[250];
  long numUnique;
  int  nI;
  int  nJ;
  int  rcode2;
  BOOL bFound;
  long most;
  int  index;
  BOOL bDoneGeneral;

  nI = 0;
  nJ = 1;
//
//  Kill the adjacent ones
//
  while(nI < numConn - 1 && nJ < numConn)
  {
    if(pCINFO[nI].connectionTime == pCINFO[nJ].connectionTime)
    {
      CONNECTIONSKey0.recordID = pCINFO[nI].recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      CONNECTIONS.toTimeOfDay = pCINFO[nJ].toTime;
      rcode2 = btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      pCINFO[nJ].connectionTime = NO_TIME;
      nJ++;
    }
    else
    {
      nI = nJ;
      nJ = nI + 1;
    }
  }
//
//  Determine the general case
//
  numUnique = 0;
  for(nI = 0; nI < numConn; nI++)
  {
    if(pCINFO[nI].connectionTime == NO_TIME)
      continue;
    bFound = FALSE;
    for(nJ = 0; nJ < numUnique; nJ++)
    {
      if(UNIQUE[nJ].connectionTime == pCINFO[nI].connectionTime)
      {
        bFound = TRUE;
        UNIQUE[nJ].numTimes = 1;
      }
    }
    if(!bFound)
    {
      UNIQUE[numUnique].connectionTime = pCINFO[nI].connectionTime;
      UNIQUE[numUnique].numTimes = 1;
      numUnique++;
    }
  }
  most = 0;
  index = NO_RECORD;
  for(nI = 0; nI < numUnique; nI++)
  {
    if(UNIQUE[nI].numTimes > most)
    {
      most = UNIQUE[nI].numTimes;
      index = nI;
    }
  }
  bDoneGeneral = FALSE;
  for(nI = 0; nI < numConn; nI++)
  {
    CONNECTIONSKey0.recordID = pCINFO[nI].recordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    if(rcode2 != 0)
      continue;
    if(pCINFO[nI].connectionTime == NO_TIME)
    {
      btrieve(B_DELETE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    }
    else if(pCINFO[nI].connectionTime == UNIQUE[index].connectionTime)
    {
      if(!bDoneGeneral)
      {
        CONNECTIONS.fromTimeOfDay = NO_TIME;
        CONNECTIONS.toTimeOfDay = NO_TIME;
        btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        bDoneGeneral = TRUE;
      }
      else
      {
        btrieve(B_DELETE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
    }
  }
}