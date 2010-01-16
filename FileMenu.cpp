//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "CCFrontEnd.h"
void SetUpRosterWeekList(HWND, int, long);
}

#include "TMS.h"
#include "MainFrm.h"


//
//  Open the Database
//
int CTMSApp::OnFileOpen()
{
  char szFileName[MAX_PATH] = "\0";
  char drive[3], dir[256], fname[256], ext[256];
  
  CString s;
  s.LoadString(TEXT_046);

  CFileDialog dlg(TRUE, "TMS", NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, "TMS Database Files|*.tms");
//
//  Set up the OPENFILENAME structure for the dialog title
//
  dlg.m_ofn.lpstrTitle = s;
  
  int xx = dlg.DoModal();
  
  if(xx == IDCANCEL)
  {
    return(IDCANCEL);
  }

  strcpy(szDatabaseFileName, dlg.m_ofn.lpstrFile);
  _splitpath(dlg.m_ofn.lpstrFile, drive, dir, fname, ext);
  strcpy(szDirectory, drive);
  strcat(szDirectory, dir);
  if(strlen(szDirectory) > 0)
    szDirectory[strlen(szDirectory) - 1] = '\0';
  szDirectory[strlen(szDirectory)] = '\0';
  WritePrivateProfileString("TMS", "defaultDirectory", (LPSTR)szDirectory, (LPSTR)TMSINIFile);
//
//  Registry
//
  CWinApp* pApp = AfxGetApp();

  pApp->WriteProfileString("General", "DefaultDirectory", szDirectory);
//
//  End Registry
//
  if(strlen(dir) > 0)
    dir[strlen(dir) - 1] = '\0';
  _chdrive((int)drive[0] - 'A' + 1);
  int result = _chdir(dir);
  strcpy(szFileName, fname);
  strcat(szFileName, ext);
//
//  Open the database
//
  if(Database() == IDOK)
  {
    SetupBalanceOfMainFrame();
    return(IDOK);
  }
  return(IDCANCEL);
}

//
//  Create a new Database
//
int CTMSApp::OnFileNew()
{
  HANDLE hFile;
  char szFilter [] = {"TMS Database Files\0*.tms\0"};
  char szDirName [MAX_PATH] = "";
  char szFileName[MAX_PATH] = "\0";
  char szFileTitle[MAX_PATH] = "\0";
  char drive[3], dir[256], fname[256], ext[256];
  char fromFile[257];
  char szFile[256];
  char dummy[256];
  int  rcode2;
  int  nI;
  
  CString s;
//  
//  Ask him what he wants the long description and the effective date to be
//
  if(!DialogBox(hInst, MAKEINTRESOURCE(IDD_NEWDATABASE), hWndMain, (DLGPROC)NEWDATABASEMsgProc))
    return(IDCANCEL);

  s.LoadString(TEXT_266);

  CFileDialog dlg(FALSE);
//
//  Set up the OPENFILENAME structure for the "Save as" common dialog
//
  dlg.m_ofn.lpstrFilter    = szFilter;
  dlg.m_ofn.nFilterIndex   = 1;
  dlg.m_ofn.lpstrFile      = (LPSTR)szFileName;
  dlg.m_ofn.lpstrFileTitle = (LPSTR)szFileTitle;
  dlg.m_ofn.lpstrDefExt    = (LPSTR)"TMS";
  dlg.m_ofn.lpstrTitle     = s;
  dlg.m_ofn.Flags         |= OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST;
  
  if(dlg.DoModal() != IDOK)
    return(IDCANCEL);
//
//  Change over to the selected drive and directory
//
  _splitpath(dlg.m_ofn.lpstrFile, drive, dir, fname, ext);
  strcpy(szDirectory, drive);
  strcat(szDirectory, dir);
  if(strlen(dir) > 0)
    dir[strlen(dir) - 1] = '\0';
  _chdrive((int)drive[0] - 'A' + 1);
  chdir(dir);
//
//  The file can't exist
//
  strcpy(szFile, fname);
  strcat(szFile, ext);
  hFile = CreateFile(szFile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
  if(hFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hFile);
    TMSError(hWndMain, MB_ICONSTOP, ERROR_040, (HANDLE)NULL);
    return(IDCANCEL);
  }
//
//  Ok, create it
//
  hFile = CreateFile(szFile, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
  CloseHandle(hFile);
//
//  All of the file names are in FILE.DDF.  Once that one's open, figure out the rest.
//
//  Change over to the "Fresh" directory
//
  strcpy(tempString, szInstallDirectory);
  strcat(tempString, "\\Fresh");
  chdir(tempString);
//  strcpy(fromFile, szInstallDirectory);
//  strcat(fromFile, "\\FRESH\\");
//  strcat(fromFile, "FILE.DDF");
  strcpy(fromFile, "FILE.DDF");
  rcode2 = btrieve(B_OPEN, 0, dummy, fromFile, 0);
  if(rcode2 != 0)
  {
    s.LoadString(ERROR_049);
    sprintf(szarString, s, fromFile, rcode2);
    MessageBeep(MB_ICONSTOP);
    m_pMainWnd->MessageBox(szarString, TMS, MB_ICONSTOP);
    return(IDCANCEL);
  }
  rcode2 = btrieve(B_GETFIRST, 0, &FILEDDF, &FILEDDFKey1, 1);
  nI = 0;
  while(rcode2 == 0)
  {
    strncpy(szDatabase[nI], FILEDDF.Loc, sizeof(szDatabase[nI]));
    trim(szDatabase[nI], sizeof(FILEDDF.Loc));
    rcode2 = btrieve(B_GETNEXT, 0, &FILEDDF, &FILEDDFKey1, 1);
    nI++;
  }
  btrieve(B_CLOSE, 0, NULL, NULL, NULL);
//
//  Change back to the selected drive and directory
//
  _splitpath(dlg.m_ofn.lpstrFile, drive, dir, fname, ext);
  strcpy(szDirectory, drive);
  strcat(szDirectory, dir);
  if(strlen(dir) > 0)
    dir[strlen(dir) - 1] = '\0';
  _chdrive((int)drive[0] - 'A' + 1);
  chdir(dir);
//
//  Check to see that the first TMS data file doesn't exist in the destination
//  directory.  If it does, kick him out.
//
  hFile = CreateFile(szDatabase[0], GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
  if(hFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hFile);
    s.LoadString(ERROR_041);
    MessageBeep(MB_ICONSTOP);
    m_pMainWnd->MessageBox(s, TMS, MB_ICONSTOP | MB_OK);
    return(IDCANCEL);
  }
//
//  Find out who this is by looking for UserName (under [network])
//  in SYSTEM.INI, working under the assumption that this is a
//  Winows for Workgroups environment.
//
  GetPrivateProfileString("Network", "UserName", "", userName, sizeof(userName), "SYSTEM.INI");
//
//  Did we get it?  If not, look in TMS.INI for it
//
  if(strcmp(userName, "") == 0)
    GetPrivateProfileString("TMS", "UserName", "", userName, sizeof(userName), "TMS.INI");
//
//  Was it there?  If not, get him to enter it.
//
  if(strcmp(userName, "") == 0)
  {
    if(!DialogBox(hInst, MAKEINTRESOURCE(IDD_USERNAME), hWndMain, (DLGPROC)USERNAMEMsgProc))
      return(IDCANCEL);
  }
//
//  Record the description and the user name
//
  strcpy(szDatabaseFileName, szDirectory);
  strcat(szDatabaseFileName, szFile);
  WritePrivateProfileString("Description", "LongDescription",
        (LPSTR)szDatabaseDescription, (LPSTR)szDatabaseFileName);
  WritePrivateProfileString("Description", "LastUpdatedBy",
        (LPSTR)userName, (LPSTR)szDatabaseFileName);
//
//  And the effective and until dates
//
  WritePrivateProfileString("Description", "EffectiveDate",
        (LPSTR)szEffectiveDate, (LPSTR)szDatabaseFileName);
  WritePrivateProfileString("Description", "UntilDate",
        (LPSTR)szUntilDate, (LPSTR)szDatabaseFileName);
//
//  Save the creation date in the database file
//
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
  
  strcpy(tempString, strTime);
  WritePrivateProfileString("Description", "CreationDate", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
//
//  Start the copy of the database files
//
  strcpy(szDatabase[TMS_RUNS], "RUNS.B60");
  for(nI = 0; nI <= TMS_LASTFILE; nI++)
  {
    strcpy(fromFile, szInstallDirectory);
    strcat(fromFile, "\\FRESH\\");
    strcat(fromFile, szDatabase[nI]);
    if(nI == TMS_CUSTOMERS || nI == TMS_COMPLAINTS || nI == TMS_ROUTINGS)
    {
      strcpy(tempString, szInstallDirectory);
      strcat(tempString, "\\");
      strcat(tempString, szDatabase[nI]);
    }
    else
    {
      strcpy(tempString, szDatabase[nI]);
    }
    hFile = CreateFile(tempString, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
    if(hFile == INVALID_HANDLE_VALUE)
    {
      CopyFile(fromFile, tempString, TRUE);
    }
    else
    {
      CloseHandle(hFile);
    }
  }
//
//  Copy over the (more or less) blank workrule.ini file
//
  strcpy(fromFile, szInstallDirectory);
  strcat(fromFile, "\\FRESH\\WORKRULE.INI");
  CopyFile(fromFile, "WORKRULE.INI", TRUE);
//
//  Show him what he's got.  DATABASEMsgProc will open all the btrieve files
//
  if(Database() == IDOK)
  {
    SetupBalanceOfMainFrame();
    return(IDOK);
  }

  return(IDCANCEL);
}


void CTMSApp::SetupBalanceOfMainFrame()
{
//
//  Set up the Dialog Bar
//
//  Routes
//
  SetUpRouteList(m_hwndDlgBar, RSDTOOLBAR_ROUTE, m_RouteRecordID);
  EnableWindow(GetDlgItem(m_hwndDlgBar, RSDTOOLBAR_ROUTE), TRUE);
//
//  Services
//
  SetUpServiceList(m_hwndDlgBar, RSDTOOLBAR_SERVICE, m_ServiceRecordID);
  EnableWindow(GetDlgItem(m_hwndDlgBar, RSDTOOLBAR_SERVICE), TRUE);
//
//  Divisions
//
  SetUpDivisionList(m_hwndDlgBar, RSDTOOLBAR_DIVISION, m_DivisionRecordID);
  EnableWindow(GetDlgItem(m_hwndDlgBar, RSDTOOLBAR_DIVISION), TRUE);
//
//  Roster week
//
  SetUpRosterWeekList(m_hwndDlgBar, RSDTOOLBAR_ROSTERWEEK, m_RosterWeek);
  EnableWindow(GetDlgItem(m_hwndDlgBar, RSDTOOLBAR_ROSTERWEEK), TRUE);
}

void CTMSApp::OnFileClose()
{
  CMainFrame* pFrameWnd = (CMainFrame *)m_pMainWnd->GetTopLevelFrame();

//
//  Reset Btrieve to effect a a "file close" on all open files
//
  btrieve(B_RESET, 0, 0, 0, 0);  //  Reset Btrieve (this also closes all the open files)
//
//  Clear and disable the RSD toolbar
//
  CWnd* pROUTE = pFrameWnd->m_dlgBar.GetDlgItem(RSDTOOLBAR_ROUTE);
  if(pROUTE)
  {
    pROUTE->SendMessage(CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    pROUTE->EnableWindow(FALSE);
  }

  CWnd* pSERVICE = pFrameWnd->m_dlgBar.GetDlgItem(RSDTOOLBAR_SERVICE);
  if(pSERVICE)
  {
    pSERVICE->SendMessage(CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    pSERVICE->EnableWindow(FALSE);
  }

  CWnd* pDIVISION = pFrameWnd->m_dlgBar.GetDlgItem(RSDTOOLBAR_DIVISION);
  if(pDIVISION)
  {
    pDIVISION->SendMessage(CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    pDIVISION->EnableWindow(FALSE);
  }

  CWnd* pROSTERWEEK = pFrameWnd->m_dlgBar.GetDlgItem(RSDTOOLBAR_ROSTERWEEK);
  if(pROSTERWEEK)
  {
    pROSTERWEEK->SendMessage(CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    pROSTERWEEK->EnableWindow(FALSE);
  }

  m_bDatabaseOpen = FALSE;
//
//  Save the date and time into the file
//
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
  
  strcpy(tempString, strTime);
  WritePrivateProfileString("Description", "LastAccessDate", (LPSTR)tempString, (LPSTR)szDatabaseFileName);
//
//  Redisplay the salutation screen
//
  m_aboutFlag = ABOUTBOX_FLAG_OPEN;
  OnAppAbout();

  if(m_aboutResult & ABOUTBOX_RESULT_EXIT)
    OnAppExit();

  else if(m_aboutResult & ABOUTBOX_RESULT_OPEN)
  {
    if(OnFileOpen() == IDCANCEL)
      OnAppExit();
  }
}

//
//  Page Setup
//
void CTMSApp::OnFilePageSetup()
{
  CMDIFrameWnd* pMDIFrameWnd = (CMDIFrameWnd *)m_pMainWnd->GetTopLevelFrame();

  if(pMDIFrameWnd == NULL)
  {
    return;
  }

  CMDIChildWnd* pChild = pMDIFrameWnd->MDIGetActive();

  if(pChild == NULL)
  {
    return;
  }

  HWND hActiveWindow = pChild->m_hWnd;
  if(hActiveWindow)
  {
    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];
  
    if(pTD)
    {
      SetDefaultPageSetup(displayIndex);
      pTD->F1.FilePageSetupDlg();
    }
  }
}

//
//  Print
//

void CTMSApp::OnFilePrint()
{
  CMDIFrameWnd* pMDIFrameWnd = (CMDIFrameWnd *)m_pMainWnd->GetTopLevelFrame();

  if(pMDIFrameWnd == NULL)
  {
    return;
  }

  CMDIChildWnd* pChild = pMDIFrameWnd->MDIGetActive();

  if(pChild == NULL)
  {
    return;
  }

  HWND hActiveWindow = pChild->m_hWnd;
  if(hActiveWindow)
  {
    long displayIndex = GetWindowLong(hActiveWindow, GWL_USERDATA);
    TableDisplayDef *pTD = &m_TableDisplay[displayIndex];

    if(pTD && pTD->F1)
    {
      SetDefaultPageSetup(displayIndex);
      strcpy(titleString, m_TableDisplay[displayIndex].title);
      pTD->F1.SetPrintLandscape(TRUE);
      pTD->F1.FilePrintEx(TRUE, FALSE);
    }
  }
}
