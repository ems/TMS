// DailyOpsOOS.cpp : implementation file
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
// CDailyOpsOOS dialog


CDailyOpsOOS::CDailyOpsOOS(CWnd* pParent /*=NULL*/, DAILYOPSBSPassedDataDef* pPassedData)
	: CDialog(CDailyOpsOOS::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CDailyOpsOOS)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsOOS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsOOS)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsOOS, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsOOS)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsOOS message handlers

BOOL CDailyOpsOOS::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up pointers to the controls
//
  pComboBoxREASON = (CComboBox *)GetDlgItem(DAILYOPSOOS_REASON);
  pDTPickerTIME = (CDTPicker *)GetDlgItem(DAILYOPSOOS_TIME);
  pComboBoxLOCATION = (CComboBox *)GetDlgItem(DAILYOPSOOS_LOCATION);
  pEditPAX = (CEdit *)GetDlgItem(DAILYOPSOOS_PAX);
//
//  Set up the Reason combobox
//
  int  nI;
  
  for(nI = 0; nI < m_numBusSwapReasons; nI++)
  {
    nI = pComboBoxREASON->AddString(m_BusSwapReasons[nI].szText);
  }

  if(m_numBusSwapReasons != 0)
  {
    pComboBoxREASON->SetCurSel(0);
  }
  else
  {
    pComboBoxREASON->EnableWindow(FALSE);
  }
//
//  Set up the list of nodes and stops
//
//  Nodes
//
  int rcode2;

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strcat(tempString, ": ");
      strncpy(szarString, NODES.intersection, NODES_INTERSECTION_LENGTH);
      trim(szarString, NODES_INTERSECTION_LENGTH);
      strcat(tempString, szarString);
      nI = pComboBoxLOCATION->AddString(tempString);
      pComboBoxLOCATION->SetItemData(nI, NODES.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
//
//  Stops
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      strcat(tempString, ": ");
      strncpy(szarString, NODES.intersection, NODES_INTERSECTION_LENGTH);
      trim(szarString, NODES_INTERSECTION_LENGTH);
      strcat(tempString, szarString);
      nI = pComboBoxLOCATION->AddString(tempString);
      pComboBoxLOCATION->SetItemData(nI, NODES.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsOOS::OnClose() 
{
  OnCancel();
}

void CDailyOpsOOS::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsOOS::OnHelp() 
{
}

void CDailyOpsOOS::OnNow() 
{
  CTime SystemTime = CTime::GetCurrentTime();
  COleVariant v;

  v = (long)SystemTime.GetHour();
  pDTPickerTIME->SetHour(v);
  v = (long)SystemTime.GetMinute();
  pDTPickerTIME->SetMinute(v);
  v = (long)SystemTime.GetSecond();
  pDTPickerTIME->SetSecond(v);
}

void CDailyOpsOOS::OnOK() 
{
  int nI;
//
//  Reason
//
  nI = pComboBoxREASON->GetCurSel();
  m_pPassedData->swapReasonIndex = (char)(nI == CB_ERR ? NO_RECORD : nI);
//
//  Time
//
  COleVariant v;
  long hours, minutes, seconds;

  v = pDTPickerTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerTIME->GetSecond();
  seconds = v.lVal;
  m_pPassedData->time = hours * 3600 + minutes * 60 + seconds;
//
//  Location
//
  nI = pComboBoxLOCATION->GetCurSel();
  if(nI == CB_ERR)
  {
    m_pPassedData->locationNODESrecordID = NO_RECORD;
  }
  else
  {
    m_pPassedData->locationNODESrecordID = pComboBoxLOCATION->GetItemData(nI);
  }
//
//  PAX
//
  pEditPAX->GetWindowText(tempString, TEMPSTRING_LENGTH);
  m_pPassedData->PAX = (char)atol(tempString);
	
	CDialog::OnOK();
}
