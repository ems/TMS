//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddTimecheck.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "AddDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddTimecheck dialog


CAddTimecheck::CAddTimecheck(CWnd* pParent, long *pUpdateRecordID, PDISPLAYINFO pDI)
	: CDialog(CAddTimecheck::IDD, pParent)
{
  if(pDI == NULL)
    CDialog::OnCancel();
  else
  {
    m_pDI = pDI;
    m_pUpdateRecordID = pUpdateRecordID;
    m_bInit = TRUE;
    m_bFinishedInProgress = FALSE;
  }

	//{{AFX_DATA_INIT(CAddTimecheck)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddTimecheck::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddTimecheck)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddTimecheck, CDialog)
	//{{AFX_MSG_MAP(CAddTimecheck)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDADD, OnAdd)
	ON_BN_CLICKED(IDFINISHED, OnFinished)
	ON_WM_CLOSE()
	ON_EN_KILLFOCUS(ADDTIMECHECK_TRIPNUMBER, OnKillfocusTripnumber)
	ON_BN_CLICKED(ADDTIMECHECK_BYROUTE, OnByroute)
	ON_BN_CLICKED(ADDTIMECHECK_BYTRIPNUMBER, OnBytripnumber)
	ON_BN_CLICKED(ADDTIMECHECK_OUTBOUND, OnOutbound)
	ON_BN_CLICKED(ADDTIMECHECK_INBOUND, OnInbound)
	ON_CBN_EDITCHANGE(ADDTIMECHECK_SCHEDULED, OnEditchangeScheduled)
	ON_CBN_SELCHANGE(ADDTIMECHECK_SPECIFICROUTE, OnSelchangeSpecificroute)
	ON_CBN_SELCHANGE(ADDTIMECHECK_SPECIFICSERVICE, OnSelchangeSpecificservice)
	ON_CBN_SELCHANGE(ADDTIMECHECK_SPECIFICNODE, OnSelchangeSpecificnode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddTimecheck message handlers

BOOL CAddTimecheck::OnInitDialog() 
{
  COleVariant v;
  CString s;
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  long day;
  long month;
  long year;
  int  nI;
  int  numRoutes;
  int  numServices;
  int  numBustypes;
  int  numDrivers;
  int  rcode2;

	CDialog::OnInitDialog();
	
  pButtonBYROUTE = (CButton *)GetDlgItem(ADDTIMECHECK_BYROUTE);
  pButtonBYTRIPNUMBER = (CButton *)GetDlgItem(ADDTIMECHECK_BYTRIPNUMBER);
  pComboBoxSPECIFICROUTE = (CComboBox *)GetDlgItem(ADDTIMECHECK_SPECIFICROUTE);
  pButtonOUTBOUND = (CButton *)GetDlgItem(ADDTIMECHECK_OUTBOUND);
  pButtonINBOUND = (CButton *)GetDlgItem(ADDTIMECHECK_INBOUND);
  pStaticSPECIFICSERVICETITLE = (CStatic *)GetDlgItem(ADDTIMECHECK_SPECIFICSERVICETITLE);
  pComboBoxSPECIFICSERVICE = (CComboBox *)GetDlgItem(ADDTIMECHECK_SPECIFICSERVICE);
  pStaticSPECIFICNODETITLE = (CStatic *)GetDlgItem(ADDTIMECHECK_SPECIFICNODETITLE);
  pComboBoxSPECIFICNODE = (CComboBox *)GetDlgItem(ADDTIMECHECK_SPECIFICNODE);
  pStaticSCHEDULEDTITLE = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULEDTITLE);
  pComboBoxSCHEDULED = (CComboBox *)GetDlgItem(ADDTIMECHECK_SCHEDULED);
  pStaticACTUALTITLE = (CStatic *)GetDlgItem(ADDTIMECHECK_ACTUALTITLE);
  pEditACTUAL00 = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL00);
  pEditTRIPNUMBER = (CEdit *)GetDlgItem(ADDTIMECHECK_TRIPNUMBER);
  pStaticROUTE = (CStatic *)GetDlgItem(ADDTIMECHECK_ROUTE);
  pStaticDIRECTION = (CStatic *)GetDlgItem(ADDTIMECHECK_DIRECTION);
  pStaticSERVICE = (CStatic *)GetDlgItem(ADDTIMECHECK_SERVICE);
  pStaticNODE[0] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE01);
  pStaticNODE[1] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE02);
  pStaticNODE[2] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE03);
  pStaticNODE[3] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE04);
  pStaticNODE[4] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE05);
  pStaticNODE[5] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE06);
  pStaticNODE[6] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE07);
  pStaticNODE[7] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE08);
  pStaticNODE[8] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE09);
  pStaticNODE[9] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE10);
  pStaticNODE[10] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE11);
  pStaticNODE[11] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE12);
  pStaticNODE[12] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE13);
  pStaticNODE[13] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE14);
  pStaticNODE[14] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE15);
  pStaticNODE[15] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE16);
  pStaticNODE[16] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE17);
  pStaticNODE[17] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE18);
  pStaticNODE[18] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE19);
  pStaticNODE[19] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE20);
  pStaticNODE[20] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE21);
  pStaticNODE[21] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE22);
  pStaticNODE[22] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE23);
  pStaticNODE[23] = (CStatic *)GetDlgItem(ADDTIMECHECK_NODE24);
  pStaticSCHEDULED[0] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED01);
  pStaticSCHEDULED[1] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED02);
  pStaticSCHEDULED[2] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED03);
  pStaticSCHEDULED[3] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED04);
  pStaticSCHEDULED[4] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED05);
  pStaticSCHEDULED[5] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED06);
  pStaticSCHEDULED[6] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED07);
  pStaticSCHEDULED[7] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED08);
  pStaticSCHEDULED[8] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED09);
  pStaticSCHEDULED[9] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED10);
  pStaticSCHEDULED[10] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED11);
  pStaticSCHEDULED[11] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED12);
  pStaticSCHEDULED[12] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED13);
  pStaticSCHEDULED[13] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED14);
  pStaticSCHEDULED[14] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED15);
  pStaticSCHEDULED[15] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED16);
  pStaticSCHEDULED[16] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED17);
  pStaticSCHEDULED[17] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED18);
  pStaticSCHEDULED[18] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED19);
  pStaticSCHEDULED[19] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED20);
  pStaticSCHEDULED[20] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED21);
  pStaticSCHEDULED[21] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED22);
  pStaticSCHEDULED[22] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED23);
  pStaticSCHEDULED[23] = (CStatic *)GetDlgItem(ADDTIMECHECK_SCHEDULED24);
  pEditACTUAL[0] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL01);
  pEditACTUAL[1] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL02);
  pEditACTUAL[2] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL03);
  pEditACTUAL[3] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL04);
  pEditACTUAL[4] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL05);
  pEditACTUAL[5] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL06);
  pEditACTUAL[6] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL07);
  pEditACTUAL[7] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL08);
  pEditACTUAL[8] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL09);
  pEditACTUAL[9] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL10);
  pEditACTUAL[10] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL11);
  pEditACTUAL[11] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL12);
  pEditACTUAL[12] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL13);
  pEditACTUAL[13] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL14);
  pEditACTUAL[14] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL15);
  pEditACTUAL[15] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL16);
  pEditACTUAL[16] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL17);
  pEditACTUAL[17] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL18);
  pEditACTUAL[18] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL19);
  pEditACTUAL[19] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL20);
  pEditACTUAL[20] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL21);
  pEditACTUAL[21] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL22);
  pEditACTUAL[22] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL23);
  pEditACTUAL[23] = (CEdit *)GetDlgItem(ADDTIMECHECK_ACTUAL24);

  pDTPickerDATE = (CDTPicker *)GetDlgItem(ADDTIMECHECK_DATE);;
  pComboBoxBUSTYPE = (CComboBox *)GetDlgItem(ADDTIMECHECK_BUSTYPE);
  pComboBoxDRIVER = (CComboBox *)GetDlgItem(ADDTIMECHECK_DRIVER);
  pComboBoxPASSENGERLOAD = (CComboBox *)GetDlgItem(ADDTIMECHECK_PASSENGERLOAD);
  pComboBoxWEATHER = (CComboBox *)GetDlgItem(ADDTIMECHECK_WEATHER);
  pComboBoxTRAFFIC = (CComboBox *)GetDlgItem(ADDTIMECHECK_TRAFFIC);
  pComboBoxOTHER = (CComboBox *)GetDlgItem(ADDTIMECHECK_OTHER);
  pComboBoxSUPERVISOR = (CComboBox *)GetDlgItem(ADDTIMECHECK_SUPERVISOR);
  pButtonADD = (CButton *)GetDlgItem(IDADD);

//
//  Routes combo box
//
  numRoutes = SetUpRouteList(m_hWnd, ADDTIMECHECK_SPECIFICROUTE, m_pDI->fileInfo.routeRecordID);  
  if(numRoutes == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    CDialog::OnCancel();
    return TRUE;
  }
//
//  Services combo box
//
  numServices = SetUpServiceList(m_hWnd, ADDTIMECHECK_SPECIFICSERVICE, m_pDI->fileInfo.serviceRecordID);
  if(numServices == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    CDialog::OnCancel();
    return TRUE;
  }
//
//  Bustypes combo box
//
  numBustypes = SetUpBustypeList(m_hWnd, ADDTIMECHECK_BUSTYPE);
  if(numBustypes == 0)
  {
    pComboBoxBUSTYPE->EnableWindow(FALSE);
  }
//
//  Set the first bustype to "Not known or N/A"
//
  s.LoadString(TEXT_238);
  nI = pComboBoxBUSTYPE->InsertString(0, s);
  pComboBoxBUSTYPE->SetItemData(nI, NO_RECORD);
  pComboBoxBUSTYPE->SetCurSel(0);
//
//  Load the drivers
//
//  Set the first driver to "Not known or N/A"
//
  s.LoadString(TEXT_238);
  numDrivers = pComboBoxDRIVER->AddString(s);
  pComboBoxDRIVER->SetItemData(numDrivers, NO_RECORD);
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(szDriver, tempString);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      numDrivers = pComboBoxDRIVER->AddString(szDriver);
      if(numDrivers != CB_ERR)
      {
        pComboBoxDRIVER->SetItemData(numDrivers, DRIVERS.recordID);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  pComboBoxDRIVER->SetCurSel(0);
//
//  Set up the text combo boxes.  All files were opened in tms.cpp
//
  SetUpTextComboBox(m_OtherComments, m_numOtherComments, pComboBoxOTHER);
  SetUpTextComboBox(m_PassengerLoads, m_numPassengerLoads, pComboBoxPASSENGERLOAD);
  SetUpTextComboBox(m_RoadSupervisors, m_numRoadSupervisors, pComboBoxSUPERVISOR);
  SetUpTextComboBox(m_TrafficConditions, m_numTrafficConditions, pComboBoxTRAFFIC);
  SetUpTextComboBox(m_WeatherConditions, m_numWeatherConditions, pComboBoxWEATHER);
//
//  Are we updating?
//
//  No - a new record
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonBYROUTE->SetCheck(TRUE);
    OnByroute();
//
//  No - Set today's date on the Date and Time picker
//
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
//
//  Set the node position
//
    m_NodePositionInTrip = NO_RECORD;
    m_TRIPSrecordID = NO_RECORD;
//
//  Set the directions
//
    OnSelchangeSpecificroute();
  }

//
//  Yes - get the record
//
  else
  {
    OnBytripnumber();
    pButtonBYROUTE->EnableWindow(FALSE);
    pButtonBYTRIPNUMBER->SetCheck(TRUE);
    TIMECHECKSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);
    m_NodePositionInTrip = TIMECHECKS.nodePositionInTrip;
    m_TRIPSrecordID = TIMECHECKS.TRIPSrecordID;
//
//  Change the Add button to Update
//
    s.LoadString(TEXT_029);
    pButtonADD->SetWindowText(s);
//
//  Get the trip
//
    TRIPSKey0.recordID = TIMECHECKS.TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Trip number - On Update, disable the trip number control
//
    if(TRIPS.tripNumber > 0)
    {
      m_TripNumber = TRIPS.tripNumber;
      sprintf(tempString, "%ld", TRIPS.tripNumber);
      pEditTRIPNUMBER->SetWindowText(tempString);
    }
    DisplayRSD(FALSE, &TRIPS);
    pEditTRIPNUMBER->EnableWindow(FALSE);
//
//  Set the date
//
    GetYMD(TIMECHECKS.checkDate, &year, &month, &day);
    v = year;
    pDTPickerDATE->SetYear(v);
    v = month;
    pDTPickerDATE->SetMonth(v);
    v = day;
    pDTPickerDATE->SetDay(v);
//
//  Show the node
//
    NODESKey0.recordID = TIMECHECKS.NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    pStaticNODE[0]->SetWindowText(tempString);
//
//  Show the scheduled time
//
    long timeAtNode = GetTimeAtNode(TIMECHECKS.TRIPSrecordID, TIMECHECKS.NODESrecordID,
          TIMECHECKS.nodePositionInTrip, (TRIPSDef *)NULL);
    pStaticSCHEDULED[0]->SetWindowText(Tchar(timeAtNode));
//
//  Put in the actual time
//
    pEditACTUAL[0]->SetWindowText(Tchar(TIMECHECKS.actualTime));
//
//  Disable the other input boxes
//
    for(nI = 1; nI < MAXTIMECHECKNODES; nI++)
    {
      pEditACTUAL[nI]->EnableWindow(FALSE);
    }
    m_numNodes = 1;
//
//  The actual bustype
//
    HighlightComboBoxValue(pComboBoxBUSTYPE, TIMECHECKS.actualBUSTYPESrecordID);
//
//  The actual driver
//
    HighlightComboBoxValue(pComboBoxDRIVER, TIMECHECKS.actualDRIVERSrecordID);
//
//  Passenger load
//
    HighlightComboBoxValue(pComboBoxPASSENGERLOAD, TIMECHECKS.passengerLoadIndex);
//
//  Weather conditions
//
    HighlightComboBoxValue(pComboBoxWEATHER, (long)LOWORD(TIMECHECKS.weatherIndex));
//
//  Traffic conditions
//
    HighlightComboBoxValue(pComboBoxTRAFFIC, (long)HIWORD(TIMECHECKS.weatherIndex));
//
//  Other comments
//
    HighlightComboBoxValue(pComboBoxOTHER, TIMECHECKS.otherCommentsIndex);
//
//  Road supervisor
//
    HighlightComboBoxValue(pComboBoxSUPERVISOR, TIMECHECKS.supervisorIndex);
  }
//
//  All done
//
  m_bInit = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAddTimecheck::OnClose() 
{
	CDialog::OnClose();
}


void CAddTimecheck::OnFinished() 
{
  m_bFinishedInProgress = TRUE;

  CDialog::OnOK();
}

void CAddTimecheck::OnHelp() 
{
}


//
//  OnAdd - Add/Update a record
//
void CAddTimecheck::OnAdd() 
{
  CString s;
  VARIANT v;
  long    year;
  long    month;
  long    day;
  int     nI, nJ;
  int     opCode;
  int     rcode2;

//
//  Additions in "trip number" mode and all updates are taken care of below
//
//  This section is just for "By Route"
//
  if(pButtonBYROUTE->GetCheck())
  {
//
//  The only check that has to be made is to
//  verify that the "actual" field has something in it.
//
    pEditACTUAL00->GetWindowText(s);
    if(s == "")
    {
      TMSError(m_hWnd, MB_ICONSTOP, ERROR_298, (HANDLE)NULL);
      pEditACTUAL00->SetFocus();
      return;
    }
//
//  Establish a recordID
//
    rcode2 = btrieve(B_GETLAST, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);
    TIMECHECKS.recordID = AssignRecID(rcode2, TIMECHECKS.recordID);
    opCode = B_INSERT;
//
//  Set up the balance of TIMECHECKS
//
//  Comment code is there but unused
//
    TIMECHECKS.COMMENTSrecordID = NO_RECORD;
//
//  Check date
//
    v = pDTPickerDATE->GetYear();
    year = v.lVal;
    v = pDTPickerDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerDATE->GetDay();
    day = v.lVal;
    TIMECHECKS.checkDate = year * 10000 + month * 100 + day;
//
//  Service
//
    TIMECHECKS.SERVICESrecordID = GetComboBoxValue(pComboBoxSPECIFICSERVICE);
//
//  Route / direction
//
    TIMECHECKS.ROUTESrecordID = GetComboBoxValue(pComboBoxSPECIFICROUTE);
    TIMECHECKS.directionIndex = pButtonOUTBOUND->GetCheck() ? 0 : 1;
//
//  Trips recordID (set on the scheduled listbox)
//
    TIMECHECKS.TRIPSrecordID = GetComboBoxValue(pComboBoxSCHEDULED);
//
//  Node
//
    TIMECHECKS.NODESrecordID = GetComboBoxValue(pComboBoxSPECIFICNODE);
//
//  Actual time (from above)
//
    strcpy(tempString, s);
    TIMECHECKS.actualTime = cTime(tempString);
//
//  Actual bustype
//
    TIMECHECKS.actualBUSTYPESrecordID = GetComboBoxValue(pComboBoxBUSTYPE);
//
//  Actual driver
//
    TIMECHECKS.actualDRIVERSrecordID = GetComboBoxValue(pComboBoxDRIVER);
//
//  Supervisor index
//
    TIMECHECKS.supervisorIndex = GetComboBoxValue(pComboBoxSUPERVISOR);
//
//  Passenger load index
//
    TIMECHECKS.passengerLoadIndex = GetComboBoxValue(pComboBoxPASSENGERLOAD);
//
//  Weather index and Traffic index
//
//  The LOWORD of the long gets the weather, the HIWORD gets the traffic.
//
//  Hindsight is always 20/20
//
    nI = (int)GetComboBoxValue(pComboBoxWEATHER);
    nJ = (int)GetComboBoxValue(pComboBoxTRAFFIC);
    TIMECHECKS.weatherIndex = MAKELONG(nI, nJ);
//
//  Other comments index
//
    TIMECHECKS.otherCommentsIndex = GetComboBoxValue(pComboBoxOTHER);
//
//  Reserved
//
    memset(TIMECHECKS.reserved, 0x00, TIMECHECKS_RESERVED_LENGTH);
//
//  Flags
//
    TIMECHECKS.flags = 0;
//
//  Insert and stay or update and leave
//
    btrieve(opCode, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);

    if(*m_pUpdateRecordID != NO_RECORD)
    {
      CDialog::OnOK();
    }
    else
    {
      pComboBoxPASSENGERLOAD->SetCurSel(0);
      pComboBoxOTHER->SetCurSel(0);
      pEditACTUAL00->SetWindowText("");
      pEditACTUAL00->SetFocus();
    }
  }
//
//  This section is for updates and "By Trip Number"
//
  else
  {
//
//  The only check that has to be made is to
//  verify that the "actual" fields have something in them.
//
    for(nI = 0; nI < m_numNodes; nI++)
    {
      pEditACTUAL[nI]->GetWindowText(s);
      if(s == "")
      {
        TMSError(m_hWnd, MB_ICONSTOP, ERROR_298, (HANDLE)NULL);
        pEditACTUAL[nI]->SetFocus();
        return;
      }
    }
//
//  Set up the static portions of TIMECHECKS
//
//
//  Insert / update?
//
    if(*m_pUpdateRecordID == NO_RECORD)
    {
      rcode2 = btrieve(B_GETLAST, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);
      TIMECHECKS.recordID = AssignRecID(rcode2, TIMECHECKS.recordID);
      opCode = B_INSERT;
    }
    else
    {
      TIMECHECKSKey0.recordID = *m_pUpdateRecordID;
      btrieve(B_GETEQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);
      opCode = B_UPDATE;
    }
//
//  Comment code is there but unused
//
    TIMECHECKS.COMMENTSrecordID = NO_RECORD;
//
//  Check date
//
    v = pDTPickerDATE->GetYear();
    year = v.lVal;
    v = pDTPickerDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerDATE->GetDay();
    day = v.lVal;
    TIMECHECKS.checkDate = year * 10000 + month * 100 + day;
//
//  Service
//
    TIMECHECKS.SERVICESrecordID = m_SERVICESrecordID;
//
//  Route
//
    TIMECHECKS.ROUTESrecordID = m_ROUTESrecordID;
//
//  Direction index
//
    TIMECHECKS.directionIndex = m_directionIndex;
//
//  Trips recordID (set on the scheduled listbox)
//
    TIMECHECKS.TRIPSrecordID = m_TRIPSrecordID;
//
//  Actual bustype
//
    TIMECHECKS.actualBUSTYPESrecordID = GetComboBoxValue(pComboBoxBUSTYPE);
//
//  Actual driver
//
    TIMECHECKS.actualDRIVERSrecordID = GetComboBoxValue(pComboBoxDRIVER);
//
//  Supervisor index
//
    TIMECHECKS.supervisorIndex = GetComboBoxValue(pComboBoxSUPERVISOR);
//
//  Passenger load index
//
    TIMECHECKS.passengerLoadIndex = GetComboBoxValue(pComboBoxPASSENGERLOAD);
//
//  Weather index and Traffic index
//
//  The LOWORD of the long gets the weather, the HIWORD gets the traffic.
//
//  Hindsight is always 20/20
//
    nI = (int)GetComboBoxValue(pComboBoxWEATHER);
    nJ = (int)GetComboBoxValue(pComboBoxTRAFFIC);
    TIMECHECKS.weatherIndex = MAKELONG(nI, nJ);
//
//  Other comments index
//
    TIMECHECKS.otherCommentsIndex = GetComboBoxValue(pComboBoxOTHER);
//
//  Reserved
//
    memset(TIMECHECKS.reserved, 0x00, TIMECHECKS_RESERVED_LENGTH);
//
//  Flags
//
    TIMECHECKS.flags = 0;
//
//  Loop through all the nodes
//
    for(nI = 0; nI < m_numNodes; nI++)
    {
//
//  Set up the balance of TIMECHECKS
//
//  Node and position in trip
//
      if(*m_pUpdateRecordID == NO_RECORD)
      {
        TIMECHECKS.NODESrecordID = m_NODESrecordIDs[nI];
        TIMECHECKS.nodePositionInTrip = m_NodePositions[nI];
      }
//
//  Actual time (from above)
//
      pEditACTUAL[nI]->GetWindowText(tempString, sizeof(tempString));
      TIMECHECKS.actualTime = cTime(tempString);
//
//  Insert and stay or update and loop back
//
      rcode2 = btrieve(opCode, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey0, 0);
      TIMECHECKS.recordID++;
    }

    if(*m_pUpdateRecordID != NO_RECORD)
    {
      CDialog::OnOK();
    }
    else
    {
      pComboBoxPASSENGERLOAD->SetCurSel(0);
      pComboBoxOTHER->SetCurSel(0);
      pEditTRIPNUMBER->SetFocus();
      long tempLong = MAKELONG(0, -1);
      pEditTRIPNUMBER->SetSel(tempLong, FALSE);
    }
  }
}

BOOL CAddTimecheck::DisplayRSD(BOOL bLookupTrip, TRIPSDef *pTRIPS)
{
  BOOL bFound;
  int  rcode2;

  if(bLookupTrip)
  {
    pEditTRIPNUMBER->GetWindowText(tempString, sizeof(tempString));
    m_TripNumber = atol(tempString);
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    bFound = FALSE;
    while(rcode2 == 0)
    {
      if(TRIPS.tripNumber == m_TripNumber)
      {
         bFound = TRUE;
         memcpy(pTRIPS, &TRIPS, sizeof(TRIPSDef));
         pTRIPS = &TRIPS;
         break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);     
    }
    if(!bFound)
    {
      sprintf(tempString, "Trip number %ld not found", m_TripNumber);
      MessageBox(tempString, TMS, MB_OK);
      pEditTRIPNUMBER->SetWindowText("");
      pEditTRIPNUMBER->SetFocus();
      return FALSE;
    }
  }
//
//  Get the route
//
  ROUTESKey0.recordID = pTRIPS->ROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  strcat(tempString, " - ");
  strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
  trim(szarString, ROUTES_NAME_LENGTH);
  strcat(tempString, szarString);
  pStaticROUTE->SetWindowText(tempString);
  m_ROUTESrecordID = pTRIPS->ROUTESrecordID;
//
//  Get the direction
//
  DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[pTRIPS->directionIndex];
  btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
  trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
  pStaticDIRECTION->SetWindowText(tempString);
  m_directionIndex = pTRIPS->directionIndex;
//
//  Get the service
//
  SERVICESKey0.recordID = pTRIPS->SERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(tempString, SERVICES_NAME_LENGTH);
  pStaticSERVICE->SetWindowText(tempString);
  m_SERVICESrecordID = pTRIPS->SERVICESrecordID;
//
//  All done
//
  return TRUE;
}

void CAddTimecheck::OnKillfocusTripnumber() 
{
  GenerateTripDef GTResults;
  TRIPSDef ThisTrip;
  int  nI, nJ;
  int  rcode2;

  if(m_bFinishedInProgress)
    return;

  if(!DisplayRSD(TRUE, &ThisTrip))
    return;

  m_TRIPSrecordID = ThisTrip.recordID;
//
//  Enable all the input fields and clear the statics
//
  for(nI = 0; nI < MAXTIMECHECKNODES; nI++)
  {
    pStaticNODE[nI]->SetWindowText("");
    pStaticSCHEDULED[nI]->SetWindowText("");
    pEditACTUAL[nI]->SetWindowText("");
    pEditACTUAL[nI]->EnableWindow(TRUE);
  }
//
//  Generate the trip
//
  m_numNodes = GenerateTrip(ThisTrip.ROUTESrecordID, ThisTrip.SERVICESrecordID,
        ThisTrip.directionIndex, ThisTrip.PATTERNNAMESrecordID,
        ThisTrip.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Loop through the pattern and set up the node names, and scheduled and actual times
//
  PATTERNSKey2.ROUTESrecordID = ThisTrip.ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = ThisTrip.SERVICESrecordID;
  PATTERNSKey2.directionIndex = ThisTrip.directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = ThisTrip.PATTERNNAMESrecordID;
  PATTERNSKey2.nodeSequence = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  nI = 0;
  while(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == ThisTrip.ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == ThisTrip.SERVICESrecordID &&
        PATTERNS.directionIndex == ThisTrip.directionIndex &&
        PATTERNS.PATTERNNAMESrecordID == ThisTrip.PATTERNNAMESrecordID)
  {
    if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
    {
      NODESKey0.recordID = PATTERNS.NODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      pStaticNODE[nI]->SetWindowText(tempString);
      strcpy(tempString, Tchar(GTResults.tripTimes[nI]));
      pStaticSCHEDULED[nI]->SetWindowText(tempString);
      pEditACTUAL[nI]->SetWindowText(tempString);
      m_NODESrecordIDs[nI] = NODES.recordID;
      m_NodePositions[nI] = 0;
      for(nJ = 0; nJ < nI; nJ++)
      {
        if(m_NODESrecordIDs[nJ] == m_NODESrecordIDs[nI])
          m_NodePositions[nI]++;
      }
      nI++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//
//  Disable the remaining input fields
//
  for(nJ = nI; nJ < MAXTIMECHECKNODES; nJ++)
  {
    pEditACTUAL[nJ]->EnableWindow(FALSE);
  }
//
//  Set the focus and selection at the first time
//
  pEditACTUAL[0]->SetFocus();
  long tempLong = MAKELONG(0, -1);
  pEditACTUAL[0]->SetSel(tempLong, FALSE);
}

void CAddTimecheck::SetUpTextComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox)
{
  int  nI, nJ;
  
  pComboBox->ResetContent();

  for(nI = 0; nI < numEntries; nI++)
  {
    nJ = pComboBox->AddString(pData[nI].szText);
    pComboBox->SetItemData(nJ, pData[nI].number);
  }
  if(numEntries != 0)
  {
    pComboBox->SetCurSel(0);
  }
  else
  {
    pComboBox->EnableWindow(FALSE);
  }
}

void CAddTimecheck::HighlightComboBoxValue(CComboBox *pComboBox, long searchValue)
{
  int nI;

  for(nI = 0; nI < pComboBox->GetCount(); nI++)
  {
    if((long)pComboBox->GetItemData(nI) == searchValue)
    {
      pComboBox->SetCurSel(nI);
      break;
    }
  }
}

long CAddTimecheck::GetComboBoxValue(CComboBox *pComboBox)
{
  int nI;

  nI = (int)pComboBox->GetCurSel();
  if(nI == CB_ERR)
    return(NO_RECORD);
  else
    return(pComboBox->GetItemData(nI));
}


void CAddTimecheck::OnByroute() 
{
  int nI;

//
//  Left side
//
  pComboBoxSPECIFICROUTE->EnableWindow(TRUE);
  pButtonOUTBOUND->EnableWindow(TRUE);
  pButtonOUTBOUND->SetCheck(TRUE);
  pButtonINBOUND->EnableWindow(TRUE);
  pButtonINBOUND->SetCheck(FALSE);
  pStaticSPECIFICSERVICETITLE->EnableWindow(TRUE);
  pComboBoxSPECIFICSERVICE->EnableWindow(TRUE);
  pStaticSPECIFICNODETITLE->EnableWindow(TRUE);
  pComboBoxSPECIFICNODE->EnableWindow(TRUE);
  pStaticSCHEDULEDTITLE->EnableWindow(TRUE);
  pComboBoxSCHEDULED->EnableWindow(TRUE);
  pStaticACTUALTITLE->EnableWindow(TRUE);
  pEditACTUAL00->EnableWindow(TRUE);
//
//  Right side
//
  pEditTRIPNUMBER->EnableWindow(FALSE);
  pEditTRIPNUMBER->SetWindowText("");
  pStaticROUTE->EnableWindow(FALSE);
  pStaticROUTE->SetWindowText("");
  pStaticDIRECTION->EnableWindow(FALSE);
  pStaticDIRECTION->SetWindowText("");
  pStaticSERVICE->EnableWindow(FALSE);
  pStaticSERVICE->SetWindowText("");
  for(nI = 0; nI < MAXTIMECHECKNODES; nI++)
  {
    pStaticNODE[nI]->EnableWindow(FALSE);
    pStaticNODE[nI]->SetWindowText("");
    pStaticSCHEDULED[nI]->EnableWindow(FALSE);
    pStaticSCHEDULED[nI]->SetWindowText("");
    pEditACTUAL[nI]->EnableWindow(FALSE);
    pEditACTUAL[nI]->SetWindowText("");
  }
}

void CAddTimecheck::OnBytripnumber() 
{
  if(!pButtonBYROUTE->IsWindowEnabled())
    return;

  int nI;

//
//  Left side
//
  pComboBoxSPECIFICROUTE->EnableWindow(FALSE);
  pComboBoxSPECIFICROUTE->SetCurSel(-1);
  pButtonOUTBOUND->EnableWindow(FALSE);
  pButtonOUTBOUND->SetCheck(FALSE);
  pButtonINBOUND->EnableWindow(FALSE);
  pButtonINBOUND->SetCheck(FALSE);
  pStaticSPECIFICSERVICETITLE->EnableWindow(FALSE);
  pComboBoxSPECIFICSERVICE->EnableWindow(FALSE);
  pComboBoxSPECIFICSERVICE->SetCurSel(-1);
  pStaticSPECIFICNODETITLE->EnableWindow(FALSE);
  pComboBoxSPECIFICNODE->EnableWindow(FALSE);
  pComboBoxSPECIFICNODE->SetCurSel(-1);
  pStaticSCHEDULEDTITLE->EnableWindow(FALSE);
  pComboBoxSCHEDULED->EnableWindow(FALSE);
  pComboBoxSCHEDULED->SetCurSel(-1);
  pStaticACTUALTITLE->EnableWindow(FALSE);
  pEditACTUAL00->EnableWindow(FALSE);
//
//  Right side
//
  pEditTRIPNUMBER->EnableWindow(TRUE);
  pStaticROUTE->EnableWindow(TRUE);
  pStaticDIRECTION->EnableWindow(TRUE);
  pStaticSERVICE->EnableWindow(TRUE);
  for(nI = 0; nI < MAXTIMECHECKNODES; nI++)
  {
    pStaticNODE[nI]->EnableWindow(TRUE);
    pStaticSCHEDULED[nI]->EnableWindow(TRUE);
    pEditACTUAL[nI]->EnableWindow(TRUE);
  }
}


void CAddTimecheck::OnOutbound() 
{
  SetUpTimepointsAndTimes(NO_RECORD);
}

void CAddTimecheck::OnInbound() 
{
  SetUpTimepointsAndTimes(NO_RECORD);
}

void CAddTimecheck::OnEditchangeScheduled() 
{
	// TODO: Add your control notification handler code here
	
}

void CAddTimecheck::SetUpTimepointsAndTimes(long NODESrecordID)
{
  long ROUTESrecordID;
  long directionIndex;
  long SERVICESrecordID;
  long MLPNODESrecordID;
  long timeAtNode;
  int  nI;
  int  rcode2;
//
//  Get the route
//
  nI = (int)pComboBoxSPECIFICROUTE->GetCurSel();
  if(nI == CB_ERR)
    return;
  ROUTESrecordID = pComboBoxSPECIFICROUTE->GetItemData(nI);
//
//  Get the direction index
//
  directionIndex = pButtonOUTBOUND->GetCheck() ? 0 : 1;
//
//  Get the service
//
  nI = (int)pComboBoxSPECIFICSERVICE->GetCurSel();
  if(nI == CB_ERR)
    return;
  SERVICESrecordID = pComboBoxSPECIFICSERVICE->GetItemData(nI);
//
//  If we come in with no NODE, get the BASE pattern nodes and select the MLP
//
  if(NODESrecordID == NO_RECORD)
  {
    pComboBoxSPECIFICNODE->ResetContent();
    PATTERNSKey2.ROUTESrecordID = ROUTESrecordID;
    PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
    PATTERNSKey2.directionIndex = directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
    PATTERNSKey2.nodeSequence = 0;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    MLPNODESrecordID = NO_RECORD;
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == ROUTESrecordID &&
          PATTERNS.SERVICESrecordID == SERVICESrecordID &&
          PATTERNS.directionIndex == directionIndex &&
          PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
    {
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        nI = (int)pComboBoxSPECIFICNODE->AddString(tempString);
        pComboBoxSPECIFICNODE->SetItemData(nI, NODES.recordID);
        if(PATTERNS.flags & PATTERNS_FLAG_MLP)
        {
          pComboBoxSPECIFICNODE->SetCurSel(nI);
          MLPNODESrecordID = NODES.recordID;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
  }
//
//  Get the node times for the MLP / passed in NODESrecordID
//
  pComboBoxSCHEDULED->ResetContent();
  TRIPSKey1.ROUTESrecordID = ROUTESrecordID;
  TRIPSKey1.SERVICESrecordID = SERVICESrecordID;
  TRIPSKey1.directionIndex = directionIndex;
  TRIPSKey1.tripSequence = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
  while(rcode2 == 0 &&
        TRIPS.ROUTESrecordID == ROUTESrecordID &&
        TRIPS.SERVICESrecordID == SERVICESrecordID &&
        TRIPS.directionIndex == directionIndex)
  {
    if(NODESrecordID == NO_RECORD)
      timeAtNode = GetTimeAtNode(NO_RECORD, MLPNODESrecordID, 0, &TRIPS);
    else
      timeAtNode = GetTimeAtNode(NO_RECORD, NODESrecordID, 0, &TRIPS);
    if(timeAtNode != NO_TIME)
    {
      strcpy(tempString, Tchar(timeAtNode));
      nI = (int)pComboBoxSCHEDULED->AddString(tempString);
      pComboBoxSCHEDULED->SetItemData(nI, TRIPS.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
  }
  pComboBoxSCHEDULED->SetCurSel(0);
}

void CAddTimecheck::OnSelchangeSpecificroute() 
{
  int nI;

  nI = (int)pComboBoxSPECIFICROUTE->GetCurSel();

  if(nI == CB_ERR)
    return;

  pButtonINBOUND->EnableWindow(TRUE);
  ROUTESKey0.recordID = pComboBoxSPECIFICROUTE->GetItemData(nI);
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  for(nI = 0; nI < 2; nI++)
  {
    if(nI == 1 && ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
    {
      pButtonINBOUND->SetWindowText("");
      pButtonINBOUND->EnableWindow(FALSE);
    }
    else
    {
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
      trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
      if(nI == 0)
        pButtonOUTBOUND->SetWindowText(tempString);
      else
        pButtonINBOUND->SetWindowText(tempString);
    }
  }
  pButtonOUTBOUND->SetCheck(TRUE);
  pButtonINBOUND->SetCheck(FALSE);
  SetUpTimepointsAndTimes(NO_RECORD);
}

void CAddTimecheck::OnSelchangeSpecificservice() 
{
  SetUpTimepointsAndTimes(NO_RECORD);
}

void CAddTimecheck::OnSelchangeSpecificnode() 
{
  long NODESrecordID;
  int  nI;
  
  nI = (int)pComboBoxSPECIFICNODE->GetCurSel();
  if(nI == CB_ERR)
    return;
  NODESrecordID = pComboBoxSPECIFICNODE->GetItemData(nI);
  SetUpTimepointsAndTimes(NODESrecordID);
}
