// DaysOffPatterns.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "DaysOffPatterns.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DaysOffPatterns dialog
static int bitCount( register unsigned int mask )
{
	register int	count = 0;
	while( mask )
	{
		mask &= (mask - 1);
		++count;
	}
	return count;
}

static  const char  szDays[7][4] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char *szSeparator = " / ";
const int sepLen = strlen(szSeparator);
static char	*patternToText( const int p )
{
	static	char	sz[4 * 7 + 1];
	char	*s = sz;
	for( register int i = 0; i < 7; ++i )
	{
		if( p & (1<<i) )
		{
			strcpy(s, szDays[i] );
			s += strlen(szDays[i]);
			strcpy( s, szSeparator );
			s += sepLen;
		}
	}
	s -= sepLen;
	*s = 0;
	return sz;
}

void	DaysOffPatterns::setDaysOffPatterns( const int *aPatterns, const int aNumDaysOff )
{
	if( aNumDaysOff > 0 && aNumDaysOff < 7 )
		numDaysOff = aNumDaysOff;
	else
		numDaysOff = 2;

	std::fill( patternFlags, patternFlags + 128, false );

	bool	validPattern = false;
	for( register const int *p = aPatterns; *p; ++p )
	{
		if( bitCount(*p) == numDaysOff )
		{
			patternFlags[*p] = true;
			validPattern = true;
		}
	}

	// If we get passed nothing or junk, set all possible days-off combinations.
	if( !validPattern )
	{
		for( register int i = 0; i < 128; ++i )
			patternFlags[i] = (bitCount(i) == numDaysOff);
	}
}

const int *DaysOffPatterns::getDaysOffPatterns()
{
	register int *p = patterns;
	register int i;
	for( i = 0; i < 128; ++i )
	{
		if( patternFlags[i] )
			*p++ = i;
	}
	*p = 0;

	// If nothing is selected, just select everything by default.
	if( p == patterns )
	{
		for( i = 0; i < 128; ++i )
		{
			if( bitCount(i) == numDaysOff )
			{
				*p++ = i;
				patternFlags[i] = true;
			}
		}
		*p = 0;
	}

	return patterns;
}

const bool *DaysOffPatterns::getDaysOffPatternFlags()
{
	getDaysOffPatterns();
	return patternFlags;
}

void	DaysOffPatterns::updateUI()
{
	// Make the UI reflect the pattern flags.
	pListBoxSELECTEDDAYSOFF->ResetContent();
	pListBoxAVAILABLEDAYSOFF->ResetContent();
	for( int p = 0; p < 128; ++p )
	{
		if( bitCount(p) == numDaysOff )
		{
			if( patternFlags[p] )
			{
				pListBoxSELECTEDDAYSOFF->AddString( patternToText(p) );
				pListBoxSELECTEDDAYSOFF->SetItemData( pListBoxSELECTEDDAYSOFF->GetCount()-1, p );
			}
			else
			{
				pListBoxAVAILABLEDAYSOFF->AddString( patternToText(p) );
				pListBoxAVAILABLEDAYSOFF->SetItemData( pListBoxAVAILABLEDAYSOFF->GetCount()-1, p );
			}
		}
	}
}

DaysOffPatterns::DaysOffPatterns(CWnd* pParent /*=NULL*/)
	: CDialog(DaysOffPatterns::IDD, pParent)
{
	//{{AFX_DATA_INIT(DaysOffPatterns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DaysOffPatterns::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DaysOffPatterns)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DaysOffPatterns, CDialog)
	//{{AFX_MSG_MAP(DaysOffPatterns)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDSELECTDAYSOFF, OnSelectdaysoff)
	ON_BN_CLICKED(IDDESELECTDAYSOFF, OnDeselectdaysoff)
	ON_BN_CLICKED(IDCONSECUTIVEDAYSONLY, OnConsecutivedaysonly)
	ON_BN_CLICKED(IDCONSECUTIVEDAYSIFWEEKDAY, OnConsecutivedaysifweekday)
	ON_BN_CLICKED(IDALLVALIDDAYSOFF, OnAllvaliddaysoff)
	ON_BN_CLICKED(IDATLEAST2CONSECUTIVEDAYSOFF, OnAtleast2consecutivedaysoff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DaysOffPatterns message handlers

BOOL DaysOffPatterns::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

#ifdef FIXLATER
	CWinApp* pApp = AfxGetApp();
    m_hIconMoveRight = pApp->LoadIcon( IDI_MOVERIGHT );
	m_hIconMoveLeft = pApp->LoadIcon( IDI_MOVELEFT );

	((CButton *)GetDlgItem(IDC_SELECTDAYSOFF))->SetIcon( m_hIconMoveRight );
	((CButton *)GetDlgItem(IDC_DESELECTDAYSOFF))->SetIcon( m_hIconMoveLeft );
#endif

	pListBoxAVAILABLEDAYSOFF = (CListBox *)GetDlgItem(DAYSOFFPATTERNS_AVAILABLEDAYSOFF);
	pListBoxSELECTEDDAYSOFF = (CListBox *)GetDlgItem(DAYSOFFPATTERNS_SELECTEDDAYSOFF);

	updateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DaysOffPatterns::OnClose() 
{
  OnCancel();
}

void DaysOffPatterns::OnCancel() 
{
	CDialog::OnCancel();
}

void DaysOffPatterns::OnHelp() 
{
}

void DaysOffPatterns::OnSelectdaysoff() 
{
	int i, iMax = pListBoxAVAILABLEDAYSOFF->GetCount();
	for( i = 0; i < iMax; ++i )
	{
		if( pListBoxAVAILABLEDAYSOFF->GetSel(i) )
			patternFlags[pListBoxAVAILABLEDAYSOFF->GetItemData(i)] = true;
	}
	updateUI();
}

void DaysOffPatterns::OnDeselectdaysoff() 
{
	int i, iMax = pListBoxSELECTEDDAYSOFF->GetCount();
	for( i = 0; i < iMax; ++i )
	{
		if( pListBoxSELECTEDDAYSOFF->GetSel(i) )
			patternFlags[pListBoxSELECTEDDAYSOFF->GetItemData(i)] = false;
	}
	updateUI();
}

void DaysOffPatterns::OnConsecutivedaysonly() 
{
	for( int p = 0; p < 128; ++p )
	{
		patternFlags[p] = false;
		if( bitCount(p) != numDaysOff )
			continue;

		register int mask = (1<<numDaysOff) - 1;
		
		for( register int d = 0; d < 7; ++d )
		{
			if( (mask & p) == mask )
			{
				patternFlags[p] = true;
				break;
			}
			// Circularly shift the mask.
			if( (mask <<= 1) & 128 )
			{
				mask |= 1;
				mask &= 127;
			}
		}
	}
	updateUI();
}

void DaysOffPatterns::OnConsecutivedaysifweekday() 
{
	for( int p = 0; p < 128; ++p )
	{
		patternFlags[p] = false;
		if( bitCount(p) != numDaysOff )
			continue;

		// Accept this pattern if it contains a weekend day.
		if( (p & (1<<5)) || (p & (1<<6)) )
		{
			patternFlags[p] = true;
			continue;
		}

		// Otherwise, check that all days off are consecutive.
		register int mask = (1<<numDaysOff) - 1;
		
		for( register int d = 0; d < 7; ++d )
		{
			if( (mask & p) == mask )
			{
				patternFlags[p] = true;
				break;
			}
			// Circularly shift the mask.
			if( (mask <<= 1) & 128 )
			{
				mask |= 1;
				mask &= 127;
			}
		}
	}
	updateUI();
}

void DaysOffPatterns::OnOK() 
{
	// Copy the results back into the roster parameters.
	fixDaysOffPatterns();

	CDialog::OnOK();
}

void DaysOffPatterns::OnAllvaliddaysoff() 
{
	// TODO: Add your control notification handler code here
	for( int p = 0; p < 128; ++p )
	{
		patternFlags[p] = (bitCount(p) == numDaysOff);
	}
	updateUI();	
}

void DaysOffPatterns::OnAtleast2consecutivedaysoff() 
{
	// TODO: Add your control notification handler code here
	for( int p = 0; p < 128; ++p )
	{
		patternFlags[p] = false;
		if( bitCount(p) != numDaysOff )
			continue;

		// Check that there are at least 2 consecutive days off.
		register int mask = 3;
		
		for( register int d = 0; d < 7; ++d )
		{
			if( (mask & p) == mask )
			{
				patternFlags[p] = true;
				break;
			}
			// Circularly shift the mask.
			if( (mask <<= 1) & 128 )
			{
				mask |= 1;
				mask &= 127;
			}
		}
	}
	updateUI();	
}
