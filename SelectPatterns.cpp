// SelectPatterns.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "ResizingDialog.h"
#include "TripGraph.h"
#include "SelectPatterns.h"

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectPatterns dialog


CSelectPatterns::CSelectPatterns(CWnd* pParent, PATSELDef* pPATSEL)
	: CDialog(CSelectPatterns::IDD, pParent)
{
  m_pPATSEL = pPATSEL;

	//{{AFX_DATA_INIT(CSelectPatterns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectPatterns::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPatterns)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectPatterns, CDialog)
	//{{AFX_MSG_MAP(CSelectPatterns)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPatterns message handlers

BOOL CSelectPatterns::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int nI, nJ, nK;

  pListCtrlOUTBOUND = (CListCtrl *)GetDlgItem(SELECTPATTERNS_OUTBOUND);
  pListCtrlINBOUND = (CListCtrl *)GetDlgItem(SELECTPATTERNS_INBOUND);
//
//  Extend the list controls
//
  DWORD dwExStyles;
  
  dwExStyles = pListCtrlOUTBOUND->GetExtendedStyle();
  pListCtrlOUTBOUND->SetExtendedStyle(dwExStyles | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
  dwExStyles = pListCtrlINBOUND->GetExtendedStyle();
  pListCtrlINBOUND->SetExtendedStyle(dwExStyles | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  CListCtrl* pListCtrl;

  ROUTESKey0.recordID = m_pPATSEL->ROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  for(nI = 0; nI < 2; nI++)
  {
    pListCtrl = nI == 0 ? pListCtrlOUTBOUND : pListCtrlINBOUND;
    if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
    {
      pListCtrl->EnableWindow(FALSE);
      break;
    }
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
    LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    LVC.fmt = LVCFMT_LEFT;
    LVC.cx = 132;
    LVC.pszText = tempString;
    pListCtrl->InsertColumn(0, &LVC);
  }
//
//  Add the pattern names to the list controls
//
  LVITEM LVI;

  nK = 0;
  for(nI = 0; nI < 2; nI++)
  {
    pListCtrl = nI == 0 ? pListCtrlOUTBOUND : pListCtrlINBOUND;
    for(nJ = 0; nJ < m_pPATSEL->numPatterns[nI]; nJ++)
    {
      PATTERNNAMESKey0.recordID = m_pPATSEL->PATTERNNAMESrecordID[nI][nJ];
      btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
      trim(tempString, PATTERNNAMES_NAME_LENGTH);
      LVI.mask = LVIF_TEXT | LVIF_PARAM;
      LVI.iItem = nK++;
      LVI.iSubItem = 0;
      LVI.pszText = tempString;
      LVI.lParam = MAKELONG(nI, nJ);
      LVI.iItem = pListCtrl->InsertItem(&LVI);
      if(m_pPATSEL->flags[nI][nJ] & TRIPGRAPH_FLAG_PATTERNACTIVE)
      {
        ListView_SetCheckState(pListCtrl->m_hWnd, nJ, TRUE);
      }
    }
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectPatterns::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CSelectPatterns::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CSelectPatterns::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CSelectPatterns::OnOK() 
{
  int nI, nJ;
  CListCtrl* pListCtrl;

  for(nI = 0; nI < 2; nI++)
  {
    pListCtrl = nI == 0 ? pListCtrlOUTBOUND : pListCtrlINBOUND;
    if(pListCtrl->IsWindowEnabled())
    {
      for(nJ = 0; nJ < m_pPATSEL->numPatterns[nI]; nJ++)
      {
        if(ListView_GetCheckState(pListCtrl->m_hWnd, nJ))
          m_pPATSEL->flags[nI][nJ] = TRIPGRAPH_FLAG_PATTERNACTIVE;
        else
          m_pPATSEL->flags[nI][nJ] = 0;
      }
    }
  }

	CDialog::OnOK();
}
