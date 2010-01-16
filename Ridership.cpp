// Ridership.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "DTPicker.h"
#include "sdigrid.h"
#include "NumericEdit.h"
#include "Ridership.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRidership dialog

#define GRIDPOS_ROUTE       1
#define GRIDPOS_TRIP        2
#define GRIDPOS_NODE        3
#define GRIDPOS_TIME        4
#define GRIDPOS_FULLFARES   5
#define GRIDPOS_FULLVALUE   6
#define GRIDPOS_HALFFARES   7
#define GRIDPOS_HALFVALUE   8
#define GRIDPOS_HCFARES     9
#define GRIDPOS_HCVALUE    10
#define GRIDPOS_OTHERFARES 11
#define GRIDPOS_OTHERVALUE 12

CRidership::CRidership(CWnd* pParent /*=NULL*/)
	: CDialog(CRidership::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRidership)
	//}}AFX_DATA_INIT
}


void CRidership::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRidership)
	DDX_Control(pDX, RIDERSHIP_ADJUSTMENTS, m_ctlAdjustments);
	DDX_Control(pDX, RIDERSHIP_MILEAGEOUT, m_ctlMileageOut);
	DDX_Control(pDX, RIDERSHIP_MILEAGEIN, m_ctlMileageIn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRidership, CDialog)
	//{{AFX_MSG_MAP(CRidership)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_CBN_SELCHANGE(RIDERSHIP_RUNNUMBER, OnSelchangeRunnumber)
	ON_BN_CLICKED(IDCALCULATE, OnCalculate)
	ON_BN_CLICKED(IDACCEPT, OnAccept)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRidership message handlers

BEGIN_EVENTSINK_MAP(CRidership, CDialog)
    //{{AFX_EVENTSINK_MAP(CRidership)
	ON_EVENT(CRidership, RIDERSHIP_DATE, 2 /* Change */, OnChangeDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CRidership::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//	
  pDTPickerDATE = (CDTPicker *)GetDlgItem(RIDERSHIP_DATE);
  pComboBoxRUNNUMBER = (CComboBox *)GetDlgItem(RIDERSHIP_RUNNUMBER);
  pComboBoxBUS = (CComboBox *)GetDlgItem(RIDERSHIP_BUS);
  pComboBoxACTUALDRIVER = (CComboBox *)GetDlgItem(RIDERSHIP_ACTUALDRIVER);
  pStaticROSTEREDDRIVER = (CStatic *)GetDlgItem(RIDERSHIP_ROSTEREDDRIVER);
  pSDIGridGRID = (CSDIGrid *)GetDlgItem(RIDERSHIP_GRID);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(RIDERSHIP_LIST);
  pStaticTOTALS = (CStatic *)GetDlgItem(RIDERSHIP_TOTALS);
  pStaticTOTALMILEAGE = (CStatic *)GetDlgItem(RIDERSHIP_TOTALMILEAGE);
  pEditNOTES = (CEdit *)GetDlgItem(RIDERSHIP_NOTES);
//
//  Set up the list control
//
//
//  Set up the action list control
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(3, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(4, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(5, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(6, &LVC);

  LVC.cx = 50;
  pListCtrlLIST->InsertColumn(7, &LVC);

//
//  Set today's date on the Date and Time picker
//
  CTime time = CTime::GetCurrentTime();

  COleVariant v;
  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);
//
//  Figure out what day of the week we are
//
  int  indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};

  CTime workDate(year, month, day, 0, 0, 0);
  
  m_Today = indexToRoster[workDate.GetDayOfWeek() - 1];
  m_DateToUse = (year * 10000) + (month * 100) + day;
  m_SERVICESrecordIDInEffect = ROSTERPARMS.serviceDays[m_Today];
//
//  Make sure a roster template's been established
//
  int rcode2;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = year * 10000 + month * 100 + day;
  DAILYOPSKey1.pertainsToTime = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  if(rcode2 == 0 && (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    m_ROSTERDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.ROSTERDIVISIONSrecordID;
    m_RUNSDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID;
  }
  else
  {
    m_ROSTERDivisionInEffect = m_DivisionRecordID;
    m_RUNSDivisionInEffect = m_DivisionRecordID;
  }
//
//  Get the list of run numbers for today
//
  int nI, nJ;

  for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
  {
    sprintf(tempString, "%6ld", m_BlockInfo[nI].startingRunNumber);
    nJ = pComboBoxRUNNUMBER->AddString(tempString);
    if(nJ != CB_ERR)
    {
      pComboBoxRUNNUMBER->SetItemData(nJ, nI);
    }
  }
//
//  Were there any special runs added?
//
//
//  See if there's anything in the Audit Trail for this run on this date
//
//  Display the previously entered fares
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPSKey1.pertainsToDate = m_DateToUse;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
         DAILYOPS.pertainsToDate == m_DateToUse)
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SPECIALTRIP)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        sprintf(tempString, "%6ld*", DAILYOPS.DOPS.Ridership.data.S.runNumber);
        nJ = pComboBoxRUNNUMBER->AddString(tempString);
        if(nJ != CB_ERR)
        {
          pComboBoxRUNNUMBER->SetItemData(nJ, -DAILYOPS.recordID);
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Finally, add "Special" to the list
//
  nJ = pComboBoxRUNNUMBER->AddString("Special");
  if(nJ != CB_ERR)
  {
    pComboBoxRUNNUMBER->SetItemData(nJ, NO_RECORD);
  }
//
//  Get the list of buses
//
  rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
    trim(tempString, BUSES_NUMBER_LENGTH);
    nI = pComboBoxBUS->AddString(tempString);
    pComboBoxBUS->SetItemData(nI, BUSES.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  }
  pComboBoxBUS->SetCurSel(-1);
//
//  Get the list of drivers
//
  rcode2 =btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
    nI = pComboBoxACTUALDRIVER->AddString(tempString);
    pComboBoxACTUALDRIVER->SetItemData(nI, DRIVERS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  pComboBoxACTUALDRIVER->SetCurSel(-1);
//
//  Set up the totals
//
  ZeroOutList();

  m_numRows = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRidership::OnChangeDate() 
{
  pComboBoxRUNNUMBER->SetCurSel(-1);
  pComboBoxBUS->SetCurSel(-1);
  pStaticROSTEREDDRIVER->SetWindowText("");
  pComboBoxACTUALDRIVER->SetCurSel(-1);
  m_ctlMileageOut.SetWindowText("");
  m_ctlMileageIn.SetWindowText("");
  pStaticTOTALMILEAGE->SetWindowText("0.0");
  pSDIGridGRID->ClearGrid();
  m_ctlAdjustments.SetWindowText("");
  pStaticTOTALS->SetWindowText("Total Trips: 0, Total Pax: 0, Total Cash: $0.00");
  pEditNOTES->SetWindowText("");
//
//  Get the new date
//
  COleVariant v;
  long year, month, day;

  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetDay();
  day = v.lVal;
//
//  Figure out what day of the week we are
//
  int  indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};

  CTime workDate(year, month, day, 0, 0, 0);
  
  m_Today = indexToRoster[workDate.GetDayOfWeek() - 1];
  m_DateToUse = (year * 10000) + (month * 100) + day;
  m_SERVICESrecordIDInEffect = ROSTERPARMS.serviceDays[m_Today];
}


void CRidership::OnSelchangeRunnumber() 
{
  BOOL bFound;
  BOOL bFirst;
  int  nI;
  int  rcode2;
  int  BIIndex;
  int  numBuses;
  int  numDrivers;
//
//  Get the run number
//
  nI = pComboBoxRUNNUMBER->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  BIIndex = (int)pComboBoxRUNNUMBER->GetItemData(nI);
//
//  Couple of special cases here.
//
//  BIIndex == NO_RECORD means he's hit "Special"
//  BIIndex == Other -ve number means he's selected an existing "Special run"
//  BIIndex >= 0 means we know the data sits in m_BlockInfo
//
//  New "Special"
//
  if(BIIndex == NO_RECORD)
  {
  }
//
//  Existing "Special"
//
  else if(BIIndex < 0)
  {
  }
//
//  Data's in m_BlockInfo
//
  else
  {
    m_RUNSrecordID = m_BlockInfo[BIIndex].startingRUNSrecordID;
    m_RunNumber = m_BlockInfo[BIIndex].startingRunNumber;
//
//  Display the vehicle that was used
//
    numBuses = pComboBoxBUS->GetCount();
    bFound = FALSE;
    if(m_BlockInfo[BIIndex].BUSESrecordID != NO_RECORD)
    {
      for(nI = 0; nI < numBuses; nI++)
      {
        if(m_BlockInfo[BIIndex].BUSESrecordID == (long)pComboBoxBUS->GetItemData(nI))
        {
          bFound = TRUE;
          pComboBoxBUS->SetCurSel(nI);
          break;
        }
      }
    }
    if(!bFound)
    {
      pComboBoxBUS->SetCurSel(-1);
    }
//
//  Display the rostered driver
//
    numDrivers = pComboBoxACTUALDRIVER->GetCount();
    bFound = FALSE;
    if(m_BlockInfo[BIIndex].startingDRIVERSrecordID != NO_RECORD)
    {
      for(nI = 0; nI < numDrivers; nI++)
      {
        if(m_BlockInfo[BIIndex].startingDRIVERSrecordID == (long)pComboBoxACTUALDRIVER->GetItemData(nI))
        {
          bFound = TRUE;
          pComboBoxACTUALDRIVER->GetLBText(nI, tempString);
          pStaticROSTEREDDRIVER->SetWindowText(tempString);
          m_RosteredDRIVERSrecordID = ROSTER.DRIVERSrecordID;
          break;
        }
      }
    }
    if(!bFound)
    {
      pStaticROSTEREDDRIVER->SetWindowText("No rostered driver");
      m_RosteredDRIVERSrecordID = NO_RECORD;
    }
//
//  Display the actual driver
// 
    for(bFound = FALSE, nI = 0; nI < numDrivers; nI++)
    {
      if(m_BlockInfo[BIIndex].startingDRIVERSrecordID == (long)pComboBoxACTUALDRIVER->GetItemData(nI))
      {
        bFound = TRUE;
        pComboBoxACTUALDRIVER->SetCurSel(nI);
        break;
      }
    }
    if(!bFound)
    {
      pComboBoxACTUALDRIVER->SetCurSel(-1);
    }
//
//  Clear the grid
//
    pSDIGridGRID->ClearGrid();
    pSDIGridGRID->SetNumRows(500);
    m_numRows = 500;
//
//  Get the trip data
//
    GenerateTripDef GTResults;
    short rowNumber;
    long blockNumber;
    long SERVICErecordID;
    long previousRoute;
    char szRouteNumber[ROUTES_NUMBER_LENGTH + 1];
    char szNodeName[NODES_ABBRNAME_LENGTH + NODES_ABBRNAME_LENGTH + 1];
    int  position;
    int  routeIndex;

    TRIPSKey0.recordID = m_BlockInfo[BIIndex].TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Cycle through all the trips on the block
//
    rowNumber = 1;
    previousRoute = NO_RECORD;
    blockNumber = TRIPS.standard.blockNumber;
    SERVICErecordID = TRIPS.standard.SGRPSERVICESrecordID;
    m_numTrips = 0;
    while(rcode2 == 0 &&
          TRIPS.standard.blockNumber == blockNumber &&
          TRIPS.standard.SGRPSERVICESrecordID == SERVICErecordID)
    {
      m_numTrips++;
      position = 0;
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
           TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
           TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex)
      {
//
//  Display the data
//
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
        {
//
//  Route
//
          if(TRIPS.ROUTESrecordID != previousRoute)
          {
            ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
            previousRoute = ROUTES.recordID;
//
//  Get the fare info for this route
//
            for(bFound = FALSE, nI = 0; nI < m_numFareTypesByRoute; nI++)
            {
              if(m_FareTypesByRoute[nI].ROUTESrecordID == ROUTES.recordID)
              {
                bFound = TRUE;
                routeIndex = nI;
                break;
              }
            }
          }
          pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_ROUTE, szRouteNumber);
          m_RowByRow[rowNumber].ROUTESrecordID = TRIPS.ROUTESrecordID;
         if(bFound)
          {
            m_IndexToFTBR[rowNumber] = routeIndex;
            sprintf(tempString, "$%5.02f", m_FareTypesByRoute[routeIndex].fareValue[0]);
            pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_FULLVALUE, tempString);
            sprintf(tempString, "$%5.02f", m_FareTypesByRoute[routeIndex].fareValue[1]);
            pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_HALFVALUE, tempString);
            sprintf(tempString, "$%5.02f", m_FareTypesByRoute[routeIndex].fareValue[2]);
            pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_HCVALUE, tempString);
            sprintf(tempString, "$%5.02f", m_FareTypesByRoute[routeIndex].fareValue[3]);
            pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_OTHERVALUE, tempString);
          }
//
//  Trip number
//
          sprintf(tempString, "%ld", TRIPS.tripNumber);
          pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_TRIP, tempString);
//
//  Trip recordID
//
          m_RowByRow[rowNumber].TRIPSrecordID = TRIPS.recordID;
//
//  Node name
//
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          strcpy(szNodeName, "");
          for(nI = 0; nI < (int)strlen(tempString); nI++)
          {
            if(tempString[nI] == '&')
            {
              strcat(szNodeName, "&&");
            }
            else
            {
              szarString[0] = tempString[nI];
              szarString[1] = '\0';
              strcat(szNodeName, szarString);
            }
          }
          m_RowByRow[rowNumber].NODESrecordID = NODES.recordID;
          pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_NODE, szNodeName);
//
//  Position in trip
//
          m_RowByRow[rowNumber].nodePositionInTrip = position;
//
//  Node time
//
          strcpy(tempString, Tchar(GTResults.tripTimes[position]));
          pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_TIME, tempString);
//
//  Move along the array
//
          position++;
          rowNumber++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
    }
    pSDIGridGRID->SetNumRows(rowNumber);
    m_numRows = rowNumber;

//
//  See if there's anything in the Audit Trail for this run on this date
//
//  Display the previously entered fares
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = m_DateToUse;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == m_DateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETMILEAGE)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          if(DAILYOPS.DOPS.Ridership.data.M.RUNSrecordID == m_BlockInfo[BIIndex].startingRUNSrecordID)
          {
//
//  Bus 
//
            numBuses = pComboBoxBUS->GetCount();
            bFound = FALSE;
            for(nI = 0; nI < numBuses; nI++)
            {
              if(DAILYOPS.DOPS.Ridership.data.M.BUSESrecordID == (long)pComboBoxBUS->GetItemData(nI))
              {
                bFound = TRUE;
                pComboBoxBUS->SetCurSel(nI);
                break;
              }
            }
            if(!bFound)
            {
              pComboBoxBUS->SetCurSel(-1);
            }
//
//  Mileage
//
            sprintf(tempString, "%8.1f", DAILYOPS.DOPS.Ridership.data.M.mileageOut);
            m_ctlMileageOut.SetWindowText(tempString);
            sprintf(tempString, "%8.1f", DAILYOPS.DOPS.Ridership.data.M.mileageIn);
            m_ctlMileageIn.SetWindowText(tempString);
            break;
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
//
//  Were there any notes?
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = m_DateToUse;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == m_DateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ATTACHNOTE)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          OFFTIMEKey1.DAILYOPSrecordID = DAILYOPS.recordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey1, 1);
          if(rcode2 == 0)
          {
            strncpy(tempString, OFFTIME.text, OFFTIME_TEXT_LENGTH);
            trim(tempString, OFFTIME_TEXT_LENGTH);
            pEditNOTES->SetWindowText(tempString);
          }
          break;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
//
//  Now get the ridership numbers
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = m_DateToUse;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    bFirst = TRUE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == m_DateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETRIDERSHIP)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
//
//  First time through, set the actual driver as recorded
//
          if(bFirst)
          {
            for(nI = 0; nI < numDrivers; nI++)
            {
              if(DAILYOPS.DOPS.Ridership.data.R.actualDRIVERSrecordID == (long)pComboBoxACTUALDRIVER->GetItemData(nI))
              {
                pComboBoxACTUALDRIVER->SetCurSel(nI);
                break;
              }
            }
            bFirst = FALSE;
          }
//
//  Same run
//
          if(DAILYOPS.DOPS.Ridership.data.M.RUNSrecordID == m_BlockInfo[BIIndex].startingRUNSrecordID)
          {
//
//  Locate the row in the grid
//
            for(rowNumber = NO_RECORD, nI = 1; nI <= m_numRows; nI++)
            {
              if(DAILYOPS.DOPS.Ridership.data.R.TRIPSrecordID == m_RowByRow[nI].TRIPSrecordID &&
                    DAILYOPS.DOPS.Ridership.data.R.NODESrecordID == m_RowByRow[nI].NODESrecordID &&
                    DAILYOPS.DOPS.Ridership.data.R.nodePositionInTrip == m_RowByRow[nI].nodePositionInTrip)
              {
                rowNumber = nI;
                break;
              }
            }
            if(rowNumber != NO_RECORD)
            {
              sprintf(tempString, "%d", DAILYOPS.DOPS.Ridership.data.R.numFares[0]);
              pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_FULLFARES, tempString);
              sprintf(tempString, "%d", DAILYOPS.DOPS.Ridership.data.R.numFares[1]);
              pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_HALFFARES, tempString);
              sprintf(tempString, "%d", DAILYOPS.DOPS.Ridership.data.R.numFares[2]);
              pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_HCFARES, tempString);
              sprintf(tempString, "%d", DAILYOPS.DOPS.Ridership.data.R.numFares[3]);
              pSDIGridGRID->SetCellContents(rowNumber, GRIDPOS_OTHERFARES, tempString);
            }
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
//
//  Any cash adjustments?
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPSKey1.pertainsToDate = m_DateToUse;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
           DAILYOPS.pertainsToDate == m_DateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_CASHADJUSTMENT)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          if(DAILYOPS.DOPS.Ridership.data.C.RUNSrecordID == m_BlockInfo[BIIndex].startingRUNSrecordID)
          {
            sprintf(tempString, "%6.2f", DAILYOPS.DOPS.Ridership.data.C.adjustments);
            m_ctlAdjustments.SetWindowText(tempString);
            break;
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }

//
//  Display any summary data
//
    OnCalculate();
    m_bNeedToUpdate = TRUE;
  }
}

void CRidership::OnCalculate() 
{
  CString s, t;
  LVITEM  LVI;
  float   totalValue[500][4];
  short   nI, nJ;
  int     numberOfFares[500][4];
  int     number;
//
//  Ensure values in "Mileage Out" and "Mileage In"
//
  m_ctlMileageOut.GetWindowText(s);
  m_ctlMileageIn.GetWindowText(t);

  m_MileageOut = (float)atof(s);
  m_MileageIn = (float)atof(t);

  if(m_MileageIn - m_MileageOut < 0)
  {
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox("Total mileage is less than 0\nAre you re-establishing mileage?", TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
    {
      return;
    }
  }
  sprintf(tempString, "%8.1f", m_MileageIn - m_MileageOut);
  pStaticTOTALMILEAGE->SetWindowText(tempString);
//
//  Get the numberOfFares
//
  memset(&numberOfFares, 0x00, sizeof(numberOfFares));
  memset(&totalValue, 0x00, sizeof(totalValue));
  for(nI = 1; nI <= m_numRows; nI++)
  {
    s = pSDIGridGRID->GetCellContents(nI, GRIDPOS_FULLFARES);
    if(s != "")
    {
      number = atoi(s);
      numberOfFares[nI][0] += number;
      totalValue[nI][0] += (float)(number * m_FareTypesByRoute[m_IndexToFTBR[nI]].fareValue[0]);
    }
   
    s = pSDIGridGRID->GetCellContents(nI, GRIDPOS_HALFFARES);
    if(s != "")
    {
      number = atoi(s);
      numberOfFares[nI][1] += number;
      totalValue[nI][1] += (float)(number * m_FareTypesByRoute[m_IndexToFTBR[nI]].fareValue[1]);
    }
   
    s = pSDIGridGRID->GetCellContents(nI, GRIDPOS_HCFARES);
    if(s != "")
    {
      number = atoi(s);
      numberOfFares[nI][2] += number;
      totalValue[nI][2] += (float)(number * m_FareTypesByRoute[m_IndexToFTBR[nI]].fareValue[2]);
    }
   
    s = pSDIGridGRID->GetCellContents(nI, GRIDPOS_OTHERFARES);
    if(s != "")
    {
      number = atoi(s);
      numberOfFares[nI][3] += number;
      totalValue[nI][3] += (float)(number * m_FareTypesByRoute[m_IndexToFTBR[nI]].fareValue[3]);
    }
  }
//
//  Determine the sums
//
  int   sumOfFares[4];
  float sumOfValues[4];
  float finalTotal;

  memset(&sumOfFares,  0x00, sizeof(sumOfFares));
  memset(&sumOfValues, 0x00, sizeof(sumOfValues));
  for(nI = 1; nI <= m_numRows; nI++)
  {
    for(nJ = 0; nJ < 4; nJ++)
    {
      sumOfFares[nJ]  += numberOfFares[nI][nJ];
      sumOfValues[nJ] += totalValue[nI][nJ];
    }
  }
  finalTotal = 0.0;
//
//  Fare type 1
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 0;
  sprintf(tempString, "%d", sumOfFares[0]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);

  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 1;
  sprintf(tempString, "$%6.02f", sumOfValues[0]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
  finalTotal += sumOfValues[0];
//
//  Fare type 2
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 2;
  sprintf(tempString, "%d", sumOfFares[1]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);

  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 3;
  sprintf(tempString, "$%6.02f", sumOfValues[1]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
  finalTotal += sumOfValues[1];
//
//  Fare type 3
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 4;
  sprintf(tempString, "%d", sumOfFares[2]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);

  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 5;
  sprintf(tempString, "$%6.02f", sumOfValues[2]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
  finalTotal += sumOfValues[2];
//
//  Fare type 4
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 6;
  sprintf(tempString, "%d", sumOfFares[3]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);

  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 7;
  sprintf(tempString, "$%6.02f", sumOfValues[3]);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
  finalTotal += sumOfValues[3];
//
//  Total fares
//
  int totalFares = 0;

  for(nI = 0; nI < 4; nI++)
  {
    totalFares += sumOfFares[nI];
  }
//
//  Any adjustments?
//
  m_ctlAdjustments.GetWindowText(s);
  m_CashAdjustments = (float)atof(s);

  sprintf(tempString, "Total Trips: %d, Total Pax: %d, Total Cash: $%6.02f", m_numTrips, totalFares, finalTotal + m_CashAdjustments);
  pStaticTOTALS->SetWindowText(tempString);
}

void CRidership::OnAccept() 
{
  OnCalculate();

//
//  Write out the data to the Audit Trail
//
//  Invalidate the previous fare entries
//
  CTime today = CTime::GetCurrentTime();

  long DAILYOPSrecordID;
  long absRecID;
  long RUNSrecordID;
  BOOL recordFlag;
  BOOL bFound;
  int  rcode2;
  int  nI, nJ;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPSKey1.pertainsToDate = m_DateToUse;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
         DAILYOPS.pertainsToDate == m_DateToUse)
  {
    if(m_DateToUse == DAILYOPS.pertainsToDate &&
          ((DAILYOPS.recordFlags & DAILYOPS_FLAG_SETMILEAGE) ||
           (DAILYOPS.recordFlags & DAILYOPS_FLAG_SETRIDERSHIP)))
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETMILEAGE)
      {
        RUNSrecordID = DAILYOPS.DOPS.Ridership.data.M.RUNSrecordID;
        recordFlag = DAILYOPS_FLAG_SETMILEAGE;
      }
      else
      {
        RUNSrecordID = DAILYOPS.DOPS.Ridership.data.R.RUNSrecordID;
        recordFlag = DAILYOPS_FLAG_SETRIDERSHIP;
      }
//
//  Make sure we're on the right run
//
      if(RUNSrecordID == m_RUNSrecordID)
      {
        rcode2 = btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &DAILYOPSKey1, 1);
        bFound = ANegatedRecord(DAILYOPS.recordID, 1) || ANegatingRecord(&DAILYOPS);
        if(!bFound)
        {
          rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
          DAILYOPS.entryDateAndTime = today.GetTime();
          DAILYOPS.pertainsToDate = 0;
          DAILYOPS.pertainsToTime = 0;
          DAILYOPS.userID = m_UserID;
          DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
          if(recordFlag & DAILYOPS_FLAG_SETMILEAGE)
          {
            DAILYOPS.recordFlags = DAILYOPS_FLAG_CLEARMILEAGE;
          }
          else
          {
            DAILYOPS.recordFlags = DAILYOPS_FLAG_CLEARRIDERSHIP;
          }
          DAILYOPS.DAILYOPSrecordID = NO_RECORD;
          DAILYOPS.DRIVERSrecordID = NO_RECORD;
          memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
          DAILYOPSrecordID = DAILYOPS.recordID;
        }
//
//  Re-establish position
//
        DAILYOPS.recordID = absRecID;
        btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
//
//  Tell the initial record about the unregister
//
        if(!bFound)
        {
          DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
          rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Cycle through and create the new entries
//
//  Mileage
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = today.GetTime();
  DAILYOPS.pertainsToDate = m_DateToUse;
  DAILYOPS.pertainsToTime = 0;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_SETMILEAGE;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.DRIVERSrecordID = NO_RECORD;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
  DAILYOPS.DOPS.Ridership.data.M.RUNSrecordID = m_RUNSrecordID;
  nI = pComboBoxBUS->GetCurSel();
  DAILYOPS.DOPS.Ridership.data.M.BUSESrecordID = 
        (nI == CB_ERR ? NO_RECORD : pComboBoxBUS->GetItemData(nI));
  DAILYOPS.DOPS.Ridership.data.M.mileageOut = m_MileageOut;
  DAILYOPS.DOPS.Ridership.data.M.mileageIn = m_MileageIn;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Cash Adjustments
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = today.GetTime();
  DAILYOPS.pertainsToDate = m_DateToUse;
  DAILYOPS.pertainsToTime = 0;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_CASHADJUSTMENT;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.DRIVERSrecordID = NO_RECORD;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
  DAILYOPS.DOPS.Ridership.data.C.RUNSrecordID = m_RUNSrecordID;
  DAILYOPS.DOPS.Ridership.data.C.adjustments = m_CashAdjustments;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Fares Collected
//
  CString s;

  for(nI = 1; nI <= m_numRows; nI++)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = today.GetTime();
    DAILYOPS.pertainsToDate = m_DateToUse;
    DAILYOPS.pertainsToTime = 0;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_SETRIDERSHIP;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    DAILYOPS.DOPS.Ridership.data.R.RUNSrecordID = m_RUNSrecordID;
    DAILYOPS.DOPS.Ridership.data.R.TRIPSrecordID = m_RowByRow[nI].TRIPSrecordID;
    DAILYOPS.DOPS.Ridership.data.R.rosteredDRIVERSrecordID = m_RosteredDRIVERSrecordID;
    nJ = pComboBoxACTUALDRIVER->GetCurSel();
    DAILYOPS.DOPS.Ridership.data.R.actualDRIVERSrecordID =
          (nJ == CB_ERR ? NO_RECORD : pComboBoxACTUALDRIVER->GetItemData(nJ));
    DAILYOPS.DOPS.Ridership.data.R.NODESrecordID = m_RowByRow[nI].NODESrecordID;
    DAILYOPS.DOPS.Ridership.data.R.nodePositionInTrip = m_RowByRow[nI].nodePositionInTrip;
    s = pSDIGridGRID->GetCellContents((short int)nI, GRIDPOS_FULLFARES);
    DAILYOPS.DOPS.Ridership.data.R.numFares[0] = atoi(s);
    s = pSDIGridGRID->GetCellContents((short int)nI, GRIDPOS_HALFFARES);
    DAILYOPS.DOPS.Ridership.data.R.numFares[1] = atoi(s);
    s = pSDIGridGRID->GetCellContents((short int)nI, GRIDPOS_HCFARES);
    DAILYOPS.DOPS.Ridership.data.R.numFares[2] = atoi(s);
    s = pSDIGridGRID->GetCellContents((short int)nI, GRIDPOS_OTHERFARES);
    DAILYOPS.DOPS.Ridership.data.R.numFares[3] = atoi(s);
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Were there any notes?
//
  pEditNOTES->GetWindowText(s);
  if(s != "")
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = today.GetTime();
    DAILYOPS.pertainsToDate = m_DateToUse;
    DAILYOPS.pertainsToTime = 0;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_ATTACHNOTE;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    DAILYOPS.DOPS.Ridership.data.N.RUNSrecordID = m_RUNSrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
//
//  Set up the OFFTIME record
//
//  RecordID
//
    rcode2 = btrieve(B_GETLAST, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
    OFFTIME.recordID = AssignRecID(rcode2, OFFTIME.recordID);
//
//  DailyOps recordID
//
    OFFTIME.DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Text
//
    strcpy(tempString, s);
    pad(tempString, OFFTIME_TEXT_LENGTH);
    strncpy(OFFTIME.text, tempString, OFFTIME_TEXT_LENGTH);
//
//  Add the record
//
    rcode2 = btrieve(B_INSERT, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Done
//
  MessageBeep(MB_ICONINFORMATION);
  MessageBox("Record Accepted\n\nPress OK to clear grid", TMS, MB_ICONINFORMATION | MB_OK);
//
//  Clear
//
  pComboBoxRUNNUMBER->SetCurSel(-1);
  pComboBoxBUS->SetCurSel(-1);
  pStaticROSTEREDDRIVER->SetWindowText("");
  pComboBoxACTUALDRIVER->SetCurSel(-1);
  pSDIGridGRID->ClearGrid();
  m_ctlMileageOut.SetWindowText("");
  m_ctlMileageIn.SetWindowText("");
  m_ctlAdjustments.SetWindowText("");
  pStaticTOTALMILEAGE->SetWindowText("");
  pEditNOTES->SetWindowText("");
  ZeroOutList();
  m_bNeedToUpdate = FALSE;
}

void CRidership::ZeroOutList()
{
//
//  Set up the totals
//
  LVITEM LVI;
//
//  Zeros for the totals
//
//  Full
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 0;
  strcpy(tempString, "0");
  LVI.pszText = tempString;
  pListCtrlLIST->InsertItem(&LVI);
//
//  Half
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 2;
  strcpy(tempString, "0");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  OCOC
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 4;
  strcpy(tempString, "0");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  HC
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 6;
  strcpy(tempString, "0");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  $0.00 for the totals
//
//  Full
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 1;
  strcpy(tempString, "$0.00");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  Half
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 3;
  strcpy(tempString, "$0.00");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  OCOC
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 5;
  strcpy(tempString, "$0.00");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  HC
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 7;
  strcpy(tempString, "$0.00");
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  Totals
//
  pStaticTOTALS->SetWindowText("Total Trips: 0, Total Pax: 0, Total Cash: $0.00");
}

void CRidership::OnClose() 
{
  if(m_bNeedToUpdate)
  {
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox("Changes to the last run were not accepted\n\nDo you really want to exit?", 
          TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
    {
      return;
    }
  }
  OnCancel();
}

void CRidership::OnCancel() 
{
	CDialog::OnCancel();
}

void CRidership::OnHelp() 
{
}

void CRidership::OnOK() 
{
  if(m_bNeedToUpdate)
  {
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox("Changes to the last run were not accepted\n\nDo you really want to exit?", 
          TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
    {
      return;
    }
  }
	CDialog::OnOK();
}



BOOL CRidership::PreTranslateMessage(MSG* pMsg) 
{
  if(pMsg->message==WM_KEYDOWN)
  {
    if(pMsg->wParam==VK_ESCAPE)
    {
      pMsg->wParam=NULL ;
    }
  } 
	
	return CDialog::PreTranslateMessage(pMsg);
}
