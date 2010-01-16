//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT02() - Busbook Detail Report
//

#include "TMSHeader.h"

static char szRSDHeader[16];
static char szRSDTrailer[16];
static char szEachNodeHeader[16];
static char szEachNodeTrailer[16];
static char szAccessible[16];
static char szSeparator[16];
static int  nTimeFormat;
static BOOL bUseColons;
static char szTimeA[16];
static char szTimeP[16];
static char szTimeX[16];
static BOOL bCodes;
static BOOL bText;
static char szSection[] = "Report1";

//
//  Local prototype
//
void LoadHeadersAndTrailers02(void);

BOOL FAR TMSRPT02(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bFound;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  *ptr;
  long  COMMENTSrecordIDs[200];
  int   numComments = 0;
  int   maxSelectedPatternNodes = 50;
  int   maxTripPatternNodes = 50;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numSelectedPatternNodes;
  int   numTripPatternNodes;
  int   rcode2;
  int   timeFormatSave = timeFormat;

  long  *pSelectedPatternNodes = NULL;
  long  *pSelectedPatternSeqs = NULL;
  long  *pTripPatternNodes = NULL;
  long  *pTripPatternSeqs = NULL;

  pPassedData->nReportNumber = 1;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.PATTERNNAMESrecordID = NO_RECORD;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_PATTERNNAMES | RPFLAG_COMMENTS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Build the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt02.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Allocate the nodes arrays
//
  pSelectedPatternNodes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pSelectedPatternNodes == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }

  pSelectedPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pSelectedPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }

  pTripPatternNodes = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pTripPatternNodes == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }

  pTripPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternNodes); 
  if(pTripPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
//
//  Load any header and trailer special characters
//
  LoadHeadersAndTrailers02();
//
//  Loop through the routes
//
  SetCursor(hCursorWait);
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
    {
      continue;
    }
//
//  Loop through the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
//
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
//
//  Find the selected pattern nodes
//
        numSelectedPatternNodes = 0;
        PATTERNNAMESKey0.recordID = REPORTPARMS.PATTERNNAMESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
        if(rcode2 == 0)
        {
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMES.recordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == PATTERNNAMES.recordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              pSelectedPatternSeqs[numSelectedPatternNodes] = PATTERNS.nodeSequence;
              pSelectedPatternNodes[numSelectedPatternNodes++] = PATTERNS.NODESrecordID;
              if(numSelectedPatternNodes >= maxSelectedPatternNodes)
              {
                maxSelectedPatternNodes += 50;
                pSelectedPatternNodes = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pSelectedPatternNodes, sizeof(long) * maxSelectedPatternNodes); 
                if(pSelectedPatternNodes == NULL)
                {
                  AllocationError(__FILE__, __LINE__, TRUE);
                  goto deallocate;
                }
                pSelectedPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pSelectedPatternSeqs, sizeof(long) * maxSelectedPatternNodes); 
                if(pSelectedPatternSeqs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, TRUE);
                  goto deallocate;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
        }
        if(numSelectedPatternNodes == 0)
        {
          continue;
        }
        m_bEstablishRUNTIMES = TRUE;
//
//  Write out the id record
//
        strcpy(tempString, szEachNodeHeader);
        strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(szarString, ROUTES_NUMBER_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, " ");
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(szarString, ROUTES_NAME_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, " ");
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(szarString, DIRECTIONS_ABBRNAME_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, " ");
        strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(szarString, SERVICES_NAME_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, szEachNodeTrailer);
        strcat(tempString, "\r\n");
        _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Get all the trips
//
        numComments = 0;
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              TRIPS.directionIndex == nK)
        {
//
//  Find the TRIP pattern nodes
//
          numTripPatternNodes = 0;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              pTripPatternSeqs[numTripPatternNodes] = PATTERNS.nodeSequence;
              pTripPatternNodes[numTripPatternNodes++] = PATTERNS.NODESrecordID;
              if(numTripPatternNodes >= maxTripPatternNodes)
              {
                maxTripPatternNodes += 50;
                pTripPatternNodes = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pTripPatternNodes, sizeof(long) * maxTripPatternNodes); 
                if(pTripPatternNodes == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto deallocate;
                }
                pTripPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pTripPatternSeqs, sizeof(long) * maxTripPatternNodes); 
                if(pTripPatternSeqs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto deallocate;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Set up the output string
//
          strcpy(tempString, szEachNodeHeader);
//
//  Is this trip accessible?
//
          if(TRIPS.BUSTYPESrecordID != NO_RECORD)
          {
            BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
            if(rcode2 == 0 && (BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE))
              strcat(tempString, szAccessible);
          }
//
//  Do we want to see the comment code?
//
          if(bCodes)
          {
            if(TRIPS.COMMENTSrecordID != NO_RECORD)
            {
              COMMENTSKey0.recordID = TRIPS.COMMENTSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
              if(rcode2 == 0 || rcode2 == 22)
              {
                strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
                trim(szarString, COMMENTS_CODE_LENGTH);
                strcat(tempString, szarString);
              }
            }
          }
          strcat(tempString, szSeparator);
//
//  Do we want to see the comment text?
//
          if(bText && TRIPS.COMMENTSrecordID != NO_RECORD)
          {
            for(bFound = FALSE, nL = 0; nL < numComments; nL++)
            {
              if(COMMENTSrecordIDs[nL] == TRIPS.COMMENTSrecordID)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              COMMENTSrecordIDs[numComments] = TRIPS.COMMENTSrecordID;
              numComments++;
            }
          }
//
//  We only want those times that exist on the selected pattern
//
          nL = 0;
          nM = 0;
          while(nL < numSelectedPatternNodes)
          {
            if(nM >= numTripPatternNodes)
            {
              break;
            }
            if(pTripPatternSeqs[nM] < pSelectedPatternSeqs[nL])
            {
              nM++;
              continue;
            }
            strcat(tempString, szSeparator);
            if(pSelectedPatternSeqs[nL] == pTripPatternSeqs[nM])
            {
              if(GTResults.tripTimes[nM] != NO_TIME)
              {
                if(GTResults.tripTimes[nM] < 43200L)
                {
                  strcat(tempString, szTimeA);
                }
                else
                {
                  strcat(tempString,  GTResults.tripTimes[nM] < 86400L ? szTimeP : szTimeX);
                }
                strcat(tempString, TcharNAP(GTResults.tripTimes[nM], bUseColons));
              }
              nM++;
            }
            nL++;
          }
          strcat(tempString, szEachNodeTrailer);
          strcat(tempString, "\r\n");
          _lwrite(hfOutputFile, tempString, strlen(tempString));
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
//
//  Dump out the code and text for any of the comments
//
        recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
        for(nL = 0; nL < numComments; nL++)
        {
          COMMENTSKey0.recordID = COMMENTSrecordIDs[nL];
          btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
          memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
          strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(tempString, COMMENTS_CODE_LENGTH);
          strcat(tempString, ": ");
          _lwrite(hfOutputFile, tempString, strlen(tempString));
          for(ptr = strtok(&pCommentText[COMMENTS_FIXED_LENGTH], "\r\n");
                ptr; ptr = strtok(NULL, "\r\n") )
          {
            strcpy(tempString, ptr);
            strcat(tempString, "\r\n");
            _lwrite(hfOutputFile, tempString, strlen(tempString));
          }
        }
        recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    timeFormat = timeFormatSave;
    TMSHeapFree(pSelectedPatternNodes);
    TMSHeapFree(pSelectedPatternSeqs);
    TMSHeapFree(pTripPatternNodes);
    TMSHeapFree(pTripPatternSeqs);
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}

//
//  TMSRPT02C() - Configuration Routine
//

BOOL CALLBACK TMSRPT02CMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlRSDHEADER;
  static HANDLE hCtlRSDTRAILER;
  static HANDLE hCtlINDNODESHEADER;
  static HANDLE hCtlINDNODESTRAILER;
  static HANDLE hCtlACCESSIBLE;
  static HANDLE hCtlSEPARATOR;
  static HANDLE hCtlMORNING;
  static HANDLE hCtlAFTERNOON;
  static HANDLE hCtlAFTERMIDNIGHT;
  static HANDLE hCtlAPX;
  static HANDLE hCtlUSECOLONS;
  static HANDLE hCtlUNLOADTRIPCOMMENTCODES;
  static HANDLE hCtlUNLOADTRIPCOMMENTTEXT;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      hCtlRSDHEADER = GetDlgItem(hWndDlg, RPTCFG02_RSDHEADER);
      hCtlRSDTRAILER = GetDlgItem(hWndDlg, RPTCFG02_RSDTRAILER);
      hCtlINDNODESHEADER = GetDlgItem(hWndDlg, RPTCFG02_INDNODESHEADER);
      hCtlINDNODESTRAILER = GetDlgItem(hWndDlg, RPTCFG02_INDNODESTRAILER);
      hCtlACCESSIBLE = GetDlgItem(hWndDlg, RPTCFG02_ACCESSIBLE);
      hCtlSEPARATOR = GetDlgItem(hWndDlg, RPTCFG02_SEPARATOR);
      hCtlMORNING = GetDlgItem(hWndDlg, RPTCFG02_MORNING);
      hCtlAFTERNOON = GetDlgItem(hWndDlg, RPTCFG02_AFTERNOON);
      hCtlAFTERMIDNIGHT = GetDlgItem(hWndDlg, RPTCFG02_AFTERMIDNIGHT);
      hCtlAPX = GetDlgItem(hWndDlg, RPTCFG02_APX);
      hCtlUSECOLONS = GetDlgItem(hWndDlg, RPTCFG02_USECOLONS);
      hCtlUNLOADTRIPCOMMENTCODES = GetDlgItem(hWndDlg, RPTCFG02_UNLOADTRIPCOMMENTCODES);
      hCtlUNLOADTRIPCOMMENTTEXT = GetDlgItem(hWndDlg, RPTCFG02_UNLOADTRIPCOMMENTTEXT);
      LoadHeadersAndTrailers02();
      SendMessage(hCtlRSDHEADER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szRSDHeader);
      SendMessage(hCtlRSDTRAILER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szRSDTrailer);
      SendMessage(hCtlINDNODESHEADER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szEachNodeHeader);
      SendMessage(hCtlINDNODESTRAILER, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szEachNodeTrailer);
      SendMessage(hCtlACCESSIBLE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szAccessible);
      SendMessage(hCtlSEPARATOR, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szSeparator);
      SendDlgItemMessage(hWndDlg, nTimeFormat, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlMORNING, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szTimeA);
      SendMessage(hCtlAFTERNOON, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szTimeP);
      SendMessage(hCtlAFTERMIDNIGHT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)szTimeX);
      SendMessage(hCtlUSECOLONS, BM_SETCHECK, (WPARAM)bUseColons, (LPARAM)0);
      SendMessage(hCtlUNLOADTRIPCOMMENTCODES, BM_SETCHECK, (WPARAM)bCodes, (LPARAM)0);
      SendMessage(hCtlUNLOADTRIPCOMMENTTEXT, BM_SETCHECK, (WPARAM)bText, (LPARAM)0);
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDOK:
          SendMessage(hCtlRSDHEADER, WM_GETTEXT, (WPARAM)sizeof(szRSDHeader), (LONG)(LPSTR)szRSDHeader);
          SendMessage(hCtlRSDTRAILER, WM_GETTEXT, (WPARAM)sizeof(szRSDTrailer), (LONG)(LPSTR)szRSDTrailer);
          SendMessage(hCtlINDNODESHEADER, WM_GETTEXT, (WPARAM)sizeof(szEachNodeHeader), (LONG)(LPSTR)szEachNodeHeader);
          SendMessage(hCtlINDNODESTRAILER, WM_GETTEXT, (WPARAM)sizeof(szEachNodeTrailer), (LONG)(LPSTR)szEachNodeTrailer);
          SendMessage(hCtlACCESSIBLE, WM_GETTEXT, (WPARAM)sizeof(szAccessible), (LONG)(LPSTR)szAccessible);
          SendMessage(hCtlSEPARATOR, WM_GETTEXT, (WPARAM)sizeof(szSeparator), (LONG)(LPSTR)szSeparator);
          if(SendMessage(hCtlAPX, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            nTimeFormat = RPTCFG02_APX;
          }
          else
          {
            nTimeFormat = RPTCFG02_MILITARY;
          }
          SendMessage(hCtlMORNING, WM_GETTEXT, (WPARAM)sizeof(szTimeA), (LONG)(LPSTR)szTimeA);
          SendMessage(hCtlAFTERNOON, WM_GETTEXT, (WPARAM)sizeof(szTimeP), (LONG)(LPSTR)szTimeP);
          SendMessage(hCtlAFTERMIDNIGHT, WM_GETTEXT, (WPARAM)sizeof(szTimeX), (LONG)(LPSTR)szTimeX);
          WritePrivateProfileString(szSection, "RSDHeader", szRSDHeader, TMSINIFile);
          WritePrivateProfileString(szSection, "RSDTrailer", szRSDTrailer, TMSINIFile);
          WritePrivateProfileString(szSection, "EachNodeHeader", szEachNodeHeader, TMSINIFile);
          WritePrivateProfileString(szSection, "EachNodeTrailer", szEachNodeTrailer, TMSINIFile);
          WritePrivateProfileString(szSection, "Accessible", szAccessible, TMSINIFile);
          WritePrivateProfileString(szSection, "Separator", szSeparator, TMSINIFile);
          itoa(nTimeFormat, tempString, 10);
          WritePrivateProfileString(szSection, "TimeFormat", tempString, TMSINIFile);
          WritePrivateProfileString(szSection, "TimeA", szTimeA, TMSINIFile);
          WritePrivateProfileString(szSection, "TimeP", szTimeP, TMSINIFile);
          WritePrivateProfileString(szSection, "TimeX", szTimeX, TMSINIFile);
          bUseColons = SendMessage(hCtlUSECOLONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          itoa(bUseColons, tempString, 10);
          WritePrivateProfileString(szSection, "UseColons", tempString, TMSINIFile);
          bCodes = SendMessage(hCtlUNLOADTRIPCOMMENTCODES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          itoa(bCodes, tempString, 10);
          WritePrivateProfileString(szSection, "UnloadTripCommentCodes", tempString, TMSINIFile);
          bText = SendMessage(hCtlUNLOADTRIPCOMMENTTEXT, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          itoa(bText, tempString, 10);
          WritePrivateProfileString(szSection, "UnloadTripCommentText", tempString, TMSINIFile);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

void LoadHeadersAndTrailers02(void)
{
  GetPrivateProfileString(szSection, "RSDHeader", "", szRSDHeader, sizeof(szRSDHeader), TMSINIFile);
  GetPrivateProfileString(szSection, "RSDTrailer", "", szRSDTrailer, sizeof(szRSDTrailer), TMSINIFile);
  GetPrivateProfileString(szSection, "EachNodeHeader", "", szEachNodeHeader, sizeof(szEachNodeHeader), TMSINIFile);
  GetPrivateProfileString(szSection, "EachNodeTrailer", "", szEachNodeTrailer, sizeof(szEachNodeTrailer), TMSINIFile);
  GetPrivateProfileString(szSection, "Accessible", "", szAccessible, sizeof(szAccessible), TMSINIFile);
  GetPrivateProfileString(szSection, "Separator", "", szSeparator, sizeof(szSeparator), TMSINIFile);
  nTimeFormat = GetPrivateProfileInt(szSection, "TimeFormat", RPTCFG02_APX, TMSINIFile);
  timeFormat = nTimeFormat == RPTCFG02_APX ? PREFERENCES_APX : PREFERENCES_MILITARY;
  bUseColons = GetPrivateProfileInt(szSection, "UseColons", 0, TMSINIFile); 
  GetPrivateProfileString(szSection, "TimeA", "", szTimeA, sizeof(szTimeA), TMSINIFile);
  GetPrivateProfileString(szSection, "TimeP", "", szTimeP, sizeof(szTimeP), TMSINIFile);
  GetPrivateProfileString(szSection, "TimeX", "", szTimeX, sizeof(szTimeX), TMSINIFile);
  bCodes = GetPrivateProfileInt(szSection, "UnloadTripCommentCodes", 0, TMSINIFile);
  bText = GetPrivateProfileInt(szSection, "UnloadTripCommentText", 0, TMSINIFile);
}
