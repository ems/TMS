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

extern "C"
{
int sort_PayrollLayout(const void *a, const void *b)
{
  PayrollLayoutDef *pa, *pb;
  pa = (PayrollLayoutDef *)a;
  pb = (PayrollLayoutDef *)b;

  return(strcmp(pa->szText, pb->szText));
}
}

BOOL CTMSApp::TextLoad()
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
//  "Common Text.accdb" is in the Common folder
//
  CString sADB = szDatabaseRoot;
  
  sADB += "\\Common\\";
  sADB += TEXT_DATABASE_FILENAME;
//
//  If it's not there, assume the worst and leave
//
  if(GetFileAttributes(sADB) == 0xFFFFFFFF)  
  {
    return(FALSE);
  }
//
//  Ok - Passed the tests
//
  DWORD type = REG_SZ;

  strcpy(tempString, sADB);
  RegSetValueEx(hKey, "DBQ", 0, type, (LPBYTE)tempString, strlen(tempString));
//
//  Open the database
//
  db.OpenEx("DSN=TMS Text Access Database", CDatabase::noOdbcDialog );
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
  CDBVariant varValue;
//
//  ***************
//  Absence Reasons
//  ***************
//
  TextAbsenceReasons.Open();
//
//  Set up the absence reasons
//
  m_numAbsenceReasons = 0;
  if(TextAbsenceReasons.GetRecordCount() > 0)
  {
    TextAbsenceReasons.MoveFirst();
    while(!TextAbsenceReasons.IsEOF())
    {
//
//  Rule pointer
//
      m_AbsenceReasons[m_numAbsenceReasons].ruleNumber = TextAbsenceReasons.m_ReasonPointer;
/*
//
//  Does this count toward unpaid time?
//
      m_AbsenceReasons[m_numAbsenceReasons].bCountsTowardUnpaidTime =
            (TextAbsenceReasons.m_ReasonCountsTowardUnpaidTime == "Y");
*/
//
//  What gets paid on an absence?
//
      if(TextAbsenceReasons.m_ReasonCountsTowardUnpaidTime == "Actual")
      {
        m_AbsenceReasons[m_numAbsenceReasons].paidOnAbsence = NO_TIME;
      }
      else
      {
        strcpy(tempString, TextAbsenceReasons.m_ReasonCountsTowardUnpaidTime);
        m_AbsenceReasons[m_numAbsenceReasons].paidOnAbsence = thhmm(tempString);
      }

//
//  Rule text
//
      strcpy(m_AbsenceReasons[m_numAbsenceReasons].szText, TextAbsenceReasons.m_ReasonText);
//
//  Rule payroll code
//
      strcpy(m_AbsenceReasons[m_numAbsenceReasons].szPayrollCode, TextAbsenceReasons.m_ReasonPayrollCode);
//
//  Get the next record
//
      TextAbsenceReasons.MoveNext();
      m_numAbsenceReasons++;
    }
  }
  TextAbsenceReasons.Close( );
//
//  ************
//  Action Codes
//  ************
//
  TextActionCodes.Open();
//
//  Set up the action codes
//
  m_numActionCodes = 0;
  if(TextActionCodes.GetRecordCount() > 0)
  {
    TextActionCodes.MoveFirst();
    while(!TextActionCodes.IsEOF())
    {
//
//  Number
//
      m_ActionCodes[m_numActionCodes].number = TextActionCodes.m_CodeNumber;
//
//  Text
//
      strcpy(m_ActionCodes[m_numActionCodes].szText, TextActionCodes.m_CodeText);
//
//  Get the next record
//
      TextActionCodes.MoveNext();
      m_numActionCodes++;
    }
  }
  TextActionCodes.Close( );
//
//  ****************
//  Bus Swap Reasons
//  ****************
//
  TextBusSwapReasons.Open();
//
//  Set up the bus swap reasons
//
  m_numBusSwapReasons = 0;
  if(TextBusSwapReasons.GetRecordCount() > 0)
  {
    TextBusSwapReasons.MoveFirst();
    while(!TextBusSwapReasons.IsEOF())
    {
//
//  Number
//
      m_BusSwapReasons[m_numBusSwapReasons].number = TextBusSwapReasons.m_ReasonNumber;
//
//  Text
//
      strcpy(m_BusSwapReasons[m_numBusSwapReasons].szText, TextBusSwapReasons.m_ReasonText);
//
//  Get the next record
//
      TextBusSwapReasons.MoveNext();
      m_numBusSwapReasons++;
    }
  }
  TextBusSwapReasons.Close( );
//
//  **********
//  Categories
//  **********
//
  TextCategories.Open();
//
//  Set up the categories
//
  m_numCategories = 0;
  if(TextCategories.GetRecordCount() > 0)
  {
    TextCategories.MoveFirst();
    while(!TextCategories.IsEOF())
    {
//
//  Number
//
      m_Categories[m_numCategories].number = TextCategories.m_CategoryNumber;
//
//  Text
//
      strcpy(m_Categories[m_numCategories].szText, TextCategories.m_CategoryText);
//
//  Get the next record
//
      TextCategories.MoveNext();
      m_numCategories++;
    }
  }
  TextCategories.Close( );
//
//  ********************
//  Classification Codes
//  ********************
//
  TextClassificationCodes.Open();
//
//  Set up the classification codes
//
  m_numClassificationCodes = 0;
  if(TextClassificationCodes.GetRecordCount() > 0)
  {
    TextClassificationCodes.MoveFirst();
    while(!TextClassificationCodes.IsEOF())
    {
//
//  Number
//
      m_ClassificationCodes[m_numClassificationCodes].number = TextClassificationCodes.m_ClassificationNumber;
//
//  Text
//
      strcpy(m_ClassificationCodes[m_numClassificationCodes].szText, TextClassificationCodes.m_ClassificationText);
//
//  Get the next record
//
      TextClassificationCodes.MoveNext();
      m_numClassificationCodes++;
    }
  }
  TextClassificationCodes.Close( );
//
//  ***********
//  Departments
//  ***********
//
  TextDepartments.Open();
//
//  Set up the departments
//
  m_numDepartments = 0;
  if(TextDepartments.GetRecordCount() > 0)
  {
    TextDepartments.MoveFirst();
    while(!TextDepartments.IsEOF())
    {
//
//  Number
//
      m_Departments[m_numDepartments].number = TextDepartments.m_DepartmentNumber;
//
//  Text
//
      strcpy(m_Departments[m_numDepartments].szText, TextDepartments.m_DepartmentText);
//
//  Get the next record
//
      TextDepartments.MoveNext();
      m_numDepartments++;
    }
  }
  TextDepartments.Close( );
//
//  ******************
//  Extra Time Reasons
//  ******************
//
  TextExtraTimeReasons.Open();
//
//  Set up the extra time reasons
//
  m_numExtraTimeReasons = 0;
  if(TextExtraTimeReasons.GetRecordCount() > 0)
  {
    TextExtraTimeReasons.MoveFirst();
    while(!TextExtraTimeReasons.IsEOF())
    {
//
//  Extra Time number
//
      m_ExtraTimeReasons[m_numExtraTimeReasons].ruleNumber = TextExtraTimeReasons.m_ExtraTimeNumber;
//
//  Extra Time text
//
      strcpy(m_ExtraTimeReasons[m_numExtraTimeReasons].szText, TextExtraTimeReasons.m_ExtraTimeText);
//
//  Extra Time payroll code
//
      strcpy(m_ExtraTimeReasons[m_numExtraTimeReasons].szPayrollCode, TextExtraTimeReasons.m_ExtraTimePayrollCode);
//
//  Get the next record
//
      TextExtraTimeReasons.MoveNext();
      m_numExtraTimeReasons++;
    }
  }
  TextExtraTimeReasons.Close( );
//
//  **************
//  Other Comments
//  **************
//
  TextOtherComments.Open();
//
//  Set up the other comments
//
  m_numOtherComments = 0;
  if(TextOtherComments.GetRecordCount() > 0)
  {
    TextOtherComments.MoveFirst();
    while(!TextOtherComments.IsEOF())
    {
//
//  Number
//
      m_OtherComments[m_numOtherComments].number = TextOtherComments.m_OtherNumber;
//
//  Text
//
      strcpy(m_OtherComments[m_numOtherComments].szText, TextOtherComments.m_OtherText);
//
//  Get the next record
//
      TextOtherComments.MoveNext();
      m_numOtherComments++;
    }
  }
  TextOtherComments.Close( );
//
//  ***************
//  Passenger Loads
//  ***************
//
  TextPassengerLoads.Open();
//
//  Set up the passenger loads
//
  m_numPassengerLoads = 0;
  if(TextPassengerLoads.GetRecordCount() > 0)
  {
    TextPassengerLoads.MoveFirst();
    while(!TextPassengerLoads.IsEOF())
    {
//
//  Number
//
      m_PassengerLoads[m_numPassengerLoads].number = TextPassengerLoads.m_LoadNumber;
//
//  Text
//
      strcpy(m_PassengerLoads[m_numPassengerLoads].szText, TextPassengerLoads.m_LoadText);
//
//  Get the next record
//
      TextPassengerLoads.MoveNext();
      m_numPassengerLoads++;
    }
  }
  TextPassengerLoads.Close( );
//
//  **************
//  Payroll Layout
//  **************
//
  TextPayrollLayout.Open();
//
//  Set up the payroll layout
//
  m_numPayrollLayoutEntries = 0;
  if(TextPayrollLayout.GetRecordCount() > 0)
  {
    TextPayrollLayout.MoveFirst();
    while(!TextPayrollLayout.IsEOF())
    {
//
//  Index
//
      m_PayrollLayout[m_numPayrollLayoutEntries].index = m_numPayrollLayoutEntries;
//
//  Text
//
      strcpy(m_PayrollLayout[m_numPayrollLayoutEntries].szText, TextPayrollLayout.m_Field_Name);
//
//  Position
//
      m_PayrollLayout[m_numPayrollLayoutEntries].position = TextPayrollLayout.m_Position;
//
//  Get the next record
//
      TextPayrollLayout.MoveNext();
      m_numPayrollLayoutEntries++;
    }
//
//  Sort m_PayrollLayout because CListCtrl is too fucking stupid to do so on its own
//
    qsort((void *)m_PayrollLayout, m_numPayrollLayoutEntries, sizeof(PayrollLayoutDef), sort_PayrollLayout);
  }
  TextPayrollLayout.Close( );
//
//  *********
//  Referrals
//  *********
//
  TextReferrals.Open();
//
//  Set up the referrals
//
  m_numReferrals = 0;
  if(TextReferrals.GetRecordCount() > 0)
  {
    TextReferrals.MoveFirst();
    while(!TextReferrals.IsEOF())
    {
//
//  Referral name
//
      strcpy(m_Referrals[m_numReferrals].szName, TextReferrals.m_ReferralName);
//
//  Referral department
//
      strcpy(m_Referrals[m_numReferrals].szDepartment, TextReferrals.m_ReferralDepartment);
//
//  Referral email address
//
      strcpy(m_Referrals[m_numReferrals].szEmailAddress, TextReferrals.m_ReferralEmail);
//
//  Get the next record
//
      TextReferrals.MoveNext();
      m_numReferrals++;
    }
  }
  TextReferrals.Close();
//
//  ****************
//  Road Supervisors
//  ****************
//
  TextRoadSupervisors.Open();
//
//  Set up the passenger loads
//
  m_numRoadSupervisors = 0;
  if(TextRoadSupervisors.GetRecordCount() > 0)
  {
    TextRoadSupervisors.MoveFirst();
    while(!TextRoadSupervisors.IsEOF())
    {
//
//  Number
//
      m_RoadSupervisors[m_numRoadSupervisors].number = TextRoadSupervisors.m_RoadSupNumber;
//
//  Text
//
      strcpy(m_RoadSupervisors[m_numRoadSupervisors].szText, TextRoadSupervisors.m_RoadSupName);
//
//  Get the next record
//
      TextRoadSupervisors.MoveNext();
      m_numRoadSupervisors++;
    }
  }
  TextRoadSupervisors.Close( );
//
//  **********
//  Stop flags
//  **********
//
  TextStopFlags.Open();
//
//  Set up the passenger loads
//
  m_numStopFlags = 0;
  if(TextStopFlags.GetRecordCount() > 0)
  {
    TextStopFlags.MoveFirst();
    while(!TextStopFlags.IsEOF())
    {
      if(strcmp(TextStopFlags.m_StopFlagText, "*Reserved*") != 0)
      {
//
//  Number
//
        m_StopFlags[m_numStopFlags].number = TextStopFlags.m_StopFlagNumber;
//
//  Text
//
        strcpy(m_StopFlags[m_numStopFlags].szText, TextStopFlags.m_StopFlagText);
        m_numStopFlags++;
      }
//
//  Get the next record
//
      TextStopFlags.MoveNext();
    }
  }
  TextStopFlags.Close( );
//
//  ***********
//  Supervisors
//  ***********
//
  TextSupervisor.Open();
//
//  Set up the supervisors
//
  m_numSupervisors = 0;
  if(TextSupervisor.GetRecordCount() > 0)
  {
    TextSupervisor.MoveFirst();
    while(!TextSupervisor.IsEOF())
    {
//
//  Supervisor number
//
      m_Supervisors[m_numSupervisors].number = TextSupervisor.m_SupervisorNumber;
//
//  Supervisor name
//
      strcpy(m_Supervisors[m_numSupervisors].szName, TextSupervisor.m_SupervisorName);
//
//  Supervisor password
//
      strcpy(m_Supervisors[m_numSupervisors].szPassword, TextSupervisor.m_SupervisorPassword);
//
//  Get the next record
//
      TextSupervisor.MoveNext();
      m_numSupervisors++;
    }
  }
  TextSupervisor.Close( );
//
//  ******************
//  Traffic Conditions
//  ******************
//
  TextTrafficConditions.Open();
//
//  Set up the traffic conditions
//
  m_numTrafficConditions = 0;
  if(TextTrafficConditions.GetRecordCount() > 0)
  {
    TextTrafficConditions.MoveFirst();
    while(!TextTrafficConditions.IsEOF())
    {
//
//  Number
//
      m_TrafficConditions[m_numTrafficConditions].number = TextTrafficConditions.m_TrafficNumber;
//
//  Text
//
      strcpy(m_TrafficConditions[m_numTrafficConditions].szText, TextTrafficConditions.m_TrafficText);
//
//  Get the next record
//
      TextTrafficConditions.MoveNext();
      m_numTrafficConditions++;
    }
  }
  TextTrafficConditions.Close( );
//
//  *******
//  Typical
//  *******
//
  TextTypical.Open();
//
//  Set up the typical complaints
//
  m_numTypical = 0;
  if(TextTypical.GetRecordCount() > 0)
  {
    TextTypical.MoveFirst();
    while(!TextTypical.IsEOF())
    {
//
//  Number
//
      m_Typical[m_numTypical].number = TextTypical.m_TypicalNumber;
//
//  Text
//
      strcpy(m_Typical[m_numTypical].szText, TextTypical.m_TypicalText);
//
//  Get the next record
//
      TextTypical.MoveNext();
      m_numTypical++;
    }
  }
  TextTypical.Close( );
//
//  **********
//  Violations
//  **********
//
  TextViolations.Open();
//
//  Set up the violations
//
  int nI;
  int previousCategory;
  int category;
  int index;

  for(nI = 0; nI < MAXVIOLATIONCATEGORIES; nI++)
  {
    m_numViolations[nI] = NO_RECORD;
  }
  if(TextViolations.GetRecordCount() > 0)
  {
    previousCategory = NO_RECORD;
    TextViolations.MoveFirst();
    index = 0;
    while(!TextViolations.IsEOF())
    {
//
//  Parse the first part of the string
//
//  Category
//
      if(TextViolations.m_ViolationCategoryIndex != previousCategory)
      {
        previousCategory = TextViolations.m_ViolationCategoryIndex;
        index = 0;
        m_numViolations[TextViolations.m_ViolationCategoryIndex] = 1;
      }
      else
      {
        m_numViolations[TextViolations.m_ViolationCategoryIndex]++;
      }
      category = TextViolations.m_ViolationCategoryIndex;
//
//  Duration
//
      m_Violations[category][index].duration = TextViolations.m_ViolationDuration;
//
//  Absence flag
//
      m_Violations[category][index].flags =
            (TextViolations.m_ViolationAbsenceRelated == "Y" ? DISCIPLINE_FLAG_ABSENCERELATED : 0);
//
//  Text
//
      strcpy(m_Violations[category][index].text, TextViolations.m_ViolationText);
//
//  Set up the action pointers
//
      char *token;
   
      strcpy(tempString, TextViolations.m_ViolationActionsIndexes);
      token = strtok(tempString, ",\n");
      m_Violations[category][index].numActions = 0;
      while(token != NULL)
      {
        m_Violations[category][index].actions[m_Violations[category][index].numActions] = atoi(token);
        m_Violations[category][index].numActions++;
        token = strtok(NULL, ",\n");
      }
      index++;
      TextViolations.MoveNext();
    }
    m_numViolationCategories = category;
  }
  TextViolations.Close();
//
//  Set up the Violation Actions
//
  TextViolationActions.Open();
  m_numViolationActions = 0;
  if(TextViolationActions.GetRecordCount() > 0)
  {
    TextViolationActions.MoveFirst();
    while(!TextViolationActions.IsEOF())
    {
//
//  Text
//
      strcpy(m_ViolationActions[m_numViolationActions], TextViolationActions.m_ActionText);
//
//  Get the next record
//
      TextViolationActions.MoveNext();
      m_numViolationActions++;
    }
  }
  TextViolationActions.Close( );
//
//  Set up the Violation Categories
//
  TextViolationCategories.Open();
  m_numViolationCategories = 0;
  if(TextViolationCategories.GetRecordCount() > 0)
  {
    TextViolationCategories.MoveFirst();
    while(!TextViolationCategories.IsEOF())
    {
//
//  Text
//
      strcpy(m_ViolationCategories[m_numViolationCategories], TextViolationCategories.m_CategoryText);
//
//  Get the next record
//
      TextViolationCategories.MoveNext();
      m_numViolationCategories++;
    }
  }
  TextViolationCategories.Close( );
//
//  ******************
//  Weather Conditions
//  ******************
//
  TextWeatherConditions.Open();
//
//  Set up the traffic conditions
//
  m_numWeatherConditions = 0;
  if(TextWeatherConditions.GetRecordCount() > 0)
  {
    TextWeatherConditions.MoveFirst();
    while(!TextWeatherConditions.IsEOF())
    {
//
//  Number
//
      m_WeatherConditions[m_numWeatherConditions].number = TextWeatherConditions.m_WeatherNumber;
//
//  Text
//
      strcpy(m_WeatherConditions[m_numWeatherConditions].szText, TextWeatherConditions.m_WeatherText);
//
//  Get the next record
//
      TextWeatherConditions.MoveNext();
      m_numWeatherConditions++;
    }
  }
  TextWeatherConditions.Close( );
//
//  All done
//
  db.Close();

  return(TRUE);

}