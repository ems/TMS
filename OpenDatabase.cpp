// OpenDatabase.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include "DTPicker.h"
#include "OpenDatabase.h"
#include "RosterOrder.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int DatabaseOpen(void)
{
  COpenDatabase dlg;

  return(dlg.DoModal());
}

extern "C" {
  int Database(void)
  {
    return(DatabaseOpen());
  }
}

#define OLDFILE 0
#define NEWFILE 1

//
//  Table names and view names
//
char *tableNames[] = {TABLENAME_FILE,          TABLENAME_FIELD,        TABLENAME_INDEX,
                      TABLENAME_ATTRIBUTES,    TABLENAME_CREATED,      TABLENAME_TABLEVIEW,
                      TABLENAME_DIRECTIONS,    TABLENAME_ROUTES,       TABLENAME_SERVICES,
                      TABLENAME_JURISDICTIONS, TABLENAME_NODES,        TABLENAME_DIVISIONS,
                      TABLENAME_PATTERNS,      TABLENAME_PATTERNNAMES, TABLENAME_CONNECTIONS,
                      TABLENAME_BUSTYPES,      TABLENAME_COMMENTS,     TABLENAME_TRIPS,
                      TABLENAME_RUNS,          TABLENAME_ROSTER,       TABLENAME_DRIVERS,
                      TABLENAME_DRIVERTYPES,   TABLENAME_BLOCKNAMES,   TABLENAME_CUSTOMERS,
                      TABLENAME_COMPLAINTS,    TABLENAME_ROUTINGS,     TABLENAME_SIGNCODES,
                      TABLENAME_TIMECHECKS,    TABLENAME_BUSES,        TABLENAME_DISCIPLINE,
                      TABLENAME_CREWONLY,      TABLENAME_DAILYOPS,     TABLENAME_OFFTIME,
                      TABLENAME_RIDERSHIP};
char *viewNames[] = { VIEWNAME_STANDARDBLOCKS, VIEWNAME_DROPBACKBLOCKS,
                      VIEWNAME_RUNS,           VIEWNAME_ROSTER,        VIEWNAME_TIMECHECKS};

/////////////////////////////////////////////////////////////////////////////
// COpenDatabase dialog


COpenDatabase::COpenDatabase(CWnd* pParent /*=NULL*/)
	: CDialog(COpenDatabase::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenDatabase)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COpenDatabase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenDatabase)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenDatabase, CDialog)
	//{{AFX_MSG_MAP(COpenDatabase)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenDatabase message handlers

BOOL COpenDatabase::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up pointers to the controls
//
  pStaticFILENAME = (CStatic *)GetDlgItem(OPENDATABASE_FILENAME);
  pEditLONGDESCRIPTION = (CEdit *)GetDlgItem(OPENDATABASE_LONGDESCRIPTION);
  pEditSHORTDESCRIPTION = (CEdit *)GetDlgItem(OPENDATABASE_SHORTDESCRIPTION);
  pDTPickerEFFECTIVEDATE = (CDTPicker *)GetDlgItem(OPENDATABASE_EFFECTIVEDATE);
  pDTPickerUNTILDATE = (CDTPicker *)GetDlgItem(OPENDATABASE_UNTILDATE);
  pStaticLASTUPDATEDATE = (CStatic *)GetDlgItem(OPENDATABASE_LASTUPDATEDATE);
  pStaticLASTUPDATEDBY = (CStatic *)GetDlgItem(OPENDATABASE_LASTUPDATEDBY);
  pButtonLL = (CButton *)GetDlgItem(OPENDATABASE_LL);
  pButtonEN = (CButton *)GetDlgItem(OPENDATABASE_EN);
  pStaticMESSAGE = (CStatic *)GetDlgItem(OPENDATABASE_MESSAGE);
//
//  Find out who this is by looking for UserName (under [network])
//  in SYSTEM.INI, working under the assumption that this is a
//  Windows for Workgroups environment.
//
  pStaticMESSAGE->SetWindowText("Establishing Internals");
  GetPrivateProfileString("Network", "UserName", "", userName, sizeof(userName), "SYSTEM.INI");
//
//  Did we get it?  If not, look in TMS.INI for it
//
  if(strcmp(userName, "") == 0)
  {
    GetPrivateProfileString("TMS", "UserName", "", userName, sizeof(userName), "TMS.INI");
  }
//
//  Was it there?  If not, get him to enter it.
//
  if(strcmp(userName, "") == 0)
  {
    if(!DialogBox(hInst, MAKEINTRESOURCE(IDD_USERNAME), m_hWnd, (DLGPROC)USERNAMEMsgProc))
    {
      OnCancel();
      return TRUE;
    }
  }
//
//  File name
//
  pStaticFILENAME->SetWindowText(szDatabaseFileName);
//
//  Now let's look in the selected .TMS file for the rest of the stuff
//
  long year, month, day;
//
//  Effective Date
//
  GetPrivateProfileString("Description", "EffectiveDate",
        "", szEffectiveDate, sizeof(szEffectiveDate), (LPSTR)szDatabaseFileName);
  effectiveDate = GetPrivateProfileInt("Description", "EffectiveDateAsNumber", 0, (LPSTR)szDatabaseFileName);
//
//  Check on alphas for backward compatibilty
//
  CString s;
  BOOL bWarnedOnce = FALSE;

  if(effectiveDate != 0)
  {
    GetYMD(effectiveDate, &year, &month, &day);
  }
  else
  {
    CTime time = CTime::GetCurrentTime();
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();
    MessageBeep(MB_ICONEXCLAMATION);
    s.LoadString(TEXT_288);
    MessageBox(s, TMS, MB_OK | MB_ICONEXCLAMATION);
    bWarnedOnce = TRUE;
    effectiveDate = year * 10000 + month * 100 * day;
  }
//
//  Set the date on the control
//
  COleVariant v;

  v = year;
  pDTPickerEFFECTIVEDATE->SetYear(v);
  v = month;
  pDTPickerEFFECTIVEDATE->SetMonth(v);
  v = day;
  pDTPickerEFFECTIVEDATE->SetDay(v);
//
//  Until Date
//
  GetPrivateProfileString("Description", "UntilDate",
        "", szUntilDate, sizeof(szUntilDate), (LPSTR)szDatabaseFileName);
  untilDate = GetPrivateProfileInt("Description", "UntilDateAsNumber", 0, (LPSTR)szDatabaseFileName);
//
//  Check on alphas for backward compatibilty
//
  if(untilDate != 0)
  {
    GetYMD(untilDate, &year, &month, &day);
  }
  else
  {
    CTime time = CTime::GetCurrentTime();
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();
    if(!bWarnedOnce)
    {
      MessageBeep(MB_ICONEXCLAMATION);
      s.LoadString(TEXT_288);
      MessageBox(s, TMS, MB_OK | MB_ICONEXCLAMATION);
    }
    untilDate = year * 10000 + month * 100 + day;
  }
//
//  Set the date on the control
//
  v = year;
  pDTPickerUNTILDATE->SetYear(v);
  v = month;
  pDTPickerUNTILDATE->SetMonth(v);
  v = day;
  pDTPickerUNTILDATE->SetDay(v);
//
//  Last access date
//
  GetPrivateProfileString("Description", "LastAccessDate",
        "", tempString, sizeof(tempString), (LPSTR)szDatabaseFileName);
  pStaticLASTUPDATEDATE->SetWindowText(tempString);
//
//  Last updated by
//
  GetPrivateProfileString("Description", "LastUpdatedBy",
        "", tempString, sizeof(tempString), (LPSTR)szDatabaseFileName);
  pStaticLASTUPDATEDBY->SetWindowText(tempString);
//
//  Long description
//
  GetPrivateProfileString("Description", "LongDescription", "",
        szDatabaseDescription, sizeof(szDatabaseDescription), (LPSTR)szDatabaseFileName);
  pEditLONGDESCRIPTION->SetWindowText(szDatabaseDescription);
//
//  Short description
//
  GetPrivateProfileString("Description", "ShortDescription", "",
        szShortDatabaseDescription, sizeof(szShortDatabaseDescription), (LPSTR)szDatabaseFileName);
  pEditSHORTDESCRIPTION->SetWindowText(szShortDatabaseDescription);
//
//  Mapping mode in effect
//
  if(m_bUseLatLong)
  {
    pButtonLL->SetCheck(TRUE);
  }
  else
  {
    pButtonEN->SetCheck(TRUE);
  }
//
//  Set up the common file area flag.  This tells the
//  rest of the file open procedure where everything is.
//
//  Database location flags
//
  m_CommonFlag  = 0;
  m_CommonFlag |= (1L << FILE_DDF);   
  m_CommonFlag |= (1L << FIELD_DDF);  
  m_CommonFlag |= (1L << INDEX_DDF);  
  m_CommonFlag |= (1L << TMS_CUSTOMERS);   
  m_CommonFlag |= (1L << TMS_COMPLAINTS);
  m_CommonFlag |= (1L << TMS_CREATED);
  m_CommonFlag |= (1L << TMS_ATTRIBUTES);
  m_CommonFlag |= (1L << TMS_TABLEVIEW);
  m_CommonFlag |= (1L << TMS_BUSTYPES);
  m_CommonFlag |= (1L << TMS_BUSES);
  m_CommonFlag |= (1L << TMS_DRIVERTYPES);
  m_CommonFlag |= (1L << TMS_DRIVERS);
  m_CommonFlag |= (1L << TMS_DISCIPLINE);
  m_CommonFlag |= (1L << TMS_DAILYOPS);
  m_CommonFlag |= (1L << TMS_OFFTIME);
	
  pStaticMESSAGE->SetWindowText("Internals Established");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpenDatabase::OnClose() 
{
  OnCancel();
}

void COpenDatabase::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void COpenDatabase::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void COpenDatabase::OnOK() 
{
  HCURSOR hSaveCursor = SetCursor(hCursorWait);
  struct  _stat  statBuf;
  CString s;
  GenerateTripDef GTResults;
  BTI_WORD dataBufferLength[2];
  char dummy[256];
  char szDir[_MAX_PATH];
  char fromFile[257];
  char toFile[257];
  BOOL bAbort;
  BOOL bCreatedTRIPSB60;
  int  nI;
  int  nJ;
  int  nK;
  int  rcode2;
  int  numRecords;

  bCreatedTRIPSB60 = FALSE;
//
//  We chdir'd over here in file.cpp.  Get the current directory
//  so that we can build up the full path name of the btrieve files.
//
  _getcwd(szDir, sizeof(szDir));
//
//  All of the file names are in FILE.DDF.  This file
//  is in the "Common" folder off of the szDatabaseRoot.
//
  pStaticMESSAGE->SetWindowText("Opening FILE.DDF");
  strcpy(tempString, szDatabaseRoot);
  strcat(tempString, "\\Common\\FILE.DDF");
  rcode2 = btrieve(B_OPEN, FILE_DDF, dummy, tempString, 0);
  if(rcode2 != 0)
  {
    SetCursor(hSaveCursor);
    if(rcode2 == 20)
    {
      LoadString(hInst, ERROR_289, szarString, sizeof(szarString));
    }
    else
    {
      LoadString(hInst, ERROR_129, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString, rcode2);
    }
    MessageBeep(MB_ICONSTOP);
    MessageBox(szarString, TMS, MB_ICONSTOP);
    btrieve(B_CLOSE, FILE_DDF, NULL, NULL, (int)NULL);
    OnCancel();
    return;
  }
//
//  Ok, we got it.  Build up a list of DDF file numbers
//  for use in the column routines.  This step also verifies
//  the presence of each of the tables in the dictionary.
//
  pStaticMESSAGE->SetWindowText("Reading FILE.DDF...");
  for(rcode2 = 0, nI = 0; nI <= TMS_LASTFILE; nI++)
  {
    strcpy(FILEDDFKey1.Name, tableNames[nI]);
    pad(FILEDDFKey1.Name, DDF_NAME_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
    if(rcode2 == 0)
    {
      strncpy(szDatabase[nI], FILEDDF.Loc, DDF_LOC_LENGTH);
      trim(szDatabase[nI], DDF_LOC_LENGTH);
      DDFFileNumbers[nI] = FILEDDF.Id;
    }
    else
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_050, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, tableNames[nI], rcode2);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
    sprintf(tempString, "Keying %s", szDatabase[nI]);
    pStaticMESSAGE->SetWindowText(tempString);
  }
  pStaticMESSAGE->SetWindowText("Closing FILE.DDF");
  btrieve(B_CLOSE, FILE_DDF, NULL, NULL, (int)NULL);
  pStaticMESSAGE->SetWindowText("FILE.DDF Closed");
//
//  Check on the old style vs. new style NODES (NODES.B60) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in NODES.DAT.  Check, though, the
//  location of NODES.B60 - it might be (but shouldn't be) in the Common folder.
//
  pStaticMESSAGE->SetWindowText("Checking NODES.B60");
  strcpy(szDatabase[TMS_NODES], "NODES.B60");
  if(m_CommonFlag & (1 << TMS_NODES))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_NODES]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_NODES]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_NODES]);
//
//  If NODES.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_NODES], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style NODES table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    btrieve(B_OPEN, OLDFILE, dummy, "NODES.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLDNODES);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(NODES);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &OLDNODES, &dataBufferLength[OLDFILE],  &OLDNODESKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      NODES.recordID = OLDNODES.recordID;
      NODES.COMMENTSrecordID = OLDNODES.COMMENTSrecordID;
      NODES.JURISDICTIONSrecordID = OLDNODES.JURISDICTIONSrecordID;
      memcpy(&NODES.abbrName, &OLDNODES.abbrName, NODES_ABBRNAME_LENGTH);
      memcpy(&NODES.longName, &OLDNODES.longName, NODES_LONGNAME_LENGTH);
      memcpy(&NODES.intersection, &OLDNODES.intersection, NODES_INTERSECTION_LENGTH);
      memcpy(&NODES.description, &OLDNODES.intersection, NODES_INTERSECTION_LENGTH);
      memset(&NODES.reliefLabels, ' ', NODES_RELIEFLABELS_LENGTH);
      memcpy(&NODES.reliefLabels, &OLDNODES.reliefLabels, OLDNODES_RELIEFLABELS_LENGTH);
      memcpy(&NODES.longitude, &OLDNODES.longitude, 4);
      memcpy(&NODES.latitude, &OLDNODES.latitude, 4);
      NODES.number = OLDNODES.number;
      memset(&NODES.mapCodes, ' ', NODES_MAPCODES_LENGTH);
      memcpy(&NODES.mapCodes, &OLDNODES.mapCodes, OLDNODES_MAPCODES_LENGTH);
      NODES.capacity = NO_RECORD;
      memset(&NODES.reserved, 0x00, NODES_RESERVED_LENGTH);
      NODES.stopFlags = OLDNODES.stopFlags;
      NODES.flags = OLDNODES.flags;
      BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &NODES, &dataBufferLength[NEWFILE],  &NODESKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLDNODES, &dataBufferLength[OLDFILE],  &OLDNODESKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
//
//  Check to see if we need to do the conversion of the old-style lat/longs or easting/northings
//  to the new version.  If so, warn his ass so that he knows exactly what he's getting in to.
//
  pStaticMESSAGE->SetWindowText("Checking Lat/Long Conversion");
  if(!(GetPrivateProfileInt((LPSTR)"Description", "DoneLLConversion", (int)FALSE, (LPSTR)szDatabaseFileName)))
  {
//
//  Warn him
//
    s.LoadString(TEXT_269);
    MessageBeep(MB_ICONQUESTION);
//
//  Get the go-ahead
//
    if(MessageBox(s, TMS, MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
      rcode2 = btrieve(B_OPEN, NEWFILE, dummy, "NODES.B60", 0);
      rcode2 = btrieve(B_STAT, NEWFILE, &BSTAT, dummy, 0);
      numRecords = (int)BSTAT.numRecords;
      nI = 0;
      dataBufferLength[NEWFILE] = sizeof(NODES);
//
//  Set up the status bar
//
      StatusBarStart(hWndMain, "Status");
      StatusBarText("Converting old-style co-ordinates to new version");
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
      rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[NEWFILE],
            &NODES, &dataBufferLength[NEWFILE],  &NODESKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
      while(rcode2 == 0)
      {
        NODES.longitude = GetLLEN(NODES.longitude, tempString);
        NODES.latitude  = GetLLEN(NODES.latitude,  tempString);
        BTRVID((BTI_WORD)B_UPDATE, positionBlock[NEWFILE],
              &NODES, &dataBufferLength[NEWFILE],  &NODESKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
//
//  Get the next record
//
        rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[NEWFILE],
              &NODES, &dataBufferLength[NEWFILE],  &NODESKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
        StatusBar((long)nI++, (long)numRecords);
      }
//
//  Save the fact that we've done this
//
      WritePrivateProfileString((LPSTR)"Description", (LPSTR)"DoneLLConversion", (LPSTR)"1", (LPSTR)szDatabaseFileName);
      StatusBarEnd();
      btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    }
  }
//
//  See if RUNS.B60 exists
//
//  If not, copy it from szInstallDirectory/FRESH and populate it with RUNS.DAT.
//  Check, though, the location of RUNS.B60 - it might be (but shouldn't be)
//  in the Common folder.
//
  pStaticMESSAGE->SetWindowText("Checking RUNS.B60");
  strcpy(szDatabase[TMS_RUNS], "RUNS.B60");
  if(m_CommonFlag & (1 << TMS_RUNS))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_RUNS]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_RUNS]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_RUNS]);
//
//  If RUNS.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_RUNS], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style RUNS table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    btrieve(B_OPEN, OLDFILE, dummy, "RUNS.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
//
//  The two data buffer lengths are the same
//
    dataBufferLength[OLDFILE] = BSTAT.recordLength;
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &RUNS, &dataBufferLength[OLDFILE],  &RUNSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &RUNS, &dataBufferLength[OLDFILE],  &RUNSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &RUNS, &dataBufferLength[OLDFILE],  &RUNSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
//
//  Now check on the old style vs. new style BUSTYPES (BUSTYPES.B60) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in BUSTYPES.DAT.  Check, though, the
//  location of BUSTYPES.B60 - it might be (but shouldn't be) in the Common folder.
//
  pStaticMESSAGE->SetWindowText("Checking BUSTYPES.B60");
  strcpy(szDatabase[TMS_BUSTYPES], "BUSTYPES.B60");
  if(m_CommonFlag & (1 << TMS_BUSTYPES))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_BUSTYPES]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_BUSTYPES]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_BUSTYPES]);
//
//  If BUSTYPES.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_BUSTYPES], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style BUSTYPES table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    btrieve(B_OPEN, OLDFILE, dummy, "BUSTYPES.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLD_BUSTYPES);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(BUSTYPES);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &OLD_BUSTYPES, &dataBufferLength[OLDFILE],  &OLD_BUSTYPESKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      BUSTYPES.recordID = OLD_BUSTYPES.recordID;
      BUSTYPES.COMMENTSrecordID = OLD_BUSTYPES.COMMENTSrecordID;
      BUSTYPES.number = OLD_BUSTYPES.number;
      memset(&BUSTYPES.fleetName, ' ', BUSTYPES_FLEETNAME_LENGTH);
      if(OLD_BUSTYPES_NAME_LENGTH > BUSTYPES_NAME_LENGTH)
        memcpy(&BUSTYPES.name, &OLD_BUSTYPES.name, BUSTYPES_NAME_LENGTH);
      else
        memcpy(&BUSTYPES.name, &OLD_BUSTYPES.name, OLD_BUSTYPES_NAME_LENGTH);
      BUSTYPES.capacitySeated = 0;
      BUSTYPES.capacityStanding = 0;
      BUSTYPES.flags = OLD_BUSTYPES.flags;
      memset(&BUSTYPES.reserved, 0x00, BUSTYPES_RESERVED_LENGTH);
      BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &BUSTYPES, &dataBufferLength[NEWFILE],  &BUSTYPESKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLD_BUSTYPES, &dataBufferLength[OLDFILE],  &OLD_BUSTYPESKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
//
//  Now check on the old style vs. new style TRIPS.B60 table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in TRIPS.DAT.  Check, though, the
//  location of TRIPS.B60 - it might be (but shouldn't be) in the Common folder.
//
  pStaticMESSAGE->SetWindowText("Checking TRIPS.B60");
  strcpy(szDatabase[TMS_TRIPS], "TRIPS.B60");
  if(m_CommonFlag & (1 << TMS_TRIPS))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_TRIPS]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_TRIPS]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_TRIPS]);
//
//  If TRIPS.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_TRIPS], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style TRIPS table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    btrieve(B_OPEN, OLDFILE, dummy, "TRIPS.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLD_TRIPS);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(TRIPS);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    bCreatedTRIPSB60 = TRUE;
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &OLD_TRIPS, &dataBufferLength[OLDFILE],  &OLD_TRIPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      TRIPS.recordID = OLD_TRIPS.recordID;
      TRIPS.COMMENTSrecordID = OLD_TRIPS.COMMENTSrecordID;
      TRIPS.SIGNCODESrecordID = OLD_TRIPS.SIGNCODESrecordID;
      TRIPS.ROUTESrecordID = OLD_TRIPS.ROUTESrecordID;
      TRIPS.SERVICESrecordID = OLD_TRIPS.SERVICESrecordID;
      TRIPS.BUSTYPESrecordID = OLD_TRIPS.BUSTYPESrecordID;
      TRIPS.PATTERNNAMESrecordID = OLD_TRIPS.PATTERNNAMESrecordID;
      TRIPS.directionIndex = OLD_TRIPS.directionIndex;
      TRIPS.tripNumber = OLD_TRIPS.tripNumber;
      TRIPS.timeAtMLP = OLD_TRIPS.timeAtMLP;
      TRIPS.tripSequence = OLD_TRIPS.tripSequence;
      TRIPS.shift.negativeMax = OLD_TRIPS.negativeShiftMax;
      TRIPS.shift.positiveMax = OLD_TRIPS.positiveShiftMax;
      TRIPS.shift.actual = OLD_TRIPS.actualShift;
      TRIPS.standard.blockNumber = OLD_TRIPS.blockNumber;
      TRIPS.standard.BLOCKNAMESrecordID = OLD_TRIPS.BLOCKNAMESrecordID;
      TRIPS.standard.POGNODESrecordID = OLD_TRIPS.POGNODESrecordID;
      TRIPS.standard.PIGNODESrecordID = OLD_TRIPS.PIGNODESrecordID;
      TRIPS.standard.RGRPROUTESrecordID = OLD_TRIPS.RGRPROUTESrecordID;
      TRIPS.standard.SGRPSERVICESrecordID = OLD_TRIPS.SGRPSERVICESrecordID;
      TRIPS.standard.assignedToNODESrecordID = OLD_TRIPS.assignedToNODESrecordID;
      TRIPS.standard.layoverMin = OLD_TRIPS.layoverMin;
      TRIPS.standard.layoverMax = OLD_TRIPS.layoverMax;
      TRIPS.standard.blockSequence = NO_RECORD;
      TRIPS.dropback.blockNumber = 0;
      TRIPS.dropback.BLOCKNAMESrecordID = NO_RECORD;
      TRIPS.dropback.POGNODESrecordID = NO_RECORD;
      TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
      TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
      TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
      TRIPS.dropback.assignedToNODESrecordID = NO_RECORD;
      TRIPS.dropback.layoverMin = NO_TIME;
      TRIPS.dropback.layoverMax = NO_TIME;
      TRIPS.dropback.blockSequence = NO_TIME;
      memset(&TRIPS.reserved, 0x00, TRIPS_RESERVED_LENGTH);
      TRIPS.flags = OLD_TRIPS.flags;
      rcode2 = BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &TRIPS, &dataBufferLength[NEWFILE],  &TRIPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLD_TRIPS, &dataBufferLength[OLDFILE],  &OLD_TRIPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
//
//  Now check on the old style vs. new style DRIVERS.B60 table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in DRIVERS.DAT.  Check, though, the
//  location of DRIVERS.B60 - it might be (but shouldn't be) in the Common folder.
//
  pStaticMESSAGE->SetWindowText("Checking DRIVERS.B60");
  strcpy(szDatabase[TMS_DRIVERS], "DRIVERS.B60");
  if(m_CommonFlag & (1 << TMS_DRIVERS))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_DRIVERS]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_DRIVERS]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_DRIVERS]);
//
//  If DRIVERS.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_DRIVERS], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style DRIVERS table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    btrieve(B_OPEN, OLDFILE, dummy, "DRIVERS.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLDDRIVERS);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(DRIVERS);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &OLDDRIVERS, &dataBufferLength[OLDFILE],  &OLDDRIVERSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      DRIVERS.recordID = OLDDRIVERS.recordID;
      DRIVERS.COMMENTSrecordID = OLDDRIVERS.COMMENTSrecordID;
      DRIVERS.DIVISIONSrecordID = OLDDRIVERS.DIVISIONSrecordID;
      DRIVERS.DRIVERTYPESrecordID = OLDDRIVERS.DRIVERTYPESrecordID;
      memcpy(&DRIVERS.lastName, &OLDDRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      memcpy(&DRIVERS.firstName, &OLDDRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      memcpy(&DRIVERS.initials, &OLDDRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      memcpy(&DRIVERS.badgeNumber, &OLDDRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      DRIVERS.crewNumber = OLDDRIVERS.crewNumber;
      memcpy(&DRIVERS.streetAddress, &OLDDRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
      memcpy(&DRIVERS.city, &OLDDRIVERS.city, DRIVERS_CITY_LENGTH);
      memcpy(&DRIVERS.province, &OLDDRIVERS.province, DRIVERS_PROVINCE_LENGTH);
      memcpy(&DRIVERS.postalCode, &OLDDRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
      DRIVERS.telephoneArea = OLDDRIVERS.telephoneArea;
      DRIVERS.telephoneNumber = OLDDRIVERS.telephoneNumber;
      DRIVERS.faxArea = 0;
      DRIVERS.faxNumber = 0;
      memset(&DRIVERS.emailAddress, ' ', DRIVERS_EMAILADDRESS_LENGTH);
      DRIVERS.hireDate = OLDDRIVERS.hireDate;
      DRIVERS.seniorityDate = OLDDRIVERS.seniorityDate;
      DRIVERS.senioritySort = (long)OLDDRIVERS.senioritySort;
      DRIVERS.vacationTime = OLDDRIVERS.vacationEntitlement;
      DRIVERS.personalTime = 0;
      DRIVERS.sickTime = 0;
      memcpy(&DRIVERS.SIN, &OLDDRIVERS.SIN, DRIVERS_SIN_LENGTH);
      memset(&DRIVERS.bitmapFileExt, ' ', DRIVERS_BITMAPFILEEXT_LENGTH);
      memset(&DRIVERS.licenseNumber, ' ', DRIVERS_LICENSENUMBER_LENGTH);
      DRIVERS.licenseExpiryDate = 0;
      memset(&DRIVERS.licenseProvinceOfIssue, ' ', DRIVERS_PROVINCE_LENGTH);
      memset(&DRIVERS.licenseType, ' ', DRIVERS_LICENSETYPE_LENGTH);
      memset(&DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
      memset(&DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
      DRIVERS.flags = OLDDRIVERS.flags;
      rcode2 = BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &DRIVERS, &dataBufferLength[NEWFILE],  &DRIVERSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLDDRIVERS, &dataBufferLength[OLDFILE],  &OLDDRIVERSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
//
//  Check on the old style vs. new style ROSTER (ROSTER.B60) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in ROSTER.DAT.  Some properties have
//  used multiple divisions in order to create multi-week rosters.  If this is
//  case, then we need to establish which is the primary division, and then
//  populate each "week" slot in the new file with the old data.
//
  pStaticMESSAGE->SetWindowText("Checking ROSTER.B60");
  strcpy(szDatabase[TMS_ROSTER], "ROSTER.B60");
  if(m_CommonFlag & (1 << TMS_ROSTER))
  {
    strcpy(toFile, szDatabaseRoot);
    strcat(toFile, "\\Common\\");
    strcat(toFile, szDatabase[TMS_ROSTER]);
  }
  else
  {
    strcpy(toFile, szDatabase[TMS_ROSTER]);
  }
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_ROSTER]);
//
//  If ROSTER.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_ROSTER], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  With only one division, this is a no-brainer - just do the copy
//
    btrieve(B_OPEN, OLDFILE, dummy, "DIVISION.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(DIVISIONS);
    if(numRecords <= 1)
    {
      btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
//
//  Set up the status bar
//
      StatusBarStart(hWndMain, "Status");
      StatusBarText("Copying old-style ROSTER table to new version");
//
//  And copy the file
//
      CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
      btrieve(B_OPEN, OLDFILE, dummy, "ROSTER.DAT", 0);
      btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
      numRecords = (int)BSTAT.numRecords;
      dataBufferLength[OLDFILE] = sizeof(OLDROSTER);
      nI = 0;
      btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
      dataBufferLength[NEWFILE] = sizeof(ROSTER);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
      rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
            &OLDROSTER, &dataBufferLength[OLDFILE],  &OLDROSTERKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
      while(rcode2 == 0)
      {
        ROSTER.recordID = OLDROSTER.recordID;
        ROSTER.COMMENTSrecordID = OLDROSTER.COMMENTSrecordID;
        ROSTER.DIVISIONSrecordID = OLDROSTER.DIVISIONSrecordID;
        ROSTER.DRIVERSrecordID = OLDROSTER.DRIVERSrecordID;
        ROSTER.rosterNumber = OLDROSTER.rosterNumber;
        ROSTER.WEEK[0].flags = OLDROSTER.flags;
        memcpy(&ROSTER.WEEK[0].RUNSrecordIDs, &OLDROSTER.RUNSrecordIDs, sizeof(long) * ROSTER_MAX_DAYS);
        memset(&ROSTER.reserved, 0x00, ROSTER_RESERVED_LENGTH);
        ROSTER.flags = 0;
        rcode2 = BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
              &ROSTER, &dataBufferLength[NEWFILE],  &DRIVERSKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
        rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
              &OLDROSTER, &dataBufferLength[OLDFILE],  &OLDDRIVERSKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
        StatusBar((long)nI++, (long)numRecords);
      }
      btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
      btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
      StatusBarEnd();
    }
//
//  More than one division - get him to set out the order
//
    else
    {
//
//  Let him know what's going on
//
      CString s;
      
      MessageBeep(MB_ICONINFORMATION);
      s.LoadString(TEXT_384);
      MessageBox(s, TMS, MB_ICONINFORMATION | MB_OK);
//
//  Load the divisions
//
      DIVISIONDATADef DIVISIONDATA;

      DIVISIONDATA.numDivisions = min(numRecords, ROSTER_MAX_WEEKS);
      rcode2 = btrieve(B_GETFIRST, OLDFILE, &DIVISIONS, &DIVISIONSKey0, 0);
      nI = 0;
      while(rcode2 == 0)
      {
        DIVISIONDATA.DIVISIONSrecordIDs[nI] = DIVISIONS.recordID;
        strncpy(DIVISIONDATA.name[nI], DIVISIONS.name, DIVISIONS_NAME_LENGTH);
        trim(DIVISIONDATA.name[nI], DIVISIONS_NAME_LENGTH);
        rcode2 = btrieve(B_GETNEXT, OLDFILE, &DIVISIONS, &DIVISIONSKey0, 0);
        nI++;
        if(nI >= ROSTER_MAX_WEEKS + 1)
        {
          break;
        }
      }
//
//  Fire up the dialog to get the division order
//
      CRosterOrder dlg(this, &DIVISIONDATA);

      if(dlg.DoModal() != IDOK)
      {
        OnCancel();
        return;
      }
//
//  Set up the status bar
//
      StatusBarStart(hWndMain, "Status");
      StatusBarText("Copying old-style ROSTER tables to new version");
//
//  And copy the file
//
      CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
      btrieve(B_OPEN, OLDFILE, dummy, "ROSTER.DAT", 0);
      btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
      numRecords = (int)BSTAT.numRecords;
      dataBufferLength[OLDFILE] = sizeof(OLDROSTER);
      btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
      dataBufferLength[NEWFILE] = sizeof(ROSTER);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
      rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
            &OLDROSTER, &dataBufferLength[OLDFILE],  &OLDROSTERKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
      long ROSTERrecordID = 1;
      long weekNumber;
      BOOL bMerged;
      BOOL bFound;

      nK = 0;
      while(rcode2 == 0)
      {
//
//  Figure out the "week"
//
        for(bFound = FALSE, nI = 1; nI < DIVISIONDATA.numDivisions; nI++)
        {
          if(OLDROSTER.DIVISIONSrecordID == DIVISIONDATA.DIVISIONSrecordIDs[nI])
          {
            weekNumber = nI - 1;
            bFound = TRUE;
            break;
          }
        }
//
//  See if there's a roster number match in the "new" roster and the same driver
//
        if(bFound)
        {
          rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[NEWFILE],
                &ROSTER, &dataBufferLength[NEWFILE],  &ROSTERKey0, (BTI_SINT)0,
                (BTI_BUFFER_PTR)&clientID);
          bMerged = FALSE;
          while(rcode2 == 0)
          {
//
//  If the driver is the same - merge it in
//
            if(ROSTER.DRIVERSrecordID == OLDROSTER.DRIVERSrecordID &&
//                  ROSTER.rosterNumber == OLDROSTER.rosterNumber &&
//                  ROSTER.DRIVERSrecordID != NO_RECORD)
                  ROSTER.rosterNumber == OLDROSTER.rosterNumber)
            {
              memcpy(&ROSTER.WEEK[weekNumber].RUNSrecordIDs, &OLDROSTER.RUNSrecordIDs, sizeof(long) * ROSTER_MAX_DAYS);
              ROSTER.WEEK[weekNumber].flags = OLDROSTER.flags;
              ROSTER.flags |= 1 << weekNumber;
              rcode2 = BTRVID((BTI_WORD)B_UPDATE, positionBlock[NEWFILE],
                    &ROSTER, &dataBufferLength[NEWFILE],  &ROSTERKey0, (BTI_SINT)0,
                    (BTI_BUFFER_PTR)&clientID);
              bMerged = TRUE;
              break;
            }
            rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[NEWFILE],
                  &ROSTER, &dataBufferLength[NEWFILE],  &ROSTERKey0, (BTI_SINT)0,
                  (BTI_BUFFER_PTR)&clientID);
          }
//
//  Not found in the new file - create a record
//
          if(!bMerged)
          {
            memset(&ROSTER, 0x00, sizeof(ROSTERDef));
            for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
            {
              for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
              {
                ROSTER.WEEK[nI].RUNSrecordIDs[nJ] = NO_RECORD;
              }
            }
            ROSTER.recordID = ROSTERrecordID++;
            ROSTER.COMMENTSrecordID = OLDROSTER.COMMENTSrecordID;
            ROSTER.DIVISIONSrecordID = DIVISIONDATA.DIVISIONSrecordIDs[0];
            ROSTER.DRIVERSrecordID = OLDROSTER.DRIVERSrecordID;
            ROSTER.rosterNumber = OLDROSTER.rosterNumber;
            memcpy(&ROSTER.WEEK[weekNumber].RUNSrecordIDs, &OLDROSTER.RUNSrecordIDs, sizeof(long) * ROSTER_MAX_DAYS);
            ROSTER.WEEK[weekNumber].flags = OLDROSTER.flags;
            ROSTER.flags = 1 << weekNumber;
            rcode2 = BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
                  &ROSTER, &dataBufferLength[NEWFILE],  &ROSTERKey0, (BTI_SINT)0,
                  (BTI_BUFFER_PTR)&clientID);
          }
        }  // bFound
//
//  Get the next old roster record and cycle back
//
        rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
              &OLDROSTER, &dataBufferLength[OLDFILE],  &OLDROSTERKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
        StatusBar((long)nK++, (long)numRecords);
      }
      btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
      btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
      StatusBarEnd();
    }
  }
//
//  Check on the old style vs. new style DAILYOPS (DAILYOPS.B61) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in DAILYOPS.DAT.
//
  pStaticMESSAGE->SetWindowText("Checking DAILYOPS.B61");
  strcpy(szDatabase[TMS_DAILYOPS], "DAILYOPS.B61");
  strcpy(toFile, szDatabaseRoot);
  strcat(toFile, "\\Common\\");
  strcat(toFile, szDatabase[TMS_DAILYOPS]);
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_DAILYOPS]);
//
//  If DAILYOPS.B61 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_DAILYOPS], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying old-style DAILYOPS table to new version");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    strcpy(tempString, szDatabaseRoot);
    strcat(tempString, "\\Common\\");
    strcat(tempString, "DAILYOPS.B60");
    btrieve(B_OPEN, OLDFILE, dummy, tempString, 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLDDAILYOPS);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(DAILYOPS);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &OLDDAILYOPS, &dataBufferLength[OLDFILE],  &OLDDAILYOPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    while(rcode2 == 0)
    {
      memset(&DAILYOPS, 0x00, sizeof(DAILYOPS));
//
//  Base portion of file 
//
//  Record ID
//
      DAILYOPS.recordID = OLDDAILYOPS.recordID;
//
//  Date/time of entry
//
      DAILYOPS.entryDateAndTime = TMSmktime(OLDDAILYOPS.dateOfEntry, OLDDAILYOPS.timeOfEntry);
//
//  User ID
//
      DAILYOPS.userID = (char)OLDDAILYOPS.userID;
//
//  Record Type Flag
//
      DAILYOPS.recordTypeFlag = (char)OLDDAILYOPS.flags;
//
//  "Pertains to" date
//  "Pertains to" time
//  By-type flags
//
//  Plus - Specifics on each DOPS type
//
//  RosterTemplate
//
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.dateOfEntry;
        DAILYOPS.pertainsToTime = OLDDAILYOPS.timeOfEntry;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.RosterTemplate.flags;
        DAILYOPS.DAILYOPSrecordID = NO_RECORD;
        DAILYOPS.DRIVERSrecordID = OLDDAILYOPS.OLDDOPS.RosterTemplate.DRIVERSrecordID;
        DAILYOPS.DOPS.RosterTemplate.ROSTERrecordID = OLDDAILYOPS.OLDDOPS.RosterTemplate.ROSTERrecordID;
      }
//
//  Bus
//
      else if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.OLDDOPS.Bus.date;
        DAILYOPS.pertainsToTime = NO_TIME;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.Bus.flags;
        DAILYOPS.DAILYOPSrecordID = OLDDAILYOPS.OLDDOPS.Bus.DAILYOPSrecordID;
        DAILYOPS.DRIVERSrecordID = NO_RECORD;
        DAILYOPS.DOPS.Bus.RGRPROUTESrecordID = OLDDAILYOPS.OLDDOPS.Bus.RGRPROUTESrecordID;
        DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID = OLDDAILYOPS.OLDDOPS.Bus.SGRPSERVICESrecordID;
        DAILYOPS.DOPS.Bus.BUSESrecordID = OLDDAILYOPS.OLDDOPS.Bus.BUSESrecordID;
        DAILYOPS.DOPS.Bus.TRIPSrecordID = OLDDAILYOPS.OLDDOPS.Bus.TRIPSrecordID;
        DAILYOPS.DOPS.Bus.newBUSESrecordID = OLDDAILYOPS.OLDDOPS.Bus.newBUSESrecordID;
        DAILYOPS.DOPS.Bus.swapReasonIndex = (char)OLDDAILYOPS.OLDDOPS.Bus.swapReasonIndex;
        DAILYOPS.DOPS.Bus.locationNODESrecordID = NO_RECORD;
      }
//
//  Operator
//
      else if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.dateOfEntry;
        DAILYOPS.pertainsToTime = OLDDAILYOPS.timeOfEntry;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.Operator.flags;
        DAILYOPS.DAILYOPSrecordID = OLDDAILYOPS.OLDDOPS.Operator.DAILYOPSrecordID;
        DAILYOPS.DRIVERSrecordID = OLDDAILYOPS.OLDDOPS.Operator.DRIVERSrecordID;
        DAILYOPS.DOPS.Operator.ROUTESrecordID = OLDDAILYOPS.OLDDOPS.Operator.ROUTESrecordID;
        DAILYOPS.DOPS.Operator.SERVICESrecordID = OLDDAILYOPS.OLDDOPS.Operator.SERVICESrecordID;
        DAILYOPS.DOPS.Operator.PATTERNNAMESrecordID = OLDDAILYOPS.OLDDOPS.Operator.PATTERNNAMESrecordID;
        DAILYOPS.DOPS.Operator.directionIndex = OLDDAILYOPS.OLDDOPS.Operator.directionIndex;
        DAILYOPS.DOPS.Operator.timeAtMLP = OLDDAILYOPS.OLDDOPS.Operator.timeAtMLP;
        DAILYOPS.DOPS.Operator.deadheadTime = OLDDAILYOPS.OLDDOPS.Operator.deadheadTime;
        DAILYOPS.DOPS.Operator.timeAdjustment = OLDDAILYOPS.OLDDOPS.Operator.newTime;
      }
//
//  Absence
//
      else if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.OLDDOPS.Absence.fromDate;
        DAILYOPS.pertainsToTime = OLDDAILYOPS.OLDDOPS.Absence.fromTime;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.Absence.flags;
        DAILYOPS.DAILYOPSrecordID = OLDDAILYOPS.OLDDOPS.Absence.DAILYOPSrecordID;
        DAILYOPS.DRIVERSrecordID = OLDDAILYOPS.OLDDOPS.Absence.DRIVERSrecordID;
        DAILYOPS.DOPS.Absence.untilDate = OLDDAILYOPS.OLDDOPS.Absence.toDate;
        DAILYOPS.DOPS.Absence.untilTime = OLDDAILYOPS.OLDDOPS.Absence.toTime;
        DAILYOPS.DOPS.Absence.reasonIndex = OLDDAILYOPS.OLDDOPS.Absence.reasonIndex;
        DAILYOPS.DOPS.Absence.timeLost = (long)(OLDDAILYOPS.OLDDOPS.Absence.timeLost * 3600);
        DAILYOPS.DOPS.Absence.paidTime[0] = (long)(OLDDAILYOPS.OLDDOPS.Absence.paidTime[0] * 3600);
        DAILYOPS.DOPS.Absence.paidTime[1] = (long)(OLDDAILYOPS.OLDDOPS.Absence.paidTime[1] * 3600);
        DAILYOPS.DOPS.Absence.unpaidTime = (long)(OLDDAILYOPS.OLDDOPS.Absence.unpaidTime * 3600);
      }
//
//  Open Work
//
      else if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.OLDDOPS.OpenWork.date;
        DAILYOPS.pertainsToTime = OLDDAILYOPS.OLDDOPS.OpenWork.time;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.OpenWork.flags;
        DAILYOPS.DAILYOPSrecordID = OLDDAILYOPS.OLDDOPS.OpenWork.DAILYOPSrecordID;
        DAILYOPS.DRIVERSrecordID = OLDDAILYOPS.OLDDOPS.OpenWork.DRIVERSrecordID;
        DAILYOPS.DOPS.OpenWork.RUNSrecordID = OLDDAILYOPS.OLDDOPS.OpenWork.RUNSrecordID;
        DAILYOPS.DOPS.OpenWork.untilTime = OLDDAILYOPS.OLDDOPS.OpenWork.time;  // should always be NO_TIME
      }
//
//  Service
//
      else if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_SERVICE)
      {
        DAILYOPS.pertainsToDate = OLDDAILYOPS.OLDDOPS.Service.date;
        DAILYOPS.pertainsToTime = NO_TIME;
        DAILYOPS.recordFlags = OLDDAILYOPS.OLDDOPS.Service.flags;
        DAILYOPS.DAILYOPSrecordID = NO_RECORD;
        DAILYOPS.DRIVERSrecordID = NO_RECORD;
        DAILYOPS.DOPS.Service.SERVICESrecordID = OLDDAILYOPS.OLDDOPS.Service.SERVICESrecordID;
      }
//
//  Fix for non-meted discipline
//
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
      {
        if((DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER) &&
              (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCETODISCIPLINE))
        {
          DAILYOPS.recordFlags |= DAILYOPS_FLAG_DISCIPLINEMETED;
        }
      }
//
//  Insert and get next
//
      BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
            &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLDDAILYOPS, &dataBufferLength[OLDFILE],  &OLDDAILYOPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
//
//  And since we're effecting a change in the data storage types of 
//  DRIVERS.vacationTime, DRIVERS.personalTime, DRIVERS.sickTime, and
//  DRIVERS.floatTime (from float to long), do that here.
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Modifying DRIVERS offtime fields");
    strcpy(tempString, szDatabaseRoot);
    strcat(tempString, "\\Common\\");
    strcat(tempString, "DRIVERS.B60");
    btrieve(B_OPEN, OLDFILE, dummy, tempString, 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(DRIVERS);
    nI = 0;
    strcpy(szarString, szDatabaseRoot);
    strcat(szarString, "\\Common\\");
    strcat(szarString, "DRIVERS.OLD");
    CopyFile(tempString, szarString, FALSE);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &DRIVERS, &dataBufferLength[OLDFILE],  &DRIVERSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    float oldTime;
    long  multiplier;

    while(rcode2 == 0)
    {
//
//  Vacation time
//
      multiplier = (DRIVERS.flags & DRIVERS_FLAG_VACATIONTIMEINHOURS ? 3600 : 86400);  // hours in seconds vs, days in seconds
      memcpy(&oldTime, &DRIVERS.vacationTime, 4);
      oldTime *= multiplier;
      DRIVERS.vacationTime = (long)oldTime;
//
//  Personal time
//
      multiplier = (DRIVERS.flags & DRIVERS_FLAG_PERSONALTIMEINHOURS ? 3600 : 86400);  // hours in seconds vs, days in seconds
      memcpy(&oldTime, &DRIVERS.personalTime, 4);
      oldTime *= multiplier;
      DRIVERS.personalTime = (long)oldTime;
//
//  Sick time
//
      multiplier = (DRIVERS.flags & DRIVERS_FLAG_SICKTIMEINHOURS ? 3600 : 86400);  // hours in seconds vs, days in seconds
      memcpy(&oldTime, &DRIVERS.sickTime, 4);
      oldTime *= multiplier;
      DRIVERS.sickTime = (long)oldTime;
//
//  Float time
//
      multiplier = (DRIVERS.flags & DRIVERS_FLAG_FLOATTIMEINHOURS ? 3600 : 86400);  // hours in seconds vs, days in seconds
      memcpy(&oldTime, &DRIVERS.floatTime, 4);
      oldTime *= multiplier;
      DRIVERS.floatTime = (long)oldTime;
//
//  Update and get next
//
      rcode2 = BTRVID((BTI_WORD)B_UPDATE, positionBlock[OLDFILE],
            &DRIVERS, &dataBufferLength[OLDFILE],  &DRIVERSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &DRIVERS, &dataBufferLength[OLDFILE],  &DRIVERSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
//
//  Change the DISCIPLINE structure's dateOfEntry and timeOfEntry
//  to be consistent with the new DAILYOPS structure's time_t entryDateAndTime
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Modifying DISCIPLINE entry date/time fields");
    btrieve(B_OPEN, OLDFILE, dummy, "DISCIPLI.DAT", 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(DISCIPLINE);
    nI = 0;
    CopyFile("DISCIPLI.DAT", "DISCIPLI.OLD", FALSE);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &DISCIPLINE, &dataBufferLength[OLDFILE],  &DISCIPLINEKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through
//
    long oldTimeOfEntry;

    while(rcode2 == 0)
    {
      memcpy(&oldTimeOfEntry, &DISCIPLINE.unused, 4);
      DISCIPLINE.entryDateAndTime = TMSmktime(DISCIPLINE.entryDateAndTime, oldTimeOfEntry);
//
//  Update and get next
//
      rcode2 = BTRVID((BTI_WORD)B_UPDATE, positionBlock[OLDFILE],
            &DISCIPLINE, &dataBufferLength[OLDFILE],  &DISCIPLINEKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &DISCIPLINE, &dataBufferLength[OLDFILE],  &DISCIPLINEKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }
/*
//  In general terms, this was probably a good idea - split out the multi-day
//  absences into individual days so that the database queries pertained just
//  to a specific day, as opposed to a range of days as set in "pertainsToDate"
//  and DOPS.Absence.untildate.  Unfortunately, the implementation uncovered
//  certain difficulties, such as placing the correct values in he paid/unpaid 
//  time fields, and UI concerns with the amount of information shown.
//
//  Roll-back time.
//
//
//
//  Check on the old style vs. new style DAILYOPS (DAILYOPS.B62) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH and
//  populate it with the existing data in DAILYOPS.DAT.
//
  strcpy(szDatabase[TMS_DAILYOPS], "DAILYOPS.B62");
  strcpy(toFile, szDatabaseRoot);
  strcat(toFile, "\\Common\\");
  strcat(toFile, szDatabase[TMS_DAILYOPS]);
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_DAILYOPS]);
//
//  If DAILYOPS.B62 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_DAILYOPS], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
    DAILYOPSDef B61DAILYOPS;
    TMSKey0Def  B61DAILYOPSKey0;
    long startDate, endDate, thisDate;
    long yyyy, mm, dd;
    long year, month, day;
    BOOL bFirst, bKeepGoing;
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying DAILYOPS table to new version - Pass 1");
//
//  And copy the file
//
    CopyFile(fromFile, toFile, FALSE);
//
//  File copied - now populate it
//
    strcpy(tempString, szDatabaseRoot);
    strcat(tempString, "\\Common\\");
    strcat(tempString, "DAILYOPS.B61");
    btrieve(B_OPEN, OLDFILE, dummy, tempString, 0);
    btrieve(B_STAT, OLDFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    dataBufferLength[OLDFILE] = sizeof(OLDDAILYOPS);
    nI = 0;
    btrieve(B_OPEN, NEWFILE, dummy, toFile, 0);
    dataBufferLength[NEWFILE] = sizeof(DAILYOPS);
//
//  Get the first record.  We're using the direct BTRVID call
//  here so we don't screw up the recordLength[] array in btrieve.c
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &B61DAILYOPS, &dataBufferLength[OLDFILE],  &B61DAILYOPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
//
//  Loop through in three passes.  
//
//  First, do all the non-absence records so we won't have to
//  worry about DAILYOPS.DAILYOPSrecordID entries pointing off
//  to the wrong thing.
//
    while(rcode2 == 0)
    {
//
//  All records (aside from absence) just get copied over to the new file
//
      if(!(B61DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
      {
        memcpy(&DAILYOPS, &B61DAILYOPS, sizeof(DAILYOPS));
        BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
              &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
              (BTI_BUFFER_PTR)&clientID);
      }
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &OLDDAILYOPS, &dataBufferLength[OLDFILE],  &OLDDAILYOPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    StatusBarEnd();
//
//  Pass 2 - Just the absence records
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying DAILYOPS table to new version - Pass 2");
    nI = 0;
//
//  Establish recordID
//
    rcode2 = BTRVID((BTI_WORD)B_GETLAST, positionBlock[OLDFILE],
          &B61DAILYOPS, &dataBufferLength[OLDFILE],  &B61DAILYOPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
    
    long recordID = AssignRecID(rcode2, B61DAILYOPS.recordID);
//
//  Cycle through
//
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[OLDFILE],
          &B61DAILYOPS, &dataBufferLength[OLDFILE],  &B61DAILYOPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
    while(rcode2 == 0)
    {
//
//  Absence records only
//
      if(B61DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
      {
//
//  Was this record "unregistered"?  If so, then it points back to 
//  the original DAILYOPS record in DAILYOPS.DAILYOPSrecordID.
//
//  If this is the case, then we need to propagate those records to
//  match the newly expanded records.  We know they're there and expanded
//  because a record can't be "unregistered" if it wasn't already
//  registered, and we're talking the file in Key0 order.
//
//  So - if we see the unregister, locate the new record in B62, and then cycle
//  through B62 until the untilDate, adding one unregister for each absence
//  day, and adjusting DAILYOPS.DAILYOPSrecordID accordingly.
//
        if(B61DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEUNREGISTER)
        {
          DAILYOPSKey2.DAILYOPSrecordID = B61DAILYOPS.DAILYOPSrecordID;
          rcode2 = BTRVID((BTI_WORD)B_GETEQUAL, positionBlock[NEWFILE],
                &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey2, (BTI_SINT)2,
                (BTI_BUFFER_PTR)&clientID);
          GetYMD(DAILYOPS.pertainsToDate, &yyyy, &mm, &dd);
          CTime currentDate(yyyy, mm, dd, 0, 0, 0);
          currentDate -= CTimeSpan(1, 0, 0, 0);
          startDate = DAILYOPS.pertainsToDate;
          endDate = DAILYOPS.DOPS.Absence.untilDate;
          if(endDate <= 0 || endDate < startDate)
          {
            endDate = startDate;
          }

          long registerRecordID = DAILYOPS.recordID;

          bKeepGoing = TRUE;
          bFirst = TRUE;
          while(bKeepGoing)
          {
            currentDate += CTimeSpan(1, 0, 0, 0);
            day = currentDate.GetDay();
            month = currentDate.GetMonth();
            year = currentDate.GetYear();
            thisDate = (year * 10000) + (month * 100) + day;
//
//  The "shut up" point
//
            if(thisDate == endDate)
            {
              bKeepGoing = FALSE;
            }
//
//  Build the "new" record
//
            memcpy(&DAILYOPS, &B61DAILYOPS, sizeof(DAILYOPS));
            if(bFirst)
            {
              DAILYOPS.DAILYOPSrecordID = DAILYOPS.recordID;
            }
            DAILYOPS.recordID = recordID++;
            DAILYOPS.DAILYOPSrecordID = registerRecordID++;
//
//  Adjust the "pertains to" fields
//
            DAILYOPS.pertainsToDate = thisDate;
            if(bFirst)
            {
              bFirst = FALSE;
            }
            else
            {
              DAILYOPS.pertainsToTime = 0;
            }
//
//  And zero out the "until" date/time
//
            DAILYOPS.DOPS.Absence.untilDate = thisDate;
            if(bKeepGoing)
            {
              DAILYOPS.DOPS.Absence.untilTime = 86400;
            }
//
//  Insert and cycle back
//
            BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
                  &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
                  (BTI_BUFFER_PTR)&clientID);
          }  // while bKeepGoing
        }  // for "unregister" type records
//
//  All the rest (except for "unregister")
//
        else
        {
          GetYMD(B61DAILYOPS.pertainsToDate, &yyyy, &mm, &dd);
          CTime currentDate(yyyy, mm, dd, 0, 0, 0);
          CTime currentDateSave = currentDate;
          currentDate -= CTimeSpan(1, 0, 0, 0);
          startDate = B61DAILYOPS.pertainsToDate;
          endDate = B61DAILYOPS.DOPS.Absence.untilDate;
          if(endDate <= 0 || endDate < startDate)
          {
            endDate = startDate;
          }
//
//  Figure out the number of days
//
          int numDays = 1;

          bKeepGoing = TRUE;
          while(bKeepGoing)
          {
            currentDate += CTimeSpan(1, 0, 0, 0);
            day = currentDate.GetDay();
            month = currentDate.GetMonth();
            year = currentDate.GetYear();
            thisDate = (year * 10000) + (month * 100) + day;
            if(thisDate == endDate)
            {
              bKeepGoing = FALSE;
            }
            else
            {
              numDays++;
            }
          }
//
//  Cycle through the records
//
          currentDate = currentDateSave;
          bKeepGoing = TRUE;
          bFirst = TRUE;
          while(bKeepGoing)
          {
            currentDate += CTimeSpan(1, 0, 0, 0);
            day = currentDate.GetDay();
            month = currentDate.GetMonth();
            year = currentDate.GetYear();
            thisDate = (year * 10000) + (month * 100) + day;
//
//  The "shut up" point
//
            if(thisDate == endDate)
            {
              bKeepGoing = FALSE;
            }
//
//  Build the "new" record
//
            memcpy(&DAILYOPS, &B61DAILYOPS, sizeof(DAILYOPS));
            DAILYOPS.recordID = recordID++;
            if(bFirst)
            {
              DAILYOPS.DAILYOPSrecordID = B61DAILYOPS.recordID;
            }
            DAILYOPS.recordID = recordID++;
//
//  Adjust the "pertains to" fields
//
            DAILYOPS.pertainsToDate = thisDate;
            if(bFirst)
            {
              bFirst = FALSE;
            }
            else
            {
              DAILYOPS.pertainsToTime = 0;
            }
//
//  And zero out the "until" date/time
//
            DAILYOPS.DOPS.Absence.untilDate = thisDate;
            if(bKeepGoing)
            {
              DAILYOPS.DOPS.Absence.untilTime = 86340;  // 1159P
            }
//
//  Adjust the time used/lost fields
//
            if(DAILYOPS.DOPS.Absence.timeLost != NO_TIME)
            {
              DAILYOPS.DOPS.Absence.timeLost = DAILYOPS.DOPS.Absence.timeLost / numDays;
            }
            if(DAILYOPS.DOPS.Absence.paidTime[0] != NO_TIME)
            {
              DAILYOPS.DOPS.Absence.paidTime[0] = DAILYOPS.DOPS.Absence.paidTime[0] / numDays;
            }
            if(DAILYOPS.DOPS.Absence.paidTime[1] != NO_TIME)
            {
              DAILYOPS.DOPS.Absence.paidTime[1] = DAILYOPS.DOPS.Absence.paidTime[1] / numDays;
            }
            if(DAILYOPS.DOPS.Absence.unpaidTime != NO_TIME)
            {
              DAILYOPS.DOPS.Absence.unpaidTime = DAILYOPS.DOPS.Absence.unpaidTime / numDays;
            }
//
//  Insert and cycle back
//
            BTRVID((BTI_WORD)B_INSERT, positionBlock[NEWFILE],
                  &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
                  (BTI_BUFFER_PTR)&clientID);
          }  // while bKeepGoing
        }  // for non-"unregister" type records
      }  // for absence records only
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[OLDFILE],
            &B61DAILYOPS, &dataBufferLength[OLDFILE],  &B61DAILYOPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }  // while rcode is 0
    StatusBarEnd();
//
//  Pass 3 - eliminate the bogus DAILYOPS.DAILYOPSrecordID used in step 2
//
    btrieve(B_STAT, NEWFILE, &BSTAT, dummy, 0);
    numRecords = (int)BSTAT.numRecords;
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Copying DAILYOPS table to new version - Pass 3");
    nI = 0;
    rcode2 = BTRVID((BTI_WORD)B_GETFIRST, positionBlock[NEWFILE],
          &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
          (BTI_BUFFER_PTR)&clientID);
    while(rcode2 == 0)
    {
//
//  All records (aside from absence) just get copied over to the new file
//
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
      {
        if(!(DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEUNREGISTER))
        {
          DAILYOPS.DAILYOPSrecordID = NO_RECORD;
          BTRVID((BTI_WORD)B_UPDATE, positionBlock[NEWFILE],
                &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
                (BTI_BUFFER_PTR)&clientID);
        }
      }
      rcode2 = BTRVID((BTI_WORD)B_GETNEXT, positionBlock[NEWFILE],
            &DAILYOPS, &dataBufferLength[NEWFILE],  &DAILYOPSKey0, (BTI_SINT)0,
            (BTI_BUFFER_PTR)&clientID);
      StatusBar((long)nI++, (long)numRecords);
    }
    StatusBarEnd();
//
//  All done the DAILYOPS.B62 changes
//
    btrieve(B_CLOSE, OLDFILE, dummy, dummy, (int)dummy);
    btrieve(B_CLOSE, NEWFILE, dummy, dummy, (int)dummy);
    StatusBarEnd();
  }  // for populating DAILYOPS.B62
*/
//
//  Check on the old style vs. new style OFFTIME (OFFTIME.B60) table
//
//  Does the new one exist?  If not, copy it from szInstallDirectory/FRESH
//
  pStaticMESSAGE->SetWindowText("Checking OFFTIME.B60");
  strcpy(szDatabase[TMS_OFFTIME], "OFFTIME.B60");
  strcpy(toFile, szDatabaseRoot);
  strcat(toFile, "\\Common\\");
  strcat(toFile, szDatabase[TMS_OFFTIME]);
  if(_stat(toFile, &statBuf) != 0)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[TMS_OFFTIME]);
//
//  If OFFTIME.B60 isn't in FRESH, let him know and quit
//
    if(_stat(fromFile, &statBuf) != 0)
    {
      SetCursor(hSaveCursor);
      LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[TMS_OFFTIME], szInstallDirectory);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_ICONSTOP);
      OnCancel();
      return;
    }
//
//  Copy the file
//
    CopyFile(fromFile, toFile, FALSE);
  }
//
//  Done the conversions.
//
//  Open the database files
//
  pStaticMESSAGE->SetWindowText("Opening Files...");
  for(bAbort = FALSE, nI = 0; nI <= TMS_LASTFILE; nI++)
  {
//
//  Set up the file name
//
    if(m_CommonFlag & (1 << nI))
    {
      strcpy(toFile, szDatabaseRoot);
      strcat(toFile, "\\Common\\");
      strcat(toFile, szDatabase[nI]);
      }
    else
    {
      strcpy(toFile, szDatabase[nI]);
    }
//
//  See if it exists
//
    if(_stat(toFile, &statBuf) != 0)
    {
//
//  Nope.  Try to get it from FRESH
//
      strcpy(fromFile, szInstallDirectory);
      strcat(fromFile, "\\FRESH\\");
      strcat(fromFile, szDatabase[nI]);
//
//  If it isn't in FRESH, let him know and quit
//
      if(_stat(fromFile, &statBuf) != 0)
      {
        SetCursor(hSaveCursor);
        LoadString(hInst, ERROR_131, szFormatString, sizeof(szFormatString));
        sprintf(tempString, szFormatString, szDatabase[nI], szInstallDirectory);
        bAbort = TRUE;
        break;
      }
//
//  Copy the file from FRESH
//
      CopyFile(fromFile, toFile, FALSE);
    }
//
//  It (now) exists - try to open it
//
    sprintf(tempString, "Opening %s", toFile);
    pStaticMESSAGE->SetWindowText(tempString);
    rcode2 = btrieve(B_OPEN, nI, dummy, toFile, 0);
    if(rcode2 != 0)
    {
      sprintf(tempString, "szDir=\"%s\"", szDir);
      MessageBox(tempString, TMS, MB_OK);
      bAbort = TRUE;
      LoadString(hInst, ERROR_044, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, toFile, rcode2);
      break;
    }
    sprintf(tempString, "%s Opened", toFile);
    pStaticMESSAGE->SetWindowText(tempString);
  }
//
//  Bad scene - dump him out and tell him why
//
  if(bAbort)
  {
    SetCursor(hSaveCursor);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_ICONSTOP);
    for(nJ = 0; nJ < nI; nJ++)
    {
      btrieve(B_CLOSE, nJ, dummy, dummy, (int)dummy);
    }
    OnCancel();
    return;
  }
//
// Check the record sizes of the btrieve files against the sizes
// of the data structures, and kick out if there's a discrepancy.
// Also, close the previously opened files.
//
  for(bAbort = FALSE, nI = 0; nI <= TMS_LASTFILE; nI++)
  {
    rcode2 = btrieve(B_STAT, nI, &BSTAT, dummy, 0);
    if(rcode2 != 0)
    {
      LoadString(hInst, ERROR_047, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, szDatabase[nI], rcode2);
      bAbort = TRUE;
      break;
    }
    else
    {
      if(BSTAT.recordLength != recordLength[nI])
      {
        LoadString(hInst, ERROR_046, szFormatString, sizeof(szFormatString));
        sprintf(tempString, szFormatString, szDatabase[nI]);
        bAbort = TRUE;
        break;
      }
    }
  }
//
//  Another bad scene, man
//
  if(bAbort)
  {
    SetCursor(hSaveCursor);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_ICONSTOP);
    for(nI = 0; nI <= TMS_LASTFILE; nI++)
    {
      btrieve(B_CLOSE, nI, dummy, dummy, (int)dummy);
    }
    OnCancel();
    return;
  }
//
//  Build up the ass end of DDFFileNumbers with the fileIds of the special views
//  for BLOCKS, RUNS, and ROSTER.  If they're's not in TABLEVIE.DAT, add them.
//
  for(nI = VIEW_FIRSTVIEW; nI <= VIEW_LASTVIEW; nI++)
  {
    strcpy(TABLEVIEWKey1.Name, viewNames[nI - VIEW_FIRSTVIEW]);
    pad(TABLEVIEWKey1.Name, TABLEVIEW_NAME_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_TABLEVIEW, &TABLEVIEW, &TABLEVIEWKey1, 1);
    if(rcode2 != 0)
    {
      TABLEVIEW.Id = nI;
      strcpy(TABLEVIEW.Name, viewNames[nI - VIEW_FIRSTVIEW]);
      pad(TABLEVIEW.Name, TABLEVIEW_NAME_LENGTH);
      if(nI == VIEW_STANDARDBLOCKS || nI == VIEW_DROPBACKBLOCKS)
        FILEDDFKey0.Id = DDFFileNumbers[TMS_TRIPS];
      else if(nI == VIEW_RUNS)
        FILEDDFKey0.Id = DDFFileNumbers[TMS_RUNS];
      else if(nI == VIEW_ROSTER)
        FILEDDFKey0.Id = DDFFileNumbers[TMS_ROSTER];
      else if(nI == VIEW_TIMECHECKS)
        FILEDDFKey0.Id = DDFFileNumbers[TMS_TIMECHECKS];
      btrieve(B_GETEQUAL, FILE_DDF, &FILEDDF, &FILEDDFKey0, 0);
      strncpy(TABLEVIEW.Loc, FILEDDF.Loc, TABLEVIEW_LOC_LENGTH);
      TABLEVIEW.Flags = 0;
      memset(&TABLEVIEW.reserved, 0x00, TABLEVIEW_RESERVED_LENGTH);
      btrieve(B_INSERT, TMS_TABLEVIEW, &TABLEVIEW, &TABLEVIEWKey0, 0);
    }
  }
//
//  What connection time characteristics did he have set?
//
  CONN.flags = 0;
  if(GetPrivateProfileInt((LPSTR)userName, "DisplayRT", (int)TRUE, (LPSTR)szDatabaseFileName))
    CONN.flags |= CONNECTIONS_FLAG_RUNNINGTIME;
  if(GetPrivateProfileInt((LPSTR)userName, "DisplayTT", (int)TRUE, (LPSTR)szDatabaseFileName))
    CONN.flags |= CONNECTIONS_FLAG_TRAVELTIME;
  if(GetPrivateProfileInt((LPSTR)userName, "DisplayDT", (int)TRUE, (LPSTR)szDatabaseFileName))
    CONN.flags |= CONNECTIONS_FLAG_DEADHEADTIME;
  if(GetPrivateProfileInt((LPSTR)userName, "DisplayEQ", (int)FALSE, (LPSTR)szDatabaseFileName))
    CONN.flags |= CONNECTIONS_FLAG_EQUIVALENT;
  if(GetPrivateProfileInt((LPSTR)userName, "DisplayNN", (int)FALSE, (LPSTR)szDatabaseFileName))
    CONN.flags |= CONNECTIONS_FLAG_STOPSTOP;
//
//  Has he set a custom width for pattern and trip node columns?
//
  nodeDisplayWidth = GetPrivateProfileInt((LPSTR)userName, "NodeDisplayWidth", 1536, (LPSTR)szDatabaseFileName);
  if(nodeDisplayWidth < 1536)  // Too small
    nodeDisplayWidth = 1536;
//
//  Set up the global route and service data, and make sure they're valid
//
//  Route
//
  m_RouteRecordID = GetPrivateProfileInt((LPSTR)userName, "Route", (int)(NO_RECORD), (LPSTR)szDatabaseFileName);
  if(m_RouteRecordID != NO_RECORD)
  {
    ROUTESKey0.recordID = m_RouteRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
      m_RouteRecordID = NO_RECORD;
  }
//
//  Service
//
  m_ServiceRecordID = GetPrivateProfileInt((LPSTR)userName, "Service", (int)(NO_RECORD), (LPSTR)szDatabaseFileName);
  if(m_ServiceRecordID != NO_RECORD)
  {
    SERVICESKey0.recordID = m_ServiceRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
      m_ServiceRecordID = NO_RECORD;
  }
//
//  Are they ok?

  if(m_RouteRecordID != NO_RECORD && m_ServiceRecordID != NO_RECORD)
  {
    ltoa(m_RouteRecordID, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"Route", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
    ltoa(m_ServiceRecordID, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"Service", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
    ROUTESKey0.recordID = m_RouteRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(m_RouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(m_RouteNumber, ROUTES_NUMBER_LENGTH);
    strncpy(m_RouteName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(m_RouteName, ROUTES_NAME_LENGTH);
    SERVICESKey0.recordID = m_ServiceRecordID;
    btrieve(B_GETEQUAL,TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(m_ServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(m_ServiceName, SERVICES_NAME_LENGTH);
    m_ServiceNumber = SERVICES.number;
  }
//
//  Set up the division and make sure it's valid
//
  m_DivisionRecordID = GetPrivateProfileInt((LPSTR)userName, "Division", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  if(m_DivisionRecordID != NO_RECORD)
  {
    DIVISIONSKey0.recordID = m_DivisionRecordID;
    if(btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0) != 0)
      m_DivisionRecordID = NO_RECORD;
  }
  if(m_DivisionRecordID != NO_RECORD)
  {
    ltoa(m_DivisionRecordID, tempString, 10);
    WritePrivateProfileString((LPSTR)userName, (LPSTR)"Division", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
    DIVISIONSKey0.recordID = m_DivisionRecordID;
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(m_DivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(m_DivisionName, DIVISIONS_NAME_LENGTH);
    m_DivisionNumber = DIVISIONS.number;
  }
//
//  Set up the roster week and make sure it's valid
//
  m_RosterWeek = GetPrivateProfileInt((LPSTR)userName, "RosterWeek", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  if(m_RosterWeek != NO_RECORD)
  {
    if(m_RosterWeek < 0 || m_RosterWeek > ROSTER_MAX_WEEKS - 1)
    {
      m_RosterWeek = NO_RECORD;
    }
  }
//
//  Set up the BASE pattern recordID
//
  strncpy(PATTERNNAMESKey1.name, basePatternName, PATTERNNAMES_NAME_LENGTH);
  rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  if(rcode2 != 0)
  {
    PATTERNNAMES.recordID = 1;
    strncpy(PATTERNNAMES.name, BASE_PATTERN_NAME, PATTERNNAMES_NAME_LENGTH);
    btrieve(B_INSERT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
    basePatternRecordID = 1;
  }
  else
  {
    basePatternRecordID = PATTERNNAMES.recordID;
  }
//
//  Get the list of garages
//
//  Set up any garages
//
  numGaragesInGarageList = 0;
  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0 && (NODES.flags & NODES_FLAG_GARAGE))
  {
    garageList[numGaragesInGarageList++] = NODES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
/*
//
//  Make the date adjustment on the DAILY.DOPS.Bus to reflect the assignment
//
  DAILYOPSKey2.flags = DAILYOPS_FLAG_BUS;
  DAILYOPSKey2.dateOfEntry = NO_RECORD;
  DAILYOPSKey2.timeOfEntry = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey2, 2);
  if(rcode2 == 0 && (DAILYOPS.flags & DAILYOPS_FLAG_BUS))
  {
    if(DAILYOPS.DOPS.Bus.date == 0)  // Make sure this is only done once
    {
      btrieve(B_STAT, TMS_DAILYOPS, &BSTAT, dummy, 0);
      numRecords = (int)BSTAT.numRecords;
      StatusBarStart(hWndMain, "Status");
      StatusBarText("Adjusting DailyOps Vehicle Assignment Dates");
      nI = 1;
      while(rcode2 == 0 &&
            (DAILYOPS.flags & DAILYOPS_FLAG_BUS))
      {
        StatusBar((long)nI++, (long)numRecords);
        if(DAILYOPS.DOPS.Bus.date == 0)
        {
          DAILYOPS.DOPS.Bus.date = DAILYOPS.dateOfEntry;
          rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey2, 2);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey2, 2);
      }
      StatusBarEnd();
    }
  }
*/
//
//  Set up standard max and min layover
//
  StandardLayover.minimum.type = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMinType", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  StandardLayover.minimum.minutes = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMinMinutes", (int)0, (LPSTR)szDatabaseFileName);
  StandardLayover.minimum.percent = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMinPercent", (int)0, (LPSTR)szDatabaseFileName);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"layoverMinLower", szTRUE,
        tempString, TEMPSTRING_LENGTH, (LPSTR)szDatabaseFileName);
  StandardLayover.minimum.lower = strcmp(tempString, szTRUE) == 0;
  StandardLayover.maximum.type = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMaxType", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  StandardLayover.maximum.minutes = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMaxMinutes", (int)0, (LPSTR)szDatabaseFileName);
  StandardLayover.maximum.percent = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"layoverMaxPercent", (int)0, (LPSTR)szDatabaseFileName);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"layoverMaxLower", szTRUE,
        tempString, TEMPSTRING_LENGTH, (LPSTR)szDatabaseFileName);
  StandardLayover.maximum.lower = strcmp(tempString, szTRUE) == 0;
//
//  Set up dropback max and min layover
//
  DropbackLayover.minimum.type = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMinType", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  DropbackLayover.minimum.minutes = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMinMinutes", (int)0, (LPSTR)szDatabaseFileName);
  DropbackLayover.minimum.percent = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMinPercent", (int)0, (LPSTR)szDatabaseFileName);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"DropbackLayoverMinLower", szTRUE,
        tempString, TEMPSTRING_LENGTH, (LPSTR)szDatabaseFileName);
  DropbackLayover.minimum.lower = strcmp(tempString, szTRUE) == 0;
  DropbackLayover.maximum.type = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMaxType", (int)NO_RECORD, (LPSTR)szDatabaseFileName);
  DropbackLayover.maximum.minutes = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMaxMinutes", (int)0, (LPSTR)szDatabaseFileName);
  DropbackLayover.maximum.percent = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"DropbackLayoverMaxPercent", (int)0, (LPSTR)szDatabaseFileName);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"DropbackLayoverMaxLower", szTRUE,
        tempString, TEMPSTRING_LENGTH, (LPSTR)szDatabaseFileName);
  DropbackLayover.maximum.lower = strcmp(tempString, szTRUE) == 0;
//
//  Set up the default time of day format
//
  timeFormat = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"timeFormat", (int)PREFERENCES_APX, (LPSTR)szDatabaseFileName);
//
//  Time wrap
//
  m_bWrapTime = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"timeWrap", (int)0, (LPSTR)szDatabaseFileName);
//
//  Set up the default time of day format
//
  timeHHMMFormat = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"timeHHMMFormat", (int)PREFERENCES_HHMM, (LPSTR)szDatabaseFileName);
//
//  Set up the default distance measure
//
  distanceMeasure = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"distanceMeasure", (int)PREFERENCES_MILES, (LPSTR)szDatabaseFileName);
//
//  What's the default bustype?
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"defaultBustype", "",
        tempString, TEMPSTRING_LENGTH, (LPSTR)szDatabaseFileName);
  defaultBustype = strcmp(tempString, "") == 0 ? NO_RECORD : atol(tempString);
//
//  Set up the roster format
//
  rosterFormat = GetPrivateProfileInt((LPSTR)userName,
        (LPSTR)"RosterFormat", (int)PREFERENCES_ROSTERCOMPRESSED, (LPSTR)szDatabaseFileName);
//
//  Update the user name
//
  WritePrivateProfileString("Description", "LastUpdatedBy",
        (LPSTR)userName, (LPSTR)szDatabaseFileName);
//
//  Flag the roster for update in GetRosterElements
//
  rosterChanged = TRUE;
//
//  Write out Long description
//
  pEditLONGDESCRIPTION->GetWindowText(szDatabaseDescription, sizeof(szDatabaseDescription));
  WritePrivateProfileString("Description", "LongDescription",
        szDatabaseDescription, (LPSTR)szDatabaseFileName);
//
//  Write out Short description
//
  pEditSHORTDESCRIPTION->GetWindowText(szShortDatabaseDescription, sizeof(szShortDatabaseDescription));
  WritePrivateProfileString("Description", "ShortDescription",
        szShortDatabaseDescription, (LPSTR)szDatabaseFileName);
//
//  Write out Effective Date
//
  COleVariant v;
  long day;
  long month;
  long year;
  long tempLong;

  v = pDTPickerEFFECTIVEDATE->GetYear();
  year = v.lVal;
  v = pDTPickerEFFECTIVEDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerEFFECTIVEDATE->GetDay();
  day = v.lVal;
  tempLong = year * 10000 + month * 100 + day;
  ltoa(tempLong, tempString, 10);
  WritePrivateProfileString("Description", "EffectiveDateAsNumber",
        tempString, (LPSTR)szDatabaseFileName);

  effectiveDate = tempLong;
  CTime ed(year, month, day, 0, 0, 0);

  s = ed.Format(_T("%B %d, %Y"));
  strcpy(szEffectiveDate, s);
  WritePrivateProfileString("Description", "EffectiveDate", szEffectiveDate, (LPSTR)szDatabaseFileName);
//
//  And the Until Date
//
  pDTPickerUNTILDATE->GetWindowText(s);
  v = pDTPickerUNTILDATE->GetYear();
  year = v.lVal;
  v = pDTPickerUNTILDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerUNTILDATE->GetDay();
  day = v.lVal;
  tempLong = year * 10000 + month * 100 + day;
  ltoa(tempLong, tempString, 10);
  WritePrivateProfileString("Description", "UntilDateAsNumber",
        tempString, (LPSTR)szDatabaseFileName);
  untilDate = tempLong;
  
  CTime ud(year, month, day, 0, 0, 0);

  s = ud.Format(_T("%B %d, %Y"));
  strcpy(szUntilDate, s);
  WritePrivateProfileString("Description", "UntilDate", szUntilDate, (LPSTR)szDatabaseFileName);
//
//  If we created TRIPS.B60, go through and adjust all the block sequences
//
  if(bCreatedTRIPSB60)
  {
//
//  Set up the status bar
//
    StatusBarStart(hWndMain, "Status");
    StatusBarText("Adjusting block sequence in new-style TRIPS table");
    StatusBarEnableCancel(FALSE);
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    nI = 0;
    while(rcode2 == 0)
    {
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
      TRIPS.standard.blockSequence = GTResults.firstNodeTime;
      rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      StatusBar((long)nI++, (long)numRecords);
    }
    StatusBarEnableCancel(TRUE);
    StatusBarEnd();
  }
//
//  Make sure we don't have any bizarre times in the Complaints File
//
  long hours, minutes, seconds;

  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  rcode2 = btrieve(B_GETFIRST, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  while(rcode2 == 0)
  {
    memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
    if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_FIXEDFUNKYTIME))
    {
      GetHMSBizarre(COMPLAINTS.timeOfComplaint, &hours, &minutes, &seconds);
      COMPLAINTS.timeOfComplaint = hours * 3600 + minutes * 60 + seconds;
      GetHMSBizarre(COMPLAINTS.timeOfOccurrence, &hours, &minutes, &seconds);
      COMPLAINTS.timeOfOccurrence = hours * 3600 + minutes * 60 + seconds;
      COMPLAINTS.flags |= COMPLAINTS_FLAG_FIXEDFUNKYTIME;
      memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
      rcode2 = btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  }
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
//
//  Check that Lat/Long is registered correctly
//
  BOOL bOddLookingLL = FALSE;
  BOOL bOddLookingEN = FALSE;

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(m_bUseLatLong)
    {
      if(NODES.latitude > 90 || fabs(NODES.longitude) > 180)
      {
        bOddLookingLL = TRUE;
        break;
      }
    }
    else
    {
      if(NODES.latitude < 1000 || NODES.longitude < 1000)
      {
        bOddLookingEN = TRUE;
        break;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
  if(bOddLookingLL || bOddLookingEN)
  {
    s.LoadString((bOddLookingLL ? ERROR_351 : ERROR_352));
    if(MessageBox(s, TMS, MB_YESNO) == IDYES)
    {
      if(bOddLookingLL)
      {
        m_bUseLatLong = FALSE;
      }
      else
      {
        m_bUseLatLong = TRUE;
      }
    }

  }
//
//  Set the right mapping mode
//
  CTMSApp *app = (CTMSApp *)AfxGetApp();

  app->WriteProfileInt("Mapping", "UseLatLong", m_bUseLatLong);
//
//  Load the fare data (if it exists)
//
//  Today
//
  CTime today = CTime::GetCurrentTime();
  long  dateToUse = today.GetYear() * 10000 + today.GetMonth() * 100 + today.GetDay();
//
//  Clear the list of fare types
//
  for(nI = 0; nI < RIDERSHIP_MAXFARETYPES; nI++)
  {
    strcpy(m_szFareTypes[nI], "");
  }
//
//  Build the list of fare types
//
  BOOL bFound;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPSKey1.pertainsToDate = 0;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
         DAILYOPS.pertainsToDate <= dateToUse)
  {
    if(dateToUse >= DAILYOPS.pertainsToDate &&
          (DAILYOPS.recordFlags & DAILYOPS_FLAG_SETFARETYPE))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        nI = DAILYOPS.DOPS.Ridership.data.FT.index;
        strncpy(tempString, DAILYOPS.DOPS.Ridership.data.FT.FareType, RIDERSHIP_FARETYPE_LENGTH);
        trim(tempString, RIDERSHIP_FARETYPE_LENGTH);
        strcpy(m_szFareTypes[nI], tempString);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Count 'em up
//
  m_numFareTypes = 0;
  for(nI = 0; nI < RIDERSHIP_MAXFARETYPES; nI++)
  {
    if(strcmp(m_szFareTypes[nI], "") != 0)
    {
      m_numFareTypes++;
    }
  }
//
//  Clear out the fare structure
//
  for(nI = 0; nI < MAXFARETYPEROUTES; nI++)
  {
    m_FareTypesByRoute[nI].ROUTESrecordID = NO_RECORD;
    for(nJ = 0; nJ < RIDERSHIP_MAXFARETYPES; nJ++)
    {
      m_FareTypesByRoute[nI].fareValue[nJ] = 0.0;
    }
  }
  m_numFareTypesByRoute = 0;
//
//  Build the list of fares (start at the beginning of time)
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPSKey1.pertainsToDate = 0;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
         DAILYOPS.pertainsToDate <= dateToUse)
  {
    if(dateToUse >= DAILYOPS.pertainsToDate &&
          (DAILYOPS.recordFlags & DAILYOPS_FLAG_SETFARE))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        nI = DAILYOPS.DOPS.Ridership.data.F.index;
        for(bFound = FALSE, nJ = 0; nJ < m_numFareTypesByRoute; nJ++)
        {
          if(m_FareTypesByRoute[nJ].ROUTESrecordID == DAILYOPS.DOPS.Ridership.data.F.ROUTESrecordID)
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
        {
          nJ = m_numFareTypesByRoute;
          m_FareTypesByRoute[nJ].ROUTESrecordID = DAILYOPS.DOPS.Ridership.data.F.ROUTESrecordID;
        }
        m_FareTypesByRoute[nJ].fareValue[nI] = DAILYOPS.DOPS.Ridership.data.F.value;
        if(!bFound)
        {
          m_numFareTypesByRoute++;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Set up the DailyOps semaphore
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);

  m_LastDAILYOPSRecordID = (rcode2 == 0 ? DAILYOPS.recordID : NO_RECORD);

/*
//
//  One-time fix to flip directions on selected routes for GRATA
//
//  long ROUTErecordIDs[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 13, 42, NO_RECORD};
//  long ROUTErecordIDs[] = {11, NO_RECORD};
  long ROUTErecordIDs[] = {1, NO_RECORD};
//
//  Go through all the routes
//
  StatusBarStart(NULL, "Flipping directions");
  nI = 0;
  while(ROUTErecordIDs[nI] != NO_RECORD)
  {
    StatusBar((long)nI, long(3));
//
//  Flip the directions on the Routes Table
//
    ROUTESKey0.recordID = ROUTErecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 == 0)
    {
      tempLong = ROUTES.DIRECTIONSrecordID[0];
      ROUTES.DIRECTIONSrecordID[0] = ROUTES.DIRECTIONSrecordID[1];
      ROUTES.DIRECTIONSrecordID[1] = tempLong;
      rcode2 = btrieve(B_UPDATE, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    }
//
//  Flip the directions on the Patterns
//
    long newDirectionIndex[] = {2, 0, 1};

    for(nJ = 0; nJ < 3; nJ++)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      while(rcode2 == 0)
      {
        rcode2 =btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
        while(rcode2 == 0)
        {
          if(PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nJ)
          {
            PATTERNS.directionIndex = newDirectionIndex[nJ];
            rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      }
    }
//
//  Flip the directions on the Trips
//
    for(nJ = 0; nJ < 3; nJ++)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      while(rcode2 == 0)
      {
        rcode2 =btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        while(rcode2 == 0)
        {
          if(TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nJ)
          {
            TRIPS.directionIndex = newDirectionIndex[nJ];
            rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      }
    }
//
//  Move on to the next route
//
    nI++;
  }
  StatusBarEnd();
*/
/*
//
//  Hardwire mebourne lat/longs
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    NODES.latitude = (float)-37.6733;
    NODES.longitude = (float)144.8433;
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
/*
//
//  Onetime fix - Barbados - Set all drivers to Full Time and clear the initials field
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS,&DRIVERSKey0, 0);
  while(rcode2 == 0)
  {
    DRIVERS.DRIVERTYPESrecordID = 1;
    memset(&DRIVERS.initials, ' ', DRIVERS_INITIALS_LENGTH);
    rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  }
*/
/*
//
//  Onetime fix - Ocean County - Read the stops file for lat/longs
//
//
//  Open the text file
//
  FILE *fp;

  fp = fopen("Stops.TXT", "r");
  if(fp == NULL)
  {
    MessageBox("Failed to open Stops.TXT", TMS, MB_OK);
  }
//
//  Read from the input file and write to the btrieve file
//
  else
  {
    char node[16], lon[16], lat[16];

    while(fscanf(fp, "%s %s %s", &node, &lon, &lat) > 0)
    {
      strcpy(NODESKey2.abbrName, node);
      pad(NODESKey2.abbrName, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
      if(rcode2 == 0)
      {
        NODES.longitude = (float)atof(lon);
        NODES.latitude = (float)atof(lat);
        rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey2, 2);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
    }
  }
*/
/*
//
//  Onetime fix - GBTA - Merge Routes
//
  long oldROUTESrecordID[] = { 2, 33, 11, 14, 16, 18, 24, 26, 28, 36, 39, 41, 43, 49};
  long newROUTESrecordID[] = { 1, 34, 12, 13, 15, 17, 23, 25, 27, 37, 38, 40, 42, 48};
  int  numToChange = 14;
//
//  Cycle through
//
  for(nI = 0; nI < numToChange; nI++)
  {
//
//  Patterns
//
    rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    while(rcode2 == 0)
    {
      if(PATTERNS.ROUTESrecordID == oldROUTESrecordID[nI])
      {
        PATTERNS.ROUTESrecordID = newROUTESrecordID[nI];
        PATTERNS.directionIndex = 1;
        rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    }
//
//  Connections
// 
    BOOL bDidUpdate;

    rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    while(rcode2 == 0)
    {
      bDidUpdate = FALSE;
      if(CONNECTIONS.fromROUTESrecordID == oldROUTESrecordID[nI])
      {
        CONNECTIONS.fromROUTESrecordID = newROUTESrecordID[nI];
        bDidUpdate = TRUE;
      }
      if(CONNECTIONS.toROUTESrecordID == oldROUTESrecordID[nI])
      {
        CONNECTIONS.toROUTESrecordID = newROUTESrecordID[nI];
        bDidUpdate = TRUE;
      }
      if(bDidUpdate)
      {
        rcode2 = btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    }
//
//  Trips
//
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    while(rcode2 == 0)
    {
      if(TRIPS.ROUTESrecordID == oldROUTESrecordID[nI])
      {
        TRIPS.ROUTESrecordID = newROUTESrecordID[nI];
        TRIPS.directionIndex = 1;
        rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
  }
*/
/*
//
//  Onetime fix - change the "Number" field on stops to the stop number for Thunder Bay
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      NODES.number = atol(tempString);
    }
    else
    {
      NODES.number = 0;
    }
    NODES.OBStopNumber = 0;
    NODES.IBStopNumber = 0;
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - Hardwire Ft McMurray lat/longs
//
/*
  double ll = 0.01;

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    NODES.latitude = (float)56.733001709 + ll;
    NODES.longitude = (float)-113.383003235 + ll;
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
    ll += 0.01;
  }
*/
/*
//
//  Onetime fix - move the description to the intersection field for Ft. Mac
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    strncpy(NODES.intersection, NODES.description, NODES_DESCRIPTION_LENGTH);
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
/*
//
//  Onetime fix - delete all stops out of the nodes and patterns tables for GBTA
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      rcode2 = btrieve(B_DELETE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
  rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  while(rcode2 == 0)
  {
    if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
    {
      rcode2 = btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  }
*/
//
//  Onetime fix - change the long stop names to Stopnnnn for GBTA
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strncpy(NODES.longName, "Stop", 4);
      strncpy(&NODES.longName[4], NODES.abbrName, 4);
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - make all piece 0s piece 1s for position holder runs
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
  while(rcode2 == 0)
  {
    if(CREWONLY.pieceNumber == 0)
    {
      CREWONLY.pieceNumber = 1;
      rcode2 = btrieve(B_UPDATE, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
  }
*/
//
//  Onetime fix - change all the NODES.JURISDICTIONSrecordIDs to -1 on non-stops for ECan
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      NODES.JURISDICTIONSrecordID = NO_RECORD;
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
/*
//
//  Onetime fix - realign nodes records, patterns, connections
//
//  CBS Database
//
//  long oldRecordIDs[] = {240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};
//  long newRecordIDs[] = {260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275};
//  int  numToChange = 16;
//
//  RedBus Database
//
//  long oldRecordIDs[] = {240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257};
//  long newRecordIDs[] = {276, 277, 278, 261, 264, 279, 280, 281, 282, 266, 283, 284, 275, 272, 273, 274, 270, 271};
//  int  numToChange = 18;

  StatusBarStart(hWndMain, "Converting...");
  for(nI = 0; nI < numToChange; nI++)
  {
//
//  Nodes
//
    NODESKey0.recordID = oldRecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    StatusBarText(tempString);
    for(nJ = 0; nJ < 4; nJ++)
    {
      NODES.abbrName[nJ] = tolower(NODES.abbrName[nJ]);
    }
    NODES.recordID = newRecordIDs[nI];
    rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Patterns
//
    rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    while(rcode2 == 0)
    {
      if(PATTERNS.NODESrecordID == oldRecordIDs[nI])
      {
        PATTERNS.NODESrecordID = newRecordIDs[nI];
        rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    }
//
//  Connections
//
    rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    while(rcode2 == 0)
    {
      if(CONNECTIONS.fromNODESrecordID == oldRecordIDs[nI] ||
            CONNECTIONS.toNODESrecordID == oldRecordIDs[nI])
      {
        if(CONNECTIONS.fromNODESrecordID == oldRecordIDs[nI])
        {
          CONNECTIONS.fromNODESrecordID = newRecordIDs[nI];
        }
        if(CONNECTIONS.toNODESrecordID == oldRecordIDs[nI])
        {
          CONNECTIONS.toNODESrecordID = newRecordIDs[nI];
        }
        rcode2 = btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    }
  }
//
//  Go back and delete the old nodes
//
  for(nI = 0; nI < numToChange; nI++)
  {
    NODESKey0.recordID = oldRecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_DELETE, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Fix the names of the new ones
//
  for(nI = 0; nI < numToChange; nI++)
  {
    NODESKey0.recordID = newRecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    StatusBarText(tempString);
    for(nJ = 0; nJ < 4; nJ++)
    {
      NODES.abbrName[nJ] = toupper(NODES.abbrName[nJ]);
    }
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
  }
  StatusBarEnd();
*/
//
//  Onetime fix - get the last recordIDs in PATTERNS, PATTERNNAMES, CONNECTIONS, and TRIPS
//
/*
  rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  rcode2 = btrieve(B_GETLAST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
  rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  rcode2 = btrieve(B_GETLAST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
*/
//
//  Onetime fix - realign routes, patterns, connections, and trips
//
/*
  ROUTESDef R;
  long oldRecordIDs[] = {49, 65, 67, 76, 77, 78, 90, 96, 111, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122};
  int  numToChange = 19;

  StatusBarStart(hWndMain, "Converting...");
  StatusBarText("Routes");
  for(nI = 0; nI < numToChange; nI++)
  {
    ROUTESKey0.recordID = oldRecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    memcpy(&R, &ROUTES, sizeof(ROUTESDef));
    rcode2 = btrieve(B_DELETE, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    R.recordID += 1000;
    rcode2 = btrieve(B_INSERT, TMS_ROUTES, &R, &ROUTESKey0, 0);
  }
//
//  Patterns
//
  StatusBarText("Patterns");
  rcode2 = btrieve(B_GETFIRST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  while(rcode2 == 0)
  {
    PATTERNS.ROUTESrecordID += 1000;
    rcode2 = btrieve(B_UPDATE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
  }
//
//  Connections
//
  StatusBarText("Connections");
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  while(rcode2 == 0)
  {
    if(CONNECTIONS.fromROUTESrecordID != NO_RECORD)
    {
      CONNECTIONS.fromROUTESrecordID += 1000;
    }
    if(CONNECTIONS.toROUTESrecordID != NO_RECORD)
    {
      CONNECTIONS.toROUTESrecordID += 1000;
    }
    rcode2 = btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
//
//  Trips
//
  StatusBarText("Trips");
  rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  while(rcode2 == 0)
  {
    TRIPS.ROUTESrecordID += 1000;
    TRIPS.standard.blockNumber = 0;
    TRIPS.standard.BLOCKNAMESrecordID = NO_RECORD;
    TRIPS.standard.POGNODESrecordID = NO_RECORD;
    TRIPS.standard.PIGNODESrecordID = NO_RECORD;
    TRIPS.standard.RGRPROUTESrecordID = NO_RECORD;
    TRIPS.standard.SGRPSERVICESrecordID = NO_RECORD;
    TRIPS.standard.assignedToNODESrecordID = NO_RECORD;
    rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
  }
  StatusBarEnd();
*/
//
//  Onetime fix = load the routes, patterns, pattern names, connections, and trips files
//
/*
typedef struct LoadROUTESStruct
{
  char junk[3];
  ROUTESDef ROUTES;
  char trailer[2];
} LoadROUTESDef;

typedef struct LoadPATTERNSStruct
{
  char junk[3];
  PATTERNSDef PATTERNS;
  char trailer[2];
} LoadPATTERNSDef;

typedef struct LoadPATTERNNAMESSStruct
{
  char junk[3];
  NORMNAMESDef PATTERNNAMES;
  char trailer[2];
} LoadPATTERNNAMESDef;

typedef struct LoadCONNECTIONSStruct
{
  char junk[3];
  CONNECTIONSDef CONNECTIONS;
  char trailer[2];
} LoadCONNECTIONSDef;

typedef struct LoadTRIPSStruct
{
  char junk[4];
  TRIPSDef TRIPS;
  char trailer[2];
} LoadTRIPSDef;
  
  LoadROUTESDef       LR;
  LoadPATTERNSDef     LP;
  LoadPATTERNNAMESDef LPN;
  LoadCONNECTIONSDef  LC;
  LoadTRIPSDef        LT;

  long lastPAT  = 33388;
  long lastPATN = 717;
  long lastCON  = 6323;
  long lastTRI  = 25314;

  FILE  *pFile;
  size_t result;

  StatusBarStart(hWndMain, "Loading...");
//
//  Do the routes
//
  StatusBarText("Routes");
  pFile = fopen ("routes.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open routes.seq", TMS, MB_OK);
    goto done;
  }

  result = fread(&LR, sizeof(LR), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&ROUTES, &LR.ROUTES, sizeof(ROUTESDef));
    rcode2 = btrieve(B_INSERT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    result = fread(&LR, sizeof(LR), 1, pFile);
  }
  fclose(pFile);
//
//  Do the pattern namess
//
typedef struct NAMECOMPStruct
{
  long oldRecordID;
  long newRecordID;
} NAMECOMPDef;

  NAMECOMPDef NAMECOMP[1000];
  int numNAMECOMPS;

  StatusBarText("Pattern Names");
  pFile = fopen ("patternn.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open patternn.seq", TMS, MB_OK);
    goto done;
  }

  numNAMECOMPS = 0;
  result = fread(&LPN, sizeof(LPN), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&PATTERNNAMES, &LPN.PATTERNNAMES, sizeof(NORMNAMESDef));
    NAMECOMP[numNAMECOMPS].oldRecordID = PATTERNNAMES.recordID;
    strncpy(PATTERNNAMESKey1.name, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
    NAMECOMP[numNAMECOMPS].newRecordID = (rcode2 == 0 ? PATTERNNAMES.recordID : NAMECOMP[numNAMECOMPS].oldRecordID + lastPATN);
    if(rcode2 != 0)
    {
      memcpy(&PATTERNNAMES, &LPN.PATTERNNAMES, sizeof(NORMNAMESDef));
      PATTERNNAMES.recordID = NAMECOMP[numNAMECOMPS].newRecordID;
      rcode2 = btrieve(B_INSERT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
    }
    numNAMECOMPS++;
    result = fread(&LPN, sizeof(LPN), 1, pFile);
  }
  fclose(pFile);
//
//  Do the patterns
//
  StatusBarText("Patterns (1)");
  pFile = fopen ("patterns.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open patterns.seq (1)", TMS, MB_OK);
    goto done;
  }
//
//  Get a list of the route/ser/dirs in the file
//
typedef struct UniqueStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
} UniqueDef;

  UniqueDef Unique[1000];
  int  numUnique;

  numUnique = 0;
  result = fread(&LP, sizeof(LP), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&PATTERNS, &LP.PATTERNS, sizeof(PATTERNSDef));
    for(bFound = FALSE, nI = 0; nI < numUnique; nI++)
    {
      if(PATTERNS.ROUTESrecordID == Unique[nI].ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == Unique[nI].SERVICESrecordID &&
            PATTERNS.directionIndex == Unique[nI].directionIndex)
      {
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
    {
      Unique[numUnique].ROUTESrecordID = PATTERNS.ROUTESrecordID;
      Unique[numUnique].SERVICESrecordID = PATTERNS.SERVICESrecordID;
      Unique[numUnique].directionIndex = PATTERNS.directionIndex;
      numUnique++;
    } 
    result = fread(&LP, sizeof(LP), 1, pFile);
  }
//
//  Delete the existing patterns on this route/ser/dir
//
  for(nI = 0; nI < numUnique; nI++)
  {
    PATTERNSKey2.ROUTESrecordID = Unique[nI].ROUTESrecordID;
    PATTERNSKey2.SERVICESrecordID = Unique[nI].SERVICESrecordID;
    PATTERNSKey2.directionIndex = Unique[nI].directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
    PATTERNSKey2.nodeSequence = NO_RECORD;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == Unique[nI].ROUTESrecordID &&
          PATTERNS.SERVICESrecordID == Unique[nI].SERVICESrecordID &&
          PATTERNS.directionIndex == Unique[nI].directionIndex)
    {
      btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
  }
  fclose(pFile);
//
//  Pass 2 - Load the patterns
//
  StatusBarText("Patterns (2)");
  pFile = fopen ("patterns.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open patterns.seq (2)", TMS, MB_OK);
    goto done;
  }
  result = fread(&LP, sizeof(LP), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&PATTERNS, &LP.PATTERNS, sizeof(PATTERNSDef));
    PATTERNS.recordID += lastPAT;
    for(nI = 0; nI < numNAMECOMPS; nI++)
    {
      if(PATTERNS.PATTERNNAMESrecordID == NAMECOMP[nI].oldRecordID)
      {
        PATTERNS.PATTERNNAMESrecordID = NAMECOMP[nI].newRecordID;
        rcode2 = btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
        break;
      }
    }
    result = fread(&LP, sizeof(LP), 1, pFile);
  }
  fclose(pFile);
//
//  Do the connections
//
  StatusBarText("Connections");
  pFile = fopen ("connecti.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open connecti.seq", TMS, MB_OK);
    goto done;
  }

  result = fread(&LC, sizeof(LC), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&CONNECTIONS, &LC.CONNECTIONS, sizeof(CONNECTIONSDef));
    CONNECTIONS.recordID += lastCON;
    if(CONNECTIONS.fromPATTERNNAMESrecordID != NO_RECORD)
    {
      for(nI = 0; nI < numNAMECOMPS; nI++)
      {
        if(CONNECTIONS.fromPATTERNNAMESrecordID == NAMECOMP[nI].oldRecordID)
        {
          CONNECTIONS.fromPATTERNNAMESrecordID = NAMECOMP[nI].newRecordID;
          break;
        }
      }
    }
    if(CONNECTIONS.toPATTERNNAMESrecordID != NO_RECORD)
    {
      for(nI = 0; nI < numNAMECOMPS; nI++)
      {
        if(CONNECTIONS.toPATTERNNAMESrecordID == NAMECOMP[nI].oldRecordID)
        {
          CONNECTIONS.toPATTERNNAMESrecordID = NAMECOMP[nI].newRecordID;
          break;
        }
      }
    }
    rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    result = fread(&LC, sizeof(LC), 1, pFile);
  }
  fclose(pFile);
//
//  Do the trips
//
  StatusBarText("Trips");
  pFile = fopen ("trips.seq", "rb" );
  if(pFile == NULL)
  {
    MessageBox("Failed to open trips.seq", TMS, MB_OK);
    goto done;
  }

  result = fread(&LT, sizeof(LT), 1, pFile);
  while(!feof(pFile))
  {
    memcpy(&TRIPS, &LT.TRIPS, sizeof(TRIPSDef));
    TRIPS.recordID += lastTRI;
    for(nI = 0; nI < numNAMECOMPS; nI++)
    {
      if(TRIPS.PATTERNNAMESrecordID == NAMECOMP[nI].oldRecordID)
      {
        TRIPS.PATTERNNAMESrecordID = NAMECOMP[nI].newRecordID;
        rcode2 = btrieve(B_INSERT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        break;
      }
    }
    result = fread(&LT, sizeof(LT), 1, pFile);
  }
  fclose(pFile);

  done:
    StatusBarEnd();
*/
/*
//
//  Onetime fix - Grata - Get rid of leading blanks on the driver' badge number
//
  char *ptr;

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS,&DRIVERSKey0, 0);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    tempString[DRIVERS_BADGENUMBER_LENGTH] = '\0';
    ptr = tempString;
    nI = 0;
    while(*ptr == ' ')
    {
      nI++;
      if(nI > DRIVERS_BADGENUMBER_LENGTH)
      {
        break;
      }
      ptr++;
    }
    strcpy(DRIVERS.badgeNumber, ptr);
    pad(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  }
*/
//
//  Onetime fix - C-Tran - Split large stop numbers into OB and IB
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      NODES.OBStopNumber = NO_RECORD;
      NODES.IBStopNumber = NO_RECORD;
    }
    else
    {
      NODES.OBStopNumber = NODES.number / 10000;
      NODES.IBStopNumber = NODES.number - (NODES.OBStopNumber * 10000);
      NODES.number = NO_RECORD;
    }
    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - Lima - Put the AbbrName in front of the description
//  in the Nodes file for bus stops only.
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strncpy(szarString, NODES.description, NODES_DESCRIPTION_LENGTH);
      trim(szarString, NODES_DESCRIPTION_LENGTH);
      strcat(tempString, " - ");
      strcat(tempString, szarString);
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - Bridgeport - Add a stop record when there isn't one
//  that's quoted as a stop association
//
/*
  BOOL bDone;
  BOOL bGotOne;
  long recordID;
  long newRecordID;
  NODESDef N;
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      bDone = FALSE;
      bGotOne = FALSE;
      while(!bDone)
      {
        strcpy(tempString, "");
        if(NODES.number > 0)
        {
          sprintf(tempString, "%04d", NODES.number);
          bDone = TRUE;
        }
        else
        {
          if(NODES.OBStopNumber > 0 && !bGotOne)
          {
            sprintf(tempString, "%04d", NODES.OBStopNumber);
            bGotOne = TRUE;
          }
          else
          {
            if(NODES.IBStopNumber > 0)
            { 
              sprintf(tempString, "%04d", NODES.IBStopNumber);
            }
            bDone = TRUE;
          }
        }
        if(strcmp(tempString, "") == 0)
        {
          break;
        }
        recordID = NODES.recordID;
        strncpy(NODESKey2.abbrName, tempString, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &N, &NODESKey2, 2);
        if(rcode2 != 0)
        {
          memcpy(&N, &NODES, sizeof(NODESDef));
          rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
          newRecordID = AssignRecID(rcode2, NODES.recordID);
          N.recordID = newRecordID;
          strncpy(N.abbrName, tempString, NODES_ABBRNAME_LENGTH);
          strcpy(N.longName, "Stop");
          strncpy(&N.longName[4], tempString, NODES_ABBRNAME_LENGTH);
          N.flags |= NODES_FLAG_STOP;
          N.number = atol(tempString);
          N.OBStopNumber = NO_RECORD;
          N.IBStopNumber = NO_RECORD;
          rcode2 = btrieve(B_INSERT, TMS_NODES, &N, &NODESKey0, 0);
        }
        NODESKey0.recordID = recordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - set the vehicle type to "1" for Jerusalem Inter-City routes
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  while(rcode2 == 0 &&
        ROUTES.recordID < 12)
  {
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    while(rcode2 == 0)
    {
      TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
      TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
      TRIPSKey1.directionIndex = NO_RECORD;
      TRIPSKey1.tripSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
      while(rcode2 == 0 &&
            TRIPS.ROUTESrecordID == ROUTES.recordID &&
            TRIPS.SERVICESrecordID == SERVICES.recordID)
      {
        TRIPS.BUSTYPESrecordID = 1;
        rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  }
*/
//
//  Onetime fix - Expand connection times from general case
//                to general case + time-of-day breakdown (Jerusalem)
//
//  00:00 - 05:29 -20%
//  05:30 - 06:29	-18%  
//  06:30 - 08:59	+30%
//  09:00 - 12:29	-16%
//  12:30 - 15:29	-14%
//  15:30 - 18:29	+30%
//  18:30 - 23:59	-20%
//
/*
#define NUMRANGES 7

typedef struct RANGEStruct
{
  long  fromTime;
  long  toTime;
  float adjustment;
} RANGEDef;

  RANGEDef Ranges[NUMRANGES];
  long     lastRecordID, thisRecordID;
  long     thisConnectionTime;
  CONNECTIONSDef C;

  Ranges[0].fromTime = 0;
  Ranges[0].toTime = (5 * 3600) + (29 * 60);
  Ranges[0].adjustment = -20.0;

  Ranges[1].fromTime = (5 * 3600) + (30 * 60);
  Ranges[1].toTime = (6 * 3600) + (29 * 60);
  Ranges[1].adjustment = -18.0;

  Ranges[2].fromTime = (6 * 3600) + (30 * 60);
  Ranges[2].toTime = (8 * 3600) + (59 * 60);
  Ranges[2].adjustment = 30.0;

  Ranges[3].fromTime = (9 * 3600) + (0 * 60);
  Ranges[3].toTime = (12 * 3600) + (29 * 60);
  Ranges[3].adjustment = -16.0;

  Ranges[4].fromTime = (12 * 3600) + (30 * 60);
  Ranges[4].toTime = (15 * 3600) + (29 * 60);
  Ranges[4].adjustment = -14.0;

  Ranges[5].fromTime = (15 * 3600) + (30 * 60);
  Ranges[5].toTime = (18 * 3600) + (29 * 60);
  Ranges[5].adjustment = 30.0;

  Ranges[6].fromTime = (18 * 3600) + (30 * 60);
  Ranges[6].toTime = (23 * 3600) + (59 * 60);
  Ranges[6].adjustment = -20.0;

  rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  lastRecordID = CONNECTIONS.recordID;
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  while(rcode2 == 0)
  {
    if(thisRecordID == lastRecordID)
    {
      break;
    }
    thisRecordID = CONNECTIONS.recordID;
    if(CONNECTIONS.connectionTime != 0) // Equivalence
    {
      thisConnectionTime = CONNECTIONS.connectionTime;
      for(nI = 0; nI < NUMRANGES; nI++)
      {
        rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &C, &CONNECTIONSKey0, 0);
        CONNECTIONS.recordID = AssignRecID(rcode2, C.recordID);
        CONNECTIONS.fromTimeOfDay = Ranges[nI].fromTime;
        CONNECTIONS.toTimeOfDay = Ranges[nI].toTime;
        CONNECTIONS.connectionTime = thisConnectionTime +
              (long)((Ranges[nI].adjustment / 100.0) * thisConnectionTime);
        rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
      }
      CONNECTIONSKey0.recordID = thisRecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
*/ 
//
//  Onetime fix - clear extraboard values from the runs table
//
/*
  rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  while(rcode2 == 0)
  {
    RUNS.prior.startTime = NO_TIME;
    RUNS.prior.endTime = NO_TIME;
    RUNS.after.startTime = NO_TIME;
    RUNS.after.endTime = NO_TIME;
    rcode2 = btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  }
*/
//
//  Onetime fix - change all the "&&" to "&" in the NODES.intersection/description fields
//
/*
  char *ptr;

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(tempString, NODES_INTERSECTION_LENGTH);
    ptr = strstr(tempString, "&&");
    if(ptr)
    {
      strcpy(ptr, (ptr + 1));
      pad(tempString, NODES_INTERSECTION_LENGTH);
      strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
    trim(tempString, NODES_DESCRIPTION_LENGTH);
    ptr = strstr(tempString, "&&");
    if(ptr)
    {
      strcpy(ptr, (ptr + 1));
      pad(tempString, NODES_DESCRIPTION_LENGTH);
      strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
      rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
*/
//
//  Onetime fix - produce a ridiculous output file for Jerusalem
//
/*
  GetConnectionTimeDef GCTData;
  GenerateTripDef previousGTResults;
  TRIPSDef previousTRIPS;
  HFILE hfOutputFile;
  float distance;
  long  deadheadTime;
  long  SERVICErecordIDs[7] = {5, 11, 9, 7, 2, 3, 4};
  long  DIVISIONSrecordID, prevDIVISIONSrecordID;
  long  TRIPSrecordID;
  long  runNumber[2];
  char  szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
  char  szDirectionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  outputString[512];
  BOOL  bGotOne;
  BOOL  bPullin;
  int   nM;

  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\JTMT.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(szarString, TMS, MB_ICONSTOP);
    goto done;
  }

  for(nI = 0; nI < 7; nI++)
  {
    prevDIVISIONSrecordID = NO_RECORD;
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      for(nJ = 0; nJ < 2; nJ++)
      {
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          continue;
        }
//
//  Save the route number
//
        strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
//
//  Save the long direction name
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(szDirectionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(szDirectionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Cycle through the trips
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICErecordIDs[nI];
        TRIPSKey1.directionIndex = nJ;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICErecordIDs[nI] &&
              TRIPS.directionIndex == nJ)
        {
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID, TRIPS.directionIndex,
                TRIPS.PATTERNNAMESrecordID, TRIPS.timeAtMLP, 0, &GTResults);
          if(ROUTES.COMMENTSrecordID == 1 || ROUTES.COMMENTSrecordID == 6)
          {
            DIVISIONSrecordID = 1;
          }
          else
          {
            DIVISIONSrecordID = 2;
          }
//
//  If the division changed, read in the runs
//
          if(DIVISIONSrecordID != prevDIVISIONSrecordID)
          {
            TRIPSrecordID = TRIPS.recordID;
            GetRunRecords(DIVISIONSrecordID, SERVICErecordIDs[nI]);
            TRIPSKey0.recordID = TRIPSrecordID;
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            prevDIVISIONSrecordID = DIVISIONSrecordID;
          }
//
//  Check on a pull-out
//
          if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
          {
//
//  Trip number
//
            sprintf(szarString, "%ld\t", TRIPS.tripNumber);
            strcpy(outputString, szarString);
//
//  Spaces
//
            strcat(outputString, "\t\t\t\t\t\t\t");
//
//  Origin
//
            NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            sprintf(szarString, "%ld\t", NODES.number);
            strcat(outputString, szarString);
//
//  Destination
//
            NODESKey0.recordID = GTResults.firstNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            sprintf(szarString, "%ld\t", NODES.number);
            strcat(outputString, szarString);
//
//  Day
//
            sprintf(szarString, "%d\t", nI + 1);
            strcat(outputString, szarString);
//
//  Get the deadhead info
//
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
//
//  Departure time
//
            strcat(outputString, Tchar(GTResults.firstNodeTime - deadheadTime));
            strcat(outputString, "\t");
//
//  End time
//
            strcat(outputString, Tchar(GTResults.firstNodeTime));
            strcat(outputString, "\t");
//
//  Block number
//
            sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
            strcat(outputString, szarString);
//
//  Bus number
//
            sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
            strcat(outputString, szarString);
//
//  Get the run number(s) on the trip
//
            runNumber[0] = 0;
            runNumber[1] = 0;
            for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
            {
              if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
              {
                break;
              }
              if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                    (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                     GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                     GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
              {
                if(bGotOne)
                {
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  break;
                }
                else
                {
                  runNumber[0] = m_pRunRecordData[nM].runNumber;
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  bGotOne = TRUE;
                }
              }
            }
//
//  Run number(s)
//
            sprintf(szarString, "%ld", runNumber[0]);
            strcat(outputString, szarString);
//            if(runNumber[0] != runNumber[1] && runNumber[1] != 0)
//            {
//              sprintf(szarString, " -> %ld", runNumber[1]);
//              strcat(outputString, szarString);
//            }
//            strcat(outputString, "\t");
//
//  Route length
//
            sprintf(szarString, "%7.2f", distance);
            strcat(outputString, szarString);
            strcat(outputString, "\t");
//
//  Travel time
//
            strcat(outputString, chhmm(deadheadTime));
            strcat(outputString, "\r\n");
//
//  Write the record
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  Trip details
//
//  Trip number
//
          sprintf(szarString, "%ld\t", TRIPS.tripNumber);
          strcpy(outputString, szarString);
//
//  Route ID (Pattern name)
//
          PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
          btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          trim(szarString, PATTERNNAMES_NAME_LENGTH);
          strcat(outputString, szarString);
          strcat(outputString, "\t");
//
//  Display (Route number)
//
          strcat(outputString, szRouteNumber);
          strcat(outputString, "\t");
//
//  Alternative (last three digits of pattern name)
//
          strcat(outputString, &szarString[strlen(szarString) - 3]);
          strcat(outputString, "\t");
//
//  Direction
//
          strcat(outputString, szDirectionName);
          strcat(outputString, "\t");
//
//  Service Type
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICErecordIDs[0];
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(PATTERNS.COMMENTSrecordID == 3)
          {
            strcat(outputString, "Regular");
          }
          else if(PATTERNS.COMMENTSrecordID == 4)
          {
            strcat(outputString, "Express");
          }
          else if(PATTERNS.COMMENTSrecordID == 5)
          {
            strcat(outputString, "Direct");
          }
          strcat(outputString, "\t");
//
//  Line type
//
          if(ROUTES.COMMENTSrecordID == 1)
          {
            strcat(outputString, "Urban");
            DIVISIONSrecordID = 1;
          }
          else if(ROUTES.COMMENTSrecordID == 2)
          {
            strcat(outputString, "Inter-City");
            DIVISIONSrecordID = 2;
          }
          else if(ROUTES.COMMENTSrecordID == 6)
          {
            strcat(outputString, "Regional");
            DIVISIONSrecordID = 1;
          }
          strcat(outputString, "\t");
//
//  Bus type
//
          if(TRIPS.BUSTYPESrecordID == 1)
          {
            strcat(outputString, "Inter-City");
          }
          else if(TRIPS.BUSTYPESrecordID == 2)
          {
            strcat(outputString, "Urban");
          }
          strcat(outputString, "\t");
//
//  Origin
//
          NODESKey0.recordID = GTResults.firstNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          sprintf(szarString, "%ld\t", NODES.number);
          strcat(outputString, szarString);
//
//  Destination
//
          NODESKey0.recordID = GTResults.lastNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          sprintf(szarString, "%ld\t", NODES.number);
          strcat(outputString, szarString);
//
//  Day
//
          sprintf(szarString, "%d\t", nI + 1);
          strcat(outputString, szarString);
//
//  Departure time
//
          strcat(outputString, Tchar(GTResults.firstNodeTime));
          strcat(outputString, "\t");
//
//  End time
//
          strcat(outputString, Tchar(GTResults.lastNodeTime));
          strcat(outputString, "\t");
//
//  Block number
//
          sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
          strcat(outputString, szarString);
//
//  Bus number
//
          sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
          strcat(outputString, szarString);
//
//  Get the run number(s) on the trip
//
          runNumber[0] = 0;
          runNumber[1] = 0;
          for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
          {
            if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
            {
              break;
            }
            if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                  (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                   GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                   GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
            {
              if(bGotOne)
              {
                runNumber[1] = m_pRunRecordData[nM].runNumber;
                break;
              }
              else
              {
                runNumber[0] = m_pRunRecordData[nM].runNumber;
                runNumber[1] = m_pRunRecordData[nM].runNumber;
                bGotOne = TRUE;
              }
            }
          }
//
//  Run number(s)
//
          sprintf(szarString, "%ld", runNumber[0]);
          strcat(outputString, szarString);
//          if(runNumber[0] != runNumber[1] && runNumber[1] != 0)
//          {
//            sprintf(szarString, " -> %ld", runNumber[1]);
//            strcat(outputString, szarString);
//          }
//          strcat(outputString, "\t");
//
//  Route length
//
          sprintf(szarString, "%7.2f", GTResults.tripDistance);
          strcat(outputString, szarString);
          strcat(outputString, "\t");
//
//  Travel time
//
          strcat(outputString, chhmm(GTResults.lastNodeTime - GTResults.firstNodeTime));
          strcat(outputString, "\r\n");
//
//  Write the record
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Check on a pull-in
//
          bPullin = (TRIPS.standard.PIGNODESrecordID != NO_RECORD);
          if(bPullin)
          {
//
//  Trip number
//
            sprintf(szarString, "%ld\t", TRIPS.tripNumber);
            strcpy(outputString, szarString);
//
//  Spaces
//
            strcat(outputString, "\t\t\t\t\t\t\t");
//
//  Origin
//
            NODESKey0.recordID = GTResults.lastNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            sprintf(szarString, "%ld\t", NODES.number);
            strcat(outputString, szarString);
//
//  Destination
//
            NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            sprintf(szarString, "%ld\t", NODES.number);
            strcat(outputString, szarString);
//
//  Day
//
            sprintf(szarString, "%d\t", nI + 1);
            strcat(outputString, szarString);
//
//  Get the deadhead info
//
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
//
//  Departure time
//
            strcat(outputString, Tchar(GTResults.lastNodeTime));
            strcat(outputString, "\t");
//
//  End time
//
            strcat(outputString, Tchar(GTResults.lastNodeTime + deadheadTime));
            strcat(outputString, "\t");
//
//  Block number
//
            sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
            strcat(outputString, szarString);
//
//  Bus number
//
            sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
            strcat(outputString, szarString);
//
//  Get the run number(s) on the trip
//
            runNumber[0] = 0;
            runNumber[1] = 0;
            for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
            {
              if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
              {
                break;
              }
              if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                    (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                     GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                     GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
              {
                if(bGotOne)
                {
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  break;
                }
                else
                {
                  runNumber[0] = m_pRunRecordData[nM].runNumber;
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  bGotOne = TRUE;
                }
              }
            }
//
//  Run number(s)
//
            sprintf(szarString, "%ld", runNumber[0]);
            strcat(outputString, szarString);
//            if(runNumber[0] != runNumber[1] && runNumber[1] != 0)
//            {
//              sprintf(szarString, " -> %ld", runNumber[1]);
//              strcat(outputString, szarString);
//            }
//            strcat(outputString, "\t");
//
//  Route length
//
            sprintf(szarString, "%7.2f", distance);
            strcat(outputString, szarString);
            strcat(outputString, "\t");
//
//  Travel time
//
            strcat(outputString, chhmm(deadheadTime));
            strcat(outputString, "\r\n");
//
//  Write the record
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  Not a pullin on the trip - check the block for an interline deadhead
//
          if(!bPullin)
          {
            previousTRIPS = TRIPS;
            previousGTResults = GTResults;
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
            if(rcode2 == 0  &&
                  TRIPS.standard.RGRPROUTESrecordID == previousTRIPS.standard.RGRPROUTESrecordID &&
                  TRIPS.standard.SGRPSERVICESrecordID == SERVICErecordIDs[nI] &&
                  TRIPS.standard.blockNumber == previousTRIPS.standard.blockNumber)
            {
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID, TRIPS.directionIndex,
                    TRIPS.PATTERNNAMESrecordID, TRIPS.timeAtMLP, 0, &GTResults);
              if(previousGTResults.lastNODESrecordID != GTResults.firstNODESrecordID &&
                    !NodesEquivalent(previousGTResults.lastNODESrecordID, GTResults.firstNODESrecordID, &tempLong))
              {
//
//  Trip number
//
                sprintf(szarString, "%ld\t", previousTRIPS.tripNumber);
                strcpy(outputString, szarString);
//
//  Spaces
//
                strcat(outputString, "\t\t\t\t\t\t\t");
//
//  Origin
//
                NODESKey0.recordID = previousGTResults.lastNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                sprintf(szarString, "%ld\t", NODES.number);
                strcat(outputString, szarString);
//
//  Destination
//
                NODESKey0.recordID = GTResults.firstNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                sprintf(szarString, "%ld\t", NODES.number);
                strcat(outputString, szarString);
//
//  Day
//
                sprintf(szarString, "%d\t", nI + 1);
                strcat(outputString, szarString);
//
//  Get the deadhead info
//
                GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                GCTData.fromNODESrecordID = previousGTResults.lastNODESrecordID;
                GCTData.toNODESrecordID = GTResults.firstNODESrecordID;;
                GCTData.timeOfDay = previousGTResults.lastNodeTime;
                deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
//
//  Departure time
//
                strcat(outputString, Tchar(previousGTResults.lastNodeTime));
                strcat(outputString, "\t");
//
//  End time
//
                strcat(outputString, Tchar(previousGTResults.lastNodeTime + deadheadTime));
                strcat(outputString, "\t");
//
//  Block number
//
                sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
                strcat(outputString, szarString);
//
//  Bus number
//
                sprintf(szarString, "%ld\t", TRIPS.standard.blockNumber);
                strcat(outputString, szarString);
//
//  Get the run number(s) on the trip
//
                runNumber[0] = 0;
                runNumber[1] = 0;
                for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
                {
                  if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
                  {
                    break;
                  }
                  if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                        (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                         GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                         GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
                  {
                    if(bGotOne)
                    {
                      runNumber[1] = m_pRunRecordData[nM].runNumber;
                      break;
                    }
                    else
                    {
                      runNumber[0] = m_pRunRecordData[nM].runNumber;
                      runNumber[1] = m_pRunRecordData[nM].runNumber;
                      bGotOne = TRUE;
                    }
                  }
                }
//
//  Run number(s)
//
                sprintf(szarString, "%ld", runNumber[0]);
                strcat(outputString, szarString);
//                if(runNumber[0] != runNumber[1] && runNumber[1] != 0)
//                {
//                  sprintf(szarString, " -> %ld", runNumber[1]);
//                  strcat(outputString, szarString);
//                }
//                strcat(outputString, "\t");
//
//  Route length
//
                sprintf(szarString, "%7.2f", distance);
                strcat(outputString, szarString);
                strcat(outputString, "\t");
//
//  Travel time
//
                strcat(outputString, chhmm(deadheadTime));
                strcat(outputString, "\r\n");
//
//  Write the record
//
                _lwrite(hfOutputFile, outputString, strlen(outputString));
              }  // different nodes
            }  // rcode2
//
//  And reposition
//
            TRIPSKey0.recordID = previousTRIPS.recordID;
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }  //  not a pullin
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // on TRIPS
      }  // nJ 
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }  // on ROUTES
  }  // nI
  done:
    _lclose(hfOutputFile);
*/
//
//  Onetime fix - compare and update NODES.B60 entries and the stop file from Connexionz
//
//  Open the text file
//
/*
  FILE *fpi, *fpo;
  long stopNumber;
  char szStopNumber[NODES_ABBRNAME_LENGTH + 1];
  char outputString[256];
  char szAddress[NODES_INTERSECTION_LENGTH + 1];
  long platformNumber;
  float latitude, longitude;
  BOOL bUpdate;

  fpi = fopen("Updates.TXT", "r");
  if(fpi == NULL)
  {
    MessageBox("Failed to open Updates.TXT", TMS, MB_OK);
  }
  else
  {
    fpo = fopen("Results.txt", "w");
    if(fpo == NULL)
    {
      MessageBox("Failed to open Results.txt", TMS, MB_OK);
    }
    else
    {
      while(fgets(szarString, sizeof(szarString), fpi))
      {
//
//  Stop number
//
        strcpy(tempString, strtok(szarString, "\t"));
        stopNumber = atol(tempString);
        sprintf(szStopNumber, "%04ld", stopNumber);
//
//  Status
//
        sprintf(outputString, "%s:\r\n", szStopNumber);
        fputs(outputString, fpo);
//
//  Address
//
        strcpy(szAddress, strtok(NULL, "\t"));
//
//  Platform number
//
        strcpy(tempString, strtok(NULL, "\t"));
        platformNumber = atol(tempString);
//
//  Latitude
//
        strcpy(tempString, strtok(NULL, "\t"));
        latitude = (float)atof(tempString);
//
//  Longitude
//
        strcpy(tempString, strtok(NULL, "\t\n"));
        longitude = (float)atof(tempString);
//
//  Look it up
//
        strncpy(NODESKey2.abbrName, szStopNumber, NODES_ABBRNAME_LENGTH);
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey2, 2);
//
//  Existing node
//
        if(rcode2 == 0)
        {
          bUpdate = FALSE;
//
//  Address
//
          strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
          trim(tempString, NODES_INTERSECTION_LENGTH);
          if(strcmp(tempString, szAddress) != 0)
          {
            sprintf(outputString, "  Changed address/description to '%s'\r\n", szAddress);
            fputs(outputString, fpo);
            strncpy(NODES.intersection, tempString, NODES_INTERSECTION_LENGTH);
            strncpy(NODES.description, tempString, NODES_DESCRIPTION_LENGTH);
            bUpdate = TRUE;
          }
//
//  Node number
//
          if(NODES.number != platformNumber)
          {
            sprintf(outputString, "  Changed platform number from %ld to %ld\r\n", NODES.number, platformNumber);
            fputs(outputString, fpo);
            NODES.number = platformNumber;
            bUpdate = TRUE;
          }
//
//  Latitude/Long
//
          if(NODES.latitude != latitude || NODES.longitude != longitude)
          {
            sprintf(outputString, "  Changed lat/long from %f,%f to %f,%f\r\n", 
                  NODES.latitude, NODES.longitude, latitude, longitude);
            fputs(outputString, fpo);
            NODES.latitude = latitude;
            NODES.longitude = longitude;
            bUpdate = TRUE;
          }
//
//  Update?
//
          if(bUpdate)
          {
            rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey2, 2);
          }
        }
//
//  New node
//
        else
        {
          rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
          NODES.recordID = AssignRecID(rcode2, NODES.recordID);

          sprintf(outputString, "  New stop\r\n", szAddress);
          fputs(outputString, fpo);
          sprintf(outputString, "  Address/description:'%s'\r\n", szAddress);
          fputs(outputString, fpo);
          sprintf(outputString, "  Platform number: %d\r\n", platformNumber);
          fputs(outputString, fpo);
          sprintf(outputString, "  Lat/Long: %f,%f\r\n", latitude, longitude);
          fputs(outputString, fpo);
//
//  Set up the record
//
          NODES.COMMENTSrecordID = NO_RECORD;
          NODES.JURISDICTIONSrecordID = NO_RECORD;
          strncpy(NODES.abbrName, szStopNumber, NODES_ABBRNAME_LENGTH);
          sprintf(tempString, "Stop%s", szStopNumber);
          strncpy(NODES.longName, tempString, NODES_LONGNAME_LENGTH);
          strncpy(NODES.intersection, szAddress, NODES_INTERSECTION_LENGTH);
          strncpy(NODES.description, szAddress, NODES_INTERSECTION_LENGTH);
          memset(&NODES.reliefLabels, 0x00, NODES_RELIEFLABELS_LENGTH);
          NODES.latitude = latitude;
          NODES.longitude = longitude;
          NODES.number = platformNumber;
          memset(&NODES.mapCodes, 0x00, NODES_MAPCODES_LENGTH);
          NODES.capacity = NO_RECORD;
          NODES.OBStopNumber = NO_RECORD;
          NODES.IBStopNumber = NO_RECORD;
          memset(&NODES.AVLStopName, 0x00, NODES_AVLSTOPNAME_LENGTH);
          memset(&NODES.reserved, 0x00, NODES_RESERVED_LENGTH);
          NODES.stopFlags = 0;
          NODES.flags = NODES_FLAG_STOP;
          rcode2 = btrieve(B_INSERT, TMS_NODES, &NODES, &NODESKey0, 0);
        }
      }
    }
  }
  fclose(fpi);
  fclose(fpo);
*/
//
//  And leave
//
  SetCursor(hSaveCursor);	

	CDialog::OnOK();
}