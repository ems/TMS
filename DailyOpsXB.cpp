// DailyOpsXB.cpp : implementation file
//

#include "stdafx.h"

extern "C"{
#include "TMSHeader.h"
}

#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsXB dialog


CDailyOpsXB::CDailyOpsXB(CWnd* pParent /*=NULL*/)
	: CDialog(CDailyOpsXB::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDailyOpsXB)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsXB::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsXB)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsXB, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsXB)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDADDTODAY, OnAddtoday)
	ON_BN_CLICKED(IDADDTONIGHT, OnAddtonight)
	ON_BN_CLICKED(IDREMOVEFROMDAY, OnRemovefromday)
	ON_BN_CLICKED(IDREMOVEFROMNIGHT, OnRemovefromnight)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(DAILYOPSXB_ALL, OnAll)
	ON_BN_CLICKED(DAILYOPSXB_JUSTUNROSTERED, OnJustunrostered)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsXB message handlers

BOOL CDailyOpsXB::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pButtonALL = (CButton *)GetDlgItem(DAILYOPSXB_ALL);
  pButtonJUSTUNROSTERED = (CButton *)GetDlgItem(DAILYOPSXB_JUSTUNROSTERED);
  pListBoxUNASSIGNED = (CListBox *)GetDlgItem(DAILYOPSXB_UNASSIGNED);
  pListBoxDAYTIME = (CListBox *)GetDlgItem(DAILYOPSXB_DAYTIME);
  pListBoxNIGHTTIME = (CListBox *)GetDlgItem(DAILYOPSXB_NIGHTTIME);
//
//  Default to all
//
  pButtonALL->SetCheck(TRUE);
//
//  Display the list of operators
//
  DisplayOperators();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsXB::DisplayOperators()
{
  BOOL bAll = pButtonALL->GetCheck();
  BOOL bFound;
  int  nI;
  int  rcode2;

  pListBoxUNASSIGNED->ResetContent();

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      bFound = FALSE;
      if(!bAll)
      {
        ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
        while(rcode2 == 0 &&
              ROSTER.DRIVERSrecordID == DRIVERS.recordID)
        {
          if(ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
          {
            bFound = TRUE;
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
        }
      }
//
//  Unassigned - see where they go
//
      if(!bFound)
      {
        strcpy(tempString, BuildOperatorString(NO_RECORD));
        if(DRIVERS.flags & DRIVERS_FLAG_DAYTIMEEXTRABOARD)
        {
          nI = pListBoxDAYTIME->AddString(tempString);
          pListBoxDAYTIME->SetItemData(nI, DRIVERS.recordID);
        }
        else if(DRIVERS.flags & DRIVERS_FLAG_NIGHTTIMEEXTRABOARD)
        {
          nI = pListBoxNIGHTTIME->AddString(tempString);
          pListBoxNIGHTTIME->SetItemData(nI, DRIVERS.recordID);
        }
        else
        {
          nI = pListBoxUNASSIGNED->AddString(tempString);
          pListBoxUNASSIGNED->SetItemData(nI, DRIVERS.recordID);
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  }
}

void CDailyOpsXB::OnAll() 
{
  DisplayOperators();
}

void CDailyOpsXB::OnJustunrostered() 
{
  DisplayOperators();
}

void CDailyOpsXB::Move(CListBox* pFrom, CListBox* pTo)
{
  long tempLong;
  int  nI, nJ;
  int  numSelected;

  numSelected = pFrom->GetSelCount();
  if(numSelected == 0 || numSelected == LB_ERR)
  {
    return;
  }

  int *pSelected;

  pSelected = (int *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int) * numSelected); 
  if(pSelected == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return;
  }
  pFrom->GetSelItems(numSelected, pSelected);

  for(nI = numSelected - 1; nI >= 0; nI--)
  {
    pFrom->GetText(pSelected[nI], tempString);
    tempLong = pFrom->GetItemData(pSelected[nI]);
    pFrom->DeleteString(pSelected[nI]);
    nJ = pTo->AddString(tempString);
    pTo->SetItemData(nJ, tempLong);
  }
  
  TMSHeapFree(pSelected);
}

void CDailyOpsXB::OnAddtoday() 
{
  Move(pListBoxUNASSIGNED, pListBoxDAYTIME);
}

void CDailyOpsXB::OnAddtonight() 
{
  Move(pListBoxUNASSIGNED, pListBoxNIGHTTIME);
}

void CDailyOpsXB::OnRemovefromday() 
{
  Move(pListBoxDAYTIME, pListBoxUNASSIGNED);
}

void CDailyOpsXB::OnRemovefromnight() 
{
  Move(pListBoxNIGHTTIME, pListBoxUNASSIGNED);
}

void CDailyOpsXB::OnClose() 
{
	CDialog::OnCancel();
}

void CDailyOpsXB::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsXB::OnHelp() 
{
}

void CDailyOpsXB::OnOK() 
{
  CListBox *pListBox[3];

  pListBox[0] = pListBoxUNASSIGNED;
  pListBox[1] = pListBoxDAYTIME;
  pListBox[2] = pListBoxNIGHTTIME;

//
//  Reset the flags in the drivers table
//
  int  numItems;
  int  nI, nJ;
  int  rcode2;

  for(nI = 0; nI < 3; nI++)
  {
    numItems = pListBox[nI]->GetCount();
    for(nJ = 0; nJ < numItems; nJ++)
    {
      DRIVERSKey0.recordID = pListBox[nI]->GetItemData(nJ);
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      if(nI == 0) // Unassigned
      {
        if(DRIVERS.flags & DRIVERS_FLAG_DAYTIMEEXTRABOARD)
        {
          DRIVERS.flags -= DRIVERS_FLAG_DAYTIMEEXTRABOARD;
        }
        if(DRIVERS.flags & DRIVERS_FLAG_NIGHTTIMEEXTRABOARD)
        {
          DRIVERS.flags -= DRIVERS_FLAG_NIGHTTIMEEXTRABOARD;
        }
      }
      else if(nI == 1) // Daytime
      {
        if(DRIVERS.flags & DRIVERS_FLAG_NIGHTTIMEEXTRABOARD)
        {
          DRIVERS.flags -= DRIVERS_FLAG_NIGHTTIMEEXTRABOARD;
        }
        DRIVERS.flags |= DRIVERS_FLAG_DAYTIMEEXTRABOARD;
      }
      else // Nighttime
      {
        if(DRIVERS.flags & DRIVERS_FLAG_DAYTIMEEXTRABOARD)
        {
          DRIVERS.flags -= DRIVERS_FLAG_DAYTIMEEXTRABOARD;
        }
        DRIVERS.flags |= DRIVERS_FLAG_NIGHTTIMEEXTRABOARD;
      }
      rcode2 = btrieve(B_UPDATE, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    }
  }
	
	CDialog::OnOK();
}
