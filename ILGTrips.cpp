// ILGTrips.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGTrips.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGTrips

IMPLEMENT_DYNAMIC(CILGTrips, CRecordset)

CILGTrips::CILGTrips(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGTrips)
	m_TripID = 0;
	m_TripNum = 0;
	m_BlockNum = 0;
	m_ServiceAbbr = _T("");
	m_RouteAbbr = _T("");
	m_RouteDirectionAbbr = _T("");
	m_PatternAbbr = _T("");
	m_BlockSequence = 0;
	m_nFields = 8;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGTrips::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGTrips::GetDefaultSQL()
{
	return _T("[Trips]");
}

void CILGTrips::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGTrips)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[TripID]"), m_TripID);
	RFX_Long(pFX, _T("[TripNum]"), m_TripNum);
	RFX_Long(pFX, _T("[BlockNum]"), m_BlockNum);
	RFX_Text(pFX, _T("[ServiceAbbr]"), m_ServiceAbbr);
	RFX_Text(pFX, _T("[RouteAbbr]"), m_RouteAbbr);
	RFX_Text(pFX, _T("[RouteDirectionAbbr]"), m_RouteDirectionAbbr);
	RFX_Text(pFX, _T("[PatternAbbr]"), m_PatternAbbr);
	RFX_Long(pFX, _T("[BlockSequence]"), m_BlockSequence);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGTrips diagnostics

#ifdef _DEBUG
void CILGTrips::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGTrips::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
