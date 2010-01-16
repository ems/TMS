// DisciplineLetter.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "AddDialogs.h"
#include "AddDiscipline.h"
#include "Discipline.h"
#include "DisciplineLetter.h"
#include "DisciplineLetterTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisciplineLetter dialog


CDisciplineLetter::CDisciplineLetter(CWnd* pParent, long* pDRIVERSrecordID)
	: CDialog(CDisciplineLetter::IDD, pParent)
{
  m_pDRIVERSrecordID = pDRIVERSrecordID;

	//{{AFX_DATA_INIT(CDisciplineLetter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDisciplineLetter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisciplineLetter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisciplineLetter, CDialog)
	//{{AFX_MSG_MAP(CDisciplineLetter)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDTEMPLATE, OnTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisciplineLetter message handlers

BOOL CDisciplineLetter::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pEditTEXT = (CEdit *)GetDlgItem(LETTERPREVIEW_TEXT);
//
//  Get the driver data
//
  DRIVERSKey0.recordID = *m_pDRIVERSrecordID;

  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);

  CString s;
//
//  First and last name
//
  s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  strcpy(tempString, s);
  strcat(tempString, " ");
  s = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  strcat(tempString, s);
  strcat(tempString, "\r\n");
//
//  Address
//
  s = GetDatabaseString(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
  strcat(tempString, s);
  strcat(tempString, "\r\n");
//
//  City and State
//
  s = GetDatabaseString(DRIVERS.city, DRIVERS_CITY_LENGTH);
  strcat(tempString, s);
  strcat(tempString, ", ");
  s = GetDatabaseString(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
  strcat(tempString, s);
  strcat(tempString, "\r\n");
//
//  Zip
//
  s = GetDatabaseString(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
  strcat(tempString, s);
  strcat(tempString, "\r\n\r\n");
//
//  Date
//
  CTime time = CTime::GetCurrentTime();
 	CString strDate = time.Format(_T("%B %d, %Y"));

  strcat(tempString, strDate);
  strcat(tempString, "\r\n\r\n");
//
//  Dear whomever
//
  strcat(tempString, "Dear ");
  s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  strcat(tempString, s);
  strcat(tempString, ",\r\n\r\n");
//
//  Display it
//
  pEditTEXT->SetWindowText(tempString);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisciplineLetter::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDisciplineLetter::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


void CDisciplineLetter::OnTemplate() 
{
  char szFile[MAX_PATH];

  CDisciplineLetterTemplate dlg(this, &szFile[0]);

  if(dlg.DoModal() == IDOK)
  {
//
//  Open the text file
//
    CString    inputLine, outputLine;
    CString    fileName;
    CStdioFile Letter;
  
    fileName = szDisciplineLetterTemplateFolder;
    fileName += "\\";
    fileName += szFile;
    Letter.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);

    if(Letter.m_pStream)
    {
      Letter.SeekToBegin();
      pEditTEXT->GetWindowText(outputLine);
      while(Letter.ReadString(inputLine))
      {
        outputLine += inputLine;
        outputLine += "\r\n";
      }
      pEditTEXT->SetWindowText(outputLine);
      Letter.Close();
    }
  }
}

void CDisciplineLetter::OnOK() 
{
  char   outputString[512];
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  char   szReportName[TMSRPT_REPORTNAME_LENGTH + 1];
  char   szReportFileName[TMSRPT_REPORTFILENAME_LENGTH + 1];
  int    nI, nJ;
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT58.txt");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(szarString, TMS, MB_ICONSTOP);
    OnCancel();
  }
//
//  Cycle through the edit control
//
  CString strText;
  int len;
  int nLineCount = pEditTEXT->GetLineCount();
   
  for(nI = 0; nI < nLineCount; nI++)
  {
    len = pEditTEXT->LineLength(pEditTEXT->LineIndex(nI));
    pEditTEXT->GetLine(nI, strText.GetBuffer(len), len);
    strncpy(tempString, strText, len);
    tempString[len] = '\0';
    strText.ReleaseBuffer(len);
    sprintf(outputString, "%d\t\"%s\"\r\n", nI, tempString);
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
  }
  CloseHandle(hOutputFile);

//
//  Fire the report
//
  FILE  *fp;
  DWORD  type = REG_SZ;
  HKEY hKey;
  long result;
  int  thisReport;

  CString s;
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS MSJet Text";
//
//  Make sure the template exists (Report 57)
//
  for(thisReport = NO_RECORD, nI = 0; nI < TMSRPT_MAX_REPORTSDEFINED; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == 57)
    {
      thisReport = nI;
      break;
    }
  }
  if(thisReport == NO_RECORD || 
        strcmp(TMSRPT[thisReport].szReportFileName[0], "") == 0)
  {
    fp = NULL;
  }
  else
  {
    fp = fopen(TMSRPT[thisReport].szReportFileName[0], "r");
  }
  if(fp == NULL)
  {
    s.LoadString(ERROR_197);
    sprintf(tempString, s, TMSRPT[thisReport].szReportFileName[0]);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK);
  }
//
//  It does...
//
  else
  {

    strcpy(szReportName, TMSRPT[thisReport].szReportName);
    strcpy(szReportFileName, TMSRPT[thisReport].szReportFileName[0]);
    fclose(fp);
    result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
    if(result != 0)
    {
      LoadString(hInst, ERROR_240, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, sRegKey);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_OK);
    }
//
//  Set the registry entries so Crystal knows where to look
//
    else
    {
      strcpy(szarString, szReportsTempFolder);
      RegSetValueEx(hKey, "DefaultDir", 0, type, (LPBYTE)szarString, strlen(szarString));
//
//  Copy the Schema.INI file from the install directory into the temporary report folder.
//  When we do this each time, we ensure that he's getting the most recent version
//
      char fileName[2][MAX_PATH];

      for(nJ = 0; nJ < 2; nJ++)
      {
        strcpy(fileName[nJ], nJ == 0 ? szInstallDirectory : szarString);
        strcat(fileName[nJ], "\\Schema.INI");
      }
      if(GetFileAttributes(fileName[0]) == 0xFFFFFFFF)  // Couldn't find INI file in szInstallDirectory
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_218, (HANDLE)NULL);
      }
//
//  Found the INI file
//
      else
      {
        CopyFile(fileName[0], fileName[1], FALSE);

        CCrystal dlg(this, szReportFileName, szReportName);

        dlg.DoModal();
      }
    }
  }
	
	CDialog::OnOK();
}
