//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//  TMS Reports - Data Structures
//
//  Note: When changing MAX_REPORTSDEFINED ensure that:
//        a) The Report Range in tms.cpp is changed
//        b) There is a corresponding increase in IDM_R_REPORTxx
//
#define TMSRPT_MAX_REPORTSDEFINED     96  // Match this in prototype.h for defined routines TMSRPTnn
#define TMSRPT_MAX_REPORTOUTPUTFILES   4
#define TMSRPT_REPORTNAME_LENGTH      64
#define TMSRPT_REPORTFILENAME_LENGTH 256
#define TMSRPT_MAX_LINESPERPAGE       42
#define TMSRPT_MAX_COLSPERPAGE       182

#define TMSRPT_MAX_BITMASKS            4  // This handles up to 128 reports - increase accordingly
EXTERN unsigned long bmReportIsActive[TMSRPT_MAX_BITMASKS];
EXTERN char szReportsTempFolder[MAX_PATH];
EXTERN int m_LastReport;

#define TMSRPT_PASSEDDATAFLAG_FROMDAILYOPS    0x0001
#define TMSRPT_PASSEDDATAFLAG_DONOTUSECRYSTAL 0x0002

typedef struct TMSRPTPassedDataStruct
{
  int  nReportNumber;
  int  numDataFiles;
  int  dayOfWeek;
  char szReportDataFile[TMSRPT_MAX_REPORTOUTPUTFILES][TMSRPT_REPORTFILENAME_LENGTH];
  long DRIVERSrecordID;
  long DIVISIONSrecordID;
  long flags;
  long date;
} TMSRPTPassedDataDef;

typedef BOOL (FAR * TMSRPTRoutineDef)(TMSRPTPassedDataDef *);
typedef BOOL (FAR * TMSRPTConfigFuncDef)(HWND, UINT, WPARAM, LPARAM);

#define TMSRPTFLAG_ACTIVE   0x0001
#define TMSRPTFLAG_REPORT   0x0002
#define TMSRPTFLAG_DOWNLOAD 0x0004
#define TMSRPTFLAG_UPLOAD   0x0008
#define TMSRPTFLAG_USEMSJET 0x0010

typedef struct TMSRPTStruct
{
  char flags;
  int  originalReportNumber;
  int  numReports;
  char szReportName[TMSRPT_REPORTNAME_LENGTH];
  char szReportFileName[2][TMSRPT_REPORTFILENAME_LENGTH];
  TMSRPTRoutineDef    lpRoutine;
  TMSRPTConfigFuncDef lpConfigFunc;
  int  ConfigFuncDlg;
} TMSRPTDef;
EXTERN TMSRPTDef TMSRPT[TMSRPT_MAX_REPORTSDEFINED];

EXTERN int TMSRPTMaxLinesPerPage;
EXTERN int TMSRPTMaxColsPerPage;
 
#define RPFLAG_ROUTES             (1 << 0)
#define RPFLAG_SERVICES           (1 << 1)
#define RPFLAG_DIVISIONS          (1 << 2)
#define RPFLAG_PATTERNNAMES       (1 << 3)
#define RPFLAG_COMMENTS           (1 << 4)
#define RPFLAG_NODES              (1 << 5)
#define RPFLAG_RUNS               (1 << 6)
#define RPFLAG_BLOCKS             (1 << 7)
#define RPFLAG_OUTPUT             (1 << 8)
#define RPFLAG_NOALLROUTES        (1 << 9)
#define RPFLAG_NOALLSERVICES      (1 << 10) 
#define RPFLAG_NOALLDIVISIONS     (1 << 11)
#define RPFLAG_NOALLRUNS          (1 << 12)
#define RPFLAG_NOALLBLOCKS        (1 << 13)
#define RPFLAG_STANDARDBLOCKS     (1 << 14)
#define RPFLAG_SIGNCODES          (1 << 15)
#define RPFLAG_TRIPNUMBERS        (1 << 16)
#define RPFLAG_SPECIFICTRIPNUMBER (1 << 17)

typedef struct REPORTPARMSStruct
{
  int     nReportNumber;
  long    ROUTESrecordID;
  int     numRoutes;
  long   *pRouteList;
  long    SERVICESrecordID;
  int     numServices;
  long   *pServiceList;
  long    DIVISIONSrecordID;
  int     numDivisions;
  long   *pDivisionList;
  long    RUNSrecordID;
  int     numRuns;
  long   *pRunList;
  long    blockNumber;
  int     numBlocks;
  long   *pBlockList;
  long    PATTERNNAMESrecordID;
  long    COMMENTSrecordID;
  long    NODESrecordID;
  char    nodeLabel;
  int     linesPerPage;
  int     colsPerPage;
  long    flags;
  long    returnedFlags;
  int     tripNumber;
} REPORTPARMSDef;

#define TMSRPT08_INCLUDEEXISTING   0x0001
#define TMSRPT08_INCLUDERT         0x0002
#define TMSRPT08_INCLUDEDT         0x0004
#define TMSRPT08_INCLUDETT         0x0008
#define TMSRPT08_INCLUDEEQ         0x0010
#define TMSRPT08_INCLUDEPOTENTIAL  0x0020
#define TMSRPT08_ALLSYSTEMNODES    0x0040
#define TMSRPT08_PATTERNNODES      0x0080
#define TMSRPT08_RELIEFPOINTS      0x0100

#define TMSRPT09_PADDLE            0
#define TMSRPT09_COSTING           1
#define TMSRPT09_NUMFILES          2

#define TMSRPT11_DIRECTIONS        0x0001
#define TMSRPT11_ROUTES            0x0002
#define TMSRPT11_SERVICES          0x0004
#define TMSRPT11_DIVISIONS         0x0008
#define TMSRPT11_GARAGES           0x0010
#define TMSRPT11_PATTERNSNODES     0x0020
#define TMSRPT11_BLOCKSTRIPSPIECES 0x0040
#define TMSRPT11_RUNS              0x0080
#define TMSRPT11_RUNTYPES          0x0100
#define TMSRPT11_USERS             0x0200

#define TMSRPT12_ROUTEINFO         0x0001
#define TMSRPT12_STOPSINFO         0x0002
#define TMSRPT12_PATHINFO          0x0004
#define TMSRPT12_DEADHEADINFO      0x0008
#define TMSRPT12_TIMETABLEINFO     0x0010
#define TMSRPT12_OPERATORBIDINFO   0x0020
#define TMSRPT12_RUNCUTINFO        0x0040
#define TMSRPT12_STOPDISTANCEINFO  0x0080

#define TMSRPT26_BEFORE            0x0001
#define TMSRPT26_AFTER             0x0002
#define TMSRPT26_ALLDAY            0x0004
#define TMSRPT26_SCHOOL            0x0008
#define TMSRPT26_SHUTTLE           0x0010

#define TMSRPT29_ROUTES            0
#define TMSRPT29_NODES             1
#define TMSRPT29_HEADWAYS          2

typedef struct RPTFLAGSStruct
{
  int  nReportNumber;
  long time;
  long flags;
} RPTFLAGSDef;

#define MAXN 250
#define MAXT 250

typedef struct NStruct
{
  long NODESrecordID;
  long numTimes;
  long times[MAXT];
} NDef;

typedef struct TStruct
{
  char runNumber[8];
  char routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char dirAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char commentCode[COMMENTS_CODE_LENGTH + 1];
  char signCode[SIGNCODES_CODE_LENGTH + 1];
  long tripNumber;
} TDef;

typedef struct RunRecordStruct
{
  long recordID;
  long runNumber;
  long pieceNumber;
  long blockNumber;
  long startNODESrecordID;
  long startTRIPSrecordID;
  long startTime;
  long endNODESrecordID;
  long endTRIPSrecordID;
  long endTime;
  long sequenceNumber;
} RunRecordDef;

EXTERN RunRecordDef *m_pRunRecordData;
EXTERN int m_maxRunRecords;
EXTERN int m_numRunRecords;

#define TMSRPT36_FLAG_ALLSERVICES        0x0001
#define TMSRPT36_FLAG_ALLSERVICEDAYS     0x0002
#define TMSRPT36_FLAG_ALLROUTES          0x0004
#define TMSRPT36_FLAG_BOTHDIRECTIONS     0x0008
#define TMSRPT36_FLAG_ALLTIMEPOINTS      0x0010
#define TMSRPT36_FLAG_ALLDAY             0x0020
#define TMSRPT36_FLAG_ALLDRIVERS         0x0040
#define TMSRPT36_FLAG_DONOTEXCLUDE       0x0080
#define TMSRPT36_FLAG_ALLBUSTYPES        0x0100
#define TMSRPT36_FLAG_ALLPASSENGERLOADS  0x0200
#define TMSRPT36_FLAG_ALLROADSUPERVISORS 0x0400

typedef struct TMSRPT36PassedDataStruct
{
  long flags;
  long fromDate;
  long toDate;
  long SERVICESrecordID;
  long dayNumber;
  long ROUTESrecordID;
  long directionIndex;
  long NODESrecordID;
  long timeOfDayFrom;
  long timeOfDayTo;
  long DRIVERSrecordID;
  long excludedDRIVERSrecordID;
  long BUSTYPESrecordID;
  long passengerLoadIndex;
  long roadSupervisorIndex;
} TMSRPT36PassedDataDef;

typedef struct TMSRPT48PassedDataStruct
{
  long fromMonth;
  long fromYear;
  long toMonth;
  long toYear;
} TMSRPT48PassedDataDef;

#define DISPATCHSHEET_FLAG_INCLUDESTARTOFPIECETRAVEL 0x0001
#define DISPATCHSHEET_FLAG_DISABLEOPTIONS            0x0002
#define DISPATCHSHEET_FLAG_SORTBY                    0x0004
#define DISPATCHSHEET_FLAG_REPORTTIME                0x0008
#define DISPATCHSHEET_FLAG_INCLUDEGARAGEANDRELIEFS   0x0010

#define DISPATCHSHEET_MAXGARAGES                     10
#define DISPATCHSHEET_MAXRELIEFPOINTS                100

#define RDATA_FLAG_CREWONLY                          0x0001
#define RDATA_FLAG_SUPERCEDED                        0x0002
#define RDATA_FLAG_SPLITRUN                          0x0004
#define RDATA_FLAG_INELIGIBLE                        0x0008

typedef struct DISPATCHSHEETPassedDataStruct
{
  long DIVISIONSrecordID;
  long numGarages;
  long garageNODESrecordIDs[DISPATCHSHEET_MAXGARAGES];
  long numReliefPoints;
  long reliefPointNODESrecordIDs[DISPATCHSHEET_MAXRELIEFPOINTS];
  long flags;
  long date;
  long dayOfWeek;
  int  nReportNumber;
  char szDate[64];
  long todaysDate;
} DISPATCHSHEETPassedDataDef;

#define EVENTREPORT_FLAG_STARTOFPIECETRAVEL 0x0001
#define EVENTREPORT_FLAG_STARTOFPIECE       0x0002
#define EVENTREPORT_FLAG_ENDOFPIECETRAVEL   0x0004
#define EVENTREPORT_FLAG_ENDOFPIECE         0x0008
#define EVENTREPORT_FLAG_EXTRABOARDDUTY     0x0010
#define EVENTREPORT_FLAG_REPORTTIME         0x0020

typedef struct EVENTREPORTPassedDataStruct
{
  long DIVISIONSrecordID;
  long flags;
  long date;
  long dayOfWeek;
  int  nReportNumber;
  char szDate[64];
  long todaysDate;
} EVENTREPORTPassedDataDef;

typedef struct DISPATCHSHEETRosterDataStruct
{
  long flags;
  long DRIVERSrecordID;
  long replacementDRIVERSrecordID;
  long RUNSrecordID;
  long startTime;
  long runNumber;
  long rosterNumber;
  long BUSESrecordID;
  long blockNumber;
  long TRIPSrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
} DISPATCHSHEETRosterDataDef;

#define TMSRPT61_MAXSERVICES      25
#define TMSRPT61_MAXROUTES       200
#define TMSRPT61_MAXPATTERNNODES 100
#define TMSRPT61_MAXTREQ        2000
#define TMSRPT61_MAXNEQ           50
#define TMSRPT61_MAXMIFLINES    2000
 

typedef struct NEQStruct
{
  long secondaryNODESrecordID;
  long primaryNODESrecordID;
} NEQDef;

EXTERN long numNEQ;
EXTERN NEQDef NEQ[TMSRPT61_MAXNEQ];


#define TMSRPT66_MAXSERVICES      25
#define TMSRPT66_MAXROUTES       200
#define TMSRPT66_MAXPATTERNNODES 100
#define TMSRPT66_MAXTREQ        2000
#define TMSRPT66_MAXNEQ           50

typedef struct UNPAIDTIMEPassedDataStruct
{
  long date;
  long DRIVERSrecordID;
} UNPAIDTIMEPassedDataDef;

