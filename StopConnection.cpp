//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// StopConnection.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "StopConnection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL   bSortForwardFrom[2];
static BOOL   bSortForwardTo[2];

int CALLBACK SCListViewFromCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  SCSORTDef *p1 = (SCSORTDef *)lParam1;
  SCSORTDef *p2 = (SCSORTDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Number
        iResult = bSortForwardFrom[lParamSort] ?
              lstrcmpi(p1->abbrName, p2->abbrName) :
              lstrcmpi(p2->abbrName, p1->abbrName);
        break;

      case 1:  // Name
        iResult = bSortForwardFrom[lParamSort] ?
              lstrcmpi(p1->intersection, p2->intersection) :
              lstrcmpi(p2->intersection, p1->intersection);
        break;
    }
  }

  return(iResult);
}

int CALLBACK SCListViewToCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  SCSORTDef *p1 = (SCSORTDef *)lParam1;
  SCSORTDef *p2 = (SCSORTDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Number
        iResult = bSortForwardTo[lParamSort] ?
              lstrcmpi(p1->abbrName, p2->abbrName) :
              lstrcmpi(p2->abbrName, p1->abbrName);
        break;

      case 1:  // Name
        iResult = bSortForwardTo[lParamSort] ?
              lstrcmpi(p1->intersection, p2->intersection) :
              lstrcmpi(p2->intersection, p1->intersection);
        break;
    }
  }

  return(iResult);
}


/////////////////////////////////////////////////////////////////////////////
// CStopConnection dialog


CStopConnection::CStopConnection(CWnd* pParent, /*=NULL*/ CONNECTIONSDef* pCONNECTIONS)
	: CDialog(CStopConnection::IDD, pParent)
{
  m_pCONNECTIONS = pCONNECTIONS;

	//{{AFX_DATA_INIT(CStopConnection)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStopConnection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStopConnection)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStopConnection, CDialog)
	//{{AFX_MSG_MAP(CStopConnection)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_CBN_SELENDOK(STOPCONNECTION_COMMENTCODE, OnSelendokCommentcode)
	ON_NOTIFY(LVN_COLUMNCLICK, STOPCONNECTION_FROM, OnColumnclickFrom)
	ON_NOTIFY(LVN_COLUMNCLICK, STOPCONNECTION_TO, OnColumnclickTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStopConnection message handlers

BOOL CStopConnection::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up the pointers to the controls
//	
  pListCtrlFROM = (CListCtrl *)GetDlgItem(STOPCONNECTION_FROM);
  pListCtrlTO = (CListCtrl *)GetDlgItem(STOPCONNECTION_TO);
  pEditDISTANCE = (CEdit *)GetDlgItem(STOPCONNECTION_DISTANCE);
  pStaticMEASURE = (CStatic *)GetDlgItem(STOPCONNECTION_MEASURE);
  pButtonNA = (CButton *)GetDlgItem(STOPCONNECTION_NA);
  pButtonCC = (CButton *)GetDlgItem(STOPCONNECTION_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(STOPCONNECTION_COMMENTCODE);

  DWORD dwExStyles = pListCtrlFROM->GetExtendedStyle();
  pListCtrlFROM->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  dwExStyles = pListCtrlTO->GetExtendedStyle();
  pListCtrlTO->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  pStaticMEASURE->SetWindowText(distanceMeasure == PREFERENCES_MILES ? "miles" : "km");
//
//  Display the hourglass
//
  HCURSOR hCursor = SetCursor(hCursorWait);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 50;
  LVC.pszText = "Abbr";
  pListCtrlFROM->InsertColumn(0, &LVC);
  pListCtrlTO->InsertColumn(0, &LVC);
  LVC.cx = 400;
  LVC.pszText = "Address / Intersection";
  pListCtrlFROM->InsertColumn(1, &LVC);
  pListCtrlTO->InsertColumn(1, &LVC);
  bSortForwardFrom[0] = TRUE;
  bSortForwardFrom[1] = TRUE;
  bSortForwardTo[0] = TRUE;
  bSortForwardTo[1] = TRUE;
//
//  Populate the list controls
//
  LVITEM LVI;
  char   dummy[256];
  int    rcode2;
  int    nI;

  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    BSTAT.numRecords = 1000;
  }
  m_pSCSORT = (SCSORTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SCSORTDef) * BSTAT.numRecords); 
  if(m_pSCSORT == NULL)
  {
    AllocationError(__FILE__, __LINE__, TRUE);
  }
  m_bAllowSort = m_pSCSORT != NULL;
  pListCtrlFROM->SetItemCount((int)BSTAT.numRecords);
  pListCtrlTO->SetItemCount((int)BSTAT.numRecords);
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
  int selectedFrom = NO_RECORD;
  int selectedTo = NO_RECORD;
  for(nI = 0; nI < BSTAT.numRecords; nI++)
  {
    if(rcode2 != 0)
    {
      break;
    }
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    LVI.pszText = tempString;
    if(m_bAllowSort)
    {
      m_pSCSORT[nI].NODESrecordID = NODES.recordID;
      strcpy(m_pSCSORT[nI].abbrName, tempString);
      LVI.mask |= LVIF_PARAM;
      LVI.lParam = (LPARAM)&m_pSCSORT[nI];
    }
    LVI.iItem = pListCtrlFROM->InsertItem(&LVI);
    LVI.iItem = pListCtrlTO->InsertItem(&LVI);
    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(tempString, NODES_INTERSECTION_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 1;
    LVI.pszText = tempString;
    if(m_bAllowSort)
    {
      strcpy(m_pSCSORT[nI].intersection, tempString);
    }
    pListCtrlFROM->SetItem(&LVI);
    pListCtrlTO->SetItem(&LVI);
    if(m_pCONNECTIONS->fromNODESrecordID == NODES.recordID)
    {
      selectedFrom = nI;
    }
    if(m_pCONNECTIONS->toNODESrecordID == NODES.recordID)
    {
      selectedTo = nI;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
  }
//
//  Highlight any selections
//
  if(selectedFrom != NO_RECORD)
  {
    ListView_SetItemState(pListCtrlFROM->m_hWnd, selectedFrom, LVIS_SELECTED, LVIS_SELECTED);
    pListCtrlFROM->EnsureVisible(selectedFrom, TRUE);
  }
  if(selectedTo != NO_RECORD)
  {
    ListView_SetItemState(pListCtrlTO->m_hWnd, selectedTo, LVIS_SELECTED, LVIS_SELECTED);
    pListCtrlTO->EnsureVisible(selectedTo, TRUE);
  }
//
//  Set the distance
//
  sprintf(tempString, "%5.2f", m_pCONNECTIONS->distance);
  pEditDISTANCE->SetWindowText(tempString);
//
//  And the Comment
//
  int  numComments;
  BOOL bFound;
  
  numComments = SetUpCommentList(m_hWnd, STOPCONNECTION_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
  for(bFound = FALSE, nI = 0; nI < numComments; nI++)
  {
    if(pComboBoxCOMMENTCODE->GetItemData(nI) == (DWORD)m_pCONNECTIONS->COMMENTSrecordID)
    {
      pComboBoxCOMMENTCODE->SetCurSel(nI);
      bFound = TRUE;
      break;
    }
  }
  if(bFound)
  {
    pButtonCC->SetCheck(TRUE);
  }
  else
  {
    pButtonNA->SetCheck(TRUE);
  }
//
//  Restore the cursor and leave
//
  SetCursor(hCursor);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStopConnection::OnOK() 
{
  SCSORTDef *pSC; 
  LVITEM    LVI;
  int nI;
  int rcode2;
   
//
//  Make sure he's selected a from and a to
//
  if(pListCtrlFROM->GetSelectedCount() == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_025, (HANDLE)NULL);
    return;
  }
  if(pListCtrlTO->GetSelectedCount() == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_026, (HANDLE)NULL);
    return;
  }
//
//  Get the selections
//
//  From node
//
  nI = pListCtrlFROM->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if(nI < 0)
    return;
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlFROM->GetItem(&LVI);
  pSC = (SCSORTDef *)LVI.lParam;
  long fromNODESrecordID = pSC->NODESrecordID;
//
//  To node
//
  nI = pListCtrlTO->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if(nI < 0)
    return;
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlTO->GetItem(&LVI);
  pSC = (SCSORTDef *)LVI.lParam;
  long toNODESrecordID = pSC->NODESrecordID;
//
//  Get the distance
//
  pEditDISTANCE->GetWindowText(tempString, TEMPSTRING_LENGTH);
  float distance = (float)atof(tempString);
//
//  Comment
//
  long COMMENTSrecordID;
  if(pButtonNA->GetCheck())
    COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = pComboBoxCOMMENTCODE->GetCurSel();
    COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Build up the record
//
  if(m_pCONNECTIONS->recordID != NO_RECORD)
  {
    m_pCONNECTIONS->COMMENTSrecordID = COMMENTSrecordID;
    m_pCONNECTIONS->fromNODESrecordID = fromNODESrecordID;
    m_pCONNECTIONS->toNODESrecordID = toNODESrecordID;
    m_pCONNECTIONS->distance = distance;
    btrieve(B_UPDATE, TMS_CONNECTIONS, m_pCONNECTIONS, &CONNECTIONSKey0, 0);
  }
  else
  {
    rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    m_pCONNECTIONS->recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
    m_pCONNECTIONS->COMMENTSrecordID = COMMENTSrecordID;
    m_pCONNECTIONS->fromNODESrecordID = fromNODESrecordID;
    m_pCONNECTIONS->fromROUTESrecordID = NO_RECORD;
    m_pCONNECTIONS->fromSERVICESrecordID = NO_RECORD;
    m_pCONNECTIONS->fromPATTERNNAMESrecordID = NO_RECORD;
    m_pCONNECTIONS->toNODESrecordID = toNODESrecordID;
    m_pCONNECTIONS->toROUTESrecordID = NO_RECORD;
    m_pCONNECTIONS->toSERVICESrecordID = NO_RECORD;
    m_pCONNECTIONS->toPATTERNNAMESrecordID = NO_RECORD;
    m_pCONNECTIONS->connectionTime = 0;
    m_pCONNECTIONS->fromTimeOfDay = NO_TIME;
    m_pCONNECTIONS->toTimeOfDay = NO_TIME;
    m_pCONNECTIONS->distance = distance;
    m_pCONNECTIONS->flags = CONNECTIONS_FLAG_STOPSTOP;
    m_pCONNECTIONS->ROUTINGSrecordID = NO_RECORD;
    btrieve(B_INSERT, TMS_CONNECTIONS, m_pCONNECTIONS, &CONNECTIONSKey0, 0);
  }
//
//  All done
//
  TMSHeapFree(m_pSCSORT);

	CDialog::OnOK();
}

void CStopConnection::OnCancel() 
{
  TMSHeapFree(m_pSCSORT);

	CDialog::OnCancel();
}

void CStopConnection::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Adding_a_Stop_to_Stop_Connection);
}

void CStopConnection::OnClose() 
{
  OnCancel();
}

void CStopConnection::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CStopConnection::OnColumnclickFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView && m_bAllowSort)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          SCListViewFromCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForwardFrom[pNMListView->iSubItem] = !bSortForwardFrom[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CStopConnection::OnColumnclickTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView && m_bAllowSort)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          SCListViewToCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForwardTo[pNMListView->iSubItem] = !bSortForwardTo[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

