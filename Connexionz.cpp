#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

}  // EXTERN C

#include "tms.h"
#include "AVLInterface.h"
#include "wininet.h"
#import "msxml3.dll"
using namespace MSXML2;


void CNZAddAssignment(long DAILYOPSrecordID, long DRIVERSrecordID, long RUNSrecordID,
      long pieceNumber, long runNumber, long BUSESrecordID, char *pszDateTime)
{
  char szBusNumber[BUSES_NUMBER_LENGTH + 1];
  long blockNumber;
  int  rcode2;

  char szFmtString[2048];

  int year  = atol(pszDateTime);
  int month = atol(&pszDateTime[5]);
  int day   = atol(&pszDateTime[8]);

  CTime date(year, month, day, 0, 0, 0);
//  date += CTimeSpan(1, 0, 0, 0);

  CString sDate = date.Format("%Y-%m-%d");

//
//  Eliminate the time from the date string
//
  pszDateTime[10] = '\0';
//
//  Create the surrounding XML
//
  strcpy(szFmtString, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
  strcat(szFmtString, "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
  strcat(szFmtString, " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\r\n");
  strcat(szFmtString, " xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\r\n");
  strcat(szFmtString, "  <soap12:Body>\r\n");
  strcat(szFmtString, "    <AssignTrips xmlns=\"urn:connexionz-co-nz\">\r\n");
  strcat(szFmtString, "      <strCompanyID>%s</strCompanyID>\r\n");
  strcat(szFmtString, "      <dtTripDate>%s</dtTripDate>\r\n");
  strcat(szFmtString, "      <docTrip>\r\n");
  strcat(szFmtString, "        <Assignments xmlns=\"\">\r\n");
//
//  Get the bus number
//
  BUSESKey0.recordID = BUSESrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
  strncpy(szBusNumber, BUSES.number, BUSES_NUMBER_LENGTH);
  trim(szBusNumber, BUSES_NUMBER_LENGTH);
//
//  Get the trip numbers
//
  RUNSKey0.recordID = RUNSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
  blockNumber = TRIPS.standard.blockNumber;
  while(rcode2 == 0 &&
        TRIPS.standard.blockNumber == blockNumber)
  {
    sprintf(tempString, "            <Assignment TripNo=\"%ld\" VehicleID=\"%s\" />\r\n", TRIPS.tripNumber, szBusNumber);
    strcat(szFmtString, tempString);
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
  }
//  strcat(szFmtString, "<Assignment TripNo=\"540\" VhicleID=\"014\" />\r\n");
//
//  Add in the rest of the XML
//
  strcat(szFmtString, "        </Assignments>\r\n");
  strcat(szFmtString, "      </docTrip>\r\n");
  strcat(szFmtString, "    </AssignTrips>\r\n");
  strcat(szFmtString, "  </soap12:Body>\r\n");
  strcat(szFmtString, "</soap12:Envelope>\r\n");
//
//  Write the passed values into the XML string
//
  CString sXMLString;

  sXMLString.Format(szFmtString, "MVT", sDate);
  
  HANDLE hOutputFile = CreateFile("XML Test.xml", GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  char  szBigString[2048];
  DWORD dwBytesWritten;

  strcpy(szBigString, sXMLString);
  WriteFile(hOutputFile, (LPCVOID *)szBigString, strlen(szBigString), &dwBytesWritten, NULL);
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

  pIXMLHTTPRequest->open("POST ", szPostAddress, vAsync, "", "");
//
//  Send the headers
//
//  Host
//
  pIXMLHTTPRequest->setRequestHeader("Host", szHostAddress);
//
//  Content type
//
  pIXMLHTTPRequest->setRequestHeader("Content-Type", "application/soap+xml; charset=utf-8");
//
//  Content length
//
  CString x;

  x.Format("%d", sXMLString.GetLength());
  pIXMLHTTPRequest->setRequestHeader("Content-Length", x.AllocSysString());
//
//  SOAPAction
//
//  pIXMLHTTPRequest->setRequestHeader("SOAPAction", "\"urn:connexionz-co-nz/AssignTrips\"");
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

}

void CNZDelAssignment(long DAILYOPSrecordID)
{
}

void CNZAddDriver(long DRIVERSrecordID)
{
}


void CNZAddBus(long BUSESrecordID)
{
}
