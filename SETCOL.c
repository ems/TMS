//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#define TABSET 5
#define DELETED_STRING "* Deleted *"

void BuildString(FIELDDEFDef *pFD)
{
  sprintf(tempString, "%3d\t", pFD->sequence);
  strcat(tempString, pFD->szFieldName);
  if(strcmp(pFD->szFieldName, DELETED_STRING) == 0)
    strcat(tempString, "\t \t \t \t ");
  else
  {
    strcat(tempString, "\t");
    strcat(tempString, pFD->szDisplayedName);
    strcat(tempString, "\t");
    strcat(tempString, pFD->hidden ? "Y" : "N");
    strcat(tempString, "\t");
    strcat(tempString, pFD->derived ? "Y" : "N");
    strcat(tempString, "\t");
    strcat(tempString, pFD->formatted ? "Y" : "N");
  }
}

void ParseString(FIELDDEFDef *pFD)
{
  char *ptr;
  strcpy(szarString, strtok(tempString, "\t"));
  pFD->sequence = atoi(szarString);
  strcpy(pFD->szFieldName, strtok(NULL, "\t"));
  strcpy(pFD->szDisplayedName, strtok(NULL, "\t"));
  ptr = strtok(NULL, "\t");
  pFD->hidden = (BOOL)(*ptr == 'Y');
  ptr = strtok(NULL, "\t");
  pFD->derived = (BOOL)(*ptr == 'Y');
  ptr = strtok(NULL, "\t");
  pFD->formatted = (BOOL)(*ptr == 'Y');
}

BOOL CALLBACK SETCOLMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlLISTBOXTITLE;
  static HANDLE hCtlLISTBOX;
  static HANDLE hCtlIDADD;
  static HANDLE hCtlIDREMOVE;
  static HANDLE hCtlIDUP;
  static HANDLE hCtlIDDOWN;
  static HANDLE hCtlIDUPDATE;
  static WORD   ThisType;
  static BOOL   bUpdatedOnce = FALSE;
  static BOOL   bCreated;
  static char   szDDFFileName[64];
  static int    tabPos[TABSET] = {5, 28, 50, 53, 56};
  FIELDDEFDef FIELDDEF;
  WORD DlgWidthUnits;
  BOOL bRC;
  BOOL bDeleted;
  long tempLong;
  int  adjustedTabPos[TABSET];
  int  rcode2;
  int  nI;
  int  nJ;
  int  seqNum;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlLISTBOXTITLE = GetDlgItem(hWndDlg, SETCOL_LISTBOXTITLE);
      hCtlLISTBOX = GetDlgItem(hWndDlg, SETCOL_LISTBOX);
      hCtlIDADD = GetDlgItem(hWndDlg, IDADD);
      hCtlIDREMOVE = GetDlgItem(hWndDlg, IDREMOVE);
      hCtlIDUP = GetDlgItem(hWndDlg, IDUP);
      hCtlIDDOWN = GetDlgItem(hWndDlg, IDDOWN);
      hCtlIDUPDATE = GetDlgItem(hWndDlg, IDUPDATE);
//
//  Set the tab stops in the listbox and listbox title
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nI = 0; nI < TABSET; nI++)
      {
        adjustedTabPos[nI] = (DlgWidthUnits * tabPos[nI] * 2);
      }
      SendMessage(hCtlLISTBOXTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      LoadString(hInst, TEXT_104, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlLISTBOXTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlLISTBOX, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  Determine who we're dealing with, and do the standard tables
//
      ThisType = (WORD)lParam;
      bCreated = ThisType == TMS_STANDARDBLOCKS || ThisType == TMS_DROPBACKBLOCKS ||
            ThisType == TMS_RUNS || ThisType == TMS_ROSTER || ThisType == TMS_TIMECHECKS;
      seqNum = 0;
      if(!bCreated)
      {
//
//  Now list out the fields that are defined for this file
//
        FIELDDDFKey1.File = DDFFileNumbers[ThisType];
        rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
        while(rcode2 == 0 && FIELDDDF.File == DDFFileNumbers[ThisType])
        {
//
//  Find out if there's an entry for it in ATTRIBUTES
//
          ATTRIBUTESKey0.Id = FIELDDDF.Id;
          rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
//
//  Sequence number - the 0 check is for backward compatibility
//
          if((rcode2 == 0 && ATTRIBUTES.sequence == 0) || rcode2 != 0)
            ATTRIBUTES.sequence = ++seqNum;
          sprintf(tempString, "%3d\t", ATTRIBUTES.sequence);
//
//  Field name
//
          strncpy(szarString, FIELDDDF.Name, sizeof(FIELDDDF.Name));
          trim(szarString, sizeof(FIELDDDF.Name));
          if(strcmp(szarString, "") == 0)
            strcpy(szarString, " ");
          strcat(tempString, szarString);
          strcat(tempString, "\t");
//
//  No attributes - Default the fieldName to the fieldTitle.
//
          if(rcode2 != 0)
            ATTRIBUTES.flags = 0;
//
//  Got the attributes record
//
          else
          {
            strncpy(szarString, ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
            trim(szarString, ATTRIBUTES_FIELDTITLE_LENGTH);
          }
//
//  Build the rest of the row on the listbox
//
          strcat(tempString, szarString);
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_HIDDEN ? "Y" : "N");
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_DERIVED ? "Y" : "N");
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_FORMATTED ? "Y" : "N");
          nI = (int)SendMessage(hCtlLISTBOX, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LONG)FIELDDDF.Id);
          rcode2 = btrieve(B_GETNEXT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
        }
//
//  Disable ADD and DELETE since he's not on Blocks, Runs, nor the Roster
//
        EnableWindow(hCtlIDADD, FALSE);
        EnableWindow(hCtlIDREMOVE, FALSE);
//
//  Get stuff ready for the window title
//
        FILEDDFKey0.Id = DDFFileNumbers[ThisType];
        btrieve(B_GETEQUAL, FILE_DDF, &FILEDDF, &FILEDDFKey0, 0);
        strncpy(szDDFFileName, FILEDDF.Name, sizeof(FILEDDF.Name));
        trim(szDDFFileName, sizeof(FILEDDF.Name));
      }
//
//  Ok, now deal with the Blocks, Runs, Roster, and Timechecks
//
      else
      {
        EnableWindow(hCtlIDADD, TRUE);
        if(ThisType == TMS_STANDARDBLOCKS)
          strcpy(TABLEVIEWKey1.Name, VIEWNAME_STANDARDBLOCKS);
        else if(ThisType == TMS_DROPBACKBLOCKS)
          strcpy(TABLEVIEWKey1.Name, VIEWNAME_DROPBACKBLOCKS);
        else if(ThisType == TMS_RUNS)
          strcpy(TABLEVIEWKey1.Name, VIEWNAME_RUNS);
        else if(ThisType == TMS_ROSTER)
          strcpy(TABLEVIEWKey1.Name, VIEWNAME_ROSTER);
        else if(ThisType == TMS_TIMECHECKS)
          strcpy(TABLEVIEWKey1.Name, VIEWNAME_TIMECHECKS);
        else
        {
          TMSError(hWndDlg, MB_ICONSTOP, ERROR_304, (HANDLE)NULL);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
          break;
        }
        pad(TABLEVIEWKey1.Name, TABLEVIEW_NAME_LENGTH);
        btrieve(B_GETEQUAL, TMS_TABLEVIEW, &TABLEVIEW, &TABLEVIEWKey1, 1);
//
//  Go through the fields that are defined for this file
//
        CREATEDKey1.File = TABLEVIEW.Id;
        CREATEDKey1.Sequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
        while(rcode2 == 0 && CREATED.File == TABLEVIEW.Id)
        {
//
//  Find out if there's an entry for it in ATTRIBUTES,
//
          ATTRIBUTESKey0.Id = CREATED.Id;
          rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
//
//  Sequence number - the 0 check is for backward compatibility
//
          if((rcode2 == 0 && ATTRIBUTES.sequence == 0) || rcode2 != 0)
            ATTRIBUTES.sequence = ++seqNum;
          sprintf(tempString, "%3d\t", ATTRIBUTES.sequence);
//
//  Did this come from somewhere else?
//
          if(CREATED.FromFileId != NO_RECORD || CREATED.FromFieldId != NO_RECORD)
          {
            FIELDDDFKey0.Id = CREATED.FromFieldId;
            btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
            strncpy(szarString, FIELDDDF.Name, sizeof(FIELDDDF.Name));
            trim(szarString, sizeof(FIELDDDF.Name));
            strcat(tempString, szarString);
            strcat(tempString, "\t");
          }
          else
            strcat(tempString, "TMS-Generated\t");
          strncpy(szarString, CREATED.Name, CREATED_NAME_LENGTH);
          trim(szarString, CREATED_NAME_LENGTH);
//
//  Nope - Default the fieldName to the fieldTitle.
//
          if(rcode2 != 0)
            ATTRIBUTES.flags = 0;
//
//  Got the attributes record
//
          else
          {
            strncpy(szarString, ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
            trim(szarString, ATTRIBUTES_FIELDTITLE_LENGTH);
          }
//
//  Build the rest of the row on the listbox
//
          if(strcmp(szarString, "") == 0)
            strcpy(szarString, " ");
          strcat(tempString, szarString);
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_HIDDEN ? "Y" : "N");
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_DERIVED ? "Y" : "N");
          strcat(tempString, "\t");
          strcat(tempString, ATTRIBUTES.flags & ATTRIBUTES_FLAG_FORMATTED ? "Y" : "N");
          nI = (int)SendMessage(hCtlLISTBOX, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
          btrieve(B_GETPOSITION, TMS_CREATED, &tempLong, &CREATEDKey1, 1);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
          rcode2 = btrieve(B_GETNEXT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
        }
//
//  Get stuff ready for the window title
//
        strncpy(szDDFFileName, TABLEVIEW.Name, TABLEVIEW_NAME_LENGTH);
        trim(szDDFFileName, TABLEVIEW_NAME_LENGTH);
      }
//
//  Set the window title
//
      strcpy(tempString, szDDFFileName);
      strcat(tempString, ": Column Characteristics");
      SetWindowText(hWndDlg, (LPCSTR)tempString);
//
//  Diable the Update, Move Up, and Move Down buttons seeing as nothing's selected
//
      EnableWindow(hCtlIDUPDATE, FALSE);
      EnableWindow(hCtlIDUP, FALSE);
      EnableWindow(hCtlIDDOWN, FALSE);
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      EndDialog(hWndDlg, FALSE);
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
//  SETCOL_LISTBOXTITLE
//
        case SETCOL_LISTBOXTITLE:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlLISTBOXTITLE, LB_GETSEL, (WPARAM)0, (LPARAM)0))
                SendMessage(hCtlLISTBOXTITLE, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
              SetFocus(hCtlLISTBOX);
              break;
          }
          break;
//
//  SETCOL_LISTBOX
//
        case SETCOL_LISTBOX:     // List box
          switch(wmEvent)
          {
            case LBN_DBLCLK:
              SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDUPDATE, 0), (LPARAM)0);
              break;

            case LBN_SELCHANGE:
              nI = (int)SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nI == LB_ERR)
              {
                EnableWindow(hCtlIDUPDATE, FALSE);
                EnableWindow(hCtlIDUP, FALSE);
                EnableWindow(hCtlIDDOWN, FALSE);
                EnableWindow(hCtlIDREMOVE, FALSE);
              }
              else
              {
                SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
                strcat(tempString, "\tJunk\tJunk\tJunk"); // in case some crap slid in
                ParseString(&FIELDDEF);
                nJ = SendMessage(hCtlLISTBOX, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                if(bCreated)
                {
                  bDeleted = strcmp(FIELDDEF.szFieldName, DELETED_STRING) == 0;
                  EnableWindow(hCtlIDUPDATE, !bDeleted);
                  EnableWindow(hCtlIDUP, nI > 0 && !bDeleted);
                  EnableWindow(hCtlIDDOWN, nI != nJ - 1 && !bDeleted);
                  EnableWindow(hCtlIDREMOVE, nJ != 0 && !bDeleted);
                }
                else
                {
                  EnableWindow(hCtlIDUPDATE, TRUE);
                  EnableWindow(hCtlIDUP, nI > 1);
                  EnableWindow(hCtlIDDOWN, (nI != nJ - 1 && nI != 0));
                }
              }
              break;
          }
          break;
//
//  IDUPDATE
//
        case IDUPDATE:
          nI = (int)SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
          FIELDDEF.bCreated = bCreated;
          strcpy(FIELDDEF.szFileName, szDDFFileName);
          SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          strcat(tempString, "\tJunk\tJunk\tJunk"); // in case some crap slid in
          ParseString(&FIELDDEF);
          if(bCreated)
          {
            tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            memcpy(&CREATED, &tempLong, sizeof(tempLong));
            rcode2 = btrieve(B_GETDIRECT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
            FIELDDEF.Id = CREATED.Id;
            FIELDDEF.createdFromFileId = CREATED.FromFileId;
            FIELDDEF.createdFromFieldId = CREATED.FromFieldId;
            bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DATAFIELD), hWndDlg,
                  (DLGPROC)DATAFIELDMsgProc, (LPARAM)&FIELDDEF);
            if(bRC)
            {
              nJ = SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(nJ != LB_ERR)
              {
                tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
                CREATEDKey1.File = LOWORD(tempLong);
                CREATEDKey1.Sequence = HIWORD(tempLong);
                btrieve(B_GETEQUAL, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
                strncpy(CREATED.Name, FIELDDEF.szDisplayedName, DDF_NAME_LENGTH);
                pad(CREATED.Name, DDF_NAME_LENGTH);
                CREATED.FromFileId = FIELDDEF.createdFromFileId;
                CREATED.FromFieldId = FIELDDEF.createdFromFieldId;
                btrieve(B_UPDATE, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
                BuildString(&FIELDDEF);
                SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
                SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
                SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
                bUpdatedOnce = TRUE;
              }
            }
          }
//
//  Not bCreated
//
          else
          {
            FIELDDEF.Id = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
            FIELDDEF.fromFile = NO_RECORD;
            FIELDDEF.fromField = NO_RECORD;
            bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FIELDDEF), hWndDlg,
                  (DLGPROC)FIELDDEFMsgProc, (LPARAM)&FIELDDEF);
//
//  If he said OK, rebuild the listbox line
//
            if(bRC)
            {
              BuildString(&FIELDDEF);
              SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
              SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
              SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)FIELDDEF.Id);
              bUpdatedOnce = TRUE;
            }
          }
          break;
//
//  IDADD (Only available for Blocks, Runs, and the Roster)
//
        case IDADD:
          memset(&FIELDDEF, 0x00, sizeof(FIELDDEF));
          FIELDDEF.Id = NO_RECORD;
          strncpy(FIELDDEF.szFileName, TABLEVIEW.Name, DDF_NAME_LENGTH);
          trim(FIELDDEF.szFileName, DDF_NAME_LENGTH);
          strcpy(FIELDDEF.szFieldName, "New");
          FIELDDEF.bCreated = TRUE;
          bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DATAFIELD), hWndDlg,
                (DLGPROC)DATAFIELDMsgProc, (LPARAM)&FIELDDEF);
          if(bRC)
          {
            nI = SendMessage(hCtlLISTBOX, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            if(nI == 0)
              CREATED.Sequence = 0;
            else
            {
              tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)(nI - 1), (LPARAM)0);
              CREATEDKey1.File = LOWORD(tempLong);
              CREATEDKey1.Sequence = HIWORD(tempLong);
              btrieve(B_GETEQUAL, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
              CREATED.Sequence++;
            }
            CREATED.Id = FIELDDEF.Id;
            CREATED.File = TABLEVIEW.Id;
            strncpy(CREATED.Name, FIELDDEF.szDisplayedName, DDF_NAME_LENGTH);
            pad(CREATED.Name, DDF_NAME_LENGTH);
            CREATED.FromFileId = FIELDDEF.createdFromFileId;
            CREATED.FromFieldId = FIELDDEF.createdFromFieldId;
            memset(CREATED.reserved, 0x00, CREATED_RESERVED_LENGTH);
            CREATED.flags = 0;
            btrieve(B_INSERT, TMS_CREATED, &CREATED, &CREATEDKey0, 0);
            FIELDDEF.sequence = (int)SendMessage(hCtlLISTBOX, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            FIELDDEF.sequence++;
            BuildString(&FIELDDEF);
            nI = SendMessage(hCtlLISTBOX, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
            btrieve(B_GETPOSITION, TMS_CREATED, &tempLong, &CREATEDKey0, 0);
            SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
            bUpdatedOnce = TRUE;
          }
          break;
//
//  IDREMOVE
//
        case IDREMOVE:
          nI = (int)SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
          tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          memcpy(&CREATED, &tempLong, sizeof(tempLong));
          rcode2 = btrieve(B_GETDIRECT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
          if(rcode2 == 0)
          {
            SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
            ParseString(&FIELDDEF);
            SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
//
//  Note the "\t " series below.  Keep them there so ParseString doesn't blow up.
//
            sprintf(tempString, "%3d\t%s\t \t \t \t \t ", FIELDDEF.sequence, DELETED_STRING);
            SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
            SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
          }
          EnableWindow(hCtlIDREMOVE, FALSE);
          break;
//
//  IDUP
//
        case IDUP:
          nI = (int)SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
          SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          ParseString(&FIELDDEF);
          FIELDDEF.sequence--;
          BuildString(&FIELDDEF);
          SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          nJ = nI - 1;
          SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nJ, (LONG)(LPSTR)tempString);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)tempLong);
          SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          ParseString(&FIELDDEF);
          FIELDDEF.sequence++;
          BuildString(&FIELDDEF);
          SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
          SendMessage(hCtlLISTBOX, LB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(SETCOL_LISTBOX, LBN_SELCHANGE), (LPARAM)0);
          break;
//
//  IDDOWN
//
        case IDDOWN:
          nI = (int)SendMessage(hCtlLISTBOX, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == LB_ERR)
            break;
          SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          ParseString(&FIELDDEF);
          FIELDDEF.sequence++;
          BuildString(&FIELDDEF);
          SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          nJ = nI + 1;
          SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nJ, (LONG)(LPSTR)tempString);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)tempLong);
          SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
          tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          ParseString(&FIELDDEF);
          FIELDDEF.sequence--;
          BuildString(&FIELDDEF);
          SendMessage(hCtlLISTBOX, LB_DELETESTRING, (WPARAM)nI, (LPARAM)0);
          SendMessage(hCtlLISTBOX, LB_INSERTSTRING, (WPARAM)nI, (LONG)(LPSTR)tempString);
          SendMessage(hCtlLISTBOX, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)tempLong);
          SendMessage(hCtlLISTBOX, LB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
          SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(SETCOL_LISTBOX, LBN_SELCHANGE), (LPARAM)0);
          break;
//
//  IDOK - Process the list box and leave.  There are two separate paths
//         to consider here: bCreated and not bCreated.
//
        case IDOK:
          nJ = (int)SendMessage(hCtlLISTBOX, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
//
//  bCreated
//
          if(bCreated)
          {
            for(nI = 0; nI < nJ; nI++)
            {
              SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
              tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              memcpy(&CREATED, &tempLong, sizeof(tempLong));
              rcode2 = btrieve(B_GETDIRECT, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
              ParseString(&FIELDDEF);
              if(strcmp(FIELDDEF.szFieldName, DELETED_STRING) == 0)
              {
                btrieve(B_DELETE, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
                ATTRIBUTESKey0.Id = CREATED.Id;
                rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
                if(rcode2 == 0)
                  btrieve(B_DELETE, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
              }
              else
              {
                CREATED.Sequence = FIELDDEF.sequence;
                btrieve(B_UPDATE, TMS_CREATED, &CREATED, &CREATEDKey1, 1);
              }
            }
          }
//
//  not bCreated
//
          else
          {
            for(nI = 0; nI < nJ; nI++)
            {
              SendMessage(hCtlLISTBOX, LB_GETTEXT, (WPARAM)nI, (LONG)(LPSTR)tempString);
              ParseString(&FIELDDEF);
              tempLong = SendMessage(hCtlLISTBOX, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              ATTRIBUTESKey0.Id = (short int)tempLong;
              rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
              if(rcode2 == 0)
              {
                ATTRIBUTES.sequence = nI + 1;
                btrieve(B_UPDATE, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
              }
              else
              {
                memset(&ATTRIBUTES, 0x00, sizeof(ATTRIBUTES));
                ATTRIBUTES.Id = (short int)tempLong;
                strcpy(ATTRIBUTES.fieldTitle, FIELDDEF.szFieldName);
                pad(ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
                ATTRIBUTES.derivedTableId = NO_RECORD;
                ATTRIBUTES.derivedFieldId = NO_RECORD;
                ATTRIBUTES.formattedDataType = NO_RECORD;
                ATTRIBUTES.sequence = nI + 1;
                btrieve(B_INSERT, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
              }
            }
          }
//
//  All done
//
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDCANCEL - The return relies upon any updates in FIELDDEF and DATAFIELD
//
        case IDCANCEL:
          EndDialog(hWndDlg, bUpdatedOnce);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Setting_Column_Widths);
          break;
     }
     break;

   default:
     return FALSE;
 }
 return TRUE;
}
