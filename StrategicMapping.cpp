#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
#include <mapi.h>
typedef ULONG (*LPFNMAPISENDMAIL)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
}  // EXTERN C

#include "tms.h"
#include "wininet.h"
#import "msxml3.dll"
using namespace MSXML2;

void SendEMail(char *, char *);
void SendDebugEMail(char *, char *);

//#define DEBUGEMAIL

void SMAddAssignment(long DAILYOPSrecordID, long DRIVERSrecordID, long RUNSrecordID,
      long pieceNumber, long runNumber, long BUSESrecordID, char *pszDateTime)
{
  BOOL bGotError = FALSE;

  if(InternetCheckConnection("http://www.mapstrat.com", FLAG_ICC_FORCE_CONNECTION, 0) == 0)
  {
    MessageBox(NULL, "No internet connection available\n\nThis assignment will not be\npassed along to Strategic Mapping",
          TMS, MB_OK | MB_ICONSTOP);
    bGotError = TRUE;
  }

  char szFmtString[2048];

//
//  Create the surrounding XML
//
  strcpy(szFmtString, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
  strcat(szFmtString, "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n");
  strcat(szFmtString, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
  strcat(szFmtString, " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\r\n");
  strcat(szFmtString, "<soap:Header>");
  strcat(szFmtString, "<AuthHeader xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<Username>smasters</Username>\r\n");
  strcat(szFmtString, "<Password>w1q5h9k4</Password>\r\n");
  strcat(szFmtString, "</AuthHeader>\r\n");
  strcat(szFmtString, "</soap:Header>\r\n");
  strcat(szFmtString, "<soap:Body>\r\n");
  strcat(szFmtString, "<AddAssignment xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<assignment>\r\n");
  strcat(szFmtString, "<AssignmentCode>%ld-%ld</AssignmentCode\r\n>");
  strcat(szFmtString, "<RunCode>%ld</RunCode>\r\n");
  strcat(szFmtString, "<PieceNumber>%ld</PieceNumber>\r\n");
  strcat(szFmtString, "<DriverCode>%ld</DriverCode>\r\n");
  strcat(szFmtString, "<VehicleCode>%ld</VehicleCode>\r\n");
  strcat(szFmtString, "<StartTime>%s</StartTime>\r\n");
  strcat(szFmtString, "</assignment>\r\n");
  strcat(szFmtString, "</AddAssignment>\r\n");
  strcat(szFmtString, "</soap:Body>\r\n");
  strcat(szFmtString, "</soap:Envelope>\r\n");
//
//  Write the passed values into the XML string
//
  CString sXMLString;

  sXMLString.Format(szFmtString, effectiveDate, DAILYOPSrecordID,
        runNumber, pieceNumber, DRIVERSrecordID, BUSESrecordID, pszDateTime);
  
  HANDLE hOutputFile = CreateFile("XML Test.xml", GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  char  szBigString[2048];
  DWORD dwBytesWritten;

  strcpy(szBigString, sXMLString);
  WriteFile(hOutputFile, (LPCVOID *)szBigString, strlen(szBigString), &dwBytesWritten, NULL);
//
//  If there was an error, send an email and leave
//
  if(bGotError)
  {
    SendEMail("Error on Add Assignment", szBigString);
    return;
  }
//
//  Initialize the request handler
//
  IXMLHTTPRequestPtr pIXMLHTTPRequest = NULL;
  BSTR bstrString = NULL;
  HRESULT hr;

  hr = pIXMLHTTPRequest.CreateInstance("Msxml2.XMLHTTP.3.0");
//
//  Open the connection with a POST
//
  VARIANT vAsync;
  vAsync.vt = VT_BOOL;
  vAsync.boolVal = FALSE;
	
  VARIANT vUser;
  vUser.vt = VT_BSTR;
  vUser.bstrVal = NULL;

  VARIANT vPassword;
  vPassword.vt = VT_BSTR;
  vPassword.bstrVal = NULL;

  pIXMLHTTPRequest->open("POST ", "http://dash.mapstrat.com/DataPartnerService/Assignment.asmx", vAsync, "smasters", "w1q5h9k4");
//
//  Send the headers
//
//  Host
//
  pIXMLHTTPRequest->setRequestHeader("Host", "dash.mapstrat.com");
//
//  Content type
//
  pIXMLHTTPRequest->setRequestHeader("Content-Type", "text/xml; charset=utf-8");
//
//  Content length
//
  CString x;

  x.Format("%d", sXMLString.GetLength());
  pIXMLHTTPRequest->setRequestHeader("Content-Length", x.AllocSysString());
//
//  SOAPAction
//
  pIXMLHTTPRequest->setRequestHeader("SOAPAction", "\"http://tempuri.org/AddAssignment\"");
//
//  Send the XML string
//
  VARIANT vRequest;
  vRequest.vt = VT_BSTR;
  vRequest.bstrVal = sXMLString.AllocSysString();

  pIXMLHTTPRequest->send(vRequest);
//
//  Get the response and status
//
  long    lHttpStatus;
  CString sHttpStatusText;

  lHttpStatus = pIXMLHTTPRequest->status;
  sHttpStatusText = (char *)pIXMLHTTPRequest->statusText;

  _bstr_t bsResponse = pIXMLHTTPRequest->responseText;

   char* resultString = _com_util::ConvertBSTRToString(bsResponse);


  WriteFile(hOutputFile, (LPCVOID *)resultString, strlen(resultString), &dwBytesWritten, NULL);
  CloseHandle(hOutputFile);

#ifdef DEBUGEMAIL
  SendDebugEMail("Add Assignment Processed", szBigString);
#endif
}

void SMDelAssignment(long DAILYOPSrecordID)
{
  BOOL bGotError = FALSE;

  if(InternetCheckConnection("http://www.mapstrat.com", FLAG_ICC_FORCE_CONNECTION, 0) == 0)
  {
    MessageBox(NULL, "No internet connection available\n\nThis deassignment will not be\npassed along to Strategic Mapping",
          TMS, MB_OK | MB_ICONSTOP);
    bGotError = TRUE;
  }

  char szFmtString[2048];

//
//  Create the surrounding XML
//
  strcpy(szFmtString, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
  strcat(szFmtString, "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n");
  strcat(szFmtString, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
  strcat(szFmtString, " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\r\n");
  strcat(szFmtString, "<soap:Header>");
  strcat(szFmtString, "<AuthHeader xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<Username>smasters</Username>\r\n");
  strcat(szFmtString, "<Password>w1q5h9k4</Password>\r\n");
  strcat(szFmtString, "</AuthHeader>\r\n");
  strcat(szFmtString, "</soap:Header>\r\n");
  strcat(szFmtString, "<soap:Body>\r\n");
  strcat(szFmtString, "<CancelAssignment xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<assignmentCode>%ld-%ld</assignmentCode>\r\n");
  strcat(szFmtString, "</CancelAssignment>\r\n");
  strcat(szFmtString, "</soap:Body>\r\n");
  strcat(szFmtString, "</soap:Envelope>\r\n");
//
//  Write the passed values into the XML string
//
  CString sXMLString;

  sXMLString.Format(szFmtString, effectiveDate, DAILYOPSrecordID);
  HANDLE hOutputFile = CreateFile("XML Test.xml", GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  char  szBigString[2048];
  DWORD dwBytesWritten;
  strcpy(szBigString, sXMLString);
  WriteFile(hOutputFile, (LPCVOID *)szBigString, strlen(szBigString), &dwBytesWritten, NULL);
//
//  If there was an error, send an email and leave
//
  if(bGotError)
  {
    SendEMail("Error on Cancel Assignment", szBigString);
    return;
  }
//
//  Initialize the request handler
//
  IXMLHTTPRequestPtr pIXMLHTTPRequest = NULL;
  BSTR bstrString = NULL;
  HRESULT hr;

  hr = pIXMLHTTPRequest.CreateInstance("Msxml2.XMLHTTP.3.0");
//
//  Open the connection with a POST
//
  VARIANT vAsync;
  vAsync.vt = VT_BOOL;
  vAsync.boolVal = FALSE;
	
  VARIANT vUser;
  vUser.vt = VT_BSTR;
  vUser.bstrVal = NULL;

  VARIANT vPassword;
  vPassword.vt = VT_BSTR;
  vPassword.bstrVal = NULL;

  pIXMLHTTPRequest->open("POST ", "http://dash.mapstrat.com/DataPartnerService/Assignment.asmx", vAsync, "smasters", "w1q5h9k4");
//
//  Send the headers
//
//  Host
//
  pIXMLHTTPRequest->setRequestHeader("Host", "dash.mapstrat.com");
//
//  Content type
//
  pIXMLHTTPRequest->setRequestHeader("Content-Type", "text/xml; charset=utf-8");
//
//  Content length
//
  CString x;

  x.Format("%d", sXMLString.GetLength());
  pIXMLHTTPRequest->setRequestHeader("Content-Length", x.AllocSysString());
//
//  SOAPAction
//
  pIXMLHTTPRequest->setRequestHeader("SOAPAction", "\"http://tempuri.org/CancelAssignment\"");
//
//  Send the XML string
//
  VARIANT vRequest;
  vRequest.vt = VT_BSTR;
  vRequest.bstrVal = sXMLString.AllocSysString();

  pIXMLHTTPRequest->send(vRequest);
//
//  Get the response and status
//
  long    lHttpStatus;
  CString sHttpStatusText;

  lHttpStatus = pIXMLHTTPRequest->status;
  sHttpStatusText = (char *)pIXMLHTTPRequest->statusText;

  _bstr_t bsResponse = pIXMLHTTPRequest->responseText;

   char* resultString = _com_util::ConvertBSTRToString(bsResponse);


  WriteFile(hOutputFile, (LPCVOID *)resultString, strlen(resultString), &dwBytesWritten, NULL);
  CloseHandle(hOutputFile);

#ifdef DEBUGEMAIL
  SendDebugEMail("Delete Assignment Processed", szBigString);
#endif
}
//
//  SMAddDriver()
//
void SMAddDriver(long DRIVERSrecordID)
{
  BOOL bGotError = FALSE;

  if(InternetCheckConnection("http://www.mapstrat.com", FLAG_ICC_FORCE_CONNECTION, 0) == 0)
  {
    MessageBox(NULL, "No internet connection available\n\nThis assignment will not be\npassed along to Strategic Mapping",
          TMS, MB_OK | MB_ICONSTOP);
    bGotError = TRUE;
  }

  char szFmtString[2048];

//
//  Create the surrounding XML
//
  strcpy(szFmtString, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
  strcat(szFmtString, "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n");
  strcat(szFmtString, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
  strcat(szFmtString, " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\r\n");
  strcat(szFmtString, "<soap:Header>");
  strcat(szFmtString, "<AuthHeader xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<Username>smasters</Username>\r\n");
  strcat(szFmtString, "<Password>w1q5h9k4</Password>\r\n");
  strcat(szFmtString, "</AuthHeader>\r\n");
  strcat(szFmtString, "</soap:Header>\r\n");
  strcat(szFmtString, "<soap:Body>\r\n");
  strcat(szFmtString, "<SetDrivers xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<list>\r\n");
  strcat(szFmtString, "<UserData>\r\n");
  strcat(szFmtString, "<UserCode>%ld</UserCode\r\n>");
  strcat(szFmtString, "<UserName>%s</UserName\r\n>");
  strcat(szFmtString, "<Password>%s</Password\r\n>");
  strcat(szFmtString, "<FirstName>%s</FirstName\r\n>");
  strcat(szFmtString, "<LastName>%s</LastName\r\n>");
  strcat(szFmtString, "<IsActive>true</IsActive\r\n>");
  strcat(szFmtString, "</UserData>\r\n");
  strcat(szFmtString, "</list>\r\n");
  strcat(szFmtString, "</SetDrivers>\r\n");
  strcat(szFmtString, "</soap:Body>\r\n");
  strcat(szFmtString, "</soap:Envelope>\r\n");
//
//  Write the driver data into the XML string
//
  char szFirstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  char szLastName[DRIVERS_LASTNAME_LENGTH + 1];
  char szUsername[DRIVERS_BADGENUMBER_LENGTH + 1];
  char szPassword[DRIVERS_BADGENUMBER_LENGTH + 1];

  DRIVERSKey0.recordID = DRIVERSrecordID;
  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);

  strncpy(szFirstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  trim(szFirstName, DRIVERS_FIRSTNAME_LENGTH);

  strncpy(szLastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  trim(szLastName, DRIVERS_FIRSTNAME_LENGTH);

  strncpy(szUsername, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
  trim(szUsername, DRIVERS_BADGENUMBER_LENGTH);
  if(strcmp(szUsername, "") == 0)
  {
    strcpy(szUsername, "None");
  }

  strcpy(szPassword, szUsername);

  CString sXMLString;

  sXMLString.Format(szFmtString, DRIVERS.recordID, szUsername, szPassword, szFirstName, szLastName);
  HANDLE hOutputFile = CreateFile("XML Test.xml", GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  char  szBigString[2048];
  DWORD dwBytesWritten;

  strcpy(szBigString, sXMLString);
  WriteFile(hOutputFile, (LPCVOID *)szBigString, strlen(szBigString), &dwBytesWritten, NULL);
//
//  If there was an error, send an email and leave
//
  if(bGotError)
  {
    SendEMail("Error on Add Driver", szBigString);
    return;
  }
//
//  Initialize the request handler
//
  IXMLHTTPRequestPtr pIXMLHTTPRequest = NULL;
  BSTR bstrString = NULL;
  HRESULT hr;

  hr = pIXMLHTTPRequest.CreateInstance("Msxml2.XMLHTTP.3.0");
//
//  Open the connection with a POST
//
  VARIANT vAsync;
  vAsync.vt = VT_BOOL;
  vAsync.boolVal = FALSE;
	
  VARIANT vUser;
  vUser.vt = VT_BSTR;
  vUser.bstrVal = NULL;

  VARIANT vPassword;
  vPassword.vt = VT_BSTR;
  vPassword.bstrVal = NULL;

  pIXMLHTTPRequest->open("POST ", "http://dash.mapstrat.com/DataPartnerService/ImportService.asmx", vAsync, "smasters", "w1q5h9k4");
//
//  Send the headers
//
//  Host
//
  pIXMLHTTPRequest->setRequestHeader("Host", "dash.mapstrat.com");
//
//  Content type
//
  pIXMLHTTPRequest->setRequestHeader("Content-Type", "text/xml; charset=utf-8");
//
//  Content length
//
  CString x;

  x.Format("%d", sXMLString.GetLength());
  pIXMLHTTPRequest->setRequestHeader("Content-Length", x.AllocSysString());
//
//  SOAPAction
//
  pIXMLHTTPRequest->setRequestHeader("SOAPAction", "\"http://tempuri.org/SetDrivers\"");
//
//  Send the XML string
//
  VARIANT vRequest;
  vRequest.vt = VT_BSTR;
  vRequest.bstrVal = sXMLString.AllocSysString();

  pIXMLHTTPRequest->send(vRequest);
//
//  Get the response and status
//
  long    lHttpStatus;
  CString sHttpStatusText;

  lHttpStatus = pIXMLHTTPRequest->status;
  sHttpStatusText = (char *)pIXMLHTTPRequest->statusText;

  _bstr_t bsResponse = pIXMLHTTPRequest->responseText;

   char* resultString = _com_util::ConvertBSTRToString(bsResponse);


  WriteFile(hOutputFile, (LPCVOID *)resultString, strlen(resultString), &dwBytesWritten, NULL);
  CloseHandle(hOutputFile);

#ifdef DEBUGEMAIL
  SendDebugEMail("Add Driver Processed", szBigString);
#endif
}

//
//  SMAddBus()
//
void SMAddBus(long BUSESrecordID)
{
  BOOL bGotError = FALSE;

  if(InternetCheckConnection("http://www.mapstrat.com", FLAG_ICC_FORCE_CONNECTION, 0) == 0)
  {
    MessageBox(NULL, "No internet connection available\n\nThis assignment will not be\npassed along to Strategic Mapping",
          TMS, MB_OK | MB_ICONSTOP);
    bGotError = TRUE;
  }

  char szFmtString[2048];

//
//  Create the surrounding XML
//
  strcpy(szFmtString, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
  strcat(szFmtString, "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n");
  strcat(szFmtString, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
  strcat(szFmtString, " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\r\n");
  strcat(szFmtString, "<soap:Header>");
  strcat(szFmtString, "<AuthHeader xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<Username>smasters</Username>\r\n");
  strcat(szFmtString, "<Password>w1q5h9k4</Password>\r\n");
  strcat(szFmtString, "</AuthHeader>\r\n");
  strcat(szFmtString, "</soap:Header>\r\n");
  strcat(szFmtString, "<soap:Body>\r\n");
  strcat(szFmtString, "<SetVehicles xmlns=\"http://tempuri.org/\">\r\n");
  strcat(szFmtString, "<list>\r\n");
  strcat(szFmtString, "<VehicleData>\r\n");
  strcat(szFmtString, "<VehicleCode>%ld</VehicleCode\r\n>");
  strcat(szFmtString, "<ShortName>%s</ShortName\r\n>");
  strcat(szFmtString, "<IsActive>true</IsActive\r\n>");
  strcat(szFmtString, "</VehicleData>\r\n");
  strcat(szFmtString, "</list>\r\n");
  strcat(szFmtString, "</SetVehicles>\r\n");
  strcat(szFmtString, "</soap:Body>\r\n");
  strcat(szFmtString, "</soap:Envelope>\r\n");
//
//  Write the bus data into the XML string
//
  char szBusNumber[BUSES_NUMBER_LENGTH + 1];

  BUSESKey0.recordID = BUSESrecordID;
  btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);

  strncpy(szBusNumber, BUSES.number, BUSES_NUMBER_LENGTH);
  trim(szBusNumber, BUSES_NUMBER_LENGTH);

  CString sXMLString;

  sXMLString.Format(szFmtString, BUSES.recordID, szBusNumber);;
  HANDLE hOutputFile = CreateFile("XML Test.xml", GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  char  szBigString[2048];
  DWORD dwBytesWritten;

  strcpy(szBigString, sXMLString);
  WriteFile(hOutputFile, (LPCVOID *)szBigString, strlen(szBigString), &dwBytesWritten, NULL);
//
//  If there was an error, send an email and leave
//
  if(bGotError)
  {
    SendEMail("Error on Add Bus", szBigString);
    return;
  }
//
//  Initialize the request handler
//
  IXMLHTTPRequestPtr pIXMLHTTPRequest = NULL;
  BSTR bstrString = NULL;
  HRESULT hr;

  hr = pIXMLHTTPRequest.CreateInstance("Msxml2.XMLHTTP.3.0");
//
//  Open the connection with a POST
//
  VARIANT vAsync;
  vAsync.vt = VT_BOOL;
  vAsync.boolVal = FALSE;
	
  VARIANT vUser;
  vUser.vt = VT_BSTR;
  vUser.bstrVal = NULL;

  VARIANT vPassword;
  vPassword.vt = VT_BSTR;
  vPassword.bstrVal = NULL;

  pIXMLHTTPRequest->open("POST ", "http://dash.mapstrat.com/DataPartnerService/ImportService.asmx", vAsync, "smasters", "w1q5h9k4");
//
//  Send the headers
//
//  Host
//
  pIXMLHTTPRequest->setRequestHeader("Host", "dash.mapstrat.com");
//
//  Content type
//
  pIXMLHTTPRequest->setRequestHeader("Content-Type", "text/xml; charset=utf-8");
//
//  Content length
//
  CString x;

  x.Format("%d", sXMLString.GetLength());
  pIXMLHTTPRequest->setRequestHeader("Content-Length", x.AllocSysString());
//
//  SOAPAction
//
  pIXMLHTTPRequest->setRequestHeader("SOAPAction", "\"http://tempuri.org/SetVehicles\"");
//
//  Send the XML string
//
  VARIANT vRequest;
  vRequest.vt = VT_BSTR;
  vRequest.bstrVal = sXMLString.AllocSysString();

  pIXMLHTTPRequest->send(vRequest);
//
//  Get the response and status
//
  long    lHttpStatus;
  CString sHttpStatusText;

  lHttpStatus = pIXMLHTTPRequest->status;
  sHttpStatusText = (char *)pIXMLHTTPRequest->statusText;

  _bstr_t bsResponse = pIXMLHTTPRequest->responseText;

   char* resultString = _com_util::ConvertBSTRToString(bsResponse);


  WriteFile(hOutputFile, (LPCVOID *)resultString, strlen(resultString), &dwBytesWritten, NULL);
  CloseHandle(hOutputFile);

#ifdef DEBUGEMAIL
  SendDebugEMail("Add Bus Processed", szBigString);
#endif
}

//
//  SendEMail()
//
void SendEMail(char *pszSubjectText, char *pszBodyText)
{
  LPFNMAPISENDMAIL m_lpfnMAPISendMail;
  HINSTANCE m_hlibMAPI;

  m_hlibMAPI = AfxLoadLibrary("mapi32.dll");
  m_lpfnMAPISendMail = (LPFNMAPISENDMAIL) GetProcAddress(m_hlibMAPI, "MAPISendMail");
 
  MapiMessage message;

  memset(&message, 0x00, sizeof(MapiMessage));
  message.lpszSubject = pszSubjectText;
  message.lpszNoteText = pszBodyText;
  message.nRecipCount = 2;

  MapiRecipDesc Recipients[2];
  Recipients[0].ulRecipClass = MAPI_TO;
  Recipients[0].lpszName = "Strategic Mapping Suppor";
  Recipients[0].lpszAddress = "SMTP:supporterrors@mapstrat.com";
  Recipients[1].ulRecipClass = MAPI_CC;
  Recipients[1].lpszName = "David Reed";
  Recipients[1].lpszAddress = "SMTP:d.reed@schedule-masters.org";
  message.lpRecips = Recipients;
  message.nFileCount = 0;
  message.lpFiles = NULL;

  ULONG ulResult = m_lpfnMAPISendMail(0, 0, &message, MAPI_LOGON_UI, 0);

  AfxFreeLibrary(m_hlibMAPI);
}

//
//  SendDebugEMail()
//
void SendDebugEMail(char *pszSubjectText, char *pszBodyText)
{
  LPFNMAPISENDMAIL m_lpfnMAPISendMail;
  HINSTANCE m_hlibMAPI;

  m_hlibMAPI = AfxLoadLibrary("mapi32.dll");
  m_lpfnMAPISendMail = (LPFNMAPISENDMAIL) GetProcAddress(m_hlibMAPI, "MAPISendMail");
 
  MapiMessage message;

  memset(&message, 0x00, sizeof(MapiMessage));
  message.lpszSubject = pszSubjectText;
  message.lpszNoteText = pszBodyText;
  message.nRecipCount = 2;

  MapiRecipDesc Recipients[2];
  Recipients[0].ulRecipClass = MAPI_TO;
  Recipients[0].lpszName = "Dmytro Berbeka";
  Recipients[0].lpszAddress = "SMTP:dmytro@mapstrat.com";
  Recipients[1].ulRecipClass = MAPI_CC;
  Recipients[1].lpszName = "David Reed";
  Recipients[1].lpszAddress = "SMTP:d.reed@schedule-masters.org";
  message.lpRecips = Recipients;
  message.nFileCount = 0;
  message.lpFiles = NULL;

  ULONG ulResult = m_lpfnMAPISendMail(0, 0, &message, MAPI_LOGON_UI, 0);

  AfxFreeLibrary(m_hlibMAPI);
}