// DailyOpsBP.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBP dialog

CDailyOpsBP::CDailyOpsBP(CWnd* pParent, TripInfoDef* pTI, int numInTripInfo)   // standard constructor
	: CDialog(CDailyOpsBP::IDD, pParent)
{
  m_pTI = pTI;
  m_numInTI = numInTripInfo;

	//{{AFX_DATA_INIT(CDailyOpsBP)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsBP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsBP)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsBP, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsBP)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDMODIFY, OnModify)
	ON_NOTIFY(NM_CLICK, DAILYOPSBP_TRIPLIST, OnClickTriplist)
	ON_NOTIFY(NM_DBLCLK, DAILYOPSBP_TRIPLIST, OnDblclkTriplist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBP message handlers

BOOL CDailyOpsBP::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pListCtrlTRIPLIST	= (CListCtrl *)GetDlgItem(DAILYOPSBP_TRIPLIST);
  pButtonIDMODIFY = (CButton *)GetDlgItem(IDMODIFY);
//
//  Extend its style
//
  DWORD dwExStyles = pListCtrlTRIPLIST->GetExtendedStyle();
  pListCtrlTRIPLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set the title of the dialog
//
  CString sRGRP;
  CString sSGRP;

  TRIPSKey0.recordID = m_pTI->TRIPSrecordID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);

  ROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  sRGRP = tempString;
  sRGRP += " - ";
  strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
  trim(tempString, ROUTES_NAME_LENGTH);
  sRGRP += tempString;

  SERVICESKey0.recordID = TRIPS.standard.SGRPSERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(tempString, SERVICES_NAME_LENGTH);
  sSGRP = tempString;

  sprintf(tempString, "Block number %ld Properties : Blocked on RGRP %s, SGRP %s",
        TRIPS.standard.blockNumber, sRGRP, sSGRP);
  SetWindowText(tempString);
//
//  Set the headers and columns on the list control
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Trip #";
  pListCtrlTRIPLIST->InsertColumn(0, &LVC);

  LVC.cx = 70;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Status";
  pListCtrlTRIPLIST->InsertColumn(1, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Run";
  pListCtrlTRIPLIST->InsertColumn(2, &LVC);

  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Roster";
  pListCtrlTRIPLIST->InsertColumn(3, &LVC);

  LVC.cx = 160;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Rostered Operator";
  pListCtrlTRIPLIST->InsertColumn(4, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Rlf At";
  pListCtrlTRIPLIST->InsertColumn(5, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Time";
  pListCtrlTRIPLIST->InsertColumn(6, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "POG";
  pListCtrlTRIPLIST->InsertColumn(7, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "POT";
  pListCtrlTRIPLIST->InsertColumn(8, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Route";
  pListCtrlTRIPLIST->InsertColumn(9, &LVC);
 
  LVC.cx = 74;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Pattern";
  pListCtrlTRIPLIST->InsertColumn(10, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "FNode";
  pListCtrlTRIPLIST->InsertColumn(11, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "FTime";
  pListCtrlTRIPLIST->InsertColumn(12, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "TTime";
  pListCtrlTRIPLIST->InsertColumn(13, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "TNode";
  pListCtrlTRIPLIST->InsertColumn(14, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "PIG";
  pListCtrlTRIPLIST->InsertColumn(15, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "PIT";
  pListCtrlTRIPLIST->InsertColumn(16, &LVC);
//
//  Display the structure
//
  int nI;

  for(nI = 0; nI < m_numInTI; nI++)
  {
    DisplayRow(nI);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsBP::DisplayRow(int rowNumber)
{
  LVITEM LVI;
  char   szRosterNumber[2][16];
  int    nI;
//
//  Trip number
//
  if(m_pTI[rowNumber].tripNumber > 0)
  {
    sprintf(tempString, "%ld", m_pTI[rowNumber].tripNumber);
  }
  else
  {
    strcpy(tempString, "");
  }
  LVI.mask = LVIF_TEXT | LVIF_PARAM;
  LVI.lParam = TRIPS.recordID;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 0;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->InsertItem(&LVI);
//
//  Status
//
  if(m_pTI[rowNumber].flags & TRIPINFO_FLAG_OK)
  {
    strcpy(tempString, "OK");
  }
  else if(m_pTI[rowNumber].flags & TRIPINFO_FLAG_LATELEAVING)
  {
    strcpy(tempString, "Late dep");
  }
  else if(m_pTI[rowNumber].flags & TRIPINFO_FLAG_LATEARRIVING)
  {
    strcpy(tempString, "Late arr");
  }
  else if(m_pTI[rowNumber].flags & TRIPINFO_FLAG_DROPPED)
  {
    strcpy(tempString, "Dropped");
  }
  else if(m_pTI[rowNumber].flags & TRIPINFO_FLAG_REPATTERNED)
  {
    strcpy(tempString, "Repatterned");
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 1;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Run number
//
  if(m_pTI[rowNumber].runNumber[1] == NO_RECORD)
  {
    sprintf(tempString, "%ld", m_pTI[rowNumber].runNumber[0]);
  }
  else
  {
    sprintf(tempString, "%ld/%ld", m_pTI[rowNumber].runNumber[0], m_pTI[rowNumber].runNumber[1]);
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 2;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Roster
//
  for(nI = 0; nI < 2; nI++)
  {
    if(m_pTI[rowNumber].rosterNumber[nI] == NO_RECORD)
    {
      strcpy(szRosterNumber[nI], "-");
    }
    else
    {
      sprintf(szRosterNumber[nI], "%ld", m_pTI[rowNumber].rosterNumber[nI]);
    }
  }
  if(m_pTI[rowNumber].rosterNumber[1] == NO_RECORD)
  {
    strcpy(tempString, szRosterNumber[0]);
  }
  else
  {
    sprintf(tempString, "%s/%s", szRosterNumber[0], szRosterNumber[1]);
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 3;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Operator
//
  if(m_pTI[rowNumber].DRIVERSrecordID[0] == NO_RECORD)
  {
    strcpy(tempString, "-");
  }
  else
  {
    strcpy(tempString, BuildOperatorString(m_pTI[rowNumber].DRIVERSrecordID[0]));
  }
  if(m_pTI[rowNumber].DRIVERSrecordID[1] != NO_RECORD)
  {
    strcat(tempString, "/");
    if(m_pTI[rowNumber].DRIVERSrecordID[0] == NO_RECORD)
    {
      strcat(tempString, "-");
    }
    else
    {
      strcat(tempString, BuildOperatorString(m_pTI[rowNumber].DRIVERSrecordID[1]));
    }
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 4;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Relief information
//
//  Relief At
//
  if(m_pTI[rowNumber].reliefAtNODESrecordID != NO_RECORD)
  {
    NODESKey0.recordID = m_pTI[rowNumber].reliefAtNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 5;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Relief time
//
//  Find the node on the pattern
//
    strcpy(tempString, Tchar(m_pTI[rowNumber].reliefAtTime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 6;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
//
//  POG and POT
//
  if(m_pTI[rowNumber].POGNODESrecordID != NO_RECORD)
  {
//
//  POG
//
    NODESKey0.recordID = m_pTI[rowNumber].POGNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 7;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  POT
//
    sprintf(tempString, "%s", Tchar(m_pTI[rowNumber].POTime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 8;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
//
//  Route
//
  ROUTESKey0.recordID = m_pTI[rowNumber].ROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 9;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Pattern
//
  PATTERNNAMESKey0.recordID = m_pTI[rowNumber].PATTERNNAMESrecordID;
  btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
  strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
  trim(tempString, PATTERNNAMES_NAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 10;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  FNode
//
  NODESKey0.recordID = m_pTI[rowNumber].fromNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 11;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  FTime
//
  sprintf(tempString, "%s", Tchar(m_pTI[rowNumber].fromTime));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 12;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  TTime
//
  sprintf(tempString, "%s", Tchar(m_pTI[rowNumber].toTime));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 13;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  TNode
//
  NODESKey0.recordID = m_pTI[rowNumber].toNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 14;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  PIG and PIT
//
  if(m_pTI[rowNumber].PIGNODESrecordID != NO_RECORD)
  {
//
//  PIG
//
    NODESKey0.recordID = m_pTI[rowNumber].PIGNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 15;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  PIT
//
    sprintf(tempString, "%s", Tchar(m_pTI[rowNumber].PITime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 16;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
}

void CDailyOpsBP::OnClose() 
{
	CDialog::OnClose();
}

void CDailyOpsBP::OnHelp() 
{
}

void CDailyOpsBP::OnOK() 
{
	CDialog::OnOK();
}

void CDailyOpsBP::OnModify() 
{
  LVITEM LVI;
  int nI = pListCtrlTRIPLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return;
  }
  else
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlTRIPLIST->GetItem(&LVI);
    TRIPS.recordID = LVI.lParam;
  }
}

void CDailyOpsBP::OnClickTriplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  int nI = pListCtrlTRIPLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  pButtonIDMODIFY->EnableWindow(nI > 0 ? TRUE : FALSE);
	
	*pResult = 0;
}

void CDailyOpsBP::OnDblclkTriplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnModify();

	*pResult = 0;
}
