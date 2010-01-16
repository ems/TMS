//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddNode.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "tms.h"
#include "AddDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddNode dialog


CAddNode::CAddNode(CWnd* pParent /*=NULL*/, long* pUpdateRecordID)
	: CDialog(CAddNode::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;

	//{{AFX_DATA_INIT(CAddNode)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddNode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddNode)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddNode, CDialog)
	//{{AFX_MSG_MAP(CAddNode)
	ON_BN_CLICKED(ADDNODE_STOP, OnStop)
	ON_BN_CLICKED(ADDNODE_CC, OnCc)
	ON_BN_CLICKED(ADDNODE_NA, OnNa)
	ON_CBN_SELENDOK(ADDNODE_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ADDNODE_THISISARELIEFPOINT, OnThisisareliefpoint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddNode message handlers

BOOL CAddNode::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pEditABBRNAME = (CEdit *)GetDlgItem(ADDNODE_ABBRNAME);
  pEditLONGNAME = (CEdit *)GetDlgItem(ADDNODE_LONGNAME);
  pEditAVLSTOPNAME = (CEdit *)GetDlgItem(ADDNODE_AVLSTOPNAME);
  pEditINTERSECTION = (CEdit *)GetDlgItem(ADDNODE_INTERSECTION);
  pEditDESCRIPTION = (CEdit *)GetDlgItem(ADDNODE_DESCRIPTION);
  pEditCAPACITY = (CEdit *)GetDlgItem(ADDNODE_CAPACITY);
  pEditLONGITUDE = (CEdit *)GetDlgItem(ADDNODE_LONGITUDE);
  pEditLATITUDE = (CEdit *)GetDlgItem(ADDNODE_LATITUDE);
  pEditLABELS = (CEdit *)GetDlgItem(ADDNODE_LABELS);
  pButtonNA = (CButton *)GetDlgItem(ADDNODE_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDNODE_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDNODE_COMMENTCODE);
  pComboBoxJURISDICTION = (CComboBox *)GetDlgItem(ADDNODE_JURISDICTION);
  pEditNUMBER = (CEdit *)GetDlgItem(ADDNODE_NUMBER);
  pEditMAPCODES = (CEdit *)GetDlgItem(ADDNODE_MAPCODES);
  pButtonGARAGE = (CButton *)GetDlgItem(ADDNODE_GARAGE);
  pButtonSTOP = (CButton *)GetDlgItem(ADDNODE_STOP);
  pButtonSECURE = (CButton *)GetDlgItem(ADDNODE_SECURE);
  pButtonAVLTRANSFERPOINT = (CButton *)GetDlgItem(ADDNODE_AVLTRANSFERPOINT);
  pStaticLONGITUDE_TEXT = (CStatic *)GetDlgItem(ADDNODE_LONGITUDE_TEXT);
  pStaticLATITUDE_TEXT = (CStatic *)GetDlgItem(ADDNODE_LATITUDE_TEXT);
  pListBoxSTOPFLAGSLIST = (CListBox *)GetDlgItem(ADDNODE_STOPFLAGSLIST);
  pStaticSTOPFLAGS = (CStatic *)GetDlgItem(ADDNODE_STOPFLAGS);
  pStaticLABELS_TEXT = (CStatic *)GetDlgItem(ADDNODE_LABELS_TEXT);
  pButtonTHISISARELIEFPOINT = (CButton *)GetDlgItem(ADDNODE_THISISARELIEFPOINT);
  pStaticOBSTOPNUMBER_TEXT = (CStatic *)GetDlgItem(ADDNODE_OBSTOPNUMBER_TEXT);
  pEditOBSTOPNUMBER = (CEdit *)GetDlgItem(ADDNODE_OBSTOPNUMBER);
  pStaticIBSTOPNUMBER_TEXT = (CStatic *)GetDlgItem(ADDNODE_IBSTOPNUMBER_TEXT);
  pEditIBSTOPNUMBER = (CEdit *)GetDlgItem(ADDNODE_IBSTOPNUMBER);

  CString s;
  BOOL    bFound;
  int     nI, nJ;

//
//  Set up easting/northing if he's not in lat/long
//
  if(!m_bUseLatLong)
  {
    pStaticLONGITUDE_TEXT->SetWindowText("Easting:");
    pStaticLATITUDE_TEXT->SetWindowText("Northing:");
  }
//
//  Set up the jurisdictions combo box
//
  if(SetUpJurisdictionList(m_hWnd, ADDNODE_JURISDICTION) == 0)
  {
    pComboBoxJURISDICTION->EnableWindow(FALSE);
  }
//
//  Comment codes combo box
//
  if(SetUpCommentList(m_hWnd, ADDNODE_COMMENTCODE) == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Set up the stop flags
//
  if(m_numStopFlags == 0)
  {
    pStaticSTOPFLAGS->EnableWindow(FALSE);
    pListBoxSTOPFLAGSLIST->EnableWindow(FALSE);
  }
  else
  {
    for(nI = 0; nI < m_numStopFlags; nI++)
    {
      nJ = pListBoxSTOPFLAGSLIST->AddString(m_StopFlags[nI].szText);
      pListBoxSTOPFLAGSLIST->SetItemData(nJ, m_StopFlags[nI].number);
    }
  }
//
//  Are we updating?
//
//  No.
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonGARAGE->SetCheck(FALSE);
    pButtonNA->SetCheck(TRUE);
    pEditLONGITUDE->SetWindowText("0.0000");
    pEditLATITUDE->SetWindowText("0.0000");
    pEditNUMBER->SetWindowText("");
    pEditCAPACITY->SetWindowText("");
    pStaticLABELS_TEXT->EnableWindow(FALSE);
    pEditLABELS->EnableWindow(FALSE);
  }
//
//  Yes - updating
//
  else
  {
//
//  Get the record
//
    NODESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  abbrName
//
    s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
    pEditABBRNAME->SetWindowText(s);
//
//  longName
//
    s = GetDatabaseString(NODES.longName, NODES_LONGNAME_LENGTH);
    pEditLONGNAME->SetWindowText(s);
//
//  AVLStopName
//
    s = GetDatabaseString(NODES.AVLStopName, NODES_AVLSTOPNAME_LENGTH);
    pEditAVLSTOPNAME->SetWindowText(s);
//
//  intersection
//
    s = GetDatabaseString(NODES.intersection, NODES_INTERSECTION_LENGTH);
    pEditINTERSECTION->SetWindowText(s);
//
//  description
//
    s = GetDatabaseString(NODES.description, NODES_DESCRIPTION_LENGTH);
    pEditDESCRIPTION->SetWindowText(s);
//
//  Longitude
//
    sprintf(tempString, "%12.6f", NODES.longitude);
    pEditLONGITUDE->SetWindowText(tempString);
//
//  Latitude
//
    sprintf(tempString, "%12.6f", NODES.latitude);
    pEditLATITUDE->SetWindowText(tempString);
//
//  reliefLabels
//
    s = GetDatabaseString(NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
    if(s == "")
    {
      pButtonTHISISARELIEFPOINT->SetCheck(FALSE);
      pStaticLABELS_TEXT->EnableWindow(FALSE);
      pEditLABELS->EnableWindow(FALSE);
    }
    else
    {
      pButtonTHISISARELIEFPOINT->SetCheck(TRUE);
      pEditLABELS->SetWindowText(s);
    }

//
//  jurisdiction
//
    nI = (int)pComboBoxJURISDICTION->GetCount();
    for(nJ = 0; nJ < nI; nJ++)
    {
      if(NODES.JURISDICTIONSrecordID == (long)pComboBoxJURISDICTION->GetItemData(nJ))
      {
        pComboBoxJURISDICTION->SetCurSel(nJ);
        break;
      }
    }
//
//  number
//
    if(NODES.number > 0)
    {
      s.Format("%ld", NODES.number);
      pEditNUMBER->SetWindowText(s);
    }
    else
    {
      pEditNUMBER->SetWindowText("");
    }
//
//  OB stop number
//
    if(NODES.OBStopNumber > 0)
    {
      s.Format("%ld", NODES.OBStopNumber);
      pEditOBSTOPNUMBER->SetWindowText(s);
    }
    else
    {
      pEditOBSTOPNUMBER->SetWindowText("");
    }
//
//  IB stop number
//
    if(NODES.IBStopNumber > 0)
    {
      s.Format("%ld", NODES.IBStopNumber);
      pEditIBSTOPNUMBER->SetWindowText(s);
    }
    else
    {
      pEditIBSTOPNUMBER->SetWindowText("");
    }
//
//  capacity
//
    if(NODES.capacity > 1)
    {
      s.Format("%ld", NODES.capacity);
      pEditCAPACITY->SetWindowText(s);
    }
    else
    {
      pEditCAPACITY->SetWindowText("");
    }
//
//  mapCodes
//
    s = GetDatabaseString(NODES.mapCodes, NODES_MAPCODES_LENGTH);
    pEditMAPCODES->SetWindowText(s);
//
//  Comment
// 
    nI = (int)pComboBoxCOMMENTCODE->GetCount();
    for(bFound = FALSE, nJ = 0; nJ < nI; nJ++)
    {
      if(NODES.COMMENTSrecordID == (long)pComboBoxCOMMENTCODE->GetItemData(nJ))
      {
        pButtonCC->SetCheck(TRUE);
        pComboBoxCOMMENTCODE->SetCurSel(nJ);
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
    {
      pButtonNA->SetCheck(TRUE);
    }
//
//  flags
//
//  Timepoint flags
//
//  Garage
//
    m_bWasGarage = NODES.flags & NODES_FLAG_GARAGE;
    pButtonGARAGE->SetCheck(m_bWasGarage);
//
//  Stop only
//
    if(NODES.flags & NODES_FLAG_STOP)
    {
      pButtonSTOP->SetCheck(TRUE);
    }
    else
    {
      EnableStopFlags(FALSE);
    }
//
//  Secure location
//
    if(NODES.flags & NODES_FLAG_SECURE)
    {
      pButtonSECURE->SetCheck(TRUE);
    }
//
//  AVL Transfer Point
//
    if(NODES.flags & NODES_FLAG_AVLTRANSFERPOINT)
    {
      pButtonAVLTRANSFERPOINT->SetCheck(TRUE);
    }
//
//  Stop flags
//
//  If necessary, copy the timepoint flags over to stop flags first
//
    if(NODES.stopFlags == 0)
    {
      NODES.stopFlags = NODES.flags;
      if(NODES.stopFlags & NODES_FLAG_GARAGE)
      {
        NODES.stopFlags &= ~NODES_FLAG_GARAGE;
      }
      if(NODES.stopFlags & NODES_FLAG_STOP)
      {
        NODES.stopFlags &= ~NODES_FLAG_STOP;
      }
      if(NODES.stopFlags & NODES_FLAG_SECURE)
      {
        NODES.stopFlags &= ~NODES_FLAG_SECURE;
      }
    }

    int numEntries = pListBoxSTOPFLAGSLIST->GetCount();
    for(nI = 0; nI < NODES_MAXSTOPFLAGS; nI++)
    {
      if(NODES.stopFlags & (1 << nI))
      {
        for(nJ = 0; nJ < numEntries; nJ++)
        {
          if((int)pListBoxSTOPFLAGSLIST->GetItemData(nJ) == nI)
          {
            pListBoxSTOPFLAGSLIST->SetSel(nJ, TRUE);
          }
        }
      }
    }
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddNode::OnStop() 
{
  CString s;
  BOOL    bStop = pButtonSTOP->GetCheck();

  if(bStop && pButtonTHISISARELIEFPOINT->GetCheck())
  {
    s.LoadString(ERROR_345);
    MessageBeep(MB_ICONSTOP);
    MessageBox(s, TMS, MB_ICONSTOP | MB_OK);
    pButtonSTOP->SetCheck(FALSE);
  }
  else
  {
    EnableStopFlags(bStop);
  }
}

void CAddNode::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddNode::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}


void CAddNode::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}


void CAddNode::OnClose() 
{
  OnCancel();
}

void CAddNode::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddNode::OnHelp() 
{
}

void CAddNode::OnOK() 
{
  BLOCKSDef *pTRIPSChunk;
  CString   s;
  HCURSOR   hSaveCursor;
  long tempLong;
//  long coord;
  int  nI;
  int  nJ;
  int  nK;
  int  opCode;
  int  rcode2;
  int  keyNumber;
//
//  Data validation: the following fields can't be blank
//
//  abbrName
//
  pEditABBRNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_021, pEditABBRNAME->m_hWnd);
    return;
  }
//
//  longName
//
  pEditLONGNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_022, pEditLONGNAME->m_hWnd);
    return;
  }
//
//  intersection
//
  pEditINTERSECTION->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_023, pEditINTERSECTION->m_hWnd);
    return;
  }
//
//  description
//
  pEditDESCRIPTION->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_133, pEditDESCRIPTION->m_hWnd);
    return;
  }
//
//  Done validation.  Was there a change in garage?
//
  if(*m_pUpdateRecordID != NO_RECORD && m_bWasGarage && !pButtonGARAGE->GetCheck())
  {
    for(keyNumber = 2; keyNumber <= 3; keyNumber ++)
    {
      pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
      TRIPSKey2.assignedToNODESrecordID = updateRecordID;
      TRIPSKey2.RGRPROUTESrecordID = NO_RECORD;
      TRIPSKey2.SGRPSERVICESrecordID = NO_RECORD;
      TRIPSKey2.blockNumber = NO_RECORD;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
      if(rcode2 == 0 && pTRIPSChunk->assignedToNODESrecordID == updateRecordID)
      {
        s.LoadString(ERROR_064);
        MessageBeep(MB_ICONQUESTION);
        if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
        {
          hSaveCursor = ::SetCursor(hCursorWait);
          while(rcode2 == 0 && pTRIPSChunk->assignedToNODESrecordID == updateRecordID)
          {
            pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
            btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            TRIPSKey2.assignedToNODESrecordID = updateRecordID;
            TRIPSKey2.RGRPROUTESrecordID = NO_RECORD;
            TRIPSKey2.SGRPSERVICESrecordID = NO_RECORD;
            TRIPSKey2.blockNumber = NO_RECORD;
            TRIPSKey2.blockSequence = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
          }
          ::SetCursor(hSaveCursor);
        }
      }
    }
  }
//
//  Insert or update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_NODES, &NODES, &NODESKey0, 0);
    NODES.recordID = AssignRecID(rcode2, NODES.recordID);
    opCode = B_INSERT;
  }
  else
  {
    NODESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    opCode = B_UPDATE;
  }
//
//  abbrName
//
  pEditABBRNAME->GetWindowText(s);
  PutDatabaseString(NODES.abbrName, s, NODES_ABBRNAME_LENGTH);
//
//  longName
//
  pEditLONGNAME->GetWindowText(s);
  PutDatabaseString(NODES.longName, s, NODES_LONGNAME_LENGTH);
//
//  AVLStopName
//
  pEditAVLSTOPNAME->GetWindowText(s);
  PutDatabaseString(NODES.AVLStopName, s, NODES_AVLSTOPNAME_LENGTH);
//
//  intersection
//
  pEditINTERSECTION->GetWindowText(s);
  PutDatabaseString(NODES.intersection, s, NODES_INTERSECTION_LENGTH);
//
//  description
//
  pEditDESCRIPTION->GetWindowText(s);
  PutDatabaseString(NODES.description, s, NODES_DESCRIPTION_LENGTH);
//
//  Longitude and Latitude
//
  pEditLONGITUDE->GetWindowText(s);
  NODES.longitude = (float)atof(s);

  pEditLATITUDE->GetWindowText(s);
  NODES.latitude = (float)atof(s);
//
//  reliefLabels
//
  pEditLABELS->GetWindowText(s);
  PutDatabaseString(NODES.reliefLabels, s, NODES_RELIEFLABELS_LENGTH);
//
//  jurisdiction
//
  nI = (int)pComboBoxJURISDICTION->GetCurSel();
  NODES.JURISDICTIONSrecordID = (nI == CB_ERR ? NO_RECORD : (int)pComboBoxJURISDICTION->GetItemData(nI));
//
//  number
//
  pEditNUMBER->GetWindowText(s);
  tempLong = atol(s);
  NODES.number = (tempLong > 0 ? tempLong : NO_RECORD);
//
//  OB stop number
//
  pEditOBSTOPNUMBER->GetWindowText(s);
  tempLong = atol(s);
  NODES.OBStopNumber = (tempLong > 0 ? tempLong : NO_RECORD);
//
//  IB stop number
//
  pEditIBSTOPNUMBER->GetWindowText(s);
  tempLong = atol(s);
  NODES.IBStopNumber = (tempLong > 0 ? tempLong : NO_RECORD);
//
//  capacity
//
  pEditCAPACITY->GetWindowText(s);
  tempLong = atol(s);
  if(tempLong > 1)
  {
    NODES.capacity = tempLong;
  }
  else
  {
    NODES.capacity = NO_RECORD;
  }
//
//  mapCodes
//
  pEditMAPCODES->GetWindowText(s);
  PutDatabaseString(NODES.mapCodes, s, NODES_MAPCODES_LENGTH);
//
//  Comment code
//
  if(pButtonNA->GetCheck())
  {
    NODES.COMMENTSrecordID = NO_RECORD;
  }
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    NODES.COMMENTSrecordID =
         nI == CB_ERR ? NO_RECORD : (int)pComboBoxCOMMENTCODE->GetItemData(nI);
  }
//
//  Timepoint flags
//
  NODES.flags = 0;
  if(pButtonGARAGE->GetCheck())
  {
    NODES.flags |= NODES_FLAG_GARAGE;
  }
  if(pButtonSTOP->GetCheck())
  {
    NODES.flags |= NODES_FLAG_STOP;
  }
  if(pButtonSECURE->GetCheck())
  {
    NODES.flags |= NODES_FLAG_SECURE;
  }
  if(pButtonAVLTRANSFERPOINT->GetCheck())
  {
    NODES.flags |= NODES_FLAG_AVLTRANSFERPOINT;
  }
//
//  Stop flags
//
  NODES.stopFlags = 0;
  nI = (int)pListBoxSTOPFLAGSLIST->GetCount();
  for(nJ = 0; nJ < nI; nJ++)
  {
    if(pListBoxSTOPFLAGSLIST->GetSel(nJ))
    {
      nK = (int)pListBoxSTOPFLAGSLIST->GetItemData(nJ);
      NODES.stopFlags |= (1 << nK);
    }
  }
//
//  Reserved
//
  memset(NODES.reserved, 0x00, NODES_RESERVED_LENGTH);
//
//  Insert / Update
//
  rcode2 = btrieve(opCode, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Just 'cause we're nice, let him know if he tried to add a duplicate node
//
  if(rcode2 == 5)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_168, pEditABBRNAME->m_hWnd);
    return;
  }
  *m_pUpdateRecordID = NODES.recordID;
//
//  Refresh the garage list
//
  numGaragesInGarageList = 0;
  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0 && (NODES.flags & NODES_FLAG_GARAGE))
  {
    garageList[numGaragesInGarageList++] = NODES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }
//
//  Start temporary data fix
//
//  Swap values in "Description" and "Geocoding Address"
//
//  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
//  while(rcode2 == 0)
//  {
//    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
//    strncpy(NODES.intersection, NODES.description, NODES_INTERSECTION_LENGTH);
//    strncpy(NODES.description, tempString, NODES_INTERSECTION_LENGTH);
//    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
//    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
//  }
//
//  End Temporary data fix
//
//
//  Start temporary data fix
//
//  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
//  while(rcode2 == 0)
//  {
//    NODES.latitude /= 10.0;
//    NODES.longitude /= 10.0;
//    rcode2 = btrieve(B_UPDATE, TMS_NODES, &NODES, &NODESKey0, 0);
//    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
//  }
//
//  End Temporary data fix
//

	
  CDialog::OnOK();
}

void CAddNode::EnableStopFlags(BOOL bStop)
{
  pStaticSTOPFLAGS->EnableWindow(bStop);
  pListBoxSTOPFLAGSLIST->EnableWindow(bStop);
  if(bStop)
  {
    pStaticOBSTOPNUMBER_TEXT->EnableWindow(FALSE);
    pEditOBSTOPNUMBER->EnableWindow(FALSE);
    pEditOBSTOPNUMBER->SetWindowText("");
    pStaticIBSTOPNUMBER_TEXT->EnableWindow(FALSE);
    pEditIBSTOPNUMBER->EnableWindow(FALSE);
    pEditIBSTOPNUMBER->SetWindowText("");
  }
  else
  {
    pStaticOBSTOPNUMBER_TEXT->EnableWindow(TRUE);
    pEditOBSTOPNUMBER->EnableWindow(TRUE);
    pStaticIBSTOPNUMBER_TEXT->EnableWindow(TRUE);
    pEditIBSTOPNUMBER->EnableWindow(TRUE);
    pListBoxSTOPFLAGSLIST->SetSel(-1, FALSE);
  }
}

void CAddNode::OnThisisareliefpoint() 
{
  CString s;
  if(pButtonTHISISARELIEFPOINT->GetCheck())
  {
    if(pButtonSTOP->GetCheck())
    {
      s.LoadString(ERROR_346);
      MessageBeep(MB_ICONSTOP);
      MessageBox(s, TMS, MB_ICONSTOP | MB_OK);
      pButtonTHISISARELIEFPOINT->SetCheck(FALSE);
    }
    else
    {
      pStaticLABELS_TEXT->EnableWindow(TRUE);
      pEditLABELS->EnableWindow(TRUE);
    }
  }
  else
  {
    pStaticLABELS_TEXT->EnableWindow(FALSE);
    pEditLABELS->SetWindowText("");
    pEditLABELS->EnableWindow(FALSE);
  }
}
