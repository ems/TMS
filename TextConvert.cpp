//
//  TextConvert() - Transfer data from stock TMS "text" files into an Access Database
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextAbsenceReasons.h"
#include "TextActionCodes.h"
#include "TextBusSwapReasons.h"
#include "TextCategories.h"
#include "TextClassificationCodes.h"
#include "TextDepartments.h"
#include "TextExtraTimeReasons.h"
#include "TextOtherComments.h"
#include "TextPassengerLoads.h"
#include "TextPayrollLayout.h"
#include "TextReferrals.h"
#include "TextRoadSupervisors.h"
#include "TextStopFlags.h"
#include "TextSupervisor.h"
#include "TextTrafficConditions.h"
#include "TextTypical.h"
#include "TextViolationActions.h"
#include "TextViolationCategories.h"
#include "TextViolations.h"
#include "TextWeatherConditions.h"

BOOL CTMSApp::TextConvert()
{
  CDatabase db;
  CString   s;
  CString   sRegKey = "Software\\ODBC\\ODBC.INI\\TMS Text Access Database"; 
//
//  Tell the registry where the database is
//
  HKEY  hKey;
  long  result;

  result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
  if(result != 0)
  {
    s.LoadString(ERROR_240);
    sprintf(tempString, s, result);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, tempString, TMS, MB_OK);
    return(FALSE);
  }
//
//  Is there a "Common Text.accdb" in the Common folder?
//
  CString sADB = szDatabaseRoot;
  
  sADB += "\\Common\\";
  sADB += TEXT_DATABASE_FILENAME;
//
//  If it's there, assume we're good and just return
//
  if(GetFileAttributes(sADB) != 0xFFFFFFFF)  
  {
    return(TRUE);
  }
//
//  If it isn't there, copy it from Fresh
//
  CString sFromFile = szInstallDirectory;

  sFromFile += "\\Fresh\\";
  sFromFile += TEXT_DATABASE_FILENAME;
  if(CopyFile(sFromFile, sADB, FALSE) == 0)
  {
    s.LoadString(ERROR_371);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, s, TMS, MB_OK);
    return(FALSE);
  }

//
//  Ok - Passed the tests - Onto the population step
//
  DWORD type = REG_SZ;

  strcpy(tempString, sADB);
  RegSetValueEx(hKey, "DBQ", 0, type, (LPBYTE)tempString, strlen(tempString));
//
//  Open the empty database
//
  db.OpenEx("DSN=TMS Text Access Database", CDatabase::noOdbcDialog );
  int xx = db.CanUpdate();
//
//  Create the recordsets
//
  CTextAbsenceReasons      TextAbsenceReasons(&db);
  CTextActionCodes         TextActionCodes(&db);
  CTextBusSwapReasons      TextBusSwapReasons(&db);
  CTextCategories          TextCategories(&db);
  CTextClassificationCodes TextClassificationCodes(&db);
  CTextDepartments         TextDepartments(&db);
  CTextExtraTimeReasons    TextExtraTimeReasons(&db);
  CTextOtherComments       TextOtherComments(&db);
  CTextPassengerLoads      TextPassengerLoads(&db);
  CTextPayrollLayout       TextPayrollLayout(&db);
  CTextReferrals           TextReferrals(&db);
  CTextRoadSupervisors     TextRoadSupervisors(&db);
  CTextStopFlags           TextStopFlags(&db);
  CTextSupervisor          TextSupervisor(&db);
  CTextTrafficConditions   TextTrafficConditions(&db);
  CTextTypical             TextTypical(&db);
  CTextViolationActions    TextViolationActions(&db);
  CTextViolationCategories TextViolationCategories(&db);
  CTextViolations          TextViolations(&db);
  CTextWeatherConditions   TextWeatherConditions(&db);
//
//  Go through all the text files
//
  CString fileName;
  CString inputLine;
  int     sequence;
//
//  ***************
//  Absence Reasons
//  ***************
//
  fileName = szInstallDirectory;
  fileName += "\\Absence Reasons.txt";
  AbsenceReasons.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the absence reasons
//
  AbsenceReasons.SeekToBegin();
  TextAbsenceReasons.Open();
  while(AbsenceReasons.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
//
//  If the pointer to the rule number is -1, it means we have an unused entry - skip it
//
    if(strcmp(inputLine.Mid(6, 2), "-1") == 0)
    {
      continue;
    }
    TextAbsenceReasons.AddNew();
//
//  Rule number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextAbsenceReasons.m_ReasonNumber = atoi(tempString);
//
//  Reason pointer
//
    strcpy(tempString, inputLine.Mid(6, 5));
    TextAbsenceReasons.m_ReasonPointer = atoi(tempString);
//
//  Does this count toward unpaid time?
//
//  Later versions of this file have a space in column 13 with column 12
//  as either Y or N.  This reflects whether the absence's unpaid time
//  counts toward the unpaid time report.  If column 13 isn't a space, it
//  means the file hasn't been updated and we'll assume that it's a Y.
//
//  Parse the string for the text
//
    strcpy(tempString, inputLine.Mid(12, 2));
    if(tempString[1] == ' ')
    {
      tempString[1] = '\0';
      TextAbsenceReasons.m_ReasonCountsTowardUnpaidTime = tempString;
      strcpy(tempString, inputLine.Mid(14));
    }
    else
    {
      TextAbsenceReasons.m_ReasonCountsTowardUnpaidTime = "Y";
      strcpy(tempString, inputLine.Mid(12));
    }
    TextAbsenceReasons.m_ReasonText = tempString;
//
//  Insert
//
    TextAbsenceReasons.Update();
    TextAbsenceReasons.MoveNext();
  }
  AbsenceReasons.Close();
//
//  ************
//  Action Codes
//  ************
//
  fileName = szInstallDirectory;
  fileName += "\\Action Codes.txt";
  ActionCodes.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Action Codes
//
  ActionCodes.SeekToBegin();
  TextActionCodes.Open();
  while(ActionCodes.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextActionCodes.AddNew();
//
//  Code number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextActionCodes.m_CodeNumber = atol(tempString);
//
//  Code text
//
    strcpy(tempString, inputLine.Mid(8));
    TextActionCodes.m_CodeText = tempString;
//
//  Insert
//
    TextActionCodes.Update();
    TextActionCodes.MoveNext();
  }
  ActionCodes.Close();
//
//  **********
//  Stop flags
//  **********
//
  fileName = szInstallDirectory;
  fileName += "\\Stop Flags.txt";
  StopFlags.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Stop flags
//
  StopFlags.SeekToBegin();
  TextStopFlags.Open();
  while(StopFlags.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    if(strcmp(inputLine.Mid(6), "*Unused*") == 0)
    {
      continue;
    }

    TextStopFlags.AddNew();
//
//  Stop flag number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextStopFlags.m_StopFlagNumber = atol(tempString);
//
//  Stop flag text
//
    strcpy(tempString, inputLine.Mid(6));
    TextStopFlags.m_StopFlagText = tempString;
//
//  Insert
//
    TextStopFlags.Update();
    TextStopFlags.MoveNext();
  }
  StopFlags.Close();
//
//  ***************
//  Passenger Loads
//  ***************
//
  fileName = szInstallDirectory;
  fileName += "\\Passenger Loads.txt";
  PassengerLoads.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Passenger Loads
//
  PassengerLoads.SeekToBegin();
  TextPassengerLoads.Open();
  while(PassengerLoads.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextPassengerLoads.AddNew();
//
//  Load number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextPassengerLoads.m_LoadNumber = atol(tempString);
//
//  Load text
//
    strcpy(tempString, inputLine.Mid(6));
    TextPassengerLoads.m_LoadText = tempString;
//
//  Insert
//
    TextPassengerLoads.Update();
    TextPassengerLoads.MoveNext();
  }
  PassengerLoads.Close();
//
//  ******************
//  Weather Conditions
//  ******************
//
  fileName = szInstallDirectory;
  fileName += "\\Weather Conditions.txt";
  WeatherConditions.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Weather Conditions
//
  WeatherConditions.SeekToBegin();
  TextWeatherConditions.Open();
  while(WeatherConditions.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextWeatherConditions.AddNew();
//
//  Weather number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextWeatherConditions.m_WeatherNumber = atol(tempString);
//
//  Weather text
//
    strcpy(tempString, inputLine.Mid(6));
    TextWeatherConditions.m_WeatherText = tempString;
//
//  Insert
//
    TextWeatherConditions.Update();
    TextWeatherConditions.MoveNext();
  }
  WeatherConditions.Close();
//
//  ******************
//  Traffic Conditions
//  ******************
//
  fileName = szInstallDirectory;
  fileName += "\\Traffic Conditions.txt";
  TrafficConditions.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Traffic Conditions
//
  TrafficConditions.SeekToBegin();
  TextTrafficConditions.Open();
  while(TrafficConditions.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextTrafficConditions.AddNew();
//
//  Traffic number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextTrafficConditions.m_TrafficNumber = atol(tempString);
//
//  Traffic text
//
    strcpy(tempString, inputLine.Mid(6));
    TextTrafficConditions.m_TrafficText = tempString;
//
//  Insert
//
    TextTrafficConditions.Update();
    TextTrafficConditions.MoveNext();
  }
  TrafficConditions.Close();
//
//  **************
//  Other Comments
//  **************
//
  fileName = szInstallDirectory;
  fileName += "\\Other Comments.txt";
  OtherComments.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Other Comments
//
  OtherComments.SeekToBegin();
  TextOtherComments.Open();
  while(OtherComments.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextOtherComments.AddNew();
//
//  Other number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextOtherComments.m_OtherNumber = atol(tempString);
//
//  Other text
//
    strcpy(tempString, inputLine.Mid(6));
    TextOtherComments.m_OtherText = tempString;
//
//  Insert
//
    TextOtherComments.Update();
    TextOtherComments.MoveNext();
  }
  OtherComments.Close();
//
//  ****************
//  Road Supervisors
//  ****************
//
  fileName = szInstallDirectory;
  fileName += "\\Road Supervisors.txt";
  RoadSupervisors.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Road Supervisors
//
  RoadSupervisors.SeekToBegin();
  TextRoadSupervisors.Open();
  while(RoadSupervisors.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextRoadSupervisors.AddNew();
//
//  Road Supervisor number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextRoadSupervisors.m_RoadSupNumber = atol(tempString);
//
//  Road Supervisor name
//
    strcpy(tempString, inputLine.Mid(6));
    TextRoadSupervisors.m_RoadSupName = tempString;
//
//  Insert
//
    TextRoadSupervisors.Update();
    TextRoadSupervisors.MoveNext();
  }
  RoadSupervisors.Close();
//
//  ****************
//  Bus Swap Reasons
//  ****************
//
  fileName = szInstallDirectory;
  fileName += "\\Bus Swap Reasons.txt";
  BusSwapReasons.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Bus Swap Reasons
//
  BusSwapReasons.SeekToBegin();
  TextBusSwapReasons.Open();
  while(BusSwapReasons.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    if(strcmp(inputLine.Mid(6), "*Unused*") == 0)
    {
      continue;
    }
    if(strcmp(inputLine.Mid(6), "*Reserved*") == 0)
    {
      continue;
    }

    TextBusSwapReasons.AddNew();
//
//  Reason number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextBusSwapReasons.m_ReasonNumber = atol(tempString);
//
//  Reason text
//
    strcpy(tempString, inputLine.Mid(6));
    TextBusSwapReasons.m_ReasonText = tempString;
//
//  Insert
//
    TextBusSwapReasons.Update();
    TextBusSwapReasons.MoveNext();
  }
  BusSwapReasons.Close();
//
//  **********
//  Violations
//  **********
//
  fileName = szInstallDirectory;
  fileName += "\\Violations.txt";
  Violations.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Violations
//
  int  previousCategory = NO_RECORD;
  int  nI;

  Violations.SeekToBegin();
  TextViolations.Open();
	sequence = 1;
  while(Violations.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
//
//  Locate the caret
//
    nI = inputLine.Find("^", 0);
    if(nI == -1)
    {
      continue;
    }
    TextViolations.AddNew();
//
//  Parse the string
//
    strncpy(tempString, inputLine, nI);
    tempString[nI] = '\0';
//
//  Violation Category index
//
    strcpy(szarString, strtok(tempString, "\t"));
    TextViolations.m_ViolationCategoryIndex = atoi(szarString);
//
//  Violation number within category
//
    if(TextViolations.m_ViolationCategoryIndex != previousCategory)
    {
      previousCategory = TextViolations.m_ViolationCategoryIndex;
      sequence = 1;
    }
    TextViolations.m_ViolationNumber = sequence++;
//
//  Violation duration
//
    strcpy(szarString, strtok(NULL, "\t"));
    TextViolations.m_ViolationDuration = atoi(szarString);
//
//  Violation absence related flag
//
    strcpy(szarString, strtok(NULL, " \t"));
    TextViolations.m_ViolationAbsenceRelated = szarString;
//
//  Violation text
//
    strcpy(szarString, strtok(NULL, "\n"));
    TextViolations.m_ViolationText = szarString;
//
//  Set up the balance of the string
//
    strcpy(szarString, inputLine);
    strcpy(tempString, &szarString[nI + 1]);
	  TextViolations.m_ViolationActionsIndexes = _T("");

    char *token;
    token = strtok(tempString, "\t\n");
    while(token != NULL)
    {
      TextViolations.m_ViolationActionsIndexes += token;
      token = strtok(NULL, "\t\n");
      if(token != NULL)
      {
        TextViolations.m_ViolationActionsIndexes += ",";
      }
    }
//
//  Insert
//
    TextViolations.Update();
    TextViolations.MoveNext();
  }
  Violations.Close();
//
//  *****************
//  Violation Actions
//  *****************
//
  fileName = szInstallDirectory;
  fileName += "\\Violation Actions.txt";
  ViolationActions.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Violation Actions
//
  ViolationActions.SeekToBegin();
  TextViolationActions.Open();
  while(ViolationActions.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextViolationActions.AddNew();
//
//  Violation Action number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextViolationActions.m_ActionNumber = atol(tempString);
//
//  Violation Action name
//
    strcpy(tempString, inputLine.Mid(6));
    TextViolationActions.m_ActionText = tempString;
//
//  Insert
//
    TextViolationActions.Update();
    TextViolationActions.MoveNext();
  }
  ViolationActions.Close();
//
//  ********************
//  Violation Categories
//  ********************
//
  fileName = szInstallDirectory;
  fileName += "\\Violation Categories.txt";
  ViolationCategories.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Violation Categories
//
  ViolationCategories.SeekToBegin();
  TextViolationCategories.Open();
  while(ViolationCategories.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextViolationCategories.AddNew();
//
//  Violation Action number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextViolationCategories.m_CategoryNumber = atol(tempString);
//
//  Violation Action name
//
    strcpy(tempString, inputLine.Mid(6));
    TextViolationCategories.m_CategoryText = tempString;
//
//  Insert
//
    TextViolationCategories.Update();
    TextViolationCategories.MoveNext();
  }
  ViolationCategories.Close();
//
//  ******************
//  Extra Time Reasons
//  ******************
//
  fileName = szInstallDirectory;
  fileName += "\\Extra Time Reasons.txt";
  ExtraTimeReasons.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Extra Time Reasons
//
  ExtraTimeReasons.SeekToBegin();
  TextExtraTimeReasons.Open();
  while(ExtraTimeReasons.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    if(strcmp(inputLine.Mid(6), "*Unused*") == 0)
    {
      continue;
    }
    if(strcmp(inputLine.Mid(6), "*Reserved*") == 0)
    {
      continue;
    }
    TextExtraTimeReasons.AddNew();
//
//  Extra Time number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextExtraTimeReasons.m_ExtraTimeNumber = atol(tempString);
//
//  Extra Time text
//
    strcpy(tempString, inputLine.Mid(6));
    TextExtraTimeReasons.m_ExtraTimeText = tempString;
//
//  Extra Time payroll code
//
	  TextExtraTimeReasons.m_ExtraTimePayrollCode = _T("");
//
//  Insert
//
    TextExtraTimeReasons.Update();
    TextExtraTimeReasons.MoveNext();
  }
  ExtraTimeReasons.Close();
//
//  **********
//  Categories
//  **********
//
  fileName = szInstallDirectory;
  fileName += "\\Categories.txt";
  Categories.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Categories
//
  Categories.SeekToBegin();
  TextCategories.Open();
  while(Categories.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextCategories.AddNew();
//
//  Category number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextCategories.m_CategoryNumber = atol(tempString);
//
//  Category text
//
    strcpy(tempString, inputLine.Mid(8));
    TextCategories.m_CategoryText = tempString;
//
//  Insert
//
    TextCategories.Update();
    TextCategories.MoveNext();
  }
  Categories.Close();
//
//  ********************
//  Classification Codes
//  ********************
//
  fileName = szInstallDirectory;
  fileName += "\\Classification Codes.txt";
  ClassificationCodes.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Classification Codes
//
  ClassificationCodes.SeekToBegin();
  TextClassificationCodes.Open();
  while(ClassificationCodes.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextClassificationCodes.AddNew();
//
//  Classification number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextClassificationCodes.m_ClassificationNumber = atol(tempString);
//
//  Classification text
//
    strcpy(tempString, inputLine.Mid(8));
    TextClassificationCodes.m_ClassificationText = tempString;
//
//  Insert
//
    TextClassificationCodes.Update();
    TextClassificationCodes.MoveNext();
  }
  ClassificationCodes.Close();
//
//  ***********
//  Departments
//  ***********
//
  fileName = szInstallDirectory;
  fileName += "\\Departments.txt";
  Departments.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Departments
//
  Departments.SeekToBegin();
  TextDepartments.Open();
  while(Departments.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextDepartments.AddNew();
//
//  Department number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextDepartments.m_DepartmentNumber = atol(tempString);
//
//  Department text
//
    strcpy(tempString, inputLine.Mid(8));
    TextDepartments.m_DepartmentText = tempString;
//
//  Insert
//
    TextDepartments.Update();
    TextDepartments.MoveNext();
  }
  Departments.Close();
//
//  *********
//  Referrals
//  *********
//
  int refNum = 1;

  fileName = szInstallDirectory;
  fileName += "\\Referrals.txt";
  Referrals.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Departments
//
  Referrals.SeekToBegin();
  TextReferrals.Open();
  while(Referrals.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextReferrals.AddNew();
//
//  Referral number
//
    TextReferrals.m_ReferralNumber = refNum++;
//
//  Referral name
//
    nI = inputLine.Find(",");
    strcpy(tempString, inputLine.Left(nI));
    inputLine = inputLine.Mid(nI + 1);
    nI = inputLine.Find(",");
    strcpy(szarString, inputLine.Left(nI));
    inputLine = inputLine.Mid(nI + 1);
    TextReferrals.m_ReferralName = tempString;
    TextReferrals.m_ReferralName += ", ";
    TextReferrals.m_ReferralName += szarString;
//
//  Referral department
//
    nI = inputLine.Find(",");
    strcpy(tempString, inputLine.Left(nI));
    TextReferrals.m_ReferralDepartment = tempString;
//
//  Referral email
//
    strcpy(tempString, inputLine.Mid(nI + 1));
    TextReferrals.m_ReferralEmail = tempString;
//
//  Insert
//
    TextReferrals.Update();
    TextReferrals.MoveNext();
  }
  Referrals.Close();
//
//  *******
//  Typical
//  *******
//
  fileName = szInstallDirectory;
  fileName += "\\Typical.txt";
  Typical.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Passenger Loads
//
  Typical.SeekToBegin();
  TextTypical.Open();
  while(Typical.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextTypical.AddNew();
//
//  Load number
//
    strcpy(tempString, inputLine.Mid(0, 5));
    TextTypical.m_TypicalNumber = atol(tempString);
//
//  Load text
//
    strcpy(tempString, inputLine.Mid(6));
    TextTypical.m_TypicalText = tempString;
//
//  Insert
//
    TextTypical.Update();
    TextTypical.MoveNext();
  }
  Typical.Close();
//
//  **********
//  Supervisor
//  **********
//
  fileName = szInstallDirectory;
  fileName += "\\Supervisor.txt";
  Supervisor.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
//
//  Set up the Supervisors
//
  Supervisor.SeekToBegin();
  TextSupervisor.Open();
  sequence = 1;
  while(Supervisor.ReadString(inputLine))
  {
    if(strcmp(inputLine, "") == 0)
    {
      continue;
    }
    TextSupervisor.AddNew();
//
//  Supervisor number
//
    TextSupervisor.m_SupervisorNumber = sequence++;
//
//  Supervisor name
//
    nI = inputLine.Find(",");
    strcpy(tempString, inputLine.Left(nI));
    TextSupervisor.m_SupervisorName = tempString;
//
//  Supervisor password
//
    inputLine = inputLine.Mid(nI + 1);
    strcpy(tempString, inputLine);
    TextSupervisor.m_SupervisorPassword = tempString;
//
//  Insert
//
    TextSupervisor.Update();
    TextSupervisor.MoveNext();
  }
  Supervisor.Close();
//
//  All done
//
  db.Close();

  return(TRUE);
}
