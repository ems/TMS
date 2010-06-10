//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK FIELDDEFMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlDISPLAYEDNAME;
  static HANDLE hCtlHIDDENNO;
  static HANDLE hCtlHIDDENYES;
  static HANDLE hCtlDERIVEDNO;
  static HANDLE hCtlDERIVEDYES;
  static HANDLE hCtlFORMATTEDNO;
  static HANDLE hCtlFORMATTEDYES;
  static HANDLE hCtlFROMTABLE;
  static HANDLE hCtlFROMFIELD;
  static HANDLE hCtlAS;
  static PFIELDDEFDef pFIELDDEF;
  char  fieldName[DDF_NAME_LENGTH + 1];
  char  *formatTypes[] = {TMSDATA_TIMEOFDAY_TEXT, TMSDATA_TIMEINMINUTES_TEXT,
        TMSDATA_TIMEINHM_TEXT, TMSDATA_CHARACTER_TEXT, TMSDATA_INTEGER_TEXT,
        TMSDATA_FLOAT_TEXT, TMSDATA_LOGICAL_TEXT, TMSDATA_PATTERNCHECKS_TEXT,
        TMSDATA_PATTERNNODES_TEXT, TMSDATA_PULLOUTTIME_TEXT, TMSDATA_FIRSTNODE_TEXT,
        TMSDATA_FIRSTTIME_TEXT, TMSDATA_LASTTIME_TEXT, TMSDATA_LASTNODE_TEXT,
        TMSDATA_PULLINTIME_TEXT, TMSDATA_TRIPTIME_TEXT, TMSDATA_LAYOVERTIME_TEXT,
        TMSDATA_DEADHEADTIME_TEXT, TMSDATA_RUNTYPE_TEXT, TMSDATA_BLOCKNUMBER_TEXT,
        TMSDATA_RUNONTIME_TEXT, TMSDATA_RUNOFFTIME_TEXT, TMSDATA_PLATFORMTIME_TEXT,
        TMSDATA_REPORTTIME_TEXT, TMSDATA_TURNINTIME_TEXT, TMSDATA_REPORTTURNIN_TEXT,
        TMSDATA_TRAVELTIME_TEXT, TMSDATA_SPREADOT_TEXT, TMSDATA_MAKEUPTIME_TEXT,
        TMSDATA_OVERTIME_TEXT, TMSDATA_PREMIUMTIME_TEXT, TMSDATA_PAYTIME_TEXT,
        TMSDATA_BARGRAPH_TEXT, TMSDATA_FIXEDFOUR_TEXT, TMSDATA_TRIPDISTANCE_TEXT,
        TMSDATA_CUMULATIVEDISTANCE_TEXT, TMSDATA_ROSTERDAY1_TEXT, TMSDATA_ROSTERDAY2_TEXT,
        TMSDATA_ROSTERDAY3_TEXT, TMSDATA_ROSTERDAY4_TEXT, TMSDATA_ROSTERDAY5_TEXT,
        TMSDATA_ROSTERDAY6_TEXT, TMSDATA_ROSTERDAY7_TEXT, TMSDATA_ROSTERHOURS_TEXT,
        TMSDATA_ROSTEROT_TEXT, TMSDATA_ROSTERPAY_TEXT, TMSDATA_MEAL_TEXT, TMSDATA_FLAGS_TEXT,
        TMSDATA_DATE_TEXT, TMSDATA_ROSTERDRIVER_TEXT, TMSDATA_PAIDBREAKS_TEXT,
        TMSDATA_STARTOFPIECETRAVEL_TEXT, TMSDATA_ENDOFPIECETRAVEL_TEXT,
        TMSDATA_REPORTATTIME_TEXT, TMSDATA_REPORTATLOCATION_TEXT, TMSDATA_BOPEXTRABOARDSTART_TEXT,
        TMSDATA_BOPEXTRABOARDEND_TEXT, TMSDATA_EOPEXTRABOARDSTART_TEXT,
        TMSDATA_EOPEXTRABOARDEND_TEXT, TMSDATA_TIMEINDECIMALMINUTES_TEXT,
        TMSDATA_PIECESTARTLOCATION_TEXT, TMSDATA_PIECEENDLOCATION_TEXT,
        TMSDATA_SIGNOFFTIME_TEXT, TMSDATA_DAYOFTIMECHECK_TEXT, TMSDATA_ROUTEDIRECTION_TEXT,
        TMSDATA_SCHEDULEDTIME_TEXT, TMSDATA_SUPERVISOR_TEXT, TMSDATA_PASSENGERLOAD_TEXT,
        TMSDATA_WEATHER_TEXT, TMSDATA_OTHERCOMMENTS_TEXT, TMSDATA_TRAFFIC_TEXT,
        TMSDATA_ACTSCHEDDIFF_TEXT, TMSDATA_SPREADTIME_TEXT, TMSDATA_ROSTERPAYNOOT_TEXT,
        TMSDATA_SERVICEOFTRIPONRUN_TEXT};
  BOOL  bHidden;
  BOOL  bDerived;
  BOOL  bFormatted;
  int   nI;
  int   nJ;
  int   rcode2;
  int   selectedTable;
  int   fieldIndex;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  See who we are
//
      if((pFIELDDEF = (PFIELDDEFDef)lParam) == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to some controls
//
      hCtlDISPLAYEDNAME = GetDlgItem(hWndDlg, FIELDDEF_DISPLAYEDNAME);
      hCtlHIDDENNO = GetDlgItem(hWndDlg, FIELDDEF_HIDDENNO);
      hCtlHIDDENYES = GetDlgItem(hWndDlg, FIELDDEF_HIDDENYES);
      hCtlDERIVEDNO = GetDlgItem(hWndDlg, FIELDDEF_DERIVEDNO);
      hCtlDERIVEDYES = GetDlgItem(hWndDlg, FIELDDEF_DERIVEDYES);
      hCtlFORMATTEDNO = GetDlgItem(hWndDlg, FIELDDEF_FORMATTEDNO);
      hCtlFORMATTEDYES = GetDlgItem(hWndDlg, FIELDDEF_FORMATTEDYES);
      hCtlFROMTABLE = GetDlgItem(hWndDlg, FIELDDEF_FROMTABLE);
      hCtlFROMFIELD = GetDlgItem(hWndDlg, FIELDDEF_FROMFIELD);
      hCtlAS = GetDlgItem(hWndDlg, FIELDDEF_AS);
//
//  Set the window title
//
      strcpy(tempString, pFIELDDEF->szFileName);
      strcat(tempString, ": ");
      strcat(tempString, pFIELDDEF->szFieldName);
      strcat(tempString, " Field Definition");
      SetWindowText(hWndDlg, tempString);
//
//  Set the "Displayed Name" edit control
//
      SendMessage(hCtlDISPLAYEDNAME, WM_SETTEXT,
            (WPARAM)0, (LONG)(LPSTR)pFIELDDEF->szDisplayedName);
//
//  See if there are any attributes set for the field
//
      ATTRIBUTESKey0.Id = pFIELDDEF->Id;
      rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
      if(rcode2 == 0)
      {
        bHidden = ATTRIBUTES.flags & ATTRIBUTES_FLAG_HIDDEN;
        bDerived = ATTRIBUTES.flags & ATTRIBUTES_FLAG_DERIVED;
        bFormatted = ATTRIBUTES.flags & ATTRIBUTES_FLAG_FORMATTED;
        strncpy(tempString, ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
        trim(tempString, ATTRIBUTES_FIELDTITLE_LENGTH);
        DERIVED.joinedToTableId = ATTRIBUTES.derivedTableId;
        DERIVED.joinedToFieldId = ATTRIBUTES.derivedFieldId;
        FORMATTED.dataType = ATTRIBUTES.formattedDataType;
      }
      else
      {
        bHidden = FALSE;
        bDerived = FALSE;
        bFormatted = FALSE;
        strcpy(tempString, "");
        DERIVED.joinedToTableId = NO_RECORD;
        DERIVED.joinedToFieldId = NO_RECORD;
        FORMATTED.dataType = NO_RECORD;
      }
//
//  Set the check boxes
//
      SendMessage(bHidden ? hCtlHIDDENYES : hCtlHIDDENNO,
            BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(bDerived ? hCtlDERIVEDYES : hCtlDERIVEDNO,
            BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(bFormatted ? hCtlFORMATTEDYES : hCtlFORMATTEDNO,
            BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Set up the comboboxes and listboxes
//
//  Table names
//
      rcode2 = btrieve(B_GETFIRST, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
      while(rcode2 == 0)
      {
        strncpy(tempString, FILEDDF.Name, sizeof(FILEDDF.Name));
        trim(tempString, sizeof(FILEDDF.Name));
        nI = (int)SendMessage(hCtlFROMTABLE, CB_ADDSTRING,
               (WPARAM)0, (LONG)(LPSTR)tempString);
        if(FILEDDF.Id == DERIVED.joinedToTableId)
          SendMessage(hCtlFROMTABLE, CB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
        SendMessage(hCtlFROMTABLE, CB_SETITEMDATA, (WPARAM)nI, (LPARAM)FILEDDF.Id);
        rcode2 = btrieve(B_GETNEXT, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
      }
//
//  Field names
//
      if(DERIVED.joinedToTableId == NO_RECORD || DERIVED.joinedToFieldId == NO_RECORD)
        SendMessage(hCtlFROMTABLE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      SendMessage(hWndDlg, WM_COMMAND,
            MAKEWPARAM(FIELDDEF_FROMTABLE, CBN_SELCHANGE), (LPARAM)0);
//
//  Formats
//
      for(nI = 0; nI < NUMBER_OF_TMSDATA_TYPES; nI++)
      {
        nJ = SendMessage(hCtlAS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)formatTypes[nI]);
        SendMessage(hCtlAS, LB_SETITEMDATA, (WPARAM)nJ, (LPARAM)nI);
      }
      for(nI = 0; nI < NUMBER_OF_TMSDATA_TYPES; nI++)  // Do this because of the alpha sort
      {
        if(FORMATTED.dataType == SendMessage(hCtlAS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0))
        {
          SendMessage(hCtlAS, LB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
          break;
        }
      }
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
//  FIELDDEF_DERIVEDNO radio button
//
        case FIELDDEF_DERIVEDNO:
          if(SendMessage(hCtlDERIVEDNO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlFROMFIELD, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;
//
//  FIELDDEF_FROMTABLE Combo box
//
        case FIELDDEF_FROMTABLE:
          switch(wmEvent)
          {
            case CBN_DROPDOWN:
              if(SendMessage(hCtlDERIVEDNO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlDERIVEDNO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlDERIVEDYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
//
//  Build the list of table fields
//
            case CBN_SELCHANGE:
              nI = (int)SendMessage(hCtlFROMTABLE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              DERIVED.joinedToTableId =
                    (int)SendMessage(hCtlFROMTABLE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              SendMessage(hCtlFROMFIELD, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
              FIELDDDFKey1.File = DERIVED.joinedToTableId;
              rcode2 = btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
              while(rcode2 == 0 && FIELDDDF.File == DERIVED.joinedToTableId)
              {
                strncpy(tempString, FIELDDDF.Name, sizeof(FIELDDDF.Name));
                trim(tempString, sizeof(FIELDDDF.Name));
                nI = SendMessage(hCtlFROMFIELD, LB_ADDSTRING,
                      (WPARAM)0, (LONG)(LPSTR)tempString);
                if(FIELDDDF.Id == DERIVED.joinedToFieldId)
                  SendMessage(hCtlFROMFIELD, LB_SETCURSEL, (WPARAM)nI, (LPARAM)0);
                SendMessage(hCtlFROMFIELD, LB_SETITEMDATA, (WPARAM)nI, (LPARAM)FIELDDDF.Id);
                rcode2 = btrieve(B_GETNEXT, FIELD_DDF, &FIELDDDF, &FIELDDDFKey1, 1);
              }
          }
          break;
//
//  FIELDDEF_FROMFIELD list box
//
        case FIELDDEF_FROMFIELD:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlDERIVEDNO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlDERIVEDNO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlDERIVEDYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
          break; 
//
//  FIELDDEF_FORMATTEDNO
//
        case FIELDDEF_FORMATTEDNO:
          if(SendMessage(hCtlFORMATTEDNO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            SendMessage(hCtlAS, LB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
          break;
//
//  FIELDDEF_AS list box
//
        case FIELDDEF_AS:
          switch(wmEvent)
          {
            case LBN_SELCHANGE:
              if(SendMessage(hCtlFORMATTEDNO, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              {
                SendMessage(hCtlFORMATTEDNO, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
                SendMessage(hCtlFORMATTEDYES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
              }
              break;
          }
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Field_Definition_Dialog_Box);
          break;
//
//  IDOK
//
        case IDOK:
//
//  The displayed name can't be blank
//
          SendMessage(hCtlDISPLAYEDNAME, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_132, hCtlDISPLAYEDNAME);
            break;
          }
//
//  Check the status of derived.  If its TRUE, we have to have values
//  for DERIVED.joinedToTableId and DERVIED.joinedToFieldId before proceeding.
//
          bDerived = (BOOL)SendMessage(hCtlDERIVEDYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(bDerived)
          {
            selectedTable = SendMessage(hCtlFROMTABLE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(hCtlFROMTABLE, CB_GETLBTEXT,
                  (WPARAM)selectedTable, (LONG)(LPSTR)tempString);
            strncpy(FILEDDFKey1.Name, tempString, sizeof(FILEDDFKey1.Name));
            pad(FILEDDFKey1.Name, sizeof(FILEDDFKey1.Name));
            btrieve(B_GETEQUAL, FILE_DDF, &FILEDDF, &FILEDDFKey1, 1);
            DERIVED.joinedToTableId = FILEDDF.Id;
            fieldIndex = SendMessage(hCtlFROMFIELD, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            FIELDDDFKey3.File = FILEDDF.Id;
            SendMessage(hCtlFROMFIELD, LB_GETTEXT,
                  (WPARAM)fieldIndex, (LONG)(LPSTR)fieldName);
            strcpy(FIELDDDFKey3.Name, fieldName);
            pad(FIELDDDFKey3.Name, sizeof(FIELDDDFKey3.Name));
            btrieve(B_GETEQUAL, FIELD_DDF, &FIELDDDF, &FIELDDDFKey3, 3);
            DERIVED.joinedToFieldId = FIELDDDF.Id;
          }
//
//  If he picked formatted, he must have made a selection
//
          bFormatted =
                (BOOL)SendMessage(hCtlFORMATTEDYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          if(bFormatted)
          {
            nI = (int)SendMessage(hCtlAS, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if(nI == LB_ERR)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_063, hCtlAS);
              break;
            }
            else
              FORMATTED.dataType =
                    (int)SendMessage(hCtlAS, LB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
          }
//
//  Is this a created field?  If so, see if there's an attributes record,
//  and use it.  If there isn't an attributes record, create one.
//
          if(pFIELDDEF->bCreated)
          {
            ATTRIBUTESKey0.Id = pFIELDDEF->Id;
            rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
            if(rcode2 != 0)
            {
              btrieve(B_GETLAST, FIELD_DDF, &FIELDDDF, &FIELDDDFKey0, 0);
              if(btrieve(B_GETLAST, TMS_CREATED, &CREATED, &CREATEDKey0, 0) != 0)
                CREATED.Id = NO_RECORD;
              memset(&ATTRIBUTES, 0x00, sizeof(ATTRIBUTES));
              ATTRIBUTES.Id = max(FIELDDDF.Id, CREATED.Id) + 1;
              if(ATTRIBUTES.Id < 10000)
                ATTRIBUTES.Id += 10000;
            }
          }
//
//  This isn't a created field.  See if there's an attributes record, and
//  use it.  If there isn't an attributes record, set one up.
//
          else
          {
            ATTRIBUTESKey0.Id = pFIELDDEF->Id;
            rcode2 = btrieve(B_GETEQUAL, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
            if(rcode2 != 0)
            {
              memset(&ATTRIBUTES, 0x00, sizeof(ATTRIBUTES));
              ATTRIBUTES.Id = pFIELDDEF->Id;
            }
          }
//
//  Get and pad the field title, and assign the logicals and derived stuff
//
          SendMessage(hCtlDISPLAYEDNAME, WM_GETTEXT,
                (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          strcpy(ATTRIBUTES.fieldTitle, tempString);
          pad(ATTRIBUTES.fieldTitle, ATTRIBUTES_FIELDTITLE_LENGTH);
          ATTRIBUTES.flags = 0;
          if(SendMessage(hCtlHIDDENYES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            ATTRIBUTES.flags |= ATTRIBUTES_FLAG_HIDDEN;
          if(bDerived)
          {
            ATTRIBUTES.flags |= ATTRIBUTES_FLAG_DERIVED;
            ATTRIBUTES.derivedTableId = DERIVED.joinedToTableId;
            ATTRIBUTES.derivedFieldId = DERIVED.joinedToFieldId;
          }
          else
          {
            ATTRIBUTES.derivedTableId = NO_RECORD;
            ATTRIBUTES.derivedFieldId = NO_RECORD;
          }
          if(bFormatted)
          {
            ATTRIBUTES.flags |= ATTRIBUTES_FLAG_FORMATTED;
            ATTRIBUTES.formattedDataType = FORMATTED.dataType;
          }
          else
          {
            ATTRIBUTES.formattedDataType = NO_RECORD;
          }
//
//  Update or insert
//
         if(rcode2 == 0)
           btrieve(B_UPDATE, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
         else
           btrieve(B_INSERT, TMS_ATTRIBUTES, &ATTRIBUTES, &ATTRIBUTESKey0, 0);
//
//  Make sure the FIELDDEF structure reflects what we have
//
         SendMessage(hCtlDISPLAYEDNAME, WM_GETTEXT,
               (WPARAM)sizeof(pFIELDDEF->szDisplayedName),
               (LONG)(LPSTR)pFIELDDEF->szDisplayedName);
         pFIELDDEF->hidden = ATTRIBUTES.flags & ATTRIBUTES_FLAG_HIDDEN;
         pFIELDDEF->derived = ATTRIBUTES.flags & ATTRIBUTES_FLAG_DERIVED;
         pFIELDDEF->formatted = ATTRIBUTES.flags & ATTRIBUTES_FLAG_FORMATTED;
         pFIELDDEF->Id = ATTRIBUTES.Id;
         EndDialog(hWndDlg, TRUE);
         break;
     }
     break;

   default:
     return FALSE;
 }
 return TRUE;
}
