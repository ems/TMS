//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include <math.h>
#include "TMS.h"

#include "MainFrm.h"

#define MAXGARAGES 30
#define IDC_VCF1CTRL 1000

void CTMSApp::GetTMSData(TableDisplayDef* pTD, long flags)
{
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  AllFilesDef ALLFILES;
  AllKey0Def  ALLKey0;
  AllKey1Def  ALLKey1;
  AllKey2Def  ALLKey2;
  AllKey3Def  ALLKey3;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HCURSOR hSaveCursor;
  float   distance;
  BOOL    bBuild;
  BOOL    bClear;
  BOOL    bFirst;
  BOOL    bFound;
  BOOL    bOnceThrough;
  BOOL    bOK;
  BOOL    bSetRow1;
  char    outString[512];
  long    assignedToNODESrecordID;
  long    deadheadTime;
  long    garageRecordIDs[MAXGARAGES];
  long    lastNodeRecordID;
  long    lastNodeTime;
  long    lastRouteRecordID;
  long    lastServiceRecordID;
  long    PIDeadheadTime;
  long    PODeadheadTime;
  long    POGNODESrecordID;
  long    recordID;
  long    width;
  long    dhd;
  int     currentGarage;
  int     databaseFile;
  int     fileNumber;
  int     keyNumber;
  int     lineID;
  int     nI;
  int     numGarages;
  int     numPieces;
  int     pieceNumber;
  int     rcode2;
  int     row;
  BLOCKSDef *pALLFILESTRIPSChunk;
//  
//  Column width stuff
//
  char szarWidths[MAXFIELDS * 6];
  int  ch;
  int  colWidth;
//
//  Font stuff
//
  COLORREF crColor;
  BOOL bBold;
  BOOL bItalic;
  BOOL bUnderline;
  BOOL bStrikeout;
  BOOL bOutline;
  BOOL bShadow;
  char szFontName[64];
  short int nFontSize;
//
//  Set the flags
//
  bBuild = flags & GETTMSDATA_FLAG_BUILD;
  bClear = flags & GETTMSDATA_FLAG_CLEAR;
  bSetRow1 = flags & GETTMSDATA_FLAG_SETROW1;
//
//  If he's managed to close the target window, just return
//
  if(pTD->pane == NULL)
  {
    return;
  }
//
//  Get the pointers
//
  CWnd* parent = pTD->pane;
  CF1Book* F1 = &pTD->F1;
//
//  and the sizes
//
  RECT rect;
  parent->GetClientRect(&rect);
//
//  Set up the data fields
//
  hSaveCursor = SetCursor(hCursorWait);
  if(bBuild)
  {
    EstablishDataFields(pTD);
//
//  Create the grid
//
//  The following piece of code was used to determine the license key for the
//  formula one grid.  It (the code) was left here in case the pinheads at
//  Visual Componenets change the grid enough to warrant a new license key.
//
//    CLSID   clsid;
//    CString m_strLicense;
//    LPVOID  pClassFactory;
//
//    if(SUCCEEDED(CLSIDFromProgID(OLESTR("VCF15.VCF1.5"), &clsid)))
//    {
//      if(SUCCEEDED(CoGetClassObject(clsid, CLSCTX_INPROC_SERVER, NULL,
//            IID_IClassFactory2, &pClassFactory)))
//      {
//        IClassFactory2* pClassFactory2 = (IClassFactory2*)pClassFactory;
//        BSTR bstrLic;
//        bstrLic = 0;
//        if(SUCCEEDED(pClassFactory2->RequestLicKey(0, &bstrLic)))
//        {
//          m_strLicense = bstrLic;
//          SysFreeString(bstrLic);
//        }
//        else
//        {
//          m_strLicense = _T("");
//        }
//        pClassFactory2->Release();
//      }
//      else
//      {
//        m_strLicense = _T("");
//      }
//      AfxMessageBox(m_strLicense);
//    }
//
    if(F1->m_hWnd == NULL)
    {
                                            // the vaule of m_strLicense goes below  
      BSTR LicenseKey = SysAllocString(OLESTR("6D5ECD55-86B8-11d0-95AD-0020AF1E9EB2"));  
//
//  Make the grid fit properly
//
      if(!F1->Create(NULL, WS_MAXIMIZE, rect, parent, IDC_VCF1CTRL, NULL, FALSE, LicenseKey))
      {
        MessageBeep(MB_ICONSTOP);
        parent->MessageBox("Create Failed", TMS, MB_OK | MB_ICONSTOP);
        SysFreeString(LicenseKey);
        return;
      }
      SysFreeString(LicenseKey);
    }
  }
//
//  Optionally clear the grid
//
  if(bClear)
  {
    parent->InvalidateRect(NULL, TRUE);
    F1->DeleteRange(-1, -1, -1, -1, F1ShiftVertical);
    F1->SetMaxRow(F1_MAXROW);
  }
//
//  Optionally position the grid to row 1
//
  if(bSetRow1)
  {
    F1->SetTopRow(1);
  }
//
//  Set the column widths
//
  for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
  {
    F1->SetColText(nI + 1, pTD->fieldInfo.columnTitles[nI]);
    width = max((long)strlen(pTD->fieldInfo.columnTitles[nI]) + 2, pTD->fieldInfo.numChars[nI]);
    width *= 256;
    F1->SetColWidth(nI + 1, nI + 1, (int)width, FALSE);
  }
  F1->SetMaxCol((long)pTD->fieldInfo.numFields);
  fileNumber = pTD->fileInfo.fileNumber;
  keyNumber = pTD->fileInfo.keyNumber;
//
//  Is there an entry for nodeWidth?
//
//  Resize the columns if there are saved widths
//
  sprintf(tempString, "Width%d", pTD->fileInfo.fileNumber);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)tempString,
        "", (LPSTR)szarWidths, sizeof(szarWidths), (LPSTR)szDatabaseFileName);
  for(ch = 0, nI = 0; nI < pTD->fieldInfo.numFields && ch < lstrlen(szarWidths); nI++)
  {
    if(pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNCHECKS + DATATYPE_BIT ||
          pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNTIMES + DATATYPE_BIT)
      colWidth = nodeDisplayWidth;
    else
    {
      sscanf(szarWidths + ch, " %d%n", &colWidth, &width);
      ch += width;
    }
    if(pTD->fieldInfo.flags[nI] & ATTRIBUTES_FLAG_HIDDEN)
    {
      F1->SetColWidth(nI + 1, nI + 1, 0, FALSE);
    }
    else
    {
      if(colWidth != 0)
        F1->SetColWidth(nI + 1, nI + 1, colWidth, FALSE);
    }
  }
//
//  Hide any hidden columns if there was no "Width" entry
//
  if(strcmp(szarWidths, "") == 0)
  {
    for(nI = pTD->fieldInfo.numFields; nI > 0; nI--)
    {
      if(pTD->fieldInfo.flags[nI - 1] & ATTRIBUTES_FLAG_HIDDEN)
      {
        F1->SetColWidth(nI, nI, 0, FALSE);
      }
    }
  }
//
//  Set the defualt font for the grid
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"Font",
        "", (LPSTR)szFontName, sizeof(szFontName), (LPSTR)szDatabaseFileName);
//
//  If there's an entry in the .TMS file, use it.  Also get the current font
//  information so we have a value for crColor, bOutline, and bShadow, as these
//  values are not recorded.
//
  if(strcmp(szFontName, "") != 0)
  {
    CString s = szFontName;
    BSTR bs = s.AllocSysString();
    F1->SetSelection(-1, -1, -1, -1);
    F1->GetFont(&bs, &nFontSize, &bBold, &bItalic, &bUnderline, &bStrikeout, (long *)&crColor, &bOutline, &bShadow);
    nFontSize = (int)GetPrivateProfileInt((LPSTR)userName, "FontSize", 10, (LPSTR)szDatabaseFileName);
    bBold = (BOOL)GetPrivateProfileInt((LPSTR)userName, "Bold", FALSE, (LPSTR)szDatabaseFileName);
    bItalic = (BOOL)GetPrivateProfileInt((LPSTR)userName, "Italic", FALSE, (LPSTR)szDatabaseFileName);
    bUnderline = (BOOL)GetPrivateProfileInt((LPSTR)userName, "Underline", FALSE, (LPSTR)szDatabaseFileName);
    bStrikeout = (BOOL)GetPrivateProfileInt((LPSTR)userName, "Strikeout", FALSE, (LPSTR)szDatabaseFileName);
    F1->SetFont(szFontName, -nFontSize,
          bBold, bItalic, bUnderline, bStrikeout, crColor, bOutline, bShadow);
    F1->SetSelection(1, 1, 1, 1);
  }
//
//  Position the file
//
//  Patterns
//
  if(fileNumber == TMS_PATTERNS)
  {
    ROUTESKey0.recordID = pTD->fileInfo.routeRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    ALLKey2.PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
    ALLKey2.PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    ALLKey2.PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
    ALLKey2.PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
    ALLKey2.PATTERNSKey2.nodeSequence = 0;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &ALLFILES, &ALLKey2, 2);
    if(rcode2 == 0 &&
          (ALLFILES.PATTERNS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
           ALLFILES.PATTERNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
           ALLFILES.PATTERNS.directionIndex != pTD->fileInfo.directionIndex ||
           ALLFILES.PATTERNS.PATTERNNAMESrecordID != basePatternRecordID))
      rcode2 = 101;
  }
//
//  Connections
//
  else if(fileNumber == TMS_CONNECTIONS)
  {
    if(CONN.numConnections == 0)
      rcode2 = 101;
    else
    {
      CONN.currentConnection = 0;
      ALLKey0.CONNECTIONSKey0.recordID = CONN.startRecordID[(int)CONN.currentConnection];
      rcode2 = btrieve(B_GETEQUAL, TMS_CONNECTIONS, &ALLFILES, &ALLKey0, 0);
      if(rcode2 == 0)
      {
        btrieve(B_GETPOSITION, TMS_CONNECTIONS, &ALLFILES, &ALLKey0, 0);
        btrieve(B_GETDIRECT, TMS_CONNECTIONS, &ALLFILES, &ALLKey1, 1);
        CONN.currentFromNODESrecordID = ALLFILES.CONNECTIONS.fromNODESrecordID;
        CONN.currentToNODESrecordID = ALLFILES.CONNECTIONS.toNODESrecordID;
      }
    }
  }
//
//  Comments - get the first record and the first
//  COMMENTS_TEXTDISPLAY_LENGTH bytes and jimmy
//  the data into ALLFILES
//
  else if(fileNumber == TMS_COMMENTS)
  {
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETFIRST, TMS_COMMENTS, pCommentText, &ALLKey1, 1);
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
    memcpy(&ALLFILES.COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
  }
//
//  Trips
//
  else if(fileNumber == TMS_TRIPS)
  {
    ROUTESKey0.recordID = pTD->fileInfo.routeRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    ALLKey1.TRIPSKey1.ROUTESrecordID = pTD->fileInfo.routeRecordID;
    ALLKey1.TRIPSKey1.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    ALLKey1.TRIPSKey1.directionIndex = pTD->fileInfo.directionIndex;
    ALLKey1.TRIPSKey1.tripSequence = (NO_TIME * 1000);  // In case he's shifted trips into negative oblivion
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &ALLFILES, &ALLKey1, 1);
    pTD->fileInfo.keyNumber = 1;
    if(rcode2 == 0 &&
         (ALLFILES.TRIPS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
          ALLFILES.TRIPS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
          ALLFILES.TRIPS.directionIndex != pTD->fileInfo.directionIndex))
      rcode2 = 101;
  }
//
//  Standard and Dropback Blocks
//
  else if(fileNumber == TMS_STANDARDBLOCKS || fileNumber == TMS_DROPBACKBLOCKS)
  {
    if(fileNumber == TMS_STANDARDBLOCKS)
    {
      keyNumber = 2;
      pALLFILESTRIPSChunk = &ALLFILES.TRIPS.standard;
    }
    else
    {
      keyNumber = 3;
      pALLFILESTRIPSChunk = &ALLFILES.TRIPS.dropback;
    }
    currentGarage = 0;
    numGarages = 0;
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
    while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
    {
      garageRecordIDs[numGarages++] = NODES.recordID;
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }
    if(numGarages == 0)
    {
      TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
	  numGarages = 1;
    }
    bFound = FALSE;
    bFirst = TRUE;
    while(currentGarage < numGarages)
    {
      assignedToNODESrecordID = bFirst ? NO_RECORD : garageRecordIDs[currentGarage];
      ALLKey2.TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
      ALLKey2.TRIPSKey2.RGRPROUTESrecordID = pTD->fileInfo.routeRecordID;
      ALLKey2.TRIPSKey2.SGRPSERVICESrecordID = pTD->fileInfo.serviceRecordID;
      ALLKey2.TRIPSKey2.blockNumber = 0;
      ALLKey2.TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &ALLFILES, &ALLKey2, keyNumber);
      if(rcode2 == 0 &&
            pALLFILESTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
            pALLFILESTRIPSChunk->RGRPROUTESrecordID == pTD->fileInfo.routeRecordID &&
            pALLFILESTRIPSChunk->SGRPSERVICESrecordID == pTD->fileInfo.serviceRecordID)
      {
        bFound = TRUE;
        break;
      }
      else
      {
        if(bFirst)
          bFirst = FALSE;
        else
          currentGarage++;
      }
    }
    if(!bFound)
      rcode2 = 101;
  }
//
//  Runs
//
  else if(fileNumber == TMS_RUNS)
  {
    ALLKey1.RUNSKey1.DIVISIONSrecordID = pTD->fileInfo.divisionRecordID;
    ALLKey1.RUNSKey1.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    ALLKey1.RUNSKey1.runNumber = NO_RECORD;
    ALLKey1.RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &ALLFILES, &ALLKey1, 1);
    if(rcode2 == 0 && 
          (ALLFILES.RUNS.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID ||
                ALLFILES.RUNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID))
      rcode2 = 101;
  }
//
//  "Crew Only" Runs
//
  else if(fileNumber == TMS_CREWONLY)
  {
    ALLKey1.CREWONLYKey1.DIVISIONSrecordID = pTD->fileInfo.divisionRecordID;
    ALLKey1.CREWONLYKey1.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    ALLKey1.CREWONLYKey1.runNumber = NO_RECORD;
    ALLKey1.CREWONLYKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &ALLFILES, &ALLKey1, 1);
    if(rcode2 == 0 && 
          (ALLFILES.CREWONLY.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID ||
                ALLFILES.CREWONLY.SERVICESrecordID != pTD->fileInfo.serviceRecordID))
      rcode2 = 101;
  }
//
//  Roster
//
  else if(fileNumber == TMS_ROSTER)
  {
    ALLKey1.ROSTERKey1.DIVISIONSrecordID = pTD->fileInfo.divisionRecordID;
    ALLKey1.ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ALLFILES, &ALLKey1, 1);
    if(rcode2 == 0 && ALLFILES.ROSTER.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID)
      rcode2 = 101;
//    rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ALLFILES, &ALLKey1, 1);
  }
//
//  The rest
//
  else
  {
    rcode2 = btrieve(B_GETFIRST, fileNumber, &ALLFILES, &ALLKey1, 1);
  }
//
//  Do we have anything?
//
  lineID = 0;
  if((pTD->fileInfo.position = rcode2 == 0 ? ALLFILES.DATABASE.recordID : NO_POSITION) == NO_POSITION)
    goto attrib;
//
//  Fill the grid
//
  if(fileNumber == TMS_STANDARDBLOCKS || fileNumber == TMS_DROPBACKBLOCKS)
    databaseFile = TMS_TRIPS;
  else
    databaseFile = fileNumber;
  ALLKey0.DATABASEKey0.recordID = pTD->fileInfo.position;
  rcode2 = btrieve(B_GETEQUAL, databaseFile, &ALLFILES, &ALLKey0, 0);
  if(pTD->fileInfo.keyNumber != 0)
  {
    rcode2 = btrieve(B_GETPOSITION, databaseFile, &ALLFILES, &ALLKey0, 0);
    switch(pTD->fileInfo.keyNumber)
    {
    case 1:
      if(databaseFile == TMS_COMMENTS)
      {
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        rcode2 = btrieve(B_GETFIRST, TMS_COMMENTS, pCommentText, &ALLKey1, 1);
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
        if(rcode2 == 0)
        {
          memcpy(&ALLFILES, pCommentText, COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH);
          memset(&ALLFILES.hackedCommentText[COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH], 0x00, 1);
        }
      }
      else
        rcode2 = btrieve(B_GETDIRECT, databaseFile, &ALLFILES, &ALLKey1, 1);
      break;
    case 2:
      rcode2 = btrieve(B_GETDIRECT, databaseFile, &ALLFILES, &ALLKey2, 2);
      break;
    case 3:
      rcode2 = btrieve(B_GETDIRECT, databaseFile, &ALLFILES, &ALLKey3, 3);
      break;
    }
  }
//
//  Special case for comments, due to its ridiculous length
//
  if(databaseFile == TMS_COMMENTS && rcode2 == 22)  // 22=Not enough data buffer size
    rcode2 = 0;
//
//  Connections
//
  if(fileNumber == TMS_CONNECTIONS)
  {
    CONNECTIONSKey1.fromNODESrecordID = ALLFILES.CONNECTIONS.fromNODESrecordID;
    CONNECTIONSKey1.toNODESrecordID = ALLFILES.CONNECTIONS.toNODESrecordID;
    CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
    if(rcode2 == 0)
    {
      for(nI = 0; nI < CONN.numConnections; nI++)
      {
        if(CONNECTIONS.recordID == CONN.startRecordID[nI])
        {
          CONN.currentConnection = nI;
          CONN.currentFromNODESrecordID = ALLFILES.CONNECTIONS.fromNODESrecordID;
          CONN.currentToNODESrecordID = ALLFILES.CONNECTIONS.toNODESrecordID;
        }
      }
    }
  }
//
//  Cycle through the data
//
  PREVBLOCKSVIEW.blockNumber = NO_RECORD;
  pieceNumber = 0;
  while(rcode2 == 0)
  {
//
//  Set up data view structure if on Blocks, Runs, or Roster
//
//  Blocks
//
    if(fileNumber == TMS_STANDARDBLOCKS || fileNumber == TMS_DROPBACKBLOCKS)
    {
      pALLFILESTRIPSChunk = keyNumber == 2 ? &ALLFILES.TRIPS.standard : &ALLFILES.TRIPS.dropback;
      BLOCKSVIEW.assignedToNODESrecordID = assignedToNODESrecordID;
      BLOCKSVIEW.blockNumber = pALLFILESTRIPSChunk->blockNumber;
      GenerateTrip(ALLFILES.TRIPS.ROUTESrecordID, ALLFILES.TRIPS.SERVICESrecordID,
            ALLFILES.TRIPS.directionIndex, ALLFILES.TRIPS.PATTERNNAMESrecordID,
            ALLFILES.TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  If we're displaying a dropback, and the standard block pulls out or in,
//  then replace the start/end node/time with the pullout/in gar/time.
//
//  Check pullouts
//
      if(fileNumber == TMS_STANDARDBLOCKS ||
            ALLFILES.TRIPS.standard.POGNODESrecordID == NO_RECORD)
      {
        BLOCKSVIEW.firstNodeRecordID = GTResults.firstNODESrecordID;
        BLOCKSVIEW.firstNodeTime = GTResults.firstNodeTime;
      }
      else
      {
        GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
        GCTData.toPATTERNNAMESrecordID = ALLFILES.TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = ALLFILES.TRIPS.ROUTESrecordID;
        GCTData.fromSERVICESrecordID = ALLFILES.TRIPS.SERVICESrecordID;
        GCTData.toROUTESrecordID = ALLFILES.TRIPS.ROUTESrecordID;
        GCTData.toSERVICESrecordID = ALLFILES.TRIPS.SERVICESrecordID;
        GCTData.fromNODESrecordID = ALLFILES.TRIPS.standard.POGNODESrecordID;
        GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
        GCTData.timeOfDay = GTResults.firstNodeTime;
        dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        if(dhd == NO_TIME)
          dhd = 0;
        BLOCKSVIEW.firstNodeRecordID = ALLFILES.TRIPS.standard.POGNODESrecordID;
        BLOCKSVIEW.firstNodeTime = GTResults.firstNodeTime - dhd;
      }
//
//  Check pullins
//
      if(fileNumber == TMS_STANDARDBLOCKS ||
            ALLFILES.TRIPS.standard.PIGNODESrecordID == NO_RECORD)
      {
        BLOCKSVIEW.lastNodeRecordID = GTResults.lastNODESrecordID;
        BLOCKSVIEW.lastNodeTime = GTResults.lastNodeTime;
      }
      else
      {
        GCTData.fromPATTERNNAMESrecordID = ALLFILES.TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = NO_RECORD;
        GCTData.fromROUTESrecordID = ALLFILES.TRIPS.ROUTESrecordID;
        GCTData.fromSERVICESrecordID = ALLFILES.TRIPS.SERVICESrecordID;
        GCTData.toROUTESrecordID = ALLFILES.TRIPS.ROUTESrecordID;
        GCTData.toSERVICESrecordID = ALLFILES.TRIPS.SERVICESrecordID;
        GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
        GCTData.toNODESrecordID = ALLFILES.TRIPS.standard.PIGNODESrecordID;
        GCTData.timeOfDay = GTResults.lastNodeTime;
        dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        if(dhd == NO_TIME)
          dhd = 0;
        BLOCKSVIEW.lastNodeRecordID = ALLFILES.TRIPS.standard.PIGNODESrecordID;
        BLOCKSVIEW.lastNodeTime = GTResults.lastNodeTime + dhd;
      }
      BLOCKSVIEW.firstRouteRecordID = ALLFILES.TRIPS.ROUTESrecordID;
      BLOCKSVIEW.firstServiceRecordID = ALLFILES.TRIPS.SERVICESrecordID;
      BLOCKSVIEW.layoverTime = NO_TIME;
      BLOCKSVIEW.deadheadTime = NO_TIME;
      BLOCKSVIEW.tripDistance = GTResults.tripDistance;
      if(BLOCKSVIEW.blockNumber != PREVBLOCKSVIEW.blockNumber)
        BLOCKSVIEW.totalDistance = BLOCKSVIEW.tripDistance;
      else
        BLOCKSVIEW.totalDistance += BLOCKSVIEW.tripDistance;
      POGNODESrecordID = pALLFILESTRIPSChunk->POGNODESrecordID;
      if(pALLFILESTRIPSChunk->POGNODESrecordID == NO_RECORD)
        BLOCKSVIEW.pullOutTime = NO_TIME;
      else
      {
        GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
        GCTData.toPATTERNNAMESrecordID = NO_RECORD;
        GCTData.fromROUTESrecordID = pTD->fileInfo.routeRecordID;
        GCTData.fromSERVICESrecordID = pTD->fileInfo.serviceRecordID;
        GCTData.toROUTESrecordID = pTD->fileInfo.routeRecordID;
        GCTData.toSERVICESrecordID = pTD->fileInfo.serviceRecordID;
        GCTData.fromNODESrecordID = pALLFILESTRIPSChunk->POGNODESrecordID;
        GCTData.toNODESrecordID = BLOCKSVIEW.firstNodeRecordID;
        GCTData.timeOfDay = BLOCKSVIEW.firstNodeTime;
        if((PODeadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance)) == NO_TIME)
        {
          MessageBeep(MB_ICONINFORMATION);
          LoadString(hInst, ERROR_179, szarString, sizeof(szarString));
          sprintf(tempString, szarString, BLOCKSVIEW.blockNumber);
          parent->MessageBox(tempString, TMS, MB_OK | MB_ICONINFORMATION);
          BLOCKSVIEW.pullOutTime = NO_TIME;
        }
        else
          BLOCKSVIEW.pullOutTime = BLOCKSVIEW.firstNodeTime - PODeadheadTime;
        distance = (float)fabs((double)distance);
      }
      if(pALLFILESTRIPSChunk->PIGNODESrecordID == NO_RECORD)
        BLOCKSVIEW.pullInTime = NO_TIME;
      else
      {
        GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
        GCTData.toPATTERNNAMESrecordID = NO_RECORD;
        GCTData.fromROUTESrecordID = pTD->fileInfo.routeRecordID;
        GCTData.fromSERVICESrecordID = pTD->fileInfo.serviceRecordID;
        GCTData.toROUTESrecordID = pTD->fileInfo.routeRecordID;
        GCTData.toSERVICESrecordID = pTD->fileInfo.serviceRecordID;
        GCTData.fromNODESrecordID = BLOCKSVIEW.lastNodeRecordID;
        GCTData.toNODESrecordID = pALLFILESTRIPSChunk->PIGNODESrecordID;
        GCTData.timeOfDay = BLOCKSVIEW.lastNodeTime;
        if((PIDeadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance)) == NO_TIME)
        {
          MessageBeep(MB_ICONINFORMATION);
          LoadString(hInst, ERROR_180, szarString, sizeof(szarString));
          sprintf(tempString, szarString, BLOCKSVIEW.blockNumber);
          parent->MessageBox(tempString, TMS, MB_OK | MB_ICONINFORMATION);
          BLOCKSVIEW.pullInTime = NO_TIME;
        }
        else
          BLOCKSVIEW.pullInTime = BLOCKSVIEW.lastNodeTime + PIDeadheadTime;
        distance = (float)fabs((double)distance);
      }
      if(m_bCompressedBlocks)
      {
        BLOCKSVIEW.layoverTime = 0;
        BLOCKSVIEW.deadheadTime = 0;
        BLOCKSVIEW.tripDistance = (float)0.0;
        lastNodeRecordID = BLOCKSVIEW.lastNodeRecordID;
        lastNodeTime = BLOCKSVIEW.lastNodeTime;
        lastRouteRecordID = pALLFILESTRIPSChunk->RGRPROUTESrecordID;
        lastServiceRecordID = pALLFILESTRIPSChunk->SGRPSERVICESrecordID;
        if(keyNumber == 2)
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey2, 2);
        else
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey3, 3);
        bOnceThrough = FALSE;
        while(rcode2 == 0 &&
            pALLFILESTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
            pALLFILESTRIPSChunk->RGRPROUTESrecordID == pTD->fileInfo.routeRecordID &&
            pALLFILESTRIPSChunk->SGRPSERVICESrecordID == pTD->fileInfo.serviceRecordID &&
            pALLFILESTRIPSChunk->blockNumber == BLOCKSVIEW.blockNumber)
        {
          GenerateTrip(ALLFILES.TRIPS.ROUTESrecordID,
                ALLFILES.TRIPS.SERVICESrecordID, ALLFILES.TRIPS.directionIndex,
                ALLFILES.TRIPS.PATTERNNAMESrecordID, ALLFILES.TRIPS.timeAtMLP,
                GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          BLOCKSVIEW.lastNodeRecordID = GTResults.lastNODESrecordID;
          BLOCKSVIEW.lastNodeTime = GTResults.lastNodeTime;
          BLOCKSVIEW.totalDistance += GTResults.tripDistance;
          deadheadTime = 0;
          if(GTResults.firstNODESrecordID != lastNodeRecordID)
          {
            GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
            GCTData.toPATTERNNAMESrecordID = NO_RECORD;
            GCTData.fromROUTESrecordID = lastRouteRecordID;
            GCTData.fromSERVICESrecordID = lastServiceRecordID;
            GCTData.toROUTESrecordID = ALLFILES.TRIPS.ROUTESrecordID;
            GCTData.toSERVICESrecordID = ALLFILES.TRIPS.SERVICESrecordID;
            GCTData.fromNODESrecordID = lastNodeRecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = lastNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            deadheadTime = (deadheadTime == NO_TIME ? 0 : deadheadTime);
          }
          BLOCKSVIEW.deadheadTime += deadheadTime;
          BLOCKSVIEW.layoverTime += GTResults.firstNodeTime - lastNodeTime - deadheadTime;
          if(pALLFILESTRIPSChunk->PIGNODESrecordID == NO_RECORD)
            BLOCKSVIEW.pullInTime = NO_TIME;
          else
          {
            GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
            GCTData.toPATTERNNAMESrecordID = NO_RECORD;
            GCTData.fromROUTESrecordID = pTD->fileInfo.routeRecordID;
            GCTData.fromSERVICESrecordID = pTD->fileInfo.serviceRecordID;
            GCTData.toROUTESrecordID = pTD->fileInfo.routeRecordID;
            GCTData.toSERVICESrecordID = pTD->fileInfo.serviceRecordID;
            GCTData.fromNODESrecordID = BLOCKSVIEW.lastNodeRecordID;
            GCTData.toNODESrecordID = pALLFILESTRIPSChunk->PIGNODESrecordID;
            GCTData.timeOfDay = BLOCKSVIEW.lastNodeTime;
            if((PIDeadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance)) == NO_TIME)
            {
              MessageBeep(MB_ICONINFORMATION);
              LoadString(hInst, ERROR_180, szarString, sizeof(szarString));
              sprintf(tempString, szarString, BLOCKSVIEW.blockNumber);
              parent->MessageBox(tempString, TMS, MB_OK | MB_ICONINFORMATION);
              BLOCKSVIEW.pullInTime = NO_TIME;
            }
            else
              BLOCKSVIEW.pullInTime = BLOCKSVIEW.lastNodeTime + PIDeadheadTime;
            distance = (float)fabs((double)distance);
          }
          lastNodeRecordID = GTResults.lastNODESrecordID;
          lastNodeTime = GTResults.lastNodeTime;
          lastRouteRecordID = ALLFILES.TRIPS.ROUTESrecordID;
          lastServiceRecordID = ALLFILES.TRIPS.SERVICESrecordID;
          if(keyNumber == 2)
            rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey2, 2);
          else
            rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey3, 3);
          bOnceThrough = TRUE;
        }
        if(keyNumber == 2)
        {
          rcode2 = btrieve(B_GETPREVIOUS, databaseFile, &ALLFILES, &ALLKey2, 2);
          ALLFILES.TRIPS.standard.POGNODESrecordID = POGNODESrecordID;  // yeah, yeah
        }
        else
        {
          rcode2 = btrieve(B_GETPREVIOUS, databaseFile, &ALLFILES, &ALLKey3, 3);
          ALLFILES.TRIPS.dropback.POGNODESrecordID = POGNODESrecordID;  // yeah, yeah
        }
      }
    }
//
//  Runs
//
    else if(fileNumber == TMS_RUNS)
    {
      if(ALLFILES.RUNS.pieceNumber == 1)
      {
        recordID = ALLFILES.RUNS.recordID;
        numPieces = GetRunElements(parent->m_hWnd, &ALLFILES.RUNS, &PROPOSEDRUN, &COST, TRUE);
        ALLKey0.RUNSKey0.recordID = recordID;              //  Re-establish position
        btrieve(B_GETEQUAL, TMS_RUNS, &ALLFILES, &ALLKey0, 0);
        btrieve(B_GETPOSITION, TMS_RUNS, &ALLFILES, &ALLKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &ALLFILES, &ALLKey1, 1);
        pieceNumber = 0;
      }
      else
      {
        pieceNumber++;
      }
//
//  See if we have to make the row red
//
//  Illegal runs and zero pay...
//
      if(numPieces > MAXPIECES ||
            LOWORD(RUNSVIEW[0].runType) == ILLEGAL_INDEX ||
            RUNSVIEW[0].runType == NO_RUNTYPE ||
            RUNSVIEW[numPieces - 1].payTime == 0L)
      {
        SetRowColor(F1, lineID + 1, PALETTERGB(255, 0, 0));
      }
//
//  Missing node/trip record pointers
//
      for(nI = 0; nI < numPieces; nI++)
      {
        if(RUNSVIEW[nI].runOnNODESrecordID == NO_RECORD ||
              RUNSVIEW[nI].runOnTRIPSrecordID == NO_RECORD ||
              RUNSVIEW[nI].runOffNODESrecordID == NO_RECORD ||
              RUNSVIEW[nI].runOffTRIPSrecordID == NO_RECORD)
        {
          SetRowColor(F1, lineID + 1, PALETTERGB(255, 0, 0));
          break;
        }
      }
    }
//
//  Crew-only runs
//
    else if(fileNumber == TMS_CREWONLY)
    {
    }
//
//  Roster
//
    else if(fileNumber == TMS_ROSTER)
    {
      memset(&RosterData, 0x00, sizeof(RosterData));
      RosterData[3] = ALLFILES.ROSTER.WEEK[m_RosterWeek].flags;
    }
//
//  Display the row's data
//
    for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
    {
      if(pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNCHECKS + DATATYPE_BIT)
      {
        DisplayPatternRow(F1, pTD, &ALLFILES.PATTERNS, lineID, &nI);
        ALLKey0.PATTERNSKey0.recordID = ALLFILES.PATTERNS.recordID;  //  Re-establish position
        btrieve(B_GETEQUAL, TMS_PATTERNS, &ALLFILES, &ALLKey0, 0);
        btrieve(B_GETPOSITION, TMS_PATTERNS, &ALLFILES, &ALLKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_PATTERNS, &ALLFILES, &ALLKey2, 2);
      }
      else if(pTD->fieldInfo.dataType[nI] == TMSDATA_PATTERNTIMES + DATATYPE_BIT)
      {
        DisplayTripRow(F1, pTD, &ALLFILES.TRIPS, lineID, &nI);
      }
      else if(pTD->fieldInfo.dataType[nI] == TMSDATA_FLAGS + DATATYPE_BIT)
      {
        DisplayFlagRow(F1, pTD, &ALLFILES, lineID, &nI);
      }
      else
      {
        bOK = RenderField(F1, pTD, nI, &ALLFILES.DATABASE, lineID, pieceNumber, outString);
        if(pTD->fieldInfo.dataType[nI] == TMSDATA_LAYOVERTIME + DATATYPE_BIT ||
              pTD->fieldInfo.dataType[nI] == TMSDATA_DEADHEADTIME + DATATYPE_BIT)
        {
          row = lineID + (m_bCompressedBlocks ? 1 : 0);
          if(pTD->fieldInfo.dataType[nI] == TMSDATA_LAYOVERTIME + DATATYPE_BIT)
          {
            if(BLOCKSVIEW.layoverTime != NO_TIME && BLOCKSVIEW.layoverTime < 0)
              SetRowColor(F1, row, PALETTERGB(255, 0, 0));
          }
          else
          {
            if(BLOCKSVIEW.deadheadTime != NO_TIME && BLOCKSVIEW.deadheadTime < 0)
              SetRowColor(F1, row, PALETTERGB(255, 0, 0));
          }
        }
        else
          row = lineID + 1;
        if(row != 0)
        {
          if((pTD->fieldInfo.dataType[nI] == TMSDATA_PULLOUTTIME + DATATYPE_BIT &&
                BLOCKSVIEW.pullOutTime == NO_TIME &&
                pALLFILESTRIPSChunk->POGNODESrecordID != NO_RECORD) ||
             (pTD->fieldInfo.dataType[nI] == TMSDATA_PULLINTIME + DATATYPE_BIT &&
                BLOCKSVIEW.pullInTime == NO_TIME &&
                pALLFILESTRIPSChunk->PIGNODESrecordID != NO_RECORD))
          {
            SetRowColor(F1, row, PALETTERGB(255, 0, 0));
          }
          if(fileNumber == TMS_ROSTER && !bOK)
          {
            SetRowColor(F1, row, PALETTERGB(255, 0, 0));
          }
          F1->SetTextRC(row, nI + 1, outString);
        }
      }
    }
    if(fileNumber == TMS_ROSTER)
    {
      COLORREF color = RGB(0, 0, 0);
      F1->SetSelection(lineID + 1, 1, lineID + 1, pTD->fieldInfo.numFields);
      F1->SetBorder(F1BorderThin, 0, 0, 0, 1, 0, color, color, color, color, color);
    }
    else if((fileNumber == TMS_RUNS || fileNumber == TMS_CREWONLY) && 
          pieceNumber == 0 && lineID != 0)
    {
      COLORREF color = RGB(0, 0, 0);
      F1->SetSelection(lineID + 1, 1, lineID + 1, pTD->fieldInfo.numFields);
      F1->SetBorder(F1BorderThin, 0, 0, 0, 1, 0, color, color, color, color, color);
    }
    else
    {
      COLORREF color = RGB(224, 224, 224);
      F1->SetSelection(lineID + 1, 1, lineID + 1, pTD->fieldInfo.numFields);
      F1->SetBorder(F1BorderThin, 0, 0, 0, 1, 0, color, color, color, color, color);
    }
    lineID++;
//
//  Patterns
//
    if(fileNumber == TMS_PATTERNS)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &ALLFILES, &ALLKey2, 2);
      if(rcode2 != 0 ||
            ALLFILES.PATTERNS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
            ALLFILES.PATTERNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
            ALLFILES.PATTERNS.directionIndex != pTD->fileInfo.directionIndex)
        break;
    }
//
//  Connections
//
    else if(fileNumber == TMS_CONNECTIONS)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &ALLFILES, &ALLKey1, 1);
      if(rcode2 != 0 || (ALLFILES.CONNECTIONS.fromNODESrecordID != CONN.currentFromNODESrecordID ||
            ALLFILES.CONNECTIONS.toNODESrecordID != CONN.currentToNODESrecordID))
      {
        if(++CONN.currentConnection >= CONN.numConnections)
          break;
        ALLKey0.CONNECTIONSKey0.recordID = CONN.startRecordID[CONN.currentConnection];
        btrieve(B_GETEQUAL, TMS_CONNECTIONS, &ALLFILES, &ALLKey0, 0);
        btrieve(B_GETPOSITION, TMS_CONNECTIONS, &ALLFILES, &ALLKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_CONNECTIONS, &ALLFILES, &ALLKey1, 1);
        CONN.currentFromNODESrecordID = ALLFILES.CONNECTIONS.fromNODESrecordID;
        CONN.currentToNODESrecordID = ALLFILES.CONNECTIONS.toNODESrecordID;
      }
    }
//
//  Comments
//
    else if(fileNumber == TMS_COMMENTS)
    {
      recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
      rcode2 = btrieve(B_GETNEXT, TMS_COMMENTS, pCommentText, &COMMENTSKey1, 1);
      recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      if(rcode2 == 0)
      {
        memcpy(&ALLFILES, pCommentText, COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH);
        memset(&ALLFILES.hackedCommentText[COMMENTS_FIXED_LENGTH + COMMENTS_TEXTDISPLAY_LENGTH], 0x00, 1);
      }
    }
//
//  Trips
//
    else if(fileNumber == TMS_TRIPS)
    {
      rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey1, 1);
      if(rcode2 == 0 &&
           (ALLFILES.TRIPS.ROUTESrecordID != pTD->fileInfo.routeRecordID ||
            ALLFILES.TRIPS.SERVICESrecordID != pTD->fileInfo.serviceRecordID ||
            ALLFILES.TRIPS.directionIndex != pTD->fileInfo.directionIndex))
        rcode2 = 101;
    }
//
//  Standard Blocks
//
    else if(fileNumber == TMS_STANDARDBLOCKS || fileNumber == TMS_DROPBACKBLOCKS)
    {
      PREVBLOCKSVIEW.assignedToNODESrecordID = assignedToNODESrecordID;
      PREVBLOCKSVIEW.blockNumber = pALLFILESTRIPSChunk->blockNumber;
      PREVBLOCKSVIEW.firstNodeRecordID = BLOCKSVIEW.firstNodeRecordID;
      PREVBLOCKSVIEW.firstNodeTime = BLOCKSVIEW.firstNodeTime;
      PREVBLOCKSVIEW.lastNodeTime = BLOCKSVIEW.lastNodeTime;
      PREVBLOCKSVIEW.lastNodeRecordID = BLOCKSVIEW.lastNodeRecordID;
      PREVBLOCKSVIEW.lastRouteRecordID = ALLFILES.TRIPS.ROUTESrecordID;
      PREVBLOCKSVIEW.lastServiceRecordID = ALLFILES.TRIPS.SERVICESrecordID;
      if(m_bCompressedBlocks)
      {
        ALLKey2.TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        ALLKey2.TRIPSKey2.RGRPROUTESrecordID = pTD->fileInfo.routeRecordID;
        ALLKey2.TRIPSKey2.SGRPSERVICESrecordID = pTD->fileInfo.serviceRecordID;
        ALLKey2.TRIPSKey2.blockNumber = ++BLOCKSVIEW.blockNumber;
        ALLKey2.TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &ALLFILES, &ALLKey2, keyNumber);
      }
      else
        rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey2, keyNumber);
      if(rcode2 != 0 ||
            pALLFILESTRIPSChunk->assignedToNODESrecordID != assignedToNODESrecordID ||
            pALLFILESTRIPSChunk->RGRPROUTESrecordID != pTD->fileInfo.routeRecordID ||
            pALLFILESTRIPSChunk->SGRPSERVICESrecordID != pTD->fileInfo.serviceRecordID)
      {
        bFound = FALSE;
        if(bFirst)
          bFirst = FALSE;
        else
          currentGarage++;
        while(currentGarage < numGarages)
        {
          assignedToNODESrecordID = garageRecordIDs[currentGarage];
          ALLKey2.TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
          ALLKey2.TRIPSKey2.RGRPROUTESrecordID = pTD->fileInfo.routeRecordID;
          ALLKey2.TRIPSKey2.SGRPSERVICESrecordID = pTD->fileInfo.serviceRecordID;
          ALLKey2.TRIPSKey2.blockNumber = 0;
          ALLKey2.TRIPSKey2.blockSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &ALLFILES, &ALLKey2, 2);
          if(rcode2 == 0 &&
                pALLFILESTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                pALLFILESTRIPSChunk->RGRPROUTESrecordID == pTD->fileInfo.routeRecordID &&
                pALLFILESTRIPSChunk->SGRPSERVICESrecordID == pTD->fileInfo.serviceRecordID)
          {
            bFound = TRUE;
            break;
          }
          currentGarage++;
        }
        if(!bFound)
          rcode2 = 101;
      }
    }
//
//  Runs
//
    else if(fileNumber == TMS_RUNS)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &ALLFILES, &ALLKey1, 1);
      if(rcode2 == 0 &&
            (ALLFILES.RUNS.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID ||
                  ALLFILES.RUNS.SERVICESrecordID != pTD->fileInfo.serviceRecordID))
        rcode2 = 101;
    }
//
//  Crew Only Runs
//
    else if(fileNumber == TMS_CREWONLY)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &ALLFILES, &ALLKey1, 1);
      if(rcode2 == 0 &&
            (ALLFILES.CREWONLY.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID ||
                  ALLFILES.CREWONLY.SERVICESrecordID != pTD->fileInfo.serviceRecordID))
        rcode2 = 101;
    }
//
//  Roster
//
    else if(fileNumber == TMS_ROSTER)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ALLFILES, &ALLKey1, 1);
      if(rcode2 == 0 && ALLFILES.ROSTER.DIVISIONSrecordID != pTD->fileInfo.divisionRecordID)
      {
        rcode2 = 101;
      }
    }
//
//  The others...
//
    else
    {
      switch(pTD->fileInfo.keyNumber)
      {
        case 0:
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey0, 0);
          break;
        case 1:
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey1, 1);
          break;
        case 2:
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey2, 2);
          break;
        case 3:
          rcode2 = btrieve(B_GETNEXT, databaseFile, &ALLFILES, &ALLKey3, 3);
          break;
      }
    }
//
//  Special case for comments, due to its ridiculous length
//
    if(databaseFile == TMS_COMMENTS && rcode2 == 22)  // 22=Not enough data buffer size
      rcode2 = 0;
  }
//
//  Set the grid attributes
//
  attrib:
    if(lineID > 0)
      F1->SetMaxRow((long)lineID);
    F1->SetAllowDelete(FALSE);
    F1->SetAllowDesigner(FALSE);
    F1->SetAllowEditHeaders(FALSE);
    F1->SetAllowFillRange(FALSE);
    F1->SetAllowFormulas(FALSE);
    F1->SetAllowInCellEditing(FALSE);
    F1->SetAllowMoveRange(FALSE);
    F1->SetAllowResize(TRUE);
    F1->SetAllowTabs(FALSE);
    F1->SetAppName(TMS);
    F1->SetEnterMovesDown(TRUE);
    F1->SetDoDblClick(TRUE);
    F1->SetDoRClick(TRUE);
//
//  Size the rows for the roster
//
    if(pTD->fileInfo.fileNumber == TMS_ROSTER && lineID > 0)
    {
      int height = (int)GetPrivateProfileInt((LPSTR)userName, "RosterRowHeight", 2160, (LPSTR)szDatabaseFileName);
      
      F1->SetRowHeight(1, lineID, height, FALSE);
    }
//
//  If need be, sort the table
//
    if(pTD->fieldInfo.sortFieldId != 0)
    {
      for(nI = 0; nI < pTD->fieldInfo.numFields; nI++)
      {
        if(abs(pTD->fieldInfo.sortFieldId) == pTD->fieldInfo.fieldId[nI])
        {
          VARIANT v;

          v.vt = VT_I2;
          v.iVal = nI + 1;
          if(pTD->fieldInfo.sortFieldId < 0)
            v.iVal = -v.iVal;
          F1->Sort(-1, -1, -1, -1, TRUE, v); 
          break;
        }
      }
    }
//
//  Set the focus and restore the cursor
//
    F1->SetShowTabs(F1TabsOff);
    if(IsWindow(F1->m_hWnd))
    {
      F1->SetFocus();
      F1->ShowWindow(SW_SHOW);
    }
    SetCursor(hSaveCursor);

  return;
}

void CTMSApp::SetRowColor(CF1Book* F1, int row, COLORREF crNewColor)
{
  COLORREF crColor;
  BOOL bBold;
  BOOL bItalic;
  BOOL bUnderline;
  BOOL bStrikeout;
  BOOL bOutline;
  BOOL bShadow;
  char szFontName[64];
  short int  nFontSize;
  long rcRow[2];
  long rcCol[2];

  CString s;
  BSTR bs = s.AllocSysString();

  F1->GetFont(&bs, &nFontSize, &bBold, &bItalic, &bUnderline, &bStrikeout, (long *)&crColor, &bOutline, &bShadow);
  F1->GetSelection(0, &rcRow[0], &rcCol[0], &rcRow[1], &rcCol[1]);
  F1->SetSelection(row, -1, row, -1);
  F1->SetFont(szFontName, -nFontSize,
        bBold, bItalic, bUnderline, bStrikeout, crNewColor, bOutline, bShadow);
  F1->SetSelection(rcRow[0], rcCol[0], rcRow[1], rcCol[1]);

  return;
}

void CTMSApp::EstablishDataFields(TableDisplayDef *pTD)
{
  char *p;
  long fieldPosition;
  int  DDFFileNumber;
  int  fieldIds[MAXFIELDS];
  int  nI;
  int  nJ;
  int  nK;
  int  numIds;
  int  numNodes;
  int  rcode2;
  int  bCreated;
  int  width;
  int  ThisType = pTD->fileInfo.fileNumber;

  memset(&(pTD->fieldInfo), 0x00, sizeof(pTD->fieldInfo));
  pTD->fileInfo.position = NO_POSITION;
//
//  Are we getting FILE and FIELD stuff from FILE_DDF/FIELD_DDF or TMS_TABLEVIEW/TMS_CREATED?
//
  bCreated = ThisType == TMS_STANDARDBLOCKS || ThisType == TMS_DROPBACKBLOCKS ||
        ThisType == TMS_RUNS || ThisType == TMS_ROSTER || ThisType == TMS_TIMECHECKS;
  if(bCreated)
  {
    if(ThisType == TMS_STANDARDBLOCKS)
    {
      DDFFileNumber = VIEW_STANDARDBLOCKS;
    }
    else if(ThisType == TMS_DROPBACKBLOCKS)
    {
      DDFFileNumber = VIEW_DROPBACKBLOCKS;
    }
    else if(ThisType == TMS_RUNS)
    {
      DDFFileNumber = VIEW_RUNS;
    }
    else if(ThisType == TMS_ROSTER)
    {
      DDFFileNumber = VIEW_ROSTER;
    }
    else 
    {
      DDFFileNumber = VIEW_TIMECHECKS;
    }
  }
  else
  {
    DDFFileNumber = DDFFileNumbers[ThisType];
  }
//
//  See if there's a "SortColumn" entry in the .TMS file for this table
//
  sprintf(szarString, "SortColumn%d", pTD->fileInfo.fileNumber);
  pTD->fieldInfo.sortFieldId = GetPrivateProfileInt(userName, szarString, 0, szDatabaseFileName);
//
//  See if there's an "Order" entry in the .TMS file for this table
//
  sprintf(szarString, "Order%d", pTD->fileInfo.fileNumber);
  GetPrivateProfileString(userName, szarString, "",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  numIds = 0;
  p = tempString;
  while(sscanf(p, " %d%n", &fieldIds[numIds], &width) != EOF)
  {
    if(fieldIds[numIds] != 0)
      numIds++;
    p += width;
  }
//
//  If we didn't get any, fill the fieldId array with the field Ids from FIELD/CREATED.
//
  if(numIds == 0)
  {
    if(bCreated)
    {
//
//  Get the record id of the file
//
      if(pTD->fileInfo.fileNumber == TMS_STANDARDBLOCKS ||
            pTD->fileInfo.fileNumber == TMS_DROPBACKBLOCKS)
        FIELDDDFKey1.File = DDFFileNumbers[TMS_TRIPS];
      else
      {
        strncpy(FILEDDFKey1.Name, TABLEVIEW.Name, TABLEVIEW_NAME_LENGTH);
        btrieve(B_GETEQUAL, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
        FIELDDDFKey1.File = FILEDDF.Id;
      }
      rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      if(rcode2 == 0)
      {
        fieldIds[numIds] = FIELDDDF.Id;
        numIds++;
      }
//
//  Get the rest
//
      CREATEDKey1.File = DDFFileNumber;
      CREATEDKey1.Sequence = 0;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
      while(rcode2 == 0 &&
            CREATED.File == DDFFileNumber &&
            numIds < MAXFIELDS)
      {
        ATTRIBUTESKey0.Id = CREATED.Id;
        rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
        if(rcode2 == 0)
        {
          fieldIds[numIds] = CREATED.Id;
          numIds++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
      }
    }
//
//  Not bCreated
//
    else
    {
      FIELDDDFKey1.File = DDFFileNumber;
      rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      while(rcode2 == 0 &&
            FIELDDDF.File == DDFFileNumber &&
            numIds < MAXFIELDS)
      {
        ATTRIBUTESKey0.Id = FIELDDDF.Id;
        rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
        if(rcode2 == 0)
        {
          if(ATTRIBUTES.sequence > 0)
            fieldIds[ATTRIBUTES.sequence - 1] = FIELDDDF.Id;
          else  // Backward compaitibility until he OK's in SETCOL
            fieldIds[numIds] = FIELDDDF.Id;
          numIds++;
        }
        rcode2 = btrieve(B_GETNEXT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      }
    }
  }
//
//  If we're dealing with patterns or trips, we have to adjust numIds by the
//  number of nodes in the BASE pattern for this route and service.
//
  numNodes = 0;
  if(DDFFileNumber == DDFFileNumbers[TMS_PATTERNS] || DDFFileNumber == DDFFileNumbers[TMS_TRIPS])
  {
    PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
    PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
    PATTERNSKey2.nodeSequence = 0;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == pTD->fileInfo.routeRecordID &&
          PATTERNS.SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
          PATTERNS.directionIndex == pTD->fileInfo.directionIndex &&
          PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
    {
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        numNodes++;
        numIds++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
  }
//
// Now that we're sure that we've got something, go through the fields one by one
//
  pTD->fieldInfo.cNumPatternNodes = numNodes;
  nK = 0;
  for(nI = 0; nI < numIds; nI++)
  {
    pTD->fieldInfo.flags[nI] = 0;
    if(bCreated)
    {
      if(nI == 0)
      {
        FIELDDDFKey0.Id = fieldIds[0];
        rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
        if(rcode2 != 0)
          break;
        trim(FIELDDDF.Name, sizeof(FIELDDDF.Name));
        strcpy(pTD->fieldInfo.columnTitles[nI], FIELDDDF.Name);
        pTD->fieldInfo.offset[nI] = (int)FIELDDDF.Offset;
        pTD->fieldInfo.dataType[nI] = (int)FIELDDDF.DataType;
        pTD->fieldInfo.numBytes[nI] = (int)FIELDDDF.Size;
        pTD->fieldInfo.numChars[nI] = (int)FIELDDDF.Size;
        pTD->fieldInfo.flags[nI] |= ATTRIBUTES_FLAG_HIDDEN;
      }
      else
      {
        CREATEDKey0.Id = fieldIds[nK]; //nI];
        btrieve(B_GETEQUAL, TMS_CREATED, &CREATED, &CREATEDKey0, 0);
        trim(CREATED.Name, CREATED_NAME_LENGTH);
        strcpy(pTD->fieldInfo.columnTitles[nI], CREATED.Name);
        if(CREATED.FromFieldId != NO_RECORD)
        {
          FIELDDDFKey0.Id = CREATED.FromFieldId;
          rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
          if(rcode2 == 0)
          {
            pTD->fieldInfo.offset[nI] = (int)FIELDDDF.Offset;
            pTD->fieldInfo.dataType[nI] = (int)FIELDDDF.DataType;
            pTD->fieldInfo.numBytes[nI] = (int)FIELDDDF.Size;
            pTD->fieldInfo.numChars[nI] = (int)FIELDDDF.Size;
          }
        }
      }
    }
    else
    {
      FIELDDDFKey0.Id = fieldIds[nK];
      rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
      if(rcode2 != 0 || nI > MAXFIELDS)
        break;
      trim(FIELDDDF.Name, sizeof(FIELDDDF.Name));
      strcpy(pTD->fieldInfo.columnTitles[nI], FIELDDDF.Name);
      pTD->fieldInfo.offset[nI] = (int)FIELDDDF.Offset;
      pTD->fieldInfo.dataType[nI] = (int)FIELDDDF.DataType;
      if(FIELDDDF.Size == COMMENTS_VARIABLE_LENGTH)
      {
        pTD->fieldInfo.numBytes[nI] = COMMENTS_TEXTDISPLAY_LENGTH;
        pTD->fieldInfo.numChars[nI] = COMMENTS_TEXTDISPLAY_LENGTH;
      }
      else
      {
        pTD->fieldInfo.numBytes[nI] = (int)FIELDDDF.Size;
        pTD->fieldInfo.numChars[nI] = (int)FIELDDDF.Size;
      }
    }
    pTD->fieldInfo.fieldId[nI] = fieldIds[nK];
//
//  Modify with attributes if available
//
    ATTRIBUTESKey0.Id = fieldIds[nK];
    rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
    if(rcode2 == 0)
    {
      trim(ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
      strcpy(pTD->fieldInfo.columnTitles[nI], ATTRIBUTES.fieldTitle);
      pTD->fieldInfo.flags[nI] = ATTRIBUTES.flags;
      if(ATTRIBUTES.flags & ATTRIBUTES_FLAG_DERIVED)
      {
        pTD->fieldInfo.linkFile[nI] = NO_RECORD;
        for(nJ = 0; nJ <= TMS_LASTFILE; nJ++)
        {
          if(ATTRIBUTES.derivedTableId == DDFFileNumbers[nJ])
          {
            pTD->fieldInfo.linkFile[nI] = nJ;
            break;
          }
        }
        btrieve(B_GETPOSITION, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
        memcpy(&fieldPosition, &FIELDDDF, 4);
        FIELDDDFKey0.Id = ATTRIBUTES.derivedFieldId;
        btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
        pTD->fieldInfo.dataType[nI] = (int)FIELDDDF.DataType;
        pTD->fieldInfo.numBytes[nI] = (int)FIELDDDF.Size;
        pTD->fieldInfo.numChars[nI] = (int)FIELDDDF.Size;
        pTD->fieldInfo.linkOffset[nI] = (int)FIELDDDF.Offset;
        memcpy(&FIELDDDF, &fieldPosition, 4);
        btrieve(B_GETDIRECT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
      }
      else
        pTD->fieldInfo.linkFile[nI] = NO_RECORD;
      if(ATTRIBUTES.flags & ATTRIBUTES_FLAG_FORMATTED)
      {
        pTD->fieldInfo.dataType[nI] = ATTRIBUTES.formattedDataType + DATATYPE_BIT;
        switch(ATTRIBUTES.formattedDataType)
        {
          case TMSDATA_TIMEOFDAY:
          case TMSDATA_TIMEINHM:
          case TMSDATA_PULLOUTTIME:
          case TMSDATA_FIRSTTIME:
          case TMSDATA_LASTTIME:
          case TMSDATA_PULLINTIME:
            pTD->fieldInfo.numChars[nI] = 5;
            break;

          case TMSDATA_TIMEINMINUTES:
          case TMSDATA_LAYOVERTIME:
            pTD->fieldInfo.numChars[nI] = 4;
            break;

//
//  I know this is graceless, but so what?
//
//  Ok - going into getNodeNames, we've possibly used some space in the pTD->fieldInfo structure.
//  Setting pTD->fieldInfo.numFields to nI means we're going to start dropping node data at the
//  right place.  Coming out of getNodeNames, we have to re-establish our position to
//  reflect the number of nodes that were added.  Since the node names are going to replace
//  the original field, we'll knock one off of numIds as well.
//
          case TMSDATA_PATTERNTIMES:
          case TMSDATA_PATTERNCHECKS:
            pTD->fieldInfo.numFields = nI;
            pTD->fieldInfo.firstNodeColumn = nI;
            GetNodeNames(pTD, DDFFileNumber);
            nI = pTD->fieldInfo.numFields - 1;
            numIds--;
            break;

//
//  Likewise...
//
          case TMSDATA_FLAGS:
            pTD->fieldInfo.numFields = nI;
            pTD->fieldInfo.firstFlagColumn = nI;
            GetFlagNames(pTD, DDFFileNumber);
            nI = pTD->fieldInfo.numFields - 1;
            numIds += (pTD->fieldInfo.cNumFlagFields - 1);
            break;

          case TMSDATA_FIRSTNODE:
          case TMSDATA_LASTNODE:
            pTD->fieldInfo.numChars[nI] = 4;
            break;

          default:
            break;
        }
      }
    }
    nK++;
  }
//
//  No Ids?  Add one for recordID
//
  if(numIds == 0)
  {
    pTD->fieldInfo.offset[0] = 0;
    pTD->fieldInfo.dataType[0] = 0;
    pTD->fieldInfo.numBytes[0] = 4;
    pTD->fieldInfo.numChars[0] = 8;
    numIds++;
  }
  pTD->fieldInfo.numFields = numIds;
}

BOOL CTMSApp::GetFlagNames(TableDisplayDef *pTD, int fileID)
{
  int  nI;
  int  nTarget;

  if(pTD == NULL)
    return(FALSE);

  for(nI = 0; nI < TMS_LASTFILE; nI++)
  {
    if(DDFFileNumbers[nI] == fileID)
    {
      nTarget = nI;
      break;
    }
  }

  switch(nTarget)
  {
    case TMS_ROUTES:
      pTD->fieldInfo.cNumFlagFields = 2;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Shuttle?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "School?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      break;

    case TMS_NODES:
      pTD->fieldInfo.cNumFlagFields = 3;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Gar?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Stop?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Secure?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      break;

    case TMS_CONNECTIONS:
      pTD->fieldInfo.cNumFlagFields = 7;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "2W?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "RT?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "TT?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "DT?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "EQ?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "AEQ?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "SS?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      break;

    case TMS_BUSTYPES:
      pTD->fieldInfo.cNumFlagFields = 4;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Accessible?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Carrier?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Surface?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Rail?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      break;

    case TMS_BUSES:
      pTD->fieldInfo.cNumFlagFields = 1;
      strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], "Retired?");
      pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_FLAGS + DATATYPE_BIT;
      pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
      pTD->fieldInfo.numFields++;
      break;
  }

  return(TRUE);
}

//BOOL CMainFrame::GetNodeNames(PDISPLAYINFO pTD, int fileID)
BOOL CTMSApp::GetNodeNames(TableDisplayDef *pTD, int fileID)
{
  int  rcode2;
  int  numNodes;
  char abbrName[NODES_ABBRNAME_LENGTH + 1];

  if(pTD == NULL)
    return(FALSE);
  if(fileID == DDFFileNumbers[TMS_PATTERNS] || fileID == DDFFileNumbers[TMS_TRIPS])
  {
    PATTERNSKey2.ROUTESrecordID = pTD->fileInfo.routeRecordID;
    PATTERNSKey2.SERVICESrecordID = pTD->fileInfo.serviceRecordID;
    PATTERNSKey2.directionIndex = pTD->fileInfo.directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
    PATTERNSKey2.nodeSequence = NO_RECORD;
    numNodes = 0;
    rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == pTD->fileInfo.routeRecordID &&
          PATTERNS.SERVICESrecordID == pTD->fileInfo.serviceRecordID &&
          PATTERNS.directionIndex == pTD->fileInfo.directionIndex &&
          PATTERNS.PATTERNNAMESrecordID == basePatternRecordID &&
          numNodes < RECORDIDS_KEPT &&
          pTD->fieldInfo.numFields < MAXFIELDS - 1)
    {
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        NODESKey0.recordID = PATTERNS.NODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(abbrName, NODES_ABBRNAME_LENGTH);
        strcpy(pTD->fieldInfo.columnTitles[pTD->fieldInfo.numFields], abbrName);
        if(fileID == DDFFileNumbers[TMS_PATTERNS])
        {
          pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_PATTERNCHECKS + DATATYPE_BIT;
          pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 3;
          pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 1;
        }
        else
        {
          pTD->fieldInfo.dataType[pTD->fieldInfo.numFields] = TMSDATA_PATTERNTIMES + DATATYPE_BIT;
          pTD->fieldInfo.numChars[pTD->fieldInfo.numFields] = 5;
          pTD->fieldInfo.numBytes[pTD->fieldInfo.numFields] = 4;
        }
        pTD->fieldInfo.recordID[pTD->fieldInfo.numFields] = NODES.recordID;
        pTD->fieldInfo.basePatternSeq[pTD->fieldInfo.numFields] = PATTERNS.nodeSequence;
        numNodes++;
        pTD->fieldInfo.numFields++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }
  }
  
  return(TRUE);
}
