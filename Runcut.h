//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#define WORKRULE_FILE   "\\WORKRULE.INI"
#define NUMRUNTYPES      6
#define STRAIGHT_TEXT   "Straight"
#define TWOPIECE_TEXT   "Two-Piece"
#define MULTIPIECE_TEXT "Multi-Piece"
#define SWING_TEXT      "Swing"
#define TRIPPER_TEXT    "Tripper"
#define ILLEGAL_TEXT    "Illegal"
#define STRAIGHT_INDEX   0
#define TWOPIECE_INDEX   1
#define MULTIPIECE_INDEX 2
#define SWING_INDEX      3
#define TRIPPER_INDEX    4
#define ILLEGAL_INDEX    5
#define NUMRUNTYPESLOTS 30
#define WEEK4DAYS        0
#define WEEK5DAYS        1
#define SPREADOT         2

EXTERN BOOL m_bWorkrulesChanged;

EXTERN char szWorkruleFile[256];

//
//  Overtime
//
//  [0] = 4 day
//  [1] = 5 day
//

#define MAXSERVICES               10
#define OTF_ROUNDUP               (1<<0)
#define OTF_ROUNDDOWN             (1<<1)
#define OTF_KEEPHALF              (1<<2)
#define OTF_PAYLOWER              (1<<3)
#define OTF_PAYHIGHER             (1<<4)
#define OTF_PAYBOTH               (1<<5)
#define OTF_ROUNDUPTONEXT         (1<<6)
#define OTF_PAYMAKEUPBEFORESPREAD (1<<7)

typedef struct OVERTIMEStruct
{
  long  dailyAfter[2][MAXSERVICES];
  float dailyRate[2][MAXSERVICES];
  long  weeklyAfter[2];
  float weeklyRate[2];
  long  spreadAfter[MAXSERVICES];
  float spreadRate[MAXSERVICES];
  long  roundedBounded;
  BOOL  payMinuteByMinute;
  long  guarantee4DayWorkWeek;
  long  guarantee5DayWorkWeek;
  char  flags;
} OVERTIMEDef;

EXTERN OVERTIMEDef OVERTIME;

//
//  Runtypes
//
#define PIECEFLAGS_TRAVELINCLUDEDINMIN 0x0001
#define PIECEFLAGS_TRAVELINCLUDEDINMAX 0x0002
#define PIECEFLAGS_PAYONLYAPPLIES      0x0004

typedef struct PIECEStruct
{
  long minOnTime;
  long maxOnTime;
  long minOffTime;
  long maxOffTime;
  long minPayTime;
  long maxPayTime;
  long minPieceSize;
  long desPieceSize;
  long maxPieceSize;
  long minBreakTime;
  long desBreakTime;
  long maxBreakTime;
  long flags;
} PIECEDef;

#define RTFLAGS_INUSE                  0x0001
#define RTFLAGS_4DAY                   0x0002
#define RTFLAGS_5DAY                   0x0004
#define RTFLAGS_PAIDBREAK              0x0008
#define RTFLAGS_TRAVELCOUNTS           0x0010
#define RTFLAGS_BRINGUPTO              0x0020
#define RTFLAGS_SMALLERPAID            0x0040
#define RTFLAGS_PAYSPDOT               0x0080
#define RTFLAGS_MINLAY                 0x0100
#define RTFLAGS_LARGESTPAIDTHROUGHWHEN 0x0200
#define RTFLAGS_SMALLERPAIDTHROUGHWHEN 0x0400
#define RTFLAGS_CREWONLY               0x0800
#define RTFLAGS_UNDERCONSIDERATION     0x1000
#define RTFLAGS_PLACEHOLDER            0x2000

EXTERN BOOL m_bCheckUnderConsideration;

#define RUNTYPE_LOCALNAME_LENGTH       20

typedef struct RUNTYPEStruct
{
  long     flags;                 //   4
  char     localName[RUNTYPE_LOCALNAME_LENGTH]; // 20
  long     numPieces;             //   4
  long     minPayTime;            //   4
  long     desPayTime;            //   4
  long     maxPayTime;            //   4
  long     minPlatformTime;       //   4
  long     desPlatformTime;       //   4
  long     maxPlatformTime;       //   4
  long     maxSpreadTime;         //   4
  long     minBreakOf;            //   4
  long     desBreakOf;            //   4
  long     maxBreakOf;            //   4
  long     minContig;             //   4
  long     desContig;             //   4
  long     maxContig;             //   4
  long     paidFrom;              //   4
  long     paidTo;                //   4
  long     paidToWhen;            //   4
  long     paidFromSmaller;       //   4
  long     paidToSmaller;         //   4
  long     paidToWhenSmaller;     //   4
  long     minLayTime;            //   4
  long     minLaySpan;            //   4
  long     minLayNotBefore;       //   4
  char     minLayLabel[4];        //   4  // really only need 1, but an odd-byte boundary hoses this thing
  PIECEDef PIECE[MAXPIECES];      // 296
} RUNTYPEDef;                     // ---> 408

EXTERN RUNTYPEDef RUNTYPE[NUMRUNTYPES][NUMRUNTYPESLOTS];  // 408 * 6 * 15 = 36720

//
//  Premiums
//

#define MAXPREMIUMS 50

#define TRAVELPAID_NONE        0
#define TRAVELPAID_START      (1<<0)
#define TRAVELPAID_END        (1<<1)
#define TRAVELPAID_ABCDFLAG   (1<<2)
#define TRAVELPAID_MANIPULATE (1<<3)

#define TRAVELINC_NO         0
#define TRAVELINC_YES       (1<<0)

#define PAYWHEN_NONE         0
#define PAYWHEN_DIRECT      (1<<0)
#define PAYWHEN_FROMBIN     (1<<1)
#define PAYWHEN_OUTTOC      (1<<2)
#define PAYWHEN_ABCDFLAG    (1<<3)
#define PAYWHEN_MANIPULATE  (1<<4)

#define PAYHOW_ACTUAL        0
#define PAYHOW_FLAT         (1<<0)
#define PAYHOW_PERCENTAGE   (1<<1)
#define PAYHOW_STAGGERED    (1<<2)

#define PREMIUMFLAGS_TIMECOUNTSINOT              (1<<0)
#define PREMIUMFLAGS_TRAVELTIMEPREMIUM           (1<<1)
#define PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING  (1<<2)
#define PREMIUMFLAGS_RECORDONAB                  (1<<3)
#define PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC    (1<<4)
#define PREMIUMFLAGS_DONTTRAVELBEQC              (1<<5)
#define PREMIUMFLAGS_DONTINCORPORATE             (1<<6)
#define PREMIUMFLAGS_TRAVELBTOCWHENINTINFEASIBLE (1<<7)

typedef struct PREMIUMStruct
{
  long flags;
  long runtype;            // Premium pertains to runtype
  char localName[40];      // And is called
  long time;               // Gets this many minutes
  int  reportedAs;         // Report column
  int  formattedDataType;  // What is it?
  int  payTravelTime;      // = NO_RECORD is not applicable
//                            = WORKRULES_TIMEENTERED if "time" is to be used
//                            = WORKRULES_ASINCONNECTIONS if the Connections entry is to be used
//                            = WORKRULES_ASPERSCHEDULE if the scheduled time is to be used
//                            = WORKRULES_ASPERSCHEDULEDWELL if the schedule time + dwell is to be used
//
  int   payHow;            // = NO_RECORD        - Not applicable
//                            = PAYHOW_ACTUAL    - Pay actual travel
//                            = PAYHOW_FLAT      - Pay at a flat rate in minutes
//                            = PAYHOW_PERCENTAGE- Pay at a specified percentage 
//                            = PAYHOW_STAGGERED - Pay at a staggered rate when less than the
//                                                 specified minutes, and "actual" thereafer
  long  payHowMinutes;
  float payHowPercent;
  long  ROUTESrecordID;            // = NO_RECORD if not applicable
  long  SERVICESrecordID;          // = NO_RECORD if not applicable
  long  DIVISIONSrecordID;         // = NO_RECORD if not applicable
  long  assignedToNODESrecordID;   // = NO_RECORD if not applicable
  int   forPieceNumber;            // = NO_RECORD if not applicable
  long  withPieceSize;             // = NO_RECORD if not applicable
//
//  On verb and noun
//
//  onVerb = NO_RECORD            - Not applicable
//           PREMIUMS_PULLOUT     - Pull-out from
//           PREMIUMS_PULLIN      - Pull-in to
//           PREMIUMS_STARTRELIEF - Start relief at
//           PREMIUMS_ENDRELIEF   - End relief at
//
//  onNoun = PREMIUMS_ANYGARAGE    - Any garage
//           PREMIUMS_ANYNONGARAGE - Any non-garage
//           PREMIUMS_ANYTIMEPOINT - Any timepoint
//           PREMIUMS_TIMEPOINT    - Specific timepoint (nounNODESrecordID)
//
  int  onVerb;
  int  onNoun;
  long nounNODESrecordID;
//
//  With
//
//  busType = NO_RECORD              - Not applicable
//            PREMIUMS_ANYACCESSIBLE - Any accessible vehicle
//            PREMIUMS_BUSTYPE       - Specific bustype (BUSTYPESrecordID)
//
  int  bustype;
  long BUSTYPESrecordID;
//
//  select tells us if there is a subset
//  to the premium, and which one it is
//
//  select = NO_RECORD              - Not applicable
//         = PREMIUMS_PAIDMEALS     - Meal payment
//         = PREMIUMS_WAITTIME      - Wait time for shuttle
//         = PREMIUMS_PAIDTRAVEL    - Paid travel between pieces
//         = PREMIUMS_PAIDSTARTEND  - Paid travel at the start / end of a run
//
  int  select;
  int  mealsPaid;    // = NO_RECORD if not applicable
  long waitMinutes;  // = NO_TIME if not applicable
//
//
//  Travel Between Pieces
//
//  pieceTravel = NO_RECORD                - Not applicable
//              = BCTRAVEL_BTOCDIRECT      - Pay B to C travel
//              = BCTRAVEL_BTOBPIGTOC      - Pay B to B PIG to C
//              = BCTRAVEL_BTOBPOGTOC      - Pay B to B POG to C
//              = BCTRAVEL_BTOCPIGTOC      - Pay B to C PIG to C
//              = BCTRAVEL_BTOCPOGTOC      - Pay B to C POG to C
//              = BCTRAVEL_BTOCLOSESTTOC   - Pay B to Closest Garage to C
//              = BCTRAVEL_BTONODE         - Pay B to payBtoNODESrecordID to C
//
  int   pieceTravel;
  long  payBtoNODESrecordID;
//
//  travelInBreak       = NO_RECORD        - Not applicable
//                      = TRAVELINC_NO     - Travel not included as part of break time
//                      = TRAVELINC_YES    - Travel time included in break time
//
  int   travelInBreak;
//
//  payWhen             = NO_RECORD          - Not applicable
//                      = PAYWHEN_NONE       - None checked
//                      = PAYWHEN_DIRECT     - "B -> C Direct" checked
//                      = PAYWHEN_FROMBIN    - "From B In" checked
//                      = PAYWHEN_OUTTOC     - "Out to C" checked
//                      = PAYWHEN_ABCDFLAG   - "Don't pay when a=b, c=d"
//                      = PAYWHEN_MANIPULATE - "Manipulate travels" checked
//
  int   payWhen;
//
//  Start/End Travel
//
//  paidTravel    = NO_RECORD             Not applicable
//                = TRAVELPAID_NONE       0x0000
//                = TRAVELPAID_START      0x0001
//                = TRAVELPAID_END        0x0002
//                = TRAVELPAID_ABCDFLAG   0x0004
//                = TRAVELPAID_MANIPULATE 0x0010
//
  int  paidTravel;
//
//  startEnd      = NO_RECORD               - Not applicable
//                = PREMIUMS_STARTENDSAME   - Must be the same
//                = PREMIUMS_STARTENDDIFFER - Can differ
//
  int  startEnd;
//
//  startLocation = NO_RECORD                   - Not applicable
//                = PREMIUMS_STARTPOG           - Start at POG
//                = PREMIUMS_STARTPIG           - Start at PIG
//                = PREMIUMS_STARTCLOSESTGARAGE - Start at the closest garage
//                = PREMIUMS_STARTTIMEPOINT     - Start at startNODESrecordID
//
  int  startLocation;
  long startNODESrecordID;
//
//  endLocation   = NO_RECORD                 - Not applicable
//                = PREMIUMS_ENDPOG           - End at POG
//                = PREMIUMS_ENDPIG           - End at PIG
//                = PREMIUMS_ENDCLOSESTGARAGE - End at the closest garage
//                = PREMIUMS_ENDTIMEPOINT     - End at endNODESrecordID
//
//
  int  endLocation;
  long endNODESrecordID;
//
//  beforeTime    = NO_TIME                   - Not applicable
//
  long beforeTime;
} PREMIUMDef;

EXTERN PREMIUMDef PREMIUM[MAXPREMIUMS];
EXTERN int        m_numPremiums;
EXTERN int        m_premiumIndex;

//
//  CUTPARMS
//

#define CUTPARMSFLAGS_SMART          (1<<0)
#define CUTPARMSFLAGS_IGNORE         (1<<1)
#define CUTPARMSFLAGS_IMPROVE        (1<<2)
#define CUTPARMSFLAGS_PEN_TRAVEL     (1<<3)
#define CUTPARMSFLAGS_PEN_MAKEUP     (1<<4)
#define CUTPARMSFLAGS_PEN_OVERTIME   (1<<5)
#define CUTPARMSFLAGS_PEN_PAIDBREAKS (1<<6)
#define CUTPARMSFLAGS_PEN_SPDOT      (1<<7)
#define CUTPARMSFLAGS_PEN_RUNSCUTAT  (1<<8)
#define CUTPARMSFLAGS_DONTCUTAT2     (1<<9)
#define CUTPARMSFLAGS_DONTCUTMIDTRIP (1<<10)
#define CUTPARMSFLAGS_CHECKRUNTYPE   (1<<11)

typedef struct CUTPARMSStruct
{
  long flags;
  int  ignoreMinutes;
  int  cutRuns;
  int  cutDirection;
  long startTime;
  long endTime;
  long minLeftover;
  BOOL runtypes[NUMRUNTYPES][NUMRUNTYPESLOTS];
  char labels[32];
  int  numPenalizedNodes;
  long penalizedNODESrecordIDs[100];
} CUTPARMSDef;

EXTERN CUTPARMSDef CUTPARMS;

//
// The following will hold true for RELIEFPOINTS in a piece:
//
// 1) for the ones in the middle, start.runNumber == end.runNumber
//
// 2) for the ones in the middle, start.pieceNumber == end.pieceNumber
//
// 3) for the ones at the beginning of a block, firstInBlock will be TRUE
//    and end.pieceNumber will be NO_RECORD
//
// 4) for the ones at the end of a block, lastInBlock will be TRUE
//    and start.pieceNumber will be NO_RECORD
//
#define RPFLAGS_INUSE        (1<<0) // If ON, this relief point is in use
#define RPFLAGS_FIRSTINBLOCK (1<<1) // If ON, this relief point is the first one in a block
#define RPFLAGS_LASTINBLOCK  (1<<2) // If ON, this relief point is the last one in a block
#define RPFLAGS_GARAGE       (1<<3) // If ON, this relief point is a garage
#define RPFLAGS_SECURE       (1<<4) // If ON, this relief is a secure location
#define RPFLAGS_SECUREPAIR   (1<<5) // If ON, this relief is part of a secure pair

typedef struct RPSE
{
  long recordID;
  long runNumber;
  long pieceNumber;
  EXTRABOARDDef prior;
  EXTRABOARDDef after;
  long flags;
} RPSEDef;

typedef struct RELIEFPOINTSStruct
{
  long    NODESrecordID;
  long    time;
  long    TRIPSrecordID;
  long    SGRPSERVICESrecordID;
  long    blockNumber;
  RPSEDef start;  // Run attributes starting at this relief point
  RPSEDef end;    // Run attributes ending at this relief point
  char    nodeName[NODES_ABBRNAME_LENGTH + 1];
  char    flags;
  POINT   plot;   // Used by visual runcutter
} RELIEFPOINTSDef;

EXTERN RELIEFPOINTSDef *m_pRELIEFPOINTS;
EXTERN long             m_maxRELIEFPOINTS;
EXTERN int              m_numRELIEFPOINTS;


//#define MAX_RELIEFPOINTS 3072

//EXTERN RELIEFPOINTSDef RELIEFPOINTS[MAX_RELIEFPOINTS];

// This struct is created by the runcut improver and can be accessed by calling
// void	GetImprovedPieceChanges( ImprovedPieceChangeDef **ipc, int *numIPC ).
// It contains a record of any changes made to the piece of the run.
typedef struct ImprovedPieceChangeStruct
{
	long	runNumber;
	long	pieceNumber;

	long	oldStartRelief, oldEndRelief;	// indexes into the RELIEFPOINTS array
	long	newStartRelief,	newEndRelief;	// indexes into the RELIEFPOINTS array
} ImprovedPieceChangeDef;

extern int RuncutImprove(long *savings);
extern void	GetImprovedPieceChanges(ImprovedPieceChangeDef **ipc, int *numIPC);
extern void	FreeImprovedPieceChanges();

// Determines if a relief point is totally matched.
#define IsReliefPointMatched(r) \
		  ( ((r).flags & RPFLAGS_FIRSTINBLOCK) ? (r).start.runNumber != NO_RECORD \
		  : ((r).flags & RPFLAGS_LASTINBLOCK)  ? (r).end.runNumber != NO_RECORD \
		  : (r).start.runNumber != NO_RECORD && (r).end.runNumber != NO_RECORD)

// Determines if this relief point could be the start of a piece.
#define IsFromReliefPointMatched(r) \
		  ( ((r).flags & RPFLAGS_LASTINBLOCK) || (r).start.runNumber != NO_RECORD )

// Determines if this relief point could be the end of a piece.
#define IsToReliefPointMatched(r) \
		  ( ((r).flags & RPFLAGS_FIRSTINBLOCK) || (r).end.runNumber != NO_RECORD )

// Determines if this relief point is in a run at all.
#define IsReliefPointInRun(r) \
		  ( (r).start.runNumber != NO_RECORD || (r).end.runNumber != NO_RECORD )

EXTERN long m_GlobalRunNumber;

//
//  ROSTERPARMS and other rostering crap
//

#define RT_STRAIGHT   (1<<0)
#define RT_TWOPIECE   (1<<1)
#define RT_MULTIPIECE (1<<2)
#define RT_SWING      (1<<3)
#define RT_TRIPPER    (1<<4)
#define RT_ILLEGAL    (1<<5)

#define ROSTERPARMS_FLAG_SAMEROUTE               0x0001
#define ROSTERPARMS_FLAG_SAMERUNNUMBER           0x0002
#define ROSTERPARMS_FLAG_SAMESTART               0x0004
#define ROSTERPARMS_FLAG_SAMEEND                 0x0008
#define ROSTERPARMS_FLAG_BIDBYDRIVERTYPE         0x0010
#define ROSTERPARMS_FLAG_CONSECUTIVEIFALLWEEKDAY 0x0020
#define ROSTERPARMS_FLAG_CONSECUTIVEONLY         0x0040
#define ROSTERPARMS_FLAG_PREFEREXTRASONWEEKENDS  0x0080
#define ROSTERPARMS_FLAG_EQUALIZEWORK            0x0100
#define ROSTERPARMS_FLAG_OFFDAYMUSTFOLLOW        0x0200

typedef struct ROSTERPARMSStruct
{
  int  startDay;
  long serviceDays[ROSTER_MAX_DAYS];
  int  runtypeChoice;
  char match[NUMRUNTYPES][NUMRUNTYPESLOTS][NUMRUNTYPESLOTS];
  long startVariance;
  long minOffTime;
  int  workWeek;
  long flags;
  long minWork;					// Target minimum amount of paid work.
  long maxWork;					// Target maximum amount of paid work.
  int  maxNightWork;
  long nightWorkAfter;
  char daysOffPatterns[128];	// 'Y' if the bit pattern at that index is selected, 'N' if not.
} ROSTERPARMSDef;

EXTERN ROSTERPARMSDef ROSTERPARMS;

EXTERN long RUNSAssignToDIVISION;

EXTERN long specificRun;

EXTERN long m_PremiumRuntype;
EXTERN long m_PremiumTimePeriod;
EXTERN BOOL m_bPremiumNew;

EXTERN long hookedRunRuntype;

EXTERN BOOL rosterChanged;

EXTERN long driversToConsider;

EXTERN char runcosterReason[512];

typedef struct TIDataStruct
{
  long dwellTime;
  long fromNODESrecordID;
  long fromTime;
  long ROUTESrecordID;
  long DIRECTIONSrecordID;
  long toNODESrecordID;
  long toTime;
} TIDataDef;

EXTERN TIDataDef TIData[MAXPIECES * 2];
EXTERN char travelInstructions[MAXPIECES * 2][128];
EXTERN int  numTravelInstructions;
EXTERN BOOL bGenerateTravelInstructions;

typedef struct ROSTERDAYSUNDOSWAPStruct
{
  BOOL bSwap;
  BOOL bDays[ROSTER_MAX_DAYS];
} ROSTERDAYSUNDOSWAPDef;

//
//  Define the functions used by the ideal run coster.
//
#include "tod.h"
typedef struct
{
  tod_t start;
  tod_t length;
} IdealRunPiece;

extern time_t IdealRunCoster(long, IdealRunPiece *, long);
extern BOOL ReliefIsPossible(long, long, long, time_t, long, long, tod_t);

//
//  Visual Runcutter
//

#define BP_DISPLAYED 0x0001
#define BP_SELECTED  0x0002

typedef struct BLOCKPLOTStruct
{
  long blockNumber;
  long startTime;
  long endTime;
  long startIndex;
  long endIndex;
  char flags;
} BLOCKPLOTDef;

EXTERN BLOCKPLOTDef *m_pBP;
EXTERN int numBPBlocks;

//
//  Dynamic travels
//

EXTERN BOOL bUseDynamicTravels;
EXTERN BOOL bUseCISPlan;

//
//  List of node equivalences for dynamic travels
//

#define MAXNODEEQUIV 100
typedef struct NODEEQUIVStruct
{
  long fromNODESrecordID;
  long toNODESrecordID;
} NODEEQUIVDef;

EXTERN int numNodeEquiv;
EXTERN NODEEQUIVDef NODEEQUIV[MAXNODEEQUIV];

typedef struct TMTRIPSStruct
{
  long TRIPSrecordID;
  long tripTime[2];
} TMTRIPSDef;

#define MAXINTRAVELMATRIX 3000
typedef struct TRAVELMATRIXStruct
{
  long fromNODESrecordID;
  long toNODESrecordID;
  int  numTrips;
  int  currentAlloc;
  TMTRIPSDef *pTrips;
} TRAVELMATRIXDef;

EXTERN int numInTM[4];
EXTERN TRAVELMATRIXDef TRAVELMATRIX[4][MAXINTRAVELMATRIX];

EXTERN long garageList[100];
EXTERN long numGaragesInGarageList;

//
//  Passed data stuff for ANALYZERUN
//

#define ANALYZERUN_FLAGS_CHILDWND        0x0001
#define ANALYZERUN_FLAGS_MANCUT          0x0002
#define ANALYZERUN_FLAGS_VISUALRUNCUTTER 0x0004

typedef struct ANALYZERUNPassedDataStruct
{
  long flags;
  long longValue;
  PROPOSEDRUNDef PROPOSEDRUN;
} ANALYZERUNPassedDataDef;

//
//  Visual runcutter colors
//

#define BITMAP_ILLEGALRUN 0
#define BITMAP_LEGALRUN   1
#define BITMAP_FROZENRUN  2
#define BITMAP_UNCUT      3
#define BITMAP_BLANK      4

typedef struct VRCOLORSStruct
{
  COLORREF illegal;
  COLORREF cut;
  COLORREF frozen;
  COLORREF uncut;
} VRCOLORSDef;

#define CUT       0
#define UNCUT     1
#define FROZEN    2
#define ILLEGAL   3
#define HASH      4
#define TXT       5
#define AXIS      6
#define PERIMETER 7
#define WTXT      8
#define LASTPEN   WTXT + 1
#define INDENT   32
#define THICK     4
#define BLEED     THICK / 2

#define BLOCKSDISPLAYED  8
#define NODEOFFSET      22
#define TIMEOFFSET      10
#define VR_INDENT       12
