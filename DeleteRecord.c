//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL DeleteRecord(int fileID, long recordID, BOOL cascade, BOOL firstTime, HWND hWnd)
{
  DATABASEDef record;
  TMSKey0Def key0;
  int        rcode2;
  long       ROUTErecordID;
  long       SERVICErecordID;
  long       directionIndex;
  long       PATTERNNAMESrecordID;
  BOOL       noWarning;
  BOOL       found;
  BOOL       bChanged;
  int        nI, nJ;
  long       fromNodeRecordID;
  long       toNodeRecordID;
  char       others[256];
  char       *tableNames[] = {TABLENAME_FILE, TABLENAME_FIELD, TABLENAME_INDEX,
        TABLENAME_ATTRIBUTES, TABLENAME_CREATED, TABLENAME_TABLEVIEW, TABLENAME_DIRECTIONS,
        TABLENAME_ROUTES, TABLENAME_SERVICES, TABLENAME_JURISDICTIONS, TABLENAME_NODES,
        TABLENAME_DIVISIONS, TABLENAME_PATTERNS, TABLENAME_PATTERNNAMES, TABLENAME_CONNECTIONS,
        TABLENAME_BUSTYPES, TABLENAME_COMMENTS, TABLENAME_TRIPS, TABLENAME_RUNS,
        TABLENAME_ROSTER};

//
//  firstTime means that he gets warned of the implications of deleting this record
//
  if(firstTime)
  {
    noWarning = FALSE;
    if(fileID == TMS_DIRECTIONS)
      LoadString(hInst, ERROR_112, others, sizeof(others));
    else if(fileID == TMS_ROUTES || fileID == TMS_SERVICES)
      LoadString(hInst, ERROR_113, others, sizeof(others));
    else if(fileID == TMS_DIVISIONS)
      LoadString(hInst, ERROR_114, others, sizeof(others));
    else if(fileID == TMS_NODES)
      LoadString(hInst, ERROR_115, others, sizeof(others));
    else if(fileID == TMS_PATTERNS)
      LoadString(hInst, ERROR_116, others, sizeof(others));
    else if(fileID == TMS_COMMENTS)
      LoadString(hInst, ERROR_172, szarString, sizeof(szarString));
    else if(fileID == TMS_RUNS)
      LoadString(hInst, ERROR_122, others, sizeof(others));
    else
      noWarning = TRUE;
    if(!noWarning)
    {
      MessageBeep(MB_ICONQUESTION);
      if(fileID != TMS_COMMENTS)
      {
        LoadString(hInst, (fileID == TMS_TRIPS || TMS_RUNS ? ERROR_118 : ERROR_111),
              tempString, TEMPSTRING_LENGTH);
        sprintf(szarString, tempString, tableNames[fileID], others);
      }
      if(MessageBox(hWnd, szarString, TMS, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
        return(FALSE);
    }
  }
//
//  Cascade means that this delete will examine other files and
//  either delete records there, or set the appropriate field to NO_RECORD
//
  if(cascade)
  {
//
//  Check for dependencies
//
//  DIRECTIONS: On ROUTES - The route goes
//
    if(fileID == TMS_DIRECTIONS)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      while(rcode2 == 0)
      {
        if(ROUTES.DIRECTIONSrecordID[0] == recordID || ROUTES.DIRECTIONSrecordID[1] == recordID)
          DeleteRecord(TMS_ROUTES, ROUTES.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      }
    }
//
//  ROUTES: On PATTERNS - The whole pattern goes
//             CONNECTIONS - The connection entry goes
//             TRIPS - The trip goes
//
    else if(fileID == TMS_ROUTES)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      while(rcode2 == 0)
      {
        if(PATTERNS.ROUTESrecordID == recordID)
          DeleteRecord(TMS_PATTERNS, PATTERNS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
      rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(CONNECTIONS.fromROUTESrecordID == recordID || CONNECTIONS.toROUTESrecordID == recordID)
          DeleteRecord(TMS_CONNECTIONS, CONNECTIONS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      while(rcode2 == 0)
      {
        if(TRIPS.ROUTESrecordID == recordID ||
               TRIPS.standard.RGRPROUTESrecordID == recordID ||
               TRIPS.dropback.RGRPROUTESrecordID == recordID)
          DeleteRecord(TMS_TRIPS, TRIPS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      }
    }
//
//  SERVICES: On PATTERNS - The whole pattern goes
//               CONNECTIONS - The connection entry goes
//               TRIPS - The trip goes
//
    else if(fileID == TMS_SERVICES)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      while(rcode2 == 0)
      {
        if(PATTERNS.SERVICESrecordID == recordID)
          DeleteRecord(TMS_PATTERNS, PATTERNS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
      rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(CONNECTIONS.fromSERVICESrecordID == recordID ||
              CONNECTIONS.toSERVICESrecordID == recordID)
          DeleteRecord(TMS_CONNECTIONS, CONNECTIONS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      while(rcode2 == 0)
      {
        if(TRIPS.SERVICESrecordID == recordID ||
              TRIPS.standard.SGRPSERVICESrecordID == recordID ||
              TRIPS.dropback.SGRPSERVICESrecordID == recordID)
          DeleteRecord(TMS_TRIPS, TRIPS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      }
    }
//
//  DIVISIONS: On RUNS   - The run is deleted
//                ROSTER - The run is deleted from the roster
//
    else if(fileID == TMS_DIVISIONS)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      while(rcode2 == 0)
      {
        if(RUNS.DIVISIONSrecordID == recordID)
        {
          btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          while(rcode2 == 0)
          {
            for(bChanged = FALSE, nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
            {
              for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
              {
                if(ROSTER.WEEK[nI].RUNSrecordIDs[nJ] == RUNS.recordID)
                {
                  ROSTER.WEEK[nI].RUNSrecordIDs[nJ] = NO_RECORD;
                  bChanged = TRUE;
                }
              }
            }
            if(bChanged)
            {
              btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
            }
            rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      }
    }
//
//  JURISDICTIONS: On NODES - NODES.JURISDICTIONSrecordID is set to NO_RECORD
//
    else if(fileID == TMS_JURISDICTIONS)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
      while(rcode2 == 0)
      {
        if(NODES.JURISDICTIONSrecordID == recordID)
        {
          NODES.JURISDICTIONSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
    }
//
//  NODES: On PATTERNS - The pattern record pertaining to this node goes
//            CONNECTIONS - The connection entry goes
//            TRIPS - If this node is a garage, the pull-in or pull-out goes
//
    else if(fileID == TMS_NODES)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      while(rcode2 == 0)
      {
        if(PATTERNS.NODESrecordID == recordID)
          DeleteRecord(TMS_PATTERNS, PATTERNS.recordID, PATTERNS.flags & PATTERNS_FLAG_MLP, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
      rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(CONNECTIONS.fromNODESrecordID == recordID ||
              CONNECTIONS.toNODESrecordID == recordID)
          DeleteRecord(TMS_CONNECTIONS, CONNECTIONS.recordID, TRUE, FALSE, hWnd);
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      if(NODES.flags & NODES_FLAG_GARAGE)
      {
        rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        while(rcode2 == 0)
        {
          if(TRIPS.standard.POGNODESrecordID == NODES.recordID)
            TRIPS.standard.POGNODESrecordID = NO_RECORD;
          if(TRIPS.standard.PIGNODESrecordID == NODES.recordID)
            TRIPS.standard.PIGNODESrecordID = NO_RECORD;
          if(TRIPS.dropback.POGNODESrecordID == NODES.recordID)
            TRIPS.dropback.POGNODESrecordID = NO_RECORD;
          if(TRIPS.dropback.PIGNODESrecordID == NODES.recordID)
            TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        }
      }
    }
//
//  PATTERNS: On CONNECTIONS - The connection entry goes
//            On TRIPS - The trip goes
//
//  There are special circumstances here.  We simply can't key on the pattern name
//  when deleting out of connections and trips, so we have to match on route and service
//  as well.  Also, if we're deleting the BASE pattern, all the sub-patterns go as well.
//
    else if(fileID == TMS_PATTERNS)
    {
      PATTERNSKey0.recordID = recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      if(rcode2 == 0)
      {
        ROUTErecordID = PATTERNS.ROUTESrecordID;
        SERVICErecordID = PATTERNS.SERVICESrecordID;
        directionIndex = PATTERNS.directionIndex;
        PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
        PATTERNSKey2.ROUTESrecordID = ROUTErecordID;
        PATTERNSKey2.SERVICESrecordID = SERVICErecordID;
        PATTERNSKey2.directionIndex = directionIndex;
        PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMESrecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        if(PATTERNNAMESrecordID == basePatternRecordID)
        {
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTErecordID &&
                PATTERNS.SERVICESrecordID == SERVICErecordID &&
                PATTERNS.directionIndex == directionIndex)
          {
            btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          TRIPSKey1.ROUTESrecordID = ROUTErecordID;
          TRIPSKey1.SERVICESrecordID = SERVICErecordID;
          TRIPSKey1.directionIndex = directionIndex;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTErecordID &&
                TRIPS.SERVICESrecordID == SERVICErecordID &&
                TRIPS.directionIndex == directionIndex)
          {
            DeleteRecord(TMS_TRIPS, TRIPS.recordID, TRUE, FALSE, hWnd);
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
        }
        else
        {
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTErecordID &&
                PATTERNS.SERVICESrecordID == SERVICErecordID &&
                PATTERNS.directionIndex == directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
          {
            btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          TRIPSKey1.ROUTESrecordID = ROUTErecordID;
          TRIPSKey1.SERVICESrecordID = SERVICErecordID;
          TRIPSKey1.directionIndex = directionIndex;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTErecordID &&
                TRIPS.SERVICESrecordID == SERVICErecordID &&
                TRIPS.directionIndex == directionIndex)
          {
            if(TRIPS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
              DeleteRecord(TMS_TRIPS, TRIPS.recordID, TRUE, FALSE, hWnd);
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
        }
        rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        if(PATTERNNAMESrecordID == basePatternRecordID)
        {
          while(rcode2 == 0)
          {
            if((CONNECTIONS.fromROUTESrecordID == ROUTErecordID &&
                        CONNECTIONS.fromSERVICESrecordID == SERVICErecordID &&
                        CONNECTIONS.fromPATTERNNAMESrecordID != NO_RECORD) ||
                  (CONNECTIONS.toPATTERNNAMESrecordID == PATTERNNAMESrecordID &&
                        CONNECTIONS.toSERVICESrecordID == SERVICErecordID &&
                        CONNECTIONS.toPATTERNNAMESrecordID != NO_RECORD))
              DeleteRecord(TMS_CONNECTIONS, CONNECTIONS.recordID, FALSE, FALSE, hWnd);
            rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
        else
        {
          while(rcode2 == 0)
          {
            if((CONNECTIONS.fromROUTESrecordID == ROUTErecordID &&
                      CONNECTIONS.fromSERVICESrecordID == SERVICErecordID &&
                      CONNECTIONS.fromPATTERNNAMESrecordID == PATTERNNAMESrecordID) ||
                (CONNECTIONS.toPATTERNNAMESrecordID == PATTERNNAMESrecordID &&
                      CONNECTIONS.toSERVICESrecordID == SERVICErecordID &&
                      CONNECTIONS.toPATTERNNAMESrecordID == PATTERNNAMESrecordID))
            DeleteRecord(TMS_CONNECTIONS, CONNECTIONS.recordID, FALSE, FALSE, hWnd);
            rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
          }
        }
      }
    }
//
//  BUSTYPES: On TRIPS - TRIPS.BUSTYPESrecordID is set to NO_RECORD
//
    else if(fileID == TMS_BUSTYPES)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      while(rcode2 == 0)
      {
        if(TRIPS.BUSTYPESrecordID == recordID)
        {
          TRIPS.BUSTYPESrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      }
    }
//
//  COMMENTS: On any file, COMMENTSrecordID is set to NO_RECORD
//
    else if(fileID == TMS_COMMENTS)
    {
      COMMENTSKey0.recordID = recordID;
      btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
      btrieve(B_DELETE, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
//
//  Update any direction records
//
      rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(DIRECTIONS.COMMENTSrecordID == recordID)
        {
          DIRECTIONS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      }
//
//  Update any route records
//
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      while(rcode2 == 0)
      {
        if(ROUTES.COMMENTSrecordID == recordID)
        {
          ROUTES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      }
//
//  Update any service records
//
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      while(rcode2 == 0)
      {
        if(SERVICES.COMMENTSrecordID == recordID)
        {
          SERVICES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      }
//
//  Update any jurisdiction records
//
      rcode2 = btrieve(B_GETFIRST, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(JURISDICTIONS.COMMENTSrecordID == recordID)
        {
          JURISDICTIONS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey0, 0);
      }
//
//  Update any node records
//
      rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
      while(rcode2 == 0)
      {
        if(NODES.COMMENTSrecordID == recordID)
        {
          NODES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
//
//  Update any division records
//
      rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
      while(rcode2 == 0)
      {
        if(DIVISIONS.COMMENTSrecordID == recordID)
        {
          DIVISIONS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
      }
//
//  Update any pattern records
//
      rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      while(rcode2 == 0)
      {
        if(PATTERNS.COMMENTSrecordID == recordID)
        {
          PATTERNS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
//
//  Update any pattern name records
//
      rcode2 = btrieve(B_GETFIRST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      while(rcode2 == 0)
      {
        if(PATTERNNAMES.COMMENTSrecordID == recordID)
        {
          PATTERNNAMES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      }
//
//  Update any bustype records
//
      rcode2 = btrieve(B_GETFIRST, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
      while(rcode2 == 0)
      {
        if(BUSTYPES.COMMENTSrecordID == recordID)
        {
          BUSTYPES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
      }
//
//  Update any trip records
//
      rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      while(rcode2 == 0)
      {
        if(TRIPS.COMMENTSrecordID == recordID)
        {
          TRIPS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      }
//
//  Update the run record(s)
//
      rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      while(rcode2 == 0)
      {
        if(RUNS.COMMENTSrecordID == recordID)
        {
          RUNS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      }
//
//  Update any roster records
//
      rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      while(rcode2 == 0)
      {
        if(ROSTER.COMMENTSrecordID == recordID)
        {
          ROSTER.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      }
//
//  Update any driver records
//
      rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      while(rcode2 == 0)
      {
        if(DRIVERS.COMMENTSrecordID == recordID)
        {
          DRIVERS.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      }
//
//  Update any driver types records
//
      rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
      while(rcode2 == 0)
      {
        if(DRIVERTYPES.COMMENTSrecordID == recordID)
        {
          DRIVERTYPES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
      }
//
//  Update any block names records
//
      rcode2 = btrieve(B_GETFIRST, TMS_BLOCKNAMES, &BLOCKNAMES, &BLOCKNAMESKey0, 0);
      while(rcode2 == 0)
      {
        if(BLOCKNAMES.COMMENTSrecordID == recordID)
        {
          BLOCKNAMES.COMMENTSrecordID = NO_RECORD;
          btrieve(B_UPDATE, TMS_BLOCKNAMES, &BLOCKNAMES, &BLOCKNAMESKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_BLOCKNAMES, &BLOCKNAMES, &BLOCKNAMESKey0, 0);
      }
    }
//
//  CONNECTIONS: Check to see if the CONN structure has to be adjusted if
//               an entry recorded there is about to be blown away.
//
    else if(fileID == TMS_CONNECTIONS)
    {
      for(found = FALSE, nI = 0; nI < CONN.numConnections; nI++)
        if((found = (recordID == CONN.startRecordID[nI])) == TRUE)
          break;
      if(found)
      {
        CONNECTIONSKey0.recordID = recordID;
        btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        fromNodeRecordID = CONNECTIONS.fromNODESrecordID;
        toNodeRecordID = CONNECTIONS.toNODESrecordID;
        btrieve(B_GETPOSITION, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        btrieve(B_GETDIRECT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
        if(rcode2 == 0 &&
              CONNECTIONS.fromNODESrecordID == fromNodeRecordID &&
              CONNECTIONS.toNODESrecordID == toNodeRecordID)
          CONN.startRecordID[nI] = CONNECTIONS.recordID;
      }
    }
  }
//
//  Nuke the record in question.
//
//  In the case of patterns and comments, the record may already be toast.
//
  key0.recordID = recordID;
  if(btrieve(B_GETEQUAL, fileID, &record, &key0, 0) == 0)
    btrieve(B_DELETE, fileID, &record, &key0, 0);

  return(TRUE);
}
