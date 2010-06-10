//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include <btrconst.h>
#include <btrapi.h>

#include <string.h>

// Since these are database structures, make sure they are packed with
// no padding.
#pragma pack( push, packDefault )
#pragma pack(1)

//
//  Database definitions and file layouts for TMS
//
//  -> Warning <-
//
//  If you are going to increase the length of any fixed-length
//  file past 1280, the following #define must be adjusted to the new length
//
//  -> Warning <-
//
//  If you are going to update any of the layouts of these data files,
//  ensure that the same changes are made (if applicable) to the layouts
//  in the map\win32 directory (MapInfo MapBasic layout files).
//

#define MAX_TMS_FILE_LENGTH 1280

//
//  clientID setup
//

#define MY_THREAD_ID    50

typedef struct clientIDStruct
{
  BTI_CHAR networkAndNode[12];
  BTI_CHAR applicationID[2];
  BTI_WORD threadID;
} clientIDDef;

EXTERN clientIDDef clientID;

//
// AssignRecID takes a btrieve return code and record ID and returns either 1
// (for the first record in a new file), or the next available record ID
//

#define AssignRecID(rcode2,recID)  (rcode2 == 9 ? 1 : recID + 1)

//
//  Btrieve OP codes
//

#define B_OPEN                       0
#define B_CLOSE                      1
#define B_INSERT                     2
#define B_UPDATE                     3
#define B_DELETE                     4
#define B_GETEQUAL                   5
#define B_GETNEXT                    6
#define B_GETPREVIOUS                7
#define B_GETGREATER                 8
#define B_GETGREATEROREQUAL          9
#define B_GETLESSTHAN               10
#define B_GETLESSTHANOREQUAL        11
#define B_GETFIRST                  12
#define B_GETLAST                   13
#define B_CREATE                    14
#define B_STAT                      15
#define B_EXTEND                    16
#define B_SETDIRECTORY              17
#define B_GETDIRECTORY              18
#define B_BEGINTRANSACTION          19
#define B_ENDTRANSACTION            20
#define B_ABORTTRANSACTION          21
#define B_GETPOSITION               22
#define B_GETDIRECT                 23
#define B_STEPNEXT                  24
#define B_STOP                      25
#define B_VERSION                   26
#define B_UNLOCK                    27
#define B_RESET                     28
#define B_SETOWNER                  29
#define B_CLEAROWNER                30
#define B_CREATESUPPLEMENTALINDEX   31
#define B_DROPSUPPLEMENTALINDEX     32
#define B_STEPFIRST                 33
#define B_STEPLAST                  34
#define B_STEPPREVIOUS              35
#define B_GETNEXTEXTENDED           36
#define B_GETPREVIOUSEXTENDED       37
#define B_STEPNEXTEXTENDED          38
#define B_STEPPREVIOUSEXTENDED      39
#define B_INSERTEXTENDED            40
#define B_GETKEY                    50
#define B_SINGLERECORDWAITLOCK     100
#define B_SINGLERECORDNOWAITLOCK   200
#define B_MULTIPLERECORDWAITLOCK   300
#define B_MULTIPLERECORDNOWAITLOCK 400

//
// Position block and record length Btrieve definitions
//

EXTERN char      positionBlock[TMS_LASTFILE + 1][128];
EXTERN short int recordLength[TMS_LASTFILE + 1];

//
//  BSTATDef: Btrieve file status structure
//
//  BSKEY is sized to allow btrieve to return 48 key vaules,
//  each representing the (potential) characteristics of one
//  key segment.
//
//  48 represents 24 maximum key segments per file, times 2, for
//  additional records if there's an alternate collating sequence
//  on any of each of the 24 segments.
//
//  File Flags: (0) 0x0001 - The file allows variable length records
//              (1) 0x0002 - Btrieve truncates trailing blanks in variable length records
//              (2) 0x0004 - Btrieve preallocated pages for the file
//              (3) 0x0008 - Btrieve compresses the data in the file
//              (4) 0x0010 - Btrieve created the file as a key-only file
//              (5) 0x0020 - Btrieve maintains a 10% free space threshold
//              (6) 0x0040 - Btrieve maintains a 20% free space threshold*
//
//              * Btrieve maintains a 30% free space
//                threshold if bit 5 and bit 6 are both 1
//
//   Key Flags: (0) 0x0001 - The key allows duplicates
//              (1) 0x0002 - The key is modifiable
//              (2) 0x0004 - The key is binary*
//              (3) 0x0008 - The key has a null value
//              (4) 0x0010 - The key has another segment
//              (5) 0x0020 - The key is sorted by an alternate collating sequence
//              (6) 0x0040 - The key is sorted in descending order
//              (7) 0x0080 - The key is a supplemental index
//              (8) 0x0100 - The key is an extended type
//              (9) 0x0200 - The key is manual
//
//              * If bit 2 and bit 8 are both 0, the key is string type
//                If bit 2 is 1 and bit 8 is 0,  the key is binary type
//

typedef struct BSKEYStruct
{
  short int keyPosition;
  short int keyLength;
  short int keyFlag;
  long      numKeys;
  char      extendedKeyType;
  char      nullValue;
  long      reserved;
} BSKEYDef;

#define BSTAT_NUM_KEYSEGMENTS 48
typedef struct BSTATStruct
{
  short int recordLength;
  short int pagesize;
  short int numIndexes;
  long      numRecords;
  short int fileFlags;
  short int reserved;
  short int unusedPages;
  BSKEYDef  BSKEY[BSTAT_NUM_KEYSEGMENTS];
} BSTATDef;

EXTERN BSTATDef BSTAT;
EXTERN BSKEYDef BSKey;

//
//  DDF File Numbers
//
EXTERN short int DDFFileNumbers[TMS_LASTFILE + 1 + VIEW_LASTVIEW + 1];

//
//  DDF Data Structures
//
//  Key 0 is the same for DDF files FILE, FIELD, and INDEX
//
typedef struct DDFKey0Struct
{
  short int Id;
} DDFKey0Def;

//
//  FILE DDF Structure
//  Record length: 97
//  Defined flags: (4) 0x0010 - Dictionary file
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-2   File ID
//                   3-22  File Name
//                  23-86  File Location
//                    87   Flags
//                  88-97  Reserved
//
#define DDF_NAME_LENGTH         20
#define DDF_LOC_LENGTH          64
#define FILEDDF_RESERVED_LENGTH 10
typedef struct FILEDDFStruct
{
  short int Id;
  char      Name[DDF_NAME_LENGTH];
  char      Loc[DDF_LOC_LENGTH];
  char      Flags;
  char      reserved[FILEDDF_RESERVED_LENGTH];
} FILEDDFDef;

typedef struct FILEDDFKey1Struct
{
  char Name[DDF_NAME_LENGTH];
} FILEDDFKey1Def;

EXTERN FILEDDFDef     FILEDDF;
EXTERN DDFKey0Def     FILEDDFKey0;
EXTERN FILEDDFKey1Def FILEDDFKey1;
EXTERN char           szDatabase[TMS_LASTFILE + 1][DDF_LOC_LENGTH + 1];

//
//  FIELD DDF Structure
//  Record length: 32
//  Defined flags: (0) 0x0001 - Case flag for strings
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-2   Field ID
//                   3-4   File ID from FILE.DDF
//                   5-24  Field Name
//                    25   Btrieve Data Type
//                  26-27  Offset in file
//                  28-29  Field size
//                    30   Number of Decimal Places
//                  31-32  Flags
//
typedef struct FIELDDDFStruct
{
  short int Id;
  short int File;
  char      Name[DDF_NAME_LENGTH];
  char      DataType;
  short int Offset;
  short int Size;
  char      Dec;
  short int Flags;
} FIELDDDFDef;

typedef struct FIELDDDFKey1Struct
{
  short int File;
} FIELDDDFKey1Def;

typedef struct FIELDDDFKey2Struct
{
  char Name[DDF_NAME_LENGTH];
} FIELDDDFKey2Def;

typedef struct FIELDDDFKey3Struct
{
  short int File;
  char      Name[DDF_NAME_LENGTH];
} FIELDDDFKey3Def;

EXTERN FIELDDDFDef     FIELDDDF;
EXTERN DDFKey0Def      FIELDDDFKey0;
EXTERN FIELDDDFKey1Def FIELDDDFKey1;
EXTERN FIELDDDFKey2Def FIELDDDFKey2;
EXTERN FIELDDDFKey3Def FIELDDDFKey3;

//
//  INDEX DDF Structure
//  Record length: 10
//  Defined flags: (0) 0x0001 - Index allows duplicates
//                 (1) 0x0002 - Index is modifiable
//                 (2) 0x0004 - Reserved
//                 (3) 0x0008 - Reserved
//                 (4) 0x0010 - Another segment is concatenated to this one in the index
//                 (5) 0x0020 - Index is not case-sensitive
//                 (6) 0x0040 - Index is in descending order
//                 (7) 0x0080 - Index is a supplemental index
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-2   File ID
//                   3-4   Field ID
//                   5-6   Index Number (0 through 23)
//                   7-8   Segment Part Number (0 through 23)
//                   9-10  Flags
//
typedef struct INDEXDDFStruct
{
  short int File;
  short int Field;
  short int Number;
  short int Part;
  short int Flags;
} INDEXDDFDef;

typedef struct INDEXDDFKey1Struct
{
  short int Field;
} INDEXDDFKey1Def;

EXTERN INDEXDDFDef     INDEXDDF;
EXTERN DDFKey0Def      INDEXDDFKey0;
EXTERN INDEXDDFKey1Def INDEXDDFKey1;

//
//  ATTRIBUTES Structure
//  Record length: 64
//  Defined flags: (0) 0x0001 - Hidden
//                 (1) 0x0002 - Derived
//                 (2) 0x0004 - Formatted
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-2   Field ID
//                   3-22  Field Name (Used to over-ride internal name)
//                  23-24  "Data is derived from" Table ID
//                  25-26  "Data is derived from" Field ID
//                  27-28  "Formatted" Data Type
//                  29-30  Column sequence in display
//                  31-60  Reserved
//                  61-64  Flags
//

#define ATTRIBUTES_FLAG_HIDDEN       0x0001
#define ATTRIBUTES_FLAG_DERIVED      0x0002
#define ATTRIBUTES_FLAG_FORMATTED    0x0004
#define ATTRIBUTES_FIELDTITLE_LENGTH DDF_NAME_LENGTH
#define ATTRIBUTES_RESERVED_LENGTH   30

typedef struct ATTRIBUTESStruct
{
  short int Id;
  char      fieldTitle[ATTRIBUTES_FIELDTITLE_LENGTH];
  short int derivedTableId;
  short int derivedFieldId;
  short int formattedDataType;
  short int sequence;
  char      reserved[ATTRIBUTES_RESERVED_LENGTH];
  long      flags;
} ATTRIBUTESDef;

typedef struct ATTRIBUTESKey0Struct
{
  short int Id;
} ATTRIBUTESKey0Def;

EXTERN ATTRIBUTESDef     ATTRIBUTES;
EXTERN ATTRIBUTESKey0Def ATTRIBUTESKey0;

//
//  CREATED Structure
//  Record length: 64
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-2   Field ID
//                   3-4   File ID
//                   5-24  Field Name
//                  25-26  Field Sequence
//                  27-28  "From" File ID (Used to replicate data items)
//                  29-30  "From" Field ID
//                  31-60  Reserved
//                  61-64  Flags
//
#define CREATED_NAME_LENGTH     DDF_NAME_LENGTH
#define CREATED_RESERVED_LENGTH 30

typedef struct CREATEDStruct
{
  short int Id;
  short int File;
  char      Name[CREATED_NAME_LENGTH];
  short int Sequence;
  short int FromFileId;
  short int FromFieldId;
  char      reserved[CREATED_RESERVED_LENGTH];
  long      flags;
} CREATEDDef;

typedef struct CREATEDKey1Struct
{
  short int File;
  short int Sequence;
} CREATEDKey1Def;

typedef struct CREATEDKey2Struct
{
  short int File;
  char Name[DDF_NAME_LENGTH];
} CREATEDKey2Def;

EXTERN CREATEDDef     CREATED;
EXTERN DDFKey0Def     CREATEDKey0;
EXTERN CREATEDKey1Def CREATEDKey1;
EXTERN CREATEDKey2Def CREATEDKey2;

//
//  TABLEVIEW Table Structure - See definition for FILE DDF (above)
//  Record length: 97
//  Defined flags: (4) 0x0010 - Dictionary file
//
//
#define TABLEVIEW_NAME_LENGTH     DDF_NAME_LENGTH        
#define TABLEVIEW_LOC_LENGTH      DDF_LOC_LENGTH         
#define TABLEVIEW_RESERVED_LENGTH FILEDDF_RESERVED_LENGTH

EXTERN FILEDDFDef     TABLEVIEW;
EXTERN DDFKey0Def     TABLEVIEWKey0;
EXTERN FILEDDFKey1Def TABLEVIEWKey1;

//
//  TMS Data Structures
//
//  Key 0 is the same for all TMS files
//
typedef struct TMSKey0Struct
{
  long recordID;
} TMSKey0Def;

//
//  DIRECTIONS Table Structure
//  Record length: 32
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-10  Abbreviated Direction Name
//                  11-26  Long Direction Name
//                  27-28  Reserved
//                  29-32  Record Flags
//
#define DIRECTIONS_ABBRNAME_LENGTH  2
#define DIRECTIONS_LONGNAME_LENGTH 16
#define DIRECTIONS_RESERVED_LENGTH  2
typedef struct DIRECTIONSStruct
{
  long recordID;
  long COMMENTSrecordID;
  char abbrName[DIRECTIONS_ABBRNAME_LENGTH];
  char longName[DIRECTIONS_LONGNAME_LENGTH];
  char reserved[DIRECTIONS_RESERVED_LENGTH];
  long flags;
} DIRECTIONSDef;

typedef struct DIRECTIONSKey1Struct
{
  char longName[DIRECTIONS_LONGNAME_LENGTH];
} DIRECTIONSKey1Def;

EXTERN DIRECTIONSDef     DIRECTIONS;
EXTERN TMSKey0Def        DIRECTIONSKey0;
EXTERN DIRECTIONSKey1Def DIRECTIONSKey1;

//
//  ROUTES Table Structure
//  Record length: 128
//  Defined flags: (0) 0x0001 - Employee Shuttle
//                 (1) 0x0002 - School Route
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Directions Table Record ID Pointer (1/2)
//                  13-16  Directions Table Record ID Pointer (2/2)
//                  17-24  Route Number (Stored as an 8-byte character)
//                  25-80  Route Name
//                  81-84  Alternate route number (numeric)
//                  85-90  Outbound Bay NODESrecordID
//                  91-92  Inbound Bay NODESrecordID
//                  93-96  Record Flags
//
#define ROUTES_FLAG_EMPSHUTTLE  0x0001
#define ROUTES_FLAG_SCHOOL      0x0002
#define ROUTES_FLAG_RGRP        0x0004
#define ROUTES_MAX_DIRECTIONS   2
#define ROUTES_NUMBER_LENGTH    8
#define ROUTES_NAME_LENGTH     60
#define ROUTES_RESERVED_LENGTH  0
typedef struct ROUTESStruct
{
  long recordID;
  long COMMENTSrecordID;
  long DIRECTIONSrecordID[ROUTES_MAX_DIRECTIONS];
  char number[ROUTES_NUMBER_LENGTH];
  char name[ROUTES_NAME_LENGTH];
  long alternate;
  short int OBBayNODESrecordID;  // Temporarily short int until more space
  short int IBBayNODESrecordID;  // Temporarily short int until more space
//  char reserved[ROUTES_RESERVED_LENGTH];
  long flags;
} ROUTESDef;

typedef struct ROUTESKey1Struct
{
  char number[ROUTES_NUMBER_LENGTH];
} ROUTESKey1Def;

EXTERN ROUTESDef     ROUTES;
EXTERN TMSKey0Def    ROUTESKey0;
EXTERN ROUTESKey1Def ROUTESKey1;

//
//  SERVICES Table Structure
//  Record length: 64
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Sort Number
//                  13-44  Service Name
//                  45-60  Reserved
//                  61-64  Record Flags
//
//  Note: This record layout / data structure definition
//        is also used by: JURISDICTIONS
//                         DIVISIONS
//                         BUSTYPES (OLD)
//
#define ORDEREDLIST_NAME_LENGTH     32
#define ORDEREDLIST_RESERVED_LENGTH 16
#define SERVICES_NAME_LENGTH        ORDEREDLIST_NAME_LENGTH
#define SERVICES_RESERVED_LENGTH    ORDEREDLIST_RESERVED_LENGTH
typedef struct ORDEREDLISTStruct
{
  long recordID;
  long COMMENTSrecordID;
  long number;
  char name[ORDEREDLIST_NAME_LENGTH];
  char reserved[ORDEREDLIST_RESERVED_LENGTH];
  long flags;
} ORDEREDLISTDef;

typedef struct ORDEREDLISTKey1Struct
{
  long number;
} ORDEREDLISTKey1Def;

EXTERN ORDEREDLISTDef     SERVICES;
EXTERN TMSKey0Def         SERVICESKey0;
EXTERN ORDEREDLISTKey1Def SERVICESKey1;

//
//  DIVISIONS Table Structure - See definition for ORDEREDLIST (above)
//  Record length: 64
//  Defined flags: None
//
#define DIVISIONS_NAME_LENGTH     ORDEREDLIST_NAME_LENGTH
#define DIVISIONS_RESERVED_LENGTH ORDEREDLIST_RESERVED_LENGTH
EXTERN ORDEREDLISTDef     DIVISIONS;
EXTERN TMSKey0Def         DIVISIONSKey0;
EXTERN ORDEREDLISTKey1Def DIVISIONSKey1;

//
//  JURISDICTIONS Table Structure - See definition for ORDEREDLIST (above)
//  Record length: 64
//  Defined flags: None
//
#define JURISDICTIONS_NAME_LENGTH     ORDEREDLIST_NAME_LENGTH
#define JURISDICTIONS_RESERVED_LENGTH ORDEREDLIST_RESERVED_LENGTH
EXTERN ORDEREDLISTDef      JURISDICTIONS;
EXTERN TMSKey0Def          JURISDICTIONSKey0;
EXTERN ORDEREDLISTKey1Def  JURISDICTIONSKey1;

//
//  OLDNODES Table Structure
//  Record length: 128
//  Defined flags: (0) 0x0001 - Garage Flag
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Jurisdictions Table Record ID Pointer
//                  13-16  Abbreviated Node Name
//                  17-24  Long Node Name
//                  25-88  Node Intersection / Address
//                  89-92  Relief Labels
//                  93-96  Longitude
//                  97-100 Latitude
//                 101-104 Node number
//                 105-108 Timepoint Map Codes
//                 109-120 Reserved
//                 121-124 Stop Flags
//                 125-128 Record Flags
//
#define OLDNODES_FLAG_GARAGE          0x0001
#define OLDNODES_FLAG_STOP            0x0002
#define OLDNODES_FLAG_SECURE          0x8000
#define OLDNODES_MAXSTOPFLAGS        32
#define OLDNODES_ABBRNAME_LENGTH      4
#define OLDNODES_LONGNAME_LENGTH      8
#define OLDNODES_INTERSECTION_LENGTH 64
#define OLDNODES_RELIEFLABELS_LENGTH  4
#define OLDNODES_MAPCODES_LENGTH      4
#define OLDNODES_RESERVED_LENGTH     12
typedef struct OLDNODESStruct
{
  long  recordID;
  long  COMMENTSrecordID;
  long  JURISDICTIONSrecordID;
  char  abbrName[OLDNODES_ABBRNAME_LENGTH];
  char  longName[OLDNODES_LONGNAME_LENGTH];
  char  intersection[OLDNODES_INTERSECTION_LENGTH];
  char  reliefLabels[OLDNODES_RELIEFLABELS_LENGTH];
  float longitude;
  float latitude;
  long  number;
  char  mapCodes[OLDNODES_MAPCODES_LENGTH];
  char  reserved[OLDNODES_RESERVED_LENGTH];
  long  stopFlags;
  long  flags;
} OLDNODESDef;

typedef struct OLDNODESKey1Struct
{
  long flags;
  char abbrName[OLDNODES_ABBRNAME_LENGTH];
} OLDNODESKey1Def;

typedef struct OLDNODESKey2Struct
{
  char abbrName[OLDNODES_ABBRNAME_LENGTH];
} OLDNODESKey2Def;

EXTERN OLDNODESDef     OLDNODES;
EXTERN TMSKey0Def      OLDNODESKey0;
EXTERN OLDNODESKey1Def OLDNODESKey1;
EXTERN OLDNODESKey2Def OLDNODESKey2;
//
//  NODES Table Structure
//  Record length: 128
//  Defined flags: (0) 0x0001 - Garage Flag
//                 (1) 0x0002 - Stop Only
//                 (2) 0x0004 - User defined
//                 (3) 0x0008 - User defined
//                 (4) 0x0010 - User defined
//                 (5) 0x0020 - User defined
//                 (6) 0x0040 - User defined
//                 (7) 0x0080 - User defined
//                 (8) 0x0100 - User defined
//                 (9) 0x0200 - User defined
//                (10) 0x0400 - User defined
//                (11) 0x0800 - User defined
//                (12) 0x1000 - User defined
//                (13) 0x2000 - User defined
//                (14) 0x4000 - User defined
//                (15) 0x8000 - Secure Location
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Jurisdictions Table Record ID Pointer
//                  13-16  Abbreviated Node Name
//                  17-24  Long Node Name
//                  25-88  Node description (for reports/downloads)
//                  89-152 Intersection/Address (for geocoding)
//                 153-160 Relief Labels
//                 161-164 Longitude
//                 165-168 Latitude
//                 169-172 Node number
//                 173-180 Timepoint Map Codes
//                 181-184 Node capacity
//                 185-188 OBStopNumber
//                 189-192 IBStopNumber
//                 193-240 AVLStopName
//                 241-248 Reserved
//                 248-252 Stop Flags
//                 253-256 Record Flags
//
#define NODES_FLAG_GARAGE           0x0001
#define NODES_FLAG_STOP             0x0002
#define NODES_FLAG_SECURE           0x8000
#define NODES_FLAG_AVLTRANSFERPOINT 0x0004
#define NODES_MAXSTOPFLAGS        32
#define NODES_ABBRNAME_LENGTH      4  // Note - also occurs in TMSHeader.h
#define NODES_LONGNAME_LENGTH      8
#define NODES_DESCRIPTION_LENGTH  64
#define NODES_INTERSECTION_LENGTH 64
#define NODES_RELIEFLABELS_LENGTH  8
#define NODES_MAPCODES_LENGTH      8
#define NODES_AVLSTOPNAME_LENGTH  48
#define NODES_RESERVED_LENGTH      8
typedef struct NODESStruct
{
  long  recordID;
  long  COMMENTSrecordID;
  long  JURISDICTIONSrecordID;
  char  abbrName[NODES_ABBRNAME_LENGTH];
  char  longName[NODES_LONGNAME_LENGTH];
  char  intersection[NODES_INTERSECTION_LENGTH];
  char  description[NODES_DESCRIPTION_LENGTH];
  char  reliefLabels[NODES_RELIEFLABELS_LENGTH];
  float longitude;
  float latitude;
  long  number;
  char  mapCodes[NODES_MAPCODES_LENGTH];
  long  capacity;
  long  OBStopNumber;
  long  IBStopNumber;
  char  AVLStopName[NODES_AVLSTOPNAME_LENGTH];
  char  reserved[NODES_RESERVED_LENGTH];
  long  stopFlags;
  long  flags;
} NODESDef;

typedef struct NODESKey1Struct
{
  long flags;
  char abbrName[NODES_ABBRNAME_LENGTH];
} NODESKey1Def;

typedef struct NODESKey2Struct
{
  char abbrName[NODES_ABBRNAME_LENGTH];
} NODESKey2Def;

EXTERN NODESDef     NODES;
EXTERN TMSKey0Def   NODESKey0;
EXTERN NODESKey1Def NODESKey1;
EXTERN NODESKey2Def NODESKey2;

//
//  PATTERNS Table Structure
//  Record length: 64
//  Defined flags: (0) 0x0001 - Maximum Load Point
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Routes Table Record ID Pointer
//                  13-16  Services Table Record ID Pointer
//                  17-20  Direction Index (0/1, as per Routes Table)
//                  21-24  Pattern Names Table Record ID Pointer
//                  25-28  Nodes Table Record ID Pointer
//                  29-32  Node Sequence Number
//                  33-44  "From" Text
//                  47-56  "To" Text
//                  57-60  Reserved
//                  61-64  Record Flags
//
#define PATTERNS_FLAG_MLP        0x0001
#define PATTERNS_FLAG_BUSSTOP    0x0002
#define PATTERNS_RESERVED_LENGTH 4
#define PATTERNS_FROMTEXT_LENGTH 12
#define PATTERNS_TOTEXT_LENGTH   12
typedef struct PATTERNSStruct
{
  long recordID;
  long COMMENTSrecordID;
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long PATTERNNAMESrecordID;
  long NODESrecordID;
  long nodeSequence;
  char fromText[PATTERNS_FROMTEXT_LENGTH];
  char toText[PATTERNS_TOTEXT_LENGTH];
  char reserved[PATTERNS_RESERVED_LENGTH];
  long flags;
} PATTERNSDef;

typedef struct PATTERNSKey1Struct
{
  long NODESrecordID;
} PATTERNSKey1Def;

typedef struct PATTERNSKey2Struct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long PATTERNNAMESrecordID;
  long nodeSequence;
} PATTERNSKey2Def;

EXTERN PATTERNSDef     PATTERNS;
EXTERN TMSKey0Def      PATTERNSKey0;
EXTERN PATTERNSKey1Def PATTERNSKey1;
EXTERN PATTERNSKey2Def PATTERNSKey2;

//
//  PATTERNNAMES Table Structure
//  Record length: 32
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-24  Pattern Name
//                  25-28  Reserved
//                  29-32  Record Flags
//
//  Note: This record layout / data structure definition
//        is also used by: BLOCKNAMES
//
#define NORMNAMES_NAME_LENGTH        16
#define NORMNAMES_RESERVED_LENGTH     4
#define PATTERNNAMES_NAME_LENGTH     NORMNAMES_NAME_LENGTH
#define PATTERNNAMES_RESERVED_LENGTH NORMNAMES_RESERVED_LENGTH
typedef struct NORMNAMESStruct
{
  long recordID;
  long COMMENTSrecordID;
  char name[NORMNAMES_NAME_LENGTH];
  char reserved[NORMNAMES_RESERVED_LENGTH];
  long flags;
} NORMNAMESDef;

typedef struct NORMNAMESKey1Struct
{
  char name[NORMNAMES_NAME_LENGTH];
} NORMNAMESKey1Def;

EXTERN NORMNAMESDef     PATTERNNAMES;
EXTERN TMSKey0Def       PATTERNNAMESKey0;
EXTERN NORMNAMESKey1Def PATTERNNAMESKey1;

//
//  CONNECTIONS Table Structure
//  Record length: 64
//  Defined flags: (0) 0x0001 - Two-way
//                 (1) 0x0002 - Running Time
//                 (2) 0x0004 - Travel Time
//                 (3) 0x0008 - Deadhead Time
//                 (4) 0x0010 - Node equivalence
//                 (5) 0x0020 - This is a stop-stop entry
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  "From" Nodes Table Record ID Pointer
//                  13-16  "From" Routes Table Record ID Pointer
//                  17-20  "From" Services Table Record ID Pointer
//                  21-24  "From" Pattern Names Table Record ID Pointer
//                  25-28  "To" Nodes Table Record ID Pointer
//                  29-32  "To" Routes Table Record ID Pointer
//                  33-36  "To" Services Table Record ID Pointer
//                  37-40  "To" Pattern Names Table Record ID Pointer
//                  41-44  Connection Time
//                  45-48  From Time of Day
//                  49-52  To Time of Day
//                  53-56  User-defined distance
//                  57-60  Routings Table Record ID (Forward pointer)
//                  61-64  Record Flags
//
#define CONNECTIONS_FLAG_TWOWAY        0x0001
#define CONNECTIONS_FLAG_RUNNINGTIME   0x0002
#define CONNECTIONS_FLAG_TRAVELTIME    0x0004
#define CONNECTIONS_FLAG_DEADHEADTIME  0x0008
#define CONNECTIONS_FLAG_EQUIVALENT    0x0010
#define CONNECTIONS_FLAG_STOPSTOP      0x0020
#define CONNECTIONS_FLAG_AVLEQUIVALENT 0x0040
#define CONNECTIONS_RESERVED_LENGTH   4
typedef struct CONNECTIONSStruct
{
  long  recordID;
  long  COMMENTSrecordID;
  long  fromNODESrecordID;
  long  fromROUTESrecordID;
  long  fromSERVICESrecordID;
  long  fromPATTERNNAMESrecordID;
  long  toNODESrecordID;
  long  toROUTESrecordID;
  long  toSERVICESrecordID;
  long  toPATTERNNAMESrecordID;
  long  connectionTime;
  long  fromTimeOfDay;
  long  toTimeOfDay;
  float distance;
  long  flags;
  long  ROUTINGSrecordID;
} CONNECTIONSDef;

typedef struct CONNECTIONSKey1Struct
{
  long fromNODESrecordID;
  long toNODESrecordID;
  long fromTimeOfDay;
} CONNECTIONSKey1Def;

EXTERN CONNECTIONSDef     CONNECTIONS;
EXTERN TMSKey0Def         CONNECTIONSKey0;
EXTERN CONNECTIONSKey1Def CONNECTIONSKey1;

//
//  ROUTINGS Table Structure
//  Record length: 64
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Connections Table Record ID (Backward pointer)
//                   9-12  Sequence number
//                  13-52  Street name
//                  53-60  Reserved
//                  61-64  Flags
// 
//  Defined flags: (0) 0x00000001 - Instruction: Start
//                 (1) 0x00000002 - Instruction: Turn left onto
//                 (2) 0x00000004 - Instruction: Turn right onto
//                 (3) 0x00000008 - Instruction: Bear left onto
//                 (4) 0x00000010 - Instruction: Bear right onto
//                 (5) 0x00000020 - Instruction: Stay on
//                 (6) 0x00000040 - Instruction: End
//                 (7) 0x00000080 - * Unused *
//                 (8) 0x00000100 - Direction: N
//                 (9) 0x00000200 - Direction: NE
//                (10) 0x00000400 - Direction: E
//                (11) 0x00000800 - Direction: SE
//                (12) 0x00001000 - Direction: S
//                (13) 0x00002000 - Direction: SW
//                (14) 0x00004000 - Direction: W
//                (15) 0x00008000 - Direction: NW
//                (16) 0x00001000 - Instruction: U Turn
//                (17) 0x00002000 - Instruction: Turn back to you
//
#define ROUTINGS_STREETNAME_LENGTH 40
#define ROUTINGS_RESERVED_LENGTH    8
#define ROUTINGS_FLAG_INSTRUCTION_START         0x00000001
#define ROUTINGS_FLAG_INSTRUCTION_TURNLEFTONTO  0x00000002
#define ROUTINGS_FLAG_INSTRUCTION_TURNRIGHTONTO 0x00000004
#define ROUTINGS_FLAG_INSTRUCTION_BEARLEFTONTO  0x00000008
#define ROUTINGS_FLAG_INSTRUCTION_BEARRIGHTONTO 0x00000010
#define ROUTINGS_FLAG_INSTRUCTION_STAYON        0x00000020
#define ROUTINGS_FLAG_INSTRUCTION_END           0x00000040
#define ROUTINGS_FLAG_DIRECTION_N               0x00000100
#define ROUTINGS_FLAG_DIRECTION_NE              0x00000200
#define ROUTINGS_FLAG_DIRECTION_E               0x00000400
#define ROUTINGS_FLAG_DIRECTION_SE              0x00000800
#define ROUTINGS_FLAG_DIRECTION_S               0x00001000
#define ROUTINGS_FLAG_DIRECTION_SW              0x00002000
#define ROUTINGS_FLAG_DIRECTION_W               0x00004000
#define ROUTINGS_FLAG_DIRECTION_NW              0x00008000
#define ROUTINGS_FLAG_INSTRUCTION_UTURN         0x00010000
#define ROUTINGS_FLAG_INSTRUCTION_TURNBACKTOYOU 0x00020000

typedef struct ROUTINGSStruct
{
  long recordID;
  long CONNECTIONSrecordID;
  long sequenceNumber;
  char streetName[ROUTINGS_STREETNAME_LENGTH];
  char reserved[ROUTINGS_RESERVED_LENGTH];
  long flags;
} ROUTINGSDef;

typedef struct ROUTINGSKey1Struct
{
  long CONNECTIONSrecordID;
  long sequenceNumber;
} ROUTINGSKey1Def;

EXTERN ROUTINGSDef     ROUTINGS;
EXTERN TMSKey0Def      ROUTINGSKey0;
EXTERN ROUTINGSKey1Def ROUTINGSKey1;

//
//  BUSTYPES (OLD) Table Structure - See definition for ORDEREDLIST (above)
//  Record length: 64
//  Defined flags: (0) 0x0001 - Accessible
//                 (1) 0x0002 - (Bicycle) Carrier Equipped
//                 (2) 0x0004 - This is a surface vehicle
//
#define OLD_BUSTYPES_FLAG_ACCESSIBLE 0x0001
#define OLD_BUSTYPES_FLAG_CARRIER    0x0002
#define OLD_BUSTYPES_FLAG_SURFACE    0x0004
#define OLD_BUSTYPES_NAME_LENGTH     ORDEREDLIST_NAME_LENGTH
#define OLD_BUSTYPES_RESERVED_LENGTH ORDEREDLIST_RESERVED_LENGTH
EXTERN ORDEREDLISTDef     OLD_BUSTYPES;
EXTERN TMSKey0Def         OLD_BUSTYPESKey0;
EXTERN ORDEREDLISTKey1Def OLD_BUSTYPESKey1;

//
//  BUSTYPES Table Structure
//  Record length: 128
// 
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Sort number
//                  13-44  Fleet name
//                  45-76  Bustype name
//                  77-80  Capacity (seated)
//                  81-84  Capacity (standing)
//                  85-100 Numbered "from" (no longer used)
//                 101-116 Numbered "to" (no longer used)   
//                 117-120 Operations factor (no longer used)
//                 121-124 Record flags
//                 125-128 Reserved
//
//  Defined flags: (0) 0x0001 - Accessible
//                 (1) 0x0002 - (Bicycle) Carrier Equipped
//                 (2) 0x0004 - This is a surface vehicle
//                 (3) 0x0008 - This is a rail vehicle
//
#define BUSTYPES_FLAG_ACCESSIBLE  0x0001
#define BUSTYPES_FLAG_CARRIER     0x0002
#define BUSTYPES_FLAG_SURFACE     0x0004
#define BUSTYPES_FLAG_RAIL        0x0008
#define BUSTYPES_NAME_LENGTH      32
#define BUSTYPES_FLEETNAME_LENGTH 32
#define BUSTYPES_NUMBER_LENGTH    16
#define BUSTYPES_RESERVED_LENGTH  4

typedef struct BUSTYPESStruct
{
  long  recordID;
  long  COMMENTSrecordID;
  long  number;
  char  fleetName[BUSTYPES_FLEETNAME_LENGTH];
  char  name[BUSTYPES_NAME_LENGTH];
  long  capacitySeated;
  long  capacityStanding;
  char  numberedFrom[BUSTYPES_NUMBER_LENGTH];
  char  numberedTo[BUSTYPES_NUMBER_LENGTH];
  float opsFactor;
  long  flags;
  char  reserved[BUSTYPES_RESERVED_LENGTH];
} BUSTYPESDef;

typedef struct BUSTYPESKey1Struct
{
  long sortNumber;
} BUSTYPESKey1Def;

EXTERN BUSTYPESDef     BUSTYPES;
EXTERN TMSKey0Def      BUSTYPESKey0;
EXTERN BUSTYPESKey1Def BUSTYPESKey1;

//
//  BUSES Table Structure
//  Record length: 128
// 
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Bustypes Table Record ID Pointer
//                  13-28  Bus number (char)
//                  29-60  Reserved
//                  61-64  Record flags
//
//  Defined flags: (0) 0x0001 - Retired
//
#define BUSES_FLAG_RETIRED     0x0001
#define BUSES_NUMBER_LENGTH    16
#define BUSES_RESERVED_LENGTH  32

typedef struct BUSESStruct
{
  long recordID;
  long COMMENTSrecordID;
  long BUSTYPESrecordID;
  char number[BUSES_NUMBER_LENGTH];
  char reserved[BUSES_RESERVED_LENGTH];
  long flags;
} BUSESDef;

typedef struct BUSESKey1Struct
{
  char number[BUSES_NUMBER_LENGTH];
} BUSESKey1Def;

typedef struct BUSESKey2Struct
{
  long flags;
  char number[BUSES_NUMBER_LENGTH];
} BUSESKey2Def;

EXTERN BUSESDef     BUSES;
EXTERN TMSKey0Def   BUSESKey0;
EXTERN BUSESKey1Def BUSESKey1;
EXTERN BUSESKey2Def BUSESKey2;
//
//  COMMENTS Table Structure
//  Record length: Variable: Fixed portion = 32
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-12  Comment Code
//                  13-28  Reserved
//                  29-32  Record Flags
//                  33-4032 Comment Text (variable)
//
#define COMMENTS_CODE_LENGTH           8
#define COMMENTS_RESERVED_LENGTH      16
#define COMMENTS_FIXED_LENGTH         32
#define COMMENTS_VARIABLE_LENGTH    4000
#define COMMENTS_TEXTDISPLAY_LENGTH   80
#define COMMENTS_TOTAL_LENGTH COMMENTS_FIXED_LENGTH + COMMENTS_VARIABLE_LENGTH
typedef struct COMMENTSStruct
{
  long recordID;
  char code[COMMENTS_CODE_LENGTH];
  char reserved[COMMENTS_RESERVED_LENGTH];
  long flags;
} COMMENTSDef;

typedef struct COMMENTSKey1Struct
{
  char code[COMMENTS_CODE_LENGTH];
} COMMENTSKey1Def;

EXTERN COMMENTSDef     COMMENTS;
EXTERN TMSKey0Def      COMMENTSKey0;
EXTERN COMMENTSKey1Def COMMENTSKey1;

//
//  SIGNCODES Table Structure
//  Record length: 64
// 
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-12  Sign Code
//                  13-44  Sign Text
//                  45-48  Record flags
//                  49-64  Reserved
//
//  Defined flags: None
//
#define SIGNCODES_CODE_LENGTH     8
#define SIGNCODES_TEXT_LENGTH     32
#define SIGNCODES_RESERVED_LENGTH 16

typedef struct SIGNCODESStruct
{
  long  recordID;
  char  code[SIGNCODES_CODE_LENGTH];
  char  text[SIGNCODES_TEXT_LENGTH];
  long  flags;
  char  reserved[SIGNCODES_RESERVED_LENGTH];
} SIGNCODESDef;

typedef struct SIGNCODESKey1Struct
{
  char code[SIGNCODES_CODE_LENGTH];
} SIGNCODESKey1Def;

EXTERN SIGNCODESDef     SIGNCODES;
EXTERN TMSKey0Def       SIGNCODESKey0;
EXTERN SIGNCODESKey1Def SIGNCODESKey1;

//
//  BLOCKNAMES Table Structure - See definition for NORMNAMES (above)
//  Record length: 32
//  Defined flags: None
//
#define BLOCKNAMES_NAME_LENGTH     NORMNAMES_NAME_LENGTH
#define BLOCKNAMES_RESERVED_LENGTH NORMNAMES_RESERVED_LENGTH
EXTERN NORMNAMESDef     BLOCKNAMES;
EXTERN TMSKey0Def       BLOCKNAMESKey0;
EXTERN NORMNAMESKey1Def BLOCKNAMESKey1;

//
//  TRIPS (OLD) Table Structure
//  Record length: 96
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Routes Table Record ID Pointer
//                  13-16  Services Table Record ID Pointer
//                  17-20  Direction Index (0/1, as per Routes Table)
//                  21-24  Trip Number
//                  25-28  Bus Types Table Record ID Pointer
//                  29-32  Pattern Names Table Record ID Pointer
//                  33-36  Time at the Maximum Load Point
//                  37-40  Trip Sequence Number
//                  41-44  Minimum Layover Permitted for this trip
//                  45-48  Maximum Layover Permitted for this trip
//                  49-52  Maximum Shift Permitted in Negative Direction
//                  53-56  Maximum Shift Permitted in Positive Direction
//                  57-60  Actual Shift Performed
//                  61-64  Block Number
//                  65-68  Block Names Table Record ID Pointer
//                  69-72  Nodes Table Record ID Pointer to POG
//                  73-76  Nodes Table Record ID Pointer to PIG
//                  77-80  Routes Table Record ID Pointer to RGRP
//                  81-84  Services Table Record ID Pointer to SGRP
//                  85-88  Nodes Table Record ID Pointer to Garage Assignment
//                  89-92  Sign Codes Table Record ID Pointer
//                  93-96  Record Flags
//
typedef struct OLD_TRIPSStruct
{
  long recordID;
  long COMMENTSrecordID;
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long tripNumber;
  long BUSTYPESrecordID;
  long PATTERNNAMESrecordID;
  long timeAtMLP;
  long tripSequence;
  long layoverMin;
  long layoverMax;
  long negativeShiftMax;
  long positiveShiftMax;
  long actualShift;
  long blockNumber;
  long BLOCKNAMESrecordID;
  long POGNODESrecordID;
  long PIGNODESrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long assignedToNODESrecordID;
  long SIGNCODESrecordID;
  long flags;
} OLD_TRIPSDef;

typedef struct OLD_TRIPSKey1Struct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long tripSequence;
} OLD_TRIPSKey1Def;

typedef struct OLD_TRIPSKey2Struct
{
  long assignedToNODESrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long blockNumber;
  long timeAtMLP;
} OLD_TRIPSKey2Def;

EXTERN OLD_TRIPSDef     OLD_TRIPS;
EXTERN TMSKey0Def       OLD_TRIPSKey0;
EXTERN OLD_TRIPSKey1Def OLD_TRIPSKey1;
EXTERN OLD_TRIPSKey2Def OLD_TRIPSKey2;

//
//  TRIPS Table Structure
//  Record length: 96
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Sign Codes Table Record ID Pointer
//                  13-16  Routes Table Record ID Pointer
//                  17-20  Services Table Record ID Pointer
//                  21-24  Bus Types Table Record ID Pointer
//                  25-28  Pattern Names Table Record ID Pointer
//                  29-32  Direction Index (0/1, as per Routes Table)
//                  33-36  Trip Number
//                  37-40  Time at the Maximum Load Point
//                  41-44  Trip Sequence Number
//                  45-48  Maximum Shift Permitted in Negative Direction
//                  49-52  Maximum Shift Permitted in Positive Direction
//                  53-56  Actual Shift Performed
//                  57-60  Standard blocking: Block Number
//                  61-64  Standard blocking: Block Names Table Record ID Pointer
//                  65-68  Standard blocking: Nodes Table Record ID Pointer to POG
//                  69-72  Standard blocking: Nodes Table Record ID Pointer to PIG
//                  73-76  Standard blocking: Routes Table Record ID Pointer to RGRP
//                  77-80  Standard blocking: Services Table Record ID Pointer to SGRP
//                  81-84  Standard blocking: Nodes Table Record ID Pointer to Garage Assignment
//                  85-88  Standard blocking: Minimum Layover Permitted for this trip
//                  89-92  Standard blocking: Maximum Layover Permitted for this trip
//                  93-96  Standard blocking: Block sequence (time at first timepoint)
//                  97-100 Dropback blocking: Block Number
//                 101-104 Dropback blocking: Block Names Table Record ID Pointer
//                 105-108 Dropback blocking: Nodes Table Record ID Pointer to POG
//                 109-112 Dropback blocking: Nodes Table Record ID Pointer to PIG
//                 113-116 Dropback blocking: Routes Table Record ID Pointer to RGRP
//                 117-120 Dropback blocking: Services Table Record ID Pointer to SGRP
//                 121-124 Dropback blocking: Nodes Table Record ID Pointer to Garage Assignment
//                 125-128 Dropback blocking: Minimum Layover Permitted for this trip
//                 129-132 Dropback blocking: Maximum Layover Permitted for this trip
//                 133-136 Dropback blocking: Block sequence (time at first timepoint)
//                 137-140 Customer Comments Table Record ID Pointer
//                 141-144 Seconday Sign Codes Table Record ID Pointer
//                 145-156 Reserved
//                 157-160 Record Flags
//
#define TRIPS_RESERVED_LENGTH 12
#define TRIPS_FLAG_SERVICEWRAP_PLUS  0x0002
#define TRIPS_FLAG_SERVICEWRAP_MINUS 0x0004

typedef struct BLOCKStruct
{
  long blockNumber;
  long BLOCKNAMESrecordID;
  long POGNODESrecordID;
  long PIGNODESrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long assignedToNODESrecordID;
  long layoverMin;
  long layoverMax;
  long blockSequence;
} BLOCKSDef;

typedef struct TRIPSHIFTStruct
{
  long negativeMax;
  long positiveMax;
  long actual;
} TRIPSHIFTDef;

typedef struct TRIPSStruct
{
  long recordID;
  long COMMENTSrecordID;
  long ROUTESrecordID;
  long SERVICESrecordID;
  long BUSTYPESrecordID;
  long PATTERNNAMESrecordID;
  long SIGNCODESrecordID;
  long directionIndex;
  long tripNumber;
  long timeAtMLP;
  long tripSequence;
  TRIPSHIFTDef shift;
  BLOCKSDef standard;
  BLOCKSDef dropback;
  long customerCOMMENTSrecordID;
  long secondarySIGNCODESrecordID;
  char reserved[TRIPS_RESERVED_LENGTH];
  long flags;
} TRIPSDef;

typedef struct TRIPSKey1Struct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long tripSequence;
} TRIPSKey1Def;

typedef struct TRIPSKey2Struct
{
  long assignedToNODESrecordID;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long blockNumber;
  long blockSequence;
} TRIPSKey2Def;

EXTERN TRIPSDef     TRIPS;
EXTERN TMSKey0Def   TRIPSKey0;
EXTERN TRIPSKey1Def TRIPSKey1;
EXTERN TRIPSKey2Def TRIPSKey2;
//
//  Note: TRIPSKey3 is typically not used.  Both keys 2 and 3 have the same
//        inherent structure, but are assigned differently.  Key 2 is used
//        for standard blocks, and key 3 is used for dropback blocks.
//
EXTERN TRIPSKey2Def TRIPSKey3;

//
//  RUNS Table Structure
//  Record length: 64
//  Defined flags: (0) 0x0001 - Extraboard duty at prior to piece
//                 (1) 0x0002 - Extraboard duty at after piece
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Services Table Record ID Pointer
//                  13-16  Divisions Table Record ID Pointer
//                  17-20  Run Number
//                  21-24  Piece Number
//                  25-28  "Cut as Runtype" - The type of run this run was cut as
//                  29-32  "Start of Piece" Trips Table Record ID Pointer
//                  33-36  "Start of Piece" Nodes Table Record ID Pointer
//                  37-40  "End of Piece" Trips Table Record ID Pointer
//                  41-44  "End of Piece" Nodes Table Record ID Pointer
//                  45-48  Extraboard start time before piece commences
//                  49-52  Extraboard end time before piece commences
//                  53-56  Extraboard start time after piece ends
//                  57-60  Extraboard end time after piece ends 
//                  61-64  Record Flags
//

typedef struct STARTENDStruct
{
  long TRIPSrecordID;
  long NODESrecordID;
} STARTENDDef;

typedef struct EXTRABOARDStruct
{
  long startTime;
  long endTime;
} EXTRABOARDDef;

#define RUNS_RESERVED_LENGTH 0
#define RUNS_FLAG_EXTRABOARDPRIOR 0x0001
#define RUNS_FLAG_EXTRABOARDAFTER 0x0002
typedef struct RUNSStruct
{
  long recordID;
  long COMMENTSrecordID;
  long SERVICESrecordID;
  long DIVISIONSrecordID;
  long runNumber;
  long pieceNumber;
  long cutAsRuntype;
  STARTENDDef start;
  STARTENDDef end;
  EXTRABOARDDef prior;
  EXTRABOARDDef after;
  long flags;
} RUNSDef;

typedef struct RUNSKey1Struct
{
  long DIVISIONSrecordID;
  long SERVICESrecordID;
  long runNumber;
  long pieceNumber;
} RUNSKey1Def;

typedef struct RUNSKey2Struct
{
  long cutAsRuntype;
} RUNSKey2Def;

typedef struct RUNSKey3Struct
{
  long DIVISIONSRecordID;
} RUNSKey3Def;

EXTERN RUNSDef     RUNS;
EXTERN TMSKey0Def  RUNSKey0;
EXTERN RUNSKey1Def RUNSKey1;
EXTERN RUNSKey2Def RUNSKey2;
EXTERN RUNSKey3Def RUNSKey3;

//
//  OLDROSTER Table Structure
//  Record length: 64
//  Defined flags: (0) 0x0001 Day 1 RUNS recordID is really from CREWONLY
//                 (1) 0x0002 Day 2 RUNS recordID is really from CREWONLY
//                 (2) 0x0004 Day 3 RUNS recordID is really from CREWONLY
//                 (3) 0x0008 Day 4 RUNS recordID is really from CREWONLY
//                 (4) 0x0010 Day 5 RUNS recordID is really from CREWONLY
//                 (5) 0x0020 Day 6 RUNS recordID is really from CREWONLY
//                 (6) 0x0040 Day 7 RUNS recordID is really from CREWONLY
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Divisions Table Record ID Pointer
//                  13-16  Drivers Table Record ID Pointer
//                  17-20  Roster Number
//                  21-48  Runs Table Record ID Pointers (each day of the week)
//                  49-60  Reserved
//                  61-64  Record Flags
//
#define OLDROSTER_MAX_DAYS         7
#define OLDROSTER_RESERVED_LENGTH 12
#define OLDROSTER_FLAG_DAY1_CREWONLY 0x0001
#define OLDROSTER_FLAG_DAY2_CREWONLY 0x0002
#define OLDROSTER_FLAG_DAY3_CREWONLY 0x0004
#define OLDROSTER_FLAG_DAY4_CREWONLY 0x0008
#define OLDROSTER_FLAG_DAY5_CREWONLY 0x0010
#define OLDROSTER_FLAG_DAY6_CREWONLY 0x0020
#define OLDROSTER_FLAG_DAY7_CREWONLY 0x0040

typedef struct OLDROSTERStruct
{
  long recordID;
  long COMMENTSrecordID;
  long DIVISIONSrecordID;
  long DRIVERSrecordID;
  long rosterNumber;
  long RUNSrecordIDs[OLDROSTER_MAX_DAYS];
  char reserved[OLDROSTER_RESERVED_LENGTH];
  long flags;
} OLDROSTERDef;

typedef struct OLDROSTERKey1Struct
{
  long DIVISIONSrecordID;
  long rosterNumber;
} OLDROSTERKey1Def;

typedef struct OLDROSTERKey2Struct
{
  long DRIVERSrecordID;
} OLDROSTERKey2Def;

EXTERN OLDROSTERDef     OLDROSTER;
EXTERN TMSKey0Def       OLDROSTERKey0;
EXTERN OLDROSTERKey1Def OLDROSTERKey1;
EXTERN OLDROSTERKey2Def OLDROSTERKey2;

//
//  ROSTER Table Structure
//  Record length: 512
//  Defined flags: (0) 0x0001 Day 1 RUNS recordID is really from CREWONLY
//                 (1) 0x0002 Day 2 RUNS recordID is really from CREWONLY
//                 (2) 0x0004 Day 3 RUNS recordID is really from CREWONLY
//                 (3) 0x0008 Day 4 RUNS recordID is really from CREWONLY
//                 (4) 0x0010 Day 5 RUNS recordID is really from CREWONLY
//                 (5) 0x0020 Day 6 RUNS recordID is really from CREWONLY
//                 (6) 0x0040 Day 7 RUNS recordID is really from CREWONLY
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Divisions Table Record ID Pointer
//                  13-16  Drivers Table Record ID Pointer
//                  17-20  Roster Number
//                  21-48  Week 0 Runs Table Record ID Pointers (each day of the week)
//                  49-52  Week 0 Flags
//                  53-80  Week 1 Runs Table Record ID Pointers (each day of the week)
//                  81-84  Week 1 Flags
//                  85-112 Week 2 Runs Table Record ID Pointers (each day of the week)
//                 113-116 Week 2 Flags
//                 117-144 Week 3 Runs Table Record ID Pointers (each day of the week)
//                 145-148 Week 3 Flags
//                 149-176 Week 4 Runs Table Record ID Pointers (each day of the week)
//                 177-180 Week 4 Flags
//                 181-208 Week 5 Runs Table Record ID Pointers (each day of the week)
//                 209-212 Week 5 Flags
//                 213-240 Week 6 Runs Table Record ID Pointers (each day of the week)
//                 241-244 Week 6 Flags
//                 245-272 Week 7 Runs Table Record ID Pointers (each day of the week)
//                 273-276 Week 7 Flags
//                 277-304 Week 8 Runs Table Record ID Pointers (each day of the week)
//                 305-308 Week 8 Flags
//                 309-336 Week 9 Runs Table Record ID Pointers (each day of the week)
//                 337-340 Week 9 Flags
//                 341-508 Reserved
//                 509-512 Record Flags
//
#define ROSTER_MAX_DAYS          7
#define ROSTER_MAX_WEEKS        10
#define ROSTER_RESERVED_LENGTH 168
#define ROSTER_FLAG_DAY1_CREWONLY 0x0001
#define ROSTER_FLAG_DAY2_CREWONLY 0x0002
#define ROSTER_FLAG_DAY3_CREWONLY 0x0004
#define ROSTER_FLAG_DAY4_CREWONLY 0x0008
#define ROSTER_FLAG_DAY5_CREWONLY 0x0010
#define ROSTER_FLAG_DAY6_CREWONLY 0x0020
#define ROSTER_FLAG_DAY7_CREWONLY 0x0040

#define ROSTER_FLAG_WEEK0ACTIVE   0x0001
#define ROSTER_FLAG_WEEK1ACTIVE   0x0002
#define ROSTER_FLAG_WEEK2ACTIVE   0x0004
#define ROSTER_FLAG_WEEK3ACTIVE   0x0008
#define ROSTER_FLAG_WEEK4ACTIVE   0x0010
#define ROSTER_FLAG_WEEK5ACTIVE   0x0020
#define ROSTER_FLAG_WEEK6ACTIVE   0x0040
#define ROSTER_FLAG_WEEK7ACTIVE   0x0080
#define ROSTER_FLAG_WEEK8ACTIVE   0x0100
#define ROSTER_FLAG_WEEK9ACTIVE   0x0200

typedef struct ROSTERWEEKStruct
{
  long RUNSrecordIDs[ROSTER_MAX_DAYS];
  long flags;
} ROSTERWEEKDef;

typedef struct ROSTERStruct
{
  long recordID;
  long COMMENTSrecordID;
  long DIVISIONSrecordID;
  long DRIVERSrecordID;
  long rosterNumber;
  ROSTERWEEKDef WEEK[ROSTER_MAX_WEEKS];
  char reserved[ROSTER_RESERVED_LENGTH];
  long flags;
} ROSTERDef;

typedef struct ROSTERKey1Struct
{
  long DIVISIONSrecordID;
  long rosterNumber;
} ROSTERKey1Def;

typedef struct ROSTERKey2Struct
{
  long DRIVERSrecordID;
} ROSTERKey2Def;

EXTERN ROSTERDef     ROSTER;
EXTERN TMSKey0Def    ROSTERKey0;
EXTERN ROSTERKey1Def ROSTERKey1;
EXTERN ROSTERKey2Def ROSTERKey2;

//
//  OLDDRIVERS Table Structure
//  Record length: 224
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Divisions Table Record ID Pointer
//                  13-16  Driver Types Table Record ID Pointer
//                  17-48  Last Name
//                  49-80  First Name
//                  81-84  Initials
//                  85-100 Driver Badge Number
//                 101-104 Crew Number
//                 105-136 Street Address
//                 137-168 City
//                 169-170 Province/State
//                 171-180 Postal/Zip Code
//                 181-184 Area Code
//                 185-188 Telephone Number
//                 189-192 Hire Date
//                 193-196 Seniority Date
//                 197-198 Seniority Sort Number (for drivers whose Seniority Date is the same)
//                 199-200 Vacation Entitlement
//                 201-216 Driver SIN/SS Number
//                 217-220 Reserved
//                 221-224 Record Flags
//
#define OLDDRIVERS_LASTNAME_LENGTH      32
#define OLDDRIVERS_FIRSTNAME_LENGTH     32
#define OLDDRIVERS_INITIALS_LENGTH       4
#define OLDDRIVERS_BADGENUMBER_LENGTH   16
#define OLDDRIVERS_STREETADDRESS_LENGTH 32
#define OLDDRIVERS_CITY_LENGTH          32
#define OLDDRIVERS_PROVINCE_LENGTH       2
#define OLDDRIVERS_POSTALCODE_LENGTH    10
#define OLDDRIVERS_SIN_LENGTH           16
#define OLDDRIVERS_RESERVED_LENGTH       4
typedef struct OLDDRIVERStruct
{
  long recordID;
  long COMMENTSrecordID;
  long DIVISIONSrecordID;
  long DRIVERTYPESrecordID;
  char lastName[OLDDRIVERS_LASTNAME_LENGTH];
  char firstName[OLDDRIVERS_FIRSTNAME_LENGTH];
  char initials[OLDDRIVERS_INITIALS_LENGTH];
  char badgeNumber[OLDDRIVERS_BADGENUMBER_LENGTH];
  long crewNumber;
  char streetAddress[OLDDRIVERS_STREETADDRESS_LENGTH];
  char city[OLDDRIVERS_CITY_LENGTH];
  char province[OLDDRIVERS_PROVINCE_LENGTH];
  char postalCode[OLDDRIVERS_POSTALCODE_LENGTH];
  long telephoneArea;
  long telephoneNumber;
  long hireDate;
  long seniorityDate;
  short int senioritySort;
  short int vacationEntitlement;
  char SIN[OLDDRIVERS_SIN_LENGTH];
  char reserved[OLDDRIVERS_RESERVED_LENGTH];
  long flags;
} OLDDRIVERSDef;

typedef struct OLDDRIVERSKey1Struct
{
  char lastName[OLDDRIVERS_LASTNAME_LENGTH];
  char firstName[OLDDRIVERS_FIRSTNAME_LENGTH];
  long seniorityDate;
  short int senioritySort;
} OLDDRIVERSKey1Def;

typedef struct OLDDRIVERSKey2Struct
{
  long seniorityDate;
  short int senioritySort;
} OLDDRIVERSKey2Def;

EXTERN OLDDRIVERSDef     OLDDRIVERS;
EXTERN TMSKey0Def        OLDDRIVERSKey0;
EXTERN OLDDRIVERSKey1Def OLDDRIVERSKey1;
EXTERN OLDDRIVERSKey2Def OLDDRIVERSKey2;


//
//  DRIVERS Table Structure
//  Record length: 224
//  Defined flags: (0) 0x0001 - Safety Sensitive
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Divisions Table Record ID Pointer
//                  13-16  Driver Types Table Record ID Pointer
//                  17-48  Last Name
//                  49-80  First Name
//                  81-84  Initials
//                  85-100 Driver Badge Number
//                 101-104 Crew Number
//                 105-136 Street Address
//                 137-168 City
//                 169-170 Province/State
//                 171-180 Postal/Zip Code
//                 181-184 Area Code (Voice)
//                 185-188 Telephone Number
//                 189-192 Area Code (Fax/Cell)
//                 193-196 Fax/Cell Number
//                 197-228 Email Address
//                 229-232 Hire Date
//                 233-236 Seniority Date
//                 237-240 Seniority Sort Number (for drivers whose Seniority Date is the same)
//                 241-244 Vacation Days
//                 245-248 Personal Days
//                 249-252 Sick Days
//                 253-268 Driver SIN/SS Number
//                 269-271 Bitmap File Extension
//                 272-431 Payroll Codes (20 @ 8 chars)
//                 432-448 Unused
//                 449-452 Area Code (Emergency)
//                 453-456 Emergency phone number
//                 457-520 Emergency contact
//                 521-524 Physical expiry date
//                 525-556 License Number
//                 557-560 License Expiry Date
//                 561-562 Province/State of Issue
//                 562-578 License Type
//                 579-582 Date of Birth
//                 583-586 Transit Hire Date
//                 587-590 Full-time Date
//                 591-594 Promotion Date
//                 595-598 Termination Date
//                 599-599 Sex  (1 byte numeric - position in dropdown)
//                 600-600 Race (1 byte numeric - position in dropdown)
//                 601-601 EEOC (1 byte numeric - position in dropdown)
//                 602-602 Labour Union (1 byte numeric - position in dropdown)
//                 603-603 Section (1 byte numeric - position in dropdown)
//                 604-604 Position (1 byte numeric - position in dropdown)
//                 605-608 Float Time
//                 609-612 Pending Vacation
//                 613-636 Reserved
//                 637-640 Record Flags
//
#define DRIVERS_FLAG_SAFETYSENSITIVE     0x0001
#define DRIVERS_FLAG_SUPERVISOR          0x0002
#define DRIVERS_FLAG_VACATIONTIMEINHOURS 0x0004
#define DRIVERS_FLAG_PERSONALTIMEINHOURS 0x0008
#define DRIVERS_FLAG_SICKTIMEINHOURS     0x0010
#define DRIVERS_FLAG_FLOATTIMEINHOURS    0x0020
#define DRIVERS_FLAG_DAYTIMEEXTRABOARD   0x0040
#define DRIVERS_FLAG_NIGHTTIMEEXTRABOARD 0x0080

#define DRIVERS_LASTNAME_LENGTH         32
#define DRIVERS_FIRSTNAME_LENGTH        32
#define DRIVERS_INITIALS_LENGTH          4
#define DRIVERS_BADGENUMBER_LENGTH      16
#define DRIVERS_STREETADDRESS_LENGTH    32
#define DRIVERS_CITY_LENGTH             32
#define DRIVERS_PROVINCE_LENGTH          2
#define DRIVERS_POSTALCODE_LENGTH       10
#define DRIVERS_EMAILADDRESS_LENGTH     32
#define DRIVERS_SIN_LENGTH              16
#define DRIVERS_LICENSENUMBER_LENGTH    32
#define DRIVERS_LICENSETYPE_LENGTH      16
#define DRIVERS_RESERVED1_LENGTH       177
#define DRIVERS_RESERVED2_LENGTH        24
#define DRIVERS_BITMAPFILEEXT_LENGTH     3
#define DRIVERS_EMERGENCYCONTACT_LENGTH 64
typedef struct DRIVERStruct
{
  long recordID;                                         //   4
  long COMMENTSrecordID;                                 //+  4 =   8                          
  long DIVISIONSrecordID;                                //+  4 =  12
  long DRIVERTYPESrecordID;                              //+  4 =  16
  char lastName[DRIVERS_LASTNAME_LENGTH];                //+ 32 =  48
  char firstName[DRIVERS_FIRSTNAME_LENGTH];              //+ 32 =  80 
  char initials[DRIVERS_INITIALS_LENGTH];                //+  4 =  84
  char badgeNumber[DRIVERS_BADGENUMBER_LENGTH];          //+ 16 = 100
  long crewNumber;                                       //+  4 = 104
  char streetAddress[DRIVERS_STREETADDRESS_LENGTH];      //+ 32 = 136
  char city[DRIVERS_CITY_LENGTH];                        //+ 32 = 168
  char province[DRIVERS_PROVINCE_LENGTH];                //+  2 = 170
  char postalCode[DRIVERS_POSTALCODE_LENGTH];            //+ 10 = 180
  long telephoneArea;                                    //+  4 = 184
  long telephoneNumber;                                  //+  4 = 188
  long faxArea;                                          //+  4 = 192
  long faxNumber;                                        //+  4 = 196
  char emailAddress[DRIVERS_EMAILADDRESS_LENGTH];        //+ 32 = 228
  long hireDate;                                         //+  4 = 232
  long seniorityDate;                                    //+  4 = 236
  long senioritySort;                                    //+  4 = 240
  long vacationTime;                                     //+  4 = 244
  long personalTime;                                     //+  4 = 248
  long sickTime;                                         //+  4 = 252
  char SIN[DRIVERS_SIN_LENGTH];                          //+ 16 = 268
  char bitmapFileExt[DRIVERS_BITMAPFILEEXT_LENGTH];      //+  3 = 271
  char reserved1[DRIVERS_RESERVED1_LENGTH];              //+177 = 448  
  long emergencyArea;                                    //+  4 = 452
  long emergencyNumber;                                  //+  4 = 456
  char emergencyContact[DRIVERS_EMERGENCYCONTACT_LENGTH];//+ 64 = 520
  long physicalExpiryDate;                               //+  4 = 524
  char licenseNumber[DRIVERS_LICENSENUMBER_LENGTH];      //+ 32 = 556
  long licenseExpiryDate;                                //+  4 = 560
  char licenseProvinceOfIssue[DRIVERS_PROVINCE_LENGTH];  //+  2 = 562
  char licenseType[DRIVERS_LICENSETYPE_LENGTH];          //+ 16 = 578
  long dateOfBirth;                                      //+  4 = 582
  long transitHireDate;                                  //+  4 = 586
  long fullTimeDate;                                     //+  4 = 590
  long promotionDate;                                    //+  4 = 594
  long terminationDate;                                  //+  4 = 598
  char sex;                                              //+  1 = 599
  char race;                                             //+  1 = 600
  char EEOC;                                             //+  1 = 601
  char labourUnion;                                      //+  1 = 602
  char section;                                          //+  1 = 603
  char position;                                         //+  1 = 604
  long floatTime;                                        //+  4 = 608
  long pendingVacation;                                  //+  4 = 612
  char reserved2[DRIVERS_RESERVED2_LENGTH];              //+ 24 = 636     
  long flags;                                            //+  4 = 640
} DRIVERSDef;

typedef struct DRIVERSKey1Struct
{
  char lastName[DRIVERS_LASTNAME_LENGTH];
  char firstName[DRIVERS_FIRSTNAME_LENGTH];
  long seniorityDate;
  short int senioritySort;
} DRIVERSKey1Def;

typedef struct DRIVERSKey2Struct
{
  long seniorityDate;
  short int senioritySort;
} DRIVERSKey2Def;

EXTERN DRIVERSDef     DRIVERS;
EXTERN TMSKey0Def     DRIVERSKey0;
EXTERN DRIVERSKey1Def DRIVERSKey1;
EXTERN DRIVERSKey2Def DRIVERSKey2;

//
//  DRIVERTYPES Table Structure 
//  Record length: 64
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Sort Number
//                  13-28  Driver Type
//                  29-60  Payroll codes (4 by 8 characters)
//                  61-64  Record Flags
//
#define DRIVERTYPES_FLAG_REGULAR       0x0001
#define DRIVERTYPES_FLAG_OVERTIME      0x0002
#define DRIVERTYPES_FLAG_SPREADPREMIUM 0x0004
#define DRIVERTYPES_FLAG_MAKEUPTIME    0x0008

#define DRIVERTYPES_PAYROLLCODE_REGULAR       0
#define DRIVERTYPES_PAYROLLCODE_OVERTIME      1
#define DRIVERTYPES_PAYROLLCODE_SPREADPREMIUM 2
#define DRIVERTYPES_PAYROLLCODE_MAKEUPTIME    3

#define DRIVERTYPES_NAME_LENGTH     16
#define DRIVERS_PAYROLLCODE_LENGTH   8
#define DRIVERS_NUMPAYROLLCODES      4

typedef struct DRIVERTYPESStruct
{
  long recordID;
  long COMMENTSrecordID;
  long number;
  char name[DRIVERTYPES_NAME_LENGTH];
  char payrollCodes[DRIVERS_NUMPAYROLLCODES][DRIVERS_PAYROLLCODE_LENGTH];
  long flags;
} DRIVERTYPESDef;

typedef struct DRIVERTYPESKey1Struct
{
  long number;
} DRIVERTYPESKey1Def;

EXTERN DRIVERTYPESDef     DRIVERTYPES;
EXTERN TMSKey0Def         DRIVERTYPESKey0;
EXTERN DRIVERTYPESKey1Def DRIVERTYPESKey1;

//
//  CUSTOMERS Table Structure
//  Record length: 256
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Salutation
//                  13-44  Last name
//                  45-76  First name
//                  77-80  Initials
//                  81-112 Street address
//                 113-144 City
//                 145-146 Province
//                 148-156 Postal Code
//                 158-160 Day telephone area code
//                 161-164 Day telephone number
//                 165-168 Night telephone area code
//                 169-172 Night telephone number
//                 173-176 Fax telephone area code
//                 177-180 Fax telephone number
//                 181-212 Email address
//                 213-252 Reserved
//                 253-256 Record Flags
//
#define CUSTOMERS_SALUTATION_LENGTH     4
#define CUSTOMERS_LASTNAME_LENGTH      32
#define CUSTOMERS_FIRSTNAME_LENGTH     32
#define CUSTOMERS_INITIALS_LENGTH       4
#define CUSTOMERS_STREETADDRESS_LENGTH 32
#define CUSTOMERS_CITY_LENGTH          32
#define CUSTOMERS_STATE_LENGTH          2
#define CUSTOMERS_ZIP_LENGTH           10
#define CUSTOMERS_EMAILADDRESS_LENGTH  32
#define CUSTOMERS_RESERVED_LENGTH      40
typedef struct CUSTOMERStruct
{
  long recordID;
  long COMMENTSrecordID;
  char salutation[CUSTOMERS_SALUTATION_LENGTH];
  char lastName[CUSTOMERS_LASTNAME_LENGTH];
  char firstName[CUSTOMERS_FIRSTNAME_LENGTH];
  char initials[CUSTOMERS_INITIALS_LENGTH];
  char streetAddress[CUSTOMERS_STREETADDRESS_LENGTH];
  char city[CUSTOMERS_CITY_LENGTH];
  char state[CUSTOMERS_STATE_LENGTH];
  char ZIP[CUSTOMERS_ZIP_LENGTH];
  long dayTelephoneArea;
  long dayTelephoneNumber;
  long nightTelephoneArea;
  long nightTelephoneNumber;
  long faxTelephoneArea;
  long faxTelephoneNumber;
  char emailAddress[CUSTOMERS_EMAILADDRESS_LENGTH];
  char reserved[CUSTOMERS_RESERVED_LENGTH];
  long flags;
} CUSTOMERSDef;

typedef struct CUSTOMERSKey1Struct
{
  char lastName[CUSTOMERS_LASTNAME_LENGTH];
  char firstName[CUSTOMERS_FIRSTNAME_LENGTH];
} CUSTOMERSKey1Def;

typedef struct CUSTOMERSKey2Struct
{
  char postalCode[CUSTOMERS_ZIP_LENGTH];
} CUSTOMERSKey2Def;

EXTERN CUSTOMERSDef     CUSTOMERS;
EXTERN TMSKey0Def       CUSTOMERSKey0;
EXTERN CUSTOMERSKey1Def CUSTOMERSKey1;
EXTERN CUSTOMERSKey2Def CUSTOMERSKey2;

//
//  COMPLAINTS Table Structure
//  Record length: Variable: Fixed portion = 256
//  Defined flags: (0) 0x0001 - Complaint
//                 (1) 0x0002 - Commendation
//                 (2) 0x0004 - Sevice request
//                 (3) 0x0008 - Information request
//                 (4) 0x0010 - Has filed previous complaints
//                 (5) 0x0020 - Not sure of how many previous complaints filed
//                 (6) 0x0040 - Not sure of driver's sex
//                 (7) 0x0080 - Driver was male
//                 (8) 0x0100 - N/A on driver speeding
//                 (9) 0x0200 - Driver speed appears in driverSpeed
//                (10) 0x0400 - N/A on driver attention
//                (11) 0x0800 - Attempted to get driver attention
//                (12) 0x1000 - Less than two minute customer time differential
//                (13) 0x2000 - Two to five minute difference
//                (14) 0x4000 - More than five minute difference
//                (15) 0x8000 - This record is marked "closed"
//
//  Record Layout:  Bytes   Description
//                  ~~~~~   ~~~~~~~~~~~
//                   1-4    Record ID
//                   5-8    Comments Table Record ID Pointer
//                   9-12   Customer Table Record ID Pointer
//                  13-24   Complaint number
//                  25-40   Original complaint taker
//                  41-44   Number of previous complaints (as claimed by customer)
//                  45-48   Date of complaint
//                  49-52   Time of complaint
//                  53-56   Date of occurrence
//                  57-60   Time of occurrence
//                  61-76   Bus number
//                  77-80   Routes Table Record ID Pointer
//                  81-84   Direction Index (0/1, as per Routes Table)
//                  85-148  Location
//                 149-152  Drivers Table Record ID Pointer
//                 153-184  Driver description
//                 185-188  Driver speed
//                 189-192  Weather index
//                 193-196  Typical complaint index
//                 197-200  Department index
//                 201-216  Assigned to
//                 217-220  Classification index
//                 221-224  Category index
//                 225-228  Supervisor entered Drivers Table Record ID Pointer
//                 229-232  Date presented to employee
//                 233-236  Action index
//                 237-240  Retain until date
//                 241-244  Date closed
//                 245-754  Conversation log
//                 755-1009 "Other" log
//                1010-1016 Reserved
//                1017-1020 Record flags
//                1021-1024 Supervisor flags
//                1025-31024 Comment Text (variable)
//
#define COMPLAINTS_FLAG_COMPLAINT            0x00000001
#define COMPLAINTS_FLAG_COMMENDATION         0x00000002
#define COMPLAINTS_FLAG_SERVICEREQUEST       0x00000004
#define COMPLAINTS_FLAG_INFORMATIONREQUEST   0x00000008
#define COMPLAINTS_FLAG_FILEDPREVIOUS        0x00000010 // Means check on NOTSUREHOWMANY
#define COMPLAINTS_FLAG_NOTSUREHOWMANY       0x00000020 // !NOTSURE means there's a number of prev
#define COMPLAINTS_FLAG_NOTSUREDRIVERSEX     0x00000040 // !NOTSURE means check on DRIVERMALE
#define COMPLAINTS_FLAG_DRIVERMALE           0x00000080 // !DRIVERMALE means female
#define COMPLAINTS_FLAG_NASPEEDING           0x00000100 // !NA means check on DRIVERSPEEDINGYES  
#define COMPLAINTS_FLAG_DRIVERSPEEDINGYES    0x00000200 // Means there's a number in driverSpeed
#define COMPLAINTS_FLAG_NAATTENTION          0x00000400 // !NA means check on ATTENTIONYES
#define COMPLAINTS_FLAG_ATTENTIONYES         0x00000800 
#define COMPLAINTS_FLAG_LESSTHANTWO          0x00001000
#define COMPLAINTS_FLAG_TWOTOFIVE            0x00002000
#define COMPLAINTS_FLAG_MORETHANFIVE         0x00004000 // Not any of these three means NA 
#define COMPLAINTS_FLAG_CLOSED               0x00008000
#define COMPLAINTS_FLAG_LETTERSENT           0x00010000
#define COMPLAINTS_FLAG_CANCELLED            0x00020000
#define COMPLAINTS_FLAG_FIXEDFUNKYTIME       0x00040000
#define COMPLAINTS_SUPFLAG_DAMAGE            0x00000001
#define COMPLAINTS_SUPFLAG_VIOLATION         0x00000002
#define COMPLAINTS_SUPFLAG_RECEIVEDSIMILAR   0x00000004
#define COMPLAINTS_SUPFLAG_TELEPHONE         0x00000008
#define COMPLAINTS_SUPFLAG_LETTER            0x00000010
#define COMPLAINTS_SUPFLAG_EMAIL             0x00000020
#define COMPLAINTS_SUPFLAG_NOVIOLATION       0x00000040
#define COMPLAINTS_SUPFLAG_INFORMATIONONLY   0x00000080
#define COMPLAINTS_SUPFLAG_INSUFFICIENT      0x00000100
#define COMPLAINTS_SUPFLAG_COMMENDATION      0x00000200
#define COMPLAINTS_SUPFLAG_OTHER             0x00000400
#define COMPLAINTS_SUPFLAG_OPERATIONS        0x00000800
#define COMPLAINTS_SUPFLAG_DEVELOPMENT       0x00001000
#define COMPLAINTS_SUPFLAG_MAINTENANCE       0x00002000
#define COMPLAINTS_SUPFLAG_SPECIALSERVICES   0x00004000
#define COMPLAINTS_SUPFLAG_FINANCE           0x00008000
#define COMPLAINTS_SUPFLAG_COMMRELATIONS     0x00010000
#define COMPLAINTS_REFERENCENUMBER_LENGTH    12      // AyyyyAddA001
#define COMPLAINTS_ENTEREDBY_LENGTH          16 
#define COMPLAINTS_BUSNUMBER_LENGTH          BUSTYPES_NUMBER_LENGTH
#define COMPLAINTS_LOCATION_LENGTH           64
#define COMPLAINTS_DRIVERDESCRIPTION_LENGTH  32
#define COMPLAINTS_ASSIGNEDTO_LENGTH         16
#define COMPLAINTS_CONVERSATION_LENGTH      510
#define COMPLAINTS_OTHER_LENGTH             255 
#define COMPLAINTS_BITMAPPATH_LENGTH        255
#define COMPLAINTS_RESERVED_LENGTH            8
#define COMPLAINTS_FIXED_LENGTH            1280
#define COMPLAINTS_VARIABLE_LENGTH         30000
#define COMPLAINTS_TOTAL_LENGTH COMPLAINTS_FIXED_LENGTH + COMPLAINTS_VARIABLE_LENGTH
typedef struct COMPLAINTSStruct
{
  long recordID;                                              //   4
  long COMMENTSrecordID;                                      //  +4 =   8
  long CUSTOMERSrecordID;                                     //  +4 =  12
  char referenceNumber[COMPLAINTS_REFERENCENUMBER_LENGTH];    // +12 =  24 
  char enteredBy[COMPLAINTS_ENTEREDBY_LENGTH];                // +16 =  40 
  long numberOfPreviousComplaints;                            //  +4 =  44 
  long dateOfComplaint;  // yyyymmdd                          //  +4 =  48
  long timeOfComplaint;  // hhmmss                            //  +4 =  52
  long dateOfOccurrence; // yyyymmdd                          //  +4 =  56
  long timeOfOccurrence; // hhmmss                            //  +4 =  60
  char busNumber[COMPLAINTS_BUSNUMBER_LENGTH];                // +16 =  76
  long ROUTESrecordID;                                        //  +4 =  80 
  long directionIndex;                                        //  +4 =  84
  char location[COMPLAINTS_LOCATION_LENGTH];                  // +64 = 148
  long DRIVERSrecordID;                                       //  +4 = 152
  char driverDescription[COMPLAINTS_DRIVERDESCRIPTION_LENGTH];// +32 = 184
  long driverSpeed;                                           //  +4 = 188
  long weatherIndex;                                          //  +4 = 192
  long typicalIndex;                                          //  +4 = 196
  long departmentIndex;                                       //  +4 = 200
  char assignedTo[COMPLAINTS_ASSIGNEDTO_LENGTH];              // +16 = 216
  long classificationIndex;                                   //  +4 = 220
  long categoryIndex;                                         //  +4 = 224
  long supDRIVERSrecordID;                                    //  +4 = 228
  long datePresented;  // yyyymmdd                            //  +4 = 232
  long actionIndex;                                           //  +4 = 236
  long retainUntil;    // yyyymmdd                            //  +4 = 240
  long dateClosed;     // yyyymmdd                            //  +4 = 244
  char conversation[COMPLAINTS_CONVERSATION_LENGTH];          //+510 = 754
  char other[COMPLAINTS_OTHER_LENGTH];                        //+255 =1009
  char bitmapPath[COMPLAINTS_BITMAPPATH_LENGTH];              //+255 =1264
  char reserved[COMPLAINTS_RESERVED_LENGTH];                  //  +8 =1272
  long flags;                                                 //  +4 =1276
  long supFlags;                                              //  +4 =1280
} COMPLAINTSDef;

typedef struct COMPLAINTSKey1Struct
{
  long CUSTOMERSrecordID;
} COMPLAINTSKey1Def;

typedef struct COMPLAINTSKey2Struct
{
  char referenceNumber[COMPLAINTS_REFERENCENUMBER_LENGTH];
} COMPLAINTSKey2Def;

typedef struct COMPLAINTSKey3Struct
{
  char enteredBy[COMPLAINTS_ENTEREDBY_LENGTH];
} COMPLAINTSKey3Def;

typedef struct COMPLAINTSKey4Struct
{
  long dateOfComplaint;
} COMPLAINTSKey4Def;

typedef struct COMPLAINTSKey5Struct
{
  long dateOfOccurrence;
} COMPLAINTSKey5Def;

typedef struct COMPLAINTSKey6Struct
{
  long ROUTESrecordID;
} COMPLAINTSKey6Def;

typedef struct COMPLAINTSKey7Struct
{
  long supDRIVERSrecordID;
} COMPLAINTSKey7Def;

EXTERN COMPLAINTSDef     COMPLAINTS;
EXTERN TMSKey0Def        COMPLAINTSKey0;
EXTERN COMPLAINTSKey1Def COMPLAINTSKey1;
EXTERN COMPLAINTSKey2Def COMPLAINTSKey2;
EXTERN COMPLAINTSKey3Def COMPLAINTSKey3;
EXTERN COMPLAINTSKey4Def COMPLAINTSKey4;
EXTERN COMPLAINTSKey5Def COMPLAINTSKey5;
EXTERN COMPLAINTSKey6Def COMPLAINTSKey6;
EXTERN COMPLAINTSKey7Def COMPLAINTSKey7;
//
//  TIMECHECKS Data Structure
//  Record length: 128
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Date of Time Check
//                  13-16  Services Table Record ID Pointer
//                  17-20  Routes Table Record ID Pointer
//                  21-24  Direction Index (0/1, as per Routes Table)
//                  25-28  Trips Table Record ID Pointer
//                  29-32  Nodes Table Record ID Pointer
//                  33-36  Actual Time
//                  37-40  Actual Bustype Used (Bustypes Table Record ID Pointer)
//                  41-44  Actual Driver (Drivers Table Record ID Pointer)
//                  45-48  Supervisor Index
//                  49-52  Passenger Load Index
//                  53-56  Weather Index
//                  57-60  Other Comments Index
//                  61-64  Node position in trip
//                  65-124 Reserved
//                 125-128 Flags
//
#define TIMECHECKS_RESERVED_LENGTH 60
typedef struct TIMECHECKSStruct
{
  long recordID;                                              //   4
  long COMMENTSrecordID;                                      //  +4 =  8
  long checkDate;                                             //  +4 = 12
  long SERVICESrecordID;                                      //  +4 = 16 
  long ROUTESrecordID;                                        //  +4 = 20 
  long directionIndex;                                        //  +4 = 24 
  long TRIPSrecordID;                                         //  +4 = 28 
  long NODESrecordID;                                         //  +4 = 32
  long actualTime;                                            //  +4 = 36
  long actualBUSTYPESrecordID;                                //  +4 = 40
  long actualDRIVERSrecordID;                                 //  +4 = 44
  long supervisorIndex;                                       //  +4 = 48
  long passengerLoadIndex;                                    //  +4 = 52
  long weatherIndex;                                          //  +4 = 56
  long otherCommentsIndex;                                    //  +4 = 60
  long nodePositionInTrip;                                    //  +4 = 64
  char reserved[TIMECHECKS_RESERVED_LENGTH];                  //  +60=124
  long flags;                                                 //  +4 =128
} TIMECHECKSDef;

typedef struct TIMECHECKSKey1Struct
{
  long checkDate;
} TIMECHECKSKey1Def;

typedef struct TIMECHECKSKey2Struct
{
  long SERVICESrecordID;
} TIMECHECKSKey2Def;

typedef struct TIMECHECKSKey3Struct
{
  long ROUTESrecordID;
  long directionIndex;
} TIMECHECKSKey3Def;

typedef struct TIMECHECKSKey4Struct
{
  long NODESrecordID;
} TIMECHECKSKey4Def;

typedef struct TIMECHECKSKey5Struct
{
  long TRIPSrecordID;
} TIMECHECKSKey5Def;

typedef struct TIMECHECKSKey6Struct
{
  long actualBUSTYPESrecordID;
} TIMECHECKSKey6Def;

typedef struct TIMECHECKSKey7Struct
{
  long actualDRIVERSrecordID;
} TIMECHECKSKey7Def;

typedef struct TIMECHECKSKey8Struct
{
  long supervisorIndex;
} TIMECHECKSKey8Def;

typedef struct TIMECHECKSKey9Struct
{
  long passengerLoadIndex;
} TIMECHECKSKey9Def;


EXTERN TIMECHECKSDef     TIMECHECKS;
EXTERN TMSKey0Def        TIMECHECKSKey0;
EXTERN TIMECHECKSKey1Def TIMECHECKSKey1;
EXTERN TIMECHECKSKey2Def TIMECHECKSKey2;
EXTERN TIMECHECKSKey3Def TIMECHECKSKey3;
EXTERN TIMECHECKSKey4Def TIMECHECKSKey4;
EXTERN TIMECHECKSKey5Def TIMECHECKSKey5;
EXTERN TIMECHECKSKey6Def TIMECHECKSKey6;
EXTERN TIMECHECKSKey7Def TIMECHECKSKey7;
EXTERN TIMECHECKSKey8Def TIMECHECKSKey8;
EXTERN TIMECHECKSKey9Def TIMECHECKSKey9;

//
//  CREWONLY Table Structure
//  Record length: 64
//  Defined flags: None.
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Services Table Record ID Pointer
//                  13-16  Divisions Table Record ID Pointer
//                  17-20  Run Number
//                  21-24  Piece Number
//                  25-28  "Cut as Runtype" - The type of run this run was cut as
//                  29-32  Start location NODES recordID
//                  33-36  Start time
//                  37-40  End location NODES recordID    
//                  41-44  End time
//                  45-60  Reserved
//                  61-64  Record Flags
//

#define CREWONLY_RESERVED_LENGTH   16
#define CREWONLY_FLAG_PLACEHOLDER  0x0001

typedef struct CREWONLYStruct
{
  long recordID;
  long COMMENTSrecordID;
  long SERVICESrecordID;
  long DIVISIONSrecordID;
  long runNumber;
  long pieceNumber;
  long cutAsRuntype;
  long startNODESrecordID;
  long startTime;
  long endNODESrecordID;
  long endTime;
  char reserved[CREWONLY_RESERVED_LENGTH];
  long flags;
} CREWONLYDef;

typedef struct CREWONLYKey1Struct
{
  long DIVISIONSrecordID;
  long SERVICESrecordID;
  long runNumber;
  long pieceNumber;
} CREWONLYKey1Def;

typedef struct CREWONLYKey2Struct
{
  long cutAsRuntype;
} CREWONLYKey2Def;

typedef struct CREWONLYKey3Struct
{
  long DIVISIONSRecordID;
} CREWONLYKey3Def;

EXTERN CREWONLYDef     CREWONLY;
EXTERN TMSKey0Def  CREWONLYKey0;
EXTERN CREWONLYKey1Def CREWONLYKey1;
EXTERN CREWONLYKey2Def CREWONLYKey2;
EXTERN CREWONLYKey3Def CREWONLYKey3;
//
//  OLDDAILYOPS Table Structure
//  Record length: 64
//  Defined flags: Multiple - see below
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Date of Entry
//                   9-12  Time of Entry
//                  13-16  Flags (old style)
//                  13-14  Flags (new style)
//                  15-16  User ID (new style)
//                  17-64  Associated Data
//

#define OLDDAILYOPS_FLAG_ROSTER                 0x00000001
#define OLDDAILYOPS_FLAG_BUS                    0x00000002
#define OLDDAILYOPS_FLAG_OPERATOR               0x00000004
#define OLDDAILYOPS_FLAG_ABSENCE                0x00000010
#define OLDDAILYOPS_FLAG_OPENWORK               0x00000020
#define OLDDAILYOPS_FLAG_SERVICE                0x00000040
#define OLDDAILYOPS_ASSOCIATEDDATA_LENGTH 48

//
//  Flag OLDDAILYOPS_FLAG_ROSTER
//
typedef struct OLDDOPSRosterStruct
{
  long flags;
  long DRIVERSrecordID;
  long ROSTERrecordID;
} OLDDOPSRosterDef;

//
//  Flag OLDDAILYOPS_FLAG_BUS
//
typedef struct OLDDOPSBusStruct
{
  long flags;
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long BUSESrecordID;
  long TRIPSrecordID;
  long DAILYOPSrecordID;
  long newBUSESrecordID;
  long blockNumber;
  long swapReasonIndex;
  long date;
} OLDDOPSBusDef;

//
//  Flag OLDDAILYOPS_FLAG_OPERATOR
//
typedef struct OLDDOPSOperatorStruct
{
  long flags;
  long DRIVERSrecordID;
  long newDate;
  long newTime;
  long DAILYOPSrecordID;
  long ROUTESrecordID;
  long SERVICESrecordID;
  long PATTERNNAMESrecordID;
  long directionIndex;
  long timeAtMLP;
  long deadheadTime;
} OLDDOPSOperatorDef;

//
//  Flag OLDDAILYOPS_FLAG_ABSENCE
//
typedef struct OLDDOPSAbsenceStruct
{
  long  flags;
  long  DRIVERSrecordID;
  long  fromDate;
  long  fromTime;
  long  toDate;
  long  toTime;
  long  reasonIndex;
  long  DAILYOPSrecordID;
  float timeLost;
  float paidTime[2];
  float unpaidTime;
} OLDDOPSAbsenceDef;

//
//  Flag OLDDAILYOPS_FLAG_OPENWORK
//
typedef struct OLDDOPSOpenWorkStruct
{
  long flags;
  long date;
  long time;
  long RUNSrecordID;
  long DRIVERSrecordID;
  long DAILYOPSrecordID;
} OLDDOPSOpenWorkDef;
  
//
//  Flag OLDDAILYOPS_FLAG_SERVICE
//
typedef struct OLDDOPSServiceStruct
{
  long flags;
  long date;
  long SERVICESrecordID;
} OLDDOPSServiceDef;

//
//  Union of OLDDOPS Structures
//
typedef union OLDDOPSUnion
{
  OLDDOPSRosterDef   RosterTemplate;
  OLDDOPSBusDef      Bus;
  OLDDOPSOperatorDef Operator;
  OLDDOPSAbsenceDef  Absence;
  OLDDOPSOpenWorkDef OpenWork;
  OLDDOPSServiceDef  Service;
  char               associatedData[OLDDAILYOPS_ASSOCIATEDDATA_LENGTH];
} OLDDOPSDef;

typedef struct OLDDAILYOPSStruct
{
  long recordID;
  long dateOfEntry;
  long timeOfEntry;
  unsigned short int flags;
  unsigned short int userID;
  OLDDOPSDef OLDDOPS;
} OLDDAILYOPSDef;

typedef struct OLDDAILYOPSKey1Struct
{
  long dateOfEntry;
  long timeOfEntry;
} OLDDAILYOPSKey1Def;

typedef struct OLDDAILYOPSKey2Struct
{
  unsigned short int flags;
  long dateOfEntry;
  long timeOfEntry;
} OLDDAILYOPSKey2Def;

typedef struct OLDDAILYOPSKey3Struct
{
  unsigned short int userID;
  long dateOfEntry;
  long timeOfEntry;
} OLDDAILYOPSKey3Def;

EXTERN OLDDAILYOPSDef     OLDDAILYOPS;
EXTERN TMSKey0Def         OLDDAILYOPSKey0;
EXTERN OLDDAILYOPSKey1Def OLDDAILYOPSKey1;
EXTERN OLDDAILYOPSKey2Def OLDDAILYOPSKey2;
EXTERN OLDDAILYOPSKey3Def OLDDAILYOPSKey3;
//
//  DAILYOPS Table Structure
//  Record length: 64
//  Defined flags: Multiple - see below
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Date/time of Entry
//                   9-12  "Pertains to" Date (yyyymmdd)
//                  13-16  "Pertains to" Time (seconds after midnight)
//                   17    User ID
//                   18    Record type flag
//                  19-22  Record flags
//                  23-26  DAILYOPS record ID
//                  27-30  DRIVERS record ID
//
//  Note: When making updates/additions here, update the relevant
//        portions of CDailyOps::ProcessExternalCommands().
//
#define DAILYOPS_FLAG_ROSTER                   0x01
#define DAILYOPS_FLAG_ROSTERESTABLISH          0x00000001
#define DAILYOPS_FLAG_ROSTERSETNEXTDAYTIME     0x00000002
#define DAILYOPS_FLAG_ROSTERSETNEXTNIGHTTIME   0x00000004
  
#define DAILYOPS_FLAG_BUS                      0x02
#define DAILYOPS_FLAG_BUSASSIGNMENT            0x00000001
#define DAILYOPS_FLAG_BUSSWAP                  0x00000002
#define DAILYOPS_FLAG_BUSRETURN                0x00000004
#define DAILYOPS_FLAG_BUSOVERRIDE              0x00000010
#define DAILYOPS_FLAG_BUSCLEARALL              0x00000020
#define DAILYOPS_FLAG_BUSMISSEDASSIGNMENT      0x00000040
#define DAILYOPS_FLAG_BLOCKDROP                0x00000080
#define DAILYOPS_FLAG_BLOCKUNDROP              0x00000100
#define DAILYOPS_FLAG_BUSADDEDTOTABLE          0x00000200
#define DAILYOPS_FLAG_BUSDELETEDFROMTABLE      0x00000400
#define DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE    0x00000800
#define DAILYOPS_FLAG_BUSMARKEDINSERVICE       0x00001000
#define DAILYOPS_FLAG_BUSISASSIGNED            0x00002000
#define DAILYOPS_FLAG_BUSISINSERVICE           0x00004000
#define DAILYOPS_FLAG_SETBUSLOCATION           0x00008000
#define DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT      0x00010000
#define DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT    0x00020000
#define DAILYOPS_FLAG_BUSMARKEDASCHARTER       0x00040000
#define DAILYOPS_FLAG_BUSUNMARKEDASCHARTER     0x00080000
#define DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING   0x00100000
#define DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING 0x00200000
#define DAILYOPS_FLAG_BUSOPERATORASSIGNMENT    0x00400000
#define DAILYOPS_FLAG_BUSOPERATORDEASSIGNMENT  0x00800000 

#define DAILYOPS_FLAG_DISPATCHER               0x03
#define DAILYOPS_FLAG_DISPATCHERSIGNIN         0x00000001

#define DAILYOPS_FLAG_OPERATOR                 0x04
#define DAILYOPS_FLAG_OPERATORCHECKIN          0x00000001
#define DAILYOPS_FLAG_OPERATORCHECKOUT         0x00000002
#define DAILYOPS_FLAG_OPERATORUNCHECK          0x00000004
#define DAILYOPS_FLAG_OPERATORDEASSIGN         0x00000008
#define DAILYOPS_FLAG_OVERTIME                 0x00000010
#define DAILYOPS_FLAG_EXTRATRIP                0x00000020
#define DAILYOPS_FLAG_OPERATOROFFTIMETEXT      0x00000040

#define DAILYOPS_FLAG_RIDERSHIP                0x08
#define DAILYOPS_FLAG_SETFARETYPE              0x00000001
#define DAILYOPS_FLAG_CLEARFARETYPE            0x00000002
#define DAILYOPS_FLAG_SETFARE                  0x00000004
#define DAILYOPS_FLAG_CLEARFARE                0x00000008
#define DAILYOPS_FLAG_SETRIDERSHIP             0x00000010
#define DAILYOPS_FLAG_CLEARRIDERSHIP           0x00000020
#define DAILYOPS_FLAG_SETMILEAGE               0x00000040
#define DAILYOPS_FLAG_CLEARMILEAGE             0x00000080
#define DAILYOPS_FLAG_ATTACHNOTE               0x00000100
#define DAILYOPS_FLAG_DETACHNOTE               0x00000200
#define DAILYOPS_FLAG_CASHADJUSTMENT           0x00000400
#define DAILYOPS_FLAG_CLEARCASHADJUSTMENT      0x00000800
#define DAILYOPS_FLAG_SPECIALTRIP              0x00001000
#define DAILYOPS_FLAG_CLEARSPECIALTRIP         0x00002000

#define DAILYOPS_FLAG_ABSENCE                  0x10
#define DAILYOPS_FLAG_ABSENCEREGISTER          0x00000001
#define DAILYOPS_FLAG_ABSENCEUNREGISTER        0x00000002
#define DAILYOPS_FLAG_ABSENCEFROMDISCIPLINE    0x00000004

#define DAILYOPS_FLAG_ABSENCETODISCIPLINE      0x00000010
#define DAILYOPS_FLAG_DISCIPLINEMETED          0x00000020
#define DAILYOPS_FLAG_ABSENCEPAID              0x00000040
#define DAILYOPS_FLAG_NOTSUREIFPAID            0x00000080
#define DAILYOPS_FLAG_ABSENCEOFFTIMETEXT       0x00000100
#define DAILYOPS_FLAG_DONTCOUNTASUNPAID        0x00000200
#define DAILYOPS_FLAG_PAID0VACATIONTIME        0x00000400
#define DAILYOPS_FLAG_PAID0PERSONALTIME        0x00000800
#define DAILYOPS_FLAG_PAID0SICKTIME            0x00001000
#define DAILYOPS_FLAG_PAID0FLOATTIME           0x00002000
#define DAILYOPS_FLAG_PAID1VACATIONTIME        0x00004000
#define DAILYOPS_FLAG_PAID1PERSONALTIME        0x00008000
#define DAILYOPS_FLAG_PAID1SICKTIME            0x00010000
#define DAILYOPS_FLAG_PAID1FLOATTIME           0x00020000
#define DAILYOPS_FLAG_CLEARASSIGNMENT          0x00040000

#define DAILYOPS_FLAG_OPENWORK                 0x20
#define DAILYOPS_FLAG_OPENWORKASSIGN           0x00000001
#define DAILYOPS_FLAG_OPENWORKCLEAR            0x00000002
#define DAILYOPS_FLAG_RUNSPLIT                 0x00000004
#define DAILYOPS_FLAG_UNRUNSPLIT               0x00000008
#define DAILYOPS_FLAG_SPLITATENDOFRELIEF       0x00000010
#define DAILYOPS_FLAG_SPLITATSTARTOFRELIEF     0x00000020
#define DAILYOPS_FLAG_RECOSTRUN                0x00000040
#define DAILYOPS_FLAG_UNRECOSTRUN              0x00000080
#define DAILYOPS_FLAG_CREWONLY                 0x00000100
#define DAILYOPS_FLAG_RUNSPLITINSERT           0x00000200
#define DAILYOPS_FLAG_UNRUNSPLITINSERT         0x00000400
#define DAILYOPS_FLAG_RUNSPLITCREWONLY         0x00000800

#define DAILYOPS_FLAG_SERVICE                  0x40
#define DAILYOPS_FLAG_SERVICEOVERRIDE          0x00000001
#define DAILYOPS_FLAG_SERVICERESTORE           0x00000002
#define DAILYOPS_FLAG_MAKEALLWORKOPEN          0x00000004

#define DAILYOPS_FLAG_DATE                     0x80
#define DAILYOPS_FLAG_DATESET                  0x00000001
#define DAILYOPS_FLAG_DATECLEAR                0x00000002

#define DAILYOPS_ASSOCIATEDDATA_LENGTH 34

//
//  Flag DAILYOPS_FLAG_ROSTER
//
typedef struct DOPSRosterStruct
{
  long ROSTERrecordID;
  long ROSTERDIVISIONSrecordID;
  long RUNSDIVISIONSrecordID;
  long nextDaytime;
  long nextNighttime;
} DOPSRosterDef;

//
//  Flag DAILYOPS_FLAG_BUS
//
typedef struct DOPSBusStruct
{
  long RGRPROUTESrecordID;
  long SGRPSERVICESrecordID;
  long BUSESrecordID;
  long TRIPSrecordID;
  long newBUSESrecordID;
  char swapReasonIndex;
  long locationNODESrecordID;
  long untilTime;
  long RUNSrecordID;
  char PAX;
} DOPSBusDef;

//
//  Flag DAILYOPS_FLAG_OPERATOR
//
typedef struct DOPSOperatorStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long PATTERNNAMESrecordID;
  long directionIndex;
  long timeAtMLP;
  long deadheadTime;
  long timeAdjustment;
  char extraTimeReasonIndex;
} DOPSOperatorDef;

//
//  Flag DAILYOPS_FLAG_ABSENCE
//
typedef struct DOPSAbsenceStruct
{
  long untilDate;
  long untilTime;
  long reasonIndex;
  long timeLost;
  long paidTime[2];
  long unpaidTime;
} DOPSAbsenceDef;

//
//  Flag DAILYOPS_FLAG_OPENWORK
//
typedef struct DOPSOpenWorkStruct
{
  long RUNSrecordID;
  long untilTime;
  long splitStartTRIPSrecordID;
  long splitStartTime;
  long splitStartNODESrecordID;
  long splitEndTime;
  long splitEndNODESrecordID;
  long payTime;
} DOPSOpenWorkDef;
  
//
//  Flag DAILYOPS_FLAG_SERVICE
//
typedef struct DOPSServiceStruct
{
  long SERVICESrecordID;
} DOPSServiceDef;

//
//  Flag DAILYOPS_FLAG_DATE
//
#define DAILYOPS_DATENAME_LENGTH 16

typedef struct DOPSDateStruct
{
  long SERVICESrecordID;
  char name[DAILYOPS_DATENAME_LENGTH];
} DOPSDateDef;

//
//  Flag DAILYOPS_FLAG_RIDERSHIP
//
#define RIDERSHIP_MAXFARETYPES     6
#define RIDERSHIP_FARETYPE_LENGTH 16

typedef struct DOPSRidershipStruct
{
  union flagged
  {
    struct _C
    {
      long  RUNSrecordID;
      float adjustments;
    } C;
    struct _R
    {
      long  RUNSrecordID;
      long  TRIPSrecordID;
      long  rosteredDRIVERSrecordID;
      long  actualDRIVERSrecordID;
      long  NODESrecordID;
      short int nodePositionInTrip;
      short int numFares[RIDERSHIP_MAXFARETYPES];
    } R;
    struct _M
    {
      long  RUNSrecordID;
      long  BUSESrecordID;
      float mileageOut;
      float mileageIn;
    } M;
    struct _FT
    {
      int   index;
      char  FareType[RIDERSHIP_FARETYPE_LENGTH];
    } FT;
    struct _F
    {
      int   index;
      long  ROUTESrecordID;
      float value;
    } F;
    struct _N
    {
      long  RUNSrecordID;
    } N;
    struct _S
    {
      long  runNumber;
      long  BUSESrecordID;
      long  actualDRIVERSrecordID;
      long  NODESrecordID;
      short int nodePositionInTrip;
      short int numFares[RIDERSHIP_MAXFARETYPES];
    } S;
  };
  union flagged data;
} DOPSRidershipDef;


//
//  Union of DOPS Structures
//
typedef union DOPSUnion
{
  DOPSRosterDef    RosterTemplate;
  DOPSBusDef       Bus;
  DOPSOperatorDef  Operator;
  DOPSAbsenceDef   Absence;
  DOPSOpenWorkDef  OpenWork;
  DOPSServiceDef   Service;
  DOPSDateDef      Date;
  DOPSRidershipDef Ridership;
  char             associatedData[DAILYOPS_ASSOCIATEDDATA_LENGTH];
} DOPSDef;

//
//  Main structure
//
typedef struct DAILYOPSStruct
{
  long   recordID;
  time_t entryDateAndTime;
  long   pertainsToDate;
  long   pertainsToTime;
  char   userID;
  char   recordTypeFlag;
  long   recordFlags;
  long   DAILYOPSrecordID;
  long   DRIVERSrecordID;
  DOPSDef DOPS;
} DAILYOPSDef;

//
//  Key 1 : Record type : "Pertains to" Date : "Pertains to" Time : Record Flags
//
typedef struct DAILYOPSKey1Struct
{
  char   recordTypeFlag;
  long   pertainsToDate;
  long   pertainsToTime;
  long   recordFlags;
} DAILYOPSKey1Def;

//
//  Key 2 : DAILYOPSrecordID
//
typedef struct DAILYOPSKey2Struct
{
  long   DAILYOPSrecordID;
} DAILYOPSKey2Def;

//
//  Key 3 : DRIVERSrecordID : "Pertains to" Date : "Pertains to" Time : Record type : Record Flags
//
typedef struct DAILYOPSKey3Struct
{
  long   DRIVERSrecordID;
  long   pertainsToDate;
  long   pertainsToTime;
  char   recordTypeFlag;
  long   recordFlags;
} DAILYOPSKey3Def;

//
//  Key 4 : User ID : "Pertains to" Date : "Pertains to" Time : Record type : Record Flags
//
typedef struct DAILYOPSKey4Struct
{
  char   userID;
  long   pertainsToDate;
  long   pertainsToTime;
  char   recordTypeFlag;
  long   recordFlags;
} DAILYOPSKey4Def;

//
//  Key 5 : Date/Time of Entry : "Pertains to" Date : "Pertains to" Time : Record type : Record Flags
//
typedef struct DAILYOPSKey5Struct
{
  time_t entryDateAndTime;
  long   pertainsToDate;
  long   pertainsToTime;
  char   recordTypeFlag;
  long   recordFlags;
} DAILYOPSKey5Def;

EXTERN DAILYOPSDef     DAILYOPS;
EXTERN TMSKey0Def      DAILYOPSKey0;
EXTERN DAILYOPSKey1Def DAILYOPSKey1;
EXTERN DAILYOPSKey2Def DAILYOPSKey2;
EXTERN DAILYOPSKey3Def DAILYOPSKey3;
EXTERN DAILYOPSKey4Def DAILYOPSKey4;
EXTERN DAILYOPSKey5Def DAILYOPSKey5;

//
//  OFFTIME Table Structure
//  Record length: 128
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   DAILYOPSrecordID
//                   9-124 Text associated with DAILYOPS record
//                 125-128 Flags
//

#define OFFTIME_TEXT_LENGTH 116

typedef struct OFFTIMEStruct
{
  long recordID;
  long DAILYOPSrecordID;
  char text[OFFTIME_TEXT_LENGTH];
  long flags;
} OFFTIMEDef;

typedef struct OFFTIMEKey1Struct
{
  long DAILYOPSrecordID;
} OFFTIMEKey1Def;

EXTERN OFFTIMEDef     OFFTIME;
EXTERN TMSKey0Def     OFFTIMEKey0;
EXTERN OFFTIMEKey1Def OFFTIMEKey1;

//
//  DISCIPLINE Data Structure
//  Record length: 128
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Comments Table Record ID Pointer
//                   9-12  Drivers Table Record ID Pointer
//                  13-16  Date of Entry
//                  17-20  Time of Entry
//                  21-24  Date of Offense
//                  25-28  Time of Offense
//                  29-32  Violation Type
//                  33-36  Violation Number
//                  37-40  Dropoff date
//                  41-104 Comments
//                 104-108 Suspension date
//                 109-112 Suspension duration
//                 113-116 Action Taken
//                 117-120 Reported by Drivers Table Record ID Pointer
//                 121-124 Reserved
//                 125-128 Flags
//

#define DISCIPLINE_FLAG_ABSENCERELATED   0x0001
#define DISCIPLINE_FLAG_OPERATIONS       0x0002
#define DISCIPLINE_FLAG_MAINTENANCE      0x0004
#define DISCIPLINE_FLAG_SUSPENSION       0x0008
#define DISCIPLINE_FLAG_FIXEDFUNKYTIME   0x0010
#define DISCIPLINE_FLAG_ACTUALACTION     0x0020
#define DISCIPLINE_FLAG_DEFERSUSPENSION  0x0040
#define DISCIPLINE_FLAG_DURATIONINHOURS  0x0080
#define DISCIPLINE_FLAG_SUSPENDEDWITHPAY 0x0100

#define DISCIPLINE_RESERVED_LENGTH  4
#define DISCIPLINE_UNUSED_LENGTH    4
#define DISCIPLINE_COMMENTS_LENGTH 64

typedef struct DISCIPLINEStruct
{
  long  recordID;                                              //   4
  long  COMMENTSrecordID;                                      //  +4 =  8
  long  DRIVERSrecordID;                                       //  +4 = 12
  time_t entryDateAndTime;                                     //  +4 = 16 
  char  unused[DISCIPLINE_UNUSED_LENGTH];                      //  +4 = 20 
  long  dateOfOffense;                                         //  +4 = 24 
  long  timeOfOffense;                                         //  +4 = 28 
  long  violationCategory;                                     //  +4 = 32
  long  violationType;                                         //  +4 = 36
  long  dropoffDate;                                           //  +4 = 40
  char  comments[DISCIPLINE_COMMENTS_LENGTH];                  //  +64=104
  long  suspensionDate;                                        //  +4 =108
  float suspensionDuration;                                    //  +4 =112
  long  actionTaken;                                           //  +4 =116
  long  reportedByDRIVERSrecordID;                             //  +4 =120
  long  DAILYOPSrecordID;                                      //  +4 =124
  unsigned short int flags;                                    //  +2 =126
  unsigned short int userID;                                   //  +2 =128
} DISCIPLINEDef;

typedef struct DISCIPLINEKey1Struct
{
  long DRIVERSrecordID;
  long dateOfOffense;
  long timeOfOffense;
} DISCIPLINEKey1Def;

typedef struct DISCIPLINEKey2Struct
{
  long DRIVERSrecordID;
  long violationCategory;
  long violationType;
} DISCIPLINEKey2Def;

typedef struct DISCIPLINEKey3Struct
{
  long violationCategory;
  long violationType;
} DISCIPLINEKey3Def;


EXTERN DISCIPLINEDef     DISCIPLINE;
EXTERN TMSKey0Def        DISCIPLINEKey0;
EXTERN DISCIPLINEKey1Def DISCIPLINEKey1;
EXTERN DISCIPLINEKey2Def DISCIPLINEKey2;
EXTERN DISCIPLINEKey3Def DISCIPLINEKey3;

//
//  RIDERSHIP Data Structure
//  Record length: 64
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-8   Date
//                   9-12  TRIPS record ID
//                  13-16  Rostered DRIVERS record ID
//                  17-20  Actual DRIVERS record ID
//                  21-24  NODES record ID
//                  25-26  Node position within pattern (typically 1 - multiple instances of a node within a pattern increase this value)
//                  27-46  Number of fares collected at this location by up to RIDERSHIP_MAXFARETYPES types
//                  47-60  Reserved
//                  61-64  Record flags
//

#define OLDRIDERSHIP_MAXFARETYPES    10
#define OLDRIDERSHIP_RESERVED_LENGTH 14

typedef struct RIDERSHIPStruct
{
  long  recordID;
  long  date;
  long  TRIPSrecordID;
  long  rosteredDRIVERSrecordID;
  long  actualDRIVERSrecordID;
  long  NODESrecordID;
  short int nodePositionInTrip;
  short int numFares[OLDRIDERSHIP_MAXFARETYPES];
  char  reserved[OLDRIDERSHIP_RESERVED_LENGTH];
  long  flags;
} RIDERSHIPDef;

typedef struct RIDERSHIPKey1Struct
{
  long TRIPSrecordID;
  long date;
} RIDERSHIPKey1Def;

typedef struct RIDERSHIPKey2Struct
{
  long actualDRIVERSrecordID;
  long date;
} RIDERSHIPKey2Def;

typedef struct RIDERSHIPKey3Struct
{
  long NODESrecordID;
  short int nodeOccurrenceInPattern;
  long date;
} RIDERSHIPKey3Def;

EXTERN RIDERSHIPDef     RIDERSHIP;
EXTERN TMSKey0Def       RIDERSHIPKey0;
EXTERN RIDERSHIPKey1Def RIDERSHIPKey1;
EXTERN RIDERSHIPKey2Def RIDERSHIPKey2;
EXTERN RIDERSHIPKey3Def RIDERSHIPKey3;
//
//  SUPPLEMENTAL Data Structure
//  Record length: Variable: Fixed portion = 32
//  Defined flags: None
//
//  Record Layout:  Bytes  Description
//                  ~~~~~  ~~~~~~~~~~~
//                   1-4   Record ID
//                   5-6   Referring Table ID
//                   7-10  Referring Record ID
//                  11-28  Reserved
//                  29-32  Record Flags
//                  33-    Supplemental Data
//
#define SUPPLEMENTAL_RESERVED_LENGTH 18
typedef struct SUPPLEMENTALStruct
{
  long      recordID;
  short int FromFileId;
  long      fromRecordID;
  char      reserved[SUPPLEMENTAL_RESERVED_LENGTH];
  long      flags;
} SUPPLEMENTALDef;

typedef struct SUPPLEMENTALKey1Struct
{
  short int FromFileId;
  long      fromRecordID;
} SUPPLEMENTALKey1Def;

EXTERN SUPPLEMENTALDef     SUPPLEMENTAL;
EXTERN TMSKey0Def          SUPPLEMENTALKey0;
EXTERN SUPPLEMENTALKey1Def SUPPLEMENTALKey1;

//
//  Database union typedefs
//
typedef struct DATABASEStruct
{
  long recordID;
  char recordData[MAX_TMS_FILE_LENGTH - 4];
} DATABASEDef;

typedef union AllFilesUnion
{
  DATABASEDef    DATABASE;
  DIRECTIONSDef  DIRECTIONS;
  ROUTESDef      ROUTES;
  ORDEREDLISTDef SERVICES;
  ORDEREDLISTDef DIVISIONS;
  ORDEREDLISTDef JURISDICTIONS;
  NODESDef       NODES;
  PATTERNSDef    PATTERNS;
  NORMNAMESDef   PATTERNNAMES;
  CONNECTIONSDef CONNECTIONS;
  BUSTYPESDef    BUSTYPES;
  BUSESDef       BUSES;
  COMMENTSDef    COMMENTS;
  char           hackedCommentText[COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH + 1];
  TRIPSDef       TRIPS;
  NORMNAMESDef   BLOCKNAMES;
  RUNSDef        RUNS;
  ROSTERDef      ROSTER;
  DRIVERSDef     DRIVERS;
  CUSTOMERSDef   CUSTOMERS;
  COMPLAINTSDef  COMPLAINTS;
  ORDEREDLISTDef DRIVERTYPES;
  TIMECHECKSDef  TIMECHECKS;
  DISCIPLINEDef  DISCIPLINE;
  CREWONLYDef    CREWONLY;
  DAILYOPSDef    DAILYOPS;
  OFFTIMEDef     OFFTIME;
} AllFilesDef;
typedef union AllKey0Union
{
  TMSKey0Def DATABASEKey0;
  TMSKey0Def DIRECTIONSKey0;
  TMSKey0Def ROUTESKey0;
  TMSKey0Def SERVICESKey0;
  TMSKey0Def DIVISIONSKey0;
  TMSKey0Def JURISDICTIONSKey0;
  TMSKey0Def NODESKey0;
  TMSKey0Def PATTERNSKey0;
  TMSKey0Def NORMNAMESKey0;
  TMSKey0Def CONNECTIONSKey0;
  TMSKey0Def BUSTYPESKey0;
  TMSKey0Def BUSESKey0;
  TMSKey0Def COMMENTSKey0;
  TMSKey0Def TRIPSKey0;
  TMSKey0Def BLOCKNAMESKey0;
  TMSKey0Def RUNSKey0;
  TMSKey0Def ROSTERKey0;
  TMSKey0Def DRIVERSKey0;
  TMSKey0Def DRIVERTYPESKey0;
  TMSKey0Def CUSTOMERSKey0;
  TMSKey0Def COMPLAINTSKey0;
  TMSKey0Def TIMECHECKSKey0;
  TMSKey0Def DISCIPLINEKey0;
  TMSKey0Def CREWONLYKey0;
  TMSKey0Def DAILYOPSKey0;
  TMSKey0Def OFFTIMEKey0;
} AllKey0Def;
typedef union AllKey1Union
{
  DATABASEDef        DATABASEKey1;
  DIRECTIONSKey1Def  DIRECTIONSKey1;
  ROUTESKey1Def      ROUTESKey1;
  ORDEREDLISTKey1Def SERVICESKey1;
  ORDEREDLISTKey1Def DIVISIONSKey1;
  ORDEREDLISTKey1Def JURISDICTIONSKey1;
  NODESKey1Def       NODESKey1;
  PATTERNSKey1Def    PATTERNSKey1;
  NORMNAMESKey1Def   PATTERNNAMESKey1;
  CONNECTIONSKey1Def CONNECTIONSKey1;
  ORDEREDLISTKey1Def BUSTYPESKey1;
  BUSESKey1Def       BUSESKey1;
  COMMENTSKey1Def    COMMENTSKey1;
  TRIPSKey1Def       TRIPSKey1;
  NORMNAMESKey1Def   BLOCKNAMESKey1;
  RUNSKey1Def        RUNSKey1;
  DRIVERSKey1Def     DRIVERSKey1;
  ORDEREDLISTKey1Def DRIVERTYPESKey1;
  ROSTERKey1Def      ROSTERKey1;
  CUSTOMERSKey1Def   CUSTOMERSKey1;
  COMPLAINTSKey1Def  COMPLAINTSKey1;
  TIMECHECKSKey1Def  TIMECHECKSKey1;
  DISCIPLINEKey1Def  DISCIPLINEKey1;
  CREWONLYKey1Def    CREWONLYKey1;
  DAILYOPSKey1Def    DAILYOPSKey1;
  OFFTIMEKey1Def     OFFTIMEKey1;
} AllKey1Def;

typedef union AllKey2Union
{
  DATABASEDef       DATABASEKey2;
  NODESKey2Def      NODESKey2;
  PATTERNSKey2Def   PATTERNSKey2;
  TRIPSKey2Def      TRIPSKey2;
  RUNSKey2Def       RUNSKey2;
  ROSTERKey2Def     ROSTERKey2;
  DRIVERSKey2Def    DRIVERSKey2;
  CUSTOMERSKey2Def  CUSTOMERSKey2;
  COMPLAINTSKey2Def COMPLAINTSKey2;
  TIMECHECKSKey2Def TIMECHECKSKey2;
  DAILYOPSKey2Def   DAILYOPSKey2;
} AllKey2Def;

typedef union AllKey3Union
{
  DATABASEDef       DATABASEKey3;
  TRIPSKey2Def      TRIPSKey3;
  DAILYOPSKey3Def   DAILYOPSKey3;
} AllKey3Def;

//
//  Base pattern stuff
//
EXTERN long basePatternRecordID;
EXTERN char basePatternName[NORMNAMES_NAME_LENGTH + 1];
#define BASE_PATTERN_NAME "BASE            "

// Reset the pack parameter to the default.
#pragma pack( pop, packDefault )

