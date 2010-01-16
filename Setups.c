//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Set up a ROUTES combobox
//
int SetUpRouteList(HWND hWnd, int idControl, long currentSelection)
{
  HANDLE hCtl;
  int    numRoutes = 0;
  int    rcode2;
  int    nI;
  char   routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char   routeName[ROUTES_NAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
    strcpy(tempString, routeNumber);
    strcat(tempString, " - ");
    strcat(tempString, routeName);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, ROUTES.recordID);
    if(ROUTES.recordID == currentSelection)
      SendMessage(hCtl, CB_SETCURSEL, (WPARAM)numRoutes, (LPARAM)0);
    numRoutes++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  if(numRoutes == 0)
    TMSError(hWnd, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);

  return(numRoutes);
}

//
//  Set up a SERVICES combobox
//
int SetUpServiceList(HWND hWnd, int idControl, long currentSelection)
{
  HANDLE hCtl;
  int    numServices = 0;
  int    rcode2;
  int    nI;
  char   serviceName[SERVICES_NAME_LENGTH+1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)serviceName);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)SERVICES.recordID);
    if(SERVICES.recordID == currentSelection)
      SendMessage(hCtl, CB_SETCURSEL, (WPARAM)numServices, (LPARAM)0);
    numServices++;
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
  if(numServices == 0)
    TMSError(hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);

  return(numServices);
}

//
//  Set up a DIVISIONS combobox
//
int SetUpDivisionList(HWND hWnd, int idControl, long currentSelection)
{
  HANDLE hCtl;
  int    numDivisions = 0;
  int    rcode2;
  int    nI;
  char   divisionName[DIVISIONS_NAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)divisionName);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)DIVISIONS.recordID);
    if(DIVISIONS.recordID == currentSelection)
      SendMessage(hCtl, CB_SETCURSEL, (WPARAM)numDivisions, (LPARAM)0);
    numDivisions++;
    rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  }
  if(numDivisions == 0)
    TMSError(hWnd, MB_ICONSTOP, ERROR_080, (HANDLE)NULL);

  return(numDivisions);
}

//
//  Set up the list of Roster Weeks
//
void SetUpRosterWeekList(HWND hWnd, int idControl, long currentSelection)
{
  HANDLE hCtl;
  int    nI, nJ;

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
  {
    sprintf(tempString, "Roster Week %ld", nI + 1);
    nJ = SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nJ, (LPARAM)nI);
    if(m_RosterWeek == nI)
    {
      SendMessage(hCtl, CB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
    }
  }
}
//
//  Set up a JURISDICTIONS combobox
//
int SetUpJurisdictionList(HWND hWnd, int idControl)
{
  HANDLE hCtl;
  int    numJurisdictions = 0;
  int    rcode2;
  int    nI;
  char   jurisdictionName[JURISDICTIONS_NAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(jurisdictionName, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
    trim(jurisdictionName, JURISDICTIONS_NAME_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)jurisdictionName);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)JURISDICTIONS.recordID);
    numJurisdictions++;
    rcode2 = btrieve(B_GETNEXT, TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey1, 1);
  }

  return(numJurisdictions);
}

//
//  Set up a DRIVER TYPES combobox
//
int SetUpDriverTypeList(HWND hWnd, int idControl, long currentSelection)
{
  HANDLE hCtl;
  int    numDriverTypes = 0;
  int    rcode2;
  int    nI;
  char   driverTypeName[DRIVERTYPES_NAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(driverTypeName, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
    trim(driverTypeName, DRIVERTYPES_NAME_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)driverTypeName);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)DRIVERTYPES.recordID);
    if(DRIVERTYPES.recordID == currentSelection)
      SendMessage(hCtl, CB_SETCURSEL, (WPARAM)numDriverTypes, (LPARAM)0);
    numDriverTypes++;
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey1, 1);
  }

  return(numDriverTypes);
}

//
// Set up a NODES combobox
//
int SetUpNodeList(HWND hWnd, int idControl, BOOL garagesOnly)
{
  HANDLE hCtl;
  BOOL   bDisplay;
  int    numNodes = 0;
  int    rcode2;
  int    nI;
  char   abbrName[NODES_ABBRNAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
  while(rcode2 == 0)
  {
    bDisplay = !garagesOnly || (garagesOnly && (NODES.flags & NODES_FLAG_GARAGE));
    if(bDisplay)
    {
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(abbrName, NODES_ABBRNAME_LENGTH);
      nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)abbrName);
      SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)NODES.recordID);
      numNodes++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
  }
  if(numNodes == 0)
    TMSError(hWnd, MB_ICONSTOP, garagesOnly ? ERROR_045 : ERROR_024, (HANDLE)NULL);

  return(numNodes);
}

//
//  Set up a PATTERNS combobox
//
int SetUpPatternList(HWND hWnd, int idControl, long ROUTESrecordID,
      long SERVICESrecordID, int directionIndex, BOOL reset, BOOL includeDirection)
{
  HANDLE hCtl;
  int    numPatterns = 0;
  int    rcode2;
  int    nI;
  long   currentPatternRecordID;
  char   patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char   abbrName[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char   bothNames[PATTERNNAMES_NAME_LENGTH + DIRECTIONS_ABBRNAME_LENGTH + 2];

  hCtl = GetDlgItem(hWnd, idControl);
  if(reset)
    SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  if(includeDirection)
  {
    ROUTESKey0.recordID = ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[directionIndex];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(abbrName, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(abbrName, DIRECTIONS_ABBRNAME_LENGTH);
  }
  PATTERNSKey2.ROUTESrecordID = ROUTESrecordID;
  PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
  PATTERNSKey2.directionIndex = directionIndex;
  PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
  PATTERNSKey2.nodeSequence = 0;
  currentPatternRecordID = NO_RECORD;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  while(rcode2 == 0 &&
        PATTERNS.ROUTESrecordID == ROUTESrecordID &&
        PATTERNS.SERVICESrecordID == SERVICESrecordID &&
        PATTERNS.directionIndex == directionIndex)
  {
    if(PATTERNS.PATTERNNAMESrecordID != currentPatternRecordID)
    {
      currentPatternRecordID = PATTERNS.PATTERNNAMESrecordID;
      PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
      btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
      trim(patternName, PATTERNNAMES_NAME_LENGTH);
      if(includeDirection)
      {
        strcpy(bothNames, abbrName);
        strcat(bothNames, ": ");
        strcat(bothNames, patternName);
        nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)bothNames);
      }
      else
        nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)patternName);
      SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)PATTERNS.PATTERNNAMESrecordID);
      numPatterns++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
  if(numPatterns != 0)
    SendMessage(hCtl, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

  return(numPatterns);
}
//
//  Set up a BUSTYPES combobox
//
int SetUpBustypeList(HWND hWnd, int idControl)
{
  HANDLE hCtl;
  int    numBustypes = 0;
  int    rcode2;
  int    nI;
  char   bustypeName[BUSTYPES_NAME_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(bustypeName, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
    trim(bustypeName, BUSTYPES_NAME_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)bustypeName);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)BUSTYPES.recordID);
    numBustypes++;
    rcode2 = btrieve(B_GETNEXT, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey1, 1);
  }

  return(numBustypes);
}
//
//  Set up COMMENTS combobox
//
int SetUpCommentList(HWND hWnd, int idControl)
{
  HANDLE hCtl;
  int    numComments = 0;
  int    nI;
  int    rcode2;
  char   commentCode[COMMENTS_CODE_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_COMMENTS, &COMMENTS, &COMMENTSKey1, 1);
  while(rcode2 == 0 || rcode2 == 22)
  {
    strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
    trim(commentCode, COMMENTS_CODE_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)commentCode);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)COMMENTS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_COMMENTS, &COMMENTS, &COMMENTSKey1, 1);
    numComments++;
  }

  return(numComments);
}
//
//  Set up SIGNCODES combobox
//
int SetUpSigncodeList(HWND hWnd, int idControl)
{
  HANDLE hCtl;
  int    numSigncodes = 0;
  int    nI;
  int    rcode2;
  char   signcode[SIGNCODES_CODE_LENGTH + 1];

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  rcode2 = btrieve(B_GETFIRST, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(signcode, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
    trim(signcode, SIGNCODES_CODE_LENGTH);
    nI = (int)SendMessage(hCtl, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)signcode);
    SendMessage(hCtl, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)SIGNCODES.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey1, 1);
    numSigncodes++;
  }

  return(numSigncodes);
}
//
//  Set up DIRECTIONS combobox(es)
//
int SetUpDirectionList(HWND hWnd, int idOutbound, int idInbound)
{
  HANDLE hCtlOUTBOUND;
  HANDLE hCtlINBOUND;
  int    numDirections = 0;
  int    rcode2;
  int    nI;
  char   longName[DIRECTIONS_LONGNAME_LENGTH + 1];
  BOOL   doOutbound;
  BOOL   doInbound;

  if((doOutbound = (idOutbound == NO_RECORD ? FALSE : TRUE)) == TRUE)
  {
    hCtlOUTBOUND = GetDlgItem(hWnd, idOutbound);
    SendMessage(hCtlOUTBOUND, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  }
  if((doInbound = (idInbound == NO_RECORD ? FALSE : TRUE)) == TRUE)
  {
    hCtlINBOUND = GetDlgItem(hWnd, idInbound);
    SendMessage(hCtlINBOUND, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  }
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(longName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(longName, DIRECTIONS_LONGNAME_LENGTH);
    if(doOutbound)
    {
      nI = (int)SendMessage(hCtlOUTBOUND, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)longName);
      SendMessage(hCtlOUTBOUND, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)DIRECTIONS.recordID);
    }
    if(doInbound)
    {
      nI = (int)SendMessage(hCtlINBOUND, CB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)longName);
      SendMessage(hCtlINBOUND, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)DIRECTIONS.recordID);
    }
    numDirections++;
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  }
//
// Can't have no directions
//
  if(numDirections == 0)
    TMSError(hWnd, MB_ICONSTOP, ERROR_005, (HANDLE)NULL);

  return(numDirections);
}

//
//  Set up a RUNS listbox
//
int SetUpRunList(HWND hWnd, long DIVISIONSrecordID, long SERVICESrecordID, int idControl)
{
  HANDLE hCtl;
  int    numRuns = 0;
  int    rcode2;
  int    nI;

  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, LB_RESETCONTENT, (WPARAM)(WPARAM)0, (LPARAM)0);
  RUNSKey1.DIVISIONSrecordID = DIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = SERVICESrecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == DIVISIONSrecordID &&
        RUNS.SERVICESrecordID == SERVICESrecordID)
  {
    if(RUNS.pieceNumber == 1)
    {
      ltoa(RUNS.runNumber, tempString, 10);
      nI = (int)SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtl, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)RUNS.recordID);
      numRuns++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
  if(numRuns == 0)
    TMSError(hWnd, MB_ICONSTOP, ERROR_201, (HANDLE)NULL);

  return(numRuns);
}

//
//  Set up the node labels listbox
//
int SetUpReliefLabels(HWND hWnd, long SERVICESrecordID,
      int idControl, char *selected, int nodelistControl)
{
  HCURSOR hSaveCursor;
  HANDLE hCtl;
  char tString[64];
  int  nI;
  int  nJ;
  int  numLabels;

  hSaveCursor = SetCursor(hCursorWait);
  hCtl = GetDlgItem(hWnd, idControl);
  SendMessage(hCtl, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Set tString up to contain the unique labels for this service
//
  numLabels = SetUpLabelArray(tString, SERVICESrecordID, GetDlgItem(hWnd, nodelistControl));
//
//  Add the individual labels to the listbox
//
  for(nI = 0; nI < numLabels; nI++)
  {
    if(tString[nI] == ' ' || tString[nI] == '-')
      continue;
    szarString[0] = tString[nI];
    szarString[1] = '\0';
    SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)szarString);
  }
//
//  And highlight any selected labels as found in CUTPARMS.labels
//
  SendMessage(hCtl, LB_SETSEL, (WPARAM)FALSE, MAKELPARAM(-1, 0));
  for(nI = 0; nI < (int)strlen(selected); nI++)
  {
    szarString[0] = selected[nI];
    szarString[1] = '\0';
    nJ = SendMessage(hCtl, LB_FINDSTRINGEXACT, (WPARAM)(-1), (LONG)(LPSTR)szarString);
    if(nJ != LB_ERR)
      SendMessage(hCtl, LB_SETSEL, (WPARAM)TRUE, MAKELPARAM(nJ, 0));
  }
//
//  All done
//
  SetCursor(hSaveCursor);

  return(numLabels);
}

//
//  SetUpLabelArray()
//

int SortLabels(const void *a, const void *b)
{
  char aVal = *((char *)a);
  char bVal = *((char *)b);
  return(aVal > bVal ? 1 : aVal < bVal ? -1 : 0);
}

int SetUpLabelArray(char *labels, long SERVICESrecordID, HANDLE hCtl)
{
  BOOL bFound;
  char reliefLabels[NODES_RELIEFLABELS_LENGTH + 1];
  int  nI;
  int  nIStart;
  int  nIEnd;
  int  nJ;
  int  rcode2;

  if(hCtl)
    SendMessage(hCtl, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
//
//  Establish serviceIndex
//
  if(SERVICESrecordID == NO_RECORD)
  {
    nIStart = 0;
    nIEnd = NODES_RELIEFLABELS_LENGTH - 1;
  }
  else
  {
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    nJ = 0;
    while(rcode2 == 0)
    {
      if(SERVICES.recordID == SERVICESrecordID)
        break;
      nJ++;
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
    nIStart = nJ;
    nIEnd = nJ;
  }
//
//  Go through all the nodes
//
  strcpy(labels, "");
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
    strncpy(reliefLabels, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
    trim(reliefLabels, NODES_RELIEFLABELS_LENGTH);
    if(strcmp(reliefLabels, "") != 0)
    {
      for(nI = nIStart; nI <= nIEnd; nI++)
      {
        if(reliefLabels[nI] == ' ' || reliefLabels[nI] == '-')
          continue;
        for(bFound = FALSE, nJ = 0; nJ < (int)strlen(labels); nJ++)
        {
          if(reliefLabels[nI] == labels[nJ])
          {
            bFound = TRUE;
            break;
          }
        }
        if(hCtl)
        {
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          nJ = SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtl, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)NODES.recordID);
        }
        if(bFound)
          continue;
        szarString[0] = reliefLabels[nI];
        szarString[1] = '\0';
        strcat(labels, szarString);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
  qsort((void *)labels, strlen(labels), sizeof(char), SortLabels);

  return(strlen(labels));
}

//
//  Set up a blocks listbox
//
int SetUpBlocksListbox(PDISPLAYINFO pDI, HANDLE hCtl)
{
  BLOCKSDef *pTRIPSChunk;
  BOOL      bFirst;
  long      assignedToNODESrecordID;
  int       nI;
  int       keyNumber = pDI->fileInfo.keyNumber;
  int       rcode2;
  int       numBlocks = 0;

  pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
  
  SendMessage(hCtl, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  NODESKey1.flags = NODES_FLAG_GARAGE;
  memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
  if(rcode2 != 0)  // No Garages
    return(NO_RECORD);
//
//  Loop through the garages
//
  bFirst = TRUE;
  while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
  {
    assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
    if(assignedToNODESrecordID == NO_RECORD)
      strcpy(szarString, "");
    else
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      sprintf(szarString, "\t(%s)", tempString);
    }
    TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
    TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
    TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
    TRIPSKey2.blockNumber = 0;
    TRIPSKey2.blockSequence = NO_TIME;
    rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
    while(rcode2 == 0)
    {
      if(pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
            pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
            pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID)
      {
        sprintf(tempString, "%6ld%s", pTRIPSChunk->blockNumber, szarString);
        nI = SendMessage(hCtl, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtl, LB_SETITEMDATA, nI, TRIPS.recordID);
        TRIPSKey2.blockNumber++;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        numBlocks++;
      }
      else
        break;
    }
    if(bFirst)
      bFirst = FALSE;
    else
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }

  return(numBlocks);
} 
