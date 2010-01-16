//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Unassigned drivers report
//
BOOL FAR TMSRPT13(TMSRPTPassedDataDef *pPassedData)
{
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  char  szOutputString[128];
  long  year, month,day;
  int   rcode2;
  int   nI;

  pPassedData->nReportNumber = 12;
  pPassedData->numDataFiles = 1;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Open the output file
//
  StatusBarText("Cycling through roster...");
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt13.txt");
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
//  Loop through the drivers in seniority order
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      if(rcode2 != 0)
      {
        if(DRIVERS.DRIVERTYPESrecordID == NO_RECORD)
        {
          strcpy(tempString, "");
        }
        else
        {
          DRIVERTYPESKey0.recordID = DRIVERS.DRIVERTYPESrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
          if(rcode2 != 0)
          {
            strcpy(tempString, "");
          }
          else
          {
            strncpy(tempString, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
            trim(tempString, DRIVERTYPES_NAME_LENGTH);
          }
        }
        strcpy(szOutputString, tempString);
        strcat(szOutputString, "\t");
        GetYMD(DRIVERS.seniorityDate, &year, &month, &day);
        sprintf(tempString, "%4ld/%02ld/%02ld - ", year, month, day);
        strcat(szOutputString, tempString);
        strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(tempString, DRIVERS_LASTNAME_LENGTH);
        strcat(szOutputString, tempString);
        strcat(szOutputString, ", ");
        strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(szOutputString, tempString);
        strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
        if(strcmp(tempString, "") != 0)
        {
          strcat(szOutputString, " (Badge ");
          strcat(szOutputString, tempString);
          strcat(szOutputString, ")");
        }
        strcat(szOutputString, "\r\n");
        _lwrite(hfOutputFile, szOutputString, strlen(szOutputString));
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  }

  bKeepGoing = TRUE;
//
//  Free allocated memory / close output file
//
  deallocate:
    _lclose(hfOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
      return(FALSE);
//
//  All done
//
  return(TRUE);
}
