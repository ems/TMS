// DailyOpsBS.cpp : implementation file
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
// CDailyOpsBS dialog


CDailyOpsBS::CDailyOpsBS(CWnd* pParent /*=NULL*/, DAILYOPSBSPassedDataDef* pPassedData)
	: CDialog(CDailyOpsBS::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CDailyOpsBS)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsBS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsBS)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsBS, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsBS)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDNOW, OnNow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBS message handlers

BOOL CDailyOpsBS::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pComboBoxREASON = (CComboBox *)GetDlgItem(DAILYOPSBS_REASON);
  pDTPickerTIME = (CDTPicker *)GetDlgItem(DAILYOPSBS_TIME);
  pComboBoxLOCATION = (CComboBox *)GetDlgItem(DAILYOPSBS_LOCATION);
  pEditPAX = (CEdit *)GetDlgItem(DAILYOPSBS_PAX);
  pButtonMARKOOS = (CButton *)GetDlgItem(DAILYOPSBS_MARKOOS);
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
//
//  Default the checkbox to false
//
  pButtonMARKOOS->SetCheck(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsBS::OnClose() 
{
	OnCancel();
}

void CDailyOpsBS::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsBS::OnHelp() 
{
}

void CDailyOpsBS::OnNow() 
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

void CDailyOpsBS::OnOK() 
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
//
//  Mark out of service?
//
  m_pPassedData->bMarkOutOfService = pButtonMARKOOS->GetCheck();

	CDialog::OnOK();
}
