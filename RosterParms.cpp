// RosterParms.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "Roster.h"
#include "DaysOffPatterns.h"
#include "RosterParms.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int bitCount( register int mask )
{
	register int	count = 0;
	while( mask )
	{
		mask &= (mask - 1);
		++count;
	}
	return count;
}

static bool	FixDaysOffPatterns( char *daysOffPatterns, int &workWeek )
{
	// Validate that the days-off patterns have integrity.
	bool	success = true;
	int		daysOff;
	register int i;

	if( workWeek < 1 || workWeek > 6 )
	{
		workWeek = 5;
		success = false;
	}
	daysOff = 7 - workWeek;

	for( i = 0; i < 128; ++i )
	{
		if( (daysOffPatterns[i] != 'Y' && daysOffPatterns[i] != 'N') ||
			(daysOffPatterns[i] == 'Y' && bitCount(i) != daysOff) )
			break;
	}

	if( i != 128 )
	{
		for( i = 0; i < 128; ++i )
			daysOffPatterns[i] = (bitCount(i) == daysOff ? 'Y' : 'N');

		success = false;
	}

	return success;
}

static	char	*GetDaysOffPatternText( char *daysOffPatterns, int &workWeek )
{
	static	char	sz[2048];
	char	*s = sz;
	register	int i, d;
	int		entriesPerLine, e = 0;

	FixDaysOffPatterns( daysOffPatterns, workWeek );
	entriesPerLine = 20 / ((7 - workWeek) * 2);
	if( entriesPerLine < 1 )
		entriesPerLine = 1;

	*s = 0;
	for( i = 0; i < 128; ++i )
	{
		if( daysOffPatterns[i] == 'Y' )
		{
			for( d = 0; d < 7; ++d )
			{
				if( i & (1<<d) )
				{
					strncpy(s, "MoTuWeThFrSaSu" + d*2, 2 );
					s += 2;
				}
			}
			if( ++e >= entriesPerLine )	{ *s++ = '\r'; *s++ = '\n'; e = 0; }	// End the line.
			else						{ *s++ = ',';  *s++ = ' '; }			// Just separate the entry.
		}
	}

	while( s > sz )
	{
		--s;
		if( *s == ',' )
		{
			*s = 0;
			break;
		}
		if( isalnum(*s) )
		{
			*++s = 0;
			break;
		}
	}

	return sz;
}

/////////////////////////////////////////////////////////////////////////////
// CRosterParms dialog


CRosterParms::CRosterParms(CWnd* pParent /*=NULL*/)
	: CDialog(CRosterParms::IDD, pParent), pEditWORKWEEK(NULL)
{
	//{{AFX_DATA_INIT(CRosterParms)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRosterParms::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRosterParms)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRosterParms, CDialog)
	//{{AFX_MSG_MAP(CRosterParms)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ROSTERPARMS_SAMERUNTYPE, OnSameruntype)
	ON_BN_CLICKED(ROSTERPARMS_DIFFRUNTYPE, OnDiffruntype)
	ON_BN_CLICKED(ROSTERPARMS_RUNTYPERULES, OnRuntyperules)
	ON_LBN_SELCHANGE(ROSTERPARMS_RUNTYPE, OnSelchangeRuntype)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDEDIT, OnEdit)
	ON_LBN_SELCHANGE(ROSTERPARMS_COMBINE, OnSelchangeCombine)
	ON_EN_CHANGE(ROSTERPARMS_WORKWEEK, OnChangeWorkweek)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRosterParms message handlers

BOOL CRosterParms::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int     nI;
  int     nJ;
  int     nK;
//
//  Set up pointers to the controls
//	
  pButtonSAMEROUTE = (CButton *)GetDlgItem(ROSTERPARMS_SAMEROUTE);
  pButtonSAMERUNNUMBER = (CButton *)GetDlgItem(ROSTERPARMS_SAMERUNNUMBER);
  pButtonSAMESTART = (CButton *)GetDlgItem(ROSTERPARMS_SAMESTART);
  pButtonSAMEEND = (CButton *)GetDlgItem(ROSTERPARMS_SAMEEND);
  pButtonSAMERUNTYPE = (CButton *)GetDlgItem(ROSTERPARMS_SAMERUNTYPE);
  pButtonDIFFRUNTYPE = (CButton *)GetDlgItem(ROSTERPARMS_DIFFRUNTYPE);
  pButtonRUNTYPERULES = (CButton *)GetDlgItem(ROSTERPARMS_RUNTYPERULES);
  pListBoxRUNTYPE = (CListBox *)GetDlgItem(ROSTERPARMS_RUNTYPE);
  pStaticCOMBINETITLE = (CStatic *)GetDlgItem(ROSTERPARMS_COMBINETITLE);
  pListBoxCOMBINE = (CListBox *)GetDlgItem(ROSTERPARMS_COMBINE);
  pEditSTARTTIME = (CEdit *)GetDlgItem(ROSTERPARMS_STARTTIME);
  pEditOFFTIME = (CEdit *)GetDlgItem(ROSTERPARMS_OFFTIME);
  pEditWORKWEEK = (CEdit *)GetDlgItem(ROSTERPARMS_WORKWEEK);
  pButtonBIDBYDRIVERTYPE = (CButton *)GetDlgItem(ROSTERPARMS_BIDBYDRIVERTYPE);
  pEditDAYSOFFPATTERNSTEXT = (CEdit *)GetDlgItem(ROSTERPARMS_DAYSOFFPATTERNSTEXT);
  pButtonIDEDIT = (CButton *)GetDlgItem(IDEDIT);
  pButtonEQUALIZE = (CButton *)GetDlgItem(ROSTERPARMS_EQUALIZE);
  pButtonFORCEEXTRASTOWEEKDAYS = (CButton *)GetDlgItem(ROSTERPARMS_FORCEEXTRASTOWEEKDAYS);
  pButtonFORCEEXTRASTOWEEKENDS = (CButton *)GetDlgItem(ROSTERPARMS_FORCEEXTRASTOWEEKENDS);
  pEditMINWORK = (CEdit *)GetDlgItem(ROSTERPARMS_MINWORK);
  pEditMAXWORK = (CEdit *)GetDlgItem(ROSTERPARMS_MAXWORK);
  pEditNIGHTWORKAFTER = (CEdit *)GetDlgItem(ROSTERPARMS_NIGHTWORKAFTER);
  pEditMAXNIGHTWORK = (CEdit *)GetDlgItem(ROSTERPARMS_MAXNIGHTWORK);
  pButtonOFFDAYMUSTFOLLOW = (CButton *)GetDlgItem(ROSTERPARMS_OFFDAYMUSTFOLLOW);
	
//
//  Set up a local copy of the runtype match structure
//
  memcpy(m_LocalMatch, ROSTERPARMS.match, sizeof(m_LocalMatch));
  memcpy(m_DaysOffPatterns, ROSTERPARMS.daysOffPatterns, sizeof(m_DaysOffPatterns));
//
//  Set up the runtype list boxes
//
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
      {
        nK = pListBoxRUNTYPE->AddString(RUNTYPE[nI][nJ].localName);
        pListBoxRUNTYPE->SetItemData(nK, MAKELONG(nI, nJ));
        nK = pListBoxCOMBINE->AddString(RUNTYPE[nI][nJ].localName);
        pListBoxCOMBINE->SetItemData(nK, MAKELONG(nI, nJ));
      }
    }
  }
  if(pListBoxRUNTYPE->GetCount() == 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_110, (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  Set up the ROUTE, RUNNUMBER, START, and END radio selections
//
  pButtonSAMEROUTE->SetCheck(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEROUTE);
  pButtonSAMERUNNUMBER->SetCheck(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMERUNNUMBER);
  pButtonSAMESTART->SetCheck(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMESTART);
  pButtonSAMEEND->SetCheck(ROSTERPARMS.flags & ROSTERPARMS_FLAG_SAMEEND);
//
//  Set up the RUNTYPECHOICE radio selections
//
  if(ROSTERPARMS.runtypeChoice == 0)
  {
    pButtonSAMERUNTYPE->SetCheck(TRUE);
  }
  else if(ROSTERPARMS.runtypeChoice == 1)
  {
    pButtonDIFFRUNTYPE->SetCheck(TRUE);
  }
  else
  {
    pButtonRUNTYPERULES->SetCheck(TRUE);
  }
//
//  Set up the STARTTIME edit control
//
  pEditSTARTTIME->SetWindowText(chhmm(ROSTERPARMS.startVariance));
//
//  Set up the OFFTIME edit control
//
  pEditOFFTIME->SetWindowText(chhmm(ROSTERPARMS.minOffTime));
//
//  Set up the WORKWEEK edit control
//
  sprintf(tempString, "%d", ROSTERPARMS.workWeek);
  pEditWORKWEEK->SetWindowText(tempString);
//
//  Set up the BIDBYDRIVERTYPE check box
//
  if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_BIDBYDRIVERTYPE)
  {
    pButtonBIDBYDRIVERTYPE->SetCheck(TRUE);
  }
//
//  Set up the days off controls
//
  pEditDAYSOFFPATTERNSTEXT->SetWindowText(GetDaysOffPatternText(m_DaysOffPatterns, ROSTERPARMS.workWeek));
//
//  Set up the extras preferences
//
  if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_PREFEREXTRASONWEEKENDS)
  {
    pButtonFORCEEXTRASTOWEEKENDS->SetCheck(TRUE);
  }
  else
  {
    pButtonFORCEEXTRASTOWEEKDAYS->SetCheck(TRUE);
  }
//
//  Set up the work equalization checkbox
//
  if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_EQUALIZEWORK)
  {
    pButtonEQUALIZE->SetCheck(TRUE);
  }
//
//  Set up the MINWORK and MAXWORK edit controls
//
  pEditMINWORK->SetWindowText(chhmm(ROSTERPARMS.minWork));
  pEditMAXWORK->SetWindowText(chhmm(ROSTERPARMS.maxWork));
//
//  Set up the "night work" edit controls
//
  pEditNIGHTWORKAFTER->SetWindowText(Tchar(ROSTERPARMS.nightWorkAfter));
  sprintf(tempString, "%d", (ROSTERPARMS.maxNightWork > 0 ? ROSTERPARMS.maxNightWork : 0));
  pEditMAXNIGHTWORK->SetWindowText(tempString);
  if(ROSTERPARMS.flags & ROSTERPARMS_FLAG_OFFDAYMUSTFOLLOW)
  {
    pButtonOFFDAYMUSTFOLLOW->SetCheck(TRUE);
  }
	
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRosterParms::OnClose() 
{
  OnCancel();
}

void CRosterParms::OnSameruntype() 
{
  pListBoxRUNTYPE->EnableWindow(FALSE);
  pStaticCOMBINETITLE->EnableWindow(FALSE);
  pListBoxCOMBINE->EnableWindow(FALSE);
}

void CRosterParms::OnDiffruntype() 
{
  OnSameruntype();
}

void CRosterParms::OnRuntyperules() 
{
  pListBoxRUNTYPE->EnableWindow(TRUE);
  pStaticCOMBINETITLE->EnableWindow(TRUE);
  pListBoxCOMBINE->EnableWindow(TRUE);
  pListBoxRUNTYPE->SetCurSel(0);
  OnSelchangeRuntype();
}

void CRosterParms::OnSelchangeRuntype() 
{
  long tempLong;
  int  nI, nJ, nK;

  nI = pListBoxRUNTYPE->GetCurSel();
  if(nI < 0)
  {
    return;
  }
  tempLong = pListBoxRUNTYPE->GetItemData(nI);
  pListBoxCOMBINE->SetSel(-1, FALSE);
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(m_LocalMatch[LOWORD(tempLong)][HIWORD(tempLong)][nJ] & (2 << nI))
      {
        for(nK = 0; nK < pListBoxCOMBINE->GetCount(); nK++)
        {
          if((long)pListBoxCOMBINE->GetItemData(nK) == MAKELONG(nI, nJ))
          {
            pListBoxCOMBINE->SetSel(nK, TRUE);
          }
        }
      }
    }
  }
}

void CRosterParms::OnSelchangeCombine() 
{
  long tempLong;
  int  nI, nJ, nK, nL;
  int  runtypesSelected[MAXRUNTYPES];

  nI = pListBoxRUNTYPE->GetCurSel();
  if(nI == LB_ERR)
  {
    pListBoxRUNTYPE->SetCurSel(0);
    nI = 0;
  }
  tempLong = pListBoxRUNTYPE->GetItemData(nI);
  nI = LOWORD(tempLong);
  nJ = HIWORD(tempLong);
  for(nK = 0; nK < NUMRUNTYPES; nK++)
  {
    for(nL = 0; nL < NUMRUNTYPESLOTS; nL++)
    {
      if(m_LocalMatch[nI][nJ][nL] &= (2 << nK))
      {
        m_LocalMatch[nI][nJ][nL] &= ~(2 << nK);
      }
    }
  }
  nL = pListBoxCOMBINE->GetSelItems(MAXRUNTYPES, &runtypesSelected[0]);
  for(nK = 0; nK < nL; nK++)
  {
    tempLong = pListBoxCOMBINE->GetItemData(runtypesSelected[nK]);
    if(pListBoxCOMBINE->GetSel(runtypesSelected[nK]))
    {
      m_LocalMatch[nI][nJ][HIWORD(tempLong)] |= (2 << LOWORD(tempLong));
    }
    else
    {
      if(m_LocalMatch[nI][nJ][HIWORD(tempLong)] & (2 << LOWORD(tempLong)))
      {
        m_LocalMatch[nI][nJ][HIWORD(tempLong)] &= ~(2 << LOWORD(tempLong));
      }
    }
  }
}

void CRosterParms::OnCancel() 
{
	CDialog::OnCancel();
}

void CRosterParms::OnHelp() 
{
}

void CRosterParms::OnEdit() 
{
  DaysOffPatterns dlg;

  pEditWORKWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  int workWeek = atoi(tempString);
  if( workWeek < 1 || workWeek > 6 )
	  workWeek = 5;

  {
	  // Initialize the current days-off patterns from the temporary value.
	  int	patterns[128], *patternCur = patterns;
	  for( register int p = 0; p < 128; ++p )
	  {
		  if( m_DaysOffPatterns[p] == 'Y' )
			  *patternCur++ = p;
	  }
	  *patternCur = 0;

	  dlg.setDaysOffPatterns( patterns, 7 - workWeek );
  }

  if(dlg.DoModal())
  {
	// If the dialog is ok, record the results to the temporary value.
	register char *dop = m_DaysOffPatterns;
	for( register const bool *p = dlg.getDaysOffPatternFlags(), *pEnd = p + 128; p != pEnd; ++p )
	{
		*dop++ = (*p ? 'Y' : 'N');
	}

    pEditDAYSOFFPATTERNSTEXT->SetWindowText(GetDaysOffPatternText(m_DaysOffPatterns, workWeek));
  }
}

void CRosterParms::OnOK() 
{
  HCURSOR saveCursor = SetCursor(hCursorWait);
  
  ROSTERPARMS.flags = 0;
//
//  Route
//
  if(pButtonSAMEROUTE->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMEROUTE;
  }
//
//  Run Number
//
  if(pButtonSAMERUNNUMBER->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMERUNNUMBER;
  }
//
//  Start
//
  if(pButtonSAMESTART->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMESTART;
  }
//
//  End
//
  if(pButtonSAMEEND->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_SAMEEND;
  }
//
//  Bid by driver type
//
  if(pButtonBIDBYDRIVERTYPE->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_BIDBYDRIVERTYPE;
  }
//
//  Days off
//
  memcpy(ROSTERPARMS.daysOffPatterns, m_DaysOffPatterns, sizeof(m_DaysOffPatterns));
//
//  Extras
//
  if(pButtonFORCEEXTRASTOWEEKENDS->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_PREFEREXTRASONWEEKENDS;
  }
//
//  Equalization
//
  if(pButtonEQUALIZE->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_EQUALIZEWORK;
  }
//
//  Runtype
//
  if(pButtonSAMERUNTYPE->GetCheck())
  {
    ROSTERPARMS.runtypeChoice = 0;
  }
  else if(pButtonDIFFRUNTYPE->GetCheck())
  {
    ROSTERPARMS.runtypeChoice = 1;
  }
  else
  {
    ROSTERPARMS.runtypeChoice = 2;
    memcpy(ROSTERPARMS.match, m_LocalMatch, sizeof(m_LocalMatch));
  }
//
//  StartTime
//
  pEditSTARTTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.startVariance = thhmm(tempString);
//
//  OffTime
//
  pEditOFFTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.minOffTime = thhmm(tempString);
//
//  Work week
//
  pEditWORKWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.workWeek = atoi(tempString);
//
//  Min and Max Work
//
  pEditMINWORK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.minWork = thhmm(tempString);
  pEditMAXWORK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.maxWork = thhmm(tempString);
//
//  Night work
//
  pEditNIGHTWORKAFTER->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.nightWorkAfter =cTime(tempString);
  pEditMAXNIGHTWORK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  ROSTERPARMS.maxNightWork = atoi(tempString);
  if(pButtonOFFDAYMUSTFOLLOW->GetCheck())
  {
    ROSTERPARMS.flags |= ROSTERPARMS_FLAG_OFFDAYMUSTFOLLOW;
  }
//
//  Flag the change
//
  m_bWorkrulesChanged = TRUE;
  SetCursor(saveCursor);
	
	CDialog::OnOK();
}


void CRosterParms::OnChangeWorkweek() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

  // Ensure that the work week is consistent with the patterns.
  if( pEditWORKWEEK )
  {
	  pEditWORKWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);
	  int workWeek = atoi(tempString);
	  bool	corrected = false;

	  if( workWeek < 1 )
	  {
		  workWeek = 1;
		  corrected = true;
	  }
	  else if( workWeek > 6 )
	  {
		  workWeek = 6;
		  corrected = true;
	  }

	  if( corrected )
	  {
  	    sprintf(tempString, "%d", workWeek);
	    pEditWORKWEEK->SetWindowText(tempString);
	  }

	  if( !FixDaysOffPatterns(m_DaysOffPatterns, workWeek) )
		pEditDAYSOFFPATTERNSTEXT->SetWindowText(GetDaysOffPatternText(m_DaysOffPatterns, workWeek));
  }
}
