//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include <math.h>

#ifdef  MAXBLOCKS
#undef  MAXBLOCKS
#endif
#define MAXBLOCKS  1000

typedef struct BSStruct
{
  long selectedBlock;
  long TRIPSrecordID;
  long time;
} BSDef;

int sort_BS(const void *a, const void *b)
{
  BSDef *pa, *pb;
  pa = (BSDef *)a;
  pb = (BSDef *)b;
  return(pa->time < pb->time ? -1 : pa->time > pb->time ? 1 : 0);
}

BOOL CALLBACK RENUMBLOCKSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlALLBLOCKS;
  static  HANDLE hCtlSPECIFICBLOCKS;
  static  HANDLE hCtlBLOCKLIST;
  static  HANDLE hCtlDONTSORT;
  static  HANDLE hCtlSORTBYPOTIME;
  static  HANDLE hCtlSORTBYPITIME;
  static  HANDLE hCtlSTARTAT;
  static  HANDLE hCtlINCREMENT;
  GetConnectionTimeDef GCTData;
  GenerateTripDef GTResults;
  BSDef   BS[MAXBLOCKS];
  float   distance;
  long    assignedToNODESrecordID;
  long    newBlockNumber;
  long    oldBlockNumber;
  long    increment;
  long    POTime;
  long    PITime;
  long    deadheadTime;
  long    blockNumber;
  long    lastTRIPSrecordID;
  int     selectedBlocks[MAXBLOCKS];
  int     numSelectedBlocks;
  int     numBlocks;
  int     nI;
  int     rcode2;
  HCURSOR saveCursor;
  short   int wmId;
  short   int wmEvent;
  static  BLOCKSDef *pTRIPSChunk;
  static  int keyNumber;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      keyNumber = pDI->fileInfo.keyNumber;
      pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
//
//  Set up handles to the controls
//
      hCtlALLBLOCKS = GetDlgItem(hWndDlg, RENUMBLOCKS_ALLBLOCKS);
      hCtlSPECIFICBLOCKS = GetDlgItem(hWndDlg, RENUMBLOCKS_SPECIFICBLOCKS);
      hCtlBLOCKLIST = GetDlgItem(hWndDlg, RENUMBLOCKS_BLOCKLIST);
      hCtlDONTSORT = GetDlgItem(hWndDlg, RENUMBLOCKS_DONTSORT);
      hCtlSORTBYPOTIME = GetDlgItem(hWndDlg, RENUMBLOCKS_SORTBYPOTIME);
      hCtlSORTBYPITIME = GetDlgItem(hWndDlg, RENUMBLOCKS_SORTBYPITIME);
      hCtlSTARTAT = GetDlgItem(hWndDlg, RENUMBLOCKS_STARTAT);
      hCtlINCREMENT = GetDlgItem(hWndDlg, RENUMBLOCKS_INCREMENT);
//
//  Default to a starting number of 1 with an increment of 1
//
      strcpy(tempString, "1");
      SendMessage(hCtlSTARTAT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlINCREMENT, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Default to "Don't Sort"
//
      SendMessage(hCtlDONTSORT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Get the blocks
//
     numBlocks = SetUpBlocksListbox(pDI, hCtlBLOCKLIST);
     if(numBlocks == NO_RECORD)
     {
       TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
     else if(numBlocks == 0)
     {
       TMSError(hWndDlg, MB_ICONSTOP, ERROR_107, (HANDLE)NULL);
       SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
       break;
     }
     SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  Comments
//
        case RENUMBLOCKS_BLOCKLIST:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlALLBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlALLBLOCKS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlSPECIFICBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;  
          }
          break;

        case RENUMBLOCKS_ALLBLOCKS:
          SendMessage(hCtlBLOCKLIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
          break;

        case RENUMBLOCKS_SPECIFICBLOCKS:
          SendMessage(hCtlBLOCKLIST, LB_SETSEL, (WPARAM)TRUE, (LPARAM)0);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Renumbering_Standard_Blocks);
          break;
//
//  IDOK
//
        case IDOK:
//
// The new block number can't be blank
//
          SendMessage(hCtlSTARTAT, WM_GETTEXT, TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0 || (newBlockNumber = atol(tempString)) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_061, hCtlSTARTAT);
            break;
          }
//
//  If the increment is blank, zero, or less than zero, set it to 1
//
          SendMessage(hCtlINCREMENT, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0 || (increment = atol(tempString)) <= 0)
            increment = 1;
//
//  If specific block is selected, at least one of the entries
//  in the blocks list box has to be selected
//
          if(SendMessage(hCtlSPECIFICBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0) &&
                SendMessage(hCtlBLOCKLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_120, hCtlBLOCKLIST);
            break;
          }
//
//  If none are highlighted, highlight 'em all.
//
          if(SendMessage(hCtlBLOCKLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0) == 0)
            SendMessage(hCtlBLOCKLIST, LB_SETSEL, TRUE, MAKELPARAM(-1, 0));
          numSelectedBlocks = SendMessage(hCtlBLOCKLIST, LB_GETSELITEMS,
                (WPARAM)MAXBLOCKS, (LPARAM)(int FAR *)selectedBlocks);
//
//  Is he sorting?
//
          if(!SendMessage(hCtlDONTSORT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
//
//  Set up the internal array
//
            for(nI = 0; nI < numSelectedBlocks; nI++)
            {
              BS[nI].selectedBlock = selectedBlocks[nI];
              BS[nI].TRIPSrecordID = SendMessage(hCtlBLOCKLIST, LB_GETITEMDATA,
                  (WPARAM)selectedBlocks[nI], (LPARAM)0);
              TRIPSKey0.recordID = BS[nI].TRIPSrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Sort by PO Time
//
              if(SendMessage(hCtlSORTBYPOTIME, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
                {
                  POTime = GTResults.firstNodeTime;
                }
                else
                {
                  GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                  GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                  GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                  GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
                  GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
                  GCTData.timeOfDay = GTResults.firstNodeTime;
                  deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                  distance = (float)fabs((double)distance);
                  if(deadheadTime != NO_TIME)
                  {
                    POTime = GTResults.firstNodeTime - deadheadTime;
                  }
                }
                BS[nI].time = POTime;
              }
//
//  Sort by PI Time
//
              else
              {
                rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
                blockNumber = pTRIPSChunk->blockNumber;
                assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
                lastTRIPSrecordID = NO_RECORD;
                while(rcode2 == 0 &&
                      pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                      pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                      pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                      pTRIPSChunk->blockNumber == blockNumber)
                {
                  lastTRIPSrecordID = TRIPS.recordID;
                  rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
                }
                PITime = NO_TIME;
                if(lastTRIPSrecordID != NO_RECORD)
                {
                  TRIPSKey0.recordID = lastTRIPSrecordID;
                  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                  GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                        TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                        TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
                  if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
                  {
                    PITime = GTResults.lastNodeTime;
                  }
                  else
                  {
                    GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
                    GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
                    GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
                    GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
                    GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
                    GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
                    GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
                    GCTData.timeOfDay = GTResults.lastNodeTime;
                    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                    distance = (float)fabs((double)distance);
                    if(deadheadTime != NO_TIME)
                    {
                      PITime = GTResults.lastNodeTime + deadheadTime;
                    }
                  }
                }
                BS[nI].time = PITime;
              }
            }
//
//  Sort the structure
//
            qsort((void *)BS, numSelectedBlocks, sizeof(BSDef), sort_BS);
//
//  Reassign the values in selectedBlocks
//
            for(nI = 0; nI < numSelectedBlocks; nI++)
            {
              selectedBlocks[nI] = BS[nI].selectedBlock;
            }
          }
//
//  Loop through the blocks and make the new assignment
//
          saveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < numSelectedBlocks; nI++)
          {
            newBlockNumber += (nI == 0 ? 0 : increment);
//
//  If a proposed block number exists, try the next one
//
            TRIPSKey0.recordID = SendMessage(hCtlBLOCKLIST, LB_GETITEMDATA,
                  (WPARAM)selectedBlocks[nI], (LPARAM)0);
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            assignedToNODESrecordID = pTRIPSChunk->assignedToNODESrecordID;
            oldBlockNumber = pTRIPSChunk->blockNumber;
            TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
            TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            TRIPSKey2.blockNumber = newBlockNumber;
            TRIPSKey2.blockSequence = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            while(rcode2 == 0)
            {
              if(pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                    pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                    pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                    pTRIPSChunk->blockNumber == newBlockNumber)
              {
                newBlockNumber += increment;
                TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
                TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
                TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
                TRIPSKey2.blockNumber = newBlockNumber;
                TRIPSKey2.blockSequence = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              }
              else
                break;
            }
//
//  Ok, we know here that we have a unique block number
//
            TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
            TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            TRIPSKey2.blockNumber = oldBlockNumber;
            TRIPSKey2.blockSequence = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
            while(rcode2 == 0)
            {
              if(pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
                   pTRIPSChunk->RGRPROUTESrecordID == pDI->fileInfo.routeRecordID &&
                   pTRIPSChunk->SGRPSERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                   pTRIPSChunk->blockNumber == oldBlockNumber)
              { 
                GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,             // Some older versions of TMS didn't set
                      TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,                // the blockSequence correctly.  This
                      TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);   // fixes the problem by forcing the
                pTRIPSChunk->blockSequence = GTResults.firstNodeTime;                  // issue on a renumber.
                pTRIPSChunk->blockNumber = newBlockNumber;
                btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
                TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
                TRIPSKey2.RGRPROUTESrecordID = pDI->fileInfo.routeRecordID;
                TRIPSKey2.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
                TRIPSKey2.blockNumber = oldBlockNumber;
                TRIPSKey2.blockSequence = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
              }
              else
                break;
            }
          }
          SetCursor(saveCursor);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}