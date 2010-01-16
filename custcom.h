//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#define CCREFER_FLAGS_SELECTED          0x0001

typedef struct CCREFERStruct
{
  char szName[CCREFER_NAME_LENGTH];
  char szDepartment[CCREFER_DEPARTMENT_LENGTH];
  char szEmailAddress[CCREFER_EMAILADDRESS_LENGTH];
  long flags;
} CCREFERDef;

#define CCFE_FLAGS_COMPLAINT            0x0001
#define CCFE_FLAGS_COMMENDATION         0x0002
#define CCFE_FLAGS_SERVICEREQUEST       0x0004
#define CCFE_FLAGS_INFORMATIONREQUEST   0x0008
#define CCFE_FLAGS_NEW                  0x0010
#define CCFE_FLAGS_MISCELLANEOUS        0x0020
#define CCFE_DATETIME_LENGTH           64
#define CCFE_RECEIVEDBY_LENGTH         64

typedef struct CCFEStruct
{
  long dateTime;
  char szDateTime[CCFE_DATETIME_LENGTH];
  char szReceivedBy[CCFE_RECEIVEDBY_LENGTH];
  long flags;
  COMPLAINTSDef COMPLAINTS;
  CCREFERDef    *pCCREFER;
  COMPLAINTSDef SNAPSHOT;
  char szWeather[128];
  long numReferrals;
  BOOL bSupervisor;
} CCFEDef; 
  

