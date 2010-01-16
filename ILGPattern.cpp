// ILGPattern.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGPattern.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGPattern

IMPLEMENT_DYNAMIC(CILGPattern, CRecordset)

CILGPattern::CILGPattern(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGPattern)
	m_PatternAbbr = _T("");
	m_RouteAbbr = _T("");
	m_RouteDirectionAbbr = _T("");
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGPattern::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGPattern::GetDefaultSQL()
{
	return _T("[Pattern]");
}

void CILGPattern::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGPattern)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[PatternAbbr]"), m_PatternAbbr);
	RFX_Text(pFX, _T("[RouteAbbr]"), m_RouteAbbr);
	RFX_Text(pFX, _T("[RouteDirectionAbbr]"), m_RouteDirectionAbbr);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGPattern diagnostics

#ifdef _DEBUG
void CILGPattern::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGPattern::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
