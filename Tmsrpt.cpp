//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"

#include "WinReg.h"

int sort_reports(const void *a, const void *b)
{
  TMSRPTDef *pa, *pb;
  pa = (TMSRPTDef *)a;
  pb = (TMSRPTDef *)b;
  return(strcmp(pa->szReportName, pb->szReportName));
}

#define LASTREPORT 88

void CTMSApp::TMSRpt(void)
{
  CString s;
  int nI;
//
//  Get the report output maximums for the block and run paddles
//
  TMSRPTMaxLinesPerPage = GetProfileInt("Reports", "MaxLinesPerPage", TMSRPT_MAX_LINESPERPAGE);
  TMSRPTMaxColsPerPage = GetProfileInt("Reports", "MaxColsPerPage", TMSRPT_MAX_COLSPERPAGE);
//
//  Get the report temporary directory
//
  s = GetProfileString("Reports", "TemporaryFileFolder");
  if(s == "")
  {
    s = "C:\\temp";
  }
  strcpy(szReportsTempFolder, s);
//
//  Zero out the flags
//
  for(nI = 0; nI < TMSRPT_MAX_REPORTSDEFINED; nI++)
  {
    TMSRPT[nI].flags = 0;
  }
//
//  Busbook header
//
  TMSRPT[0].flags = TMSRPTFLAG_REPORT;
  TMSRPT[0].originalReportNumber = 0;
  strcpy(TMSRPT[0].szReportName, "Busbook Header");
  TMSRPT[0].numReports = 1;
  strcpy(TMSRPT[0].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[0].szReportFileName[0], "\\Busbook Header.rpt");
  TMSRPT[0].lpRoutine = TMSRPT01;
  TMSRPT[0].lpConfigFunc = (TMSRPTConfigFuncDef)TMSRPT01CMsgProc;
  TMSRPT[0].ConfigFuncDlg = IDD_RPTCFG01;
//
//  Busbook detail
//
  TMSRPT[1].flags = TMSRPTFLAG_REPORT;
  TMSRPT[1].originalReportNumber = 1;
  strcpy(TMSRPT[1].szReportName, "Busbook Detail");
  TMSRPT[1].numReports = 1;
  strcpy(TMSRPT[1].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[1].szReportFileName[0], "\\Busbook Detail.rpt");
  TMSRPT[1].lpRoutine = TMSRPT02;
  TMSRPT[1].lpConfigFunc = (TMSRPTConfigFuncDef)TMSRPT02CMsgProc;
  TMSRPT[1].ConfigFuncDlg = IDD_RPTCFG02;
//
//  Time and Distance by Route (Detailed)
//
  TMSRPT[2].flags = TMSRPTFLAG_REPORT;
  TMSRPT[2].originalReportNumber = 2;
  strcpy(TMSRPT[2].szReportName, "T/D by Route (Detailed)");
  TMSRPT[2].numReports = 1;
  strcpy(TMSRPT[2].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[2].szReportFileName[0], "\\T+D by Route - Detailed.rpt");
  TMSRPT[2].lpRoutine = TMSRPT03;
  TMSRPT[2].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[2].ConfigFuncDlg = NO_RECORD;
//
//  Time and Distance by Route (Summary)
//
  TMSRPT[3].flags = TMSRPTFLAG_REPORT;
  TMSRPT[3].originalReportNumber = 3;
  strcpy(TMSRPT[3].szReportName, "T/D by Route (Summary)");
  TMSRPT[3].numReports = 1;
  strcpy(TMSRPT[3].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[3].szReportFileName[0], "\\T+D by Route - Summary.rpt");
  TMSRPT[3].lpRoutine = TMSRPT04;
  TMSRPT[3].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[3].ConfigFuncDlg = NO_RECORD;
//
//  Dispatch sheet (Format 1)
//
  TMSRPT[4].flags = TMSRPTFLAG_REPORT;
  TMSRPT[4].originalReportNumber = 4;
  strcpy(TMSRPT[4].szReportName, "Dispatch Sheet (1)");
  TMSRPT[4].numReports = 1;
  strcpy(TMSRPT[4].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[4].szReportFileName[0], "\\Dispatch Sheet (1).rpt");
  TMSRPT[4].lpRoutine = TMSRPT05;
  TMSRPT[4].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[4].ConfigFuncDlg = NO_RECORD;
//
//  System-wide block listing
//
  TMSRPT[5].flags = TMSRPTFLAG_REPORT;
  TMSRPT[5].originalReportNumber = 5;
  strcpy(TMSRPT[5].szReportName, "System Blocks");
  TMSRPT[5].numReports = 1;
  strcpy(TMSRPT[5].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[5].szReportFileName[0], "\\System Blocks.rpt");
  TMSRPT[5].lpRoutine = TMSRPT06;
  TMSRPT[5].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[5].ConfigFuncDlg = NO_RECORD;
//
//  Precursor Download
//
  TMSRPT[6].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[6].originalReportNumber = 6;
  strcpy(TMSRPT[6].szReportName, "Precursor Download");
  TMSRPT[6].numReports = 1;
  strcpy(TMSRPT[6].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[6].szReportFileName[0], "\\Precursor Download.rpt");
  TMSRPT[6].lpRoutine = TMSRPT07;
  TMSRPT[6].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[6].ConfigFuncDlg = NO_RECORD;
//
//  Connections/Potentials
//
  TMSRPT[7].flags = TMSRPTFLAG_REPORT;
  TMSRPT[7].originalReportNumber = 7;
  strcpy(TMSRPT[7].szReportName, "Connections and Potentials");
  TMSRPT[7].numReports = 1;
  strcpy(TMSRPT[7].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[7].szReportFileName[0], "\\Connections and Potentials.rpt");
  TMSRPT[7].lpRoutine = TMSRPT08;
  TMSRPT[7].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[7].ConfigFuncDlg = NO_RECORD;
//
//  Driver Paddle (1)
//
  TMSRPT[8].flags = TMSRPTFLAG_REPORT;
  TMSRPT[8].originalReportNumber = 8;
  strcpy(TMSRPT[8].szReportName, "Driver Paddle (1)");
  TMSRPT[8].numReports = 1;
  strcpy(TMSRPT[8].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[8].szReportFileName[0], "\\Driver Paddle (1).rpt");
  TMSRPT[8].lpRoutine = TMSRPT09;
  TMSRPT[8].lpConfigFunc = (TMSRPTConfigFuncDef)TMSRPT09CMsgProc;
  TMSRPT[8].ConfigFuncDlg = IDD_RPTCFG09;
//
//  Block Paddle (1)
//
  TMSRPT[9].flags = TMSRPTFLAG_REPORT;
  TMSRPT[9].originalReportNumber = 9;
  strcpy(TMSRPT[9].szReportName, "Block Paddle (1)");
  TMSRPT[9].numReports = 1;
  strcpy(TMSRPT[9].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[9].szReportFileName[0], "\\Block Paddle (1).rpt");
  TMSRPT[9].lpRoutine = TMSRPT10;
  TMSRPT[9].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[9].ConfigFuncDlg = NO_RECORD;
//
//  Teleride Publish
//
  TMSRPT[10].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[10].originalReportNumber = 10;
  strcpy(TMSRPT[10].szReportName, "Teleride Publish");
  TMSRPT[10].numReports = 1;
  strcpy(TMSRPT[10].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[10].lpRoutine = TMSRPT11;
  TMSRPT[10].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[10].ConfigFuncDlg = NO_RECORD;
//
//  C-Tran HP Download
//
  TMSRPT[11].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[11].originalReportNumber = 11;
  strcpy(TMSRPT[11].szReportName, "C-Tran Download to HP");
  TMSRPT[11].numReports = 1;
  strcpy(TMSRPT[11].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[11].lpRoutine = TMSRPT12;
  TMSRPT[11].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[11].ConfigFuncDlg = NO_RECORD;
//
//  Unassigned Drivers
//
  TMSRPT[12].flags = TMSRPTFLAG_REPORT;
  TMSRPT[12].originalReportNumber = 12;
  strcpy(TMSRPT[12].szReportName, "Unassigned Drivers");
  TMSRPT[12].numReports = 1;
  strcpy(TMSRPT[12].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[12].szReportFileName[0], "\\Unassigned Drivers.rpt");
  TMSRPT[12].lpRoutine = TMSRPT13;
  TMSRPT[12].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[12].ConfigFuncDlg = NO_RECORD;
//
//  Unrostered Runs
//
  TMSRPT[13].flags = TMSRPTFLAG_REPORT;
  TMSRPT[13].originalReportNumber = 13;
  strcpy(TMSRPT[13].szReportName, "Unrostered Runs");
  TMSRPT[13].numReports = 1;
  strcpy(TMSRPT[13].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[13].szReportFileName[0], "\\Unrostered Runs.rpt");
  TMSRPT[13].lpRoutine = TMSRPT14;
  TMSRPT[13].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[13].ConfigFuncDlg = NO_RECORD;
//
//  Deadhead Load
//
  TMSRPT[14].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[14].originalReportNumber = 14;
  strcpy(TMSRPT[14].szReportName, "Load Deadheads/Travels");
  TMSRPT[14].numReports = 1;
  strcpy(TMSRPT[14].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[14].lpRoutine = TMSRPT15;
  TMSRPT[14].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[14].ConfigFuncDlg = NO_RECORD;
//
//  Timepoint Activity (1)
//
  TMSRPT[15].flags = TMSRPTFLAG_REPORT;
  TMSRPT[15].originalReportNumber = 15;
  strcpy(TMSRPT[15].szReportName, "Timepoint Activity (1)");
  TMSRPT[15].numReports = 1;
  strcpy(TMSRPT[15].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[15].szReportFileName[0], "\\Timepoint Activity (1).rpt");
  TMSRPT[15].lpRoutine = TMSRPT16;
  TMSRPT[15].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[15].ConfigFuncDlg = NO_RECORD;
//
//  GRATA Runcut Download
//
  TMSRPT[16].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[16].originalReportNumber = 16;
  strcpy(TMSRPT[16].szReportName, "GRATA Runcut Download");
  TMSRPT[16].numReports = 1;
  strcpy(TMSRPT[16].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[16].lpRoutine = TMSRPT17;
  TMSRPT[16].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[16].ConfigFuncDlg = NO_RECORD;
//
//  Driver Data Load
//
  TMSRPT[17].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[17].originalReportNumber = 17;
  strcpy(TMSRPT[17].szReportName, "Load Driver Data");
  TMSRPT[17].numReports = 1;
  strcpy(TMSRPT[17].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[17].lpRoutine = TMSRPT18;
  TMSRPT[17].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[17].ConfigFuncDlg = NO_RECORD;
//
//  Dispatch sheet (Format 2)
//
  TMSRPT[18].flags = TMSRPTFLAG_REPORT;
  TMSRPT[18].originalReportNumber = 18;
  strcpy(TMSRPT[18].szReportName, "Dispatch Sheet (2)");
  TMSRPT[18].numReports = 1;
  strcpy(TMSRPT[18].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[18].szReportFileName[0], "\\Dispatch Sheet (2).rpt");
  TMSRPT[18].lpRoutine = TMSRPT19;
  TMSRPT[18].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[18].ConfigFuncDlg = NO_RECORD;
//
//  Connection overlap
//
  TMSRPT[19].flags = TMSRPTFLAG_REPORT;
  TMSRPT[19].originalReportNumber = 19;
  strcpy(TMSRPT[19].szReportName, "Connections Overlap Checker");
  TMSRPT[19].numReports = 1;
  strcpy(TMSRPT[19].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[19].szReportFileName[0], "\\Connections Overlap Checker.rpt");
  TMSRPT[19].lpRoutine = TMSRPT20;
  TMSRPT[19].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[19].ConfigFuncDlg = NO_RECORD;
//
//  Book pages
//
  TMSRPT[20].flags = TMSRPTFLAG_REPORT;
  TMSRPT[20].originalReportNumber = 20;
  strcpy(TMSRPT[20].szReportName, "Book Pages");
  TMSRPT[20].numReports = 1;
  strcpy(TMSRPT[20].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[20].szReportFileName[0], "\\Book Pages.rpt");
  TMSRPT[20].lpRoutine = TMSRPT21;
  TMSRPT[20].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[20].ConfigFuncDlg = NO_RECORD;
//
//  Block paddle (2)
//
  TMSRPT[21].flags = TMSRPTFLAG_REPORT;
  TMSRPT[21].originalReportNumber = 21;
  strcpy(TMSRPT[21].szReportName, "Block Paddle (2)");
  TMSRPT[21].numReports = 1;
  strcpy(TMSRPT[21].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[21].szReportFileName[0], "\\Block Paddle (2).rpt");
  TMSRPT[21].lpRoutine = TMSRPT22;
  TMSRPT[21].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[21].ConfigFuncDlg = NO_RECORD;
//
//  Stop Listing
//
  TMSRPT[22].flags = TMSRPTFLAG_REPORT;
  TMSRPT[22].originalReportNumber = 22;
  strcpy(TMSRPT[22].szReportName, "Stop Listing");
  TMSRPT[22].numReports = 1;
  strcpy(TMSRPT[22].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[22].szReportFileName[0], "\\Stop Listing.rpt");
  TMSRPT[22].lpRoutine = TMSRPT23;
  TMSRPT[22].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[22].ConfigFuncDlg = NO_RECORD;
//
//  Connexionz Download
//
  TMSRPT[23].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[23].originalReportNumber = 23;
  strcpy(TMSRPT[23].szReportName, "AVL - Connexionz Interface (V1) Download");
  TMSRPT[23].numReports = 1;
  strcpy(TMSRPT[23].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[23].lpRoutine = TMSRPT86;  // Was TMSRPT24
  TMSRPT[23].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[23].ConfigFuncDlg = NO_RECORD;
//
//  Telerider Download
//
  TMSRPT[24].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[24].originalReportNumber = 24;
  strcpy(TMSRPT[24].szReportName, "Telerider Download");
  TMSRPT[24].numReports = 1;
  strcpy(TMSRPT[24].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[24].lpRoutine = TMSRPT25;
  TMSRPT[24].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[24].ConfigFuncDlg = NO_RECORD;
//
//  Productivity Report (1)
//
  TMSRPT[25].flags = TMSRPTFLAG_REPORT;
  TMSRPT[25].originalReportNumber = 25;
  strcpy(TMSRPT[25].szReportName, "Productivity Report (1)");
  TMSRPT[25].numReports = 1;
  strcpy(TMSRPT[25].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[25].szReportFileName[0], "\\Productivity Report (1).rpt");
  TMSRPT[25].lpRoutine = TMSRPT26;
  TMSRPT[25].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[25].ConfigFuncDlg = NO_RECORD;
//
//  BC Transit Web Page Download
//
  TMSRPT[26].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[26].originalReportNumber = 26;
  strcpy(TMSRPT[26].szReportName, "BC Transit Web Page Download");
  TMSRPT[26].numReports = 1;
  strcpy(TMSRPT[26].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[26].lpRoutine = TMSRPT27;
  TMSRPT[26].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[26].ConfigFuncDlg = NO_RECORD;
//
//  HTML Download (1)
//
  TMSRPT[27].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[27].originalReportNumber = 27;
  strcpy(TMSRPT[27].szReportName, "HTML Download (1)");
  TMSRPT[27].numReports = 1;
  strcpy(TMSRPT[27].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[27].lpRoutine = TMSRPT28;
  TMSRPT[27].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[27].ConfigFuncDlg = NO_RECORD;
//
//  ATC/Vancom Denver Data Load
//
  TMSRPT[28].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[28].originalReportNumber = 28;
  strcpy(TMSRPT[28].szReportName, "ATC/Vancom Denver Data Load");
  TMSRPT[28].numReports = 1;
  strcpy(TMSRPT[28].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[28].lpRoutine = TMSRPT29;
  TMSRPT[28].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[28].ConfigFuncDlg = NO_RECORD;
//
//  Dispatch sheet (Format 3)
//
  TMSRPT[29].flags = TMSRPTFLAG_REPORT;
  TMSRPT[29].originalReportNumber = 29;
  strcpy(TMSRPT[29].szReportName, "Dispatch Sheet (3)");
  TMSRPT[29].numReports = 1;
  strcpy(TMSRPT[29].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[29].szReportFileName[0], "\\Dispatch Sheet (3).rpt");
  TMSRPT[29].lpRoutine = TMSRPT30;
  TMSRPT[29].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[29].ConfigFuncDlg = NO_RECORD;
//
//  Excel-type download of bus stops with distances
//
  TMSRPT[30].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[30].originalReportNumber = 30;
  strcpy(TMSRPT[30].szReportName, "Bus Stops Download to Excel");
  TMSRPT[30].numReports = 1;
  strcpy(TMSRPT[30].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[30].lpRoutine = TMSRPT31;
  TMSRPT[30].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[30].ConfigFuncDlg = NO_RECORD;
//
//  Excel-type download of schedule data for transfer to WMATA
//
  TMSRPT[31].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[31].originalReportNumber = 31;
  strcpy(TMSRPT[31].szReportName, "Schedule Download to Excel");
  TMSRPT[31].numReports = 1;
  strcpy(TMSRPT[31].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[31].lpRoutine = TMSRPT32;
  TMSRPT[31].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[31].ConfigFuncDlg = NO_RECORD;
//
//  System time period detail
//
  TMSRPT[32].flags = TMSRPTFLAG_REPORT;
  TMSRPT[32].originalReportNumber = 32;
  strcpy(TMSRPT[32].szReportName, "System Time Period Detail");
  TMSRPT[32].numReports = 1;
  strcpy(TMSRPT[32].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[32].szReportFileName[0], "\\System Time Period Detail.rpt");
  TMSRPT[32].lpRoutine = TMSRPT33;
  TMSRPT[32].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[32].ConfigFuncDlg = NO_RECORD;
//
//  Productivity Report (2)
//
  TMSRPT[33].flags = TMSRPTFLAG_REPORT;
  TMSRPT[33].originalReportNumber = 33;
  strcpy(TMSRPT[33].szReportName, "Productivity Report (2)");
  TMSRPT[33].numReports = 1;
  strcpy(TMSRPT[33].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[33].szReportFileName[0], "\\Productivity Report (2).rpt");
  TMSRPT[33].lpRoutine = TMSRPT34;
  TMSRPT[33].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[33].ConfigFuncDlg = NO_RECORD;
//
//  Driver Paddle (2)
//
  TMSRPT[34].flags = TMSRPTFLAG_REPORT;
  TMSRPT[34].originalReportNumber = 34;
  strcpy(TMSRPT[34].szReportName, "Driver Paddle (2)");
  TMSRPT[34].numReports = 1;
  strcpy(TMSRPT[34].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[34].szReportFileName[0], "\\Driver Paddle (2).rpt");
  TMSRPT[34].lpRoutine = TMSRPT35;
  TMSRPT[34].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[34].ConfigFuncDlg = NO_RECORD;
//
//  On-Time Performance (1)
//
  TMSRPT[35].flags = TMSRPTFLAG_REPORT;
  TMSRPT[35].originalReportNumber = 35;
  strcpy(TMSRPT[35].szReportName, "On-Time Performance (1)");
  TMSRPT[35].numReports = 1;
  strcpy(TMSRPT[35].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[35].szReportFileName[0], "\\On-Time Performance (1).rpt");
  TMSRPT[35].lpRoutine = TMSRPT36;
  TMSRPT[35].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[35].ConfigFuncDlg = NO_RECORD;
//
//  Time Checker Worksheet
//
  TMSRPT[36].flags = TMSRPTFLAG_REPORT;
  TMSRPT[36].originalReportNumber = 36;
  strcpy(TMSRPT[36].szReportName, "Time Checker Worksheet");
  TMSRPT[36].numReports = 1;
  strcpy(TMSRPT[36].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[36].szReportFileName[0], "\\Time Checker Worksheet.rpt");
  TMSRPT[36].lpRoutine = TMSRPT37;
  TMSRPT[36].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[36].ConfigFuncDlg = NO_RECORD;
//
//  Dispatch sheet (Format 4)
//
  TMSRPT[37].flags = TMSRPTFLAG_REPORT;
  TMSRPT[37].originalReportNumber = 37;
  strcpy(TMSRPT[37].szReportName, "Dispatch Sheet (4)");
  TMSRPT[37].numReports = 1;
  strcpy(TMSRPT[37].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[37].szReportFileName[0], "\\Dispatch Sheet (4).rpt");
  TMSRPT[37].lpRoutine = TMSRPT38;
  TMSRPT[37].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[37].ConfigFuncDlg = NO_RECORD;
//
//  System runs
//
  TMSRPT[38].flags = TMSRPTFLAG_REPORT;
  TMSRPT[38].originalReportNumber = 38;
  strcpy(TMSRPT[38].szReportName, "System Runs");
  TMSRPT[38].numReports = 1;
  strcpy(TMSRPT[38].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[38].szReportFileName[0], "\\System Runs.rpt");
  TMSRPT[38].lpRoutine = TMSRPT39;
  TMSRPT[38].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[38].ConfigFuncDlg = NO_RECORD;
//
//  Connections analysis
//
  TMSRPT[39].flags = TMSRPTFLAG_REPORT;
  TMSRPT[39].originalReportNumber = 39;
  strcpy(TMSRPT[39].szReportName, "Connections Analysis");
  TMSRPT[39].numReports = 1;
  strcpy(TMSRPT[39].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[39].szReportFileName[0], "\\Connections Analysis.rpt");
  TMSRPT[39].lpRoutine = TMSRPT40;
  TMSRPT[39].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[39].ConfigFuncDlg = NO_RECORD;
//
//  Load connection data
//
  TMSRPT[40].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[40].originalReportNumber = 40;
  strcpy(TMSRPT[40].szReportName, "Load Connection Data");
  TMSRPT[40].numReports = 1;
  strcpy(TMSRPT[40].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[40].lpRoutine = TMSRPT41;
  TMSRPT[40].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[40].ConfigFuncDlg = NO_RECORD;
//
//  ATIS Download
//
  TMSRPT[41].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[41].originalReportNumber = 41;
  strcpy(TMSRPT[41].szReportName, "ATIS Stops/Patterns/Trips");
  TMSRPT[41].numReports = 1;
  strcpy(TMSRPT[41].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[41].lpRoutine = TMSRPT42;
  TMSRPT[41].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[41].ConfigFuncDlg = NO_RECORD;
//
//  Daily Operations Audit Trail
//
  TMSRPT[42].flags = TMSRPTFLAG_REPORT;
  TMSRPT[42].originalReportNumber = 42;
  strcpy(TMSRPT[42].szReportName, "Daily Operations Audit Trail");
  TMSRPT[42].numReports = 1;
  strcpy(TMSRPT[42].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[42].szReportFileName[0], "\\Daily Operations Audit Trail.rpt");
  TMSRPT[42].lpRoutine = TMSRPT43;
  TMSRPT[42].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[42].ConfigFuncDlg = NO_RECORD;
//
//  Dispatch sheet (Format 5)
//
  TMSRPT[43].flags = TMSRPTFLAG_REPORT;
  TMSRPT[43].originalReportNumber = 43;
  strcpy(TMSRPT[43].szReportName, "Dispatch Sheet (5)");
  TMSRPT[43].numReports = 1;
  strcpy(TMSRPT[43].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[43].szReportFileName[0], "\\Dispatch Sheet (5).rpt");
  TMSRPT[43].lpRoutine = TMSRPT44;
  TMSRPT[43].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[43].ConfigFuncDlg = NO_RECORD;
//
//  Block / Bus Assignment Sheet
//
  TMSRPT[44].flags = TMSRPTFLAG_REPORT;
  TMSRPT[44].originalReportNumber = 44;
  strcpy(TMSRPT[44].szReportName, "Block/Bus Assignment Sheet");
  TMSRPT[44].numReports = 1;
  strcpy(TMSRPT[44].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[44].szReportFileName[0], "\\Block+Bus Assignment Sheet.rpt");
  TMSRPT[44].lpRoutine = TMSRPT45;
  TMSRPT[44].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[44].ConfigFuncDlg = NO_RECORD;
//
//  Relief Car Assignment Sheet
//
  TMSRPT[45].flags = TMSRPTFLAG_REPORT;
  TMSRPT[45].originalReportNumber = 45;
  strcpy(TMSRPT[45].szReportName, "Relief Car Assignment Sheet");
  TMSRPT[45].numReports = 1;
  strcpy(TMSRPT[45].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[45].szReportFileName[0], "\\Relief Car Assignment Sheet.rpt");
  TMSRPT[45].lpRoutine = TMSRPT46;
  TMSRPT[45].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[45].ConfigFuncDlg = NO_RECORD;
//
//  Timepoint Activity (2)
//
  TMSRPT[46].flags = TMSRPTFLAG_REPORT;
  TMSRPT[46].originalReportNumber = 46;
  strcpy(TMSRPT[46].szReportName, "Timepoint Activity (2)");
  TMSRPT[46].numReports = 1;
  strcpy(TMSRPT[46].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[46].szReportFileName[0], "\\Timepoint Activity (2).rpt");
  TMSRPT[46].lpRoutine = TMSRPT16;
  TMSRPT[46].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[46].ConfigFuncDlg = NO_RECORD;
//
//  On-Time Performance (2)
//
  TMSRPT[47].flags = TMSRPTFLAG_REPORT;
  TMSRPT[47].originalReportNumber = 47;
  strcpy(TMSRPT[47].szReportName, "On-Time Performance (2)");
  TMSRPT[47].numReports = 2;
  strcpy(TMSRPT[47].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[47].szReportFileName[0], "\\On-Time Performance (2) - Summary.rpt");
  strcpy(TMSRPT[47].szReportFileName[1], szReportTemplateFolder);
  strcat(TMSRPT[47].szReportFileName[1], "\\On-Time Performance (2) - Detail.rpt");
  TMSRPT[47].lpRoutine = TMSRPT48;
  TMSRPT[47].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[47].ConfigFuncDlg = NO_RECORD;
//
//  MCT Tripcard Data
//
  TMSRPT[48].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[48].originalReportNumber = 48;
  strcpy(TMSRPT[48].szReportName, "MCT Tripcard Data");
  TMSRPT[48].numReports = 1;
  strcpy(TMSRPT[48].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[48].lpRoutine = TMSRPT49;
  TMSRPT[48].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[48].ConfigFuncDlg = NO_RECORD;
//
//  Events report
//
  TMSRPT[49].flags = TMSRPTFLAG_REPORT;
  TMSRPT[49].originalReportNumber = 49;
  strcpy(TMSRPT[49].szReportName, "Event Report");
  TMSRPT[49].numReports = 1;
  strcpy(TMSRPT[49].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[49].szReportFileName[0], "\\Event Report.rpt");
  TMSRPT[49].lpRoutine = TMSRPT50;
  TMSRPT[49].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[49].ConfigFuncDlg = NO_RECORD;
//
//  Fleet-Net Payroll  Download
//
  TMSRPT[50].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[50].originalReportNumber = 50;
  strcpy(TMSRPT[50].szReportName, "Fleet-Net Payroll download");
  TMSRPT[50].numReports = 1;
  strcpy(TMSRPT[50].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[50].lpRoutine = TMSRPT51;
  TMSRPT[50].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[50].ConfigFuncDlg = NO_RECORD;
//
//  Weekly Open Work Assignment Sheet
//
  TMSRPT[51].flags = TMSRPTFLAG_REPORT;
  TMSRPT[51].originalReportNumber = 51;
  strcpy(TMSRPT[51].szReportName, "Weekly Open Work Assignment Sheet");
  TMSRPT[51].numReports = 1;
  strcpy(TMSRPT[51].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[51].szReportFileName[0], "\\Weekly Open Work Assignment Sheet.rpt");
  TMSRPT[51].lpRoutine = TMSRPT52;
  TMSRPT[51].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[51].ConfigFuncDlg = NO_RECORD;
//
//  Init AVL Download for C-Tran
//
  TMSRPT[52].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[52].originalReportNumber = 52;
  strcpy(TMSRPT[52].szReportName, "AVL - Init Interface Download");
  TMSRPT[52].numReports = 1;
  strcpy(TMSRPT[52].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[52].lpRoutine = TMSRPT53;
  TMSRPT[52].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[52].ConfigFuncDlg = NO_RECORD;
//
//  Kalamazoo Payroll  Download
//
  TMSRPT[53].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[53].originalReportNumber = 53;
  strcpy(TMSRPT[53].szReportName, "Kalamazoo Payroll download");
  TMSRPT[53].numReports = 1;
  strcpy(TMSRPT[53].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[53].lpRoutine = TMSRPT54;
  TMSRPT[53].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[53].ConfigFuncDlg = NO_RECORD;
//
//  Operator's Bid Sheet
//
  TMSRPT[54].flags = TMSRPTFLAG_REPORT;
  TMSRPT[54].originalReportNumber = 54;
  strcpy(TMSRPT[54].szReportName, "Operator's Bid List");
  TMSRPT[54].numReports = 1;
  strcpy(TMSRPT[54].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[54].szReportFileName[0], "\\Operator's Bid List.rpt");
  TMSRPT[54].lpRoutine = TMSRPT55;
  TMSRPT[54].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[54].ConfigFuncDlg = NO_RECORD;
//
//  Absence / Discipline Data Load
//
  TMSRPT[55].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[55].originalReportNumber = 55;
  strcpy(TMSRPT[55].szReportName, "Load Absence / Discipline Data");
  TMSRPT[55].numReports = 1;
  strcpy(TMSRPT[55].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[55].lpRoutine = TMSRPT56;
  TMSRPT[55].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[55].ConfigFuncDlg = NO_RECORD;
//
//  HTML Download (2)
//
  TMSRPT[56].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[56].originalReportNumber = 56;
  strcpy(TMSRPT[56].szReportName, "HTML Download (2)");
  TMSRPT[56].numReports = 1;
  strcpy(TMSRPT[56].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[56].lpRoutine = TMSRPT57;
  TMSRPT[56].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[56].ConfigFuncDlg = NO_RECORD;
//
//  Discipline Letter
//
  TMSRPT[57].flags = TMSRPTFLAG_REPORT;
  TMSRPT[57].originalReportNumber = 57;
  strcpy(TMSRPT[57].szReportName, "Discipline Letter");
  TMSRPT[57].numReports = 1;
  strcpy(TMSRPT[57].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[57].szReportFileName[0], "\\Discipline Letter.rpt");
  TMSRPT[57].lpRoutine = TMSRPT58;
  TMSRPT[57].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[57].ConfigFuncDlg = NO_RECORD;
//
//  BC Transit Download to Excel
//
  TMSRPT[58].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[58].originalReportNumber = 58;
  strcpy(TMSRPT[58].szReportName, "BC Transit Download to Excel");
  TMSRPT[58].numReports = 1;
  strcpy(TMSRPT[58].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[58].lpRoutine = TMSRPT59;
  TMSRPT[58].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[58].ConfigFuncDlg = NO_RECORD;
//
//  Daily Discipline Report
//
  TMSRPT[59].flags = TMSRPTFLAG_REPORT;
  TMSRPT[59].originalReportNumber = 59;
  strcpy(TMSRPT[59].szReportName, "Discipline Report");
  TMSRPT[59].numReports = 1;
  strcpy(TMSRPT[59].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[59].szReportFileName[0], "\\Discipline Report.rpt");
  TMSRPT[59].lpRoutine = TMSRPT60;
  TMSRPT[59].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[59].ConfigFuncDlg = NO_RECORD;
//
//  Orbital OrbCAD Download for Olympia
//
  TMSRPT[60].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[60].originalReportNumber = 60;
  strcpy(TMSRPT[60].szReportName, "AVL - ACS OrbCAD Interface Download");
  TMSRPT[60].numReports = 1;
  strcpy(TMSRPT[60].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[60].lpRoutine = TMSRPT61;
  TMSRPT[60].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[60].ConfigFuncDlg = NO_RECORD;
//
//  Roster Reconciliation
//
  TMSRPT[61].flags = TMSRPTFLAG_REPORT;
  TMSRPT[61].originalReportNumber = 61;
  strcpy(TMSRPT[61].szReportName, "Roster Reconciliation");
  TMSRPT[61].numReports = 1;
  strcpy(TMSRPT[61].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[61].szReportFileName[0], "\\Roster Reconciliation.rpt");
  TMSRPT[61].lpRoutine = TMSRPT62;
  TMSRPT[61].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[61].ConfigFuncDlg = NO_RECORD;
//
//  Turn sheet
//
  TMSRPT[62].flags = TMSRPTFLAG_REPORT;
  TMSRPT[62].originalReportNumber = 62;
  strcpy(TMSRPT[62].szReportName, "Turn Sheet");
  TMSRPT[62].numReports = 1;
  strcpy(TMSRPT[62].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[62].szReportFileName[0], "\\Turn Sheet.rpt");
  TMSRPT[62].lpRoutine = TMSRPT63;
  TMSRPT[62].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[62].ConfigFuncDlg = NO_RECORD;
//
//  Existing Node Data Load
//
  TMSRPT[63].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[63].originalReportNumber = 63;
  strcpy(TMSRPT[63].szReportName, "Load Data from TMSweb Bus Stop Editor");
  TMSRPT[63].numReports = 1;
  strcpy(TMSRPT[63].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[63].lpRoutine = TMSRPT64;
  TMSRPT[63].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[63].ConfigFuncDlg = NO_RECORD;
//
//  Web-Based Trip Planner Download
//
  TMSRPT[64].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[64].originalReportNumber = 64;
  strcpy(TMSRPT[64].szReportName, "Web-Based Trip Planner Download");
  TMSRPT[64].numReports = 1;
  strcpy(TMSRPT[64].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[64].lpRoutine = TMSRPT65;
  TMSRPT[64].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[64].ConfigFuncDlg = NO_RECORD;
//
//  Avail Download for Grand Rapids
//
  TMSRPT[65].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[65].originalReportNumber = 65;
  strcpy(TMSRPT[65].szReportName, "AVL - Avail Interface Download");
  TMSRPT[65].numReports = 1;
  strcpy(TMSRPT[65].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[65].lpRoutine = TMSRPT66;
  TMSRPT[65].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[65].ConfigFuncDlg = NO_RECORD;
//
//  Ontira Download for MCT
//
  TMSRPT[66].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[66].originalReportNumber = 66;
  strcpy(TMSRPT[66].szReportName, "AVL - Ontira Interface Download");
  TMSRPT[66].numReports = 1;
  strcpy(TMSRPT[66].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[66].lpRoutine = TMSRPT67;
  TMSRPT[66].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[66].ConfigFuncDlg = NO_RECORD;
//
//  Unpaid time report
//
  TMSRPT[67].flags = TMSRPTFLAG_REPORT;
  TMSRPT[67].originalReportNumber = 67;
  strcpy(TMSRPT[67].szReportName, "Unpaid Time");
  TMSRPT[67].numReports = 1;
  strcpy(TMSRPT[67].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[67].szReportFileName[0], "\\Unpaid Time.rpt");
  TMSRPT[67].lpRoutine = TMSRPT68;
  TMSRPT[67].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[67].ConfigFuncDlg = NO_RECORD;
//
//  Bus Data Load
//
  TMSRPT[68].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[68].originalReportNumber = 68;
  strcpy(TMSRPT[68].szReportName, "Load Bus Data");
  TMSRPT[68].numReports = 1;
  strcpy(TMSRPT[68].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[68].lpRoutine = TMSRPT69;
  TMSRPT[68].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[68].ConfigFuncDlg = NO_RECORD;
//
//  Continental Download for Bridgeport
//
  TMSRPT[69].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[69].originalReportNumber = 69;
  strcpy(TMSRPT[69].szReportName, "AVL - Continental Interface Download");
  TMSRPT[69].numReports = 1;
  strcpy(TMSRPT[69].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[69].lpRoutine = TMSRPT70;
  TMSRPT[69].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[69].ConfigFuncDlg = NO_RECORD;
//
//  Driver Paddle (3)
//
  TMSRPT[70].flags = TMSRPTFLAG_REPORT;
  TMSRPT[70].originalReportNumber = 70;
  strcpy(TMSRPT[70].szReportName, "Driver Paddle (3)");
  TMSRPT[70].numReports = 1;
  strcpy(TMSRPT[70].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[70].szReportFileName[0], "\\Driver Paddle (3).rpt");
  TMSRPT[70].lpRoutine = TMSRPT71;
  TMSRPT[70].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[70].ConfigFuncDlg = NO_RECORD;
//
//  RouteMatch Download for Ocean County
//
  TMSRPT[71].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[71].originalReportNumber = 71;
  strcpy(TMSRPT[71].szReportName, "AVL - RouteMatch Interface Download");
  TMSRPT[71].numReports = 1;
  strcpy(TMSRPT[71].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[71].lpRoutine = TMSRPT72;
  TMSRPT[71].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[71].ConfigFuncDlg = NO_RECORD;
//
//  Ridership - Daily Detail
//
  TMSRPT[72].flags = TMSRPTFLAG_REPORT;
  TMSRPT[72].originalReportNumber = 72;
  strcpy(TMSRPT[72].szReportName, "Ridership - Daily Detail");
  TMSRPT[72].numReports = 1;
  strcpy(TMSRPT[72].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[72].szReportFileName[0], "\\Ridership - Daily Detail.rpt");
  TMSRPT[72].lpRoutine = TMSRPT73;
  TMSRPT[72].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[72].ConfigFuncDlg = NO_RECORD;
//
//  Ridership - Daily Passenger Tally
//
  TMSRPT[73].flags = TMSRPTFLAG_REPORT;
  TMSRPT[73].originalReportNumber = 73;
  strcpy(TMSRPT[73].szReportName, "Ridership - Daily Passenger Tally");
  TMSRPT[73].numReports = 1;
  strcpy(TMSRPT[73].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[73].szReportFileName[0], "\\Ridership - Daily Passenger Tally.rpt");
  TMSRPT[73].lpRoutine = TMSRPT74;
  TMSRPT[73].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[73].ConfigFuncDlg = NO_RECORD;
//
//  Ridership - Monthly Service Report
//
  TMSRPT[74].flags = TMSRPTFLAG_REPORT;
  TMSRPT[74].originalReportNumber = 74;
  strcpy(TMSRPT[74].szReportName, "Ridership - Monthly Service Report");
  TMSRPT[74].numReports = 1;
  strcpy(TMSRPT[74].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[74].szReportFileName[0], "\\Ridership - Monthly Service Report.rpt");
  TMSRPT[74].lpRoutine = TMSRPT75;
  TMSRPT[74].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[74].ConfigFuncDlg = NO_RECORD;
//
//  Ridership - Operator's Daily Report
//
  TMSRPT[75].flags = TMSRPTFLAG_REPORT;
  TMSRPT[75].originalReportNumber = 75;
  strcpy(TMSRPT[75].szReportName, "Ridership - Operator's Daily Report");
  TMSRPT[75].numReports = 2;
  strcpy(TMSRPT[75].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[75].szReportFileName[0], "\\Ridership - Operator's Daily Report.rpt");
  strcpy(TMSRPT[75].szReportFileName[1], szReportTemplateFolder);
  strcat(TMSRPT[75].szReportFileName[1], "\\Ridership - Operator's Daily Summary.rpt");
  TMSRPT[75].lpRoutine = TMSRPT76;
  TMSRPT[75].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[75].ConfigFuncDlg = NO_RECORD;
//
//  Jurisdictional Time and Distance Report
//
  TMSRPT[76].flags = TMSRPTFLAG_REPORT;
  TMSRPT[76].originalReportNumber = 76;
  strcpy(TMSRPT[76].szReportName, "Jurisdictional Time and Distance Report");
  TMSRPT[76].numReports = 1;
  strcpy(TMSRPT[76].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[76].szReportFileName[0], "\\Jurisdictional Time and Distance.rpt");
  TMSRPT[76].lpRoutine = TMSRPT77;
  TMSRPT[76].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[76].ConfigFuncDlg = NO_RECORD;
//
//  Bus Condition Report
//
  TMSRPT[77].flags = TMSRPTFLAG_REPORT;
  TMSRPT[77].originalReportNumber = 77;
  strcpy(TMSRPT[77].szReportName, "Bus Condition Report");
  TMSRPT[77].numReports = 1;
  strcpy(TMSRPT[77].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[77].szReportFileName[0], "\\Bus Condition Report.rpt");
  TMSRPT[77].lpRoutine = TMSRPT78;
  TMSRPT[77].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[77].ConfigFuncDlg = NO_RECORD;
//
//  Excel-type download of Kalamazoo Payroll
//
  TMSRPT[78].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[78].originalReportNumber = 78;
  strcpy(TMSRPT[78].szReportName, "Kalamazoo Payroll Download to Excel");
  TMSRPT[78].numReports = 1;
  strcpy(TMSRPT[78].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[78].lpRoutine = TMSRPT79;
  TMSRPT[78].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[78].ConfigFuncDlg = NO_RECORD;
//
//  Licence and medical expiries
//
  TMSRPT[79].flags = TMSRPTFLAG_REPORT;
  TMSRPT[79].originalReportNumber = 79;
  strcpy(TMSRPT[79].szReportName, "License and Medical Expiries");
  TMSRPT[79].numReports = 1;
  strcpy(TMSRPT[79].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[79].szReportFileName[0], "\\License and Medical Expiries.rpt");
  TMSRPT[79].lpRoutine = TMSRPT80;
  TMSRPT[79].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[79].ConfigFuncDlg = NO_RECORD;
//
//  Excel-type download of schedule data for C-Tran
//
  TMSRPT[80].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[80].originalReportNumber = 80;
  strcpy(TMSRPT[80].szReportName, "C-Tran Excel Export");
  TMSRPT[80].numReports = 1;
  strcpy(TMSRPT[80].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[80].lpRoutine = TMSRPT81;
  TMSRPT[80].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[80].ConfigFuncDlg = NO_RECORD;
//
//  Load Driver Vacation/Sick Pool data
//
  TMSRPT[81].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[81].originalReportNumber = 81;
  strcpy(TMSRPT[81].szReportName, "Load Driver Vacation/Sick Bank Data");
  TMSRPT[81].numReports = 1;
  strcpy(TMSRPT[81].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[81].lpRoutine = TMSRPT82;
  TMSRPT[81].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[81].ConfigFuncDlg = NO_RECORD;
//
//  Payroll Detail and Summary
//
  TMSRPT[82].flags = TMSRPTFLAG_REPORT;
  TMSRPT[82].originalReportNumber = 82;
  strcpy(TMSRPT[82].szReportName, "Payroll Detail and Summary");
  TMSRPT[82].numReports = 1;
  strcpy(TMSRPT[82].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[82].szReportFileName[0], "\\Payroll Detail and Summary.rpt");
  TMSRPT[82].lpRoutine = TMSRPT83;
  TMSRPT[82].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[82].ConfigFuncDlg = NO_RECORD;
//
//  Strategic Mapping Download for DASH
//
  TMSRPT[83].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[83].originalReportNumber = 83;
  strcpy(TMSRPT[83].szReportName, "AVL - Strategic Mapping Interface Download");
  TMSRPT[83].numReports = 1;
  strcpy(TMSRPT[83].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[83].lpRoutine = TMSRPT84;
  TMSRPT[83].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[83].ConfigFuncDlg = NO_RECORD;
//
//  Driver Paddle (4)
//
  TMSRPT[84].flags = TMSRPTFLAG_REPORT;
  TMSRPT[84].originalReportNumber = 84;
  strcpy(TMSRPT[84].szReportName, "Driver Paddle (4)");
  TMSRPT[84].numReports = 1;
  strcpy(TMSRPT[84].szReportFileName[0], szReportTemplateFolder);
  strcat(TMSRPT[84].szReportFileName[0], "\\Driver Paddle (4).rpt");
  TMSRPT[84].lpRoutine = TMSRPT85;
  TMSRPT[84].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[84].ConfigFuncDlg = NO_RECORD;
//
//  Connexionz Download (V2)
//
  TMSRPT[85].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[85].originalReportNumber = 85;
  strcpy(TMSRPT[85].szReportName, "AVL - Connexionz Interface (V2) Download");
  TMSRPT[85].numReports = 1;
  strcpy(TMSRPT[85].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[85].lpRoutine = TMSRPT86;
  TMSRPT[85].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[85].ConfigFuncDlg = NO_RECORD;
//
//  Connexions Load
//
  TMSRPT[86].flags = TMSRPTFLAG_UPLOAD;
  TMSRPT[86].originalReportNumber = 86;
  strcpy(TMSRPT[86].szReportName, "Load Data from Connexionz");
  TMSRPT[86].numReports = 1;
  strcpy(TMSRPT[86].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[86].lpRoutine = TMSRPT87;
  TMSRPT[86].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[86].ConfigFuncDlg = NO_RECORD;
//
//  ITS4mobility Download for Sudbury Transit
//
  TMSRPT[87].flags = TMSRPTFLAG_DOWNLOAD;
  TMSRPT[87].originalReportNumber = 87;
  strcpy(TMSRPT[87].szReportName, "AVL - ITS4mobility Interface Download");
  TMSRPT[87].numReports = 1;
  strcpy(TMSRPT[87].szReportFileName[0], "");  // This report doesn't use Crystal
  TMSRPT[87].lpRoutine = TMSRPT88;
  TMSRPT[87].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[87].ConfigFuncDlg = NO_RECORD;
//
//  Last one - hard coded to "-Done-" to let us know we're finished
//
  TMSRPT[LASTREPORT].flags = 0;
  TMSRPT[LASTREPORT].originalReportNumber = 0;
  TMSRPT[LASTREPORT].numReports = 1;
  strcpy(TMSRPT[LASTREPORT].szReportName, "-Done-");
  strcpy(TMSRPT[LASTREPORT].szReportFileName[0], "");
  TMSRPT[LASTREPORT].lpRoutine = NULL;
  TMSRPT[LASTREPORT].lpConfigFunc = (TMSRPTConfigFuncDef)NULL;
  TMSRPT[LASTREPORT].ConfigFuncDlg = NO_RECORD;
  m_LastReport = LASTREPORT;
//
//  Sort the reports so they display in a reasonable fashion
//
  qsort((void *)TMSRPT, LASTREPORT, sizeof(TMSRPTDef), sort_reports);
//
//  See which reports are active
//
  DetermineActiveReports();
//
//  All done
//
  return;
}

//
//  DetermineActiveReports() - Check the registry to see which reports are "switched on"
//
void CTMSApp::DetermineActiveReports(void)
{
  int nI, nJ, nK;
//
//  Get all the bitmasks
//
  for(nI = 0; nI < TMSRPT_MAX_BITMASKS; nI++)
  {
    sprintf(tempString, "ActiveReportMask%02d", nI + 1);
    bmReportIsActive[nI] = GetProfileInt("Reports", tempString, 0);    
  }
//
//  Cycle through to populate the flag fields of TMSRPT
//
  for(nI = 0; nI < m_LastReport; nI++)
  {
    nJ = nI / 32;
    nK = 1 << (nI % 32);
    if(bmReportIsActive[nJ] & nK)
    {
      TMSRPT[nI].flags |= TMSRPTFLAG_ACTIVE;
    }
  }
}