// PlaceHolder.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "PlaceHolder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaceHolder dialog


CPlaceHolder::CPlaceHolder(CWnd* pParent /*=NULL*/, long* pUpdateRecordID)
	: CDialog(CPlaceHolder::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
	//{{AFX_DATA_INIT(CPlaceHolder)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPlaceHolder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaceHolder)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlaceHolder, CDialog)
	//{{AFX_MSG_MAP(CPlaceHolder)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaceHolder message handlers

BOOL CPlaceHolder::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//	
  pEditHOWMANY = (CEdit *)GetDlgItem(PLACEHOLDER_HOWMANY);
  pComboBoxSTARTNODE = (CComboBox *)GetDlgItem(PLACEHOLDER_STARTNODE);
  pComboBoxENDNODE = (CComboBox *)GetDlgItem(PLACEHOLDER_ENDNODE);
  pEditSTARTAT = (CEdit *)GetDlgItem(PLACEHOLDER_STARTAT);
  pEditINCREMENT = (CEdit *)GetDlgItem(PLACEHOLDER_INCREMENT);

  pEditHOWMANY->SetWindowText("1");
  pEditSTARTAT->SetWindowText("1");
  pEditINCREMENT->SetWindowText("1");

//
//  Display the possible nodes
//
  int serviceIndex;
  int rcode2;
  int nI;
//
//  Set up the service index
//
  SERVICESKey0.recordID = m_ServiceRecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  serviceIndex = SERVICES.number - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Cycle through the nodes
//
  NODESKey1.flags = 0;
  memset(&NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATER, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0 && NODES.flags <= NODES_FLAG_GARAGE)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      if((NODES.flags & NODES_FLAG_GARAGE) || (NODES.reliefLabels[serviceIndex] != ' ' &&
            NODES.reliefLabels[serviceIndex] != '-' && NODES.reliefLabels[serviceIndex] != 0))
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        nI = pComboBoxSTARTNODE->AddString(tempString);
        pComboBoxSTARTNODE->SetItemData(nI, NODES.recordID);
        nI = pComboBoxENDNODE->AddString(tempString);
        pComboBoxENDNODE->SetItemData(nI, NODES.recordID);
        if(NODES.flags & NODES_FLAG_GARAGE)
        {
          pComboBoxSTARTNODE->SetCurSel(nI);
          pComboBoxENDNODE->SetCurSel(nI);
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlaceHolder::OnClose() 
{
	OnCancel();
}

void CPlaceHolder::OnCancel() 
{
	CDialog::OnCancel();
}

void CPlaceHolder::OnHelp() 
{
}

void CPlaceHolder::OnOK() 
{
  HCURSOR hSaveCursor;
  long cutAsRuntype;
  long recordID;
  long startNODESrecordID;
  long endNODESrecordID;
  int  nI, nJ;
  int  runtypeIndex, slotNumber;
  int  startAt, increment;
  int  rcode2;
//
//  Make sure there's a "Place Holder" runtype
//
  for(cutAsRuntype = NO_RECORD, nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(!RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
      {
        continue;
      }
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_PLACEHOLDER)
      {
        cutAsRuntype = MAKELONG(nI, nJ);
        break;
      }
    }
    if(cutAsRuntype != NO_RECORD)
    {
      break;
    }
  }
//
//  Not found?  Add it to the RUNTYPE structure
//
  if(cutAsRuntype == NO_RECORD)
  {
    for(nI = 0; nI < NUMRUNTYPES; nI++)
    {
      for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
      {
        if(!RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
        {
          cutAsRuntype = MAKELONG(nI, nJ);
          break;
        }
      }
      if(cutAsRuntype != NO_RECORD)
      {
        break;
      }
    }
    if(cutAsRuntype != NO_RECORD)
    {
      runtypeIndex = LOWORD(cutAsRuntype);
      slotNumber = HIWORD(cutAsRuntype);
      memset(&RUNTYPE[runtypeIndex][slotNumber], 0x00, sizeof(RUNTYPEDef));
      RUNTYPE[runtypeIndex][slotNumber].flags = RTFLAGS_INUSE | RTFLAGS_CREWONLY | RTFLAGS_PLACEHOLDER;
      strcpy(RUNTYPE[runtypeIndex][slotNumber].localName, "Place Holder");
//
//  Write out the workrules file
//
      hSaveCursor = SetCursor(hCursorWait);
      writeWorkrules(hWndMain);
      SetCursor(hSaveCursor);
    }
  }
//
//  Still not found?  Barf.
//
  if(cutAsRuntype == NO_RECORD)
  {
    MessageBeep(MB_ICONSTOP);
    MessageBox("Could not add \"Place Holder\" runtype.\n\nPlease contact Schedule Masters, Inc.", TMS, MB_ICONSTOP | MB_OK);
  }
//
//  Got it - let's go
//
  else
  {
    pEditHOWMANY->GetWindowText(tempString, TEMPSTRING_LENGTH);
    nJ = atol(tempString);
    nI = (int)pComboBoxSTARTNODE->GetCurSel();
    startNODESrecordID = pComboBoxSTARTNODE->GetItemData(nI);
    nI = (int)pComboBoxENDNODE->GetCurSel();
    endNODESrecordID = pComboBoxENDNODE->GetItemData(nI);
    pEditSTARTAT->GetWindowText(tempString, TEMPSTRING_LENGTH);
    startAt = atol(tempString);
    pEditINCREMENT->GetWindowText(tempString, TEMPSTRING_LENGTH);
    increment = atol(tempString);

    rcode2 = btrieve(B_GETLAST, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    recordID = AssignRecID(rcode2, CREWONLY.recordID);
    *m_pUpdateRecordID = recordID;
    for(nI = 0; nI < nJ; nI++)
    {
      memset(&CREWONLY, 0x00, sizeof(CREWONLYDef));
      CREWONLY.recordID = recordID;
      CREWONLY.SERVICESrecordID = m_ServiceRecordID;
      CREWONLY.DIVISIONSrecordID = m_DivisionRecordID;
      CREWONLY.COMMENTSrecordID = NO_RECORD;
      CREWONLY.runNumber = startAt;
      CREWONLY.pieceNumber = 1;
      CREWONLY.cutAsRuntype = cutAsRuntype;
      CREWONLY.startNODESrecordID = startNODESrecordID;
      CREWONLY.startTime = NO_TIME;
      CREWONLY.endNODESrecordID = endNODESrecordID;
      CREWONLY.endTime = NO_TIME;
      CREWONLY.flags = CREWONLY_FLAG_PLACEHOLDER;
      btrieve(B_INSERT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
      recordID++;
      startAt += increment;
    }
  }

	CDialog::OnOK();
}
