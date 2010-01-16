// Roster.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "Roster.h"
#include "DaysOffPatterns.h"
#include "RosterParms.h"
#include "RosterMerge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MENU1_NUMITEMS            6
#define MENU1POS_IDMANUAL         0
#define MENU1POS_IDUNDOUNROSTERED 1
#define MENU1POS_IDSWAPUNROSTERED 2
#define MENU1POS_SEPARATOR1       3
#define MENU1POS_IDTAKERUN        4
#define MENU1POS_IDAUTOMATIC      5

#define MENU2_NUMITEMS            5
#define MENU2POS_IDSWAPROSTERED   0
#define MENU2POS_SEPARATOR1       1
#define MENU2POS_IDMERGE          2
#define MENU2POS_SEPARATOR2       3
#define MENU2POS_IDUNDOROSTERED   4

BOOL RosterDlg(PDISPLAYINFO);

#define LISTCTRL_NUMCOLS         11

static BOOL   bSortForwardUnrostered[LISTCTRL_NUMCOLS];
static BOOL   bSortForwardRostered[LISTCTRL_NUMCOLS];

extern "C" {

BOOL RosterDialog(PDISPLAYINFO pDI)
{
  return(RosterDlg(pDI));
}

static int bitCount( register unsigned int mask )
{
	register int	count = 0;
	while( mask )
	{
		mask &= (mask - 1);
		++count;
	}
	return count;
}

int CompressCOMBINED(void);

} // extern "C"

BOOL RosterDlg(PDISPLAYINFO pDI)
{
  CRoster dlg(NULL, pDI);

  return(dlg.DoModal());
}

int CALLBACK SCListViewUnrosteredCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  COMBINEDDef *p1 = &m_pCOMBINED[lParam1];
  COMBINEDDef *p2 = &m_pCOMBINED[lParam2];
  long p1number, p2number;
  int  iResult = 0;


  if(p1 && p2 && lParamSort <= 7)
  {
    if(lParamSort == 0) // Roster number
    {
      p1number = p1->rosterNumber;
      p2number = p2->rosterNumber;
    }
    else if(lParamSort >= 1 && lParamSort <= 7)
    {
      p1number = p1->runNumber[lParamSort - 1];
      if(p1number == 0)
      {
        p1number = 9999999;
      }
      p2number = p2->runNumber[lParamSort - 1];
      if(p2number == 0)
      {
        p2number = 9999999;
      }
   }
    if(bSortForwardUnrostered[lParamSort])
    {
      iResult = (p1number < p2number ? -1 : p1number > p2number ? 1 : 0);
    }
    else
    {
      iResult = (p1number > p2number ? -1 : p1number < p2number ? 1 : 0);
    }
  }

  return(iResult);
}

int CALLBACK SCListViewRosteredCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  COMBINEDDef *p1 = &m_pCOMBINED[lParam1];
  COMBINEDDef *p2 = &m_pCOMBINED[lParam2];
  long p1number, p2number;
  int  iResult = 0;


  if(p1 && p2 && lParamSort <= 7)
  {
    if(lParamSort == 0) // Roster number
    {
      p1number = p1->rosterNumber;
      p2number = p2->rosterNumber;
    }
    else if(lParamSort >= 1 && lParamSort <= 7)
    {
      p1number = p1->runNumber[lParamSort - 1];
      p2number = p2->runNumber[lParamSort - 1];
    }
    if(bSortForwardRostered[lParamSort])
    {
      iResult = (p1number < p2number ? -1 : p1number > p2number ? 1 : 0);
    }
    else
    {
      iResult = (p1number > p2number ? -1 : p1number < p2number ? 1 : 0);
    }
  }

  return(iResult);
}


/////////////////////////////////////////////////////////////////////////////
// CRoster dialog


CRoster::CRoster(CWnd* pParent, PDISPLAYINFO pDI)
	: CDialog(CRoster::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoster)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
 
  if(pDI == NULL)
  {
    OnCancel();
  }
  else
  {
    m_pDI = pDI;
  }

  m_dayBits[0] = 64;
  m_dayBits[1] = 32;
  m_dayBits[2] = 16;
  m_dayBits[3] =  8;
  m_dayBits[4] =  4;
  m_dayBits[5] =  2;
  m_dayBits[6] =  1;
}


void CRoster::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoster)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoster, CDialog)
	//{{AFX_MSG_MAP(CRoster)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, ROSTER_ROSTERED, OnDblclkRostered)
	ON_NOTIFY(NM_DBLCLK, ROSTER_UNROSTERED, OnDblclkUnrostered)
	ON_BN_CLICKED(IDPARAMETERS, OnParameters)
	ON_BN_CLICKED(IDSAVE, OnSave)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDMANUAL, OnManual)
	ON_COMMAND(IDAUTOMATIC, OnAutomatic)
	ON_COMMAND(IDTAKERUN, OnTakerun)
	ON_COMMAND(IDUNDOROSTERED, OnUndorostered)
	ON_COMMAND(IDUNDOUNROSTERED, OnUndounrostered)
	ON_NOTIFY(LVN_COLUMNCLICK, ROSTER_UNROSTERED, OnColumnclickUnrostered)
	ON_NOTIFY(LVN_COLUMNCLICK, ROSTER_ROSTERED, OnColumnclickRostered)
	ON_COMMAND(IDSWAPUNROSTERED, OnSwapunrostered)
	ON_COMMAND(IDSWAPROSTERED, OnSwaprostered)
	ON_COMMAND(IDMERGE, OnMerge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoster message handlers

BOOL CRoster::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CString s;
  int     nI;
  int     rcode2;
//
//  Set up pointers to the list controls
//
  pListCtrlUNROSTERED = (CListCtrl *)GetDlgItem(ROSTER_UNROSTERED);
  pListCtrlROSTERED = (CListCtrl *)GetDlgItem(ROSTER_ROSTERED);
  
  DWORD dwExStyles = pListCtrlUNROSTERED->GetExtendedStyle();
  pListCtrlUNROSTERED->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  
  dwExStyles = pListCtrlROSTERED->GetExtendedStyle();
  pListCtrlROSTERED->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

//
//  Set up the handles to the popup menus
//
  m_hMenu1 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANROSTER1));
  m_hMenu1 = GetSubMenu(m_hMenu1, 0);
  m_hMenu2 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANROSTER2));
  m_hMenu2 = GetSubMenu(m_hMenu2, 0);
//
//  And set up serviceDays to match to what's saved in ROSTERPARMS
//
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    if((m_serviceDays[nI] = ROSTERPARMS.serviceDays[nI]) != NO_RECORD)
    {
      SERVICESKey0.recordID = ROSTERPARMS.serviceDays[nI];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      m_serviceDays[nI] = (rcode2 == 0 ? SERVICES.recordID : NO_RECORD);
    }
  }
//
//  Build the list control titles
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
  int col = 0;
//
//  "Number"
//
  LVC.cx = 50;
  s.LoadString(TEXT_072);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlUNROSTERED->InsertColumn(col, &LVC);
  pListCtrlROSTERED->InsertColumn(col, &LVC);
//
//  Cycle through the days of the week
//
  LVC.cx = 100;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    col++;
    s.LoadString(TEXT_009 + nI);
    strcpy(tempString, s);
    LVC.pszText = tempString;
    pListCtrlUNROSTERED->InsertColumn(col, &LVC);
    pListCtrlROSTERED->InsertColumn(col, &LVC);
  }
//
//  "Hours"
//
  LVC.cx = 56;
  col++;
  s.LoadString(TEXT_073);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlUNROSTERED->InsertColumn(col, &LVC);
  pListCtrlROSTERED->InsertColumn(col, &LVC);
//
//  "O/T"
//
  col++;
  s.LoadString(TEXT_075);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlUNROSTERED->InsertColumn(col, &LVC);
  pListCtrlROSTERED->InsertColumn(col, &LVC);
//
//  "Pay"
//
  col++;
  s.LoadString(TEXT_076);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlUNROSTERED->InsertColumn(col, &LVC);
  pListCtrlROSTERED->InsertColumn(col, &LVC);
//
//  Establish GlobalRosterNumber
//
  ROSTERKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID + 1;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  if(rcode2 != 0)
  {
    rcode2 = btrieve(B_GETLAST, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
  else
  {
    rcode2 = btrieve(B_GETPREVIOUS, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
  GlobalRosterNumber = ((rcode2 == 0 && ROSTER.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID) ? ROSTER.rosterNumber + 1 : 1);
//
//  Set the column sorts
//
  for(nI = 0; nI < LISTCTRL_NUMCOLS; nI++)
  {
    bSortForwardUnrostered[nI] = TRUE;
    bSortForwardRostered[nI] = TRUE;
  }
//
//  Set the title
//
  GetWindowText(szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, m_pDI->fileInfo.rosterWeek + 1);
  SetWindowText(tempString);
//
// 	Display what we've got
//
  DisplayPanels(TRUE, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoster::OnClose() 
{
  OnCancel();
}

void CRoster::OnDblclkRostered(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnUndorostered();

	*pResult = 0;
}

void CRoster::OnDblclkUnrostered(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnTakerun();

	*pResult = 0;
}

void CRoster::OnParameters() 
{
  CRosterParms dlg;

  dlg.DoModal();
}

void CRoster::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  HMENU hMenu = NULL;
  int nI;

  if(pWnd == pListCtrlUNROSTERED)
  {
    hMenu = m_hMenu1;
    for(nI = 0; nI < MENU1_NUMITEMS; nI++)
    {
      EnableMenuItem(hMenu, nI, MF_BYPOSITION | MF_ENABLED);
    }
    if(pListCtrlUNROSTERED->GetItemCount() == 0)
    {
      EnableMenuItem(hMenu, MENU1POS_IDAUTOMATIC, MF_BYPOSITION	| MF_GRAYED);
    }
    EnableMenuItem(hMenu, MENU1POS_IDSWAPUNROSTERED, MF_BYPOSITION	| MF_GRAYED);
    nI = pListCtrlUNROSTERED->GetSelectedCount();
    if(nI == 0 || nI == LB_ERR)
    {
      EnableMenuItem(hMenu, MENU1POS_IDMANUAL, MF_BYPOSITION	| MF_GRAYED);
      EnableMenuItem(hMenu, MENU1POS_IDUNDOUNROSTERED, MF_BYPOSITION	| MF_GRAYED);
      EnableMenuItem(hMenu, MENU1POS_IDTAKERUN, MF_BYPOSITION	| MF_GRAYED);
    }
    else if(nI == 1)
    {
      EnableMenuItem(hMenu, MENU1POS_IDMANUAL, MF_BYPOSITION	| MF_GRAYED);
    }
    else if(nI == 2)
    {
      EnableMenuItem(hMenu, MENU1POS_IDSWAPUNROSTERED, MF_BYPOSITION	| MF_ENABLED);
    }
  }
  else if(pWnd == pListCtrlROSTERED)
  {
    hMenu = m_hMenu2;
    for(nI = 0; nI < MENU2_NUMITEMS; nI++)
    {
      EnableMenuItem(hMenu, nI, MF_BYPOSITION | MF_ENABLED);
    }
    EnableMenuItem(hMenu, MENU2POS_IDSWAPROSTERED, MF_BYPOSITION	| MF_GRAYED);
    EnableMenuItem(hMenu, MENU2POS_IDMERGE, MF_BYPOSITION | MF_GRAYED);
    nI = pListCtrlROSTERED->GetSelectedCount();
    if(nI == 0 || nI == LB_ERR)
    {
      EnableMenuItem(hMenu, MENU2POS_IDUNDOROSTERED, MF_BYPOSITION	| MF_GRAYED);
    }
    else if(nI == 1)
    {
      if(m_pDI->fileInfo.rosterWeek != 0)
      {
        EnableMenuItem(hMenu, MENU2POS_IDMERGE, MF_BYPOSITION	| MF_ENABLED);
      }
    }
    else if(nI == 2)
    {
      EnableMenuItem(hMenu, MENU2POS_IDSWAPROSTERED, MF_BYPOSITION	| MF_ENABLED);
    }
  }
  if(hMenu != NULL)
  {
    TrackPopupMenu(hMenu, 0, point.x, point.y, 0, m_hWnd, NULL);
  }
}

int CRoster::GetSelections(CListCtrl* pListCtrl, CUIntArray* pIndexesIntoCOMBINED)
{
  long previousIndex;
  long index;
  int  nItem;
//
//  Get the selections
//  
  POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
  {
    return(0);
  }

  previousIndex = NO_RECORD;
  pIndexesIntoCOMBINED->RemoveAll();
//
//  Save a list of indexes into COMBINED
//
  while(pos)
  {
    nItem = pListCtrl->GetNextSelectedItem(pos);
    index = pListCtrl->GetItemData(nItem);
    if(index == NO_RECORD)
    {
      continue;
    }
    if(index == previousIndex)
    {
      continue;
    }
    previousIndex = index;
    pIndexesIntoCOMBINED->Add(index);
  }

  return(pIndexesIntoCOMBINED->GetSize());
}

void CRoster::OnManual() 
{
  CUIntArray indexesIntoCOMBINED;
  BOOL bOverlap;
  int  nI, nJ, nK, nL;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlUNROSTERED, &indexesIntoCOMBINED);

  if(numIndexes == 0)
  {
    return;
  }
//
//  Loop through each of the selections to ensure that he's not
//  about to clobber one day with another on the combine
//
  char testBits = 0;

  for(bOverlap = FALSE, nI = 0; nI < numIndexes; nI++)
  {
    if(bOverlap)
    {
      break;
    }
    nJ = indexesIntoCOMBINED.GetAt(nI);
    for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
    {
      if(m_pCOMBINED[nJ].RUNSrecordID[nK] != NO_RECORD)
      {
        if(testBits & (char)m_dayBits[nK])
        {
          TMSError(m_hWnd, MB_ICONSTOP, ERROR_164, (HANDLE)NULL);
          bOverlap = TRUE;
          break;
        }
        testBits |= (char)m_dayBits[nK];
      }
    }
  }
  if(bOverlap)
  {
    return;
  }
//
//  For the combine, the first record selected in the list box
//  is the one that will contain all the others
//
  nI = indexesIntoCOMBINED.GetAt(0);
  for(nJ = 1; nJ < numIndexes; nJ++)
  {
    nK = indexesIntoCOMBINED.GetAt(nJ);
    for(nL = 0; nL < ROSTER_MAX_DAYS; nL++)
    {
      if(m_pCOMBINED[nK].RUNSrecordID[nL] != NO_RECORD)
      {
        m_pCOMBINED[nI].RUNSrecordID[nL] = m_pCOMBINED[nK].RUNSrecordID[nL];
        m_pCOMBINED[nI].flags[nL] = m_pCOMBINED[nK].flags[nL];
        m_pCOMBINED[nK].RUNSrecordID[nL] = NO_RECORD;
        m_pCOMBINED[nK].flags[nL] = 0;
      }
    }
  }
//
//  Collapse the COMBINED structure
//
  m_numCOMBINED = CompressCOMBINED();
//
//  Refresh the display
//
  DisplayPanels(TRUE, FALSE);
}

void CRoster::OnAutomatic() 
{
  CString s;
  HCURSOR saveCursor;
//
//  Check the number of days to roster
//
  if(ROSTERPARMS.workWeek < 1 || ROSTERPARMS.workWeek > ROSTER_MAX_DAYS)
  {
    MessageBeep(MB_ICONSTOP);
    s.LoadString(ERROR_170);
    sprintf(tempString, s, ROSTER_MAX_DAYS);
    MessageBox(tempString, TMS, MB_ICONSTOP);
    return;
  }
//
//  Start combining...
//
  saveCursor = SetCursor(hCursorWait);
//
//  Setup the allowable days off patterns based on the flags.
//  Monday is (1<<0), Tuesday is (1<<2), etc.
//
	  unsigned int patterns[128], *pCur = patterns, p;
	  for( p = 0; p < 128; ++p )
	  {
		  if( ROSTERPARMS.daysOffPatterns[p] == 'Y' )
			  *pCur++ = (127 ^ p);
	  }
	  *pCur = 0;	// Terminate the patterns with a zero.
//
//  Create the optimized rosters.
//
  makeRoster(ROSTERPARMS.workWeek, patterns,
        (ROSTERPARMS.flags & ROSTERPARMS_FLAG_PREFEREXTRASONWEEKENDS) != 0,
        (ROSTERPARMS.flags & ROSTERPARMS_FLAG_EQUALIZEWORK) != 0,
        ROSTERPARMS.minWork != NO_TIME ? ROSTERPARMS.minWork : 0,
        ROSTERPARMS.maxWork != NO_TIME ? ROSTERPARMS.maxWork : 1000L*60*60,
        ROSTERPARMS.nightWorkAfter < 0 ? 7 : (0 <= ROSTERPARMS.maxNightWork && ROSTERPARMS.maxNightWork <= 7) ? ROSTERPARMS.maxNightWork : 7);
//
//  Refresh the display
//
  DisplayPanels(TRUE, TRUE);
  SetCursor(saveCursor);
}

void CRoster::OnTakerun() 
{
  CUIntArray indexesIntoCOMBINED;
  int  nI, nJ;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlUNROSTERED, &indexesIntoCOMBINED);

  if(numIndexes == 0)
  {
    return;
  }
//
//  Adjust COMBINED to reflect these rows as "Rostered"
//
  for(nI = 0; nI < numIndexes; nI++)
  {
    nJ = indexesIntoCOMBINED.GetAt(nI);
    m_pCOMBINED[nJ].frozen = TRUE;
    m_pCOMBINED[nJ].rosterNumber = GlobalRosterNumber++;
    m_pCOMBINED[nJ].DRIVERSrecordID = NO_RECORD;
    m_pCOMBINED[nJ].COMMENTSrecordID = NO_RECORD;
  }
//
//  Refresh the display
//
  DisplayPanels(TRUE, TRUE);
}

void CRoster::OnUndounrostered() 
{
  ROSTERDAYSUNDOSWAPDef RDUS;
  HCURSOR saveCursor;
  int  nI, nJ, nK, nL;
//
//  Get the days to undo
//
  RDUS.bSwap = FALSE;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    RDUS.bDays[nI] = FALSE;
  }
  if(!DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UNDOROSTER), m_hWnd, (DLGPROC)UNDOROSTERMsgProc, (LPARAM)&RDUS))
  {
    return;
  }

  CUIntArray indexesIntoCOMBINED;
  BOOL bFound;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlUNROSTERED, &indexesIntoCOMBINED);

  if(numIndexes == 0)
  {
    return;
  }
//
//  Loop through the selections undoing the days specified in UNDOROSTER
//
  saveCursor = SetCursor(hCursorWait);
  for(nI = 0; nI < numIndexes; nI++)
  {
    nJ = indexesIntoCOMBINED.GetAt(nI);
    m_pCOMBINED[nJ].frozen = FALSE;
    m_pCOMBINED[nJ].rosterNumber = NO_RECORD;
    for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
    {
      if(!RDUS.bDays[nK] || m_pCOMBINED[nJ].RUNSrecordID[nK] == NO_RECORD)
      {
        continue;
      }
//
//  See if this is the only day on the run to undo.  If it is, leave it be.
//
      for(bFound = FALSE, nL = 0; nL < ROSTER_MAX_DAYS; nL++)
      {
        if(nK != nL && m_pCOMBINED[nJ].RUNSrecordID[nL] != NO_RECORD)
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        continue;
      }
      if(m_numCOMBINED >= m_maxCOMBINED)
      {
        m_maxCOMBINED += 512;
        m_pCOMBINED = (COMBINEDDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, m_pCOMBINED, sizeof(COMBINEDDef) * m_maxCOMBINED); 
        if(m_pCOMBINED == NULL)
        {
          AllocationError(__FILE__, __LINE__, TRUE);
          break;
        }
      }
      m_pCOMBINED[m_numCOMBINED].COMMENTSrecordID = NO_RECORD;
      m_pCOMBINED[m_numCOMBINED].DIVISIONSrecordID = NO_RECORD;
      m_pCOMBINED[m_numCOMBINED].DRIVERSrecordID = NO_RECORD;
      m_pCOMBINED[m_numCOMBINED].rosterNumber = NO_RECORD;
      m_pCOMBINED[m_numCOMBINED].frozen = FALSE;
      for(nL = 0; nL < ROSTER_MAX_DAYS; nL++)
      {
        m_pCOMBINED[m_numCOMBINED].RUNSrecordID[nL] = NO_RECORD;
      }
      m_pCOMBINED[m_numCOMBINED].RUNSrecordID[nK] = m_pCOMBINED[nJ].RUNSrecordID[nK];
      m_pCOMBINED[m_numCOMBINED].flags[nK] = m_pCOMBINED[nJ].flags[nK];
      m_pCOMBINED[nJ].RUNSrecordID[nK] = NO_RECORD;
      m_pCOMBINED[nJ].flags[nK] = 0;
      m_numCOMBINED++;
    }
  }
//
//  Refresh the display
//
  DisplayPanels(TRUE, FALSE);
  SetCursor(saveCursor);
}

void CRoster::OnUndorostered() 
{

  CUIntArray indexesIntoCOMBINED;
  int  nI, nJ;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlROSTERED, &indexesIntoCOMBINED);

  if(numIndexes == 0)
  {
    return;
  }

  for(nI = 0; nI < numIndexes; nI++)
  {
    nJ = indexesIntoCOMBINED.GetAt(nI);
    m_pCOMBINED[nJ].frozen = FALSE;
    m_pCOMBINED[nJ].rosterNumber = NO_RECORD;
  }
//
//  Refresh the display
//
  DisplayPanels(TRUE, TRUE);
}

void CRoster::DisplayPanels(BOOL bDoUNROSTERED, BOOL bDoROSTERED)
{
  CListCtrl* pListCtrl;
  CString s;
  LVITEM LVI;
//
//  Save the current position in the list controls
//
  int currentUnrosteredRow = pListCtrlUNROSTERED->GetTopIndex();
  int currentRosteredRow = pListCtrlROSTERED->GetTopIndex();

  if(currentUnrosteredRow < 0)
  {
    currentUnrosteredRow = 0;
  }

  if(currentRosteredRow < 0)
  {
    currentRosteredRow = 0;
  }
//
//  Clear the contents
//
  if(bDoUNROSTERED)
  {
    pListCtrlUNROSTERED->DeleteAllItems();
  }
  if(bDoROSTERED)
  {
    pListCtrlROSTERED->DeleteAllItems();
  }
//
//  Display COMBINED
//
  long weekPayTime;
  long overtimePay;
  BOOL bFound;
  int  itemNumber[2];
  int  iItem;
  int  nI, nJ, nK, nL;
  int  serviceIndex;
  int  runIndex;

  itemNumber[0] = 0;
  itemNumber[1] = 0;

  for(nI = 0; nI < m_numCOMBINED; nI++)
  {
//
//  Which control?
//
    if(m_pCOMBINED[nI].frozen)
    {
      if(!bDoROSTERED)
      {
        continue;
      }
      pListCtrl = pListCtrlROSTERED;
      iItem = 1;
    }
    else
    {
      if(!bDoUNROSTERED)
      {
        continue;
      }
      pListCtrl = pListCtrlUNROSTERED;
      iItem = 0;
    }
//
//  Roster Number
//
    if(m_pCOMBINED[nI].rosterNumber == NO_RECORD)
    {
      strcpy(tempString, "");
    }
    else
    {
      sprintf(tempString, "%ld", m_pCOMBINED[nI].rosterNumber);
    }
    LVI.mask = LVIF_PARAM | LVIF_TEXT;
    LVI.iItem = itemNumber[iItem];
    LVI.iSubItem = 0;
    LVI.lParam = nI;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrl->InsertItem(&LVI);
//
//  Go through the days - the data's actually held in m_pRRLIST
//
    LVI.iSubItem = 1;
    weekPayTime = 0;
    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
//
//  If the day is NO_RECORD, display OFF if it's a rostered run
//
      s = "";
      if(m_pCOMBINED[nI].RUNSrecordID[nJ] == NO_RECORD)
      {
        if(m_pCOMBINED[nI].frozen)
        {
          s.LoadString(TEXT_074);
          LVI.mask = LVIF_TEXT;
          strcpy(tempString, s);
          LVI.pszText = tempString;
          pListCtrl->SetItem(&LVI);
        }
      }
//
//  The day wasn't NO_RECORD
//
      else
      {
        nK = 0;
        bFound = FALSE;
        while(m_pRRLIST[nK].SERVICESrecordID != NO_RECORD)
        {
          for(nL = 0; nL < m_pRRLIST[nK].numRuns; nL++)
          {
            if((m_pRRLIST[nK].pData[nL].RUNSrecordID == m_pCOMBINED[nI].RUNSrecordID[nJ]) &&
                  (m_pRRLIST[nK].pData[nL].flags == m_pCOMBINED[nI].flags[nJ]))
            {
              serviceIndex = nK;
              runIndex = nL;
              bFound = TRUE;
              break;
            }
          }
          if(bFound)
          {
            break;
          }
          nK++;
        }
//
//  Display data as per selection
//
        if(bFound)
        {
//
//  Run number
//
          sprintf(szarString, "%ld", m_pRRLIST[serviceIndex].pData[runIndex].runNumber);
          s += szarString;
          s += "-";
          m_pCOMBINED[nI].runNumber[nJ] = m_pRRLIST[serviceIndex].pData[runIndex].runNumber;
//
//  On location
//
          NODESKey0.recordID = m_pRRLIST[serviceIndex].pData[runIndex].onNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          s += tempString;
          s += "@";
//
//  On time
//
          s += Tchar(m_pRRLIST[serviceIndex].pData[runIndex].onTime);
          s += "/";
//
//  Off location
//
          NODESKey0.recordID = m_pRRLIST[serviceIndex].pData[runIndex].offNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          s += tempString;
          s += "@";
//
//  Off time
//
          s += Tchar(m_pRRLIST[serviceIndex].pData[runIndex].offTime);
          s += "(";
//
//  Run type
//
          nK = (short int)LOWORD(m_pRRLIST[serviceIndex].pData[runIndex].runtype);
          nL = (short int)HIWORD(m_pRRLIST[serviceIndex].pData[runIndex].runtype);
          s += RUNTYPE[nK][nL].localName;
          s += " Pays ";
//
//  Pay time
//
          s += chhmm(m_pRRLIST[serviceIndex].pData[runIndex].payTime);
          s += ")";
          weekPayTime += m_pRRLIST[serviceIndex].pData[runIndex].payTime;
//
//  Add it in
//
          LVI.mask = LVIF_TEXT;
          strcpy(tempString, s);
          LVI.pszText = tempString;
          pListCtrl->SetItem(&LVI);
        }
      }
      LVI.iSubItem++;
    }
//
//  Weekly pay
//
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(weekPayTime));
    LVI.pszText = tempString;
    pListCtrl->SetItem(&LVI);
    LVI.iSubItem++;
//
//  Overtime
//
    if(weekPayTime > OVERTIME.weeklyAfter[WEEK5DAYS])
    {
      overtimePay = (long)((weekPayTime - OVERTIME.weeklyAfter[WEEK5DAYS]) * (OVERTIME.weeklyRate[WEEK5DAYS] - 1));
    }
    else
    {
      overtimePay = 0;
    }
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(overtimePay));
    LVI.pszText = tempString;
    pListCtrl->SetItem(&LVI);
    LVI.iSubItem++;
    weekPayTime += overtimePay;
//
//  Weekly pay including overtime
//
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(weekPayTime));
    LVI.pszText = tempString;
    pListCtrl->SetItem(&LVI);
//
//  Bump up the item number and go back
//
    itemNumber[iItem]++;
  }
//
//  (Re)position to the previous top-of-list
//
  if(bDoUNROSTERED)
  {
    pListCtrlUNROSTERED->EnsureVisible(currentUnrosteredRow, FALSE);
  }
  if(bDoROSTERED)
  {
    bSortForwardRostered[0] = TRUE;
    ListView_SortItems(pListCtrlROSTERED->m_hWnd, SCListViewRosteredCompareProc, 0L);
    pListCtrlROSTERED->EnsureVisible(currentRosteredRow, FALSE);
  }
}

void CRoster::OnCancel() 
{
  CString s;

  s.LoadString(ERROR_159);

  MessageBeep(MB_ICONINFORMATION);
  if(MessageBox(s, TMS, MB_ICONINFORMATION | MB_YESNO) == IDYES)
  {
    CDialog::OnCancel();
  }
}

void CRoster::OnHelp() 
{
	// Edward Sitarski
	DaysOffPatterns	dop(NULL);

	// Initialize the starting days off.
	const int numDaysOff = 7 - ROSTERPARMS.workWeek;	// In the real code, work week needs to come from the dialog.
	int patterns[128], *p = patterns, i;
	for( i = 0; i < 128; ++i )
	{
		if( bitCount(i) == numDaysOff && ROSTERPARMS.daysOffPatterns[i] == 'Y' )
			*p++ = i;
	}
	*p = 0;
	dop.setDaysOffPatterns( patterns, numDaysOff );

	// If the dialog exited with OK, keep the result, otherwise discard it.
	// In the real code, we will need to copy this into the local copy.
	if( dop.DoModal() == 1 )
	{
		const bool *b = dop.getDaysOffPatternFlags();
		for( i = 0; i < 128; ++i )
			ROSTERPARMS.daysOffPatterns[i] = (b[i] ? 'Y' : 'N');
	}
	// End Edward Sitarski

  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Roster_Table);
}

void CRoster::OnSave() 
{
  CString s;

  MessageBeep(MB_ICONINFORMATION);
  s.LoadString(ERROR_162);
  if(MessageBox(s, TMS, MB_ICONINFORMATION | MB_YESNO) == IDNO)
  {
    return;
  }

  HCURSOR saveCursor = SetCursor(hCursorWait);
  int nI, nJ, nK, nL;
  int rcode2;
//
//  Zero out the existing roster
//
  ROSTERKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 && ROSTER.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID)
  {
    memset(&ROSTER.WEEK[m_pDI->fileInfo.rosterWeek], 0x00, sizeof(ROSTERWEEKDef));
    btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Go through the "ROSTERED" listbox and save the incore roster
//
  BOOL bUpdate;

  nI = pListCtrlROSTERED->GetItemCount();
  for(nJ = 0; nJ < nI; nJ++)
  {
    nK = (int)pListCtrlROSTERED->GetItemData(nJ);
//
//  Find the roster number
//
    ROSTERKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
    ROSTERKey1.rosterNumber = m_pCOMBINED[nK].rosterNumber;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    bUpdate = (rcode2 == 0);
//
//  If it wasn't there set up a new record
//
    if(!bUpdate)
    {
      rcode2 = btrieve(B_GETLAST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      ROSTER.recordID = AssignRecID(rcode2, ROSTER.recordID);
      ROSTER.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
      ROSTER.rosterNumber = m_pCOMBINED[nK].rosterNumber;
      ROSTER.DRIVERSrecordID = m_pCOMBINED[nK].DRIVERSrecordID;
      ROSTER.COMMENTSrecordID = m_pCOMBINED[nK].COMMENTSrecordID;
      memset(&ROSTER.WEEK, 0x00, sizeof(ROSTERWEEKDef) * ROSTER_MAX_WEEKS);
      memset(&ROSTER.reserved, 0x00, ROSTER_RESERVED_LENGTH);
      ROSTER.flags = 0;
    }
//
//  Modify the record
//
    ROSTER.WEEK[m_pDI->fileInfo.rosterWeek].flags = 0;
    for(nL = 0; nL < ROSTER_MAX_DAYS; nL++)
    {
      ROSTER.WEEK[m_pDI->fileInfo.rosterWeek].RUNSrecordIDs[nL] = m_pCOMBINED[nK].RUNSrecordID[nL];
      if(m_pCOMBINED[nK].flags[nL] & RTFLAGS_CREWONLY)
      {
        ROSTER.WEEK[m_pDI->fileInfo.rosterWeek].flags |= (1 << nL);
      }
    }
//
//  Insert / update
//
    if(bUpdate)
    {
      btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
    else
    {
      btrieve(B_INSERT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
    }
  }

  SetCursor(saveCursor);
}

void CRoster::OnOK() 
{
	OnSave();
	
	CDialog::OnOK();
}


void CRoster::OnColumnclickUnrostered(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          SCListViewUnrosteredCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForwardUnrostered[pNMListView->iSubItem] = !bSortForwardUnrostered[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CRoster::OnColumnclickRostered(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
 
  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          SCListViewRosteredCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForwardRostered[pNMListView->iSubItem] = !bSortForwardRostered[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CRoster::OnSwapunrostered() 
{
  ROSTERDAYSUNDOSWAPDef RDUS;
  int  nI, nJ, nK;
//
//  Get the days to swap
//
  RDUS.bSwap = TRUE;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    RDUS.bDays[nI] = FALSE;
  }
  if(!DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UNDOROSTER), m_hWnd, (DLGPROC)UNDOROSTERMsgProc, (LPARAM)&RDUS))
  {
    return;
  }

  CUIntArray indexesIntoCOMBINED;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlUNROSTERED, &indexesIntoCOMBINED);

  if(numIndexes != 2)
  {
    return;
  }
//
//  Perform the swap
//
  long tempLong;

  nI = indexesIntoCOMBINED.GetAt(0);
  nJ = indexesIntoCOMBINED.GetAt(1);

  for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
  {
    if(!RDUS.bDays[nK])
    {
      continue;
    }
//
//  Flags
//
    tempLong = m_pCOMBINED[nI].flags[nK];
    m_pCOMBINED[nI].flags[nK] = m_pCOMBINED[nJ].flags[nK];
    m_pCOMBINED[nJ].flags[nK] = tempLong;
//
//  Runs recordIDs
//
    tempLong = m_pCOMBINED[nI].RUNSrecordID[nK];
    m_pCOMBINED[nI].RUNSrecordID[nK] = m_pCOMBINED[nJ].RUNSrecordID[nK];
    m_pCOMBINED[nJ].RUNSrecordID[nK] = tempLong;
//
//  Run numbers
//
    tempLong = m_pCOMBINED[nI].runNumber[nK];
    m_pCOMBINED[nI].runNumber[nK] = m_pCOMBINED[nJ].runNumber[nK];
    m_pCOMBINED[nJ].runNumber[nK] = tempLong;
  }
//
//  Refresh the display
//
  DisplayPanels(TRUE, FALSE);
}

void CRoster::OnSwaprostered() 
{
  ROSTERDAYSUNDOSWAPDef RDUS;
  int  nI, nJ, nK;
//
//  Get the days to swap
//
  RDUS.bSwap = TRUE;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    RDUS.bDays[nI] = FALSE;
  }
  if(!DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UNDOROSTER), m_hWnd, (DLGPROC)UNDOROSTERMsgProc, (LPARAM)&RDUS))
  {
    return;
  }

  CUIntArray indexesIntoCOMBINED;
//
//  Get the selections
//  
  int numIndexes = GetSelections(pListCtrlROSTERED, &indexesIntoCOMBINED);

  if(numIndexes != 2)
  {
    return;
  }
//
//  Perform the swap
//
  long tempLong;

  nI = indexesIntoCOMBINED.GetAt(0);
  nJ = indexesIntoCOMBINED.GetAt(1);

  for(nK = 0; nK < ROSTER_MAX_DAYS; nK++)
  {
    if(!RDUS.bDays[nK])
    {
      continue;
    }
//
//  Flags
//
    tempLong = m_pCOMBINED[nI].flags[nK];
    m_pCOMBINED[nI].flags[nK] = m_pCOMBINED[nJ].flags[nK];
    m_pCOMBINED[nJ].flags[nK] = tempLong;
//
//  Runs recordIDs
//
    tempLong = m_pCOMBINED[nI].RUNSrecordID[nK];
    m_pCOMBINED[nI].RUNSrecordID[nK] = m_pCOMBINED[nJ].RUNSrecordID[nK];
    m_pCOMBINED[nJ].RUNSrecordID[nK] = tempLong;
//
//  Run numbers
//
    tempLong = m_pCOMBINED[nI].runNumber[nK];
    m_pCOMBINED[nI].runNumber[nK] = m_pCOMBINED[nJ].runNumber[nK];
    m_pCOMBINED[nJ].runNumber[nK] = tempLong;
  }
//
//  Refresh the display
//
  DisplayPanels(FALSE, TRUE);
}

void CRoster::OnMerge() 
{
  CUIntArray indexesIntoCOMBINED;
//
//  Get the selection
//  
  int numIndexes = GetSelections(pListCtrlROSTERED, &indexesIntoCOMBINED);

  if(numIndexes == 0)
  {
    return;
  }

  CRosterMerge dlg(NULL, indexesIntoCOMBINED.GetAt(0), m_pDI->fileInfo.divisionRecordID);

  if(dlg.DoModal() == IDOK)
  {
    DisplayPanels(FALSE, TRUE);
  }
}
