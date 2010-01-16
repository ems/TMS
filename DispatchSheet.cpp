// DispatchSheet.cpp : implementation file
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

//
//  Call the dialog
//
BOOL DispatchSheetDialog(DISPATCHSHEETPassedDataDef *pPassedData)
{
  CDispatchSheet dlg(NULL, pPassedData);

  return(dlg.DoModal());

}

//
//  Dispatch Sheet callable from C
//
extern "C"
{
  BOOL DispatchSheet(DISPATCHSHEETPassedDataDef *pPassedData)
  {
    return(DispatchSheetDialog(pPassedData));
  }
}

//
//  Adjust the internal dispatch sheet structure
//  with updates from the Daily Operations Module
//
extern "C"
{
  void AdjustDispatchSheetRosterData(DISPATCHSHEETRosterDataDef *RData,
        long todaysDate, long date, long numRuns, BOOL bDoOperator, BOOL bDoBus)
  {
    long BUSESrecordID;
    int  nI;
    int  rcode2;
//
//  Cycle through the rostered and unrostered runs
//  to pick up all the assignments for today
//
    if(bDoOperator)
    {
      DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
      DAILYOPSKey1.pertainsToDate = date;
      DAILYOPSKey1.pertainsToTime = NO_TIME;
      DAILYOPSKey1.recordFlags = 0;
      rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
            (DAILYOPS.pertainsToDate == date))
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN)
          {
            for(nI = 0; nI < numRuns; nI++)
            {
              if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == RData[nI].RUNSrecordID)
              {
                RData[nI].replacementDRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
              }
            }
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
//
//  See who's away
//
      DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
      DAILYOPSKey1.pertainsToDate = date - 10000;
      DAILYOPSKey1.pertainsToTime = NO_TIME;
      DAILYOPSKey1.recordFlags = 0;
      rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE) &&
             DAILYOPS.pertainsToDate <= date)
      {
        if(date >= DAILYOPS.pertainsToDate &&
              date <= DAILYOPS.DOPS.Absence.untilDate &&
              (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER))
        {
          if(!ANegatedRecord(DAILYOPS.recordID, 1))
          {
            for(nI = 0; nI < numRuns; nI++) 
            {
              if(RData[nI].DRIVERSrecordID == NO_RECORD && 
                    RData[nI].replacementDRIVERSrecordID == NO_RECORD)
              {
                continue;
              }
              if(RData[nI].DRIVERSrecordID == DAILYOPS.DRIVERSrecordID)
              {
                RData[nI].DRIVERSrecordID = NO_RECORD;
              }
              if(RData[nI].replacementDRIVERSrecordID == DAILYOPS.DRIVERSrecordID)
              {
                RData[nI].replacementDRIVERSrecordID = NO_RECORD;
              }
            }
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
    }
//
//  Vehicle changes: Cycle through the rostered and unrostered runs
//
    if(bDoBus)
    {
      for(nI = 0; nI < numRuns; nI++)
      {
//
//  Get the block
//
        DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_BUS;
        DAILYOPSKey1.pertainsToDate = date;
        DAILYOPSKey1.pertainsToTime = 0;
        DAILYOPSKey1.recordFlags = 0;
        BUSESrecordID = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        while(rcode2 == 0 &&
              (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS) &&
              (DAILYOPS.pertainsToDate == date))
        {
          if(!ANegatedRecord(DAILYOPS.recordID, 1))
          {
            if(DAILYOPS.DOPS.Bus.TRIPSrecordID == RData[nI].TRIPSrecordID)
            {
              if((DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSCLEARALL) ||
                    (DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKDROP))
              {
                BUSESrecordID = NO_RECORD;
              }
              else
              {
                BUSESrecordID = DAILYOPS.DOPS.Bus.BUSESrecordID;
              }
              break;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        }
        RData[nI].BUSESrecordID = BUSESrecordID;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDispatchSheet dialog


CDispatchSheet::CDispatchSheet(CWnd* pParent, DISPATCHSHEETPassedDataDef *pPassedData)
	: CDialog(CDispatchSheet::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CDispatchSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDispatchSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDispatchSheet)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDispatchSheet, CDialog)
	//{{AFX_MSG_MAP(CDispatchSheet)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDispatchSheet message handlers

BOOL CDispatchSheet::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pComboBoxDIVISION = (CComboBox *)GetDlgItem(DISPATCHSHEET_DIVISION);
  pStaticGARAGES_TITLE = (CStatic *)GetDlgItem(DISPATCHSHEET_GARAGES_TITLE);
  pListBoxGARAGES = (CListBox *)GetDlgItem(DISPATCHSHEET_GARAGES);
  pStaticRELIEFPOINTS_TITLE = (CStatic *)GetDlgItem(DISPATCHSHEET_RELIEFPOINTS_TITLE);
  pListBoxRELIEFPOINTS = (CListBox *)GetDlgItem(DISPATCHSHEET_RELIEFPOINTS);
  pDTPickerDATE = (CDTPicker *)GetDlgItem(DISPATCHSHEET_DATE);
  pButtonOPTIONS = (CButton *)GetDlgItem(DISPATCHSHEET_OPTIONS);
  pButtonINCLUDESTARTOFPIECETRAVEL = (CButton *)GetDlgItem(DISPATCHSHEET_INCLUDESTARTOFPIECETRAVEL);
  pButtonSORTBY = (CButton *)GetDlgItem(DISPATCHSHEET_SORTBY);
  pButtonREPORTTIME = (CButton *)GetDlgItem(DISPATCHSHEET_REPORTTIME);
  pButtonENDTIME = (CButton *)GetDlgItem(DISPATCHSHEET_ENDTIME);
//
//  Set up the division list
//
  if((SetUpDivisionList(this->m_hWnd, DISPATCHSHEET_DIVISION, m_DivisionRecordID)) == 0)
  {
    OnCancel();
    return TRUE;
  }
//
//  Garages and relief points
//
  if(m_pPassedData->flags & DISPATCHSHEET_FLAG_INCLUDEGARAGEANDRELIEFS)
  {
    int nI;
    int rcode2;
//
//  Set up the lists of garages and relief points
//
    pStaticGARAGES_TITLE->EnableWindow(TRUE);
    pListBoxGARAGES->EnableWindow(TRUE);
    pStaticRELIEFPOINTS_TITLE->EnableWindow(TRUE);
    pListBoxRELIEFPOINTS->EnableWindow(TRUE);
    rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
    while(rcode2 == 0)
    {
      if(NODES.flags & NODES_FLAG_GARAGE)
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        nI = pListBoxGARAGES->AddString(tempString);
        pListBoxGARAGES->SetItemData(nI, NODES.recordID);
      }
      else
      {
        strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
        trim(tempString, NODES_RELIEFLABELS_LENGTH);
        if(strcmp(tempString, "") != 0)
        {
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          nI = pListBoxRELIEFPOINTS->AddString(tempString);
          pListBoxRELIEFPOINTS->SetItemData(nI, NODES.recordID);
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
    }
    pListBoxGARAGES->SetCurSel(-1);
    pListBoxRELIEFPOINTS->SetCurSel(-1);
  }
//
//  Set today's date on the Date and Time picker
//
  COleVariant v;
  long day;
  long month;
  long year;

  CTime time = CTime::GetCurrentTime();
  day = time.GetDay();
  month = time.GetMonth();
  year = time.GetYear();

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);
  
  m_pPassedData->todaysDate = year * 10000 + month * 100 + day;
//
//  Disable the options if requested
//
  if(m_pPassedData->flags & DISPATCHSHEET_FLAG_DISABLEOPTIONS)
  {
    pButtonOPTIONS->EnableWindow(FALSE);
    pButtonINCLUDESTARTOFPIECETRAVEL->EnableWindow(FALSE);
    pButtonINCLUDESTARTOFPIECETRAVEL->SetCheck(FALSE);
  }
//
//  Enable "Sort by" if requested
//
  if(m_pPassedData->flags & DISPATCHSHEET_FLAG_DISABLEOPTIONS)
  {
    pButtonSORTBY->EnableWindow(TRUE);
    pButtonREPORTTIME->EnableWindow(TRUE);
    pButtonENDTIME->EnableWindow(TRUE);
    pButtonREPORTTIME->SetCheck(TRUE);
  }
//
//  Done
//
  ReleaseCapture();
  	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDispatchSheet::OnClose() 
{
  OnCancel();
}

void CDispatchSheet::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDispatchSheet::OnCancel() 
{
  CDialog::OnCancel();
}

void CDispatchSheet::OnOK() 
{
//
//  Extract the year and day of week from the date control
//
  COleVariant v;
  long year, month, day;
  long days[7] = {6, 0, 1, 2, 3, 4, 5};
  int  nI, nJ;

  v = pDTPickerDATE->GetDay();
  day = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetDayOfWeek();
  m_pPassedData->dayOfWeek = days[v.lVal - 1];
  m_pPassedData->date = year * 10000 + month * 100 + day;
//
//  Set up the date as a text string
//
  CTime   t(year, month, day, 0, 0, 0);
  CString x = t.Format(_T("%A, %B %d, %Y"));
  strcpy(m_pPassedData->szDate, x);
//
//  Get the rest of the selections
//
//  Division
//
  nI = pComboBoxDIVISION->GetCurSel();

  m_pPassedData->DIVISIONSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxDIVISION->GetItemData(nI));
//
//  Garage and relief label - if requested
//
  if(m_pPassedData->flags & DISPATCHSHEET_FLAG_INCLUDEGARAGEANDRELIEFS)
  {
//
//  Garage
// 
    if(pListBoxGARAGES->GetSelCount() == 0)
    {
      pListBoxGARAGES->SetSel(-1, TRUE);
    }
    m_pPassedData->numGarages = pListBoxGARAGES->GetSelCount();
    if(m_pPassedData->numGarages > DISPATCHSHEET_MAXGARAGES)
    {
      m_pPassedData->numGarages = DISPATCHSHEET_MAXGARAGES;
    }
    for(nJ = 0, nI = 0; nI < m_pPassedData->numGarages; nI++)
    {
      if(pListBoxGARAGES->GetSel(nI))
      {
        m_pPassedData->garageNODESrecordIDs[nJ] = pListBoxGARAGES->GetItemData(nI);
        nJ++;
      }
    }
//
//  Relief point
//
    if(pListBoxRELIEFPOINTS->GetSelCount() == 0)
    {
      pListBoxRELIEFPOINTS->SetSel(-1, TRUE);
    }
    m_pPassedData->numReliefPoints = pListBoxRELIEFPOINTS->GetSelCount();
    if(m_pPassedData->numReliefPoints > DISPATCHSHEET_MAXRELIEFPOINTS)
    {
      m_pPassedData->numReliefPoints = DISPATCHSHEET_MAXRELIEFPOINTS;
    }
    for(nJ = 0, nI = 0; nI < m_pPassedData->numReliefPoints; nI++)
    {
      if(pListBoxRELIEFPOINTS->GetSel(nI))
      {
        m_pPassedData->reliefPointNODESrecordIDs[nJ] = pListBoxRELIEFPOINTS->GetItemData(nI);
        nJ++;
      }
    }
  }
//
//  Start of piece travel
//
  m_pPassedData->flags = 0;
  if(pButtonINCLUDESTARTOFPIECETRAVEL->GetCheck())
  {
    m_pPassedData->flags |= DISPATCHSHEET_FLAG_INCLUDESTARTOFPIECETRAVEL;
  }
//
//  Sort by
//
  if(pButtonSORTBY->IsWindowEnabled())
  {
    if(pButtonREPORTTIME->GetCheck())
    {
      m_pPassedData->flags |= DISPATCHSHEET_FLAG_REPORTTIME;
    }
  }
//
//  All done
//	
  CDialog::OnOK();
}
