// AddExtraTrip.cpp : implementation file
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
// CAddExtraTrip dialog


CAddExtraTrip::CAddExtraTrip(CWnd* pParent, DAILYOPSDef* pDAILYOPS)
	: CDialog(CAddExtraTrip::IDD, pParent)
{
  m_pDAILYOPS = pDAILYOPS;

	//{{AFX_DATA_INIT(CAddExtraTrip)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddExtraTrip::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddExtraTrip)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddExtraTrip, CDialog)
	//{{AFX_MSG_MAP(CAddExtraTrip)
	ON_BN_CLICKED(ADDEXTRATRIP_OUTBOUND, OnOutbound)
	ON_BN_CLICKED(ADDEXTRATRIP_INBOUND, OnInbound)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_CBN_SELCHANGE(ADDEXTRATRIP_ROUTE, OnSelchangeRoute)
	ON_CBN_SELCHANGE(ADDEXTRATRIP_SERVICE, OnSelchangeService)
	ON_CBN_SELCHANGE(ADDEXTRATRIP_PATTERNNAME, OnSelchangePatternname)
	ON_LBN_SELCHANGE(ADDEXTRATRIP_PATTERNNODES, OnSelchangePatternnodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddExtraTrip message handlers

BOOL CAddExtraTrip::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pComboBoxROUTE = (CComboBox *)GetDlgItem(ADDEXTRATRIP_ROUTE);
  pComboBoxSERVICE = (CComboBox *)GetDlgItem(ADDEXTRATRIP_SERVICE);
  pButtonOUTBOUND = (CButton *)GetDlgItem(ADDEXTRATRIP_OUTBOUND);
  pButtonINBOUND = (CButton *)GetDlgItem(ADDEXTRATRIP_INBOUND);
  pComboBoxPATTERNNAME = (CComboBox *)GetDlgItem(ADDEXTRATRIP_PATTERNNAME);
  pListBoxPATTERNNODES = (CListBox *)GetDlgItem(ADDEXTRATRIP_PATTERNNODES);
  pEditTIMEATMLP = (CEdit *)GetDlgItem(ADDEXTRATRIP_TIMEATMLP);
  pEditDEADHEADS = (CEdit *)GetDlgItem(ADDEXTRATRIP_DEADHEADS);
//
//  Set up the route and service combo boxes
//
  m_ROUTESrecordID = NO_RECORD;
  SetUpRouteList(this->m_hWnd, ADDEXTRATRIP_ROUTE, m_ROUTESrecordID);
  m_SERVICESrecordID = NO_RECORD;
  SetUpServiceList(this->m_hWnd, ADDEXTRATRIP_SERVICE, m_SERVICESrecordID);
//
//  Default to the outbound direction
//
  m_directionIndex = 0;
  pButtonOUTBOUND->SetCheck(TRUE);
//
//  No pattern yet
//
  m_PATTERNNAMESrecordID = NO_RECORD;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddExtraTrip::OnOutbound() 
{
  m_directionIndex = 0;
  PopulatePatternName();
}

void CAddExtraTrip::OnInbound() 
{
  m_directionIndex = 1;
  PopulatePatternName();
}


void CAddExtraTrip::OnHelp() 
{
}

void CAddExtraTrip::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddExtraTrip::OnOK() 
{
  GenerateTripDef GTResults;
  CString s;
  CString outputString;
//
//  Get the time at the MLP
//
  pEditTIMEATMLP->GetWindowText(tempString, TEMPSTRING_LENGTH);
  if(strcmp(tempString, "") == 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_032, pEditTIMEATMLP->m_hWnd);
    return;
  }
  long timeAtMLP = cTime(tempString);
//
//  Generate the trip
//
  GenerateTrip(m_ROUTESrecordID, m_SERVICESrecordID, m_directionIndex,
        m_PATTERNNAMESrecordID, timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Any deadhead time added?
//
  long deadheadTime;

  pEditDEADHEADS->GetWindowText(tempString, TEMPSTRING_LENGTH);
  deadheadTime = (strcmp(tempString, "") == 0 ? 0 : thhmm(tempString));
//
//  Display the results
//
//  Build up the string
//
//  From node
//
  NODESKey0.recordID = GTResults.firstNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
  outputString = s;
  outputString += " ";
//
//  From time
//
  outputString += Tchar(GTResults.firstNodeTime);
  outputString += " ";
//
//  To time
//
  outputString += Tchar(GTResults.lastNodeTime);
  outputString += " ";
//
//  To node
//
  NODESKey0.recordID = GTResults.lastNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
  outputString += s;
  outputString += " = ";
//
//  Trip time
//
  long totalTime = GTResults.lastNodeTime - GTResults.firstNodeTime;
  
  outputString += chhmm(totalTime);
//
//  Deadhead time?
//
  if(deadheadTime != 0)
  {
    outputString += " + deadhead of ";
    outputString += chhmm(deadheadTime);
    outputString += " = ";
    totalTime += deadheadTime;
    outputString += chhmm(totalTime);
  }
//
//  Display it
//
//  Database verify: 11-Jan-07
//
  MessageBeep(MB_ICONINFORMATION);
  if(MessageBox(outputString, "Add this trip?", MB_ICONINFORMATION | MB_YESNO) == IDYES)
  {
    m_pDAILYOPS->DOPS.Operator.ROUTESrecordID = m_ROUTESrecordID;
    m_pDAILYOPS->DOPS.Operator.SERVICESrecordID = m_SERVICESrecordID;
    m_pDAILYOPS->DOPS.Operator.PATTERNNAMESrecordID = m_PATTERNNAMESrecordID;
    m_pDAILYOPS->DOPS.Operator.directionIndex = m_directionIndex;
    m_pDAILYOPS->DOPS.Operator.timeAtMLP = timeAtMLP;
    m_pDAILYOPS->DOPS.Operator.deadheadTime = deadheadTime;
    m_pDAILYOPS->DOPS.Operator.timeAdjustment = totalTime;
    CDialog::OnOK();
  }
}

void CAddExtraTrip::OnSelchangeRoute() 
{
  CString s;
  int nI = pComboBoxROUTE->GetCurSel();
//
//  Only proceed if no error
//
  if(nI != CB_ERR)
  {
    m_ROUTESrecordID = pComboBoxROUTE->GetItemData(nI);
//
//  Set the direction names
//
    ROUTESKey0.recordID = m_ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    s = GetDatabaseString(DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    pButtonOUTBOUND->SetWindowText(s);
    if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
    {
      pButtonINBOUND->SetCheck(FALSE);
      pButtonINBOUND->EnableWindow(FALSE);
      s.LoadString(TEXT_017);
      pButtonOUTBOUND->SetCheck(TRUE);
    }
    else
    {
      pButtonINBOUND->EnableWindow(TRUE);
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      s = GetDatabaseString(DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    }
    pButtonINBOUND->SetWindowText(s);
//
//  Is there a service too?  If so, extablish the pattern names
//
    nI = pComboBoxSERVICE->GetCurSel();
    if(nI != CB_ERR)
    {
      m_SERVICESrecordID = pComboBoxSERVICE->GetItemData(nI);
      PopulatePatternName();
    }
  }
}

void CAddExtraTrip::OnSelchangeService() 
{
  CString s;
  int nI = pComboBoxSERVICE->GetCurSel();

//
//  Only proceed if no error
//
  if(nI != CB_ERR)
  {
    m_SERVICESrecordID = pComboBoxSERVICE->GetItemData(nI);
//
//  Is there a route too?  If so, establish the pattern names
//
    nI = pComboBoxROUTE->GetCurSel();
    if(nI != CB_ERR)
    {
      m_ROUTESrecordID = pComboBoxROUTE->GetItemData(nI);
      PopulatePatternName();
    }
  }
}

void CAddExtraTrip::OnSelchangePatternname() 
{
  CString s;
  int rcode2;
  int nI;

  pListBoxPATTERNNODES->ResetContent();
  if(m_ROUTESrecordID != NO_RECORD && m_SERVICESrecordID != NO_RECORD)
  {
    nI = pComboBoxPATTERNNAME->GetCurSel();
    if(nI == CB_ERR)
    {
      return;
    }
    m_PATTERNNAMESrecordID = pComboBoxPATTERNNAME->GetItemData(nI);
    PATTERNSKey2.ROUTESrecordID = m_ROUTESrecordID;
    PATTERNSKey2.SERVICESrecordID = m_SERVICESrecordID;
    PATTERNSKey2.directionIndex = m_directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = m_PATTERNNAMESrecordID;
    PATTERNSKey2.nodeSequence = 0;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == m_ROUTESrecordID &&
          PATTERNS.SERVICESrecordID == m_SERVICESrecordID &&
          PATTERNS.directionIndex == m_directionIndex &&
          PATTERNSKey2.PATTERNNAMESrecordID == m_PATTERNNAMESrecordID)
    {
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
        nI = pListBoxPATTERNNODES->AddString(s);
        if(PATTERNS.flags & PATTERNS_FLAG_MLP)
        {
          m_MLPIndex = nI;
          pListBoxPATTERNNODES->SetCurSel(nI);
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
  }
}

void CAddExtraTrip::PopulatePatternName()
{
  SetUpPatternList(this->m_hWnd, ADDEXTRATRIP_PATTERNNAME, ROUTES.recordID,
        m_SERVICESrecordID, m_directionIndex, TRUE, FALSE);
  pComboBoxPATTERNNAME->SetCurSel(0);
  OnSelchangePatternname();
}

void CAddExtraTrip::OnSelchangePatternnodes() 
{
  pListBoxPATTERNNODES->SetCurSel(m_MLPIndex);
}
