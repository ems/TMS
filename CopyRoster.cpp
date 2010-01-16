// CopyRoster.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "TMS.h"
#include "CopyRoster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyRoster dialog


CCopyRoster::CCopyRoster(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyRoster::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyRoster)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCopyRoster::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyRoster)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyRoster, CDialog)
	//{{AFX_MSG_MAP(CCopyRoster)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_CBN_EDITCHANGE(COPYRUNCUT_DIVISION, OnEditchangeDivision)
	ON_BN_CLICKED(COPYROSTER_WEEKBUTTON, OnWeekbutton)
	ON_BN_CLICKED(COPYROSTER_DIVISIONBUTTON, OnDivisionbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyRoster message handlers

BOOL CCopyRoster::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//	
  pStaticDESTINATION = (CStatic *)GetDlgItem(COPYROSTER_DESTINATION);
  pButtonWEEKBUTTON = (CButton *)GetDlgItem(COPYROSTER_WEEKBUTTON);
  pEditWEEK = (CEdit *)GetDlgItem(COPYROSTER_WEEK);
  pButtonDIVISIONBUTTON = (CButton *)GetDlgItem(COPYROSTER_DIVISIONBUTTON);
  pComboBoxDIVISION = (CComboBox *)GetDlgItem(COPYROSTER_DIVISION);
//
//
//  Set up the division combo box
//
  int numDivisions = SetUpDivisionList(m_hWnd, COPYROSTER_DIVISION, m_DivisionRecordID);

  if(numDivisions == 0)
  {
    OnCancel();
    return TRUE;
  }
//
//  Display the destination string
//
  CString s;

  pStaticDESTINATION->GetWindowText(s);
  sprintf(szarString, s, m_RosterWeek + 1);
  pStaticDESTINATION->SetWindowText(szarString);
//
//  Default to week 1
//
  pButtonWEEKBUTTON->SetCheck(TRUE);
  pEditWEEK->SetWindowText("1");
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCopyRoster::OnClose() 
{
	OnCancel();
}

void CCopyRoster::OnCancel() 
{
	CDialog::OnCancel();
}

void CCopyRoster::OnHelp() 
{
}


void CCopyRoster::OnEditchangeDivision() 
{
  pButtonWEEKBUTTON->SetCheck(FALSE);
  pButtonDIVISIONBUTTON->SetCheck(TRUE);
}

void CCopyRoster::OnWeekbutton() 
{
}

void CCopyRoster::OnDivisionbutton() 
{
  pComboBoxDIVISION->SetCurSel(0);
}

void CCopyRoster::OnOK() 
{
  CString s;
  long fromWeek;
  long fromDIVISIONSrecordID;
  long absRecID, recordID;;
  int  nI;
  int  rcode2;
//
//  Copy a week or copy from another division
//
//  Copy a week
//
  if(pButtonWEEKBUTTON->GetCheck())
  {
//
//  Make sure he's not copying on top of himself
//
    pEditWEEK->GetWindowText(s);
    fromWeek = atol(s);
    if(fromWeek == m_RosterWeek - 1)
    {
      TMSError(m_hWnd, MB_ICONSTOP, ERROR_347, (HANDLE)pEditWEEK->m_hWnd);
      return;
    }
//
//  Gotta be between 1 and 8
//
    if(fromWeek < 1 || fromWeek > ROSTER_MAX_WEEKS)
    {
      TMSError(m_hWnd, MB_ICONSTOP, ERROR_363, (HANDLE)pEditWEEK->m_hWnd);
      return;
    }
//
//  Let him know what he's about to do
//
    MessageBeep(MB_ICONQUESTION);
    s.LoadString(ERROR_348);
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_OK) != IDOK)
    {
      OnCancel();
      return;
    }
//
//  The critter went and done it...
//
    HCURSOR saveCursor = SetCursor(hCursorWait);
//
//  Delete the existing roster for this week
//
    ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
    ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0 &&
          ROSTER.DIVISIONSrecordID == m_DivisionRecordID)
    {
      memset(&ROSTER.WEEK[m_RosterWeek], 0x00, sizeof(ROSTERWEEKDef));    
      rcode2 = btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
//
//  Perform the copy
//
    ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
    ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0 &&
          ROSTER.DIVISIONSrecordID == m_DivisionRecordID)
    {
      memcpy(&ROSTER.WEEK[m_RosterWeek], &ROSTER.WEEK[fromWeek - 1], sizeof(ROSTERWEEKDef));
      rcode2 = btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
    SetCursor(saveCursor);
// 
//  Let him know it's done
//
    MessageBeep(MB_ICONINFORMATION);
    s.LoadString(TEXT_153);
    MessageBox(s, TMS, MB_ICONINFORMATION);
  }
//
//  Copy a division
//
  else
  {
//
//  Make sure he's not copying on top of himself
//
    nI = pComboBoxDIVISION->GetCurSel();
    if(nI == CB_ERR)
    {
      return;
    }
    fromDIVISIONSrecordID = pComboBoxDIVISION->GetItemData(nI);
    if(fromDIVISIONSrecordID == m_DivisionRecordID)
    {
      TMSError(m_hWnd, MB_ICONSTOP, ERROR_347, (HANDLE)pEditWEEK->m_hWnd);
      return;
    }
//
//  Perform the copy
//
    rcode2 = btrieve(B_GETLAST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
    recordID = AssignRecID(rcode2, ROSTER.recordID);

    ROSTERKey1.DIVISIONSrecordID = fromDIVISIONSrecordID;
    ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0 &&
          ROSTER.DIVISIONSrecordID == fromDIVISIONSrecordID)
    {
      btrieve(B_GETPOSITION, TMS_ROSTER, &absRecID, &ROSTERKey1, 1);
      ROSTER.recordID = recordID++;
      ROSTER.DIVISIONSrecordID = m_DivisionRecordID;
      rcode2 = btrieve(B_INSERT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      ROSTER.recordID = absRecID;
      btrieve(B_GETDIRECT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
  }

	CDialog::OnOK();
}
