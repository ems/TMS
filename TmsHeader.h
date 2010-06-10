//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#ifndef TmsHeader_H
#define TmsHeader_H

#ifndef EXTERN
#define EXTERN extern
#endif

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <memory.h>
#include <time.h>
#include <dos.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <commdlg.h>

#ifndef OTHER_PROJECT
  #include "resource.h"
#endif
//
//  Formula one specs
//
#include "F1ControlSpecs.h"
//
//  Help file defines
//
#include "Context.hh"
#include "TMS.hh"

//
// #defines
//
#define CONST_YES "Yes"
#define CONST_NO  "No"
#define NO_RECORD                -1
#define NO_TIME                  -32767
#define NO_POSITION              -1
#define SECURELOCATION_RUNNUMBER -2

#define TEXT_DATABASE_FILENAME "Common Text.accdb"

#define T1159P 86340

#define NUMDECS 4

#define NO_RUNTYPE           NO_TIME
#define UNCLASSIFIED_RUNTYPE NO_RECORD

#define FILE_DDF           0
#define FIELD_DDF          1
#define INDEX_DDF          2
#define TMS_ATTRIBUTES     3
#define TMS_CREATED        4
#define TMS_TABLEVIEW      5
#define TMS_DIRECTIONS     6
#define TMS_ROUTES         7
#define TMS_SERVICES       8
#define TMS_JURISDICTIONS  9
#define TMS_NODES         10
#define TMS_DIVISIONS     11
#define TMS_PATTERNS      12
#define TMS_PATTERNNAMES  13
#define TMS_CONNECTIONS   14
#define TMS_BUSTYPES      15
#define TMS_COMMENTS      16
#define TMS_TRIPS         17
#define TMS_RUNS          18
#define TMS_ROSTER        19
#define TMS_DRIVERS       20
#define TMS_DRIVERTYPES   21
#define TMS_BLOCKNAMES    22
#define TMS_CUSTOMERS     23
#define TMS_COMPLAINTS    24
#define TMS_ROUTINGS      25
#define TMS_SIGNCODES     26
#define TMS_TIMECHECKS    27
#define TMS_BUSES         28
#define TMS_DISCIPLINE    29
#define TMS_CREWONLY      30
#define TMS_DAILYOPS      31
#define TMS_OFFTIME       32
#define TMS_RIDERSHIP     33
#define TMS_LASTFILE      TMS_RIDERSHIP

#define TMS_STANDARDBLOCKS  TMS_LASTFILE + 101
#define TMS_DROPBACKBLOCKS  TMS_LASTFILE + 102

#define VIEW_FIRSTVIEW      1001
#define VIEW_STANDARDBLOCKS 1001
#define VIEW_DROPBACKBLOCKS 1002
#define VIEW_RUNS           1003
#define VIEW_ROSTER         1004
#define VIEW_TIMECHECKS     1005
#define VIEW_LASTVIEW       VIEW_TIMECHECKS

#define TABLENAME_FILE          "X$File"
#define TABLENAME_FIELD         "X$Field"
#define TABLENAME_INDEX         "X$Index"
#define TABLENAME_ATTRIBUTES    "ATTRIBUTES"
#define TABLENAME_CREATED       "CREATED"
#define TABLENAME_TABLEVIEW     "TABLEVIEW"
#define TABLENAME_DIRECTIONS    "DIRECTIONS"
#define TABLENAME_ROUTES        "ROUTES"
#define TABLENAME_SERVICES      "SERVICES"
#define TABLENAME_JURISDICTIONS "JURISDICTIONS"
#define TABLENAME_NODES         "NODES"
#define TABLENAME_DIVISIONS     "DIVISIONS"
#define TABLENAME_PATTERNS      "PATTERNS"
#define TABLENAME_PATTERNNAMES  "PATTERNNAMES"
#define TABLENAME_CONNECTIONS   "CONNECTIONS"
#define TABLENAME_BUSTYPES      "BUSTYPES"
#define TABLENAME_COMMENTS      "COMMENTS"
#define TABLENAME_TRIPS         "TRIPS"
#define TABLENAME_RUNS          "RUNS"
#define TABLENAME_ROSTER        "ROSTER"
#define TABLENAME_DRIVERS       "DRIVERS"
#define TABLENAME_DRIVERTYPES   "DRIVERTYPES"
#define TABLENAME_BLOCKNAMES    "BLOCKNAMES"
#define TABLENAME_CUSTOMERS     "CUSTOMERS"
#define TABLENAME_COMPLAINTS    "COMPLAINTS"
#define TABLENAME_ROUTINGS      "ROUTINGS"
#define TABLENAME_SIGNCODES     "SIGNCODES"
#define TABLENAME_TIMECHECKS    "TIMECHECKS"
#define TABLENAME_BUSES         "BUSES"
#define TABLENAME_DISCIPLINE    "DISCIPLINE"
#define TABLENAME_CREWONLY      "CREWONLY"
#define TABLENAME_DAILYOPS      "DAILYOPS"
#define TABLENAME_OFFTIME       "OFFTIME"
#define TABLENAME_RIDERSHIP     "RIDERSHIP"

#define VIEWNAME_STANDARDBLOCKS "STANDARDBLOCKS"
#define VIEWNAME_DROPBACKBLOCKS "DROPBACKBLOCKS"
#define VIEWNAME_RUNS           "RUNS"
#define VIEWNAME_ROSTER         "ROSTER"
#define VIEWNAME_TIMECHECKS     "TIMECHECKS"

//
//  Database modification rights flags
//
#define RIGHTS_FILEDDF       (1L << FILE_DDF)
#define RIGHTS_FIELDDDF      (1L << FIELD_DDF)
#define RIGHTS_INDEXDDF      (1L << INDEX_DDF)
#define RIGHTS_ATTRIBUTES    (1L << TMS_ATTRIBUTES)
#define RIGHTS_CREATED       (1L << TMS_CREATED)
#define RIGHTS_TABLEVIEW     (1L << TMS_TABLEVIEW)
#define RIGHTS_DIRECTIONS    (1L << TMS_DIRECTIONS)
#define RIGHTS_ROUTES        (1L << TMS_ROUTES)
#define RIGHTS_SERVICES      (1L << TMS_SERVICES)
#define RIGHTS_JURISDICTIONS (1L << TMS_JURISDICTIONS)
#define RIGHTS_NODES         (1L << TMS_NODES)
#define RIGHTS_DIVISIONS     (1L << TMS_DIVISIONS)
#define RIGHTS_PATTERNS      (1L << TMS_PATTERNS)
#define RIGHTS_PATTERNNAMES  (1L << TMS_PATTERNNAMES)
#define RIGHTS_CONNECTIONS   (1L << TMS_CONNECTIONS)
#define RIGHTS_BUSTYPES      (1L << TMS_BUSTYPES)
#define RIGHTS_BUSES         (1L << TMS_BUSES)
#define RIGHTS_COMMENTS      (1L << TMS_COMMENTS)
#define RIGHTS_TRIPS         (1L << TMS_TRIPS)
#define RIGHTS_RUNS          (1L << TMS_RUNS)
#define RIGHTS_ROSTER        (1L << TMS_ROSTER)
#define RIGHTS_DRIVERS       (1L << TMS_DRIVERS)
#define RIGHTS_DRIVERTYPES   (1L << TMS_DRIVERTYPES)
#define RIGHTS_BLOCKNAMES    (1L << TMS_BLOCKNAMES)
#define RIGHTS_CUSTOMERS     (1L << TMS_CUSTOMERS)
#define RIGHTS_COMPLAINTS    (1L << TMS_COMPLAINTS)
#define RIGHTS_ROUTINGS      (1L << TMS_ROUTINGS)
#define RIGHTS_SIGNCODES     (1L << TMS_SIGNCODES)
#define RIGHTS_TIMECHECKS    (1L << TMS_TIMECHECKS)
#define RIGHTS_BIDDING       (1L << 30)
#define RIGHTS_DAILYOPS      (1L << 31)

EXTERN long m_UserAccessRights;

//
//  Specific includes
//
#ifndef MAX_PATH
  #define MAX_PATH 260
#endif
EXTERN char      userName[80];
EXTERN char      m_UserID;
EXTERN char      TMS[64];
EXTERN HCURSOR   hCursorArrow;
EXTERN HCURSOR   hCursorWait;
EXTERN HICON     hIconTMS;
EXTERN HINSTANCE hInst;
EXTERN HWND      hWndMain;
EXTERN char      titleString[128];
EXTERN char      TMSINIFile[MAX_PATH];
EXTERN char      szDatabaseFileName[MAX_PATH];
EXTERN char      szDatabaseRoot[MAX_PATH];
EXTERN char      szEffectiveDate[64];
EXTERN char      szUntilDate[64];
EXTERN long      effectiveDate;
EXTERN long      untilDate;
EXTERN char      szDirectory[MAX_PATH];
EXTERN char      szInstallDirectory[MAX_PATH];
EXTERN char      szCrystalReportsPath[MAX_PATH];
EXTERN char      szMappingProgram[MAX_PATH];
EXTERN char      szStreetFile[MAX_PATH];
EXTERN char      szStreetColumn[64];
EXTERN char      szLandmarkFile[MAX_PATH];
EXTERN char      szUTMZone[16];
EXTERN char      szDatabaseDescription[64];
EXTERN char      szShortDatabaseDescription[64];
EXTERN char      szPropertyName[80];
EXTERN char      szTRUE[8];
EXTERN char      szFALSE[8];
EXTERN char      szMonth[12][4];
EXTERN char      szReportTemplateFolder[MAX_PATH];
EXTERN char      szDisciplineLetterTemplateFolder[MAX_PATH];

// Application messages

#define UM_USERCOMMAND    (WM_USER + 6)
#define WM_USERSETUP      (WM_USER + 18)
#define WM_USERSETUP2     (WM_USER + 19)

#define BUTTONMAX

#define DISPLAY 1
#define CTRL    2
#define ALL     3

//  Memory Management Stuff

typedef struct POOLINFO POOLINFO;
struct POOLINFO
{
  POOLINFO *piPrevious;
  HGLOBAL  hMem;
  int      sLength;
  int      sOffset;
  char     start[1];
};
#define POOLSIZE      4096
#define MM_TIMERQUEUE    0
#define MM_EVENTQUEUE    1
#define MM_BLOCKS        2
#define MM_GRAPHICS      3
#define MM_RUNS          4
#define MM_MINCOSTMATCH  5
#define MAXPOOLS         6

#define TEMPSTRING_LENGTH     MAX_PATH * 2
#define SZARSTRING_LENGTH     MAX_PATH * 2
#define SZFORMATSTRING_LENGTH MAX_PATH * 2
#define CLASSNAME_LENGTH      32

EXTERN POOLINFO         *piFreeArea[MAXPOOLS];
EXTERN char szarHelpFile[MAX_PATH];
EXTERN char szarString[SZARSTRING_LENGTH];
EXTERN char tempString[TEMPSTRING_LENGTH];
EXTERN char szFormatString[SZFORMATSTRING_LENGTH];

//
//  Externs for keeping track of current selections
//
EXTERN long m_RouteRecordID;
EXTERN long m_ServiceRecordID;
EXTERN long m_DivisionRecordID;
EXTERN long m_RosterWeek;

//
//  Layover info
//
typedef struct LAYOVERTYPEStruct
{
  int  type;
  int  minutes;
  int  percent;
  BOOL lower;
} LAYOVERTYPEDef;

typedef struct LAYOVERStruct
{
  LAYOVERTYPEDef minimum;
  LAYOVERTYPEDef maximum;
} LAYOVERDef;

EXTERN LAYOVERDef StandardLayover;
EXTERN LAYOVERDef DropbackLayover;

//
//  Font stuff
//

EXTERN HFONT    hBFont, hMFont, hFont, hPFont;
EXTERN LOGFONT  lfCurrent;
EXTERN COLORREF rgbColor;

//
//  Global options
//

EXTERN int  timeFormat;         /* time of day display format */
EXTERN int  timeHHMMFormat;     /* time in hours and minutes display format */
EXTERN BOOL m_bWrapTime;        // send time display back to 0000 after 2359
EXTERN int  distanceMeasure;    // Miles or Km
EXTERN long defaultBustype;     // default bustype when building trips

EXTERN BOOL m_bShowNodeTimes;
EXTERN BOOL m_bCompressedBlocks;
EXTERN BOOL m_bAllowTripPlanning;
EXTERN BOOL m_bUseLatLong;
EXTERN BOOL m_bEnableDailyOps;
EXTERN BOOL m_bMonitorCheckins;

EXTERN int  rosterFormat;

typedef struct COMPBLOCKStruct
{
  long  tripTime;
  long  layTime;
  long  dhdTime;
  long  lastNode;
  long  lastTime;
  long  PIGNODESrecordID;
  float tripDistance;
  float cumulativeDistance;
} COMPBLOCKDef;
EXTERN COMPBLOCKDef COMPBLOCK;

EXTERN long specificBlock;

//
//  Trip Generation
//
#define ALERTMAX 100
typedef struct ALERTStruct
{
  long from;
  long to;
} ALERTDef;
EXTERN ALERTDef m_ALERT[ALERTMAX];
EXTERN int      m_numConnectionAlerts;

//
//  Renumber parameter structure
//

typedef struct RENUMBERPARAMStruct
{
  char type[8];
  long number;
} RENUMBERPARAMDef;

//
//  Saved block/bus assignment information
//

#define BLOCKINFO_FLAG_TOBEASSIGNED 0x0001
#define BLOCKINFO_FLAG_ASSIGNED     0x0002
#define BLOCKINFO_FLAG_BUSRETURN    0x0004
#define BLOCKINFO_FLAG_BLOCKDROP    0x0008
#define BLOCKINFO_FLAG_DISPLAYALL   0x0010

typedef struct BlockInfoStruct
{
  long  POTime;
  long  POGNODESrecordID;
  long  blockNumber;
  long  TRIPSrecordID;
  long  BUSTYPESrecordID;
  long  BUSESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  PITime;
  long  PIGNODESrecordID;
  float distance;
  long  flags;
  long  startingRunNumber;
  long  startingRUNSrecordID;
  long  startingRosterNumber;
  long  startingROSTERrecordID;
  long  startingDRIVERSrecordID;
  char  *pszDriver;
} BlockInfoDef;

#define MAXINBLOCKINFO 500

EXTERN BlockInfoDef m_BlockInfo[MAXINBLOCKINFO];
EXTERN long m_BlockInfoShowFlags;
EXTERN long m_bPegboardSortFlags;
EXTERN long m_bPegboardShowFlags;
EXTERN int  m_numBlocksInBlockInfo;

//
//  Blocking mode
//
EXTERN BOOL m_bNetworkMode;

//
//  DailyOps structures and defines, located here for access outside of the DailyOps module
//

EXTERN long m_LastDAILYOPSRecordID;
EXTERN BOOL m_bSemaphoreUpdate;
EXTERN BOOL m_bMultipleDispatchMachines;
EXTERN int  m_PegboardMagicNumber;

EXTERN long m_DailyOpsROSTERDivisionInEffect;
EXTERN long m_DailyOpsRUNSDivisionInEffect;
EXTERN long m_DailyOpsServiceInEffect;

//
//  Fare types
//
#define MAXFARETYPEROUTES 100
#define RIDERSHIP_MAXFARETYPES     6
#define RIDERSHIP_FARETYPE_LENGTH 16

typedef struct FareTypesStruct
{
  long ROUTESrecordID;
  float fareValue[RIDERSHIP_MAXFARETYPES];
} FareTypesDef;

EXTERN char         m_szFareTypes[RIDERSHIP_MAXFARETYPES][RIDERSHIP_FARETYPE_LENGTH];
EXTERN int          m_numFareTypes;
EXTERN FareTypesDef m_FareTypesByRoute[MAXFARETYPEROUTES];
EXTERN int          m_numFareTypesByRoute;

typedef struct RDataStruct
{
  long recordID;
  long runNumber;
  long cutAsRuntype;
  long DRIVERSrecordID;
  long flags;
  long onNODESrecordID;
  long onTime;
  long offNODESrecordID;
  long offTime;
  long pay;
  long rosterNumber;
  long ROSTERrecordID;
} RDataDef;
  
EXTERN RDataDef* m_pRData;
EXTERN long      m_numRunsInRData;
EXTERN RDataDef* m_pRDataHold;
EXTERN long      m_numRunsInRDataHold;

//
//  Access text file stuff
//
//  Absence Reasons (DailyOps)
//
#define MAXABSENCEREASONS 64

typedef struct AbsenceReasonsStruct
{
  long ruleNumber;
  char szText[TEMPSTRING_LENGTH];
//  BOOL bCountsTowardUnpaidTime;
  long paidOnAbsence;
  char szPayrollCode[16];
} AbsenceReasonsDef;

EXTERN AbsenceReasonsDef m_AbsenceReasons[MAXABSENCEREASONS];
EXTERN int m_numAbsenceReasons;

//
//  Action Codes (Customer Complaints)
//
#define MAXACTIONCODES 64

typedef struct GenericTextStruct
{
  long number;
  char szText[TEMPSTRING_LENGTH];
} GenericTextDef;

EXTERN GenericTextDef m_ActionCodes[MAXACTIONCODES];
EXTERN int m_numActionCodes;

//
//  Bus Swap Reasons (DailyOps)
//
#define MAXBUSSWAPREASONS 32

EXTERN GenericTextDef m_BusSwapReasons[MAXBUSSWAPREASONS];
EXTERN int  m_numBusSwapReasons;

//
//  Categories (Customer Complaints)
//
#define MAXCATEGORIES 32

EXTERN GenericTextDef m_Categories[MAXCATEGORIES];
EXTERN int  m_numCategories;

//
//  Classification Codes (Customer Complaints)
//
#define MAXCLASSIFICATIONCODES 128

EXTERN GenericTextDef m_ClassificationCodes[MAXCLASSIFICATIONCODES];
EXTERN int  m_numClassificationCodes;

//
//  Departments (Customer Complaints)
//
#define MAXDEPARTMENTS 32

EXTERN GenericTextDef m_Departments[MAXDEPARTMENTS];
EXTERN int  m_numDepartments;

//
//  Extra Time Reasons (DailyOps)
//
#define MAXEXTRATIMEREASONS 32

typedef struct ExtraTimeReasonsStruct
{
  long ruleNumber;
  char szText[TEMPSTRING_LENGTH];
  char szPayrollCode[16];
} ExtraTimeReasonsDef;

EXTERN ExtraTimeReasonsDef m_ExtraTimeReasons[MAXEXTRATIMEREASONS];
EXTERN int  m_numExtraTimeReasons;

//
//  Other Comments (Timechecks)
//
#define MAXOTHERCOMMENTS 32

EXTERN GenericTextDef m_OtherComments[MAXOTHERCOMMENTS];
EXTERN int  m_numOtherComments;

//
//  Passenger Loads (Timechecks)
//
#define MAXPASSENGERLOADS 32

EXTERN GenericTextDef m_PassengerLoads[MAXPASSENGERLOADS];
EXTERN int  m_numPassengerLoads;

//
//  Payroll Layout
//
#define MAXPAYROLLLAYOUTENTRIES 64

typedef struct PayrollLayoutStruct
{
  int  index;
  long position;
  char szText[64];
} PayrollLayoutDef;

EXTERN PayrollLayoutDef m_PayrollLayout[MAXPAYROLLLAYOUTENTRIES];
EXTERN int m_numPayrollLayoutEntries;
//
//  Referrals (Customer complaints)
//
#define MAXREFERRALS                   64
#define CCREFER_NAME_LENGTH            64
#define CCREFER_DEPARTMENT_LENGTH      64
#define CCREFER_EMAILADDRESS_LENGTH    64

typedef struct ReferralsStruct
{
  char szName[CCREFER_NAME_LENGTH];
  char szDepartment[CCREFER_DEPARTMENT_LENGTH];
  char szEmailAddress[CCREFER_EMAILADDRESS_LENGTH];
} ReferralsDef;

EXTERN ReferralsDef m_Referrals[MAXREFERRALS];
EXTERN int  m_numReferrals;

//
//  Road Supervisors (Timechecks)
//
#define MAXROADSUPERVISORS 32

EXTERN GenericTextDef m_RoadSupervisors[MAXROADSUPERVISORS];
EXTERN int  m_numRoadSupervisors;

//
//  Stop flags (Nodes Table)
//
#define MAXSTOPFLAGS 32

EXTERN GenericTextDef m_StopFlags[MAXSTOPFLAGS];
EXTERN int  m_numStopFlags;

//
//  Supervisor (Customer Complaints)
//
#define MAXSUPERVISORS                 32
#define SUPERVISORS_NAME_LENGTH        64
#define SUPERVISORS_PASSWORD_LENGTH    64

typedef struct SupervisorsStruct
{
  int  number;
  char szName[SUPERVISORS_NAME_LENGTH];
  char szPassword[SUPERVISORS_PASSWORD_LENGTH];
} SupervisorsDef;

EXTERN SupervisorsDef m_Supervisors[MAXSUPERVISORS];
EXTERN int  m_numSupervisors;

//
//  Traffic Conditions (Timechecks)
//
#define MAXTRAFFICCONDITIONS 32

EXTERN GenericTextDef m_TrafficConditions[MAXTRAFFICCONDITIONS];
EXTERN int  m_numTrafficConditions;

//
//  Typical [complaints] (Customer Complaints)
//
#define MAXTYPICAL 64

EXTERN GenericTextDef m_Typical[MAXTYPICAL];
EXTERN int  m_numTypical;

//
//  Violations, Violation Categories, and Violation Actions (Discipline)
//
#define MAXVIOLATIONCATEGORIES 32
#define MAXVIOLATIONTYPES      32
#define MAXVIOLATIONACTIONS    32

EXTERN char m_ViolationCategories[MAXVIOLATIONCATEGORIES][128];
EXTERN int  m_numViolationCategories;

EXTERN char m_ViolationActions[MAXVIOLATIONACTIONS][128];
EXTERN int  m_numViolationActions;

typedef struct ViolationsStruct
{
  long flags;
  long duration;
  char text[255];
  int  numActions;
  int  actions[MAXVIOLATIONACTIONS];
} ViolationsDef;

EXTERN ViolationsDef m_Violations[MAXVIOLATIONCATEGORIES][MAXVIOLATIONTYPES];
EXTERN int  m_numViolations[MAXVIOLATIONCATEGORIES];

//
//  Weather Conditions (Timechecks)
//
#define MAXWEATHERCONDITIONS 32

EXTERN GenericTextDef m_WeatherConditions[MAXWEATHERCONDITIONS];
EXTERN int  m_numWeatherConditions;

//
//  Mapping coordinates for Easting/Northing maps
//
EXTERN char szCoordsys[128];
//
//  Route Tracings folder for ORBCad interface
//
EXTERN char szRouteTracingsFolder[MAX_PATH];
//
//  Flag to tell us if we're using MapInfo for the trip planner and
//  a char to tell us the URL of the web-based version if we're not
//
EXTERN BOOL m_bUseMapInfo;
EXTERN char m_szTripPlannerURL[MAX_PATH];
//
//  Structures for MapInfo MIF files
//
typedef struct MAPPOINTStruct
{
  double longitude;
  double latitude;
  long   flags;
  long   NODESrecordID;
  long   associatedStopNODESrecordID;
  long   associatedStopNumber;
} MAPPOINTDef;

#define MIFLINES_FLAG_START              0x0001
#define MIFLINES_FLAG_USED               0x0002
#define MIFLINES_FLAG_NEXTISSAMELOCATION 0x0004

typedef struct MIFLINESStruct
{
  MAPPOINTDef from;
  MAPPOINTDef to;
  long        flags;
} MIFLINESDef; 

typedef struct MIFPASSEDDATAStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long PATTERNNAMESrecordID;
  char szMIFFileName[MAX_PATH];
  int  numNodes;
  long NODESrecordIDs[100];
  HFILE hfErrorLog;
} MIFPASSEDDATADef;

//
//  Passed data for Supervisor/Password dialog
//

#define SUPERVISOR_FLAG_BIDDING    0x0001
#define SUPERVISOR_FLAG_DAILYOPS   0x0002

typedef struct SUPERVISORPassedDataStruct
{
  char szMessage[TEMPSTRING_LENGTH];
  long flags;
  BOOL bAuthenticated;
  BOOL bDisabled;
} SUPERVISORPassedDataDef;

//
//  DailyOps
//
#define NODES_ABBRNAME_LENGTH      4  // Note - Also occurs in btrieve.h

typedef struct RunsStruct
{
  long recordID;
  long runNumber;
  long cutAsRuntype;
  long numPieces;
  long startTRIPSrecordID;
  long startNODESrecordID;
  char szStartNodeName[NODES_ABBRNAME_LENGTH + 1];
  long startTime;
  long reportAtNODESrecordID;
  char szReportAtNodeName[NODES_ABBRNAME_LENGTH + 1];
  long reportAtTime;
  long endTRIPSrecordID;
  long endNODESrecordID;
  char szEndNodeName[NODES_ABBRNAME_LENGTH + 1];
  long endTime;
  long turninAtNODESrecordID;
  char szTurninAtNodeName[NODES_ABBRNAME_LENGTH + 1];
  long turninAtTime;
  long payTime;
  long flags;
} RunsDef;

typedef struct IncoreStruct
{
  int      numRecords;
  long    *pRecordIDs;
  RunsDef *pRuns;
} IncoreDef;


//
//  Incore references to the Runs Tables
//
EXTERN IncoreDef  m_IncoreServices;
EXTERN IncoreDef *m_pIncoreRuns;

//
//  Pegboard colors
//

typedef struct PBCOLORSStruct
{
  COLORREF crAvailable;
  COLORREF crAssigned;
  COLORREF crOutOfService;
  COLORREF crShortShift;
  COLORREF crCharterReserve;
  COLORREF crSightseeingReserve;
} PBCOLORSDef;

EXTERN PBCOLORSDef m_PBCOLORS;

//
//  Database stuff
//
#define GETTMSDATA_FLAG_BUILD   0x0001
#define GETTMSDATA_FLAG_CLEAR   0x0002
#define GETTMSDATA_FLAG_SETROW1 0x0004

#include "database.h"
//
//  Reports
//
#include "tmsrpt.h"
//
//  Function prototypes
//
#include "Prototypes.h"
//
//  Runcutter
//
#include "runcut.h"
//
//  Customer comments
//
#include "custcom.h"
//
//  Windows common controls
//
#include "commctrl.h"
//
//  Excel constants
//
#include "excel.h"

#endif // TmsHeader_H