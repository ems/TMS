//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// JumpToRun.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "JumpToRun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJumpToRun dialog


CJumpToRun::CJumpToRun(CWnd* pParent, CUIntArray *pRunNumbers, long *pRun)
	: CDialog(CJumpToRun::IDD, pParent)
{
  m_pRunNumbers = pRunNumbers;
  m_pRun = pRun;

	//{{AFX_DATA_INIT(CJumpToRun)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CJumpToRun::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJumpToRun)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJumpToRun, CDialog)
	//{{AFX_MSG_MAP(CJumpToRun)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJumpToRun message handlers

BOOL CJumpToRun::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pListCtrlLIST = (CListCtrl *)GetDlgItem(JUMPTORUN_LIST);
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

void CJumpToRun::OnOK() 
{
  int nItem;

  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
//
//  Determine the selection(s)
//
  if(pos == NULL)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_006 , (HANDLE)NULL);
  }
  else
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    *m_pRun = pListCtrlLIST->GetItemData(nItem);
	
    CDialog::OnOK();
  }
}

void CJumpToRun::OnCancel() 
{
  *m_pRun = NO_RECORD;

	CDialog::OnCancel();
}

void CJumpToRun::OnClose() 
{
  *m_pRun = NO_RECORD;

	CDialog::OnClose();
}

void CJumpToRun::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}
