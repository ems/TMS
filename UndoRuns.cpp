// UndoRuns.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "tms.h"
#include "UndoRuns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUndoRuns dialog


CUndoRuns::CUndoRuns(CWnd* pParent, CUIntArray *pRunNumbers)
	: CDialog(CUndoRuns::IDD, pParent)
{
  m_pRunNumbers = pRunNumbers;

	//{{AFX_DATA_INIT(CUndoRuns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUndoRuns::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUndoRuns)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUndoRuns, CDialog)
	//{{AFX_MSG_MAP(CUndoRuns)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDSELECTALL, OnSelectall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUndoRuns message handlers

BOOL CUndoRuns::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pListCtrlLIST = (CListCtrl *)GetDlgItem(UNDORUNS_LIST);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  pListCtrlLIST->DeleteAllItems();

//
//  Set up the list control
//
//  Titles
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 65;
  LVC.pszText = "Run";
  pListCtrlLIST->InsertColumn(0, &LVC);
  LVC.cx = 130;
  LVC.pszText = "Runtype";
  pListCtrlLIST->InsertColumn(1, &LVC);
  LVC.cx = 70;
  LVC.pszText = "Pay Time";
  pListCtrlLIST->InsertColumn(2, &LVC);
//
//  Runs/runtypes
//
  COSTDef COST; 
  LVITEM  LVI;
  BOOL bFound;
  long cutAsRuntype;
  long tempLong;
  int  RLDIndex;
  int  nI, nJ;

  LVI.iItem = 0;
  for(nI = 0; nI < m_pRunNumbers->GetSize(); nI++)
  {
//
//  Run number
//
    LVI.mask = LVIF_TEXT| LVIF_PARAM;
    LVI.lParam = m_pRunNumbers->GetAt(nI);
    LVI.iSubItem = 0;
    sprintf(tempString, "%ld", (long)m_pRunNumbers->GetAt(nI));
    LVI.pszText = tempString;
    LVI.iItem = nI;
    pListCtrlLIST->InsertItem(&LVI);
//
//  Runtype
//
    for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLIST[nJ].runNumber == (long)m_pRunNumbers->GetAt(nI))
      {
        bFound = TRUE;
        break;
      }
    }
    if(bFound)
    {
      cutAsRuntype = NO_RUNTYPE;
      for(RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
      {
        if(RUNLISTDATA[RLDIndex].runNumber == (long)m_pRunNumbers->GetAt(nI))
        {
          cutAsRuntype = RUNLISTDATA[RLDIndex].cutAsRuntype;
          break;
        }
      }
      tempLong = RunCoster(&RUNLIST[nJ].run, cutAsRuntype, &COST);
      if(tempLong == UNCLASSIFIED_RUNTYPE)
        strcpy(tempString, "Illegal");
      else
      {
        int runtype = LOWORD(tempLong);
        int runtypeSlot = HIWORD(tempLong);
        strcpy(tempString, RUNTYPE[runtype][runtypeSlot].localName);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem = 2;
      LVI.pszText = chhmm(COST.TOTAL.payTime);
      pListCtrlLIST->SetItem(&LVI);
    }
  }

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUndoRuns::OnClose() 
{
  OnCancel();
}

void CUndoRuns::OnHelp() 
{
}

void CUndoRuns::OnCancel() 
{
	CDialog::OnCancel();
}

void CUndoRuns::OnSelectall() 
{
  int nI;
  int numRows;
  
  numRows = pListCtrlLIST->GetItemCount();

  for(nI = 0; nI < numRows; nI++)
  {
    pListCtrlLIST->SetItemState(nI, LVIS_SELECTED, LVIS_SELECTED);
  }
  pListCtrlLIST->SetFocus();
}

void CUndoRuns::OnOK() 
{
  long runNumber;
  long previousRunNumber = NO_RECORD;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  m_pRunNumbers->RemoveAll();

  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
    {
      continue;
    }
    if(runNumber == previousRunNumber)
    {
      continue;
    }
    previousRunNumber = runNumber;
    m_pRunNumbers->Add(runNumber);
  }
	
	CDialog::OnOK();
}
