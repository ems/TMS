// PremiumDefinition.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "PremiumDefinition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Call the dialog
//
BOOL PremiumDefinitionDialog(PDISPLAYINFO pDI)
{
  CPremiumDefinition dlg(NULL, pDI);

  return(dlg.DoModal());

}

//
//  Premium Definition callable from C
//
extern "C"
{
  BOOL PremiumDefinition(PDISPLAYINFO pDI)
  {
    return(PremiumDefinitionDialog(pDI));
  }
}

/////////////////////////////////////////////////////////////////////////////
// CPremiumDefinition dialog


CPremiumDefinition::CPremiumDefinition(CWnd* pParent /*=NULL*/, PDISPLAYINFO pDI)
	: CDialog(CPremiumDefinition::IDD, pParent)
{
  m_pDI = pDI;

	//{{AFX_DATA_INIT(CPremiumDefinition)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPremiumDefinition::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPremiumDefinition)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPremiumDefinition, CDialog)
	//{{AFX_MSG_MAP(CPremiumDefinition)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDNEWPREMIUM, OnNewpremium)
	ON_BN_CLICKED(IDSETPREMIUM, OnSetpremium)
	ON_LBN_SELCHANGE(WORKRULES_PREMIUMLIST, OnSelchangePremiumlist)
	ON_LBN_DBLCLK(WORKRULES_PREMIUMLIST, OnDblclkPremiumlist)
	ON_BN_CLICKED(WORKRULES_ONERUNTYPE, OnOneruntype)
	ON_CBN_SELCHANGE(WORKRULES_RUNTYPES, OnSelchangeRuntypes)
	ON_BN_CLICKED(WORKRULES_ALLRUNTYPES, OnAllruntypes)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(WORKRULES_CLASSIFIEDASREPORTTIME, OnClassifiedasreporttime)
	ON_BN_CLICKED(WORKRULES_CLASSIFIEDASTRAVELTIME, OnClassifiedastraveltime)
	ON_BN_CLICKED(WORKRULES_CLASSIFIEDASTURNINTIME, OnClassifiedasturnintime)
	ON_BN_CLICKED(WORKRULES_CLASSIFIEDASPAIDBREAK, OnClassifiedaspaidbreak)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPremiumDefinition message handlers

BOOL CPremiumDefinition::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int  nI, nJ, nK;
//
//  Set up pointers to the controls
//
  pButtonALLRUNTYPES = (CButton *)GetDlgItem(WORKRULES_ALLRUNTYPES);
  pButtonONERUNTYPE = (CButton *)GetDlgItem(WORKRULES_ONERUNTYPE);
  pComboBoxRUNTYPES = (CComboBox *)GetDlgItem(WORKRULES_RUNTYPES);
  pListBoxPREMIUMLIST = (CListBox *)GetDlgItem(WORKRULES_PREMIUMLIST);
  pEditPREMIUMTIME = (CEdit *)GetDlgItem(WORKRULES_PREMIUMTIME);
  pButtonTIMECOUNTSINOT = (CButton *)GetDlgItem(WORKRULES_TIMECOUNTSINOT);
  pButtonDONTPAY = (CButton *)GetDlgItem(WORKRULES_DONTPAY);
  pButtonDONTINCORPORATE = (CButton *)GetDlgItem(WORKRULES_DONTINCORPORATE);
  pButtonIDNEWPREMIUM = (CButton *)GetDlgItem(IDNEWPREMIUM);
  pButtonIDSETPREMIUM = (CButton *)GetDlgItem(IDSETPREMIUM);

  pButtonCLASSIFIEDASREPORTTIME = (CButton *)GetDlgItem(WORKRULES_CLASSIFIEDASREPORTTIME);
  pButtonCLASSIFIEDASTRAVELTIME = (CButton *)GetDlgItem(WORKRULES_CLASSIFIEDASTRAVELTIME);
  pButtonCLASSIFIEDASTURNINTIME = (CButton *)GetDlgItem(WORKRULES_CLASSIFIEDASTURNINTIME);
  pButtonCLASSIFIEDASPAIDBREAK = (CButton *)GetDlgItem(WORKRULES_CLASSIFIEDASPAIDBREAK);

  pButtonTRAVELTIMEONLY = (CButton *)GetDlgItem(WORKRULES_TRAVELTIMEONLY);

  pButtonACTUALTRAVEL = (CButton *)GetDlgItem(WORKRULES_ACTUALTRAVEL);
  pButtonTIMEENTERED = (CButton *)GetDlgItem(WORKRULES_TIMEENTERED);
  pButtonASINCONNECTIONS = (CButton *)GetDlgItem(WORKRULES_ASINCONNECTIONS);
  pButtonASPERSCHEDULE = (CButton *)GetDlgItem(WORKRULES_ASPERSCHEDULE);
  pButtonASPERSCHEDULEDWELL = (CButton *)GetDlgItem(WORKRULES_ASPERSCHEDULEDWELL);
  pButtonUSECONNIFNOSCHED = (CButton *)GetDlgItem(WORKRULES_USECONNIFNOSCHED);

  pButtonPAIDTRAVEL = (CButton *)GetDlgItem(WORKRULES_PAIDTRAVEL);
  pEditPAYFLATMINUTES = (CEdit *)GetDlgItem(WORKRULES_PAYFLATMINUTES);
  pEditPERCENTOFACTUAL = (CEdit *)GetDlgItem(WORKRULES_PERCENTOFACTUAL);
  pEditSTAGGEREDPERCENT = (CEdit *)GetDlgItem(WORKRULES_STAGGEREDPERCENT);
  pEditSTAGGEREDMINUTES = (CEdit *)GetDlgItem(WORKRULES_STAGGEREDMINUTES);
  pButtonPAYACTUAL = (CButton *)GetDlgItem(WORKRULES_PAYACTUAL);
  pButtonPAYFLAT = (CButton *)GetDlgItem(WORKRULES_PAYFLAT);
  pButtonPAYPERCENTAGE = (CButton *)GetDlgItem(WORKRULES_PAYPERCENTAGE);
  pButtonPAYSTAGGERED = (CButton *)GetDlgItem(WORKRULES_PAYSTAGGERED);
  pStaticSTATIC1 = (CStatic *)GetDlgItem(WORKRULES_STATIC1);
  pStaticSTATIC2 = (CStatic *)GetDlgItem(WORKRULES_STATIC2);
//
//  Set up the runtypes combo box
//
  pComboBoxRUNTYPES->ResetContent();
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
      {
        nK = pComboBoxRUNTYPES->AddString(RUNTYPE[nI][nJ].localName);
        pComboBoxRUNTYPES->SetItemData(nK, MAKELPARAM(nI, nJ));
      }
    }
  }
//
//  If no runtypes are defined, disable the combobox and its associated radio button
//
  if(pComboBoxRUNTYPES->GetCount() == 0)
  {
    pButtonONERUNTYPE->EnableWindow(FALSE);
  }
  pComboBoxRUNTYPES->EnableWindow(FALSE);
//
//  Display the premium(s) for the current (All runtypes) selection
//
  pButtonALLRUNTYPES->SetCheck(TRUE);
  ShowPremiums(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPremiumDefinition::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CPremiumDefinition::OnNewpremium() 
{
  long tempLong;
  int  runtypeIndex;
  int  slotNumber;
  int  nI;
  int  nRc;

  if(pButtonALLRUNTYPES->GetCheck())
  {
    runtypeIndex = NO_RECORD;
    slotNumber = NO_RECORD;
  }
  else
  {
    nI = pComboBoxRUNTYPES->GetCurSel();
    if(nI == LB_ERR)
    {
      runtypeIndex = NO_RECORD;
      slotNumber = NO_RECORD;
    }
    else
    {
      tempLong = pComboBoxRUNTYPES->GetItemData(nI);
      runtypeIndex = LOWORD(tempLong);
      slotNumber = HIWORD(tempLong);
    }
  }

  m_PremiumRuntype = runtypeIndex;
  m_PremiumTimePeriod = slotNumber;
  m_bPremiumNew = TRUE;
  nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREMIUMS), this->m_hWnd, (DLGPROC)PREMIUMSMsgProc, (LPARAM)m_pDI);
  if(nRc)
  {
    long selectedRuntype;

    if(pButtonALLRUNTYPES->GetCheck())
    {
      selectedRuntype = NO_RECORD;
    }
    else
    {
      nI = pComboBoxRUNTYPES->GetCurSel();
      selectedRuntype = (nI == CB_ERR ? NO_RECORD : pComboBoxRUNTYPES->GetItemData(nI));
    }
    PREMIUM[m_premiumIndex].runtype = selectedRuntype;
    nI = pListBoxPREMIUMLIST->GetCount();
    ShowPremiums(nI == LB_ERR ? 0 : nI);
  }
}

void CPremiumDefinition::OnSetpremium() 
{
  long tempLong;
  int  runtypeIndex;
  int  slotNumber;
  int  nI;
  int  nRc;

  if(pButtonALLRUNTYPES->GetCheck())
  {
    runtypeIndex = NO_RECORD;
    slotNumber = NO_RECORD;
  }
  else
  {
    nI = pComboBoxRUNTYPES->GetCurSel();
    if(nI == LB_ERR)
    {
      runtypeIndex = NO_RECORD;
      slotNumber = NO_RECORD;
    }
    else
    {
      tempLong = pComboBoxRUNTYPES->GetItemData(nI);
      runtypeIndex = LOWORD(tempLong);
      slotNumber = HIWORD(tempLong);
    }
  }

  m_PremiumRuntype = runtypeIndex;
  m_PremiumTimePeriod = slotNumber;
  m_bPremiumNew = FALSE;

  nI = pListBoxPREMIUMLIST->GetCurSel();
  if(nI == LB_ERR)
  {
    return;
  }
  tempLong = pListBoxPREMIUMLIST->GetItemData(nI);
  m_premiumIndex = HIWORD(tempLong);
  nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREMIUMS), this->m_hWnd, (DLGPROC)PREMIUMSMsgProc, (LPARAM)m_pDI);
  if(nRc)
  {
    ShowPremiums(nI);
  }
}

void CPremiumDefinition::ShowPremiums(int selection)
{
  long selectedRuntype;
  int  nI, nJ;

  if(pButtonALLRUNTYPES->GetCheck())
  {
    selectedRuntype = NO_RECORD;
  }
  else
  {
    nI = pComboBoxRUNTYPES->GetCurSel();
    selectedRuntype = (nI == CB_ERR ? NO_RECORD : pComboBoxRUNTYPES->GetItemData(nI));
  }

  pListBoxPREMIUMLIST->ResetContent();
  
  for(nI = 0; nI < m_numPremiums; nI++)
  {
    if(PREMIUM[nI].runtype == selectedRuntype)
    {
      nJ = pListBoxPREMIUMLIST->AddString(PREMIUM[nI].localName);
      pListBoxPREMIUMLIST->SetItemData(nJ, MAKELPARAM(0, nI));
    }
  }

  nI = pListBoxPREMIUMLIST->GetCount();
  pButtonIDSETPREMIUM->EnableWindow(nI > 0);
  if(nI > 0)
  {
    pListBoxPREMIUMLIST->SetCurSel(selection);
    OnSelchangePremiumlist();
  }
}

void CPremiumDefinition::OnSelchangePremiumlist() 
{
  BOOL bEnable;
  long tempLong;
  int  nI;

  nI = pListBoxPREMIUMLIST->GetCurSel();

  bEnable = (nI != LB_ERR);
  pEditPREMIUMTIME->EnableWindow(bEnable);
  pButtonTIMECOUNTSINOT->EnableWindow(bEnable);

  pEditPREMIUMTIME->EnableWindow(bEnable);
  pButtonTIMECOUNTSINOT->EnableWindow(bEnable);
  pButtonIDSETPREMIUM->EnableWindow(bEnable);
//
//  Display the premium characteristics
//
//  Pay type
//
  if(bEnable)
  {
    tempLong = pListBoxPREMIUMLIST->GetItemData(nI);
    nI = HIWORD(tempLong);
    pEditPREMIUMTIME->SetWindowText(chhmm(PREMIUM[nI].time));
//
//  Time counts in OT
//
    pButtonTIMECOUNTSINOT->SetCheck(PREMIUM[nI].flags & PREMIUMFLAGS_TIMECOUNTSINOT);
//
//  Don't pay when paying intervening time
//
    pButtonDONTPAY->SetCheck(PREMIUM[nI].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING);
//
//  Don't incorporate this premium into a break
//
    pButtonDONTINCORPORATE->SetCheck(PREMIUM[nI].flags & PREMIUMFLAGS_DONTINCORPORATE);
//
//  Premium classified as...
//
    pButtonCLASSIFIEDASREPORTTIME->SetCheck(FALSE);
    pButtonCLASSIFIEDASTRAVELTIME->SetCheck(FALSE);
    pButtonCLASSIFIEDASTURNINTIME->SetCheck(FALSE);
    pButtonCLASSIFIEDASPAIDBREAK->SetCheck(FALSE);

    if(PREMIUM[nI].formattedDataType == TMSDATA_REPORTTIME)
    {
      pButtonCLASSIFIEDASREPORTTIME->SetCheck(TRUE);
    }
    else if(PREMIUM[nI].formattedDataType == TMSDATA_TRAVELTIME)
    {
      pButtonCLASSIFIEDASTRAVELTIME->SetCheck(TRUE);
    }
    else if(PREMIUM[nI].formattedDataType == TMSDATA_TURNINTIME)
    {
      pButtonCLASSIFIEDASTURNINTIME->SetCheck(TRUE);
    }
    else if(PREMIUM[nI].formattedDataType == TMSDATA_PAIDBREAKS ||
          PREMIUM[nI].formattedDataType == TMSDATA_MEAL)
    {
      pButtonCLASSIFIEDASPAIDBREAK->SetCheck(TRUE);
    }
//
//  Travel time premiums
//
    if(!pButtonCLASSIFIEDASTRAVELTIME->GetCheck())
    {
      EnableTravelSection(FALSE);
    }
    else
    {
      EnableTravelSection(TRUE);
//
//  Paid travel
//      
      CButton* pCtl;

      if(PREMIUM[nI].payTravelTime == 0)
      {
        pCtl = (CButton *)GetDlgItem(WORKRULES_ASINCONNECTIONS);
      }
      else
      {
        pCtl = (CButton *)GetDlgItem(PREMIUM[nI].payTravelTime);
      }

      pCtl->SetCheck(TRUE);
      
      if(PREMIUM[nI].payTravelTime == WORKRULES_TIMEENTERED ||
            PREMIUM[nI].payTravelTime == WORKRULES_ASINCONNECTIONS)
      {
        pButtonUSECONNIFNOSCHED->EnableWindow(FALSE);
      }
      else if(PREMIUM[nI].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
      {
        pButtonUSECONNIFNOSCHED->EnableWindow(TRUE);
      }
//
//  Feasible travel
//
      if(PREMIUM[nI].payHow == 0)
      {
        pCtl = (CButton *)GetDlgItem(WORKRULES_PAYACTUAL);
      }
      else
      {
        pCtl = (CButton *)GetDlgItem(PREMIUM[nI].payHow);
      }

      pCtl->SetCheck(TRUE);

      if(PREMIUM[nI].payHow != WORKRULES_PAYACTUAL)
      {
        gcvt(PREMIUM[nI].payHowPercent, NUMDECS, tempString);
        ltoa(PREMIUM[nI].payHowMinutes, szarString, 10);
        if(PREMIUM[nI].payHow == WORKRULES_PAYFLAT)
        {
          pEditPAYFLATMINUTES->SetWindowText(szarString);
        }
        else if(PREMIUM[nI].payHow == WORKRULES_PAYPERCENTAGE)
        {
          pEditPERCENTOFACTUAL->SetWindowText(tempString);
        }
        else if(PREMIUM[nI].payHow == WORKRULES_PAYSTAGGERED)
        {
          pEditSTAGGEREDPERCENT->SetWindowText(tempString);
          pEditSTAGGEREDMINUTES->SetWindowText(szarString);
        }
      }
    }
  }
}

void CPremiumDefinition::OnDblclkPremiumlist() 
{
  OnSetpremium();
}

void CPremiumDefinition::EnableTravelSection(BOOL bEnable)
{
//
//  Enable / disable everything on the "Travel Only" side
//
  pButtonTRAVELTIMEONLY->EnableWindow(bEnable);
  pButtonACTUALTRAVEL->EnableWindow(bEnable);
  pButtonPAIDTRAVEL->EnableWindow(bEnable);
  pStaticSTATIC1->EnableWindow(bEnable);
  pStaticSTATIC2->EnableWindow(bEnable);

  pButtonTIMEENTERED->EnableWindow(bEnable);
  pButtonASINCONNECTIONS->EnableWindow(bEnable);
  pButtonASPERSCHEDULE->EnableWindow(bEnable);
  pButtonASPERSCHEDULEDWELL->EnableWindow(bEnable);
  pButtonUSECONNIFNOSCHED->EnableWindow(bEnable);

  pEditPAYFLATMINUTES->EnableWindow(bEnable);
  pEditPERCENTOFACTUAL->EnableWindow(bEnable);
  pEditSTAGGEREDPERCENT->EnableWindow(bEnable);
  pEditSTAGGEREDMINUTES->EnableWindow(bEnable);
  pButtonPAYACTUAL->EnableWindow(bEnable);
  pButtonPAYFLAT->EnableWindow(bEnable);
  pButtonPAYPERCENTAGE->EnableWindow(bEnable);
  pButtonPAYSTAGGERED->EnableWindow(bEnable);

//
//  Clear the buttons
//
  pButtonTIMEENTERED->SetCheck(FALSE);
  pButtonASINCONNECTIONS->SetCheck(FALSE);
  pButtonASPERSCHEDULE->SetCheck(FALSE);
  pButtonASPERSCHEDULEDWELL->SetCheck(FALSE);
  pButtonUSECONNIFNOSCHED->SetCheck(FALSE);
//
//  Clear the fields
//
  pEditPAYFLATMINUTES->SetWindowText("");
  pEditPERCENTOFACTUAL->SetWindowText("");
  pEditSTAGGEREDPERCENT->SetWindowText("");
  pEditSTAGGEREDMINUTES->SetWindowText("");
  pButtonPAYACTUAL->SetCheck(FALSE);
  pButtonPAYFLAT->SetCheck(FALSE);
  pButtonPAYPERCENTAGE->SetCheck(FALSE);
  pButtonPAYSTAGGERED->SetCheck(FALSE);
}

void CPremiumDefinition::OnOneruntype() 
{
  pComboBoxRUNTYPES->EnableWindow(TRUE);
  pComboBoxRUNTYPES->SetCurSel(0);
  ShowPremiums(0);
}

void CPremiumDefinition::OnSelchangeRuntypes() 
{
  ShowPremiums(0);
}

void CPremiumDefinition::OnAllruntypes() 
{
  pComboBoxRUNTYPES->EnableWindow(FALSE);
  pComboBoxRUNTYPES->SetCurSel(-1);
  ShowPremiums(0);
}

void CPremiumDefinition::OnHelp() 
{
  ::WinHelp(this->m_hWnd, szarHelpFile, HELP_CONTEXT, Premium_Definition_Step_1);
}

void CPremiumDefinition::OnOK() 
{
	CDialog::OnOK();
}

void CPremiumDefinition::OnUpdate() 
{
  long tempLong;
  int  nI;
  int  m_premiumIndex;

  nI = pListBoxPREMIUMLIST->GetCurSel();
  if(nI == LB_ERR)
  {
    return;
  }

  tempLong = pListBoxPREMIUMLIST->GetItemData(nI);
  m_premiumIndex = HIWORD(tempLong);
//
//  Premiumtype and amount
//
  pEditPREMIUMTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
  PREMIUM[m_premiumIndex].time = thhmm(tempString);
//
//  Minutes count...
//
  if(pButtonTIMECOUNTSINOT->GetCheck())
  {
    PREMIUM[m_premiumIndex].flags |= PREMIUMFLAGS_TIMECOUNTSINOT;
  }
  else
  {
    if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_TIMECOUNTSINOT)
    {
      PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_TIMECOUNTSINOT;
    }
  }
//
//  Don't pay
//
  if(pButtonDONTPAY->GetCheck())
  {
    PREMIUM[m_premiumIndex].flags |= PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING;
  }
  else
  {
    if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING)
    {
      PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_DONTPAYIFPAYINGINTERVENING;
    }
  }
//
//  Don't incorporate
//
  if(pButtonDONTINCORPORATE->GetCheck())
  {
    PREMIUM[m_premiumIndex].flags |= PREMIUMFLAGS_DONTINCORPORATE;
  }
  else
  {
    if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_DONTINCORPORATE)
    {
      PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_DONTINCORPORATE;
    }
  }
//
//  Premium reported as
//
  if(pButtonCLASSIFIEDASREPORTTIME->GetCheck())
  {
    PREMIUM[m_premiumIndex].formattedDataType = TMSDATA_REPORTTIME;
  }
  else if(pButtonCLASSIFIEDASTRAVELTIME->GetCheck())
  {
    PREMIUM[m_premiumIndex].formattedDataType = TMSDATA_TRAVELTIME;
  }
  else if(pButtonCLASSIFIEDASTURNINTIME->GetCheck())
  {
    PREMIUM[m_premiumIndex].formattedDataType = TMSDATA_TURNINTIME;
  }
  else if(pButtonCLASSIFIEDASPAIDBREAK->GetCheck())
  {
    PREMIUM[m_premiumIndex].formattedDataType = TMSDATA_PAIDBREAKS;
  }
//
//  Travel Time Premium?
//
//  Not
//
  if(!pButtonCLASSIFIEDASTRAVELTIME->GetCheck())
  {
    if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_TRAVELTIMEPREMIUM)
    {
      PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_TRAVELTIMEPREMIUM;
    }
    if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
    {
      PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
    }
    PREMIUM[m_premiumIndex].payTravelTime = NO_RECORD;
    PREMIUM[m_premiumIndex].payHow = NO_RECORD;
    PREMIUM[m_premiumIndex].payHowMinutes = 0;
    PREMIUM[m_premiumIndex].payHowPercent = (float)0.0;
  }
//
//  Yes a travel time premium
//
  else
  {
    int feasibleRG[4] = {WORKRULES_TIMEENTERED, WORKRULES_ASINCONNECTIONS,
                         WORKRULES_ASPERSCHEDULE, WORKRULES_ASPERSCHEDULEDWELL};
    int paidHowRG[4]  = {WORKRULES_PAYACTUAL, WORKRULES_PAYFLAT, 
                         WORKRULES_PAYPERCENTAGE, WORKRULES_PAYSTAGGERED};
    CButton* pCtl;

    PREMIUM[m_premiumIndex].flags |= PREMIUMFLAGS_TRAVELTIMEPREMIUM;
    for(nI = 0; nI < 4; nI++)
    {
      pCtl = (CButton *)GetDlgItem(feasibleRG[nI]);
      if(pCtl == NULL)
      {
        continue;
      }
      if(pCtl->GetCheck())
      {
        PREMIUM[m_premiumIndex].payTravelTime = feasibleRG[nI];
        break;
      }
    }
    if(PREMIUM[m_premiumIndex].payTravelTime == feasibleRG[0] ||
          PREMIUM[m_premiumIndex].payTravelTime == feasibleRG[1])
    {
      if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
      {
        PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
      }
    }
    else
    {
      if(pButtonUSECONNIFNOSCHED->GetCheck())
      {
        PREMIUM[m_premiumIndex].flags |= PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
      }
      else if(PREMIUM[m_premiumIndex].flags & PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC)
      {
        PREMIUM[m_premiumIndex].flags &= ~PREMIUMFLAGS_USECONNECTIONIFNODYNAMIC;
      }
    }
    for(nI = 0; nI < 4; nI++)
    {
      pCtl = (CButton *)GetDlgItem(paidHowRG[nI]);
      if(pCtl == NULL)
      {
        continue;
      }
      if(pCtl->GetCheck())
      {
        PREMIUM[m_premiumIndex].payHow = paidHowRG[nI];
        strcpy(tempString, "0");
        strcpy(szarString, "0");
        if(paidHowRG[nI] == WORKRULES_PAYFLAT)
        {
          pEditPAYFLATMINUTES->GetWindowText(szarString, SZARSTRING_LENGTH);
        }
        else if(paidHowRG[nI] == WORKRULES_PAYPERCENTAGE)
        {
          pEditPERCENTOFACTUAL->GetWindowText(tempString, TEMPSTRING_LENGTH);
        }
        else if(PREMIUM[m_premiumIndex].payHow == WORKRULES_PAYSTAGGERED)
        {
          pEditSTAGGEREDPERCENT->GetWindowText(tempString, TEMPSTRING_LENGTH);
          pEditSTAGGEREDMINUTES->GetWindowText(szarString, SZARSTRING_LENGTH);
        }
        PREMIUM[m_premiumIndex].payHowMinutes = atol(szarString);
        PREMIUM[m_premiumIndex].payHowPercent = (float)atof(tempString);
        break;
      }
    }
  }
//
//  Flag the changes to the INI file
//
  m_bWorkrulesChanged = TRUE;
}

void CPremiumDefinition::OnClassifiedasreporttime() 
{
  EnableTravelSection(FALSE);
}

void CPremiumDefinition::OnClassifiedastraveltime() 
{
  EnableTravelSection(TRUE);
}

void CPremiumDefinition::OnClassifiedasturnintime() 
{
  EnableTravelSection(FALSE);
}

void CPremiumDefinition::OnClassifiedaspaidbreak() 
{
  EnableTravelSection(FALSE);
}
