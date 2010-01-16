// Fares.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "sdigrid.h"
#include "Fares.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFares dialog


CFares::CFares(CWnd* pParent /*=NULL*/)
	: CDialog(CFares::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFares)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFares::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFares)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFares, CDialog)
	//{{AFX_MSG_MAP(CFares)
	ON_BN_CLICKED(IDESTABLISH, OnEstablish)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDPOPULATE, OnPopulate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFares message handlers

BOOL CFares::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pSDIGridTYPES = (CSDIGrid *)GetDlgItem(FARES_TYPES);
  pSDIGridFARES = (CSDIGrid *)GetDlgItem(FARES_FARES);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFares::OnClose() 
{
  OnCancel();
}

void CFares::OnCancel() 
{
	CDialog::OnCancel();
}

void CFares::OnHelp() 
{
}

void CFares::OnEstablish() 
{
  CString s;
  short  rowNumber = 1;
  int    nI;
  int    rcode2;

  pSDIGridFARES->ClearGrid();
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    for(nI = 0; nI < RIDERSHIP_MAXFARETYPES; nI++)
    {
      s = pSDIGridTYPES->GetCellContents(nI + 1, 1);
      if(s == "")
      {
        break;
      }
      pSDIGridFARES->SetCellContents(rowNumber, 1, tempString);
      pSDIGridFARES->SetCellContents(rowNumber, 2, s);
      rowNumber++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
}

void CFares::OnOK() 
{
//
//  Clear the internal list of fare types
//
  int nI, nJ;

  for(nI = 0; nI < RIDERSHIP_MAXFARETYPES; nI++)
  {
    strcpy(m_szFareTypes[nI], "");
  }
//
//  Get the list of fare types from the grid
//
  CString s;

  m_numFareTypes = 0;
  for(nI = 0; nI < RIDERSHIP_MAXFARETYPES; nI++)
  {
    s = pSDIGridTYPES->GetCellContents(nI + 1, 1);
    if(s == "")
    {
      break;
    }
    strcpy(m_szFareTypes[m_numFareTypes], s);
    m_numFareTypes++;
  }
//
//  Get the fares by route information (500 is wired into the grid)
//
  BOOL bFound;
  int  typeIndex;
  int  rcode2;

//
//  Clear out the fare structure
//
  for(nI = 0; nI < MAXFARETYPEROUTES; nI++)
  {
    m_FareTypesByRoute[nI].ROUTESrecordID = NO_RECORD;
    for(nJ = 0; nJ < RIDERSHIP_MAXFARETYPES; nJ++)
    {
      m_FareTypesByRoute[nI].fareValue[nJ] = 0.0;
    }
  }
  m_numFareTypesByRoute = 0;
//
//  Read the grid
//
  for(nI = 0; nI < 500; nI++)
  {
//
//  Blank in the first column means we're done
//
    s = pSDIGridFARES->GetCellContents(nI + 1, 1);
    if(s == "")
    {
      break;
    }
//
//  Validate the fare type - "continue" if it's not valid
//
    s = pSDIGridFARES->GetCellContents(nI + 1, 2);
    for(bFound = FALSE, nJ = 0; nJ < m_numFareTypes; nJ++)
    {
      if(strcmp(m_szFareTypes[nJ], s) == 0)
      {
        bFound = TRUE;
        typeIndex = nJ;
        break;
      }
    }
    if(!bFound)
    {
      continue;
    }
//
//  Validate the route number - "continue" if it's not valid
//
    s = pSDIGridFARES->GetCellContents(nI + 1, 1);
    strcpy(tempString, s);
    pad(tempString, ROUTES_NUMBER_LENGTH);
    strncpy(ROUTESKey1.number, tempString, ROUTES_NUMBER_LENGTH);
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    if(rcode2 != 0)
    {
      continue;
    }
    for(bFound = FALSE, nJ = 0; nJ < m_numFareTypesByRoute; nJ++)
    {
      if(m_FareTypesByRoute[nJ].ROUTESrecordID == ROUTES.recordID)
      {
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
    {
      nJ = m_numFareTypesByRoute;
      m_FareTypesByRoute[nJ].ROUTESrecordID = ROUTES.recordID;
      m_numFareTypesByRoute++;
    }
    s = pSDIGridFARES->GetCellContents(nI + 1, 3);
    m_FareTypesByRoute[nJ].fareValue[typeIndex] = (float)atof(s);
  }
//
//  Got 'em - write out the data to the Audit Trail
//
//  Invalidate the previous fare entries
//
  CTime today = CTime::GetCurrentTime();
  long  dateToUse = today.GetYear() * 10000 + today.GetMonth() * 100 + today.GetDay();
  long DAILYOPSrecordID;
  BOOL recordFlag;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_RIDERSHIP;
  DAILYOPSKey1.pertainsToDate = 0;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_RIDERSHIP) &&
         DAILYOPS.pertainsToDate <= dateToUse)
  {
    if(dateToUse >= DAILYOPS.pertainsToDate &&
          ((DAILYOPS.recordFlags & DAILYOPS_FLAG_SETFARETYPE) ||
           (DAILYOPS.recordFlags & DAILYOPS_FLAG_SETFARE)))
    {
      recordFlag = ((DAILYOPS.recordFlags & DAILYOPS_FLAG_SETFARETYPE) ? DAILYOPS_FLAG_SETFARETYPE : DAILYOPS_FLAG_SETFARE);
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
        DAILYOPS.entryDateAndTime = today.GetTime();
        DAILYOPS.pertainsToDate = 0;
        DAILYOPS.pertainsToTime = NO_TIME;
        DAILYOPS.userID = m_UserID;
        DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
        if(recordFlag & DAILYOPS_FLAG_SETFARETYPE)
        {
          DAILYOPS.recordFlags = DAILYOPS_FLAG_CLEARFARETYPE;
        }
        else
        {
          DAILYOPS.recordFlags = DAILYOPS_FLAG_CLEARFARE;
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
//  Tell the initial record about the unregister
//
      if(!bFound)
      {
        DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
        rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Cycle through and create the new entries
//
//  Fare types
//
  for(nI = 0; nI < m_numFareTypes; nI++)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = today.GetTime();
    DAILYOPS.pertainsToDate = dateToUse;
    DAILYOPS.pertainsToTime = NO_TIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_SETFARETYPE;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    DAILYOPS.DOPS.Ridership.data.FT.index = nI;
    strcpy(DAILYOPS.DOPS.Ridership.data.FT.FareType, m_szFareTypes[nI]);
    pad(DAILYOPS.DOPS.Ridership.data.FT.FareType, RIDERSHIP_FARETYPE_LENGTH);
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Fare types by route
//
  for(nI = 0; nI < m_numFareTypesByRoute; nI++)
  {
    for(nJ = 0; nJ < m_numFareTypes; nJ++)
    {
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
      DAILYOPS.entryDateAndTime = today.GetTime();
      DAILYOPS.pertainsToDate = dateToUse;
      DAILYOPS.pertainsToTime = NO_TIME;
      DAILYOPS.userID = m_UserID;
      DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_RIDERSHIP;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_SETFARE;
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
      DAILYOPS.DRIVERSrecordID = NO_RECORD;
      memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
      DAILYOPS.DOPS.Ridership.data.F.ROUTESrecordID = m_FareTypesByRoute[nI].ROUTESrecordID;
      DAILYOPS.DOPS.Ridership.data.F.index = nJ;
      DAILYOPS.DOPS.Ridership.data.F.value = m_FareTypesByRoute[nI].fareValue[nJ];
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
    }
  }
//
//  All done
//
	CDialog::OnOK();
}

void CFares::OnPopulate() 
{
//
//  Display what we've got
//
  short rowNumber;
  int   nI, nJ;
  int   rcode2;
//
//  Fare types by route
//
  pSDIGridFARES->ClearGrid();
  rowNumber = 1;
  for(nI = 0; nI < m_numFareTypesByRoute; nI++)
  {
    ROUTESKey0.recordID = m_FareTypesByRoute[nI].ROUTESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 == 0)
    {
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      for(nJ = 0; nJ < m_numFareTypes; nJ++)
      {
        pSDIGridFARES->SetCellContents(rowNumber, 1, tempString);
        pSDIGridFARES->SetCellContents(rowNumber, 2, m_szFareTypes[nJ]);
        sprintf(szarString, "%5.2f", m_FareTypesByRoute[nI].fareValue[nJ]);
        pSDIGridFARES->SetCellContents(rowNumber, 3, szarString);
        rowNumber++;
      }
    }
  }
//
//  Fare types
//
  pSDIGridTYPES->ClearGrid();
  rowNumber = 1;
  for(nI = 0; nI < m_numFareTypes; nI++)
  {
    pSDIGridTYPES->SetCellContents(rowNumber, 1, m_szFareTypes[nI]);
    rowNumber++;
  }

}
