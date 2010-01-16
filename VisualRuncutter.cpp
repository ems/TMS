//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
// VisualRuncutter.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}

#include "ResizingDialog.h"
#include "FlatSB.h"
#include "CutRuns.h"
#include "BlockProperties.h"
#include "VisualRuncutter.h"
#include "ColorBtn.h"
#include "VRColors.h"
#include "JumpToRun.h"
#include "UndoRuns.h"
#include "PremiumDefinition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Sorting hooks
//
//  SortByBlockNumber()
//

int SortByBlockNumber(const void *a, const void *b)
{
  BLOCKPLOTDef *pa, *pb;

  pa = (BLOCKPLOTDef *)a;
  pb = (BLOCKPLOTDef *)b;
  
  return(pa->blockNumber < pb->blockNumber ? -1 : pa->blockNumber > pb->blockNumber ? 1 : 0);
}

//
//  SortByStartTime()
//

int SortByStartTime(const void *a, const void *b)
{
  BLOCKPLOTDef *pa, *pb;

  pa = (BLOCKPLOTDef *)a;
  pb = (BLOCKPLOTDef *)b;
  
  return(pa->startTime < pb->startTime ? -1 : pa->startTime > pb->startTime ? 1 : 0);
}

//
//  SortByEndTime()
//

int SortByEndTime(const void *a, const void *b)
{
  BLOCKPLOTDef *pa, *pb;

  pa = (BLOCKPLOTDef *)a;
  pb = (BLOCKPLOTDef *)b;
  
  return(pa->endTime < pb->endTime ? -1 : pa->endTime > pb->endTime ? 1 : 0);
}

//
//  SortByBlockLength()
//

int SortByBlockLength(const void *a, const void *b)
{
  BLOCKPLOTDef *pa, *pb;
  long aLength, bLength;

  pa = (BLOCKPLOTDef *)a;
  pb = (BLOCKPLOTDef *)b;

  aLength = pa->endTime - pa->startTime;
  bLength = pb->endTime - pb->startTime;
  
  return(aLength < bLength ? -1 : aLength > bLength ? 1 : 0);
}

//
//  C++ front-end to the visual runcutter
//

int VisualRuncutterDialog(PDISPLAYINFO pDI)
{
  CVisualRuncutter dlg(NULL, pDI);

  return(dlg.DoModal());
}

//
//  C front-end to the visual runcutter
//

extern "C"
{
int VisualRuncutter(PDISPLAYINFO pDI)
{
  return(VisualRuncutterDialog(pDI));
}
}  // extern "C"


//
//  CVisualRuncutter dialog
//
//  Constructor
//

CVisualRuncutter::CVisualRuncutter(CWnd* pParent, PDISPLAYINFO pDI)
	: CResizingDialog(CVisualRuncutter::IDD, pParent)
{
  m_pDI = pDI;
  pButtonSAVE = NULL;
  pFlatSBSCROLLBAR = NULL;
  m_CurrentDisplayFlag = VR_FLAG_ALLBLOCKS;
  m_CurrentSortFlag = VR_FLAG_SORTEDBYBLOCKNUMBER;
  m_bDisplayReliefPointHashMarks = FALSE;
  m_numInDisplayList = 0;
  m_pBP = NULL;
  m_LButtonPoint.x = NO_RECORD;
  m_LButtonPoint.y = NO_RECORD;

	//{{AFX_DATA_INIT(CVisualRuncutter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
//
//  Hold the dialog controls in place during resize
//
  CDWordArray c_info;
  SetControlInfo(IDPARAMETERS, ANCHORE_RIGHT);
  SetControlInfo(IDRUNTYPES,   ANCHORE_RIGHT);
  SetControlInfo(IDWORKRULES,  ANCHORE_RIGHT);
  SetControlInfo(IDCOLORS,     ANCHORE_RIGHT);
  SetControlInfo(IDSAVE,       ANCHORE_RIGHT);
  SetControlInfo(IDOK,         ANCHORE_RIGHT);
  SetControlInfo(IDCANCEL,     ANCHORE_RIGHT);
  SetControlInfo(IDHELP,       ANCHORE_RIGHT);
  SetControlInfo(IDRECURSIVE,  ANCHORE_RIGHT);
  SetControlInfo(IDMINCOST,    ANCHORE_RIGHT);
  SetControlInfo(VR_SCROLLBAR, ANCHORE_RIGHT | RESIZE_VER);
  SetControlInfo(VR_DISPLAYSTATUS, ANCHORE_BOTTOM);
	m_bDrawGripper = TRUE;
}

//
//  DoDataExchange
//
void CVisualRuncutter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVisualRuncutter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

//
//  Message map
//

BEGIN_MESSAGE_MAP(CVisualRuncutter, CDialog)
	//{{AFX_MSG_MAP(CVisualRuncutter)
	ON_BN_CLICKED(IDSAVE, OnSave)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDPARAMETERS, OnParameters)
	ON_BN_CLICKED(IDRUNTYPES, OnRuntypes)
	ON_BN_CLICKED(IDWORKRULES, OnWorkrules)
	ON_COMMAND(VRB_CUT_RECURSIVE, OnCutRecursive)
	ON_COMMAND(VRB_CUT_MINCOST, OnCutMincost)
	ON_COMMAND(VRB_CUT_CREWCUT, OnCutCrewcut)
	ON_COMMAND(VRB_PROPERTIES, OnProperties)
	ON_COMMAND(VRB_SELECTALL, OnSelectall)
	ON_COMMAND(VRB_DESELECTALL, OnDeselectall)
	ON_BN_CLICKED(IDCOLORS, OnColors)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(VRB_DISPLAY_ALLBLOCKS, OnDisplayAllblocks)
	ON_COMMAND(VRB_DISPLAY_LEFTOVERS, OnDisplayLeftovers)
	ON_WM_CLOSE()
	ON_COMMAND(VRB_SORT_BLOCKNUMBER, OnSortBlocknumber)
	ON_COMMAND(VRB_SORT_OFFTIME, OnSortOfftime)
	ON_COMMAND(VRB_SORT_ONTIME, OnSortOntime)
	ON_COMMAND(VRB_SORT_BLOCKLENGTH, OnSortBlocklength)
	ON_COMMAND(VRB_JUMPTORUN, OnJumptorun)
	ON_COMMAND(VRB_CUT_WHATSLEFT, OnCutWhatsleft)
	ON_COMMAND(VRB_DISPLAY_ILLEGAL, OnDisplayIllegal)
	ON_COMMAND(VRB_RUNSLIDER, OnRunslider)
	ON_COMMAND(VRB_DISPLAY_RELIEFHASHMARKS, OnDisplayReliefhashmarks)
	ON_BN_CLICKED(IDRECURSIVE, OnRecursive)
	ON_BN_CLICKED(IDMINCOST, OnMincost)
	ON_COMMAND(VRB_UNDORUNS, OnUndoruns)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//  CVisualRuncutter message handlers
//
//  OnInitDialog()
//

BOOL CVisualRuncutter::OnInitDialog() 
{
	CResizingDialog::OnInitDialog();

  long blockNumber;
  int  nI;
  int  nJ;
  int  rcode2;
//
//  Set up the handles to the controls
//
  pFlatSBSCROLLBAR = (CFlatSB *)GetDlgItem(VR_SCROLLBAR);
  pButtonSAVE = (CButton *)GetDlgItem(IDSAVE);
  pButtonOK = (CButton *)GetDlgItem(IDOK);
  pButtonCANCEL = (CButton *)GetDlgItem(IDCANCEL);
  pButtonHELP = (CButton *)GetDlgItem(IDHELP);
  pStaticDISPLAYSTATUS = (CStatic *)GetDlgItem(VR_DISPLAYSTATUS);
//
//  Colors
//
  COLORREF cRed = RGB(255, 0, 0);
  COLORREF cGreen = RGB(0, 255, 0);
  COLORREF cBlue = RGB(0, 0, 255);
  COLORREF cYellow = RGB(255, 255, 0);
   
  m_VRColors.illegal = GetPrivateProfileInt(userName, "VRColorIllegal", cRed, TMSINIFile);
  m_VRColors.cut = GetPrivateProfileInt(userName, "VRColorCut", cGreen, TMSINIFile);
  m_VRColors.frozen = GetPrivateProfileInt(userName, "VRColorFrozen", cBlue, TMSINIFile);
  m_VRColors.uncut = GetPrivateProfileInt(userName, "VRColorUncut", cYellow, TMSINIFile);
//
//  Verify we have at least one garage
//
  bGenerateTravelInstructions = FALSE;
  if(numGaragesInGarageList == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
    AbortInit();
    return(TRUE);
  }
//
//  Establish m_GlobalRunNumber
//
  RUNSKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
  RUNSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID + 1;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETLESSTHAN, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  m_GlobalRunNumber = rcode2 == 0 ? RUNS.runNumber + 1 : 1;
//
//  Build the relief point list from the selected blocks
//
  SetUpReliefPoints(m_hWnd, m_pDI, NULL, 0, NULL, TRUE);
  if(m_numRELIEFPOINTS == NO_RECORD)
  {
    AbortInit();
    return(TRUE);
  }
//
//  Count the unique blocks
//
  blockNumber = NO_RECORD;
  numBPBlocks = 0;
  for(nI = 0; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].blockNumber != blockNumber)
    {
      numBPBlocks++;
      blockNumber = m_pRELIEFPOINTS[nI].blockNumber;
    }
  }
//
//  There needs to be at least one
//
  if(numBPBlocks == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_299, (HANDLE)NULL);
    AbortInit();
    return(TRUE);
  }
//
//  Allocate space for the structure
//
//
  m_pBP = (BLOCKPLOTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BLOCKPLOTDef) * numBPBlocks); 
  if(m_pBP == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    AbortInit();
    return(TRUE);
  }
//
//  Save the pertinent block information
//
  m_pBP[0].blockNumber = m_pRELIEFPOINTS[0].blockNumber;
  m_pBP[0].startTime = m_pRELIEFPOINTS[0].time;
  m_pBP[0].startIndex = 0;
  m_pBP[0].endIndex = 0;
  m_pBP[0].flags = BP_DISPLAYED;
  nJ = 0;
  for(nI = 1; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].blockNumber != m_pBP[nJ].blockNumber)
    {
      m_pBP[nJ].endTime = m_pRELIEFPOINTS[nI - 1].time;
      nJ++;
      m_pBP[nJ].blockNumber = m_pRELIEFPOINTS[nI].blockNumber;
      m_pBP[nJ].startTime = m_pRELIEFPOINTS[nI].time;
      m_pBP[nJ].startIndex = nI;
      m_pBP[nJ].flags = BP_DISPLAYED;
    }
    m_pBP[nJ].endTime = m_pRELIEFPOINTS[nI].time;
    m_pBP[nJ].endIndex = nI;
  }
//
//  Set up the pens
//
  m_Pens[CUT].CreatePen(PS_SOLID,     THICK, m_VRColors.cut);
  m_Pens[UNCUT].CreatePen(PS_SOLID,   THICK, m_VRColors.uncut);
  m_Pens[FROZEN].CreatePen(PS_SOLID,  THICK, m_VRColors.frozen);
  m_Pens[ILLEGAL].CreatePen(PS_SOLID, THICK, m_VRColors.illegal);
  m_Pens[PERIMETER].CreatePen(PS_SOLID,   1, RGB(  0,   0, 255));  // Blue
  m_Pens[HASH].CreatePen(PS_SOLID,    THICK, RGB(  0,   0,   0));  // Black
  m_Pens[TXT].CreatePen(PS_SOLID, THICK + 2, RGB(  0,   0,   0));  // Black
  m_Pens[AXIS].CreatePen(PS_SOLID,        1, RGB(192, 192, 192));  // Light Grey
  m_Pens[WTXT].CreatePen(PS_SOLID,        1, RGB(255, 255, 255));  // White
//
//  Is there a size/position from before?
//
  m_nCaptionSize = GetSystemMetrics(SM_CYCAPTION);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"VisualRuncutterPosition", "",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  int x;
  int y;
  int nWidth;
  int nHeight;

  if(strcmp(tempString, "") != 0)
  {
    strcpy(szarString, strtok(tempString, " "));
    x = atol(szarString);
    strcpy(szarString, strtok(NULL, " "));
    y = atol(szarString);
    strcpy(szarString, strtok(NULL, " "));
    nWidth = atol(szarString);
    strcpy(szarString, strtok(NULL, "\0"));
    nHeight = atol(szarString);
    MoveWindow(x, y, nWidth, nHeight);
  }
//
//  Nope - get where we are and save it
//
  else
  {
    SaveDialogRect();
  }
//
//  Set the rectangle of the viewing area, its viewing
//  extents, and the latest and earliest block times 
//
  EstablishViewingArea();
  UpdateDisplayStatus();
//
//  Set the maximum value for the scroll bar and the jump increment
//
  short int nChange = BLOCKSDISPLAYED - 1;

  pFlatSBSCROLLBAR->SetMax(numBPBlocks);
  pFlatSBSCROLLBAR->SetLargeChange(nChange);
//
//  Set the starting position of the display
//
  m_firstDisplayed = 0;
  m_LastIndex = NO_RECORD;
//
//  And display the cut runs window
//
  m_CutRunsDlg.Create(IDD_CUTRUNS, this);
  m_CutRunsDlg.SayHello(this);
  m_bDisplayingCutRuns = TRUE;
  MoveCutRunsWindow();
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//
//  OnSave() - Write out the data structures and stay within the visual runcutter
//

void CVisualRuncutter::OnSave() 
{
  SaveRuncut();
}

//
//  SaveRuncut() - Used by OnSave() and OnOK()
//

BOOL CVisualRuncutter::SaveRuncut()
{
  HCURSOR hSaveCursor;
  CString s;
  long recordID;
  int  rcode2;
  int  numInternalRuns;
  int  nI;
  int  nJ;
  int  nK;

  MessageBeep(MB_ICONINFORMATION);
  s.LoadString(ERROR_160);
  if(MessageBox(s, TMS, MB_ICONINFORMATION | MB_YESNO) == IDNO)
  {
    return(FALSE);
  }
  hSaveCursor = SetCursor(hCursorWait);
//
//  Nuke the previous runcut, but get the last recordID first
//
  rcode2 = btrieve(B_GETLAST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  recordID = AssignRecID(rcode2, RUNS.recordID);
  RUNSKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
  RUNSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 &&
        RUNS.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID &&
        RUNS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID)
  {
    btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Loop through the cut runs
//
  numInternalRuns = AssembleRuns();
  for(nI = 0; nI < numInternalRuns; nI++)
  {
//
//  Establish the runtype and comment code
//
    RUNS.cutAsRuntype = NO_RECORD;
    RUNS.COMMENTSrecordID = NO_RECORD;
    for(nK = 0; nK < MAXRUNSINRUNLIST; nK++)
    {
      if(RUNLIST[nI].runNumber == RUNLISTDATA[nK].runNumber)
      {
        RUNS.cutAsRuntype = RUNLISTDATA[nK].cutAsRuntype;
        RUNS.COMMENTSrecordID = RUNLISTDATA[nK].COMMENTSrecordID;
        break;
      }
    }
//
//  Loop through the pieces and write out the runs
//
    for(nJ = 0; nJ < RUNLIST[nI].run.numPieces; nJ++)
    {
      if(RUNLIST[nI].recordID[nJ] == NO_RECORD)
      {
        RUNS.recordID = recordID++;
        RUNS.flags = 0L;
        RUNS.prior.startTime = NO_TIME;
        RUNS.prior.endTime = NO_TIME;
        RUNS.after.startTime = NO_TIME;
        RUNS.after.endTime = NO_TIME;
      }
      else
      {
        RUNS.recordID = RUNLIST[nI].recordID[nJ];
        RUNS.flags = RUNLIST[nI].run.piece[nJ].flags;
		    RUNS.prior.startTime = RUNLIST[nI].run.piece[nJ].prior.startTime;
			  RUNS.prior.endTime = RUNLIST[nI].run.piece[nJ].prior.endTime;
        RUNS.after.startTime = RUNLIST[nI].run.piece[nJ].after.startTime;
        RUNS.after.endTime = RUNLIST[nI].run.piece[nJ].after.endTime;
      }
      RUNS.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
      RUNS.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
      RUNS.start.TRIPSrecordID = RUNLIST[nI].run.piece[nJ].fromTRIPSrecordID;
      RUNS.start.NODESrecordID = RUNLIST[nI].run.piece[nJ].fromNODESrecordID;
      RUNS.end.TRIPSrecordID = RUNLIST[nI].run.piece[nJ].toTRIPSrecordID;
      RUNS.end.NODESrecordID = RUNLIST[nI].run.piece[nJ].toNODESrecordID;
      RUNS.runNumber = RUNLIST[nI].runNumber;
      RUNS.pieceNumber = nJ + 1;
      btrieve(B_INSERT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    }
  }
  SetCursor(hSaveCursor);

  return(TRUE);
}

//
//  OnOK()
//

void CVisualRuncutter::OnOK() 
{
  if(!SaveRuncut())
  {
    return;
  }

  Cleanup(TRUE);
	CDialog::OnOK();
}

//
//  OnCancel()
//

void CVisualRuncutter::OnCancel() 
{
  CString s;

  MessageBeep(MB_ICONINFORMATION);
  s.LoadString(ERROR_159);
  if(MessageBox(s, TMS, MB_ICONINFORMATION | MB_YESNO) == IDYES)
  {
    Cleanup(TRUE);
		CDialog::OnCancel();
  }
}

//
//  OnClose() - Essentially an OnCancel()
//

void CVisualRuncutter::OnClose() 
{
  OnCancel();
}

//
//  Cleanup() - Deallocate memory
//

void CVisualRuncutter::Cleanup(BOOL bSaveAndDestroy)
{
  TMSHeapFree(m_pBP);

  for(int nI = 0; nI < LASTPEN; nI++)
  {
    m_Pens[nI].Detach();
    m_Pens[nI].DeleteObject();
  }

  if(bSaveAndDestroy)
  {
    if(m_bDisplayingCutRuns)
    {
      SaveCutRunsRect();
      m_CutRunsDlg.DestroyWindow();
    }
  
    SaveDialogRect();
  }
}

//
//  OnHelp() - Display help
//

void CVisualRuncutter::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Technical_Support);
}

//
//  OnPaint() - Draw the blocks window
//

void CVisualRuncutter::OnPaint() 
{
  CResizingDialog::OnPaint();

  CPaintDC dc(this);

  RECT  rcDraw;
  long  tempTime;
  long  previousRunNumber;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;

  POINT startPiece;
  POINT midPiece;
  long  fromTime, totalTime;
    
//
//  Set up the font
//
  CFont   Font;
  LOGFONT lf;

  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(7, dc.GetDeviceCaps(LOGPIXELSY), 72);
  lf.lfWeight = FW_THIN;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
  strcpy(lf.lfFaceName, "Tahoma");
  Font.CreateFontIndirect(&lf);
//
//  Get the average character width
//
  TEXTMETRIC tm;

  dc.GetTextMetrics(&tm);
  if(tm.tmAveCharWidth <= 0)
    tm.tmAveCharWidth = 4;
//
//  Set up the rest of the device context's properties
//
  dc.SelectObject(Font);
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(RGB(0, 0, 0));
//
//  Write out the time of day on the top
//
  memcpy(&rcDraw, &m_rc, sizeof(RECT));
//
//  Adjust the left and right to ensure there's room for a block number
//
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  rcDraw.top -= m_nCaptionSize;
  dc.SelectObject(m_Pens[PERIMETER]);
  dc.MoveTo(m_rc.left, m_rc.top);
  dc.LineTo(m_rc.right - 2, m_rc.top);
  dc.LineTo(m_rc.right - 2, m_rc.bottom - 2);
  dc.LineTo(m_rc.left, m_rc.bottom - 2);
  dc.LineTo(m_rc.left, m_rc.top);
  dc.SelectObject(m_Pens[TXT]);
  tempTime = m_earliestTime;
  while(tempTime <= m_latestTime)
  {
    strcpy(tempString, Tchar(tempTime));
    if(m_timeInc < 28)  // So we get no overlapping times
    {
      tempString[2] = tempString[4];
      tempString[3] = '\0';
    } 
    dc.TextOut(rcDraw.left - 10, rcDraw.top, tempString, strlen(tempString));
    tempTime += 3600;
    rcDraw.left += m_timeInc;
  }
//
//  Write out the axis lines dropping from the time of day
//
  memcpy(&rcDraw, &m_rc, sizeof(RECT));
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  dc.SelectObject(m_Pens[AXIS]);
  tempTime = m_earliestTime;
  while(tempTime <= m_latestTime)
  {
    dc.MoveTo(rcDraw.left, rcDraw.top - 4);
    dc.LineTo(rcDraw.left, rcDraw.bottom - 2);
    tempTime += 3600;
    rcDraw.left += m_timeInc;
  }
//
//  Set up the display list
//
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
//
//  Set up the RELIEFPOINTS x,y co-ords
//
  int   dl;
  POINT point;

  memcpy(&rcDraw, &m_rc, sizeof(RECT));
  rcDraw.left += INDENT;
  rcDraw.right -= INDENT;
  for(nI = 0; nI < m_numInDisplayList; nI++)
  {
    dl = m_DisplayList.GetAt(nI);
    m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.x = 0;
    m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y = 0;
  }
  point.y = rcDraw.top + INDENT - 2;
  for(nI = m_firstDisplayed; nI < m_numInDisplayList; nI++)
  {
    dl = m_DisplayList.GetAt(nI);
    if(point.y > rcDraw.bottom)
    {
      m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y = NO_RECORD;
      break;
    }
    for(nJ = m_pBP[dl].startIndex; nJ <= m_pBP[dl].endIndex; nJ++)
    {
      point.x = (long)(rcDraw.left +
            (((float)(m_pRELIEFPOINTS[nJ].time - m_earliestTime) / 3600) * m_timeInc));
      m_pRELIEFPOINTS[nJ].plot.x = point.x;
      m_pRELIEFPOINTS[nJ].plot.y = point.y;
    }
    m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.x += 4;  // It looks better
    point.y += (long)(INDENT * 1.5);
  }
//
//  Set up the clipping region
//
  CRgn rgn;
  
  rgn.CreateRectRgn(m_rc.left, m_rc.top, m_rc.right - 2, m_rc.bottom - 2);
  dc.SelectClipRgn(&rgn);
//
//  Display the selected blocks
//
  for(nI = m_firstDisplayed; nI < m_numInDisplayList; nI++)
  {
    dl = m_DisplayList.GetAt(nI);
    if(m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y == NO_RECORD)  // End of paintable area
      break;
//
//  Front of block
//
    nJ = m_pBP[dl].startIndex;
//
//  Block number
//
    sprintf(tempString, "%ld", m_pBP[dl].blockNumber);
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - ((strlen(tempString) + 1) * tm.tmAveCharWidth),
          m_pRELIEFPOINTS[nJ].plot.y - 6,
          tempString, strlen(tempString));
//
//  Pullout node and time
//
    nJ = m_pBP[dl].startIndex;
    strcpy(tempString, NodeAbbrFromRecID(m_pRELIEFPOINTS[nJ].NODESrecordID));
//
//  The node name goes above the line
//
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth * 2,
          m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
    strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
    if(tempString[0] == ' ')
      memcpy(tempString, &tempString[1], strlen(tempString));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth * 2,
          m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));
//
//  End of block
//
    nJ = m_pBP[dl].endIndex;
//
//  Block length
//
    long blockLength = m_pRELIEFPOINTS[nJ].time -
          m_pRELIEFPOINTS[m_pBP[dl].startIndex].time;

    strcpy(tempString, chhmm(blockLength));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x + tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y - 6,
          tempString, strlen(tempString));
//
//  Pullin node and time
//
    strcpy(tempString, NodeAbbrFromRecID(m_pRELIEFPOINTS[nJ].NODESrecordID));
//
//  The node name goes above the line
//
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
    strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
    if(tempString[0] == ' ')
      memcpy(tempString, &tempString[1], strlen(tempString));
    dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - tm.tmAveCharWidth,
          m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));

//
//  Cycle through all the relief points
//
    previousRunNumber = NO_RECORD;
    for(nJ = m_pBP[dl].startIndex + 1;
          nJ <= m_pBP[dl].endIndex; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == NO_RECORD)
      {
        dc.SelectObject(m_Pens[UNCUT]);
        previousRunNumber = NO_RECORD;
      }
      else
      {
        if(m_pRELIEFPOINTS[nJ - 1].start.runNumber != previousRunNumber)
        {
          PROPOSEDRUNDef PROPOSEDRUN;
          COSTDef COST;
          long cutAsRuntype;
          BOOL bFrozen;

          if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == SECURELOCATION_RUNNUMBER)
            dc.SelectObject(m_Pens[HASH]);
          else
          {
            for(nK = 0; nK < MAXRUNSINRUNLIST; nK++)
            {
              if(RUNLIST[nK].runNumber == m_pRELIEFPOINTS[nJ - 1].start.runNumber)
              {
                memcpy(&PROPOSEDRUN, &RUNLIST[nK].run, sizeof(PROPOSEDRUN));
                for(cutAsRuntype = NO_RECORD, nL = 0; nL < MAXRUNSINRUNLIST; nL++)
                {
                  if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == RUNLISTDATA[nL].runNumber)
                  {
                    cutAsRuntype = RUNLISTDATA[nL].cutAsRuntype;
                    bFrozen = RUNLISTDATA[nL].frozenFlag;
                    break;
                  }
                }
                RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
                if(COST.TOTAL.payTime == 0)
                  dc.SelectObject(m_Pens[ILLEGAL]);
                else
                {
                  if(bFrozen)
                    dc.SelectObject(m_Pens[FROZEN]);
                  else
                    dc.SelectObject(m_Pens[CUT]);
                }
                break;
              }
            }
          }
          previousRunNumber = m_pRELIEFPOINTS[nJ - 1].start.runNumber;
        }
      }
      dc.Rectangle(m_pRELIEFPOINTS[nJ - 1].plot.x, m_pRELIEFPOINTS[nJ - 1].plot.y - 1,
            m_pRELIEFPOINTS[nJ].plot.x, m_pRELIEFPOINTS[nJ].plot.y + 1);
    }
//
//  If it's selected, draw a box around it
//
    if(m_pBP[dl].flags & BP_SELECTED)
      HighlightBlock(&dc, WTXT, m_pBP[dl].startIndex, m_pBP[dl].endIndex);
    else
      HighlightBlock(&dc, AXIS, m_pBP[dl].startIndex, m_pBP[dl].endIndex);
//
//  Do a second pass through so the hash marks aren't overwritten
//
    nJ = m_pBP[dl].startIndex;

    startPiece.x = m_pRELIEFPOINTS[nJ].plot.x;
    startPiece.y = m_pRELIEFPOINTS[nJ].plot.y;
    fromTime = m_pRELIEFPOINTS[nJ].time;

    for(nJ = m_pBP[dl].startIndex + 1; nJ <= m_pBP[dl].endIndex; nJ++)
    {
//
//  If this is a cut point, then display the node and its time along with a hash mark
//
      if(m_pRELIEFPOINTS[nJ].end.runNumber != m_pRELIEFPOINTS[nJ].start.runNumber &&
            nJ != m_pBP[dl].endIndex)
      {
//
//  Hash mark
//
        dc.SelectObject(m_Pens[HASH]);
        dc.Rectangle(m_pRELIEFPOINTS[nJ].plot.x, m_pRELIEFPOINTS[nJ].plot.y - 4,  
              m_pRELIEFPOINTS[nJ].plot.x + 2, m_pRELIEFPOINTS[nJ].plot.y + 4);
//
//  Node and time
//
        strcpy(tempString, NodeAbbrFromRecID(m_pRELIEFPOINTS[nJ].NODESrecordID));
//
//  The node name goes above the line
//
        int xAdj = (int)(tm.tmAveCharWidth * (NODES_ABBRNAME_LENGTH * 0.375));

        dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
              m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
//
//  The time goes below the line
//
        strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
        if(tempString[0] == ' ')
          memcpy(tempString, &tempString[1], strlen(tempString));
        dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
              m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));
      }
//
//  Not a cut point - display a narrow hash mark to indicate a relief point
//  (assuming that m_bDisplayReliefPointHashMarks has been set via the menu)
//
      else
      {
        if(m_bDisplayReliefPointHashMarks)
        {
          if(nJ == m_pBP[dl].endIndex)
            break;
          dc.SelectObject(m_Pens[HASH]);
          dc.Rectangle(m_pRELIEFPOINTS[nJ].plot.x, m_pRELIEFPOINTS[nJ].plot.y - 1,  
                m_pRELIEFPOINTS[nJ].plot.x + 2, m_pRELIEFPOINTS[nJ].plot.y + 1);
          strcpy(tempString, NodeAbbrFromRecID(m_pRELIEFPOINTS[nJ].NODESrecordID));
          int xAdj = (int)(tm.tmAveCharWidth * (NODES_ABBRNAME_LENGTH * 0.375));
          dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
                m_pRELIEFPOINTS[nJ].plot.y - NODEOFFSET, tempString, strlen(tempString));
          strcpy(tempString, Tchar(m_pRELIEFPOINTS[nJ].time));
          if(tempString[0] == ' ')
            memcpy(tempString, &tempString[1], strlen(tempString));
          dc.TextOut(m_pRELIEFPOINTS[nJ].plot.x - xAdj,
                m_pRELIEFPOINTS[nJ].plot.y + TIMEOFFSET, tempString, strlen(tempString));
        }
      }
//
//  Put out either the time in hh:mm in the middle of uncut pieces,
//  or the time and run number in the middle of cut pieces
//
//  Same if as before, just include the pullin this time
//
      if(m_pRELIEFPOINTS[nJ].end.runNumber != m_pRELIEFPOINTS[nJ].start.runNumber ||
            nJ == m_pBP[dl].endIndex)
      {
        totalTime = m_pRELIEFPOINTS[nJ].time - fromTime;
        midPiece.x = (startPiece.x + m_pRELIEFPOINTS[nJ].plot.x) / 2;
        midPiece.y = startPiece.y;
        strcpy(tempString, chhmm(totalTime));
//
//  Length of piece
//
        CSize size;
        int   cx;

        dc.SetTextColor(RGB(0, 128, 0));
        size = dc.GetTextExtent(tempString, strlen(tempString));
        cx = size.cx;
        dc.TextOut(midPiece.x - (cx / 2), midPiece.y + 2, tempString, strlen(tempString));
//
//  Run number in cut piece
//
        if(m_pRELIEFPOINTS[nJ].end.runNumber != NO_RECORD)
        {
          if(m_pRELIEFPOINTS[nJ].end.runNumber == SECURELOCATION_RUNNUMBER)
            strcpy(tempString, "SL");
          else
            sprintf(tempString, "%ld", m_pRELIEFPOINTS[nJ].end.runNumber);
          size = dc.GetTextExtent(tempString, strlen(tempString));
          cx = size.cx;
          dc.TextOut(midPiece.x - (cx / 2), midPiece.y - 14, tempString, strlen(tempString));
        }
        dc.SetTextColor(RGB(0, 0, 0));
//
//  Now set the start of this (next) piece
//
        startPiece.x = m_pRELIEFPOINTS[nJ].plot.x;
        startPiece.y = m_pRELIEFPOINTS[nJ].plot.y;
        fromTime = m_pRELIEFPOINTS[nJ].time;

      }
    }
  }
//
//  Reset the clipping region
//
  dc.SelectClipRgn(NULL);
  m_bCheckUnderConsideration = FALSE;
}

//
//  HighlightBlock()
//

void CVisualRuncutter::HighlightBlock(CPaintDC *pDC, int color, int start, int end)
{
  POINT top;
  POINT bottom;

  top.x = m_pRELIEFPOINTS[start].plot.x - 32;
  top.y = m_pRELIEFPOINTS[start].plot.y - 23;
  bottom.x = m_pRELIEFPOINTS[end].plot.x + 32;
  bottom.y = m_pRELIEFPOINTS[end].plot.y + 21;
  pDC->SelectObject(m_Pens[color]);
  pDC->MoveTo(top);
  pDC->LineTo(bottom.x, top.y);
  pDC->LineTo(bottom);
  pDC->LineTo(top.x, bottom.y);
  pDC->LineTo(top);
}

//
//  Event Sink Map for the scrollbar
//

BEGIN_EVENTSINK_MAP(CVisualRuncutter, CDialog)
    //{{AFX_EVENTSINK_MAP(CVisualRuncutter)
	ON_EVENT(CVisualRuncutter, VR_SCROLLBAR, 1 /* Change */, OnChangeScrollbar, VTS_NONE)
	ON_EVENT(CVisualRuncutter, VR_SCROLLBAR, 2 /* Scroll */, OnScrollScrollbar, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//
//  OnChangeScrollbar()
//

void CVisualRuncutter::OnChangeScrollbar() 
{
  m_firstDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_rcRepaint, TRUE);
}

//
//  On ScrollScrollbar()
//

void CVisualRuncutter::OnScrollScrollbar() 
{
  m_firstDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_rcRepaint, TRUE);
}

//
//  PreTranslateMessage - Capture <Home>, <End>, <PgUp>, and <PgDn>
//

BOOL CVisualRuncutter::PreTranslateMessage(MSG* pMsg) 
{
  if(pMsg->message == WM_KEYUP)
  {
    int numDisplayable = m_numInDisplayList;

    switch LOWORD(pMsg->wParam)
    {
      case VK_HOME:
        m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_END:
        m_firstDisplayed = numDisplayable - (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed < 0)
          m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_PRIOR:
        m_firstDisplayed -= (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed < 0)
          m_firstDisplayed = 0;
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
      case VK_NEXT:
        m_firstDisplayed += (BLOCKSDISPLAYED - 1);
        if(m_firstDisplayed > numDisplayable - (BLOCKSDISPLAYED - 1))
          m_firstDisplayed = numDisplayable - (BLOCKSDISPLAYED - 1);
        pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
        InvalidateRect(&m_rcRepaint, TRUE);
        break;
    }
  }
	return CDialog::PreTranslateMessage(pMsg);
}

//
//  OnLButtonDblClick()
//

void CVisualRuncutter::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
//  point comes in in dialog coords.  This should come as 
//  no surprize, as the right-click for the context menu
//  comes in as screen coords.
//
//  If it's outside the drawing region, don't bother
//
  if(point.x >= m_rc.left && point.x <= m_rc.right &&
        point.y >= m_rc.top && point.y <= m_rc.bottom)
  {
    BOOL bFound;
    int  index;
    int  nI;
    int  dl;
//
//  Look for the position
//
    for(bFound = FALSE, nI = m_firstDisplayed; nI < m_numInDisplayList; nI++)
    {
      dl = m_DisplayList.GetAt(nI);
      if(m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y == NO_RECORD)  // End of paintable area
        break;
      if(abs(point.y - m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y) <= 24)
      {
        index = dl;
        bFound = TRUE;
        break;
      }
    }
//
//  Found it.  Check on nFlags...
//
    if(bFound)
    {
      for(bFound = FALSE, nI = 0; nI < m_BlockPropertiesNumbers.GetSize(); nI++)
      {
        if(m_pBP[index].blockNumber == (int)m_BlockPropertiesNumbers.GetAt(nI))
        {
          bFound = TRUE;
          break;
        }
      }

      CBlockProperties *pBPDlg;

      if(bFound)
      {
        pBPDlg = (CBlockProperties *)m_BlockPropertiesDialogs.GetAt(nI);
        if(pBPDlg->GetSafeHwnd() == NULL)
          bFound = FALSE;
        else
          pBPDlg->BringWindowToTop();
      }
      if(!bFound)
      {
        pBPDlg = new CBlockProperties(this, m_pBP[index].blockNumber, &m_CutRunsDlg);
        pBPDlg->Create(IDD_BLOCKPROPERTIES, this);
        pBPDlg->ShowWindow(TRUE);
        m_BlockPropertiesDialogs.Add(pBPDlg);
        m_BlockPropertiesNumbers.Add(m_pBP[index].blockNumber);
      }
    }
  }

	CDialog::OnLButtonDblClk(nFlags, point);
}

//
//  OnContextMenu()
//

void CVisualRuncutter::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  if(m_menu != NULL)
    return;
//
//  point comes in in screen coords.  Translate it
//  to dialog coords, which is what it should come
//  in in the first place.  Jeez.
//
  RECT r;

  pWnd->GetWindowRect(&r);

  long dlgX = point.x - r.left;
  long dlgY = point.y - r.top - m_nCaptionSize;
//
//  If it's outside the drawing region, don't bother
//
  if(dlgX >= m_rc.left && dlgX <= m_rc.right &&
        dlgY >= m_rc.top && dlgY <= m_rc.bottom)
  {
    m_menu.LoadMenu(IDR_VRB);
    CMenu* pMenu = m_menu.GetSubMenu(0);
    if(pMenu)
    {
      CMenu* pMenu2 = pMenu->GetSubMenu(4);
      if(pMenu2)
      {
        if(m_bDisplayReliefPointHashMarks)
          pMenu2->CheckMenuItem(VRB_DISPLAY_RELIEFHASHMARKS, MF_BYCOMMAND | MF_CHECKED);
        else
          pMenu2->CheckMenuItem(VRB_DISPLAY_RELIEFHASHMARKS, MF_BYCOMMAND | MF_UNCHECKED);
      }
      pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, pWnd, NULL);
    }
    m_menu.DestroyMenu();
  }
}


//
//  OnDisplayrunswindow()
//

void CVisualRuncutter::OnDisplayrunswindow() 
{
  CString s;

  if(m_bDisplayingCutRuns)
  {
    SaveCutRunsRect();
    m_CutRunsDlg.DestroyWindow();
    m_bDisplayingCutRuns = FALSE;
    s.LoadString(TEXT_235);
  }
  else
  {
    m_CutRunsDlg.Create(IDD_CUTRUNS, this);
    m_bDisplayingCutRuns = TRUE;
    s.LoadString(TEXT_236);
    MoveCutRunsWindow();
  }	
//
//  Adjust menu item here
//
}

//
//  SaveCutRunsRect() - Write out the size of the Cut Runs window
//

void CVisualRuncutter::SaveCutRunsRect()
{
  m_CutRunsDlg.GetWindowRect(&m_CutRunsRect);
  sprintf(tempString, "%d %d %d %d", 
        m_CutRunsRect.top, m_CutRunsRect.left, m_CutRunsRect.bottom, m_CutRunsRect.right);
  WritePrivateProfileString(userName, "CutRunsPosition", tempString, szDatabaseFileName);
}

//
//  MoveCutRunsWindow()
//

void CVisualRuncutter::MoveCutRunsWindow()
{
//
//  Is there a size/position from before?
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"CutRunsPosition", "",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  if(strcmp(tempString, "") != 0)
  {
    strcpy(szarString, strtok(tempString, " "));
    m_CutRunsRect.top = atol(szarString);
    strcpy(szarString, strtok(NULL, " "));
    m_CutRunsRect.left = atol(szarString);
    strcpy(szarString, strtok(NULL, " "));
    m_CutRunsRect.bottom = atol(szarString);
    strcpy(szarString, strtok(NULL, "\0"));
    m_CutRunsRect.right = atol(szarString);
    m_CutRunsDlg.MoveWindow(&m_CutRunsRect, TRUE);
  }
//
//  Nope - Create one
//
  else
    SaveCutRunsRect();
}

//
//  SaveDialogRect() - Write out the position of the blocks window
//

void CVisualRuncutter::SaveDialogRect()
{
  RECT r;

  GetWindowRect(&r);
  sprintf(tempString, "%d %d %d %d", r.top, r.left, r.right - r.left, r.bottom - r.top);
  WritePrivateProfileString(userName, "VisualRuncutterPosition", tempString, szDatabaseFileName);
}

//
//  OnSize() - Resize the blocks window
//

void CVisualRuncutter::OnSize(UINT nType, int cx, int cy) 
{
  CResizingDialog::OnSize(nType, cx, cy);
	
//
//  Get the dimensions and repaint
//
  if(pButtonSAVE == NULL || !IsWindow(pButtonSAVE->m_hWnd))
    return;
  if(pFlatSBSCROLLBAR == NULL || !IsWindow(pFlatSBSCROLLBAR->m_hWnd))
    return;

  EstablishViewingArea();
  InvalidateRect(NULL, TRUE);  // Redraw the whole client area
	
}

//
//  OnCreate() - Force the window through the resizing class
//

int CVisualRuncutter::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CResizingDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

//
//  OnGetMinMaxInfo() - Subclass for the resizing class
//

void CVisualRuncutter::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	CResizingDialog::OnGetMinMaxInfo(lpMMI);
}

//
//  EstablishViewingArea() - Set out the extents for the blocks window
//

void CVisualRuncutter::EstablishViewingArea()
{
  RECT r;
  RECT tempRect;
  int  nI;
  int  width;
  int  height;

  if(pButtonSAVE == NULL || !IsWindow(pButtonSAVE->m_hWnd))
    return;
  if(pFlatSBSCROLLBAR == NULL || !IsWindow(pFlatSBSCROLLBAR->m_hWnd))
    return;

  GetWindowRect(&m_rcDialog);
  pButtonSAVE->GetWindowRect(&tempRect);

  r.left = m_rcDialog.right - tempRect.right;

  pFlatSBSCROLLBAR->GetWindowRect(&tempRect);
  r.top = tempRect.top - m_rcDialog.top - m_nCaptionSize - 3;

  width = m_rcDialog.right - m_rcDialog.left;
  height = m_rcDialog.bottom - m_rcDialog.top;

  r.right = width - (m_rcDialog.right - tempRect.left) - 3;
  r.bottom = height - (m_rcDialog.bottom - tempRect.bottom) - m_nCaptionSize - 3;

//
//  Set up the window extents
//
  width = r.right - r.left;
  height = r.bottom - r.top;

  m_rc = r;
  m_rcRepaint = m_rc;
  m_rcRepaint.top = 0;
  m_rcRepaint.left = 0;
  m_rcRepaint.bottom += 4;
//
//  Determine the earliest and latest block times
//
  m_earliestTime = 86400L;
  m_latestTime = NO_TIME;
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      if(m_pRELIEFPOINTS[m_pBP[nI].startIndex].time < m_earliestTime)
        m_earliestTime = m_pRELIEFPOINTS[m_pBP[nI].startIndex].time;
      if(m_pRELIEFPOINTS[m_pBP[nI].endIndex].time > m_latestTime)
        m_latestTime = m_pRELIEFPOINTS[m_pBP[nI].endIndex].time;
    }
  }
//
//  Round the earliest down to the previous hour
//  Round the latest up to the next hour
//
  width -= INDENT * 2;
  m_earliestTime -= 60;
  m_earliestTime -= m_earliestTime % 3600;
  m_latestTime += 60;
  m_latestTime += 3600 - (m_latestTime % 3600);
  m_timeInc = (long)(width / (float)((m_latestTime - m_earliestTime) / 3600));
}

//
//  OnParameters() - Respond to a Parameters button press
//

void CVisualRuncutter::OnParameters() 
{
  int  nRc;
  char szLabelsWere[sizeof(CUTPARMS.labels)];

  strcpy(szLabelsWere, CUTPARMS.labels);
  nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_PARAMETERS),
        this->m_hWnd, (DLGPROC)PARAMETERSMsgProc);
  if(nRc && strcmp(szLabelsWere, CUTPARMS.labels) != 0)
  {
    TMSError(this->m_hWnd, MB_ICONINFORMATION, ERROR_158, (HANDLE)NULL);
  }
}

//
//  OnRuntypes() - Respond to a Runtypes button press
//

void CVisualRuncutter::OnRuntypes() 
{
  if(DialogBox(hInst, MAKEINTRESOURCE(IDD_RUNTYPES), this->m_hWnd, (DLGPROC)RUNTYPESMsgProc))
  {
    m_CutRunsDlg.ShowRuns(&m_VRColors);
    InvalidateRect(&m_rcRepaint, TRUE);
  }
}

//
//  OnWorkrules() - Respond to a Workrules button press
//

void CVisualRuncutter::OnWorkrules() 
{
  CPremiumDefinition dlg(this, m_pDI);

  if(dlg.DoModal() == IDOK)
//  if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WORKRULES),
//        this->m_hWnd, (DLGPROC)WORKRULESMsgProc, (LPARAM)m_pDI))
  {
    m_CutRunsDlg.ShowRuns(&m_VRColors);
    InvalidateRect(&m_rcRepaint, TRUE);
  }
}

//
//  OnCutRecursive() - Menu selection
//

void CVisualRuncutter::OnCutRecursive() 
{
  CWaitCursor wait;

  StatusBarStart(NULL, "Recursive runcut in progress...");
  StatusBarText("Runs cut: 0");
  StatusBarEnableCancel(FALSE);
  StatusBar(-1L, -1L);

  m_bCheckUnderConsideration = TRUE;
  AutomaticRunCut(IDRECURSIVE);

  StatusBarEnableCancel(TRUE);
  StatusBarEnd();
  wait.Restore();

  m_CutRunsDlg.ShowRuns(&m_VRColors);

  InvalidateRect(&m_rcRepaint, TRUE);
  m_bCheckUnderConsideration = FALSE;
}

//
//  OnCutMincost() - Menu selection
//

void CVisualRuncutter::OnCutMincost() 
{
  HCURSOR hcSave = ::SetCursor(hCursorWait);

  StatusBarStart(NULL, "Min-cost matcher runcut in progress...");
  StatusBarEnableCancel(FALSE);

  m_bCheckUnderConsideration = TRUE;
  AutomaticRunCut(IDAUTOMATIC);
  m_bCheckUnderConsideration = FALSE;

  StatusBarEnableCancel(TRUE);
  StatusBarEnd();
  ::SetCursor(hcSave);

  m_CutRunsDlg.ShowRuns(&m_VRColors);

  InvalidateRect(&m_rcRepaint, TRUE);
}

//
//  OnCutCrewcut() - Menu selection
//

void CVisualRuncutter::OnCutCrewcut() 
{
  StatusBarStart(NULL, "CrewCut in progress...");
  StatusBarText("Performing first pass");
  StatusBar(-1L, -1L);
  CutCrew();
  StatusBarEnd();
  m_CutRunsDlg.ShowRuns(&m_VRColors);
  InvalidateRect(&m_rcRepaint, TRUE);
}

//
//  OnProperties() - Menu selection
//

void CVisualRuncutter::OnProperties() 
{
  BOOL bFound;
  int  nI, nJ;
  
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_SELECTED)
    {
      for(bFound = FALSE, nJ = 0; nJ < m_BlockPropertiesNumbers.GetSize(); nJ++)
      {
        if(m_pBP[nI].blockNumber == (int)m_BlockPropertiesNumbers.GetAt(nJ))
        {
          bFound = TRUE;
          break;
        }
      }

      CBlockProperties *pBPDlg;

      if(bFound)
      {
        pBPDlg = (CBlockProperties *)m_BlockPropertiesDialogs.GetAt(nJ);
        if(pBPDlg->GetSafeHwnd() == NULL)
          bFound = FALSE;
        else
          pBPDlg->BringWindowToTop();
      }
      if(!bFound)
      {
        pBPDlg = new CBlockProperties(this, m_pBP[nI].blockNumber, &m_CutRunsDlg);
        pBPDlg->Create(IDD_BLOCKPROPERTIES, this);
        pBPDlg->ShowWindow(TRUE);
        m_BlockPropertiesDialogs.Add(pBPDlg);
        m_BlockPropertiesNumbers.Add(m_pBP[nI].blockNumber);
      }
    }
  }

}

//
//  OnSelectall() - Menu selection
//

void CVisualRuncutter::OnSelectall() 
{
  int nI;

  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
      m_pBP[nI].flags |= BP_SELECTED;
  }
  InvalidateRect(&m_rcRepaint, FALSE);
}

//
//  OnDeselectall() - Menu selection
//

void CVisualRuncutter::OnDeselectall() 
{
  int nI;

  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if((m_pBP[nI].flags & BP_DISPLAYED) && (m_pBP[nI].flags & BP_SELECTED))
    {
      m_pBP[nI].flags &= ~BP_SELECTED;
    }
  }
  InvalidateRect(&m_rcRepaint, FALSE);
}

//
//  OnColors() - Respond to a Colors button press
//

void CVisualRuncutter::OnColors() 
{
  VRCOLORSDef tempVRC = m_VRColors;

  CVRColors dlg(this, &m_VRColors);

  if(dlg.DoModal() == IDOK)
  {
    BOOL bDidOne = FALSE;

    if(tempVRC.cut != m_VRColors.cut)
    {
      m_Pens[CUT].Detach();
      m_Pens[CUT].DeleteObject();
      m_Pens[CUT].CreatePen(PS_SOLID, THICK, m_VRColors.cut);
      ltoa(m_VRColors.cut, tempString, 10);
      WritePrivateProfileString(userName, "VRColorCut", tempString, TMSINIFile);
      bDidOne = TRUE;
    }
    if(tempVRC.illegal != m_VRColors.illegal)
    {
      m_Pens[ILLEGAL].Detach();
      m_Pens[ILLEGAL].DeleteObject();
      m_Pens[ILLEGAL].CreatePen(PS_SOLID, THICK, m_VRColors.illegal);
      ltoa(m_VRColors.illegal, tempString, 10);
      WritePrivateProfileString(userName, "VRColorIllegal", tempString, TMSINIFile);
      bDidOne = TRUE;
    }
    if(tempVRC.frozen != m_VRColors.frozen)
    {
      m_Pens[FROZEN].Detach();
      m_Pens[FROZEN].DeleteObject();
      m_Pens[FROZEN].CreatePen(PS_SOLID, THICK, m_VRColors.frozen);
      ltoa(m_VRColors.frozen, tempString, 10);
      WritePrivateProfileString(userName, "VRColorFrozen", tempString, TMSINIFile);
      bDidOne = TRUE;
    }
    if(tempVRC.uncut != m_VRColors.uncut)
    {
      m_Pens[UNCUT].Detach();
      m_Pens[UNCUT].DeleteObject();
      m_Pens[UNCUT].CreatePen(PS_SOLID, THICK, m_VRColors.uncut);
      ltoa(m_VRColors.uncut, tempString, 10);
      WritePrivateProfileString(userName, "VRColorUncut", tempString, TMSINIFile);
      bDidOne = TRUE;
    }
//
//  Any changes?
//
    if(bDidOne)
    {
      int nI;
//
//  Do the runs
//
      m_CutRunsDlg.ShowRuns(&m_VRColors);
      InvalidateRect(&m_rcRepaint, TRUE);
//
//  And any open block properties windows
//
      CBlockProperties *pBPDlg;
      for(nI = 0; nI < m_BlockPropertiesDialogs.GetSize(); nI++)
      {
        pBPDlg = (CBlockProperties *)m_BlockPropertiesDialogs.GetAt(nI);
        if(pBPDlg->GetSafeHwnd() != NULL)
        {
          pBPDlg->ShowReliefPoints();
        }
      }
    }
  }
}

//
//  OnLButtonDown() - Intercept the mouse click to save the current position
//

void CVisualRuncutter::OnLButtonDown(UINT nFlags, CPoint point) 
{
//  point comes in in dialog coords.  This should come as 
//  no surprize, as the right-click for the context menu
//  comes in as screen coords.
//
//  If it's outside the drawing region, don't bother
//
  if(point.x >= m_rc.left && point.x <= m_rc.right &&
        point.y >= m_rc.top && point.y <= m_rc.bottom)
  {
    m_LButtonPoint = point;
  }
  else
  {
    m_LButtonPoint.x = NO_RECORD;
    m_LButtonPoint.y = NO_RECORD;
  }
	
	CDialog::OnLButtonDown(nFlags, point);
}

//
//  OnLButtonUp() - Determine selections
//

void CVisualRuncutter::OnLButtonUp(UINT nFlags, CPoint point) 
{
  int nI;
  int index;
  int dl;

//
//  Were we in the region?
//
  if(m_LButtonPoint.x != NO_RECORD)
	{
//
//  Check to see that the mouse hasn't strayed too far
//
    if(abs(m_LButtonPoint.x - point.x) <= 4 &&
          abs(m_LButtonPoint.y - point.y) <= 4)
    {
      BOOL bFound;
//
//  Look for the position
//
      for(bFound = FALSE, nI = m_firstDisplayed; nI < m_numInDisplayList; nI++)
      {
        dl = m_DisplayList.GetAt(nI);
        if(m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y == NO_RECORD)  // End of paintable area
          break;
        if(abs(point.y - m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y) <= 24)
        {
          index = dl;
          bFound = TRUE;
          break;
        }
      }
//
//  Found it.  Check on nFlags...
//
      if(bFound)
      {
//
//  Left-click only - Deselect anything that was selected and make this selection only
//
        if(!(nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT))
        {
          for(nI = 0; nI < numBPBlocks; nI++)
          {
            if(!(m_pBP[nI].flags & BP_DISPLAYED))
              continue;
            if(m_pBP[nI].flags & BP_SELECTED)
            {
              m_pBP[nI].flags &= ~BP_SELECTED;
            }
          }
        }
//
//  <Ctrl> Left-click - Select this entry and leave the rest intact (fall-through)
//
        else if(nFlags & MK_CONTROL)
        {
        }
//
//  <Shift> Left-click - Take everything from the previous selection to here
//
        else if(nFlags & MK_SHIFT)
        {
          if(m_LastIndex != NO_RECORD)
          {
            if(m_LastIndex < index)
            {
              for(nI = m_LastIndex; nI < index; nI++)
              {
                if(!(m_pBP[nI].flags & BP_DISPLAYED))
                  continue;
                m_pBP[nI].flags |= BP_SELECTED;
              }
            }
            else if(m_LastIndex > index)
            {
              for(nI = index; nI < m_LastIndex; nI++)
              {
                if(!(m_pBP[nI].flags & BP_DISPLAYED))
                  continue;
                m_pBP[nI].flags |= BP_SELECTED;
              }
            }
          }
        }
//
//  Repaint
//    
        m_pBP[index].flags |= BP_SELECTED;
        m_LastIndex = index;
        InvalidateRect(&m_rcRepaint, FALSE);
      }
    }
  }

	CDialog::OnLButtonUp(nFlags, point);
}

//
//  OnRButtonDown() - Intercept the mouse click to save the current position
//

void CVisualRuncutter::OnRButtonDown(UINT nFlags, CPoint point) 
{
//  point comes in in dialog coords.  This should come as 
//  no surprize, as the right-click for the context menu
//  comes in as screen coords.
//
//  If it's outside the drawing region, don't bother
//
  if(point.x >= m_rc.left && point.x <= m_rc.right &&
        point.y >= m_rc.top && point.y <= m_rc.bottom)
  {
    m_RButtonPoint = point;
  }
  else
  {
    m_RButtonPoint.x = NO_RECORD;
    m_RButtonPoint.y = NO_RECORD;
  }
	
	CDialog::OnRButtonDown(nFlags, point);
}

//
//  OnRButtonUp() - Highlight a selection prior to displaying the context menu
//

void CVisualRuncutter::OnRButtonUp(UINT nFlags, CPoint point) 
{
  int nI;
  int index;
  int dl;

//
//  Were we in the region?
//
  if(m_RButtonPoint.x != NO_RECORD)
	{
//
//  Check to see that the mouse hasn't strayed too far
//
    if(abs(m_RButtonPoint.x - point.x) <= 4 &&
          abs(m_RButtonPoint.y - point.y) <= 4)
    {
      BOOL bFound;
//
//  Look for the position
//
      for(bFound = FALSE, nI = m_firstDisplayed; nI < m_numInDisplayList; nI++)
      {
        dl = m_DisplayList.GetAt(nI);
        if(m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y == NO_RECORD)  // End of paintable area
          break;
        if(abs(point.y - m_pRELIEFPOINTS[m_pBP[dl].startIndex].plot.y) <= 24)
        {
          index = dl;
          bFound = TRUE;
          break;
        }
      }
//
//  Found it - if it wasn't selected, clear everything, select it and repaint
//
      if(bFound && !(m_pBP[index].flags & BP_SELECTED))
      {
        for(nI = 0; nI < numBPBlocks; nI++)
        {
          if(m_pBP[nI].flags & BP_SELECTED)
          {
            m_pBP[nI].flags &= ~BP_SELECTED;
          }
        }
        m_pBP[index].flags |= BP_SELECTED;
        InvalidateRect(&m_rcRepaint, FALSE);
      }
    }
  }
	
	CDialog::OnRButtonUp(nFlags, point);
}

//
//  OnDisplayAllblocks() - Display all the blocks in the graph
//

void CVisualRuncutter::OnDisplayAllblocks() 
{
  int nI;

  for(nI = 0; nI < numBPBlocks; nI++)
  {
    m_pBP[nI].flags = BP_DISPLAYED;
  }
  m_CurrentDisplayFlag = VR_FLAG_ALLBLOCKS;
  pFlatSBSCROLLBAR->SetMax(numBPBlocks);
  m_firstDisplayed = 0;
  pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
  InvalidateRect(&m_rcRepaint, TRUE);
  UpdateDisplayStatus();
}

//
//  OnDisplayLeftovers() - Display only those blocks with remaining work
//

void CVisualRuncutter::OnDisplayLeftovers() 
{
  int nI, nJ;
  int numDisplayed = 0;
  int dl;

  for(nI = 0; nI < m_numInDisplayList; nI++)
  {
    dl = m_DisplayList.GetAt(nI);
    m_pBP[dl].flags = 0;
    for(nJ = m_pBP[dl].startIndex; nJ < m_pBP[dl].endIndex; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ].start.runNumber == NO_RECORD)
      {
        m_pBP[dl].flags = BP_DISPLAYED;
        numDisplayed++;
        break;
      }
    }
  }
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  m_firstDisplayed = 0;
  m_CurrentDisplayFlag = VR_FLAG_SUBSET;
  pFlatSBSCROLLBAR->SetMax(numDisplayed);
  pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
  InvalidateRect(&m_rcRepaint, TRUE);
  UpdateDisplayStatus();
}

void CVisualRuncutter::OnSortBlocknumber() 
{
  int nI;

  qsort((void *)m_pBP, numBPBlocks, sizeof(BLOCKPLOTDef), SortByBlockNumber);
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  m_CurrentSortFlag = VR_FLAG_SORTEDBYBLOCKNUMBER;
  if(m_CurrentDisplayFlag & VR_FLAG_ALLBLOCKS)
    OnDisplayAllblocks();
  else if(m_CurrentDisplayFlag & VR_FLAG_ILLEGAL)
    OnDisplayIllegal();
  else
    OnDisplayLeftovers();
}

void CVisualRuncutter::OnSortOfftime() 
{
  int nI;

  qsort((void *)m_pBP, numBPBlocks, sizeof(BLOCKPLOTDef), SortByEndTime);
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  m_CurrentSortFlag = VR_FLAG_SORTEDBYOFFTIME;
  if(m_CurrentDisplayFlag & VR_FLAG_ALLBLOCKS)
    OnDisplayAllblocks();
  else if(m_CurrentDisplayFlag & VR_FLAG_ILLEGAL)
    OnDisplayIllegal();
  else
    OnDisplayLeftovers();
}

void CVisualRuncutter::OnSortOntime() 
{
  int nI;

  qsort((void *)m_pBP, numBPBlocks, sizeof(BLOCKPLOTDef), SortByStartTime);
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  m_CurrentSortFlag = VR_FLAG_SORTEDBYONTIME;
  if(m_CurrentDisplayFlag & VR_FLAG_ALLBLOCKS)
    OnDisplayAllblocks();
  else if(m_CurrentDisplayFlag & VR_FLAG_ILLEGAL)
    OnDisplayIllegal();
  else
    OnDisplayLeftovers();
}

void CVisualRuncutter::OnSortBlocklength() 
{
  int nI;

  qsort((void *)m_pBP, numBPBlocks, sizeof(BLOCKPLOTDef), SortByBlockLength);
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  m_CurrentSortFlag = VR_FLAG_SORTEDBYBLOCKLENGTH;
  if(m_CurrentDisplayFlag & VR_FLAG_ALLBLOCKS)
    OnDisplayAllblocks();
  else if(m_CurrentDisplayFlag & VR_FLAG_ILLEGAL)
    OnDisplayIllegal();
  else
    OnDisplayLeftovers();
}

//
//  UpdateDisplayStatus() - Write out the current display options
//

void CVisualRuncutter::UpdateDisplayStatus()
{
  CString sSortType;
  CString sDisplayType;
//
//  Display type
//
  if(m_CurrentDisplayFlag & VR_FLAG_ALLBLOCKS)
    sDisplayType = "all blocks";
  else if(m_CurrentDisplayFlag & VR_FLAG_SUBSET)
    sDisplayType = "remaining work";
  else
    sDisplayType = "blocks containing illegal runs";
//
//  Sort value
//
  if(m_CurrentSortFlag & VR_FLAG_SORTEDBYBLOCKNUMBER)
    sSortType = "block number";
  else if(m_CurrentSortFlag & VR_FLAG_SORTEDBYBLOCKLENGTH)
    sSortType = "block length";
  else if(m_CurrentSortFlag & VR_FLAG_SORTEDBYONTIME)
    sSortType = "on time";
  else
    sSortType = "off time";
//
//  Show it
//
  sprintf(tempString, "Current view: Displaying %s, sorted by %s", sDisplayType, sSortType);
  pStaticDISPLAYSTATUS->SetWindowText(tempString);
}

//
//  OnJumptoRun() - Scroll the list of runs to show a particular run number
//

void CVisualRuncutter::OnJumptorun() 
{
  CUIntArray runNumbers;
  long runNumber;
  BOOL bFound;
  int  nI, nJ, nK;

  runNumbers.RemoveAll();
//
//  Build a list of unique run numbers from what's highlighted
//  
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_SELECTED)
    {
      for(nJ = m_pBP[nI].startIndex; nJ <= m_pBP[nI].endIndex; nJ++)
      {
        if(nJ == m_pBP[nI].endIndex)
          runNumber = m_pRELIEFPOINTS[nJ].start.runNumber;
        else
          runNumber = m_pRELIEFPOINTS[nJ].end.runNumber;
        if(runNumber == NO_RECORD)
          continue;
        for(bFound = FALSE, nK = 0; nK < runNumbers.GetSize(); nK++)
        {
          if(runNumber == (long)runNumbers.GetAt(nK))
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
          runNumbers.Add(runNumber);
      }
    }
  }
//
//  Were there any?
//
  if(runNumbers.GetSize() == 0)
    return;
//
//  Yes.  Display them and return the run number in question,
//  unless there was just one, in which case jump right to it.
//
  if(runNumbers.GetSize() == 1)
    runNumber = runNumbers.GetAt(0);
  else
  {
    CJumpToRun dlg(this, &runNumbers, &runNumber);

    dlg.DoModal();
  }
//
//  Do the jump
//
  if(runNumber != NO_RECORD)
  {
    m_CutRunsDlg.SetRunAtTop(runNumber);
  }
}

//
//  OnCutWhatsLeft() - Cut out remaining pieces from highlighted blocks
//

void CVisualRuncutter::OnCutWhatsleft() 
{
  BOOL bGotStart;
  BOOL bCutOne = FALSE;
  int  startIndex;
  int  endIndex;
  int  nI, nJ, nK;
//
//  Find uncut pieces of work in what's highlighted
//  
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_SELECTED)
    {
      for(bGotStart = FALSE, nJ = m_pBP[nI].startIndex; nJ <= m_pBP[nI].endIndex; nJ++)
      {
//
//  Look for a starting point
//
        if(!bGotStart)
        {
          if(m_pRELIEFPOINTS[nJ].start.runNumber == NO_RECORD)
          {
            bGotStart = TRUE;
            startIndex = nJ;
          }
        }
//
//  We've got a starting point - look for the end
//
        else
        {
          if(m_pRELIEFPOINTS[nJ].start.runNumber == NO_RECORD &&
                nJ != m_pBP[nI].endIndex)
            continue;
//
//  Got the end - cut the piece
//
          endIndex = nJ;
          for(nK = startIndex; nK <= endIndex; nK++)
          {
            if(nK != endIndex)
            {
              m_pRELIEFPOINTS[nK].start.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nK].start.runNumber = m_GlobalRunNumber;
              m_pRELIEFPOINTS[nK].start.pieceNumber = 1;
            }
            if(nK != startIndex)
            {
              m_pRELIEFPOINTS[nK].end.recordID = NO_RECORD;
              m_pRELIEFPOINTS[nK].end.runNumber = m_GlobalRunNumber;
              m_pRELIEFPOINTS[nK].end.pieceNumber = 1;
            }
          }
          bCutOne = TRUE;
          m_GlobalRunNumber++;
          bGotStart = FALSE;
        }
      }  // nJ
    }
  }  // nI
//
//  If at least one run was cut, update everything
//
  if(bCutOne)
  {
    AssembleRuns();
    m_CutRunsDlg.ShowRuns(&m_VRColors);
    if(m_CurrentDisplayFlag & VR_FLAG_SUBSET)
      OnDisplayLeftovers();
    else if(m_CurrentDisplayFlag & VR_FLAG_ILLEGAL)
      OnDisplayIllegal();
    else
      OnDisplayAllblocks();
  }
	
}

//
//  OnDisplayIllegal() - Set the graph to just show those blocks containing illegal runs
//

void CVisualRuncutter::OnDisplayIllegal() 
{
  long previousRunNumber;
  BOOL bGotIllegal;
  int  nI, nJ, nK, nL;
  int  dl;
//
//  Cycle through and reset the display
//
  HCURSOR hSaveCursor = ::SetCursor(hCursorWait);
  for(nI = 0; nI < m_numInDisplayList; nI++)
  {
    dl = m_DisplayList.GetAt(nI);
    m_pBP[dl].flags &= ~BP_DISPLAYED;
    previousRunNumber = NO_RECORD;
    bGotIllegal = FALSE;
    for(nJ = m_pBP[dl].startIndex + 1; nJ <= m_pBP[dl].endIndex; nJ++)
    {
      if(bGotIllegal)
        break;
      if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == NO_RECORD)
        continue;
      if(m_pRELIEFPOINTS[nJ - 1].start.runNumber != previousRunNumber)
      {
        PROPOSEDRUNDef PROPOSEDRUN;
        COSTDef COST;
        long cutAsRuntype;

        for(nK = 0; nK < MAXRUNSINRUNLIST; nK++)
        {
          if(RUNLIST[nK].runNumber == m_pRELIEFPOINTS[nJ - 1].start.runNumber)
          {
            memcpy(&PROPOSEDRUN, &RUNLIST[nK].run, sizeof(PROPOSEDRUN));
            for(cutAsRuntype = NO_RECORD, nL = 0; nL < MAXRUNSINRUNLIST; nL++)
            {
              if(m_pRELIEFPOINTS[nJ - 1].start.runNumber == RUNLISTDATA[nL].runNumber)
              {
                cutAsRuntype = RUNLISTDATA[nL].cutAsRuntype;
                break;
              }
            }
            RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
            if(COST.TOTAL.payTime == 0)
            {
              m_pBP[dl].flags |= BP_DISPLAYED;
              bGotIllegal = TRUE;
            }
            break;
          }
        }
        previousRunNumber = m_pRELIEFPOINTS[nJ - 1].start.runNumber;
      }
    }
  }
  m_numInDisplayList = 0;
  m_DisplayList.RemoveAll();
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_DISPLAYED)
    {
      m_DisplayList.Add(nI);
      m_numInDisplayList++;
    }
  }
  ::SetCursor(hSaveCursor);
  m_firstDisplayed = 0;
  m_CurrentDisplayFlag = VR_FLAG_ILLEGAL;
  pFlatSBSCROLLBAR->SetMax(m_numInDisplayList);
  pFlatSBSCROLLBAR->SetValue((short int)m_firstDisplayed);
  InvalidateRect(&m_rcRepaint, TRUE);
  UpdateDisplayStatus();
}

void CVisualRuncutter::OnRunslider() 
{
/*
  BOOL bFound;
  int  nI, nJ;
  
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_SELECTED)
    {
      for(bFound = FALSE, nJ = 0; nJ < m_RunSliderBlockNumbers.GetSize(); nJ++)
      {
        if(m_pBP[nI].blockNumber == (int)m_RunSliderBlockNumbers.GetAt(nJ))
        {
          bFound = TRUE;
          break;
        }
      }

      CRunSlider *pRSDlg;

      if(bFound)
      {
        pRSDlg = (CRunSlider *)m_RunSliderDialogs.GetAt(nJ);
        if(pRSDlg->GetSafeHwnd() == NULL)
          bFound = FALSE;
        else
          pRSDlg->BringWindowToTop();
      }
      if(!bFound)
      {
        pRSDlg = new CRunSlider(this, m_pBP[nI].blockNumber);
        pRSDlg->Create(IDLG_RUNSLIDER, this);
        pRSDlg->ShowWindow(TRUE);
        m_RunSliderDialogs.Add(pRSDlg);
        m_RunSliderBlockNumbers.Add(m_pBP[nI].blockNumber);
      }
    }
  }
*/	
}

void CVisualRuncutter::AbortInit() 
{
  Cleanup(FALSE);
	CDialog::OnCancel();
}

void CVisualRuncutter::OnDisplayReliefhashmarks() 
{
  m_bDisplayReliefPointHashMarks = !m_bDisplayReliefPointHashMarks;
  InvalidateRect(&m_rcRepaint, TRUE);
}

void CVisualRuncutter::OnRecursive() 
{
	OnCutRecursive();
}

void CVisualRuncutter::OnMincost() 
{
  OnCutMincost();
}

void CVisualRuncutter::OnUndoruns() 
{
  CUIntArray runNumbers;
  long runNumber;
  BOOL bFound;
  int  nI, nJ, nK;

  runNumbers.RemoveAll();
//
//  Build a list of unique run numbers from what's highlighted
//  
  for(nI = 0; nI < numBPBlocks; nI++)
  {
    if(m_pBP[nI].flags & BP_SELECTED)
    {
      for(nJ = m_pBP[nI].startIndex; nJ <= m_pBP[nI].endIndex; nJ++)
      {
        if(nJ == m_pBP[nI].endIndex)
        {
          runNumber = m_pRELIEFPOINTS[nJ].start.runNumber;
        }
        else
        {
          runNumber = m_pRELIEFPOINTS[nJ].end.runNumber;
        }
        if(runNumber == NO_RECORD)
        {
          continue;
        }
        for(bFound = FALSE, nK = 0; nK < runNumbers.GetSize(); nK++)
        {
          if(runNumber == (long)runNumbers.GetAt(nK))
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
        {
          runNumbers.Add(runNumber);
        }
      }
    }
  }
//
//  Were there any?
//
  if(runNumbers.GetSize() == 0)
  {
    return;
  }
//
//  Yes
//
  CUndoRuns dlg(this, &runNumbers);

  if(dlg.DoModal())
  {
    long runNumber;
    BOOL bFound;
    int  nI, nJ;
//
//  Loop through the runs
//
    for(nI = 0; nI < runNumbers.GetSize(); nI++)
    {
      runNumber = runNumbers.GetAt(nI);
//
//  If it's frozen, skip it
//
      for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
      {
        if(RUNLISTDATA[nJ].runNumber == runNumber)
        {
          bFound = RUNLISTDATA[nJ].frozenFlag;
          break;
        }
      }
      if(bFound)
      {
        continue;
      }
//
//  Not frozen - continue.
//
//  Zero out the run's data in the RELIEFPOINTS structure
//
      for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
      {
        if(m_pRELIEFPOINTS[nJ].start.runNumber == runNumber)
        {
          m_pRELIEFPOINTS[nJ].start.runNumber = NO_RECORD;
          m_pRELIEFPOINTS[nJ].start.pieceNumber = NO_RECORD;
          m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
        }
        if(m_pRELIEFPOINTS[nJ].end.runNumber == runNumber)
        {
          m_pRELIEFPOINTS[nJ].end.runNumber = NO_RECORD;
          m_pRELIEFPOINTS[nJ].end.pieceNumber = NO_RECORD;
          m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
        }
      }
//
//  And delete it fron RUNLISTDATA
//
      for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
      {
        if(RUNLISTDATA[nJ].runNumber == runNumber)
        {
          RUNLISTDATA[nJ].runNumber = NO_RECORD;
          RUNLISTDATA[nJ].cutAsRuntype = NO_RUNTYPE;
          RUNLISTDATA[nJ].COMMENTSrecordID = NO_RECORD;
          RUNLISTDATA[nJ].frozenFlag = FALSE;
          break;
        }
      }
    }
//
//  And update the displays
//
    if(runNumbers.GetSize() > 0)  // this means we did something
    {
      AssembleRuns();  // Bring the internal lists up to date
      m_CutRunsDlg.ShowRuns(&m_VRColors);

      InvalidateRect(&m_rcRepaint, TRUE);
    }
  }
}
