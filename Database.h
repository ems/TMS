//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "btrieve.h"


// Since these are database structures, make sure they are packed with
// no padding.
#pragma pack( push, packDefault )
#pragma pack(1)

//
//  DISPLAYINFO Legacy stuff
//

#define MAXFIELDS                  100

typedef struct FIELDSStruct
{
  long numFields;                                           // number of fields
  long sortFieldId;                                         // field Id to sort table on
  long cNumPatternNodes;                                    // number of node columns for patterns and trips
  long firstNodeColumn;                                     // first column containing a node (pat & trip)
  long cNumFlagFields;                                      // number of flag columns
  long firstFlagColumn;                                     // first column containing a flag
  long fieldId[MAXFIELDS];                                  // Id from FIELD.DDF
  long offset[MAXFIELDS];                                   // offset in database file
  long flags[MAXFIELDS];                                    // value processing flags
  long linkFile[MAXFIELDS];                                 // dependent file number
  long linkOffset[MAXFIELDS];                               // dependent field offset
  long numBytes[MAXFIELDS];                                 // size of field in database file
  char columnTitles[MAXFIELDS][DDF_NAME_LENGTH + 1];        // displayed field name
  long dataType[MAXFIELDS];                                 // displayed data type
  long numChars[MAXFIELDS];                                 // number of characters to display
  long recordID[MAXFIELDS];                                 // pointers to node names
  long basePatternSeq[MAXFIELDS];                           // ptrs to node position in pat
} FIELDINFO;

#define FILE_CHECKRSDD          0x0001
#define FILE_ALLBLOCKS          0x0002
#define FILE_GETBLOCKS          0x0004

typedef struct FILEStruct
{
  long routeRecordID;
  long serviceRecordID;
  long divisionRecordID;
  long directionIndex;
  long rosterWeek;
  int  fileNumber;
  int  keyNumber;
  long position;
  int  flags;
} FILEINFO;

typedef struct DISPLAYStruct
{
  HGLOBAL    hMem;
  HGLOBAL    hMemF1;
  char       subWindow[2];
  HWND       hWndOther;
  HWND       hWndAspect;
  FIELDINFO  fieldInfo;
  FILEINFO   fileInfo;
  long       baggage;
  long       otherData;
} DISPLAYINFO;

typedef DISPLAYINFO *PDISPLAYINFO;

//
//  Connection Record IDs - used by pickconn and
//  and position to figure out what to display
//

#define RECORDIDS_KEPT 50
#define MAXCONNECTIONSKEPT 20000
typedef struct CONNStruct
{
  long currentFromNODESrecordID;
  long currentToNODESrecordID;
  long currentConnection;
  long startRecordID[MAXCONNECTIONSKEPT];
  int  numConnections;
  char flags;
} CONNDef;

EXTERN CONNDef CONN;

//
//  Defines and Structures for GenerateTrip and GetConnectionTime
//
#define GCT_FLAG_RUNNINGTIME    0x0001
#define GCT_FLAG_DEADHEADTIME   0x0002
#define GCT_FLAG_TRAVELTIME     0x0004
#define GCT_FLAG_ATLEASTONESTOP 0x0008


typedef struct GetConnectionTimeStruct
{
  long fromNODESrecordID;
  long fromROUTESrecordID;
  long fromSERVICESrecordID;
  long fromPATTERNNAMESrecordID;
  long toNODESrecordID;
  long toROUTESrecordID;
  long toSERVICESrecordID;
  long toPATTERNNAMESrecordID;
  long timeOfDay;
} GetConnectionTimeDef;

//EXTERN GTDef GT;

//
//  Defines and Structures for FindTrip
//

typedef struct PICKEDTRIPSStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
} PICKEDTRIPSDef;

EXTERN PICKEDTRIPSDef PT;
EXTERN int previewSelection;

//
//  Defines and Structures for FindBlock
//

typedef struct PICKEDBLOCKSStruct
{
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long NODESrecordID;
  long flags;
} PICKEDBLOCKSDef;

//
//  Runtimes
//

#define MAXCONNECTIONS 20000
EXTERN CONNECTIONSDef *m_pRUNTIMES;
EXTERN BOOL  m_bEstablishRUNTIMES;
EXTERN int   m_numRUNTIMES;

//
//  Globals for the current route number and name, service name and number, and division name and number
//

EXTERN char m_RouteNumber[ROUTES_NUMBER_LENGTH + 1];
EXTERN char m_RouteName[ROUTES_NAME_LENGTH + 1];
EXTERN char m_ServiceName[SERVICES_NAME_LENGTH + 1];
EXTERN long m_ServiceNumber;
EXTERN char m_DivisionName[DIVISIONS_NAME_LENGTH + 1];
EXTERN long m_DivisionNumber;
//
// DATATYPES - Taken from the Btrieve definitions
//

#define DATATYPE_CHARACTER           0
#define DATATYPE_INTEGER             1
#define DATATYPE_FLOAT               2
#define DATATYPE_DATE                3
#define DATATYPE_TIME                4
#define DATATYPE_DECIMAL             5
#define DATATYPE_MONEY               6
#define DATATYPE_LOGICAL             7
#define DATATYPE_NUMERIC             8
#define DATATYPE_BFLOAT              9
#define DATATYPE_LSTRING            10
#define DATATYPE_ZSTRING            11
#define DATATYPE_NOTE               12
#define DATATYPE_LVAR               13
#define DATATYPE_AUTOINC            15
#define DATATYPE_BIT                16

//
//  TMS DATATYPE extensions
//

#define TMSDATA_TIMEOFDAY             0
#define TMSDATA_TIMEINMINUTES         1
#define TMSDATA_TIMEINHM              2
#define TMSDATA_CHARACTER             3
#define TMSDATA_INTEGER               4
#define TMSDATA_FLOAT                 5
#define TMSDATA_LOGICAL               6
#define TMSDATA_PATTERNCHECKS         7
#define TMSDATA_PATTERNTIMES          8
#define TMSDATA_PULLOUTTIME           9
#define TMSDATA_FIRSTNODE            10
#define TMSDATA_FIRSTTIME            11
#define TMSDATA_LASTTIME             12
#define TMSDATA_LASTNODE             13
#define TMSDATA_PULLINTIME           14
#define TMSDATA_TRIPTIME             15
#define TMSDATA_LAYOVERTIME          16
#define TMSDATA_DEADHEADTIME         17
#define TMSDATA_RUNTYPE              18
#define TMSDATA_BLOCKNUMBER          19
#define TMSDATA_RUNONTIME            20
#define TMSDATA_RUNOFFTIME           21
#define TMSDATA_PLATFORMTIME         22
#define TMSDATA_REPORTTIME           23
#define TMSDATA_TURNINTIME           24
#define TMSDATA_REPORTTURNIN         25
#define TMSDATA_TRAVELTIME           26
#define TMSDATA_SPREADOT             27
#define TMSDATA_MAKEUPTIME           28
#define TMSDATA_OVERTIME             29
#define TMSDATA_PREMIUMTIME          30
#define TMSDATA_PAYTIME              31
#define TMSDATA_BARGRAPH             32
#define TMSDATA_FIXEDFOUR            33
#define TMSDATA_TRIPDISTANCE         34
#define TMSDATA_CUMULATIVEDISTANCE   35
#define TMSDATA_ROSTERDAY1           36
#define TMSDATA_ROSTERDAY2           37
#define TMSDATA_ROSTERDAY3           38
#define TMSDATA_ROSTERDAY4           39
#define TMSDATA_ROSTERDAY5           40
#define TMSDATA_ROSTERDAY6           41
#define TMSDATA_ROSTERDAY7           42
#define TMSDATA_ROSTERHOURS          43
#define TMSDATA_ROSTEROT             44
#define TMSDATA_ROSTERPAY            45
#define TMSDATA_MEAL                 46
#define TMSDATA_FLAGS                47
#define TMSDATA_DATE                 48
#define TMSDATA_ROSTERDRIVER         49
#define TMSDATA_PAIDBREAKS           50
#define TMSDATA_STARTOFPIECETRAVEL   51
#define TMSDATA_ENDOFPIECETRAVEL     52
#define TMSDATA_REPORTATTIME         53
#define TMSDATA_REPORTATLOCATION     54
#define TMSDATA_BOPEXTRABOARDSTART   55
#define TMSDATA_BOPEXTRABOARDEND     56
#define TMSDATA_EOPEXTRABOARDSTART   57
#define TMSDATA_EOPEXTRABOARDEND     58
#define TMSDATA_TIMEINDECIMALMINUTES 59
#define TMSDATA_PIECESTARTLOCATION   60
#define TMSDATA_PIECEENDLOCATION     61
#define TMSDATA_SIGNOFFTIME          62
#define TMSDATA_DAYOFTIMECHECK       63
#define TMSDATA_ROUTEDIRECTION       64
#define TMSDATA_SCHEDULEDTIME        65
#define TMSDATA_SUPERVISOR           66
#define TMSDATA_PASSENGERLOAD        67
#define TMSDATA_WEATHER              68
#define TMSDATA_OTHERCOMMENTS        69
#define TMSDATA_TRAFFIC              70
#define TMSDATA_ACTSCHEDDIFF         71 
#define TMSDATA_SPREADTIME           72
#define TMSDATA_ROSTERPAYNOOT        73
#define TMSDATA_SERVICEOFTRIPONRUN   74
#define TMSDATA_LAST                 74
#define NUMBER_OF_TMSDATA_TYPES      TMSDATA_LAST + 1

#define TMSDATA_TIMEOFDAY_TEXT            "Time of day"
#define TMSDATA_TIMEINMINUTES_TEXT        "Time in minutes"
#define TMSDATA_TIMEINHM_TEXT             "Time in hours:minutes"
#define TMSDATA_CHARACTER_TEXT            "Character"
#define TMSDATA_INTEGER_TEXT              "Integer"
#define TMSDATA_FLOAT_TEXT                "Float"
#define TMSDATA_LOGICAL_TEXT              "Logical"
#define TMSDATA_PATTERNCHECKS_TEXT        "Pattern checkmarks"
#define TMSDATA_PATTERNNODES_TEXT         "Time at pattern nodes"
#define TMSDATA_PULLOUTTIME_TEXT          "Pull-out time"
#define TMSDATA_FIRSTNODE_TEXT            "First node of a trip"
#define TMSDATA_FIRSTTIME_TEXT            "First time on a trip"
#define TMSDATA_LASTTIME_TEXT             "Last time on a trip"
#define TMSDATA_LASTNODE_TEXT             "Last node on a trip"
#define TMSDATA_PULLINTIME_TEXT           "Pull-in time"
#define TMSDATA_TRIPTIME_TEXT             "Total trip time"
#define TMSDATA_LAYOVERTIME_TEXT          "Layover until next trip"
#define TMSDATA_DEADHEADTIME_TEXT         "Deadhead to next trip"
#define TMSDATA_RUNTYPE_TEXT              "Runtype"
#define TMSDATA_BLOCKNUMBER_TEXT          "Block number"
#define TMSDATA_RUNONTIME_TEXT            "On time of a run"
#define TMSDATA_RUNOFFTIME_TEXT           "Off time of a run"
#define TMSDATA_PLATFORMTIME_TEXT         "Platform time"
#define TMSDATA_REPORTTIME_TEXT           "Report time (in minutes)"
#define TMSDATA_TURNINTIME_TEXT           "Turn-in time"
#define TMSDATA_REPORTTURNIN_TEXT         "Report + turn-in times"
#define TMSDATA_TRAVELTIME_TEXT           "Travel time (piece total)"
#define TMSDATA_SPREADOT_TEXT             "Spread time premium"
#define TMSDATA_MAKEUPTIME_TEXT           "Make-up time"
#define TMSDATA_OVERTIME_TEXT             "Overtime"
#define TMSDATA_PREMIUMTIME_TEXT          "Premium time"
#define TMSDATA_PAYTIME_TEXT              "Total pay time"
#define TMSDATA_BARGRAPH_TEXT             "Bar graph"
#define TMSDATA_FIXEDFOUR_TEXT            "Fixed decimal 4"
#define TMSDATA_TRIPDISTANCE_TEXT         "Trip distance"
#define TMSDATA_CUMULATIVEDISTANCE_TEXT   "Cumulative distance"
#define TMSDATA_ROSTERDAY1_TEXT           "Roster day 1"
#define TMSDATA_ROSTERDAY2_TEXT           "Roster day 2"
#define TMSDATA_ROSTERDAY3_TEXT           "Roster day 3"
#define TMSDATA_ROSTERDAY4_TEXT           "Roster day 4"
#define TMSDATA_ROSTERDAY5_TEXT           "Roster day 5"
#define TMSDATA_ROSTERDAY6_TEXT           "Roster day 6"
#define TMSDATA_ROSTERDAY7_TEXT           "Roster day 7"
#define TMSDATA_ROSTERHOURS_TEXT          "Roster hours"
#define TMSDATA_ROSTEROT_TEXT             "Roster overtime"
#define TMSDATA_ROSTERPAY_TEXT            "Roster pay time"
#define TMSDATA_MEAL_TEXT                 "Meal"
#define TMSDATA_FLAGS_TEXT                "Flags on a record"
#define TMSDATA_DATE_TEXT                 "Date (yyyy/mm/dd)"
#define TMSDATA_ROSTERDRIVER_TEXT         "Roster driver"
#define TMSDATA_PAIDBREAKS_TEXT           "Paid breaks"
#define TMSDATA_STARTOFPIECETRAVEL_TEXT   "Travel time (Start of piece)"
#define TMSDATA_ENDOFPIECETRAVEL_TEXT     "Travel time (End of piece)"
#define TMSDATA_REPORTATTIME_TEXT         "Report at time"
#define TMSDATA_REPORTATLOCATION_TEXT     "Report at location"
#define TMSDATA_BOPEXTRABOARDSTART_TEXT   "Extraboard Start Time - Start of Piece"
#define TMSDATA_BOPEXTRABOARDEND_TEXT     "Extraboard End Time - Start of Piece"
#define TMSDATA_EOPEXTRABOARDSTART_TEXT   "Extraboard Start Time - End of Piece"
#define TMSDATA_EOPEXTRABOARDEND_TEXT     "Extraboard End Time - End of Piece"
#define TMSDATA_TIMEINDECIMALMINUTES_TEXT "Pay time in hours and decimal minutes"
#define TMSDATA_PIECESTARTLOCATION_TEXT   "Piece start location"
#define TMSDATA_PIECEENDLOCATION_TEXT     "Piece end location"
#define TMSDATA_SIGNOFFTIME_TEXT          "Sign-off time"
#define TMSDATA_DAYOFTIMECHECK_TEXT       "Day of Time Check"
#define TMSDATA_ROUTEDIRECTION_TEXT       "Route direction"
#define TMSDATA_SCHEDULEDTIME_TEXT        "Scheduled time at a node"
#define TMSDATA_SUPERVISOR_TEXT           "Supervisor"
#define TMSDATA_PASSENGERLOAD_TEXT        "Passenger load"
#define TMSDATA_WEATHER_TEXT              "Weather conditions"
#define TMSDATA_OTHERCOMMENTS_TEXT        "Other comments"
#define TMSDATA_TRAFFIC_TEXT              "Traffic conditions"
#define TMSDATA_ACTSCHEDDIFF_TEXT         "Difference between actual and scheduled times"
#define TMSDATA_SPREADTIME_TEXT           "Spread Time"
#define TMSDATA_ROSTERPAYNOOT_TEXT        "Roster pay time w/o daily overtime"
#define TMSDATA_SERVICEOFTRIPONRUN_TEXT   "Service of a trip on a run"



EXTERN int nodeDisplayWidth;

//
//  Structure for GenerateTrip
//
#define MAXTRIPTIMES 256

#define GENERATETRIP_FLAG_GETLABELS     0x0001
#define GENERATETRIP_FLAG_DISPLAYERRORS 0x0002
#define GENERATETRIP_FLAG_DISTANCECALC  0x0004

typedef struct GenerateTripStruct
{
  long  tripTimes[MAXTRIPTIMES];
  long  firstNODESrecordID;
  long  firstNodeTime;
  long  lastNODESrecordID;
  long  lastNodeTime;
  float tripDistance;
  long  labelledNODESrecordIDs[MAXTRIPTIMES];
  BOOL  bNodeIsAGarage[MAXTRIPTIMES];
  long  returnFlags;
} GenerateTripDef;


typedef struct RTCALCStruct
{
  long fromNODErecordID;
  long fromROUTErecordID;
  long fromSERVICErecordID;
  long toNODErecordID;
  long toROUTErecordID;
  long toSERVICErecordID;
} RTCALCDef;

EXTERN RTCALCDef RTCALC;

//
//  Structures for blocker input
//

EXTERN PICKEDTRIPSDef *m_pPICKEDTRIPS;
EXTERN int m_maxPICKEDTRIPS;
EXTERN int m_numPICKEDTRIPS;

typedef struct TRIPINFOStruct
{
  long TRIPSrecordID;
  long startTime;
  long startNODESrecordID;
  long endTime;
  long endNODESrecordID;
  long layoverMin;
  long layoverMax;
  long BUSTYPESrecordID;
  long ROUTESrecordID;
} TRIPINFODef;

EXTERN int m_numTripsForBlocker;

typedef struct BLOCKSDATAStruct
{
  long blockNumber;
  long TRIPSrecordID;
} BLOCKSDATADef;

EXTERN BLOCKSDATADef *m_pBLOCKSDATA;

typedef struct DEADHEADINFOStruct
{
  long fromTimeOfDay;
  long fromNODESrecordID;
  long toTimeOfDay;
  long toNODESrecordID;
  long deadheadTime;
  long flags;
} DEADHEADINFODef;

EXTERN int numDeadheadsForBlocker;

//
//  Structure for runcutter input
//

#define PICKEDBLOCKS_FLAG_STANDARD 0x0001
#define PICKEDBLOCKS_FLAG_DROPBACK 0x0002

EXTERN PICKEDBLOCKSDef *m_pPICKEDBLOCKS;
EXTERN int              m_numPICKEDBLOCKS;
EXTERN int              m_maxPICKEDBLOCKS;

//
//  Structure for derived data
//

typedef struct DERVIEDStruct
{
  char tableName[DDF_NAME_LENGTH + 1];
  char fieldName[DDF_NAME_LENGTH + 1];
  int  joinedToTableId;
  int  joinedToFieldId;
} DERIVEDDef;

EXTERN DERIVEDDef DERIVED;

//
//  Structure for formatted data
//

typedef struct FORMATTEDStruct
{
  char tableName[DDF_NAME_LENGTH + 1];
  char fieldName[DDF_NAME_LENGTH + 1];
  int  dataType;
} FORMATTEDDef;

EXTERN FORMATTEDDef FORMATTED;

//
//  BLOCKDATA structure for use by mancut
//

typedef struct BLOCKDATAStruct
{
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long blockNumber;
  long firstTRIPSrecordID;
  long onLocNODESrecordID;
  long onTime;
  long lastTRIPSrecordID;
  long offTime;
  long offLocNODESrecordID;
  int  blocksBoxPosition;
  long flags;
} BLOCKDATADef;

EXTERN BLOCKDATADef *m_pBLOCKDATA;
EXTERN int           m_numBLOCKDATA;
EXTERN int           m_maxBLOCKDATA;

//
//  COST structure for use by runcutter - here so prototyp.h knows what COST means
//

#define MAXPIECES     15

typedef struct PIECECOSTStruct
{
  long platformTime;
  long dwellTime;
  long paidBreak;
  long paidMeal;
  long reportTime;
  long turninTime;
  long makeUpTime;
  long overTime;
  long premiumTime;
  long payTime;
} PIECECOSTDef;

#define TRAVEL_FLAGS_STARTPAID 0x0001
#define TRAVEL_FLAGS_ENDPAID   0x0002

typedef struct TRAVELStruct
{
  long startNODESrecordID;
  long startTravelTime;
  long startDwellTime;
  long startAPointNODESrecordID;
  long startAPointTime;
  long endBPointNODESrecordID;
  long endBPointTime;
  long endTravelTime;
  long endDwellTime;
  long endNODESrecordID;
  long flags;
} TRAVELDef;

typedef struct COSTStruct
{
  int  numPieces;
  long runtype;
  long spreadOvertime;
  long spreadTime;
  PIECECOSTDef TOTAL;
  PIECECOSTDef PIECECOST[MAXPIECES];
  TRAVELDef    TRAVEL[MAXPIECES];
} COSTDef;

typedef COSTDef *PCOSTDef;

//
//  PROPOSEDRUN structure for use by runcutter - here also for the benefit of prototyp.h
//

typedef struct PROPOSEDRUNPIECESStruct
{
  long fromTime;
  long fromNODESrecordID;
  long fromTRIPSrecordID;
  long toTime;
  long toNODESrecordID;
  long toTRIPSrecordID;
  EXTRABOARDDef prior;
  EXTRABOARDDef after;
  long flags;
} PROPOSEDRUNPIECESDef;

typedef struct PROPOSEDRUNStruct
{
  int  numPieces;
  PROPOSEDRUNPIECESDef piece[MAXPIECES];
} PROPOSEDRUNDef;

typedef PROPOSEDRUNDef PROPOSEDRUN;

#define MAXRUNSINRUNLIST 1000
typedef struct RUNLISTStruct
{
  long runNumber;
  PROPOSEDRUNDef run;
  long recordID[MAXPIECES];
} RUNLISTDef;

EXTERN RUNLISTDef  RUNLIST[MAXRUNSINRUNLIST];

typedef struct RUNLISTDATAStruct
{
  long runNumber;
  long cutAsRuntype;
  long COMMENTSrecordID;
  BOOL frozenFlag;
} RUNLISTDATADef;

EXTERN RUNLISTDATADef RUNLISTDATA[MAXRUNSINRUNLIST];
#ifdef FIXLATER
EXTERN RUNLISTDef *RUNLIST;
extern VARARRAYDef   *VARunListArray;
#endif  // FIXLATER

//
//  Rostering input
//

typedef struct RRDATAStruct
{
  long RUNSrecordID;
  long runNumber;
  long runtype;
  long RGRPROUTESrecordID;
  long onTime;
  long onNODESrecordID;
  long offTime;
  long offNODESrecordID;
  long payTime;
  long flags;
} RRDATADef;

typedef struct RRLISTStruct
{
  int  numRuns;
  int  maxRuns;
  long SERVICESrecordID;
  RRDATADef *pData;
} RRLISTDef;

EXTERN RRLISTDef *m_pRRLIST;
EXTERN BOOL       m_bAdjustRRLIST;

typedef struct COMBINEDStruct
{
  long COMMENTSrecordID;
  long DIVISIONSrecordID;
  long DRIVERSrecordID;
  long flags[7];
  long rosterNumber;
  long RUNSrecordID[7];
  long runNumber[7];
  BOOL frozen;
} COMBINEDDef;

EXTERN COMBINEDDef *m_pCOMBINED;
EXTERN long         m_numCOMBINED;
EXTERN long         m_maxCOMBINED;
EXTERN long GlobalRosterNumber;
EXTERN long RosterData[4];  // 0 - Hrs, 1 - OT, 2 - Pay, 3 - Flags: used in GetTMSData & Renderfield

//
//  Fun and games for displaying deadhead times
//

EXTERN BOOL deadheadTimeCalculated;
EXTERN long deadheadTimeValue;

//
//  Database update
//

EXTERN long updateRecordID;
EXTERN BOOL inboundUpdate;

//
//  Data view structures
//
//  Blocks
//

typedef struct BLOCKSVIEWStruct
{
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  pullOutTime;
  long  firstNodeRecordID;
  long  firstNodeTime;
  long  firstRouteRecordID;
  long  firstServiceRecordID;
  long  lastNodeTime;
  long  lastNodeRecordID;
  long  pullInTime;
  long  tripTime;
  long  layoverTime;
  long  deadheadTime;
  float tripDistance;
  float totalDistance;
} BLOCKSVIEWDef;

EXTERN BLOCKSVIEWDef BLOCKSVIEW;

typedef struct PREVBLOCKSVIEWStruct
{
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  firstNodeRecordID;
  long  firstNodeTime;
  long  lastNodeTime;
  long  lastNodeRecordID;
  long  lastRouteRecordID;
  long  lastServiceRecordID;
  float totalDistance;
} PREVBLOCKSVIEWDef;

EXTERN PREVBLOCKSVIEWDef PREVBLOCKSVIEW;

//
//  Runs
//

typedef struct RUNSVIEWStruct
{
  long runNumber;
  long runType;
  long blockNumber;
  long ROUTESrecordID;
  long runOnNODESrecordID;
  long runOnTRIPSrecordID;
  long runOnTime;
  long runOffNODESrecordID;
  long runOffTRIPSrecordID;
  long runOffTime;
  long platformTime;
  long reportTime;
  long turninTime;
  long reportTurninTime;
  long travelTime;
  long startOfPieceTravel;
  long startOfPieceNODESrecordID;
  long endOfPieceTravel;
  long endOfPieceNODESrecordID;
  long paidMeal;
  long paidBreaks;
  long spreadOT;
  long makeUpTime;
  long overTime;
  long premiumTime;
  long payTime;
  long flags;
  long startOfPieceExtraboardStart;
  long startOfPieceExtraboardEnd;
  long endOfPieceExtraboardStart;
  long endOfPieceExtraboardEnd;
  long spreadTime;
  long onTripSERVICESrecordID;
} RUNSVIEWDef;

EXTERN RUNSVIEWDef RUNSVIEW[20];

//
//  Roster
//

typedef struct ROSTERVIEWStruct
{
  long rosterDay[7];
  long rosterHours;
  long rosterOT;
  long rosterPay;
} ROSTERVIEWDef;

EXTERN ROSTERVIEWDef ROSTERVIEW;

//
//  Conversations between SETCOL and FIELDDEF
//

typedef struct FIELDDEFStruct
{
  BOOL bCreated;
  BOOL hidden;
  BOOL derived;
  BOOL formatted;
  int  Id;
  int  fromFile;
  int  fromField;
  char szFileName[DDF_NAME_LENGTH + 1];
  char szFieldName[DDF_NAME_LENGTH + 1];
  char szDisplayedName[DDF_NAME_LENGTH + 1];
  int  createdFromFileId;
  int  createdFromFieldId;
  int  sequence;
} FIELDDEFDef, *PFIELDDEFDef;

//
//  Comment text
//
EXTERN HGLOBAL hMemCommentText;
EXTERN char    *pCommentText;

//
//  Complaint text
//
EXTERN HGLOBAL hMemComplaintText;
EXTERN char    *pComplaintText;

//
//  Bidding
//
#define BIDDING_MAXRUNS      300
#define BIDDING_MAXSERVICES    7
#define BIDDING_NUMSERVICEDAYS 7

typedef struct BIDDINGRUNLISTStruct
{
  long RUNSrecordID;
  long runNumber;
  long runtype;
  BOOL bTaken;
} BIDDINGRUNLISTDef;

EXTERN BIDDINGRUNLISTDef BIDDINGRUNLIST[BIDDING_MAXRUNS][BIDDING_NUMSERVICEDAYS];

typedef struct EXTRABOARDEDITORPARAMStruct
{
  long runNumber;
  long pieceNumber;
  long availableTime;
  long pieceStart;
  long pieceEnd;
  long startStart;
  long startEnd;
  long endStart;
  long endEnd;
  long flags;
  long maxPieceSize;
} EXTRABOARDEDITORPARAMDef;

//
//  Placement flags
//
#define PLACEMENT_ROUTE     0x0001
#define PLACEMENT_SERVICE   0x0002
#define PLACEMENT_DIRECTION 0x0004
#define PLACEMENT_DIVISION  0x0008

//
//  Parameters for SUMMARIZEBLOCK
//
typedef struct SUMMARIZEBLOCKParameterStruct
{
  long blockNumber;
  int  numTrips;
  long assignedToNODESrecordID;
  long totalPlat;
  long totalLay;
  long totalILDhd;
  long totalGADhd;
} SUMMARIZEBLOCKParameterDef;

//
//  Node recordIDs and ABBRs for Runcutter
//
typedef struct NODEABBRSStruct
{
  long recordID;
  char abbrName[NODES_ABBRNAME_LENGTH + 1];
} NODEABBRSDef;

EXTERN NODEABBRSDef *m_pNodeAbbrs;
EXTERN int m_numNodeAbbrs;

//
//  Timecheck stuff
//

EXTERN long timecheckScheduled;
EXTERN long timecheckActual;

#define TIMECHECK_FILE_PASSENGERLOADS    0
#define TIMECHECK_FILE_WEATHERCONDITIONS 1
#define TIMECHECK_FILE_TRAFFICCONDITIONS 2
#define TIMECHECK_FILE_OTHERCOMMENTS     3
#define TIMECHECK_FILE_ROADSUPERVISORS   4

//
//  Filter parameters for the Daily Operations Audit Trail
//

#define AUDITFILTER_FLAGS_ALL                     0x0001
#define AUDITFILTER_FLAGS_ROSTER                  0x0002
#define AUDITFILTER_FLAGS_BUS                     0x0004
#define AUDITFILTER_FLAGS_OPERATOR                0x0008
#define AUDITFILTER_FLAGS_ABSENCE                 0x0010
#define AUDITFILTER_FLAGS_OPENWORK                0x0020
#define AUDITFILTER_FLAGS_ALLOPERATORS            0x0040
#define AUDITFILTER_FLAGS_ALLREASONS              0x0080
#define AUDITFILTER_FLAGS_REGISTRATIONDATE        0x0100

typedef struct AUDITFILTERSELECTIONSStruct
{
  long fromDate;
  long toDate;
  long flags;
  long DRIVERSrecordID;
  long reasonNumber;
} AUDITFILTERSELECTIONSDef;

//
//  Filter parameters for the Discipline Report
//
typedef struct DISCIPLINEFILTERSELECTIONSStruct
{
  long fromDate;
  long toDate;
} DISCIPLINEFILTERSELECTIONSDef;


// Reset the pack parameter to the default.
#pragma pack( pop, packDefault )

