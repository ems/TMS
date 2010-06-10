//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//  Payroll Report and Download
//
//  From Schema.ini:
//
//  [TMSRPT83.TXT]
//  ColNameHeader=False
//  Format=TabDelimited
//  CharacterSet=ANSI
//  Col1 = Sequence Integer
//  Col2 = BadgeNumber Char
//  Col3 = DriverName Char
//  Col4 = Date Char
//  Col5 = PayTime Integer
//  Col6 = PayCode Char
//  Col7 = DateAsNumber Integer
//  Col7 = Text Char
//
#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}  // extern "C"

#include "DailyOpsHeader.h"
#include "PayrollSelections.h"

extern "C" {

int  sort_OutputOrder(const void *a, const void *b);  // found in PayrollLayout.cpp

BOOL FAR TMSRpt83Ex(TMSRPTPassedDataDef *pPassedData);

BOOL FAR TMSRPT83(TMSRPTPassedDataDef *pPassedData)
{
  return TMSRpt83Ex(pPassedData);
}

char  szPayrollCodes[DRIVERS_NUMPAYROLLCODES][DRIVERS_PAYROLLCODE_LENGTH + 1];
int   today;
int   rosterWeek;
int   serviceIndex;


} // extern "C"

#include "TMS.h"

extern "C"
{
typedef struct OutputStruct
{
  long payTime;
  char szPayCode[32];
  char szWorkDay[TEMPSTRING_LENGTH];
  long recordType;
  long recordFlags;
  long checkinTime;
  long checkoutTime;
  long platformTime;
  long spreadTime;
  long runNumber;
  long ROUTESrecordID;
  BOOL bTripper;
  BOOL bForcedToWorkDayOff;
} OutputDef;

#define MAX_OUTPUTRECORDS 64


BOOL TMSRpt83Crystal(TMSRPTPassedDataDef *, PayrollSelectionsDef *);
BOOL TMSRpt83File(TMSRPTPassedDataDef *, PayrollSelectionsDef *);
int  TMSRpt83Operator(long, long, PayrollSelectionsDef *, OutputDef *, ABSENTDef *, int);
int  TMSRpt83RunDetails(PROPOSEDRUNDef *, COSTDef *, OutputDef *, int);

static long runNumber;
}

BOOL FAR TMSRpt83Ex(TMSRPTPassedDataDef *pPassedData)
{
  PayrollSelectionsDef PayrollSelections;
  CString   s;
  CTime     dateInEffect;
  CTimeSpan oneDay;
  BOOL  bFinishedOK;
//
//  Get the filter
//
  CPayrollSelections dlg(NULL, &PayrollSelections);

  PayrollSelections.pSelectedRecordIDs = NULL;
  if(dlg.DoModal() == IDCANCEL)
  {
    bFinishedOK = FALSE;
  }
//
//  Figure out what we're going to do - Crystal or an output file
//
  else
  {
    if(PayrollSelections.flags & PAYROLLSELECTIONS_FLAG_CRYSTAL)
    {
      bFinishedOK = TMSRpt83Crystal(pPassedData, &PayrollSelections);
    }
    else
    {
      bFinishedOK = TMSRpt83File(pPassedData, &PayrollSelections);
      pPassedData->flags |= TMSRPT_PASSEDDATAFLAG_DONOTUSECRYSTAL;
    }
  }

  TMSHeapFree(PayrollSelections.pSelectedRecordIDs);

  return(bFinishedOK);
}

BOOL TMSRpt83Crystal(TMSRPTPassedDataDef *pPD, PayrollSelectionsDef *pPS)
{
  OutputDef Output[MAX_OUTPUTRECORDS];
  CString   s;
  CTime     dateInEffect;
  CTimeSpan oneDay;
  HFILE hfOutputFile;
  char  outputString[512];
  char  szDate[64];
  char  szBadgeNumber[DRIVERS_BADGENUMBER_LENGTH + 1];
  char  szDriver[DRIVERS_FIRSTNAME_LENGTH + 2 + DRIVERS_LASTNAME_LENGTH + 1 + DRIVERTYPES_NAME_LENGTH + 3];
  char  szDriverType[DRIVERTYPES_NAME_LENGTH + 1];
  long  year, month, day;
  long  SERVICESrecordID;
  long  previousSERVICESrecordID;
  long  YYYYMMDD;
  int   nI, nJ, nK;
  int   rcode2;
  int   seq;
  int   numberOfDays;
  int   numInOutput;
  ABSENTDef AbsentList[500];
  int   numAbsent;
//
//  Open the output file
//
  pPD->nReportNumber = 82;
  pPD->numDataFiles = 1;
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT83.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    return(FALSE);
  }
  strcpy(pPD->szReportDataFile[0], tempString);
//
//  Filter the filter
//
//  Number of days to process and the roster week
//
  numberOfDays = (pPS->flags & PAYROLLSELECTIONS_FLAG_DAILY ? 1 : pPS->daysInPeriod);
  rosterWeek = pPS->startingRosterWeek;
//
//  Figure out the start date
//
  YYYYMMDD = pPS->startDateYYYYMMDD;
  GetYMD(YYYYMMDD, &year, &month, &day);

  dateInEffect = CTime(year, month, day, 0, 0, 0);
  oneDay       = CTimeSpan(1, 0, 0, 0);
//
//  Cycle through the days
//
  StatusBarStart(hWndMain, "Generating output file...");
  previousSERVICESrecordID = NO_RECORD;
  seq = 1;
  for(nI = 0; nI < numberOfDays; nI++)
  {
    SERVICESrecordID = CDailyOps::DetermineServiceDay(YYYYMMDD, FALSE, &today, tempString);
    strcpy(szDate, dateInEffect.Format(_T("%A, %B %d, %Y")));
//
//  Figure out who's away
//
    numAbsent = CDailyOps::RefreshAbsenceList(YYYYMMDD, AbsentList);
//
//  Initialize the trip planner
//
    if(SERVICESrecordID != previousSERVICESrecordID)
    {
      SERVICESKey0.recordID = SERVICESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      serviceIndex = SERVICES.number - 1;
      if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
      {
        serviceIndex = 0;
      }
      if(bUseDynamicTravels)
      {
        if(bUseCISPlan)
        {
          CISfree();
          if(!CISbuildService(FALSE, TRUE, SERVICESrecordID))
          {
            _lclose(hfOutputFile);
            return(FALSE);
          }
        }
        else
        {
          InitTravelMatrix(SERVICESrecordID, TRUE);
        }
      }
      previousSERVICESrecordID = SERVICESrecordID;
    }
//
//  Cycle through the operators
//
    for(nJ = 0; nJ < pPS->numSelected; nJ++)
    {
      DRIVERSKey0.recordID = pPS->pSelectedRecordIDs[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      memset(&Output, 0x00, sizeof(OutputDef) * MAX_OUTPUTRECORDS);
      numInOutput = TMSRpt83Operator(YYYYMMDD, DRIVERS.recordID, pPS, Output, AbsentList, numAbsent);
      if(numInOutput == 0)
      {
        continue;
      }
//
//  Save some DRIVERTYPE data
//
      DRIVERTYPESKey0.recordID = DRIVERS.DRIVERTYPESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
      if(rcode2 == 0)
      {
        for(nK = 0; nK < DRIVERS_NUMPAYROLLCODES; nK++)
        {
          strncpy(szPayrollCodes[nK], DRIVERTYPES.payrollCodes[nK], DRIVERS_PAYROLLCODE_LENGTH);
          trim(szPayrollCodes[nK], DRIVERS_PAYROLLCODE_LENGTH);
        }
      }
      else
      {
        memset(szPayrollCodes, 0x00, sizeof(szPayrollCodes));
      }
      strncpy(szDriverType, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
      trim(szDriverType, DRIVERTYPES_NAME_LENGTH);
//
//  Get some driver data
//
      strncpy(szBadgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(szBadgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strncpy(szDriver, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(szDriver, DRIVERS_LASTNAME_LENGTH);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      strcat(szDriver, " (");
      strcat(szDriver, szDriverType);
      strcat(szDriver, ")");
//
//  Dump out what we've got
//
      for(nK = 0; nK < numInOutput; nK++)
      {
        sprintf(outputString, "%d\t\"%s\"\t\"%s\"\t\"%s\"\t%ld\t\"%s\"\t%ld\t\"%s\"\r\n",
              seq++, szBadgeNumber, szDriver, szDate,
              Output[nK].payTime, Output[nK].szPayCode, YYYYMMDD, Output[nK].szWorkDay);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      }
    } // nJ
//
//  Get the next day
//
    dateInEffect += CTimeSpan(1, 0, 0, 0);
    year = dateInEffect.GetYear();
    month = dateInEffect.GetMonth();
    day = dateInEffect.GetDay();
    YYYYMMDD = (year * 10000) + (month * 100) + day;
    if(dateInEffect.GetDayOfWeek() == 2) // Monday rollover
    {
      rosterWeek++;
      if(rosterWeek >= ROSTER_MAX_WEEKS || rosterWeek > pPS->lastRosterWeek)
      {
        rosterWeek = 0;
      }
    }
  }
//
//  All done
//
  _lclose(hfOutputFile);
  StatusBarEnd();

  return(TRUE);
}

#define PAYROLL_SPACER                1
#define PAYROLL_DATE_YYYYMMDD         2
#define PAYROLL_DATE_DD_MMM_YYYY      3
#define PAYROLL_DATE_DD_MM_YYYY       4
#define PAYROLL_DATE_MM_DD_YYYY       5
#define PAYROLL_ROUTE                 6
#define PAYROLL_RUNNUMBER             7
#define PAYROLL_ROSTERNUMBER          8
#define PAYROLL_FIRSTNAME             9
#define PAYROLL_LASTNAME             10
#define PAYROLL_LASTPLUSFIRSTNAMES   11
#define PAYROLL_CHECKIN              12
#define PAYROLL_CHECKOUT             13
#define PAYROLL_LEAVEYARD            14
#define PAYROLL_RETURNYARD           15
#define PAYROLL_STARTLUNCH           16
#define PAYROLL_ENDLUNCH             17
#define PAYROLL_FIRSTPICKUP          18
#define PAYROLL_LASTDROPOFF          19
#define PAYROLL_VEHICLENUMBER        20
#define PAYROLL_DRIVERID             21
#define PAYROLL_JOBCODE              22
#define PAYROLL_PAYCODE              23
#define PAYROLL_STARTLUNCH2          24
#define PAYROLL_ENDLUNCH2            25
#define PAYROLL_STARTBREAK1          26
#define PAYROLL_ENDBREAK1            27
#define PAYROLL_STARTBREAK2          28
#define PAYROLL_ENDBREAK2            29
#define PAYROLL_STARTOUTOFSERVICE    30
#define PAYROLL_ENDOUTOFSERVICE      31
#define PAYROLL_TOTALTRIPS           32
#define PAYROLL_HOURS                33
#define PAYROLL_PLATFORMTIME         34
#define PAYROLL_REPORTTIME           35
#define PAYROLL_TURNINTIME           36
#define PAYROLL_TRAVELTIME           37
#define PAYROLL_DAILYGUARANTEE       38
#define PAYROLL_SPREADTIME           39
#define PAYROLL_SCHOOLSTRAIGHTTIME   40
#define PAYROLL_RUNHOURSREGULAR      41
#define PAYROLL_RUNHOURSOT           42
#define PAYROLL_RUNORTRIPPER         43
#define PAYROLL_WORKEDDAYOFF         44

typedef struct PayrollFieldsPointersStruct
{
  int  numFields;
  char *ptr[64];
} PayrollFieldsPointerDef;

typedef struct PayrollFieldsStruct
{
  char filler;
  char szSpacer[16];
  char szDateYYYYMMDD[16];
  char szDateDDMMMYYYY[16];
  char szDateDDMMYYYY[16];
  char szDateMMDDYYYY[16];
  char szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
  char szRunNumber[16];
  char szRosterNumber[16];
  char szFirstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  char szLastName[DRIVERS_LASTNAME_LENGTH + 1];
  char szLastPlusFirstNames[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1 + DRIVERTYPES_NAME_LENGTH + 3];
  char szCheckinTime[16];
  char szCheckoutTime[16];
  char szLeaveYardTime[16];
  char szReturnYardTime[16];
  char szStartLunchTime[16];
  char szEndLunchTime[16];
  char szFirstPickupTime[16];
  char szLastDropoffTime[16];
  char szVehicleNumber[BUSES_NUMBER_LENGTH + 1];
  char szDriverID[DRIVERS_BADGENUMBER_LENGTH + 1];
  char szJobCode[16];
  char szPayCode[16];
  char szStartLunch2[16];
  char szEndLunch2[16];
  char szStartBreak1[16];
  char szEndBreak1[16];
  char szStartBreak2[16];
  char szEndBreak2[16];
  char szStartOfOfServiceTime[16];
  char szEndOutOfServiceTime[16];
  char szTotalTrips[16];
  char szHours[16];
  char szPlatformTime[16];
  char szReportTime[16];
  char szTurninTime[16];
  char szTravelTime[16];
  char szDailyGuaranteeTime[16];
  char szSpreadTime[16];
  char szSchoolStraightTime[16];
  char szRunHoursRegular[16];
  char szRunHoursOT[16];
  char szRunOrTripper[2];  // R or T
  char szWorkedDayOff[2];  // T or F
} PayrollFieldsDef;

static PayrollFieldsPointerDef PFP;
static PayrollFieldsDef        PayrollFields;
static PayrollLayoutDef        PayrollFieldsLayout[MAXPAYROLLLAYOUTENTRIES];

static char szSeparator[2];
static BOOL bXML;
static BOOL bCSV;

#define OUTPUT_FOLDER_NAME "Payroll Data Files"

BOOL DumpRecord(char *);
void ClearFields();

BOOL TMSRpt83File(TMSRPTPassedDataDef *pPD, PayrollSelectionsDef *pPS)
{
  ABSENTDef AbsentList[500];
  int   numAbsent;
  OutputDef Output[MAX_OUTPUTRECORDS];
  char szOutputString[1024];
  int  nI, nJ, nK, nL, nM;
  int  numInOutput;

  bXML = (pPS->flags & PAYROLLSELECTIONS_FLAG_XML);
  bCSV = (pPS->flags & PAYROLLSELECTIONS_FLAG_CSV);

  if(bXML)
  {
    szSeparator[0] = '\0';
  }
  else
  {

    szSeparator[0] = (bCSV ? pPS->delimiter : '\t');
    szSeparator[1] = '\0';
  }
//
//  Sort PayrollFieldsLayout by output order
//
  memcpy(&PayrollFieldsLayout, &m_PayrollLayout, m_numPayrollLayoutEntries * sizeof(PayrollLayoutDef));
  qsort((void *)PayrollFieldsLayout, m_numPayrollLayoutEntries, sizeof(PayrollLayoutDef), sort_OutputOrder);
//
//  Set up the pointers to the structure
//
  PFP.numFields = 0;
  PFP.ptr[PFP.numFields++] = PayrollFields.szSpacer;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDateYYYYMMDD;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDateDDMMMYYYY;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDateDDMMYYYY;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDateMMDDYYYY;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRouteNumber;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRunNumber;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRosterNumber;
  PFP.ptr[PFP.numFields++] = PayrollFields.szFirstName;
  PFP.ptr[PFP.numFields++] = PayrollFields.szLastName;
  PFP.ptr[PFP.numFields++] = PayrollFields.szLastPlusFirstNames;
  PFP.ptr[PFP.numFields++] = PayrollFields.szCheckinTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szCheckoutTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szLeaveYardTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szReturnYardTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szStartLunchTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szEndLunchTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szFirstPickupTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szLastDropoffTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szVehicleNumber;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDriverID;
  PFP.ptr[PFP.numFields++] = PayrollFields.szJobCode;
  PFP.ptr[PFP.numFields++] = PayrollFields.szPayCode;
  PFP.ptr[PFP.numFields++] = PayrollFields.szStartLunch2;
  PFP.ptr[PFP.numFields++] = PayrollFields.szEndLunch2;
  PFP.ptr[PFP.numFields++] = PayrollFields.szStartBreak1;
  PFP.ptr[PFP.numFields++] = PayrollFields.szEndBreak1;
  PFP.ptr[PFP.numFields++] = PayrollFields.szStartBreak2;
  PFP.ptr[PFP.numFields++] = PayrollFields.szEndBreak2;
  PFP.ptr[PFP.numFields++] = PayrollFields.szStartOfOfServiceTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szEndOutOfServiceTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szTotalTrips;
  PFP.ptr[PFP.numFields++] = PayrollFields.szHours;
  PFP.ptr[PFP.numFields++] = PayrollFields.szPlatformTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szReportTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szTurninTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szTravelTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szDailyGuaranteeTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szSpreadTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szSchoolStraightTime;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRunHoursRegular;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRunHoursOT;
  PFP.ptr[PFP.numFields++] = PayrollFields.szRunOrTripper;
  PFP.ptr[PFP.numFields++] = PayrollFields.szWorkedDayOff;
//
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  LoadString(hInst, nI, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  Filter the filter
//
  BOOL bCombineOutput = (pPS->flags & PAYROLLSELECTIONS_FLAG_COMBINEOUTPUT);
//
//  Number of days to process and the roster week
//
  int  numberOfDays = (pPS->flags & PAYROLLSELECTIONS_FLAG_DAILY ? 1 : pPS->daysInPeriod);
  int  rosterWeek = pPS->startingRosterWeek;
//
//  Figure out the start and end dates
//
  long YYYYMMDD = pPS->startDateYYYYMMDD;
  long year, month, day;

  GetYMD(YYYYMMDD, &year, &month, &day);

  CTime startDate = CTime(year, month, day, 0, 0, 0);
  CTime endDate = startDate;
  if(numberOfDays > 1)
  {
    endDate = startDate + CTimeSpan(numberOfDays - 1, 0, 0, 0);
  }
//
//  Cycle through the days
//
  StatusBarStart(hWndMain, "Generating output file...");

  CString s;
  HFILE hfOutputFile;
  CTime thisDay;
  BOOL  bDoOpen;
  BOOL  bDoWrite;
  BOOL  bFound;
  char  szDate[64];
  char  szDriverType[DRIVERTYPES_NAME_LENGTH + 1];
  long  SERVICESrecordID;
  long  previousSERVICESrecordID = NO_RECORD;
  int   today;
  int   rcode2;
  int   dontDump;

  for(nI = 0; nI < numberOfDays; nI++)
  {
    thisDay = startDate + CTimeSpan(nI, 0, 0, 0);
//
//  Figure out the file name(s)
//
//  Case 1: One day (startDate - convention is "TMS Payroll Download YYYYMMDD")
//  Case 2:  >1 day (startDate to endDate in one file - convention is "TMS Payroll Download YYYYMMDD to YYYYMMDD")
//  Case 3:  >1 day (startDate to endDate in separate files - convention is "TMS Payroll Download YYYYMMDD")
//
//  Case 1: One day
//
    if(numberOfDays == 1)
    {
      YYYYMMDD = thisDay.GetYear() * 10000 + thisDay.GetMonth() * 100 + thisDay.GetDay();
      sprintf(tempString, "TMS Payroll Download %ld.", YYYYMMDD);
      bDoOpen = TRUE;
    }
//
//  Case 2: >1 day in the same file
//
    else
    {
      if(bCombineOutput)
      {
        bDoOpen = (nI == 0);
        if(bDoOpen)
        {
          YYYYMMDD = startDate.GetYear() * 10000 + startDate.GetMonth() * 100 + startDate.GetDay();
          sprintf(tempString, "TMS Payroll Download %ld to ", YYYYMMDD);
          YYYYMMDD = endDate.GetYear() * 10000 + endDate.GetMonth() * 100 + endDate.GetDay();
          sprintf(szarString, "%ld.", YYYYMMDD);
          strcat(tempString, szarString);
          bDoOpen = TRUE;
        }
      }
//
//  Case 3: >1 day in separate files
//
      else
      {
        if(nI > 0)
        {
          _lclose(hfOutputFile);
        }
        YYYYMMDD = thisDay.GetYear() * 10000 + thisDay.GetMonth() * 100 + thisDay.GetDay();
        sprintf(tempString, "TMS Payroll Download %ld.", YYYYMMDD);
        bDoOpen = TRUE;
      }
    }
//
//  Now append the extension
//
    strcat(tempString, (bXML ? "XML" : bCSV ? "CSV" : "txt"));
//
//  Open the file
//
    if(bDoOpen)
    {
      hfOutputFile = _lcreat(tempString, 0);
      if(hfOutputFile == HFILE_ERROR)
      {
        LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, tempString);
        MessageBeep(MB_ICONSTOP);
        MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
        return(FALSE);
      }
    }
//
//  Figure out the service day
//
    YYYYMMDD = thisDay.GetYear() * 10000 + thisDay.GetMonth() * 100 + thisDay.GetDay();
    SERVICESrecordID = CDailyOps::DetermineServiceDay(YYYYMMDD, FALSE, &today, tempString);
    strcpy(szDate, startDate.Format(_T("%A, %B %d, %Y")));
//
//  Figure out who's away
//
    numAbsent = CDailyOps::RefreshAbsenceList(YYYYMMDD, AbsentList);
//
//  Set up the output date fields
//
//  YYYYMMDD
//
    sprintf(PayrollFields.szDateYYYYMMDD, "%ld", YYYYMMDD);
//
//  DD-MMM-YYYY
//
    s.LoadString(TEXT_105 + thisDay.GetMonth() - 1);
    sprintf(PayrollFields.szDateDDMMMYYYY, "%02d-%s-%d", thisDay.GetDay(), s.Left(3), thisDay.GetYear());
//
//  DD/MM/YYYY
//
    sprintf(PayrollFields.szDateDDMMYYYY, "%02d/%02d/%d", thisDay.GetDay(), thisDay.GetMonth(), thisDay.GetYear());
//
//  MM/DD/YYYY
//
    sprintf(PayrollFields.szDateMMDDYYYY, "%02d/%02d/%d", thisDay.GetMonth(), thisDay.GetDay(), thisDay.GetYear());
//
//  Initialize the trip planner
//
    if(SERVICESrecordID != previousSERVICESrecordID)
    {
      if(bUseDynamicTravels)
      {
        if(bUseCISPlan)
        {
          CISfree();
          if(!CISbuildService(FALSE, TRUE, SERVICESrecordID))
          {
            _lclose(hfOutputFile);
            return(FALSE);
          }
        }
        else
        {
          InitTravelMatrix(SERVICESrecordID, TRUE);
        }
      }
      previousSERVICESrecordID = SERVICESrecordID;
    }
//
//  Cycle through the operators - what happened with this guy today?
//
    for(nJ = 0; nJ < pPS->numSelected; nJ++)
    {
      DRIVERSKey0.recordID = pPS->pSelectedRecordIDs[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
//
//  Save some DRIVERTYPE data
//
      DRIVERTYPESKey0.recordID = DRIVERS.DRIVERTYPESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
      if(rcode2 == 0)
      {
        for(nK = 0; nK < DRIVERS_NUMPAYROLLCODES; nK++)
        {
          strncpy(szPayrollCodes[nK], DRIVERTYPES.payrollCodes[nK], DRIVERS_PAYROLLCODE_LENGTH);
          trim(szPayrollCodes[nK], DRIVERS_PAYROLLCODE_LENGTH);
        }
      }
      else
      {
        memset(szPayrollCodes, 0x00, sizeof(szPayrollCodes));
      }
      strncpy(szDriverType, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
      trim(szDriverType, DRIVERTYPES_NAME_LENGTH);
//
//  See what happened
//
      numInOutput = TMSRpt83Operator(YYYYMMDD, DRIVERS.recordID, pPS, Output, AbsentList, numAbsent);
      if(numInOutput == 0)
      {
        continue;
      }
//
//  Set up the badge number, first name, last name, and last and first names
//
//  Badge number
//
      strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      strcpy(PayrollFields.szDriverID, tempString);
//
//  First name
//
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(PayrollFields.szFirstName, tempString);
//
//  Last name
//
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(PayrollFields.szLastName, tempString);
//
//  Last+first names with driver type appended
//
      sprintf(PayrollFields.szLastPlusFirstNames, "%s, %s", PayrollFields.szLastName, PayrollFields.szFirstName);
      strcat(PayrollFields.szLastPlusFirstNames, " (");
      strcat(PayrollFields.szLastPlusFirstNames, szDriverType);
      strcat(PayrollFields.szLastPlusFirstNames, ")");
//
//  Go through the output list to see if we've got an absence recorded
//  at 0:00.  This is our indication that he's been away for the whole
//  day, and that he should be paid either the actual time for it, or
//  the value found in AbsenceReasons.
//
      dontDump = NO_RECORD;
      for(nK = 0; nK < numInOutput; nK++)
      {
        if(Output[nK].payTime == 0)
        {
          for(bFound = FALSE, nL = 0; nL < DRIVERS_NUMPAYROLLCODES; nL++)
          {
            if(strcmp(Output[nK].szPayCode, szPayrollCodes[nL]) == 0)
            {
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
          {
            for(bFound = FALSE, nL = 0; nL < nK; nL++)
            {
              if(strcmp(Output[nL].szPayCode, szPayrollCodes[0]) == 0)
              {
                bFound = TRUE;
                break;
              }
            }
            if(bFound)
            {
              Output[nK].payTime = Output[nL].payTime;
              for(nM = 0; nM < m_numAbsenceReasons; nM++)
              {
                if(strcmp(Output[nK].szPayCode, m_AbsenceReasons[nM].szPayrollCode) == 0)
                {
                  if(m_AbsenceReasons[nM].paidOnAbsence != NO_TIME)
                  {
                    Output[nK].payTime = m_AbsenceReasons[nM].paidOnAbsence;
                    break;
                  }
                }
              }
              dontDump = nL;
              break;
            }
          }
        }
      }
        
//
//  Cycle through the output
//
      for(nK = 0; nK < numInOutput; nK++)
      {
        if(nK == dontDump)
        {
          continue;
        }
        if(Output[nK].szPayCode[0] == NO_RECORD)
        {
          continue;
        }
        ClearFields();
//
//  Job code
//  Payroll code
//
        strcpy(PayrollFields.szJobCode, Output[nK].szPayCode);
        strcpy(PayrollFields.szPayCode, Output[nK].szPayCode);
//
//  Forced to work?
//
        strcpy(PayrollFields.szWorkedDayOff, (Output[nK].bForcedToWorkDayOff ? "Y" : "N"));
//
//  Hours
//
        strcpy(PayrollFields.szHours, chhmm(Output[nK].payTime));
//
//  Checkin and checkout time
//
        strcpy(PayrollFields.szCheckinTime, chhmm(Output[nK].checkinTime));
        strcpy(PayrollFields.szCheckoutTime, chhmm(Output[nK].checkoutTime));
//
//  Platform time
//
        strcpy(PayrollFields.szPlatformTime, chhmm(Output[nK].platformTime));
//
//  Spread time
//
        strcpy(PayrollFields.szSpreadTime, chhmm(Output[nK].spreadTime));
//
//  Run or tripper
//
        strcpy(PayrollFields.szRunOrTripper, (Output[nK].bTripper ? "T" : "R"));
//
//  Run number
//
        sprintf(tempString, "%ld", Output[nK].runNumber);
        strcpy(PayrollFields.szRunNumber, tempString);
//
//  Route number
//
        if(Output[nK].ROUTESrecordID != NO_RECORD)
        {
          ROUTESKey0.recordID = Output[nK].ROUTESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          strcpy(PayrollFields.szRouteNumber, tempString);
        }
//
//  Dump and write the record
//
        bDoWrite = DumpRecord(szOutputString);
        if(bDoWrite)
        {
          _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
        }
      }
    }
  }
  _lclose(hfOutputFile);
  chdir("..");
  StatusBarEnd();

  return(TRUE);
}

BOOL DumpRecord(char *pszOutputString)
{
  int nI;

  strcpy(pszOutputString, "");
  for(nI = 0; nI < m_numPayrollLayoutEntries; nI++)
  {
    if(PayrollFieldsLayout[nI].position <= 0)
    {
      continue;
    }
    strcat(pszOutputString, PFP.ptr[PayrollFieldsLayout[nI].index]);
    if(!bXML)
    {
      strcat(pszOutputString, szSeparator);
    }
  }
  pszOutputString[strlen(pszOutputString)] = '\0';
  strcat(pszOutputString, "\r\n");

  return(TRUE);
}

void ClearFields()
{
  PayrollFields.szRouteNumber[0] = '\0';
  PayrollFields.szRunNumber[0] = '\0';
  PayrollFields.szRosterNumber[0] = '\0';
  PayrollFields.szCheckinTime[0] = '\0';
  PayrollFields.szCheckoutTime[0] = '\0';
  PayrollFields.szLeaveYardTime[0] = '\0';
  PayrollFields.szReturnYardTime[0] = '\0';
  PayrollFields.szStartLunchTime[0] = '\0';
  PayrollFields.szEndLunchTime[0] = '\0';
  PayrollFields.szFirstPickupTime[0] = '\0';
  PayrollFields.szLastDropoffTime[0] = '\0';
  PayrollFields.szVehicleNumber[0] = '\0';
  PayrollFields.szJobCode[0] = '\0';
  PayrollFields.szPayCode[0] = '\0';
  PayrollFields.szStartLunch2[0] = '\0';
  PayrollFields.szEndLunch2[0] = '\0';
  PayrollFields.szStartBreak1[0] = '\0';
  PayrollFields.szEndBreak1[0] = '\0';
  PayrollFields.szStartBreak2[0] = '\0';
  PayrollFields.szEndBreak2[0] = '\0';
  PayrollFields.szStartOfOfServiceTime[0] = '\0';
  PayrollFields.szEndOutOfServiceTime[0] = '\0';
  PayrollFields.szTotalTrips[0] = '\0';
  PayrollFields.szHours[0] = '\0';
  PayrollFields.szPlatformTime[0] = '\0';
  PayrollFields.szReportTime[0] = '\0';
  PayrollFields.szTurninTime[0] = '\0';
  PayrollFields.szTravelTime[0] = '\0';
  PayrollFields.szDailyGuaranteeTime[0] = '\0';
  PayrollFields.szSpreadTime[0] = '\0';
  PayrollFields.szSchoolStraightTime[0] = '\0';
  PayrollFields.szRunHoursRegular[0] = '\0';
  PayrollFields.szRunHoursOT[0] = '\0';
  PayrollFields.szRunOrTripper[0] = '\0';
  PayrollFields.szWorkedDayOff[0] = '\0';
}

int TMSRpt83Operator(long YYYYMMDD, long DRIVERSrecordID, PayrollSelectionsDef* pPS, OutputDef* Output, ABSENTDef *AbsentList, int numAbsent)
{
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef   COST;
  CString   s;
  CTime     dateInEffect;
  CTimeSpan oneDay;
  BOOL  bDoneOne;
  BOOL  bFound;
  BOOL  bCrewOnly;
  BOOL  bRecostFound;
  long  pieceNumber;
  long  cutAsRuntype;
  long  startTime;
  long  endTime;
  long  absRecID;
  long  RUNSrecordID;
  long  splitStartTime;
  long  splitEndTime;
  long  splitStartNODESrecordID;
  long  splitEndNODESrecordID;
  int   numPieces;
  int   nK;
  int   rcode2;
  int   numInOutput;

  numInOutput = 0;
//
//  See if this guy is rostered for today
//
  bDoneOne = FALSE;
  ROSTERKey2.DRIVERSrecordID = DRIVERSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
  while(rcode2 == 0 &&
        ROSTER.DRIVERSrecordID == DRIVERSrecordID)
  {
    Output[numInOutput].payTime = 0;
    Output[numInOutput].szPayCode[0] = NO_RECORD;
//
//  Not rostered
//
    if(ROSTER.WEEK[rosterWeek].RUNSrecordIDs[today] == NO_RECORD)
    {
      if(!bDoneOne)
      {
        sprintf(szarString, "Roster %ld - Day off", ROSTER.rosterNumber);
        strcpy(Output[numInOutput].szWorkDay, szarString);
        bDoneOne = TRUE;
      }
    }
//
//  Rostered
//
    else
    {
//
//  Crew-only run
//
      if(ROSTER.WEEK[rosterWeek].flags & (1 << today))
      {
        CREWONLYKey0.recordID = ROSTER.WEEK[rosterWeek].RUNSrecordIDs[today];
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        pieceNumber = 0;
        runNumber = CREWONLY.runNumber;
        while(rcode2 == 0 && CREWONLY.runNumber == runNumber)
        {
          if(CREWONLY.pieceNumber == 1)
          {
            cutAsRuntype = CREWONLY.cutAsRuntype;
          }
          PROPOSEDRUN.piece[pieceNumber].fromTime = CREWONLY.startTime;
          PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = CREWONLY.startNODESrecordID;
          PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = NO_RECORD;
          PROPOSEDRUN.piece[pieceNumber].toTime = CREWONLY.endTime;
          PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = CREWONLY.endNODESrecordID;
          PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = NO_RECORD;
          PROPOSEDRUN.piece[pieceNumber].prior.startTime = NO_TIME;
          PROPOSEDRUN.piece[pieceNumber].prior.endTime = NO_TIME;
          PROPOSEDRUN.piece[pieceNumber].after.startTime = NO_TIME;
          PROPOSEDRUN.piece[pieceNumber].after.endTime = NO_TIME;
          pieceNumber++;
          rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
        }
        PROPOSEDRUN.numPieces = pieceNumber;
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  Dump out the run details, if requested
//
        if(pPS->flags & PAYROLLSELECTIONS_FLAG_SHOWRUNDETAILS)
        {
          numInOutput = TMSRpt83RunDetails(&PROPOSEDRUN, &COST, Output, numInOutput);
        }
//
//  Load up the rest of the data
//
        Output[numInOutput].payTime = COST.TOTAL.payTime;
        sprintf(Output[numInOutput].szWorkDay,
              "Roster %ld: Crew-Only run %ld, paying %s",
               ROSTER.rosterNumber, runNumber, chhmm(Output[numInOutput].payTime));
        strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
        Output[numInOutput].checkinTime = PROPOSEDRUN.piece[0].fromTime;
        Output[numInOutput].checkoutTime = PROPOSEDRUN.piece[pieceNumber - 1].toTime;
        Output[numInOutput].platformTime = COST.TOTAL.platformTime;
        Output[numInOutput].spreadTime = COST.spreadTime;
        Output[numInOutput].bTripper = FALSE;
        Output[numInOutput].runNumber = runNumber;
        Output[numInOutput].ROUTESrecordID = NO_RECORD;
      }
//
//  Regular run
//
      else
      {
        RUNSKey0.recordID = ROSTER.WEEK[rosterWeek].RUNSrecordIDs[today];
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        Output[numInOutput].ROUTESrecordID = TRIPS.ROUTESrecordID;
        runNumber = RUNS.runNumber;
        numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
//
//  Dump out the run details, if requested
//
        if(pPS->flags & PAYROLLSELECTIONS_FLAG_SHOWRUNDETAILS)
        {
          numInOutput = TMSRpt83RunDetails(&PROPOSEDRUN, &COST, Output, numInOutput);
        }
//
//  Load up the rest of the data
//
        Output[numInOutput].payTime = COST.TOTAL.payTime;
        sprintf(Output[numInOutput].szWorkDay,
              "Roster %ld: Run %ld, paying %s",
               ROSTER.rosterNumber, runNumber, chhmm(Output[numInOutput].payTime));
        strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
        Output[numInOutput].checkinTime = RUNSVIEW[0].runOnTime;
        Output[numInOutput].checkoutTime = RUNSVIEW[numPieces - 1].runOffTime;
        Output[numInOutput].platformTime = COST.TOTAL.platformTime;
        Output[numInOutput].spreadTime = COST.spreadTime;
        Output[numInOutput].bTripper = (COST.TOTAL.payTime < 60 * 60 * 7);
        Output[numInOutput].runNumber = runNumber;
      }
    }
    Output[numInOutput].recordType = DAILYOPS_FLAG_ROSTER;
    Output[numInOutput].recordFlags = 0;
    if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
    {
      strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
    }
    else
    {
      strcpy(Output[numInOutput].szPayCode, "");
    }
    Output[numInOutput].bForcedToWorkDayOff = FALSE;
    numInOutput++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
  }
//
//  Anything from DailyOps?
//
//  Check on a deassign
//
  DAILYOPSKey3.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPSKey3.pertainsToDate = YYYYMMDD;
  DAILYOPSKey3.pertainsToTime = NO_TIME;
  DAILYOPSKey3.recordTypeFlag = NO_RECORD;
  DAILYOPSKey3.recordFlags = NO_RECORD;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  while(rcode2 == 0 &&
        (DAILYOPS.DRIVERSrecordID == DRIVERSrecordID) &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
        (DAILYOPS.pertainsToDate == YYYYMMDD))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3))
      {
        sprintf(Output[numInOutput].szWorkDay,
              "A Deassign was issued.  Rostered work removed");
        Output[numInOutput].szPayCode[0] = NO_RECORD;  // Prevent dump of this record to file
        Output[numInOutput].recordType = DAILYOPS_FLAG_OPERATOR;
        Output[numInOutput].recordFlags = DAILYOPS_FLAG_OPERATORDEASSIGN;
        Output[numInOutput].bForcedToWorkDayOff = FALSE;
        Output[numInOutput].checkinTime = NO_TIME;
        Output[numInOutput].checkoutTime = NO_TIME;
        Output[numInOutput].platformTime = NO_TIME;
        Output[numInOutput].spreadTime = NO_TIME;
        Output[numInOutput].bTripper = FALSE;
        Output[numInOutput].runNumber = NO_RECORD;
        Output[numInOutput].ROUTESrecordID = NO_RECORD;
        Output[numInOutput].payTime = 0;
        numInOutput++;
        bFound = TRUE;
        break;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  }
//
//  If a deassign was issued, go through what we have
//  to this point an set the pay hours to 0
//
  if(bFound)
  {
    for(nK = 0; nK < numInOutput; nK++)
    {
      if(Output[nK].recordType & DAILYOPS_FLAG_ROSTER)
      {
        Output[nK].payTime = 0;
      }
    }
  }
//
//  Check on overtime (or additional time)
//
  DAILYOPSKey3.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPSKey3.pertainsToDate = YYYYMMDD;
  DAILYOPSKey3.pertainsToTime = NO_TIME;
  DAILYOPSKey3.recordTypeFlag = NO_RECORD;
  DAILYOPSKey3.recordFlags = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  while(rcode2 == 0 &&
        (DAILYOPS.DRIVERSrecordID == DRIVERSrecordID) &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
        (DAILYOPS.pertainsToDate == YYYYMMDD))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OVERTIME)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3))
      {
        nK = DAILYOPS.DOPS.Operator.extraTimeReasonIndex;
        if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
        {
          strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_OVERTIME]);
        }
        else
        {
          strcpy(Output[numInOutput].szPayCode, "");
        }
        sprintf(Output[numInOutput].szWorkDay, "DailyOps Adjustment: %s.  Reason: %s",
              chhmm(DAILYOPS.DOPS.Operator.timeAdjustment), m_ExtraTimeReasons[nK].szText);
        Output[numInOutput].recordType = DAILYOPS_FLAG_OPERATOR;
        Output[numInOutput].recordFlags = DAILYOPS_FLAG_OVERTIME;
        Output[numInOutput].bForcedToWorkDayOff = FALSE;
        Output[numInOutput].checkinTime = NO_TIME;
        Output[numInOutput].checkoutTime = NO_TIME;
        Output[numInOutput].platformTime = NO_TIME;
        Output[numInOutput].spreadTime = NO_TIME;
        Output[numInOutput].bTripper = FALSE;
        Output[numInOutput].ROUTESrecordID = NO_RECORD;
        Output[numInOutput].payTime = DAILYOPS.DOPS.Operator.timeAdjustment;
        numInOutput++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  }
//
//  Check on an extra trip
//
  DAILYOPSKey3.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPSKey3.pertainsToDate = YYYYMMDD;
  DAILYOPSKey3.pertainsToTime = NO_TIME;
  DAILYOPSKey3.recordTypeFlag = NO_RECORD;
  DAILYOPSKey3.recordFlags = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  while(rcode2 == 0 &&
        (DAILYOPS.DRIVERSrecordID == DRIVERSrecordID) &&
        (DAILYOPS.pertainsToDate == YYYYMMDD))
  {
    if((DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) && (DAILYOPS.recordFlags & DAILYOPS_FLAG_EXTRATRIP))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3))
      {
        if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
        {
          strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_OVERTIME]);
        }
        else
        {
          strcpy(Output[numInOutput].szPayCode, "");
        }
        ROUTESKey0.recordID = DAILYOPS.DOPS.Operator.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szarString, ROUTES_NUMBER_LENGTH);
        sprintf(Output[numInOutput].szWorkDay,
              "DailyOps Adjustment: %s.  Reason: Extra trip on route %s",
              chhmm(DAILYOPS.DOPS.Operator.timeAdjustment), szarString);
        Output[numInOutput].recordType = DAILYOPS_FLAG_OPERATOR;
        Output[numInOutput].recordFlags = DAILYOPS_FLAG_EXTRATRIP;
        Output[numInOutput].bForcedToWorkDayOff = FALSE;
        Output[numInOutput].checkinTime = NO_TIME;
        Output[numInOutput].checkoutTime = NO_TIME;
        Output[numInOutput].platformTime = DAILYOPS.DOPS.Operator.timeAdjustment;
        Output[numInOutput].spreadTime = NO_TIME;
        Output[numInOutput].bTripper = TRUE;
        Output[numInOutput].ROUTESrecordID = ROUTES.recordID;
        Output[numInOutput].payTime = DAILYOPS.DOPS.Operator.timeAdjustment;
        numInOutput++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  }
//
//  Is he absent for any reason today?
//
  for(nK = 0; nK < numAbsent; nK++)
  {
    if(AbsentList[nK].DRIVERSrecordID != DRIVERSrecordID)
    {
      continue;
    }
    if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
    {
      strcpy(Output[numInOutput].szPayCode, m_AbsenceReasons[AbsentList[nK].reasonIndex].szPayrollCode);
    }
    else
    {
      strcpy(Output[numInOutput].szPayCode, "");
    }
//
//  Figure out the time he's away
//
    startTime = AbsentList[nK].fromTime;
    endTime = AbsentList[nK].toTime;
//
//  Build the output string
//
    sprintf(Output[numInOutput].szWorkDay, "Absent for %s.  Reason: %s",
          chhmm(AbsentList[nK].timeLost), m_AbsenceReasons[AbsentList[nK].reasonIndex].szText);
    Output[numInOutput].recordType = DAILYOPS_FLAG_ABSENCE;
    Output[numInOutput].recordFlags = DAILYOPS_FLAG_ABSENCEREGISTER;
    Output[numInOutput].payTime = AbsentList[nK].timeLost;
    Output[numInOutput].bForcedToWorkDayOff = FALSE;
    Output[numInOutput].checkinTime = NO_TIME;
    Output[numInOutput].checkoutTime = NO_TIME;
    Output[numInOutput].platformTime = NO_TIME;
    Output[numInOutput].spreadTime = NO_TIME;
    Output[numInOutput].bTripper = FALSE;
    Output[numInOutput].ROUTESrecordID = NO_RECORD;
    Output[numInOutput].runNumber = NO_RECORD;
    numInOutput++;
  }
//
//  Check the audit trail for any assignments that have been made for today
//
//  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
//  DAILYOPSKey1.pertainsToDate = YYYYMMDD;
//  DAILYOPSKey1.pertainsToTime = NO_RECORD;
//  DAILYOPSKey1.recordFlags = NO_RECORD;
//  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
//  while(rcode2 == 0 &&
//        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
//         DAILYOPS.pertainsToDate == YYYYMMDD)
//  {
//    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN)
//    {
//      if(!ANegatedRecord(DAILYOPS.recordID, 1))
//
//  Check the audit trail for any assignments that have been made for today
//
  DAILYOPSKey3.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPSKey3.pertainsToDate = YYYYMMDD;
  DAILYOPSKey3.pertainsToTime = NO_TIME;
  DAILYOPSKey3.recordTypeFlag = NO_RECORD;
  DAILYOPSKey3.recordFlags = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  while(rcode2 == 0 &&
         DAILYOPS.DRIVERSrecordID == DRIVERSrecordID &&
         DAILYOPS.pertainsToDate == YYYYMMDD)
  {
    if((DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) && (DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3))
      {
        bCrewOnly = DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY;
        RUNSrecordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
        if(bCrewOnly)
        {
          CREWONLYKey0.recordID = RUNSrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          runNumber = (rcode2 == 0 ? CREWONLY.runNumber : NO_RECORD);
        }
        else
        {
          RUNSKey0.recordID = RUNSrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          runNumber = (rcode2 == 0 ? RUNS.runNumber : NO_RECORD);
        }
//
//  Run splits
//
        if((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY) ||
              (DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT))
        {
          splitStartTime = DAILYOPS.DOPS.OpenWork.splitStartTime;
          splitEndTime = DAILYOPS.DOPS.OpenWork.splitEndTime;
          splitStartNODESrecordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
          splitEndNODESrecordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
          btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &DAILYOPSKey3, 3);
//          btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &DAILYOPSKey1, 1);
//
//  See if the split was recosted
//
          bRecostFound = FALSE;
          DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
          DAILYOPSKey1.pertainsToDate = YYYYMMDD;
          DAILYOPSKey1.pertainsToTime = 999999;
          DAILYOPSKey1.recordFlags = 999999;
          rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
          while(rcode2 == 0 &&
                DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_OPENWORK &&
                DAILYOPS.pertainsToDate == YYYYMMDD)
          {
            if(DAILYOPS.recordFlags & DAILYOPS_FLAG_RECOSTRUN)
            {
              if(DAILYOPS.DAILYOPSrecordID == NO_RECORD)
              {
                if((bCrewOnly && (DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY)) ||
                      (!bCrewOnly && !(DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY)))
                {
                  if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == RUNSrecordID &&
                        DAILYOPS.DOPS.OpenWork.splitStartTime == splitStartTime &&
                        DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID == splitStartNODESrecordID &&
                        DAILYOPS.DOPS.OpenWork.splitEndTime == splitEndTime &&
                        DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID == splitEndNODESrecordID)
                  {
                    bRecostFound = TRUE;
                    Output[numInOutput].payTime = DAILYOPS.DOPS.OpenWork.payTime;
                    if(bCrewOnly)
                    {
                      sprintf(Output[numInOutput].szWorkDay,
                            "Assigned to portion of crew-only run %ld, paying %s", runNumber, chhmm(DAILYOPS.DOPS.OpenWork.payTime));
                    }
                    else
                    {
                      sprintf(Output[numInOutput].szWorkDay,
                             "Assigned to portion of run %ld, paying %s", runNumber, chhmm(DAILYOPS.DOPS.OpenWork.payTime));
                    }
                    Output[numInOutput].recordType = DAILYOPS_FLAG_OPENWORK;
                    Output[numInOutput].recordFlags = 0;
                    if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
                    {
                      strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
                    }
                    else
                    {
                      strcpy(Output[numInOutput].szPayCode, "");
                    }
                    Output[numInOutput].bForcedToWorkDayOff = FALSE;
                    Output[numInOutput].checkinTime = splitStartTime;
                    Output[numInOutput].checkoutTime = splitEndTime;
                    Output[numInOutput].platformTime = splitEndTime - splitStartTime;
                    Output[numInOutput].spreadTime = NO_TIME;
                    Output[numInOutput].bTripper = TRUE;
                    Output[numInOutput].runNumber = runNumber;
                    if(bCrewOnly)
                    {
                      Output[numInOutput].ROUTESrecordID = NO_RECORD;
                    }
                    else
                    {
                      RUNSKey0.recordID = RUNSrecordID;
                      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
                      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                      Output[numInOutput].ROUTESrecordID = TRIPS.ROUTESrecordID;
                    }
                    numInOutput++;
                    break;
                  }
                }
              }
            }
            rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
          }
          if(!bRecostFound)
          {
            Output[numInOutput].payTime = 0;
            if(bCrewOnly)
            {
              sprintf(Output[numInOutput].szWorkDay,
                    "Assigned to portion of crew-only run %ld: *** Not recosted ***", runNumber);
            }
            else
            {
              sprintf(Output[numInOutput].szWorkDay,
                    "Assigned to portion of run %ld: *** Not recosted ***", runNumber);
            }
            Output[numInOutput].recordType = DAILYOPS_FLAG_OPENWORK;
            Output[numInOutput].recordFlags = 0;
            if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
            {
              strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
            }
            else
            {
              strcpy(Output[numInOutput].szPayCode, "");
            }
            Output[numInOutput].bForcedToWorkDayOff = FALSE;
            Output[numInOutput].checkinTime = splitStartTime;
            Output[numInOutput].checkoutTime = splitEndTime;
            Output[numInOutput].platformTime = splitEndTime - splitStartTime;
            Output[numInOutput].spreadTime = NO_TIME;
            Output[numInOutput].bTripper = TRUE;
            Output[numInOutput].runNumber = runNumber;
            if(bCrewOnly)
            {
              Output[numInOutput].ROUTESrecordID = NO_RECORD;
            }
            else
            {
              RUNSKey0.recordID = RUNSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
              TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
              Output[numInOutput].ROUTESrecordID = TRIPS.ROUTESrecordID;
            }
            numInOutput++;
          }
          DAILYOPS.recordID = absRecID;
          btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
//          btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        }
//
//  Whole runs
//
        else
        {
//          bCrewOnly = DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY;
          cutAsRuntype = CDailyOps::SetupRun(DAILYOPS.DOPS.OpenWork.RUNSrecordID, bCrewOnly, &PROPOSEDRUN);
          numPieces = PROPOSEDRUN.numPieces;
          RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
          Output[numInOutput].payTime = COST.TOTAL.payTime;
          if(bCrewOnly)
          {
            sprintf(Output[numInOutput].szWorkDay,
                  "Assigned to crew-only run %ld, paying %s", runNumber, chhmm(COST.TOTAL.payTime));
          }
          else
          {
            sprintf(Output[numInOutput].szWorkDay,
                  "Assigned to run %ld, paying %s", runNumber, chhmm(COST.TOTAL.payTime));
          }
          Output[numInOutput].recordType = DAILYOPS_FLAG_OPENWORK;
          Output[numInOutput].recordFlags = 0;
          if(pPS->flags & PAYROLLSELECTIONS_FLAG_CODES)
          {
            strcpy(Output[numInOutput].szPayCode, szPayrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR]);
          }
          else
          {
            strcpy(Output[numInOutput].szPayCode, "");
          }
          Output[numInOutput].bForcedToWorkDayOff = FALSE;
          Output[numInOutput].checkinTime = PROPOSEDRUN.piece[0].fromTime;
          Output[numInOutput].checkoutTime = PROPOSEDRUN.piece[numPieces - 1].toTime;
          Output[numInOutput].platformTime = COST.TOTAL.platformTime;
          Output[numInOutput].spreadTime = COST.spreadTime;
          Output[numInOutput].bTripper = (COST.TOTAL.payTime < 60 * 60 * 7);
          Output[numInOutput].runNumber = runNumber;
          if(bCrewOnly)
          {
            Output[numInOutput].ROUTESrecordID = NO_RECORD;
          }
          else
          {
            RUNSKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            Output[numInOutput].ROUTESrecordID = TRIPS.ROUTESrecordID;
          }
          numInOutput++;
//
//  Dump out the run details, if requested
//
          if(pPS->flags & PAYROLLSELECTIONS_FLAG_SHOWRUNDETAILS)
          {
            numInOutput = TMSRpt83RunDetails(&PROPOSEDRUN, &COST, Output, numInOutput);
          }
        }
      }
    }
//    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
//    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);             
  }
  
  return(numInOutput);
}

//
//  Load up the rest of the data
//
int TMSRpt83RunDetails(PROPOSEDRUNDef *pPROPOSEDRUN, COSTDef *pCOST, OutputDef *pOutput, int startAt)
{
  long tempLong;
  long travelTime;
  long OTIndex;
  int  nI;
  int  nType, nSlot;
  
  int  pos = startAt;
//
//  Fill in the controls
//
//  Run characteristics group box title
//
  nType = (short int)LOWORD(pCOST->runtype);
  nSlot = (short int)HIWORD(pCOST->runtype);
  if(nType < 0 || nSlot < 0)
  {
    LoadString(hInst, TEXT_052, tempString, TEMPSTRING_LENGTH);
  }
  else
  {
    strcpy(tempString, RUNTYPE[nType][nSlot].localName);
  }

  sprintf(pOutput[pos].szWorkDay, "Run %4d (%s)", runNumber, tempString);
  pos++;
//
//  The list box of the pieces of the run
//
  long totalPlat = 0;
  long totalRpt = 0;
  long totalTrv = 0;
  long totalTI = 0;
  long regularTime;
  long overTime;

//
//  Header
//
  strcpy(pOutput[pos].szWorkDay, "Block   Pce   Report   At    On      Off     At    Plat    Rpt     Trv     T/I    Total");
  pos++;
//
//  Run details
//
  for(nI = 0; nI < pPROPOSEDRUN->numPieces; nI++)
  {
    memset(&pOutput[pos], 0x00, sizeof(OutputDef));
//
//  Block Number
//
    if(pPROPOSEDRUN->piece[nI].fromTRIPSrecordID == NO_RECORD)
    {
      strcpy(pOutput[pos].szWorkDay, "       ");
    }
    else
    {
      TRIPSKey0.recordID = pPROPOSEDRUN->piece[nI].fromTRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(TRIPS.standard.blockNumber <= 0)
      {
        strcpy(pOutput[pos].szWorkDay, "       ");
      }
      else
      {
        sprintf(pOutput[pos].szWorkDay, "%5ld  ", TRIPS.standard.blockNumber);
      }
    }

//
//  Piece Number
//
    sprintf(szarString, "%2d     ", nI + 1);
    strcat(pOutput[pos].szWorkDay, szarString);
//
//  On Location
//
    NODESKey0.recordID = pPROPOSEDRUN->piece[nI].fromNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    tempString[NODES_ABBRNAME_LENGTH] = '\0';
    strcat(pOutput[pos].szWorkDay, tempString);
    strcat(pOutput[pos].szWorkDay, "  ");
//
//  Report at time
//
    tempLong = pPROPOSEDRUN->piece[nI].fromTime - pCOST->PIECECOST[nI].reportTime;
    sprintf(tempString, "%5s   ", Tchar(tempLong));
    strcat(pOutput[pos].szWorkDay, tempString);
//
//  On Time
//
    sprintf(tempString, "%5s   ", Tchar(pPROPOSEDRUN->piece[nI].fromTime));
    strcat(pOutput[pos].szWorkDay, tempString);
//
//  Off Time
//
    sprintf(tempString, "%5s   ", Tchar(pPROPOSEDRUN->piece[nI].toTime));
    strcat(pOutput[pos].szWorkDay, tempString);
//
//  Off Location
//
    NODESKey0.recordID = pPROPOSEDRUN->piece[nI].toNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    tempString[NODES_ABBRNAME_LENGTH] = '\0';
    strcat(pOutput[pos].szWorkDay, tempString);
    strcat(pOutput[pos].szWorkDay, "  ");
//
//  Platform time
//
    sprintf(tempString, "%5s   ", chhmm(pCOST->PIECECOST[nI].platformTime));
    strcat(pOutput[pos].szWorkDay, tempString);
    totalPlat += pCOST->PIECECOST[nI].platformTime;
//
//  Report time
//
    sprintf(tempString, "%5s   ", chhmm(pCOST->PIECECOST[nI].reportTime));
    strcat(pOutput[pos].szWorkDay, tempString);
    totalRpt += pCOST->PIECECOST[nI].reportTime;
//
//  Travel time
//
    travelTime = pCOST->TRAVEL[nI].startTravelTime + pCOST->TRAVEL[nI].endTravelTime;
    sprintf(tempString, "%5s   ", chhmm(travelTime));
    strcat(pOutput[pos].szWorkDay, tempString);
    totalTrv += travelTime;
//
//  Turnin time
//
    sprintf(tempString, "%5s   ", chhmm(pCOST->PIECECOST[nI].turninTime));
    strcat(pOutput[pos].szWorkDay, tempString);
    totalTI += pCOST->PIECECOST[nI].turninTime;
    if(pPROPOSEDRUN->numPieces == 1)
    {
      sprintf(tempString, "%5s", chhmm(pCOST->TOTAL.payTime));
      strcat(pOutput[pos].szWorkDay, tempString);
    }
    pos++;
  }

  char szTotalPlat[16];
  char szTotalRpt[16];
  char szTotalTrv[16];
  char szTotalTI[16];
  char szTotalPayTime[16];
  char szTotalSpdOT[16];
  char szRegularTime[16];
  char szOverTime[16];

  strcpy(szTotalPlat, chhmm(totalPlat));
  strcpy(szTotalRpt, chhmm(totalRpt));
  strcpy(szTotalTrv, chhmm(totalTrv));
  strcpy(szTotalTI, chhmm(totalTI));
  strcpy(szTotalPayTime, chhmm(pCOST->TOTAL.payTime));
  strcpy(szTotalSpdOT, chhmm(pCOST->spreadOvertime));

//
//  Add a total line if there's more than one piece
//
  if(pPROPOSEDRUN->numPieces > 1)
  {
    strcpy(pOutput[pos].szWorkDay, "Totals -----------------------------------------> ");
    sprintf(tempString, "%5s   ", szTotalPlat);
    strcat(pOutput[pos].szWorkDay, tempString);
    sprintf(tempString, "%5s   ", szTotalRpt);
    strcat(pOutput[pos].szWorkDay, tempString);
    sprintf(tempString, "%5s   ", szTotalTrv);
    strcat(pOutput[pos].szWorkDay, tempString);
    sprintf(tempString, "%5s   ", szTotalTI);
    strcat(pOutput[pos].szWorkDay, tempString);
    sprintf(szarString, "%s", szTotalPayTime);
    strcat(pOutput[pos].szWorkDay, szarString);
    pos++;
  }
//
//  Do the summary lines
//
//  strcpy(pOutput[pos].szWorkDay, "");
//  pos++;
//  sprintf(tempString, "Piece Details:  Platform: %s  Report time: %s  Travel time: %s  Turnin Time: %s",
//        szTotalPlat, szTotalRpt, szTotalTrv, szTotalTI);
//  strcpy(pOutput[pos].szWorkDay, tempString);
//  pos++;

  strcpy(pOutput[pos].szWorkDay, "");
  if(pCOST->TOTAL.makeUpTime > 0)
  {
    sprintf(tempString, "MakeUp Time: %s  ", chhmm(pCOST->TOTAL.makeUpTime));
    strcat(pOutput[pos].szWorkDay, tempString);
  }
  if(pCOST->spreadOvertime > 0)
  {
    sprintf(tempString, "Spread Premium: %s (%s)", szTotalSpdOT, chhmm(pCOST->spreadTime));
    strcat(pOutput[pos].szWorkDay, tempString);
  }
  if(strcmp(pOutput[pos].szWorkDay, "") != 0)
  {
    pos++;
  }
  if(nType >= 0 && nSlot >= 0)
  {
    OTIndex = RUNTYPE[nType][nSlot].flags & RTFLAGS_4DAY ? 0 : 1;
    if(OVERTIME.dailyRate[OTIndex][serviceIndex] <= 0)
    {
      overTime = 0;
    }
    else
    {
      overTime = (long)(pCOST->TOTAL.overTime / OVERTIME.dailyRate[OTIndex][serviceIndex]);
    }
    regularTime = pCOST->TOTAL.payTime - pCOST->TOTAL.overTime;
    strcpy(szRegularTime, chhmm(regularTime));
    strcpy(szOverTime, chhmm(overTime));
    sprintf(tempString, "Total Regular Time: %s  Total Time at OT: %s  Total Work Pay: %s",
          szRegularTime,  szOverTime,  szTotalPayTime);
    strcpy(pOutput[pos].szWorkDay, tempString);
    pos++;
  }
//
//  All done
//
  return(pos);
}